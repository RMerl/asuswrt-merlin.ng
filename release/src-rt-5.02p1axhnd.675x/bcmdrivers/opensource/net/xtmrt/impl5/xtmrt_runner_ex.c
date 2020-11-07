/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/ppp_channel.h>
#include <linux/blog.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/ip.h>
#include <linux/bcm_log.h>
#include <bcmtypes.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include "bcmnet.h"
#include "bcmxtmrt.h"
#include <linux/nbuff.h>
#include "bcmxtmrtimpl.h"
#include "xtmrt_runner.h"

extern bdmf_object_handle rdpa_cpu_xtm_obj;
extern int g_xtm_rxq_stats_received[];
extern int g_xtm_rxq_stats_dropped[];
struct proc_dir_entry *xtm_dir;
int xtm_dump_rx;

#ifdef CONFIG_BCM_XRDP
static void xtm_rdpa_rx_dump_data_cb(bdmf_index queue, bdmf_boolean enabled);
#else
static void xtm_rdpa_rxq_stat_cb(int qid, extern_rxq_stat_t *stat, bdmf_boolean clear);
static inline int xtm_rdpa_rxq_queued_get(int qidx);
RING_DESCRIPTOR_S xtm_ring[XTM_RNR_CPU_RX_QUEUES]  = {};


static int service_queues_enabled;
static bdmf_object_handle svc_queues_scheduler;
static bdmf_object_handle svc_queues[32];
static int     create_service_queues(bdmf_object_handle tm);
static int     delete_service_queues(bdmf_object_handle tm);
static int     svc_q_open(struct inode *inode, struct file *file);
static ssize_t svc_q_write(struct file *file, const char __user *buf, size_t len, loff_t *offset);

static const struct file_operations svc_queues_fops = {
   .open    = svc_q_open,
   .read    = seq_read,
   .llseek  = seq_lseek,
   .write   = svc_q_write,
   .release = single_release
};

#endif

                                    /* RDP functions are listed here */
#ifndef CONFIG_BCM_XRDP

/*---------------------------------------------------------------------------
 * int xtm_rdpa_rxq_queued_get(void)
 * Description:
 *    Get the number of packets queued in the ring
 * Returns:
 *    0 or error code
 *---------------------------------------------------------------------------
 */
static inline int xtm_rdpa_rxq_queued_get(int qidx)
{
    CPU_RX_DESCRIPTOR *p;
    int count;

    //walk over the ring and check the onwership flag.
    for (p = xtm_ring[qidx].base, count = 0; p != xtm_ring[qidx].end; p++)
        count += (swap4bytes(p->word2) & 0x80000000) ? 1 : 0;
    return count;
} 

/*---------------------------------------------------------------------------
 * int xtm_rdpa_rxq_stat_cb(void)
 * Description:
 *    Get the Queue stats
 * Returns:
 *    0 or error code
 *---------------------------------------------------------------------------
 */
static void xtm_rdpa_rxq_stat_cb(int qid, extern_rxq_stat_t *stat, bdmf_boolean clear)
{
    int queue_idx = qid -  RDPA_XTM_CPU_LO_RX_QUEUE_ID;

    if (!stat)
        return;

    stat->received = g_xtm_rxq_stats_received[queue_idx];
    stat->dropped = g_xtm_rxq_stats_dropped[queue_idx];
    stat->queued  = xtm_rdpa_rxq_queued_get(queue_idx);

    if (clear)
        g_xtm_rxq_stats_received[queue_idx] = g_xtm_rxq_stats_dropped[queue_idx] = 0;
}

/*---------------------------------------------------------------------------
 * int bcmxapiex_cpu_object_get(void)
 * Description:
 *    Called to get the CPU object
 * Returns:
 *    0 or error status
 *---------------------------------------------------------------------------
 */

int  bcmxapiex_cpu_object_get (bdmf_object_handle  *xtm_obj)
{
   int rc= 0;

   if (rdpa_cpu_get(XTM_RDPA_CPU_PORT, xtm_obj))
      rc = -ESRCH;
   return (rc) ;
}

int bcmxapiex_ring_create_delete(int q_id, int q_size, rdpa_cpu_rxq_cfg_t *rxq_cfg)
{
   int rc = 0;
   uint32_t *ring_base = NULL;

   rc = rdpa_cpu_rx_create_ring(&xtm_ring[q_id - RDPA_XTM_CPU_RX_QUEUE_ID_BASE],q_size,&ring_base);
   rxq_cfg->ring_head = ring_base;
   rxq_cfg->rxq_stat = xtm_rdpa_rxq_stat_cb;
   return rc;
}

int bcmxapiex_get_pkt_from_ring(int hw_q_id, FkBuff_t **ppFkb, rdpa_cpu_rx_info_t *info)
{
   int rc ;

   rc = rdpa_cpu_get_pkt_from_ring (&xtm_ring[XTM_RDPA_CPU_RX_QUEUE_IDX(hw_q_id)], ppFkb, info);
   return (rc) ;
}

/*---------------------------------------------------------------------------
 * int bcmxapi_add_proc_files(void)
 * Description:
 *    Adds proc file system directories and entries.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
int bcmxapiex_add_proc_files(void)
{
   xtm_dir = proc_mkdir("driver/xtm", NULL);
   proc_create("svc-queues", 0440, xtm_dir, &svc_queues_fops);
   return 0;
}

/*---------------------------------------------------------------------------
 * int bcmxapiex_del_proc_files(void)
 * Description:
 *    Deletes proc file system directories and entries.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
int bcmxapiex_del_proc_files(void)
{
   remove_proc_entry("svc-queues", xtm_dir);
   proc_remove(xtm_dir);
   xtm_dir = NULL ;
   return 0;
    
}  /* bcmxapi_del_proc_files() */


int bcmxapiex_runner_xtm_objects_init(bdmf_object_handle wan, bdmf_object_handle *pXtm_orl_tm)
{
   return (0);
}

int bcmxapiex_runner_xtm_orl_rl_set(bdmf_object_handle Xtm_orl_tm, PXTMRT_PORT_SHAPER_INFO pPortRateShaperInfo)
{
   return (0);
}

int bcmxapiex_cfg_cpu_ds_queues (rdpa_cpu_reason reason, uint8_t tc, uint8_t queue_id)
{
   int rc;

   rdpa_cpu_reason_cfg_t   reason_cfg = {};
   rdpa_cpu_reason_index_t cpu_reason;

   cpu_reason.reason = reason;
   cpu_reason.dir    = rdpa_dir_ds;
   cpu_reason.table_index = CPU_REASON_WAN0_TABLE_INDEX;
   reason_cfg.queue  = queue_id; 
   reason_cfg.meter  = BDMF_INDEX_UNASSIGNED; 
   rc = rdpa_cpu_reason_cfg_set(rdpa_cpu_xtm_obj, &cpu_reason, &reason_cfg);

   return rc;
   
}

/*---------------------------------------------------------------------------
 * int create_service_queues(bdmf_object_handle tm)
 *
 * Description:
 *    When DPI is enabled, this function adds DPI queues under the best-effort
 *    egress_tm object passed in the arguments.
 * Returns:
 *    0 on succes, error condition otherwise
 *---------------------------------------------------------------------------
 */
static int create_service_queues(bdmf_object_handle tm)
{
   BDMF_MATTR(sched_attrs, rdpa_egress_tm_drv());
   rdpa_tm_service_queue_t service_queue = {.enable = 1};
   bdmf_object_handle sub_sched = NULL;
   int i, rc;

   if (!service_queues_enabled || !tm || svc_queues_scheduler)
      return 0;

   rc = rdpa_egress_tm_subsidiary_get(tm, 0, &sub_sched);
   if (rc)
   {
      BCM_LOG_NOTICE(BCM_LOG_ID_XTM, "rdpa_egress_tm_subsidiary_get() failed: rc(%d)", rc);
      return rc;
   }
   /* if someone else programmed our service queues scheduler, save a reference
    * to it and ignore creating anything further */
   if (sub_sched)
   {
      svc_queues_scheduler = sub_sched;
      return 0;
   }

   /* Create service queue scheduler */
   rdpa_egress_tm_dir_set(sched_attrs, rdpa_dir_us);
   rdpa_egress_tm_level_set(sched_attrs, rdpa_tm_level_egress_tm);
   rdpa_egress_tm_mode_set(sched_attrs, rdpa_tm_sched_sp);
   rdpa_egress_tm_service_queue_set(sched_attrs, &service_queue);

   rc = bdmf_new_and_set(rdpa_egress_tm_drv(), tm, sched_attrs, &svc_queues_scheduler);
   if (rc || !svc_queues_scheduler)
   {
      BCM_LOG_ERROR(BCM_LOG_ID_XTM, "Failed to create service queues scheduler, rc %d\n", rc);
      return rc;
   }

   rc = rdpa_egress_tm_subsidiary_set(tm, 0, svc_queues_scheduler);
   if (rc)
   {
      BCM_LOG_ERROR(BCM_LOG_ID_XTM, "rdpa_egress_tm_subsidiary_set() failed: rc(%d)", rc);
      return rc;
   }

   /* Create queues under scheduler */
   for (i = 0; i < ARRAY_SIZE(svc_queues); i++)
   {
      BDMF_MATTR(attrs, rdpa_egress_tm_drv());
      rdpa_tm_queue_cfg_t queue_cfg = {};

      /* Create service queue scheduler */
      rdpa_egress_tm_dir_set(attrs, rdpa_dir_us);
      rdpa_egress_tm_level_set(attrs, rdpa_tm_level_queue);
      rdpa_egress_tm_mode_set(attrs, rdpa_tm_sched_disabled);
      rdpa_egress_tm_service_queue_set(attrs, &service_queue);

      rc = bdmf_new_and_set(rdpa_egress_tm_drv(), tm, attrs, &svc_queues[i]);
      if (rc || !svc_queues[i])
      {
         BCM_LOG_ERROR(BCM_LOG_ID_XTM, "Failed to create service queue %d, rc %d\n", i, rc);
         continue;
      }

      rc = rdpa_egress_tm_subsidiary_set(svc_queues_scheduler, i, svc_queues[i]);
      if (rc) {
         BCM_LOG_ERROR(BCM_LOG_ID_XTM, "rdpa_egress_tm_subsidiary_set() failed: rc(%d)", rc);
         continue;
      }

      queue_cfg.queue_id = i;

      rc = rdpa_egress_tm_queue_cfg_set(svc_queues[i], 0, &queue_cfg);
      if (rc)
         BCM_LOG_ERROR(BCM_LOG_ID_XTM, "rdpa_egress_tm_queue_cfg_set() failed: rc(%d)", rc);
   }

   return 0;
}

/*---------------------------------------------------------------------------
 * int delete_service_queues(bdmf_object_handle tm)
 *
 * Description:
 *    When DPI is enabled, this function adds DPI queues under the best-effort
 *    egress_tm object passed in the arguments.
 * Returns:
 *    0 on succes, error condition otherwise
 *---------------------------------------------------------------------------
 */
static int delete_service_queues(bdmf_object_handle tm)
{
   int i;

   /* destroy service queues */
   for (i = 0; i < ARRAY_SIZE(svc_queues); i++)
   {
      if (!svc_queues[i])
         continue;

      bdmf_destroy(svc_queues[i]);
      svc_queues[i] = NULL;
   }

   /* destroy svc queue scheduler */
   if (svc_queues_scheduler)
   {
      bdmf_destroy(svc_queues_scheduler);
      svc_queues_scheduler = NULL;
   }

   return 0;
}

static int svc_q_seq_show(struct seq_file *s, void *v)
{
   return seq_printf(s, "%d\n", service_queues_enabled);
}
static int svc_q_open(struct inode *inode, struct file *file)
{
   return single_open(file, svc_q_seq_show, NULL);
};
static ssize_t svc_q_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
   int changed;
   long val = 0;
   ssize_t ret;

   ret = kstrtol_from_user(buf, len, 10, &val);
   if (ret < 0)
      return ret;
   val = val ? 1 : 0;

   changed = (service_queues_enabled != val);
   service_queues_enabled = val;
   if (!changed)
      return len;

   if (service_queues_enabled)
      create_service_queues(g_GlobalInfo.txBdmfObjs[0].egress_tm);
   else
      delete_service_queues(g_GlobalInfo.txBdmfObjs[0].egress_tm);

   return len;
}

void bcmxapiex_SetOrStartTxQueue (rdpa_tm_queue_cfg_t *pQueueCfg, bdmf_object_handle egress_tm)
{
   if (pQueueCfg->best_effort)
      create_service_queues(egress_tm);
}

void bcmxapiex_StopTxQueue (rdpa_tm_queue_cfg_t *pQueueCfg, bdmf_object_handle egress_tm)
{
   //if (pQueueCfg->best_effort)
      //delete_service_queues(egress_tm);
}


#else
                                    /* xRDP functions are listed here */

static void xtm_rdpa_rx_dump_data_cb(bdmf_index queue, bdmf_boolean enabled)
{
    xtm_dump_rx = enabled;
}

/*---------------------------------------------------------------------------
 * int bcmxapiex_cpu_object_get(void)
 * Description:
 *    Called to get the CPU object
 * Returns:
 *    0 or error status
 *---------------------------------------------------------------------------
 */

int  bcmxapiex_cpu_object_get (bdmf_object_handle  *xtm_obj)
{
   int rc = 0;

   BDMF_MATTR(cpu_xtm_attrs, rdpa_cpu_drv());

   /* create cpu port instance for xtm */
   rdpa_cpu_index_set(cpu_xtm_attrs, XTM_RDPA_CPU_PORT) ;

   rdpa_cpu_num_queues_set(cpu_xtm_attrs, XTM_RNR_CPU_RX_QUEUES);

   if ((rc = bdmf_new_and_set(rdpa_cpu_drv(), NULL, cpu_xtm_attrs, xtm_obj)))
   {
      printk("%s:%s Failed to create cpu xtm%d object rc(%d)\n", __FILE__, __FUNCTION__, XTM_RDPA_CPU_PORT, rc);
   }

   if ((rc = rdpa_cpu_int_connect_set(rdpa_cpu_xtm_obj, 1 /*true*/)) && rc != BDMF_ERR_ALREADY)
   {
      pr_err("XTM : Failed to connect cpu interrupts rc(%d)\n", rc);
   }

   return (rc);
}

int bcmxapiex_ring_create_delete(int q_id, int q_size, rdpa_cpu_rxq_cfg_t *rxq_cfg)
{
   int rc = 0;

   rxq_cfg->ring_head = NULL; /* NULL required by RDPA to create AND REMOVE rdp_ring */
   rxq_cfg->rx_dump_data_cb = xtm_rdpa_rx_dump_data_cb;
   return rc;
}

int bcmxapiex_get_pkt_from_ring(int hw_q_id, FkBuff_t **ppFkb, rdpa_cpu_rx_info_t *info)
{
   int rc ;
   rc = rdpa_cpu_packet_get(XTM_RDPA_CPU_PORT, hw_q_id, info);
   if (!rc) {
      *ppFkb = fkb_init((uint8_t *)info->data , BCM_PKT_HEADROOM, (uint8_t *)(info->data + info->data_offset), info->size);
      (*ppFkb)->recycle_hook = (RecycleFuncP)bdmf_sysb_recycle;
      if (unlikely(xtm_dump_rx))
         rdpa_cpu_rx_dump_packet("xtm_rx", XTM_RDPA_CPU_PORT, hw_q_id, info, 0);
   }
   return (rc) ;
}

int bcmxapiex_add_proc_files(void)
{
   return 0;
}

int bcmxapiex_del_proc_files(void)
{
   return 0;
}

int bcmxapiex_runner_xtm_orl_rl_set(bdmf_object_handle Xtm_orl_tm, PXTMRT_PORT_SHAPER_INFO pPortRateShaperInfo)
{
   int rc = 0 ;
   rdpa_tm_rl_cfg_t rl_cfg ;

   rl_cfg.af_rate    = pPortRateShaperInfo->ulShapingRate ;
   rl_cfg.be_rate    = pPortRateShaperInfo->ulShapingRate ;
   rl_cfg.burst_size = pPortRateShaperInfo->usShapingBurstSize ;

   if ((rc = rdpa_egress_tm_rl_set (Xtm_orl_tm, &rl_cfg)) != 0)
      BCM_XTM_ERROR ("bcmxtmrt: xtm orl rl set failed \n") ;
   return (rc) ;
}

int bcmxapiex_runner_xtm_objects_init(bdmf_object_handle wan, bdmf_object_handle *pXtm_orl_tm)
{
   int rc;
   bdmf_object_handle cpu_obj = NULL;
   bdmf_object_handle xtm_orl_tm = NULL;
   bdmf_object_handle system_obj = NULL;
   XTMRT_PORT_SHAPER_INFO portRateShaperInfo = {0, 0} ;

   rdpa_filter_ctrl_t filter_ctrl;

   BDMF_MATTR(xtm_orl_tm_attr, rdpa_egress_tm_drv());

   if ((rc = rdpa_system_get(&system_obj)) || (system_obj == NULL)) {
      printk("rdpa_system_get() failed: rc(%d)", rc);
      return rc;
   }

   /* create egress xtm orl tm */
   //rc = rdpa_egress_tm_index_set(xtm_orl_tm_attr, XTM_ORL_TM_INDEX);
   rc = rdpa_egress_tm_dir_set(xtm_orl_tm_attr, rdpa_dir_us);
   rc = rc? rc : rdpa_egress_tm_level_set(xtm_orl_tm_attr, rdpa_tm_level_egress_tm);
   rc = rc? rc : rdpa_egress_tm_mode_set(xtm_orl_tm_attr, rdpa_tm_sched_disabled);
   rc = rc? rc : rdpa_egress_tm_rl_rate_mode_set(xtm_orl_tm_attr, rdpa_tm_rl_dual_rate);
   rc = rc? rc : rdpa_egress_tm_overall_rl_set(xtm_orl_tm_attr, TRUE);

   rc = rc? rc : bdmf_new_and_set(rdpa_egress_tm_drv(), system_obj, xtm_orl_tm_attr, &xtm_orl_tm);
   rc = rc? rc : bcmxapiex_runner_xtm_orl_rl_set (xtm_orl_tm, &portRateShaperInfo);

   if (rc) {
      BCM_XTM_ERROR(CARDNAME "Failed to create XTM ORL egress TM globally %d");
      if (xtm_orl_tm)
         bdmf_destroy (xtm_orl_tm) ;
      return rc;
   }
   else {
       bdmf_put(system_obj);
      *pXtm_orl_tm = xtm_orl_tm ;
   }

   rc = rdpa_cpu_get(XTM_RDPA_CPU_PORT, &cpu_obj);
   rc = rc ? rc : rdpa_port_cpu_obj_set(wan, cpu_obj);
   if (rc) {
      BCM_XTM_ERROR(CARDNAME "Failed to set CPU object for port %s, error %d", bdmf_object_name(wan), rc);
      bdmf_destroy (xtm_orl_tm) ;
      *pXtm_orl_tm = NULL ;
      return rc ;
   }

   /* adding default entry to always trap IP_FRAG packet */
   filter_ctrl.enabled = TRUE;
   filter_ctrl.action = rdpa_forward_action_host;
   rc = rdpa_port_ingress_filter_set(wan, RDPA_FILTER_IP_FRAG, &filter_ctrl);
   if (rc) {
      BCM_LOG_ERROR(BCM_LOG_ID_XTM, "Failed to add filter '%u' for port %s, error %d\n",
                     RDPA_FILTER_IP_FRAG, bdmf_object_name(wan), rc);
      bdmf_destroy (xtm_orl_tm) ;
      *pXtm_orl_tm = NULL ;
   }

   return rc;
}

int bcmxapiex_cfg_cpu_ds_queues (rdpa_cpu_reason reason, uint8_t tc, uint8_t queue_id)
{
   int rc ;

   rc = rdpa_cpu_tc_to_rxq_set (rdpa_cpu_xtm_obj, tc, queue_id);
   return rc;
}

void bcmxapiex_SetOrStartTxQueue (rdpa_tm_queue_cfg_t *pQueueCfg, bdmf_object_handle egress_tm)
{
}

void bcmxapiex_StopTxQueue (rdpa_tm_queue_cfg_t *pQueueCfg, bdmf_object_handle egress_tm)
{
}

#endif /* #ifndef CONFIG_BCM_XRDP */

void bcmxapiex_ShutdownTxQueue (UINT32 queueIdx, bdmf_object_handle egress_tm)
{
   int rc ;
   rdpa_stat_t          queue_stat ;
   rdpa_tm_queue_index_t q_index = {} ;

   q_index.channel = US_CHANNEL_OFFSET_DSL + queueIdx ;
   q_index.queue_id = queueIdx;

   memset (&queue_stat, 0, sizeof (rdpa_stat_t)) ;


   if ((rc = rdpa_egress_tm_queue_occupancy_get (egress_tm, &q_index, &queue_stat) == 0)) {

      printk("bcmxtmrt: Egress TM Q %d Occupancy Pre %d \n", (unsigned int) queueIdx, (UINT32) queue_stat.packets) ;

      printk("bcmxtmrt: Egress TM Q %d Flush/Shutdown %d \n", (unsigned int) queueIdx, (UINT32) jiffies) ;

      if ((rc = rdpa_egress_tm_queue_cfg_delete(egress_tm, 0)))
         printk(CARDNAME "Delete TxQueue: rdpa_egress_tm_queue_cfg_delete error rc=%d\n", rc) ;
   }

   if (!rc)
      printk("bcmxtmrt: Egress TM Q %d Flush & Shutdown Done. %d \n", (unsigned int) queueIdx, (UINT32) jiffies) ;
   else
      printk("bcmxtmrt: Egress TM Q %d Flush & Shutdown Error. %d \n", (unsigned int) queueIdx, rc) ;
}

