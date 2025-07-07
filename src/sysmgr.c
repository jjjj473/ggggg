#include "sysmgr.h"
#include <malloc.h>

void sysmgr_init(SystemManager *mgr) {
    g_mutex_init(&mgr->lock);
    mgr->history_pos = 0;
    for (int i = 0; i < SYSMGR_HISTORY_SIZE; i++) {
        mgr->history[i] = NULL;
    }
}

void sysmgr_clear(SystemManager *mgr) {
    for (int i = 0; i < SYSMGR_HISTORY_SIZE; i++) {
        g_free(mgr->history[i]);
        mgr->history[i] = NULL;
    }
    mgr->history_pos = 0;
}

char *sysmgr_run(SystemManager *mgr, const char *cmd, GError **error) {
    g_mutex_lock(&mgr->lock);
    gchar *stdout_data = NULL;
    gint exit_status = 0;
    gboolean ok = g_spawn_command_line_sync(cmd, &stdout_data, NULL,
                                            &exit_status, error);
    if (!ok) {
        g_mutex_unlock(&mgr->lock);
        return NULL;
    }
    if (exit_status != 0) {
        if (error && *error == NULL) {
            *error = g_error_new(g_quark_from_static_string("sysmgr"),
                                 exit_status,
                                 "Process exited with status %d", exit_status);
        }
        g_free(stdout_data);
        stdout_data = NULL;
    }
    g_free(mgr->history[mgr->history_pos]);
    mgr->history[mgr->history_pos] = g_strdup(cmd);
    mgr->history_pos = (mgr->history_pos + 1) % SYSMGR_HISTORY_SIZE;

    malloc_trim(0);
    g_mutex_unlock(&mgr->lock);
    return stdout_data;
}
