#ifndef __NFFC_H__
#define __NFFC_H__

#include "list.h"
#include "nfcm.h"

/* conntrack attributes */
enum fc_node_attr {
    FC_ATTR_OBJ_ID = 0,    /* obj_id */
    FC_ATTR_IDLE,
    FC_ATTR_SW_HIT,
    FC_ATTR_SW_TOT_HITS,
    FC_ATTR_TOT_BYTES,
    FC_ATTR_HW_TPL,
#if defined(ARCHER)
    FC_ATTR_FHW_IDX,
#endif
    FC_ATTR_HW_TOT_HITS,
    FC_ATTR_DL_CONNT,
    FC_ATTR_PD_CONNT,
    FC_ATTR_LAYER1_TYPE,   /* eth or wireless */
    FC_ATTR_LAYER1_PORT,   /* switch port num */
    FC_ATTR_PROTO,         /* tcp, udp, sctp */
    FC_ATTR_IPV4_SRC,      /* src ip and port */
    FC_ATTR_IPV4_DST,      /* dst ip and port */
    FC_ATTR_VLAN0,
    FC_ATTR_VLAN1,
    FC_ATTR_TAG,
    FC_ATTR_IQ_PRIO,
    FC_ATTR_SKB_MARK,
    FC_ATTR_TCP_PURE_ACK,  /* tcp ack or not */

    FC_ATTR_MAX
};

extern int fc_list_parse(char *fname, struct list_head *fclist);
extern void fc_list_free(struct list_head *fclist);

#endif // __NFFC_H__
