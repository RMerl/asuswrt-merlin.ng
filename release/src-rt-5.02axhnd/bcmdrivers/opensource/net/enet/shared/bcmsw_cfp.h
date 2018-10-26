/*
       <:copyright-BRCM:2016:DUAL/GPL:standard    
       
          Copyright (c) 2016 Broadcom 
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
#ifndef _BCMSW_CFP_H_
#define _BCMSW_CFP_H_

int bcmeapi_ioctl_cfp(struct ethswctl_data *e);

/****************************************************************************
   CFP Registers
****************************************************************************/
#define PAGE_CFP                                0xa0
    #define REG_CFP_ACC                         0
        #define CFPACC_READ_STATUS_M            (0xf << 28)
            #define CFPACC_READ_ST_STATISC_RAM      (0x8 << 28)
            #define CFPACC_READ_ST_RATE_METER_RAM   (0x4 << 28)
            #define CFPACC_READ_ST_ACTION_RAM       (0x2 << 28)
            #define CFPACC_READ_ST_TCAM             (0x1 << 28)
            #define CFPACC_READ_ST_NOT_READY        (0x0 << 28)
        #define REG_CFPACC_SEARCH_STS           (1<<27)
        #define REG_CFPACC_XCESS_ADDR_S         16
        #define REG_CFPACC_XCESS_ADDR_M         (0xff<<REG_CFPACC_XCESS_ADDR_S)
        #define REG_CFPACC_TCAMRESET            (1<<15)
        #define REG_CFPACC_RAM_SEL_M            (0x1f << 10)
            #define CFPACC_RED_STTS_RAM_SEL         (0x18 << 10)
            #define CFPACC_YELLOW_STTS_RAM_SEL      (0x10 << 10)
            #define CFPACC_GREEN_STTS_RAM_SEL       (0x08 << 10)
            #define CFPACC_RATE_METER_RAM_SEL       (0x4 << 10)
            #define CFPACC_ACTION_RAM_SEL           (0x2 << 10)
            #define CFPACC_TCAM_SEL                 (0x1 << 10)
            #define CFPACC_NO_OP                (0x0 << 10)
        #define CFPACC_KEY_0_1_RAW_ENC      (1<<5)
        #define CFPACC_RAM_CLEAR            (1<<4)
        #define CFPACC_OP_SEL_M             (7<<1)
            #define CFPACC_OP_READ          (1<<1)
            #define CFPACC_OP_WRITE         (2<<1)
            #define CFPACC_OP_SEARCH        (4<<1)
        #define CFPACC_OP_START_DONE        (1<<0)

    #define REG_CFP_DATA                        0x10
    #define REG_CFP_MASK                        0x30

    #define REG_CFP_ACT_DATA0               0x50
        #define CFP_NEW_DSCP_IB_S           26
        #define CFP_NEW_DSCP_IB_M           (0x3f<<26)
        #define CFP_CHG_FPMAP_IB_S          24
        #define CFP_CHG_FPMAP_IB_M          (3<<24)
            #define CFP_CHG_FPMAP_IB_RMV_ARL    (1<<24)
            #define CFP_CHG_FPMAP_IB_RPL_ARL    (2<<24)
            #define CFP_CHG_FPMAP_IB_ADD_ARL    (3<<24)
        #define CFP_FPMAP_IB_S              14
        #define CFP_FPMAP_IB_M              (0x3ff<<14)
        #define CFP_CHG_TC                  (1<<13)
        #define CFP_NEW_TC_S                10
        #define CFP_NEW_TC_M                (7<<10)
        #define CFP_NEW_LKBK_EN             (1<<9)
        #define CFP_REASON_CODE_S           3
        #define CFP_REASON_CODE_M           (0x3f<<3)
        #define CFP_STP_BYP                 (1<<2)
        #define CFP_EAP_BYP                 (1<<1)
        #define CFP_VLAN_BYP                (1<<0)

    #define REG_CFP_ACT_DATA1               0x54
        #define CFP_RED_DEFAULT             (1<<31)
        #define CFP_NEW_COLOR_S             29
        #define CFP_NEW_COLOR_M             (3<<29)
        #define CFP_CHG_COLOR               (1<<28)
        #define CFP_CHAIN_ID_S              20
        #define CFP_CHAIN_ID_M              (0xff<<20)
        #define CFP_CHG_DSCP_OB             (1<<19)
        #define CFP_NEW_DSCP_OB_S           13
        #define CFP_NEW_DSCP_OB_M           (0x3f<<13)
        #define CFP_CHG_FPMAP_OB_S          11
        #define CFP_FPMAP_OB_S              1
        #define CFP_FPMAP_OB_M              (0x3ff<<1)
        #define CFP_CHG_DSCP_IB             (1<<0)

    #define REG_CFP_RATE_REGS               7
    #define REG_CFP_RATE_DATA0              0x60
        #define CFP_RATE_POLICER_MODE_S     3
        #define CFP_RATE_POLICER_MODE_M     (3<<3)
        #define CFP_RATE_RFC2698_MODE       (0<<3)
        #define CFP_RATE_RFC4115_MODE       (1<<3)
        #define CFP_RATE_MEF_MODE           (2<<3)
        #define CFP_RATE_DISABLED_MODE       (3<<3)
        #define CFP_RATE_MEF_COUPLING_FLAG  (1<<2)
        #define CFP_RATE_POLICER_RED_DROP   (1<<1)
        #define CFP_RATE_COLOR_WARE_MODE    (1<<0)
    #define REG_CFP_RATE_DATA1              0x64
        #define CFP_RATE_EIR_TK_BKT_M       ((1<<23)-1)
    #define REG_CFP_RATE_DATA2
        #define CFP_RATE_EIR_BKT_SIZE_M     ((1<<20)-1)
    #define REG_CFP_RATE_DATA3
        #define CFP_RATE_EIR_REF_CNT_M      ((1<<19)-1)
    #define REG_CFP_RATE_DATA4
        #define CFP_RATE_TK_BKT_M           ((1<<23)-1)
    #define REG_CFP_RATE_DATA5
        #define CFP_RATE_CIR_BLK_SIZE_M     ((1<<20)-1)
    #define REG_CFP_RATE_DATA6
        #define CFP_RATE_CIR_REF_CNT_M      ((1<<19)-1)

typedef struct cfpRateCtl_s
{
    int policer_mode;
} cfpRateCtl_t;
enum {CFP_REFC2698_MODE, CFP_RFC4115_MODE, CFP_MEF_MODE, CFP_DISABLED_RATE_MODE};

/*
    Define all bit fields in little endian and do the conversion during the R/W
*/
#define CfpTcamCom_m \
    u32 slice_valid         :2; \
    u32 slice_id            :2; \
    u32 reserved2           :4; \
    u32 udf_n_x0            :16; \
    u32 udf_n_x1_l          :8; \
 \
    u32 udf_n_x1_h          :8; \
     u32 udf_n_x2            :16; \
     u32 udf_n_x3_l          :8; \
 \
     u32 udf_n_x3_h          :8; \
     u32 udf_n_x4            :16; \
     u32 udf_n_x5_l          :8; \
 \
     u32 udf_n_x5_h          :8; \
     u32 udf_n_x6            :16; \
     u32 udf_n_x7_l          :8; \
 \
     u32 udf_n_x7_h          :8; \
     u32 udf_n_x8            :16; \
     u32 cvtag_l             :8; \
 \
     u32 cvtag_h             :8; \
     u32 svtag               :16; \
     u32 udf_valid_0_7       :8

/* Common fields definition for three different TCAM types */
typedef struct cfpTcamCom_s {
    CfpTcamCom_m;
} cfpTcamCom_t;

typedef struct cfpIpv4Tcam_s {
    CfpTcamCom_m;

    u32 udf_valid_8         :1;
    u32 reserved            :1;
    u32 pppoe_session       :1;
    u32 ttl_range           :2;
    u32 ip_authentication   :1;
    u32 non_first_fragment  :1;
    u32 ip_fragmentation    :1;

    u32 ip_protocol         :8;
    u32 ip_tos              :8;

    u32 l3_framing          :2;
    u32 l2_framing          :2;
    u32 cvtag_status        :2;
    u32 svtag_status        :2;

#if defined(CONFIG_BCM963158)
#define SPMAP_BITS          9
    u32 spmap               :SPMAP_BITS; 
    u32 reseved             :23;
#else
#define SPMAP_BITS          8
    u32 spmap               :SPMAP_BITS; 
    u32 reseved             :24;
#endif
#define SPMAP_BITS_MASK     ((1<<SPMAP_BITS)-1)
} cfpIpv4Tcam_t;

enum {
    VTAG_ST_NOTAG, 
    VTAG_ST_VID0, 
    VTAG_ST_RESERVED, 
    VTAG_ST_VIDN
};

/* 
   IMP port(P8) and P7 are squeezed into b7 and b6 in HW 
   P8 ->P7, P7->P6
 */
#if defined(CONFIG_BCM963158)
#define SF2_LOG_TO_CHIP_PMAP(pb) (pb)
#define SF2_CHIP_TO_LOG_PMAP(pb) (pb)
#else
#define SF2_LOG_TO_CHIP_PMAP(pb) ((pb&0x3f)|((((pb)&0x80)>0)<<6)|((((pb)&0x100)>0)<<7))
#define SF2_CHIP_TO_LOG_PMAP(pb) ((pb&0x7f)|((((pb)&0x40)>0)<<7)|((((pb)&0x80)>0)<<8))
/* Using 0x7f instead of 0x3f to retain unused physical port 6 ie. bit 6 set in the final result, 
   so that the value will appear all 1s for all ports without a hole in b6 */
#endif


typedef struct cfpNoIpTcam_s {
    CfpTcamCom_m;

    u32 udf_valid_8         :1;
    u32 reserved            :1;
    u32 pppoe_session       :1;
    u32 reserved3           :5;

    u32 etype_sap           :16;

    u32 l3_framing          :2;
    u32 l2_framing          :2;
    u32 cvtag_status        :2;
    u32 svtag_status        :2;

#if defined(CONFIG_BCM963158)
    u32 spmap               :9; 
    u32 reseved             :23;
#else
    u32 spmap               :8; 
    u32 reseved             :24;
#endif
} cfpNoIpTcam_t;

typedef union cfpTcam_u
{
    cfpIpv4Tcam_t ipv4;
    //cfpIpv6Tcam_t;
    cfpNoIpTcam_t noIp;
    //cfpIpv6DTcam_t;
} cfpTcam_t;

#define CFP_MAX_UDF_FIELDS_D        12
#define CFP_MAX_UDF_FIELDS_A_C      9
#define CFP_SLICES      3
#define CFP_L3_FRAME_TYPES 3
typedef union udf_u
{
    struct 
    {
        u8 offset:5; 
        u8 pos:3;
    };
    u8 udf;
} udf_t;

typedef struct udfDsc_s 
{
    udf_t udf;
    u8  refCnt;
    u16 mask;
} udfDsc_t;

typedef struct udfCtl_s {
    udfDsc_t udfDsc[CFP_MAX_UDF_FIELDS_D];
    union {u32 useMap, argOffset;};     /* UDF usage map, or member offset in argument structure for UDF Pattern definition */
    union {u32 maxCnt, memberSize;};    /* UDF maximum count, or member size in agrument strucutre for UDF pattern definition */ 
    u32 usedCnt, l3framing, sliceId;
    union {u32 patIdx, avaCnt;};    /* UDF pattern definition index for UDF pattern definition, or available empty UDF counts */
} udfCtl_t;

#define MAX_CFP_ENTRIES 256
typedef struct cfpTcamCtl_s
{
    udfCtl_t *udfCtl;   /* Pointer to UDF Control Structure */
    u8  sliceId, 
        vidx,   /* User space virtual index based inside each slice ID */
        tidx,   /* Physical TCAM index */
        flag;
    u16 argFlag;    /* Command line argument flag */
} cfpTcamCtl_t;

/*
    CFP control structures describing each port enabling status 
    and reference count by rules.
*/
typedef struct cfpCtl_s
{
    int portRefCnt[BP_MAX_SWITCH_PORTS];
    u16 portEnbMap;
} cfpCtl_t;

enum {L3_FRAMING_IPv4, L3_FRAMING_IPv6, L3_FRAMING_RSV, L3_FRAMING_NON_IP};

enum {
    UDF_START_OF_PACKET = 0,
    UDF_END_OF_L2_HEADER = 2,
    UDF_END_OF_L3_HEADER = 3,
    UDF_POS_NON = 7,
};

#define CFP_UDF_FLAG (CFP_ARG_DA_M|CFP_ARG_SA_M|CFP_ARG_IP_PROTOCOL_M|CFP_ARG_DSCP_M|CFP_ARG_IPSA_M|CFP_ARG_IPDA_M|CFP_ARG_TCPUDP_SPORT_M|CFP_ARG_TCPUDP_DPORT_M)

#define PAGE_CFP_CONFIG     0xA1
    #define CFP_CONTROL     0
    #define CFP_UDF_REG     0x10

/****************************************************************************
   FFE Registers
****************************************************************************/

#define PAGE_FFE                                          0xB4
    #define REG_FFE_RXPORT_CTRL                           0x1C
	    #define FFE_PAD_ENABLE_M                          0x1
	    #define FFE_PAD_ENABLE_S                          8
	    #define FFE_PAD_SIZE_M                            0xFF
	    #define FFE_PAD_SIZE_S                            0

/****************************************************************************
   Shared Defines for bcmsw.c and ethsw.c files
****************************************************************************/
#define TOTAL_SWITCH_PORTS   9
#define PBMAP_ALL            0x1FF
#define PBMAP_MIPS           0x100
#define PBMAP_MIPS_N_GPON    0x180
#define DEFAULT_PBVLAN_MAP   0x1FF
#define PBMAP_MIPS_N_EPON    0x180
#define PBMAP_UNIS           0x7F
#define PBMAP_EPON           0x80
#define MAX_EXT_SWITCH_PORTS 6     // This applies to Legacy external switches
/* ***************************************************
 * Extended Register definitions for Star Fighter2.
 *****************************************************/
#if defined(STAR_FIGHTER2)

#include "bcmmii_xtn.h"
#endif

#endif /* _BCMSW_CFP_H_ */
