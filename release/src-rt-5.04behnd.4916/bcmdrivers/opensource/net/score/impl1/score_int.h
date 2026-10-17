/*
   <:copyright-BRCM:2024:DUAL/GPL:standard

      Copyright (c) 2024 Broadcom
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

#ifndef ___SCORE_INT_H___
#define ___SCORE_INT_H___

#include <linux/bcm_log.h>
#include <linux/blog.h>
#include "score_hw.h"
#include "score_queue.h"
#include "bcm_score.h"
#include "score_procfs.h"

//#define CC_SCORE_DEBUG_ENABLE

#if defined (__KERNEL__)
#if defined(CONFIG_BCM96858) ||  defined(CONFIG_BCM96856) || defined(CONFIG_BCM96855) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM96764) || defined(CONFIG_BCM96764L) || defined(CONFIG_BCM96756)
#define CC_SW_ONLY_SCORE_SUPPORT
#endif
#endif

#define SCORE_RET_SUCCESS 0
#define SCORE_RET_FAILURE -1

/* TODO should these macros be score_id or stream_id */

#define STREAM_ID_IDX_MASK      (STREAM_ID_MAX_ENTRIES - 1)
#define STREAM_ID_AVAIL_BITMAP_NUM_WORDS         (STREAM_ID_MAX_ENTRIES / (sizeof(unsigned long) * 8))
#define STREAM_ID_RESERVE_BITS  ((sizeof(uint16_t) * 8) - (STREAM_IDX_MAX_BITS + STREAM_ID_INCARN_BITS))

#define SCORE_MSG_QSTAT_ID      (SCORE_MAX_SDRS)
#define SCORE_MSG_BNL_ID        (SCORE_MAX_SDRS+1)


/* stream_id_key is simialr to blog_score, but with out direction 
 * as streamid will be same for both directions
 * */
typedef struct {
    /* this is only accsed by host, no need of LE & BE */
    union {
       struct {
           uint16_t idx:STREAM_IDX_MAX_BITS;
           uint16_t incarn:STREAM_ID_INCARN_BITS;
           uint16_t reserved:STREAM_ID_RESERVE_BITS;
       };

       uint16_t key;
    };
} stream_id_key_t;

#define IS_BIT_ON(bitmap, bit) ((bitmap) & ((typeof(bitmap))1 << (bit)))
#define SET_BIT(bitmap, bit) ((bitmap) |= ((typeof(bitmap))1 << (bit)))
#define CLR_BIT(bitmap, bit) ((bitmap) &= ~((typeof(bitmap))1 << (bit)))

#define isLogDebug bcmLog_logIsEnabled(BCM_LOG_ID_SCORE, BCM_LOG_LEVEL_ERROR)
#define __logDebug(fmt, arg...)   BCM_LOG_DEBUG(BCM_LOG_ID_SCORE, fmt, ##arg)
#define __logInfo(fmt, arg...)    BCM_LOG_INFO(BCM_LOG_ID_SCORE, fmt, ##arg)
#define __logNotice(fmt, arg...)  BCM_LOG_NOTICE(BCM_LOG_ID_SCORE, fmt, ##arg)
#define __logError(fmt, arg...)   BCM_LOG_ERROR(BCM_LOG_ID_SCORE, fmt, ##arg)

#define __print(fmt, arg...) bcm_print(fmt, ##arg)

#if defined(CC_SCORE_DEBUG_ENABLE)
#define __debug(fmt, arg...)                    \
    BCM_LOGCODE(                                \
        if(isLogDebug)                          \
            __print("%s:%d: " fmt "\n", __FUNCTION__, __LINE__, ##arg); )

#else
#define __debug(fmt, arg...)
#endif

#define SCORE_VERSION              "1.0"
#if defined (__KERNEL__)
#define SCORE_VER_STR              "v" SCORE_VERSION
#else
#define SCORE_VER_STR              "v" SCORE_VERSION " " __DATE__ " " __TIME__
#endif
#define SCORE_MODNAME              "Broadcom SCORE Driver"

typedef struct {
    /* TODO -- remove this and bring in SCORE license */
    int spdsvc_license_verified;
    void *ring_ptr;
} score_t;

extern int score_prints_enable_g;
extern int score_global_enable_g;
extern score_dcp_cfg_data_t  dcp_db[SCORE_MAX_DCP_NUM];

extern score_dcp_cfg_data_t  dcp_db[];

void score_procfs_entry_queue(score_data_t *queue_entry_p);
void score_procfs_dcp_pid(int dcp_id, uint32_t pid);

void qstat_procfs_entry_queue(score_qstat_data_t *queue_entry_p);

int score_procfs_construct(void);
int score_procfs_destroy(void);


int score_msg_rings_init(void);
void score_msg_rings_uninit(void);

int score_dcp_add(struct file *filep, score_dcp_cfg_data_t *score_dcp_cfg_data);
/* top level dcp release function */
int score_dcp_release(void * data);
void score_id_tbl_flush_by_dcp(uint8_t dcp_id);

int score_monitor_start(score_mon_start_data_t *score_mon_start_data);
int score_monitor_stop(score_mon_stop_data_t *score_mon_stop_data);
int qstat_config_report(score_qstat_cfg_report_t *qstat_cfg);
int qstat_delete_report(uint8_t qstat_cfg_id);
int qstat_get_results(score_qstat_result_t *score_qstat_result);
void score_notify_async_q_data(void);
int process_qstat_async_q(bcm_async_queue_t *queue_p, int bound);
int process_qstat_nic(int bound); /* for NIC qstat processing */
int process_qstat_ring(int bound);/* Runner:DHD/Wired, Archer/Crossbow:DHD */
int score_qstats_asyncq_stats_get(int *idx_p, bcm_async_queue_stats_t *stats_p, 
                                  char *disp_str_p, int str_sz);
int qstat_get_nic_asyncq_stats(bcm_async_queue_stats_t *stats_p);
int qstat_get_asyncq_stats(bcm_async_queue_t *queue_p, bcm_async_queue_stats_t *stats_p);

int score_hw_init(void);
void score_hw_destroy(void);

void score_hw_msg_callback(int param);
int score_dcp_notify(score_dcp_stream_cmd_t cmd, void *dcp_stream_config,
        void **dcp_priv);
int score_monitor_start_by_streamtuple(score_mon_start_data_t *stream_mon_start_cfg);
int score_monitor_stop_by_streamid(score_mon_stop_data_t *stream_mon_stop_cfg);
int score_dcp_flow_notify(uint16_t stream_id, uint16_t dcp_add_bmap, uint16_t dcp_del_bmap,
        blog_score_flow_info_t *score_flow_info);

#endif /* ___SCORE_INT_H___ */
