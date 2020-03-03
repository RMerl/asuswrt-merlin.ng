
/*
 * mac80211 debugfs for wireless PHYs
 *
 * Copyright 2007	Johannes Berg <johannes@sipsolutions.net>
 * Copyright 2013-2014  Intel Mobile Communications GmbH
 *
 * GPLv2
 *
 */

#include <linux/debugfs.h>
#include <linux/rtnetlink.h>
#include "ieee80211_i.h"
#include "driver-ops.h"
#include "rate.h"
#include "debugfs.h"

#define DEBUGFS_FORMAT_BUFFER_SIZE 100

int mac80211_format_buffer(char __user *userbuf, size_t count,
				  loff_t *ppos, char *fmt, ...)
{
	va_list args;
	char buf[DEBUGFS_FORMAT_BUFFER_SIZE];
	int res;

	va_start(args, fmt);
	res = vscnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	return simple_read_from_buffer(userbuf, count, ppos, buf, res);
}

#define DEBUGFS_READONLY_FILE_FN(name, fmt, value...)			\
static ssize_t name## _read(struct file *file, char __user *userbuf,	\
			    size_t count, loff_t *ppos)			\
{									\
	struct ieee80211_local *local = file->private_data;		\
									\
	return mac80211_format_buffer(userbuf, count, ppos, 		\
				      fmt "\n", ##value);		\
}

#define DEBUGFS_READONLY_FILE_OPS(name)			\
static const struct file_operations name## _ops = {			\
	.read = name## _read,						\
	.open = simple_open,						\
	.llseek = generic_file_llseek,					\
};

#define DEBUGFS_READONLY_FILE(name, fmt, value...)		\
	DEBUGFS_READONLY_FILE_FN(name, fmt, value)		\
	DEBUGFS_READONLY_FILE_OPS(name)

#define DEBUGFS_ADD(name)						\
	debugfs_create_file(#name, 0400, phyd, local, &name## _ops);

#define DEBUGFS_ADD_MODE(name, mode)					\
	debugfs_create_file(#name, mode, phyd, local, &name## _ops);


DEBUGFS_READONLY_FILE(user_power, "%d",
		      local->user_power_level);
DEBUGFS_READONLY_FILE(power, "%d",
		      local->hw.conf.power_level);
DEBUGFS_READONLY_FILE(total_ps_buffered, "%d",
		      local->total_ps_buffered);
DEBUGFS_READONLY_FILE(wep_iv, "%#08x",
		      local->wep_iv & 0xffffff);
DEBUGFS_READONLY_FILE(rate_ctrl_alg, "%s",
	local->rate_ctrl ? local->rate_ctrl->ops->name : "hw/driver");

#ifdef CONFIG_PM
static ssize_t reset_write(struct file *file, const char __user *user_buf,
			   size_t count, loff_t *ppos)
{
	struct ieee80211_local *local = file->private_data;

	rtnl_lock();
	__ieee80211_suspend(&local->hw, NULL);
	__ieee80211_resume(&local->hw);
	rtnl_unlock();

	return count;
}

static const struct file_operations reset_ops = {
	.write = reset_write,
	.open = simple_open,
	.llseek = noop_llseek,
};
#endif

static ssize_t hwflags_read(struct file *file, char __user *user_buf,
			    size_t count, loff_t *ppos)
{
	struct ieee80211_local *local = file->private_data;
	int mxln = 500;
	ssize_t rv;
	char *buf = kzalloc(mxln, GFP_KERNEL);
	int sf = 0; /* how many written so far */

	if (!buf)
		return 0;

	sf += scnprintf(buf, mxln - sf, "0x%x\n", local->hw.flags);
	if (local->hw.flags & IEEE80211_HW_HAS_RATE_CONTROL)
		sf += scnprintf(buf + sf, mxln - sf, "HAS_RATE_CONTROL\n");
	if (local->hw.flags & IEEE80211_HW_RX_INCLUDES_FCS)
		sf += scnprintf(buf + sf, mxln - sf, "RX_INCLUDES_FCS\n");
	if (local->hw.flags & IEEE80211_HW_HOST_BROADCAST_PS_BUFFERING)
		sf += scnprintf(buf + sf, mxln - sf,
				"HOST_BCAST_PS_BUFFERING\n");
	if (local->hw.flags & IEEE80211_HW_2GHZ_SHORT_SLOT_INCAPABLE)
		sf += scnprintf(buf + sf, mxln - sf,
				"2GHZ_SHORT_SLOT_INCAPABLE\n");
	if (local->hw.flags & IEEE80211_HW_2GHZ_SHORT_PREAMBLE_INCAPABLE)
		sf += scnprintf(buf + sf, mxln - sf,
				"2GHZ_SHORT_PREAMBLE_INCAPABLE\n");
	if (local->hw.flags & IEEE80211_HW_SIGNAL_UNSPEC)
		sf += scnprintf(buf + sf, mxln - sf, "SIGNAL_UNSPEC\n");
	if (local->hw.flags & IEEE80211_HW_SIGNAL_DBM)
		sf += scnprintf(buf + sf, mxln - sf, "SIGNAL_DBM\n");
	if (local->hw.flags & IEEE80211_HW_NEED_DTIM_BEFORE_ASSOC)
		sf += scnprintf(buf + sf, mxln - sf,
				"NEED_DTIM_BEFORE_ASSOC\n");
	if (local->hw.flags & IEEE80211_HW_SPECTRUM_MGMT)
		sf += scnprintf(buf + sf, mxln - sf, "SPECTRUM_MGMT\n");
	if (local->hw.flags & IEEE80211_HW_AMPDU_AGGREGATION)
		sf += scnprintf(buf + sf, mxln - sf, "AMPDU_AGGREGATION\n");
	if (local->hw.flags & IEEE80211_HW_SUPPORTS_PS)
		sf += scnprintf(buf + sf, mxln - sf, "SUPPORTS_PS\n");
	if (local->hw.flags & IEEE80211_HW_PS_NULLFUNC_STACK)
		sf += scnprintf(buf + sf, mxln - sf, "PS_NULLFUNC_STACK\n");
	if (local->hw.flags & IEEE80211_HW_SUPPORTS_DYNAMIC_PS)
		sf += scnprintf(buf + sf, mxln - sf, "SUPPORTS_DYNAMIC_PS\n");
	if (local->hw.flags & IEEE80211_HW_MFP_CAPABLE)
		sf += scnprintf(buf + sf, mxln - sf, "MFP_CAPABLE\n");
	if (local->hw.flags & IEEE80211_HW_REPORTS_TX_ACK_STATUS)
		sf += scnprintf(buf + sf, mxln - sf,
				"REPORTS_TX_ACK_STATUS\n");
	if (local->hw.flags & IEEE80211_HW_CONNECTION_MONITOR)
		sf += scnprintf(buf + sf, mxln - sf, "CONNECTION_MONITOR\n");
	if (local->hw.flags & IEEE80211_HW_SUPPORTS_PER_STA_GTK)
		sf += scnprintf(buf + sf, mxln - sf, "SUPPORTS_PER_STA_GTK\n");
	if (local->hw.flags & IEEE80211_HW_AP_LINK_PS)
		sf += scnprintf(buf + sf, mxln - sf, "AP_LINK_PS\n");
	if (local->hw.flags & IEEE80211_HW_TX_AMPDU_SETUP_IN_HW)
		sf += scnprintf(buf + sf, mxln - sf, "TX_AMPDU_SETUP_IN_HW\n");

	rv = simple_read_from_buffer(user_buf, count, ppos, buf, strlen(buf));
	kfree(buf);
	return rv;
}

static ssize_t queues_read(struct file *file, char __user *user_buf,
			   size_t count, loff_t *ppos)
{
	struct ieee80211_local *local = file->private_data;
	unsigned long flags;
	char buf[IEEE80211_MAX_QUEUES * 20];
	int q, res = 0;

	spin_lock_irqsave(&local->queue_stop_reason_lock, flags);
	for (q = 0; q < local->hw.queues; q++)
		res += sprintf(buf + res, "%02d: %#.8lx/%d\n", q,
				local->queue_stop_reasons[q],
				skb_queue_len(&local->pending[q]));
	spin_unlock_irqrestore(&local->queue_stop_reason_lock, flags);

	return simple_read_from_buffer(user_buf, count, ppos, buf, res);
}

DEBUGFS_READONLY_FILE_OPS(hwflags);
DEBUGFS_READONLY_FILE_OPS(queues);

/* statistics stuff */

static ssize_t format_devstat_counter(struct ieee80211_local *local,
	char __user *userbuf,
	size_t count, loff_t *ppos,
	int (*printvalue)(struct ieee80211_low_level_stats *stats, char *buf,
			  int buflen))
{
	struct ieee80211_low_level_stats stats;
	char buf[20];
	int res;

	rtnl_lock();
	res = drv_get_stats(local, &stats);
	rtnl_unlock();
	if (res)
		return res;
	res = printvalue(&stats, buf, sizeof(buf));
	return simple_read_from_buffer(userbuf, count, ppos, buf, res);
}

#define DEBUGFS_DEVSTATS_FILE(name)					\
static int print_devstats_##name(struct ieee80211_low_level_stats *stats,\
				 char *buf, int buflen)			\
{									\
	return scnprintf(buf, buflen, "%u\n", stats->name);		\
}									\
static ssize_t stats_ ##name## _read(struct file *file,			\
				     char __user *userbuf,		\
				     size_t count, loff_t *ppos)	\
{									\
	return format_devstat_counter(file->private_data,		\
				      userbuf,				\
				      count,				\
				      ppos,				\
				      print_devstats_##name);		\
}									\
									\
static const struct file_operations stats_ ##name## _ops = {		\
	.read = stats_ ##name## _read,					\
	.open = simple_open,						\
	.llseek = generic_file_llseek,					\
};

#define DEBUGFS_STATS_ADD(name, field)					\
	debugfs_create_u32(#name, 0400, statsd, (u32 *) &field);
#define DEBUGFS_DEVSTATS_ADD(name)					\
	debugfs_create_file(#name, 0400, statsd, local, &stats_ ##name## _ops);

DEBUGFS_DEVSTATS_FILE(dot11ACKFailureCount);
DEBUGFS_DEVSTATS_FILE(dot11RTSFailureCount);
DEBUGFS_DEVSTATS_FILE(dot11FCSErrorCount);
DEBUGFS_DEVSTATS_FILE(dot11RTSSuccessCount);

void debugfs_hw_add(struct ieee80211_local *local)
{
	struct dentry *phyd = local->hw.wiphy->debugfsdir;
	struct dentry *statsd;

	if (!phyd)
		return;

	local->debugfs.keys = debugfs_create_dir("keys", phyd);

	DEBUGFS_ADD(total_ps_buffered);
	DEBUGFS_ADD(wep_iv);
	DEBUGFS_ADD(queues);
#ifdef CONFIG_PM
	DEBUGFS_ADD_MODE(reset, 0200);
#endif
	DEBUGFS_ADD(hwflags);
	DEBUGFS_ADD(user_power);
	DEBUGFS_ADD(power);

	statsd = debugfs_create_dir("statistics", phyd);

	/* if the dir failed, don't put all the other things into the root! */
	if (!statsd)
		return;

	DEBUGFS_STATS_ADD(transmitted_fragment_count,
		local->dot11TransmittedFragmentCount);
	DEBUGFS_STATS_ADD(multicast_transmitted_frame_count,
		local->dot11MulticastTransmittedFrameCount);
	DEBUGFS_STATS_ADD(failed_count, local->dot11FailedCount);
	DEBUGFS_STATS_ADD(retry_count, local->dot11RetryCount);
	DEBUGFS_STATS_ADD(multiple_retry_count,
		local->dot11MultipleRetryCount);
	DEBUGFS_STATS_ADD(frame_duplicate_count,
		local->dot11FrameDuplicateCount);
	DEBUGFS_STATS_ADD(received_fragment_count,
		local->dot11ReceivedFragmentCount);
	DEBUGFS_STATS_ADD(multicast_received_frame_count,
		local->dot11MulticastReceivedFrameCount);
	DEBUGFS_STATS_ADD(transmitted_frame_count,
		local->dot11TransmittedFrameCount);
#ifdef CONFIG_MAC80211_DEBUG_COUNTERS
	DEBUGFS_STATS_ADD(tx_handlers_drop, local->tx_handlers_drop);
	DEBUGFS_STATS_ADD(tx_handlers_queued, local->tx_handlers_queued);
	DEBUGFS_STATS_ADD(tx_handlers_drop_fragment,
		local->tx_handlers_drop_fragment);
	DEBUGFS_STATS_ADD(tx_handlers_drop_wep,
		local->tx_handlers_drop_wep);
	DEBUGFS_STATS_ADD(tx_handlers_drop_not_assoc,
		local->tx_handlers_drop_not_assoc);
	DEBUGFS_STATS_ADD(tx_handlers_drop_unauth_port,
		local->tx_handlers_drop_unauth_port);
	DEBUGFS_STATS_ADD(rx_handlers_drop, local->rx_handlers_drop);
	DEBUGFS_STATS_ADD(rx_handlers_queued, local->rx_handlers_queued);
	DEBUGFS_STATS_ADD(rx_handlers_drop_nullfunc,
		local->rx_handlers_drop_nullfunc);
	DEBUGFS_STATS_ADD(rx_handlers_drop_defrag,
		local->rx_handlers_drop_defrag);
	DEBUGFS_STATS_ADD(rx_handlers_drop_short,
		local->rx_handlers_drop_short);
	DEBUGFS_STATS_ADD(tx_expand_skb_head,
		local->tx_expand_skb_head);
	DEBUGFS_STATS_ADD(tx_expand_skb_head_cloned,
		local->tx_expand_skb_head_cloned);
	DEBUGFS_STATS_ADD(rx_expand_skb_head,
		local->rx_expand_skb_head);
	DEBUGFS_STATS_ADD(rx_expand_skb_head2,
		local->rx_expand_skb_head2);
	DEBUGFS_STATS_ADD(rx_handlers_fragments,
		local->rx_handlers_fragments);
	DEBUGFS_STATS_ADD(tx_status_drop,
		local->tx_status_drop);
#endif
	DEBUGFS_DEVSTATS_ADD(dot11ACKFailureCount);
	DEBUGFS_DEVSTATS_ADD(dot11RTSFailureCount);
	DEBUGFS_DEVSTATS_ADD(dot11FCSErrorCount);
	DEBUGFS_DEVSTATS_ADD(dot11RTSSuccessCount);
}
