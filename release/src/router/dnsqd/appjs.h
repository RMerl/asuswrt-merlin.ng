#ifndef __APPJS_H__
#define __APPJS_H__

#include "list.h"
#include "dnssql.h"

extern int appstats_list_to_json(struct list_head *list);
extern int block_history_list_to_json(struct list_head *list);
extern int block_entry_list_to_json(struct list_head *list);

extern int device_query_list_to_json(struct list_head *list);

#endif /* __APPJS_H__ */