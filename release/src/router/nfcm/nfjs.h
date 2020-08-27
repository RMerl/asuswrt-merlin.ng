#ifndef __NFJS_H__
#define __NFJS_H__

#include "list.h"

extern int nf_list_to_json(struct list_head *iplist, struct list_head *arlist);
extern int nf_list_statistics_to_json(struct list_head *smlist, struct list_head *arlist);

#endif /* __NFJS_H__ */
