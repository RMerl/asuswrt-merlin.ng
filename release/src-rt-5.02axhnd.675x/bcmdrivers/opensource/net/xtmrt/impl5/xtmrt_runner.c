/*
<:copyright-BRCM:2011:DUAL/GPL:standard

   Copyright (c) 2011 Broadcom 
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
#include <linux/string.h>
#include <linux/bcm_log.h>
#include <bcmtypes.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include "bcmnet.h"
#include "bcmxtmrt.h"
#include <linux/nbuff.h>
#include "bcmxtmrtimpl.h"
#include "xtmrt_runner.h"

/**** Externs ****/
extern bdmf_type_handle rdpa_xtmchannel_drv(void);
/**** Prototypes ****/

static void bcmxtmrt_timer(PBCMXTMRT_GLOBAL_INFO pGi);
static int cfg_xtmflows(UINT32 queueIdx, UINT32 trafficType, UINT32 hdrType, UINT8 vcid, UINT32 ulTxPafEnabled);
static int add_xtmflow(bdmf_object_handle attrs, UINT32 ctType, UINT32 queueIdx, UINT32 hdrType, UINT32 fstat, int ptmBonding);
static UINT32 calc_xtmflow_fstat(UINT32 ctType, UINT32 hdrType, UINT32 vcid, UINT32 flag);
static int  runner_xtm_objects_init(void);
static void runner_xtm_objects_uninit(void);
static int runner_tx_queue_init(bdmf_object_handle xtm, int queueIdx);
static void runner_tx_queue_uninit(int queueIdx);
static int cfg_cpu_rx_queue(int queue_id, UINT32 queue_size, void *rx_isr);
static void cpu_rxq_isr(int queue_id);

bdmf_object_handle rdpa_cpu_xtm_obj = NULL;
int g_xtm_rxq_stats_received[XTM_RNR_CPU_RX_QUEUES]  = {};
int g_xtm_rxq_stats_dropped[XTM_RNR_CPU_RX_QUEUES]  = {};


/**
** All the Static functions are defined here.
**/


/*---------------------------------------------------------------------------
 * Function Name: bcmxtmrt_timer
 * Description:
 *    Periodic timer that calls the rx task to receive TEQ cells.
 * Returns: void
 *---------------------------------------------------------------------------
 */
static void bcmxtmrt_timer(PBCMXTMRT_GLOBAL_INFO pGi)
{
   if (pGi->pTeqNetDev && (!pGi->ulDevCtxMask))
   {
      UINT32 ulNotUsed;
      bcmxapi_rxtask(XTMRT_BUDGET, &ulNotUsed);

      /* Restart the timer. */
      pGi->Timer.expires = jiffies + SAR_TIMEOUT;
      add_timer(&pGi->Timer);
   }
}  /* bcmxtmrt_timer() */


/*---------------------------------------------------------------------------
 * int runner_xtm_objects_init(void)
 * Description:
 *    Initialize the following runner bdmf objects:
 *    - xtm system object
 *    - wan1 port object owned by xtm
 *    - egress tm orl object
 *    - 16 xtm transmit queues
 * Returns:
 *    0 or bdmf error code
 *---------------------------------------------------------------------------
 */
static int runner_xtm_objects_init(void)
{
   int   rc = 0;
   bdmf_object_handle   xtm;
   bdmf_object_handle   wan;
   bdmf_object_handle   xtm_orl_tm = NULL ;
      
   BDMF_MATTR(xtm_attr,  rdpa_xtm_drv());
   BDMF_MATTR(port_attr, rdpa_port_drv());

   /* Create the xtm system object */   
   rc = bdmf_new_and_set(rdpa_xtm_drv(), NULL, xtm_attr, &xtm);
   if (rc)
   {
      BCM_LOG_NOTICE(BCM_LOG_ID_XTM, "Failed to create xtm object, rc %d\n", rc);
      return rc;
   }
      
   /* Create logical ports object on top of the physical wan */
   rdpa_port_index_set(port_attr, rdpa_wan_type_to_if (rdpa_wan_dsl));
   rdpa_port_wan_type_set(port_attr, rdpa_wan_dsl);

   rc = bdmf_new_and_set(rdpa_port_drv(), xtm, port_attr, &wan); 
   if (rc)
   {
      BCM_LOG_NOTICE(BCM_LOG_ID_XTM,"Problem creating xtm wan port object\n");
      bdmf_destroy(xtm);
      return rc;
   }

   rc = bcmxapiex_runner_xtm_objects_init(wan, &xtm_orl_tm);

   if (rc)
   {
      BCM_LOG_NOTICE(BCM_LOG_ID_XTM, "Failed to init extended objects for WAN %s, error %d\n", bdmf_object_name(wan), rc);
      return rc;
   }

   /* xtmchannel and egress tm objects for all tx queues will be initialized dynamically at the request time as and when the tx queues are added. */
   g_GlobalInfo.bdmfXtm = xtm;
   g_GlobalInfo.bdmfWan = wan;
   g_GlobalInfo.bdmfXtm_Orl_Tm = xtm_orl_tm;
      
   return rc;
   
}  /* runner_xtm_objects_init() */


/*---------------------------------------------------------------------------
 * void runner_xtm_objects_uninit(void)
 * Description:
 *    Uninitialize all the runner xtm bdmf objects:
 * Returns: void
 *---------------------------------------------------------------------------
 */
static void runner_xtm_objects_uninit(void)
{
   if (g_GlobalInfo.bdmfWan != NULL)
   {
      bdmf_destroy(g_GlobalInfo.bdmfWan);
      g_GlobalInfo.bdmfWan = NULL;
   }
   if (g_GlobalInfo.bdmfXtm != NULL)
   {
      bdmf_destroy(g_GlobalInfo.bdmfXtm);
      g_GlobalInfo.bdmfXtm = NULL;
   }
   if (g_GlobalInfo.bdmfXtm_Orl_Tm != NULL)
   {
      bdmf_destroy(g_GlobalInfo.bdmfXtm_Orl_Tm);
      g_GlobalInfo.bdmfXtm_Orl_Tm = NULL;
   }
}  /* runner_xtm_objects_uninit() */


/*---------------------------------------------------------------------------
 * int runner_tx_queue_init(bdmf_object_handle xtm, int queueIdx)
 * Description:
 *    Initialize bdmf objects for xtm tx queue with index.
 *    In the Runner system, SAR BBH has 16 channels, each corresponds to an
 *    iudma tx channel (queue). For each BBH channel, an xtmchannel (tcont)
 *    object associated with an egress_tm and one tx queue is created.
 *    The configuration is as below:
 *       BBH_Channel0----xtmchannel0----egress_tm0----queue0
 *       BBH_Channel1----xtmchannel1----egress_tm1----queue1
 *       BBH_Channel2----xtmchannel2----egress_tm2----queue2
 *       .....................................
 *       .....................................
 *       BBH_Channel15----xtmchannel15----egress_tm15----queue15
 * Returns:
 *    0 or bdmf error code
 *---------------------------------------------------------------------------
 */
static int runner_tx_queue_init(bdmf_object_handle xtm, int queueIdx)
{
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   bdmf_object_handle xtmchannel;
   bdmf_object_handle tm;
   int rc = 0;

   BDMF_MATTR(xtmchannel_attr, rdpa_xtmchannel_drv());
   BDMF_MATTR(tm_attr, rdpa_egress_tm_drv());

   /* create xtmchannel for this tx queue */
   /* set xtmchannel index as queueIdx */
   rc = rdpa_xtmchannel_index_set(xtmchannel_attr, queueIdx);
   rc = rc? rc : bdmf_new_and_set(rdpa_xtmchannel_drv(), xtm, xtmchannel_attr, &xtmchannel);

   if (rc) {
      printk(KERN_ERR "Failed to create xtm channel for this tx queue %d", queueIdx);
      return rc;
   }
   pGi->txBdmfObjs[queueIdx].xtmchannel = xtmchannel;

   /* create egress tm for this xtmchannel */
   rc = rdpa_egress_tm_dir_set(tm_attr, rdpa_dir_us);
   rc = rc? rc : rdpa_egress_tm_level_set(tm_attr, rdpa_tm_level_queue);
   rc = rc? rc : rdpa_egress_tm_mode_set(tm_attr, rdpa_tm_sched_disabled);
   rc = rc? rc : bdmf_new_and_set(rdpa_egress_tm_drv(), xtmchannel, tm_attr, &tm);

   if (rc) {
      printk(KERN_ERR "Failed to create XTM egress TM for this tx queue %d", queueIdx);
      if (xtmchannel != NULL) {
         bdmf_destroy(xtmchannel);
         pGi->txBdmfObjs[queueIdx].xtmchannel = NULL;
      }
      return rc;
   }
   pGi->txBdmfObjs[queueIdx].egress_tm = tm;

   /* Set xtmchannel/egress_tm attribute */
   rc = rc? rc : rdpa_xtmchannel_egress_tm_set(xtmchannel, tm);

   if (rc) {
      printk(KERN_ERR "Failed to bind XTM egress TM for this tx queue with xtm channel %d", queueIdx);
      if (xtmchannel != NULL) {
         bdmf_destroy(xtmchannel);
         pGi->txBdmfObjs[queueIdx].xtmchannel = NULL;
      }
      if (tm != NULL) {
         bdmf_destroy(tm);
         pGi->txBdmfObjs[queueIdx].egress_tm = NULL;
      }
   }

   /* Set xtmchannel/ORL priority to low. */
   rc = rc? rc : rdpa_xtmchannel_orl_prty_set (xtmchannel, rdpa_tm_orl_prty_low);

   if (rc) {
      BCM_XTM_ERROR( "Failed to set ORL priority for this tx queue with xtm channel %d", queueIdx);
      if (xtmchannel != NULL) {
         bdmf_destroy(xtmchannel);
         pGi->txBdmfObjs[queueIdx].xtmchannel = NULL;
      }
      if (tm != NULL) {
         bdmf_destroy(tm);
         pGi->txBdmfObjs[queueIdx].egress_tm = NULL;
      }
   }

   /* Link xtmchannel & ORL */
   if (g_GlobalInfo.bdmfXtm_Orl_Tm != NULL)
      bdmf_link (pGi->bdmfXtm_Orl_Tm, xtmchannel, NULL) ;

   return rc;
}


/*---------------------------------------------------------------------------
 * void runner_tx_queue_uninit(queueIdx)
 * Description:
 *    Uninitialize the runner xtm tx queue (indexed) by destroying the xtmchannel object,
 *    the egress_tm object and the xtmflow objects associated with the queue.
 * Returns: void
 *---------------------------------------------------------------------------
 */
static void runner_tx_queue_uninit(int queueIdx)
{
   PBCMXTMRT_GLOBAL_INFO   pGi = &g_GlobalInfo;
   bdmf_object_handle      xtmchannel, egress_tm, xtmflow;

   int j;

   for (j = 0; j < MAX_CIRCUIT_TYPES; j++)
   {
      xtmflow = pGi->txBdmfObjs[queueIdx].xtmflow[j];
      if (xtmflow != NULL)
      {
         bdmf_destroy(xtmflow);
         pGi->txBdmfObjs[queueIdx].xtmflow[j] = NULL;
      }
   }

   egress_tm = pGi->txBdmfObjs[queueIdx].egress_tm;
   if (egress_tm != NULL)
   {
      bdmf_destroy(egress_tm);
      pGi->txBdmfObjs[queueIdx].egress_tm = NULL;
   }

   /* Unlink xtmchannel & ORL
   ** Uninitialize xtmchanel
   **/
   xtmchannel = pGi->txBdmfObjs[queueIdx].xtmchannel;

   if (xtmchannel != NULL)
   {
      if (g_GlobalInfo.bdmfXtm_Orl_Tm != NULL)
         bdmf_unlink (pGi->bdmfXtm_Orl_Tm, xtmchannel) ;
      bdmf_destroy(xtmchannel);
      pGi->txBdmfObjs[queueIdx].xtmchannel = NULL;
   }

}  /* runner_tx_queue_uninit() */


/*---------------------------------------------------------------------------
 * int cfg_xtmflows(UINT32 queueIdx, UINT32 trafficType, UINT32 hdrType, UINT8 vcid)
 * Description:
 *    This function configures rdpa xtmflows for a tx queue. One xtmflow is
 *    created for each XTM circuit type. 
 * Returns:
 *    0 or bdmf error code
 *---------------------------------------------------------------------------
 */
static int cfg_xtmflows(UINT32 queueIdx, UINT32 trafficType, UINT32 hdrType, UINT8 vcid, UINT32 ulTxPafEnabled)
{
   int      rc   = 0;
   UINT32   flag = CNI_USE_ALT_FSTAT | CNI_HW_ADD_HEADER;
   
   BDMF_MATTR(packet_attr, rdpa_xtmflow_drv());
   
   if ((trafficType & TRAFFIC_TYPE_ATM_MASK) == TRAFFIC_TYPE_ATM)
   {
      BDMF_MATTR(f4_seg_attr, rdpa_xtmflow_drv());
      BDMF_MATTR(f4_e2e_attr, rdpa_xtmflow_drv());
      BDMF_MATTR(f5_seg_attr, rdpa_xtmflow_drv());
      BDMF_MATTR(f5_e2e_attr, rdpa_xtmflow_drv());
      BDMF_MATTR(asm_p0_attr, rdpa_xtmflow_drv());
      BDMF_MATTR(asm_p1_attr, rdpa_xtmflow_drv());
      BDMF_MATTR(asm_p2_attr, rdpa_xtmflow_drv());
      BDMF_MATTR(asm_p3_attr, rdpa_xtmflow_drv());
      
      UINT32 packet_fstat = calc_xtmflow_fstat(XCT_AAL5, hdrType, vcid, flag);
      UINT32 f4_seg_fstat = calc_xtmflow_fstat(XCT_OAM_F4_SEG, 0, vcid, flag);
      UINT32 f4_e2e_fstat = calc_xtmflow_fstat(XCT_OAM_F4_E2E, 0, vcid, flag);
      UINT32 f5_seg_fstat = calc_xtmflow_fstat(XCT_OAM_F5_SEG, 0, vcid, flag);
      UINT32 f5_e2e_fstat = calc_xtmflow_fstat(XCT_OAM_F5_E2E, 0, vcid, flag);
      UINT32 asm_p0_fstat = calc_xtmflow_fstat(XCT_ASM_P0,     0, vcid, flag);
      UINT32 asm_p1_fstat = calc_xtmflow_fstat(XCT_ASM_P1,     0, vcid, flag);
      UINT32 asm_p2_fstat = calc_xtmflow_fstat(XCT_ASM_P2,     0, vcid, flag);
      UINT32 asm_p3_fstat = calc_xtmflow_fstat(XCT_ASM_P3,     0, vcid, flag);
      
      rc = rc? rc : add_xtmflow(packet_attr, XCT_AAL5,       queueIdx, hdrType, packet_fstat, BC_PTM_BONDING_DISABLE);
      rc = rc? rc : add_xtmflow(f4_seg_attr, XCT_OAM_F4_SEG, queueIdx, hdrType, f4_seg_fstat, BC_PTM_BONDING_DISABLE);
      rc = rc? rc : add_xtmflow(f4_e2e_attr, XCT_OAM_F4_E2E, queueIdx, hdrType, f4_e2e_fstat, BC_PTM_BONDING_DISABLE);
      rc = rc? rc : add_xtmflow(f5_seg_attr, XCT_OAM_F5_SEG, queueIdx, hdrType, f5_seg_fstat, BC_PTM_BONDING_DISABLE);
      rc = rc? rc : add_xtmflow(f5_e2e_attr, XCT_OAM_F5_E2E, queueIdx, hdrType, f5_e2e_fstat, BC_PTM_BONDING_DISABLE);
      rc = rc? rc : add_xtmflow(asm_p0_attr, XCT_ASM_P0,     queueIdx, hdrType, asm_p0_fstat, BC_PTM_BONDING_DISABLE);
      rc = rc? rc : add_xtmflow(asm_p1_attr, XCT_ASM_P1,     queueIdx, hdrType, asm_p1_fstat, BC_PTM_BONDING_DISABLE);
      rc = rc? rc : add_xtmflow(asm_p2_attr, XCT_ASM_P2,     queueIdx, hdrType, asm_p2_fstat, BC_PTM_BONDING_DISABLE);
      rc = rc? rc : add_xtmflow(asm_p3_attr, XCT_ASM_P3,     queueIdx, hdrType, asm_p3_fstat, BC_PTM_BONDING_DISABLE);
   }
   else
   {
      UINT32 packet_fstat = calc_xtmflow_fstat(XCT_PTM, 0, 0, flag);

      if (trafficType == TRAFFIC_TYPE_PTM_BONDED) {
         printk ("bcmxtmrt: Traffic Type is PTM Bonded.  TxPAF control-%d. \n", (unsigned int) ulTxPafEnabled) ;
         rc = rc? rc : add_xtmflow(packet_attr, XCT_PTM, queueIdx, hdrType, packet_fstat, BC_PTM_BONDING_ENABLE) ;
      }
      else {
         printk ("bcmxtmrt: Traffic Type is Non-bonded-%d. \n", (unsigned int) trafficType) ;
         rc = rc? rc : add_xtmflow(packet_attr, XCT_PTM, queueIdx, hdrType, packet_fstat, BC_PTM_BONDING_DISABLE) ;
      }
   }
   
   BCM_XTM_DEBUG ("xtm: hdrType = %x\n", HT_TYPE(hdrType));
   return rc;
   
}  /* cfg_xtmflows() */


/*---------------------------------------------------------------------------
 * int add_xtmflow(bdmf_object_handle attrs, UINT32 ctType, UINT32 queueIdx,
 *                 UINT32 fstat, int ptmBonding)
 * Description:
 *    This function adds a rdpa xtmflow for the xtmchannel object.
 * Returns:
 *    0 or bdmf error code
 *---------------------------------------------------------------------------
 */
static int add_xtmflow(bdmf_object_handle attrs, UINT32 ctType, UINT32 queueIdx,
                       UINT32 hdrType, UINT32 fstat, int ptmBonding)
{
   int   rc = 0;
   bdmf_number             index;
   bdmf_object_handle      xtmflow;
   rdpa_xtmflow_us_cfg_t   us_cfg;

   /* set xtmflow index */
   index = (MAX_CIRCUIT_TYPES * queueIdx) + ctType;
   rc = rdpa_xtmflow_index_set(attrs, index);
   
   /* set xtmflow hdrType */
   rc = rc? rc : rdpa_xtmflow_hdr_type_set(attrs, HT_TYPE(hdrType));
   
   /* set xtmflow fstat */
   rc = rc? rc : rdpa_xtmflow_fstat_set(attrs, fstat);
   
   /* don't need to set ds_cfg */
   
   /* set us_cfg */
   us_cfg.xtmchannel = g_GlobalInfo.txBdmfObjs[queueIdx].xtmchannel;

   rc = rc? rc : rdpa_xtmflow_us_cfg_set(attrs, &us_cfg);
   
   /* set xtmflow ptmBonding */
   rc = rc? rc : rdpa_xtmflow_ptmBonding_set(attrs, ptmBonding);
   
   /* create wan flow */
   rc = rc? rc : bdmf_new_and_set(rdpa_xtmflow_drv(), NULL, attrs, &xtmflow);
   
   if (rc == 0)
   {
      g_GlobalInfo.txBdmfObjs[queueIdx].xtmflow[ctType] = xtmflow;
   }
   
   return rc; 
   
}  /* add_xtmflow() */


/*---------------------------------------------------------------------------
 * UINT32 calc_xtmflow_fstat(UINT32 ctType, UINT32 hdrType, UINT32 vcid,
 *                           UINT32 flag)
 * Description:
 *    This function returns the xtmflow fstat that has the same format
 *    as the tx dma buffer descriptor frame status word defined for
 *    SAR.
 * Returns:
 *    xtmflow fstat
 *---------------------------------------------------------------------------
 */
static UINT32 calc_xtmflow_fstat(UINT32 ctType, UINT32 hdrType, UINT32 vcid,
                                 UINT32 flag)
{
   UINT32 fstat;
   
   fstat = ctType << FSTAT_CT_SHIFT;

   switch (ctType)
   {
   case XCT_AAL5:
      fstat |= vcid;
      if (flag & CNI_USE_ALT_FSTAT)
      {
         fstat |= FSTAT_MODE_COMMON;
         if (HT_LEN(hdrType) && (flag & CNI_HW_ADD_HEADER))
         {
            fstat |= FSTAT_COMMON_INS_HDR_EN |
                     ((HT_TYPE(hdrType) - 1) << FSTAT_COMMON_HDR_INDEX_SHIFT);
         }
      }
      break;
      
   case XCT_PTM:
      fstat |= FSTAT_PTM_ENET_FCS | FSTAT_PTM_CRC;
      break;
      
   default:    /* atm cells */
      fstat |= vcid;
      if (flag & CNI_USE_ALT_FSTAT)
      {
         fstat |= FSTAT_MODE_COMMON;
      }
      break;
   }
   
   return fstat;
       
}  /* calc_xtmflow_fstat() */


/*---------------------------------------------------------------------------
 * int cfg_cpu_rx_queue(int queue_id, UINT32_t queue_size, void *rx_isr)
 * Description:
 *    Set cpu rx queue configuration including cpu rx queue isr.
 * Returns:
 *    0 or error code
 *---------------------------------------------------------------------------
 */
static int cfg_cpu_rx_queue(int queue_id, UINT32 queue_size, void *rx_isr)
{
   int rc;
   rdpa_cpu_rxq_cfg_t rxq_cfg = {};

   /* Read current configuration, set new drop threshold and ISR and write
    * back.
    */
   rc = rdpa_cpu_rxq_cfg_get(rdpa_cpu_xtm_obj, queue_id, &rxq_cfg);

   if (!rc) {

      rxq_cfg.size     = queue_size;
      rxq_cfg.isr_priv = queue_id;
      rxq_cfg.rx_isr   = (rdpa_cpu_rxq_rx_isr_cb_t)rx_isr;

      rc = bcmxapiex_ring_create_delete(queue_id, queue_size, &rxq_cfg);

      if (rc < 0)
         goto xtm_exit;

      rxq_cfg.ic_cfg.ic_enable = false; /* No coalescing for now */
      rc = rdpa_cpu_rxq_cfg_set(rdpa_cpu_xtm_obj, queue_id, &rxq_cfg);
   }

xtm_exit:
   if (!rc) 
      printk(KERN_NOTICE "Created XTM CPU-RUnner Rx queue %d with size %d\n", queue_id, (int)queue_size);
   else 
      printk(KERN_ERR "Failed to configure XTM CPU-Runner Rx queue %d with size %d \n", queue_id, (int) queue_size);

   return rc;

}  /* cfg_cpu_rx_queue() */


/*---------------------------------------------------------------------------
 * void cpu_rxq_isr(int queue_id)
 * Description:
 *    xtm receive interrupt service routine
 * Returns: void
 *---------------------------------------------------------------------------
 */
static void cpu_rxq_isr(int queue_id)
{
   /* disable and clear interrupts from both high and low rx queue */
   rdpa_cpu_int_disable(XTM_RDPA_CPU_PORT, RDPA_XTM_CPU_HI_RX_QUEUE_ID);
   rdpa_cpu_int_disable(XTM_RDPA_CPU_PORT, RDPA_XTM_CPU_LO_RX_QUEUE_ID);
   rdpa_cpu_int_clear(XTM_RDPA_CPU_PORT, RDPA_XTM_CPU_HI_RX_QUEUE_ID);
   rdpa_cpu_int_clear(XTM_RDPA_CPU_PORT, RDPA_XTM_CPU_LO_RX_QUEUE_ID);

   BCMXTMRT_WAKEUP_RXWORKER(&g_GlobalInfo);
}  /* cpu_rxq_isr() */


/**
** All the external functions are defined here.
**/


/*---------------------------------------------------------------------------
 * int bcmxapi_module_init(void)
 * Description:
 *    Called when the driver is loaded.
 * Returns:
 *    0 or error status
 *---------------------------------------------------------------------------
 */
int bcmxapi_module_init(void)
{
   int rc = 0;
   bdmf_object_handle system_obj = NULL;
   rdpa_cpu_reason   reason ;
   uint8_t tc=0, queue_id ;
   rdpa_cpu_tc rdpa_sys_tc_threshold = rdpa_cpu_tc1;
   int rdpa_cpu_tc_high;

   rc = rdpa_system_get(&system_obj);
   if (rc == 0)
   {
      if (rdpa_system_high_prio_tc_threshold_get(system_obj, &rdpa_sys_tc_threshold))
      {
        bdmf_put(system_obj);
        return -1;
      }

      rdpa_cpu_tc_high = rdpa_sys_tc_threshold + 1; 
      bdmf_put(system_obj);
   }
   else
   {
      printk(KERN_ERR CARDNAME ": Error(%d) getting system object\n", rc);
      return -EFAULT;
   }

   bcmxapiex_cpu_object_get (&rdpa_cpu_xtm_obj);

   rdpa_cpu_int_disable(XTM_RDPA_CPU_PORT, RDPA_XTM_CPU_LO_RX_QUEUE_ID);
   rdpa_cpu_int_clear(XTM_RDPA_CPU_PORT, RDPA_XTM_CPU_LO_RX_QUEUE_ID);

   /* configure cpu rx queues */
   rc = cfg_cpu_rx_queue(RDPA_XTM_CPU_LO_RX_QUEUE_ID, RDPA_XTM_CPU_RX_QUEUE_SIZE,
                         &cpu_rxq_isr);
   if (rc)
   {
      printk(KERN_ERR CARDNAME ": Error(%d) cfg CPU low priority rx queue\n", rc);
      return -EFAULT;
   }
    
   rdpa_cpu_int_disable(XTM_RDPA_CPU_PORT, RDPA_XTM_CPU_HI_RX_QUEUE_ID);
   rdpa_cpu_int_clear(XTM_RDPA_CPU_PORT, RDPA_XTM_CPU_HI_RX_QUEUE_ID);

   rc = cfg_cpu_rx_queue(RDPA_XTM_CPU_HI_RX_QUEUE_ID, RDPA_XTM_CPU_RX_QUEUE_SIZE,
                         &cpu_rxq_isr);
   if (rc)
   {
      printk(KERN_ERR CARDNAME ": Error(%d) cfg CPU high priority rx queue\n", rc);
      return -EFAULT;
   }

   for (reason = rdpa_cpu_reason_min; reason < rdpa_cpu_reason__num_of; reason++) {

      if ((reason == rdpa_cpu_rx_reason_oam) ||
          (reason == rdpa_cpu_rx_reason_omci) ||
          (reason == rdpa_cpu_rx_reason_direct_flow) ||
          (reason == rdpa_cpu_rx_reason_ipsec) ||
          (reason == rdpa_cpu_rx_reason_tcpspdtst))
      {
         continue;
      }

      if (reason == rdpa_cpu_rx_reason_etype_pppoe_d ||
          reason == rdpa_cpu_rx_reason_etype_pppoe_s ||
          reason == rdpa_cpu_rx_reason_etype_arp ||
          reason == rdpa_cpu_rx_reason_etype_802_1ag_cfm ||
          reason == rdpa_cpu_rx_reason_l4_icmp ||
          reason == rdpa_cpu_rx_reason_icmpv6 ||
          reason == rdpa_cpu_rx_reason_igmp ||
          reason == rdpa_cpu_rx_reason_dhcp ||
          reason == rdpa_cpu_rx_reason_hit_trap_high ||
          reason == rdpa_cpu_rx_reason_ingqos ||
          reason == rdpa_cpu_rx_reason_l4_udef_0)
      {
         tc = rdpa_cpu_tc_high ;
         queue_id = RDPA_XTM_CPU_HI_RX_QUEUE_ID ;
      }
      else {
         tc = XTM_RDPA_CPU_TC_LOW ;
         queue_id = RDPA_XTM_CPU_LO_RX_QUEUE_ID ;
      }

      rc = rc ? rc : bcmxapiex_cfg_cpu_ds_queues (reason, tc, queue_id);

      if (rc)
      {
         printk(KERN_ERR CARDNAME ": Error(%d) cfg CPU reason to TC to priority rx queue\n", rc);
         printk(KERN_ERR CARDNAME "failed to set Map TC to RXQ, error: %d, RDPA reason %d, TC %d, CPU RXQ %d\n", rc,
               (int) reason, (int)tc, (int)queue_id);
         break;
      }

   } /* for (reason) */

   return rc;
   
}  /* bcmxapi_module_init() */


/*---------------------------------------------------------------------------
 * void bcmxapi_module_cleanup(void)
 * Description:
 *    Called when the driver is unloaded.
 * Returns: void
 *---------------------------------------------------------------------------
 */
void bcmxapi_module_cleanup(void)
{
   bcmxapi_disable_rx_interrupt();
   
   bdmf_put(rdpa_cpu_xtm_obj);

}  /* bcmxapi_module_cleanup() */


/*---------------------------------------------------------------------------
 * int bcmxapi_enable_rx_interrupt(void)
 * Description:
 *    Enable cpu rx queue interrupt
 * Returns:
 *    0 or error code
 *---------------------------------------------------------------------------
 */
int bcmxapi_enable_rx_interrupt(void)
{
   rdpa_cpu_int_enable(XTM_RDPA_CPU_PORT, RDPA_XTM_CPU_LO_RX_QUEUE_ID);
   rdpa_cpu_int_enable(XTM_RDPA_CPU_PORT, RDPA_XTM_CPU_HI_RX_QUEUE_ID);
    
   return 0;
   
}  /* bcmxapi_enable_rx_interrupt() */


/*---------------------------------------------------------------------------
 * int bcmxapi_disable_rx_interrupt(void)
 * Description:
 *    Disable cpu rx queue interrupt
 * Returns:
 *    0 or error code
 *---------------------------------------------------------------------------
 */
int bcmxapi_disable_rx_interrupt(void)
{
   rdpa_cpu_int_disable(XTM_RDPA_CPU_PORT, RDPA_XTM_CPU_LO_RX_QUEUE_ID);
   rdpa_cpu_int_disable(XTM_RDPA_CPU_PORT, RDPA_XTM_CPU_HI_RX_QUEUE_ID);
   rdpa_cpu_rxq_flush_set(rdpa_cpu_xtm_obj, RDPA_XTM_CPU_LO_RX_QUEUE_ID, TRUE);
   rdpa_cpu_rxq_flush_set(rdpa_cpu_xtm_obj, RDPA_XTM_CPU_HI_RX_QUEUE_ID, TRUE);
   rdpa_cpu_int_clear(XTM_RDPA_CPU_PORT, RDPA_XTM_CPU_LO_RX_QUEUE_ID);
   rdpa_cpu_int_clear(XTM_RDPA_CPU_PORT, RDPA_XTM_CPU_HI_RX_QUEUE_ID);

   return 0;
   
}  /* bcmxapi_disable_rx_interrupt() */


/*---------------------------------------------------------------------------
 * UINT32 bcmxapi_rxtask(UINT32 ulBudget, UINT32 *pulMoreToDo)
 * Description:
 *    xtm receive task called RX thread.
 * Returns:
 *    0 - success, Error code - failure
 *---------------------------------------------------------------------------
 */
UINT32 bcmxapi_rxtask(UINT32 ulBudget, UINT32 *pulMoreToDo)
{
   int rc;
   
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   PBCMXTMRT_DEV_CONTEXT pDevCtx;
   struct sk_buff *skb;
   unsigned long irqFlags;
   UINT32 ulCell;
   UINT32 ulVcId;
   UINT32 ulMoreToReceive;
   UINT32 ulRxPktGood = 0;
   UINT32 ulRxPktProcessed = 0;
   UINT32 ulRxPktMax = ulBudget + (ulBudget / 2);
   rdpa_cpu_rx_info_t info = {};
   FkBuff_t *pFkb = NULL;
   UINT32 flow_key = 0;
#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS)
    fc_class_ctx_t fc_key;
#endif
   
   /* Receive packets from every receive queue in a round robin order until
   * there are no more packets to receive.
   */
   do
   {
      ulMoreToReceive = 0;

      if (ulBudget == 0)
      {
         *pulMoreToDo = 1;
         break;
      }

      spin_lock_irqsave(&pGi->xtmlock_rx, irqFlags);

      /* Receive packets from high priority cpu rx queue */
      rc = bcmxapiex_get_pkt_from_ring (RDPA_XTM_CPU_HI_RX_QUEUE_ID, &pFkb, &info);

      if (rc == BDMF_ERR_NO_MORE)
      {
          /* Receive packet from low priority cpu rx queue */
          rc = bcmxapiex_get_pkt_from_ring (RDPA_XTM_CPU_LO_RX_QUEUE_ID, &pFkb, &info);

          if (rc == BDMF_ERR_NO_MORE)
          {
             ulRxPktGood |= XTM_POLL_DONE;
             spin_unlock_irqrestore(&pGi->xtmlock_rx, irqFlags);
             break;
          }
          else if (!rc) 
          {
              g_xtm_rxq_stats_received[XTM_RDPA_CPU_LO_RX_QUEUE_IDX]++;
          }
          else
          {
              spin_unlock_irqrestore(&pGi->xtmlock_rx, irqFlags);
              g_xtm_rxq_stats_dropped[XTM_RDPA_CPU_LO_RX_QUEUE_IDX]++;
              printk(KERN_ERR "Error in bcmxapiex_get_pkt_from_ring() qid %d rc (%d)\n", 
                     RDPA_XTM_CPU_LO_RX_QUEUE_ID, rc);
              goto drop_pkt;
          }
      }
      else if (!rc)
      {
          g_xtm_rxq_stats_received[XTM_RDPA_CPU_HI_RX_QUEUE_IDX]++;
      }
      else
      {
          spin_unlock_irqrestore(&pGi->xtmlock_rx, irqFlags);
          g_xtm_rxq_stats_dropped[XTM_RDPA_CPU_HI_RX_QUEUE_IDX]++;
          printk(KERN_ERR "Error in bcmxapiex_get_pkt_from_ring () qid %d rc (%d)\n", 
                 RDPA_XTM_CPU_HI_RX_QUEUE_ID, rc);
          goto drop_pkt;
      }
      
      spin_unlock_irqrestore(&pGi->xtmlock_rx, irqFlags);
      ulRxPktProcessed++;
      
      ulVcId = ((info.reason_data>>FSTAT_MATCH_ID_SHIFT) & FSTAT_MATCH_ID_MASK) ;
      pDevCtx = pGi->pDevCtxsByMatchId[ulVcId] ;
      ulCell  = (info.reason_data & FSTAT_PACKET_CELL_MASK) == FSTAT_CELL;

      /* error status, or packet with no pDev */
      if (((info.reason_data & FSTAT_ERROR) != 0) ||
          ((!ulCell) && (pDevCtx == NULL)))   /* packet */
      {
         if (ulVcId == TEQ_DATA_VCID && pGi->pTeqNetDev)
         {
            unsigned long flags;

            /* create a sysb and initilize it with packet data & len */
            skb = bdmf_sysb_header_alloc(bdmf_sysb_skb, (uint8_t *)info.data, info.size, 0, 0);
            if (!skb)
            {
                /* free the data buffer */
                bdmf_sysb_databuf_free((uint8_t *)info.data, 0);

                printk("%s:sysb_header allocation failed\n",__FUNCTION__);
                goto drop_pkt;
            }

            skb = bdmf_sysb_2_fkb_or_skb(skb);          
                                                
            /* Sending TEQ data to interface told to us by DSL Diags */
            skb->dev      = pGi->pTeqNetDev;
            skb->protocol = htons(ETH_P_802_3);
            local_irq_save(flags);
            local_irq_enable();
            dev_queue_xmit(skb);
            local_irq_restore(flags);
         }
         else
         {
            //DUMP_PKT(skb->data, skb->len);
            /* free the data buffer */
            bdmf_sysb_databuf_free((uint8_t *)info.data, 0);
            if (pDevCtx)
               pDevCtx->DevStats.rx_errors++;
         }
      }
      else if (!ulCell) /* process packet, pDev != NULL */
      {
#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS)
         if (!info.is_exception)
         {
             BCM_XTM_DEBUG("Pkt_len=%d, offset=%x, reason=%d\n", info.size, info.data_offset, info.reason);
                 
             /* Extract flow_ctxt from hit_trap hybrid flow */
             info.data_offset += L2_FLOW_P_LEN;
             info.size -= L2_FLOW_P_LEN;
        
             flow_key = *((uint32_t*)info.data);
             fc_key.word = flow_key;
             fc_key.id.src_port = 0x0;
             flow_key = fc_key.word;
        
             info.data += L2_FLOW_P_LEN;
             BCM_XTM_DEBUG("Flw_key=%x, data=%p, Pkt_len=%d\n", flow_key, info.data, info.size);
         }
#endif
         bcmxtmrt_processRxPkt(pDevCtx, NULL, (UINT8 *)info.data,
                               (UINT16)info.reason_data, info.size, flow_key);
         ulRxPktGood++;
         ulBudget--;
      }
      else  /* process cell */
      {
         bcmxtmrt_processRxCell((UINT8 *)info.data);
         bdmf_sysb_databuf_free((uint8_t *)info.data, 0);
      }
drop_pkt:
      if (ulRxPktProcessed >= ulRxPktMax)
         break;
      else
         ulMoreToReceive = 1; /* more packets to receive on Rx queue? */
      
   } while (ulMoreToReceive);

   return (ulRxPktGood);
}  /* bcmxtmrt_rxtask() */


/*---------------------------------------------------------------------------
 * int bcmxapi_add_proc_files(void)
 * Description:
 *    Adds proc file system directories and entries.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
int bcmxapi_add_proc_files(void)
{
   return (bcmxapiex_add_proc_files());
}  /* bcmxapi_add_proc_files() */


/*---------------------------------------------------------------------------
 * int bcmxapi_del_proc_files(void)
 * Description:
 *    Deletes proc file system directories and entries.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
int bcmxapi_del_proc_files(void)
{
   return (bcmxapiex_del_proc_files());
}  /* bcmxapi_del_proc_files() */


/*---------------------------------------------------------------------------
 * int bcmxapi_DoGlobInitReq(PXTMRT_GLOBAL_INIT_PARMS pGip)
 * Description:
 *    Processes an XTMRT_CMD_GLOBAL_INITIALIZATION command.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
int bcmxapi_DoGlobInitReq(PXTMRT_GLOBAL_INIT_PARMS pGip)
{
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   int rc = 0;
   
   if (pGi->ulDrvState != XTMRT_UNINITIALIZED)
      return -EPERM;

   bcmLog_setLogLevel(BCM_LOG_ID_XTM, BCM_LOG_LEVEL_ERROR);

   spin_lock_init(&pGi->xtmlock_tx);
   spin_lock_init(&pGi->xtmlock_rx);
   spin_lock_init(&pGi->xtmlock_rx_regs);

   /* Save MIB counter/Cam registers. */
   pGi->pulMibTxOctetCountBase = pGip->pulMibTxOctetCountBase;
   pGi->ulMibRxClrOnRead       = pGip->ulMibRxClrOnRead;
   pGi->pulMibRxCtrl           = pGip->pulMibRxCtrl;
   pGi->pulMibRxMatch          = pGip->pulMibRxMatch;
   pGi->pulMibRxOctetCount     = pGip->pulMibRxOctetCount;
   pGi->pulMibRxPacketCount    = pGip->pulMibRxPacketCount;
   pGi->pulRxCamBase           = pGip->pulRxCamBase;
   
   pGi->bondConfig.uConfig = pGip->bondConfig.uConfig;
   if ((pGi->bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE) ||
       (pGi->bondConfig.sConfig.atmBond == BC_ATM_BONDING_ENABLE))
      printk(CARDNAME ": PTM/ATM Bonding Mode configured in system\n");
   else
      printk(CARDNAME ": PTM/ATM Non-Bonding Mode configured in system\n");

   pGi->atmBondSidMode = ATMBOND_ASM_MESSAGE_TYPE_NOSID;

   /* Initialize a timer function for TEQ */
   init_timer(&pGi->Timer);
   pGi->Timer.data     = (unsigned long)pGi;
   pGi->Timer.function = (void *)bcmxtmrt_timer;

   printk("E-RXIntr\n");
   bcmxapi_enable_rx_interrupt();

   /* create and initialize xtm objects */
   rc = runner_xtm_objects_init();
   if (rc)
      rc = -EFAULT;
   else
      pGi->ulDrvState = XTMRT_INITIALIZED;
      
   return rc;
   
}  /* bcmxapi_DoGlobInitReq() */


/*---------------------------------------------------------------------------
 * int bcmxapi_DoGlobUninitReq(void)
 * Description:
 *    Processes an XTMRT_CMD_GLOBAL_UNINITIALIZATION command.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
int bcmxapi_DoGlobUninitReq(void)
{
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

   if (pGi->ulDrvState == XTMRT_UNINITIALIZED)
      return -EPERM;
      
   del_timer_sync(&pGi->Timer);
      
   runner_xtm_objects_uninit();
     
   printk("D-RXIntr\n");
   bcmxapi_disable_rx_interrupt();

   pGi->ulDrvState = XTMRT_UNINITIALIZED;
      
   return 0;
   
}  /* bcmxapi_DoGlobUninitReq() */


/*---------------------------------------------------------------------------
 * int bcmxapi_DoSetTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
 *                          PXTMRT_TRANSMIT_QUEUE_ID pTxQId)
 * Description:
 *    Allocate memory for and initialize a transmit queue.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
int bcmxapi_DoSetTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                         PXTMRT_TRANSMIT_QUEUE_ID pTxQId)
{
   int rc = 0, queueIdx;
   UINT32 ulPort;
   BcmPktDma_XtmTxDma  *txdma;
   bdmf_object_handle   egress_tm ;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

   BCM_XTM_DEBUG("DoSetTxQueue\n");

   local_bh_enable();  // needed to avoid kernel error

   txdma = (BcmPktDma_XtmTxDma *)kzalloc(sizeof(BcmPktDma_XtmTxDma), GFP_ATOMIC);

   local_bh_disable();

   if (txdma == NULL) {
      printk(KERN_ERR "Unable to allocate memory for xtm tx dma info\n");
      return -ENOMEM;
   }

   ulPort = PORTID_TO_PORT(pTxQId->ulPortId);

   if ((ulPort < MAX_PHY_PORTS) && (pTxQId->ucSubPriority < MAX_SUB_PRIORITIES)) {

      /* Configure a TM egress queue */
      queueIdx  = pTxQId->ulQueueIndex;
      egress_tm = pGi->txBdmfObjs[queueIdx].egress_tm;

      if (egress_tm == NULL) {

         rc = runner_tx_queue_init (pGi->bdmfXtm, queueIdx);
         if (rc) 
            goto _End ;
         egress_tm = pGi->txBdmfObjs[queueIdx].egress_tm;
      }/* egress_tm */

      if (egress_tm != NULL) {

         rdpa_tm_queue_cfg_t  queue_cfg = {};

         queue_cfg.queue_id       = queueIdx;
         queue_cfg.weight         = 0;
         printk ("\n bcmxtmrt:  Tx Q Size = %d \n", pTxQId->usQueueSize) ;
         queue_cfg.drop_threshold = QUEUE_DROP_THRESHOLD(pTxQId->usQueueSize) ;
         queue_cfg.stat_enable    = 1;
         queue_cfg.best_effort    = (queueIdx == 0);
         queue_cfg.reserved_packet_buffers = QUEUE_RESV_BUFFERS(pTxQId->ucSubPriority);

         if (pTxQId->ucDropAlg == WA_RED)
         {
            /* Currently Runner firmware does not support RED.
             * RED can be implemented as WRED with equal low and high
             * class thresholds.
             */
            queue_cfg.drop_alg = rdpa_tm_drop_alg_wred;
            queue_cfg.low_class.min_threshold  = (pTxQId->ucLoMinThresh * queue_cfg.drop_threshold) / 100;
            queue_cfg.low_class.max_threshold  = (pTxQId->ucLoMaxThresh * queue_cfg.drop_threshold) / 100;
            queue_cfg.high_class.min_threshold = queue_cfg.low_class.min_threshold;
            queue_cfg.high_class.max_threshold = queue_cfg.low_class.max_threshold;
         }
         else if (pTxQId->ucDropAlg == WA_WRED)
         {
            queue_cfg.drop_alg = rdpa_tm_drop_alg_wred;
            queue_cfg.low_class.min_threshold  = (pTxQId->ucLoMinThresh * queue_cfg.drop_threshold) / 100;
            queue_cfg.low_class.max_threshold  = (pTxQId->ucLoMaxThresh * queue_cfg.drop_threshold) / 100;
            queue_cfg.high_class.min_threshold = (pTxQId->ucHiMinThresh * queue_cfg.drop_threshold) / 100;
            queue_cfg.high_class.max_threshold = (pTxQId->ucHiMaxThresh * queue_cfg.drop_threshold) / 100;
         }
         else
         {
            /* DT */
            queue_cfg.drop_alg = rdpa_tm_drop_alg_dt;
            queue_cfg.low_class.min_threshold  = 0;
            queue_cfg.low_class.max_threshold  = 0;
            queue_cfg.high_class.min_threshold = 0;
            queue_cfg.high_class.max_threshold = 0;
         }

         printk("bcmxtmrt: Egress TM Q %d Setup. Buffering/Q - %d \n", (unsigned int)queueIdx, 
                (unsigned int) queue_cfg.drop_threshold );
         if ((rc = rdpa_egress_tm_queue_cfg_set(egress_tm, 0, &queue_cfg)))
         {
            printk(CARDNAME "DoSetTxQueue: rdpa_egress_tm_queue_cfg_set error rc=%d\n", rc);
         }
         /* create rdpa xtmflows for this queue */
         else if (cfg_xtmflows(queueIdx, pDevCtx->ulTrafficType, pDevCtx->ulHdrType,
                               pDevCtx->ucTxVcid, pDevCtx->ulTxPafEnabled) != 0)
         {
            printk(CARDNAME "DoSetTxQueue: Failed to create xtmflows for Q %d\n",
                   (unsigned int)queueIdx);
            rc = -EFAULT;
         }
         else
         {
            UINT32 ulPtmPrioIdx = PTM_FLOW_PRI_LOW;

            txdma->ulPort        = ulPort;
            txdma->ulPtmPriority = pTxQId->ulPtmPriority;
            txdma->ulSubPriority = pTxQId->ucSubPriority;
            txdma->ucDropAlg     = pTxQId->ucDropAlg;
            txdma->ucLoMinThresh = pTxQId->ucLoMinThresh;
            txdma->ucLoMaxThresh = pTxQId->ucLoMaxThresh;
            txdma->ucHiMinThresh = pTxQId->ucHiMinThresh;
            txdma->ucHiMaxThresh = pTxQId->ucHiMaxThresh;
            txdma->ulQueueSize   = pTxQId->usQueueSize ;
           
            if ((pDevCtx->Addr.ulTrafficType == TRAFFIC_TYPE_PTM) ||
                (pDevCtx->Addr.ulTrafficType == TRAFFIC_TYPE_PTM_BONDED))
               ulPtmPrioIdx = (txdma->ulPtmPriority == PTM_PRI_HIGH)?
                              PTM_FLOW_PRI_HIGH : PTM_FLOW_PRI_LOW;

            pDevCtx->pTxPriorities[ulPtmPrioIdx][ulPort][txdma->ulSubPriority] = txdma;
            pDevCtx->pTxQids[pTxQId->ucQosQId] = txdma;

            if (pDevCtx->pHighestPrio == NULL ||
                pDevCtx->pHighestPrio->ulSubPriority < txdma->ulSubPriority)
               pDevCtx->pHighestPrio = txdma;

            /* Increment channels per dev context */
            pDevCtx->txdma[pDevCtx->ulTxQInfosSize++] = txdma;

            txdma->ulDmaIndex = queueIdx;
            txdma->txEnabled  = 1;

            bcmxapiex_SetOrStartTxQueue(&queue_cfg, egress_tm) ;

            return 0;
         }
      } /* if (egress_tm != NULL) */
   } /* if ((ulPort < MAX_PHY_PORTS) && (pTxQId->ucSubPriority < MAX_SUB_PRIORITIES)) */

_End :
   kfree(txdma);
   return rc;

}  /* bcmxapi_DoSetTxQueue() */


/*---------------------------------------------------------------------------
 * void bcmxapi_ShutdownTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
 *                              volatile BcmPktDma_XtmTxDma *txdma)
 * Description:
 *    Flush a transmit queue and delete all the wan flows associated
 *    with the queue.
 * Returns: void
 * Notes:
 *    pDevCtx is not used.
 *---------------------------------------------------------------------------
 */
void bcmxapi_ShutdownTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                             volatile BcmPktDma_XtmTxDma *txdma)
{
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   UINT32   queueIdx;
   bdmf_object_handle   egress_tm;

   /* Changing txEnabled to 0 prevents any more packets
    * from being queued on a transmit channel. Allow all currenlty
    * queued transmit packets to be transmitted before disabling the DMA.
    */
   txdma->txEnabled = 0;
   
   queueIdx = txdma->ulDmaIndex ;
   
   egress_tm = pGi->txBdmfObjs[queueIdx].egress_tm;

   if (egress_tm != NULL)
      bcmxapiex_ShutdownTxQueue (queueIdx, egress_tm) ;

   runner_tx_queue_uninit (queueIdx) ;

}  /* bcmxapi_ShutdownTxQueue() */

                             
/*---------------------------------------------------------------------------
 * void bcmxapi_FlushdownTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
 *                               volatile BcmPktDma_XtmTxDma *txdma)
 * Description:
 *    Flush a transmit queue and delete all the wan flows associated
 *    with the queue.
 * Returns: void
 * Notes:
 *    pDevCtx is not used.
 *---------------------------------------------------------------------------
 */
void bcmxapi_FlushdownTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                              volatile BcmPktDma_XtmTxDma *txdma)
{
   /* for runner case, we do not need flush operation due to the following */
   /* We either run in CPU mode that means nothing to delete from runners as
    * CPU is in control as well as in runner mode, we always run with TxPAF,
    * which needs data on only one port and it will distribute onto available
    * ports automatically. No need to flush the data on the down port, as there 
    * may nothing pending
    */

   return ;
}  /* bcmxapi_FlushdownTxQueue() */


/*---------------------------------------------------------------------------
 * void bcmxapi_StopTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
 *                          volatile BcmPktDma_XtmTxDma *txdma)
 * Description:
 *    Stop a transmit queue.
 * Returns: void
 * Notes:
 *    pDevCtx is not used.
 *---------------------------------------------------------------------------
 */
void bcmxapi_StopTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                         volatile BcmPktDma_XtmTxDma *txdma)
{
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   UINT32   queueIdx;
   bdmf_object_handle   egress_tm;

   /* Changing txEnabled to 0 prevents any more packets
    * from being queued on a transmit channel. Allow all currenlty
    * queued transmit packets to be transmitted before disabling the DMA.
    */
   txdma->txEnabled = 0;
   
   queueIdx  = txdma->ulDmaIndex;
   
   /* Set the tx queue suze to 0 by setting drop threshold to 0
    * This will be set back when the Q gets (re)created.
    */
   egress_tm = pGi->txBdmfObjs[queueIdx].egress_tm;
   if (egress_tm != NULL)
   {
      int   rc = 0;
      rdpa_tm_queue_cfg_t  queue_cfg = {};
      
      queue_cfg.queue_id                 = queueIdx;
      queue_cfg.weight                   = 0;
      queue_cfg.drop_alg                 = rdpa_tm_drop_alg_dt;
      queue_cfg.drop_threshold           = 0;
      queue_cfg.stat_enable              = 1;
      queue_cfg.high_class.min_threshold = 0;
      queue_cfg.high_class.max_threshold = 0;
      queue_cfg.low_class.min_threshold  = 0;
      queue_cfg.low_class.max_threshold  = 0;
      queue_cfg.reserved_packet_buffers  = 0;
      queue_cfg.best_effort              = (queueIdx == 0);

      if ((rc = rdpa_egress_tm_queue_cfg_set(egress_tm, 0, &queue_cfg)))
      {
         printk(CARDNAME "StopTxQueue: rdpa_egress_tm_queue_cfg_set error rc=%d\n", rc);
      }

      bcmxapiex_StopTxQueue(&queue_cfg, egress_tm) ;

   } /* egress_tm */

}  /* bcmxapi_StopTxQueue() */


/*---------------------------------------------------------------------------
 * void bcmxapi_StartTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
 *                          volatile BcmPktDma_XtmTxDma *txdma)
 * Description:
 *    Start a transmit queue.
 * Returns: void
 * Notes:
 *    pDevCtx is not used.
 *---------------------------------------------------------------------------
 */
void bcmxapi_StartTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                          volatile BcmPktDma_XtmTxDma *txdma)
{
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   UINT32   queueIdx;
   bdmf_object_handle   egress_tm;

   queueIdx  = txdma->ulDmaIndex;
   
   egress_tm = pGi->txBdmfObjs[queueIdx].egress_tm;
   if (egress_tm != NULL)
   {
      int   rc = 0;
      rdpa_tm_queue_cfg_t  queue_cfg = {};
      
      queue_cfg.queue_id       = queueIdx;
      queue_cfg.weight         = 0;
      printk ("\n bcmxtmrt:  Start Tx Q Size = %d \n", txdma->ulQueueSize) ;
      queue_cfg.drop_threshold = QUEUE_DROP_THRESHOLD(txdma->ulQueueSize) ;
      queue_cfg.stat_enable    = 1;
      queue_cfg.best_effort    = (queueIdx == 0);
      queue_cfg.reserved_packet_buffers = QUEUE_RESV_BUFFERS(txdma->ulSubPriority);

      if (txdma->ucDropAlg == WA_RED)
      {
         /* Currently Runner firmware does not support RED.
          * RED can be implemented as WRED with equal low and high
          * class thresholds.
          */
         queue_cfg.drop_alg = rdpa_tm_drop_alg_wred;
         queue_cfg.low_class.min_threshold  = (txdma->ucLoMinThresh * queue_cfg.drop_threshold) / 100;
         queue_cfg.low_class.max_threshold  = (txdma->ucLoMaxThresh * queue_cfg.drop_threshold) / 100;
         queue_cfg.high_class.min_threshold = queue_cfg.low_class.min_threshold;
         queue_cfg.high_class.max_threshold = queue_cfg.low_class.max_threshold;
      }
      else if (txdma->ucDropAlg == WA_WRED)
      {
         queue_cfg.drop_alg = rdpa_tm_drop_alg_wred;
         queue_cfg.low_class.min_threshold  = (txdma->ucLoMinThresh * queue_cfg.drop_threshold) / 100;
         queue_cfg.low_class.max_threshold  = (txdma->ucLoMaxThresh * queue_cfg.drop_threshold) / 100;
         queue_cfg.high_class.min_threshold = (txdma->ucHiMinThresh * queue_cfg.drop_threshold) / 100;
         queue_cfg.high_class.max_threshold = (txdma->ucHiMaxThresh * queue_cfg.drop_threshold) / 100;
      }
      else
      {
         /* DT */
         queue_cfg.drop_alg = rdpa_tm_drop_alg_dt;
         queue_cfg.low_class.min_threshold  = 0;
         queue_cfg.low_class.max_threshold  = 0;
         queue_cfg.high_class.min_threshold = 0;
         queue_cfg.high_class.max_threshold = 0;
      }


      if ((rc = rdpa_egress_tm_queue_cfg_set(egress_tm, 0, &queue_cfg)))
      {
         printk(CARDNAME "StartTxQueue: rdpa_egress_tm_queue_cfg_set error rc=%d\n", rc);
      }

      bcmxapiex_SetOrStartTxQueue(&queue_cfg, egress_tm) ;
   }/* egress_tm */

}  /* bcmxapi_StartTxQueue() */

/*---------------------------------------------------------------------------
 * int bcmxapi_SetPortShaperInfo (PBCMXTMRT_GLOBAL_INFO pGi)
 * Description:
 *    Set/UnSet the global port shaper information.
 * Returns: void
 * Notes:
 *    pDevCtx is not used.
 *---------------------------------------------------------------------------
 */
int bcmxapi_SetTxPortShaperInfo(PBCMXTMRT_GLOBAL_INFO pGi, PXTMRT_PORT_SHAPER_INFO pShaperInfo)
{
   int nRet = -1 ;
   bdmf_object_handle   Xtm_Orl_tm;

   Xtm_Orl_tm  = pGi->bdmfXtm_Orl_Tm ;

   BCM_XTM_DEBUG("bcmxtmrt: Set TxPortShaper") ;
   
   if (Xtm_Orl_tm != NULL)
   {
      nRet = bcmxapiex_runner_xtm_orl_rl_set (Xtm_Orl_tm, pShaperInfo) ;
   }/* Xtm_Orl_tm */
   
   return (nRet) ;

}  /* bcmxapi_SetTxPortShaper () */

/*---------------------------------------------------------------------------
 * void bcmxapi_SetPtmBondPortMask(UINT32 portMask)
 * Description:
 *    Set the value of portMask in ptmBondInfo data structure.
 * Returns: void
 *---------------------------------------------------------------------------
 */
void bcmxapi_SetPtmBondPortMask(UINT32 portMask)
{
   g_GlobalInfo.ptmBondInfo.portMask = portMask;
   
}  /* bcmxapi_SetPtmBondPortMask() */


/*---------------------------------------------------------------------------
 * void bcmxapi_SetPtmBonding(UINT32 bonding)
 * Description:
 *    Set the value of bonding in ptmBondInfo data structure.
 * Returns: void
 *---------------------------------------------------------------------------
 */
void bcmxapi_SetPtmBonding(UINT32 bonding)
{
   g_GlobalInfo.ptmBondInfo.bonding = bonding;
   
}  /* bcmxapi_SetPtmBonding() */


/*---------------------------------------------------------------------------
 * void bcmxapi_XtmGetStats(UINT8 vport, UINT32 *rxDropped, UINT32 *txDropped)
 * Description:
 *
 * Returns: void
 *---------------------------------------------------------------------------
 */
void bcmxapi_XtmGetStats(UINT8 vport, UINT32 *rxDropped, UINT32 *txDropped)
{
    //FIXME: This Will be fixed in the future releases as this logic needs to
    //be added to map the netdevice to corresponding queues.
    rdpa_port_stat_t stat = {};
    int rc;

    /* rxDiscards not supported */
    *rxDropped = 0;

    /* only vport 0 is supported on Runner */
    if (vport != 0)
    {
       *txDropped = 0;
       return;
    }
    
    bdmf_lock();

    /* Read RDPA statistic */
    rc = rdpa_port_stat_get(g_GlobalInfo.bdmfWan, &stat);
    if (rc )
    {
       printk("rdpa_port_stat_get returned error rc %d\n",rc);
       goto unlock_exit;
    }

    /* Add up the TX discards */
    *txDropped += stat.tx_discard + stat.discard_pkt;

unlock_exit:
    bdmf_unlock();
}  /* bcmxapi_XtmGetStats() */

/*---------------------------------------------------------------------------
 * void bcmxapi_XtmResetStats(UINT8 vport)
 * Description:
 *
 * Returns: void
 *---------------------------------------------------------------------------
 */
void bcmxapi_XtmResetStats(UINT8 vport)
{
    rdpa_port_stat_t stat = {};
    int rc;

    /* Only vport 0 is supported on Runner */
    if (vport != 0) 
    {
       return;
    }

    bdmf_lock();

    /* Clear RDPA statistic */
    rc = rdpa_port_stat_set(g_GlobalInfo.bdmfWan, &stat);
    if (rc)
    {
        printk("rdpa_port_stat_set returned error rc %d\n", rc);
    }

    bdmf_unlock();

    return;
}  /* bcmxapi_XtmResetStats() */

#if defined(CONFIG_BLOG)
/*---------------------------------------------------------------------------
 * void bcmxapi_blog_ptm_us_bonding (UINT32 ulTxPafEnabled, sk_buff *skb)
 * Description:
 *
 * Returns: void
 *---------------------------------------------------------------------------
 */
void bcmxapi_blog_ptm_us_bonding(UINT32 ulTxPafEnabled, struct sk_buff *skb)
{
   blog_ptm_us_bonding (skb, BLOG_PTM_US_BONDING_ENABLED) ;
}  /* bcmxapi_blog_ptm_us_bonding() */
#endif
