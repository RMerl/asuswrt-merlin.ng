#if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BCM_DPI_MODULE)
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
#define DPI_CT_BLOCK_BIT		31

#define DPI_NL_CHANGE_MASK		(1 << DPI_CT_BLOCK_BIT)

#define DPI_URLINFO_MAX_HOST_LEN	64

#define dpi_ct_init_from_wan(ct) \
	test_bit(DPI_CT_INIT_FROM_WAN_BIT, &(ct)->dpi.flags)

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

	struct dpi_ct_stats	us;
	struct dpi_ct_stats	ds;

	u16			classified;
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

struct dpi_hooks {
	int (*cpu_enqueue)(pNBuff_t pNBuff, struct net_device *dev);
};

struct nf_conn;

/* ----- dpicore driver functions ----- */
struct dpi_info *dpi_info_get(struct nf_conn *conn);
u32 dpi_app_id(struct dpi_app *app);
u32 dpi_dev_id(struct dpi_dev *dev);
u8 *dpi_mac(struct dpi_dev *dev);
int dpi_url_len(struct dpi_url *url);
char *dpi_url(struct dpi_url *url);
struct dpi_ct_stats *dpi_appinst_stats(struct nf_conn *ct, int dir);
struct dpi_ct_stats *dpi_dev_stats(struct nf_conn *ct, int dir);
void dpi_print_flow(struct seq_file *s, struct nf_conn *ct);
int dpi_bind(struct dpi_hooks *hooks);
void dpi_block(struct nf_conn *conn);
int dpi_cpu_enqueue(pNBuff_t pNBuff, struct net_device *dev);
void dpi_ct_evicting(struct nf_conn *ct);

/* ----- dpicore driver variables ----- */
extern struct proc_dir_entry *dpi_dir;
extern int dpi_enabled;

#endif /* _LINUX_DPI_H */
#endif /* defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BCM_DPI_MODULE) */
