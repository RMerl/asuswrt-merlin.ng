#ifndef __CODB_CONFIG_H__
#define __CODB_CONFIG_H__

#include "list.h"

typedef struct codb_config {
    sqlite3* pdb;
    int enable_debug;
    struct list_head list;
} codb_config_t;

extern int cosql_set_config(sqlite3* pdb, codb_config_t* cfg);

extern codb_config_t* cosql_get_config(sqlite3* pdb);

#endif