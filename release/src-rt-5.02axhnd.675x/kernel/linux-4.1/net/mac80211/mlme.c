/*
 * BSS client mode implementation
 * Copyright 2003-2008, Jouni Malinen <j@w1.fi>
 * Copyright 2004, Instant802 Networks, Inc.
 * Copyright 2005, Devicescape Software, Inc.
 * Copyright 2006-2007	Jiri Benc <jbenc@suse.cz>
 * Copyright 2007, Michael Wu <flamingice@sourmilk.net>
 * Copyright 2013-2014  Intel Mobile Communications GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/delay.h>
#include <linux/if_ether.h>
#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/etherdevice.h>
#include <linux/moduleparam.h>
#include <linux/rtnetlink.h>
#include <linux/pm_qos.h>
#include <linux/crc32.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <net/mac80211.h>
#include <asm/unaligned.h>

#include "ieee80211_i.h"
#include "driver-ops.h"
#include "rate.h"
#include "led.h"

#define IEEE80211_AUTH_TIMEOUT		(HZ / 5)
#define IEEE80211_AUTH_TIMEOUT_LONG	(HZ / 2)
#define IEEE80211_AUTH_TIMEOUT_SHORT	(HZ / 10)
#define IEEE80211_AUTH_MAX_TRIES	3
#define IEEE80211_AUTH_WAIT_ASSOC	(HZ * 5)
#define IEEE80211_ASSOC_TIMEOUT		(HZ / 5)
#define IEEE80211_ASSOC_TIMEOUT_LONG	(HZ / 2)
#define IEEE80211_ASSOC_TIMEOUT_SHORT	(HZ / 10)
#define IEEE80211_ASSOC_MAX_TRIES	3

static int max_nullfunc_tries = 2;
module_param(max_nullfunc_tries, int, 0644);
MODULE_PARM_DESC(max_nullfunc_tries,
		 "Maximum nullfunc tx tries before disconnecting (reason 4).");

static int max_probe_tries = 5;
module_param(max_probe_tries, int, 0644);
MODULE_PARM_DESC(max_probe_tries,
		 "Maximum probe tries before disconnecting (reason 4).");

/*
 * Beacon loss timeout is calculated as N frames times the
 * advertised beacon interval.  This may need to be somewhat
 * higher than what hardware might detect to account for
 * delays in the host processing frames. But since we also
 * probe on beacon miss before declaring the connection lost
 * default to what we want.
 */
static int beacon_loss_count = 7;
module_param(beacon_loss_count, int, 0644);
MODULE_PARM_DESC(beacon_loss_count,
		 "Number of beacon intervals before we decide beacon was lost.");

/*
 * Time the connection can be idle before we probe
 * it to see if we can still talk to the AP.
 */
#define IEEE80211_CONNECTION_IDLE_TIME	(30 * HZ)
/*
 * Time we wait for a probe response after sending
 * a probe request because of beacon loss or for
 * checking the connection still works.
 */
static int probe_wait_ms = 500;
module_param(probe_wait_ms, int, 0644);
MODULE_PARM_DESC(probe_wait_ms,
		 "Maximum time(ms) to wait for probe response"
		 " before disconnecting (reason 4).");

/*
 * Weight given to the latest Beacon frame when calculating average signal
 * strength for Beacon frames received in the current BSS. This must be
 * between 1 and 15.
 */
#define IEEE80211_SIGNAL_AVE_WEIGHT	3

/*
 * How many Beacon frames need to have been used in average signal strength
 * before starting to indicate signal change events.
 */
#define IEEE80211_SIGNAL_AVE_MIN_COUNT	4

/*
 * We can have multiple work items (and connection probing)
 * scheduling this timer, but we need to take care to only
 * reschedule it when it should fire _earlier_ than it was
 * asked for before, or if it's not pending right now. This
 * function ensures that. Note that it then is required to
 * run this function for all timeouts after the first one
 * has happened -- the work that runs from this timer will
 * do that.
 */
static void run_again(struct ieee80211_sub_if_data *sdata,
		      unsigned long timeout)
{
	sdata_assert_lock(sdata);

	if (!timer_pending(&sdata->u.mgd.timer) ||
	    time_before(timeout, sdata->u.mgd.timer.expires))
		mod_timer(&sdata->u.mgd.timer, timeout);
}

void ieee80211_sta_reset_beacon_monitor(struct ieee80211_sub_if_data *sdata)
{
	if (sdata->vif.driver_flags & IEEE80211_VIF_BEACON_FILTER)
		return;

	if (sdata->local->hw.flags & IEEE80211_HW_CONNECTION_MONITOR)
		return;

	mod_timer(&sdata->u.mgd.bcn_mon_timer,
		  round_jiffies_up(jiffies + sdata->u.mgd.beacon_timeout));
}

void ieee80211_sta_reset_conn_monitor(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;

	if (unlikely(!sdata->u.mgd.associated))
		return;

	ifmgd->probe_send_count = 0;

	if (sdata->local->hw.flags & IEEE80211_HW_CONNECTION_MONITOR)
		return;

	mod_timer(&sdata->u.mgd.conn_mon_timer,
		  round_jiffies_up(jiffies + IEEE80211_CONNECTION_IDLE_TIME));
}

static int ecw2cw(int ecw)
{
	return (1 << ecw) - 1;
}

static u32
ieee80211_determine_chantype(struct ieee80211_sub_if_data *sdata,
			     struct ieee80211_supported_band *sband,
			     struct ieee80211_channel *channel,
			     const struct ieee80211_ht_cap *ht_cap,
			     const struct ieee80211_ht_operation *ht_oper,
			     const struct ieee80211_vht_operation *vht_oper,
			     struct cfg80211_chan_def *chandef, bool tracking)
{
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	struct cfg80211_chan_def vht_chandef;
	struct ieee80211_sta_ht_cap sta_ht_cap;
	u32 ht_cfreq, ret;

	memcpy(&sta_ht_cap, &sband->ht_cap, sizeof(sta_ht_cap));
	ieee80211_apply_htcap_overrides(sdata, &sta_ht_cap);

	chandef->chan = channel;
	chandef->width = NL80211_CHAN_WIDTH_20_NOHT;
	chandef->center_freq1 = channel->center_freq;
	chandef->center_freq2 = 0;

	if (!ht_cap || !ht_oper || !sta_ht_cap.ht_supported) {
		ret = IEEE80211_STA_DISABLE_HT | IEEE80211_STA_DISABLE_VHT;
		goto out;
	}

	chandef->width = NL80211_CHAN_WIDTH_20;

	if (!(ht_cap->cap_info &
	      cpu_to_le16(IEEE80211_HT_CAP_SUP_WIDTH_20_40))) {
		ret = IEEE80211_STA_DISABLE_40MHZ;
		vht_chandef = *chandef;
		goto out;
	}

	ht_cfreq = ieee80211_channel_to_frequency(ht_oper->primary_chan,
						  channel->band);
	/* check that channel matches the right operating channel */
	if (!tracking && channel->center_freq != ht_cfreq) {
		/*
		 * It's possible that some APs are confused here;
		 * Netgear WNDR3700 sometimes reports 4 higher than
		 * the actual channel in association responses, but
		 * since we look at probe response/beacon data here
		 * it should be OK.
		 */
		sdata_info(sdata,
			   "Wrong control channel: center-freq: %d ht-cfreq: %d ht->primary_chan: %d band: %d - Disabling HT\n",
			   channel->center_freq, ht_cfreq,
			   ht_oper->primary_chan, channel->band);
		ret = IEEE80211_STA_DISABLE_HT | IEEE80211_STA_DISABLE_VHT;
		goto out;
	}

	/* check 40 MHz support, if we have it */
	if (sta_ht_cap.cap & IEEE80211_HT_CAP_SUP_WIDTH_20_40) {
		switch (ht_oper->ht_param & IEEE80211_HT_PARAM_CHA_SEC_OFFSET) {
		case IEEE80211_HT_PARAM_CHA_SEC_ABOVE:
			chandef->width = NL80211_CHAN_WIDTH_40;
			chandef->center_freq1 += 10;
			break;
		case IEEE80211_HT_PARAM_CHA_SEC_BELOW:
			chandef->width = NL80211_CHAN_WIDTH_40;
			chandef->center_freq1 -= 10;
			break;
		}
	} else {
		/* 40 MHz (and 80 MHz) must be supported for VHT */
		ret = IEEE80211_STA_DISABLE_VHT;
		/* also mark 40 MHz disabled */
		ret |= IEEE80211_STA_DISABLE_40MHZ;
		goto out;
	}

	if (!vht_oper || !sband->vht_cap.vht_supported) {
		ret = IEEE80211_STA_DISABLE_VHT;
		goto out;
	}

	vht_chandef.chan = channel;
	vht_chandef.center_freq1 =
		ieee80211_channel_to_frequency(vht_oper->center_freq_seg1_idx,
					       channel->band);
	vht_chandef.center_freq2 = 0;

	switch (vht_oper->chan_width) {
	case IEEE80211_VHT_CHANWIDTH_USE_HT:
		vht_chandef.width = chandef->width;
		vht_chandef.center_freq1 = chandef->center_freq1;
		break;
	case IEEE80211_VHT_CHANWIDTH_80MHZ:
		vht_chandef.width = NL80211_CHAN_WIDTH_80;
		break;
	case IEEE80211_VHT_CHANWIDTH_160MHZ:
		vht_chandef.width = NL80211_CHAN_WIDTH_160;
		break;
	case IEEE80211_VHT_CHANWIDTH_80P80MHZ:
		vht_chandef.width = NL80211_CHAN_WIDTH_80P80;
		vht_chandef.center_freq2 =
			ieee80211_channel_to_frequency(
				vht_oper->center_freq_seg2_idx,
				channel->band);
		break;
	default:
		if (!(ifmgd->flags & IEEE80211_STA_DISABLE_VHT))
			sdata_info(sdata,
				   "AP VHT operation IE has invalid channel width (%d), disable VHT\n",
				   vht_oper->chan_width);
		ret = IEEE80211_STA_DISABLE_VHT;
		goto out;
	}

	if (!cfg80211_chandef_valid(&vht_chandef)) {
		if (!(ifmgd->flags & IEEE80211_STA_DISABLE_VHT))
			sdata_info(sdata,
				   "AP VHT information is invalid, disable VHT\n");
		ret = IEEE80211_STA_DISABLE_VHT;
		goto out;
	}

	if (cfg80211_chandef_identical(chandef, &vht_chandef)) {
		ret = 0;
		goto out;
	}

	if (!cfg80211_chandef_compatible(chandef, &vht_chandef)) {
		if (!(ifmgd->flags & IEEE80211_STA_DISABLE_VHT))
			sdata_info(sdata,
				   "AP VHT information doesn't match HT, disable VHT\n");
		ret = IEEE80211_STA_DISABLE_VHT;
		goto out;
	}

	*chandef = vht_chandef;

	ret = 0;

out:
	/*
	 * When tracking the current AP, don't do any further checks if the
	 * new chandef is identical to the one we're currently using for the
	 * connection. This keeps us from playing ping-pong with regulatory,
	 * without it the following can happen (for example):
	 *  - connect to an AP with 80 MHz, world regdom allows 80 MHz
	 *  - AP advertises regdom US
	 *  - CRDA loads regdom US with 80 MHz prohibited (old database)
	 *  - the code below detects an unsupported channel, downgrades, and
	 *    we disconnect from the AP in the caller
	 *  - disconnect causes CRDA to reload world regdomain and the game
	 *    starts anew.
	 * (see https://bugzilla.kernel.org/show_bug.cgi?id=70881)
	 *
	 * It seems possible that there are still scenarios with CSA or real
	 * bandwidth changes where a this could happen, but those cases are
	 * less common and wouldn't completely prevent using the AP.
	 */
	if (tracking &&
	    cfg80211_chandef_identical(chandef, &sdata->vif.bss_conf.chandef))
		return ret;

	/* don't print the message below for VHT mismatch if VHT is disabled */
	if (ret & IEEE80211_STA_DISABLE_VHT)
		vht_chandef = *chandef;

	/*
	 * Ignore the DISABLED flag when we're already connected and only
	 * tracking the APs beacon for bandwidth changes - otherwise we
	 * might get disconnected here if we connect to an AP, update our
	 * regulatory information based on the AP's country IE and the
	 * information we have is wrong/outdated and disables the channel
	 * that we're actually using for the connection to the AP.
	 */
	while (!cfg80211_chandef_usable(sdata->local->hw.wiphy, chandef,
					tracking ? 0 :
						   IEEE80211_CHAN_DISABLED)) {
		if (WARN_ON(chandef->width == NL80211_CHAN_WIDTH_20_NOHT)) {
			ret = IEEE80211_STA_DISABLE_HT |
			      IEEE80211_STA_DISABLE_VHT;
			break;
		}

		ret |= ieee80211_chandef_downgrade(chandef);
	}

	if (chandef->width != vht_chandef.width && !tracking)
		sdata_info(sdata,
			   "capabilities/regulatory prevented using AP HT/VHT configuration, downgraded\n");

	WARN_ON_ONCE(!cfg80211_chandef_valid(chandef));
	return ret;
}

static int ieee80211_config_bw(struct ieee80211_sub_if_data *sdata,
			       struct sta_info *sta,
			       const struct ieee80211_ht_cap *ht_cap,
			       const struct ieee80211_ht_operation *ht_oper,
			       const struct ieee80211_vht_operation *vht_oper,
			       const u8 *bssid, u32 *changed)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	struct ieee80211_supported_band *sband;
	struct ieee80211_channel *chan;
	struct cfg80211_chan_def chandef;
	u16 ht_opmode;
	u32 flags;
	enum ieee80211_sta_rx_bandwidth new_sta_bw;
	int ret;

	/* if HT was/is disabled, don't track any bandwidth changes */
	if (ifmgd->flags & IEEE80211_STA_DISABLE_HT || !ht_oper)
		return 0;

	/* don't check VHT if we associated as non-VHT station */
	if (ifmgd->flags & IEEE80211_STA_DISABLE_VHT)
		vht_oper = NULL;

	if (WARN_ON_ONCE(!sta))
		return -EINVAL;

	/*
	 * if bss configuration changed store the new one -
	 * this may be applicable even if channel is identical
	 */
	ht_opmode = le16_to_cpu(ht_oper->operation_mode);
	if (sdata->vif.bss_conf.ht_operation_mode != ht_opmode) {
		*changed |= BSS_CHANGED_HT;
		sdata->vif.bss_conf.ht_operation_mode = ht_opmode;
	}

	chan = sdata->vif.bss_conf.chandef.chan;
	sband = local->hw.wiphy->bands[chan->band];

	/* calculate new channel (type) based on HT/VHT operation IEs */
	flags = ieee80211_determine_chantype(sdata, sband, chan,
					     ht_cap, ht_oper, vht_oper,
					     &chandef, true);

	/*
	 * Downgrade the new channel if we associated with restricted
	 * capabilities. For example, if we associated as a 20 MHz STA
	 * to a 40 MHz AP (due to regulatory, capabilities or config
	 * reasons) then switching to a 40 MHz channel now won't do us
	 * any good -- we couldn't use it with the AP.
	 */
	if (ifmgd->flags & IEEE80211_STA_DISABLE_80P80MHZ &&
	    chandef.width == NL80211_CHAN_WIDTH_80P80)
		flags |= ieee80211_chandef_downgrade(&chandef);
	if (ifmgd->flags & IEEE80211_STA_DISABLE_160MHZ &&
	    chandef.width == NL80211_CHAN_WIDTH_160)
		flags |= ieee80211_chandef_downgrade(&chandef);
	if (ifmgd->flags & IEEE80211_STA_DISABLE_40MHZ &&
	    chandef.width > NL80211_CHAN_WIDTH_20)
		flags |= ieee80211_chandef_downgrade(&chandef);

	if (cfg80211_chandef_identical(&chandef, &sdata->vif.bss_conf.chandef))
		return 0;

	sdata_info(sdata,
		   "AP %pM changed bandwidth, new config is %d MHz, width %d (%d/%d MHz)\n",
		   ifmgd->bssid, chandef.chan->center_freq, chandef.width,
		   chandef.center_freq1, chandef.center_freq2);

	if (flags != (ifmgd->flags & (IEEE80211_STA_DISABLE_HT |
				      IEEE80211_STA_DISABLE_VHT |
				      IEEE80211_STA_DISABLE_40MHZ |
				      IEEE80211_STA_DISABLE_80P80MHZ |
				      IEEE80211_STA_DISABLE_160MHZ)) ||
	    !cfg80211_chandef_valid(&chandef)) {
		sdata_info(sdata,
			   "AP %pM changed bandwidth in a way we can't support - disconnect\n",
			   ifmgd->bssid);
		return -EINVAL;
	}

	switch (chandef.width) {
	case NL80211_CHAN_WIDTH_20_NOHT:
	case NL80211_CHAN_WIDTH_20:
		new_sta_bw = IEEE80211_STA_RX_BW_20;
		break;
	case NL80211_CHAN_WIDTH_40:
		new_sta_bw = IEEE80211_STA_RX_BW_40;
		break;
	case NL80211_CHAN_WIDTH_80:
		new_sta_bw = IEEE80211_STA_RX_BW_80;
		break;
	case NL80211_CHAN_WIDTH_80P80:
	case NL80211_CHAN_WIDTH_160:
		new_sta_bw = IEEE80211_STA_RX_BW_160;
		break;
	default:
		return -EINVAL;
	}

	if (new_sta_bw > sta->cur_max_bandwidth)
		new_sta_bw = sta->cur_max_bandwidth;

	if (new_sta_bw < sta->sta.bandwidth) {
		sta->sta.bandwidth = new_sta_bw;
		rate_control_rate_update(local, sband, sta,
					 IEEE80211_RC_BW_CHANGED);
	}

	ret = ieee80211_vif_change_bandwidth(sdata, &chandef, changed);
	if (ret) {
		sdata_info(sdata,
			   "AP %pM changed bandwidth to incompatible one - disconnect\n",
			   ifmgd->bssid);
		return ret;
	}

	if (new_sta_bw > sta->sta.bandwidth) {
		sta->sta.bandwidth = new_sta_bw;
		rate_control_rate_update(local, sband, sta,
					 IEEE80211_RC_BW_CHANGED);
	}

	return 0;
}

/* frame sending functions */

static void ieee80211_add_ht_ie(struct ieee80211_sub_if_data *sdata,
				struct sk_buff *skb, u8 ap_ht_param,
				struct ieee80211_supported_band *sband,
				struct ieee80211_channel *channel,
				enum ieee80211_smps_mode smps)
{
	u8 *pos;
	u32 flags = channel->flags;
	u16 cap;
	struct ieee80211_sta_ht_cap ht_cap;

	BUILD_BUG_ON(sizeof(ht_cap) != sizeof(sband->ht_cap));

	memcpy(&ht_cap, &sband->ht_cap, sizeof(ht_cap));
	ieee80211_apply_htcap_overrides(sdata, &ht_cap);

	/* determine capability flags */
	cap = ht_cap.cap;

	switch (ap_ht_param & IEEE80211_HT_PARAM_CHA_SEC_OFFSET) {
	case IEEE80211_HT_PARAM_CHA_SEC_ABOVE:
		if (flags & IEEE80211_CHAN_NO_HT40PLUS) {
			cap &= ~IEEE80211_HT_CAP_SUP_WIDTH_20_40;
			cap &= ~IEEE80211_HT_CAP_SGI_40;
		}
		break;
	case IEEE80211_HT_PARAM_CHA_SEC_BELOW:
		if (flags & IEEE80211_CHAN_NO_HT40MINUS) {
			cap &= ~IEEE80211_HT_CAP_SUP_WIDTH_20_40;
			cap &= ~IEEE80211_HT_CAP_SGI_40;
		}
		break;
	}

	/*
	 * If 40 MHz was disabled associate as though we weren't
	 * capable of 40 MHz -- some broken APs will never fall
	 * back to trying to transmit in 20 MHz.
	 */
	if (sdata->u.mgd.flags & IEEE80211_STA_DISABLE_40MHZ) {
		cap &= ~IEEE80211_HT_CAP_SUP_WIDTH_20_40;
		cap &= ~IEEE80211_HT_CAP_SGI_40;
	}

	/* set SM PS mode properly */
	cap &= ~IEEE80211_HT_CAP_SM_PS;
	switch (smps) {
	case IEEE80211_SMPS_AUTOMATIC:
	case IEEE80211_SMPS_NUM_MODES:
		WARN_ON(1);
	case IEEE80211_SMPS_OFF:
		cap |= WLAN_HT_CAP_SM_PS_DISABLED <<
			IEEE80211_HT_CAP_SM_PS_SHIFT;
		break;
	case IEEE80211_SMPS_STATIC:
		cap |= WLAN_HT_CAP_SM_PS_STATIC <<
			IEEE80211_HT_CAP_SM_PS_SHIFT;
		break;
	case IEEE80211_SMPS_DYNAMIC:
		cap |= WLAN_HT_CAP_SM_PS_DYNAMIC <<
			IEEE80211_HT_CAP_SM_PS_SHIFT;
		break;
	}

	/* reserve and fill IE */
	pos = skb_put(skb, sizeof(struct ieee80211_ht_cap) + 2);
	ieee80211_ie_build_ht_cap(pos, &ht_cap, cap);
}

static void ieee80211_add_vht_ie(struct ieee80211_sub_if_data *sdata,
				 struct sk_buff *skb,
				 struct ieee80211_supported_band *sband,
				 struct ieee80211_vht_cap *ap_vht_cap)
{
	u8 *pos;
	u32 cap;
	struct ieee80211_sta_vht_cap vht_cap;
	u32 mask, ap_bf_sts, our_bf_sts;

	BUILD_BUG_ON(sizeof(vht_cap) != sizeof(sband->vht_cap));

	memcpy(&vht_cap, &sband->vht_cap, sizeof(vht_cap));
	ieee80211_apply_vhtcap_overrides(sdata, &vht_cap);

	/* determine capability flags */
	cap = vht_cap.cap;

	if (sdata->u.mgd.flags & IEEE80211_STA_DISABLE_80P80MHZ) {
		u32 bw = cap & IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_MASK;

		cap &= ~IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_MASK;
		if (bw == IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ ||
		    bw == IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ)
			cap |= IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ;
	}

	if (sdata->u.mgd.flags & IEEE80211_STA_DISABLE_160MHZ) {
		cap &= ~IEEE80211_VHT_CAP_SHORT_GI_160;
		cap &= ~IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_MASK;
	}

	/*
	 * Some APs apparently get confused if our capabilities are better
	 * than theirs, so restrict what we advertise in the assoc request.
	 */
	if (!(ap_vht_cap->vht_cap_info &
			cpu_to_le32(IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE)))
		cap &= ~IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE;

	mask = IEEE80211_VHT_CAP_BEAMFORMEE_STS_MASK;

	ap_bf_sts = le32_to_cpu(ap_vht_cap->vht_cap_info) & mask;
	our_bf_sts = cap & mask;

	if (ap_bf_sts < our_bf_sts) {
		cap &= ~mask;
		cap |= ap_bf_sts;
	}

	/* reserve and fill IE */
	pos = skb_put(skb, sizeof(struct ieee80211_vht_cap) + 2);
	ieee80211_ie_build_vht_cap(pos, &vht_cap, cap);
}

static void ieee80211_send_assoc(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	struct ieee80211_mgd_assoc_data *assoc_data = ifmgd->assoc_data;
	struct sk_buff *skb;
	struct ieee80211_mgmt *mgmt;
	u8 *pos, qos_info;
	size_t offset = 0, noffset;
	int i, count, rates_len, supp_rates_len, shift;
	u16 capab;
	struct ieee80211_supported_band *sband;
	struct ieee80211_chanctx_conf *chanctx_conf;
	struct ieee80211_channel *chan;
	u32 rate_flags, rates = 0;

	sdata_assert_lock(sdata);

	rcu_read_lock();
	chanctx_conf = rcu_dereference(sdata->vif.chanctx_conf);
	if (WARN_ON(!chanctx_conf)) {
		rcu_read_unlock();
		return;
	}
	chan = chanctx_conf->def.chan;
	rate_flags = ieee80211_chandef_rate_flags(&chanctx_conf->def);
	rcu_read_unlock();
	sband = local->hw.wiphy->bands[chan->band];
	shift = ieee80211_vif_get_shift(&sdata->vif);

	if (assoc_data->supp_rates_len) {
		/*
		 * Get all rates supported by the device and the AP as
		 * some APs don't like getting a superset of their rates
		 * in the association request (e.g. D-Link DAP 1353 in
		 * b-only mode)...
		 */
		rates_len = ieee80211_parse_bitrates(&chanctx_conf->def, sband,
						     assoc_data->supp_rates,
						     assoc_data->supp_rates_len,
						     &rates);
	} else {
		/*
		 * In case AP not provide any supported rates information
		 * before association, we send information element(s) with
		 * all rates that we support.
		 */
		rates_len = 0;
		for (i = 0; i < sband->n_bitrates; i++) {
			if ((rate_flags & sband->bitrates[i].flags)
			    != rate_flags)
				continue;
			rates |= BIT(i);
			rates_len++;
		}
	}

	skb = alloc_skb(local->hw.extra_tx_headroom +
			sizeof(*mgmt) + /* bit too much but doesn't matter */
			2 + assoc_data->ssid_len + /* SSID */
			4 + rates_len + /* (extended) rates */
			4 + /* power capability */
			2 + 2 * sband->n_channels + /* supported channels */
			2 + sizeof(struct ieee80211_ht_cap) + /* HT */
			2 + sizeof(struct ieee80211_vht_cap) + /* VHT */
			assoc_data->ie_len + /* extra IEs */
			9, /* WMM */
			GFP_KERNEL);
	if (!skb)
		return;

	skb_reserve(skb, local->hw.extra_tx_headroom);

	capab = WLAN_CAPABILITY_ESS;

	if (sband->band == IEEE80211_BAND_2GHZ) {
		if (!(local->hw.flags & IEEE80211_HW_2GHZ_SHORT_SLOT_INCAPABLE))
			capab |= WLAN_CAPABILITY_SHORT_SLOT_TIME;
		if (!(local->hw.flags & IEEE80211_HW_2GHZ_SHORT_PREAMBLE_INCAPABLE))
			capab |= WLAN_CAPABILITY_SHORT_PREAMBLE;
	}

	if (assoc_data->capability & WLAN_CAPABILITY_PRIVACY)
		capab |= WLAN_CAPABILITY_PRIVACY;

	if ((assoc_data->capability & WLAN_CAPABILITY_SPECTRUM_MGMT) &&
	    (local->hw.flags & IEEE80211_HW_SPECTRUM_MGMT))
		capab |= WLAN_CAPABILITY_SPECTRUM_MGMT;

	if (ifmgd->flags & IEEE80211_STA_ENABLE_RRM)
		capab |= WLAN_CAPABILITY_RADIO_MEASURE;

	mgmt = (struct ieee80211_mgmt *) skb_put(skb, 24);
	memset(mgmt, 0, 24);
	memcpy(mgmt->da, assoc_data->bss->bssid, ETH_ALEN);
	memcpy(mgmt->sa, sdata->vif.addr, ETH_ALEN);
	memcpy(mgmt->bssid, assoc_data->bss->bssid, ETH_ALEN);

	if (!is_zero_ether_addr(assoc_data->prev_bssid)) {
		skb_put(skb, 10);
		mgmt->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT |
						  IEEE80211_STYPE_REASSOC_REQ);
		mgmt->u.reassoc_req.capab_info = cpu_to_le16(capab);
		mgmt->u.reassoc_req.listen_interval =
				cpu_to_le16(local->hw.conf.listen_interval);
		memcpy(mgmt->u.reassoc_req.current_ap, assoc_data->prev_bssid,
		       ETH_ALEN);
	} else {
		skb_put(skb, 4);
		mgmt->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT |
						  IEEE80211_STYPE_ASSOC_REQ);
		mgmt->u.assoc_req.capab_info = cpu_to_le16(capab);
		mgmt->u.assoc_req.listen_interval =
				cpu_to_le16(local->hw.conf.listen_interval);
	}

	/* SSID */
	pos = skb_put(skb, 2 + assoc_data->ssid_len);
	*pos++ = WLAN_EID_SSID;
	*pos++ = assoc_data->ssid_len;
	memcpy(pos, assoc_data->ssid, assoc_data->ssid_len);

	/* add all rates which were marked to be used above */
	supp_rates_len = rates_len;
	if (supp_rates_len > 8)
		supp_rates_len = 8;

	pos = skb_put(skb, supp_rates_len + 2);
	*pos++ = WLAN_EID_SUPP_RATES;
	*pos++ = supp_rates_len;

	count = 0;
	for (i = 0; i < sband->n_bitrates; i++) {
		if (BIT(i) & rates) {
			int rate = DIV_ROUND_UP(sband->bitrates[i].bitrate,
						5 * (1 << shift));
			*pos++ = (u8) rate;
			if (++count == 8)
				break;
		}
	}

	if (rates_len > count) {
		pos = skb_put(skb, rates_len - count + 2);
		*pos++ = WLAN_EID_EXT_SUPP_RATES;
		*pos++ = rates_len - count;

		for (i++; i < sband->n_bitrates; i++) {
			if (BIT(i) & rates) {
				int rate;
				rate = DIV_ROUND_UP(sband->bitrates[i].bitrate,
						    5 * (1 << shift));
				*pos++ = (u8) rate;
			}
		}
	}

	if (capab & WLAN_CAPABILITY_SPECTRUM_MGMT ||
	    capab & WLAN_CAPABILITY_RADIO_MEASURE) {
		pos = skb_put(skb, 4);
		*pos++ = WLAN_EID_PWR_CAPABILITY;
		*pos++ = 2;
		*pos++ = 0; /* min tx power */
		 /* max tx power */
		*pos++ = ieee80211_chandef_max_power(&chanctx_conf->def);
	}

	if (capab & WLAN_CAPABILITY_SPECTRUM_MGMT) {
		/* TODO: get this in reg domain format */
		pos = skb_put(skb, 2 * sband->n_channels + 2);
		*pos++ = WLAN_EID_SUPPORTED_CHANNELS;
		*pos++ = 2 * sband->n_channels;
		for (i = 0; i < sband->n_channels; i++) {
			*pos++ = ieee80211_frequency_to_channel(
					sband->channels[i].center_freq);
			*pos++ = 1; /* one channel in the subband*/
		}
	}

	/* if present, add any custom IEs that go before HT */
	if (assoc_data->ie_len) {
		static const u8 before_ht[] = {
			WLAN_EID_SSID,
			WLAN_EID_SUPP_RATES,
			WLAN_EID_EXT_SUPP_RATES,
			WLAN_EID_PWR_CAPABILITY,
			WLAN_EID_SUPPORTED_CHANNELS,
			WLAN_EID_RSN,
			WLAN_EID_QOS_CAPA,
			WLAN_EID_RRM_ENABLED_CAPABILITIES,
			WLAN_EID_MOBILITY_DOMAIN,
			WLAN_EID_FAST_BSS_TRANSITION,	/* reassoc only */
			WLAN_EID_RIC_DATA,		/* reassoc only */
			WLAN_EID_SUPPORTED_REGULATORY_CLASSES,
		};
		static const u8 after_ric[] = {
			WLAN_EID_SUPPORTED_REGULATORY_CLASSES,
			WLAN_EID_HT_CAPABILITY,
			WLAN_EID_BSS_COEX_2040,
			WLAN_EID_EXT_CAPABILITY,
			WLAN_EID_QOS_TRAFFIC_CAPA,
			WLAN_EID_TIM_BCAST_REQ,
			WLAN_EID_INTERWORKING,
			/* 60GHz doesn't happen right now */
			WLAN_EID_VHT_CAPABILITY,
			WLAN_EID_OPMODE_NOTIF,
		};

		noffset = ieee80211_ie_split_ric(assoc_data->ie,
						 assoc_data->ie_len,
						 before_ht,
						 ARRAY_SIZE(before_ht),
						 after_ric,
						 ARRAY_SIZE(after_ric),
						 offset);
		pos = skb_put(skb, noffset - offset);
		memcpy(pos, assoc_data->ie + offset, noffset - offset);
		offset = noffset;
	}

	if (WARN_ON_ONCE((ifmgd->flags & IEEE80211_STA_DISABLE_HT) &&
			 !(ifmgd->flags & IEEE80211_STA_DISABLE_VHT)))
		ifmgd->flags |= IEEE80211_STA_DISABLE_VHT;

	if (!(ifmgd->flags & IEEE80211_STA_DISABLE_HT))
		ieee80211_add_ht_ie(sdata, skb, assoc_data->ap_ht_param,
				    sband, chan, sdata->smps_mode);

	/* if present, add any custom IEs that go before VHT */
	if (assoc_data->ie_len) {
		static const u8 before_vht[] = {
			WLAN_EID_SSID,
			WLAN_EID_SUPP_RATES,
			WLAN_EID_EXT_SUPP_RATES,
			WLAN_EID_PWR_CAPABILITY,
			WLAN_EID_SUPPORTED_CHANNELS,
			WLAN_EID_RSN,
			WLAN_EID_QOS_CAPA,
			WLAN_EID_RRM_ENABLED_CAPABILITIES,
			WLAN_EID_MOBILITY_DOMAIN,
			WLAN_EID_SUPPORTED_REGULATORY_CLASSES,
			WLAN_EID_HT_CAPABILITY,
			WLAN_EID_BSS_COEX_2040,
			WLAN_EID_EXT_CAPABILITY,
			WLAN_EID_QOS_TRAFFIC_CAPA,
			WLAN_EID_TIM_BCAST_REQ,
			WLAN_EID_INTERWORKING,
		};

		/* RIC already taken above, so no need to handle here anymore */
		noffset = ieee80211_ie_split(assoc_data->ie, assoc_data->ie_len,
					     before_vht, ARRAY_SIZE(before_vht),
					     offset);
		pos = skb_put(skb, noffset - offset);
		memcpy(pos, assoc_data->ie + offset, noffset - offset);
		offset = noffset;
	}

	if (!(ifmgd->flags & IEEE80211_STA_DISABLE_VHT))
		ieee80211_add_vht_ie(sdata, skb, sband,
				     &assoc_data->ap_vht_cap);

	/* if present, add any custom non-vendor IEs that go after HT */
	if (assoc_data->ie_len) {
		noffset = ieee80211_ie_split_vendor(assoc_data->ie,
						    assoc_data->ie_len,
						    offset);
		pos = skb_put(skb, noffset - offset);
		memcpy(pos, assoc_data->ie + offset, noffset - offset);
		offset = noffset;
	}

	if (assoc_data->wmm) {
		if (assoc_data->uapsd) {
			qos_info = ifmgd->uapsd_queues;
			qos_info |= (ifmgd->uapsd_max_sp_len <<
				     IEEE80211_WMM_IE_STA_QOSINFO_SP_SHIFT);
		} else {
			qos_info = 0;
		}

		pos = ieee80211_add_wmm_info_ie(skb_put(skb, 9), qos_info);
	}

	/* add any remaining custom (i.e. vendor specific here) IEs */
	if (assoc_data->ie_len) {
		noffset = assoc_data->ie_len;
		pos = skb_put(skb, noffset - offset);
		memcpy(pos, assoc_data->ie + offset, noffset - offset);
	}

	drv_mgd_prepare_tx(local, sdata);

	IEEE80211_SKB_CB(skb)->flags |= IEEE80211_TX_INTFL_DONT_ENCRYPT;
	if (local->hw.flags & IEEE80211_HW_REPORTS_TX_ACK_STATUS)
		IEEE80211_SKB_CB(skb)->flags |= IEEE80211_TX_CTL_REQ_TX_STATUS |
						IEEE80211_TX_INTFL_MLME_CONN_TX;
	ieee80211_tx_skb(sdata, skb);
}

void ieee80211_send_pspoll(struct ieee80211_local *local,
			   struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_pspoll *pspoll;
	struct sk_buff *skb;

	skb = ieee80211_pspoll_get(&local->hw, &sdata->vif);
	if (!skb)
		return;

	pspoll = (struct ieee80211_pspoll *) skb->data;
	pspoll->frame_control |= cpu_to_le16(IEEE80211_FCTL_PM);

	IEEE80211_SKB_CB(skb)->flags |= IEEE80211_TX_INTFL_DONT_ENCRYPT;
	ieee80211_tx_skb(sdata, skb);
}

void ieee80211_send_nullfunc(struct ieee80211_local *local,
			     struct ieee80211_sub_if_data *sdata,
			     int powersave)
{
	struct sk_buff *skb;
	struct ieee80211_hdr_3addr *nullfunc;
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;

	skb = ieee80211_nullfunc_get(&local->hw, &sdata->vif);
	if (!skb)
		return;

	nullfunc = (struct ieee80211_hdr_3addr *) skb->data;
	if (powersave)
		nullfunc->frame_control |= cpu_to_le16(IEEE80211_FCTL_PM);

	IEEE80211_SKB_CB(skb)->flags |= IEEE80211_TX_INTFL_DONT_ENCRYPT |
					IEEE80211_TX_INTFL_OFFCHAN_TX_OK;

	if (local->hw.flags & IEEE80211_HW_REPORTS_TX_ACK_STATUS)
		IEEE80211_SKB_CB(skb)->flags |= IEEE80211_TX_CTL_REQ_TX_STATUS;

	if (ifmgd->flags & IEEE80211_STA_CONNECTION_POLL)
		IEEE80211_SKB_CB(skb)->flags |= IEEE80211_TX_CTL_USE_MINRATE;

	ieee80211_tx_skb(sdata, skb);
}

static void ieee80211_send_4addr_nullfunc(struct ieee80211_local *local,
					  struct ieee80211_sub_if_data *sdata)
{
	struct sk_buff *skb;
	struct ieee80211_hdr *nullfunc;
	__le16 fc;

	if (WARN_ON(sdata->vif.type != NL80211_IFTYPE_STATION))
		return;

	skb = dev_alloc_skb(local->hw.extra_tx_headroom + 30);
	if (!skb)
		return;

	skb_reserve(skb, local->hw.extra_tx_headroom);

	nullfunc = (struct ieee80211_hdr *) skb_put(skb, 30);
	memset(nullfunc, 0, 30);
	fc = cpu_to_le16(IEEE80211_FTYPE_DATA | IEEE80211_STYPE_NULLFUNC |
			 IEEE80211_FCTL_FROMDS | IEEE80211_FCTL_TODS);
	nullfunc->frame_control = fc;
	memcpy(nullfunc->addr1, sdata->u.mgd.bssid, ETH_ALEN);
	memcpy(nullfunc->addr2, sdata->vif.addr, ETH_ALEN);
	memcpy(nullfunc->addr3, sdata->u.mgd.bssid, ETH_ALEN);
	memcpy(nullfunc->addr4, sdata->vif.addr, ETH_ALEN);

	IEEE80211_SKB_CB(skb)->flags |= IEEE80211_TX_INTFL_DONT_ENCRYPT;
	ieee80211_tx_skb(sdata, skb);
}

/* spectrum management related things */
static void ieee80211_chswitch_work(struct work_struct *work)
{
	struct ieee80211_sub_if_data *sdata =
		container_of(work, struct ieee80211_sub_if_data, u.mgd.chswitch_work);
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	int ret;

	if (!ieee80211_sdata_running(sdata))
		return;

	sdata_lock(sdata);
	mutex_lock(&local->mtx);
	mutex_lock(&local->chanctx_mtx);

	if (!ifmgd->associated)
		goto out;

	if (!sdata->vif.csa_active)
		goto out;

	/*
	 * using reservation isn't immediate as it may be deferred until later
	 * with multi-vif. once reservation is complete it will re-schedule the
	 * work with no reserved_chanctx so verify chandef to check if it
	 * completed successfully
	 */

	if (sdata->reserved_chanctx) {
		/*
		 * with multi-vif csa driver may call ieee80211_csa_finish()
		 * many times while waiting for other interfaces to use their
		 * reservations
		 */
		if (sdata->reserved_ready)
			goto out;

		ret = ieee80211_vif_use_reserved_context(sdata);
		if (ret) {
			sdata_info(sdata,
				   "failed to use reserved channel context, disconnecting (err=%d)\n",
				   ret);
			ieee80211_queue_work(&sdata->local->hw,
					     &ifmgd->csa_connection_drop_work);
			goto out;
		}

		goto out;
	}

	if (!cfg80211_chandef_identical(&sdata->vif.bss_conf.chandef,
					&sdata->csa_chandef)) {
		sdata_info(sdata,
			   "failed to finalize channel switch, disconnecting\n");
		ieee80211_queue_work(&sdata->local->hw,
				     &ifmgd->csa_connection_drop_work);
		goto out;
	}

	/* XXX: shouldn't really modify cfg80211-owned data! */
	ifmgd->associated->channel = sdata->csa_chandef.chan;

	ifmgd->csa_waiting_bcn = true;

	ieee80211_sta_reset_beacon_monitor(sdata);
	ieee80211_sta_reset_conn_monitor(sdata);

out:
	mutex_unlock(&local->chanctx_mtx);
	mutex_unlock(&local->mtx);
	sdata_unlock(sdata);
}

static void ieee80211_chswitch_post_beacon(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	int ret;

	sdata_assert_lock(sdata);

	WARN_ON(!sdata->vif.csa_active);

	if (sdata->csa_block_tx) {
		ieee80211_wake_vif_queues(local, sdata,
					  IEEE80211_QUEUE_STOP_REASON_CSA);
		sdata->csa_block_tx = false;
	}

	sdata->vif.csa_active = false;
	ifmgd->csa_waiting_bcn = false;

	ret = drv_post_channel_switch(sdata);
	if (ret) {
		sdata_info(sdata,
			   "driver post channel switch failed, disconnecting\n");
		ieee80211_queue_work(&local->hw,
				     &ifmgd->csa_connection_drop_work);
		return;
	}

	cfg80211_ch_switch_notify(sdata->dev, &sdata->reserved_chandef);
}

void ieee80211_chswitch_done(struct ieee80211_vif *vif, bool success)
{
	struct ieee80211_sub_if_data *sdata = vif_to_sdata(vif);
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;

	trace_api_chswitch_done(sdata, success);
	if (!success) {
		sdata_info(sdata,
			   "driver channel switch failed, disconnecting\n");
		ieee80211_queue_work(&sdata->local->hw,
				     &ifmgd->csa_connection_drop_work);
	} else {
		ieee80211_queue_work(&sdata->local->hw, &ifmgd->chswitch_work);
	}
}
EXPORT_SYMBOL(ieee80211_chswitch_done);

static void ieee80211_chswitch_timer(unsigned long data)
{
	struct ieee80211_sub_if_data *sdata =
		(struct ieee80211_sub_if_data *) data;

	ieee80211_queue_work(&sdata->local->hw, &sdata->u.mgd.chswitch_work);
}

static void
ieee80211_sta_process_chanswitch(struct ieee80211_sub_if_data *sdata,
				 u64 timestamp, u32 device_timestamp,
				 struct ieee802_11_elems *elems,
				 bool beacon)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	struct cfg80211_bss *cbss = ifmgd->associated;
	struct ieee80211_chanctx_conf *conf;
	struct ieee80211_chanctx *chanctx;
	enum ieee80211_band current_band;
	struct ieee80211_csa_ie csa_ie;
	struct ieee80211_channel_switch ch_switch;
	int res;

	sdata_assert_lock(sdata);

	if (!cbss)
		return;

	if (local->scanning)
		return;

	/* disregard subsequent announcements if we are already processing */
	if (sdata->vif.csa_active)
		return;

	current_band = cbss->channel->band;
	memset(&csa_ie, 0, sizeof(csa_ie));
	res = ieee80211_parse_ch_switch_ie(sdata, elems, current_band,
					   ifmgd->flags,
					   ifmgd->associated->bssid, &csa_ie);
	if (res	< 0)
		ieee80211_queue_work(&local->hw,
				     &ifmgd->csa_connection_drop_work);
	if (res)
		return;

	if (!cfg80211_chandef_usable(local->hw.wiphy, &csa_ie.chandef,
				     IEEE80211_CHAN_DISABLED)) {
		sdata_info(sdata,
			   "AP %pM switches to unsupported channel (%d MHz, width:%d, CF1/2: %d/%d MHz), disconnecting\n",
			   ifmgd->associated->bssid,
			   csa_ie.chandef.chan->center_freq,
			   csa_ie.chandef.width, csa_ie.chandef.center_freq1,
			   csa_ie.chandef.center_freq2);
		ieee80211_queue_work(&local->hw,
				     &ifmgd->csa_connection_drop_work);
		return;
	}

	if (cfg80211_chandef_identical(&csa_ie.chandef,
				       &sdata->vif.bss_conf.chandef)) {
		if (ifmgd->csa_ignored_same_chan)
			return;
		sdata_info(sdata,
			   "AP %pM tries to chanswitch to same channel, ignore\n",
			   ifmgd->associated->bssid);
		ifmgd->csa_ignored_same_chan = true;
		return;
	}

	mutex_lock(&local->mtx);
	mutex_lock(&local->chanctx_mtx);
	conf = rcu_dereference_protected(sdata->vif.chanctx_conf,
					 lockdep_is_held(&local->chanctx_mtx));
	if (!conf) {
		sdata_info(sdata,
			   "no channel context assigned to vif?, disconnecting\n");
		goto drop_connection;
	}

	chanctx = container_of(conf, struct ieee80211_chanctx, conf);

	if (local->use_chanctx &&
	    !(local->hw.flags & IEEE80211_HW_CHANCTX_STA_CSA)) {
		sdata_info(sdata,
			   "driver doesn't support chan-switch with channel contexts\n");
		goto drop_connection;
	}

	ch_switch.timestamp = timestamp;
	ch_switch.device_timestamp = device_timestamp;
	ch_switch.block_tx = csa_ie.mode;
	ch_switch.chandef = csa_ie.chandef;
	ch_switch.count = csa_ie.count;

	if (drv_pre_channel_switch(sdata, &ch_switch)) {
		sdata_info(sdata,
			   "preparing for channel switch failed, disconnecting\n");
		goto drop_connection;
	}

	res = ieee80211_vif_reserve_chanctx(sdata, &csa_ie.chandef,
					    chanctx->mode, false);
	if (res) {
		sdata_info(sdata,
			   "failed to reserve channel context for channel switch, disconnecting (err=%d)\n",
			   res);
		goto drop_connection;
	}
	mutex_unlock(&local->chanctx_mtx);

	sdata->vif.csa_active = true;
	sdata->csa_chandef = csa_ie.chandef;
	sdata->csa_block_tx = csa_ie.mode;
	ifmgd->csa_ignored_same_chan = false;

	if (sdata->csa_block_tx)
		ieee80211_stop_vif_queues(local, sdata,
					  IEEE80211_QUEUE_STOP_REASON_CSA);
	mutex_unlock(&local->mtx);

	cfg80211_ch_switch_started_notify(sdata->dev, &csa_ie.chandef,
					  csa_ie.count);

	if (local->ops->channel_switch) {
		/* use driver's channel switch callback */
		drv_channel_switch(local, sdata, &ch_switch);
		return;
	}

	/* channel switch handled in software */
	if (csa_ie.count <= 1)
		ieee80211_queue_work(&local->hw, &ifmgd->chswitch_work);
	else
		mod_timer(&ifmgd->chswitch_timer,
			  TU_TO_EXP_TIME((csa_ie.count - 1) *
					 cbss->beacon_interval));
	return;
 drop_connection:
	ieee80211_queue_work(&local->hw, &ifmgd->csa_connection_drop_work);
	mutex_unlock(&local->chanctx_mtx);
	mutex_unlock(&local->mtx);
}

static bool
ieee80211_find_80211h_pwr_constr(struct ieee80211_sub_if_data *sdata,
				 struct ieee80211_channel *channel,
				 const u8 *country_ie, u8 country_ie_len,
				 const u8 *pwr_constr_elem,
				 int *chan_pwr, int *pwr_reduction)
{
	struct ieee80211_country_ie_triplet *triplet;
	int chan = ieee80211_frequency_to_channel(channel->center_freq);
	int i, chan_increment;
	bool have_chan_pwr = false;

	/* Invalid IE */
	if (country_ie_len % 2 || country_ie_len < IEEE80211_COUNTRY_IE_MIN_LEN)
		return false;

	triplet = (void *)(country_ie + 3);
	country_ie_len -= 3;

	switch (channel->band) {
	default:
		WARN_ON_ONCE(1);
		/* fall through */
	case IEEE80211_BAND_2GHZ:
	case IEEE80211_BAND_60GHZ:
		chan_increment = 1;
		break;
	case IEEE80211_BAND_5GHZ:
		chan_increment = 4;
		break;
	}

	/* find channel */
	while (country_ie_len >= 3) {
		u8 first_channel = triplet->chans.first_channel;

		if (first_channel >= IEEE80211_COUNTRY_EXTENSION_ID)
			goto next;

		for (i = 0; i < triplet->chans.num_channels; i++) {
			if (first_channel + i * chan_increment == chan) {
				have_chan_pwr = true;
				*chan_pwr = triplet->chans.max_power;
				break;
			}
		}
		if (have_chan_pwr)
			break;

 next:
		triplet++;
		country_ie_len -= 3;
	}

	if (have_chan_pwr && pwr_constr_elem)
		*pwr_reduction = *pwr_constr_elem;
	else
		*pwr_reduction = 0;

	return have_chan_pwr;
}

static void ieee80211_find_cisco_dtpc(struct ieee80211_sub_if_data *sdata,
				      struct ieee80211_channel *channel,
				      const u8 *cisco_dtpc_ie,
				      int *pwr_level)
{
	/* From practical testing, the first data byte of the DTPC element
	 * seems to contain the requested dBm level, and the CLI on Cisco
	 * APs clearly state the range is -127 to 127 dBm, which indicates
	 * a signed byte, although it seemingly never actually goes negative.
	 * The other byte seems to always be zero.
	 */
	*pwr_level = (__s8)cisco_dtpc_ie[4];
}

static u32 ieee80211_handle_pwr_constr(struct ieee80211_sub_if_data *sdata,
				       struct ieee80211_channel *channel,
				       struct ieee80211_mgmt *mgmt,
				       const u8 *country_ie, u8 country_ie_len,
				       const u8 *pwr_constr_ie,
				       const u8 *cisco_dtpc_ie)
{
	bool has_80211h_pwr = false, has_cisco_pwr = false;
	int chan_pwr = 0, pwr_reduction_80211h = 0;
	int pwr_level_cisco, pwr_level_80211h;
	int new_ap_level;
	__le16 capab = mgmt->u.probe_resp.capab_info;

	if (country_ie &&
	    (capab & cpu_to_le16(WLAN_CAPABILITY_SPECTRUM_MGMT) ||
	     capab & cpu_to_le16(WLAN_CAPABILITY_RADIO_MEASURE))) {
		has_80211h_pwr = ieee80211_find_80211h_pwr_constr(
			sdata, channel, country_ie, country_ie_len,
			pwr_constr_ie, &chan_pwr, &pwr_reduction_80211h);
		pwr_level_80211h =
			max_t(int, 0, chan_pwr - pwr_reduction_80211h);
	}

	if (cisco_dtpc_ie) {
		ieee80211_find_cisco_dtpc(
			sdata, channel, cisco_dtpc_ie, &pwr_level_cisco);
		has_cisco_pwr = true;
	}

	if (!has_80211h_pwr && !has_cisco_pwr)
		return 0;

	/* If we have both 802.11h and Cisco DTPC, apply both limits
	 * by picking the smallest of the two power levels advertised.
	 */
	if (has_80211h_pwr &&
	    (!has_cisco_pwr || pwr_level_80211h <= pwr_level_cisco)) {
		sdata_dbg(sdata,
			  "Limiting TX power to %d (%d - %d) dBm as advertised by %pM\n",
			  pwr_level_80211h, chan_pwr, pwr_reduction_80211h,
			  sdata->u.mgd.bssid);
		new_ap_level = pwr_level_80211h;
	} else {  /* has_cisco_pwr is always true here. */
		sdata_dbg(sdata,
			  "Limiting TX power to %d dBm as advertised by %pM\n",
			  pwr_level_cisco, sdata->u.mgd.bssid);
		new_ap_level = pwr_level_cisco;
	}

	if (sdata->ap_power_level == new_ap_level)
		return 0;

	sdata->ap_power_level = new_ap_level;
	if (__ieee80211_recalc_txpower(sdata))
		return BSS_CHANGED_TXPOWER;
	return 0;
}

/* powersave */
static void ieee80211_enable_ps(struct ieee80211_local *local,
				struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_conf *conf = &local->hw.conf;

	/*
	 * If we are scanning right now then the parameters will
	 * take effect when scan finishes.
	 */
	if (local->scanning)
		return;

	if (conf->dynamic_ps_timeout > 0 &&
	    !(local->hw.flags & IEEE80211_HW_SUPPORTS_DYNAMIC_PS)) {
		mod_timer(&local->dynamic_ps_timer, jiffies +
			  msecs_to_jiffies(conf->dynamic_ps_timeout));
	} else {
		if (local->hw.flags & IEEE80211_HW_PS_NULLFUNC_STACK)
			ieee80211_send_nullfunc(local, sdata, 1);

		if ((local->hw.flags & IEEE80211_HW_PS_NULLFUNC_STACK) &&
		    (local->hw.flags & IEEE80211_HW_REPORTS_TX_ACK_STATUS))
			return;

		conf->flags |= IEEE80211_CONF_PS;
		ieee80211_hw_config(local, IEEE80211_CONF_CHANGE_PS);
	}
}

static void ieee80211_change_ps(struct ieee80211_local *local)
{
	struct ieee80211_conf *conf = &local->hw.conf;

	if (local->ps_sdata) {
		ieee80211_enable_ps(local, local->ps_sdata);
	} else if (conf->flags & IEEE80211_CONF_PS) {
		conf->flags &= ~IEEE80211_CONF_PS;
		ieee80211_hw_config(local, IEEE80211_CONF_CHANGE_PS);
		del_timer_sync(&local->dynamic_ps_timer);
		cancel_work_sync(&local->dynamic_ps_enable_work);
	}
}

static bool ieee80211_powersave_allowed(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_managed *mgd = &sdata->u.mgd;
	struct sta_info *sta = NULL;
	bool authorized = false;

	if (!mgd->powersave)
		return false;

	if (mgd->broken_ap)
		return false;

	if (!mgd->associated)
		return false;

	if (mgd->flags & IEEE80211_STA_CONNECTION_POLL)
		return false;

	if (!mgd->have_beacon)
		return false;

	rcu_read_lock();
	sta = sta_info_get(sdata, mgd->bssid);
	if (sta)
		authorized = test_sta_flag(sta, WLAN_STA_AUTHORIZED);
	rcu_read_unlock();

	return authorized;
}

/* need to hold RTNL or interface lock */
void ieee80211_recalc_ps(struct ieee80211_local *local, s32 latency)
{
	struct ieee80211_sub_if_data *sdata, *found = NULL;
	int count = 0;
	int timeout;

	if (!(local->hw.flags & IEEE80211_HW_SUPPORTS_PS)) {
		local->ps_sdata = NULL;
		return;
	}

	list_for_each_entry(sdata, &local->interfaces, list) {
		if (!ieee80211_sdata_running(sdata))
			continue;
		if (sdata->vif.type == NL80211_IFTYPE_AP) {
			/* If an AP vif is found, then disable PS
			 * by setting the count to zero thereby setting
			 * ps_sdata to NULL.
			 */
			count = 0;
			break;
		}
		if (sdata->vif.type != NL80211_IFTYPE_STATION)
			continue;
		found = sdata;
		count++;
	}

	if (count == 1 && ieee80211_powersave_allowed(found)) {
		s32 beaconint_us;

		if (latency < 0)
			latency = pm_qos_request(PM_QOS_NETWORK_LATENCY);

		beaconint_us = ieee80211_tu_to_usec(
					found->vif.bss_conf.beacon_int);

		timeout = local->dynamic_ps_forced_timeout;
		if (timeout < 0) {
			/*
			 * Go to full PSM if the user configures a very low
			 * latency requirement.
			 * The 2000 second value is there for compatibility
			 * until the PM_QOS_NETWORK_LATENCY is configured
			 * with real values.
			 */
			if (latency > (1900 * USEC_PER_MSEC) &&
			    latency != (2000 * USEC_PER_SEC))
				timeout = 0;
			else
				timeout = 100;
		}
		local->hw.conf.dynamic_ps_timeout = timeout;

		if (beaconint_us > latency) {
			local->ps_sdata = NULL;
		} else {
			int maxslp = 1;
			u8 dtimper = found->u.mgd.dtim_period;

			/* If the TIM IE is invalid, pretend the value is 1 */
			if (!dtimper)
				dtimper = 1;
			else if (dtimper > 1)
				maxslp = min_t(int, dtimper,
						    latency / beaconint_us);

			local->hw.conf.max_sleep_period = maxslp;
			local->hw.conf.ps_dtim_period = dtimper;
			local->ps_sdata = found;
		}
	} else {
		local->ps_sdata = NULL;
	}

	ieee80211_change_ps(local);
}

void ieee80211_recalc_ps_vif(struct ieee80211_sub_if_data *sdata)
{
	bool ps_allowed = ieee80211_powersave_allowed(sdata);

	if (sdata->vif.bss_conf.ps != ps_allowed) {
		sdata->vif.bss_conf.ps = ps_allowed;
		ieee80211_bss_info_change_notify(sdata, BSS_CHANGED_PS);
	}
}

void ieee80211_dynamic_ps_disable_work(struct work_struct *work)
{
	struct ieee80211_local *local =
		container_of(work, struct ieee80211_local,
			     dynamic_ps_disable_work);

	if (local->hw.conf.flags & IEEE80211_CONF_PS) {
		local->hw.conf.flags &= ~IEEE80211_CONF_PS;
		ieee80211_hw_config(local, IEEE80211_CONF_CHANGE_PS);
	}

	ieee80211_wake_queues_by_reason(&local->hw,
					IEEE80211_MAX_QUEUE_MAP,
					IEEE80211_QUEUE_STOP_REASON_PS,
					false);
}

void ieee80211_dynamic_ps_enable_work(struct work_struct *work)
{
	struct ieee80211_local *local =
		container_of(work, struct ieee80211_local,
			     dynamic_ps_enable_work);
	struct ieee80211_sub_if_data *sdata = local->ps_sdata;
	struct ieee80211_if_managed *ifmgd;
	unsigned long flags;
	int q;

	/* can only happen when PS was just disabled anyway */
	if (!sdata)
		return;

	ifmgd = &sdata->u.mgd;

	if (local->hw.conf.flags & IEEE80211_CONF_PS)
		return;

	if (local->hw.conf.dynamic_ps_timeout > 0) {
		/* don't enter PS if TX frames are pending */
		if (drv_tx_frames_pending(local)) {
			mod_timer(&local->dynamic_ps_timer, jiffies +
				  msecs_to_jiffies(
				  local->hw.conf.dynamic_ps_timeout));
			return;
		}

		/*
		 * transmission can be stopped by others which leads to
		 * dynamic_ps_timer expiry. Postpone the ps timer if it
		 * is not the actual idle state.
		 */
		spin_lock_irqsave(&local->queue_stop_reason_lock, flags);
		for (q = 0; q < local->hw.queues; q++) {
			if (local->queue_stop_reasons[q]) {
				spin_unlock_irqrestore(&local->queue_stop_reason_lock,
						       flags);
				mod_timer(&local->dynamic_ps_timer, jiffies +
					  msecs_to_jiffies(
					  local->hw.conf.dynamic_ps_timeout));
				return;
			}
		}
		spin_unlock_irqrestore(&local->queue_stop_reason_lock, flags);
	}

	if ((local->hw.flags & IEEE80211_HW_PS_NULLFUNC_STACK) &&
	    !(ifmgd->flags & IEEE80211_STA_NULLFUNC_ACKED)) {
		if (drv_tx_frames_pending(local)) {
			mod_timer(&local->dynamic_ps_timer, jiffies +
				  msecs_to_jiffies(
				  local->hw.conf.dynamic_ps_timeout));
		} else {
			ieee80211_send_nullfunc(local, sdata, 1);
			/* Flush to get the tx status of nullfunc frame */
			ieee80211_flush_queues(local, sdata, false);
		}
	}

	if (!((local->hw.flags & IEEE80211_HW_REPORTS_TX_ACK_STATUS) &&
	      (local->hw.flags & IEEE80211_HW_PS_NULLFUNC_STACK)) ||
	    (ifmgd->flags & IEEE80211_STA_NULLFUNC_ACKED)) {
		ifmgd->flags &= ~IEEE80211_STA_NULLFUNC_ACKED;
		local->hw.conf.flags |= IEEE80211_CONF_PS;
		ieee80211_hw_config(local, IEEE80211_CONF_CHANGE_PS);
	}
}

void ieee80211_dynamic_ps_timer(unsigned long data)
{
	struct ieee80211_local *local = (void *) data;

	ieee80211_queue_work(&local->hw, &local->dynamic_ps_enable_work);
}

void ieee80211_dfs_cac_timer_work(struct work_struct *work)
{
	struct delayed_work *delayed_work =
		container_of(work, struct delayed_work, work);
	struct ieee80211_sub_if_data *sdata =
		container_of(delayed_work, struct ieee80211_sub_if_data,
			     dfs_cac_timer_work);
	struct cfg80211_chan_def chandef = sdata->vif.bss_conf.chandef;

	mutex_lock(&sdata->local->mtx);
	if (sdata->wdev.cac_started) {
		ieee80211_vif_release_channel(sdata);
		cfg80211_cac_event(sdata->dev, &chandef,
				   NL80211_RADAR_CAC_FINISHED,
				   GFP_KERNEL);
	}
	mutex_unlock(&sdata->local->mtx);
}

static bool
__ieee80211_sta_handle_tspec_ac_params(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	bool ret = false;
	int ac;

	if (local->hw.queues < IEEE80211_NUM_ACS)
		return false;

	for (ac = 0; ac < IEEE80211_NUM_ACS; ac++) {
		struct ieee80211_sta_tx_tspec *tx_tspec = &ifmgd->tx_tspec[ac];
		int non_acm_ac;
		unsigned long now = jiffies;

		if (tx_tspec->action == TX_TSPEC_ACTION_NONE &&
		    tx_tspec->admitted_time &&
		    time_after(now, tx_tspec->time_slice_start + HZ)) {
			tx_tspec->consumed_tx_time = 0;
			tx_tspec->time_slice_start = now;

			if (tx_tspec->downgraded)
				tx_tspec->action =
					TX_TSPEC_ACTION_STOP_DOWNGRADE;
		}

		switch (tx_tspec->action) {
		case TX_TSPEC_ACTION_STOP_DOWNGRADE:
			/* take the original parameters */
			if (drv_conf_tx(local, sdata, ac, &sdata->tx_conf[ac]))
				sdata_err(sdata,
					  "failed to set TX queue parameters for queue %d\n",
					  ac);
			tx_tspec->action = TX_TSPEC_ACTION_NONE;
			tx_tspec->downgraded = false;
			ret = true;
			break;
		case TX_TSPEC_ACTION_DOWNGRADE:
			if (time_after(now, tx_tspec->time_slice_start + HZ)) {
				tx_tspec->action = TX_TSPEC_ACTION_NONE;
				ret = true;
				break;
			}
			/* downgrade next lower non-ACM AC */
			for (non_acm_ac = ac + 1;
			     non_acm_ac < IEEE80211_NUM_ACS;
			     non_acm_ac++)
				if (!(sdata->wmm_acm & BIT(7 - 2 * non_acm_ac)))
					break;
			/* The loop will result in using BK even if it requires
			 * admission control, such configuration makes no sense
			 * and we have to transmit somehow - the AC selection
			 * does the same thing.
			 */
			if (drv_conf_tx(local, sdata, ac,
					&sdata->tx_conf[non_acm_ac]))
				sdata_err(sdata,
					  "failed to set TX queue parameters for queue %d\n",
					  ac);
			tx_tspec->action = TX_TSPEC_ACTION_NONE;
			ret = true;
			schedule_delayed_work(&ifmgd->tx_tspec_wk,
				tx_tspec->time_slice_start + HZ - now + 1);
			break;
		case TX_TSPEC_ACTION_NONE:
			/* nothing now */
			break;
		}
	}

	return ret;
}

void ieee80211_sta_handle_tspec_ac_params(struct ieee80211_sub_if_data *sdata)
{
	if (__ieee80211_sta_handle_tspec_ac_params(sdata))
		ieee80211_bss_info_change_notify(sdata, BSS_CHANGED_QOS);
}

static void ieee80211_sta_handle_tspec_ac_params_wk(struct work_struct *work)
{
	struct ieee80211_sub_if_data *sdata;

	sdata = container_of(work, struct ieee80211_sub_if_data,
			     u.mgd.tx_tspec_wk.work);
	ieee80211_sta_handle_tspec_ac_params(sdata);
}

/* MLME */
static bool ieee80211_sta_wmm_params(struct ieee80211_local *local,
				     struct ieee80211_sub_if_data *sdata,
				     const u8 *wmm_param, size_t wmm_param_len)
{
	struct ieee80211_tx_queue_params params;
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	size_t left;
	int count;
	const u8 *pos;
	u8 uapsd_queues = 0;

	if (!local->ops->conf_tx)
		return false;

	if (local->hw.queues < IEEE80211_NUM_ACS)
		return false;

	if (!wmm_param)
		return false;

	if (wmm_param_len < 8 || wmm_param[5] /* version */ != 1)
		return false;

	if (ifmgd->flags & IEEE80211_STA_UAPSD_ENABLED)
		uapsd_queues = ifmgd->uapsd_queues;

	count = wmm_param[6] & 0x0f;
	if (count == ifmgd->wmm_last_param_set)
		return false;
	ifmgd->wmm_last_param_set = count;

	pos = wmm_param + 8;
	left = wmm_param_len - 8;

	memset(&params, 0, sizeof(params));

	sdata->wmm_acm = 0;
	for (; left >= 4; left -= 4, pos += 4) {
		int aci = (pos[0] >> 5) & 0x03;
		int acm = (pos[0] >> 4) & 0x01;
		bool uapsd = false;
		int queue;

		switch (aci) {
		case 1: /* AC_BK */
			queue = 3;
			if (acm)
				sdata->wmm_acm |= BIT(1) | BIT(2); /* BK/- */
			if (uapsd_queues & IEEE80211_WMM_IE_STA_QOSINFO_AC_BK)
				uapsd = true;
			break;
		case 2: /* AC_VI */
			queue = 1;
			if (acm)
				sdata->wmm_acm |= BIT(4) | BIT(5); /* CL/VI */
			if (uapsd_queues & IEEE80211_WMM_IE_STA_QOSINFO_AC_VI)
				uapsd = true;
			break;
		case 3: /* AC_VO */
			queue = 0;
			if (acm)
				sdata->wmm_acm |= BIT(6) | BIT(7); /* VO/NC */
			if (uapsd_queues & IEEE80211_WMM_IE_STA_QOSINFO_AC_VO)
				uapsd = true;
			break;
		case 0: /* AC_BE */
		default:
			queue = 2;
			if (acm)
				sdata->wmm_acm |= BIT(0) | BIT(3); /* BE/EE */
			if (uapsd_queues & IEEE80211_WMM_IE_STA_QOSINFO_AC_BE)
				uapsd = true;
			break;
		}

		params.aifs = pos[0] & 0x0f;
		params.cw_max = ecw2cw((pos[1] & 0xf0) >> 4);
		params.cw_min = ecw2cw(pos[1] & 0x0f);
		params.txop = get_unaligned_le16(pos + 2);
		params.acm = acm;
		params.uapsd = uapsd;

		mlme_dbg(sdata,
			 "WMM queue=%d aci=%d acm=%d aifs=%d cWmin=%d cWmax=%d txop=%d uapsd=%d, downgraded=%d\n",
			 queue, aci, acm,
			 params.aifs, params.cw_min, params.cw_max,
			 params.txop, params.uapsd,
			 ifmgd->tx_tspec[queue].downgraded);
		sdata->tx_conf[queue] = params;
		if (!ifmgd->tx_tspec[queue].downgraded &&
		    drv_conf_tx(local, sdata, queue, &params))
			sdata_err(sdata,
				  "failed to set TX queue parameters for queue %d\n",
				  queue);
	}

	/* enable WMM or activate new settings */
	sdata->vif.bss_conf.qos = true;
	return true;
}

static void __ieee80211_stop_poll(struct ieee80211_sub_if_data *sdata)
{
	lockdep_assert_held(&sdata->local->mtx);

	sdata->u.mgd.flags &= ~IEEE80211_STA_CONNECTION_POLL;
	ieee80211_run_deferred_scan(sdata->local);
}

static void ieee80211_stop_poll(struct ieee80211_sub_if_data *sdata)
{
	mutex_lock(&sdata->local->mtx);
	__ieee80211_stop_poll(sdata);
	mutex_unlock(&sdata->local->mtx);
}

static u32 ieee80211_handle_bss_capability(struct ieee80211_sub_if_data *sdata,
					   u16 capab, bool erp_valid, u8 erp)
{
	struct ieee80211_bss_conf *bss_conf = &sdata->vif.bss_conf;
	u32 changed = 0;
	bool use_protection;
	bool use_short_preamble;
	bool use_short_slot;

	if (erp_valid) {
		use_protection = (erp & WLAN_ERP_USE_PROTECTION) != 0;
		use_short_preamble = (erp & WLAN_ERP_BARKER_PREAMBLE) == 0;
	} else {
		use_protection = false;
		use_short_preamble = !!(capab & WLAN_CAPABILITY_SHORT_PREAMBLE);
	}

	use_short_slot = !!(capab & WLAN_CAPABILITY_SHORT_SLOT_TIME);
	if (ieee80211_get_sdata_band(sdata) == IEEE80211_BAND_5GHZ)
		use_short_slot = true;

	if (use_protection != bss_conf->use_cts_prot) {
		bss_conf->use_cts_prot = use_protection;
		changed |= BSS_CHANGED_ERP_CTS_PROT;
	}

	if (use_short_preamble != bss_conf->use_short_preamble) {
		bss_conf->use_short_preamble = use_short_preamble;
		changed |= BSS_CHANGED_ERP_PREAMBLE;
	}

	if (use_short_slot != bss_conf->use_short_slot) {
		bss_conf->use_short_slot = use_short_slot;
		changed |= BSS_CHANGED_ERP_SLOT;
	}

	return changed;
}

static void ieee80211_set_associated(struct ieee80211_sub_if_data *sdata,
				     struct cfg80211_bss *cbss,
				     u32 bss_info_changed)
{
	struct ieee80211_bss *bss = (void *)cbss->priv;
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_bss_conf *bss_conf = &sdata->vif.bss_conf;

	bss_info_changed |= BSS_CHANGED_ASSOC;
	bss_info_changed |= ieee80211_handle_bss_capability(sdata,
		bss_conf->assoc_capability, bss->has_erp_value, bss->erp_value);

	sdata->u.mgd.beacon_timeout = usecs_to_jiffies(ieee80211_tu_to_usec(
		beacon_loss_count * bss_conf->beacon_int));

	sdata->u.mgd.associated = cbss;
	memcpy(sdata->u.mgd.bssid, cbss->bssid, ETH_ALEN);

	sdata->u.mgd.flags |= IEEE80211_STA_RESET_SIGNAL_AVE;

	if (sdata->vif.p2p) {
		const struct cfg80211_bss_ies *ies;

		rcu_read_lock();
		ies = rcu_dereference(cbss->ies);
		if (ies) {
			int ret;

			ret = cfg80211_get_p2p_attr(
					ies->data, ies->len,
					IEEE80211_P2P_ATTR_ABSENCE_NOTICE,
					(u8 *) &bss_conf->p2p_noa_attr,
					sizeof(bss_conf->p2p_noa_attr));
			if (ret >= 2) {
				sdata->u.mgd.p2p_noa_index =
					bss_conf->p2p_noa_attr.index;
				bss_info_changed |= BSS_CHANGED_P2P_PS;
			}
		}
		rcu_read_unlock();
	}

	/* just to be sure */
	ieee80211_stop_poll(sdata);

	ieee80211_led_assoc(local, 1);

	if (sdata->u.mgd.have_beacon) {
		/*
		 * If the AP is buggy we may get here with no DTIM period
		 * known, so assume it's 1 which is the only safe assumption
		 * in that case, although if the TIM IE is broken powersave
		 * probably just won't work at all.
		 */
		bss_conf->dtim_period = sdata->u.mgd.dtim_period ?: 1;
		bss_conf->beacon_rate = bss->beacon_rate;
		bss_info_changed |= BSS_CHANGED_BEACON_INFO;
	} else {
		bss_conf->beacon_rate = NULL;
		bss_conf->dtim_period = 0;
	}

	bss_conf->assoc = 1;

	/* Tell the driver to monitor connection quality (if supported) */
	if (sdata->vif.driver_flags & IEEE80211_VIF_SUPPORTS_CQM_RSSI &&
	    bss_conf->cqm_rssi_thold)
		bss_info_changed |= BSS_CHANGED_CQM;

	/* Enable ARP filtering */
	if (bss_conf->arp_addr_cnt)
		bss_info_changed |= BSS_CHANGED_ARP_FILTER;

	ieee80211_bss_info_change_notify(sdata, bss_info_changed);

	mutex_lock(&local->iflist_mtx);
	ieee80211_recalc_ps(local, -1);
	mutex_unlock(&local->iflist_mtx);

	ieee80211_recalc_smps(sdata);
	ieee80211_recalc_ps_vif(sdata);

	netif_carrier_on(sdata->dev);
}

static void ieee80211_set_disassoc(struct ieee80211_sub_if_data *sdata,
				   u16 stype, u16 reason, bool tx,
				   u8 *frame_buf)
{
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	struct ieee80211_local *local = sdata->local;
	u32 changed = 0;

	sdata_assert_lock(sdata);

	if (WARN_ON_ONCE(tx && !frame_buf))
		return;

	if (WARN_ON(!ifmgd->associated))
		return;

	ieee80211_stop_poll(sdata);

	ifmgd->associated = NULL;
	netif_carrier_off(sdata->dev);

	/*
	 * if we want to get out of ps before disassoc (why?) we have
	 * to do it before sending disassoc, as otherwise the null-packet
	 * won't be valid.
	 */
	if (local->hw.conf.flags & IEEE80211_CONF_PS) {
		local->hw.conf.flags &= ~IEEE80211_CONF_PS;
		ieee80211_hw_config(local, IEEE80211_CONF_CHANGE_PS);
	}
	local->ps_sdata = NULL;

	/* disable per-vif ps */
	ieee80211_recalc_ps_vif(sdata);

	/* make sure ongoing transmission finishes */
	synchronize_net();

	/*
	 * drop any frame before deauth/disassoc, this can be data or
	 * management frame. Since we are disconnecting, we should not
	 * insist sending these frames which can take time and delay
	 * the disconnection and possible the roaming.
	 */
	if (tx)
		ieee80211_flush_queues(local, sdata, true);

	/* deauthenticate/disassociate now */
	if (tx || frame_buf)
		ieee80211_send_deauth_disassoc(sdata, ifmgd->bssid, stype,
					       reason, tx, frame_buf);

	/* flush out frame - make sure the deauth was actually sent */
	if (tx)
		ieee80211_flush_queues(local, sdata, false);

	/* clear bssid only after building the needed mgmt frames */
	eth_zero_addr(ifmgd->bssid);

	/* remove AP and TDLS peers */
	sta_info_flush(sdata);

	/* finally reset all BSS / config parameters */
	changed |= ieee80211_reset_erp_info(sdata);

	ieee80211_led_assoc(local, 0);
	changed |= BSS_CHANGED_ASSOC;
	sdata->vif.bss_conf.assoc = false;

	ifmgd->p2p_noa_index = -1;
	memset(&sdata->vif.bss_conf.p2p_noa_attr, 0,
	       sizeof(sdata->vif.bss_conf.p2p_noa_attr));

	/* on the next assoc, re-program HT/VHT parameters */
	memset(&ifmgd->ht_capa, 0, sizeof(ifmgd->ht_capa));
	memset(&ifmgd->ht_capa_mask, 0, sizeof(ifmgd->ht_capa_mask));
	memset(&ifmgd->vht_capa, 0, sizeof(ifmgd->vht_capa));
	memset(&ifmgd->vht_capa_mask, 0, sizeof(ifmgd->vht_capa_mask));

	sdata->ap_power_level = IEEE80211_UNSET_POWER_LEVEL;

	del_timer_sync(&local->dynamic_ps_timer);
	cancel_work_sync(&local->dynamic_ps_enable_work);

	/* Disable ARP filtering */
	if (sdata->vif.bss_conf.arp_addr_cnt)
		changed |= BSS_CHANGED_ARP_FILTER;

	sdata->vif.bss_conf.qos = false;
	changed |= BSS_CHANGED_QOS;

	/* The BSSID (not really interesting) and HT changed */
	changed |= BSS_CHANGED_BSSID | BSS_CHANGED_HT;
	ieee80211_bss_info_change_notify(sdata, changed);

	/* disassociated - set to defaults now */
	ieee80211_set_wmm_default(sdata, false);

	del_timer_sync(&sdata->u.mgd.conn_mon_timer);
	del_timer_sync(&sdata->u.mgd.bcn_mon_timer);
	del_timer_sync(&sdata->u.mgd.timer);
	del_timer_sync(&sdata->u.mgd.chswitch_timer);

	sdata->vif.bss_conf.dtim_period = 0;
	sdata->vif.bss_conf.beacon_rate = NULL;

	ifmgd->have_beacon = false;

	ifmgd->flags = 0;
	mutex_lock(&local->mtx);
	ieee80211_vif_release_channel(sdata);

	sdata->vif.csa_active = false;
	ifmgd->csa_waiting_bcn = false;
	ifmgd->csa_ignored_same_chan = false;
	if (sdata->csa_block_tx) {
		ieee80211_wake_vif_queues(local, sdata,
					  IEEE80211_QUEUE_STOP_REASON_CSA);
		sdata->csa_block_tx = false;
	}
	mutex_unlock(&local->mtx);

	/* existing TX TSPEC sessions no longer exist */
	memset(ifmgd->tx_tspec, 0, sizeof(ifmgd->tx_tspec));
	cancel_delayed_work_sync(&ifmgd->tx_tspec_wk);

	sdata->encrypt_headroom = IEEE80211_ENCRYPT_HEADROOM;
}

void ieee80211_sta_rx_notify(struct ieee80211_sub_if_data *sdata,
			     struct ieee80211_hdr *hdr)
{
	/*
	 * We can postpone the mgd.timer whenever receiving unicast frames
	 * from AP because we know that the connection is working both ways
	 * at that time. But multicast frames (and hence also beacons) must
	 * be ignored here, because we need to trigger the timer during
	 * data idle periods for sending the periodic probe request to the
	 * AP we're connected to.
	 */
	if (is_multicast_ether_addr(hdr->addr1))
		return;

	ieee80211_sta_reset_conn_monitor(sdata);
}

static void ieee80211_reset_ap_probe(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	struct ieee80211_local *local = sdata->local;

	mutex_lock(&local->mtx);
	if (!(ifmgd->flags & IEEE80211_STA_CONNECTION_POLL))
		goto out;

	__ieee80211_stop_poll(sdata);

	mutex_lock(&local->iflist_mtx);
	ieee80211_recalc_ps(local, -1);
	mutex_unlock(&local->iflist_mtx);

	if (sdata->local->hw.flags & IEEE80211_HW_CONNECTION_MONITOR)
		goto out;

	/*
	 * We've received a probe response, but are not sure whether
	 * we have or will be receiving any beacons or data, so let's
	 * schedule the timers again, just in case.
	 */
	ieee80211_sta_reset_beacon_monitor(sdata);

	mod_timer(&ifmgd->conn_mon_timer,
		  round_jiffies_up(jiffies +
				   IEEE80211_CONNECTION_IDLE_TIME));
out:
	mutex_unlock(&local->mtx);
}

static void ieee80211_sta_tx_wmm_ac_notify(struct ieee80211_sub_if_data *sdata,
					   struct ieee80211_hdr *hdr,
					   u16 tx_time)
{
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	u16 tid = *ieee80211_get_qos_ctl(hdr) & IEEE80211_QOS_CTL_TID_MASK;
	int ac = ieee80211_ac_from_tid(tid);
	struct ieee80211_sta_tx_tspec *tx_tspec = &ifmgd->tx_tspec[ac];
	unsigned long now = jiffies;

	if (likely(!tx_tspec->admitted_time))
		return;

	if (time_after(now, tx_tspec->time_slice_start + HZ)) {
		tx_tspec->consumed_tx_time = 0;
		tx_tspec->time_slice_start = now;

		if (tx_tspec->downgraded) {
			tx_tspec->action = TX_TSPEC_ACTION_STOP_DOWNGRADE;
			schedule_delayed_work(&ifmgd->tx_tspec_wk, 0);
		}
	}

	if (tx_tspec->downgraded)
		return;

	tx_tspec->consumed_tx_time += tx_time;

	if (tx_tspec->consumed_tx_time >= tx_tspec->admitted_time) {
		tx_tspec->downgraded = true;
		tx_tspec->action = TX_TSPEC_ACTION_DOWNGRADE;
		schedule_delayed_work(&ifmgd->tx_tspec_wk, 0);
	}
}

void ieee80211_sta_tx_notify(struct ieee80211_sub_if_data *sdata,
			     struct ieee80211_hdr *hdr, bool ack, u16 tx_time)
{
	ieee80211_sta_tx_wmm_ac_notify(sdata, hdr, tx_time);

	if (!ieee80211_is_data(hdr->frame_control))
	    return;

	if (ieee80211_is_nullfunc(hdr->frame_control) &&
	    sdata->u.mgd.probe_send_count > 0) {
		if (ack)
			ieee80211_sta_reset_conn_monitor(sdata);
		else
			sdata->u.mgd.nullfunc_failed = true;
		ieee80211_queue_work(&sdata->local->hw, &sdata->work);
		return;
	}

	if (ack)
		ieee80211_sta_reset_conn_monitor(sdata);
}

static void ieee80211_mgd_probe_ap_send(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	const u8 *ssid;
	u8 *dst = ifmgd->associated->bssid;
	u8 unicast_limit = max(1, max_probe_tries - 3);

	/*
	 * Try sending broadcast probe requests for the last three
	 * probe requests after the first ones failed since some
	 * buggy APs only support broadcast probe requests.
	 */
	if (ifmgd->probe_send_count >= unicast_limit)
		dst = NULL;

	/*
	 * When the hardware reports an accurate Tx ACK status, it's
	 * better to send a nullfunc frame instead of a probe request,
	 * as it will kick us off the AP quickly if we aren't associated
	 * anymore. The timeout will be reset if the frame is ACKed by
	 * the AP.
	 */
	ifmgd->probe_send_count++;

	if (sdata->local->hw.flags & IEEE80211_HW_REPORTS_TX_ACK_STATUS) {
		ifmgd->nullfunc_failed = false;
		ieee80211_send_nullfunc(sdata->local, sdata, 0);
	} else {
		int ssid_len;

		rcu_read_lock();
		ssid = ieee80211_bss_get_ie(ifmgd->associated, WLAN_EID_SSID);
		if (WARN_ON_ONCE(ssid == NULL))
			ssid_len = 0;
		else
			ssid_len = ssid[1];

		ieee80211_send_probe_req(sdata, sdata->vif.addr, dst,
					 ssid + 2, ssid_len, NULL,
					 0, (u32) -1, true, 0,
					 ifmgd->associated->channel, false);
		rcu_read_unlock();
	}

	ifmgd->probe_timeout = jiffies + msecs_to_jiffies(probe_wait_ms);
	run_again(sdata, ifmgd->probe_timeout);
}

static void ieee80211_mgd_probe_ap(struct ieee80211_sub_if_data *sdata,
				   bool beacon)
{
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	bool already = false;

	if (!ieee80211_sdata_running(sdata))
		return;

	sdata_lock(sdata);

	if (!ifmgd->associated)
		goto out;

	mutex_lock(&sdata->local->mtx);

	if (sdata->local->tmp_channel || sdata->local->scanning) {
		mutex_unlock(&sdata->local->mtx);
		goto out;
	}

	if (beacon) {
		mlme_dbg_ratelimited(sdata,
				     "detected beacon loss from AP (missed %d beacons) - probing\n",
				     beacon_loss_count);

		ieee80211_cqm_beacon_loss_notify(&sdata->vif, GFP_KERNEL);
	}

	/*
	 * The driver/our work has already reported this event or the
	 * connection monitoring has kicked in and we have already sent
	 * a probe request. Or maybe the AP died and the driver keeps
	 * reporting until we disassociate...
	 *
	 * In either case we have to ignore the current call to this
	 * function (except for setting the correct probe reason bit)
	 * because otherwise we would reset the timer every time and
	 * never check whether we received a probe response!
	 */
	if (ifmgd->flags & IEEE80211_STA_CONNECTION_POLL)
		already = true;

	ifmgd->flags |= IEEE80211_STA_CONNECTION_POLL;

	mutex_unlock(&sdata->local->mtx);

	if (already)
		goto out;

	mutex_lock(&sdata->local->iflist_mtx);
	ieee80211_recalc_ps(sdata->local, -1);
	mutex_unlock(&sdata->local->iflist_mtx);

	ifmgd->probe_send_count = 0;
	ieee80211_mgd_probe_ap_send(sdata);
 out:
	sdata_unlock(sdata);
}

struct sk_buff *ieee80211_ap_probereq_get(struct ieee80211_hw *hw,
					  struct ieee80211_vif *vif)
{
	struct ieee80211_sub_if_data *sdata = vif_to_sdata(vif);
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	struct cfg80211_bss *cbss;
	struct sk_buff *skb;
	const u8 *ssid;
	int ssid_len;

	if (WARN_ON(sdata->vif.type != NL80211_IFTYPE_STATION))
		return NULL;

	sdata_assert_lock(sdata);

	if (ifmgd->associated)
		cbss = ifmgd->associated;
	else if (ifmgd->auth_data)
		cbss = ifmgd->auth_data->bss;
	else if (ifmgd->assoc_data)
		cbss = ifmgd->assoc_data->bss;
	else
		return NULL;

	rcu_read_lock();
	ssid = ieee80211_bss_get_ie(cbss, WLAN_EID_SSID);
	if (WARN_ON_ONCE(ssid == NULL))
		ssid_len = 0;
	else
		ssid_len = ssid[1];

	skb = ieee80211_build_probe_req(sdata, sdata->vif.addr, cbss->bssid,
					(u32) -1, cbss->channel,
					ssid + 2, ssid_len,
					NULL, 0, true);
	rcu_read_unlock();

	return skb;
}
EXPORT_SYMBOL(ieee80211_ap_probereq_get);

static void ieee80211_report_disconnect(struct ieee80211_sub_if_data *sdata,
					const u8 *buf, size_t len, bool tx,
					u16 reason)
{
	struct ieee80211_event event = {
		.type = MLME_EVENT,
		.u.mlme.data = tx ? DEAUTH_TX_EVENT : DEAUTH_RX_EVENT,
		.u.mlme.reason = reason,
	};

	if (tx)
		cfg80211_tx_mlme_mgmt(sdata->dev, buf, len);
	else
		cfg80211_rx_mlme_mgmt(sdata->dev, buf, len);

	drv_event_callback(sdata->local, sdata, &event);
}

static void __ieee80211_disconnect(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	u8 frame_buf[IEEE80211_DEAUTH_FRAME_LEN];

	sdata_lock(sdata);
	if (!ifmgd->associated) {
		sdata_unlock(sdata);
		return;
	}

	ieee80211_set_disassoc(sdata, IEEE80211_STYPE_DEAUTH,
			       WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY,
			       true, frame_buf);
	mutex_lock(&local->mtx);
	sdata->vif.csa_active = false;
	ifmgd->csa_waiting_bcn = false;
	if (sdata->csa_block_tx) {
		ieee80211_wake_vif_queues(local, sdata,
					  IEEE80211_QUEUE_STOP_REASON_CSA);
		sdata->csa_block_tx = false;
	}
	mutex_unlock(&local->mtx);

	ieee80211_report_disconnect(sdata, frame_buf, sizeof(frame_buf), true,
				    WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY);

	sdata_unlock(sdata);
}

static void ieee80211_beacon_connection_loss_work(struct work_struct *work)
{
	struct ieee80211_sub_if_data *sdata =
		container_of(work, struct ieee80211_sub_if_data,
			     u.mgd.beacon_connection_loss_work);
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	struct sta_info *sta;

	if (ifmgd->associated) {
		rcu_read_lock();
		sta = sta_info_get(sdata, ifmgd->bssid);
		if (sta)
			sta->beacon_loss_count++;
		rcu_read_unlock();
	}

	if (ifmgd->connection_loss) {
		sdata_info(sdata, "Connection to AP %pM lost\n",
			   ifmgd->bssid);
		__ieee80211_disconnect(sdata);
	} else {
		ieee80211_mgd_probe_ap(sdata, true);
	}
}

static void ieee80211_csa_connection_drop_work(struct work_struct *work)
{
	struct ieee80211_sub_if_data *sdata =
		container_of(work, struct ieee80211_sub_if_data,
			     u.mgd.csa_connection_drop_work);

	__ieee80211_disconnect(sdata);
}

void ieee80211_beacon_loss(struct ieee80211_vif *vif)
{
	struct ieee80211_sub_if_data *sdata = vif_to_sdata(vif);
	struct ieee80211_hw *hw = &sdata->local->hw;

	trace_api_beacon_loss(sdata);

	sdata->u.mgd.connection_loss = false;
	ieee80211_queue_work(hw, &sdata->u.mgd.beacon_connection_loss_work);
}
EXPORT_SYMBOL(ieee80211_beacon_loss);

void ieee80211_connection_loss(struct ieee80211_vif *vif)
{
	struct ieee80211_sub_if_data *sdata = vif_to_sdata(vif);
	struct ieee80211_hw *hw = &sdata->local->hw;

	trace_api_connection_loss(sdata);

	sdata->u.mgd.connection_loss = true;
	ieee80211_queue_work(hw, &sdata->u.mgd.beacon_connection_loss_work);
}
EXPORT_SYMBOL(ieee80211_connection_loss);


static void ieee80211_destroy_auth_data(struct ieee80211_sub_if_data *sdata,
					bool assoc)
{
	struct ieee80211_mgd_auth_data *auth_data = sdata->u.mgd.auth_data;

	sdata_assert_lock(sdata);

	if (!assoc) {
		/*
		 * we are not authenticated yet, the only timer that could be
		 * running is the timeout for the authentication response which
		 * which is not relevant anymore.
		 */
		del_timer_sync(&sdata->u.mgd.timer);
		sta_info_destroy_addr(sdata, auth_data->bss->bssid);

		eth_zero_addr(sdata->u.mgd.bssid);
		ieee80211_bss_info_change_notify(sdata, BSS_CHANGED_BSSID);
		sdata->u.mgd.flags = 0;
		mutex_lock(&sdata->local->mtx);
		ieee80211_vif_release_channel(sdata);
		mutex_unlock(&sdata->local->mtx);
	}

	cfg80211_put_bss(sdata->local->hw.wiphy, auth_data->bss);
	kfree(auth_data);
	sdata->u.mgd.auth_data = NULL;
}

static void ieee80211_auth_challenge(struct ieee80211_sub_if_data *sdata,
				     struct ieee80211_mgmt *mgmt, size_t len)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_mgd_auth_data *auth_data = sdata->u.mgd.auth_data;
	u8 *pos;
	struct ieee802_11_elems elems;
	u32 tx_flags = 0;

	pos = mgmt->u.auth.variable;
	ieee802_11_parse_elems(pos, len - (pos - (u8 *) mgmt), false, &elems);
	if (!elems.challenge)
		return;
	auth_data->expected_transaction = 4;
	drv_mgd_prepare_tx(sdata->local, sdata);
	if (local->hw.flags & IEEE80211_HW_REPORTS_TX_ACK_STATUS)
		tx_flags = IEEE80211_TX_CTL_REQ_TX_STATUS |
			   IEEE80211_TX_INTFL_MLME_CONN_TX;
	ieee80211_send_auth(sdata, 3, auth_data->algorithm, 0,
			    elems.challenge - 2, elems.challenge_len + 2,
			    auth_data->bss->bssid, auth_data->bss->bssid,
			    auth_data->key, auth_data->key_len,
			    auth_data->key_idx, tx_flags);
}

static void ieee80211_rx_mgmt_auth(struct ieee80211_sub_if_data *sdata,
				   struct ieee80211_mgmt *mgmt, size_t len)
{
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	u8 bssid[ETH_ALEN];
	u16 auth_alg, auth_transaction, status_code;
	struct sta_info *sta;
	struct ieee80211_event event = {
		.type = MLME_EVENT,
		.u.mlme.data = AUTH_EVENT,
	};

	sdata_assert_lock(sdata);

	if (len < 24 + 6)
		return;

	if (!ifmgd->auth_data || ifmgd->auth_data->done)
		return;

	memcpy(bssid, ifmgd->auth_data->bss->bssid, ETH_ALEN);

	if (!ether_addr_equal(bssid, mgmt->bssid))
		return;

	auth_alg = le16_to_cpu(mgmt->u.auth.auth_alg);
	auth_transaction = le16_to_cpu(mgmt->u.auth.auth_transaction);
	status_code = le16_to_cpu(mgmt->u.auth.status_code);

	if (auth_alg != ifmgd->auth_data->algorithm ||
	    auth_transaction != ifmgd->auth_data->expected_transaction) {
		sdata_info(sdata, "%pM unexpected authentication state: alg %d (expected %d) transact %d (expected %d)\n",
			   mgmt->sa, auth_alg, ifmgd->auth_data->algorithm,
			   auth_transaction,
			   ifmgd->auth_data->expected_transaction);
		return;
	}

	if (status_code != WLAN_STATUS_SUCCESS) {
		sdata_info(sdata, "%pM denied authentication (status %d)\n",
			   mgmt->sa, status_code);
		ieee80211_destroy_auth_data(sdata, false);
		cfg80211_rx_mlme_mgmt(sdata->dev, (u8 *)mgmt, len);
		event.u.mlme.status = MLME_DENIED;
		event.u.mlme.reason = status_code;
		drv_event_callback(sdata->local, sdata, &event);
		return;
	}

	switch (ifmgd->auth_data->algorithm) {
	case WLAN_AUTH_OPEN:
	case WLAN_AUTH_LEAP:
	case WLAN_AUTH_FT:
	case WLAN_AUTH_SAE:
		break;
	case WLAN_AUTH_SHARED_KEY:
		if (ifmgd->auth_data->expected_transaction != 4) {
			ieee80211_auth_challenge(sdata, mgmt, len);
			/* need another frame */
			return;
		}
		break;
	default:
		WARN_ONCE(1, "invalid auth alg %d",
			  ifmgd->auth_data->algorithm);
		return;
	}

	event.u.mlme.status = MLME_SUCCESS;
	drv_event_callback(sdata->local, sdata, &event);
	sdata_info(sdata, "authenticated\n");
	ifmgd->auth_data->done = true;
	ifmgd->auth_data->timeout = jiffies + IEEE80211_AUTH_WAIT_ASSOC;
	ifmgd->auth_data->timeout_started = true;
	run_again(sdata, ifmgd->auth_data->timeout);

	if (ifmgd->auth_data->algorithm == WLAN_AUTH_SAE &&
	    ifmgd->auth_data->expected_transaction != 2) {
		/*
		 * Report auth frame to user space for processing since another
		 * round of Authentication frames is still needed.
		 */
		cfg80211_rx_mlme_mgmt(sdata->dev, (u8 *)mgmt, len);
		return;
	}

	/* move station state to auth */
	mutex_lock(&sdata->local->sta_mtx);
	sta = sta_info_get(sdata, bssid);
	if (!sta) {
		WARN_ONCE(1, "%s: STA %pM not found", sdata->name, bssid);
		goto out_err;
	}
	if (sta_info_move_state(sta, IEEE80211_STA_AUTH)) {
		sdata_info(sdata, "failed moving %pM to auth\n", bssid);
		goto out_err;
	}
	mutex_unlock(&sdata->local->sta_mtx);

	cfg80211_rx_mlme_mgmt(sdata->dev, (u8 *)mgmt, len);
	return;
 out_err:
	mutex_unlock(&sdata->local->sta_mtx);
	/* ignore frame -- wait for timeout */
}

#define case_WLAN(type) \
	case WLAN_REASON_##type: return #type

static const char *ieee80211_get_reason_code_string(u16 reason_code)
{
	switch (reason_code) {
	case_WLAN(UNSPECIFIED);
	case_WLAN(PREV_AUTH_NOT_VALID);
	case_WLAN(DEAUTH_LEAVING);
	case_WLAN(DISASSOC_DUE_TO_INACTIVITY);
	case_WLAN(DISASSOC_AP_BUSY);
	case_WLAN(CLASS2_FRAME_FROM_NONAUTH_STA);
	case_WLAN(CLASS3_FRAME_FROM_NONASSOC_STA);
	case_WLAN(DISASSOC_STA_HAS_LEFT);
	case_WLAN(STA_REQ_ASSOC_WITHOUT_AUTH);
	case_WLAN(DISASSOC_BAD_POWER);
	case_WLAN(DISASSOC_BAD_SUPP_CHAN);
	case_WLAN(INVALID_IE);
	case_WLAN(MIC_FAILURE);
	case_WLAN(4WAY_HANDSHAKE_TIMEOUT);
	case_WLAN(GROUP_KEY_HANDSHAKE_TIMEOUT);
	case_WLAN(IE_DIFFERENT);
	case_WLAN(INVALID_GROUP_CIPHER);
	case_WLAN(INVALID_PAIRWISE_CIPHER);
	case_WLAN(INVALID_AKMP);
	case_WLAN(UNSUPP_RSN_VERSION);
	case_WLAN(INVALID_RSN_IE_CAP);
	case_WLAN(IEEE8021X_FAILED);
	case_WLAN(CIPHER_SUITE_REJECTED);
	case_WLAN(DISASSOC_UNSPECIFIED_QOS);
	case_WLAN(DISASSOC_QAP_NO_BANDWIDTH);
	case_WLAN(DISASSOC_LOW_ACK);
	case_WLAN(DISASSOC_QAP_EXCEED_TXOP);
	case_WLAN(QSTA_LEAVE_QBSS);
	case_WLAN(QSTA_NOT_USE);
	case_WLAN(QSTA_REQUIRE_SETUP);
	case_WLAN(QSTA_TIMEOUT);
	case_WLAN(QSTA_CIPHER_NOT_SUPP);
	case_WLAN(MESH_PEER_CANCELED);
	case_WLAN(MESH_MAX_PEERS);
	case_WLAN(MESH_CONFIG);
	case_WLAN(MESH_CLOSE);
	case_WLAN(MESH_MAX_RETRIES);
	case_WLAN(MESH_CONFIRM_TIMEOUT);
	case_WLAN(MESH_INVALID_GTK);
	case_WLAN(MESH_INCONSISTENT_PARAM);
	case_WLAN(MESH_INVALID_SECURITY);
	case_WLAN(MESH_PATH_ERROR);
	case_WLAN(MESH_PATH_NOFORWARD);
	case_WLAN(MESH_PATH_DEST_UNREACHABLE);
	case_WLAN(MAC_EXISTS_IN_MBSS);
	case_WLAN(MESH_CHAN_REGULATORY);
	case_WLAN(MESH_CHAN);
	default: return "<unknown>";
	}
}

static void ieee80211_rx_mgmt_deauth(struct ieee80211_sub_if_data *sdata,
				     struct ieee80211_mgmt *mgmt, size_t len)
{
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	const u8 *bssid = NULL;
	u16 reason_code;

	sdata_assert_lock(sdata);

	if (len < 24 + 2)
		return;

	if (!ifmgd->associated ||
	    !ether_addr_equal(mgmt->bssid, ifmgd->associated->bssid))
		return;

	bssid = ifmgd->associated->bssid;

	reason_code = le16_to_cpu(mgmt->u.deauth.reason_code);

	sdata_info(sdata, "deauthenticated from %pM (Reason: %u=%s)\n",
		   bssid, reason_code, ieee80211_get_reason_code_string(reason_code));

	ieee80211_set_disassoc(sdata, 0, 0, false, NULL);

	ieee80211_report_disconnect(sdata, (u8 *)mgmt, len, false, reason_code);
}


static void ieee80211_rx_mgmt_disassoc(struct ieee80211_sub_if_data *sdata,
				       struct ieee80211_mgmt *mgmt, size_t len)
{
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	u16 reason_code;

	sdata_assert_lock(sdata);

	if (len < 24 + 2)
		return;

	if (!ifmgd->associated ||
	    !ether_addr_equal(mgmt->bssid, ifmgd->associated->bssid))
		return;

	reason_code = le16_to_cpu(mgmt->u.disassoc.reason_code);

	sdata_info(sdata, "disassociated from %pM (Reason: %u)\n",
		   mgmt->sa, reason_code);

	ieee80211_set_disassoc(sdata, 0, 0, false, NULL);

	ieee80211_report_disconnect(sdata, (u8 *)mgmt, len, false, reason_code);
}

static void ieee80211_get_rates(struct ieee80211_supported_band *sband,
				u8 *supp_rates, unsigned int supp_rates_len,
				u32 *rates, u32 *basic_rates,
				bool *have_higher_than_11mbit,
				int *min_rate, int *min_rate_index,
				int shift, u32 rate_flags)
{
	int i, j;

	for (i = 0; i < supp_rates_len; i++) {
		int rate = supp_rates[i] & 0x7f;
		bool is_basic = !!(supp_rates[i] & 0x80);

		if ((rate * 5 * (1 << shift)) > 110)
			*have_higher_than_11mbit = true;

		/*
		 * BSS_MEMBERSHIP_SELECTOR_HT_PHY is defined in 802.11n-2009
		 * 7.3.2.2 as a magic value instead of a rate. Hence, skip it.
		 *
		 * Note: Even through the membership selector and the basic
		 *	 rate flag share the same bit, they are not exactly
		 *	 the same.
		 */
		if (!!(supp_rates[i] & 0x80) &&
		    (supp_rates[i] & 0x7f) == BSS_MEMBERSHIP_SELECTOR_HT_PHY)
			continue;

		for (j = 0; j < sband->n_bitrates; j++) {
			struct ieee80211_rate *br;
			int brate;

			br = &sband->bitrates[j];
			if ((rate_flags & br->flags) != rate_flags)
				continue;

			brate = DIV_ROUND_UP(br->bitrate, (1 << shift) * 5);
			if (brate == rate) {
				*rates |= BIT(j);
				if (is_basic)
					*basic_rates |= BIT(j);
				if ((rate * 5) < *min_rate) {
					*min_rate = rate * 5;
					*min_rate_index = j;
				}
				break;
			}
		}
	}
}

static void ieee80211_destroy_assoc_data(struct ieee80211_sub_if_data *sdata,
					 bool assoc)
{
	struct ieee80211_mgd_assoc_data *assoc_data = sdata->u.mgd.assoc_data;

	sdata_assert_lock(sdata);

	if (!assoc) {
		/*
		 * we are not associated yet, the only timer that could be
		 * running is the timeout for the association response which
		 * which is not relevant anymore.
		 */
		del_timer_sync(&sdata->u.mgd.timer);
		sta_info_destroy_addr(sdata, assoc_data->bss->bssid);

		eth_zero_addr(sdata->u.mgd.bssid);
		ieee80211_bss_info_change_notify(sdata, BSS_CHANGED_BSSID);
		sdata->u.mgd.flags = 0;
		mutex_lock(&sdata->local->mtx);
		ieee80211_vif_release_channel(sdata);
		mutex_unlock(&sdata->local->mtx);
	}

	kfree(assoc_data);
	sdata->u.mgd.assoc_data = NULL;
}

static bool ieee80211_assoc_success(struct ieee80211_sub_if_data *sdata,
				    struct cfg80211_bss *cbss,
				    struct ieee80211_mgmt *mgmt, size_t len)
{
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_supported_band *sband;
	struct sta_info *sta;
	u8 *pos;
	u16 capab_info, aid;
	struct ieee802_11_elems elems;
	struct ieee80211_bss_conf *bss_conf = &sdata->vif.bss_conf;
	const struct cfg80211_bss_ies *bss_ies = NULL;
	struct ieee80211_mgd_assoc_data *assoc_data = ifmgd->assoc_data;
	u32 changed = 0;
	int err;
	bool ret;

	/* AssocResp and ReassocResp have identical structure */

	aid = le16_to_cpu(mgmt->u.assoc_resp.aid);
	capab_info = le16_to_cpu(mgmt->u.assoc_resp.capab_info);

	if ((aid & (BIT(15) | BIT(14))) != (BIT(15) | BIT(14)))
		sdata_info(sdata, "invalid AID value 0x%x; bits 15:14 not set\n",
			   aid);
	aid &= ~(BIT(15) | BIT(14));

	ifmgd->broken_ap = false;

	if (aid == 0 || aid > IEEE80211_MAX_AID) {
		sdata_info(sdata, "invalid AID value %d (out of range), turn off PS\n",
			   aid);
		aid = 0;
		ifmgd->broken_ap = true;
	}

	pos = mgmt->u.assoc_resp.variable;
	ieee802_11_parse_elems(pos, len - (pos - (u8 *) mgmt), false, &elems);

	if (!elems.supp_rates) {
		sdata_info(sdata, "no SuppRates element in AssocResp\n");
		return false;
	}

	ifmgd->aid = aid;
	ifmgd->tdls_chan_switch_prohibited =
		elems.ext_capab && elems.ext_capab_len >= 5 &&
		(elems.ext_capab[4] & WLAN_EXT_CAPA5_TDLS_CH_SW_PROHIBITED);

	/*
	 * Some APs are erroneously not including some information in their
	 * (re)association response frames. Try to recover by using the data
	 * from the beacon or probe response. This seems to afflict mobile
	 * 2G/3G/4G wifi routers, reported models include the "Onda PN51T",
	 * "Vodafone PocketWiFi 2", "ZTE MF60" and a similar T-Mobile device.
	 */
	if ((assoc_data->wmm && !elems.wmm_param) ||
	    (!(ifmgd->flags & IEEE80211_STA_DISABLE_HT) &&
	     (!elems.ht_cap_elem || !elems.ht_operation)) ||
	    (!(ifmgd->flags & IEEE80211_STA_DISABLE_VHT) &&
	     (!elems.vht_cap_elem || !elems.vht_operation))) {
		const struct cfg80211_bss_ies *ies;
		struct ieee802_11_elems bss_elems;

		rcu_read_lock();
		ies = rcu_dereference(cbss->ies);
		if (ies)
			bss_ies = kmemdup(ies, sizeof(*ies) + ies->len,
					  GFP_ATOMIC);
		rcu_read_unlock();
		if (!bss_ies)
			return false;

		ieee802_11_parse_elems(bss_ies->data, bss_ies->len,
				       false, &bss_elems);
		if (assoc_data->wmm &&
		    !elems.wmm_param && bss_elems.wmm_param) {
			elems.wmm_param = bss_elems.wmm_param;
			sdata_info(sdata,
				   "AP bug: WMM param missing from AssocResp\n");
		}

		/*
		 * Also check if we requested HT/VHT, otherwise the AP doesn't
		 * have to include the IEs in the (re)association response.
		 */
		if (!elems.ht_cap_elem && bss_elems.ht_cap_elem &&
		    !(ifmgd->flags & IEEE80211_STA_DISABLE_HT)) {
			elems.ht_cap_elem = bss_elems.ht_cap_elem;
			sdata_info(sdata,
				   "AP bug: HT capability missing from AssocResp\n");
		}
		if (!elems.ht_operation && bss_elems.ht_operation &&
		    !(ifmgd->flags & IEEE80211_STA_DISABLE_HT)) {
			elems.ht_operation = bss_elems.ht_operation;
			sdata_info(sdata,
				   "AP bug: HT operation missing from AssocResp\n");
		}
		if (!elems.vht_cap_elem && bss_elems.vht_cap_elem &&
		    !(ifmgd->flags & IEEE80211_STA_DISABLE_VHT)) {
			elems.vht_cap_elem = bss_elems.vht_cap_elem;
			sdata_info(sdata,
				   "AP bug: VHT capa missing from AssocResp\n");
		}
		if (!elems.vht_operation && bss_elems.vht_operation &&
		    !(ifmgd->flags & IEEE80211_STA_DISABLE_VHT)) {
			elems.vht_operation = bss_elems.vht_operation;
			sdata_info(sdata,
				   "AP bug: VHT operation missing from AssocResp\n");
		}
	}

	/*
	 * We previously checked these in the beacon/probe response, so
	 * they should be present here. This is just a safety net.
	 */
	if (!(ifmgd->flags & IEEE80211_STA_DISABLE_HT) &&
	    (!elems.wmm_param || !elems.ht_cap_elem || !elems.ht_operation)) {
		sdata_info(sdata,
			   "HT AP is missing WMM params or HT capability/operation\n");
		ret = false;
		goto out;
	}

	if (!(ifmgd->flags & IEEE80211_STA_DISABLE_VHT) &&
	    (!elems.vht_cap_elem || !elems.vht_operation)) {
		sdata_info(sdata,
			   "VHT AP is missing VHT capability/operation\n");
		ret = false;
		goto out;
	}

	mutex_lock(&sdata->local->sta_mtx);
	/*
	 * station info was already allocated and inserted before
	 * the association and should be available to us
	 */
	sta = sta_info_get(sdata, cbss->bssid);
	if (WARN_ON(!sta)) {
		mutex_unlock(&sdata->local->sta_mtx);
		ret = false;
		goto out;
	}

	sband = local->hw.wiphy->bands[ieee80211_get_sdata_band(sdata)];

	/* Set up internal HT/VHT capabilities */
	if (elems.ht_cap_elem && !(ifmgd->flags & IEEE80211_STA_DISABLE_HT))
		ieee80211_ht_cap_ie_to_sta_ht_cap(sdata, sband,
						  elems.ht_cap_elem, sta);

	if (elems.vht_cap_elem && !(ifmgd->flags & IEEE80211_STA_DISABLE_VHT))
		ieee80211_vht_cap_ie_to_sta_vht_cap(sdata, sband,
						    elems.vht_cap_elem, sta);

	/*
	 * Some APs, e.g. Netgear WNDR3700, report invalid HT operation data
	 * in their association response, so ignore that data for our own
	 * configuration. If it changed since the last beacon, we'll get the
	 * next beacon and update then.
	 */

	/*
	 * If an operating mode notification IE is present, override the
	 * NSS calculation (that would be done in rate_control_rate_init())
	 * and use the # of streams from that element.
	 */
	if (elems.opmode_notif &&
	    !(*elems.opmode_notif & IEEE80211_OPMODE_NOTIF_RX_NSS_TYPE_BF)) {
		u8 nss;

		nss = *elems.opmode_notif & IEEE80211_OPMODE_NOTIF_RX_NSS_MASK;
		nss >>= IEEE80211_OPMODE_NOTIF_RX_NSS_SHIFT;
		nss += 1;
		sta->sta.rx_nss = nss;
	}

	rate_control_rate_init(sta);

	if (ifmgd->flags & IEEE80211_STA_MFP_ENABLED) {
		set_sta_flag(sta, WLAN_STA_MFP);
		sta->sta.mfp = true;
	} else {
		sta->sta.mfp = false;
	}

	sta->sta.wme = elems.wmm_param && local->hw.queues >= IEEE80211_NUM_ACS;

	err = sta_info_move_state(sta, IEEE80211_STA_ASSOC);
	if (!err && !(ifmgd->flags & IEEE80211_STA_CONTROL_PORT))
		err = sta_info_move_state(sta, IEEE80211_STA_AUTHORIZED);
	if (err) {
		sdata_info(sdata,
			   "failed to move station %pM to desired state\n",
			   sta->sta.addr);
		WARN_ON(__sta_info_destroy(sta));
		mutex_unlock(&sdata->local->sta_mtx);
		ret = false;
		goto out;
	}

	mutex_unlock(&sdata->local->sta_mtx);

	/*
	 * Always handle WMM once after association regardless
	 * of the first value the AP uses. Setting -1 here has
	 * that effect because the AP values is an unsigned
	 * 4-bit value.
	 */
	ifmgd->wmm_last_param_set = -1;

	if (!(ifmgd->flags & IEEE80211_STA_DISABLE_WMM) && elems.wmm_param)
		ieee80211_sta_wmm_params(local, sdata, elems.wmm_param,
					 elems.wmm_param_len);
	else
		ieee80211_set_wmm_default(sdata, false);
	changed |= BSS_CHANGED_QOS;

	/* set AID and assoc capability,
	 * ieee80211_set_associated() will tell the driver */
	bss_conf->aid = aid;
	bss_conf->assoc_capability = capab_info;
	ieee80211_set_associated(sdata, cbss, changed);

	/*
	 * If we're using 4-addr mode, let the AP know that we're
	 * doing so, so that it can create the STA VLAN on its side
	 */
	if (ifmgd->use_4addr)
		ieee80211_send_4addr_nullfunc(local, sdata);

	/*
	 * Start timer to probe the connection to the AP now.
	 * Also start the timer that will detect beacon loss.
	 */
	ieee80211_sta_rx_notify(sdata, (struct ieee80211_hdr *)mgmt);
	ieee80211_sta_reset_beacon_monitor(sdata);

	ret = true;
 out:
	kfree(bss_ies);
	return ret;
}

static void ieee80211_rx_mgmt_assoc_resp(struct ieee80211_sub_if_data *sdata,
					 struct ieee80211_mgmt *mgmt,
					 size_t len)
{
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	struct ieee80211_mgd_assoc_data *assoc_data = ifmgd->assoc_data;
	u16 capab_info, status_code, aid;
	struct ieee802_11_elems elems;
	int ac, uapsd_queues = -1;
	u8 *pos;
	bool reassoc;
	struct cfg80211_bss *bss;
	struct ieee80211_event event = {
		.type = MLME_EVENT,
		.u.mlme.data = ASSOC_EVENT,
	};

	sdata_assert_lock(sdata);

	if (!assoc_data)
		return;
	if (!ether_addr_equal(assoc_data->bss->bssid, mgmt->bssid))
		return;

	/*
	 * AssocResp and ReassocResp have identical structure, so process both
	 * of them in this function.
	 */

	if (len < 24 + 6)
		return;

	reassoc = ieee80211_is_reassoc_req(mgmt->frame_control);
	capab_info = le16_to_cpu(mgmt->u.assoc_resp.capab_info);
	status_code = le16_to_cpu(mgmt->u.assoc_resp.status_code);
	aid = le16_to_cpu(mgmt->u.assoc_resp.aid);

	sdata_info(sdata,
		   "RX %sssocResp from %pM (capab=0x%x status=%d aid=%d)\n",
		   reassoc ? "Rea" : "A", mgmt->sa,
		   capab_info, status_code, (u16)(aid & ~(BIT(15) | BIT(14))));

	pos = mgmt->u.assoc_resp.variable;
	ieee802_11_parse_elems(pos, len - (pos - (u8 *) mgmt), false, &elems);

	if (status_code == WLAN_STATUS_ASSOC_REJECTED_TEMPORARILY &&
	    elems.timeout_int &&
	    elems.timeout_int->type == WLAN_TIMEOUT_ASSOC_COMEBACK) {
		u32 tu, ms;
		tu = le32_to_cpu(elems.timeout_int->value);
		ms = tu * 1024 / 1000;
		sdata_info(sdata,
			   "%pM rejected association temporarily; comeback duration %u TU (%u ms)\n",
			   mgmt->sa, tu, ms);
		assoc_data->timeout = jiffies + msecs_to_jiffies(ms);
		assoc_data->timeout_started = true;
		if (ms > IEEE80211_ASSOC_TIMEOUT)
			run_again(sdata, assoc_data->timeout);
		return;
	}

	bss = assoc_data->bss;

	if (status_code != WLAN_STATUS_SUCCESS) {
		sdata_info(sdata, "%pM denied association (code=%d)\n",
			   mgmt->sa, status_code);
		ieee80211_destroy_assoc_data(sdata, false);
		event.u.mlme.status = MLME_DENIED;
		event.u.mlme.reason = status_code;
		drv_event_callback(sdata->local, sdata, &event);
	} else {
		if (!ieee80211_assoc_success(sdata, bss, mgmt, len)) {
			/* oops -- internal error -- send timeout for now */
			ieee80211_destroy_assoc_data(sdata, false);
			cfg80211_assoc_timeout(sdata->dev, bss);
			return;
		}
		event.u.mlme.status = MLME_SUCCESS;
		drv_event_callback(sdata->local, sdata, &event);
		sdata_info(sdata, "associated\n");

		/*
		 * destroy assoc_data afterwards, as otherwise an idle
		 * recalc after assoc_data is NULL but before associated
		 * is set can cause the interface to go idle
		 */
		ieee80211_destroy_assoc_data(sdata, true);

		/* get uapsd queues configuration */
		uapsd_queues = 0;
		for (ac = 0; ac < IEEE80211_NUM_ACS; ac++)
			if (sdata->tx_conf[ac].uapsd)
				uapsd_queues |= BIT(ac);
	}

	cfg80211_rx_assoc_resp(sdata->dev, bss, (u8 *)mgmt, len, uapsd_queues);
}

static void ieee80211_rx_bss_info(struct ieee80211_sub_if_data *sdata,
				  struct ieee80211_mgmt *mgmt, size_t len,
				  struct ieee80211_rx_status *rx_status,
				  struct ieee802_11_elems *elems)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_bss *bss;
	struct ieee80211_channel *channel;

	sdata_assert_lock(sdata);

	channel = ieee80211_get_channel(local->hw.wiphy, rx_status->freq);
	if (!channel)
		return;

	bss = ieee80211_bss_info_update(local, rx_status, mgmt, len, elems,
					channel);
	if (bss) {
		sdata->vif.bss_conf.beacon_rate = bss->beacon_rate;
		ieee80211_rx_bss_put(local, bss);
	}
}


static void ieee80211_rx_mgmt_probe_resp(struct ieee80211_sub_if_data *sdata,
					 struct sk_buff *skb)
{
	struct ieee80211_mgmt *mgmt = (void *)skb->data;
	struct ieee80211_if_managed *ifmgd;
	struct ieee80211_rx_status *rx_status = (void *) skb->cb;
	size_t baselen, len = skb->len;
	struct ieee802_11_elems elems;

	ifmgd = &sdata->u.mgd;

	sdata_assert_lock(sdata);

	if (!ether_addr_equal(mgmt->da, sdata->vif.addr))
		return; /* ignore ProbeResp to foreign address */

	baselen = (u8 *) mgmt->u.probe_resp.variable - (u8 *) mgmt;
	if (baselen > len)
		return;

	ieee802_11_parse_elems(mgmt->u.probe_resp.variable, len - baselen,
			       false, &elems);

	ieee80211_rx_bss_info(sdata, mgmt, len, rx_status, &elems);

	if (ifmgd->associated &&
	    ether_addr_equal(mgmt->bssid, ifmgd->associated->bssid))
		ieee80211_reset_ap_probe(sdata);

	if (ifmgd->auth_data && !ifmgd->auth_data->bss->proberesp_ies &&
	    ether_addr_equal(mgmt->bssid, ifmgd->auth_data->bss->bssid)) {
		/* got probe response, continue with auth */
		sdata_info(sdata, "direct probe responded\n");
		ifmgd->auth_data->tries = 0;
		ifmgd->auth_data->timeout = jiffies;
		ifmgd->auth_data->timeout_started = true;
		run_again(sdata, ifmgd->auth_data->timeout);
	}
}

/*
 * This is the canonical list of information elements we care about,
 * the filter code also gives us all changes to the Microsoft OUI
 * (00:50:F2) vendor IE which is used for WMM which we need to track,
 * as well as the DTPC IE (part of the Cisco OUI) used for signaling
 * changes to requested client power.
 *
 * We implement beacon filtering in software since that means we can
 * avoid processing the frame here and in cfg80211, and userspace
 * will not be able to tell whether the hardware supports it or not.
 *
 * XXX: This list needs to be dynamic -- userspace needs to be able to
 *	add items it requires. It also needs to be able to tell us to
 *	look out for other vendor IEs.
 */
static const u64 care_about_ies =
	(1ULL << WLAN_EID_COUNTRY) |
	(1ULL << WLAN_EID_ERP_INFO) |
	(1ULL << WLAN_EID_CHANNEL_SWITCH) |
	(1ULL << WLAN_EID_PWR_CONSTRAINT) |
	(1ULL << WLAN_EID_HT_CAPABILITY) |
	(1ULL << WLAN_EID_HT_OPERATION) |
	(1ULL << WLAN_EID_EXT_CHANSWITCH_ANN);

static void ieee80211_rx_mgmt_beacon(struct ieee80211_sub_if_data *sdata,
				     struct ieee80211_mgmt *mgmt, size_t len,
				     struct ieee80211_rx_status *rx_status)
{
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	struct ieee80211_bss_conf *bss_conf = &sdata->vif.bss_conf;
	size_t baselen;
	struct ieee802_11_elems elems;
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_chanctx_conf *chanctx_conf;
	struct ieee80211_channel *chan;
	struct sta_info *sta;
	u32 changed = 0;
	bool erp_valid;
	u8 erp_value = 0;
	u32 ncrc;
	u8 *bssid;
	u8 deauth_buf[IEEE80211_DEAUTH_FRAME_LEN];

	sdata_assert_lock(sdata);

	/* Process beacon from the current BSS */
	baselen = (u8 *) mgmt->u.beacon.variable - (u8 *) mgmt;
	if (baselen > len)
		return;

	rcu_read_lock();
	chanctx_conf = rcu_dereference(sdata->vif.chanctx_conf);
	if (!chanctx_conf) {
		rcu_read_unlock();
		return;
	}

	if (rx_status->freq != chanctx_conf->def.chan->center_freq) {
		rcu_read_unlock();
		return;
	}
	chan = chanctx_conf->def.chan;
	rcu_read_unlock();

	if (ifmgd->assoc_data && ifmgd->assoc_data->need_beacon &&
	    ether_addr_equal(mgmt->bssid, ifmgd->assoc_data->bss->bssid)) {
		ieee802_11_parse_elems(mgmt->u.beacon.variable,
				       len - baselen, false, &elems);

		ieee80211_rx_bss_info(sdata, mgmt, len, rx_status, &elems);
		if (elems.tim && !elems.parse_error) {
			const struct ieee80211_tim_ie *tim_ie = elems.tim;
			ifmgd->dtim_period = tim_ie->dtim_period;
		}
		ifmgd->have_beacon = true;
		ifmgd->assoc_data->need_beacon = false;
		if (local->hw.flags & IEEE80211_HW_TIMING_BEACON_ONLY) {
			sdata->vif.bss_conf.sync_tsf =
				le64_to_cpu(mgmt->u.beacon.timestamp);
			sdata->vif.bss_conf.sync_device_ts =
				rx_status->device_timestamp;
			if (elems.tim)
				sdata->vif.bss_conf.sync_dtim_count =
					elems.tim->dtim_count;
			else
				sdata->vif.bss_conf.sync_dtim_count = 0;
		}
		/* continue assoc process */
		ifmgd->assoc_data->timeout = jiffies;
		ifmgd->assoc_data->timeout_started = true;
		run_again(sdata, ifmgd->assoc_data->timeout);
		return;
	}

	if (!ifmgd->associated ||
	    !ether_addr_equal(mgmt->bssid, ifmgd->associated->bssid))
		return;
	bssid = ifmgd->associated->bssid;

	/* Track average RSSI from the Beacon frames of the current AP */
	ifmgd->last_beacon_signal = rx_status->signal;
	if (ifmgd->flags & IEEE80211_STA_RESET_SIGNAL_AVE) {
		ifmgd->flags &= ~IEEE80211_STA_RESET_SIGNAL_AVE;
		ifmgd->ave_beacon_signal = rx_status->signal * 16;
		ifmgd->last_cqm_event_signal = 0;
		ifmgd->count_beacon_signal = 1;
		ifmgd->last_ave_beacon_signal = 0;
	} else {
		ifmgd->ave_beacon_signal =
			(IEEE80211_SIGNAL_AVE_WEIGHT * rx_status->signal * 16 +
			 (16 - IEEE80211_SIGNAL_AVE_WEIGHT) *
			 ifmgd->ave_beacon_signal) / 16;
		ifmgd->count_beacon_signal++;
	}

	if (ifmgd->rssi_min_thold != ifmgd->rssi_max_thold &&
	    ifmgd->count_beacon_signal >= IEEE80211_SIGNAL_AVE_MIN_COUNT) {
		int sig = ifmgd->ave_beacon_signal / 16;
		int last_sig = ifmgd->last_ave_beacon_signal;
		struct ieee80211_event event = {
			.type = RSSI_EVENT,
		};

		/*
		 * if signal crosses either of the boundaries, invoke callback
		 * with appropriate parameters
		 */
		if (sig > ifmgd->rssi_max_thold &&
		    (last_sig <= ifmgd->rssi_min_thold || last_sig == 0)) {
			ifmgd->last_ave_beacon_signal = sig;
			event.u.rssi.data = RSSI_EVENT_HIGH;
			drv_event_callback(local, sdata, &event);
		} else if (sig < ifmgd->rssi_min_thold &&
			   (last_sig >= ifmgd->rssi_max_thold ||
			   last_sig == 0)) {
			ifmgd->last_ave_beacon_signal = sig;
			event.u.rssi.data = RSSI_EVENT_LOW;
			drv_event_callback(local, sdata, &event);
		}
	}

	if (bss_conf->cqm_rssi_thold &&
	    ifmgd->count_beacon_signal >= IEEE80211_SIGNAL_AVE_MIN_COUNT &&
	    !(sdata->vif.driver_flags & IEEE80211_VIF_SUPPORTS_CQM_RSSI)) {
		int sig = ifmgd->ave_beacon_signal / 16;
		int last_event = ifmgd->last_cqm_event_signal;
		int thold = bss_conf->cqm_rssi_thold;
		int hyst = bss_conf->cqm_rssi_hyst;
		if (sig < thold &&
		    (last_event == 0 || sig < last_event - hyst)) {
			ifmgd->last_cqm_event_signal = sig;
			ieee80211_cqm_rssi_notify(
				&sdata->vif,
				NL80211_CQM_RSSI_THRESHOLD_EVENT_LOW,
				GFP_KERNEL);
		} else if (sig > thold &&
			   (last_event == 0 || sig > last_event + hyst)) {
			ifmgd->last_cqm_event_signal = sig;
			ieee80211_cqm_rssi_notify(
				&sdata->vif,
				NL80211_CQM_RSSI_THRESHOLD_EVENT_HIGH,
				GFP_KERNEL);
		}
	}

	if (ifmgd->flags & IEEE80211_STA_CONNECTION_POLL) {
		mlme_dbg_ratelimited(sdata,
				     "cancelling AP probe due to a received beacon\n");
		ieee80211_reset_ap_probe(sdata);
	}

	/*
	 * Push the beacon loss detection into the future since
	 * we are processing a beacon from the AP just now.
	 */
	ieee80211_sta_reset_beacon_monitor(sdata);

	ncrc = crc32_be(0, (void *)&mgmt->u.beacon.beacon_int, 4);
	ncrc = ieee802_11_parse_elems_crc(mgmt->u.beacon.variable,
					  len - baselen, false, &elems,
					  care_about_ies, ncrc);

	if (local->hw.flags & IEEE80211_HW_PS_NULLFUNC_STACK) {
		bool directed_tim = ieee80211_check_tim(elems.tim,
							elems.tim_len,
							ifmgd->aid);
		if (directed_tim) {
			if (local->hw.conf.dynamic_ps_timeout > 0) {
				if (local->hw.conf.flags & IEEE80211_CONF_PS) {
					local->hw.conf.flags &= ~IEEE80211_CONF_PS;
					ieee80211_hw_config(local,
							    IEEE80211_CONF_CHANGE_PS);
				}
				ieee80211_send_nullfunc(local, sdata, 0);
			} else if (!local->pspolling && sdata->u.mgd.powersave) {
				local->pspolling = true;

				/*
				 * Here is assumed that the driver will be
				 * able to send ps-poll frame and receive a
				 * response even though power save mode is
				 * enabled, but some drivers might require
				 * to disable power save here. This needs
				 * to be investigated.
				 */
				ieee80211_send_pspoll(local, sdata);
			}
		}
	}

	if (sdata->vif.p2p) {
		struct ieee80211_p2p_noa_attr noa = {};
		int ret;

		ret = cfg80211_get_p2p_attr(mgmt->u.beacon.variable,
					    len - baselen,
					    IEEE80211_P2P_ATTR_ABSENCE_NOTICE,
					    (u8 *) &noa, sizeof(noa));
		if (ret >= 2) {
			if (sdata->u.mgd.p2p_noa_index != noa.index) {
				/* valid noa_attr and index changed */
				sdata->u.mgd.p2p_noa_index = noa.index;
				memcpy(&bss_conf->p2p_noa_attr, &noa, sizeof(noa));
				changed |= BSS_CHANGED_P2P_PS;
				/*
				 * make sure we update all information, the CRC
				 * mechanism doesn't look at P2P attributes.
				 */
				ifmgd->beacon_crc_valid = false;
			}
		} else if (sdata->u.mgd.p2p_noa_index != -1) {
			/* noa_attr not found and we had valid noa_attr before */
			sdata->u.mgd.p2p_noa_index = -1;
			memset(&bss_conf->p2p_noa_attr, 0, sizeof(bss_conf->p2p_noa_attr));
			changed |= BSS_CHANGED_P2P_PS;
			ifmgd->beacon_crc_valid = false;
		}
	}

	if (ifmgd->csa_waiting_bcn)
		ieee80211_chswitch_post_beacon(sdata);

	/*
	 * Update beacon timing and dtim count on every beacon appearance. This
	 * will allow the driver to use the most updated values. Do it before
	 * comparing this one with last received beacon.
	 * IMPORTANT: These parameters would possibly be out of sync by the time
	 * the driver will use them. The synchronized view is currently
	 * guaranteed only in certain callbacks.
	 */
	if (local->hw.flags & IEEE80211_HW_TIMING_BEACON_ONLY) {
		sdata->vif.bss_conf.sync_tsf =
			le64_to_cpu(mgmt->u.beacon.timestamp);
		sdata->vif.bss_conf.sync_device_ts =
			rx_status->device_timestamp;
		if (elems.tim)
			sdata->vif.bss_conf.sync_dtim_count =
				elems.tim->dtim_count;
		else
			sdata->vif.bss_conf.sync_dtim_count = 0;
	}

	if (ncrc == ifmgd->beacon_crc && ifmgd->beacon_crc_valid)
		return;
	ifmgd->beacon_crc = ncrc;
	ifmgd->beacon_crc_valid = true;

	ieee80211_rx_bss_info(sdata, mgmt, len, rx_status, &elems);

	ieee80211_sta_process_chanswitch(sdata, rx_status->mactime,
					 rx_status->device_timestamp,
					 &elems, true);

	if (!(ifmgd->flags & IEEE80211_STA_DISABLE_WMM) &&
	    ieee80211_sta_wmm_params(local, sdata, elems.wmm_param,
				     elems.wmm_param_len))
		changed |= BSS_CHANGED_QOS;

	/*
	 * If we haven't had a beacon before, tell the driver about the
	 * DTIM period (and beacon timing if desired) now.
	 */
	if (!ifmgd->have_beacon) {
		/* a few bogus AP send dtim_period = 0 or no TIM IE */
		if (elems.tim)
			bss_conf->dtim_period = elems.tim->dtim_period ?: 1;
		else
			bss_conf->dtim_period = 1;

		changed |= BSS_CHANGED_BEACON_INFO;
		ifmgd->have_beacon = true;

		mutex_lock(&local->iflist_mtx);
		ieee80211_recalc_ps(local, -1);
		mutex_unlock(&local->iflist_mtx);

		ieee80211_recalc_ps_vif(sdata);
	}

	if (elems.erp_info) {
		erp_valid = true;
		erp_value = elems.erp_info[0];
	} else {
		erp_valid = false;
	}
	changed |= ieee80211_handle_bss_capability(sdata,
			le16_to_cpu(mgmt->u.beacon.capab_info),
			erp_valid, erp_value);

	mutex_lock(&local->sta_mtx);
	sta = sta_info_get(sdata, bssid);

	if (ieee80211_config_bw(sdata, sta,
				elems.ht_cap_elem, elems.ht_operation,
				elems.vht_operation, bssid, &changed)) {
		mutex_unlock(&local->sta_mtx);
		ieee80211_set_disassoc(sdata, IEEE80211_STYPE_DEAUTH,
				       WLAN_REASON_DEAUTH_LEAVING,
				       true, deauth_buf);
		ieee80211_report_disconnect(sdata, deauth_buf,
					    sizeof(deauth_buf), true,
					    WLAN_REASON_DEAUTH_LEAVING);
		return;
	}

	if (sta && elems.opmode_notif)
		ieee80211_vht_handle_opmode(sdata, sta, *elems.opmode_notif,
					    rx_status->band, true);
	mutex_unlock(&local->sta_mtx);

	changed |= ieee80211_handle_pwr_constr(sdata, chan, mgmt,
					       elems.country_elem,
					       elems.country_elem_len,
					       elems.pwr_constr_elem,
					       elems.cisco_dtpc_elem);

	ieee80211_bss_info_change_notify(sdata, changed);
}

void ieee80211_sta_rx_queued_mgmt(struct ieee80211_sub_if_data *sdata,
				  struct sk_buff *skb)
{
	struct ieee80211_rx_status *rx_status;
	struct ieee80211_mgmt *mgmt;
	u16 fc;
	struct ieee802_11_elems elems;
	int ies_len;

	rx_status = (struct ieee80211_rx_status *) skb->cb;
	mgmt = (struct ieee80211_mgmt *) skb->data;
	fc = le16_to_cpu(mgmt->frame_control);

	sdata_lock(sdata);

	switch (fc & IEEE80211_FCTL_STYPE) {
	case IEEE80211_STYPE_BEACON:
		ieee80211_rx_mgmt_beacon(sdata, mgmt, skb->len, rx_status);
		break;
	case IEEE80211_STYPE_PROBE_RESP:
		ieee80211_rx_mgmt_probe_resp(sdata, skb);
		break;
	case IEEE80211_STYPE_AUTH:
		ieee80211_rx_mgmt_auth(sdata, mgmt, skb->len);
		break;
	case IEEE80211_STYPE_DEAUTH:
		ieee80211_rx_mgmt_deauth(sdata, mgmt, skb->len);
		break;
	case IEEE80211_STYPE_DISASSOC:
		ieee80211_rx_mgmt_disassoc(sdata, mgmt, skb->len);
		break;
	case IEEE80211_STYPE_ASSOC_RESP:
	case IEEE80211_STYPE_REASSOC_RESP:
		ieee80211_rx_mgmt_assoc_resp(sdata, mgmt, skb->len);
		break;
	case IEEE80211_STYPE_ACTION:
		if (mgmt->u.action.category == WLAN_CATEGORY_SPECTRUM_MGMT) {
			ies_len = skb->len -
				  offsetof(struct ieee80211_mgmt,
					   u.action.u.chan_switch.variable);

			if (ies_len < 0)
				break;

			ieee802_11_parse_elems(
				mgmt->u.action.u.chan_switch.variable,
				ies_len, true, &elems);

			if (elems.parse_error)
				break;

			ieee80211_sta_process_chanswitch(sdata,
						 rx_status->mactime,
						 rx_status->device_timestamp,
						 &elems, false);
		} else if (mgmt->u.action.category == WLAN_CATEGORY_PUBLIC) {
			ies_len = skb->len -
				  offsetof(struct ieee80211_mgmt,
					   u.action.u.ext_chan_switch.variable);

			if (ies_len < 0)
				break;

			ieee802_11_parse_elems(
				mgmt->u.action.u.ext_chan_switch.variable,
				ies_len, true, &elems);

			if (elems.parse_error)
				break;

			/* for the handling code pretend this was also an IE */
			elems.ext_chansw_ie =
				&mgmt->u.action.u.ext_chan_switch.data;

			ieee80211_sta_process_chanswitch(sdata,
						 rx_status->mactime,
						 rx_status->device_timestamp,
						 &elems, false);
		}
		break;
	}
	sdata_unlock(sdata);
}

static void ieee80211_sta_timer(unsigned long data)
{
	struct ieee80211_sub_if_data *sdata =
		(struct ieee80211_sub_if_data *) data;

	ieee80211_queue_work(&sdata->local->hw, &sdata->work);
}

static void ieee80211_sta_connection_lost(struct ieee80211_sub_if_data *sdata,
					  u8 *bssid, u8 reason, bool tx)
{
	u8 frame_buf[IEEE80211_DEAUTH_FRAME_LEN];

	ieee80211_set_disassoc(sdata, IEEE80211_STYPE_DEAUTH, reason,
			       tx, frame_buf);

	ieee80211_report_disconnect(sdata, frame_buf, sizeof(frame_buf), true,
				    reason);
}

static int ieee80211_probe_auth(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	struct ieee80211_mgd_auth_data *auth_data = ifmgd->auth_data;
	u32 tx_flags = 0;

	sdata_assert_lock(sdata);

	if (WARN_ON_ONCE(!auth_data))
		return -EINVAL;

	auth_data->tries++;

	if (auth_data->tries > IEEE80211_AUTH_MAX_TRIES) {
		sdata_info(sdata, "authentication with %pM timed out\n",
			   auth_data->bss->bssid);

		/*
		 * Most likely AP is not in the range so remove the
		 * bss struct for that AP.
		 */
		cfg80211_unlink_bss(local->hw.wiphy, auth_data->bss);

		return -ETIMEDOUT;
	}

	drv_mgd_prepare_tx(local, sdata);

	if (auth_data->bss->proberesp_ies) {
		u16 trans = 1;
		u16 status = 0;

		sdata_info(sdata, "send auth to %pM (try %d/%d)\n",
			   auth_data->bss->bssid, auth_data->tries,
			   IEEE80211_AUTH_MAX_TRIES);

		auth_data->expected_transaction = 2;

		if (auth_data->algorithm == WLAN_AUTH_SAE) {
			trans = auth_data->sae_trans;
			status = auth_data->sae_status;
			auth_data->expected_transaction = trans;
		}

		if (local->hw.flags & IEEE80211_HW_REPORTS_TX_ACK_STATUS)
			tx_flags = IEEE80211_TX_CTL_REQ_TX_STATUS |
				   IEEE80211_TX_INTFL_MLME_CONN_TX;

		ieee80211_send_auth(sdata, trans, auth_data->algorithm, status,
				    auth_data->data, auth_data->data_len,
				    auth_data->bss->bssid,
				    auth_data->bss->bssid, NULL, 0, 0,
				    tx_flags);
	} else {
		const u8 *ssidie;

		sdata_info(sdata, "direct probe to %pM (try %d/%i)\n",
			   auth_data->bss->bssid, auth_data->tries,
			   IEEE80211_AUTH_MAX_TRIES);

		rcu_read_lock();
		ssidie = ieee80211_bss_get_ie(auth_data->bss, WLAN_EID_SSID);
		if (!ssidie) {
			rcu_read_unlock();
			return -EINVAL;
		}
		/*
		 * Direct probe is sent to broadcast address as some APs
		 * will not answer to direct packet in unassociated state.
		 */
		ieee80211_send_probe_req(sdata, sdata->vif.addr, NULL,
					 ssidie + 2, ssidie[1],
					 NULL, 0, (u32) -1, true, 0,
					 auth_data->bss->channel, false);
		rcu_read_unlock();
	}

	if (tx_flags == 0) {
		auth_data->timeout = jiffies + IEEE80211_AUTH_TIMEOUT;
		auth_data->timeout_started = true;
		run_again(sdata, auth_data->timeout);
	} else {
		auth_data->timeout =
			round_jiffies_up(jiffies + IEEE80211_AUTH_TIMEOUT_LONG);
		auth_data->timeout_started = true;
		run_again(sdata, auth_data->timeout);
	}

	return 0;
}

static int ieee80211_do_assoc(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_mgd_assoc_data *assoc_data = sdata->u.mgd.assoc_data;
	struct ieee80211_local *local = sdata->local;

	sdata_assert_lock(sdata);

	assoc_data->tries++;
	if (assoc_data->tries > IEEE80211_ASSOC_MAX_TRIES) {
		sdata_info(sdata, "association with %pM timed out\n",
			   assoc_data->bss->bssid);

		/*
		 * Most likely AP is not in the range so remove the
		 * bss struct for that AP.
		 */
		cfg80211_unlink_bss(local->hw.wiphy, assoc_data->bss);

		return -ETIMEDOUT;
	}

	sdata_info(sdata, "associate with %pM (try %d/%d)\n",
		   assoc_data->bss->bssid, assoc_data->tries,
		   IEEE80211_ASSOC_MAX_TRIES);
	ieee80211_send_assoc(sdata);

	if (!(local->hw.flags & IEEE80211_HW_REPORTS_TX_ACK_STATUS)) {
		assoc_data->timeout = jiffies + IEEE80211_ASSOC_TIMEOUT;
		assoc_data->timeout_started = true;
		run_again(sdata, assoc_data->timeout);
	} else {
		assoc_data->timeout =
			round_jiffies_up(jiffies +
					 IEEE80211_ASSOC_TIMEOUT_LONG);
		assoc_data->timeout_started = true;
		run_again(sdata, assoc_data->timeout);
	}

	return 0;
}

void ieee80211_mgd_conn_tx_status(struct ieee80211_sub_if_data *sdata,
				  __le16 fc, bool acked)
{
	struct ieee80211_local *local = sdata->local;

	sdata->u.mgd.status_fc = fc;
	sdata->u.mgd.status_acked = acked;
	sdata->u.mgd.status_received = true;

	ieee80211_queue_work(&local->hw, &sdata->work);
}

void ieee80211_sta_work(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;

	sdata_lock(sdata);

	if (ifmgd->status_received) {
		__le16 fc = ifmgd->status_fc;
		bool status_acked = ifmgd->status_acked;

		ifmgd->status_received = false;
		if (ifmgd->auth_data &&
		    (ieee80211_is_probe_req(fc) || ieee80211_is_auth(fc))) {
			if (status_acked) {
				ifmgd->auth_data->timeout =
					jiffies + IEEE80211_AUTH_TIMEOUT_SHORT;
				run_again(sdata, ifmgd->auth_data->timeout);
			} else {
				ifmgd->auth_data->timeout = jiffies - 1;
			}
			ifmgd->auth_data->timeout_started = true;
		} else if (ifmgd->assoc_data &&
			   (ieee80211_is_assoc_req(fc) ||
			    ieee80211_is_reassoc_req(fc))) {
			if (status_acked) {
				ifmgd->assoc_data->timeout =
					jiffies + IEEE80211_ASSOC_TIMEOUT_SHORT;
				run_again(sdata, ifmgd->assoc_data->timeout);
			} else {
				ifmgd->assoc_data->timeout = jiffies - 1;
			}
			ifmgd->assoc_data->timeout_started = true;
		}
	}

	if (ifmgd->auth_data && ifmgd->auth_data->timeout_started &&
	    time_after(jiffies, ifmgd->auth_data->timeout)) {
		if (ifmgd->auth_data->done) {
			/*
			 * ok ... we waited for assoc but userspace didn't,
			 * so let's just kill the auth data
			 */
			ieee80211_destroy_auth_data(sdata, false);
		} else if (ieee80211_probe_auth(sdata)) {
			u8 bssid[ETH_ALEN];
			struct ieee80211_event event = {
				.type = MLME_EVENT,
				.u.mlme.data = AUTH_EVENT,
				.u.mlme.status = MLME_TIMEOUT,
			};

			memcpy(bssid, ifmgd->auth_data->bss->bssid, ETH_ALEN);

			ieee80211_destroy_auth_data(sdata, false);

			cfg80211_auth_timeout(sdata->dev, bssid);
			drv_event_callback(sdata->local, sdata, &event);
		}
	} else if (ifmgd->auth_data && ifmgd->auth_data->timeout_started)
		run_again(sdata, ifmgd->auth_data->timeout);

	if (ifmgd->assoc_data && ifmgd->assoc_data->timeout_started &&
	    time_after(jiffies, ifmgd->assoc_data->timeout)) {
		if ((ifmgd->assoc_data->need_beacon && !ifmgd->have_beacon) ||
		    ieee80211_do_assoc(sdata)) {
			struct cfg80211_bss *bss = ifmgd->assoc_data->bss;
			struct ieee80211_event event = {
				.type = MLME_EVENT,
				.u.mlme.data = ASSOC_EVENT,
				.u.mlme.status = MLME_TIMEOUT,
			};

			ieee80211_destroy_assoc_data(sdata, false);
			cfg80211_assoc_timeout(sdata->dev, bss);
			drv_event_callback(sdata->local, sdata, &event);
		}
	} else if (ifmgd->assoc_data && ifmgd->assoc_data->timeout_started)
		run_again(sdata, ifmgd->assoc_data->timeout);

	if (ifmgd->flags & IEEE80211_STA_CONNECTION_POLL &&
	    ifmgd->associated) {
		u8 bssid[ETH_ALEN];
		int max_tries;

		memcpy(bssid, ifmgd->associated->bssid, ETH_ALEN);

		if (local->hw.flags & IEEE80211_HW_REPORTS_TX_ACK_STATUS)
			max_tries = max_nullfunc_tries;
		else
			max_tries = max_probe_tries;

		/* ACK received for nullfunc probing frame */
		if (!ifmgd->probe_send_count)
			ieee80211_reset_ap_probe(sdata);
		else if (ifmgd->nullfunc_failed) {
			if (ifmgd->probe_send_count < max_tries) {
				mlme_dbg(sdata,
					 "No ack for nullfunc frame to AP %pM, try %d/%i\n",
					 bssid, ifmgd->probe_send_count,
					 max_tries);
				ieee80211_mgd_probe_ap_send(sdata);
			} else {
				mlme_dbg(sdata,
					 "No ack for nullfunc frame to AP %pM, disconnecting.\n",
					 bssid);
				ieee80211_sta_connection_lost(sdata, bssid,
					WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY,
					false);
			}
		} else if (time_is_after_jiffies(ifmgd->probe_timeout))
			run_again(sdata, ifmgd->probe_timeout);
		else if (local->hw.flags & IEEE80211_HW_REPORTS_TX_ACK_STATUS) {
			mlme_dbg(sdata,
				 "Failed to send nullfunc to AP %pM after %dms, disconnecting\n",
				 bssid, probe_wait_ms);
			ieee80211_sta_connection_lost(sdata, bssid,
				WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY, false);
		} else if (ifmgd->probe_send_count < max_tries) {
			mlme_dbg(sdata,
				 "No probe response from AP %pM after %dms, try %d/%i\n",
				 bssid, probe_wait_ms,
				 ifmgd->probe_send_count, max_tries);
			ieee80211_mgd_probe_ap_send(sdata);
		} else {
			/*
			 * We actually lost the connection ... or did we?
			 * Let's make sure!
			 */
			wiphy_debug(local->hw.wiphy,
				    "%s: No probe response from AP %pM"
				    " after %dms, disconnecting.\n",
				    sdata->name,
				    bssid, probe_wait_ms);

			ieee80211_sta_connection_lost(sdata, bssid,
				WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY, false);
		}
	}

	sdata_unlock(sdata);
}

static void ieee80211_sta_bcn_mon_timer(unsigned long data)
{
	struct ieee80211_sub_if_data *sdata =
		(struct ieee80211_sub_if_data *) data;
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;

	if (sdata->vif.csa_active && !ifmgd->csa_waiting_bcn)
		return;

	sdata->u.mgd.connection_loss = false;
	ieee80211_queue_work(&sdata->local->hw,
			     &sdata->u.mgd.beacon_connection_loss_work);
}

static void ieee80211_sta_conn_mon_timer(unsigned long data)
{
	struct ieee80211_sub_if_data *sdata =
		(struct ieee80211_sub_if_data *) data;
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	struct ieee80211_local *local = sdata->local;

	if (sdata->vif.csa_active && !ifmgd->csa_waiting_bcn)
		return;

	ieee80211_queue_work(&local->hw, &ifmgd->monitor_work);
}

static void ieee80211_sta_monitor_work(struct work_struct *work)
{
	struct ieee80211_sub_if_data *sdata =
		container_of(work, struct ieee80211_sub_if_data,
			     u.mgd.monitor_work);

	ieee80211_mgd_probe_ap(sdata, false);
}

static void ieee80211_restart_sta_timer(struct ieee80211_sub_if_data *sdata)
{
	u32 flags;

	if (sdata->vif.type == NL80211_IFTYPE_STATION) {
		__ieee80211_stop_poll(sdata);

		/* let's probe the connection once */
		flags = sdata->local->hw.flags;
		if (!(flags & IEEE80211_HW_CONNECTION_MONITOR))
			ieee80211_queue_work(&sdata->local->hw,
					     &sdata->u.mgd.monitor_work);
	}
}

#ifdef CONFIG_PM
void ieee80211_mgd_quiesce(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	u8 frame_buf[IEEE80211_DEAUTH_FRAME_LEN];

	sdata_lock(sdata);

	if (ifmgd->auth_data || ifmgd->assoc_data) {
		const u8 *bssid = ifmgd->auth_data ?
				ifmgd->auth_data->bss->bssid :
				ifmgd->assoc_data->bss->bssid;

		/*
		 * If we are trying to authenticate / associate while suspending,
		 * cfg80211 won't know and won't actually abort those attempts,
		 * thus we need to do that ourselves.
		 */
		ieee80211_send_deauth_disassoc(sdata, bssid,
					       IEEE80211_STYPE_DEAUTH,
					       WLAN_REASON_DEAUTH_LEAVING,
					       false, frame_buf);
		if (ifmgd->assoc_data)
			ieee80211_destroy_assoc_data(sdata, false);
		if (ifmgd->auth_data)
			ieee80211_destroy_auth_data(sdata, false);
		cfg80211_tx_mlme_mgmt(sdata->dev, frame_buf,
				      IEEE80211_DEAUTH_FRAME_LEN);
	}

	/* This is a bit of a hack - we should find a better and more generic
	 * solution to this. Normally when suspending, cfg80211 will in fact
	 * deauthenticate. However, it doesn't (and cannot) stop an ongoing
	 * auth (not so important) or assoc (this is the problem) process.
	 *
	 * As a consequence, it can happen that we are in the process of both
	 * associating and suspending, and receive an association response
	 * after cfg80211 has checked if it needs to disconnect, but before
	 * we actually set the flag to drop incoming frames. This will then
	 * cause the workqueue flush to process the association response in
	 * the suspend, resulting in a successful association just before it
	 * tries to remove the interface from the driver, which now though
	 * has a channel context assigned ... this results in issues.
	 *
	 * To work around this (for now) simply deauth here again if we're
	 * now connected.
	 */
	if (ifmgd->associated && !sdata->local->wowlan) {
		u8 bssid[ETH_ALEN];
		struct cfg80211_deauth_request req = {
			.reason_code = WLAN_REASON_DEAUTH_LEAVING,
			.bssid = bssid,
		};

		memcpy(bssid, ifmgd->associated->bssid, ETH_ALEN);
		ieee80211_mgd_deauth(sdata, &req);
	}

	sdata_unlock(sdata);
}

void ieee80211_sta_restart(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;

	sdata_lock(sdata);
	if (!ifmgd->associated) {
		sdata_unlock(sdata);
		return;
	}

	if (sdata->flags & IEEE80211_SDATA_DISCONNECT_RESUME) {
		sdata->flags &= ~IEEE80211_SDATA_DISCONNECT_RESUME;
		mlme_dbg(sdata, "driver requested disconnect after resume\n");
		ieee80211_sta_connection_lost(sdata,
					      ifmgd->associated->bssid,
					      WLAN_REASON_UNSPECIFIED,
					      true);
		sdata_unlock(sdata);
		return;
	}
	sdata_unlock(sdata);
}
#endif

/* interface setup */
void ieee80211_sta_setup_sdata(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_managed *ifmgd;

	ifmgd = &sdata->u.mgd;
	INIT_WORK(&ifmgd->monitor_work, ieee80211_sta_monitor_work);
	INIT_WORK(&ifmgd->chswitch_work, ieee80211_chswitch_work);
	INIT_WORK(&ifmgd->beacon_connection_loss_work,
		  ieee80211_beacon_connection_loss_work);
	INIT_WORK(&ifmgd->csa_connection_drop_work,
		  ieee80211_csa_connection_drop_work);
	INIT_WORK(&ifmgd->request_smps_work, ieee80211_request_smps_mgd_work);
	INIT_DELAYED_WORK(&ifmgd->tdls_peer_del_work,
			  ieee80211_tdls_peer_del_work);
	setup_timer(&ifmgd->timer, ieee80211_sta_timer,
		    (unsigned long) sdata);
	setup_timer(&ifmgd->bcn_mon_timer, ieee80211_sta_bcn_mon_timer,
		    (unsigned long) sdata);
	setup_timer(&ifmgd->conn_mon_timer, ieee80211_sta_conn_mon_timer,
		    (unsigned long) sdata);
	setup_timer(&ifmgd->chswitch_timer, ieee80211_chswitch_timer,
		    (unsigned long) sdata);
	INIT_DELAYED_WORK(&ifmgd->tx_tspec_wk,
			  ieee80211_sta_handle_tspec_ac_params_wk);

	ifmgd->flags = 0;
	ifmgd->powersave = sdata->wdev.ps;
	ifmgd->uapsd_queues = sdata->local->hw.uapsd_queues;
	ifmgd->uapsd_max_sp_len = sdata->local->hw.uapsd_max_sp_len;
	ifmgd->p2p_noa_index = -1;

	if (sdata->local->hw.wiphy->features & NL80211_FEATURE_DYNAMIC_SMPS)
		ifmgd->req_smps = IEEE80211_SMPS_AUTOMATIC;
	else
		ifmgd->req_smps = IEEE80211_SMPS_OFF;

	/* Setup TDLS data */
	spin_lock_init(&ifmgd->teardown_lock);
	ifmgd->teardown_skb = NULL;
	ifmgd->orig_teardown_skb = NULL;
}

/* scan finished notification */
void ieee80211_mlme_notify_scan_completed(struct ieee80211_local *local)
{
	struct ieee80211_sub_if_data *sdata;

	/* Restart STA timers */
	rcu_read_lock();
	list_for_each_entry_rcu(sdata, &local->interfaces, list) {
		if (ieee80211_sdata_running(sdata))
			ieee80211_restart_sta_timer(sdata);
	}
	rcu_read_unlock();
}

int ieee80211_max_network_latency(struct notifier_block *nb,
				  unsigned long data, void *dummy)
{
	s32 latency_usec = (s32) data;
	struct ieee80211_local *local =
		container_of(nb, struct ieee80211_local,
			     network_latency_notifier);

	mutex_lock(&local->iflist_mtx);
	ieee80211_recalc_ps(local, latency_usec);
	mutex_unlock(&local->iflist_mtx);

	return NOTIFY_OK;
}

static u8 ieee80211_ht_vht_rx_chains(struct ieee80211_sub_if_data *sdata,
				     struct cfg80211_bss *cbss)
{
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	const u8 *ht_cap_ie, *vht_cap_ie;
	const struct ieee80211_ht_cap *ht_cap;
	const struct ieee80211_vht_cap *vht_cap;
	u8 chains = 1;

	if (ifmgd->flags & IEEE80211_STA_DISABLE_HT)
		return chains;

	ht_cap_ie = ieee80211_bss_get_ie(cbss, WLAN_EID_HT_CAPABILITY);
	if (ht_cap_ie && ht_cap_ie[1] >= sizeof(*ht_cap)) {
		ht_cap = (void *)(ht_cap_ie + 2);
		chains = ieee80211_mcs_to_chains(&ht_cap->mcs);
		/*
		 * TODO: use "Tx Maximum Number Spatial Streams Supported" and
		 *	 "Tx Unequal Modulation Supported" fields.
		 */
	}

	if (ifmgd->flags & IEEE80211_STA_DISABLE_VHT)
		return chains;

	vht_cap_ie = ieee80211_bss_get_ie(cbss, WLAN_EID_VHT_CAPABILITY);
	if (vht_cap_ie && vht_cap_ie[1] >= sizeof(*vht_cap)) {
		u8 nss;
		u16 tx_mcs_map;

		vht_cap = (void *)(vht_cap_ie + 2);
		tx_mcs_map = le16_to_cpu(vht_cap->supp_mcs.tx_mcs_map);
		for (nss = 8; nss > 0; nss--) {
			if (((tx_mcs_map >> (2 * (nss - 1))) & 3) !=
					IEEE80211_VHT_MCS_NOT_SUPPORTED)
				break;
		}
		/* TODO: use "Tx Highest Supported Long GI Data Rate" field? */
		chains = max(chains, nss);
	}

	return chains;
}

static int ieee80211_prep_channel(struct ieee80211_sub_if_data *sdata,
				  struct cfg80211_bss *cbss)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	const struct ieee80211_ht_cap *ht_cap = NULL;
	const struct ieee80211_ht_operation *ht_oper = NULL;
	const struct ieee80211_vht_operation *vht_oper = NULL;
	struct ieee80211_supported_band *sband;
	struct cfg80211_chan_def chandef;
	int ret;

	sband = local->hw.wiphy->bands[cbss->channel->band];

	ifmgd->flags &= ~(IEEE80211_STA_DISABLE_40MHZ |
			  IEEE80211_STA_DISABLE_80P80MHZ |
			  IEEE80211_STA_DISABLE_160MHZ);

	rcu_read_lock();

	if (!(ifmgd->flags & IEEE80211_STA_DISABLE_HT) &&
	    sband->ht_cap.ht_supported) {
		const u8 *ht_oper_ie, *ht_cap_ie;

		ht_oper_ie = ieee80211_bss_get_ie(cbss, WLAN_EID_HT_OPERATION);
		if (ht_oper_ie && ht_oper_ie[1] >= sizeof(*ht_oper))
			ht_oper = (void *)(ht_oper_ie + 2);

		ht_cap_ie = ieee80211_bss_get_ie(cbss, WLAN_EID_HT_CAPABILITY);
		if (ht_cap_ie && ht_cap_ie[1] >= sizeof(*ht_cap))
			ht_cap = (void *)(ht_cap_ie + 2);

		if (!ht_cap) {
			ifmgd->flags |= IEEE80211_STA_DISABLE_HT;
			ht_oper = NULL;
		}
	}

	if (!(ifmgd->flags & IEEE80211_STA_DISABLE_VHT) &&
	    sband->vht_cap.vht_supported) {
		const u8 *vht_oper_ie, *vht_cap;

		vht_oper_ie = ieee80211_bss_get_ie(cbss,
						   WLAN_EID_VHT_OPERATION);
		if (vht_oper_ie && vht_oper_ie[1] >= sizeof(*vht_oper))
			vht_oper = (void *)(vht_oper_ie + 2);
		if (vht_oper && !ht_oper) {
			vht_oper = NULL;
			sdata_info(sdata,
				   "AP advertised VHT without HT, disabling both\n");
			ifmgd->flags |= IEEE80211_STA_DISABLE_HT;
			ifmgd->flags |= IEEE80211_STA_DISABLE_VHT;
		}

		vht_cap = ieee80211_bss_get_ie(cbss, WLAN_EID_VHT_CAPABILITY);
		if (!vht_cap || vht_cap[1] < sizeof(struct ieee80211_vht_cap)) {
			ifmgd->flags |= IEEE80211_STA_DISABLE_VHT;
			vht_oper = NULL;
		}
	}

	ifmgd->flags |= ieee80211_determine_chantype(sdata, sband,
						     cbss->channel,
						     ht_cap, ht_oper, vht_oper,
						     &chandef, false);

	sdata->needed_rx_chains = min(ieee80211_ht_vht_rx_chains(sdata, cbss),
				      local->rx_chains);

	rcu_read_unlock();

	/* will change later if needed */
	sdata->smps_mode = IEEE80211_SMPS_OFF;

	mutex_lock(&local->mtx);
	/*
	 * If this fails (possibly due to channel context sharing
	 * on incompatible channels, e.g. 80+80 and 160 sharing the
	 * same control channel) try to use a smaller bandwidth.
	 */
	ret = ieee80211_vif_use_channel(sdata, &chandef,
					IEEE80211_CHANCTX_SHARED);

	/* don't downgrade for 5 and 10 MHz channels, though. */
	if (chandef.width == NL80211_CHAN_WIDTH_5 ||
	    chandef.width == NL80211_CHAN_WIDTH_10)
		goto out;

	while (ret && chandef.width != NL80211_CHAN_WIDTH_20_NOHT) {
		ifmgd->flags |= ieee80211_chandef_downgrade(&chandef);
		ret = ieee80211_vif_use_channel(sdata, &chandef,
						IEEE80211_CHANCTX_SHARED);
	}
 out:
	mutex_unlock(&local->mtx);
	return ret;
}

static int ieee80211_prep_connection(struct ieee80211_sub_if_data *sdata,
				     struct cfg80211_bss *cbss, bool assoc)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	struct ieee80211_bss *bss = (void *)cbss->priv;
	struct sta_info *new_sta = NULL;
	struct ieee80211_supported_band *sband;
	struct ieee80211_sta_ht_cap sta_ht_cap;
	bool have_sta = false, is_override = false;
	int err;

	sband = local->hw.wiphy->bands[cbss->channel->band];

	if (WARN_ON(!ifmgd->auth_data && !ifmgd->assoc_data))
		return -EINVAL;

	/* If a reconfig is happening, bail out */
	if (local->in_reconfig)
		return -EBUSY;

	if (assoc) {
		rcu_read_lock();
		have_sta = sta_info_get(sdata, cbss->bssid);
		rcu_read_unlock();
	}

	if (!have_sta) {
		new_sta = sta_info_alloc(sdata, cbss->bssid, GFP_KERNEL);
		if (!new_sta)
			return -ENOMEM;
	}

	memcpy(&sta_ht_cap, &sband->ht_cap, sizeof(sta_ht_cap));
	ieee80211_apply_htcap_overrides(sdata, &sta_ht_cap);

	is_override = (sta_ht_cap.cap & IEEE80211_HT_CAP_SUP_WIDTH_20_40) !=
		      (sband->ht_cap.cap &
		       IEEE80211_HT_CAP_SUP_WIDTH_20_40);

	if (new_sta || is_override) {
		err = ieee80211_prep_channel(sdata, cbss);
		if (err) {
			if (new_sta)
				sta_info_free(local, new_sta);
			return -EINVAL;
		}
	}

	if (new_sta) {
		u32 rates = 0, basic_rates = 0;
		bool have_higher_than_11mbit;
		int min_rate = INT_MAX, min_rate_index = -1;
		struct ieee80211_chanctx_conf *chanctx_conf;
		const struct cfg80211_bss_ies *ies;
		int shift = ieee80211_vif_get_shift(&sdata->vif);
		u32 rate_flags;

		rcu_read_lock();
		chanctx_conf = rcu_dereference(sdata->vif.chanctx_conf);
		if (WARN_ON(!chanctx_conf)) {
			rcu_read_unlock();
			sta_info_free(local, new_sta);
			return -EINVAL;
		}
		rate_flags = ieee80211_chandef_rate_flags(&chanctx_conf->def);
		rcu_read_unlock();

		ieee80211_get_rates(sband, bss->supp_rates,
				    bss->supp_rates_len,
				    &rates, &basic_rates,
				    &have_higher_than_11mbit,
				    &min_rate, &min_rate_index,
				    shift, rate_flags);

		/*
		 * This used to be a workaround for basic rates missing
		 * in the association response frame. Now that we no
		 * longer use the basic rates from there, it probably
		 * doesn't happen any more, but keep the workaround so
		 * in case some *other* APs are buggy in different ways
		 * we can connect -- with a warning.
		 */
		if (!basic_rates && min_rate_index >= 0) {
			sdata_info(sdata,
				   "No basic rates, using min rate instead\n");
			basic_rates = BIT(min_rate_index);
		}

		new_sta->sta.supp_rates[cbss->channel->band] = rates;
		sdata->vif.bss_conf.basic_rates = basic_rates;

		/* cf. IEEE 802.11 9.2.12 */
		if (cbss->channel->band == IEEE80211_BAND_2GHZ &&
		    have_higher_than_11mbit)
			sdata->flags |= IEEE80211_SDATA_OPERATING_GMODE;
		else
			sdata->flags &= ~IEEE80211_SDATA_OPERATING_GMODE;

		memcpy(ifmgd->bssid, cbss->bssid, ETH_ALEN);

		/* set timing information */
		sdata->vif.bss_conf.beacon_int = cbss->beacon_interval;
		rcu_read_lock();
		ies = rcu_dereference(cbss->beacon_ies);
		if (ies) {
			const u8 *tim_ie;

			sdata->vif.bss_conf.sync_tsf = ies->tsf;
			sdata->vif.bss_conf.sync_device_ts =
				bss->device_ts_beacon;
			tim_ie = cfg80211_find_ie(WLAN_EID_TIM,
						  ies->data, ies->len);
			if (tim_ie && tim_ie[1] >= 2)
				sdata->vif.bss_conf.sync_dtim_count = tim_ie[2];
			else
				sdata->vif.bss_conf.sync_dtim_count = 0;
		} else if (!(local->hw.flags &
					IEEE80211_HW_TIMING_BEACON_ONLY)) {
			ies = rcu_dereference(cbss->proberesp_ies);
			/* must be non-NULL since beacon IEs were NULL */
			sdata->vif.bss_conf.sync_tsf = ies->tsf;
			sdata->vif.bss_conf.sync_device_ts =
				bss->device_ts_presp;
			sdata->vif.bss_conf.sync_dtim_count = 0;
		} else {
			sdata->vif.bss_conf.sync_tsf = 0;
			sdata->vif.bss_conf.sync_device_ts = 0;
			sdata->vif.bss_conf.sync_dtim_count = 0;
		}
		rcu_read_unlock();

		/* tell driver about BSSID, basic rates and timing */
		ieee80211_bss_info_change_notify(sdata,
			BSS_CHANGED_BSSID | BSS_CHANGED_BASIC_RATES |
			BSS_CHANGED_BEACON_INT);

		if (assoc)
			sta_info_pre_move_state(new_sta, IEEE80211_STA_AUTH);

		err = sta_info_insert(new_sta);
		new_sta = NULL;
		if (err) {
			sdata_info(sdata,
				   "failed to insert STA entry for the AP (error %d)\n",
				   err);
			return err;
		}
	} else
		WARN_ON_ONCE(!ether_addr_equal(ifmgd->bssid, cbss->bssid));

	/* Cancel scan to ensure that nothing interferes with connection */
	if (local->scanning)
		ieee80211_scan_cancel(local);

	return 0;
}

/* config hooks */
int ieee80211_mgd_auth(struct ieee80211_sub_if_data *sdata,
		       struct cfg80211_auth_request *req)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	struct ieee80211_mgd_auth_data *auth_data;
	u16 auth_alg;
	int err;

	/* prepare auth data structure */

	switch (req->auth_type) {
	case NL80211_AUTHTYPE_OPEN_SYSTEM:
		auth_alg = WLAN_AUTH_OPEN;
		break;
	case NL80211_AUTHTYPE_SHARED_KEY:
		if (IS_ERR(local->wep_tx_tfm))
			return -EOPNOTSUPP;
		auth_alg = WLAN_AUTH_SHARED_KEY;
		break;
	case NL80211_AUTHTYPE_FT:
		auth_alg = WLAN_AUTH_FT;
		break;
	case NL80211_AUTHTYPE_NETWORK_EAP:
		auth_alg = WLAN_AUTH_LEAP;
		break;
	case NL80211_AUTHTYPE_SAE:
		auth_alg = WLAN_AUTH_SAE;
		break;
	default:
		return -EOPNOTSUPP;
	}

	auth_data = kzalloc(sizeof(*auth_data) + req->sae_data_len +
			    req->ie_len, GFP_KERNEL);
	if (!auth_data)
		return -ENOMEM;

	auth_data->bss = req->bss;

	if (req->sae_data_len >= 4) {
		__le16 *pos = (__le16 *) req->sae_data;
		auth_data->sae_trans = le16_to_cpu(pos[0]);
		auth_data->sae_status = le16_to_cpu(pos[1]);
		memcpy(auth_data->data, req->sae_data + 4,
		       req->sae_data_len - 4);
		auth_data->data_len += req->sae_data_len - 4;
	}

	if (req->ie && req->ie_len) {
		memcpy(&auth_data->data[auth_data->data_len],
		       req->ie, req->ie_len);
		auth_data->data_len += req->ie_len;
	}

	if (req->key && req->key_len) {
		auth_data->key_len = req->key_len;
		auth_data->key_idx = req->key_idx;
		memcpy(auth_data->key, req->key, req->key_len);
	}

	auth_data->algorithm = auth_alg;

	/* try to authenticate/probe */

	if ((ifmgd->auth_data && !ifmgd->auth_data->done) ||
	    ifmgd->assoc_data) {
		err = -EBUSY;
		goto err_free;
	}

	if (ifmgd->auth_data)
		ieee80211_destroy_auth_data(sdata, false);

	/* prep auth_data so we don't go into idle on disassoc */
	ifmgd->auth_data = auth_data;

	if (ifmgd->associated) {
		u8 frame_buf[IEEE80211_DEAUTH_FRAME_LEN];

		ieee80211_set_disassoc(sdata, IEEE80211_STYPE_DEAUTH,
				       WLAN_REASON_UNSPECIFIED,
				       false, frame_buf);

		ieee80211_report_disconnect(sdata, frame_buf,
					    sizeof(frame_buf), true,
					    WLAN_REASON_UNSPECIFIED);
	}

	sdata_info(sdata, "authenticate with %pM\n", req->bss->bssid);

	err = ieee80211_prep_connection(sdata, req->bss, false);
	if (err)
		goto err_clear;

	err = ieee80211_probe_auth(sdata);
	if (err) {
		sta_info_destroy_addr(sdata, req->bss->bssid);
		goto err_clear;
	}

	/* hold our own reference */
	cfg80211_ref_bss(local->hw.wiphy, auth_data->bss);
	return 0;

 err_clear:
	eth_zero_addr(ifmgd->bssid);
	ieee80211_bss_info_change_notify(sdata, BSS_CHANGED_BSSID);
	ifmgd->auth_data = NULL;
 err_free:
	kfree(auth_data);
	return err;
}

static bool ieee80211_usable_wmm_params(struct ieee80211_sub_if_data *sdata,
					const u8 *wmm_param, int len)
{
	const u8 *pos;
	size_t left;

	if (len < 8)
		return false;

	if (wmm_param[5] != 1 /* version */)
		return false;

	pos = wmm_param + 8;
	left = len - 8;

	for (; left >= 4; left -= 4, pos += 4) {
		u8 aifsn = pos[0] & 0x0f;
		u8 ecwmin = pos[1] & 0x0f;
		u8 ecwmax = (pos[1] & 0xf0) >> 4;
		int aci = (pos[0] >> 5) & 0x03;

		if (aifsn < 2) {
			sdata_info(sdata,
				   "AP has invalid WMM params (AIFSN=%d for ACI %d), disabling WMM\n",
				   aifsn, aci);
			return false;
		}
		if (ecwmin > ecwmax) {
			sdata_info(sdata,
				   "AP has invalid WMM params (ECWmin/max=%d/%d for ACI %d), disabling WMM\n",
				   ecwmin, ecwmax, aci);
			return false;
		}
	}

	return true;
}

int ieee80211_mgd_assoc(struct ieee80211_sub_if_data *sdata,
			struct cfg80211_assoc_request *req)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	struct ieee80211_bss *bss = (void *)req->bss->priv;
	struct ieee80211_mgd_assoc_data *assoc_data;
	const struct cfg80211_bss_ies *beacon_ies;
	struct ieee80211_supported_band *sband;
	const u8 *ssidie, *ht_ie, *vht_ie;
	int i, err;

	assoc_data = kzalloc(sizeof(*assoc_data) + req->ie_len, GFP_KERNEL);
	if (!assoc_data)
		return -ENOMEM;

	rcu_read_lock();
	ssidie = ieee80211_bss_get_ie(req->bss, WLAN_EID_SSID);
	if (!ssidie) {
		rcu_read_unlock();
		kfree(assoc_data);
		return -EINVAL;
	}
	memcpy(assoc_data->ssid, ssidie + 2, ssidie[1]);
	assoc_data->ssid_len = ssidie[1];
	rcu_read_unlock();

	if (ifmgd->associated) {
		u8 frame_buf[IEEE80211_DEAUTH_FRAME_LEN];

		ieee80211_set_disassoc(sdata, IEEE80211_STYPE_DEAUTH,
				       WLAN_REASON_UNSPECIFIED,
				       false, frame_buf);

		ieee80211_report_disconnect(sdata, frame_buf,
					    sizeof(frame_buf), true,
					    WLAN_REASON_UNSPECIFIED);
	}

	if (ifmgd->auth_data && !ifmgd->auth_data->done) {
		err = -EBUSY;
		goto err_free;
	}

	if (ifmgd->assoc_data) {
		err = -EBUSY;
		goto err_free;
	}

	if (ifmgd->auth_data) {
		bool match;

		/* keep sta info, bssid if matching */
		match = ether_addr_equal(ifmgd->bssid, req->bss->bssid);
		ieee80211_destroy_auth_data(sdata, match);
	}

	/* prepare assoc data */

	ifmgd->beacon_crc_valid = false;

	assoc_data->wmm = bss->wmm_used &&
			  (local->hw.queues >= IEEE80211_NUM_ACS);
	if (assoc_data->wmm) {
		/* try to check validity of WMM params IE */
		const struct cfg80211_bss_ies *ies;
		const u8 *wp, *start, *end;

		rcu_read_lock();
		ies = rcu_dereference(req->bss->ies);
		start = ies->data;
		end = start + ies->len;

		while (true) {
			wp = cfg80211_find_vendor_ie(
				WLAN_OUI_MICROSOFT,
				WLAN_OUI_TYPE_MICROSOFT_WMM,
				start, end - start);
			if (!wp)
				break;
			start = wp + wp[1] + 2;
			/* if this IE is too short, try the next */
			if (wp[1] <= 4)
				continue;
			/* if this IE is WMM params, we found what we wanted */
			if (wp[6] == 1)
				break;
		}

		if (!wp || !ieee80211_usable_wmm_params(sdata, wp + 2,
							wp[1] - 2)) {
			assoc_data->wmm = false;
			ifmgd->flags |= IEEE80211_STA_DISABLE_WMM;
		}
		rcu_read_unlock();
	}

	/*
	 * IEEE802.11n does not allow TKIP/WEP as pairwise ciphers in HT mode.
	 * We still associate in non-HT mode (11a/b/g) if any one of these
	 * ciphers is configured as pairwise.
	 * We can set this to true for non-11n hardware, that'll be checked
	 * separately along with the peer capabilities.
	 */
	for (i = 0; i < req->crypto.n_ciphers_pairwise; i++) {
		if (req->crypto.ciphers_pairwise[i] == WLAN_CIPHER_SUITE_WEP40 ||
		    req->crypto.ciphers_pairwise[i] == WLAN_CIPHER_SUITE_TKIP ||
		    req->crypto.ciphers_pairwise[i] == WLAN_CIPHER_SUITE_WEP104) {
			ifmgd->flags |= IEEE80211_STA_DISABLE_HT;
			ifmgd->flags |= IEEE80211_STA_DISABLE_VHT;
			netdev_info(sdata->dev,
				    "disabling HT/VHT due to WEP/TKIP use\n");
		}
	}

	if (req->flags & ASSOC_REQ_DISABLE_HT) {
		ifmgd->flags |= IEEE80211_STA_DISABLE_HT;
		ifmgd->flags |= IEEE80211_STA_DISABLE_VHT;
	}

	if (req->flags & ASSOC_REQ_DISABLE_VHT)
		ifmgd->flags |= IEEE80211_STA_DISABLE_VHT;

	/* Also disable HT if we don't support it or the AP doesn't use WMM */
	sband = local->hw.wiphy->bands[req->bss->channel->band];
	if (!sband->ht_cap.ht_supported ||
	    local->hw.queues < IEEE80211_NUM_ACS || !bss->wmm_used ||
	    ifmgd->flags & IEEE80211_STA_DISABLE_WMM) {
		ifmgd->flags |= IEEE80211_STA_DISABLE_HT;
		if (!bss->wmm_used &&
		    !(ifmgd->flags & IEEE80211_STA_DISABLE_WMM))
			netdev_info(sdata->dev,
				    "disabling HT as WMM/QoS is not supported by the AP\n");
	}

	/* disable VHT if we don't support it or the AP doesn't use WMM */
	if (!sband->vht_cap.vht_supported ||
	    local->hw.queues < IEEE80211_NUM_ACS || !bss->wmm_used ||
	    ifmgd->flags & IEEE80211_STA_DISABLE_WMM) {
		ifmgd->flags |= IEEE80211_STA_DISABLE_VHT;
		if (!bss->wmm_used &&
		    !(ifmgd->flags & IEEE80211_STA_DISABLE_WMM))
			netdev_info(sdata->dev,
				    "disabling VHT as WMM/QoS is not supported by the AP\n");
	}

	memcpy(&ifmgd->ht_capa, &req->ht_capa, sizeof(ifmgd->ht_capa));
	memcpy(&ifmgd->ht_capa_mask, &req->ht_capa_mask,
	       sizeof(ifmgd->ht_capa_mask));

	memcpy(&ifmgd->vht_capa, &req->vht_capa, sizeof(ifmgd->vht_capa));
	memcpy(&ifmgd->vht_capa_mask, &req->vht_capa_mask,
	       sizeof(ifmgd->vht_capa_mask));

	if (req->ie && req->ie_len) {
		memcpy(assoc_data->ie, req->ie, req->ie_len);
		assoc_data->ie_len = req->ie_len;
	}

	assoc_data->bss = req->bss;

	if (ifmgd->req_smps == IEEE80211_SMPS_AUTOMATIC) {
		if (ifmgd->powersave)
			sdata->smps_mode = IEEE80211_SMPS_DYNAMIC;
		else
			sdata->smps_mode = IEEE80211_SMPS_OFF;
	} else
		sdata->smps_mode = ifmgd->req_smps;

	assoc_data->capability = req->bss->capability;
	assoc_data->supp_rates = bss->supp_rates;
	assoc_data->supp_rates_len = bss->supp_rates_len;

	rcu_read_lock();
	ht_ie = ieee80211_bss_get_ie(req->bss, WLAN_EID_HT_OPERATION);
	if (ht_ie && ht_ie[1] >= sizeof(struct ieee80211_ht_operation))
		assoc_data->ap_ht_param =
			((struct ieee80211_ht_operation *)(ht_ie + 2))->ht_param;
	else
		ifmgd->flags |= IEEE80211_STA_DISABLE_HT;
	vht_ie = ieee80211_bss_get_ie(req->bss, WLAN_EID_VHT_CAPABILITY);
	if (vht_ie && vht_ie[1] >= sizeof(struct ieee80211_vht_cap))
		memcpy(&assoc_data->ap_vht_cap, vht_ie + 2,
		       sizeof(struct ieee80211_vht_cap));
	else
		ifmgd->flags |= IEEE80211_STA_DISABLE_VHT;
	rcu_read_unlock();

	if (WARN((sdata->vif.driver_flags & IEEE80211_VIF_SUPPORTS_UAPSD) &&
		 (local->hw.flags & IEEE80211_HW_PS_NULLFUNC_STACK),
	     "U-APSD not supported with HW_PS_NULLFUNC_STACK\n"))
		sdata->vif.driver_flags &= ~IEEE80211_VIF_SUPPORTS_UAPSD;

	if (bss->wmm_used && bss->uapsd_supported &&
	    (sdata->vif.driver_flags & IEEE80211_VIF_SUPPORTS_UAPSD)) {
		assoc_data->uapsd = true;
		ifmgd->flags |= IEEE80211_STA_UAPSD_ENABLED;
	} else {
		assoc_data->uapsd = false;
		ifmgd->flags &= ~IEEE80211_STA_UAPSD_ENABLED;
	}

	if (req->prev_bssid)
		memcpy(assoc_data->prev_bssid, req->prev_bssid, ETH_ALEN);

	if (req->use_mfp) {
		ifmgd->mfp = IEEE80211_MFP_REQUIRED;
		ifmgd->flags |= IEEE80211_STA_MFP_ENABLED;
	} else {
		ifmgd->mfp = IEEE80211_MFP_DISABLED;
		ifmgd->flags &= ~IEEE80211_STA_MFP_ENABLED;
	}

	if (req->flags & ASSOC_REQ_USE_RRM)
		ifmgd->flags |= IEEE80211_STA_ENABLE_RRM;
	else
		ifmgd->flags &= ~IEEE80211_STA_ENABLE_RRM;

	if (req->crypto.control_port)
		ifmgd->flags |= IEEE80211_STA_CONTROL_PORT;
	else
		ifmgd->flags &= ~IEEE80211_STA_CONTROL_PORT;

	sdata->control_port_protocol = req->crypto.control_port_ethertype;
	sdata->control_port_no_encrypt = req->crypto.control_port_no_encrypt;
	sdata->encrypt_headroom = ieee80211_cs_headroom(local, &req->crypto,
							sdata->vif.type);

	/* kick off associate process */

	ifmgd->assoc_data = assoc_data;
	ifmgd->dtim_period = 0;
	ifmgd->have_beacon = false;

	err = ieee80211_prep_connection(sdata, req->bss, true);
	if (err)
		goto err_clear;

	rcu_read_lock();
	beacon_ies = rcu_dereference(req->bss->beacon_ies);

	if (sdata->local->hw.flags & IEEE80211_HW_NEED_DTIM_BEFORE_ASSOC &&
	    !beacon_ies) {
		/*
		 * Wait up to one beacon interval ...
		 * should this be more if we miss one?
		 */
		sdata_info(sdata, "waiting for beacon from %pM\n",
			   ifmgd->bssid);
		assoc_data->timeout = TU_TO_EXP_TIME(req->bss->beacon_interval);
		assoc_data->timeout_started = true;
		assoc_data->need_beacon = true;
	} else if (beacon_ies) {
		const u8 *tim_ie = cfg80211_find_ie(WLAN_EID_TIM,
						    beacon_ies->data,
						    beacon_ies->len);
		u8 dtim_count = 0;

		if (tim_ie && tim_ie[1] >= sizeof(struct ieee80211_tim_ie)) {
			const struct ieee80211_tim_ie *tim;
			tim = (void *)(tim_ie + 2);
			ifmgd->dtim_period = tim->dtim_period;
			dtim_count = tim->dtim_count;
		}
		ifmgd->have_beacon = true;
		assoc_data->timeout = jiffies;
		assoc_data->timeout_started = true;

		if (local->hw.flags & IEEE80211_HW_TIMING_BEACON_ONLY) {
			sdata->vif.bss_conf.sync_tsf = beacon_ies->tsf;
			sdata->vif.bss_conf.sync_device_ts =
				bss->device_ts_beacon;
			sdata->vif.bss_conf.sync_dtim_count = dtim_count;
		}
	} else {
		assoc_data->timeout = jiffies;
		assoc_data->timeout_started = true;
	}
	rcu_read_unlock();

	run_again(sdata, assoc_data->timeout);

	if (bss->corrupt_data) {
		char *corrupt_type = "data";
		if (bss->corrupt_data & IEEE80211_BSS_CORRUPT_BEACON) {
			if (bss->corrupt_data &
					IEEE80211_BSS_CORRUPT_PROBE_RESP)
				corrupt_type = "beacon and probe response";
			else
				corrupt_type = "beacon";
		} else if (bss->corrupt_data & IEEE80211_BSS_CORRUPT_PROBE_RESP)
			corrupt_type = "probe response";
		sdata_info(sdata, "associating with AP with corrupt %s\n",
			   corrupt_type);
	}

	return 0;
 err_clear:
	eth_zero_addr(ifmgd->bssid);
	ieee80211_bss_info_change_notify(sdata, BSS_CHANGED_BSSID);
	ifmgd->assoc_data = NULL;
 err_free:
	kfree(assoc_data);
	return err;
}

int ieee80211_mgd_deauth(struct ieee80211_sub_if_data *sdata,
			 struct cfg80211_deauth_request *req)
{
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	u8 frame_buf[IEEE80211_DEAUTH_FRAME_LEN];
	bool tx = !req->local_state_change;

	if (ifmgd->auth_data &&
	    ether_addr_equal(ifmgd->auth_data->bss->bssid, req->bssid)) {
		sdata_info(sdata,
			   "aborting authentication with %pM by local choice (Reason: %u=%s)\n",
			   req->bssid, req->reason_code,
			   ieee80211_get_reason_code_string(req->reason_code));

		drv_mgd_prepare_tx(sdata->local, sdata);
		ieee80211_send_deauth_disassoc(sdata, req->bssid,
					       IEEE80211_STYPE_DEAUTH,
					       req->reason_code, tx,
					       frame_buf);
		ieee80211_destroy_auth_data(sdata, false);
		ieee80211_report_disconnect(sdata, frame_buf,
					    sizeof(frame_buf), true,
					    req->reason_code);

		return 0;
	}

	if (ifmgd->assoc_data &&
	    ether_addr_equal(ifmgd->assoc_data->bss->bssid, req->bssid)) {
		sdata_info(sdata,
			   "aborting association with %pM by local choice (Reason: %u=%s)\n",
			   req->bssid, req->reason_code,
			   ieee80211_get_reason_code_string(req->reason_code));

		drv_mgd_prepare_tx(sdata->local, sdata);
		ieee80211_send_deauth_disassoc(sdata, req->bssid,
					       IEEE80211_STYPE_DEAUTH,
					       req->reason_code, tx,
					       frame_buf);
		ieee80211_destroy_assoc_data(sdata, false);
		ieee80211_report_disconnect(sdata, frame_buf,
					    sizeof(frame_buf), true,
					    req->reason_code);
		return 0;
	}

	if (ifmgd->associated &&
	    ether_addr_equal(ifmgd->associated->bssid, req->bssid)) {
		sdata_info(sdata,
			   "deauthenticating from %pM by local choice (Reason: %u=%s)\n",
			   req->bssid, req->reason_code,
			   ieee80211_get_reason_code_string(req->reason_code));

		ieee80211_set_disassoc(sdata, IEEE80211_STYPE_DEAUTH,
				       req->reason_code, tx, frame_buf);
		ieee80211_report_disconnect(sdata, frame_buf,
					    sizeof(frame_buf), true,
					    req->reason_code);
		return 0;
	}

	return -ENOTCONN;
}

int ieee80211_mgd_disassoc(struct ieee80211_sub_if_data *sdata,
			   struct cfg80211_disassoc_request *req)
{
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	u8 bssid[ETH_ALEN];
	u8 frame_buf[IEEE80211_DEAUTH_FRAME_LEN];

	/*
	 * cfg80211 should catch this ... but it's racy since
	 * we can receive a disassoc frame, process it, hand it
	 * to cfg80211 while that's in a locked section already
	 * trying to tell us that the user wants to disconnect.
	 */
	if (ifmgd->associated != req->bss)
		return -ENOLINK;

	sdata_info(sdata,
		   "disassociating from %pM by local choice (Reason: %u=%s)\n",
		   req->bss->bssid, req->reason_code, ieee80211_get_reason_code_string(req->reason_code));

	memcpy(bssid, req->bss->bssid, ETH_ALEN);
	ieee80211_set_disassoc(sdata, IEEE80211_STYPE_DISASSOC,
			       req->reason_code, !req->local_state_change,
			       frame_buf);

	ieee80211_report_disconnect(sdata, frame_buf, sizeof(frame_buf), true,
				    req->reason_code);

	return 0;
}

void ieee80211_mgd_stop(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;

	/*
	 * Make sure some work items will not run after this,
	 * they will not do anything but might not have been
	 * cancelled when disconnecting.
	 */
	cancel_work_sync(&ifmgd->monitor_work);
	cancel_work_sync(&ifmgd->beacon_connection_loss_work);
	cancel_work_sync(&ifmgd->request_smps_work);
	cancel_work_sync(&ifmgd->csa_connection_drop_work);
	cancel_work_sync(&ifmgd->chswitch_work);
	cancel_delayed_work_sync(&ifmgd->tdls_peer_del_work);

	sdata_lock(sdata);
	if (ifmgd->assoc_data) {
		struct cfg80211_bss *bss = ifmgd->assoc_data->bss;
		ieee80211_destroy_assoc_data(sdata, false);
		cfg80211_assoc_timeout(sdata->dev, bss);
	}
	if (ifmgd->auth_data)
		ieee80211_destroy_auth_data(sdata, false);
	spin_lock_bh(&ifmgd->teardown_lock);
	if (ifmgd->teardown_skb) {
		kfree_skb(ifmgd->teardown_skb);
		ifmgd->teardown_skb = NULL;
		ifmgd->orig_teardown_skb = NULL;
	}
	spin_unlock_bh(&ifmgd->teardown_lock);
	del_timer_sync(&ifmgd->timer);
	sdata_unlock(sdata);
}

void ieee80211_cqm_rssi_notify(struct ieee80211_vif *vif,
			       enum nl80211_cqm_rssi_threshold_event rssi_event,
			       gfp_t gfp)
{
	struct ieee80211_sub_if_data *sdata = vif_to_sdata(vif);

	trace_api_cqm_rssi_notify(sdata, rssi_event);

	cfg80211_cqm_rssi_notify(sdata->dev, rssi_event, gfp);
}
EXPORT_SYMBOL(ieee80211_cqm_rssi_notify);

void ieee80211_cqm_beacon_loss_notify(struct ieee80211_vif *vif, gfp_t gfp)
{
	struct ieee80211_sub_if_data *sdata = vif_to_sdata(vif);

	trace_api_cqm_beacon_loss_notify(sdata->local, sdata);

	cfg80211_cqm_beacon_loss_notify(sdata->dev, gfp);
}
EXPORT_SYMBOL(ieee80211_cqm_beacon_loss_notify);
