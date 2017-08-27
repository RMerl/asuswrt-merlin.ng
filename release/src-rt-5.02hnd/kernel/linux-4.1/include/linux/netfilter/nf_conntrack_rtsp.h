#if defined(CONFIG_BCM_KF_NETFILTER)

/*
* <:copyright-BRCM:2012:DUAL/GPL:standard
* 
*    Copyright (c) 2012 Broadcom 
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
:>
*/

#ifndef _NF_CONNTRACK_RTSP_H
#define _NF_CONNTRACK_RTSP_H

#ifdef __KERNEL__

/* This structure exists only once per master */
struct nf_ct_rtsp_master {
	/* The client has sent PAUSE message and not replied */
	int paused;
};

/* Single data channel */
extern int (*nat_rtsp_channel_hook) (struct sk_buff *skb,
				     unsigned int protoff,
				     struct nf_conn *ct,
				     enum ip_conntrack_info ctinfo,
				     unsigned int matchoff,
				     unsigned int matchlen,
				     struct nf_conntrack_expect *exp,
				     int *delta);

/* A pair of data channels (RTP/RTCP) */
extern int (*nat_rtsp_channel2_hook) (struct sk_buff *skb,
				      unsigned int protoff,
				      struct nf_conn *ct,
				      enum ip_conntrack_info ctinfo,
				      unsigned int matchoff,
				      unsigned int matchlen,
				      struct nf_conntrack_expect *rtp_exp,
				      struct nf_conntrack_expect *rtcp_exp,
				      char dash, int *delta);

/* Modify parameters like client_port in Transport for single data channel */
extern int (*nat_rtsp_modify_port_hook) (struct sk_buff *skb,
				         unsigned int protoff,
					 struct nf_conn *ct,
			      	  	 enum ip_conntrack_info ctinfo,
			      	  	 unsigned int matchoff,
					 unsigned int matchlen,
			      	  	 __be16 rtpport, int *delta);

/* Modify parameters like client_port in Transport for multiple data channels*/
extern int (*nat_rtsp_modify_port2_hook) (struct sk_buff *skb,
				          unsigned int protoff,
					  struct nf_conn *ct,
			       	   	  enum ip_conntrack_info ctinfo,
			       	   	  unsigned int matchoff,
					  unsigned int matchlen,
			       	   	  __be16 rtpport, __be16 rtcpport,
				   	  char dash, int *delta);

/* Modify parameters like destination in Transport */
extern int (*nat_rtsp_modify_addr_hook) (struct sk_buff *skb,
				         unsigned int protoff,
					 struct nf_conn *ct,
				 	 enum ip_conntrack_info ctinfo,
					 int matchoff, int matchlen,
					 int *delta);
#endif /* __KERNEL__ */

#endif /* _NF_CONNTRACK_RTSP_H */

#endif
