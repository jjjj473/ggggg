#ifndef SYSMGR_H
#define SYSMGR_H

#include <glib.h>

typedef struct {
    GMutex lock;
} SystemManager;

void sysmgr_init(SystemManager *mgr);
char *sysmgr_run(SystemManager *mgr, const char *cmd, GError **error);

#endif
