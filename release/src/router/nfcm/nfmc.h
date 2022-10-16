#ifndef __NFMC_H__
#define __NFMC_H__

#include "list.h"
#include "log.h"

extern void mc_list_free(struct list_head *list);
extern int mc_list_parse(struct list_head *list);

#endif // __NFMC_H__
