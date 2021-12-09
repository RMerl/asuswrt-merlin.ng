#if defined(CONFIG_BCM_KF_SKB_DEFINES)
/*
* <:copyright-BRCM:2014:DUAL/GPL:standard
* 
*    Copyright (c) 2014 Broadcom 
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

#ifndef _BCM_SKB_DEFINES_
#define _BCM_SKB_DEFINES_

/* queue = mark[4:0] */
#define SKBMARK_Q_S             0
#define SKBMARK_Q_M             (0x1F << SKBMARK_Q_S)
#define SKBMARK_GET_Q(MARK)     ((MARK & SKBMARK_Q_M) >> SKBMARK_Q_S)
#define SKBMARK_SET_Q(MARK, Q)  ((MARK & ~SKBMARK_Q_M) | (Q << SKBMARK_Q_S))
/* traffic_class_id = mark[10:5] */
#define SKBMARK_TC_ID_S         5
#define SKBMARK_TC_ID_M         (0x3F << SKBMARK_TC_ID_S)
#define SKBMARK_GET_TC_ID(MARK) ((MARK & SKBMARK_TC_ID_M) >> SKBMARK_TC_ID_S)
#define SKBMARK_SET_TC_ID(MARK, TC) \
    ((MARK & ~SKBMARK_TC_ID_M) | (TC << SKBMARK_TC_ID_S))
/* flow_id = mark[18:11] */
#define SKBMARK_FLOW_ID_S       11
#define SKBMARK_FLOW_ID_M       (0xFF << SKBMARK_FLOW_ID_S)
#define SKBMARK_GET_FLOW_ID(MARK) \
    ((MARK & SKBMARK_FLOW_ID_M) >> SKBMARK_FLOW_ID_S)
#define SKBMARK_SET_FLOW_ID(MARK, FLOW) \
    ((MARK & ~SKBMARK_FLOW_ID_M) | (FLOW << SKBMARK_FLOW_ID_S))
/* iq_prio = mark[19]; for Ingress QoS used when TX is WLAN */
#define SKBMARK_IQPRIO_MARK_S    19
#define SKBMARK_IQPRIO_MARK_M    (0x01 << SKBMARK_IQPRIO_MARK_S)
#define SKBMARK_GET_IQPRIO_MARK(MARK) \
    ((MARK & SKBMARK_IQPRIO_MARK_M) >> SKBMARK_IQPRIO_MARK_S)
#define SKBMARK_SET_IQPRIO_MARK(MARK, IQPRIO_MARK) \
    ((MARK & ~SKBMARK_IQPRIO_MARK_M) | (IQPRIO_MARK << SKBMARK_IQPRIO_MARK_S))
/* port = mark[26:20]; for enet driver of gpon port, this is gem_id */
#define SKBMARK_PORT_S          20
#define SKBMARK_PORT_M          (0x7F << SKBMARK_PORT_S)
#define SKBMARK_GET_PORT(MARK) \
    ((MARK & SKBMARK_PORT_M) >> SKBMARK_PORT_S)
#define SKBMARK_SET_PORT(MARK, PORT) \
    ((MARK & ~SKBMARK_PORT_M) | (PORT << SKBMARK_PORT_S))
#if defined(CONFIG_BCM_KF_WANDEV)
/* iffwan_mark = mark[27] --  BRCM defined-- */
#define SKBMARK_IFFWAN_MARK_S    27
#define SKBMARK_IFFWAN_MARK_M    (0x01 << SKBMARK_IFFWAN_MARK_S)
#define SKBMARK_GET_IFFWAN_MARK(MARK) \
    ((MARK & SKBMARK_IFFWAN_MARK_M) >> SKBMARK_IFFWAN_MARK_S)
#define SKBMARK_SET_IFFWAN_MARK(MARK, IFFWAN_MARK) \
    ((MARK & ~SKBMARK_IFFWAN_MARK_M) | (IFFWAN_MARK << SKBMARK_IFFWAN_MARK_S))
#endif
/* ipsec_mark = mark[28] */
#define SKBMARK_IPSEC_MARK_S    28
#define SKBMARK_IPSEC_MARK_M    (0x01 << SKBMARK_IPSEC_MARK_S)
#define SKBMARK_GET_IPSEC_MARK(MARK) \
    ((MARK & SKBMARK_IPSEC_MARK_M) >> SKBMARK_IPSEC_MARK_S)
#define SKBMARK_SET_IPSEC_MARK(MARK, IPSEC_MARK) \
    ((MARK & ~SKBMARK_IPSEC_MARK_M) | (IPSEC_MARK << SKBMARK_IPSEC_MARK_S))
/* policy_routing = mark[31:29] */
#define SKBMARK_POLICY_RTNG_S   29
#define SKBMARK_POLICY_RTNG_M   (0x07 << SKBMARK_POLICY_RTNG_S)
#define SKBMARK_GET_POLICY_RTNG(MARK)  \
    ((MARK & SKBMARK_POLICY_RTNG_M) >> SKBMARK_POLICY_RTNG_S)
#define SKBMARK_SET_POLICY_RTNG(MARK, POLICY) \
    ((MARK & ~SKBMARK_POLICY_RTNG_M) | (POLICY << SKBMARK_POLICY_RTNG_S))

/* dpi_queue = mark[31:27] */
/* Overlaps with SKBMARK_IFFWAN, SKBMARK_IPSEC, and SKBMARK_POLICY_RTNG  */
#define SKBMARK_DPIQ_MARK_S    27
#define SKBMARK_DPIQ_MARK_M    (0x1F << SKBMARK_DPIQ_MARK_S)
#define SKBMARK_GET_DPIQ_MARK(MARK) \
    ((MARK & SKBMARK_DPIQ_MARK_M) >> SKBMARK_DPIQ_MARK_S)
#define SKBMARK_SET_DPIQ_MARK(MARK, DPIQ_MARK) \
    ((MARK & ~SKBMARK_DPIQ_MARK_M) | (DPIQ_MARK << SKBMARK_DPIQ_MARK_S))

/* The enet driver subdivides queue field (mark[4:0]) in the skb->mark into
   priority and channel */
/* priority = queue[2:0] (=>mark[2:0]) */
#define SKBMARK_Q_PRIO_S        (SKBMARK_Q_S)
#define SKBMARK_Q_PRIO_M        (0x07 << SKBMARK_Q_PRIO_S)
#define SKBMARK_GET_Q_PRIO(MARK) \
    ((MARK & SKBMARK_Q_PRIO_M) >> SKBMARK_Q_PRIO_S)
#define SKBMARK_SET_Q_PRIO(MARK, Q) \
    ((MARK & ~SKBMARK_Q_PRIO_M) | (Q << SKBMARK_Q_PRIO_S))
/* channel = queue[4:3] (=>mark[4:3]) */
#define SKBMARK_Q_CH_S          (SKBMARK_Q_S + 3)
#define SKBMARK_Q_CH_M          (0x03 << SKBMARK_Q_CH_S)
#define SKBMARK_GET_Q_CHANNEL(MARK) ((MARK & SKBMARK_Q_CH_M) >> SKBMARK_Q_CH_S)
#define SKBMARK_SET_Q_CHANNEL(MARK, CH) \
    ((MARK & ~SKBMARK_Q_CH_M) | (CH << SKBMARK_Q_CH_S))

#define SKBMARK_ALL_GEM_PORT  (0xFF) 

#define WLAN_PRIORITY_BIT_POS  (1)
#define WLAN_PRIORITY_MASK     (0x7 << WLAN_PRIORITY_BIT_POS)
#define GET_WLAN_PRIORITY(VAL) ((VAL & WLAN_PRIORITY_MASK) >> WLAN_PRIORITY_BIT_POS)
#define SET_WLAN_PRIORITY(ENCODEVAL, PRIO) ((ENCODEVAL & (~WLAN_PRIORITY_MASK)) | (PRIO << WLAN_PRIORITY_BIT_POS))

#define WLAN_IQPRIO_BIT_POS    (0)
#define WLAN_IQPRIO_MASK       (0x1 << WLAN_IQPRIO_BIT_POS)
#define GET_WLAN_IQPRIO(VAL)   ((VAL & WLAN_IQPRIO_MASK) >> WLAN_IQPRIO_BIT_POS)
#define SET_WLAN_IQPRIO(ENCODEVAL, IQPRIO) ((ENCODEVAL & (~WLAN_IQPRIO_MASK)) | (IQPRIO << WLAN_IQPRIO_BIT_POS))

// LINUX_PRIORITY_BIT_POS_IN_MARK macro must be in sync with PRIO_LOC_NFMARK
// defined in linux_osl_dslcpe.h
#define LINUX_PRIORITY_BIT_POS_IN_MARK    16
#define LINUX_PRIORITY_BIT_MASK          (0x7 << LINUX_PRIORITY_BIT_POS_IN_MARK)
#define LINUX_GET_PRIO_MARK(MARK)        ((MARK & LINUX_PRIORITY_BIT_MASK) >> LINUX_PRIORITY_BIT_POS_IN_MARK)
#define LINUX_SET_PRIO_MARK(MARK, PRIO)  ((MARK & (~LINUX_PRIORITY_BIT_MASK)) | (PRIO << LINUX_PRIORITY_BIT_POS_IN_MARK)) 

//Encode 3 bits of priority and 1 bit of IQPRIO into 4 bits as follows (3bitPrio:1bitIQPrio)
#define ENCODE_WLAN_PRIORITY_MARK(u8EncodeVal, u32Mark) \
    (u8EncodeVal = SET_WLAN_PRIORITY(u8EncodeVal, LINUX_GET_PRIO_MARK(u32Mark)) | SET_WLAN_IQPRIO(u8EncodeVal, SKBMARK_GET_IQPRIO_MARK(u32Mark)))
#define DECODE_WLAN_PRIORITY_MARK(encodedVal, u32Mark) \
    do { (u32Mark) = LINUX_SET_PRIO_MARK(u32Mark, GET_WLAN_PRIORITY(encodedVal)); \
        (u32Mark) = SKBMARK_SET_IQPRIO_MARK(u32Mark, GET_WLAN_IQPRIO(encodedVal)); \
    } while(0)


#endif /* _BCM_SKB_DEFINES_ */
#endif /* CONFIG_BCM_KF_SKB_DEFINES */
