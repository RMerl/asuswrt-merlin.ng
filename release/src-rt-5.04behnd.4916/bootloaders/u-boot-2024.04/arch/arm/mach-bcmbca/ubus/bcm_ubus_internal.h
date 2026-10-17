/*
 * <:copyright-BRCM:2021:DUAL/GPL:standard
 * 
 *    Copyright (c) 2021 Broadcom 
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

#ifndef __BCM_UBUS_INTERNAL__H__
#define __BCM_UBUS_INTERNAL__H__

#include <linux/io.h>

#define UBUS_FLAGS_MODULE_XRDP   0x1

/**
 * m = memory, f = field
 */
#define GET_FIELD(m,f) \
    (((m) & f##_MASK) >> f##_SHIFT)

typedef struct ubus_t {
    void __iomem *systop;
    void __iomem *registration;
    void __iomem *cohport;
} ubus_t;

typedef struct ubus_dcm_t {
    void __iomem *clk;
    void __iomem *rcq_gen;
    int          xrdp_module;
} ubus_dcm_t;

typedef struct ub_mst_addr_map_t {
    int           port_id;
    char          *str;
    phys_addr_t   phys_base;
    void __iomem  *base;  
} ub_mst_addr_map_t;

typedef struct
{
#define AXI_CFG_AWCACHE_BYPASS     (0x1<<5)
    uint32_t cfg;
    uint32_t bufsize;
} AXICfgRegs;

typedef struct ubus_credit_cfg {
    int port_id;
    int credit;
}ubus_credit_cfg_t;

typedef struct Ubus4SysModuleTop {
    uint32_t port_cfg[8];      /* 0x00 */
    uint32_t unused0[8];       /* 0x20 */
    uint32_t UcbData;          /* 0x40 */
    uint32_t UcbHdr;           /* 0x44 */
    uint32_t UcbCntl;          /* 0x48 */
    uint32_t unused1;          /* 0x4c */
    uint32_t ReadUcbHdr;       /* 0x50 */
#define UCB_CMD_RD               0
#define UCB_CMD_RD_RPLY          1
#define UCB_CMD_WR               2
#define UCB_CMD_WR_ACK           3
#define UCB_CMD_BCAST            4
    uint32_t ReadUcbData;      /* 0x54 */
    uint32_t ReadUcbStatus;    /* 0x58 */
    uint32_t ReacUcbFifoStatus; /* 0x5c */
} Ubus4SysModuleTop;

typedef struct Ubus4ModuleClientRegistration {
    uint32_t SlvStatus[16];		/* 0x0	 */
    uint32_t MstStatus[ 8];    	/* 0x240 */
    uint32_t RegCntl;    			/* 0x260 */
    uint32_t SlvStopProgDelay;	/* 0x264 */
} Ubus4ModuleClientRegistration;

typedef struct Ubus4ClkCtrlCfgRegs {
    uint32_t ClockCtrl;
#define UBUS4_CLK_CTRL_EN_SHIFT    (0)
#define UBUS4_CLK_CTRL_EN_MASK     (0x1 << UBUS4_CLK_CTRL_EN_SHIFT)
#define UBUS4_CLK_BYPASS_SHIFT     (2)
#define UBUS4_CLK_BYPASS_MASK      (0x1 << UBUS4_CLK_BYPASS_SHIFT)
#define UBUS4_MIN_CLK_SEL_SHIFT    (4)
#define UBUS4_MIN_CLK_SEL_MASK     (0x7 << UBUS4_MIN_CLK_SEL_SHIFT)
#define UBUS4_MID_CLK_SEL_SHIFT    (8)
#define UBUS4_MID_CLK_SEL_MASK     (0x7 << UBUS4_MID_CLK_SEL_SHIFT)
    uint32_t reserved0[3];
    uint32_t Min2Mid_threshhold;
    uint32_t Mid2Max_threshhold;
    uint32_t Mid2Min_threshhold;
    uint32_t Max2Mid_threshhold;
    uint32_t ClkIntoMin;
    uint32_t ClkIntoMid;
    uint32_t ClkIntoMax;
    uint32_t reserved1;
    uint32_t ClkMinTime;
    uint32_t ClkMidTime;
    uint32_t ClkMaxTime;
} Ubus4ClkCtrlCfgRegs;

typedef struct
{
/*SRC_PID_CFG */
#define SRCPID_TO_QUEUE_0_BITS_SHIFT       (0)
#define SRCPID_TO_QUEUE_1_BITS_SHIFT       (4)
#define SRCPID_TO_QUEUE_2_BITS_SHIFT       (8)
#define SRCPID_TO_QUEUE_3_BITS_SHIFT       (12)
#define SRCPID_TO_QUEUE_4_BITS_SHIFT       (16)
#define SRCPID_TO_QUEUE_5_BITS_SHIFT       (20)
#define SRCPID_TO_QUEUE_6_BITS_SHIFT       (24)
#define SRCPID_TO_QUEUE_7_BITS_SHIFT       (28)
#define SRCPID_TO_QUEUE_0                  (1 << SRCPID_TO_QUEUE_0_BITS_SHIFT)                      
#define SRCPID_TO_QUEUE_1                  (1 << SRCPID_TO_QUEUE_1_BITS_SHIFT)
#define SRCPID_TO_QUEUE_2                  (1 << SRCPID_TO_QUEUE_2_BITS_SHIFT) 
#define SRCPID_TO_QUEUE_3                  (1 << SRCPID_TO_QUEUE_3_BITS_SHIFT) 
#define SRCPID_TO_QUEUE_4                  (1 << SRCPID_TO_QUEUE_4_BITS_SHIFT) 
#define SRCPID_TO_QUEUE_5                  (1 << SRCPID_TO_QUEUE_5_BITS_SHIFT) 
#define SRCPID_TO_QUEUE_6                  (1 << SRCPID_TO_QUEUE_6_BITS_SHIFT)
#define SRCPID_TO_QUEUE_7                  (1 << SRCPID_TO_QUEUE_7_BITS_SHIFT)
/* Depth CFG */     
#define DEPTH_TO_QUEUE_0_BITS_SHIFT       (0)
#define DEPTH_TO_QUEUE_1_BITS_SHIFT       (8)
#define DEPTH_TO_QUEUE_2_BITS_SHIFT       (16)
#define DEPTH_TO_QUEUE_3_BITS_SHIFT       (24)
#define DEPTH_TO_QUEUE_0                  (1 << DEPTH_TO_QUEUE_0_BITS_SHIFT)                      
#define DEPTH_TO_QUEUE_1                  (1 << DEPTH_TO_QUEUE_1_BITS_SHIFT)
#define DEPTH_TO_QUEUE_2                  (1 << DEPTH_TO_QUEUE_2_BITS_SHIFT) 
#define DEPTH_TO_QUEUE_3                  (1 << DEPTH_TO_QUEUE_3_BITS_SHIFT)
/* CBS Thresh */
#define THRESH_TO_QUEUE_0_BITS_SHIFT       (0)
#define THRESH_TO_QUEUE_1_BITS_SHIFT       (16)
#define THRESH_TO_QUEUE_0                  (1 << THRESH_TO_QUEUE_0_BITS_SHIFT)                      
#define THRESH_TO_QUEUE_1                  (1 << THRESH_TO_QUEUE_1_BITS_SHIFT)
/* Cir Inc CFG */     
#define CIR_INCR_TO_QUEUE_0_BITS_SHIFT    (0)
#define CIR_INCR_TO_QUEUE_1_BITS_SHIFT    (8)
#define CIR_INCR_TO_QUEUE_2_BITS_SHIFT    (16)
#define CIR_INCR_TO_QUEUE_3_BITS_SHIFT    (24)
#define CIR_INCR_TO_QUEUE_0               (1 << CIR_INCR_TO_QUEUE_0_BITS_SHIFT)                      
#define CIR_INCR_TO_QUEUE_1               (1 << CIR_INCR_TO_QUEUE_1_BITS_SHIFT)
#define CIR_INCR_TO_QUEUE_2               (1 << CIR_INCR_TO_QUEUE_2_BITS_SHIFT) 
#define CIR_INCR_TO_QUEUE_3               (1 << CIR_INCR_TO_QUEUE_3_BITS_SHIFT)
/* Ref Counters */
#define REF_CNT_0_BITS_SHIFT       (0)
#define REF_CNT_1_BITS_SHIFT       (4)
#define REF_CNT_2_BITS_SHIFT       (8)
#define REF_CNT_3_BITS_SHIFT       (12)
#define REF_CNT_4_BITS_SHIFT       (16)
#define REF_CNT_5_BITS_SHIFT       (20)
#define REF_CNT_6_BITS_SHIFT       (24)
#define REF_CNT_7_BITS_SHIFT       (28)
#define REF_CNT_0                  (1 << REF_CNT_0_BITS_SHIFT)                      
#define REF_CNT_1                  (1 << REF_CNT_1_BITS_SHIFT)
#define REF_CNT_2                  (1 << REF_CNT_2_BITS_SHIFT) 
#define REF_CNT_3                  (1 << REF_CNT_3_BITS_SHIFT) 
#define REF_CNT_4                  (1 << REF_CNT_4_BITS_SHIFT) 
#define REF_CNT_5                  (1 << REF_CNT_5_BITS_SHIFT) 
#define REF_CNT_6                  (1 << REF_CNT_6_BITS_SHIFT)
#define REF_CNT_7                  (1 << REF_CNT_7_BITS_SHIFT)         
   uint32_t lut[32];           /* 0x00 */
   uint32_t queue_depth[4];    /* 0x80 */
   uint32_t cbs_thresh[8];     /* 0x90 */
   uint32_t cir_incr[4];       /* 0xb0 */
   uint32_t ref_cnt[4];        /* 0xc0 */
   uint32_t max_bonus[2];      /* 0xd0 */
#define QUE_ID_NUM_BITS     (4) /* Number of queue_id bits per source port */
#define DEPTH_NUM_BITS      (8) /* Number of depth bits per queue id */
#define CBS_NUM_BITS        (16)/* Number of CBS bits per queue id */
#define CIR_INCR_NUM_BITS   (8) /* Number of CIS_INCR bits per queue id */
#define REF_CNT_NUM_BITS    (8) /* Number of refresh cnt bits per queue id */
#define MAX_BONUS_NUM_BITS  (4) /* Number of max bonus bits per queue id
                                   Actually 3-bits but 4th is reserved */

#define MAX_WLU_SRCPID_NUM                    256
#define MAX_WLU_SRCPID_REG_NUM                8
#define WLU_SRCPID_TO_REG_OFFSET(srcpid)      ((srcpid)>>5)
#define WLU_SRCPID_TO_REG_BIT(srcpid)         ((srcpid)%32)
   uint32_t wlu_srcpid[MAX_WLU_SRCPID_REG_NUM];     /* 0xd8 */
   uint32_t qos_reg[2];        /* 0xf8 */
}CoherencyPortCfgReg_t;

#include "bcm_ubus_chip.h"

extern ubus_t *ubus_sys;
extern ubus_t *ubus_xrdp;
extern ub_mst_addr_map_t ub_mst_addr_map_tbl[];
extern ubus_credit_cfg_t ubus_credit_tbl[UBUS_NUM_OF_MST_PORTS][UBUS_MAX_PORT_NUM+2];

static inline char *ubus_port_id_to_name(int port)
{
    int i;

    for (i = 0; ub_mst_addr_map_tbl[i].port_id != -1 && ub_mst_addr_map_tbl[i].port_id != port; i++);
    
    return ub_mst_addr_map_tbl[i].port_id == -1 ? NULL : ub_mst_addr_map_tbl[i].str;
}

#endif
