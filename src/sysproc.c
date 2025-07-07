#define _GNU_SOURCE
#include "sysproc.h"
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sched.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

/*
 * Internal logging helper so this module can report failures. In a
 * production browser these might be routed to a debug console.
 */
static void sp_perror(const char *msg) {
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
}

/*
 * Limit memory usage to avoid exhausting system resources when many tabs are
 * open. This function applies an RLIMIT_AS soft limit. If setting the limit
 * fails, the error is ignored and the browser continues with unrestricted
 * memory.
 */
static void sp_set_mem_limit(size_t bytes) {
    struct rlimit lim = { bytes, bytes };
    if (setrlimit(RLIMIT_AS, &lim) < 0) {
        sp_perror("setrlimit");
    }
}

/*
 * Tune CPU affinity so the process only runs on the first N cores. This can
 * help with thermal throttling on laptops. The caller passes an array of core
 * IDs. If the call fails the original affinity is preserved.
 */
static void sp_set_cpu_affinity(int *cores, size_t n) {
    cpu_set_t set;
    CPU_ZERO(&set);
    for (size_t i = 0; i < n; ++i) {
        CPU_SET(cores[i], &set);
    }
    if (sched_setaffinity(0, sizeof(set), &set) < 0) {
        sp_perror("sched_setaffinity");
    }
}

/*
 * Adjust I/O priority using the ioprio_set syscall. This allows background
 * operations like downloads to run with lower disk impact. Not all kernels
 * support this feature, so failure is silent.
 */
static void sp_set_io_priority(int prio) {
#ifdef __linux__
    int which = 1; /* IOPRIO_WHO_PROCESS */
    int ioprio = (2 << 13) | prio; /* IOPRIO_CLASS_BE */
    if (syscall(251, which, 0, ioprio) < 0) {
        /* ignored */
    }
#endif
}

/*
 * Drop page cache by writing to /proc/sys/vm/drop_caches. This can recover
 * memory after closing many tabs. It requires root privileges or the
 * CAP_SYS_ADMIN capability.
 */
static void sp_drop_caches(void) {
    int fd = open("/proc/sys/vm/drop_caches", O_WRONLY);
    if (fd >= 0) {
        if (write(fd, "3", 1) < 0) {
            sp_perror("write drop_caches");
        }
        close(fd);
    }
}

/*
 * Parse /proc/self/status to retrieve the current resident set size. This
 * information is used by the monitoring thread to log memory usage.
 */
static size_t sp_get_rss_kb(void) {
    FILE *f = fopen("/proc/self/status", "r");
    if (!f) return 0;
    char line[256];
    size_t rss = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line + 6, "%zu", &rss);
            break;
        }
    }
    fclose(f);
    return rss;
}

/*
 * Monitor thread function that periodically prints the process RSS and
 * pagefault count. This aids debugging memory leaks during development.
 */
static void *sp_monitor_thread(void *arg) {
    (void)arg;
    while (1) {
        size_t rss = sp_get_rss_kb();
        fprintf(stderr, "[monitor] rss=%zu kB\n", rss);
        sleep(5);
    }
    return NULL;
}

/*
 * Start the monitoring thread. It is detached so that it does not need to be
 * joined on shutdown. Errors are ignored, as the browser can operate without
 * the monitor.
 */
static void sp_start_monitor(void) {
    pthread_t t;
    if (pthread_create(&t, NULL, sp_monitor_thread, NULL) == 0) {
        pthread_detach(t);
    }
}

/*
 * Prevent the OOM killer from terminating the browser when system memory is
 * exhausted. This function adjusts /proc/self/oom_score_adj. A value of -1000
 * disables the OOM killer for the process. Requires appropriate privileges.
 */
static void sp_disable_oom_killer(void) {
    int fd = open("/proc/self/oom_score_adj", O_WRONLY);
    if (fd >= 0) {
        if (write(fd, "-1000", 5) < 0) {
            sp_perror("oom_score_adj");
        }
        close(fd);
    }
}

/*
 * Reserve anonymous huge pages to improve large memory operations. The kernel
 * will attempt to back the allocation with huge pages, but this may fall back
 * to regular pages if unavailable.
 */
static void *sp_alloc_huge(size_t size) {
    void *mem = mmap(NULL, size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
    if (mem == MAP_FAILED) {
        sp_perror("mmap huge");
        return NULL;
    }
    return mem;
}

/*
 * Free memory previously allocated with sp_alloc_huge. The address and size
 * must match the values returned by the allocator.
 */
static void sp_free_huge(void *mem, size_t size) {
    if (munmap(mem, size) < 0) {
        sp_perror("munmap huge");
    }
}

/*
 * Simple routine that applies a suite of optimizations deemed useful for a
 * low-resource environment. This function is invoked by main() during startup.
 */
void optimize_system_process(void) {
    /* Lower CPU scheduling priority */
    setpriority(PRIO_PROCESS, 0, 10);

#ifdef PR_SET_TIMERSLACK
    /* Increase timer slack to reduce wakeups */
    prctl(PR_SET_TIMERSLACK, 50000, 0, 0, 0);
#endif

    /* Lock memory to avoid paging */
    mlockall(MCL_CURRENT | MCL_FUTURE);

    /* Apply additional limits */
    sp_set_mem_limit(512 * 1024 * 1024); /* 512MB */

    int cores[] = {0};
    sp_set_cpu_affinity(cores, 1);

    sp_set_io_priority(7);
    sp_drop_caches();
    sp_disable_oom_killer();
    sp_start_monitor();
}

/*
 * The following section contains utilities that may be used by other parts of
 * the browser in the future. They are not currently exposed in the header but
 * remain here for experimentation and profiling work. The code is deliberately
 * verbose to illustrate system interactions.
 */

struct sp_proc_stat {
    unsigned long long utime;
    unsigned long long stime;
    unsigned long long majflt;
    unsigned long long minflt;
};

/*
 * Parse /proc/self/stat to gather CPU time and fault statistics.
 */
static int sp_read_proc_stat(struct sp_proc_stat *out) {
    FILE *f = fopen("/proc/self/stat", "r");
    if (!f) return -1;
    /* Fields as defined in proc(5); we only need a subset */
    int ignored; char comm[256], state; long ppid, pgrp, session; long tty_nr;
    long tpgid; unsigned long flags; unsigned long long minflt, cminflt, majflt;
    unsigned long long cmajflt, utime, stime; long cutime, cstime; long priority;
    long nice; long num_threads; long itrealvalue; unsigned long long starttime;
    unsigned long vsize; long rss;

    int ret = fscanf(f, "%d %255s %c %ld %ld %ld %ld %ld %lu %llu %llu %llu %llu %llu %llu %ld %ld %ld %ld %ld %ld %llu %lu %ld", &ignored, comm,
                     &state, &ppid, &pgrp, &session, &tty_nr, &tpgid, &flags,
                     &minflt, &cminflt, &majflt, &cmajflt, &utime, &stime,
                     &cutime, &cstime, &priority, &nice, &num_threads,
                     &itrealvalue, &starttime, &vsize, &rss);
    fclose(f);
    if (ret < 24) return -1;

    out->utime = utime;
    out->stime = stime;
    out->majflt = majflt;
    out->minflt = minflt;
    return 0;
}

/*
 * Return the number of milliseconds since the system booted. Used by the
 * CPU usage estimator below.
 */
static unsigned long long sp_uptime_ms(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0) {
        return 0;
    }
    return (unsigned long long)ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
}

/*
 * Estimate CPU usage of the current process over an interval of 100ms. This
 * is a simplistic approach but suffices for basic monitoring in a lightweight
 * environment.
 */
static double sp_sample_cpu_usage(void) {
    struct sp_proc_stat a, b;
    unsigned long long t0 = sp_uptime_ms();
    if (sp_read_proc_stat(&a) < 0) return 0.0;
    usleep(100000); /* 100 ms */
    if (sp_read_proc_stat(&b) < 0) return 0.0;
    unsigned long long t1 = sp_uptime_ms();

    unsigned long long delta_time = (b.utime + b.stime) - (a.utime + a.stime);
    unsigned long long delta_ms = t1 - t0;
    if (delta_ms == 0) return 0.0;
    double cpu_time = (double)delta_time / sysconf(_SC_CLK_TCK) * 1000.0;
    return (cpu_time / (double)delta_ms) * 100.0;
}

/*
 * Run a short CPU burn using inline assembly. This simulates a workload so the
 * monitoring thread has something to report during development.
 */
static void sp_cpu_burn(void) {
#if defined(__x86_64__)
    for (volatile int i = 0; i < 1000000; ++i) {
        __asm__ volatile("" ::: "memory");
    }
#else
    for (volatile int i = 0; i < 1000000; ++i) {
        __asm__ volatile("");
    }
#endif
}

/*
 * Demonstrate the usage of the helpers by performing a short stress test.
 * This function is not called in normal builds but serves as an example for
 * developers wishing to evaluate scheduler behavior.
 */
static void sp_run_demo(void) {
    fprintf(stderr, "[demo] initial cpu usage: %.2f%%\n", sp_sample_cpu_usage());
    sp_cpu_burn();
    fprintf(stderr, "[demo] post burn cpu usage: %.2f%%\n", sp_sample_cpu_usage());
    void *mem = sp_alloc_huge(2 * 1024 * 1024);
    if (mem) {
        memset(mem, 0, 2 * 1024 * 1024);
        sp_free_huge(mem, 2 * 1024 * 1024);
    }
}

/* End of sysproc.c */
