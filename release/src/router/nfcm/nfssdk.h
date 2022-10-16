#ifndef __NFSSDK_H__
#define __NFSSDK_H__

#include "list.h"
#include "log.h"

#include "ssdk_init.h"
#include "shell.h"
#include "sw_error.h"
#include "fal_uk_if.h"
#include <fal_fdb.h>

extern int qca_fdb_mac_table_get(ethsw_mac_table *tbl);

#endif // __NFSSDK_H__
