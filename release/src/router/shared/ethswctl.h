/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

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
 *
 ************************************************************************/

#ifndef _ETHSWCTL_H_
#define _ETHSWCTL_H_

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

#define ETHSWCTL_VERSION "0.0.2"
#define ETHSWCTL_RELDATE "January 27, 2009"
#define ETHSWCTL_NAME    "ethswctl"
#if 0
static char *version =
ETHSWCTL_NAME ".c:v" ETHSWCTL_VERSION " (" ETHSWCTL_RELDATE ")\n"
"o Pratapa Reddy Vaka (pvaka@broadcom.com).\n";
#endif

#define cpu_to_le64(x) (u64)__cpu_to_le64((u64)(x))
#define le64_to_cpu(x) (u64)__le64_to_cpu((u64)(x))

typedef enum cmds_e {
	REGACCESS,
	PMDIOACCESS,
}ecmd_t;

typedef enum msgs_e {
  HWSWITCHING_MSG = 0,
  MIBDUMP_MSG,
  PAGEDUMP_MSG,
  IUDMADUMP_MSG,
  IUDMASPLIT_MSG,
  SWCTRL_MSG,
  SWPRIOCTRL_MSG,
#if defined(CHIP_63138) || defined(CHIP_63148) || defined(CHIP_4908)
  SWFLOWCTRL_MSG,
  SWQUEMAP_MSG,
  SWQUEMON_MSG,
  SWACB_MSG,
#endif
  RXSCHED_MSG,
  WRRPARAMS_MSG,
  VLAN_MSG,
  PBVLANMAP_MSG,
  GETSTAT_MSG,
  GETSTAT32_MSG,
  PAUSE_MSG,
  COSQ_MSG,
  COSTXQ_MSG,
  BYTERATECFG_MSG,
  BYTERATE_MSG,
  PKTRATECFG_MSG,
  PKTRATE_MSG,
  DEFTXQ_MSG,
  DEFTXQEN_MSG,
  GETRXCNTRS_MSG,
  RESETRXCNTRS_MSG,
  ARL_MSG,
  ARLDUMP_MSG,
  ARLFLUSH_MSG,
  UNTAGGEDPRIO_MSG,
  COSQPRIOMETHOD_MSG,
  COSQSCHED_MSG,
  PORTCTRL_MSG,
  PORTLOOPBACK_MSG,
  PHYMODE_MSG,
  TEST_MSG,
  JUMBO_MSG,
  PID2PRIO_MSG,
  PCP2PRIO_MSG,
  DSCP2PRIO_MSG,
  REGACCESS_MSG,
  SPIACCESS_MSG,
  PSEUDOMDIOACCESS_MSG,
  SWITCHINFO_MSG,
  SETLINKSTATUS_MSG,
  PORTRXRATECTRL_MSG,
  PORTTXRATECTRL_MSG,
#if defined(CHIP_63138) || defined(CHIP_63148) || defined(CHIP_4908)
  PORTTXRATECFG_MSG,
#endif
  DOSCTRLCTRL_MSG,
  SOFTSWITCH_MSG,
  HWSTP_MSG,
  WAN_MSG,
  PORT_MIRROR_MSG,
  PORT_TRUNK_MSG,
#if defined(CHIP_63138) || defined(CHIP_63148) || defined(CHIP_4908)
  PORT_STORM_CTRL_MSG,
#endif
  OAMIDX_MSG,
  MAX_NUM_COMMANDS,
}msgs_t;

/* Defines for swctrl value parameter */
#define SWITCH_BUFFER_CONTROL   0

/* Defines for swprioctrl type parameter */
enum {
#if defined(CHIP_63138) || defined(CHIP_63148) || defined(CHIP_4908)
    TXQ_RSERVE_THRESHOLD,
#else
    TXQ_LO_DROP_THRESHOLD,
#endif
    TXQ_HI_HYST_THRESHOLD,
    TXQ_HI_PAUSE_THRESHOLD,
    TXQ_HI_DROP_THRESHOLD,
    TOTAL_HYST_THRESHOLD,
    TOTAL_PAUSE_THRESHOLD,
    TOTAL_DROP_THRESHOLD,
    TXQ_MAX_TYPES,
    TXQ_THRED_CONFIG_MODE,
    GET_TOTAL_PORTS,
    GET_LINK_UP_LAN_PORTS,
    GET_LINK_UP_WAN_PORTS,
    TXQ_MAX_STREAMS,
};
#define SF2_MAX_QUEUES 8

#if 0
static char *buf_thresh_literals [] = {
#if defined(CHIP_63138) || defined(CHIP_63148) || defined(CHIP_4908)
          "SwitchTxQHiReserveThreshold",
#else
          "SwitchTxQLowDropThreshold",
#endif
          "SwitchTxQHiHysteresisThreshold",
          "SwitchTxQHiPauseThreshold",
          "SwitchTxQHiDropThreshold",
          "SwitchTotalHysteresisThreshold",
          "SwitchTotalPauseThreshold",
          "SwitchTotalDropThreshold",
};
#endif
/* Local functions */
#if 0
static int ethswctl_control_get(int unit, int type, unsigned int *val);
#endif
int ethswctl_flow_control_get(int unit, int type, unsigned int *val);
int ethswctl_flow_control_set(int unit, int port, int type, unsigned int val);
int ethswctl_acb_cfg_get(int unit, int queue, int type);
int ethswctl_control_priority_op(int unit, int port, int queue, int type, unsigned int *val, int *thrMode, int *maxStr, int get);
#define ethswctl_control_priority_get(unit, port, queue, type, val, thrMod, maxStre) \
    ethswctl_control_priority_op(unit, port, queue, val, manMode, maxStr, 1)
#define ethswctl_control_priority_set(unit, port, queue, type, val, thrMode, maxStr) \
    ethswctl_control_priority_op(unit, port, queue, val, manMode, maxStr, 0)
int ethswctl_quemon_op(int unit, int port, int queue, int type, int *val);
int ethswctl_quemap(int unit, int val, int priority);
int ethswctl_wrr_param_get(int unit);
int ethswctl_wrr_param_set(int unit, int type, unsigned int val);

/* Defines for WRR parameter type */
#define WRR_MAX_PKTS_PER_RND    0
#define WRR_CH0_WEIGHT          1
#define WRR_CH1_WEIGHT          2
#define WRR_CH2_WEIGHT          3
#define WRR_CH3_WEIGHT          4

#define ETHSWCTL_DISABLE 0
#define ETHSWCTL_ENABLE  1

#define SF2_IMP0_PORT                       8
#define SF2_WAN_IMP1_PORT                   5

// Defines for JUMBO register control
#define ETHSWCTL_JUMBO_PORT_ALL                       9   // bill
#define ETHSWCTL_JUMBO_PORT_MIPS                      8
#define ETHSWCTL_JUMBO_PORT_GPON                      7
#define ETHSWCTL_JUMBO_PORT_USB                       6
#define ETHSWCTL_JUMBO_PORT_MOCA                      5
#define ETHSWCTL_JUMBO_PORT_GPON_SERDES               4
#define ETHSWCTL_JUMBO_PORT_GMII_2                    3
#define ETHSWCTL_JUMBO_PORT_GMII_1                    2
#define ETHSWCTL_JUMBO_PORT_GPHY_1                    1
#define ETHSWCTL_JUMBO_PORT_GPHY_0                    0

#define ETHSWCTL_JUMBO_PORT_MIPS_MASK                 0x0100
#define ETHSWCTL_JUMBO_PORT_GPON_MASK                 0x0080
#define ETHSWCTL_JUMBO_PORT_USB_MASK                  0x0040
#define ETHSWCTL_JUMBO_PORT_MOCA_MASK                 0x0020
#define ETHSWCTL_JUMBO_PORT_GPON_SERDES_MASK          0x0010
#define ETHSWCTL_JUMBO_PORT_GMII_2_MASK               0x0008
#define ETHSWCTL_JUMBO_PORT_GMII_1_MASK               0x0004
#define ETHSWCTL_JUMBO_PORT_GPHY_1_MASK               0x0002
#define ETHSWCTL_JUMBO_PORT_GPHY_0_MASK               0x0001
#define ETHSWCTL_JUMBO_PORT_MASK_VAL                  0x01FF

#endif  /* _ETHSWCTL_H_ */
