#include "sysmgr.h"
#include <malloc.h>

void sysmgr_init(SystemManager *mgr) {
    g_mutex_init(&mgr->lock);
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
    malloc_trim(0);
    g_mutex_unlock(&mgr->lock);
    return stdout_data;
}
