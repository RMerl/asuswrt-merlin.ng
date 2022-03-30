#ifndef _BCM_NFCONN_EXT_H
#define _BCM_NFCONN_EXT_H

/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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

#include <linux/dpi.h>
#include <linux/ndi.h>
#include <linux/sgs.h>

typedef struct biqos_cb {
	uint32_t skb_mark;
} biqos_cb_t;

struct bcm_nf_conn_ext {
#if defined(CONFIG_BLOG)
	unsigned int blog_key[2];	/* Associating 2=IP_CT_DIR_MAX blogged flows */
	unsigned long extra_jiffies;	/* connection timeout value */
#endif

#if defined(CONFIG_BCM_KF_NF_REGARDLESS_DROP)
	uint32_t hw_accel_flows;	/* Number of HW accelerated flows, need 32 bit for tunnel connection */
	uint32_t sw_accel_flows;	/* Number of SW accelerated flows, need 32 bit for tunnel connection */
	struct list_head safe_list;	/* regardless drop of connections */
	struct list_head *curr_safe_list; /* current safe_list for this connection*/
#endif

#if defined(CONFIG_BCM_NF_DERIVED_CONN)
	struct list_head derived_connections;	/* Used by master connection */
	struct list_head derived_list;		/* Used by child connection */
	unsigned int derived_timeout;		/* if non-zero override linux timeout */
#endif

#if IS_ENABLED(CONFIG_BCM_INGQOS)
	uint8_t iq_prio;			/* Ingress QoS Prio */
	uint8_t unused0;
	uint16_t unused1;
#endif

#if IS_ENABLED(CONFIG_NF_DYNDSCP)
	struct nf_tos_inheritance {
		u_int16_t status;
		u_int8_t dscp[2];		/* IP_CT_DIR_MAX */
	} dyndscp;
#endif

#if IS_ENABLED(CONFIG_BCM_NDI)
	struct ndi_info ndi;
#endif

#if IS_ENABLED(CONFIG_BCM_DPI)
	struct dpi_info dpi;
#endif

#if IS_ENABLED(CONFIG_BCM_SGS)
	struct sgs_info sgs;
#endif
	biqos_cb_t cb;
};

#define bcm_nfconn_ext_field_get(ct, f)		(ct->bcm_ext.f)
#define bcm_nfconn_ext_field_get_ptr(ct, f)	(&ct->bcm_ext.f)
#define bcm_nfconn_ext_field_set(ct, f, val)	(ct->bcm_ext.f = val)

#if defined(CONFIG_BCM_NF_DERIVED_CONN)
extern spinlock_t bcm_derived_conn_lock;
#define BCM_DERIVED_CONN_LOCK_BH()	spin_lock_bh(&bcm_derived_conn_lock)
#define BCM_DERIVED_CONN_UNLOCK_BH()	spin_unlock_bh(&bcm_derived_conn_lock)
#endif


int ct_show_bcm_ext(struct seq_file *s, const struct nf_conn *ct);


#endif /*_BCM_NFCONN_EXT_H */
