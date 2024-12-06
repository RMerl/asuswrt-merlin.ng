/*
    Copyright 2000-2011 Broadcom Corporation

    <:label-BRCM:2011:DUAL/GPL:standard
    
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

/***********************************************************************/
/*                                                                     */
/*   MODULE:  bcmnet.h                                                 */
/*   DATE:    05/16/02                                                 */
/*   PURPOSE: network interface ioctl definition                       */
/*                                                                     */
/***********************************************************************/
#ifndef _IF_NET_H_
#define _IF_NET_H_

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__GLIBC__)
/*
 * FIXME: SIOCGSTAMP is in sys/ioctl.h in musl. this is a mixed-used
 * kernel/userspace header. need to tidy it up.
 */
#undef SIOCGSTAMP
#undef SIOCGSTAMPNS
#endif
#include <linux/sockios.h>
#if defined(CONFIG_COMPAT)
#include <linux/compat.h>
#endif
#include "bcmPktDma_defines.h"
#include "bcmtypes.h"

#define LINKSTATE_DOWN      0
#define LINKSTATE_UP        1

#define MAX_SYS_ETH_PORT 64
/*---------------------------------------------------------------------*/
/* Ethernet Switch Type                                                */
/*---------------------------------------------------------------------*/
#define ESW_TYPE_UNDEFINED                  0
#define ESW_TYPE_BCM5325M                   1
#define ESW_TYPE_BCM5325E                   2
#define ESW_TYPE_BCM5325F                   3
#define ESW_TYPE_BCM53101                   4

#define ETHERNET_ROOT_DEVICE_NAME     "bcmsw"

/*
 * Ioctl definitions.
 */
/* Note1: The maximum device private ioctls allowed are 16 */
/* Note2: SIOCDEVPRIVATE is reserved */
enum {
    SIOCGLINKSTATE = SIOCDEVPRIVATE + 1,
    SIOCSCLEARMIBCNTR,
    SIOCMIBINFO,
    SIOCGQUERYNUMPORTS,
    SIOCPORTMIRROR,  /* set port mirror */
    SIOCGPORTMIRROR, /* get port mirror */
    SIOCSWANPORT,
    SIOCGWANPORT,
    SIOCETHCTLOPS,
    SIOCETHSWCTLOPS,
    SIOCGSWITCHPORT,
    SIOCIFREQ_EXT,
    SIOCLAST,
};

/* Various operations through the SIOCETHCTLOPS */
enum {
    ETHGETNUMTXDMACHANNELS = 0,
    ETHSETNUMTXDMACHANNELS,
    ETHGETNUMRXDMACHANNELS,
    ETHSETNUMRXDMACHANNELS,   
    ETHGETSOFTWARESTATS,
    ETHGETMIIREG,
    ETHSETMIIREG,
    ETHSETLINKSTATE,
    ETHGETCOREID,
    ETHGETPHYEEE,
    ETHSETPHYEEEON,
    ETHSETPHYEEEOFF,
    ETHGETPHYEEERESOLUTION,
    ETHPHYWAKEENABLE,
    ETHGETPHYAPD,
    ETHSETPHYAPDON,
    ETHSETPHYAPDOFF,
    ETHGETPHYPWR,
    ETHSETPHYPWRON,
    ETHSETPHYPWROFF,
    ETHMOVESUBPORT,
    ETHPHYMAP,
    ETHGETPHYID,
    ETHG9991CARRIERON,
    ETHG9991CARRIEROFF,
    ETHCDCTL,
    ETHGETSFPINFO,
    ETHPHYMACSEC,
    ETHWIRESPEEDGET,
    ETHWIRESPEEDSET,
    ETHCTLTXFIR,
};

#pragma pack(push, 4)
typedef uint32 uint32_t;
#define PHY_TXFIR \
    uint32_t speed_mbps; \
    uint32_t pre, main, post1, post2, hpf; \
    union { \
        struct { \
            uint32_t pre_reg, main_reg, post1_reg, post2_reg, hpf_reg; \
        }; \
        struct { \
            uint32_t pre_def, main_def, post1_def, post2_def, hpf_def; \
        }; \
    }

/* User-Kernal Space match enforced */
#define PHY_TXFIR_SPEED_MBPS 1000, 2500, 5000, 10000
/* User-Kernal Space match enforced */
#define PHY_TXFIR_SPEED_STRING_ARRAY "1G", "2.5G", "5G", "10G"
/* User-Kernal Space match enforced */
#define PHY_TXFIR_SET_DEFAULT(txfir)    (((txfir)->pre + (txfir)->main + (txfir)->post1 + (txfir)->post2) == 0)

typedef struct txfir_s {
    PHY_TXFIR;
} txfir_t;
    

/* Ethcd operation code */
#define ETHCD_SET   1
#define ETHCD_GET   2
#define ETHCD_RUN   3
#define ETHCD_QUERY 4

/* Return Value */
#define ETHCD_AUTORUN_DISABLED  0
#define ETHCD_AUTORUN_ENABLED   1
#define ETHCD_OK                2
#define ETHCD_INVALID           -1
#define ETHCD_NOT_SUPPORTED     -2
#define ETHCD_FAILED            -3
#define ETHCD_NO_NEXT_IF        -4

/* for cable_code */
#define ETHCD_STATUS_INVALID        -1
#define ETHCD_STATUS_GOOD_CONNECTED 1
#define ETHCD_STATUS_GOOD_OPEN      2
#define ETHCD_STATUS_NO_CABLE       3
#define ETHCD_STATUS_BAD_OPEN       4
#define ETHCD_STATUS_MIXED_BAD      5

/* Pair Code */
#define ETHCD_PAIR_UNKNOWN      -1
#define ETHCD_PAIR_OK           0
#define ETHCD_PAIR_OPEN         1
#define ETHCD_PAIR_INTRA_SHORT  2
#define ETHCD_PAIR_INTER_SHORT  3

/* Cable Pair Color */
#define ETHCD_PAIR_BROWN    0
#define ETHCD_PAIR_BLUE     1
#define ETHCD_PAIR_GREEN    2
#define ETHCD_PAIR_ORANGE   3

/* Flags for Cable Diagnostic */
#define ETHCD_FLAG_INTERFACE_NEXT    (1<<0)
#define ETHCD_FLAG_AUTORUN_ENABLED   (1<<1)
#define ETHCD_FLAG_DATA_VALID        (1<<2)

typedef struct {
    int op;
    int value;
    int return_value;
    int phy_addr;
    int flag;
    int link;
    int cable_code; /* Driver interpretation from pair code etc. */
    int pair_len_cm[4];
    int pair_code[4];
    uint64 time_stamp;
} ethcd_t; /* Enforce USER_SPACE-KERNEL matching */

struct ethctl_data {
    /* ethctl ioctl operation */
    int op;
    /* number of DMA channels */
    int num_channels;
    /* return value for get operations */
    int ret_val;
    int ret_val2;
    /* value for set operations */
    int val;
    int val2;
    int sub_port;
#define ETHCTL_SUBPORT_CASCADE_ENDPHY   (1<<31)
#define ETHCTL_SUBPORT_GET_SUBPORT(s) ((s) & ~ETHCTL_SUBPORT_CASCADE_ENDPHY )
    unsigned int phy_addr;
    unsigned int phy_reg;
    int short_tmr_ms;
    int long_tmr_ms;
    int short_tmr_weight;
    int long_tmr_weight;
    /* flags to indicate to ioctl functions */
    unsigned int flags;
    char ifname[OBJIFNAMSIZ];
    BCM_IOC_PTR(char *, buf);
    int buf_size;
    ethcd_t ethcd;
#define ETHCTL_FLAG_WAKE_MAC_SET    (1<<0)
#define ETHCTL_FLAG_WAKE_PSW_SET    (1<<1)
#define ETHCTL_FLAG_WAKE_REP_SET    (1<<2)
#define ETHCTL_FLAG_WAKE_INT_SET    (1<<3)
#define ETHCTL_FLAG_WAKE_MPD_SET    (1<<4)
#define ETHCTL_FLAG_WAKE_ARD_SET    (1<<5)
#define ETHCTL_FLAG_WAKE_LNK_SET    (1<<6)
#define ETHCTL_FLAG_WAKE_SPD_SET    (1<<7)
#define ETHCTL_FLAG_WAKE_BTN_SET    (1<<8)
#define ETHCTL_FLAG_WAKE_TME_SET    (1<<9)
#define ETHCTL_FLAG_WAKE_WAN_SET    (1<<10)
    char mac[6];
    char psw[6];
    int rep;
    int spd;
    txfir_t txfir;
};

#define ETHCTL_VAL_SET_FLAG ((~(-1))|1<<31)

typedef enum {
    SFP_TYPE_XPON,
    SFP_TYPE_ETHERNET,
    SFP_TYPE_UNKNOWN,
    SFP_TYPE_NOT_ETHERNET,  /* Active Ethernet Port not defined in board paramters */
    SFP_TYPE_NO_MODULE,
} sfp_type_t;

/* ethctl ret_val definitions */
enum {
    ETHCTL_RET_OK   = 0,
    ETHMOVESUBPORT_RET_INVALID_EP,
    ETHMOVESUBPORT_RET_SRCDEV_UP,
    ETHMOVESUBPORT_RET_DSTDEV_UP,
    ETHMOVESUBPORT_RET_MAC2MAC,
    ETHMOVESUBPORT_RET_NOT_MOVEABLE,
};

/* PHY type */
enum {
    ETHCTL_FLAG_ACCESS_INT_PHY              = 0,
    ETHCTL_FLAG_ACCESS_EXT_PHY              = (1<<0),
    ETHCTL_FLAG_ACCESS_EXTSW_PHY            = (1<<1),
    ETHCTL_FLAG_ACCESS_I2C_PHY              = (1<<2),
    ETHCTL_FLAG_ACCESS_SERDES               = (1<<3),
    ETHCTL_FLAG_ACCESS_SERDES_POWER_MODE    = (1<<4),
    ETHCTL_FLAG_ACCESS_32BIT                = (1<<5),
    ETHCTL_FLAG_ACCESS_10GSERDES            = (1<<6),
    ETHCTL_FLAG_ACCESS_10GPCS               = (1<<7),
    ETHCTL_FLAG_ACCESS_SILENT_START         = (1<<8),
    ETHCTL_FLAG_ACCESS_SERDES_TIMER         = (1<<9),
};
#define ETHCTL_FLAG_ANY_SERDES  (ETHCTL_FLAG_ACCESS_SERDES|ETHCTL_FLAG_ACCESS_10GSERDES)

enum {
    ETHCTL_SET          = (1<<0),
};

enum ethctl_error {
    ETHCTL_ERROR_POWER_SAVING_DOWN = -100,
    ETHCTL_ERROR_POWER_ADMIN_DOWN = -101,
};

#if 1	/* used by router/shared/ */
#define ETHCTL_FLAG_SHIFT_IN_PHYID          16
#define ETHCTL_FLAG_MASK_IN_PHYID           0xffff
#define ETHCTL_GET_FLAG_FROM_PHYID(phyId)   ((phyId >> ETHCTL_FLAG_SHIFT_IN_PHYID) & ETHCTL_FLAG_MASK_IN_PHYID)
#define ETHCTL_PHY_UID_SHIFT_IN_VAL_OUT     8
#define ETHCTL_PHY_UID_MASK                 0x1f
#define ETHCTL_GET_PHY_UID_FROM_VAL_OUT(v)      ((v >> ETHCTL_PHY_UID_SHIFT_IN_VAL_OUT) & ETHCTL_PHY_UID_MASK)
#define ETHCTL_GET_PHY_UID_FROM_PHYID(v)        ((v >> (ETHCTL_FLAG_SHIFT_IN_PHYID + ETHCTL_PHY_UID_SHIFT_IN_VAL_OUT)) & ETHCTL_PHY_UID_MASK)

#define MII_IOCTL_2_PHYID(mii_ioctl_data) (((mii_ioctl_data->val_out & ETHCTL_FLAG_MASK_IN_PHYID) \
                    << ETHCTL_FLAG_SHIFT_IN_PHYID) | (mii_ioctl_data->phy_id))
#define PHYID_2_MII_IOCTL(phy_id, mii_ioctl_data) \
                    {mii_ioctl_data->val_out = (phy_id >> ETHCTL_FLAG_SHIFT_IN_PHYID) & ETHCTL_FLAG_MASK_IN_PHYID; \
                    mii_ioctl_data->phy_id = phy_id & BCM_PHY_ID_M;}
#endif


enum {
    SERDES_NO_POWER_SAVING, 
    SERDES_BASIC_POWER_SAVING, 
    SERDES_ADVANCED_POWER_SAVING, 
    SERDES_FORCE_OFF,
    SERDES_POWER_MODE_MAX};

struct interface_data{
    char ifname[IFNAMSIZ];
    int switch_port_id;
};

/* Definition for IFREQ extension structure to
   support more IFREQs than kernel allocated 16 types */
struct ifreq_ext {
    int opcode;
    /* add union struct for different opcode's data below if needed */
    union 
    {
        struct {
            BCM_IOC_PTR(char *, stringBuf);
            int bufLen;
            int count;
        };
    };
};
typedef struct ifreq_ext ifreq_ext_t;

#pragma pack(pop)
/*! 
\addtogroup ethswctl
@{ 
*/
/*!
\brief      Various ethswctl operations through SIOCIFREQ_EXT.\n
			Definitions are in bcmnet.h
*/
/* Definition for opcode */
enum
{
    SIOCGPORTWANONLY,           //!< CLI- ::CMD_getwanonly              \n API- bcm_enet_driver_getWANOnlyEthPortIfNameList()
    SIOCGPORTWANPREFERRED,      //!< CLI- ::CMD_getlanwan               \n API- bcm_enet_driver_getWanPreferredPortIfNameList()
    SIOCGPORTLANONLY,           //!< CLI- ::CMD_getlanonly              \n API- bcm_enet_driver_getLANOnlyEthPortIfNameList()
    SIOCGPORTLANWAN,            //!< CLI- ::CMD_getlanwan               \n API- bcm_enet_driver_getLANWANEthPortIfNameList()
    SIOCGPORTLANALL,            //!< CLI- ::CMD_getlanall               \n API- bcm_enet_driver_getLANEthPortIfNameList()
};
/*! @} end of ethswctl group */

#define SPEED_10MBIT        10000000
#define SPEED_100MBIT       100000000
#define SPEED_200MBIT       200000000
#define SPEED_1000MBIT      1000000000
#define SPEED_2500MBIT      2500000000u
#define SPEED_5000MBIT      5000000000ull
#define SPEED_10000MBIT     10000000000ull
#define SPEED_DOWN          0

#define BCMNET_DUPLEX_HALF         0
#define BCMNET_DUPLEX_FULL         1

// Use for Auto negotiation capability
#define AN_10M_HALF           0x0001
#define AN_10M_FULL           0x0002
#define AN_100M_HALF          0x0004
#define AN_100M_FULL          0x0008
#define AN_1000M_HALF         0x0010
#define AN_1000M_FULL         0x0020
#define AN_2500               0x0040
#define AN_5000               0x0080
#define AN_10000              0x0100
#define AN_AUTONEG            0x0200
#define AN_FLOW_CONTROL       0x0400
#define AN_FLOW_CONTROL_ASYM  0x0800
#define AN_REPEATER           0x1000

#define AUTONEG_CTRL_MASK 0x01
#define AUTONEG_RESTART_MASK 0x02


typedef struct IoctlMibInfo
{
    uint32 ulIfLastChange;
    uint64 ulIfSpeed;
    uint32 ulIfDuplex;
} IOCTL_MIB_INFO, *PIOCTL_MIB_INFO;


#define MIRROR_INTF_SIZE    32
#define MIRROR_DIR_IN       0
#define MIRROR_DIR_OUT      1
#define MIRROR_DISABLED     0
#define MIRROR_ENABLED      1

typedef struct _MirrorCfg
{
    char szMonitorInterface[MIRROR_INTF_SIZE];
    char szMirrorInterface[MIRROR_INTF_SIZE];
    int nDirection;
    int nStatus;
#if defined(DMP_X_ITU_ORG_GPON_1) && defined(CONFIG_BCM_MAX_GEM_PORTS)
    /* +1 is when CONFIG_BCM_MAX_GEM_PORTS is not a multiple of 8 */
    unsigned char nGemPortMaskArray[(CONFIG_BCM_MAX_GEM_PORTS/8)+1]; 
#endif
} MirrorCfg ;

int sfp_i2c_phy_read( int reg, int *data);
int sfp_i2c_phy_write( int reg, int data);

int bcmeapi_init_wan(void);

/* VLAN TPIDs that need to be checked
   ETH_P_8021Q  0x8100
   ETH_P_8021AD 0x88A8
   ETH_P_QINQ1  0x9100
   ETH_P_QINQ2  0x9200
 */
#define BCM_VLAN_TPID_CHECK(x) ( (x) == htons(ETH_P_8021Q) \
                                || (x) == htons(ETH_P_8021AD)  \
                             /* || (x) == htons(ETH_P_QINQ1) */\
                             /* || (x) == htons(ETH_P_QINQ2) */)

#define check_arp_lcp_pkt(pkt_p, ret_val)   {                                                                       \
            unsigned char l3_offset = sizeof(struct ethhdr);                                                        \
            struct vlan_hdr    *vlanhdr = (struct vlan_hdr *)(pkt_p + l3_offset - sizeof(struct vlan_hdr));         \
            ret_val = 0;                                                                                            \
            /* Skip over all the VLAN Tags */                                                                       \
            while ( BCM_VLAN_TPID_CHECK(vlanhdr->h_vlan_encapsulated_proto) )                                       \
            {                                                                                                       \
                vlanhdr = (struct vlan_hdr *)(pkt_p + l3_offset);                                                   \
                l3_offset +=  sizeof(struct vlan_hdr);                                                              \
            }                                                                                                       \
            if (vlanhdr->h_vlan_encapsulated_proto == htons(ETH_P_ARP))                                             \
            {                                                                                                       \
                ret_val = 1;                                                                                        \
            }                                                                                                       \
            else if ( vlanhdr->h_vlan_encapsulated_proto == htons(ETH_P_PPP_DISC) )                                 \
            {                                                                                                       \
                ret_val = 1;                                                                                        \
            }                                                                                                       \
            else if ( vlanhdr->h_vlan_encapsulated_proto == htons(ETH_P_PPP_SES) )                                  \
            {                                                                                                       \
                struct pppoe_hdr *pppoe = (struct pppoe_hdr *)(pkt_p + l3_offset);                                  \
                                                                                                                    \
                if ( ! (pppoe->tag[0].tag_type  == htons(PPP_IP) || pppoe->tag[0].tag_type  == htons(PPP_IPV6)) )   \
                {                                                                                                   \
                    ret_val = 1;                                                                                    \
                }                                                                                                   \
            }                                                                                                       \
    }

enum Bcm63xxEnetStats {
    ET_TX_BYTES = 0,
    ET_TX_PACKETS,
    ET_TX_ERRORS,
    ET_TX_CAPACITY,
    ET_RX_BYTES,
    ET_RX_PACKETS, 
    ET_RX_ERRORS,
#ifdef CONFIG_BCM_XDP
    ET_RX_XDP_PACKETS,
    ET_RX_XDP_BYTES,
    ET_RX_XDP_REDIRECT,
    ET_RX_XDP_DROPS,
    ET_RX_XDP_PASS,
    ET_TX_XDP,
    ET_TX_XDP_ERR,
    ET_TX_XDP_XMIT,
    ET_TX_XDP_XMIT_ERR,
#endif
    ET_MAX
};

typedef struct mac_limit_arg{
    uint32 cmd;
    uint32 val;
    union {
        void *mac_limit;
        char rsvd[8];
    };
}mac_limit_arg_t;

enum mac_limit_cmd{
    MAC_LIMIT_IOCTL_GET = 0,
    MAC_LIMIT_IOCTL_SET,
    MAC_LIMIT_IOCTL_CLR,
    MAC_LIMIT_IOCTL_EN
};

enum mac_limit_set_op{
    MAC_LIMIT_SET_MAX = 0,
    MAC_LIMIT_SET_MIN,
};

/*------------------------------------------------------------------------*/
/* BCM net character device for ioctl to get/set netdev BRCM private info */
/*------------------------------------------------------------------------*/
#define BCMNET_DRV_NAME             "bcmnet"
#define BCMNET_DRV_DEVICE_NAME      "/dev/" BCMNET_DRV_NAME

typedef enum bcmnet_ioctl_cmd
{
    BCMNET_IOCTL_GET_EXT_FLAGS,
    BCMNET_IOCTL_GET_LAST_CHANGE,
    BCMNET_IOCTL_CLR_STATS,
    BCMNET_IOCTL_ADD_NETDEV_PATH,
    BCMNET_IOCTL_MAC_LIMIT,
    BCMNET_IOCTL_SET_ACCEL_GDX,
    BCMNET_IOCTL_SET_ACCEL_TC_EGRESS,
    BCMNET_IOCTL_SET_ACCEL_FC_TX_THREAD,
    BCMNET_IOCTL_GET_ACCEL,
    BCMNET_IOCTL_DUMP_NETDEV_DEVID,
    BCMNET_IOCTL_SET_SDN_IGNORE,
    BCMNET_IOCTL_MAX
} bcmnet_ioctl_cmd_t;

typedef struct {
    unsigned int unused : 24;
    unsigned int is_bcm_dev : 1;
    unsigned int is_wlan : 1;
    unsigned int is_hw_switch : 1;
    unsigned int is_hw_fdb : 1;
    unsigned int is_ppp : 1;
    unsigned int is_vlan : 1;
    unsigned int is_wan : 1;
    unsigned int is_sdn : 1;
} bcmnet_extflags;

/* */
typedef struct {
    uint32 unused:26;
    uint32 fc_tx_thread:1;
    uint32 tc_egress:1;
    uint32 gdx_debug:1;
    uint32 gdx_hw:1;
    uint32 gdx_tx:1;
    uint32 gdx_rx:1;
}bcmnet_accel_t;

typedef struct {
    char if_name[IFNAMSIZ];
    union {
        struct {
            bcmnet_extflags ret_val;
        }st_get_ext_flags;
        bcmnet_accel_t accel;
        struct {
            unsigned long last_change;
        }st_get_last_change;
        struct {
            char next_if_name[IFNAMSIZ];
        }st_add_netdev_path;
        mac_limit_arg_t st_mac_limit;
    };
}bcmnet_info_t;

#if defined(CONFIG_COMPAT)
typedef struct mac_limit_compat_arg{
    uint32 cmd;
    uint32 val;
    union {
        compat_uptr_t mac_limit;
        char rsvd[8];
    };
}compat_mac_limit_arg_t;

typedef struct {
    char if_name[IFNAMSIZ];
    union {
        struct {
            bcmnet_extflags ret_val;
        }st_get_ext_flags;
        struct {
            uint32 unused:26;
            uint32 fc_tx_thread:1;
            uint32 tc_egress:1;
            uint32 gdx_debug:1;
            uint32 gdx_hw:1;
            uint32 gdx_tx:1;
            uint32 gdx_rx:1;
        }accel;
        struct {
            unsigned long last_change;
        }st_get_last_change;
        struct {
            char next_if_name[IFNAMSIZ];
        }st_add_netdev_path;
        compat_mac_limit_arg_t st_mac_limit;
    };
}compat_bcmnet_info_t;
#endif

#if !defined(__KERNEL__)
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h> 

// return 0 if successful, -1 - error
static inline int bcmnet_ioctl_get_ext_flags(char const* if_name, bcmnet_extflags* flags)
{
    bcmnet_info_t info;
    int fd, err;

    fd = open(BCMNET_DRV_DEVICE_NAME, O_RDWR);
    if (fd < 0)
        return -1;

    strncpy(info.if_name, if_name, sizeof(info.if_name)-1);
    info.if_name[sizeof(info.if_name)-1] = '\0';
    
    err = ioctl(fd, BCMNET_IOCTL_GET_EXT_FLAGS, &info);
    close(fd);
    if (err == -1)
        return err;
    *flags = info.st_get_ext_flags.ret_val;
    return 0;
}

// return 1 - dev is WAN, 0 - dev is not WAN, -1 - error
static inline int bcmnet_ioctl_iswandev(char const* if_name)
{
    bcmnet_extflags flags;
    int ret;

    ret = bcmnet_ioctl_get_ext_flags(if_name, &flags);
    if (ret < 0)
        return -1;

    return flags.is_wan;
}

// return 1 - dev is VLAN, 0 - dev is not VLAN, -1 - error
static inline int bcmnet_ioctl_isvlandev(char const* if_name)
{
    bcmnet_extflags flags;
    int ret;

    ret = bcmnet_ioctl_get_ext_flags(if_name, &flags);
    if (ret < 0)
        return -1;

    return flags.is_vlan;
}

// return last transction jiffies, 0 if there is error
static inline unsigned long bcmnet_ioctl_get_last_change(char* if_name)
{
    bcmnet_info_t info;
    int fd, err;

    fd = open(BCMNET_DRV_DEVICE_NAME, O_RDWR);
    if (fd < 0)
        return 0;

    strncpy(info.if_name, if_name, IFNAMSIZ);
    err = ioctl(fd, BCMNET_IOCTL_GET_LAST_CHANGE, &info);
    close(fd);
    if (err == -1)
        return 0;
    return info.st_get_last_change.last_change;
}

static inline void bcmnet_ioctl_clr_stat(char* if_name)
{
    bcmnet_info_t info;
    int fd, err;

    fd = open(BCMNET_DRV_DEVICE_NAME, O_RDWR);
    if (fd < 0)
        return;

    strncpy(info.if_name, if_name, sizeof(info.if_name)-1);
    info.if_name[sizeof(info.if_name)-1] = '\0';
    err = ioctl(fd, BCMNET_IOCTL_CLR_STATS, &info);
    if (err == -1)
        perror("ioctl(BCMNET_IOCTL_CLR_STATS)");
    close(fd);
}

static inline int bcmnet_ioctl_add_netdev_path(char const* dev_name, char const* next_dev_name)
{
    bcmnet_info_t info;
    int fd, err;

    fd = open(BCMNET_DRV_DEVICE_NAME, O_RDWR);
    if (fd < 0)
        return -1;

    strncpy(info.if_name, dev_name, sizeof(info.if_name)-1);
    info.if_name[sizeof(info.if_name)-1] = '\0';
    strncpy(info.st_add_netdev_path.next_if_name, next_dev_name, sizeof(info.st_add_netdev_path.next_if_name)-1);
    info.st_add_netdev_path.next_if_name[sizeof(info.st_add_netdev_path.next_if_name)-1] = '\0';
    err = ioctl(fd, BCMNET_IOCTL_ADD_NETDEV_PATH, &info);
    close(fd);
    return err;
}

#endif //!__KERNEL__

#ifdef __cplusplus
}
#endif

#endif /* _IF_NET_H_ */
