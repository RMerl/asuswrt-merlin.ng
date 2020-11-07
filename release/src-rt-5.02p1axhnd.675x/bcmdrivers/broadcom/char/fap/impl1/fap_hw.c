/*
<:copyright-BRCM:2012:proprietary:standard

   Copyright (c) 2012 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
  :>
*/

#include "bcm_OS_Deps.h"
#include "fap_hw.h"
#include <linux/bcm_log.h>

void fapHostHw_PrintRegs(uint32 fapIdx, fapRegGroups_t regGroup)
{
  int i;
  if (regGroup==FAP_CNTRL_REG || regGroup==FAP_ALL_REG)
  {
    hwPrint("CoprocCtlRegs (FAP%u)\n", fapIdx);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), irq_4ke_mask);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), irq_4ke_status);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), irq_mips_mask);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), irq_mips_status);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), gp_mask);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), gp_status);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), gp_tmr0_ctl);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), gp_tmr0_cnt);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), gp_tmr1_ctl);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), gp_tmr1_cnt);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), host_mbox_in);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), host_mbox_out);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), gp_out);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), gp_in);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), gp_in_irq_mask);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), gp_in_irq_status);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), dma_control);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), dma_status);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), dma0_3_fifo_status);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), diag_cntrl);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), diag_hi);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), diag_lo);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), bad_address);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), addr1_mask);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), addr1_base_in);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), addr1_base_out);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), addr2_mask);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), addr2_base_in);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), addr2_base_out);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), scratch);
#if defined(CONFIG_BCM963268)
    FAP_PRINT_REG(hostRegCntrl(fapIdx), tm_ctl);
#endif
    FAP_PRINT_REG(hostRegCntrl(fapIdx), soft_resets);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), eb2ubus_timeout);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), m4ke_core_status);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), gp_in_irq_sense);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), ub_slave_timeout);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), diag_en);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), dev_timeout);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), ubus_error_out_mask);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), diag_capt_stop_mask);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), rev_id);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), gp_tmr2_ctl);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), gp_tmr2_cnt);
#if defined(CONFIG_BCM963268)
    FAP_PRINT_REG(hostRegCntrl(fapIdx), legacy_mode);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), smisb_mon);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), diag_ctl);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), diag_stat);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), diag_mask);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), diag_result);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), diag_compare);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), diag_capture);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), daig_count);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), diag_edge_cnt);
#if defined(CONFIG_BCM963268)
    FAP_PRINT_REG(hostRegCntrl(fapIdx), hw_counter_0);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), hw_counter_1);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), hw_counter_2);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), hw_counter_3);
#endif
    FAP_PRINT_REG(hostRegCntrl(fapIdx), lfsr);
#endif

  }

  if (regGroup==FAP_OGMSG_REG || regGroup==FAP_ALL_REG)
  {
    hwPrint("OGMsgFifoRegs (FAP%u)\n", fapIdx);
    FAP_PRINT_REG(hostOgMsgReg(fapIdx), og_msg_ctl);
    FAP_PRINT_REG(hostOgMsgReg(fapIdx), og_msg_sts);
//    FAP_PRINT_REG(hostOgMsgReg(fapIdx), og_msg_data);
//    FAP_PRINT_REG(hostOgMsgReg(fapIdx), og_msg_data_cont); // only for 268
  }

  if (regGroup==FAP_INMSG_REG || regGroup==FAP_ALL_REG)
  {
    hwPrint("INMsgFifoRegs (FAP%u)\n", fapIdx);
    FAP_PRINT_REG(hostInMsgReg(fapIdx), in_msg_ctl);
    FAP_PRINT_REG(hostInMsgReg(fapIdx), in_msg_sts);
//    FAP_PRINT_REG(hostInMsgReg(fapIdx), in_msg_last);
//    FAP_PRINT_REG(hostInMsgReg(fapIdx), in_msg_data);
  }
  
  if (regGroup==FAP_DMA_REG || regGroup==FAP_ALL_REG)
  {
    hwPrint("DmaRegs[0] (FAP%u)\n", fapIdx);
    FAP_PRINT_REG_ELEM(hostDmaReg(fapIdx)->dma_ch[0], dma_source);
    FAP_PRINT_REG_ELEM(hostDmaReg(fapIdx)->dma_ch[0], dma_dest);
    FAP_PRINT_REG_ELEM(hostDmaReg(fapIdx)->dma_ch[0], dma_cmd_list);
    FAP_PRINT_REG_ELEM(hostDmaReg(fapIdx)->dma_ch[0], dma_len_ctl);
    FAP_PRINT_REG_ELEM(hostDmaReg(fapIdx)->dma_ch[0], dma_rslt_source);
    FAP_PRINT_REG_ELEM(hostDmaReg(fapIdx)->dma_ch[0], dma_rslt_dest);
    FAP_PRINT_REG_ELEM(hostDmaReg(fapIdx)->dma_ch[0], dma_rslt_hcs);
    FAP_PRINT_REG_ELEM(hostDmaReg(fapIdx)->dma_ch[0], dma_rslt_len_stat);

    hwPrint("DmaRegs[1] (FAP%u)\n", fapIdx);
    FAP_PRINT_REG_ELEM(hostDmaReg(fapIdx)->dma_ch[1], dma_source);
    FAP_PRINT_REG_ELEM(hostDmaReg(fapIdx)->dma_ch[1], dma_dest);
    FAP_PRINT_REG_ELEM(hostDmaReg(fapIdx)->dma_ch[1], dma_cmd_list);
    FAP_PRINT_REG_ELEM(hostDmaReg(fapIdx)->dma_ch[1], dma_len_ctl);
    FAP_PRINT_REG_ELEM(hostDmaReg(fapIdx)->dma_ch[1], dma_rslt_source);
    FAP_PRINT_REG_ELEM(hostDmaReg(fapIdx)->dma_ch[1], dma_rslt_dest);
    FAP_PRINT_REG_ELEM(hostDmaReg(fapIdx)->dma_ch[1], dma_rslt_hcs);
    FAP_PRINT_REG_ELEM(hostDmaReg(fapIdx)->dma_ch[1], dma_rslt_len_stat);
  }

  if (regGroup==FAP_TKNINTF_REG || regGroup==FAP_ALL_REG)
  {
    hwPrint("TknIntfRegs (FAP%u)\n", fapIdx);
    FAP_PRINT_REG(hostTknIntfReg(fapIdx), tok_buf_size);
    FAP_PRINT_REG(hostTknIntfReg(fapIdx), tok_buf_base);
    FAP_PRINT_REG(hostTknIntfReg(fapIdx), tok_idx2ptr_idx);
    FAP_PRINT_REG(hostTknIntfReg(fapIdx), tok_idx2ptr_ptr);
  }

  if (regGroup==FAP_MSGID_REG || regGroup==FAP_ALL_REG)
  {
    hwPrint("MsgIdRegs (FAP%u)\n", fapIdx);
      for (i=0; i<64; i++)
    {
        hwPrint("  msg_id[%d]            0x%08x\n", i, hostMsgIdReg(fapIdx)->msg_id[i]);
    }
  }

  if (regGroup==FAP_DQM_REG || regGroup==FAP_ALL_REG)
  {
    hwPrint("DQMCtlRegs (FAP%u)\n", fapIdx);
    FAP_PRINT_REG(hostDqmReg(fapIdx), cfg);
    FAP_PRINT_REG(hostDqmReg(fapIdx), _4ke_low_wtmk_irq_msk);
    FAP_PRINT_REG(hostDqmReg(fapIdx), mips_low_wtmk_irq_msk);
    FAP_PRINT_REG(hostDqmReg(fapIdx), low_wtmk_irq_sts);
    FAP_PRINT_REG(hostDqmReg(fapIdx), _4ke_not_empty_irq_msk);
    FAP_PRINT_REG(hostDqmReg(fapIdx), mips_not_empty_irq_msk);
    FAP_PRINT_REG(hostDqmReg(fapIdx), not_empty_irq_sts);
    FAP_PRINT_REG(hostDqmReg(fapIdx), queue_rst);
    FAP_PRINT_REG(hostDqmReg(fapIdx), not_empty_sts);
    FAP_PRINT_REG(hostDqmReg(fapIdx), next_avail_mask);
    FAP_PRINT_REG(hostDqmReg(fapIdx), next_avail_queue);
  }

  if (regGroup==FAP_DQMQCNTRL_REG || regGroup==FAP_ALL_REG)
  {
    hwPrint("DQMQCntrlRegs (FAP%u)\n", fapIdx);
      for (i=0; i<32; i++)
    {
        hwPrint("  DQM Q%d\n", i);
        hwPrint("    size                0x%08x\n", hostDqmQCntrlReg(fapIdx)->q[i].size);
        hwPrint("    cfgA                0x%08x\n", hostDqmQCntrlReg(fapIdx)->q[i].cfgA);
        hwPrint("    cfgB                0x%08x\n", hostDqmQCntrlReg(fapIdx)->q[i].cfgB);
        hwPrint("    status              0x%08x\n", hostDqmQCntrlReg(fapIdx)->q[i].sts);
    }
  }

  if (regGroup==FAP_DQMQDATA_REG || regGroup==FAP_ALL_REG)
  {
    hwPrint("DQMQDataRegs (FAP%u)\n", fapIdx);
      for (i=0; i<32; i++)
    {
        hwPrint("  DQM Q%d\n", i);
        hwPrint("    word0               0x%08x\n", hostDqmQDataReg(fapIdx)->q[i].word0);
        hwPrint("    word1               0x%08x\n", hostDqmQDataReg(fapIdx)->q[i].word1);
        hwPrint("    word2               0x%08x\n", hostDqmQDataReg(fapIdx)->q[i].word2);
        hwPrint("    word3               0x%08x\n", hostDqmQDataReg(fapIdx)->q[i].word3);
    }
  }

  if (regGroup==FAP_DQMQMIB_REG || regGroup==FAP_ALL_REG)
  {
    hwPrint("DQMQMibRegs (FAP%u)\n", fapIdx);
      for (i=0; i<32; i++)
    {
        hwPrint("  DQM Q%d\n", i);
        hwPrint("    MIB_NumFull         0x%08x\n", hostDqmQMibReg(fapIdx)->MIB_NumFull[i]);
        hwPrint("    MIB_NumEmpty        0x%08x\n", hostDqmQMibReg(fapIdx)->MIB_NumEmpty[i]);
        hwPrint("    MIB_TokensPushed    0x%08x\n", hostDqmQMibReg(fapIdx)->MIB_TokensPushed[i]);
    }
  }  
}


