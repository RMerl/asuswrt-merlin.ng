#ifdef CONFIG_BCM_KF_MHI
/*
<:copyright-BRCM:2011:DUAL/GPL:standard

   Copyright (c) 2011 Broadcom 
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
/*
 * File: l2mux.c
 *
 * Modem-Host Interface (MHI) L2MUX layer
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/if_mhi.h>
#include <linux/mhi.h>
#include <linux/mhi_l2mux.h>

#ifdef ACTIVATE_L2MUX_STAT
#include <linux/proc_fs.h>
#include <linux/time.h>
#include <linux/spinlock.h>
#include <linux/poll.h>
#endif /* ACTIVATE_L2MUX_STAT */

#include <net/af_mhi.h>

#ifdef CONFIG_MHI_DEBUG
# define DPRINTK(...)    pr_debug("MHI/L2MUX: " __VA_ARGS__)
#else
# define DPRINTK(...)
#endif

#ifdef ACTIVATE_L2MUX_STAT
#define MAX_COOKIE_LENGTH       PAGE_SIZE

/* MAX_COOKIE_LENGTH/sizeof(struct l2muxstat) */
#define MAX_DEBUG_MESSAGES      5000

#define list_l2mux_first_entry_safe(head, type, member) \
					(list_empty(head) ? NULL : \
					list_first_entry(head, type, member))
static DEFINE_RWLOCK(l2mux_stat_lock);

static struct l2mux_stat_info l2mux_sinf;

#endif

/* Handle ONLY Non DIX types 0x00-0xff */
#define ETH_NON_DIX_NPROTO   0x0100

/* L2MUX master lock */
static DEFINE_SPINLOCK(l2mux_lock);

/* L3 ID -> RX function table */
static l2mux_skb_fn *l2mux_id2rx_tab[MHI_L3_NPROTO] __read_mostly;

/* Packet Type -> TX function table */
static l2mux_skb_fn *l2mux_pt2tx_tab[ETH_NON_DIX_NPROTO] __read_mostly;

/* audio RX/TX fn table */
static l2mux_audio_fn *l2mux_audio_rx_fn __read_mostly;
static int l2mux_audio_rx_handle __read_mostly;

static l2mux_audio_fn *l2mux_audio_tx_tab[L2MUX_AUDIO_DEV_MAX] __read_mostly;
static uint8_t l2mux_audio_tx_pn_map[L2MUX_AUDIO_DEV_MAX] __read_mostly;

#ifdef ACTIVATE_L2MUX_STAT

static void l2mux_write_stat(unsigned l3pid, unsigned l3len,
			     enum l2mux_direction dir, struct net_device *dev);
static ssize_t store_l2mux_traces_state(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count);
static ssize_t show_l2mux_traces_state(struct device *dev,
				       struct device_attribute *attr,
				       char *buf);

static struct device_attribute l2mux_dev_attrs[] = {
	__ATTR(l2mux_trace_status,
	       S_IRUGO | S_IWUSR,
	       show_l2mux_traces_state,
	       store_l2mux_traces_state),
	__ATTR_NULL,
};

void l2mux_stat_dowork(struct work_struct *work)
{
	int err;
	struct l2mux_stat_info *info =
	    container_of(work, struct l2mux_stat_info, l2mux_stat_work);

	struct net_device *dev = info->dev;

	if (l2mux_sinf.l2mux_traces_activation_done != 1) {

		err = device_create_file(&dev->dev, &l2mux_dev_attrs[0]);

		if (err == 0)
			l2mux_sinf.l2mux_traces_activation_done = 1;
		else
			pr_err("L2MUX cannot create device file\n");
	}
}

/*call this function to update the l2mux write statistic*/
static void
l2mux_write_stat(unsigned l3pid,
		 unsigned l3len,
		 enum l2mux_direction dir, struct net_device *dev)
{

	struct l2muxstat *tmp_stat;
	struct l2muxstat *old_stat;

	l2mux_sinf.l2mux_total_stat_counter++;

	if ((dev != NULL) && (l2mux_sinf.l2mux_traces_activation_done == 0)) {
		l2mux_sinf.dev = dev;
		schedule_work(&l2mux_sinf.l2mux_stat_work);
		return;

	} else {

		if ((ON == l2mux_sinf.l2mux_traces_state) ||
		    (KERNEL == l2mux_sinf.l2mux_traces_state)) {

			if (write_trylock(&l2mux_stat_lock)) {

				tmp_stat = kmalloc(sizeof(struct l2muxstat),
						   GFP_ATOMIC);
				if (NULL == tmp_stat) {
					write_unlock(&l2mux_stat_lock);
					return;
				}

				tmp_stat->l3pid = l3pid;
				tmp_stat->l3len = l3len;
				tmp_stat->dir = dir;
				do_gettimeofday(&(tmp_stat->time_val));
				tmp_stat->stat_counter =
				    l2mux_sinf.l2mux_total_stat_counter;

				if (l2mux_sinf.l2mux_stat_id < 0)
					l2mux_sinf.l2mux_stat_id = 0;

				l2mux_sinf.l2mux_stat_id++;

				if (l2mux_sinf.l2mux_stat_id >=
				    MAX_DEBUG_MESSAGES) {

					old_stat =
					    list_l2mux_first_entry_safe
					    (&l2mux_sinf.l2muxstat_tab.list,
					     struct l2muxstat, list);
					if (old_stat != NULL) {
						list_del(&old_stat->list);
						kfree(old_stat);
						l2mux_sinf.l2mux_stat_id =
						    MAX_DEBUG_MESSAGES;
					}
				}

				list_add_tail(&(tmp_stat->list),
					      &(l2mux_sinf.l2muxstat_tab.list));

				write_unlock(&l2mux_stat_lock);
			}
		}
	}
	/*in the case lock is taken, information is missed */
}

/* start() method */
static void *l2mux_seq_start(struct seq_file *seq, loff_t *pos)
{
	void *ret = NULL;

	if (l2mux_sinf.l2mux_traces_state == OFF) {
		pr_err("L2MUX traces are off. activation -echo on > /sys/class/net/my_modem_net_device/l2mux_trace_status -sizeof(l2muxstat) = %zu\n",
		       sizeof(struct l2muxstat));
	} else {
		if (write_trylock(&l2mux_stat_lock)) {
			ret =
			    list_l2mux_first_entry_safe(&l2mux_sinf.
							l2muxstat_tab.list,
							struct l2muxstat, list);
			write_unlock(&l2mux_stat_lock);
		}
	}

	return ret;
}

/* next() method */
static void *l2mux_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	return list_l2mux_first_entry_safe(&l2mux_sinf.l2muxstat_tab.list,
					   struct l2muxstat, list);
}

/* show() method */
static int l2mux_seq_show(struct seq_file *seq, void *v)
{
	struct l2muxstat *tmp_stat = v;
	char temp_string[100];

	if (write_trylock(&l2mux_stat_lock)) {

		while (l2mux_sinf.previous_stat_counter !=
		       (tmp_stat->stat_counter - 1)) {

			sprintf(temp_string,
				"L2MHI_%d : missed : NA : NA : NA : NA\n",
				l2mux_sinf.previous_stat_counter + 1);

			/* Interpret the iterator, 'v' */
			seq_puts(seq, temp_string);

			l2mux_sinf.previous_stat_counter++;
		}

		l2mux_sinf.previous_stat_counter = tmp_stat->stat_counter;

		sprintf(temp_string, "L2MHI_%d : %d : %d : %x : %d : %d\n",
			tmp_stat->stat_counter, tmp_stat->dir,
			tmp_stat->l3pid, tmp_stat->l3len,
			(unsigned int)tmp_stat->time_val.tv_sec,
			(unsigned int)tmp_stat->time_val.tv_usec);

		/* Interpret the iterator, 'v' */
		seq_puts(seq, temp_string);

		if (l2mux_sinf.l2mux_traces_state == KERNEL)
			pr_err("%s", temp_string);

		list_del(&tmp_stat->list);
		kfree(tmp_stat);
		tmp_stat = NULL;
		l2mux_sinf.l2mux_stat_id--;

		write_unlock(&l2mux_stat_lock);
	}

	return 0;
}

/* stop() method */
static void l2mux_seq_stop(struct seq_file *seq, void *v)
{
	/* No cleanup needed */
}

/* Define iterator operations */
static const struct seq_operations l2mux_seq_ops = {
	.start = l2mux_seq_start,
	.next = l2mux_seq_next,
	.stop = l2mux_seq_stop,
	.show = l2mux_seq_show,
};

static int l2mux_seq_open(struct inode *inode, struct file *file)
{
	/* Register the operators */
	return seq_open(file, &l2mux_seq_ops);
}

static const struct file_operations l2mux_proc_fops = {
	.owner = THIS_MODULE,
	.open = l2mux_seq_open,	/* User supplied */
	.read = seq_read,	/* Built-in helper function */
	.llseek = seq_lseek,	/* Built-in helper function */
	.release = seq_release,	/* Built-in helper funciton */
};

/*call this function to init the l2mux write statistic*/
void init_l2mux_stat(void)
{
	l2mux_sinf.proc_entry =
	    proc_create("l2mux_mhi", 0644, NULL, &l2mux_proc_fops);

	if (l2mux_sinf.proc_entry == NULL)
		DPRINTK("cannot create proc file l2mux_mhi\n");
	else {

		l2mux_sinf.l2mux_stat_id = 0;
		l2mux_sinf.previous_stat_counter = 0;
		l2mux_sinf.l2mux_total_stat_counter = 0;
		l2mux_sinf.l2mux_traces_state = OFF;
		l2mux_sinf.l2mux_traces_activation_done = 0;
		INIT_LIST_HEAD(&l2mux_sinf.l2muxstat_tab.list);
		INIT_WORK(&l2mux_sinf.l2mux_stat_work, l2mux_stat_dowork);
	}
}

/*call this function to exit the l2mux write statistic*/
void exit_l2mux_stat(void)
{
	remove_proc_entry("l2mux_mhi", l2mux_sinf.proc_entry);
}

/**
 * store_l2mux_traces_state - store the l2mux traces status
 * @dev: Device to be created
 * @attr: attribute of sysfs
 * @buf: output stringwait
 */
static ssize_t
store_l2mux_traces_state(struct device *dev,
			 struct device_attribute *attr,
			 const char *buf, size_t count)
{
	int retval = count;

	if (sysfs_streq(buf, "on")) {
		l2mux_sinf.l2mux_traces_state = ON;
		pr_err("L2MUX traces activated and available in proc fs\n");
	} else if (sysfs_streq(buf, "off")) {
		l2mux_sinf.l2mux_traces_state = OFF;
	} else if (sysfs_streq(buf, "kernel")) {
		l2mux_sinf.l2mux_traces_state = KERNEL;
	} else {
		retval = -EINVAL;
	}
	return retval;
}

/**
 * show_l2mux_traces_state - show l2mux traces state
 * @dev: Funnel device
 * @attr: attribute of sysfs
 * @buf: string written to sysfs file
 */
static ssize_t
show_l2mux_traces_state(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	int retval = 0;
	char *temp_buf = buf;

	switch (l2mux_sinf.l2mux_traces_state) {
	case ON:
		return sprintf(temp_buf, "on\n");
	case OFF:
		return sprintf(temp_buf, "off\n");
	case KERNEL:
		return sprintf(temp_buf, "kernel\n");
	default:
		return -ENODEV;
	}

	return retval;
}
#endif /* ACTIVATE_L2MUX_STAT */

int l2mux_netif_rx_register(int l3, l2mux_skb_fn *fn)
{
	int err = 0;

	DPRINTK("l2mux_netif_rx_register(l3:%d, fn:%p)\n", l3, fn);

	if (l3 < 0 || l3 >= MHI_L3_NPROTO)
		return -EINVAL;

	if (!fn)
		return -EINVAL;

	spin_lock(&l2mux_lock);
	{
		if (l2mux_id2rx_tab[l3] == NULL)
			l2mux_id2rx_tab[l3] = fn;
		else
			err = -EBUSY;
	}
	spin_unlock(&l2mux_lock);

	return err;
}
EXPORT_SYMBOL(l2mux_netif_rx_register);

int l2mux_netif_rx_unregister(int l3)
{
	int err = 0;

	DPRINTK("l2mux_netif_rx_unregister(l3:%d)\n", l3);

	if (l3 < 0 || l3 >= MHI_L3_NPROTO)
		return -EINVAL;

	spin_lock(&l2mux_lock);
	{
		if (l2mux_id2rx_tab[l3])
			l2mux_id2rx_tab[l3] = NULL;
		else
			err = -EPROTONOSUPPORT;
	}
	spin_unlock(&l2mux_lock);

	return err;
}
EXPORT_SYMBOL(l2mux_netif_rx_unregister);

int l2mux_netif_tx_register(int pt, l2mux_skb_fn *fn)
{
	int err = 0;

	DPRINTK("l2mux_netif_tx_register(pt:%d, fn:%p)\n", pt, fn);

	if (pt <= 0 || pt >= ETH_NON_DIX_NPROTO)
		return -EINVAL;

	if (!fn)
		return -EINVAL;

	spin_lock(&l2mux_lock);
	{
		if (l2mux_pt2tx_tab[pt] == NULL)
			l2mux_pt2tx_tab[pt] = fn;
		else
			err = -EBUSY;
	}
	spin_unlock(&l2mux_lock);

	return err;
}
EXPORT_SYMBOL(l2mux_netif_tx_register);

int l2mux_netif_tx_unregister(int pt)
{
	int err = 0;

	DPRINTK("l2mux_netif_tx_unregister(pt:%d)\n", pt);

	if (pt <= 0 || pt >= ETH_NON_DIX_NPROTO)
		return -EINVAL;

	spin_lock(&l2mux_lock);
	{
		if (l2mux_pt2tx_tab[pt])
			l2mux_pt2tx_tab[pt] = NULL;
		else
			err = -EPROTONOSUPPORT;
	}
	spin_unlock(&l2mux_lock);

	return err;
}
EXPORT_SYMBOL(l2mux_netif_tx_unregister);

int l2mux_skb_rx(struct sk_buff *skb, struct net_device *dev)
{
	struct l2muxhdr *l2hdr;
	unsigned l3pid;
	unsigned l3len;
	l2mux_skb_fn *rxfn;

	/* Set the device in the skb */
	skb->dev = dev;

	/* Set MAC header here */
	skb_reset_mac_header(skb);

	/* L2MUX header */
	l2hdr = l2mux_hdr(skb);

	/* proto id and length in L2 header */
	l3pid = l2mux_get_proto(l2hdr);
	l3len = l2mux_get_length(l2hdr);

#ifdef ACTIVATE_L2MUX_STAT
	l2mux_write_stat(l3pid, l3len, DOWNLINK_DIR, dev);
#endif /* ACTIVATE_L2MUX_STAT */

#ifdef CONFIG_MHI_DUMP_FRAMES
	{
		u8 *ptr = skb->data;
		int len = skb_headlen(skb);
		int i;

		pr_debug("L2MUX: RX dev:%d skb_len:%d l3_len:%d l3_pid:%d\n",
		       dev->ifindex, skb->len, l3len, l3pid);

		for (i = 0; i < len; i++) {
			if (i % 8 == 0)
				pr_debug("L2MUX: RX [%04X] ", i);
			pr_debug(" 0x%02X", ptr[i]);
			if (i % 8 == 7 || i == len - 1)
				pr_debug("\n");
		}
	}
#endif
	/* check that the advertised length is correct */
	if (l3len != skb->len - L2MUX_HDR_SIZE) {
		pr_warn("L2MUX: l2mux_skb_rx: L3_id:%d - skb length mismatch L3:%d (+4) <> SKB:%d",
		       l3pid, l3len, skb->len);
		goto drop;
	}

	/* get RX function */
	rxfn = l2mux_id2rx_tab[l3pid];

	/* Not registered */
	if (!rxfn)
		goto drop;

	/* Update RX statistics */
	dev->stats.rx_packets++;
	dev->stats.rx_bytes += skb->len;

	/* Call the receiver function */
	return rxfn(skb, dev);

drop:
	dev->stats.rx_dropped++;
	kfree_skb(skb);
	return NET_RX_DROP;
}
EXPORT_SYMBOL(l2mux_skb_rx);

int l2mux_skb_tx(struct sk_buff *skb, struct net_device *dev)
{
	l2mux_skb_fn *txfn;
	unsigned type;
	int err = 0;
#ifdef ACTIVATE_L2MUX_STAT
	struct l2muxhdr *l2hdr;
	unsigned l3pid;
	unsigned l3len;
#endif /* ACTIVATE_L2MUX_STAT */

	if (unlikely(!skb)) {
		pr_err("L2MUX TX skb invalid\n");
		return -EINVAL;
	}

	/* Packet type ETH_P_XXX */
	type = ntohs(skb->protocol);

#ifdef CONFIG_MHI_DUMP_FRAMES
	{
		u8 *ptr = skb->data;
		int len = skb_headlen(skb);
		int i;

		pr_debug("L2MUX: TX dev:%d skb_len:%d ETH_P:%d\n",
		       dev->ifindex, skb->len, type);

		for (i = 0; i < len; i++) {
			if (i % 8 == 0)
				pr_debug("L2MUX: TX [%04X] ", i);
			pr_debug(" 0x%02X", ptr[i]);
			if (i % 8 == 7 || i == len - 1)
				pr_debug("\n");
		}
	}
#endif
	/* Only handling non DIX types */
	if (type <= 0 || type >= ETH_NON_DIX_NPROTO)
		return -EINVAL;

	/* TX function for this packet type */
	txfn = l2mux_pt2tx_tab[type];

	if (txfn)
		err = txfn(skb, dev);

#ifdef ACTIVATE_L2MUX_STAT

	if (0 == err) {
		/* L2MUX header */
		l2hdr = l2mux_hdr(skb);
		/* proto id and length in L2 header */
		l3pid = l2mux_get_proto(l2hdr);
		l3len = l2mux_get_length(l2hdr);

		l2mux_write_stat(l3pid, l3len, UPLINK_DIR, dev);
	} else {
		pr_err("L2MUX TX skb invalid\n");
	}

#endif /* ACTIVATE_L2MUX_STAT */

	return err;
}
EXPORT_SYMBOL(l2mux_skb_tx);

int l2mux_audio_rx_register(l2mux_audio_fn *fn)
{
	int err = -EBUSY, handle;

	handle = ((int)fn & 0x00ffffff) | L2MUX_AUDIO_DEV_TYPE_RX;
	spin_lock(&l2mux_lock);
	if (l2mux_audio_rx_fn == NULL) {
		l2mux_audio_rx_handle = handle;
		l2mux_audio_rx_fn = fn;
		err = handle;
	}
	spin_unlock(&l2mux_lock);
	return err;
}
EXPORT_SYMBOL(l2mux_audio_rx_register);

int l2mux_audio_rx_unregister(int handle)
{
	int err = -EPROTONOSUPPORT;

	spin_lock(&l2mux_lock);
	if (l2mux_audio_rx_handle == handle) {
		l2mux_audio_rx_handle = 0;
		l2mux_audio_rx_fn = NULL;
		err = 0;
	}
	spin_unlock(&l2mux_lock);
	return err;
}
EXPORT_SYMBOL(l2mux_audio_rx_unregister);

int l2mux_audio_tx_register(uint8_t phonet_dev_id, l2mux_audio_fn *fn)
{
	int i;

	spin_lock(&l2mux_lock);
	for (i = 0; i < L2MUX_AUDIO_DEV_MAX; i++) {
		if (l2mux_audio_tx_tab[i] == NULL) {
			l2mux_audio_tx_tab[i] = fn;
			l2mux_audio_tx_pn_map[i] = phonet_dev_id;
			spin_unlock(&l2mux_lock);
			return (i | L2MUX_AUDIO_DEV_TYPE_TX);
		}
	}
	spin_unlock(&l2mux_lock);
	return -EBUSY;
}
EXPORT_SYMBOL(l2mux_audio_tx_register);

int l2mux_audio_tx_unregister(int handle)
{
	int err = -EPROTONOSUPPORT;
	int internal_dev_id = handle & (~L2MUX_AUDIO_DEV_TYPE_TX);

	if ((internal_dev_id < 0) || (internal_dev_id >= L2MUX_AUDIO_DEV_MAX))
		return err;

	spin_lock(&l2mux_lock);
	if (l2mux_audio_tx_tab[internal_dev_id] != NULL) {
		l2mux_audio_tx_tab[internal_dev_id] = NULL;
		l2mux_audio_tx_pn_map[internal_dev_id] = 0;
		err = 0;
	}
	spin_unlock(&l2mux_lock);
	return err;
}
EXPORT_SYMBOL(l2mux_audio_tx_unregister);

int l2mux_audio_rx(unsigned char *buffer, uint8_t pn_dev_id)
{
	struct l2muxhdr *l2hdr = (struct l2muxhdr *)buffer;
	unsigned l3len;

	/* proto id and length in L2 header */
	if (l2mux_get_proto(l2hdr) != MHI_L3_CELLULAR_AUDIO)
		return -EINVAL;

	l3len = l2mux_get_length(l2hdr);

	if (l2mux_audio_rx_fn == NULL)
		return -EINVAL;

	return l2mux_audio_rx_fn(buffer + L2MUX_HDR_SIZE, l3len, pn_dev_id);
}
EXPORT_SYMBOL(l2mux_audio_rx);

int l2mux_audio_tx(unsigned char *buffer, size_t size, uint8_t pn_dev_id)
{
	int i;
	l2mux_audio_fn *txfn = NULL;
	struct l2muxhdr *l2hdr;

	/* TODO for the future!!
	 * since we don't support multiple devices in RIL yet, we do not
	 * want to create confusion here, so we simply search for the
	 * first available txfn registered.  (supposedly, there should
	 * only be one */
#if 0
	for (i = 0; i < L2MUX_AUDIO_DEV_MAX; i++) {
		if (l2mux_audio_tx_pn_map[i] == pn_dev_id) {
			txfn = l2mux_audio_tx_tab[i];
			break;
		}
	}
#else
	for (i = 0; i < L2MUX_AUDIO_DEV_MAX; i++) {
		if (l2mux_audio_tx_tab[i] != NULL) {
			txfn = l2mux_audio_tx_tab[i];
			break;
		}
	}
#endif
	
	/* didn't find txfn for the pn_dev_id */
	if (txfn == NULL)
		return -EINVAL;

	l2hdr = (struct l2muxhdr *)(buffer - 4);
	l2mux_set_proto(l2hdr, MHI_L3_CELLULAR_AUDIO);
	l2mux_set_length(l2hdr, size);

	/* we don't care about return value from txfn */
	return txfn((unsigned char *)l2hdr, size + L2MUX_HDR_SIZE, pn_dev_id);
}
EXPORT_SYMBOL(l2mux_audio_tx);

static int __init l2mux_init(void)
{
	int i;

	DPRINTK("l2mux_init\n");

	for (i = 0; i < MHI_L3_NPROTO; i++)
		l2mux_id2rx_tab[i] = NULL;

	for (i = 0; i < ETH_NON_DIX_NPROTO; i++)
		l2mux_pt2tx_tab[i] = NULL;

	l2mux_audio_rx_fn = NULL;
	l2mux_audio_rx_handle = 0;

	for (i = 0; i < L2MUX_AUDIO_DEV_MAX; i++) {
		l2mux_audio_tx_tab[i] = NULL;
		l2mux_audio_tx_pn_map[i] = 0;
	}

#ifdef ACTIVATE_L2MUX_STAT
	init_l2mux_stat();

#endif /* ACTIVATE_L2MUX_STAT */

	return 0;
}

static void __exit l2mux_exit(void)
{
#ifdef ACTIVATE_L2MUX_STAT
	exit_l2mux_stat();
#endif /* ACTIVATE_L2MUX_STAT */
	DPRINTK("l2mux_exit\n");
}

module_init(l2mux_init);
module_exit(l2mux_exit);

MODULE_DESCRIPTION("L2MUX for MHI Protocol Stack");
#endif /* CONFIG_BCM_KF_MHI */
