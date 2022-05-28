/* 
* <:copyright-BRCM:2011:DUAL/GPL:standard
* 
*    Copyright (c) 2011 Broadcom 
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


#ifndef __BCM_SWAPI_TYPES_H__
#define __BCM_SWAPI_TYPES_H__

#include <bcmtypes.h>

/*
 * BCM API error codes.
 *
 * Note: An error code may be converted to a string by passing the code
 * to bcm_errmsg().
 */
typedef enum bcm_error_e {
  BCM_E_NONE = 0,
  BCM_E_INTERNAL,
  BCM_E_MEMORY,
  BCM_E_UNIT,
  BCM_E_PARAM,
  BCM_E_EMPTY,
  BCM_E_FULL,
  BCM_E_NOT_FOUND,
  BCM_E_EXISTS,
  BCM_E_TIMEOUT,
  BCM_E_BUSY,
  BCM_E_FAIL,
  BCM_E_DISABLED,
  BCM_E_BADID,
  BCM_E_RESOURCE,
  BCM_E_CONFIG,
  BCM_E_UNAVAIL,
  BCM_E_INIT,
  BCM_E_PORT
} bcm_error_t;

typedef int bcm_port_t;

/* bcm_vlan_t */
typedef unsigned short bcm_vlan_t;

/* bcm_vlan_tag_t */
typedef unsigned int bcm_vlan_tag_t;

/* bcm_cos_t */
typedef int bcm_cos_t;

/* bcm_cos_queue_t */
typedef int bcm_cos_queue_t;

/* Various operations through the SIOCETHSWCTLOPS */
enum {
    ETHSWDUMPPAGE = 0,
    ETHSWDUMPIUDMA,
    ETHSWIUDMASPLIT,
    ETHSWDUMPMIB,
    ETHSWSWITCHING,
    ETHSWRXSCHEDULING,
    ETHSWWRRPARAM,
    ETHSWUSEDEFTXQ,
    ETHSWDEFTXQ,
    ETHSWRXRATECFG,
    ETHSWRXRATELIMITCFG, /* 10 */
    ETHSWRXPKTRATECFG,
    ETHSWRXPKTRATELIMITCFG,
    ETHSWCONTROL,
    ETHSWPRIOCONTROL,
    ETHSWPORTTAGREPLACE,
    ETHSWPORTTAGMANGLE,
    ETHSWPORTTAGMANGLEMATCHVID,
    ETHSWPORTTAGSTRIP,
    ETHSWPORTPAUSECAPABILITY,
    ETHSWVLAN,  /* 20 */
    ETHSWGETRXCOUNTERS,
    ETHSWRESETRXCOUNTERS,
    ETHSWPBVLAN,
    ETHSWCOSCONF,
    ETHSWCOSSCHED,
    ETHSWCOSPORTMAP,
    ETHSWCOSRXCHMAP,
    ETHSWCOSTXCHMAP,
    ETHSWCOSTXQSEL,
    ETHSWSTATCLR,  /* 30 */
    ETHSWSTATPORTCLR,
    ETHSWSTATSYNC,
    ETHSWSTATGET,
    ETHSWPORTRXRATE,
    ETHSWPORTTXRATE,
    ETHSWTEST1,
    ETHSWARLACCESS,
    ETHSWPORTDEFTAG,
    ETHSWCOSPRIORITYMETHOD,
    ETHSWREGACCESS, /* 40 */
    ETHSWSPIACCESS,
    ETHSWPSEUDOMDIOACCESS,
    ETHSWINFO,
    ETHSWLINKSTATUS,
    ETHSWPORTTRAFFICCTRL,
    ETHSWPORTLOOPBACK,
    ETHSWPHYMODE,
    ETHSWPKTPAD,
    ETHSWJUMBO,
    ETHSWCOSDSCPPRIOMAP, /* 50 */
    ETHSWKERNELPOLL,
    ETHSWPHYCFG,
    ETHSWEPONUNITOUNICTRL,
    ETHSWDEBUG,
    ETHSWGETIFNAME,
    ETHSWMULTIPORT, 
    ETHSWUNITPORT,
    ETHSWDOSCTRL,
    ETHSWHWSTP,
    ETHSWSOFTSWITCHING, /* 60 */
    ETHSWEMACGET,
    ETHSWEMACCLEAR,
    ETHSWPHYAUTONEG,
    ETHSWPORTSALDAL,
    ETHSWPORTTRANSPARENT,
    ETHSWPORTVLANISOLATION,
    ETHSWACBCONTROL,
    ETHSWPORTMTU,
    ETHSWMIRROR,
    ETHSWPORTTRUNK,	/* 70 */
    ETHSWCPUMETER,
    ETHSWPORTSHAPERCFG,
    ETHSWCOSPCPPRIOMAP,
    ETHSWCOSPIDPRIOMAP,
    ETHSWRDPAPORTGET,
    ETHSWRDPAPORTGETFROMNAME,
    ETHSWEXTPHYLINKSTATUS,
    ETHSWPHYAPD,
    ETHSWPHYEEE,
    ETHSWDEEPGREENMODE, /* 80 */
    ETHSWSTATPORTGET,
    ETHSWPHYAUTONEGCAPADV,
    ETHSWQUEMON,
    ETHSWQUEMAP,
    ETHSWPORTSTORMCTRL,
    ETHSWOAMIDXMAPPING,
    ETHSWBONDINGPORTS,
    ETHSWCFP,

    // add by Andrew
    ETHSWARLDUMP = 201,
    ETHSWMIBDUMP,
    ETHSWPORTDUMP,
};

typedef struct cfpArg_s {
    unsigned int rc;
    unsigned int argFlag;
    unsigned int unit;
    unsigned int priority;
    unsigned int index;
    unsigned int l2_framing;
    unsigned int l3_framing;
    unsigned int chg_fpmap_ib;
    unsigned int fpmap_ib;
    unsigned int spmap;
    unsigned int pppoe;
    unsigned int etype_sap;
    unsigned int etype_sap_mask;
    unsigned int svtag;
    unsigned int svtag_mask;
    unsigned int cvtag;
    unsigned int cvtag_mask;
    unsigned int ip_protocol;
    unsigned int ip_protocol_mask;
    unsigned int ipsa;
    unsigned int ipsa_mask;
    unsigned int ipda;
    unsigned int ipda_mask;
    unsigned int tcpudp_sport;
    unsigned int tcpudp_sport_mask;
    unsigned int tcpudp_dport;
    unsigned int tcpudp_dport_mask;
    unsigned int dscp;
    unsigned int dscp_mask;
    unsigned int new_dscp_ib;
    unsigned int op;
    unsigned long long da;
    unsigned long long da_mask;
    unsigned long long sa;
    unsigned long long sa_mask;
} cfpArg_t;
enum {
    CFP_RC_SUCCESS,
    CFP_RC_NON_EXISTING_INDEX,
    CFP_RC_CFP_FULL,
    CFP_RC_UDF_FULL,
};

enum {CfpL3Ipv4, CfpL3Ipv6, CfpL3NoIP};
enum {CfpL2EtherII, CfpL2SnapPublic, CfpL2LLC, CfpL2SnapPrivate};

enum {
    CFPOP_READ,
    CFPOP_READ_NEXT,
    CFPOP_ADD,
    CFPOP_INSERT,
    CFPOP_APPEND,
    CFPOP_DELETE,
    CFPOP_DELETE_ALL,
};

enum {
    OPT_READ,
    OPT_ADD,
    OPT_APPEND,
    OPT_INSERT,
    OPT_DELETE,
    OPT_DELETE_ALL,

    OPT_PRIORITY,
    OPT_INDEX,
    OPT_SPMAP,
    OPT_DA,

    OPT_SA,
    OPT_SVLAN_VID,
    OPT_SVLAN_PCP,
    OPT_SVLAN_TAG,

    OPT_CVLAN_VID,
    OPT_CVLAN_PCP,
    OPT_CVLAN_TAG,
    OPT_L2,

    OPT_PPPOE,
    OPT_ETYPE,
    OPT_L3,
    OPT_DSCP,

    OPT_IP_PROTOCOL,
    OPT_IPSA,
    OPT_IPDA,
    OPT_TCPUDP_SPORT,

    OPT_TCPUDP_DPORT,
    OPT_NEW_DSCP_IB,
    OPT_CHANGE_FPMAP_IB,
    OPT_FPMAP_IB,
};

enum {
    CFP_ARG_SPMAP_M =    (1<<0),
    CFP_ARG_DA_M  =      (1<<1),
    CFP_ARG_SA_M  =      (1<<2),
    CFP_ARG_IP_PROTOCOL_M =  (1<<3),

    CFP_ARG_L2_FRAMING_M = (1<<4),
    CFP_ARG_L3_FRAMING_M = (1<<5),
    CFP_ARG_DSCP_M = (1<<6),
    CFP_ARG_PRIORITY_M = (1<<7),

    CFP_ARG_INDEX_M = (1<<8),
    CFP_ARG_IPSA_M = (1<<9),
    CFP_ARG_IPDA_M = (1<<10),
    CFP_ARG_TCPUDP_SPORT_M = (1<<11),

    CFP_ARG_TCPUDP_DPORT_M = (1<<12),
    CFP_ARG_NEW_DSCP_IB_M = (1<<13),
    CFP_ARG_CHG_FPMAP_IB_M = (1<<14),
    CFP_ARG_FPMAP_IB_M = (1<<15),

    CFP_ARG_SVLAN_TAG_M = (1<<16),
    CFP_ARG_CVLAN_TAG_M = (1<<17),
    CFP_ARG_PPPOE_M     = (1<<18),
    CFP_ARG_ETYPE_SAP_M = (1<<19),

    CFP_ARG_OP_M =        (1<<20),
};

#define CFP_ARG_MUST_ARGS_M (CFP_ARG_OP_M|CFP_ARG_NEW_DSCP_IB_M|CFP_ARG_CHG_FPMAP_IB_M)
#define CFP_ARG_IP_FLAGS (CFP_ARG_DSCP_M|CFP_ARG_IPSA_M|CFP_ARG_IPDA_M|CFP_ARG_TCPUDP_SPORT_M|CFP_ARG_TCPUDP_DPORT_M)
#define CFP_ARG_NON_IP_FLAGS CFP_ARG_ETYPE_SAP_M

enum {
    CFP_CHG_FPMAP_NO_CHG = 0,
    CFP_CHG_FPMAP_RMV_ARL,
    CFP_CHG_FPMAP_RPL_ARL,
    CFP_CHG_FPMAP_ADD_ARL,
};

#define OPT_UDF_MASK = (CFP_ARG_DA_M | CFP_ARG_SA_M)

#define BMCR_SPEED2500      0x0080  /* Command Parameter definition */

/*************************************************
 *  Flow Control Diagnosis Definitions           *
 *************************************************/
enum {
    QUE_CUR_COUNT,
    QUE_PEAK_COUNT,
    SYS_TOTAL_PEAK_COUNT,
    SYS_TOTAL_USED_COUNT,

    PORT_PEAK_RX_BUFFER,
    QUE_FINAL_CONGESTED_STATUS,
    PORT_PAUSE_HISTORY,
    PORT_PAUSE_QUAN_HISTORY,

    PORT_RX_BASE_PAUSE_HISTORY,
    PORT_RX_BUFFER_ERROR_HISTORY,
    QUE_CONGESTED_STATUS,
    QUE_TOTAL_CONGESTED_STATUS,

    QUE_MON_MAX_TYPES,
};

    
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(CHIP_63138) || defined(CHIP_63148) || defined(CHIP_4908)
#define BCM_COS_COUNT  8
#else
#define BCM_COS_COUNT  4
#endif
enum {
    QOS_SCHED_SP_CAP        = 1 << 0,
    QOS_SCHED_WRR_CAP       = 1 << 1,
    QOS_SCHED_WDR_CAP       = 1 << 2,
    QOS_SCHED_COMBO         = 1 << 3,
    QOS_PORT_SHAPER_CAP     = 1 << 4,
    QOS_QUEUE_SHAPER_CAP    = 1 << 5,
};

/* PHY Configuration Mode value between User Space and driver */
enum phy_cfg_flag {
    PHY_CFG_AUTO_NEGO   = 1 << 0,
    PHY_CFG_10FD        = 1 << 1,
    PHY_CFG_10HD        = 1 << 2,
    PHY_CFG_100FD       = 1 << 3,

    PHY_CFG_100HD       = 1 << 4,
    PHY_CFG_1000FD      = 1 << 5,
    PHY_CFG_1000HD      = 1 << 6,
    PHY_CFG_2500FD      = 1 << 7,
};

#ifndef IFNAMSIZ
#define IFNAMSIZ  16
#endif

/* eth switch mac entry -- add by Andrew 2020/05/04 */
typedef struct ethsw_mac_entry_s {
    unsigned char mac[6];
    unsigned short port;
} ethsw_mac_entry;

#ifndef MAX_MAC_ENTRY
#define MAX_MAC_ENTRY 128
#endif

typedef struct ethsw_mac_table_s {
    unsigned int count;
    unsigned int len;
    ethsw_mac_entry entry[MAX_MAC_ENTRY];
} ethsw_mac_table;

typedef struct ethsw_port_stats_s {
    uint32 txPackets;
    uint64 txBytes;
    uint32 txDrops;
    uint32 rxPackets;
    uint64 rxBytes;
    uint32 rxDrops;
    uint32 rxDiscards;
} ethsw_port_stats;
/* end of add */

struct ethswctl_data
{
    /* NOTE : Common attribute for all messages */
    /* ethswctl ioctl operation */
    int op;

    /* page number */
    int page;

    /* switch unit number */
    int unit;

    /* switch port number */
    int port;

    /* sub_port number, ex. cross_bar port, trunk port */
    int sub_unit, sub_port;

    int addressing_flag;
    #define ETHSW_ADDRESSING_DEV        (1<<0)      /* Linux device valid */
    #define ETHSW_ADDRESSING_SUBPORT    (1<<1)      /* Sub port valid */

    /* Action type */
    int type;

    /* NOTE : Do not add new params to this structure above for
       new command - this is kept for backward compatibility.
       Define a new structure for new command within union below. */
    union
    {
        cfpArg_t cfpArgs;

		ethsw_mac_table mac_table;  // add by Andrew
		ethsw_port_stats port_stats; // add by Andrew

        struct
        {
#define TYPE_SUBSET  0
#define TYPE_ALL     1
#define TYPE_DISABLE 0
#define TYPE_ENABLE  1
#define TYPE_GET     2
#define TYPE_SET     3
#define TYPE_DUMP    4
#define TYPE_FLUSH   5
#define TYPE_CLEAR   6
            int sub_type;
#define SUBTYPE_ISRCFG      0
#define SUBTYPE_RXDUMP      1
#define SUBTYPE_RESETMIB    2
#define SUBTYPE_RESETSWITCH 3
            /* hardware switching enable/disable status */
            int status;
#define STATUS_DISABLED 0
#define STATUS_ENABLED  1
            int vlan_tag;
            int vlan_param;
            int replace_type;
/* Defines for indicating the parameter in tag replace register */
#define  REPLACE_VLAN_TAG    0
#define  REPLACE_VLAN_TPID   1
#define  REPLACE_VLAN_TCI    2
#define  REPLACE_VLAN_VID    3
#define  REPLACE_VLAN_8021P  4
#define  REPLACE_VLAN_CFI  5
            int op_map;
            int ret_val;
            int val;
            int max_pkts_per_iter;
            int weights[BCM_COS_COUNT];
            unsigned char qos_egrq_sched_cap[BCM_COS_COUNT];
            int priority;
            int sw_port_type;
            int pkt_type_mask;
            int sw_ctrl_type;
            /* scheduling value for ETHSETSCHEDULING */
            int scheduling;
            int vid;
            unsigned char mac[6];
            unsigned char data[8];
            int chip_id;
            int spi_id;
            int bus_num;
            int fwd_map;
            int untag_map;
            int queue;
            int channel;
            int numq;
            uint32 limit;
            uint32 burst_size;
            uint64 counter_val;
            unsigned int vendor_id;
            unsigned int dev_id;
            unsigned int rev_id;
            int counter_type;
            unsigned int offset;
            int length;
            int speed;
            int duplex;
            unsigned int port_map;
            unsigned int phy_portmap;
            int phycfg;
            int epon_port;
            char ifname[IFNAMSIZ];
#define AUTONEG_CTRL_MASK 0x01
#define AUTONEG_RESTART_MASK 0x02
            unsigned char autoneg_info;
            unsigned short autoneg_local;
            unsigned short autoneg_ad;
            BCM_IOC_PTR(void *, vptr);
        };
        struct dos_ctrl_params
        {
            unsigned char ip_lan_drop_en;   
            unsigned char tcp_blat_drop_en; 
            unsigned char udp_blat_drop_en; 
            unsigned char tcp_null_scan_drop_en;
            unsigned char tcp_xmas_scan_drop_en;
            unsigned char tcp_synfin_scan_drop_en;
            unsigned char tcp_synerr_drop_en; 
            unsigned char tcp_shorthdr_drop_en;
            unsigned char tcp_fragerr_drop_en; 
            unsigned char icmpv4_frag_drop_en; 
            unsigned char icmpv6_frag_drop_en; 
            unsigned char icmpv4_longping_drop_en;
            unsigned char icmpv6_longping_drop_en;
            unsigned char dos_disable_lrn;
        }dosCtrl;
        struct up_len_s
        {
            BCM_IOC_PTR(void *, uptr);
            unsigned int len;
        }up_len;
        struct port_qos_sched_s
        {
            unsigned short sched_mode;        // configured val  -- SP/WRR
            unsigned short num_spq;           // configured SP q value in Combo mode
            unsigned short wrr_type;          // WRR or WDR?              
            unsigned short weights_upper;     // when setting, upper/lower 4 weights -- useful for CLI
            unsigned short max_egress_spq;    // CAP -  max SP q in Combo mode 
            unsigned short max_egress_q;      // CAP - per port max queues supported 
            unsigned int port_qos_caps;       // CAP - scheduling/shaping capabilities
        } port_qos_sched;

        struct emac_stats
        {
            uint64 rx_byte;              /**< Receive Byte Counter */
            uint64 rx_packet;            /**< Receive Packet Counter */
            uint64 rx_frame_64;          /**< Receive 64 Byte Frame Counter */
            uint64 rx_frame_65_127;      /**< Receive 65 to 127 Byte Frame Counter */
            uint64 rx_frame_128_255;     /**< Receive 128 to 255 Byte Frame Counter */
            uint64 rx_frame_256_511;     /**< Receive 256 to 511 Byte Frame Counter */
            uint64 rx_frame_512_1023;    /**< Receive 512 to 1023 Byte Frame Counter */
            uint64 rx_frame_1024_1518;   /**< Receive 1024 to 1518 Byte Frame Counter */
            uint64 rx_frame_1519_mtu;   /**< Receive 1519 to MTU  Frame Counter */
            uint64 rx_multicast_packet;  /**< Receive Multicast Packet */
            uint64 rx_broadcast_packet;  /**< Receive Broadcast Packet */
            uint64 rx_unicast_packet;    /**< Receive Unicast Packet */
            uint64 rx_alignment_error;   /**< Receive Alignment error */
            uint64 rx_frame_length_error;/**< Receive Frame Length Error Counter */
            uint64 rx_code_error;        /**< Receive Code Error Counter */
            uint64 rx_carrier_sense_error;/**< Receive Carrier sense error */
            uint64 rx_fcs_error;         /**< Receive FCS Error Counter */
            uint64 rx_undersize_packet;  /**< Receive Undersize Packet */
            uint64 rx_oversize_packet;   /**< Receive Oversize Packet */
            uint64 rx_fragments;         /**< Receive Fragments */
            uint64 rx_jabber;            /**< Receive Jabber counter */
            uint64 rx_overflow;          /**< Receive Overflow counter */
            uint64 rx_control_frame;     /**< Receive Control Frame Counter */
            uint64 rx_pause_control_frame;/**< Receive Pause Control Frame */
            uint64 rx_unknown_opcode;    /**< Receive Unknown opcode */

            uint64 tx_byte;              /**< Transmit Byte Counter */
            uint64 tx_packet;            /**< Transmit Packet Counter */
            uint64 tx_frame_64;          /**< Transmit 64 Byte Frame Counter */
            uint64 tx_frame_65_127;      /**< Transmit 65 to 127 Byte Frame Counter */
            uint64 tx_frame_128_255;     /**< Transmit 128 to 255 Byte Frame Counter */
            uint64 tx_frame_256_511;     /**< Transmit 256 to 511 Byte Frame Counter */
            uint64 tx_frame_512_1023;    /**< Transmit 512 to 1023 Byte Frame Counter */
            uint64 tx_frame_1024_1518;   /**< Transmit 1024 to 1518 Byte Frame Counter */
            uint64 tx_frame_1519_mtu;   /**< Transmit 1519 to MTU Frame Counter */
            uint64 tx_fcs_error;         /**< Transmit FCS Error */
            uint64 tx_multicast_packet;  /**< Transmit Multicast Packet */
            uint64 tx_broadcast_packet;  /**< Transmit Broadcast Packet */
            uint64 tx_unicast_packet;    /**< Transmit Unicast Packet */
            uint64 tx_total_collision;   /**< Transmit Total Collision Counter */
            uint64 tx_jabber_frame;      /**< Transmit Jabber Frame */
            uint64 tx_oversize_frame;    /**< Transmit Oversize Frame counter */
            uint64 tx_undersize_frame;   /**< Transmit Undersize Frame */
            uint64 tx_fragments_frame;   /**< Transmit Fragments Frame counter */
            uint64 tx_error;             /**< Transmission errors*/
            uint64 tx_underrun;          /**< Transmission underrun */
            uint64 tx_excessive_collision; /**< Transmit Excessive collision counter */
            uint64 tx_late_collision;    /**< Transmit Late collision counter */
            uint64 tx_single_collision;  /**< Transmit Single collision frame counter */
            uint64 tx_multiple_collision;/**< Transmit Multiple collision frame counter */
            uint64 tx_pause_control_frame; /**< Transmit PAUSE Control Frame */
            uint64 tx_deferral_packet;   /**< Transmit Deferral Packet */
            uint64 tx_excessive_deferral_packet; /**< Transmit Excessive Deferral Packet */
            uint64 tx_control_frame;     /**< Transmit Control Frame */
        }emac_stats_s; 
        unsigned char sal_dal_en;   /*enable/disable sa lookup and da lookup for port*/
        unsigned int mtu; /*denotes the maximum frame size allowed across this interface*/
        unsigned char transparent;/*enable/disable  transparent for port*/
        struct vlan_isolation_cfg
        {
            unsigned char us_enable;
            unsigned char ds_enable;
        }vlan_isolation;
        struct port_mirror_params
        {
            int enable;
            int mirror_port;
            unsigned int ing_pmap;
            unsigned int eg_pmap;
            unsigned int blk_no_mrr;
            int tx_port; /*Optional - if not supplied all tx traffic is mirrored to \"mirror_port\"; Applicable only to Runner*/
            int rx_port; /*Optional - if not supplied all rx traffic is mirrored to \"mirror_port\"; Applicable only to Runner*/
        }port_mirror_cfg;
        struct port_trunk_params
        {
            int enable; /* Read-only */
            unsigned int hash_sel; /* RW; 0=DA+SA, 1=DA, 2=SA */
            unsigned int grp0_pmap; /* Read-only */
            unsigned int grp1_pmap; /* Read-only */
        }port_trunk_cfg;		
        struct cpu_meter_rate_limit_cfg
        {
#define METER_TYPE_BROADCAST 4 /*correspond to rdpa_cpu_rx_reason_bcast*/
#define METER_TYPE_MULTICAST 3 /*correspond to rdpa_cpu_rx_reason_mcast*/
            unsigned int meter_type;
            unsigned int rate_limit;
        }cpu_meter_rate_limit;
        struct mdk_kernel_poll
        {
           unsigned int link_change;
        }mdk_kernel_poll;
        /** Port statistics */
       struct rdpa_port_stats
       {
           uint64 rx_valid_pkt;               /**< Valid Received packets */
           uint64 rx_crc_error_pkt;           /**< Received packets with CRC error */
           uint64 rx_discard_1;               /**< RX discard 1 */
           uint64 rx_discard_2;               /**< RX discard 2  */    
           uint64 bbh_drop_1;                 /**< BBH drop 1  */    
           uint64 bbh_drop_2;                 /**< BBH drop 2 */    
           uint64 bbh_drop_3;                 /**< BBH drop 3 */    
           uint64 rx_discard_max_length;      /**< Packets discarded due to size bigger than MTU  */    
           uint64 rx_discard_min_length;      /**< Packets discarded due to size smaller than 64  */    
           uint64 tx_valid_pkt;               /**< Valid transmitted packets */    
           uint64 tx_discard;                 /**< TX discarded packets (TX FIFO full) */
           uint64 discard_pkt;                /**< Dropped filtered Packets */
           uint64 rx_valid_bytes;
           uint64 tx_valid_bytes;
           uint64 rx_broadcast_pkt;
           uint64 tx_broadcast_pkt;
           uint64 rx_multicast_pkt;
           uint64 tx_multicast_pkt;
        } rdpa_port_stats_s;
        struct oam_idx_str
        {
            #define OAM_MAP_SUB_TYPE_TO_UNIT_PORT 0
            #define OAM_MAP_SUB_TYPE_TO_RDPA_IF   1
            #define OAM_MAP_SUB_TYPE_FROM_UNIT_PORT   2
            int map_sub_type;
            int oam_idx;
            union {
                int rdpa_if;
            };

        }oam_idx_str;
    };  /* Union */
};

/* Defines for set/get of various fields in VLAN TAG */
#define BCM_NET_VLAN_TPID_S        16
#define BCM_NET_VLAN_TPID_M        0xFFFF
#define BCM_NET_VLAN_VID_S         0
#define BCM_NET_VLAN_VID_M       0xFFF
#define BCM_NET_VLAN_TCI_S         0
#define BCM_NET_VLAN_TCI_M       0xFFFF
#define BCM_NET_VLAN_8021P_S       13
#define BCM_NET_VLAN_8021P_M       0x7
#define BCM_NET_VLAN_CFI_S         12
#define BCM_NET_VLAN_CFI_M         0x1

#define VLAN_FWD_MAP_M   0x1FF
#define VLAN_UNTAG_MAP_M 0x1FF
#define VLAN_UNTAG_MAP_S 9
/* Switch flow control modes for sw_ctrl_type */
typedef enum bcm_switch_fc_e {
  bcmSwitchFcMode, // Global/port
  bcmSwitchTxBasedFc,
  bcmSwitchTxQpauseEn,
  bcmSwitchTxQdropEn,
  bcmSwitchTxTotPauseEn,
  bcmSwitchTxTotdropEn,
  bcmSwitchQbasedpauseEn,
  bcmSwitchTxQpauseEnImp0,
  bcmSwitchTxTotPauseEnImp0,
}bcm_switch_fc_t;


/* Switch controls for sw_ctrl_type */
typedef enum bcm_switch_control_e {
  bcmSwitchBufferControl,       /* Enable/Disable Total/TxQ Pause/Drop */
  bcmSwitch8021QControl,          /* Enable/Disable 802.1Q */
  bcmSwitchTotalDropThreshold,    /* Configure Total Drop Threshold */
  bcmSwitchTotalPauseThreshold,   /* Configure Total Pause Threshold */
  bcmSwitchTotalHysteresisThreshold,  /* Configure Total Hysteresis Threshold */
  bcmSwitchTxQHiDropThreshold,    /* Configure TxQ Hi Drop Threshold */
  bcmSwitchTxQHiPauseThreshold,   /* Configure TxQ Hi Pause Threshold */
  bcmSwitchTxQHiHysteresisThreshold,  /* Configure TxQ Hi Hysteresis Threshold */
  bcmSwitchTxQLowDropThreshold,   /* Configure TxQ LOW Drop Threshold */
  bcmSwitchTxQHiReserveThreshold,
  bcmSwitchTxQThresholdConfigMode, /* COnfigure TxQ Configuration Mode */
  bcmSwitchTotalPorts,             /* COnfigure TxQ Configuration Mode */
  bcmSwitchLinkUpLanPorts,          /* COnfigure TxQ Configuration Mode */
  bcmSwitchLinkUpWanPorts,      /* COnfigure TxQ Configuration Mode */
  bcmSwitchMaxStreams,          /* Configure Queue Max Streams */
  bcmSwitch__Count,
} bcm_switch_control_t;

/* Threshold mode */
enum {
    ThreModeMin,
    ThreModeDynamic = ThreModeMin,
    ThreModeStatic,
    ThreModeManual,
    ThreModeTotalCnt, 
    ThreModeMax = ThreModeTotalCnt - 1,
};


/* For bcmSwitchBufferControl */
#define BCM_SWITCH_TXQ_PAUSE     0x1      /* Enable/Disable TXQ DROP. */
#define BCM_SWITCH_TXQ_DROP      0x2      /* Enable/Disable TXQ PAUSE. */
#define BCM_SWITCH_TOTAL_PAUSE     0x4      /* Enable/Disable TOTAL DROP. */
#define BCM_SWITCH_TOTAL_DROP    0x8      /* Enable/Disable TOTAL PAUSE. */

/* Switch acb controls for sw_ctrl_type */
typedef enum bcm_switch_acb_control_e {
    acb_parms_all = 0,
    acb_en,
    acb_eop_delay,
    acb_flush,

    acb_algorithm,
    acb_tot_xon_hyst,
    acb_xon_hyst,
    acb_q_pessimistic_mode,

    acb_q_total_xon_en,
    acb_q_xon_en,
    acb_q_total_xoff_en,
    acb_q_pkt_len,

    acb_q_tot_xoff_thresh,
    acb_q_xoff_thresh,
    acb_q_pkts_in_flight,
} bcm_switch_acb_control_t;

typedef struct acb_queue_config_s {
    unsigned short pessimistic_mode:1;
    unsigned short total_xon_en:1;
    unsigned short xon_en:1;
    unsigned short total_xoff_en:1;
    unsigned short pkt_len;
    unsigned short total_xoff_threshold;
    unsigned short xoff_threshold;
} acb_queue_config_t;

typedef struct {
    unsigned short acb_en;
    unsigned short eop_delay;
    unsigned short flush;
    unsigned short algorithm;
    unsigned short total_xon_hyst;
    unsigned short xon_hyst;
    unsigned short pkts_in_flight;
    acb_queue_config_t acb_queue_config;
} acb_q_params_t;

typedef struct {
    int egress_rate_cfg;
    int egress_burst_sz_cfg;
    int egress_cur_tokens;
    int egress_shaper_flags;
} egress_shaper_stats_t;

#define SHAPER_ENABLE                  0x1
#define SHAPER_RATE_PACKET_MODE        0x2
#define SHAPER_BLOCKING_MODE           0x4 
#define SHAPER_INCLUDE_IFG             0x8 
#define SHAPER_OVF_FLAG                0x10 
#define SHAPER_INPF_FLAG               0x20

/* Defines for the op_map in tag_mangle_set/get */
#define BCM_PORT_REPLACE_TPID    0x8000  /* Replace TPID */
#define BCM_PORT_REPLACE_8021P     0x4000  /* Replace 802.1p bits */
#define BCM_PORT_REPLACE_CFI     0x2000  /* Replace CFI bit */
#define BCM_PORT_REPLACE_VID     0x1000  /* Replace VLAN ID */
#define BCM_PORT_TAG_MANGLE_OP_MAP_M 0xF000  /* Mask for all tag mangling ops */


/*  For scheduling mechanism selection */
#define SP_SCHEDULING  0
#define WRR_SCHEDULING 1
#define FAP_SCHEDULING 2
#define BCM_COSQ_STRICT  0
#define BCM_COSQ_WRR     1
#define BCM_COSQ_COMBO   2
#define BCM_COSQ_SP      0

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(CHIP_63138) || defined(CHIP_63148) || defined(CHIP_4908)
#define NUM_EGRESS_QUEUES  8
#else
#define NUM_EGRESS_QUEUES  4
#endif

#define QOS_PAUSE_DROP_EN_MAP 0xF
#define MAX_PRIORITY_VALUE    7

/* Return Value Definitions */
#define BCM_E_ERROR  1

/* For pause_capability set/get functions */
#define PAUSE_FLOW_CTRL_NONE 0
#define PAUSE_FLOW_CTRL_AUTO 1
#define PAUSE_FLOW_CTRL_BOTH 2
#define PAUSE_FLOW_CTRL_TX   3
#define PAUSE_FLOW_CTRL_RX   4
#define PAUSE_FLOW_CTRL_BCMSWITCH_OFF  5
#define PAUSE_FLOW_CTRL_BCMSWITCH_ON   6
#define SWITCH_PORTS_ALL_PHYS  255

/* For ETHSWCOSTXQSEL */
/* Use Tx BD priority for egress queue selection */
#define USE_TX_BD_PRIORITY  0
/* Use Tx iuDMA channel for egress queue selection */
#define USE_TX_DMA_CHANNEL  1
/* Indicates neither BD nor channel is used for queue selection*/
#define NONE_OF_THE_METHODS 0xFF

#define PORT_QOS      0
#define MAC_QOS       1
#define IEEE8021P_QOS 2
#define DIFFSERV_QOS  3
#define COMBO_QOS     4
#define IMP_PORT_ID   8
#define P4_PORT_ID    4
#define P5_PORT_ID    5
#define TC_ZERO_QOS   4
#define SWAP_TYPE_MASK  (0xff<<24)
#define PORT_RXDISABLE 0x1
#define PORT_TXDISABLE 0x2
#define MAX_EGRESS_SPQ 4

#define MBUS_PCI     0
#define MBUS_SPI     1
#define MBUS_MDIO    2
#define MBUS_UBUS    3
#define MBUS_HS_SPI  4
#define MBUS_MMAP    5
#define MBUS_NONE    -1
#define QOS_ENUM_WRR_PKT                                 1
#define QOS_ENUM_WDRR_PKT                                2

/* Link Handling */
#define ETHSW_LINK_MIGHT_CHANGED    (1<<0)
#define ETHSW_LINK_FORCE_CHECK      (1<<1)

/* DMA Dump */
#define ETHSW_DMA_RX    1
#define ETHSW_DMA_TX    2
#define ETHSW_DMA_GMAC_CHAN 100

#endif /* __BCM_SWAPI_TYPES_H__ */
