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

#ifndef IS_BCMCHIP
#define IS_BCMCHIP(num) (defined(_BCM9##num##_)||defined(CONFIG_BCM9##num)|| defined(CONFIG_BCM##num))
#endif

typedef struct ub_mst_addr_map_t {
    int           port_id;
    unsigned long base;
} ub_mst_addr_map_t;

#if IS_BCMCHIP(6858)
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


#if IS_BCMCHIP(6856)
#define UCB_NODE_ID_SLV_SYS     0
#define UCB_NODE_ID_MST_PCIE0   3
#define UCB_NODE_ID_SLV_PCIE0   4
#define UCB_NODE_ID_MST_USB     14
#define UCB_NODE_ID_SLV_USB     15
#define UCB_NODE_ID_SLV_MEMC    16
#elif IS_BCMCHIP(6846)
#define UCB_NODE_ID_MST_PCIE0   3
#define UCB_NODE_ID_SLV_PCIE0   4
#define UCB_NODE_ID_MST_USB     14
#define UCB_NODE_ID_SLV_USB     15
#define UCB_NODE_ID_SLV_MEMC    16
#endif


#if IS_BCMCHIP(6846) || IS_BCMCHIP(6856) || IS_BCMCHIP(6855)
extern unsigned int g_board_size_power_of_2;
#endif

#if IS_BCMCHIP(63158)

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

#elif IS_BCMCHIP(63146)
#if (CONFIG_BRCM_CHIP_REV == 0x63146A0)
#define UBUS_MAX_PORT_NUM        17
#define UBUS_NUM_OF_MST_PORTS    13
#else
#define UBUS_MAX_PORT_NUM        18
#define UBUS_NUM_OF_MST_PORTS    14
#endif
#define UBUS_PORT_ID_LAST_SYSTOP 0

#define UBUS_PORT_ID_MEMC        1
#define UBUS_PORT_ID_BIU         2
#define UBUS_PORT_ID_PER         3
#define UBUS_PORT_ID_USB         4
#define UBUS_PORT_ID_DSL         5
#define UBUS_PORT_ID_DSLCPU      6
#define UBUS_PORT_ID_ETHPHY      7
#define UBUS_PORT_ID_PCIE0       8
#define UBUS_PORT_ID_PCIE1       9
#define UBUS_PORT_ID_PCIE2       10
#define UBUS_PORT_ID_PSRAM       11
#define UBUS_PORT_ID_DMA0        12
#define UBUS_PORT_ID_DMA1        13
#define UBUS_PORT_ID_RQ0         14
#define UBUS_PORT_ID_QM          15
#define UBUS_PORT_ID_VPB         16
#if (CONFIG_BRCM_CHIP_REV != 0x63146A0)
#define UBUS_PORT_ID_MPM         17
#endif

#elif IS_BCMCHIP(4912) || IS_BCMCHIP(6813)
#define UBUS_MAX_PORT_NUM        24
#define UBUS_NUM_OF_MST_PORTS    16
#define UBUS_PORT_ID_LAST_SYSTOP 0

#define UBUS_PORT_ID_MEMC        1
#define UBUS_PORT_ID_BIU         2
#define UBUS_PORT_ID_PER         3
#define UBUS_PORT_ID_USB         4
#define UBUS_PORT_ID_ETHPHY      5
#define UBUS_PORT_ID_PCIE0       8
#define UBUS_PORT_ID_PCIE1       9
#define UBUS_PORT_ID_PCIE2       10
#define UBUS_PORT_ID_PCIE3       11
#define UBUS_PORT_ID_SPU         12
#define UBUS_PORT_ID_VPB         14
#define UBUS_PORT_ID_MPM         15
#define UBUS_PORT_ID_DMA0        16
#define UBUS_PORT_ID_DMA1        17
#define UBUS_PORT_ID_DMA2        18
#define UBUS_PORT_ID_QM          19
#define UBUS_PORT_ID_RQ0         20
#define UBUS_PORT_ID_RQ1         21
#define UBUS_PORT_ID_PSRAM       22
#define UBUS_PORT_ID_PSRAM1      23

#elif IS_BCMCHIP(6858)
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

#elif IS_BCMCHIP(6846)
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

#elif IS_BCMCHIP(6878)
#define UBUS_PORT_ID_MEMC        1
#define UBUS_MAX_PORT_NUM        14
#define UBUS_NUM_OF_MST_PORTS    8
#define UBUS_PORT_ID_LAST_SYSTOP 7

#define UBUS_PORT_ID_BIU         2
#define UBUS_PORT_ID_PER         3
#define UBUS_PORT_ID_USB         4
#define UBUS_PORT_ID_WIFI        6
#define UBUS_PORT_ID_PCIE0       7
#define UBUS_PORT_ID_QM          11
#define UBUS_PORT_ID_DMA0        13
#define UBUS_PORT_ID_RQ0         14

#define UBUS_PORT_ID_VPB         9

#elif IS_BCMCHIP(6855)
#define UBUS_PORT_ID_MEMC        1
#define UBUS_MAX_PORT_NUM        21
#define UBUS_NUM_OF_MST_PORTS    11
#define UBUS_PORT_ID_LAST_SYSTOP 12

#define UBUS_PORT_ID_BIU         2
#define UBUS_PORT_ID_PER         3
#define UBUS_PORT_ID_USB         4
#define UBUS_PORT_ID_PCIE0       10
#define UBUS_PORT_ID_WIFI        12
#define UBUS_PORT_ID_DMA0        16
#define UBUS_PORT_ID_DMA1        17
#define UBUS_PORT_ID_DMA2        18
#define UBUS_PORT_ID_QM          19
#define UBUS_PORT_ID_RQ0         20
#define UBUS_PORT_ID_RQ1         21
#define UBUS_PORT_ID_PSRAM       22
#define UBUS_PORT_ID_VPB         24

#elif IS_BCMCHIP(63178)
#define UBUS_MAX_PORT_NUM        11
#define UBUS_NUM_OF_MST_PORTS    9
#define UBUS_PORT_ID_LAST_SYSTOP 0

#define UBUS_PORT_ID_SYS         0
#define UBUS_PORT_ID_MEMC        1
#define UBUS_PORT_ID_BIU         2
#define UBUS_PORT_ID_PER         3
#define UBUS_PORT_ID_USB         4
#define UBUS_PORT_ID_DSL         5
#define UBUS_PORT_ID_DSLCPU      6
#define UBUS_PORT_ID_PMC         7
#define UBUS_PORT_ID_SWH         8
#define UBUS_PORT_ID_PCIE0       10
#define UBUS_PORT_ID_WIFI        12

#elif IS_BCMCHIP(47622)
#define UBUS_MAX_PORT_NUM        12
#define UBUS_NUM_OF_MST_PORTS    10
#define UBUS_PORT_ID_LAST_SYSTOP 0

#define UBUS_PORT_ID_SYS         0
#define UBUS_PORT_ID_MEMC        1
#define UBUS_PORT_ID_BIU         2
#define UBUS_PORT_ID_PER         3
#define UBUS_PORT_ID_USB         4
#define UBUS_PORT_ID_PMC         7
#define UBUS_PORT_ID_SYSPORT     8
#define UBUS_PORT_ID_SYSPORT1    9
#define UBUS_PORT_ID_PCIE0       10
#define UBUS_PORT_ID_SPU         11
#define UBUS_PORT_ID_WIFI        13
#define UBUS_PORT_ID_WIFI1       14

#elif IS_BCMCHIP(6856)
#define UBUS_MAX_PORT_NUM        19
#define UBUS_NUM_OF_MST_PORTS    12
#define UBUS_PORT_ID_LAST_SYSTOP 7

#define UBUS_PORT_ID_MEMC        1
#define UBUS_PORT_ID_BIU         2
#define UBUS_PORT_ID_PER         3
#define UBUS_PORT_ID_USB         4
#define UBUS_PORT_ID_PCIE2       6
#define UBUS_PORT_ID_PCIE0       7
#define UBUS_PORT_ID_VPB         9
#define UBUS_PORT_ID_QM          11
#define UBUS_PORT_ID_DQM         12
#define UBUS_PORT_ID_DMA0        13
#define UBUS_PORT_ID_NATC        14
#define UBUS_PORT_ID_DMA1        17
#define UBUS_PORT_ID_RQ0         18
#define UBUS_PORT_ID_RQ1         19

#elif defined (CONFIG_BCM96756) || defined(_BCM96756_)
#define UBUS_MAX_PORT_NUM        12
#define UBUS_NUM_OF_MST_PORTS    10
#define UBUS_PORT_ID_LAST_SYSTOP 0

#define UBUS_PORT_ID_SYS         0
#define UBUS_PORT_ID_MEMC        1
#define UBUS_PORT_ID_BIU         2
#define UBUS_PORT_ID_PER         3
#define UBUS_PORT_ID_USB         4
#define UBUS_PORT_ID_PMC         7
#define UBUS_PORT_ID_SWH         8
#define UBUS_PORT_ID_MPM         9
#define UBUS_PORT_ID_PCIE0       10
#define UBUS_PORT_ID_SPU         11
#define UBUS_PORT_ID_WIFI        13
#define UBUS_PORT_ID_WIFI1       14

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
    uint32_t base_addr;
    uint32_t remap_addr;
    uint32_t attributes;
}DecodeCfgMstWndRegs;

/* BIU Registers */

typedef struct
{
    uint32_t ctrl;
    uint32_t cache_cfg;
    uint32_t reserved[2];
    DecodeCfgMstWndRegs window[20];
} DecodeCfgRegs;

typedef struct
{
#define AXI_CFG_AWCACHE_BYPASS     (0x1<<5)
    uint32_t cfg;
    uint32_t bufsize;
} AXICfgRegs;

#if IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813)

#define ROUTE_ADDR_SIZE        0x400
#define TOKEN_SIZE             0x400
typedef struct
{
    uint32_t port_cfg[12];                  /* 0x0 */
#define DCM_UBUS_CONGESTION_THRESHOLD 3  
    uint32_t reserved1[52];                 /* 0x30 */
    uint32_t loopback[20];                  /* 0x100 */
    uint32_t reserved2[556];                /* 0x150 */  
    DecodeCfgRegs decode_cfg;               /* 0xA00 */
    uint32_t reserved3[128];                /* 0xB00 */
    AXICfgRegs axi_cfg;                     /* 0xD00 */
    uint32_t reserved4[190];                /* 0xD08 */  
    uint32_t routing_addr[ROUTE_ADDR_SIZE]; /* 0x1000 */
    uint32_t token[TOKEN_SIZE];             /* 0x2000 */
} MstPortNode;

#else

#if IS_BCMCHIP(63178) || IS_BCMCHIP(47622) || IS_BCMCHIP(6878) || IS_BCMCHIP(6756) || IS_BCMCHIP(6855)
#define ROUTE_ADDR_SIZE        0x100
#define TOKEN_SIZE             0x100
#else
#define ROUTE_ADDR_SIZE        0x80
#define TOKEN_SIZE             0x80
#endif

typedef struct
{
    uint32_t port_cfg[8];
#define DCM_UBUS_CONGESTION_THRESHOLD 3
    uint32_t reserved1[56];
    uint32_t loopback[20];
    uint32_t reserved2[44];
    uint32_t routing_addr[ROUTE_ADDR_SIZE];
    uint32_t token[TOKEN_SIZE];
    DecodeCfgRegs decode_cfg;
    uint8_t rsvd1[512];
    AXICfgRegs axi_cfg;
} MstPortNode;

#endif

typedef struct ubus_credit_cfg {
    int port_id;
    int credit;
}ubus_credit_cfg_t;

void ubus_master_port_init(void);
void bcm_ubus_config(void);
void ubus_cong_threshold_wr(int port_id, unsigned int val);
int ubus_master_remap_port(int master_port_id);
int log2_32 (unsigned long value);
int ubus_master_set_token_credits(int master_port_id, int token, int credits);
void ubus_deregister_port(int ucbid);
void ubus_register_port(int ucbid);
#if IS_BCMCHIP(63158) || IS_BCMCHIP(6858)
void apply_ubus_credit_each_master(int master);
#endif
#if IS_BCMCHIP(6858)
void ubus_master_rte_cfg(void);
#endif
#if IS_BCMCHIP(63178)
void configure_ubus_sar_reg_decode(void);
#endif
void ubus_master_cpu_enable_axi_write_cache(int enable);

#endif
