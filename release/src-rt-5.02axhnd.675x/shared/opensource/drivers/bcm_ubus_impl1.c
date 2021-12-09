/*
 * <:copyright-BRCM:2017:DUAL/GPL:standard
 * 
 *    Copyright (c) 2017 Broadcom 
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

/* BCM UBUS4 supporting routines */

#include "bcm_map_part.h"
#include "bcm_ubus4.h"
#ifdef _CFE_
#include "lib_types.h"
#include "lib_printf.h"
#define printk  printf
#define udelay  cfe_usleep
#else // Linux
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>
#include "bcm_physical_map_part.h"
#include "board.h"
#include "bcm_rsvmem.h"
#include <linux/bug.h>
#include "enum_str.h"
#endif

//#define ENABLE_UBUS_REMAP_DEBUG_LOG     
#ifdef  ENABLE_UBUS_REMAP_DEBUG_LOG
#define UBUS_REMAP_DEBUG_LOG(fmt, ...) printk("%s:%d "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define UBUS_REMAP_DEBUG_LOG(fmt, ...)  
#endif

#ifdef CONFIG_BCM_UBUS_DECODE_REMAP
#define DECODE_WIN0 0
#define DECODE_WIN1 1
#define CACHE_BIT_OFF 0
#define CACHE_BIT_ON 1
#endif

extern unsigned long getMemorySize(void);
#define MST_START_DDR_ADDR              0

#ifndef _CFE_
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define PROC_DIR            "driver/ubus"
#define UBUS_DECODE_FILE      "decode_cfg"
#define UBUS_TOKENS_FILE      "tokens"
static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_decode_cfg, *proc_tokens;

static ssize_t ubus_decode_get_proc(struct file *file, char *buff, size_t len, loff_t *offset);
static ssize_t ubus_tokens_get_proc(struct file *file, char *buff, size_t len, loff_t *offset);

static const struct file_operations ubus_decode_proc_fops = {
    .read  = ubus_decode_get_proc,
};

static const struct file_operations ubus_tokens_proc_fops = {
    .read  = ubus_tokens_get_proc,
};
#endif

#ifdef CONFIG_BCM_UBUS_DECODE_REMAP
/* CONFIG_BCM_FPM_COHERENCY_EXCLUDE is the case when all the memory is coherent except for FPM pool,
 * since CCI-400 won't sustain 10G traffic load. In that case we remap FPM pool area to be non-coherent
 */
#if defined(CONFIG_BCM_FPM_COHERENCY_EXCLUDE)
static uint32_t fpm_pool_addr;
static uint32_t fpm_pool_size;
#endif /* CONFIG_BCM_FPM_COHERENCY_EXCLUDE */

unsigned int g_board_size_power_of_2;
EXPORT_SYMBOL(g_board_size_power_of_2);
#endif /* CONFIG_BCM_UBUS_DECODE_REMAP */

ub_mst_addr_map_t ub_mst_addr_map_tbl[] =
{
#if defined (CONFIG_BCM963158) || defined(_BCM963158_)
    {UBUS_PORT_ID_BIU,    MST_PORT_NODE_B53_PHYS_BASE},
    {UBUS_PORT_ID_PER,    MST_PORT_NODE_PER_PHYS_BASE},
    {UBUS_PORT_ID_USB,    MST_PORT_NODE_USB_PHYS_BASE},
    {UBUS_PORT_ID_SPU,    MST_PORT_NODE_SPU_PHYS_BASE},
    {UBUS_PORT_ID_DSL,    MST_PORT_NODE_DSL_PHYS_BASE},
    {UBUS_PORT_ID_PERDMA, MST_PORT_NODE_PER_DMA_PHYS_BASE},
    {UBUS_PORT_ID_PCIE0,  MST_PORT_NODE_PCIE0_PHYS_BASE},
    {UBUS_PORT_ID_PCIE2,  MST_PORT_NODE_PCIE2_PHYS_BASE},
    {UBUS_PORT_ID_PCIE3,  MST_PORT_NODE_PCIE3_PHYS_BASE},
    {UBUS_PORT_ID_DSLCPU, MST_PORT_NODE_DSLCPU_PHYS_BASE},
    {UBUS_PORT_ID_PMC,    MST_PORT_NODE_PMC_PHYS_BASE},
    {UBUS_PORT_ID_SWH,    MST_PORT_NODE_SWH_PHYS_BASE},
    {UBUS_PORT_ID_QM,     MST_PORT_NODE_QM_PHYS_BASE},
    {UBUS_PORT_ID_DQM,    MST_PORT_NODE_DQM_PHYS_BASE},
    {UBUS_PORT_ID_DMA0,   MST_PORT_NODE_DMA0_PHYS_BASE},
    {UBUS_PORT_ID_NATC,   MST_PORT_NODE_NATC_PHYS_BASE},
    {UBUS_PORT_ID_RQ0,    MST_PORT_NODE_RQ0_PHYS_BASE},
#elif defined (CONFIG_BCM963178) || defined(_BCM963178_)
    {UBUS_PORT_ID_BIU,    MST_PORT_NODE_CPU_PHYS_BASE},
    {UBUS_PORT_ID_PER,    MST_PORT_NODE_PER_PHYS_BASE},
    {UBUS_PORT_ID_USB,    MST_PORT_NODE_USB_PHYS_BASE},
    {UBUS_PORT_ID_DSL,    MST_PORT_NODE_DSL_PHYS_BASE},
    {UBUS_PORT_ID_DSLCPU, MST_PORT_NODE_DSLCPU_PHYS_BASE},
    {UBUS_PORT_ID_PCIE0,  MST_PORT_NODE_PCIE0_PHYS_BASE},
    {UBUS_PORT_ID_PMC,    MST_PORT_NODE_PMC_PHYS_BASE},
    {UBUS_PORT_ID_SWH,    MST_PORT_NODE_SWH_PHYS_BASE},
    {UBUS_PORT_ID_WIFI,   MST_PORT_NODE_WIFI_PHYS_BASE},
#elif defined (CONFIG_BCM947622) || defined(_BCM947622_)
    {UBUS_PORT_ID_BIU,    MST_PORT_NODE_CPU_PHYS_BASE},
    {UBUS_PORT_ID_PER,    MST_PORT_NODE_PER_PHYS_BASE},
    {UBUS_PORT_ID_USB,    MST_PORT_NODE_USB_PHYS_BASE},
    {UBUS_PORT_ID_PCIE0,  MST_PORT_NODE_PCIE0_PHYS_BASE},
    {UBUS_PORT_ID_PMC,    MST_PORT_NODE_PMC_PHYS_BASE},
    {UBUS_PORT_ID_SYSPORT,  MST_PORT_NODE_SYSPORT_PHYS_BASE},
    {UBUS_PORT_ID_SYSPORT1, MST_PORT_NODE_SYSPORT1_PHYS_BASE},
    {UBUS_PORT_ID_WIFI,   MST_PORT_NODE_WIFI_PHYS_BASE},
    {UBUS_PORT_ID_WIFI1,  MST_PORT_NODE_WIFI1_PHYS_BASE},
    {UBUS_PORT_ID_SPU,    MST_PORT_NODE_SPU_PHYS_BASE},
#elif defined (CONFIG_BCM96858) || defined(_BCM96858_)
    {UBUS_PORT_ID_BIU,       MST_PORT_NODE_B53_PHYS_BASE},
    {UBUS_PORT_ID_PER,       MST_PORT_NODE_PER_PHYS_BASE},
    {UBUS_PORT_ID_USB,       MST_PORT_NODE_USB_PHYS_BASE},
    {UBUS_PORT_ID_PERDMA,    MST_PORT_NODE_PER_DMA_PHYS_BASE},
    {UBUS_PORT_ID_SPU,       MST_PORT_NODE_SPU_PHYS_BASE},
    {UBUS_PORT_ID_PCIE0,     MST_PORT_NODE_PCIE0_PHYS_BASE},
    {UBUS_PORT_ID_PCIE2,     MST_PORT_NODE_PCIE2_PHYS_BASE},
    {UBUS_PORT_ID_PMC,       MST_PORT_NODE_PMC_PHYS_BASE},
    {UBUS_PORT_ID_QM,        MST_PORT_NODE_QM_PHYS_BASE},
    {UBUS_PORT_ID_DQM,       MST_PORT_NODE_DQM_PHYS_BASE},
    {UBUS_PORT_ID_DMA0,      MST_PORT_NODE_DMA0_PHYS_BASE},
    {UBUS_PORT_ID_DMA1,      MST_PORT_NODE_DMA1_PHYS_BASE},
    {UBUS_PORT_ID_NATC,      MST_PORT_NODE_NATC_PHYS_BASE},
    {UBUS_PORT_ID_TOP_BUFF,  MST_PORT_NODE_TOP_BUFF_PHYS_BASE},
    {UBUS_PORT_ID_XRDP_BUFF, MST_PORT_NODE_XRDP_BUFF_PHYS_BASE},
    {UBUS_PORT_ID_RQ0,       MST_PORT_NODE_RQ0_PHYS_BASE},
    {UBUS_PORT_ID_RQ1,       MST_PORT_NODE_RQ1_PHYS_BASE},
    {UBUS_PORT_ID_RQ2,       MST_PORT_NODE_RQ2_PHYS_BASE},
    {UBUS_PORT_ID_RQ3,       MST_PORT_NODE_RQ3_PHYS_BASE},
#elif defined (CONFIG_BCM96878) || defined(_BCM96878_)
    {UBUS_PORT_ID_BIU,       MST_PORT_NODE_B53_PHYS_BASE},
    {UBUS_PORT_ID_PER,       MST_PORT_NODE_PER_PHYS_BASE},
    {UBUS_PORT_ID_USB,       MST_PORT_NODE_USB_PHYS_BASE},
    {UBUS_PORT_ID_WIFI,      MST_PORT_NODE_WIFI_PHYS_BASE},
    {UBUS_PORT_ID_PCIE0,     MST_PORT_NODE_PCIE0_PHYS_BASE},
    {UBUS_PORT_ID_QM,        MST_PORT_NODE_QM_PHYS_BASE},
    {UBUS_PORT_ID_DMA0,      MST_PORT_NODE_DMA0_PHYS_BASE},
    {UBUS_PORT_ID_RQ0,       MST_PORT_NODE_RQ0_PHYS_BASE},
#elif defined (CONFIG_BCM96846) || defined(_BCM96846_)
    {UBUS_PORT_ID_BIU,       MST_PORT_NODE_B53_PHYS_BASE},
    {UBUS_PORT_ID_PER,       MST_PORT_NODE_PER_PHYS_BASE},
    {UBUS_PORT_ID_USB,       MST_PORT_NODE_USB_PHYS_BASE},
    {UBUS_PORT_ID_PCIE0,     MST_PORT_NODE_PCIE0_PHYS_BASE},
    {UBUS_PORT_ID_QM,        MST_PORT_NODE_QM_PHYS_BASE},
    {UBUS_PORT_ID_DQM,       MST_PORT_NODE_DQM_PHYS_BASE},
    {UBUS_PORT_ID_DMA0,      MST_PORT_NODE_DMA0_PHYS_BASE},
    {UBUS_PORT_ID_NATC,      MST_PORT_NODE_NATC_PHYS_BASE},
    {UBUS_PORT_ID_RQ0,       MST_PORT_NODE_RQ0_PHYS_BASE},
#elif defined (CONFIG_BCM96856) || defined(_BCM96856_)
    {UBUS_PORT_ID_BIU,       MST_PORT_NODE_B53_PHYS_BASE},
    {UBUS_PORT_ID_PER,       MST_PORT_NODE_PER_PHYS_BASE},
    {UBUS_PORT_ID_USB,       MST_PORT_NODE_USB_PHYS_BASE},
    {UBUS_PORT_ID_PCIE2,     MST_PORT_NODE_PCIE2_PHYS_BASE},
    {UBUS_PORT_ID_PCIE0,     MST_PORT_NODE_PCIE0_PHYS_BASE},
    {UBUS_PORT_ID_QM,        MST_PORT_NODE_QM_PHYS_BASE},
    {UBUS_PORT_ID_DQM,       MST_PORT_NODE_DQM_PHYS_BASE},
    {UBUS_PORT_ID_DMA0,      MST_PORT_NODE_DMA0_PHYS_BASE},
    {UBUS_PORT_ID_NATC,      MST_PORT_NODE_NATC_PHYS_BASE},
    {UBUS_PORT_ID_DMA1,      MST_PORT_NODE_DMA1_PHYS_BASE},
    {UBUS_PORT_ID_RQ0,       MST_PORT_NODE_RQ0_PHYS_BASE},
    {UBUS_PORT_ID_RQ1,       MST_PORT_NODE_RQ1_PHYS_BASE},
#endif
    {-1, 0}
};

#ifndef _CFE_
val_to_str_t ubus_port_id_to_str[] = {
#if !defined(CONFIG_BCM96878)
    {UBUS_PORT_ID_MEMC, "MEMC"},
#endif
    {UBUS_PORT_ID_BIU, "BIU"},     
    {UBUS_PORT_ID_USB, "USB"},    
    {UBUS_PORT_ID_PER, "PER"},    
    {UBUS_PORT_ID_PCIE0, "PCIE0"},  
#if defined (CONFIG_BCM963178)
    {UBUS_PORT_ID_DSLCPU, "DSLCPU"},
    {UBUS_PORT_ID_DSL, "DSL"},  
    {UBUS_PORT_ID_WIFI, "WIFI"}, 
    {UBUS_PORT_ID_PMC, "PMC"},    
    {UBUS_PORT_ID_SWH, "SWH"},    
    {UBUS_PORT_ID_SYS, "SYS"}, 
#elif defined (CONFIG_BCM947622)
    {UBUS_PORT_ID_WIFI, "WIFI0"},
    {UBUS_PORT_ID_WIFI1, "WIFI1"},
    {UBUS_PORT_ID_PMC, "PMC"},    
    {UBUS_PORT_ID_SYSPORT, "SYSPORT0"},
    {UBUS_PORT_ID_SYSPORT1, "SYSPORT1"},
    {UBUS_PORT_ID_SYS, "SYS"}, 
    {UBUS_PORT_ID_SPU, "SPU"},
#else
    {UBUS_PORT_ID_DMA0, "DMA0"},   
#if !defined(CONFIG_BCM96878)
    {UBUS_PORT_ID_DQM, "DQM"},    
    {UBUS_PORT_ID_NATC, "NATC"},   
#endif
    {UBUS_PORT_ID_QM, "QM"},     
    {UBUS_PORT_ID_RQ0, "RQ0"},    
#if defined (CONFIG_BCM963158)
    {UBUS_PORT_ID_DSLCPU, "DSLCPU"}, 
    {UBUS_PORT_ID_DSL, "DSL"},
    {UBUS_PORT_ID_FPM, "FPM"},    
    {UBUS_PORT_ID_PCIE2, "PCIE2"},  
    {UBUS_PORT_ID_PCIE3, "PCIE3"},  
    {UBUS_PORT_ID_PERDMA, "PERDMA"}, 
    {UBUS_PORT_ID_PMC, "PMC"},    
    {UBUS_PORT_ID_PSRAM, "PSRAM"},  
    {UBUS_PORT_ID_SPU, "SPU"},    
    {UBUS_PORT_ID_SWH, "SWH"},    
    {UBUS_PORT_ID_SYS, "SYS"},    
    {UBUS_PORT_ID_SYSXRDP, "SYSXRDP"},
    {UBUS_PORT_ID_VPB, "VPB"},    
    {UBUS_PORT_ID_WAN, "WAN"},
#elif defined (CONFIG_BCM96858)
    {UBUS_PORT_ID_PERDMA, "PERDMA"},   
    {UBUS_PORT_ID_SPU, "SPU"},      
    {UBUS_PORT_ID_PCIE2, "PCIE2"},    
    {UBUS_PORT_ID_PMC, "PMC"},      
    {UBUS_PORT_ID_DMA1, "DMA1"},     
    {UBUS_PORT_ID_TOP_BUFF, "TOP_BUFF"}, 
    {UBUS_PORT_ID_XRDP_BUFF, "XRDP_BUFF"},
    {UBUS_PORT_ID_RQ1, "RQ1"},      
    {UBUS_PORT_ID_RQ2, "RQ2"},      
    {UBUS_PORT_ID_RQ3, "RQ3"},      
#elif defined (CONFIG_BCM96856)
    {UBUS_PORT_ID_PCIE2, "PCIE2"},
    {UBUS_PORT_ID_DMA1, "DMA1"}, 
    {UBUS_PORT_ID_RQ1, "RQ1"},  
#endif
#endif
};
#endif

void ubus_cong_threshold_wr(int port_id, unsigned int val)
{
    int i=0;

    while (ub_mst_addr_map_tbl[i].port_id != -1)
    {
        if (ub_mst_addr_map_tbl[i].port_id == port_id)
        {
            ((MstPortNode*) ub_mst_addr_map_tbl[i].base )->port_cfg[DCM_UBUS_CONGESTION_THRESHOLD] = val;
            break;
        }
        i++;
    }
}

/*ubus4 credit table */
#if defined(CONFIG_BCM963158)
static ubus_credit_cfg_t ubus_credit_tbl[UBUS_NUM_OF_MST_PORTS][UBUS_MAX_PORT_NUM+2] = {
    /* only includes the non default credit value, default is 4 in 63158 */
    /* The first credit data in each row inidicates the master port */
    { {UBUS_PORT_ID_BIU, -1}, {UBUS_PORT_ID_MEMC, 3}, {UBUS_PORT_ID_SYS, 1}, {UBUS_PORT_ID_PSRAM, 8}, {UBUS_PORT_ID_SYSXRDP, 1}, {-1,-1} }, 
    { {UBUS_PORT_ID_USB, -1}, {UBUS_PORT_ID_BIU, 2}, {UBUS_PORT_ID_SYS, 1}, {-1,-1} },                                                      
    { {UBUS_PORT_ID_PCIE0, -1}, {UBUS_PORT_ID_BIU, 4}, {UBUS_PORT_ID_PCIE0, 1}, {UBUS_PORT_ID_PER, 1}, {UBUS_PORT_ID_SYS, 1},               
      {UBUS_PORT_ID_FPM, 1}, {UBUS_PORT_ID_VPB, 1}, {UBUS_PORT_ID_SYSXRDP, 1}, {-1,-1} }, 
    { {UBUS_PORT_ID_PCIE3, -1}, {UBUS_PORT_ID_BIU, 6}, {UBUS_PORT_ID_SYS, 1}, {UBUS_PORT_ID_FPM, 1}, {UBUS_PORT_ID_VPB, 1}, {-1,-1} },      
    { {UBUS_PORT_ID_PCIE2, -1}, {UBUS_PORT_ID_BIU, 3}, {UBUS_PORT_ID_SYS, 1}, {UBUS_PORT_ID_FPM, 1}, {UBUS_PORT_ID_VPB, 1}, {-1,-1} },      
    { {UBUS_PORT_ID_PMC, -1}, {UBUS_PORT_ID_BIU, 1}, {UBUS_PORT_ID_MEMC, 1}, {UBUS_PORT_ID_USB, 1}, {UBUS_PORT_ID_PCIE0, 1},                
      {UBUS_PORT_ID_PCIE3, 1}, {UBUS_PORT_ID_PCIE2, 1}, {UBUS_PORT_ID_PER, 1}, {UBUS_PORT_ID_PMC, 1}, 
      {UBUS_PORT_ID_WAN, 1}, {UBUS_PORT_ID_SYS, 1}, {UBUS_PORT_ID_SWH, 1}, {UBUS_PORT_ID_SPU, 1}, 
      {UBUS_PORT_ID_DSL, 1}, {UBUS_PORT_ID_QM, 1}, {UBUS_PORT_ID_FPM, 1}, {UBUS_PORT_ID_VPB, 1}, 
      {UBUS_PORT_ID_PSRAM, 1}, {UBUS_PORT_ID_SYSXRDP, 1}, {-1,-1} },
    { {UBUS_PORT_ID_PER, -1}, {UBUS_PORT_ID_BIU, 1}, {UBUS_PORT_ID_SYS, 1}, {-1,-1} },                                                      
    { {UBUS_PORT_ID_PERDMA, -1}, {UBUS_PORT_ID_BIU, 5}, {UBUS_PORT_ID_PER, 1}, {UBUS_PORT_ID_SYS, 1}, {UBUS_PORT_ID_DSL, 4},                
      {UBUS_PORT_ID_PSRAM, 8}, {-1,-1} },
    { {UBUS_PORT_ID_DSLCPU, -1}, {UBUS_PORT_ID_MEMC, 8}, {UBUS_PORT_ID_PER, 1}, {UBUS_PORT_ID_WAN, 1}, {UBUS_PORT_ID_SYS, 1},               
      {UBUS_PORT_ID_DSL, 1}, {-1,-1} },
    { {UBUS_PORT_ID_DSL, -1}, {UBUS_PORT_ID_BIU, 1}, {UBUS_PORT_ID_PER, 1}, {UBUS_PORT_ID_WAN, 1}, {UBUS_PORT_ID_SYS, 1},                   
      {UBUS_PORT_ID_DSL, 1}, {-1,-1} },
    { {UBUS_PORT_ID_SPU, -1}, {UBUS_PORT_ID_BIU, 5}, {UBUS_PORT_ID_PER, 1}, {UBUS_PORT_ID_SYS, 1}, {UBUS_PORT_ID_DSL, 4},                   
      {UBUS_PORT_ID_PSRAM, 8}, {-1,-1} },
    { {UBUS_PORT_ID_QM, -1}, {UBUS_PORT_ID_BIU, 16}, {UBUS_PORT_ID_SYSXRDP, 1}, {-1,-1} },                                               
    { {UBUS_PORT_ID_DQM, -1}, {UBUS_PORT_ID_BIU, 1}, {UBUS_PORT_ID_FPM, 2}, {UBUS_PORT_ID_SYSXRDP, 1}, {-1,-1} },           
    { {UBUS_PORT_ID_NATC, -1}, {UBUS_PORT_ID_BIU, 2}, {UBUS_PORT_ID_SYSXRDP, 1}, {-1,-1} },                                                 
    { {UBUS_PORT_ID_DMA0, -1}, {UBUS_PORT_ID_BIU, 8}, {UBUS_PORT_ID_SYSXRDP, 1}, {-1,-1} },                                                 
    { {UBUS_PORT_ID_RQ0, -1}, {UBUS_PORT_ID_BIU, 11}, {UBUS_PORT_ID_USB, 1}, {UBUS_PORT_ID_PCIE0, 2}, {UBUS_PORT_ID_PCIE3, 2},               
      {UBUS_PORT_ID_PCIE2, 2}, {UBUS_PORT_ID_PER, 1}, {UBUS_PORT_ID_WAN, 1}, {UBUS_PORT_ID_SWH, 1}, 
      {UBUS_PORT_ID_SPU, 1}, {UBUS_PORT_ID_QM, 10}, {UBUS_PORT_ID_FPM, 2}, {UBUS_PORT_ID_VPB, 2}, 
      {UBUS_PORT_ID_PSRAM, 10}, {UBUS_PORT_ID_SYSXRDP, 1}, {-1,-1} },
    { {UBUS_PORT_ID_SWH, -1}, {UBUS_PORT_ID_BIU, 8}, {UBUS_PORT_ID_SYS, 1}, {UBUS_PORT_ID_DSL, 8}, {UBUS_PORT_ID_PSRAM, 8}, {-1,-1} },      
};
#elif defined(CONFIG_BCM96858)
static ubus_credit_cfg_t ubus_credit_tbl[UBUS_NUM_OF_MST_PORTS][UBUS_MAX_PORT_NUM+2] = {
    { {UBUS_PORT_ID_BIU, -1}, {-1,-1} },
    { {UBUS_PORT_ID_PER, -1}, {UBUS_PORT_ID_MEMC, 1}, {-1,-1} },
    { {UBUS_PORT_ID_USB, -1}, {UBUS_PORT_ID_MEMC, 4}, {-1,-1} },
    { {UBUS_PORT_ID_PERDMA, -1}, {UBUS_PORT_ID_BIU, 0} },
    { {UBUS_PORT_ID_SPU, -1}, {-1,-1} },
    { {UBUS_PORT_ID_PCIE0, -1}, {UBUS_PORT_ID_RQ0, 1}, {UBUS_PORT_ID_RQ1, 1}, {UBUS_PORT_ID_RQ2, 1}, {UBUS_PORT_ID_RQ3, 1}, {-1,-1} },
    { {UBUS_PORT_ID_PCIE2, -1}, {UBUS_PORT_ID_RQ0, 1}, {UBUS_PORT_ID_RQ1, 1}, {UBUS_PORT_ID_RQ2, 1}, {UBUS_PORT_ID_RQ3, 1}, {-1,-1} },
    { {UBUS_PORT_ID_PMC, -1}, {UBUS_PORT_ID_MEMC, 1}, {UBUS_PORT_ID_RQ0, 1}, {UBUS_PORT_ID_RQ1, 1}, {UBUS_PORT_ID_RQ2, 1}, {UBUS_PORT_ID_RQ3, 1}, {-1,-1} },
    { {UBUS_PORT_ID_QM, -1}, {UBUS_PORT_ID_MEMC, 10}, {-1,-1} },
    { {UBUS_PORT_ID_DQM, -1}, {UBUS_PORT_ID_MEMC, 7}, {-1,-1} },
    { {UBUS_PORT_ID_DMA0, -1}, {UBUS_PORT_ID_MEMC, 9}, {-1,-1} },
    { {UBUS_PORT_ID_DMA1, -1}, {UBUS_PORT_ID_MEMC, 9}, {-1,-1} },
    { {UBUS_PORT_ID_NATC, -1}, {UBUS_PORT_ID_BIU, 10}, {-1,-1} },
    { {UBUS_PORT_ID_TOP_BUFF, -1}, {-1,-1} },
    { {UBUS_PORT_ID_XRDP_BUFF, -1}, {-1,-1} },
    { {UBUS_PORT_ID_RQ0, -1}, {UBUS_PORT_ID_MEMC, 8}, {UBUS_PORT_ID_XRDP_VPB, 1}, {UBUS_PORT_ID_RQ0, 1}, {-1,-1} },
    { {UBUS_PORT_ID_RQ1, -1}, {UBUS_PORT_ID_MEMC, 4}, {UBUS_PORT_ID_XRDP_VPB, 1}, {UBUS_PORT_ID_RQ1, 1}, {-1,-1} },
    { {UBUS_PORT_ID_RQ2, -1}, {UBUS_PORT_ID_MEMC, 4}, {UBUS_PORT_ID_XRDP_VPB, 1}, {UBUS_PORT_ID_RQ2, 1}, {-1,-1} },
    { {UBUS_PORT_ID_RQ3, -1}, {UBUS_PORT_ID_MEMC, 4}, {UBUS_PORT_ID_XRDP_VPB, 1}, {UBUS_PORT_ID_RQ3, 1}, {-1,-1} },
};
#endif

MstPortNode* bcm_get_ubus_mst_addr(int master_port_id)
{
    MstPortNode *master_addr = NULL;
    int i=0;

    while(ub_mst_addr_map_tbl[i].port_id != -1)
    {
        if (ub_mst_addr_map_tbl[i].port_id == master_port_id)
        {
            master_addr = (MstPortNode*)ub_mst_addr_map_tbl[i].base;
            break;
        }
        i++;
    }

    return master_addr;
}

// XXX  Dima To check
int log2_32 (unsigned int value)
{
    unsigned int result = 0;
    if( value < 1)
        return 0;

    while (value > 1) {
        ++result;
        value >>= 1;
    }

    return result;
}
#ifndef _CFE_
EXPORT_SYMBOL(log2_32);

static int ubus_master_decode_wnd_cfg(int master_port_id, int win, unsigned int phys_addr, unsigned int size_power_of_2, int port_id, unsigned int cache_bit_en)
{
    MstPortNode *master_addr = NULL; 
    int ret = 0;

#if defined(CONFIG_BCM96858) || defined(_BCM96858_)
    if ((master_port_id==UBUS_PORT_ID_TOP_BUFF) || (master_port_id==UBUS_PORT_ID_XRDP_BUFF))
        return 0;
#endif

    UBUS_REMAP_DEBUG_LOG("\x1b[35m port[%d] win[%d] phys_addr[0x%x] power_of_2[%d] port_id[%d] cache_bit[%d]\x1b[0m\n", 
                         master_port_id, win, phys_addr, size_power_of_2, port_id,cache_bit_en);


    if((win > 3) || (size_power_of_2 > 31))
    {
        printk("\x1b[35m Paramets Error:  win[%d] phys_addr[0x%x] power_of_2[%d] port_id[%d] cache_bit[%d]\x1b[0m\n", 
               win, phys_addr, size_power_of_2, port_id, cache_bit_en);
        return -1;
    }

    master_addr = bcm_get_ubus_mst_addr(master_port_id);

    if(master_addr)
    {
#if defined(CONFIG_BCM963158) || defined(_BCM963158_) || defined(CONFIG_BCM963178) || defined(_BCM963178_) ||\
    defined(CONFIG_BCM947622) || defined(_BCM947622_)
        /* 63158 has the all the master connected to the CCI as default so no need to
        configure the map. Just turn on the cache configuration */
        if( cache_bit_en )
        {
            master_addr->decode_cfg.cache_cfg = 0x1;
            master_addr->decode_cfg.ctrl &= ~DECODE_CFG_CTRL_CACHE_SEL_MASK;
            master_addr->decode_cfg.ctrl |= DECODE_CFG_CTRL_CACHE_SEL_CFG_REG;  
        }
        else
        {
            master_addr->decode_cfg.cache_cfg = 0x0;
            master_addr->decode_cfg.ctrl &= ~DECODE_CFG_CTRL_CACHE_SEL_MASK;
            master_addr->decode_cfg.ctrl |= DECODE_CFG_CTRL_CACHE_SEL_DEF;  
        }

#else
        if(size_power_of_2)
        {
            master_addr->decode_cfg.window[win].base_addr =  (phys_addr>>8);
            master_addr->decode_cfg.window[win].remap_addr = (phys_addr>>8);

            if( (port_id == UBUS_PORT_ID_BIU) && (cache_bit_en))
            {
                    master_addr->decode_cfg.window[win].attributes = 
                                (DECODE_CFG_CACHE_BITS | DECODE_CFG_ENABLE_ADDR_ONLY | (size_power_of_2 << DECODE_CFG_SIZE_SHIFT) | port_id) ;
                    UBUS_REMAP_DEBUG_LOG("\x1b[35m base_addr[0x%x] remap_addr[0x%x] attributes[0x%x]\x1b[0m\n", 
                                         master_addr->decode_cfg.window[win].base_addr, 
                                         master_addr->decode_cfg.window[win].remap_addr,
                                         master_addr->decode_cfg.window[win].attributes);
             }
            else
            {
                master_addr->decode_cfg.window[win].attributes = 
                                (DECODE_CFG_ENABLE_ADDR_ONLY | (size_power_of_2 << DECODE_CFG_SIZE_SHIFT) | port_id) ;

                UBUS_REMAP_DEBUG_LOG("\x1b[35m base_addr[0x%x] remap_addr[0x%x] attributes[0x%x]\x1b[0m\n",
                                     master_addr->decode_cfg.window[win].base_addr, 
                                     master_addr->decode_cfg.window[win].remap_addr,
                                     master_addr->decode_cfg.window[win].attributes); 
            }
        }
        else
        {
            master_addr->decode_cfg.window[win].base_addr = 0;
            master_addr->decode_cfg.window[win].remap_addr = 0;
            master_addr->decode_cfg.window[win].attributes = 0;
        }
#endif
    }

    return ret;
}
#endif   // #ifndef _CFE_

#ifndef _CFE_
int ubus_decode_pcie_wnd_cfg(unsigned int base, unsigned int size, unsigned int core)
{
    /* Not implemented */
    return -1;
}
EXPORT_SYMBOL(ubus_decode_pcie_wnd_cfg);
#endif


#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622)
int ubus_remap_to_biu_cfg_wlu_srcpid(int srcpid, int enable)
{
    volatile CoherencyPortCfgReg_t* cpcfg_reg = (volatile CoherencyPortCfgReg_t * const)UBUS_COHERENCY_PORT_CFG_BASE;
    int i = 0, reg, bit;

    if( srcpid > MAX_WLU_SRCPID_NUM )
        return -1;

    if( srcpid == MAX_WLU_SRCPID_NUM ) {
        for( i = 0; i < MAX_WLU_SRCPID_REG_NUM; i++ )
            cpcfg_reg->wlu_srcpid[i] = enable ? 0xffffffff : 0;
    } else {
        reg = WLU_SRCPID_TO_REG_OFFSET(srcpid);
        bit = WLU_SRCPID_TO_REG_BIT(srcpid);
        if( enable ) 
            cpcfg_reg->wlu_srcpid[reg] |= (0x1<<bit);
        else 
            cpcfg_reg->wlu_srcpid[reg] &= ~(0x1<<bit);
    }

    return 0;
}
#endif //#if defined(CONFIG_BCM963158) || defined(_BCM963158_)

#define SIZE_OF_REG_BYTES     (4)
#if defined(CONFIG_BCM_UBUS_DECODE_REMAP)
#if !defined(CONFIG_BCM963158) && !defined(CONFIG_BCM963178) && !defined(CONFIG_BCM947622)
static int ubus_remap_to_biu_cfg_queue_srcpid(unsigned long lut_idx, unsigned int *p_srcpid_queus_value)
{
    CoherencyPortCfgReg_t *reg_addr = 
        (CoherencyPortCfgReg_t*)(UBUS_COHERENCY_PORT_CFG_LUT_BASE + lut_idx * SIZE_OF_REG_BYTES); 

    if((lut_idx > 31) || (NULL == p_srcpid_queus_value))
        return -1;

    reg_addr->queue_cfg = (((p_srcpid_queus_value[0] & 0xf) << SRCPID_TO_QUEUE_0_BITS_SHIFT) |
                           ((p_srcpid_queus_value[1] & 0xf) << SRCPID_TO_QUEUE_1_BITS_SHIFT) |
                           ((p_srcpid_queus_value[2] & 0xf) << SRCPID_TO_QUEUE_2_BITS_SHIFT) |
                           ((p_srcpid_queus_value[3] & 0xf) << SRCPID_TO_QUEUE_3_BITS_SHIFT) |
                           ((p_srcpid_queus_value[4] & 0xf) << SRCPID_TO_QUEUE_4_BITS_SHIFT) |
                           ((p_srcpid_queus_value[5] & 0xf) << SRCPID_TO_QUEUE_5_BITS_SHIFT) |
                           ((p_srcpid_queus_value[6] & 0xf) << SRCPID_TO_QUEUE_6_BITS_SHIFT) |
                           ((p_srcpid_queus_value[7] & 0xf) << SRCPID_TO_QUEUE_7_BITS_SHIFT));

    UBUS_REMAP_DEBUG_LOG("\x1b[35m reg_addr[0x%p] reg_value[0x%x]\x1b[0m\n",
                         (unsigned int*)(reg_addr), reg_addr->queue_cfg); 
    
    return 0;                        
}

static int ubus_remap_to_biu_cfg_queue_depth(unsigned long q_depth_idx, unsigned int *p_depth_queus_value)
{
    CoherencyPortCfgReg_t *reg_addr = 
        (CoherencyPortCfgReg_t*)(UBUS_COHERENCY_PORT_CFG_DEPTH_BASE + q_depth_idx * SIZE_OF_REG_BYTES); 

    if((q_depth_idx > 3) || (NULL == p_depth_queus_value))
        return -1;

    reg_addr->queue_cfg = (((p_depth_queus_value[0] & 0xff) << DEPTH_TO_QUEUE_0_BITS_SHIFT) |
                           ((p_depth_queus_value[1] & 0xff) << DEPTH_TO_QUEUE_1_BITS_SHIFT) |
                           ((p_depth_queus_value[2] & 0xff) << DEPTH_TO_QUEUE_2_BITS_SHIFT) |
                           ((p_depth_queus_value[3] & 0xff) << DEPTH_TO_QUEUE_3_BITS_SHIFT));

    UBUS_REMAP_DEBUG_LOG("\x1b[35m reg_addr[0x%p] reg_value[0x%x]\x1b[0m\n",
                         (unsigned int*)(reg_addr), reg_addr->queue_cfg); 

    return 0;                        
}

static int ubus_remap_to_biu_cfg_queue_thresh(unsigned long q_thresh_idx, unsigned int *p_thresh_queus_value)
{
    CoherencyPortCfgReg_t *reg_addr = 
        (CoherencyPortCfgReg_t*)(UBUS_COHERENCY_PORT_CFG_CBS_BASE + q_thresh_idx * SIZE_OF_REG_BYTES); 

    if((q_thresh_idx > 8) || (NULL == p_thresh_queus_value))
        return -1;

    reg_addr->queue_cfg = (((p_thresh_queus_value[0] & 0xffff) << THRESH_TO_QUEUE_0_BITS_SHIFT) |
                          ((p_thresh_queus_value [1] & 0xffff) << THRESH_TO_QUEUE_1_BITS_SHIFT));
                    
    UBUS_REMAP_DEBUG_LOG("\x1b[35m reg_addr[0x%p] reg_value[0x%x]\x1b[0m\n",
                         (unsigned int*)(reg_addr), reg_addr->queue_cfg); 

    return 0;                        
}

static int ubus_remap_to_biu_cfg_cir_incr(unsigned long q_cirinc_idx, unsigned int *p_cirinc_queus_value)
{
    CoherencyPortCfgReg_t *reg_addr =
         (CoherencyPortCfgReg_t*)(UBUS_COHERENCY_PORT_CFG_CIR_INCR_BASE + q_cirinc_idx * SIZE_OF_REG_BYTES); 

    if((q_cirinc_idx > 3) || (NULL == p_cirinc_queus_value))
        return -1;

    reg_addr->queue_cfg = (((p_cirinc_queus_value[0] & 0xff) << CIR_INCR_TO_QUEUE_0_BITS_SHIFT) |
                          ((p_cirinc_queus_value [1] & 0xff) << CIR_INCR_TO_QUEUE_1_BITS_SHIFT) |
                          ((p_cirinc_queus_value [2] & 0xff) << CIR_INCR_TO_QUEUE_2_BITS_SHIFT) |
                          ((p_cirinc_queus_value [3] & 0xff) << CIR_INCR_TO_QUEUE_3_BITS_SHIFT));

    UBUS_REMAP_DEBUG_LOG("\x1b[35m reg_addr[0x%p] reg_value[0x%x]\x1b[0m\n",
                         (unsigned int*)(reg_addr), reg_addr->queue_cfg); 

    return 0;                        
}

static int ubus_remap_to_biu_cfg_ref_cnt(unsigned long q_ref_cnt_idx, unsigned int *p_ref_cnt_value)
{
    CoherencyPortCfgReg_t *reg_addr = 
       (CoherencyPortCfgReg_t*)(UBUS_COHERENCY_PORT_CFG_REF_COUNT_BASE + q_ref_cnt_idx * SIZE_OF_REG_BYTES); 

    if((q_ref_cnt_idx > 1) || (NULL == p_ref_cnt_value))
        return -1;

    reg_addr->queue_cfg = (((p_ref_cnt_value[0] & 0xf) << REF_CNT_0_BITS_SHIFT) |
                           ((p_ref_cnt_value[1] & 0xf) << REF_CNT_1_BITS_SHIFT) |
                           ((p_ref_cnt_value[2] & 0xf) << REF_CNT_2_BITS_SHIFT) |
                           ((p_ref_cnt_value[3] & 0xf) << REF_CNT_3_BITS_SHIFT) |
                           ((p_ref_cnt_value[4] & 0xf) << REF_CNT_4_BITS_SHIFT) |
                           ((p_ref_cnt_value[5] & 0xf) << REF_CNT_5_BITS_SHIFT) |
                           ((p_ref_cnt_value[6] & 0xf) << REF_CNT_6_BITS_SHIFT) |
                           ((p_ref_cnt_value[7] & 0xf) << REF_CNT_7_BITS_SHIFT));
                    
    UBUS_REMAP_DEBUG_LOG("\x1b[35m  reg_addr[0x%p] reg_value[0x%x]\x1b[0m\n",
                         (unsigned int*)(reg_addr), reg_addr->queue_cfg); 
    
    return 0;                        
}

   
#define  SRC_PID_Q_NUM      (8)
#define  DEPTH_Q_NUM        (4)
#define  THRESH_Q_NUM       (2)
#define  CIR_INCR_Q_NUM     (4)
#define  REF_CNT_NUM        (8)
static int configure_biu_pid_to_queue(void)
{
    int rc = 0;
    unsigned int srcpid_queus_value[SRC_PID_Q_NUM] = {0};
    unsigned int depth_queus_value[DEPTH_Q_NUM] = {0};
    unsigned int thresh_queus_value[THRESH_Q_NUM] = {0};
    unsigned int cir_incr_queus_value[CIR_INCR_Q_NUM] = {0};
    unsigned int ref_cnt_value[REF_CNT_NUM] = {0};
    unsigned long lut_idx;
    unsigned long depth_idx;
    unsigned long thresh_idx;
    unsigned long cir_incr_idx;
    unsigned long ref_cnt_idx;

    lut_idx = 0;
    srcpid_queus_value[0] = 0;
    srcpid_queus_value[1] = 0;
    srcpid_queus_value[2] = 0;
    srcpid_queus_value[3] = 0;
    srcpid_queus_value[4] = 1;
    srcpid_queus_value[5] = 1;
    srcpid_queus_value[6] = 1;
    srcpid_queus_value[7] = 1;
    rc = ubus_remap_to_biu_cfg_queue_srcpid(lut_idx, srcpid_queus_value);
    if (rc < 0)
    {
        printk("Error %s line[%d]\n",__FILE__, __LINE__);
        goto exit_;
    }

    lut_idx = 1;
    srcpid_queus_value[0] = 0;
    srcpid_queus_value[1] = 0;
    srcpid_queus_value[2] = 0;
    srcpid_queus_value[3] = 2;
    srcpid_queus_value[4] = 3;
    srcpid_queus_value[5] = 4;
    srcpid_queus_value[6] = 3;
    srcpid_queus_value[7] = 5;
    rc = ubus_remap_to_biu_cfg_queue_srcpid(lut_idx, srcpid_queus_value);
    if (rc < 0)
    {
        printk("Error %s line[%d]\n",__FILE__, __LINE__);
        goto exit_;
    }

    depth_idx = 0;
    depth_queus_value[0] = 0x10;
    depth_queus_value[1] = 0x10;
    depth_queus_value[2] = 8;
    depth_queus_value[3] = 8;
    rc = ubus_remap_to_biu_cfg_queue_depth(depth_idx, depth_queus_value);
    if (rc < 0)
    {
        printk("Error %s line[%d]\n",__FILE__, __LINE__);
        goto exit_;
    }

    depth_idx = 1;
    depth_queus_value[0] = 8;
    depth_queus_value[1] = 8;
    depth_queus_value[2] = 0;
    depth_queus_value[3] = 0;
    rc = ubus_remap_to_biu_cfg_queue_depth(depth_idx, depth_queus_value);
    if (rc < 0)
    {
        printk("Error %s line[%d]\n",__FILE__, __LINE__);
        goto exit_;
    }

    thresh_idx = 0;
    thresh_queus_value[0] = 0x100;
    thresh_queus_value[1] = 0x100;
    rc = ubus_remap_to_biu_cfg_queue_thresh(thresh_idx, thresh_queus_value);
    if (rc < 0)
    {
        printk("Error %s line[%d]\n",__FILE__, __LINE__);
        goto exit_;
    }

    thresh_idx = 1;
    thresh_queus_value[0] = 0x1000;
    thresh_queus_value[1] = 0x400;
    rc = ubus_remap_to_biu_cfg_queue_thresh(thresh_idx, thresh_queus_value);
    if (rc < 0)
    {
        printk("Error %s line[%d]\n",__FILE__, __LINE__);
        goto exit_;
    }

    thresh_idx = 2;
    thresh_queus_value[0] = 0x400;
    thresh_queus_value[1] = 0x1000;
    rc = ubus_remap_to_biu_cfg_queue_thresh(thresh_idx, thresh_queus_value);
    if (rc < 0)
    {
        printk("Error %s line[%d]\n",__FILE__, __LINE__);
        goto exit_;
    }

    cir_incr_idx = 0;
    cir_incr_queus_value[0] = 1;
    cir_incr_queus_value[1] = 1;
    cir_incr_queus_value[2] = 4;
    cir_incr_queus_value[3] = 4;
    rc = ubus_remap_to_biu_cfg_cir_incr(cir_incr_idx, cir_incr_queus_value);
    if (rc < 0)
    {
        printk("Error %s line[%d]\n",__FILE__, __LINE__);
        goto exit_;
    }

    cir_incr_idx = 1;
    cir_incr_queus_value[0] = 2;
    cir_incr_queus_value[1] = 3;
    cir_incr_queus_value[2] = 0;
    cir_incr_queus_value[3] = 0;
    rc = ubus_remap_to_biu_cfg_cir_incr(cir_incr_idx, cir_incr_queus_value);
    if (rc < 0)
    {
        printk("Error %s line[%d]\n",__FILE__, __LINE__);
        goto exit_;
    }

    ref_cnt_idx = 0;
    ref_cnt_value[0] = 7;
    ref_cnt_value[1] = 4;
    ref_cnt_value[2] = 2;
    ref_cnt_value[3] = 2;
    ref_cnt_value[4] = 3;
    ref_cnt_value[5] = 2;
    ref_cnt_value[6] = 0;
    ref_cnt_value[7] = 0;
    rc = ubus_remap_to_biu_cfg_ref_cnt(ref_cnt_idx, ref_cnt_value);
    if (rc < 0)
    {
        printk("Error %s line[%d]\n",__FILE__, __LINE__);
        goto exit_;
    }

exit_:
    if (rc < 0)
        printk("Error: line[%d]\n",__LINE__);

    return rc;                       
}
#elif defined(CONFIG_BCM963158)

/* Helper MACROS */
#define SIZE_OF_REG_BITS (SIZE_OF_REG_BYTES*8)

#define CLEAR_BITS(r, p, m) (r) = ((r) & ~((m) << (p)))
#define SET_BITS(r, p, v) (r) = ((r) | ((v) << (p)))

#define BITS_MASK(bits)       ( (1<<(bits)) - 1 )
#define BITS_POS(fld_num, bits) ( ((fld_num)*(bits)) % SIZE_OF_REG_BITS ) 

#define FIELD_INDEX(fld_num, bits)      ( (fld_num) / (SIZE_OF_REG_BITS/(bits)) )

#define SRCPID_2_LUT_IDX(src_pid)       ( FIELD_INDEX(src_pid, QUE_ID_NUM_BITS) )
#define QUEUE_2_DEPTH_IDX(que_id)       ( FIELD_INDEX(que_id, DEPTH_NUM_BITS) )
#define QUEUE_2_CBS_IDX(que_id)         ( FIELD_INDEX(que_id, CBS_NUM_BITS) )
#define QUEUE_2_CIR_INCR_IDX(que_id)    ( FIELD_INDEX(que_id, CIR_INCR_NUM_BITS) )
#define QUEUE_2_REF_CNT_IDX(que_id)     ( FIELD_INDEX(que_id, REF_CNT_NUM_BITS) )
#define QUEUE_2_MAX_BONUS_IDX(que_id)   ( FIELD_INDEX(que_id, MAX_BONUS_NUM_BITS) )

/* DDR Rate/Width mapping to array index */
typedef enum {
    eDDR_RATE_WIDTH_2133_32,
    eDDR_RATE_WIDTH_2133_16,
    eDDR_RATE_WIDTH_1600_32,
    eDDR_RATE_WIDTH_1600_16,
    eDDR_RATE_WIDTH_MAX,
}eDDR_RATE_WIDTH;

/* Local structure to store configured values */
typedef struct {
    char blk_name[8];
    uint32_t mstr_node;
    uint32_t src_pid;
    uint32_t que_id;
    uint32_t depth;
    uint32_t ref_cnt[eDDR_RATE_WIDTH_MAX];
    uint32_t cir;
    uint32_t cbs;
    uint32_t bonus;
}biu_cfg_t;

eDDR_RATE_WIDTH g_ddr_rate_width_idx = eDDR_RATE_WIDTH_2133_32; /* Gets initialized to the correct index based on rate/width */

#define BIU_CFG_MAX_SRC_PID     (UBUS_MAX_PORT_NUM+1)

static biu_cfg_t biu_cfg_port_array[BIU_CFG_MAX_SRC_PID] = {
    /* queue depth will be set based on ubus credits */
    [UBUS_PORT_ID_PER]      = { .blk_name = "PCM",    .src_pid = 3,  .que_id = 7,  .cir = 2,  .cbs = 128, .bonus = 0, .ref_cnt = {12, 18, 22, 40 }, .mstr_node = UBUS_PORT_ID_PER},
    [UBUS_PORT_ID_USB]      = { .blk_name = "USB",    .src_pid = 4,  .que_id = 6,  .cir = 3,  .cbs = 128, .bonus = 0, .ref_cnt = {38, 71, 90, 161}, .mstr_node = UBUS_PORT_ID_USB},
    [UBUS_PORT_ID_SPU]      = { .blk_name = "SPU",    .src_pid = 5,  .que_id = 9,  .cir = 1,  .cbs = 1,   .bonus = 0, .ref_cnt = {38, 71, 90, 161}, .mstr_node = UBUS_PORT_ID_SPU},
    [UBUS_PORT_ID_PERDMA]   = { .blk_name = "M2M",    .src_pid = 7,  .que_id = 8,  .cir = 1,  .cbs = 1,   .bonus = 0, .ref_cnt = {38, 71, 90, 161}, .mstr_node = UBUS_PORT_ID_PERDMA},
    [UBUS_PORT_ID_PCIE0]    = { .blk_name = "PCIe0",  .src_pid = 8,  .que_id = 3,  .cir = 6,  .cbs = 128, .bonus = 0, .ref_cnt = {38, 71, 90, 161}, .mstr_node = UBUS_PORT_ID_PCIE0},
    [UBUS_PORT_ID_PCIE2]    = { .blk_name = "PCIe2",  .src_pid = 9,  .que_id = 4,  .cir = 3,  .cbs = 128, .bonus = 0, .ref_cnt = {38, 71, 90, 161}, .mstr_node = UBUS_PORT_ID_PCIE2},
    [UBUS_PORT_ID_PCIE3]    = { .blk_name = "PCIe3",  .src_pid = 10, .que_id = 5,  .cir = 9,  .cbs = 256, .bonus = 0, .ref_cnt = {38, 71, 90, 161}, .mstr_node = UBUS_PORT_ID_PCIE3},
    [UBUS_PORT_ID_QM]       = { .blk_name = "QM",     .src_pid = 22, .que_id = 10, .cir = 38, .cbs = 512, .bonus = 4, .ref_cnt = {38, 71, 90, 161}, .mstr_node = UBUS_PORT_ID_QM},
    [UBUS_PORT_ID_DQM]      = { .blk_name = "QM_DQM", .src_pid = 23, .que_id = 1,  .cir = 5,  .cbs = 256, .bonus = 4, .ref_cnt = {6,  9,  11, 20 }, .mstr_node = UBUS_PORT_ID_DQM},
    [UBUS_PORT_ID_DMA0]     = { .blk_name = "DMA0",   .src_pid = 24, .que_id = 11, .cir = 39, .cbs = 128, .bonus = 4, .ref_cnt = {38, 71, 90, 161}, .mstr_node = UBUS_PORT_ID_DMA0},
    [UBUS_PORT_ID_NATC]     = { .blk_name = "NAT$",   .src_pid = 26, .que_id = 2,  .cir = 6,  .cbs = 512, .bonus = 4, .ref_cnt = {21, 36, 45, 81 }, .mstr_node = UBUS_PORT_ID_NATC},
    [UBUS_PORT_ID_RQ0]      = { .blk_name = "RNR",    .src_pid = 32, .que_id = 0,  .cir = 14, .cbs = 256, .bonus = 4, .ref_cnt = {12, 18, 22, 40 }, .mstr_node = UBUS_PORT_ID_RQ0},
};
/* Function to dump configuration per source port */
void dump_biu_cfg_array(biu_cfg_t *p)
{
    uint32_t idx;
    uint32_t depth_total = 0;
    printk("\n");
    printk("PID     BLOCK     QUE  DEPTH  REFRESH  CIR_INCR  CBS  BONUS MSTR_NODE\n");
    for (idx=0; idx < BIU_CFG_MAX_SRC_PID; idx++) {
        if (p->src_pid) {
            printk("[%2u] %8s : %4u %5u %7u %8u %8u %4u %6u\n",p->src_pid, p->blk_name, p->que_id, p->depth, p->ref_cnt[0], p->cir, p->cbs, p->bonus, p->mstr_node);
            depth_total += p->depth;
        }
        p++;
    }
    printk("TOTAL Depth = Credits = %u\n",depth_total);
    printk("\n");
}
/* Helper function to extract the value based on field index and bits */
static uint32_t extract_field_value(uint32_t val, uint32_t fld_num, uint32_t bits)
{
    uint32_t mask, pos;
    mask = BITS_MASK(bits);
    pos = BITS_POS(fld_num, bits);
    val = (val & (mask << pos));
    val = val >> pos;
    return val;
}
/* Helper functions to read configured queue id value for a given source port */
static uint32_t biu_read_queue(uint32_t src_pid)
{
    volatile CoherencyPortCfgReg_t* reg_addr = (volatile CoherencyPortCfgReg_t * const)UBUS_COHERENCY_PORT_CFG_BASE;
    uint32_t idx;

    idx = SRCPID_2_LUT_IDX(src_pid);
    return extract_field_value(reg_addr->lut[idx], src_pid, QUE_ID_NUM_BITS);
}
/* Helper functions to read configured queue depth value for a given queue id */
static uint32_t biu_read_queue_depth(uint32_t queue_id)
{
    volatile CoherencyPortCfgReg_t* reg_addr = (volatile CoherencyPortCfgReg_t * const)UBUS_COHERENCY_PORT_CFG_BASE;
    uint32_t idx;

    idx = QUEUE_2_DEPTH_IDX(queue_id);
    return extract_field_value(reg_addr->queue_depth[idx], queue_id, DEPTH_NUM_BITS);
}
/* Helper functions to read configured queue cbs value for a given queue id */
static uint32_t biu_read_queue_cbs(uint32_t queue_id)
{
    volatile CoherencyPortCfgReg_t* reg_addr = (volatile CoherencyPortCfgReg_t * const)UBUS_COHERENCY_PORT_CFG_BASE;
    uint32_t idx;

    idx = QUEUE_2_CBS_IDX(queue_id);
    return extract_field_value(reg_addr->cbs_thresh[idx], queue_id, CBS_NUM_BITS);
}
/* Helper functions to read configured queue cir_incr value for a given queue id */
static uint32_t biu_read_queue_cir_incr(uint32_t queue_id)
{
    volatile CoherencyPortCfgReg_t* reg_addr = (volatile CoherencyPortCfgReg_t * const)UBUS_COHERENCY_PORT_CFG_BASE;
    uint32_t idx;

    idx = QUEUE_2_CIR_INCR_IDX(queue_id);
    return extract_field_value(reg_addr->cir_incr[idx], queue_id, CIR_INCR_NUM_BITS);
}
/* Helper functions to read configured queue refresh count value for a given queue id */
static uint32_t biu_read_queue_ref_cnt(uint32_t queue_id)
{
    volatile CoherencyPortCfgReg_t* reg_addr = (volatile CoherencyPortCfgReg_t * const)UBUS_COHERENCY_PORT_CFG_BASE;
    uint32_t idx;

    idx = QUEUE_2_REF_CNT_IDX(queue_id);
    return extract_field_value(reg_addr->ref_cnt[idx], queue_id, REF_CNT_NUM_BITS);
}
/* Helper functions to read configured queue max bonus value for a given queue id */
static uint32_t biu_read_queue_max_bonus(uint32_t queue_id)
{
    volatile CoherencyPortCfgReg_t* reg_addr = (volatile CoherencyPortCfgReg_t * const)UBUS_COHERENCY_PORT_CFG_BASE;
    uint32_t idx;

    idx = QUEUE_2_MAX_BONUS_IDX(queue_id);
    return extract_field_value(reg_addr->max_bonus[idx], queue_id, MAX_BONUS_NUM_BITS);
}
/* Helper functions to read & dump configuration for all source ports of interest */
void dump_biu_cfg(void)
{
    biu_cfg_t cfg_array[BIU_CFG_MAX_SRC_PID] = {}; 
    biu_cfg_t *p;
    uint32_t src_pid;
    uint32_t que_id;

    for (src_pid = 0; src_pid < BIU_CFG_MAX_SRC_PID; src_pid++) {
        if (biu_cfg_port_array[src_pid].src_pid) {
            p = &cfg_array[src_pid];
            /* Assign static values */
            memcpy(p->blk_name, biu_cfg_port_array[src_pid].blk_name, sizeof(p->blk_name));
            p->mstr_node = biu_cfg_port_array[src_pid].mstr_node;
            p->src_pid = src_pid;
            /* Assign read values */
            que_id = p->que_id = biu_read_queue(src_pid);
            p->depth = biu_read_queue_depth(que_id);
            p->cbs = biu_read_queue_cbs(que_id);
            p->cir = biu_read_queue_cir_incr(que_id);
            p->ref_cnt[g_ddr_rate_width_idx] = biu_read_queue_ref_cnt(que_id);
            p->bonus = biu_read_queue_max_bonus(que_id);
        }
    }
    dump_biu_cfg_array(cfg_array);

}
/* Functions to reset all the coherency port configuration */
static void reset_biu_cfg(void)
{
    volatile CoherencyPortCfgReg_t* reg_addr = (volatile CoherencyPortCfgReg_t * const)UBUS_COHERENCY_PORT_CFG_BASE; 
    int idx;
    for (idx = 0; idx < ARRAY_SIZE(reg_addr->lut); idx++) {
        reg_addr->lut[idx] = 0;
    }
    for (idx = 0; idx < ARRAY_SIZE(reg_addr->queue_depth); idx++) {
        reg_addr->queue_depth[idx] = 0;
    }
    for (idx = 0; idx < ARRAY_SIZE(reg_addr->cbs_thresh); idx++) {
        reg_addr->cbs_thresh[idx] = 0;
    }
    for (idx = 0; idx < ARRAY_SIZE(reg_addr->cir_incr); idx++) {
        reg_addr->cir_incr[idx] = 0;
    }
    for (idx = 0; idx < ARRAY_SIZE(reg_addr->ref_cnt); idx++) {
        reg_addr->ref_cnt[idx] = 0;
    }
    for (idx = 0; idx < ARRAY_SIZE(reg_addr->max_bonus); idx++) {
        reg_addr->max_bonus[idx] = 0;
    }
}

/* Functions to set coherency port configuration for a given source port */
void ubus_remap_to_biu_cfg(const biu_cfg_t *biu_cfg_p)
{
    volatile CoherencyPortCfgReg_t* reg_addr = (volatile CoherencyPortCfgReg_t * const)UBUS_COHERENCY_PORT_CFG_BASE; 
    uint8_t idx;
    uint32_t mask;
    uint32_t *reg_p;
    uint8_t pos;

    /* COHERENCY_PORT_CFG_REG_LUT*/
    idx = SRCPID_2_LUT_IDX(biu_cfg_p->src_pid);
    reg_p = &reg_addr->lut[idx];
    mask = BITS_MASK(QUE_ID_NUM_BITS);
    pos =  BITS_POS(biu_cfg_p->src_pid, QUE_ID_NUM_BITS);
    CLEAR_BITS(*reg_p, pos, mask);
    SET_BITS(*reg_p, pos, biu_cfg_p->que_id);

    /* COHERENCY_PORT_CFG_REG_QUEUE_DEPTH */
    idx = QUEUE_2_DEPTH_IDX(biu_cfg_p->que_id);
    reg_p = &reg_addr->queue_depth[idx];
    mask = BITS_MASK(DEPTH_NUM_BITS);
    pos =  BITS_POS(biu_cfg_p->que_id, DEPTH_NUM_BITS);
    CLEAR_BITS(*reg_p, pos, mask);
    SET_BITS(*reg_p, pos, biu_cfg_p->depth);
 
    /* COHERENCY_PORT_CFG_REG_CBS_THRESH */
    idx = QUEUE_2_CBS_IDX(biu_cfg_p->que_id);
    reg_p = &reg_addr->cbs_thresh[idx];
    mask = BITS_MASK(CBS_NUM_BITS);
    pos =  BITS_POS(biu_cfg_p->que_id, CBS_NUM_BITS);
    CLEAR_BITS(*reg_p, pos, mask);
    SET_BITS(*reg_p, pos, biu_cfg_p->cbs);

    /* COHERENCY_PORT_CFG_REG_CIR_INCR */
    idx = QUEUE_2_CIR_INCR_IDX(biu_cfg_p->que_id);
    reg_p = &reg_addr->cir_incr[idx];
    mask = BITS_MASK(CIR_INCR_NUM_BITS);
    pos =  BITS_POS(biu_cfg_p->que_id, CIR_INCR_NUM_BITS);
    CLEAR_BITS(*reg_p, pos, mask);
    SET_BITS(*reg_p, pos, biu_cfg_p->cir);

    /* COHERENCY_PORT_CFG_REG_REF_CNT */
    idx = QUEUE_2_REF_CNT_IDX(biu_cfg_p->que_id);
    reg_p = &reg_addr->ref_cnt[idx];
    mask = BITS_MASK(REF_CNT_NUM_BITS);
    pos =  BITS_POS(biu_cfg_p->que_id,REF_CNT_NUM_BITS);
    CLEAR_BITS(*reg_p, pos, mask);
    SET_BITS(*reg_p, pos, biu_cfg_p->ref_cnt[g_ddr_rate_width_idx]); /* Value based on DDR rate/width on current board */

    /* COHERENCY_PORT_CFG_REG_MAX_BONUS */
    idx = QUEUE_2_MAX_BONUS_IDX(biu_cfg_p->que_id);
    reg_p = &reg_addr->max_bonus[idx];
    mask = BITS_MASK(MAX_BONUS_NUM_BITS);;
    pos =  BITS_POS(biu_cfg_p->que_id,MAX_BONUS_NUM_BITS);
    CLEAR_BITS(*reg_p, pos, mask);
    SET_BITS(*reg_p, pos, biu_cfg_p->bonus);
}

/* Helper functions to read DDR configuration in board parameters and return mapped index in array */
static int get_ddr_rate_width_idx(void)
{
    unsigned int memcfg;
    unsigned int rate_1600;
    unsigned int width_16;
    if (BP_SUCCESS == BpGetMemoryConfig( &memcfg) ) {

        rate_1600 = BP_DDR_SPEED_IS_800( (memcfg & BP_DDR_SPEED_MASK<<BP_DDR_SPEED_SHIFT) >> BP_DDR_SPEED_SHIFT ) ? 1 : 0;
        width_16 = ( (memcfg & BP_DDR_TOTAL_WIDTH_MASK) == BP_DDR_TOTAL_WIDTH_32BIT ) ? 0 : 1;

        switch (rate_1600) {
        case 0: /* 2133 */
            switch(width_16) {
            case 0: return eDDR_RATE_WIDTH_2133_32; /* 32bit */
            case 1: return eDDR_RATE_WIDTH_2133_16;/* 16bit */
            }
        case 1: /* 1600 */
            switch(width_16) {
            case 0: return eDDR_RATE_WIDTH_1600_32;/* 32bit */
            case 1: return eDDR_RATE_WIDTH_1600_16;/* 16bit */
            }
        }
    }
    pr_err("\n%s : Failed to find the DDR rate and width \n", __FUNCTION__);
    return eDDR_RATE_WIDTH_2133_32;
}
/* Helper functions to read UBUS credits for each source port as queue depth */
static void set_queue_depth_from_ubus_credits(void)
{
    uint32_t i, idx, jdx;
    uint32_t mstr_node_idx;
    uint32_t depth_sum = 0;
    for (idx=0; idx < BIU_CFG_MAX_SRC_PID; idx++) {
        if (biu_cfg_port_array[idx].src_pid) { /* Valid Entry */

            for ( i = 0; i < UBUS_NUM_OF_MST_PORTS; i++ ) {
                if (ubus_credit_tbl[i][0].port_id == biu_cfg_port_array[idx].mstr_node)
                {
                    mstr_node_idx = i;
                    break;
                }
            }

            for (jdx=0; ubus_credit_tbl[mstr_node_idx][jdx].port_id != -1 ; jdx++) {
                if (ubus_credit_tbl[mstr_node_idx][jdx].port_id == UBUS_PORT_ID_BIU) {
                    biu_cfg_port_array[idx].depth = ubus_credit_tbl[mstr_node_idx][jdx].credit;
                }
            }
        }
    }
    /* sum of all queue depth must be <= 64 */
    for (idx=0; idx < BIU_CFG_MAX_SRC_PID; idx++) {
        if (biu_cfg_port_array[idx].src_pid) { /* Valid Entry */
            depth_sum += biu_cfg_port_array[idx].depth;
            if (depth_sum > 64 || !biu_cfg_port_array[idx].depth) {
                printk("\n%s() : ERROR - queue depth config error %d %d \n",
                       __FUNCTION__, depth_sum,biu_cfg_port_array[idx].depth);
                BUG();
            }
        }
    }
}
/* Main functions to do coherency port configuration */
static void configure_biu_pid_to_queue(void)
{
    uint32_t idx;
    /* Reset everything to ZERO */
    reset_biu_cfg();
    /* Read DDR rate/width from board and map to array index */
    g_ddr_rate_width_idx = get_ddr_rate_width_idx();
    /* queue depths must be same as UBUS credits */
    set_queue_depth_from_ubus_credits();

    for (idx=0; idx < BIU_CFG_MAX_SRC_PID; idx++) {
        if (biu_cfg_port_array[idx].src_pid) { /* Valid Entry */
            ubus_remap_to_biu_cfg(&biu_cfg_port_array[idx]);
        }
    }

    dump_biu_cfg();
}
#endif /* !defined(CONFIG_BCM963158) */
#endif //#if defined(CONFIG_BCM_UBUS_DECODE_REMAP) 

#if defined(CONFIG_BCM963178)
void configure_ubus_sar_reg_decode(void)
{
    /* work around for HW63178-272 */
    int win;
    MstPortNode *master_addr = bcm_get_ubus_mst_addr(UBUS_PORT_ID_BIU);

    for (win = 0; win < 4; win++)
    {
        uint32_t *attr = &master_addr->decode_cfg.window[win].attributes;

        if (!GET_FIELD(*attr, DECODE_CFG_ENABLE))
        {
            /* this window is free, use it */
            /* add 0x4000 address range, starting at offset 0x4000 of the SAR register space, to the CPU UBUS master port */
            master_addr->decode_cfg.window[win].base_addr =  ((SAR_PHYS_BASE + 0x4000)>>8);
            master_addr->decode_cfg.window[win].remap_addr = ((SAR_PHYS_BASE + 0x4000)>>8);
            master_addr->decode_cfg.window[win].attributes = 
                                (DECODE_CFG_ENABLE_ADDR_ONLY | (1 << DECODE_CFG_CMD_DTA_SHIFT) |  (14 << DECODE_CFG_SIZE_SHIFT) | UBUS_PORT_ID_DSL);
            break;
        }
    }
}
#endif

#ifndef _CFE_
static char *size_by_exponent_to_str(int exponent)
{
    static char buf[8] = {0};

    if (exponent < 20)
        snprintf(buf, sizeof(buf), "%dK", 1 << (exponent - 10));
    else
        snprintf(buf, sizeof(buf), "%dM", 1 << (exponent - 20));

    return buf;
}

static ssize_t ubus_tokens_get_proc(struct file *file, char *buff, size_t len, loff_t *offset)
{
    int i = 1;

    if (*offset)
        return 0;

    *offset += sprintf(buff, "%9s | %s", " ", enum_val_to_str(ub_mst_addr_map_tbl[0].port_id, ubus_port_id_to_str));
    while (ub_mst_addr_map_tbl[i].port_id != -1)
        *offset += sprintf(buff + *offset, " | %s", enum_val_to_str(ub_mst_addr_map_tbl[i++].port_id, ubus_port_id_to_str));

    *offset += sprintf(buff + *offset, "\n---------------------------------------------------------------------------------------------------------------------");

    i = 0;
    while (ub_mst_addr_map_tbl[i].port_id != -1)
    {
        volatile MstPortNode *master_reg = bcm_get_ubus_mst_addr(ub_mst_addr_map_tbl[i].port_id);
        int j = 0;

        *offset += sprintf(buff + *offset, "\n%9s", enum_val_to_str(ub_mst_addr_map_tbl[i].port_id, ubus_port_id_to_str));
        while (ub_mst_addr_map_tbl[j].port_id != -1)
        {
            uint32_t port_id = ub_mst_addr_map_tbl[j].port_id;
            uint32_t field_len = strlen(enum_val_to_str(ub_mst_addr_map_tbl[j].port_id, ubus_port_id_to_str));

            if (master_reg->token[port_id] > 256)
                *offset += sprintf(buff + *offset, " | %*s", field_len, "-");
            else
                *offset += sprintf(buff + *offset, " | %*d", field_len, master_reg->token[port_id]);
            j++;
        }
        i++;
    }
    *offset += sprintf(buff + *offset, "\n");

    return *offset;
}

static ssize_t ubus_decode_get_proc(struct file *file, char *buff, size_t len, loff_t *offset)
{
    int i = 0;

    if (*offset)
        return 0;

    *offset += sprintf(buff + *offset, "Master Port (addr)   | Base     | Remap    | Size  | PortId | Cache:Enable:CD:Strict\n");
    *offset += sprintf(buff + *offset, "------------------------------------------------------------------------------------\n");

    while (ub_mst_addr_map_tbl[i].port_id != -1)
    {
        MstPortNode *master_reg = bcm_get_ubus_mst_addr(ub_mst_addr_map_tbl[i].port_id);
        int win;

        for (win = 0; win < 4; win++)
        {
            char dest_port[16];
            uint32_t *attr = &master_reg->decode_cfg.window[win].attributes;

            if (!GET_FIELD(*attr, DECODE_CFG_ENABLE))
                continue;

            strncpy(dest_port, 
                    enum_val_to_str(GET_FIELD(*attr, DECODE_CFG_PORT_ID), ubus_port_id_to_str)
                    ? : "-", sizeof(dest_port));

            *offset += sprintf(buff + *offset, "%-9s (%p) | %08x | %08x | %5s |  %3s   | %x:%x:%x:%x\n", 
                    enum_val_to_str(ub_mst_addr_map_tbl[i].port_id, ubus_port_id_to_str),
                    master_reg, 
                    master_reg->decode_cfg.window[win].base_addr, 
                    master_reg->decode_cfg.window[win].remap_addr,
                    size_by_exponent_to_str(GET_FIELD(*attr, DECODE_CFG_SIZE)),
                    dest_port,
                    GET_FIELD(*attr, DECODE_CFG_CACHE_BITS),
                    GET_FIELD(*attr, DECODE_CFG_ENABLE),
                    GET_FIELD(*attr, DECODE_CFG_CMD_DTA),
                    GET_FIELD(*attr, DECODE_CFG_STRICT));
        }
        i++;
    }

    return *offset;
}

static int create_ubus_proc(void)
{
    int rc = 0;

    proc_dir = proc_mkdir(PROC_DIR, NULL);
    if (!proc_dir) 
    {
        pr_err("Failed to create PROC directory %s.\n",
            PROC_DIR);
        goto error;
    }

    if (!(proc_decode_cfg = proc_create(PROC_DIR "/" UBUS_DECODE_FILE, 0644, NULL, &ubus_decode_proc_fops))) 
		goto error;

    if (!(proc_tokens = proc_create(PROC_DIR "/" UBUS_TOKENS_FILE, 0644, NULL, &ubus_tokens_proc_fops))) 
		goto error;

    return rc;

error:
    if (proc_decode_cfg) 
    {
        remove_proc_entry(UBUS_DECODE_FILE, proc_dir);
        proc_decode_cfg = NULL;
    }	

    if (proc_tokens)
    {
        remove_proc_entry(UBUS_TOKENS_FILE, proc_dir);
        proc_tokens = NULL;
    }
    if (proc_dir)
    {
        remove_proc_entry(PROC_DIR, NULL);
        proc_dir = NULL;
    }
    
    return -1;	
}
#endif // #ifndef _CFE_

#ifdef CONFIG_BCM_UBUS_DECODE_REMAP
int ubus_master_remap_port(int master_port_id)
{
    int rc = 0;

    rc = ubus_master_decode_wnd_cfg(master_port_id, 
                                    DECODE_WIN0, 
                                    MST_START_DDR_ADDR, 
                                    g_board_size_power_of_2, 
                                    UBUS_PORT_ID_BIU, 
                                    IS_DDR_COHERENT ? CACHE_BIT_ON : CACHE_BIT_OFF);
    if (rc < 0)
    {
        printk("Error %s line[%d] port[%d] address[0x%x] size[%d]: \n",
               __FILE__, __LINE__, master_port_id, MST_START_DDR_ADDR,g_board_size_power_of_2);
    }
#if defined(CONFIG_BCM_FPM_COHERENCY_EXCLUDE)
    /*
     * CONFIG_BCM_FPM_COHERENCY_EXCLUDE is the case when all the memory is coherent
     * except for FPM pool, since CCI-400 won't sustain 10G traffic load. In that
     * case we remap FPM pool area to be non-coherent
     */
    else
    {
        rc = ubus_master_decode_wnd_cfg(master_port_id,
                                        DECODE_WIN1,
                                        fpm_pool_addr,
                                        fpm_pool_size,
                                        UBUS_PORT_ID_BIU,
                                        CACHE_BIT_OFF);
        if (rc < 0)
        {
            printk("Error %s line[%d] port[%d] address[0x%x] size[%d]: \n",
                   __FILE__, __LINE__, master_port_id, MST_START_DDR_ADDR,g_board_size_power_of_2);
        }
    }
#endif /* CONFIG_BCM_FPM_COHERENCY_EXCLUDE */
    return rc;
}
EXPORT_SYMBOL(ubus_master_remap_port);

int remap_ubus_masters_biu(void)
{
    int rc = 0;
    unsigned int i = 0;

    /* Calculate board size of power 2 */
    g_board_size_power_of_2 = log2_32(getMemorySize());

#if defined(CONFIG_BCM_FPM_COHERENCY_EXCLUDE)
    rc = BcmMemReserveGetByName(FPMPOOL_BASE_ADDR_STR, NULL, &fpm_pool_addr, &fpm_pool_size);
    if (rc < 0)
    {
        printk("Error %s line[%d]: failed to get fpm_pool base address\n",
               __FILE__, __LINE__);
        goto exit_;
    }
    fpm_pool_size = log2_32(fpm_pool_size);
#endif /* CONFIG_BCM_FPM_COHERENCY_EXCLUDE */

    UBUS_REMAP_DEBUG_LOG("\x1b[35m board_sdram_size[0x%lx] board_size_power_of_2[%d]\x1b[0m\n",
                         getMemorySize(), g_board_size_power_of_2);

    while(ub_mst_addr_map_tbl[i].port_id != -1)
    {
        rc = ubus_master_remap_port(ub_mst_addr_map_tbl[i].port_id);
        if (rc < 0)
            goto exit_;
        i++;
    }

exit_:
    if (rc < 0)
        printk("Error: line[%d] rc = %d\n",__LINE__, rc);

    return rc;                       
}
#endif /* CONFIG_BCM_UBUS_DECODE_REMAP */

#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM96858)
void apply_ubus_credit_each_master(int master)
{
    int i, master_idx;
    ubus_credit_cfg_t credit;

    for ( i = 0; i < UBUS_NUM_OF_MST_PORTS; i++ ) {
        if (ubus_credit_tbl[i][0].port_id == master)
        {
            master_idx = i;
            break;
        }
    }

    if (i == (UBUS_NUM_OF_MST_PORTS)) {
        printk("Error: master port id %d not found in credit table\n", master);
        return;
    }

    for( i = 1; i < UBUS_MAX_PORT_NUM; i++ ) {
        credit = ubus_credit_tbl[master_idx][i];
        if( credit.port_id == -1 )
            break;
        ubus_master_set_token_credits(master, credit.port_id, credit.credit);
    }
    return;
}
#ifndef _CFE_
static void apply_ubus_credit(void) 
{
    int i = 0;

    while(ub_mst_addr_map_tbl[i].port_id != -1)
    {
        apply_ubus_credit_each_master(ub_mst_addr_map_tbl[i].port_id);
        i++;
    }

    return;
}
#endif
#endif

#if defined(_BCM96858_)
void ubus_master_rte_cfg(void)
{
    ubus_master_set_rte_addr(UBUS_PORT_ID_RQ0, UBUS_PORT_ID_RQ0, 0x9);
    ubus_master_set_rte_addr(UBUS_PORT_ID_RQ0, UBUS_PORT_ID_RQ1, 0x209);
    ubus_master_set_rte_addr(UBUS_PORT_ID_RQ0, UBUS_PORT_ID_RQ2, 0x109);
    ubus_master_set_rte_addr(UBUS_PORT_ID_RQ0, UBUS_PORT_ID_RQ3, 0x309);
    ubus_master_set_rte_addr(UBUS_PORT_ID_RQ1, UBUS_PORT_ID_RQ0, 0x9);
    ubus_master_set_rte_addr(UBUS_PORT_ID_RQ1, UBUS_PORT_ID_RQ1, 0x209);
    ubus_master_set_rte_addr(UBUS_PORT_ID_RQ1, UBUS_PORT_ID_RQ2, 0x109);
    ubus_master_set_rte_addr(UBUS_PORT_ID_RQ1, UBUS_PORT_ID_RQ3, 0x309);
    ubus_master_set_rte_addr(UBUS_PORT_ID_RQ2, UBUS_PORT_ID_RQ0, 0x8);
    ubus_master_set_rte_addr(UBUS_PORT_ID_RQ2, UBUS_PORT_ID_RQ1, 0x208);
    ubus_master_set_rte_addr(UBUS_PORT_ID_RQ2, UBUS_PORT_ID_RQ2, 0x108);
    ubus_master_set_rte_addr(UBUS_PORT_ID_RQ2, UBUS_PORT_ID_RQ3, 0x308);
    ubus_master_set_rte_addr(UBUS_PORT_ID_RQ3, UBUS_PORT_ID_RQ0, 0x8);
    ubus_master_set_rte_addr(UBUS_PORT_ID_RQ3, UBUS_PORT_ID_RQ1, 0x208);
    ubus_master_set_rte_addr(UBUS_PORT_ID_RQ3, UBUS_PORT_ID_RQ2, 0x108);
    ubus_master_set_rte_addr(UBUS_PORT_ID_RQ3, UBUS_PORT_ID_RQ3, 0x308);
    ubus_master_set_rte_addr(UBUS_PORT_ID_DMA0, UBUS_PORT_ID_RQ0, 0x0);
    ubus_master_set_rte_addr(UBUS_PORT_ID_DMA0, UBUS_PORT_ID_RQ1, 0x8);
    ubus_master_set_rte_addr(UBUS_PORT_ID_DMA0, UBUS_PORT_ID_RQ2, 0x4);
    ubus_master_set_rte_addr(UBUS_PORT_ID_DMA0, UBUS_PORT_ID_RQ3, 0xc);
    ubus_master_set_rte_addr(UBUS_PORT_ID_DMA1, UBUS_PORT_ID_RQ0, 0x1);
    ubus_master_set_rte_addr(UBUS_PORT_ID_DMA1, UBUS_PORT_ID_RQ1, 0x9);
    ubus_master_set_rte_addr(UBUS_PORT_ID_DMA1, UBUS_PORT_ID_RQ2, 0x5);
    ubus_master_set_rte_addr(UBUS_PORT_ID_DMA1, UBUS_PORT_ID_RQ3, 0xd);
    ubus_master_set_rte_addr(UBUS_PORT_ID_DQM, UBUS_PORT_ID_RQ0, 0x2);
    ubus_master_set_rte_addr(UBUS_PORT_ID_DQM, UBUS_PORT_ID_RQ1, 0x6);
    ubus_master_set_rte_addr(UBUS_PORT_ID_DQM, UBUS_PORT_ID_RQ2, 0x24);
    ubus_master_set_rte_addr(UBUS_PORT_ID_DQM, UBUS_PORT_ID_RQ3, 0x64);
    ubus_master_set_rte_addr(UBUS_PORT_ID_NATC, UBUS_PORT_ID_RQ0, 0x0);
    ubus_master_set_rte_addr(UBUS_PORT_ID_NATC, UBUS_PORT_ID_RQ1, 0x8);
    ubus_master_set_rte_addr(UBUS_PORT_ID_NATC, UBUS_PORT_ID_RQ2, 0x4);
    ubus_master_set_rte_addr(UBUS_PORT_ID_NATC, UBUS_PORT_ID_RQ3, 0xc);
}
#endif

#if defined(CONFIG_BCM96858) || defined(_BCM96858_) || defined(CONFIG_BCM96846) || defined(_BCM96846_) || defined(CONFIG_BCM96856) || defined(_BCM96856_)
// ucb commands
#define UCB_CMD_RD               0
#define UCB_CMD_RD_RPLY          1
#define UCB_CMD_WR               2
#define UCB_CMD_WR_ACK           3
#define UCB_CMD_BCAST            4

static void write_reg_thr_sm(uint32_t ucbid, uint32_t addr, uint32_t data)
{
    // check if response fifo is full
    while( UBUSSYSTOP->ReadUcbStatus & 0x40000000);

    UBUSSYSTOP->UcbData = data;
    
    UBUSSYSTOP->UcbHdr = (addr/4) | (UCB_CMD_WR<<12) | (ucbid<<16) | (0x1<<24);
    
    // check if resp fifo has data
    while( !(UBUSSYSTOP->ReadUcbStatus & 0x80000000 ) );
  
    UBUSSYSTOP->ReadUcbHdr;
}

static uint32_t read_reg_thr_sm(uint32_t ucbid, uint32_t addr)
{
    // check if response fifo is full
    while( UBUSSYSTOP->ReadUcbStatus & 0x40000000);    

    UBUSSYSTOP->UcbHdr = (addr/4) | (UCB_CMD_RD<<12) | (ucbid<<16) | (0x1<<24);

    // check if resp fifo has data
    while( !(UBUSSYSTOP->ReadUcbStatus & 0x80000000 ) );

    UBUSSYSTOP->ReadUcbHdr;

    return UBUSSYSTOP->ReadUcbData;
}


void ubus_deregister_port(int ucbid)
{
    // Never play with invalid ubus id
    if (ucbid < 0) return;

    write_reg_thr_sm(ucbid, 0x1c, 0x1);

    // poll status bit for port unregistered
    while ( read_reg_thr_sm(ucbid, 0x1c) != 0x1 );
}

void ubus_register_port(int ucbid)
{
    // Never play with invalid ubus id
    if (ucbid < 0) return;

#if defined(CONFIG_BCM96858) || defined(_BCM96858_) || defined(CONFIG_BCM96846) || defined(_BCM96846_) || \
    defined (CONFIG_BCM96856) || defined(_BCM96856_)
#if defined(CONFIG_BCM96858) || defined(_BCM96858_)
    if ((ucbid==UCB_NODE_ID_MST_USB) || (ucbid==UCB_NODE_ID_SLV_USB) || (ucbid==UCB_NODE_ID_MST_SATA) || (ucbid==UCB_NODE_ID_SLV_SATA))
#endif
    {
    write_reg_thr_sm(ucbid, 0x1c, 0x0);
    // poll status bit for port registered
    while ( read_reg_thr_sm(ucbid, 0x1c) != 0x2 );
    }
#endif
}
#endif //#if defined(CONFIG_BCM96858) || defined(_BCM96858_) || defined(CONFIG_BCM96846) || defined(_BCM96846_)

#if defined(CONFIG_BCM96858) || defined(_BCM96858_)
int ubus_master_set_rte_addr(int master_port_id, int port, int val)
{
    volatile MstPortNode *master_addr = NULL;

    master_addr = bcm_get_ubus_mst_addr(master_port_id);    

    if (!master_addr) {
        printk("Error setting ubus rte addr, master %d not found\n", master_port_id);
        return -1;
    }

    master_addr->routing_addr[port] = val;

    return 0;
}
#endif //#if defined(CONFIG_BCM96858) || defined(_BCM96858_)

/*this function is used to set UBUS route credits per ubus master, should be equivalent configuration at masters*/
int ubus_master_set_token_credits(int master_port_id, int token, int credits)
{
    volatile MstPortNode *master_addr = NULL;

    master_addr = bcm_get_ubus_mst_addr(master_port_id);

    if (!master_addr) {
        printk("Error setting ubus credits, master %d not found\n", master_port_id);
        return -1;
    }

    //printk("Master node %02d(%p) credit for ubus port %02d is 0x%08x\n", node, &master_addr->token[token], token, master_addr->token[token]);
        master_addr->token[token] = credits;
    //printk("Master node %02d(%p) credit for ubus port %02d set to %02d, read back 0x%08x\n", node, &master_addr->token[token], token, credits, master_addr->token[token]);
    return 0;
}
#ifndef _CFE_
EXPORT_SYMBOL(ubus_master_set_token_credits);
#endif

// This function initialize ubus master port structure
void ubus_master_port_init(void)
{
#ifndef _CFE_
    int i=0;

    while (ub_mst_addr_map_tbl[i].port_id != -1)
    {
        ub_mst_addr_map_tbl[i].base = (uintptr_t)bcm_dev_phy2vir(ub_mst_addr_map_tbl[i].base);
        i++;
    }
#endif
}

void bcm_ubus_config(void)
{
#ifndef _CFE_
    create_ubus_proc();
#endif
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM96858)
#ifndef _CFE_
    apply_ubus_credit();
#endif
#endif
#if defined(_BCM96858_)
    ubus_master_rte_cfg();
#endif
#if defined(CONFIG_BCM96878)
    ubus_master_set_token_credits(UBUS_PORT_ID_PCIE0, UBUS_PORT_ID_VPB, 1);
#endif
#ifdef CONFIG_BCM_UBUS_DECODE_REMAP
    remap_ubus_masters_biu();
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622)
    ubus_remap_to_biu_cfg_wlu_srcpid(MAX_WLU_SRCPID_NUM, 1);
#endif
#if !defined(CONFIG_BCM963178) && !defined(CONFIG_BCM947622)
    configure_biu_pid_to_queue();
#endif
#endif /* CONFIG_BCM_UBUS_DECODE_REMAP */
}
