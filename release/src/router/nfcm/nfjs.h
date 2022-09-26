#ifndef __NFJS_H__
#define __NFJS_H__

#include "list.h"

#define JSON_OUTPUT_APP_FILE "/jffs/nfcm_app.json"
#define JSON_OUTPUT_SUM_FILE "/jffs/nfcm_sum.json"
#define JSON_OUTPUT_TCP_FILE "/jffs/nfcm_tcp.json"

extern int nf_list_to_json(struct list_head *iplist, struct list_head *arlist);
extern int nf_list_statistics_to_json(struct list_head *smlist, struct list_head *arlist);
extern int nf_list_tcp_to_json(struct list_head *tcplist);

#endif /* __NFJS_H__ */
