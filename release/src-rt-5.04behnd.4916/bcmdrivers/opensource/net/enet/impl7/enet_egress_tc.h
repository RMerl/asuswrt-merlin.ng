/*
   <:copyright-BRCM:2015:DUAL/GPL:standard

      Copyright (c) 2015 Broadcom 
      All Rights Reserved

   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:

      As a special exception, the copyright holders of this software give
      you permission to link this software with independent modules, and
      to copy and distribute the resulting executable under terms of your
      choice, provided that you also meet, for each linked independent
      module, the terms and conditions of the license of that module.
      An independent module is a module which is not derived from this
      software.  The special exception does not apply to any modifications
      of the software.

   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.

   :>
 */

/*
 *  Created on: Oct/2023
 *      Author: stefan.chulski@broadcom.com
 */ 
#ifndef __ENET_EGRESS_TC__
#define __ENET_EGRESS_TC__

#include <linux/netdevice.h>
#include <net/pkt_sched.h>
#include <net/pkt_cls.h>

#ifndef UINT32_MAX
#define UINT32_MAX		0xFFFFFFFF
#endif

#define BYTES_TO_BITS(bytes) ((bytes) * 8)

#define egress_tm_err(fmt, ...) pr_err("EGRESS_TM err %s:L%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define egress_tm_warn(fmt, ...) pr_warn("EGRESS_TM warn %s:L%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define egress_tm_info(fmt, ...) pr_info("EGRESS_TM info: " fmt, ##__VA_ARGS__)

#define EGRESS_TM_TCONT_ID_MASK        0xFFFF
#define EGRESS_TM_HANDLE_ID_MASK       0xFFFFFFFF
#define EGRESS_TM_HANDLE_ID_LOW_MASK   0xFFFF
#define EGRESS_TM_HANDLE_ID_HIGH_MASK  0xFFFF0000
#define EGRESS_TM_QUEUE_OFFSET_MASK     0xFF0000
#define EGRESS_TM_HANDLE_ID_HIGH_OFF   16

#define EGRESS_TM_HANDLE_BUILD_HANDLE(par, queue)  ((((u64)(par) & EGRESS_TM_HANDLE_ID_LOW_MASK) << EGRESS_TM_HANDLE_ID_HIGH_OFF) | ((u64)(queue) & EGRESS_TM_HANDLE_ID_LOW_MASK))
#define EGRESS_TM_HANDLE_ID_HIGH(handle)           (((handle) & EGRESS_TM_HANDLE_ID_HIGH_MASK) >> EGRESS_TM_HANDLE_ID_HIGH_OFF)
#define EGRESS_TM_QUEUE_OFFSET(handle)              (((handle) & EGRESS_TM_QUEUE_OFFSET_MASK) >> EGRESS_TM_HANDLE_ID_HIGH_OFF)
#define EGRESS_TM_HANDLE_ID2QUEUE(handle)          (((handle) & EGRESS_TM_HANDLE_ID_LOW_MASK))
#define EGRESS_TM_HANDLE_ETS_PAR_ID(handle)        ((handle) & EGRESS_TM_HANDLE_ID_HIGH_MASK)

#define EGRESS_MAX_SPEEDS     6
#define EGRESS_MAX_PRIO       16
#define EGRESS_MAX_DEF_SPEEDS (EGRESS_MAX_PRIO * (EGRESS_DEV_MAX - 1) * EGRESS_MAX_SPEEDS)

#define EGRESS_TM_QDISC_ID       0
#define EGRESS_TM_CLASS_ID       BIT(1)
#define EGRESS_TM_HANDLE_TYPE_OFFS     56
#define EGRESS_TM_HANDLE_SUB_ID_OFFS   60
#define EGRESS_TM_HANDLE(id, type) ((id) | ((u64)(type) << EGRESS_TM_HANDLE_TYPE_OFFS))
#define EGRESS_TM_SUBID(id, type)  ((id) | ((u64)(type) << EGRESS_TM_HANDLE_SUB_ID_OFFS))
#define EGRESS_TM_HAS_SUBID(id)     (((id) >> EGRESS_TM_HANDLE_SUB_ID_OFFS) & 0xF)
#define EGRESS_TM_WRED_CLASSES 2
#define EGRESS_TM_FLOAT2PERC(float)  (100 / (UINT32_MAX / float))

#define EGRESS_TM_DF_RES_BUFFERS     1
#define EGRESS_TM_DF_QUEUE_THRESHOLD EGRESS_DEF_Q_SIZE_1_MB

#define QUEUE_SIZE_FACTOR           1536
#define EGRESS_DEF_Q_SIZE_1_MB      (1024 * 1024)
#define EGRESS_DEF_Q_SIZE_4_MB      (4 * (EGRESS_DEF_Q_SIZE_1_MB))

#define EGRESS_DEF_ETH_Q_SZ_LE_EPON (130 * QUEUE_SIZE_FACTOR) /* ~200K bytes */

#define EGRESS_DEF_SPEED_KEY_SPEED_OFF    0
#define EGRESS_DEF_SPEED_KEY_SPEED_MASK   0xFFFFFFFF
#define EGRESS_DEF_SPEED_KEY_TYPE_OFF     32
#define EGRESS_DEF_SPEED_KEY_TYPE_MASK    0xFF
#define EGRESS_DEF_SPEED_KEY_PRIO_OFF     40
#define EGRESS_DEF_SPEED_KEY_PRIO_MASK    0xFF
#define EGRESS_DEF_SPEED_KEY(type, speed, prio) \
            (((u64)((type) & EGRESS_DEF_SPEED_KEY_TYPE_MASK) << EGRESS_DEF_SPEED_KEY_TYPE_OFF) | \
             ((u64)((speed) & EGRESS_DEF_SPEED_KEY_SPEED_MASK) << EGRESS_DEF_SPEED_KEY_SPEED_OFF) | \
             ((u64)((prio & EGRESS_DEF_SPEED_KEY_PRIO_MASK)) << EGRESS_DEF_SPEED_KEY_PRIO_OFF))

#define EGRESS_TM_DUMP_FLAG_SHORT   BIT(0)
#define EGRESS_TM_DUMP_FLAG_CAP     BIT(1)

#define EGRESS_TM_CONTROL_LLID_OFFS 0x1000

typedef enum {
   EGRESS_TM_PON,
   EGRESS_TM_TCONT,/* used also for EPON LLID */
   EGRESS_TM_ETS_PARENT,
   EGRESS_TM_ETS_CHILD,
   EGRESS_TM_HTB_PARENT,
   EGRESS_TM_HTB_CHILD,
   EGRESS_TM_RED,
   EGRESS_TM_GRED,
   EGRESS_TM_PIE,
   EGRESS_TM_DUALQ_LAQM
} egress_tm_node_type;

#define EGRESS_IS_DROP_ALGO(type) (type == EGRESS_TM_RED || type == EGRESS_TM_GRED || type == EGRESS_TM_PIE || \
                                   type == EGRESS_TM_DUALQ_LAQM)

struct egress_tm_node_dict {
   const char *name;
   egress_tm_node_type type;
};

typedef enum {
   EGRESS_TM_SP,
   EGRESS_TM_WRR,
   EGRESS_TM_SP_WRR
} egress_tm_node_sched_mode;

typedef enum {
   EGRESS_NODE_NEW,
   EGRESS_NODE_CONF,
   EGRESS_NODE_CHANGED
} egress_tm_node_state;

typedef enum {
   EGRESS_DEV_ETH,
   EGRESS_DEV_EPON,
   EGRESS_DEV_GPON,
   EGRESS_DEV_SVCQ,
   EGRESS_DEV_MAX
} egress_tm_port_type;

typedef enum {
   EGRESS_TM_TYPE_NONE,
   EGRESS_TM_TYPE_CONTROL,
   EGRESS_TM_TYPE_DUALQ_PI2
} egress_tm_type;

struct egress_tm_speed_dict {
   egress_tm_port_type type;
   u32 speed;
   u8 prio;
   u32 queue_size;
   struct hlist_node hnode;
};

struct egress_tm_pon_child_data {
   int id;
   u8 control;
};

struct egress_tm_ets_data {
   int64_t index;
   unsigned int num_queues;
   unsigned int num_sp_queues;
   unsigned int num_subsidiaries;
   unsigned int level;
   egress_tm_type type;
   struct gnet_stats_basic_packed sent;
   struct gnet_stats_basic_packed dropped;
   egress_tm_node_sched_mode tm_sched_mode;
};

typedef struct {
   u32 min;
   u32 max;
   u32 drop_prob;
} egress_tm_priority_class;

struct egress_tm_red_data {
   egress_tm_priority_class high_class;
   egress_tm_priority_class low_class;
};

struct egress_tm_ets_q_data {
   u8 priomap;
   u8 has_subsidiaries;
   u16 qid; /* queue identification(queue_id) */
   u16 priority; /* place of queue in Egress TM */
   u32 size;
   u32 reserved_packet_buffers;
   struct gnet_stats_basic_packed sent;
   struct gnet_stats_basic_packed dropped;
   unsigned int quanta;
   unsigned int weights;
};

struct egress_tm_htb_data {
   int64_t index;
   u64 rate;
   u64 ceil;
};

typedef struct egress_tm_node {
   struct egress_tm_node *parent;
   u64 parentid;
   u64 handle;
   egress_tm_node_state state; 
   egress_tm_node_type type;
   struct list_head my_list; /* list handle of this node */
   u16 num_of_childs;
   struct list_head childs;
   struct hlist_node hnode;
   union {
      struct egress_tm_pon_child_data ponc;
      struct egress_tm_ets_q_data queue;
      struct egress_tm_ets_data ets;
      struct egress_tm_htb_data htb;
      struct egress_tm_red_data red;
   };
} egress_tm_node;

typedef struct egress_tm_parms
{
   uint32_t schedCaps;   /* A bitmap indicating the queue scheduler
                          * capability of this port. Each bit denotes
                          * a scheduling capability defined by constants
                          * TMCTL_SP_CAPABLE, TMCTL_WRR_CAPABLE, etc.
                          */
   int      maxQueues;   /* Max number of queues supported by this port */
   int      maxSpQueues; /* Max number of SP queues allowed by the
                          * queue scheduler when co-exist with other
                          * WRR queues on this port.
                          */
   bool     portShaper;  /* Boolean to indicate whether port rate
                          * shaping is supported by this port.
                          */
   bool     queueShaper; /* Boolean to indicate whether queue rate
                          * shaping is supported by this port.
                          */
   bool     dualRate;    /* Boolean to indicate whether dual rate
                          * is supported by this port.
                          */
   uint32_t cfgFlags;    /* Port TM actual initialization config flags.
                          * See bit definitions above. (dynamic variable)
                          */
   int      numQueues;   /* Port TM actual number of queues set at initialization.
                          * supported only for PON. (dynamic variable)
                          */
   bool     dpiQueueExt; /* Boolean to indicate whether DPI queues extention for
                          * best effort queue is supported by this port
                          */
} egress_tm_parms;

/* Port TM scheduling capability */
#define EGRESS_TM_SP_CAPABLE               BIT(0)
#define EGRESS_TM_WRR_CAPABLE              BIT(1)
#define EGRESS_TM_SP_WRR_CAPABLE           BIT(4)
#define EGRESS_TM_1LEVEL_CAPABLE           BIT(6)

#define EGRESS_TM_CAP_SCHED_TYPE_SP_WRR   0x00000000
#define EGRESS_TM_CAP_SCHED_TYPE_SP       0x00000100
#define EGRESS_TM_CAP_SCHED_TYPE_WRR      0x00000200

struct port_police_entry {
    struct list_head list;
    unsigned long cookie;
    int index;
};

int enet_setup_mqprio(struct net_device *dev, struct tc_mqprio_qopt_offload *mqprio);
int enet_setup_policer_tc_block(struct net_device *dev, void *type_data);
int enet_setup_pon(struct net_device *dev, struct tc_pon_qopt_offload *qopt);
int enet_setup_ets(struct net_device *dev, struct tc_ets_qopt_offload *qopt);
int enet_setup_htb(struct net_device *dev, struct tc_htb_qopt_offload *qopt);
int enet_setup_red(struct net_device *dev, struct tc_red_qopt_offload *qopt);
int enet_setup_gred(struct net_device *dev, struct tc_gred_qopt_offload *qopt);
int enet_setup_pie(struct net_device *dev, struct tc_pie_qopt_offload *qopt);
int enet_setup_dualpi2(struct net_device *dev, struct tc_dualp2_qopt_offload *qopt);
void enet_create_tc_egress_sysfs(void);
void enet_remove_tc_egress_sysfs(void);

#endif	// __ENET_EGRESS_TC__