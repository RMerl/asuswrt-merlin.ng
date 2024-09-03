/*
 *  Copyright: (c) 2020 Broadcom.
 *  All rights reserved.
 *
 * <:label-BRCM:2022:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :> 
 */

#ifndef XFLOW_MACSEC_DEFS_H
#define XFLOW_MACSEC_DEFS_H

#include "macsec_types.h"

typedef uint32 bcm_mpls_label_t;

/*******************************************************************************
 * Macro definitions
 */
#define XFLOW_MACSEC_CRYPTO_AES128GCM_KEY_SIZE 16
#define XFLOW_MACSEC_CRYPTO_AES256GCM_KEY_SIZE 32

/* XFLOW MACSEC TPID */
#define XFLOW_MACSEC_VLAN_TPID_MAX      5
#define XFLOW_MACSEC_MATCH_TPID_SEL_0   0x01
#define XFLOW_MACSEC_MATCH_TPID_SEL_1   0x02
#define XFLOW_MACSEC_MATCH_TPID_SEL_2   0x04
#define XFLOW_MACSEC_MATCH_TPID_SEL_3   0x08
#define XFLOW_MACSEC_MATCH_TPID_SEL_4   0x10

/* Macsec API flags (common to all API) */
/* Generic */
#define XFLOW_MACSEC_ENCRYPT_DECRYPT_NONE       0x00
#define XFLOW_MACSEC_ENCRYPT                    0x01
#define XFLOW_MACSEC_DECRYPT                    0x02
/* Secure Channel */
#define XFLOW_MACSEC_SECURE_CHAN_WITH_ID        0x04
/* Policy entry */
#define XFLOW_MACSEC_POLICY_WITH_ID             0x08
/* Flow entry */
#define XFLOW_MACSEC_FLOW_WITH_ID               0x10
/* MTU with ID */
#define XFLOW_MACSEC_MTU_WITH_ID                0x20
/* Ethertype with ID */
#define XFLOW_MACSEC_ETHERTYPE_WITH_ID          0x40

/* Secure Channel API Flags */
/* Enable the TCI.SC bit. */
#define XFLOW_MACSEC_SECURE_CHAN_INFO_INCLUDE_SCI 0x02
/*
 * Enable all data packets on the controlled port.
 * Else, discard all data packets. Encrypt only flag.
 */
#define XFLOW_MACSEC_SECURE_CHAN_INFO_CONTROLLED_PORT       0x04
/*
 * Do not encrypt or authenticate the egress packet.
 * Not applicable for Inline Xflow Macsec.
 */
#define XFLOW_MACSEC_SECURE_CHAN_INFO_ENCRYPT_DISABLE       0x08
/* Enable replay protect on the ingress path. */
#define XFLOW_MACSEC_SECURE_CHAN_INFO_REPLAY_PROTECT_ENABLE 0x10
/*
 * Add the offset extracted from SVTAG to first_auth_range_offset_start.
 * Applicable only to Inline Xflow Macsec.
 */
#define XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_SVTAG_OFFSET_TO_RANGE_START \
                                                            0x20
/*
 * Add the offset extracted from SVTAG to
 * first_auth_range_offset_start + first_auth_range_offset_end.
 * Applicable only to Inline Xflow Macsec.
 */
#define XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_SVTAG_OFFSET_TO_RANGE_END  \
                                                            0x40
/*
 * Add the offset extracted from SVTAG to sectag_offset.
 * Applicable only to Inline Xflow Macsec.
 */
#define XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_SVTAG_OFFSET_TO_SECTAG     \
                                                            0x80
/*
 * Add the offset extracted from SVTAG to header offsets calculated
 * based on vxlansec_pkt_type.
 * Header offsets calculated include L3 length, L3 checksum,
 * L4 length, L4 checksum, and L4 dest-port.
 * Applicable only to Inline Xflow Macsec.
 */
#define XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_SVTAG_OFFSET_TO_VXLANSEC   \
                                                            0x100
/*
 * All SA-invalid packets belonging to this SC are dropped inside
 * the MACSec engine.
 */
#define XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_ZERO_OUT_SA_INVALID_PKT    \
                                                            0x200
/*
 * Same as XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_SVTAG_OFFSET_TO_RANGE_START
 * for decrypt.
 */
#define XFLOW_MACSEC_SECURE_CHAN_DECRYPT_ADJUST_RANGE_START         \
                                                            0x400
/*
 * Same as XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_SVTAG_OFFSET_TO_RANGE_END
 * for decrypt.
 */
#define XFLOW_MACSEC_SECURE_CHAN_DECRYPT_ADJUST_RANGE_END           \
                                                            0x800
/*
 * Prior to Sectag, only bytes between first_auth_range_offset_start
 * and (first_auth_range_offset_start + first_auth_range_offset_end)
 * will be authenticated.
 */
#define XFLOW_MACSEC_SECURE_CHAN_RANGE_AUTHENTICATE                 \
                                                            0x1000
/*
 * Secured data packets (Tagged packets) are forwarded to
 * Macsec engine and undergo Macsec functions.
 * Else secured data packets are dropped and not accounted.
 */
#define XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_CONTROLLED_SECURED_DATA    \
                                                            0x2000
/*
 * Unsecured data packets (Untagged packets) are forwarded to
 * Macsec engine.
 * Else unsecured data packets are dropped and not accounted.
 */
#define XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_CONTROLLED_UNSECURED_DATA  \
                                                            0x4000

/*
 * Do not encrypt or authenticate the egress packet.
 * Default is to authenticate the packet.
 * Additionally, the packet will be encrypted if TCI.E is set.
 * Applicable only to Inline Xflow Macsec.
 * Deprecates XFLOW_MACSEC_SECURE_CHAN_INFO_ENCRYPT_DISABLE.
 */
#define XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_PROTECT_DISABLE            \
                                                            0x8000


/*
 * When set, will set the Bottom-of-Stack bit in the last MPLS label before
 * SECTAG of a received Secure MPLS packet whose SECTAG and ICV are both
 * stripped.
 */
#define XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_SET_MPLS_BOS_ENABLE              \
                                                            0x10000

/*
 * When set, set in UDPIPSec/ L4 MACsec with UDP encapsulation modes, to
 * update the UDP header after encryption. It should be set only when the
 * XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_USE_SVTAG_PKT_TYPE flag is set and intermixing of IPv4/6
 * packets on a single flow is desired.
 */
#define XFLOW_MACSEC_SECURE_CHAN_UDP_PKT      0x20000

/*
 * This bit is set in  UDPIPSec/ L4 MACsec with TCP encapsulation modes, to
 * update the TCP header after encryption. It should be set only when the
 * XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_USE_SVTAG_PKT_TYPE flag is set and intermixing of IPv4/6
 * packets on a single flow is desired.
 */
#define XFLOW_MACSEC_SECURE_CHAN_TCP_PKT      0x40000


/*
 * On MACSec flows, when this bit is set to 1, the packet type is derived from
 * the IPV6_PKT bit in the incoming MACSEC_ESEC_SVTAG
 * On MACSec flows, when this bit is reset to 0, the packet type is derived
 * from the ESEC_SC_TABLE.VXLANSEC_ENC_PKT_TYPE
 * On IPSEC flows, the packet type is always derived from the IPV6_PKT bit in
 * the incoming MACSEC_ESEC_SVTAG and this configuration should always be set
 * to 1.
 */
#define XFLOW_MACSEC_SECURE_CHAN_ENCRYPT_USE_SVTAG_PKT_TYPE                  \
                                                            0x80000

/* Disable both soft and hard SA expiry notification in the decrypt direction. */
#define XFLOW_MACSEC_SECURE_CHAN_DECRYPT_SA_EXPIRE_DISABLE  0x100000

/*
 * Reject all secured duplicate packets within 128 Packet Number for all secure associations
 * for the channel. Applicable only if replay protect is enabled.
 */
#define XFLOW_MACSEC_SECURE_CHAN_DECRYPT_DUPLICATE_REJECT   0x200000

/*
 * When set, will reset the Bottom-of-Stack bit in the last MPLS label
 * before SECTAG of a received Secure MPLS packet whose SECTAG and ICV are both stripped.
 */
#define XFLOW_MACSEC_SECURE_CHAN_DECRYPT_RESET_MPLS_BOS     0x400000

/*
 * In case of any invalid SA error, zero out the data from confidentiality_offset onwards before sending
 * out the packet. Applicable only for IPsec configuration.
 */
#define XFLOW_MACSEC_IPSEC_SECURE_CHAN_ENCRYPT_ZERO_OUT_SA_INVALID_PKT      0x01

/*
 * Set the Bottom-of-Stack bit in the last MPLS label before the ESP header.
 * Applicable only for IPsec configuration.
 */
#define XFLOW_MACSEC_IPSEC_SECURE_CHAN_ENCRYPT_SET_MPLS_BOS_ENABLE          0x02

/*
 * The offset extracted from SVTAG is added to the base offsets calculated using l3_l4_pkt_type.
 * Applicable only for IPsec configuration.
 */
#define XFLOW_MACSEC_IPSEC_SECURE_CHAN_ENCRYPT_SVTAG_OFFSET_TO_L3_L4     0x04

/*
 * Enable replay protect for IPsec configuration.
 */
#define XFLOW_MACSEC_IPSEC_SECURE_CHAN_INFO_REPLAY_PROTECT_ENABLE           0x08

/*
 * Do not encrypt or authenticate the egress packet. Default is to authenticate the packet.
 * Applicable only for IPsec configuration.
 */
#define XFLOW_MACSEC_IPSEC_SECURE_CHAN_ENCRYPT_PROTECT_DISABLE              0x10

/*
 * ControlledPort is enabled for secured data packets. Secured data packets are allowed and accounted in MIBs.
 * Applicable only for IPsec configuration.
 */
#define XFLOW_MACSEC_IPSEC_SECURE_CHAN_ENCRYPT_CONTROLLED_SECURED_DATA      0x20

/*
 * ControlledPort is enabled for un-secured data packets. Unsecured data packets are
 * allowed and accounted in MIBs.
 * Applicable only for IPsec configuration.
 */
#define XFLOW_MACSEC_IPSEC_SECURE_CHAN_ENCRYPT_CONTROLLED_UNSECURED_DATA    0x40

/*
 * Enable IPsec authentication only mode (GMAC mode - RFC 4543).
 */
#define XFLOW_MACSEC_IPSEC_SECURE_CHAN_AUTH_AES_GMAC                        0x80

/*
 * Disable both soft and hard SA expiry notification in the decrypt direction.
 * Applicable only for IPsec configuration.
 */
#define XFLOW_MACSEC_IPSEC_SECURE_CHAN_DECRYPT_SA_EXPIRE_DISABLE            0x100

/*
 * Reject all secured duplicate packets within 128 Sequence Number for all
 * secure associations for the channel. Applicable only if replay protect is enabled.
 * Applicable only for IPsec configuration.
 */
#define XFLOW_MACSEC_IPSEC_SECURE_CHAN_DECRYPT_DUPLICATE_REJECT             0x200

/*
 * Enable all data packets on the controlled port.
 * Else, discard all data packets. Encrypt only flag.
 * Applicable only for IPsec configuration.
 */
#define XFLOW_MACSEC_IPSEC_SECURE_CHAN_INFO_CONTROLLED_PORT                 0x400

/*
 * Allocate second match action entry (when provided both match entries are used)
 * Applicable only for IPsec configuration. Decrypt only flag.
 */
#define XFLOW_MACSEC_IPSEC_SECURE_CHAN_ALLOCATE_MATCH_ACTION_ENTRY1         0x800

/*
 * Add the offset extracted from SVTAG to ESP offset
 */
#define XFLOW_MACSEC_IPSEC_SECURE_CHAN_ENCRYPT_SVTAG_OFFSET_TO_ESP          0x1000

/* Secure Association API Flags */
#define XFLOW_MACSEC_SECURE_ASSOC_INFO_SET_NEXT_PKT_NUM     0x03

#define XFLOW_MACSEC_IPSEC_SECURE_ASSOC_INFO_SET_NEXT_PKT_NUM 0x01


/* xflow_macsec_policy_info_t flags */

/*
 * All untagged (no SecTAG) data packets are dropped and not accounted.
 * Default is to allow and account it in MIB.
 */
#define XFLOW_MACSEC_DECRYPT_POLICY_UNTAGGED_FRAME_DENY          0x01
/*
 * Allow all packets, but check for ICV errors.
 * Not applicable to Inline Xflow Macsec.
 */
#define XFLOW_MACSEC_DECRYPT_POLICY_CHECK_ICV                    0x02
/*
 * Allow all packets irrespective of policy violations.
 * Not applicable to Inline Xflow Macsec.
 */
#define XFLOW_MACSEC_DECRYPT_POLICY_CHECK_NONE                   0x04
/*
 * All untagged data packets on the control port are allowed and accounted.
 * Default is to drop all untagged data packets without accounting in MIB.
 */
#define XFLOW_MACSEC_DECRYPT_POLICY_UNTAGGED_CONTROL_PORT_ENABLE 0x08
/*
 * All tagged data packets on the control port are allowed and processed
 * by Macsec.
 */
#define XFLOW_MACSEC_DECRYPT_POLICY_TAGGED_CONTROL_PORT_ENABLE   0x10
/*
 * The control port operates in point to point mode.
 */
#define XFLOW_MACSEC_DECRYPT_POLICY_POINT_TO_POINT_ENABLE        0x20
/*
 * Identify the decrypt policy as a control packet policy.
 * Not applicable to Inline Xflow Macsec.
 */
#define XFLOW_MACSEC_DECRYPT_POLICY_FOR_CONTROL_PACKET           0x40
/*
 * The SecY identified by the decrypt policy carries custom protocol
 * and the SecTAG is identified using sectag_offset.
 * Applicable only for Inline Xflow Macsec.
 */
#define XFLOW_MACSEC_DECRYPT_POLICY_CUSTOM_PROTOCOL              0x80
/*
 * Based on number of outer vlan and MPLS lables, sectag_offset and
 * inner_l2_offset value gets adjusted for every packet.
 * Applicable only for Inline Xflow Macsec.
 */
#define XFLOW_MACSEC_DECRYPT_POLICY_SECTAG_OFFSET_ADJUST         0x100
/*
 * IPv4 checksum fail and MPLS BOS not found packets are dropped if
 * they are not copied to CPU.
 * Applicable only for Inline Xflow Macsec.
 */
#define XFLOW_MACSEC_DECRYPT_POLICY_IPV4_CHKSUM_FAIL_AND_MPLS_BOS_MISS_DENY \
                                                                 0x200
/*
 * Inner DA and Inner SA are in clear and they are avaialable before SecTag.
 * Otherwise, either inner L2 header is not in clear or packet doesn't
 * have inner L2 header.
 * Applicable only for Inline Xflow Macsec.
 */
#define XFLOW_MACSEC_DECRYPT_POLICY_INNER_L2_VALID               0x400

/*
 * The subport carries IPsec flow and ipsec_outer_ip_offset is valid.
 * Applicable only for Inline Xflow Macsec Gen 2.
 */
#define XFLOW_MACSEC_IPSEC_DECRYPT_POLICY_OUTER_IP_VALID         0x800

/*
 * Adjust ipsec_esp_offset based on the number of VLAN tags or MPLS labels.
 * Applicable only for Inline Xflow Macsec Gen 2.
 */
#define XFLOW_MACSEC_IPSEC_DECRYPT_POLICY_ESP_OFFSET_ADJUST      0x1000

/*
 * Adjust ipsec_outer_ip_offset based on the number of VLAN tags
 * or MPLS labels. Applicable only for Inline Xflow Macsec Gen 2.
 */
#define XFLOW_MACSEC_IPSEC_DECRYPT_POLICY_OUTER_IP_OFFSET_ADJUST    0x2000

/*
 * Identify the subport (SecY) as an IPsec subport.
 * Needed to configure ipsec_esp_offset.
 */
#define XFLOW_MACSEC_IPSEC_DECRYPT_POLICY                        0x4000

/*
 * Used by stat APIs to
 */
#define XFLOW_MACSEC_STAT_SYNC_DISABLE                            0x1


/* vlan_tag_mpls_label_flags flags */
#define XFLOW_MACSEC_NO_TAGS_NO_LABELS                  0x01
#define XFLOW_MACSEC_1_VLAN_TAG_1_MPLS_LABEL            0x02
#define XFLOW_MACSEC_2_VLAN_TAG_2_MPLS_LABEL            0x04
#define XFLOW_MACSEC_3_VLAN_TAG_3_MPLS_LABEL            0x08
#define XFLOW_MACSEC_4_VLAN_TAG_4_MPLS_LABEL            0x10
#define XFLOW_MACSEC_GREATER_4_VLAN_TAG_5_MPLS_LABEL    0x20

/* TPID API Flags */
#define XFLOW_MACSEC_FLOW_TPID_SEL_0           (1 << 0)
#define XFLOW_MACSEC_FLOW_TPID_SEL_1           (1 << 1)
#define XFLOW_MACSEC_FLOW_TPID_SEL_2           (1 << 2)
#define XFLOW_MACSEC_FLOW_TPID_SEL_3           (1 << 3)

/* DECRYPT SOP ERROR handling Flags */
#define XFLOW_MACSEC_DECRYPT_SOP_ERROR_UNKNOWN_POLICY       (1 << 0)
#define XFLOW_MACSEC_DECRYPT_SOP_ERROR_TAG_CTRL_PORT        (1 << 1)
#define XFLOW_MACSEC_DECRYPT_SOP_ERROR_UNTAG_CTRL_PORT      (1 << 2)
#define XFLOW_MACSEC_DECRYPT_SOP_ERROR_IPV4_MPLS            (1 << 3)
#define XFLOW_MACSEC_DECRYPT_SOP_ERROR_INVALID_SECTAG       (1 << 4)
#define XFLOW_MACSEC_DECRYPT_SOP_ERROR_UNKNOWN_SECURE_CHAN  (1 << 5)
#define XFLOW_MACSEC_DECRYPT_SOP_ERROR_UNKNOWN_SECURE_ASSOC (1 << 6)
#define XFLOW_MACSEC_DECRYPT_SOP_ERROR_REPLAY_FAILURE       (1 << 7)

/* Flags for xflowMacsecPortSectagRuleEnable enum. */
/* Invalidate packet if SECTAG.TCI.V != (configural value). */
#define XFLOW_MACSEC_VALIDATE_SECTAG_VERSION_INVALID    (1 << 0)
/* Invalidate packet if Short_length rcv > 47B */
#define XFLOW_MACSEC_VALIDATE_SECTAG_SHORT_LEN_MAX      (1 << 1)
/* Invalidate packet if short length not set but a
 * short packet has arrived. */
#define XFLOW_MACSEC_VALIDATE_SECTAG_SHORT_LEN_UNSET    (1 << 2)
/* Invalidate packet if SECTAG.TCI.E = 0 and SECTAG.TCI.C = 1. */
#define XFLOW_MACSEC_VALIDATE_SECTAG_E0_C1              (1 << 4)
/* Invalidate packet if SECTAG.TCI.ES = 1 and SECTAG.TCI.SC = 1. */
#define XFLOW_MACSEC_VALIDATE_SECTAG_ES1_SC1            (1 << 5)
/* Invalidate packet if SECTAG.TCI.SC = 1 and SECTAG.TCI.SCB = 1. */
#define XFLOW_MACSEC_VALIDATE_SECTAG_SC1_SCB1           (1 << 6)
/* Invalidate packet if SECTAG.PN = 0. */
#define XFLOW_MACSEC_VALIDATE_SECTAG_PN0                (1 << 7)
/* Invalidate packet if short length does not match the
 * received packet length. */
#define XFLOW_MACSEC_VALIDATE_SECTAG_SHORT_LEN_MISMATCH (1 << 8)

/* Flags for xflowMacsecPortMPLS enum. */
/* Enable the configured MPLS Ethertype. */
#define XFLOW_MACSEC_MPLS_ETYPE_0   0x1
#define XFLOW_MACSEC_MPLS_ETYPE_1   0x2
#define XFLOW_MACSEC_MPLS_ETYPE_2   0x4
#define XFLOW_MACSEC_MPLS_ETYPE_3   0x8

/* Flags for xflowMacsecPortPTPMatchRule enum. */
/* Enable untagged PTP EtherType match. */
#define XFLOW_MACSEC_MATCH_PTP_UNTAGGED     (1 << 0)
/* Enable PTP EtherType match with 1 VLAN. */
#define XFLOW_MACSEC_MATCH_PTP_ONE_VLAN     (1 << 1)
/* Enable PTP packet with UDP over IPV4 match. */
#define XFLOW_MACSEC_MATCH_PTP_UDP_IPV4     (1 << 2)
/* Enable PTP packet with UDP over IPV6 match. */
#define XFLOW_MACSEC_MATCH_PTP_UDP_IPV6     (1 << 3)

/* Flags for xflowMacsecPortMgmtPktRules enum. */
/* Match as management packet when MAC_DA[47:4] == 0x0180_c200_000. */
#define XFLOW_MACSEC_MGMT_DEST_MAC_0X0180C200000             (1 << 0)
/* Match as management packet when MAC_DA == 0x0100_0ccc_cccc. */
#define XFLOW_MACSEC_MGMT_DEST_MAC_0X01000CCCCCCC            (1 << 1)
/* Enable rule to match as management packet when any of 8
 * configurable MAC_DA matches. */
#define XFLOW_MACSEC_MGMT_DEST_MAC0                          (1 << 2)
#define XFLOW_MACSEC_MGMT_DEST_MAC1                          (1 << 3)
#define XFLOW_MACSEC_MGMT_DEST_MAC2                          (1 << 4)
#define XFLOW_MACSEC_MGMT_DEST_MAC3                          (1 << 5)
#define XFLOW_MACSEC_MGMT_DEST_MAC4                          (1 << 6)
#define XFLOW_MACSEC_MGMT_DEST_MAC5                          (1 << 7)
#define XFLOW_MACSEC_MGMT_DEST_MAC6                          (1 << 8)
#define XFLOW_MACSEC_MGMT_DEST_MAC7                          (1 << 9)
/* Enable rule to match as management packet when any of 8
 * configurable Ethertype matches. */
#define XFLOW_MACSEC_MGMT_ETYPE0                             (1 << 10)
#define XFLOW_MACSEC_MGMT_ETYPE1                             (1 << 11)
#define XFLOW_MACSEC_MGMT_ETYPE2                             (1 << 12)
#define XFLOW_MACSEC_MGMT_ETYPE3                             (1 << 13)
#define XFLOW_MACSEC_MGMT_ETYPE4                             (1 << 14)
#define XFLOW_MACSEC_MGMT_ETYPE5                             (1 << 15)
#define XFLOW_MACSEC_MGMT_ETYPE6                             (1 << 16)
#define XFLOW_MACSEC_MGMT_ETYPE7                             (1 << 17)
/* Enable rule to match as management packet when the
 * programmable MAC_DA address range match. */
#define XFLOW_MACSEC_MGMT_DEST_MAC_RANGE                     (1 << 18)
/* Enable rule to match as management packet when the sets of
 * programmable MAC_DA and EtherType match. */
#define XFLOW_MACSEC_MGMT_DEST_MAC_ETYPE0                    (1 << 19)
#define XFLOW_MACSEC_MGMT_DEST_MAC_ETYPE1                    (1 << 20)
/* Enable rule to match as management packet when SECTAG is present
 * and SECTAG.TCI.E = 1 and SECTAG.TCI.C = 0. */
#define XFLOW_MACSEC_MGMT_E1_C0                              (1 << 21)
/* Enable rule to match PTP/IEEE 1588 packet as management packet. */
#define XFLOW_MACSEC_MGMT_PTP                                (1 << 22)
/* Enable to allow IPsec IKE packet having UDP destination port provided by the
 * control xflowMacsecIpsecControlUdpDstPortWithNonEsp, when the 4 byte
 * NON-ESP marker after UDP header is 0. */
#define XFLOW_MACSEC_MGMT_IPSEC_UDP_DST_PORT_WITH_NON_ESP    (1 << 23)
/* Enable to allow IPsec IKE packet having UDP destination port provided by the
 * control xflowMacsecIpsecControlUdpDstPortWithoutNonEsp, when the 4 byte
 * NON-ESP marker is not present. */
#define XFLOW_MACSEC_MGMT_IPSEC_UDP_DST_PORT_WITHOUT_NON_ESP (1 << 24)
/* Enable to allow IPsec NAT keep-alive packet with UDP destination port
 * xflowMacsecIpsecControlNatUdpDstPort, 1 byte payload value 0xFF and
 * optional UDP source port xflowMacsecIpsecControlNatUdpSrcPort and
 * per-port control xflowMacsecIPsecPortNatKeepalive set. */
#define XFLOW_MACSEC_MGMT_IPSEC_UDP_NAT_PORT                 (1 << 25)


/* Common macro definitions. */
#define XFLOW_MACSEC_ID_TYPE_GET(id)                         \
    ((id >> 28) & 0xf)
#define XFLOW_MACSEC_DIR_TYPE_GET(id)                        \
    ((id >> 26) & 0x3)
#define XFLOW_MACSEC_INDEX_GET(id)                           \
    ((id) & 0x3FFFFF)
#define XFLOW_MACSEC_INSTANCE_GET(id)                        \
    ((id >> 22) & 0xF)

/* Secure Channel macro definitions. */
#define XFLOW_MACSEC_SECURE_CHAN_ID_CREATE(flag, chan_id)    \
    ((xflowMacsecIdTypeSecureChan << 28) |                   \
     ((flag & 0x3) << 26) | (chan_id & 0x3FF))

#define XFLOW_MACSEC_SECURE_CHAN_ID_INDEX_GET(chan_id)       \
    ((XFLOW_MACSEC_ID_TYPE_GET(chan_id) ==                   \
      xflowMacsecIdTypeSecureChan) ?                         \
     (((chan_id & 0x3FFFFF) > 0x3FF) ? BCM_E_PARAM :        \
      (chan_id & 0x3FF)) : BCM_E_PARAM)

#define XFLOW_MACSEC_SECURE_CHAN_ID_IS_ENCRYPT(chan_id)      \
    ((XFLOW_MACSEC_ID_TYPE_GET(chan_id) ==                   \
      xflowMacsecIdTypeSecureChan) ?                         \
     (((chan_id >> 26) & 0x3) == XFLOW_MACSEC_ENCRYPT) :     \
     -1)

#define XFLOW_MACSEC_SECURE_CHAN_ID_IS_DECRYPT(chan_id)      \
    ((XFLOW_MACSEC_ID_TYPE_GET(chan_id) ==                   \
      xflowMacsecIdTypeSecureChan) ?                         \
     (((chan_id >> 26) & 0x3) == XFLOW_MACSEC_DECRYPT) :     \
     -1)

#define XFLOW_MACSEC_SECURE_CHAN_ID_WITH_INSTANCE_CREATE(inst, flag, chan_id)   \
    ((xflowMacsecIdTypeSecureChan << 28) |                                      \
     ((flag & 0x3) << 26) |                                                     \
     ((inst & 0xf) << 22) |                                                     \
     (chan_id & 0x3FF))


/* Secure Association macro definitions. */
#define XFLOW_MACSEC_SECURE_ASSOC_ID_CREATE(flag, assoc_id)  \
    ((xflowMacsecIdTypeSecureAssoc << 28) |                  \
     ((flag & 0x3) << 26) | (assoc_id & 0x7FF))

#define XFLOW_MACSEC_SECURE_ASSOC_ID_INDEX_GET(assoc_id)     \
    ((XFLOW_MACSEC_ID_TYPE_GET(assoc_id) ==                  \
      xflowMacsecIdTypeSecureAssoc) ?                        \
     (((assoc_id & 0x3FFFFF) > 0x7FF) ? BCM_E_PARAM :       \
      (assoc_id & 0x7FF)) : BCM_E_PARAM)

#define XFLOW_MACSEC_SECURE_ASSOC_ID_IS_ENCRYPT(assoc_id)    \
    ((XFLOW_MACSEC_ID_TYPE_GET(assoc_id) ==                  \
      xflowMacsecIdTypeSecureAssoc) ?                        \
     (((assoc_id >> 26) & 0x3) == XFLOW_MACSEC_ENCRYPT) :    \
     -1)

#define XFLOW_MACSEC_SECURE_ASSOC_ID_IS_DECRYPT(assoc_id)    \
    ((XFLOW_MACSEC_ID_TYPE_GET(assoc_id) ==                  \
      xflowMacsecIdTypeSecureAssoc) ?                        \
     (((assoc_id >> 26) & 0x3) == XFLOW_MACSEC_DECRYPT) :    \
     -1)
#define XFLOW_MACSEC_SECURE_ASSOC_ID_WITH_INSTANCE_CREATE(inst, flag, assoc_id)  \
    ((xflowMacsecIdTypeSecureAssoc << 28) |                  \
     ((flag & 0x3) << 26) |                                  \
     ((inst & 0xf) << 22) |                                  \
     (assoc_id & 0x7FF))


/* Decrypt Policy macro definitions. */
#define XFLOW_MACSEC_POLICY_ID_CREATE(flag, id)              \
    ((xflowMacsecIdTypePolicy << 28) |                       \
     ((flag & 0x3) << 26) |                                  \
     (id & 0x3FF))

#define XFLOW_MACSEC_POLICY_ID_INDEX_GET(policy_id)          \
    ((XFLOW_MACSEC_ID_TYPE_GET(policy_id) ==                 \
      xflowMacsecIdTypePolicy) ?                             \
     (((policy_id & 0x3FFFFF) > 0x3FF) ? BCM_E_PARAM :      \
      (policy_id & 0x3FF)) : BCM_E_PARAM)

#define XFLOW_MACSEC_POLICY_ID_IS_ENCRYPT(policy_id)         \
    ((XFLOW_MACSEC_ID_TYPE_GET(policy_id) ==                 \
      xflowMacsecIdTypePolicy) ?                             \
     (((policy_id >> 26) & 0x3) == XFLOW_MACSEC_ENCRYPT) :   \
     -1)

#define XFLOW_MACSEC_POLICY_ID_IS_DECRYPT(policy_id)         \
    ((XFLOW_MACSEC_ID_TYPE_GET(policy_id) ==                 \
      xflowMacsecIdTypePolicy) ?                             \
     (((policy_id >> 26) & 0x3) == XFLOW_MACSEC_DECRYPT) :   \
     -1)

#define XFLOW_MACSEC_POLICY_ID_WITH_INSTANCE_CREATE(instance, flag, id)              \
    ((xflowMacsecIdTypePolicy << 28) |                       \
     ((flag & 0x3) << 26) |                                  \
     ((instance & 0xf) << 22) |                                  \
     (id & 0x3FF))

#define XFLOW_MACSEC_POLICY_ID_INSTANCE_GET(policy_id)          \
    ((XFLOW_MACSEC_ID_TYPE_GET(policy_id) ==                 \
      xflowMacsecIdTypePolicy) ?                             \
     ((policy_id >> 22)& 0xF) : BCM_E_PARAM)

/* Decrypt Flow macro definitions. */
#define XFLOW_MACSEC_FLOW_ID_CREATE(flag, id)              \
    ((xflowMacsecIdTypeFlow << 28) |                       \
    ((flag & 0x3) << 26) |                                 \
    (id & 0x3FF))

#define XFLOW_MACSEC_FLOW_ID_INDEX_GET(flow_id)            \
    ((XFLOW_MACSEC_ID_TYPE_GET(flow_id) ==                 \
      xflowMacsecIdTypeFlow) ?                             \
     (((flow_id & 0x3FFFFFF) > 0x3FF) ? BCM_E_PARAM :      \
      (flow_id & 0x3FF)) : BCM_E_PARAM)

#define XFLOW_MACSEC_FLOW_ID_IS_ENCRYPT(flow_id)           \
    ((XFLOW_MACSEC_ID_TYPE_GET(flow_id) ==                 \
        xflowMacsecIdTypeFlow) ?                           \
        (((flow_id >> 26) & 0x3) == XFLOW_MACSEC_ENCRYPT) :\
        -1)

#define XFLOW_MACSEC_FLOW_ID_IS_DECRYPT(flow_id)           \
    ((XFLOW_MACSEC_ID_TYPE_GET(flow_id) ==                 \
        xflowMacsecIdTypeFlow) ?                           \
        (((flow_id >> 26) & 0x3) == XFLOW_MACSEC_DECRYPT) :\
        -1)

#define XFLOW_MACSEC_FLOW_ID_WITH_INSTANCE_CREATE(instance, flag, id)              \
    ((xflowMacsecIdTypeFlow << 28) |                       \
    ((flag & 0x3) << 26) |                                 \
     ((instance & 0xf) << 22) |                                  \
    (id & 0x7FF))

/* Subport macro definitions. */
#define XFLOW_MACSEC_SUBPORT_ID_CREATE(flag, id)               \
    ((xflowMacsecIdTypeSubportNum << 28) |                     \
     ((flag & 0x3) << 26) |                                    \
     (id & 0x3FF))

#define XFLOW_MACSEC_SUBPORT_ID_INDEX_GET(subport_id)          \
    ((XFLOW_MACSEC_ID_TYPE_GET(subport_id) ==                  \
      xflowMacsecIdTypeSubportNum) ?                           \
     (((subport_id & 0x3FFFFF) > 0x3FF) ? BCM_E_PARAM :       \
      (subport_id & 0x3FF)) : BCM_E_PARAM)

#define XFLOW_MACSEC_SUBPORT_ID_IS_ENCRYPT(subport_id)         \
    ((XFLOW_MACSEC_ID_TYPE_GET(subport_id) ==                  \
      xflowMacsecIdTypeSubportNum) ?                           \
     (((subport_id >> 26) & 0x3) == XFLOW_MACSEC_ENCRYPT) :    \
     -1)

#define XFLOW_MACSEC_SUBPORT_ID_IS_DECRYPT(subport_id)         \
    ((XFLOW_MACSEC_ID_TYPE_GET(subport_id) ==                  \
      xflowMacsecIdTypeSubportNum) ?                           \
     (((subport_id >> 26) & 0x3) == XFLOW_MACSEC_DECRYPT) :    \
     -1)

#define XFLOW_MACSEC_SUBPORT_NUM_GET(subport_id)               \
    ((XFLOW_MACSEC_ID_TYPE_GET(subport_id) ==                  \
      xflowMacsecIdTypeSubportNum) ?                           \
     (((subport_id & 0x3FFFFF) > 0x3FF) ? BCM_E_PARAM :       \
      (subport_id & 0x3FF)) : BCM_E_PARAM)

#define XFLOW_MACSEC_SUBPORT_ID_WITH_INSTANCE_CREATE(instance, flag, id)               \
    ((xflowMacsecIdTypeSubportNum << 28) |                     \
     ((flag & 0x3) << 26) |                                    \
     ((instance & 0xf) << 22) |                                  \
     (id & 0x3FF))

/*******************************************************************************
 * Structures and Enums
 */

typedef uint32 xflow_macsec_policy_id_t;
typedef uint32 xflow_macsec_secure_chan_id_t;
typedef uint32 xflow_macsec_secure_assoc_id_t;
typedef uint32 xflow_macsec_secure_assoc_num_t;
typedef uint32 xflow_macsec_flow_id_t;
typedef uint32 xflow_macsec_subport_id_t;
typedef uint32 xflow_macsec_id_t;
typedef uint32 xflow_macsec_instance_id_t;

/*
 * Macsec id:
 * 31 - 28 : Type of id (SC,SA,Policy)
 * 27 - 26 : Encrypt or Decrypt
 * 25 - 22 : Instance number
 * 21 - 11 : Unused
 * 10 -  0 : Index - SC, SA, policy, flow or subport_id
 */
typedef enum xflow_macsec_index_type_s {
    xflowMacsecIdTypeSecureChan     = 0,
    xflowMacsecIdTypeSecureAssoc    = 1,
    xflowMacsecIdTypePolicy         = 2,
    xflowMacsecIdTypeFlow           = 3,
    xflowMacsecIdTypeSubportNum     = 4,
    xflowMacsecIdTypePort           = 5,
    xflowMacsecIdTypeInvalid        = 6,
    xflowMacsecIdTypeCount          = 7,
} xflow_macsec_index_type_t;

typedef enum xflow_macsec_crypto_e {
    xflowMacsecCryptoAes128GcmIntegrityOnly     = 0,
    xflowMacsecCryptoAes128Gcm                  = 1,
    xflowMacsecCryptoAes128GcmXpnIntegrityOnly  = 2,
    xflowMacsecCryptoAes128GcmXpn               = 3,
    xflowMacsecCryptoAes256GcmIntegrityOnly     = 4,
    xflowMacsecCryptoAes256Gcm                  = 5,
    xflowMacsecCryptoAes256GcmXpnIntegrityOnly  = 6,
    xflowMacsecCryptoAes256GcmXpn               = 7,
    xflowMacsecCryptoCount = 8 /* Always last not to be used */
} xflow_macsec_crypto_t;

typedef enum xflow_macsec_mtu_e {
    xflowMacsecMtu0 = 0,
    xflowMacsecMtu1 = 1,
    xflowMacsecMtu2 = 2,
    xflowMacsecMtu3 = 3,
    xflowMacsecMtuCount = 4 /* Always last not to be used */
} xflow_macsec_mtu_t;

typedef enum xflow_macsec_sectag_ethertype_s {
    xflowMacsecSecTagEtype0 = 0,
    xflowMacsecSecTagEtype1 = 1,
    xflowMacsecSecTagEtype2 = 2,
    xflowMacsecSecTagEtype3 = 3,
    xflowMacsecSecTagEtypeCount = 4 /* Always last not to be used */
} xflow_macsec_sectag_ethertype_t;

typedef enum xflow_macsec_flow_pkt_type_s {
    xflowMacsecDecryptFlowAny            = 0,
    xflowMacsecDecryptFlowNonMacsec      = 1,
    xflowMacsecDecryptFlowMacSec         = 2,
    xflowMacsecDecryptFlowManagement     = 3,
    xflowMacsecDecryptFlowKay            = 4,
    xflowMacsecDecryptFlowCount = 5 /* Always last not to be used */
} xflow_macsec_flow_pkt_type_t;

typedef enum xflow_macsec_flow_etype_s {
    xflowMacsecFlowEtypeAny     = 0,
    xflowMacsecFlowEtypeEII     = 1,
    xflowMacsecFlowEtypeSnap    = 2,
    xflowMacsecFlowEtypeLlc     = 3,
    xflowMacsecFlowEtypeMpls    = 4,
    xflowMacsecFlowEtypeCount = 5 /* Always last not to be used */
} xflow_macsec_flow_etype_t;

typedef enum xflow_macsec_vlan_mpls_tag_status_s {
    xflowMacsecTagAny               = 0,
    xflowMacsecTagUntaggedVlan      = 1,
    xflowMacsecTagSingleVlan        = 2,
    xflowMacsecTagDoubleVlan        = 3,
    xflowMacsecTagOneMplsLabel      = 4,
    xflowMacsecTagTwoMplsLabel      = 5,
    xflowMacsecTagThreeMplsLabel    = 6,
    xflowMacsecTagCount = 7 /* Always last not to be used */
} xflow_macsec_vlan_mpls_tag_status_t;

typedef enum xflow_macsec_control_e {
    /* PN Threshold value. */
    xflowMacsecControlPNThreshold           = 0,

    /* XPN Threshold value. */
    xflowMacsecControlXPNThreshold          = 1,

    /* Enable encrypt failure copy to CPU. */
    xflowMacsecControlEncryptFailCopyToCpu  = 2,

    /* Enable decrypt failure copy to CPU. */
    xflowMacsecControlDecryptFailCopyToCpu  = 3,

    /* Enable encrypt failure drop. */
    xflowMacsecControlEncryptFailDrop       = 4,

    /* Enable decrypt failure drop. */
    xflowMacsecControlDecryptFailDrop       = 5,

    /* The following enums are applicable only for Inline Xflow Macsec architecture. */

    /* The value of maximum transmission unit (MTU) for management packets. */
    xflowMacsecControlMgmtMTU               = 6,

    /* Special Vlan Tag (SVTAG) is present after outer MAC SA. */
    xflowMacsecControlSVTagEnable           = 7,

    /* The following enums are applicable only for the decrypt flow. */

    /* TCP/UDP Destination Port Number for PTP */
    xflowMacsecControlPTPDestPortGeneral    = 8,

    /* TCP/UDP Destination Port Number for PTP */
    xflowMacsecControlPTPDestPortEvent      = 9,

    /* PBB packet TPID value. B-TAG TPID. */
    xflowMacsecControlPbbTpidBTag           = 10,

    /* PBB packet TPID value. I-TAG TPID. */
    xflowMacsecControlPbbTpidITag           = 11,

    /* VNTAG Ethertype. */
    xflowMacsecControlEtypeNIV              = 12,

    /* ETAG Ethertype. */
    xflowMacsecControlEtypePE               = 13,

    /* First Ethertype to match rudimentary management packets. */
    xflowMacsecControlEtypeMgmt0            = 14,

    /* Second Ethertype to match rudimentary management packets. */
    xflowMacsecControlEtypeMgmt1            = 15,

    /* Third Ethertype to match rudimentary management packets. */
    xflowMacsecControlEtypeMgmt2            = 16,

    /* Fourth Ethertype to match rudimentary management packets. */
    xflowMacsecControlEtypeMgmt3            = 17,

    /* Fifth Ethertype to match rudimentary management packets. */
    xflowMacsecControlEtypeMgmt4            = 18,

    /* Sixth Ethertype to match rudimentary management packets. */
    xflowMacsecControlEtypeMgmt5            = 19,

    /* Seventh Ethertype to match rudimentary management packets. */
    xflowMacsecControlEtypeMgmt6            = 20,

    /* Eighth Ethertype to match rudimentary management packets. */
    xflowMacsecControlEtypeMgmt7            = 21,

    /* TCP/UDP Destination Port Number for VxLANSec. */
    xflowMacsecControlVxLANSecDestPort      = 22,

    /*
     * TCP/UDP Destination Port Number for L4 secure packet.
     * The destination port is replaced based on the parameters specified
     * using xflow_macsec_secure_chan_vxlansec_hdr_t when configuring
     * Security Channel policies.
     */
    xflowMacsecControlOutDestPort           = 23,

    /* MPLS ethertype to match incoming packet. */
    xflowMacsecControlMplsEtype0            = 24,

    /* MPLS ethertype to match incoming packet. */
    xflowMacsecControlMplsEtype1            = 25,

    /* MPLS ethertype to match incoming packet. */
    xflowMacsecControlMplsEtype2            = 26,

    /* MPLS ethertype to match incoming packet. */
    xflowMacsecControlMplsEtype3            = 27,

    /* Special Vlan Tag (SVTAG) TPID/Ethertype value to be matched on. */
    xflowMacsecControlSVTagTPIDEtype        = 28,

    /* Select ip info after MPLSBOS for SP tcam key */
    xflowMacsecControlSelIpInfoAfterMplsBos = 29,

    /* Select default drop action for ingress SVTAG SOP error types */
    xflowMacsecControlSvtagSopErrorDrop     = 30,

    /*
     * Set the mask for each one of the 4 PN or XPN Threshold profiles.
     * The mask is calculated based on the formula (2^(value+1) - 1).
     */
    xflowMacsecControlXPNThresholdMask0     = 31,
    xflowMacsecControlXPNThresholdMask1     = 32,
    xflowMacsecControlXPNThresholdMask2     = 33,
    xflowMacsecControlXPNThresholdMask3     = 34,

    /*
     * IPSec IKE UDP destination port number used with 4-byte non-ESP
     * marker value 0.
     * Applicable if  XFLOW_MACSEC_MGMT_IPSEC_UDP_DST_PORT_WITH_NON_ESP
     * is set.
     */
    xflowMacsecIpsecControlUdpDstPortWithNonEsp = 35,

    /*
     * IPSec IKE UDP Destination port number used without non-ESP marker.
     * Applicable if XFLOW_MACSEC_MGMT_IPSEC_UDP_DST_PORT_WITHOUT_NON_ESP
     * is set.
     */
    xflowMacsecIpsecControlUdpDstPortWithoutNonEsp  =   36,

    /*
     * IPSec NAT-keepalive packet UDP Destination port number used with
     * 1-byte payload value 0xFF.
     * Applicable if XFLOW_MACSEC_MGMT_IPSEC_UDP_NAT_PORT is set.
     */
    xflowMacsecIpsecControlNatUdpDstPort    = 37,

    /*
     * If enabled, IPSec NAT-keepalive packet UDP source port number
     * is used to detect NAT KeepAlive packet in addition to UDP destination
     * port match (xflowMacsecIpsecNatUdpDstPort) and with 1-byte payload
     * value  0xFF.
     * Applicable only if xflowMacsecIPsecPortNatKeepalive is enabled.
     */
    xflowMacsecIpsecControlNatUdpSrcPort    = 38,

    /*
     * If enabled, IPSec UDP encapsulated ESP packets are identified
     * by comparing the packet UDP destination port number with this parameter.
     * Applicable only if xflowMacsecIPsecPortEspUdp is enabled.
     */
    xflowMacsecIpsecControlESPUdpDstPort    = 39,

    /*
     * IPSec UDP encapsulated ESP packets are identified
     * by comparing packet UDP source port number with this parameter
     * in addition to UDP destination port match
     * (xflowMacsecIpsecControlESPUdpDstPort).
     * Applicable only if xflowMacsecIPsecPortEspUdpSrcPort is enabled.
     */
    xflowMacsecIpsecControlESPUdpSrcPort    = 40,

    /*
     * Enable to configure IPsec padding bytes as a
     * monotonically incrementing sequence with the first
     * padding byte appended as 1.
     */
    xflowMacsecIpsecControlIncrementPadBytes= 41,

    /*
     * Value populated in the next hop (NH) field of the ESP trailer
     * in the encrypt direction.
     */
    xflowMacsecIpsecControlNextHopDummy     = 42,

    /*
     * UDP Destination Port Number for L4 secure packet.
     * The destination port is replaced based on the parameters specified
     * using xflow_macsec_secure_chan_vxlansec_hdr_t when configuring
     * Security Channel policies.
     */
    xflowMacsecControlUdpDestPort        = 43,

    /*
     * TCP Destination Port Number for L4 secure packet.
     * The destination port is replaced based on the parameters specified
     * using xflow_macsec_secure_chan_vxlansec_hdr_t when configuring
     * Security Channel policies.
     */
    xflowMacsecControlTcpDestPort        = 44,

    /*
     * Treating Packet with SecTAG.TCI.C=1 and SecTAG.TCI.E=0 as error packet
     */
    xflowMacsecControlSectagE0C1Error    = 45,

    /*
     * Encrypt:
     * When set to 1, the egress OutOctets count for Secured Data packets is the
     * sum of all the octets of the MSDUs delivered by the user of the Controlled
     * Port to the Secure Frame Generation process, plus the octets of the destination
     * and source MAC addresses.
     * When configured to 0, the egress OutOctets count for Secured Data packets is
     * the sum of all the octets of the packet excluding SecTAG and ICV and all the
     * bytes which are not authenticated.
     *
     * Decrypt:
     * When set to 1, the ingress InOctets count for Secured Data packets is the sum
     * of all the octets of the MSDUs delivered to the user of the Controlled Port by
     * the Secure Frame Verification process, plus the octets of the destination and
     * source MAC addresses i.e MSDU+DA+SA.
     * When configured to 0, the ingress InOctets count for Secured Data packets is
     * the sum of all the octets of the packet excluding SecTAG and ICV and all the
     * bytes which are not authenticated i.e MSDU+PreSecTag Authentication bytes.
     */
    xflowMacsecControlMibOctetMode       = 46,

    /*
     * If set to 1, IPSec flow ESP trailer pad bytes are compared with zeroes for validation.
     * If they don't match, ESP trailer Pad mismatch is set.
     * Otherwise, ESP trailer pad data is not checked.
     */
    xflowMacsecIpsecControlEspTrailPadChk       = 47,

    /*
     * If xflowMacsecIpsecEspTrailPadChk is set to 1, xflowMacsecIpsecControlIncrByteForEspTrailPadChk
     * is used to specify pad bytes value for ESP trailer pad bytes check.
     * If xflowMacsecIpsecControlIncrByteForEspTrailPadChk is set to 1, incremental values starting
     * from integer 1 is used for ESP trailer pad check (1,2,...,PadLen-1, PadLen).
     * Otherwise, value 0 is used for ESP trailer pad check.
     */
    xflowMacsecIpsecControlIncrByteForEspTrailPadChk       = 48,

    /*
     * If set to 1, MACSec flow secure packet IP length is checked against actual packet
     * length (excluding CRC/FCS) and length_error is set at EOP for mismatched packets.
     * This check is done for secure packets with IP header getting updated because of SecTag/ICV removal.
     * Otherwise, IP length check is disabled for all MACSec flow packets.
     */
    xflowMacsecControlSecurePktIpLengthChk       = 49,

    /*
     * If set to 1, IPSec flow secure packet IP length is checked against actual packet
     * length (excluding CRC/FCS) and length_error is set at EOP for mismatched packets.
     * This check is done for secure packets with IP header getting updated because of ESP Header/ICV removal.
     * Otherwise, IP length check is disabled for all IPSec flow packets.
     */
    xflowMacsecIpsecControlSecurePktIpLengthChk       = 50,

    xflowMacsecControlCount              = 51 /* Always last not to be used */
} xflow_macsec_control_t;

typedef enum xflow_macsec_tag_validate_e {
    /* Packets are forwarded without MACSEC functions. */
    xflowMacsecTagValidateBypassMacsec   = 0,

    /*
     * Account and drop all errored/policy violated tagged data packets.
     */
    xflowMacsecTagValidateStrict         = 1,

    /*
     * All data packets are allowed and accounted in Controlled Port
     * irrespective of policy violation, but ICV check is done and accounted.
     */
    xflowMacsecTagValidateCheckICV       = 2,

    /*
     * All data packets are allowed and accounted in Controlled Port
     * irrespective of policy violation and do not perform ICV check.
     */
    xflowMacsecTagValidateCheckNone      = 3,

    /*
     * Deny all data and control packets and accounted through the Controlled Port.
     */
    xflowMacsecTagValidateDenyAll        = 4,

    /* Max value of the enum. Not to be used. */
    xflowMacsecTagValidateCount          = 5
} xflow_macsec_tag_validate_t;

typedef enum xflow_macsec_stat_type_e {
    xflowMacsecStatTypeInvalid              = 0,

/* MIB Counters for MACsec Uncontrolled Port (per flow / sub-port) as per RFC2863*/
    xflowMacsecUnctrlPortInOctets           = 1,
    xflowMacsecUnctrlPortInUcastPkts        = 2,
    xflowMacsecUnctrlPortInMulticastPkts    = 3,
    xflowMacsecUnctrlPortInBroadcastPkts    = 4,
    xflowMacsecUnctrlPortInDiscards         = 5,
    xflowMacsecUnctrlPortOutOctets          = 6,
    xflowMacsecUnctrlPortOutUcastPkts       = 7,
    xflowMacsecUnctrlPortOutMulticastPkts   = 8,
    xflowMacsecUnctrlPortOutBroadcastPkts   = 9,
    xflowMacsecUnctrlPortOutErrors          = 10,

/* MIB Counters for MACsec Controlled Port (per flow / sub-port) as per RFC2863 */
    xflowMacsecCtrlPortInOctets             = 11,
    xflowMacsecCtrlPortInUcastPkts          = 12,
    xflowMacsecCtrlPortInMulticastPkts      = 13,
    xflowMacsecCtrlPortInBroadcastPkts      = 14,
    xflowMacsecCtrlPortInDiscards           = 15,
    xflowMacsecCtrlPortInErrors             = 16,
    xflowMacsecCtrlPortOutOctets            = 17,
    xflowMacsecCtrlPortOutUcastPkts         = 18,
    xflowMacsecCtrlPortOutMulticastPkts     = 19,
    xflowMacsecCtrlPortOutBroadcastPkts     = 20,
    xflowMacsecCtrlPortOutErrors            = 21,

/* MIB Counters for MACsec Controlled Port per 802.1AE MIB */
/* Per flow / sub-port counters as per 802.1AE MIB*/
    xflowMacsecSecyStatsTxUntaggedPkts      = 22,
    xflowMacsecSecyStatsTxTooLongPkts       = 23,
    xflowMacsecSecyStatsRxUntaggedPkts      = 24,
    xflowMacsecSecyStatsRxNoTagPkts         = 25,
    xflowMacsecSecyStatsRxBadTagPkts        = 26,
    xflowMacsecSecyStatsRxUnknownSCIPkts    = 27,
    xflowMacsecSecyStatsRxNoSCIPkts         = 28,
    xflowMacsecSecyStatsRxOverrunPkts       = 29,

/* Per Secure Channel counters as per 802.1AE MIB*/
    xflowMacsecSecyTxSCStatsProtectedPkts   = 30,
    xflowMacsecSecyTxSCStatsEncryptedPkts   = 31,
    xflowMacsecSecyTxSCStatsOctetsProtected = 32,
    xflowMacsecSecyTxSCStatsOctetsEncrypted = 33,
    xflowMacsecSecyRxSCStatsUnusedSAPkts    = 34,
    xflowMacsecSecyRxSCStatsNotUsingSAPkts  = 35,
    xflowMacsecSecyRxSCStatsLatePkts        = 36,
    xflowMacsecSecyRxSCStatsNotValidPkts    = 37,
    xflowMacsecSecyRxSCStatsInvalidPkts     = 38,
    xflowMacsecSecyRxSCStatsDelayedPkts     = 39,
    xflowMacsecSecyRxSCStatsUncheckedPkts   = 40,
    xflowMacsecSecyRxSCStatsOKPkts          = 41,
    xflowMacsecSecyRxSCStatsOctetsValidated = 42,
    xflowMacsecSecyRxSCStatsOctetsDecrypted = 43,

/* Per Secure Association counters as per 802.1AE MIB */
    xflowMacsecSecyTxSAStatsProtectedPkts   = 44,
    xflowMacsecSecyTxSAStatsEncryptedPkts   = 45,
    xflowMacsecSecyRxSAStatsUnusedSAPkts    = 46,
    xflowMacsecSecyRxSAStatsNotUsingSAPkts  = 47,
    xflowMacsecSecyRxSAStatsNotValidPkts    = 48,
    xflowMacsecSecyRxSAStatsInvalidPkts     = 49,
    xflowMacsecSecyRxSAStatsOKPkts          = 50,

/* Additional New Macsec MIB counters (non-RFC MIB) */
    xflowMacsecInMgmtPkts                   = 51,
    xflowMacsecFlowTcamHitCntr              = 52,
    xflowMacsecFlowTcamMissCntr             = 53,
    xflowMacsecScTcamHitCntr                = 54,
    xflowMacsecScTcamMissCntr               = 55,
    xflowMacsecOutMgmtPkts                  = 56,
    xflowMacsecInPacketDropCntr             = 57,
    xflowMacsecOutPacketDropCntr            = 58,
    xflowMacsecBadOlpHdrCntr                = 59,
    xflowMacsecBadSvtagHdrCntr              = 60,
    xflowMacsecUnctrlPortInKayPkts          = 61,
    xflowMacsecIcvFailPktsCntr              = 62,
    xflowMacsecMtuFailPktsCntr              = 63,

/*  New IPSec MIB counters  */
    xflowMacsecIPsecCtrlPortDummyPkts       = 64,
    xflowMacsecIPsecCtrlPortIPLengthMismatch= 65,
    xflowMacsecIPsecTxOutErrors             = 66,
    xflowMacsecIPsecUnctrlPortInIkePkts     = 67,
    xflowMacsecIPsecSecyRxNoSPIPkts         = 68,
    xflowMacsecIPsecSecyRxIPFragmentsSetPkts= 69,
    xflowMacsecIPSecSecyRxIllegalNxtHdrPkts = 70,
    xflowMacsecIPSecRxNoSAPkts              = 71,
    xflowMacsecIPSecRxSADummyPkts           = 72,
    xflowMacsecIPSecRxSAPadMismatchPkts     = 73,

    xflowMacsecStatTypeCount                = 74,
} xflow_macsec_stat_type_t;

typedef enum xflow_macsec_secure_assoc_an_control_e {
    xflowMacsecSecureAssocAnNormal = 0,
    xflowMacsecSecureAssocAnRollover,
    xflowMacsecSecureAssocAnAuto,
    xflowMacsecSecureAssocAnCount /* Always last not to be used */
} xflow_macsec_secure_assoc_an_control_t;

typedef enum xflow_macsec_secure_chan_vxlansec_hdr_e {
    /*
     * No change in any Header fields.
     */
    xflowMacsecSecureChanVxLanSecHdrNoChange        = 0,

    /*
     * Update Length Fields in L3 and L4 Header based on Packet_Type.
     * No update to Dest Port Field in L4 Header. L3 and L4 checksums replaced with 0.
     */
    xflowMacsecSecureChanVxLanSecHdrUpdateLength    = 1,

    /*
     * Update Dest Port Field in L4 Header. No Length Fields update.
     * L4 checksum replaced with 0.
     */
    xflowMacsecSecureChanVxLanSecHdrUpdateDestPort  = 2,

    /*
     * Update Length Fields in L3 and L4 Header based on Packet_Type.
     * Update Dest Port Field in L4 Header. L3 and L4 checksums replaced with 0.
     */
    xflowMacsecSecureChanVxLanSecHdrUpdateAll       = 3,

    /*
     * Max value for enum. Not to be used.
     */
    xflowMacsecSecureChanVxLanSecHdrCount           = 4,

} xflow_macsec_secure_chan_vxlansec_hdr_t;

typedef enum xflow_macsec_secure_chan_vxlansec_pkt_type_e {
    /* IPv4 with no VLAN tag. */
    xflowMacsecSecureChanVxLanSecIPv40Vlan      = 0,

    /* IPv4 with single VLAN tag. */
    xflowMacsecSecureChanVxLanSecIPv41Vlan      = 1,

    /* IPv4 with double VLAN tag. */
    xflowMacsecSecureChanVxLanSecIPv42Vlan      = 2,

    /* Reserved. */
    xflowMacsecSecureChanReserved0              = 3,

    /* IPv6 with no VLAN tag. */
    xflowMacsecSecureChanVxLanSecIPv60Vlan      = 4,

    /* IPv6 with single VLAN tag. */
    xflowMacsecSecureChanVxLanSecIPv61Vlan      = 5,

    /* IPv6 with double VLAN tag. */
    xflowMacsecSecureChanVxLanSecIPv62Vlan      = 6,

    /* Reserved. */
    xflowMacsecSecureChanReserved1              = 7,

    /* TCP over IPv4 with no VLAN tag. */
    xflowMacsecSecureChanVxLanSecIPv4TCP0Vlan   = 8,

    /* TCP over IPv4 with single VLAN tag. */
    xflowMacsecSecureChanVxLanSecIPv4TCP1Vlan   = 9,

    /* TCP over IPv4 with double VLAN tag. */
    xflowMacsecSecureChanVxLanSecIPv4TCP2Vlan   = 10,

    /* Reserved. */
    xflowMacsecSecureChanReserved2              = 11,

    /* UDP over IPv4 with no VLAN tag. */
    xflowMacsecSecureChanVxLanSecIPv4UDP0Vlan   = 12,

    /* UDP over IPv4 with single VLAN tag. */
    xflowMacsecSecureChanVxLanSecIPv4UDP1Vlan   = 13,

    /* UDP over IPv4 with double VLAN tag. */
    xflowMacsecSecureChanVxLanSecIPv4UDP2Vlan   = 14,

    /* Reserved. */
    xflowMacsecSecureChanReserved3              = 15,

    /* TCP over IPv6 with no VLAN tag. */
    xflowMacsecSecureChanVxLanSecIPv6TCP0Vlan   = 16,

    /* TCP over IPv6 with single VLAN. */
    xflowMacsecSecureChanVxLanSecIPv6TCP1Vlan   = 17,

    /* TCP over IPv6 with double VLAN. */
    xflowMacsecSecureChanVxLanSecIPv6TCP2Vlan   = 18,

    /* Reserved. */
    xflowMacsecSecureChanReserved4              = 19,

    /* UDP over IPv6 with no VLAN. */
    xflowMacsecSecureChanVxLanSecIPv6UDP0Vlan   = 20,

    /* UDP over IPv6 with single VLAN. */
    xflowMacsecSecureChanVxLanSecIPv6UDP1Vlan   = 21,

    /* UDP over IPv6 with double VLAN. */
    xflowMacsecSecureChanVxLanSecIPv6UDP2Vlan   = 22,

    /* Reserved. */
    xflowMacsecSecureChanReserved5              = 23,
    xflowMacsecSecureChanReserved6              = 24,
    xflowMacsecSecureChanReserved7              = 25,
    xflowMacsecSecureChanReserved8              = 26,
    xflowMacsecSecureChanReserved9              = 27,
    xflowMacsecSecureChanReserved10             = 28,
    xflowMacsecSecureChanReserved11             = 29,
    xflowMacsecSecureChanReserved12             = 30,

    /* Not a VxLANSec/CloudSec packet. */
    xflowMacsecSecureChanVxLanSecNoVxLanSec     = 31,

    /* Max value of enum. */
    xflowMacsecSecureChanVxLanSecCount          = 32

} xflow_macsec_secure_chan_vxlansec_pkt_type_t;

typedef enum xflow_macsec_flow_frame_e {
    xflowMacsecFlowFrameEII     = 0,
    xflowMacsecFlowFrameSnap    = 1,
    xflowMacsecFlowFrameLlc     = 2,
    xflowMacsecFlowFrameMpls    = 3,
    xflowMacsecFlowFramePBB     = 4,
    xflowMacsecFlowFrameVNTag   = 5,
    xflowMacsecFlowFrameETag    = 6,
    xflowMacsecFlowFrameIPv4    = 7,
    xflowMacsecFlowFrameUDPIPv4 = 8,
    xflowMacsecFlowFrameTCPIPv4 = 9,
    xflowMacsecFlowFrameIPv6    = 10,
    xflowMacsecFlowFrameUDPIPv6 = 11,
    xflowMacsecFlowFrameTCPIPv6 = 12,
    xflowMacsecFlowFrameAny     = 13,

    /* Always last not to be used */
    xflowMacsecFlowFrameCount = 14
} xflow_macsec_flow_frame_type_t;

/*
 * Enums describing the various events generated by Xflow Macsec. These events
 * are captured using the callback mechanism registered using the API
 * bcm_xflow_macsec_event_register.
 */
typedef enum {
    /*
     * SA Soft Expire event. This indicates that the PN for a given SA has
     * hit the threshold value configured.
     * The index id has to be typecasted to bcm_xflow_macsec_secure_assoc_id_t.
     */
    xflowMacsecEventSASoftExpire = 0,

    /*
     * SA Expire event. This indicates that the PN for a given SA has
     * hit the maximum value possible. For AES algorithm, this is 2^32.
     * For AES XPN algorithm, this is 2^64.
     * The index id has to be typecasted to bcm_xflow_macsec_secure_assoc_id_t.
     */
    xflowMacsecEventSAExpire,

    /*
     * SA PN/XPN Min Expire event.
     * This indicates that the incoming SA PN/XPN is outside the SC
     * replay_protect_window value. The index id has to be typecasted to
     * bcm_xflow_macsec_secure_assoc_id_t.
     * Applicable only for decrypt case.
     */
    xflowMacsecEventSAMinExpire,

    /* ICV failure event.
     * The index id has to be typecasted to bcm_xflow_macsec_secure_assoc_id_t.
     * Applicable only for decrypt case.
     * Not applicable for inline xflow-macsec
     */
    xflowMacsecEventICVFailure,

    xflowMacsecEventCount
} xflow_macsec_event_t;

typedef enum xflow_macsec_mac_addr_control_e {

    /* The following enums are applicable only for the decrypt case. */

    /*
     *  MACDA to be classified as a rudimentary management packet.
     *  Argument mac_addr specifies the MACDA. Argument value is ignored.
     */
    xflowMacsecMgmtDstMac0          = 0,

    /*
     *  MACDA to be classified as a rudimentary management packet.
     *  Argument mac_addr specifies the MACDA. Argument value is ignored.
     */
    xflowMacsecMgmtDstMac1          = 1,

    /*
     *  MACDA to be classified as a rudimentary management packet.
     *  Argument mac_addr specifies the MACDA. Argument value is ignored.
     */
    xflowMacsecMgmtDstMac2          = 2,

    /*
     *  MACDA to be classified as a rudimentary management packet.
     *  Argument mac_addr specifies the MACDA. Argument value is ignored.
     */
    xflowMacsecMgmtDstMac3          = 3,

    /*
     *  MACDA to be classified as a rudimentary management packet.
     *  Argument mac_addr specifies the MACDA. Argument value is ignored.
     */
    xflowMacsecMgmtDstMac4          = 4,

    /*
     *  MACDA to be classified as a rudimentary management packet.
     *  Argument mac_addr specifies the MACDA. Argument value is ignored.
     */
    xflowMacsecMgmtDstMac5          = 5,

    /*
     *  MACDA to be classified as a rudimentary management packet.
     *  Argument mac_addr specifies the MACDA. Argument value is ignored.
     */
    xflowMacsecMgmtDstMac6          = 6,

    /*
     *  MACDA to be classified as a rudimentary management packet.
     *  Argument mac_addr specifies the MACDA. Argument value is ignored.
     */
    xflowMacsecMgmtDstMac7          = 7,

    /*
     * Lower limit in a range of MACDA to be classified as a management packet.
     * Argument mac_addr specifies the MACDA. Argument value is ignored.
     */
    xflowMacsecMgmtDstMacRangeLow   = 8,

    /*
     * Higher limit in a range of MACDA to be classified as a management packet.
     * Argument mac_addr specifies the MACDA. Argument value is ignored.
     */
    xflowMacsecMgmtDstMacRangeHigh  = 9,

    /*
     * First set of MACDA and Ethertype to be classified as a
     * management packet.
     * Argument mac_addr specifies the MACDA.
     * Argument value specifies the ethertype.
     */
    xflowMacsecMgmtDstMacEthertype0 = 10,

    /*
     * Second set of MACDA and Ethertype to be classified as a management packet.
     * Argument mac_addr specifies the MACDA.
     * Argument value specifies the ethertype.
     */
    xflowMacsecMgmtDstMacEthertype1 = 11,

    /*
     * Station mac addressed to be used when a dropped packet is to be copied
     * to cpu.
     * Argument mac_addr specifies the station MACDA. Argument value is ignored.
     * Not valid for Inline Xflow Macsec.
     */
    xflowMacsecStationDstMac        = 12,

    /* Max value. */
    xflowMacsecMacAddrControlCount = 13
} xflow_macsec_mac_addr_control_t;

typedef enum xflow_macsec_port_control_e {
    /*
     * The enums below are applicable only for the Inline Xflow Macsec architecture.
     * The following enums are applicable only for the decrypt case.
     */

    /* Select from the configured Ethertypes. */
    xflowMacsecPortSectagEtypeSel           = 1,

    /* Configure Macsec version for frame validation. */
    xflowMacsecPortSectagVersion            = 2,

    /*
     * Enable validation rules for ingress matching. Select among the following flags:
     *    XFLOW_MACSEC_VALIDATE_SECTAG_VERSION_INVALID
     *    XFLOW_MACSEC_VALIDATE_SECTAG_SHORT_LEN_MAX
     *    XFLOW_MACSEC_VALIDATE_SECTAG_SHORT_LEN_UNSET
     *    XFLOW_MACSEC_VALIDATE_SECTAG_SHORT_LEN_MISMATCH
     *    XFLOW_MACSEC_VALIDATE_SECTAG_E0_C1
     *    XFLOW_MACSEC_VALIDATE_SECTAG_ES1_SC1
     *    XFLOW_MACSEC_VALIDATE_SECTAG_SC1_SCB1
     *    XFLOW_MACSEC_VALIDATE_SECTAG_PN0
     */
    xflowMacsecPortSectagRuleEnable         = 3,


    /*
     * Enable configured TPID values. Select among the following flags.
     *    XFLOW_MACSEC_FLOW_TPID_SEL_0
     *    XFLOW_MACSEC_FLOW_TPID_SEL_1
     *    XFLOW_MACSEC_FLOW_TPID_SEL_2
     *    XFLOW_MACSEC_FLOW_TPID_SEL_3
     *    XFLOW_MACSEC_FLOW_TPID_SEL_4
     */
    xflowMacsecPortTPIDEnable               = 4,

    /* Per port enable for PBB packet identification. */
    xflowMacsecPortPBBEnable                = 5,

    /*
     * Enable configured MPLS Ethertype. Select among the following flags:
     *     XFLOW_MACSEC_MPLS_ETYPE_0
     *     XFLOW_MACSEC_MPLS_ETYPE_1
     *     XFLOW_MACSEC_MPLS_ETYPE_2
     *     XFLOW_MACSEC_MPLS_ETYPE_3
     */
    xflowMacsecPortMPLSEnable               = 6,

    /* Per port enable for IPv4 ethertype. */
    xflowMacsecPortIPv4EtypeEnable          = 7,

    /* Per port enable for IPv6 ethertype. */
    xflowMacsecPortIPv6EtypeEnable          = 8,

    /* Per port enable for PTP ethertype. */
    xflowMacsecPortPTPEtypeEnable           = 9,

    /* Per port enable for VNTAG ethertype. */
    xflowMacsecPortNIVEtypeEnable           = 10,

    /* Per port enable for PE/ETAG ethertype. */
    xflowMacsecPortPEEtypeEnable            = 11,

    /* Per port enable for UDP protocol packet. */
    xflowMacsecPortUDPEnable                = 12,

    /* Per port enable for TCP protocol packet. */
    xflowMacsecPortTCPEnable                = 13,

    /* Per port enable for PTP destination port number match. */
    xflowMacsecPortPTPDestPortEnable        = 14,

    /*
     * Per-port enable for PTP packet identification types.
     * Select among the following flags.
     *     XFLOW_MACSEC_MATCH_PTP_UNTAGGED
     *     XFLOW_MACSEC_MATCH_PTP_ONE_VLAN
     *     XFLOW_MACSEC_MATCH_PTP_UDP_IPV4
     *     XFLOW_MACSEC_MATCH_PTP_UDP_IPV6
     */
    xflowMacsecPortPTPMatchRuleEnable       = 15,

    /*
     * If enabled, 2B after IPV4 header is matched with SecTag Ethertype.
     * Otherwise, Sectag EtherType is not checked after IPv4 header.
     */
    xflowMacsecPortSectagAfterIPv4Enable    = 16,

    /*
     * If enabled, 2B after IPV6 header is matched with SecTag Ethertype.
     * Otherwise, Sectag EtherType is not checked after IPv6 header.
     */
    xflowMacsecPortSectagAfterIPv6Enable    = 17,

    /*
     * If enabled, 2B after TCP header is matched with SecTag Ethertype.
     * Otherwise, Sectag EtherType is not checked after TCP header.
     */
    xflowMacsecPortSectagAfterTCPEnable     = 18,

    /*
     * If enabled, 2B after UDP header is matched with SecTag Ethertype.
     * Otherwise, Sectag EtherType is not checked after UDP header.
     */
    xflowMacsecPortSectagAfterUDPEnable     = 19,

    /*
     * If enabled, IPv4 checksum is checked for secure data packets
     * (MACSEC packets). If failed, the packet is purged.
     * Otherwise, IPV4 checksum is not checked.
     */
    xflowMacsecPortIPv4ChecksumEnable       = 20,

    /*
     * If enabled and if UDP destination port matches the programmed value,
     * IPv6 with UDP packet Flags and VNI information is extracted and used for
     * decrypt flow match.
     */
    xflowMacsecPortVxLANIpv6UDPVNIMatchEnable = 21,

    /*
     * The value of per port MTU (maximum transmission unit) for ingress management,
     * KaY and SP TCAM Miss packets. A packet will be marked as an error packet if
     * its (ingress) size is greater than MTU. This value doesn't include the CRC bytes.
     * This must be set to a value less than or equal to 2^14-1 minus 16 and
     * greater than or equal to 196.
     */
    xflowMacsecPortMTU                      = 22,

    /*
     * Enable the following rules used in rudimentary management packet detection.
     * Select among the following flags.
     *     XFLOW_MACSEC_MGMT_DEST_MAC_0X0180C200000
     *     XFLOW_MACSEC_MGMT_DEST_MAC_0X01000CCCCCCC
     *     XFLOW_MACSEC_MGMT_DEST_MAC
     *     XFLOW_MACSEC_MGMT_ETYPE
     *     XFLOW_MACSEC_MGMT_DEST_MAC_RANGE
     *     XFLOW_MACSEC_MGMT_DEST_MAC_ETYPE
     *     XFLOW_MACSEC_MGMT_E1_C0
     *     XFLOW_MACSEC_MGMT_PTP
     *     XFLOW_MACSEC_MGMT_IPSEC_UDP_DST_PORT_WITH_NON_ESP
     *     XFLOW_MACSEC_MGMT_IPSEC_UDP_DST_PORT_WITHOUT_NON_ESP
     *     XFLOW_MACSEC_MGMT_IPSEC_UDP_NAT_PORT
     */
    xflowMacsecPortMgmtPktRulesEnable       = 23,

    /*
     * The default sub port assigned if management packet is detected
     * by the enabled rules for the port.
     * Subport ID is passed in the value argument.
     */
    xflowMacsecPortMgmtDefaultSubPort       = 24,
    /*
     * Relevant for Encrypt Only:
     * If working in this mode for this port, the SC Index is derived
     * by the core and is equal to the egress port number
     */
    xflowMacsecPortBasedScEnable            = 25,

    /*
     * Used for enabling/disabling MACSEC to this logical port.
    */
    xflowMacsecPortEnable                   = 26,

    /* Enable ESP IPsec packet detection. */
    xflowMacsecIPsecPortEsp                 = 27,

    /* Enable UDP encapsulated ESP IPsec packet detection. */
    xflowMacsecIPsecPortEspUdp              = 28,

    /* Enable to allow sequence number (SN) value in ESP to be 0. */
    xflowMacsecIPsecPortEspSnZero           = 29,

    /* Enable to allow security parameter index (SPI) field in ESP to be 0. */
    xflowMacsecIPsecPortEspSpiZero          = 30,

    /* Enable to allow SPI field in ESP to be in the range 1 to 255. */
    xflowMacsecIPsecPortEspSpi1To255        = 31,

    /* Enable to allow IPsec IPv4 fragment.  */
    xflowMacsecIPsecPortIPv4Fragment        = 32,

    /* Enable UDP encapsulated ESP packet detection using UDP source port.  */
    xflowMacsecIPsecPortEspUdpSrcPort       = 33,

    /* Enable NAT keepalive packet detection.  */
    xflowMacsecIPsecPortNatKeepalive        = 34,

    /* Maximum enum value. */
    xflowMacsecPortCount                    = 35

} xflow_macsec_port_control_t;

typedef struct xflow_macsec_instance_pbmp_t {
    /* Xflow Macsec Instance ID. */
    xflow_macsec_instance_id_t macsec_instance_id;

} xflow_macsec_instance_pbmp_t;

typedef struct xflow_macsec_secure_chan_info_s {
    /* Secure channel Identifier. */
    uint64 sci;

    /*
     * The crypto suite used in Macsec.
     */
    xflow_macsec_crypto_t crypto;

    /* Specifies in number of bytes after SecTag where encryption/decryption
     * starts.*/
    uint32 confidentiality_offset;

    /*
     * The active association number.
     * For Inline Xflow Macsec, this overrides the TCI.an bits and
     * is an encrypt only parameter.
     */
    uint32 active_an;

    uint32 flags;

    /*
     * Update the L3,L4 Length fields, L3,L4 Checksum fields and L4 dest port
     * Fields in the corresponding headers for VXLANSec or L3, L4 packets.
     * Applicable only for Inline Xflow Macsec architecture.
     */
    xflow_macsec_secure_chan_vxlansec_hdr_t  vxlansec_hdr_update;

    /*
     * Specify the number of bytes from the start of the packet
     * where the first  authentication range starts.
     * For Inline Xflow Macsec, configured value must be
     * less than the SECTAG offset.
     */
    uint8 first_auth_range_offset_start;

    /*
     * Specify the number of bytes after first authentication range start
     * where the first authentication range ends.
     * A value of 0 will be treated as range not configured.
     * For Inline Xflow Macsec (Encrypt), sum of first_auth_range_offset_start
     * and first_auth_range_offset_end should be less than
     * the SECTAG offset.
     */
    uint8 first_auth_range_offset_end;

    /* Encrypt Only Parameters */

    /*
     * Specifies the AN behavior upon expiry. (Encrypt Only).
     * Applicable only for Inline Xflow Macsec architecture.
     */
    xflow_macsec_secure_assoc_an_control_t an_control;

    /* Tag Control Information (Encrypt only)*/
    uint8 tci;

    /* Specifies in number of bytes from start of frame where
     * to insert the SecTAG. Not used for decrypt. (Encrypt only)*/
    uint32 sectag_offset;

    /*
     * The selection of one of the four MTU Configuration
     * value (Encrypt only).
     * Not applicable to Inline Xflow Macsec.
     */
    xflow_macsec_mtu_t mtu_sel;

    /*
     * The allowed Maximum Transmission Unit(MTU) for egress encrypted
     * packets. Applicable only for Inline Xflow Macsec architecture.
     */
    uint32 mtu;

    /* The selection of one of the four Macsec Ethertype
     * configuration value (Encrypt only)*/
    xflow_macsec_sectag_ethertype_t sectag_etype_sel;

    /*
     * Destination port only for Encrypt flow.
     * Not applicable to Inline Xflow Macsec.
     */
    bcm_port_t dest_port;

    /*
     * VxLANSec or L3, L4 packet type. Applicable only for Encrypt
     * Inline Xflow Macsec architecture.
     */
    xflow_macsec_secure_chan_vxlansec_pkt_type_t vxlansec_pkt_type;

    /*
     * Used to set the profile for soft expiry threshold
     * selection.
     */
    uint8 soft_expiry_threshold_select;

    /* Decrypt only parameters */

    /* SCI Mask. (Decrypt only) */
    uint64 sci_mask;

    /* Only for decrypt path. Replay Protection Window Size. If the size is 0,
     * the strict replay check will be imposed. (Decrypt only)*/
    uint32 replay_protect_window_size;

    /* Policy id used as a match criteria in the SC TCAM. (Decrypt only)*/
    xflow_macsec_policy_id_t policy_id;

} xflow_macsec_secure_chan_info_t;

typedef struct xflow_macsec_port_info_s {
    /*
     * 0 - Disable MACSEC
     * 1 - Enable MACSEC
     * Not applicable to Inline Xflow Macsec.
     */
    uint8 enable;

    /*
     * Encryption Port offset 0 or 1.
     * Use different offsets to load balance between the flows.
     * Not applicable to Inline Xflow Macsec.
     */
    uint8 encrypt_port_offset;

    /* Port control parameter. */
    uint64 value;
} xflow_macsec_port_info_t;

typedef struct bcm_xflow_macsec_crypto_aes128_gcm_s {
    uint8       key[XFLOW_MACSEC_CRYPTO_AES128GCM_KEY_SIZE];
} xflow_macsec_crypto_aes128_gcm_t;

typedef struct bcm_xflow_macsec_crypto_aes256_gcm_s {
    uint8       key[XFLOW_MACSEC_CRYPTO_AES256GCM_KEY_SIZE];
} xflow_macsec_crypto_aes256_gcm_t;

typedef struct xflow_macsec_secure_assoc_info_s {
    /* Association number */
    uint8 an;

    /* The AES key passed. Use AES128 for Monterey. */
    xflow_macsec_crypto_aes128_gcm_t aes;

    /*
     * Specifies the AN behavior upon expiry. (Encrypt Only)
     * Not applicable for Inline Xflow Macsec.
     */
    xflow_macsec_secure_assoc_an_control_t an_control;

    /* The next packet sequence number. */
    uint32 next_pkt_num;

    /*
     * The higher 32 bits in the next packet sequence number.
     * This is used only for AES-GCM-XPN algorithm.
     */
    uint32 next_pkt_num_upper;

    /* SALT Parameter for AES-GCM-XPN algorithm. */
    uint8 salt[12];

    /* SSCI parameter for AES-GCM-XPN algorithm. */
    uint32 ssci;
	
	uint8 hash[16];

    uint32 flags;

    /* AES-256 key. */
    xflow_macsec_crypto_aes256_gcm_t aes_256;

    /*
     * Enable the secure association.
     * Applicable only for Inline Xflow Macsec.
     */
    uint8 enable;
} xflow_macsec_secure_assoc_info_t;

typedef struct xflow_macsec_policy_info_s {
    uint32 flags;

    /* The sci value to be used in operPointToPointMAC mode. */
    uint64 sci;

    /*
     * The MTU value to be selected.
     * Not valid for Inline Xflow Macsec.
     */
    xflow_macsec_mtu_t mtu_sel;

    /* The sectag Ethertype to be used for decoding. */
    xflow_macsec_sectag_ethertype_t etype_sel;

    /* The following fields are applicable only to the Inline Xflow Macsec architecture. */

    /* The MTU used for ingress MTU check on this sub-port. */
    uint32 mtu;

    /*
     * Location of the SecTAG in the packet.
     * For Inline Xflow Macsec, it is in terms of the number of the bytes
     * from the start of packet.
     * For other architectures, this is in terms of number of bytes
     * after the last MPLS label.
     * This location cannot guarantee a SecTAG will be found.
     * If the SecTAG ETYPE is not found,
     * the packet is treated as a non-MACsec packet.
     */
    uint32 sectag_offset;

    /*
     * Validate tagged frames for errors and policy violations.
     */
    xflow_macsec_tag_validate_t tag_validate;

    /*
     * Location location of the Inner DA in terms of the number of the bytes.
     * Applicable only when XFLOW_MACSEC_DECRYPT_POLICY_INNER_L2_VALID is
     * configured.
     */
    uint32 inner_l2_offset;

    /*
     * Source port.
     * Not applicable for Inline Xflow Macsec.
     */
    bcm_port_t port;

    /*
     * Location of the ESP header in the packet. in terms of the number of the bytes
     * from the start of the packet. Applicable only for IPsec (MPLS over ESP header).
     * Applicable only if XFLOW_MACSEC_IPSEC_DECRYPT_POLICY
     * is configured.
     */
    uint32 ipsec_esp_offset;

    /*
     * Location location of the outer IP header in terms of the number of the bytes.
     * Applicable only when
     * XFLOW_MACSEC_IPSEC_DECRYPT_POLICY_OUTER_IP_VALID is
     * configured.
     * Used for MPLS over IPsec. Applicable only if
     * XFLOW_MACSEC_IPSEC_DECRYPT_POLICY is configured.
     */
    uint32 ipsec_outer_ip_offset;

} xflow_macsec_policy_info_t;

typedef struct xflow_macsec_flow_info_mpls_s {
    /* MPLS label */
    bcm_mpls_label_t mpls_label;

    /* MPLS EXP bits. */
    uint8 mpls_exp;

    /* MPLS S bit. */
    uint8 mpls_sbit;

} xflow_macsec_flow_info_mpls_t;

/*
 * Match on User defined Fields.
 * The variables selected for the UDF key will depend on the
 * frame type specified in the flow_info structure.
 * Please refer to the Xflow Macsec documentation to know
 * the mapping.
 */
typedef struct xflow_macsec_flow_udf_param_s {
    /*
     * Ethertype for Ethernet II/SNAP packets.
     * {DSAP, SSAP} for LLC packets. Not valid for Inline Xflow Macsec.
     */
    uint32 ethertype;

    /* First VLAN ID. */
    bcm_vlan_t first_vlan;

    /* First VLAN CFI. */
    uint8 first_vlan_cfi;

    /* First VLAN priority. */
    uint8 first_vlan_priority;

    /* Second VLAN ID. */
    bcm_vlan_t second_vlan;

    /* Second VLAN CFI. */
    uint8 second_vlan_cfi;

    /* Second VLAN priority. */
    uint8 second_vlan_priority;

    /* Third VLAN ID. */
    bcm_vlan_t third_vlan;

    /* Third VLAN CFI. */
    uint8 third_vlan_cfi;

    /* Third VLAN priority. */
    uint8 third_vlan_priority;

    /* Fourth VLAN ID. */
    bcm_vlan_t fourth_vlan;

    /* Fourth VLAN CFI. */
    uint8 fourth_vlan_cfi;

    /* Fourth VLAN priority. */
    uint8 fourth_vlan_priority;

    /* IP protocol ID. */
    uint8 protocol_id;

    /* Source IPv4 or IPv6 address. */
    bcm_ipv4_ipv6_addr_t sip_addr;

    /* Destination IPv4 or IPv6 address. */
    bcm_ipv4_ipv6_addr_t dip_addr;

    /* UDP/TCP source port. */
    uint32 source_port;

    /* UDP/TCP dest port. */
    uint32 dest_port;

    /* Source Mac address. */
    bcm_mac_t outer_src_mac;

    /* Destination Mac address. */
    bcm_mac_t outer_dst_mac;

    /* Inner source Mac address. */
    bcm_mac_t inner_src_mac;

    /* Inner destination Mac address. */
    bcm_mac_t inner_dst_mac;

    /* Inner first VLAN ID */
    bcm_vlan_t inner_first_vlan;

    /* Inner first VLAN CFI. */
    uint8 inner_first_vlan_cfi;

    /* Inner first VLAN priority. */
    uint8 inner_first_vlan_priority;

    /* Inner second VLAN ID */
    bcm_vlan_t inner_second_vlan;

    /* Inner second VLAN CFI. */
    uint8 inner_second_vlan_cfi;

    /* Inner second VLAN priority. */
    uint8 inner_second_vlan_priority;

    /* BB Tag Vlan ID + PCP */
    uint8 bbtag_vid_pcp[2];

    /* ITAG PCP + ISID */
    uint8 itag_pcp_isid[4];

    /* ETag TCI. */
    uint8 etag_tci[6];

    /* First MPLS label. */
    xflow_macsec_flow_info_mpls_t first_mpls;

    /* Second MPLS label. */
    xflow_macsec_flow_info_mpls_t second_mpls;

    /* Third MPLS label. */
    xflow_macsec_flow_info_mpls_t third_mpls;

    /* Fourth MPLS label. */
    xflow_macsec_flow_info_mpls_t fourth_mpls;

    /* Free form payload after the defined headers.*/
    /* Payload starts from 0. Max index depends on device. */
    uint8 payload[64];

} xflow_macsec_flow_udf_param_t;

typedef struct xflow_macsec_flow_info_s {
    uint32 flags;

    /* The policy index that this rule points to. */
    xflow_macsec_policy_id_t policy_id;

    /* The rule is used to identify a management packet. */
    uint8 set_management_pkt;

    /*
     * Match on user defined fields.
     * The validity of the fields defined will depend on the
     * device used. Please refer the Xflow Macsec documentation for the target
     * device.
     */
    xflow_macsec_flow_udf_param_t udf;

    xflow_macsec_flow_udf_param_t udf_mask;

    /* Match on Decrpt flow type. Not applicable for Inline Xflow Macsec. */
    xflow_macsec_flow_pkt_type_t pkt_type;

    /* Match on packet TPID(s)
     * Valid only when xflow_macsec_skip_decrypt_pkt_parser
     * is set to 0.
     * Use flags defined as BCM_XFLOW_MACSEC_FLOW_TPID_SEL_X
     * to set tpid and tpid_mask.
     * Not applicable for Inline Xflow Macsec.
     */
    uint32 tpid;

    uint32 tpid_mask;

    /*
     * Match on VLAN/MPLS TAG status
     * Valid only when xflow_macsec_skip_decrypt_pkt_parser
     * is set to 1.
     * Not applicable for Inline Xflow Macsec.
     */
    xflow_macsec_vlan_mpls_tag_status_t vlan_mpls_tag_status;

    /* Match on Packet ethertype. Not applicable for Inline Xflow Macsec. */
    xflow_macsec_flow_etype_t etype_sel;

    /*
     * Match on Packet ingress port.
     * Not applicable for Inline Xflow Macsec.
     */
    bcm_port_t src_port;

    /* Mask for ingress port. Not applicable for Inline Xflow Macsec. */
    uint32 src_port_mask;

    /* Match on Packet src_mac. Not applicable for Inline Xflow Macsec. */
    bcm_mac_t src_mac;

    bcm_mac_t src_mac_mask;

    /* Match on Packet dst_mac. Not applicable for Inline Xflow Macsec. */
    bcm_mac_t dst_mac;

    bcm_mac_t dst_mac_mask;

    /* The following fields are applicable only for Inline Xflow Macsec. */

    /*
     * VLAN Tag status for the non-MPLS packets and MPLS label status for
     * MPLS/EoMPLS packets. MPLS packets are selected when frame_type is
     * xflowMacsecFlowFrameMpls. Applicable only for Inline Xflow Macsec.
     * Select among the following flags.
     *     XFLOW_MACSEC_NO_TAGS_NO_LABELS
     *     XFLOW_MACSEC_1_VLAN_TAG_1_MPLS_LABEL
     *     XFLOW_MACSEC_2_VLAN_TAG_2_MPLS_LABEL
     *     XFLOW_MACSEC_3_VLAN_TAG_3_MPLS_LABEL
     *     XFLOW_MACSEC_4_VLAN_TAG_4_MPLS_LABEL
     *     XFLOW_MACSEC_GREATER_4_VLAN_TAG_5_MPLS_LABEL
     * Applicable only for Inline Xflow Macsec.
     */
    uint32 vlan_tag_mpls_label_flags;

    /*
     * Match on packet format.
     * Applicable only for Inline Xflow Macsec.
     */
    xflow_macsec_flow_frame_type_t frame_type;

    /*
     * Match on port bitmap. Multiple ports can be configured to match on
     * packets incoming from any of the ports.
     * Applicable only for Inline Xflow Macsec.
     */
    //bcm_pbmp_t src_pbm;

    /*
     * If set to 1, current packet is IPSec flow secure packet.
     * Otherwise, it belongs to MACSec flow.
     */
    uint8  is_ipsec;

} xflow_macsec_flow_info_t;

typedef struct xflow_macsec_vlan_tpid_s {
    int enable[XFLOW_MACSEC_VLAN_TPID_MAX];

    /* Configure TPID for MACSEC : 0-outermost, MAX-innermost */
    uint32 tpid[XFLOW_MACSEC_VLAN_TPID_MAX];
} xflow_macsec_vlan_tpid_t;

typedef struct xflow_macsec_mac_addr_info_s {
    /* Mac address. */
    bcm_mac_t mac_addr;

    /*
     * Ethertype to be matched. Used only when specified in the type
     * description.
     */
    bcm_ethertype_t ethertype;
} xflow_macsec_mac_addr_info_t;


typedef struct xflow_macsec_port_map_info_s {
    /*
    * MACSec instance id
    */
    xflow_macsec_instance_id_t macsec_instance_id;

    /*
    * MACSec port number  in the instance
    */
   int macsec_port;
} xflow_macsec_port_map_info_t;

typedef struct xflow_macsec_handle_info_s {
    /*
     * MACSec handle id type
     */
    xflow_macsec_index_type_t type;

    /*
     * Handle direction. encrypt/decrypt corresponds to egress/ingress flow
     */
    int dir;

    /*
     * MACSec block id
     */
    int instance_id;

    /*
     * HW index in table
     */
    int hw_index;

} xflow_macsec_handle_info_t;


typedef struct xflow_macsec_svtag_cpu_flex_map_param_s {

    /* Ingress Port bitmap for this rule. One-hot encoded port map for the ingress port number of the packet */
    //bcm_pbmp_t port_bmp;

    /* sop error code  */
    uint32 sop_error_code;

    /* Packet type.*/
    xflow_macsec_flow_pkt_type_t  svtag_packet_type;

    /*
     * NAT Keep Alive packet detected by Per port Rudimentary rule.
     */
    uint8 nat_keep_alive_pkt;

     /*
     * If set to 1, current packet is IPSec flow secure packet. Otherwise, it belongs to MACSec flow.
     */
    uint8  ipsec;

} xflow_macsec_svtag_cpu_flex_map_param_t;

typedef struct xflow_macsec_svtag_cpu_flex_map_info_s {

    /* Match on param fields */
    xflow_macsec_svtag_cpu_flex_map_param_t param_value;

    /* mask for param fields */
    xflow_macsec_svtag_cpu_flex_map_param_t param_mask;

} xflow_macsec_svtag_cpu_flex_map_info_t;

typedef struct xflow_macsec_ipsec_sc_match_action_info_s {
    /* TCAM valid */
    int valid;

    /* TCAM priority */
    int priority;

    /* Security parameter index (match parameter). */
    uint32 spi;

    /* SPI Mask. (match parameter) */
    uint32 spi_mask;

    /* Policy id used as a match criteria in the SC TCAM (match parameter). */
    xflow_macsec_policy_id_t policy_id;

    /*
     * IPsec association number. It specifies the internal association number to
     * differentiate between the active and passive keys (Action parameter).
     */
    uint32 ipsec_an;
} xflow_macsec_ipsec_sc_match_action_info_t;

typedef enum xflow_macsec_secure_chan_l3_l4_hdr_e {
    /*
     * No change in any L3 or L4 header fields.
     */
    xflowMacsecSecureChanL3L4HdrNoChange            = 0,

    /*
     * Update Length Fields in L3 and L4 Header based on Packet_Type.
     * No update to Dest Port Field in L4 Header. L3 and L4 checksums replaced with 0.
     */
    xflowMacsecSecureChanL3L4HdrUpdateLength        = 1,

    /*
     * Update Dest Port Field in L4 Header. No Length Fields update.
     * L4 checksum replaced with 0.
     */
    xflowMacsecSecureChanL3L4HdrUpdateDestPort   = 2,

    /*
     * Update Length Fields in L3 and L4 Header based on Packet_Type.
     * Update Dest Port Field in L4 Header. L3 and L4 checksums replaced with 0.
     */
    xflowMacsecSecureChanL3L4HdrUpdateAll           = 3,

    /*
     * Max value for enum. Not to be used.
     */
    xflowMacsecSecureChanL3L4HdrCount               = 4,

} xflow_macsec_secure_chan_l3_l4_hdr_t;

typedef enum xflow_macsec_secure_chan_l3_l4_pkt_type_e {
    /* IPv4 with no VLAN tag. */
    xflowMacsecSecureChanL3L4IPv40Vlan          = 0,

    /* IPv4 with single VLAN tag. */
    xflowMacsecSecureChanL3L4IPv41Vlan          = 1,

    /* IPv4 with double VLAN tag. */
    xflowMacsecSecureChanL3L4IPv42Vlan          = 2,

    /* Reserved. */
    xflowMacsecSecureChanL3L4Reserved0          = 3,

    /* IPv6 with no VLAN tag. */
    xflowMacsecSecureChanL3L4IPv60Vlan          = 4,

    /* IPv6 with single VLAN tag. */
    xflowMacsecSecureChanL3L4IPv61Vlan          = 5,

    /* IPv6 with double VLAN tag. */
    xflowMacsecSecureChanL3L4IPv62Vlan          = 6,

    /* Reserved. */
    xflowMacsecSecureChanL3L4Reserved1          = 7,

    /* TCP over IPv4 with no VLAN tag. */
    xflowMacsecSecureChanL3L4IPv4TCP0Vlan       = 8,

    /* TCP over IPv4 with single VLAN tag. */
    xflowMacsecSecureChanL3L4IPv4TCP1Vlan       = 9,

    /* TCP over IPv4 with double VLAN tag. */
    xflowMacsecSecureChanL3L4IPv4TCP2Vlan       = 10,

    /* Reserved. */
    xflowMacsecSecureChanL3L4Reserved2          = 11,

    /* UDP over IPv4 with no VLAN tag. */
    xflowMacsecSecureChanL3L4IPv4UDP0Vlan       = 12,

    /* UDP over IPv4 with single VLAN tag. */
    xflowMacsecSecureChanL3L4IPv4UDP1Vlan       = 13,

    /* UDP over IPv4 with double VLAN tag. */
    xflowMacsecSecureChanL3L4IPv4UDP2Vlan       = 14,

    /* Reserved. */
    xflowMacsecSecureChanL3L4Reserved3          = 15,

    /* TCP over IPv6 with no VLAN tag. */
    xflowMacsecSecureChanL3L4IPv6TCP0Vlan       = 16,

    /* TCP over IPv6 with single VLAN. */
    xflowMacsecSecureChanL3L4IPv6TCP1Vlan       = 17,

    /* TCP over IPv6 with double VLAN. */
    xflowMacsecSecureChanL3L4IPv6TCP2Vlan       = 18,

    /* Reserved. */
    xflowMacsecSecureChanL3L4Reserved4          = 19,

    /* UDP over IPv6 with no VLAN. */
    xflowMacsecSecureChanL3L4IPv6UDP0Vlan       = 20,

    /* UDP over IPv6 with single VLAN. */
    xflowMacsecSecureChanL3L4IPv6UDP1Vlan       = 21,

    /* UDP over IPv6 with double VLAN. */
    xflowMacsecSecureChanL3L4IPv6UDP2Vlan       = 22,

    /* Reserved. */
    xflowMacsecSecureChanL3L4Reserved5          = 23,
    xflowMacsecSecureChanL3L4Reserved6          = 24,
    xflowMacsecSecureChanL3L4Reserved7          = 25,
    xflowMacsecSecureChanL3L4Reserved8          = 26,
    xflowMacsecSecureChanL3L4Reserved9          = 27,
    xflowMacsecSecureChanL3L4Reserved10         = 28,
    xflowMacsecSecureChanL3L4Reserved11         = 29,
    xflowMacsecSecureChanL3L4Reserved12         = 30,

    /* Not a L3 L4 packet. */
    xflowMacsecSecureChanL3L4NoL3L4             = 31,

    /* Max value of enum. */
    xflowMacsecSecureChanL3L4Count              = 32

} xflow_macsec_secure_chan_l3_l4_pkt_type_t;

typedef struct xflow_macsec_ipsec_secure_chan_info_s {

    /*
     * The crypto suite used in Macsec.
     */
    xflow_macsec_crypto_t crypto;

    /* Flags */
    uint32 flags;

    /*
     * Update the L3,L4 Length fields, L3,L4 Checksum fields and L4
     * Dest Port Fields in the corresponding headers for L3 L4 packets.
     * Applicable only for Inline Xflow Macsec architecture.
     */
    xflow_macsec_secure_chan_l3_l4_hdr_t  l3_l4_hdr_update;

    /* Encrypt Only Parameters */

    /*
     * Specifies the AN behavior upon expiry. (Encrypt Only).
     * Applicable only for Inline Xflow Macsec architecture.
     */
    xflow_macsec_secure_assoc_an_control_t an_control;

    /* Specifies in number of bytes from start of frame where
     * to insert the ESP header  (Encrypt only). */
    uint32 esp_offset;

    /*
     * The allowed Maximum Transmission Unit(MTU) for egress encrypted
     * packets. Applicable only for Inline Xflow Macsec architecture.
     */
    uint32 mtu;

    /*
     * Used to set the profile for soft expiry threshold
     * selection.
     * Applicable only for encrypt direction.
     */
    uint8 soft_expiry_threshold_select;

    /* Identify as UDP packet. */
    uint8     udp;

    /* Decrypt only parameters */

    /* IPsec secure channel match and action parameters for entry 0. (Decrypt) */
    xflow_macsec_ipsec_sc_match_action_info_t  sc_match_action0;

    /*
     * IPsec secure channel match and action parameters for entry 1. (Decrypt)
     * This entry is allocated in the TCAM only if the flag
     * XFLOW_MACSEC_IPSEC_SECURE_CHAN_ALLOCATE_MATCH_ACTION_ENTRY1
     * is configured.
     */
    xflow_macsec_ipsec_sc_match_action_info_t  sc_match_action1;

    /* Only for the decrypt path. Replay Protection Window Size. If the size is 0,
     * strict replay check will be imposed. (Decrypt only)*/
    uint32 replay_protect_window_size;

} xflow_macsec_ipsec_secure_chan_info_t;

typedef struct xflow_macsec_ipsec_secure_assoc_info_s {

    /* Association number (Decrypt only). */
    uint8 an;

    /* The next packet sequence number. */
    uint32 next_pkt_num;

    /*
     * The higher 32 bits in the next packet sequence number.
     * This is used only for the AES-GCM-XPN algorithm.
     */
    uint32 next_pkt_num_upper;

    /*
     * SALT Parameter for AES-GCM-XPN algorithm. Only the first
     * four bytes are applicable for IPsec.
     */
    uint8 salt[12];

    /*
     * Security Parameter Index for IPsec. Values between 0 - 255 must not
     * be configured. Applicable only for encrypt direction.
     */
    uint32 spi;

    uint32 flags;

    /* AES-256 key. */
    xflow_macsec_crypto_aes256_gcm_t aes_256;

    /*
     * Enable the secure association.
     * Applicable only for Inline Xflow Macsec.
     */
    uint8 enable;
} xflow_macsec_ipsec_secure_assoc_info_t;

/*******************************************************************************
 * Function declarations
 */


/*
 * Function    - xflow_macsec_port_info_t_init
 * Description - Initialization function for xflow_macsec_port_info_t.
 * Port        - port info
 */
extern void
xflow_macsec_port_info_t_init(xflow_macsec_port_info_t *port_info);

extern int
xflow_macsec_secure_chan_info_t_init(
            xflow_macsec_secure_chan_info_t *chan_info);

extern int
xflow_macsec_secure_assoc_info_t_init(
            xflow_macsec_secure_assoc_info_t *assoc_info);

extern int
xflow_macsec_policy_info_t_init(xflow_macsec_policy_info_t *policy_info);

extern int
xflow_macsec_flow_info_t_init (xflow_macsec_flow_info_t *flow_info);

extern int
xflow_macsec_ipsec_secure_assoc_info_t_init(xflow_macsec_ipsec_secure_assoc_info_t *assoc_info);

extern int
xflow_macsec_ipsec_secure_chan_info_t_init(xflow_macsec_ipsec_secure_chan_info_t *chan_info);

typedef xflow_macsec_port_info_t bcm_xflow_macsec_port_info_t;

#endif /* XFLOW_MACSEC_DEFS_H */
