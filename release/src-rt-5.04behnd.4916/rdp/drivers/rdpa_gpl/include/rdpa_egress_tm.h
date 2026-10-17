/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
*
*    Copyright (c) 2013 Broadcom
*    All Rights Reserved
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
*
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
*
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
*
* :>
*/


#ifndef _RDPA_EGRESS_TM_H_
#define _RDPA_EGRESS_TM_H_

/** \defgroup egress_tm Egress Traffic Manager
 * \ingroup tm
 * The RDPA egress traffic manager controls a number of egress-TM elements, whereas
 * egress TM element is either a transmit queue or a next-level egress-TM. Then total
 * number of supported egress-TM elements and transmit queues is platform dependent retrieved via system object.
 * The RDPA egress traffic manager has the following capabilities:
 *  - Egress TM element can be configured to support either rate limiter or scheduler or both.
 *  - Up to 3-level hierarchical egress-TM.
 *  - The transmit queues support different drop policies ::rdpa_tm_drop_alg.
 * All egress_tm are created with rdpa_tm_drop_alg_dt meaning no special drop algorithm
 * @{
 */
/* Max ds service queue shapers */
#define RDPA_EGRESS_TM_CHANNEL_IS_GROUP_ID 0xFFFF
#define RDPA_MAX_SERVICE_QUEUES      32
#define RDPA_MAX_SQ_WAN_SCHEDULERS    2
#define RDPA_MAX_DS_TM_QUEUE        128
#define RDPA_MAX_US_TM_QUEUE        128
#define RDPA_EGRESS_TM_SVCQ_INDEX   128
#define RDPA_TM_MAX_US_SCHED        (RDPA_MAX_US_TM_QUEUE + (RDPA_MAX_SERVICE_QUEUES * RDPA_MAX_SQ_WAN_SCHEDULERS))
#define RDPA_TM_MAX_DS_SCHED        (RDPA_MAX_DS_TM_QUEUE + RDPA_MAX_SERVICE_QUEUES)
#define RDPA_TM_MAX_SCHED_ELEMENTS 32   /**< Max number of subsidiary elements in egress-TM group */
#define RDPA_WEIGHT_UNASSIGNED 0
#define RDPA_MAX_EGRESS_QUEUES 32       /**< Max number of egress queues per egress-TM element */
#define RDPA_MAX_QUEUE_STATS RDPA_MAX_EGRESS_QUEUES
#define RDPA_DFT_NUM_EGRESS_QUEUES 8    /**< Default number of egress queues per egress-TM element */
#define RDPA_MAX_WEIGHT 100
#define RDPA_MIN_WEIGHT 1
#define RDPA_MAX_WRED_PROFILE_PER_DIRECTION 8  /**< Max number of WRED profilers */
#define RDPA_WRED_MAX_DROP_PROBABILITY     100
#define RDPA_ETH_TX_PRIORITY_QUEUE_THRESHOLD  0xFFFFFFF
#include "rdpa_egress_tm_basic.h"
#include "bdmf_interface.h"
#include "rdpa_rate_limit.h"

#ifdef DCL_SUPPORT
/* seperate structure for tm DCL data*/
typedef struct {
    uint32_t epon_request;                      /* is this image support epon*/
    uint32_t mirror_request;                    /* is this image support mirror*/
    uint32_t port_stat_request;                 /* is this image port_stat*/
} dcl_tm_debug_flag_cfg_t;
#endif

/** Egress-TM next level type */
/* The secondary level was created to accomodate L4S or the DPI schedulers. Since it can
 * accomodate two schedulers it is the respnsibility of the user to properly configure them.
 * The assumptions throught the code are that it must be a subsidiary Egress TM, that it
 * cannot have its own subsidiary and that it is limited to four queues in pure WRR or SP modes.
 */
typedef enum {
    rdpa_tm_level_queue,      /**< Next level type is queue */
    rdpa_tm_level_egress_tm,  /**< Next level type is Egress-TM */
#ifdef TM_C_CODE
    rdpa_tm_level_secondary   /**< Next level type is secondary */
#endif
} rdpa_tm_level_type;

/** Egress-TM rate limiter mode */
typedef enum {
    rdpa_tm_rl_single_rate,     /**< Single rate rate limiter (default mode) */
    rdpa_tm_rl_dual_rate        /**< Dual rate rate limiter */
} rdpa_tm_rl_rate_mode;

/** Scheduler operating modes */
typedef enum {
    rdpa_tm_sched_disabled,     /**< Scheduling disabled */
    rdpa_tm_sched_sp,           /**< SP scheduler */
    rdpa_tm_sched_wrr,          /**< WRR scheduler */
    rdpa_tm_sched_sp_wrr,       /**< SP+WRR scheduler \XRDP_LIMITED */

    rdpa_tm_sched__num_of,
} rdpa_tm_sched_mode;

/** Number of SP elements (queues or subsidiary egress_t objects)
 * for SP_WRR scheduling mode \XRDP_LIMITED
 */
typedef enum {
    rdpa_tm_num_sp_elem_0  = 0, /**< All elements WRR */
    rdpa_tm_num_sp_elem_2  = 2, /**< 2 SP elements, the rest are WRR */
    rdpa_tm_num_sp_elem_4  = 4, /**< 4 SP elements, the rest are WRR */
    rdpa_tm_num_sp_elem_8  = 8, /**< 8 SP elements, the rest are WRR */
    rdpa_tm_num_sp_elem_16 = 16,/**< 16 SP elements, the rest are WRR */
    rdpa_tm_num_sp_elem_32 = 32,/**< All elements SP */
} rdpa_tm_num_sp_elem;

/** Egress-TM enable configuration.\n
 */
typedef struct {
    bdmf_boolean enable;                  /**< Enable or disable egress TM */
    bdmf_boolean flush;                   /**< Flush to be done as part of enable \ disable */
} rdpa_tm_enable_cfg_t;

/** Drop policy algorithm type
*/
typedef enum {
    rdpa_tm_drop_alg_dt,                /**< Drop tail */
    rdpa_tm_drop_alg_red,               /**< RED (random early detection)*/
    rdpa_tm_drop_alg_wred,              /**< WRED (weighted RED) */
    rdpa_tm_drop_alg_flow_ctrl,         /**< Queues with this profile work as flow control */
    rdpa_tm_drop_alg_codel,             /**< CoDel */
    rdpa_tm_drop_alg_pi2,               /**< PI2 */
    rdpa_tm_drop_alg_dualq_laqm,        /**< LAQM in a dual queue arrangement (L4S) */
    rdpa_tm_drop_alg__num_of            /* Number of drop algorithms */
} rdpa_tm_drop_alg;

/** priority class thresholds.\n
 */
typedef struct
{
    uint32_t min_threshold;             /**< Min threshold for priority class used by WRED algorithm. */
    uint32_t max_threshold;             /**< Max threshold for priority class used by WRED algorithm. */
    uint32_t max_drop_probability;      /**< Max drop probability for WRED algorithm (allowed values are 1-100, matching to corresponding percent. ) \XRDP_LIMITED */
} rdpa_tm_priority_class_t;

/** Egress queue parameters configuration.\n
 */
typedef struct {
    uint32_t queue_id;           /**< queue_id. Assigned by management application */
    uint32_t drop_threshold;     /**< Drop threshold (queue size) */
    uint32_t weight;             /**< Weight in WFQ/WRR/DWRR Egress-TM group */
    rdpa_tm_drop_alg drop_alg;   /**< Drop algorithm */
    rdpa_tm_priority_class_t high_class;/**< High class thresholds (min and max) used by WRED algorithm */
    rdpa_tm_priority_class_t low_class; /**< Low class thresholds (min and max) used by WRED algorithm */
    uint32_t reserved_packet_buffers; /**< Top priority packet buffer number. 0 - no reservation \XRDP_LIMITED */
    uint32_t priority_mask_0;    /**< \DSLRDP_LIMITED */
    uint32_t priority_mask_1;    /**< \DSLRDP_LIMITED */
    bdmf_boolean qstat_enable;   /**< Enable queue statistics */
    bdmf_boolean best_effort;    /**< Best effort queue */
    bdmf_object_handle rl_cfg; /**< Rate limit Object for queue_cfg */
    bdmf_boolean exclusive;      /**< high priority queue */
} rdpa_tm_queue_cfg_t;

/** Service queue
 */
typedef struct {
    bdmf_boolean enable; /**< Egress_tm is of type service queue */
} rdpa_tm_service_queue_t;

/** Queue index and the egress_tm holding it
 */
typedef struct {
    int queue_idx; /**< Egress queue index */
    bdmf_object_handle queue_tm; /**< egress_tm object holding the queue */
} rdpa_tm_queue_location_t;

/** Subsidiary type
 */
typedef enum
{
    rdpa_subsidiary_generic,     /**< Default, general subsidiary egress_tm */
    rdpa_subsidiary_l4s,         /**< L4S subsidiary egress_tm */
} rdpa_subsidiary_type;

#ifdef BDMF_DRIVER
extern bdmf_attr_enum_table_t orl_prty_enum_table;
extern bdmf_attr_enum_table_t tm_drop_policy_enum_table;
#endif

/* Callback pointer to register EPON stack global rate limiter function */
typedef unsigned char (*epon_global_shaper_cb_t)(uint32_t);
extern epon_global_shaper_cb_t global_shaper_cb;

/* Callback pointer to register EPON stack link rate limiter function */
typedef unsigned char (*epon_link_shaper_cb_t)(uint8_t, uint32_t, uint16_t);
extern epon_link_shaper_cb_t epon_link_shaper_cb;

/* API to check if the given queue_id exists for the vport */
int egress_tm_is_empty_on_channel(bdmf_object_handle tm_obj, bdmf_boolean *is_empty);

/* Check if egress_tm hierarchy contains AQM queues */
int egress_tm_if_has_aqm_queues(bdmf_object_handle tm_obj);

rdpa_tm_rl_cfg_t *egress_tm_rate_limit_get_by_pointer(bdmf_object_handle _obj_, rdpa_tm_rl_cfg_t *cfg);

void rdpa_update_wrr(bdmf_object_handle _obj_);
int rdpa_egress_tm_lan_port_queue_to_rdd(rdpa_port_type port_type, uint32_t port_index, uint32_t queue_id, uint16_t *qm_queue);
int rdpa_egress_tm_find_queue_id(bdmf_object_handle egress_tm, uint32_t queue_id, bdmf_object_handle *_egress_tm, uint8_t *_index);

#ifdef DCL_SUPPORT
void tm_dcl_set_xepon_support(void);
void tm_dcl_set_epon_support(void);
void tm_dcl_pkt_size_stat_set(bdmf_boolean enable);
void tm_dcl_tx_mirror_set(void);
#endif

/** @} end of sched Doxygen group */

#endif /* _RDPA_EGRESS_TM_H_ */
