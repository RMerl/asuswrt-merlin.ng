/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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

#ifndef PHY_MACSEC_COMMON_H_
#define PHY_MACSEC_COMMON_H_

#define MACSEC_RULE_NON_CTRL_WORD_COUNT 5

typedef enum 
{
    MACSEC_OPER_RESTART = 0,
    MACSEC_OPER_SET_LOG_LEVEL,
    MACSEC_OPER_INIT,
    MACSEC_OPER_EN_DS,
    MACSEC_OPER_VPORT_ADD,
    MACSEC_OPER_VPORT_REMOVE,
    MACSEC_OPER_SA_ADD,
    MACSEC_OPER_SA_CHAIN,
    MACSEC_OPER_SA_SWITCH,
    MACSEC_OPER_SA_REMOVE,
    MACSEC_OPER_RULE_ADD,
    MACSEC_OPER_RULE_REMOVE,
    MACSEC_OPER_RULE_ENABLE,
    MACSEC_OPER_VPORT_E_STAT_GET,
    MACSEC_OPER_VPORT_I_STAT_GET,
    MACSEC_OPER_TCAM_STAT_GET,
    MACSEC_OPER_RXCAM_STAT_GET,
    MACSEC_OPER_SA_E_STAT_GET,
    MACSEC_OPER_SA_I_STAT_GET,
} macsec_oper_mode;

typedef enum
{
    MACSEC_RET_ERROR = -1,
    MACSEC_RET_OK = 0,
    MACSEC_RET_ALREADY_INIT = 2,
} macsec_ret_t;

/**----------------------------------------------------------------------------
 * macsec_sa_sction_type_t
 *
 * SA action type:\n
 *      bypass,\n
 *      drop,\n
 *      MACsec ingress (do not use for an egress only device),\n
 *      MACsec egress (do not use for an ingress only device),\n
 *      Crypt-Authenticate.
 */
typedef enum
{
    MACSEC_SA_ACTION_BYPASS,                /**< \n */
    MACSEC_SA_ACTION_DROP,                  /**< \n */
    MACSEC_SA_ACTION_INGRESS,               /**< \n */
    MACSEC_SA_ACTION_EGRESS,                /**< \n */
    MACSEC_SA_ACTION_CRYPT_AUTH             /**< \n */
} macsec_sa_sction_type_t;

typedef enum
{
    MACSEC_FRAME_VALIDATE_DISABLE,          /**< \n */
    MACSEC_FRAME_VALIDATE_CHECK,            /**< \n */
    MACSEC_FRAME_VALIDATE_STRICT            /**< \n */
} macsec_validate_frames_t;

/* Device settings */
typedef struct
{
    /* Disable the MACsec crypto-core (EIP-62), */
    /* send the packets around it to minimize latency */
    unsigned int fStaticBypass;

    /* Outbound sequence number threshold value (one global for all SA's) */
    /* When non-0 the device will generate an interrupt to indicate */
    /* the threshold event which can be used to start the re-keying procedure. */
    /* If set to zero then only the sequence number roll-over interrupt */
    /* will be generated. */
    unsigned int SeqNrThreshold;

    /* Outbound sequence number threshold value for 64-bit packet numbering */
    unsigned int SeqNrThreshold64Lo;
    unsigned int SeqNrThreshold64Hi;

    /* Initial processing rules for default/bypass flows (all vPorts) */
    /* true - bypass, false - drop*/
    unsigned int non_control_untagged_bypass;
    unsigned int non_control_tagged_bypass;
    unsigned int non_control_badtag_bypass;
    unsigned int non_control_kay_bypass;
    unsigned int control_untagged_bypass;
    unsigned int control_tagged_bypass;
    unsigned int control_badtag_bypass;
    unsigned int control_kay_bypass;

    /* Mark as control all packets with dst MAC address = 01:80:C2:00:00:0x */ 
    unsigned int fcontrol_mc_group_addr;

} macsec_api_settings_t;


/** SA parameters for Egress action type */
typedef struct
{
    /** true - SA is in use, packets classified for it can be transformed\n
        false - SA not in use, packets classified for it can not be
                transformed */
    unsigned int fsa_inuse;

    /** true - enable frame protection,\n
        false - bypass frame through device */
    unsigned int fprotect_frames;

    /** true - inserts explicit SCI in the packet,\n
        false - use implicit SCI (not transferred) */
    unsigned int finclude_sci;

    /** Egress Association Number */
    unsigned int an;

    /** true - enable ES bit in the generated SecTAG\n
        false - disable ES bit in the generated SecTAG */
    unsigned int fuse_es;

    /** true - enable SCB bit in the generated SecTAG\n
        false - disable SCB bit in the generated SecTAG */
    unsigned int fuse_scb;

    /** true - enable confidentiality protection\n
        false - disable confidentiality protection */
    unsigned int fconf_protect;

    /** true - allow data (non-control) packets.\n
        false - drop data packets.*/
    unsigned int fallow_data_pkts;

    unsigned int seq_num_lo;

} macsec_api_sa_e_t;

/** SA parameters for Ingress action type */
typedef struct
{
    /** true - SA is in use, packets classified for it can be transformed\n
        false - SA not in use, packets classified for it can not be
                transformed */
    unsigned int fsa_inuse;

    /** true - enable replay protection\n
        false - disable replay protection */
    unsigned int freplay_protect;

    /** MACsec frame validation level (tagged). */
    macsec_validate_frames_t validate_frames_tagged;

    /** Association number to which ingress SA applies. */
    unsigned int an;

    /** true - allow tagged packets.\n
        false - drop tagged packets.*/
    unsigned int fallow_tagged;

    /** true - allow untagged packets.\n
        false - drop untagged packets. */
    unsigned int fallow_untagged;

    /** true - enable validate untagged packets.\n
        false - disable validate untagged packets.*/
    unsigned int fvalidate_untagged;

    unsigned int seq_num_lo;

} macsec_api_sa_i_t;

/** SA parameters for Bypass/Drop action type */
typedef struct
{
    /** true - enable statistics counting for the associated SA\n
       false - disable statistics counting for the associated SA */
    unsigned int fsa_inuse;

} macsec_api_sa_bd_t;

/** SA parameters for Crypt-Authenticate action type */
typedef struct
{
    /** true - enable ICV verification\n
        false - disable ICV verification */
    unsigned int ficv_verify;

    /** true - enable confidentiality protection (AES-GCM/CTR operation)\n
        false - disable confidentiality protection (AES-GMAC operation) */
    unsigned int fconf_protect;

} macsec_api_sa_ca_t;

typedef union
{
    macsec_api_sa_e_t egress;
    macsec_api_sa_i_t ingress;
    macsec_api_sa_bd_t bypass_drop;
    macsec_api_sa_ca_t crypt_auth;
} macsec_api_sa_action_t;

typedef struct
{
    unsigned char sci[8];
    unsigned char ssci[4];
    unsigned char salt[12];
    unsigned char key[32];
    unsigned char hkey[16];
    int key_size;
    unsigned int fextended_pn;

    /** SA parameters */
    macsec_api_sa_action_t params;

    /** SA action type, see macsec_sa_sction_type_t */
    macsec_sa_sction_type_t action_type;

} macsec_api_sa_t;

typedef struct
{
    /** Bit 0 = 1 : No (ST)VLAN tags\n
    Bit 1 = 1 : 1 (ST)VLAN tag\n
    Bit 2 = 1 : 2 VLAN tags\n
    Bit 3 = 1 : 3 VLAN tags\n
    Bit 4 = 1 : 4 VLAN tags\n
    Bit 5 = 1 : 5 Reserved\n
    Bit 6 = 1 : >4 VLAN tags */
    unsigned int num_tags; /**< bit mask, only 7 bits [6:0] are used, see above how */

    /** Data[0] : MAC Destination Address least significant bytes (3..0)\n
    Data[1] : MAC Destination Address most significant bytes (5, 4)\n
    Data[1] : MAC Source Address least significant bytes (1, 0)\n
    Data[2] : MAC Source Address most significant bytes (5..2)\n
    Data[3] : Frame data (ether_type, VLAN tag, MPLS label, etc..)\n
    Data[4] : Frame data (ether_type, VLAN tag, MPLS label, etc..)\n
    See TCAM packet data fields description in the */
    unsigned int data[MACSEC_RULE_NON_CTRL_WORD_COUNT];

    /** Mask for data values, can be used to mask out irrelevant Data bits */
    unsigned int data_mask[MACSEC_RULE_NON_CTRL_WORD_COUNT];

    /** Priority value that is used to resolve multiple rule matches.
        When multiple rules are hit by a packet simultaneously, the rule with
        the higher priority value will be returned. If multiple rules with
        an identical priority value are hit, the rule with the lowest
        rule index is used. */
    unsigned int priority;

    /** true : drop the packet */
    unsigned int fdrop;

    /** true : process the packet as control packet */
    unsigned int fcontrol_packet;

} macsec_api_rule_t;

typedef struct
{
    /** Low 32-bit counter word */
    unsigned int low;
    /** High 32-bit counter word */
    unsigned int high;
} macsec_api_stat_counter;

typedef struct
{
    /** Packet counters - Transform Error */
    macsec_api_stat_counter OutPktsTransformError;
    /** Packet counters - Control */
    macsec_api_stat_counter OutPktsControl;
    /** Packet counters - Untagged */
    macsec_api_stat_counter OutPktsUntagged;

    /** Octet counters - Uncontrolled */
    macsec_api_stat_counter OutOctetsUncontrolled;
    /** Octet counters - Controlled */
    macsec_api_stat_counter OutOctetsControlled;
    /** Octet counters - Common */
    macsec_api_stat_counter OutOctetsCommon;

    /** Packet counters - Unicast Uncontrolled */
    macsec_api_stat_counter OutPktsUnicastUncontrolled;
    /** Packet counters - Multicast Uncontrolled */
    macsec_api_stat_counter OutPktsMulticastUncontrolled;
    /** Packet counters - Broadcast Uncontrolled */
    macsec_api_stat_counter OutPktsBroadcastUncontrolled;

    /** Packet counters - Unicast Controlled */
    macsec_api_stat_counter OutPktsUnicastControlled;
    /** Packet counters - Multicast Controlled */
    macsec_api_stat_counter OutPktsMulticastControlled;
    /** Packet counters - Broadcast Controlled */
    macsec_api_stat_counter OutPktsBroadcastControlled;
} macsec_api_secy_e_stats;

/** XtSecY counters */
typedef struct
{
    /** Packet counters - Transform Error */
    macsec_api_stat_counter InPktsTransformError;
    /** Packet counters - Control */
    macsec_api_stat_counter InPktsControl;
    /** Packet counters - Untagged */
    macsec_api_stat_counter InPktsUntagged;
    /** Packet counters - No Tag */
    macsec_api_stat_counter InPktsNoTag;
    /** Packet counters - Bad Tag */
    macsec_api_stat_counter InPktsBadTag;
    /** Packet counters - No SCI */
    macsec_api_stat_counter InPktsNoSCI;
    /** Packet counters - Unknown SCI */
    macsec_api_stat_counter InPktsUnknownSCI;
    /** Packet counters - Tagged Ctrl */
    macsec_api_stat_counter InPktsTaggedCtrl;

    /** Octet counters - Uncontrolled */
    macsec_api_stat_counter InOctetsUncontrolled;
    /** Octet counters - Controlled */
    macsec_api_stat_counter InOctetsControlled;

    /** Packet counters - Unicast uncontrolled */
    macsec_api_stat_counter InPktsUnicastUncontrolled;
    /** Packet counters - Multicast Uncontrolled */
    macsec_api_stat_counter InPktsMulticastUncontrolled;
    /** Packet counters - Broadcast Uncontrolled */
    macsec_api_stat_counter InPktsBroadcastUncontrolled;

    /** Packet counters - Unicast Controlled */
    macsec_api_stat_counter InPktsUnicastControlled;
    /** Packet counters - Multicast Controlled */
    macsec_api_stat_counter InPktsMulticastControlled;
    /** Packet counters - Broadcast Controlled */
    macsec_api_stat_counter InPktsBroadcastControlled;
} macsec_api_secy_i_stats;

/** TCAM statistics */
typedef struct
{
    /** Statistics counter */
    macsec_api_stat_counter tcam_hit;
} macsec_api_secy_tcam_stats;

/** RxCAM statistics (ingress only) */
typedef struct
{
    /** Packet counter */
    macsec_api_stat_counter cam_hit;
} macsec_api_secy_rxcam_stats;

/* SA statistics */
typedef struct
{
    /* Octet counters */
    macsec_api_stat_counter OutOctetsEncryptedProtected;

    /* Packet counters */
    macsec_api_stat_counter OutPktsEncryptedProtected;
    macsec_api_stat_counter OutPktsTooLong;
    macsec_api_stat_counter OutPktsSANotInUse;
} macsec_api_secy_sa_e_stats;

typedef struct
{
    /* Octet counters */
    macsec_api_stat_counter InOctetsDecrypted;
    macsec_api_stat_counter InOctetsValidated;

    /* Packet counters */
    macsec_api_stat_counter InPktsUnchecked;
    macsec_api_stat_counter InPktsDelayed;
    macsec_api_stat_counter InPktsLate;
    macsec_api_stat_counter InPktsOK;
    macsec_api_stat_counter InPktsInvalid;
    macsec_api_stat_counter InPktsNotValid;
    macsec_api_stat_counter InPktsNotUsingSA;
    macsec_api_stat_counter InPktsUnusedSA;
} macsec_api_secy_sa_i_stats;

typedef union
{
    macsec_api_settings_t secy_conf;
    macsec_api_sa_t sa_conf;
    macsec_api_rule_t rule_conf;
    macsec_api_secy_e_stats secy_e_stats;
    macsec_api_secy_i_stats secy_i_stats;
    macsec_api_secy_tcam_stats tcam_stats;
    macsec_api_secy_rxcam_stats rxcam_stats;
    macsec_api_secy_sa_e_stats sa_e_stats;
    macsec_api_secy_sa_i_stats sa_i_stats;
} macsec_api_ext_data;

typedef struct 
{
    int op;
    int ret_val;
    int direction;
    int index1;
    int index2;
    int data1; 
    macsec_api_ext_data ext_data;
} macsec_api_data;

#endif