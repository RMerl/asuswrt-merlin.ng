/*
  <:copyright-BRCM:2015:proprietary:standard
  
     Copyright (c) 2015 Broadcom 
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

/****************************************************************************
 *
 * merlin_serdes_debug.c -- BCM Merlin Serdes Debug Method
 *
 * Description:
 *	This file contains the debug implementation for Merlin Serdes
 *    work with the script "serdesctrl"
 *
 * Authors: Lei Cai
 *
 * $Revision: 1.1 $
 *
 * $Id: serdes_debug.c,v 1.1 2015/12/30 LeiCai Exp $
 *
 * $Log: serdes_debug.c,v $
 * Revision 1.1  2015/12/30 LeiCai
 * Initial version.
 *
 ****************************************************************************/

#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include "lport_defs.h"
#include "lport_drv.h"
#include "merlin_serdes.h"
#include "serdes_access.h"
#include "merlin_mptwo_interface.h"
#include "merlin_mptwo_functions.h"

#define PROC_CMD_MAX_LEN 64

static uint16_t g_reg_read_addr = 0;
static uint16_t g_merlin_id = 0;

static ssize_t proc_serdes_loopback_set(struct file *file, const char *buff, size_t len, loff_t *offset)
{
    char input[PROC_CMD_MAX_LEN];
    uint32_t lane_index, mode, enable;
    int ret=0;

    void help(void)
    {
        printk("    Params: lane_index loopback_mode enable\n");
        printk("    - lane_index: <0-3>\n"
               "    - mode:    0:none, 1:pcs local, 2:pcs remote, 3:pmd local, 4:pmd remote\n"
               "    - enable:  <0-1>\n");
    }
    
    if (len > PROC_CMD_MAX_LEN)
        len = PROC_CMD_MAX_LEN;

    if (copy_from_user(input, buff, len) != 0)
        return -EFAULT;

    if (strncasecmp(input, "help", 4) == 0)
    {
        help();
        return len;
    }

    ret = sscanf(input, "%x %x %x", &lane_index, &mode, &enable);
    if(3 == ret)
    {
        printk("\nlane_index=%d mode=%d, eanble=%dx\n", lane_index, mode, enable);
        merlin_loopback_set(lane_index, mode, enable);
    }
    else
    {
        printk("Error format!");
        help();
        return -EFAULT;
    }

    return len;
}

static ssize_t proc_serdes_reg_get(struct file *file, char *buff, size_t len, loff_t *offset)
{
    int ret=0;
    uint16_t val;

    printk("\nmerlin=%d, addr=0x%08x\n", g_merlin_id, g_reg_read_addr);
    if (0 == *offset)
    {
        read_serdes_reg(g_merlin_id, g_reg_read_addr, 0xFFFF, &val);
        *offset = sprintf(buff, "%04x", val);
        ret = *offset;
    }
    return ret;
}

static ssize_t proc_serdes_reg_set(struct file *file, const char *buff, size_t len, loff_t *offset)
{
    char input[PROC_CMD_MAX_LEN];
    uint32_t merlin_id, addr, val;
    int ret=0;

    void help(void)
    {
        printk("    Params: merlin_id<0-1> address(Hex) value(Hex)\n");
    }
    
    if (len > PROC_CMD_MAX_LEN)
        len = PROC_CMD_MAX_LEN;

    if (copy_from_user(input, buff, len) != 0)
        return -EFAULT;

    if (strncasecmp(input, "help", 4) == 0)
    {
        help();
        return len;
    }

    ret = sscanf(input, "%x %x %x", &merlin_id, &addr, &val);
    if (2 == ret)
    {
        g_merlin_id = merlin_id;
        g_reg_read_addr = addr;
    }
    else if(3 == ret)
    {
        printk("\nmerlin=%d addr=0x%08x, val=0x%04x\n", merlin_id, addr, val);
        write_serdes_reg(merlin_id, addr, 0xFFFF, val);
    }
    else
    {
        printk("Error format!");
        help();
        return -EFAULT;
    }

    return len;
}

static ssize_t proc_serdes_prbs_get(struct file *file, char *buff, size_t len, loff_t *offset)
{
    int ret=0;
    uint8_t locked=0;
    uint16_t lane_index=0;
    merlin_prbs_stats_t stats;

    if (0 == *offset)
    {
        merlin_prbs_check(lane_index, &locked, &stats);
        printk("\nprbs_tx_pkt %d, prbs_rx_pkt %d, prbs_error %d, crcerrcnt %d\n", stats.prbs_tx_pkt,
                        stats.prbs_rx_pkt, stats.prbs_error, stats.crcerrcnt);
        *offset = sprintf(buff, "\nlocked = %d", locked);
        ret = *offset;
    }
    return ret;
}

static ssize_t proc_serdes_prbs_set(struct file *file, const char *buff, size_t len, loff_t *offset)
{
    uint32_t lane_index;
    merlin_prbs_mode_t mode;
    uint8_t enable;
    char input[PROC_CMD_MAX_LEN];
    
    void help(void)
    {
        printk("    Params: lane_index mode enable\n");
        printk("    - lane_index: <0-3>\n"
               "    - mode: 0:PRBS_7, 1:PRBS_9, 2:PRBS_11, 3:PRBS_15, 4:PRBS_23, 5:PRBS_31, 6:PRBS_58\n");
    }
    if (len > PROC_CMD_MAX_LEN)
        len = PROC_CMD_MAX_LEN;

    if (copy_from_user(input, buff, len) != 0)
        return -EFAULT;

    if (strncasecmp(input, "help", 4) == 0)
    {
        help();
        return len;
    }

    if (sscanf(input, "%d %d %d", &lane_index, &mode, &enable) != 3)
    {
        printk("Error format!");
        help();
        return -EFAULT;
    }

    printk("lane_index=%d, mode=%d, enable=%d\n", lane_index, mode, enable);
    merlin_prbs_enable_set(lane_index, mode, enable);

    return len;
}

static ssize_t proc_serdes_display(struct file *file, const char *buff, size_t len, loff_t *offset)
{
    char input[PROC_CMD_MAX_LEN]={0}, *ptr;
    int ret=0;

    void help(void)
    {
        printk("    Params: <diag lane_index diag_level>\n");
        printk("    - diag: display diag data\n"
               "    - lane_index: <0 - 3>\n"
               "    - diag_level: <0 - 19>\n");
        printk("    Params: <config lane_index>\n");
        printk("    - lane_index: <0 - 3>\n");
    }

    if (len > PROC_CMD_MAX_LEN)
        len = PROC_CMD_MAX_LEN;

    if (copy_from_user(input, buff, len) != 0)
        return -EFAULT;

    if (strncasecmp(input, "help", 4) == 0)
    {
        help();
        return len;
    }
    
    if (strncasecmp(input, "diag", 4) == 0)
    {
        merlin_access_t ma_in, *ma=&ma_in;
        uint32_t  diag_level;
        uint32_t lane_index;
        ptr = &input[4];
        ret = sscanf(ptr, "%d %d", &lane_index, &diag_level);
        if (ret != 2)
        {
            printk("Error format\n!");
            help();
            return -EFAULT;
        }
        printk("\nlane %d, diag_level=%d\n", lane_index, diag_level);
        ma->index = lane_index;
        merlin_mptwo_display_diag_data(ma, diag_level);
    }
    else if(strncasecmp(input, "config", 5) == 0)
    {    
        merlin_access_t ma_in, *ma=&ma_in;
        uint32_t lane_index;
	err_code_t __err;
        uint8_t data;
        ptr = &input[5];
        ret = sscanf(ptr, "%d", &lane_index);
        if (ret != 1)
        {
            printk("Error format\n!");
            help();
            return -EFAULT;
        }
        ma->index = lane_index;
        printk("Lane %d configuration: \n",lane_index);
        
        data = rd_pcs_dig_use_ieee_reg_ctrl_sel();
        printk("%d ",data);
        data = rd_pcs_an_sgmii_master_mode();
        printk("%d ",data);
        data = rd_pcs_an_cl37_enable();
        printk("%d ",data);
        data = rd_pcs_an_cl37_sgmii_enable();
        printk("%d ",data);
        data = rd_pcs_dig_credit_sw_en();
        printk("%d ",data);
        data = rd_pcs_dig_sw_actual_speed_force_en();
        printk("%d ",data);
       	data = rd_pcs_rx_rstb_lane();
        printk("%d ",data);
        data = rd_pcs_dig_mac_creditenable();
        printk("%d ",data);
        data = rd_pcs_tx_rstb_tx_lane();
        printk("%d ",data);
        data = rd_pcs_tx_rstb_tx_lane();
        printk("%d ",data);
        printk("\n");
    }
    else
    {
        printk("Unknow cmds: %s\n", input);
        help();
        return -EFAULT;
    }

    return len;
}

static ssize_t proc_serdes_control(struct file *file, const char *buff, size_t len, loff_t *offset)
{
    uint16_t lane_index;
    merlin_command_t cmd;
    merlin_control_t control;
    char input[PROC_CMD_MAX_LEN];
    
    void help(void)
    {
        printk("    Params: lane_index cmd\n");
        printk("    - lane_index: <0-3>\n"
               "    - cmd: 5:stats , 6:status\n");
    }
    if (len > PROC_CMD_MAX_LEN)
        len = PROC_CMD_MAX_LEN;

    if (copy_from_user(input, buff, len) != 0)
        return -EFAULT;

    if (strncasecmp(input, "help", 4) == 0)
    {
        help();
        return len;
    }

    if (sscanf(input, "%d %d", &lane_index, &cmd) != 2)
    {
        printk("Error format!");
        help();
        return -EFAULT;
    }
    
    memset(&control, 0, sizeof(merlin_control_t));
    printk("lane_index=%d, cmd=%d\n", lane_index, cmd);
    merlin_control(lane_index, cmd, &control);

    switch(cmd)
    {
        case MERLIN_CMD_STATUS_GET:
            printk("tx_LOCAL_FAULT: %d\n", control.status.tx_LOCAL_FAULT);
            printk("tx_REMOTE_FAULT: %d\n", control.status.tx_REMOTE_FAULT);
            printk("PMD_LOCK: %d\n", control.status.PMD_LOCK);
            printk("signal_ok: %d\n", control.status.signal_ok);
            printk("rx_LOCAL_FAULT: %d\n", control.status.rx_LOCAL_FAULT);
            printk("rx_REMOTE_FAULT: %d\n", control.status.rx_REMOTE_FAULT);
            printk("rx_LINK_STATUS: %d\n", control.status.rx_LINK_STATUS);
            printk("rx_SYNC_STATUS: %d\n", control.status.rx_SYNC_STATUS);
            printk("pll_lock: %d\n", control.status.pll_lock);
            printk("cl36_syncacq_state_coded_per_ln 0x%x\n",control.status.cl36_syncacq_state_coded_per_ln);
            printk("cl36_syncacq_his_state_per_ln: 0x%x\n",control.status.cl36_syncacq_his_state_per_ln);
        break;
        case MERLIN_CMD_STATS_GET:
            printk("\nkcode66ErrCount: %d\n", control.stats.kcode66ErrCount);
            printk("sync66ErrCount: %d\n", control.stats.sync66ErrCount);
            printk("cl49ieee_errored_blocks: %d\n", control.stats.cl49ieee_errored_blocks);
            printk("BER_count_per_ln: %d\n", control.stats.BER_count_per_ln);
            printk("cl49_valid_sh_cnt: %d\n", control.stats.cl49_valid_sh_cnt);
            printk("cl49_invalid_sh_cnt: %d\n", control.stats.cl49_invalid_sh_cnt);
            printk("fec_corrected: %d\n", control.stats.fec_corrected);
            printk("fec_uncorrected: %d\n", control.stats.fec_uncorrected);
        break;
        default:
            printk("Not implemented yet\n");
        break;
    }
    return len;
}

static struct file_operations serdes_reg_proc =
{
    .read  = proc_serdes_reg_get,
    .write = proc_serdes_reg_set,
};

static struct file_operations serdes_prbs_proc =
{
    .read  = proc_serdes_prbs_get,
    .write = proc_serdes_prbs_set,
};

static struct file_operations serdes_display_proc =
{
    .write = proc_serdes_display,
};

static struct file_operations serdes_control_proc =
{
    .write = proc_serdes_control,
};

static struct file_operations serdes_loopback_proc =
{
    .write = proc_serdes_loopback_set,
};

int serdes_debug_init(void)
{
    struct proc_dir_entry *p0;
    struct proc_dir_entry *p1;
    struct proc_dir_entry *p2;

    p0 = proc_mkdir("lport_serdes", NULL);
    if (!p0)
    {
        printk("failed to create /proc/serdes !");
        return -1;
    }

    p1 = proc_mkdir("lan", p0);
    if (!p1)
    {
        printk("failed to create /proc/serdes/lan !");
        return -1;
    }

    p2 = proc_create("reg", S_IWUSR | S_IRUSR, p1, &serdes_reg_proc);
    if (!p2)
    {
        printk("failed to create /proc/serdes/lan/reg !");
        return -1;
    }

    p2 = proc_create("prbs", S_IWUSR | S_IRUSR, p1, &serdes_prbs_proc);
    if (!p2)
    {
        printk("failed to create /proc/serdes/lan/prbs !");
        return -1;
    }

    p2 = proc_create("display", S_IWUSR | S_IRUSR, p1, &serdes_display_proc);
    if (!p2)
    {
        printk("failed to create /proc/serdes/lan/display !");
        return -1;
    }
    
    p2 = proc_create("control", S_IWUSR | S_IRUSR, p1, &serdes_control_proc);
    if (!p2)
    {
        printk("failed to create /proc/serdes/lan/display !");
        return -1;
    }

    p2 = proc_create("loopback", S_IWUSR | S_IRUSR, p1, &serdes_control_proc);
    if (!p2)
    {
        printk("failed to create /proc/serdes/lan/display !");
        return -1;
    }
    
    return 0;
}

