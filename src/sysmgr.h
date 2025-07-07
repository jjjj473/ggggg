#ifndef SYSMGR_H
#define SYSMGR_H

#include <glib.h>


#define SYSMGR_HISTORY_SIZE 70

typedef struct {
    GMutex lock;
    char *history[SYSMGR_HISTORY_SIZE];
    int history_pos;
} SystemManager;

void sysmgr_clear(SystemManager *mgr);

void sysmgr_init(SystemManager *mgr);
char *sysmgr_run(SystemManager *mgr, const char *cmd, GError **error);

#endif
