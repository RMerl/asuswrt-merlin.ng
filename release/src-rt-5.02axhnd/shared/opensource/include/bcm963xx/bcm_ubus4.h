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

#ifndef __BCM_UBUS4_H__
#define __BCM_UBUS4_H__

#include <bcm_map_part.h>

typedef struct ub_mst_addr_map_t {
    int           port_id;
    unsigned long base;
} ub_mst_addr_map_t;

#if defined (CONFIG_BCM96858) || defined(_BCM96858_) 
#define UCB_NODE_ID_SLV_SYS     0
#define UCB_NODE_ID_MST_PCIE0   3
#define UCB_NODE_ID_SLV_PCIE0   4
#define UCB_NODE_ID_MST_PCIE2   5
#define UCB_NODE_ID_SLV_PCIE2   6
#define UCB_NODE_ID_MST_SATA    UCB_NODE_ID_MST_PCIE2
#define UCB_NODE_ID_SLV_SATA    UCB_NODE_ID_SLV_PCIE2
#define UCB_NODE_ID_MST_USB     14
#define UCB_NODE_ID_SLV_USB     15
#define UCB_NODE_ID_SLV_LPORT   19
#define UCB_NODE_ID_SLV_WAN     21

int ubus_master_set_rte_addr(int master_port_id, int port, int val);
#endif


#if defined(CONFIG_BCM96856) || defined(_BCM96856_)
#define UCB_NODE_ID_SLV_SYS     0
#define UCB_NODE_ID_MST_PCIE0   3
#define UCB_NODE_ID_SLV_PCIE0   4
#define UCB_NODE_ID_MST_USB     14
#define UCB_NODE_ID_SLV_USB     15
#define UCB_NODE_ID_SLV_MEMC    16
#elif defined(CONFIG_BCM96846) || defined(_BCM96846_)
#define UCB_NODE_ID_MST_PCIE0   3
#define UCB_NODE_ID_SLV_PCIE0   4
#define UCB_NODE_ID_MST_USB     14
#define UCB_NODE_ID_SLV_USB     15
#define UCB_NODE_ID_SLV_MEMC    16
#endif


#if defined(CONFIG_BCM96846) || defined(_BCM96846_) || defined (CONFIG_BCM96856) || defined(_BCM96856_) 
extern unsigned int g_board_size_power_of_2;
#endif

#if defined (CONFIG_BCM963158) || defined(_BCM963158_)

#define UBUS_MAX_PORT_NUM        32
#define UBUS_NUM_OF_MST_PORTS    17
#define UBUS_PORT_ID_LAST_SYSTOP 13

#define UBUS_PORT_ID_BIU         2
#define UBUS_PORT_ID_DMA0        24
#define UBUS_PORT_ID_DQM         23
#define UBUS_PORT_ID_DSLCPU      11
#define UBUS_PORT_ID_DSL         6
#define UBUS_PORT_ID_FPM         21
#define UBUS_PORT_ID_MEMC        1
#define UBUS_PORT_ID_NATC        26
#define UBUS_PORT_ID_PCIE0       8
#define UBUS_PORT_ID_PCIE2       9
#define UBUS_PORT_ID_PCIE3       10
#define UBUS_PORT_ID_PERDMA      7
#define UBUS_PORT_ID_PER         3
#define UBUS_PORT_ID_PMC         13
#define UBUS_PORT_ID_PSRAM       16
#define UBUS_PORT_ID_QM          22
#define UBUS_PORT_ID_RQ0         32
#define UBUS_PORT_ID_SPU         5
#define UBUS_PORT_ID_SWH         14
#define UBUS_PORT_ID_SYS         31
#define UBUS_PORT_ID_SYSXRDP     27
#define UBUS_PORT_ID_USB         4
#define UBUS_PORT_ID_VPB         20
#define UBUS_PORT_ID_WAN         12

#elif defined (CONFIG_BCM96858) || defined(_BCM96858_)
#define UBUS_MAX_PORT_NUM        35
#define UBUS_NUM_OF_MST_PORTS    19
#define UBUS_PORT_ID_LAST_SYSTOP 15

#define UBUS_PORT_ID_MEMC        1
#define UBUS_PORT_ID_BIU         2
#define UBUS_PORT_ID_PER         3
#define UBUS_PORT_ID_USB         4
#define UBUS_PORT_ID_PERDMA      5
#define UBUS_PORT_ID_SPU         6
#define UBUS_PORT_ID_PCIE0       8
#define UBUS_PORT_ID_PCIE2       9
#define UBUS_PORT_ID_PMC         15
#define UBUS_PORT_ID_XRDP_VPB    20
#define UBUS_PORT_ID_QM          22
#define UBUS_PORT_ID_DQM         23
#define UBUS_PORT_ID_DMA0        24
#define UBUS_PORT_ID_DMA1        25
#define UBUS_PORT_ID_NATC        26
#define UBUS_PORT_ID_TOP_BUFF    28
#define UBUS_PORT_ID_XRDP_BUFF   29
#define UBUS_PORT_ID_RQ0         32
#define UBUS_PORT_ID_RQ1         33
#define UBUS_PORT_ID_RQ2         34
#define UBUS_PORT_ID_RQ3         35

#elif defined (CONFIG_BCM96846) || defined(_BCM96846_)
#define UBUS_MAX_PORT_NUM        15
#define UBUS_NUM_OF_MST_PORTS    9
#define UBUS_PORT_ID_LAST_SYSTOP 7

#define UBUS_PORT_ID_MEMC        1
#define UBUS_PORT_ID_BIU         2
#define UBUS_PORT_ID_PER         3
#define UBUS_PORT_ID_USB         4
#define UBUS_PORT_ID_PCIE0       7
#define UBUS_PORT_ID_QM          11
#define UBUS_PORT_ID_DQM         12
#define UBUS_PORT_ID_DMA0        13
#define UBUS_PORT_ID_NATC        14
#define UBUS_PORT_ID_RQ0         15

#elif defined (CONFIG_BCM96856) || defined(_BCM96856_)
#define UBUS_MAX_PORT_NUM        19
#define UBUS_NUM_OF_MST_PORTS    12
#define UBUS_PORT_ID_LAST_SYSTOP 7

#define UBUS_PORT_ID_MEMC        1
#define UBUS_PORT_ID_BIU         2
#define UBUS_PORT_ID_PER         3
#define UBUS_PORT_ID_USB         4
#define UBUS_PORT_ID_PCIE2       6
#define UBUS_PORT_ID_PCIE0       7
#define UBUS_PORT_ID_QM          11
#define UBUS_PORT_ID_DQM         12
#define UBUS_PORT_ID_DMA0        13
#define UBUS_PORT_ID_NATC        14
#define UBUS_PORT_ID_DMA1        17
#define UBUS_PORT_ID_RQ0         18
#define UBUS_PORT_ID_RQ1         19
#endif

#ifdef CONFIG_BCM_GLB_COHERENCY
#define IS_DDR_COHERENT 1
#else
#define IS_DDR_COHERENT 0
#endif

#define DECODE_CFG_SIZE_MASK        0xff00
#define DECODE_CFG_SIZE_SHIFT       8
#define DECODE_CFG_ENABLE_MASK      0x60000
#define DECODE_CFG_ENABLE_SHIFT     17
#define DECODE_CFG_CACHE_BITS_MASK  0x380000
#define DECODE_CFG_CACHE_BITS_SHIFT 19
#define DECODE_CFG_CMD_DTA_MASK     0x10000
#define DECODE_CFG_CMD_DTA_SHIFT    16
#define DECODE_CFG_STRICT_MASK      0x400000
#define DECODE_CFG_STRICT_SHIFT     22
#define DECODE_CFG_PORT_ID_MASK     0xff
#define DECODE_CFG_PORT_ID_SHIFT    0
#define DECODE_CFG_CTRL_CACHE_SEL_SHIFT 4
#define DECODE_CFG_CTRL_CACHE_SEL_MASK  (0x3<<DECODE_CFG_CTRL_CACHE_SEL_SHIFT)

#define DECODE_CFG_ENABLE_OFF       (0 << DECODE_CFG_ENABLE_SHIFT)

#define DECODE_CFG_ENABLE_ADDR_ONLY (1 << DECODE_CFG_ENABLE_SHIFT)
#define DECODE_CFG_ENABLE_CDSM_0    (2 << DECODE_CFG_ENABLE_SHIFT)
#define DECODE_CFG_ENABLE_CDSM_1    (3 << DECODE_CFG_ENABLE_SHIFT)
#define DECODE_CFG_CACHE_BITS       (1 << DECODE_CFG_CACHE_BITS_SHIFT)

#define DECODE_CFG_CTRL_CACHE_SEL_DEF     (0x0<<DECODE_CFG_CTRL_CACHE_SEL_SHIFT)
#define DECODE_CFG_CTRL_CACHE_SEL_INPUT   (0x1<<DECODE_CFG_CTRL_CACHE_SEL_SHIFT)
#define DECODE_CFG_CTRL_CACHE_SEL_CFG_REG (0x2<<DECODE_CFG_CTRL_CACHE_SEL_SHIFT)

/**
 * m = memory, f = field
 */
#define GET_FIELD(m,f) \
	(((m) & f##_MASK) >> f##_SHIFT)

typedef struct
{
   uint32 base_addr;
   uint32 remap_addr;
   uint32 attributes;
}DecodeCfgMstWndRegs;

/* BIU Registers */

/* not relevant for 6846 platform but stil compile in pon stack */
#define NGPON2_IDX 0

typedef struct
{
   uint32 ctrl;
   uint32 cache_cfg;
   uint32 reserved[2];
   DecodeCfgMstWndRegs window[4];
} DecodeCfgRegs;

typedef struct
{
   uint32 port_cfg[8];
#define DCM_UBUS_CONGESTION_THRESHOLD 3
   uint32 reserved1[56];
   uint32 loopback[20];
   uint32 reserved2[44];
   uint32 routing_addr[128];
   uint32 token[128];
   DecodeCfgRegs decode_cfg;
} MstPortNode;

typedef struct ubus_credit_cfg {
    int port_id;
    int credit;
}ubus_credit_cfg_t;

void ubus_master_port_init(void);
void bcm_ubus_config(void);
void ubus_cong_threshold_wr(int port_id, unsigned int val);
int ubus_master_remap_port(int master_port_id);
int log2_32 (unsigned int value);
int ubus_master_set_token_credits(int master_port_id, int token, int credits);
void ubus_deregister_port(int ucbid);
void ubus_register_port(int ucbid);
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM96858)
void apply_ubus_credit_each_master(int master);
#endif
#if defined(_BCM96858_)
void ubus_master_rte_cfg(void);
#endif

#endif
