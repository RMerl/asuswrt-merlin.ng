/*
 * <:copyright-BRCM:2017:DUAL/GPL:standard 
 * 
 *    Copyright (c) 2017 Broadcom 
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

#ifndef _DPI_LOCAL_H
#define _DPI_LOCAL_H

#include <bcmdpi.h>

#define DPI_URL_RECORD

#define DPI_URLINFO_MAX_HOST_LEN	64

#define nfct(skb)		((struct nf_conn *)skb->nfct)
#define dpi_info(skb)		(nfct(skb)->dpi)
#define is_wan_dev(skb)		((skb)->dev->priv_flags & IFF_WANDEV)

struct dpi_app {
	uint32_t app_id;

	struct hlist_node node;
};

struct dpi_dev {
	uint8_t			mac[ETH_ALEN];

	uint16_t		vendor_id;
	uint16_t		os_id;
	uint16_t		class_id;
	uint16_t		type_id;
	uint32_t		dev_id;

	struct dpi_ct_stats	us;
	struct dpi_ct_stats	ds;

	uint16_t		classified;
	struct hlist_node	node;
};

struct dpi_appinst {
	struct dpi_app		*app;
	struct dpi_dev		*dev;
	struct dpi_ct_stats	us;
	struct dpi_ct_stats	ds;

	struct hlist_node	node;
};

struct dpi_url {
	int  len;
	char hostname[DPI_URLINFO_MAX_HOST_LEN];

	struct hlist_node node;
};

struct dpi_classification_stats {
	uint32_t lookups;
	uint32_t hits;
	uint32_t misses;
	uint32_t classified;
	uint32_t unclassified;
};

struct dpi_stats {
	struct dpi_classification_stats apps;
	struct dpi_classification_stats devs;
	struct dpi_classification_stats urls;
	uint32_t total_lookups;
	uint32_t app_count;
	uint32_t dev_count;
	uint32_t appinst_count;
	uint32_t url_count;
	uint32_t engine_errors;
	uint64_t blocked_pkts;
};

extern struct dpi_stats dpi_stats;

/* dpi tables */
struct dpi_dev *dpi_dev_find_or_alloc(uint8_t *mac);
struct dpi_app *dpi_app_find_or_alloc(uint32_t app_id);
struct dpi_appinst *dpi_appinst_find_or_alloc(struct dpi_app *app,
					      struct dpi_dev *dev);
struct dpi_url *dpi_url_find_or_alloc(char *hostname, int len);
void dpi_reset_stats(void);
int __init dpi_init_tables(void);
void __exit dpi_deinit_tables(void);

#endif /* _DPI_LOCAL_H */
