#if defined(CONFIG_BCM_KF_DPI)
#ifndef _LINUX_DPI_H
#define _LINUX_DPI_H

#include <linux/if_ether.h>
#include <linux/list.h>

#define DPI_APPID_ONGOING_BIT		0
#define DPI_APPID_IDENTIFIED_BIT	1
#define DPI_APPID_FINAL_BIT		2
#define DPI_APPID_STOP_CLASSIFY_BIT	3
#define DPI_APPID_RESYNC_BIT		4
#define DPI_DEVID_ONGOING_BIT		5
#define DPI_DEVID_IDENTIFIED_BIT	6
#define DPI_DEVID_FINAL_BIT		7
#define DPI_DEVID_STOP_CLASSIFY_BIT	8
#define DPI_URL_STOP_CLASSIFY_BIT	9
#define DPI_CLASSIFICATION_STOP_BIT	14
#define DPI_CT_INIT_FROM_WAN_BIT	15
#define DPI_CT_DS_BYPASS_BIT		29
#define DPI_CT_US_BYPASS_BIT		30
#define DPI_CT_BLOCK_BIT		31

#define DPI_NL_CHANGE_MASK		(1 << DPI_CT_BLOCK_BIT)

#define DPI_URLINFO_MAX_HOST_LEN	64
/* 256 was chosen as the max length of a hostname in a DHCP packet is 255. */
#define DPI_HOSTNAME_MAX_LEN		256

#define dpi_ct_init_from_wan(ct) \
	test_bit(DPI_CT_INIT_FROM_WAN_BIT, &(ct)->bcm_ext.dpi.flags)

struct dpi_ct_stats {
	u64	pkts;
	u64	bytes;
};

struct dpi_app {
	u32			app_id;
	atomic_t		refcount;
	struct hlist_node	node;
};

struct dpi_dev {
	u8			mac[ETH_ALEN];

	u32			dev_id;
	u16			category;
	u16			family;
	u16			vendor;
	u16			os;
	u16			os_class;
	u16			prio;
	char			hostname[DPI_HOSTNAME_MAX_LEN];

	struct dpi_ct_stats	us;
	struct dpi_ct_stats	ds;

	atomic_t		refcount;
	struct hlist_node	node;
};

struct dpi_appinst {
	struct dpi_app		*app;
	struct dpi_dev		*dev;
	struct dpi_ct_stats	us;
	struct dpi_ct_stats	ds;
	atomic_t		refcount;
	struct hlist_node	node;
};

struct dpi_url {
	u32			len;
	char			hostname[DPI_URLINFO_MAX_HOST_LEN];
	atomic_t		refcount;
	struct hlist_node	node;
};

struct dpi_info {
	struct dpi_dev		*dev;
	struct dpi_app		*app;
	struct dpi_appinst	*appinst;
	struct dpi_url		*url;
	unsigned long		flags;
};

struct nf_conn;

struct dpi_core_hooks {
	void (*delete)(struct nf_conn *ct);
};

struct dpi_ct_hooks {
	int (*event_report)(int eventmask, struct nf_conn *ct, u32 portid,
			    int report);
};

/* ----- dpi functions ----- */
struct dpi_info *dpi_info_get(struct nf_conn *conn);
u32 dpi_app_id(struct dpi_app *app);
u32 dpi_dev_id(struct dpi_dev *dev);
u8 *dpi_mac(struct dpi_dev *dev);
int dpi_url_len(struct dpi_url *url);
char *dpi_url(struct dpi_url *url);
struct dpi_ct_stats *dpi_appinst_stats(struct nf_conn *ct, int dir);
struct dpi_ct_stats *dpi_dev_stats(struct nf_conn *ct, int dir);
void dpi_block(struct nf_conn *conn);
void dpi_nf_ct_delete_from_lists(struct nf_conn *ct);
int dpi_core_hooks_register(struct dpi_core_hooks *h);
void dpi_core_hooks_unregister(void);
int dpi_nf_ct_event_report(struct nf_conn *ct, u32 portid);
void dpi_conntrack_init(void);
void dpi_conntrack_cleanup(void);

/* dpi notification chain */
struct notifier_block;
enum {
	DPI_NOTIFY_DEVICE,
};
int dpi_register_notifier(struct notifier_block *nb);
int dpi_unregister_notifier(struct notifier_block *nb);
int dpi_notify(long event, void *data);

#endif /* _LINUX_DPI_H */
#endif /* defined(CONFIG_BCM_KF_DPI) */
