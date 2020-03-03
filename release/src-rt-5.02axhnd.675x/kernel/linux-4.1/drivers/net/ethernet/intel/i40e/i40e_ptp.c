/*******************************************************************************
 *
 * Intel Ethernet Controller XL710 Family Linux Driver
 * Copyright(c) 2013 - 2014 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 *
 * Contact Information:
 * e1000-devel Mailing List <e1000-devel@lists.sourceforge.net>
 * Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497
 *
 ******************************************************************************/

#include "i40e.h"
#include <linux/ptp_classify.h>

/* The XL710 timesync is very much like Intel's 82599 design when it comes to
 * the fundamental clock design. However, the clock operations are much simpler
 * in the XL710 because the device supports a full 64 bits of nanoseconds.
 * Because the field is so wide, we can forgo the cycle counter and just
 * operate with the nanosecond field directly without fear of overflow.
 *
 * Much like the 82599, the update period is dependent upon the link speed:
 * At 40Gb link or no link, the period is 1.6ns.
 * At 10Gb link, the period is multiplied by 2. (3.2ns)
 * At 1Gb link, the period is multiplied by 20. (32ns)
 * 1588 functionality is not supported at 100Mbps.
 */
#define I40E_PTP_40GB_INCVAL 0x0199999999ULL
#define I40E_PTP_10GB_INCVAL 0x0333333333ULL
#define I40E_PTP_1GB_INCVAL  0x2000000000ULL

#define I40E_PRTTSYN_CTL1_TSYNTYPE_V1  (0x1 << \
					I40E_PRTTSYN_CTL1_TSYNTYPE_SHIFT)
#define I40E_PRTTSYN_CTL1_TSYNTYPE_V2  (0x2 << \
					I40E_PRTTSYN_CTL1_TSYNTYPE_SHIFT)

/**
 * i40e_ptp_read - Read the PHC time from the device
 * @pf: Board private structure
 * @ts: timespec structure to hold the current time value
 *
 * This function reads the PRTTSYN_TIME registers and stores them in a
 * timespec. However, since the registers are 64 bits of nanoseconds, we must
 * convert the result to a timespec before we can return.
 **/
static void i40e_ptp_read(struct i40e_pf *pf, struct timespec64 *ts)
{
	struct i40e_hw *hw = &pf->hw;
	u32 hi, lo;
	u64 ns;

	/* The timer latches on the lowest register read. */
	lo = rd32(hw, I40E_PRTTSYN_TIME_L);
	hi = rd32(hw, I40E_PRTTSYN_TIME_H);

	ns = (((u64)hi) << 32) | lo;

	*ts = ns_to_timespec64(ns);
}

/**
 * i40e_ptp_write - Write the PHC time to the device
 * @pf: Board private structure
 * @ts: timespec structure that holds the new time value
 *
 * This function writes the PRTTSYN_TIME registers with the user value. Since
 * we receive a timespec from the stack, we must convert that timespec into
 * nanoseconds before programming the registers.
 **/
static void i40e_ptp_write(struct i40e_pf *pf, const struct timespec64 *ts)
{
	struct i40e_hw *hw = &pf->hw;
	u64 ns = timespec64_to_ns(ts);

	/* The timer will not update until the high register is written, so
	 * write the low register first.
	 */
	wr32(hw, I40E_PRTTSYN_TIME_L, ns & 0xFFFFFFFF);
	wr32(hw, I40E_PRTTSYN_TIME_H, ns >> 32);
}

/**
 * i40e_ptp_convert_to_hwtstamp - Convert device clock to system time
 * @hwtstamps: Timestamp structure to update
 * @timestamp: Timestamp from the hardware
 *
 * We need to convert the NIC clock value into a hwtstamp which can be used by
 * the upper level timestamping functions. Since the timestamp is simply a 64-
 * bit nanosecond value, we can call ns_to_ktime directly to handle this.
 **/
static void i40e_ptp_convert_to_hwtstamp(struct skb_shared_hwtstamps *hwtstamps,
					 u64 timestamp)
{
	memset(hwtstamps, 0, sizeof(*hwtstamps));

	hwtstamps->hwtstamp = ns_to_ktime(timestamp);
}

/**
 * i40e_ptp_adjfreq - Adjust the PHC frequency
 * @ptp: The PTP clock structure
 * @ppb: Parts per billion adjustment from the base
 *
 * Adjust the frequency of the PHC by the indicated parts per billion from the
 * base frequency.
 **/
static int i40e_ptp_adjfreq(struct ptp_clock_info *ptp, s32 ppb)
{
	struct i40e_pf *pf = container_of(ptp, struct i40e_pf, ptp_caps);
	struct i40e_hw *hw = &pf->hw;
	u64 adj, freq, diff;
	int neg_adj = 0;

	if (ppb < 0) {
		neg_adj = 1;
		ppb = -ppb;
	}

	smp_mb(); /* Force any pending update before accessing. */
	adj = ACCESS_ONCE(pf->ptp_base_adj);

	freq = adj;
	freq *= ppb;
	diff = div_u64(freq, 1000000000ULL);

	if (neg_adj)
		adj -= diff;
	else
		adj += diff;

	wr32(hw, I40E_PRTTSYN_INC_L, adj & 0xFFFFFFFF);
	wr32(hw, I40E_PRTTSYN_INC_H, adj >> 32);

	return 0;
}

/**
 * i40e_ptp_adjtime - Adjust the PHC time
 * @ptp: The PTP clock structure
 * @delta: Offset in nanoseconds to adjust the PHC time by
 *
 * Adjust the frequency of the PHC by the indicated parts per billion from the
 * base frequency.
 **/
static int i40e_ptp_adjtime(struct ptp_clock_info *ptp, s64 delta)
{
	struct i40e_pf *pf = container_of(ptp, struct i40e_pf, ptp_caps);
	struct timespec64 now, then = ns_to_timespec64(delta);
	unsigned long flags;

	spin_lock_irqsave(&pf->tmreg_lock, flags);

	i40e_ptp_read(pf, &now);
	now = timespec64_add(now, then);
	i40e_ptp_write(pf, (const struct timespec64 *)&now);

	spin_unlock_irqrestore(&pf->tmreg_lock, flags);

	return 0;
}

/**
 * i40e_ptp_gettime - Get the time of the PHC
 * @ptp: The PTP clock structure
 * @ts: timespec structure to hold the current time value
 *
 * Read the device clock and return the correct value on ns, after converting it
 * into a timespec struct.
 **/
static int i40e_ptp_gettime(struct ptp_clock_info *ptp, struct timespec64 *ts)
{
	struct i40e_pf *pf = container_of(ptp, struct i40e_pf, ptp_caps);
	unsigned long flags;

	spin_lock_irqsave(&pf->tmreg_lock, flags);
	i40e_ptp_read(pf, ts);
	spin_unlock_irqrestore(&pf->tmreg_lock, flags);

	return 0;
}

/**
 * i40e_ptp_settime - Set the time of the PHC
 * @ptp: The PTP clock structure
 * @ts: timespec structure that holds the new time value
 *
 * Set the device clock to the user input value. The conversion from timespec
 * to ns happens in the write function.
 **/
static int i40e_ptp_settime(struct ptp_clock_info *ptp,
			    const struct timespec64 *ts)
{
	struct i40e_pf *pf = container_of(ptp, struct i40e_pf, ptp_caps);
	unsigned long flags;

	spin_lock_irqsave(&pf->tmreg_lock, flags);
	i40e_ptp_write(pf, ts);
	spin_unlock_irqrestore(&pf->tmreg_lock, flags);

	return 0;
}

/**
 * i40e_ptp_feature_enable - Enable/disable ancillary features of the PHC subsystem
 * @ptp: The PTP clock structure
 * @rq: The requested feature to change
 * @on: Enable/disable flag
 *
 * The XL710 does not support any of the ancillary features of the PHC
 * subsystem, so this function may just return.
 **/
static int i40e_ptp_feature_enable(struct ptp_clock_info *ptp,
				   struct ptp_clock_request *rq, int on)
{
	return -EOPNOTSUPP;
}

/**
 * i40e_ptp_rx_hang - Detect error case when Rx timestamp registers are hung
 * @vsi: The VSI with the rings relevant to 1588
 *
 * This watchdog task is scheduled to detect error case where hardware has
 * dropped an Rx packet that was timestamped when the ring is full. The
 * particular error is rare but leaves the device in a state unable to timestamp
 * any future packets.
 **/
void i40e_ptp_rx_hang(struct i40e_vsi *vsi)
{
	struct i40e_pf *pf = vsi->back;
	struct i40e_hw *hw = &pf->hw;
	struct i40e_ring *rx_ring;
	unsigned long rx_event;
	u32 prttsyn_stat;
	int n;

	/* Since we cannot turn off the Rx timestamp logic if the device is
	 * configured for Tx timestamping, we check if Rx timestamping is
	 * configured. We don't want to spuriously warn about Rx timestamp
	 * hangs if we don't care about the timestamps.
	 */
	if (!(pf->flags & I40E_FLAG_PTP) || !pf->ptp_rx)
		return;

	prttsyn_stat = rd32(hw, I40E_PRTTSYN_STAT_1);

	/* Unless all four receive timestamp registers are latched, we are not
	 * concerned about a possible PTP Rx hang, so just update the timeout
	 * counter and exit.
	 */
	if (!(prttsyn_stat & ((I40E_PRTTSYN_STAT_1_RXT0_MASK <<
			       I40E_PRTTSYN_STAT_1_RXT0_SHIFT) |
			      (I40E_PRTTSYN_STAT_1_RXT1_MASK <<
			       I40E_PRTTSYN_STAT_1_RXT1_SHIFT) |
			      (I40E_PRTTSYN_STAT_1_RXT2_MASK <<
			       I40E_PRTTSYN_STAT_1_RXT2_SHIFT) |
			      (I40E_PRTTSYN_STAT_1_RXT3_MASK <<
			       I40E_PRTTSYN_STAT_1_RXT3_SHIFT)))) {
		pf->last_rx_ptp_check = jiffies;
		return;
	}

	/* Determine the most recent watchdog or rx_timestamp event. */
	rx_event = pf->last_rx_ptp_check;
	for (n = 0; n < vsi->num_queue_pairs; n++) {
		rx_ring = vsi->rx_rings[n];
		if (time_after(rx_ring->last_rx_timestamp, rx_event))
			rx_event = rx_ring->last_rx_timestamp;
	}

	/* Only need to read the high RXSTMP register to clear the lock */
	if (time_is_before_jiffies(rx_event + 5 * HZ)) {
		rd32(hw, I40E_PRTTSYN_RXTIME_H(0));
		rd32(hw, I40E_PRTTSYN_RXTIME_H(1));
		rd32(hw, I40E_PRTTSYN_RXTIME_H(2));
		rd32(hw, I40E_PRTTSYN_RXTIME_H(3));
		pf->last_rx_ptp_check = jiffies;
		pf->rx_hwtstamp_cleared++;
		dev_warn(&vsi->back->pdev->dev,
			 "%s: clearing Rx timestamp hang\n",
			 __func__);
	}
}

/**
 * i40e_ptp_tx_hwtstamp - Utility function which returns the Tx timestamp
 * @pf: Board private structure
 *
 * Read the value of the Tx timestamp from the registers, convert it into a
 * value consumable by the stack, and store that result into the shhwtstamps
 * struct before returning it up the stack.
 **/
void i40e_ptp_tx_hwtstamp(struct i40e_pf *pf)
{
	struct skb_shared_hwtstamps shhwtstamps;
	struct i40e_hw *hw = &pf->hw;
	u32 hi, lo;
	u64 ns;

	if (!(pf->flags & I40E_FLAG_PTP) || !pf->ptp_tx)
		return;

	/* don't attempt to timestamp if we don't have an skb */
	if (!pf->ptp_tx_skb)
		return;

	lo = rd32(hw, I40E_PRTTSYN_TXTIME_L);
	hi = rd32(hw, I40E_PRTTSYN_TXTIME_H);

	ns = (((u64)hi) << 32) | lo;

	i40e_ptp_convert_to_hwtstamp(&shhwtstamps, ns);
	skb_tstamp_tx(pf->ptp_tx_skb, &shhwtstamps);
	dev_kfree_skb_any(pf->ptp_tx_skb);
	pf->ptp_tx_skb = NULL;
	clear_bit_unlock(__I40E_PTP_TX_IN_PROGRESS, &pf->state);
}

/**
 * i40e_ptp_rx_hwtstamp - Utility function which checks for an Rx timestamp
 * @pf: Board private structure
 * @skb: Particular skb to send timestamp with
 * @index: Index into the receive timestamp registers for the timestamp
 *
 * The XL710 receives a notification in the receive descriptor with an offset
 * into the set of RXTIME registers where the timestamp is for that skb. This
 * function goes and fetches the receive timestamp from that offset, if a valid
 * one exists. The RXTIME registers are in ns, so we must convert the result
 * first.
 **/
void i40e_ptp_rx_hwtstamp(struct i40e_pf *pf, struct sk_buff *skb, u8 index)
{
	u32 prttsyn_stat, hi, lo;
	struct i40e_hw *hw;
	u64 ns;

	/* Since we cannot turn off the Rx timestamp logic if the device is
	 * doing Tx timestamping, check if Rx timestamping is configured.
	 */
	if (!(pf->flags & I40E_FLAG_PTP) || !pf->ptp_rx)
		return;

	hw = &pf->hw;

	prttsyn_stat = rd32(hw, I40E_PRTTSYN_STAT_1);

	if (!(prttsyn_stat & (1 << index)))
		return;

	lo = rd32(hw, I40E_PRTTSYN_RXTIME_L(index));
	hi = rd32(hw, I40E_PRTTSYN_RXTIME_H(index));

	ns = (((u64)hi) << 32) | lo;

	i40e_ptp_convert_to_hwtstamp(skb_hwtstamps(skb), ns);
}

/**
 * i40e_ptp_set_increment - Utility function to update clock increment rate
 * @pf: Board private structure
 *
 * During a link change, the DMA frequency that drives the 1588 logic will
 * change. In order to keep the PRTTSYN_TIME registers in units of nanoseconds,
 * we must update the increment value per clock tick.
 **/
void i40e_ptp_set_increment(struct i40e_pf *pf)
{
	struct i40e_link_status *hw_link_info;
	struct i40e_hw *hw = &pf->hw;
	u64 incval;

	hw_link_info = &hw->phy.link_info;

	i40e_aq_get_link_info(&pf->hw, true, NULL, NULL);

	switch (hw_link_info->link_speed) {
	case I40E_LINK_SPEED_10GB:
		incval = I40E_PTP_10GB_INCVAL;
		break;
	case I40E_LINK_SPEED_1GB:
		incval = I40E_PTP_1GB_INCVAL;
		break;
	case I40E_LINK_SPEED_100MB:
	{
		static int warn_once;

		if (!warn_once) {
			dev_warn(&pf->pdev->dev,
				 "1588 functionality is not supported at 100 Mbps. Stopping the PHC.\n");
			warn_once++;
		}
		incval = 0;
		break;
	}
	case I40E_LINK_SPEED_40GB:
	default:
		incval = I40E_PTP_40GB_INCVAL;
		break;
	}

	/* Write the new increment value into the increment register. The
	 * hardware will not update the clock until both registers have been
	 * written.
	 */
	wr32(hw, I40E_PRTTSYN_INC_L, incval & 0xFFFFFFFF);
	wr32(hw, I40E_PRTTSYN_INC_H, incval >> 32);

	/* Update the base adjustement value. */
	ACCESS_ONCE(pf->ptp_base_adj) = incval;
	smp_mb(); /* Force the above update. */
}

/**
 * i40e_ptp_get_ts_config - ioctl interface to read the HW timestamping
 * @pf: Board private structure
 * @ifreq: ioctl data
 *
 * Obtain the current hardware timestamping settigs as requested. To do this,
 * keep a shadow copy of the timestamp settings rather than attempting to
 * deconstruct it from the registers.
 **/
int i40e_ptp_get_ts_config(struct i40e_pf *pf, struct ifreq *ifr)
{
	struct hwtstamp_config *config = &pf->tstamp_config;

	if (!(pf->flags & I40E_FLAG_PTP))
		return -EOPNOTSUPP;

	return copy_to_user(ifr->ifr_data, config, sizeof(*config)) ?
		-EFAULT : 0;
}

/**
 * i40e_ptp_set_timestamp_mode - setup hardware for requested timestamp mode
 * @pf: Board private structure
 * @config: hwtstamp settings requested or saved
 *
 * Control hardware registers to enter the specific mode requested by the
 * user. Also used during reset path to ensure that timestamp settings are
 * maintained.
 *
 * Note: modifies config in place, and may update the requested mode to be
 * more broad if the specific filter is not directly supported.
 **/
static int i40e_ptp_set_timestamp_mode(struct i40e_pf *pf,
				       struct hwtstamp_config *config)
{
	struct i40e_hw *hw = &pf->hw;
	u32 tsyntype, regval;

	/* Reserved for future extensions. */
	if (config->flags)
		return -EINVAL;

	switch (config->tx_type) {
	case HWTSTAMP_TX_OFF:
		pf->ptp_tx = false;
		break;
	case HWTSTAMP_TX_ON:
		pf->ptp_tx = true;
		break;
	default:
		return -ERANGE;
	}

	switch (config->rx_filter) {
	case HWTSTAMP_FILTER_NONE:
		pf->ptp_rx = false;
		/* We set the type to V1, but do not enable UDP packet
		 * recognition. In this way, we should be as close to
		 * disabling PTP Rx timestamps as possible since V1 packets
		 * are always UDP, since L2 packets are a V2 feature.
		 */
		tsyntype = I40E_PRTTSYN_CTL1_TSYNTYPE_V1;
		break;
	case HWTSTAMP_FILTER_PTP_V1_L4_SYNC:
	case HWTSTAMP_FILTER_PTP_V1_L4_DELAY_REQ:
	case HWTSTAMP_FILTER_PTP_V1_L4_EVENT:
		pf->ptp_rx = true;
		tsyntype = I40E_PRTTSYN_CTL1_V1MESSTYPE0_MASK |
			   I40E_PRTTSYN_CTL1_TSYNTYPE_V1 |
			   I40E_PRTTSYN_CTL1_UDP_ENA_MASK;
		config->rx_filter = HWTSTAMP_FILTER_PTP_V1_L4_EVENT;
		break;
	case HWTSTAMP_FILTER_PTP_V2_EVENT:
	case HWTSTAMP_FILTER_PTP_V2_L2_EVENT:
	case HWTSTAMP_FILTER_PTP_V2_L4_EVENT:
	case HWTSTAMP_FILTER_PTP_V2_SYNC:
	case HWTSTAMP_FILTER_PTP_V2_L2_SYNC:
	case HWTSTAMP_FILTER_PTP_V2_L4_SYNC:
	case HWTSTAMP_FILTER_PTP_V2_DELAY_REQ:
	case HWTSTAMP_FILTER_PTP_V2_L2_DELAY_REQ:
	case HWTSTAMP_FILTER_PTP_V2_L4_DELAY_REQ:
		pf->ptp_rx = true;
		tsyntype = I40E_PRTTSYN_CTL1_V2MESSTYPE0_MASK |
			   I40E_PRTTSYN_CTL1_TSYNTYPE_V2 |
			   I40E_PRTTSYN_CTL1_UDP_ENA_MASK;
		config->rx_filter = HWTSTAMP_FILTER_PTP_V2_EVENT;
		break;
	case HWTSTAMP_FILTER_ALL:
	default:
		return -ERANGE;
	}

	/* Clear out all 1588-related registers to clear and unlatch them. */
	rd32(hw, I40E_PRTTSYN_STAT_0);
	rd32(hw, I40E_PRTTSYN_TXTIME_H);
	rd32(hw, I40E_PRTTSYN_RXTIME_H(0));
	rd32(hw, I40E_PRTTSYN_RXTIME_H(1));
	rd32(hw, I40E_PRTTSYN_RXTIME_H(2));
	rd32(hw, I40E_PRTTSYN_RXTIME_H(3));

	/* Enable/disable the Tx timestamp interrupt based on user input. */
	regval = rd32(hw, I40E_PRTTSYN_CTL0);
	if (pf->ptp_tx)
		regval |= I40E_PRTTSYN_CTL0_TXTIME_INT_ENA_MASK;
	else
		regval &= ~I40E_PRTTSYN_CTL0_TXTIME_INT_ENA_MASK;
	wr32(hw, I40E_PRTTSYN_CTL0, regval);

	regval = rd32(hw, I40E_PFINT_ICR0_ENA);
	if (pf->ptp_tx)
		regval |= I40E_PFINT_ICR0_ENA_TIMESYNC_MASK;
	else
		regval &= ~I40E_PFINT_ICR0_ENA_TIMESYNC_MASK;
	wr32(hw, I40E_PFINT_ICR0_ENA, regval);

	/* Although there is no simple on/off switch for Rx, we "disable" Rx
	 * timestamps by setting to V1 only mode and clear the UDP
	 * recognition. This ought to disable all PTP Rx timestamps as V1
	 * packets are always over UDP. Note that software is configured to
	 * ignore Rx timestamps via the pf->ptp_rx flag.
	 */
	regval = rd32(hw, I40E_PRTTSYN_CTL1);
	/* clear everything but the enable bit */
	regval &= I40E_PRTTSYN_CTL1_TSYNENA_MASK;
	/* now enable bits for desired Rx timestamps */
	regval |= tsyntype;
	wr32(hw, I40E_PRTTSYN_CTL1, regval);

	return 0;
}

/**
 * i40e_ptp_set_ts_config - ioctl interface to control the HW timestamping
 * @pf: Board private structure
 * @ifreq: ioctl data
 *
 * Respond to the user filter requests and make the appropriate hardware
 * changes here. The XL710 cannot support splitting of the Tx/Rx timestamping
 * logic, so keep track in software of whether to indicate these timestamps
 * or not.
 *
 * It is permissible to "upgrade" the user request to a broader filter, as long
 * as the user receives the timestamps they care about and the user is notified
 * the filter has been broadened.
 **/
int i40e_ptp_set_ts_config(struct i40e_pf *pf, struct ifreq *ifr)
{
	struct hwtstamp_config config;
	int err;

	if (!(pf->flags & I40E_FLAG_PTP))
		return -EOPNOTSUPP;

	if (copy_from_user(&config, ifr->ifr_data, sizeof(config)))
		return -EFAULT;

	err = i40e_ptp_set_timestamp_mode(pf, &config);
	if (err)
		return err;

	/* save these settings for future reference */
	pf->tstamp_config = config;

	return copy_to_user(ifr->ifr_data, &config, sizeof(config)) ?
		-EFAULT : 0;
}

/**
 * i40e_ptp_create_clock - Create PTP clock device for userspace
 * @pf: Board private structure
 *
 * This function creates a new PTP clock device. It only creates one if we
 * don't already have one, so it is safe to call. Will return error if it
 * can't create one, but success if we already have a device. Should be used
 * by i40e_ptp_init to create clock initially, and prevent global resets from
 * creating new clock devices.
 **/
static long i40e_ptp_create_clock(struct i40e_pf *pf)
{
	/* no need to create a clock device if we already have one */
	if (!IS_ERR_OR_NULL(pf->ptp_clock))
		return 0;

	strncpy(pf->ptp_caps.name, i40e_driver_name, sizeof(pf->ptp_caps.name));
	pf->ptp_caps.owner = THIS_MODULE;
	pf->ptp_caps.max_adj = 999999999;
	pf->ptp_caps.n_ext_ts = 0;
	pf->ptp_caps.pps = 0;
	pf->ptp_caps.adjfreq = i40e_ptp_adjfreq;
	pf->ptp_caps.adjtime = i40e_ptp_adjtime;
	pf->ptp_caps.gettime64 = i40e_ptp_gettime;
	pf->ptp_caps.settime64 = i40e_ptp_settime;
	pf->ptp_caps.enable = i40e_ptp_feature_enable;

	/* Attempt to register the clock before enabling the hardware. */
	pf->ptp_clock = ptp_clock_register(&pf->ptp_caps, &pf->pdev->dev);
	if (IS_ERR(pf->ptp_clock)) {
		return PTR_ERR(pf->ptp_clock);
	}

	/* clear the hwtstamp settings here during clock create, instead of
	 * during regular init, so that we can maintain settings across a
	 * reset or suspend.
	 */
	pf->tstamp_config.rx_filter = HWTSTAMP_FILTER_NONE;
	pf->tstamp_config.tx_type = HWTSTAMP_TX_OFF;

	return 0;
}

/**
 * i40e_ptp_init - Initialize the 1588 support after device probe or reset
 * @pf: Board private structure
 *
 * This function sets device up for 1588 support. The first time it is run, it
 * will create a PHC clock device. It does not create a clock device if one
 * already exists. It also reconfigures the device after a reset.
 **/
void i40e_ptp_init(struct i40e_pf *pf)
{
	struct net_device *netdev = pf->vsi[pf->lan_vsi]->netdev;
	struct i40e_hw *hw = &pf->hw;
	u32 pf_id;
	long err;

	/* Only one PF is assigned to control 1588 logic per port. Do not
	 * enable any support for PFs not assigned via PRTTSYN_CTL0.PF_ID
	 */
	pf_id = (rd32(hw, I40E_PRTTSYN_CTL0) & I40E_PRTTSYN_CTL0_PF_ID_MASK) >>
		I40E_PRTTSYN_CTL0_PF_ID_SHIFT;
	if (hw->pf_id != pf_id) {
		pf->flags &= ~I40E_FLAG_PTP;
		dev_info(&pf->pdev->dev, "%s: PTP not supported on %s\n",
			 __func__,
			 netdev->name);
		return;
	}

	/* we have to initialize the lock first, since we can't control
	 * when the user will enter the PHC device entry points
	 */
	spin_lock_init(&pf->tmreg_lock);

	/* ensure we have a clock device */
	err = i40e_ptp_create_clock(pf);
	if (err) {
		pf->ptp_clock = NULL;
		dev_err(&pf->pdev->dev, "%s: ptp_clock_register failed\n",
			__func__);
	} else {
		struct timespec64 ts;
		u32 regval;

		dev_info(&pf->pdev->dev, "%s: added PHC on %s\n", __func__,
			 netdev->name);
		pf->flags |= I40E_FLAG_PTP;

		/* Ensure the clocks are running. */
		regval = rd32(hw, I40E_PRTTSYN_CTL0);
		regval |= I40E_PRTTSYN_CTL0_TSYNENA_MASK;
		wr32(hw, I40E_PRTTSYN_CTL0, regval);
		regval = rd32(hw, I40E_PRTTSYN_CTL1);
		regval |= I40E_PRTTSYN_CTL1_TSYNENA_MASK;
		wr32(hw, I40E_PRTTSYN_CTL1, regval);

		/* Set the increment value per clock tick. */
		i40e_ptp_set_increment(pf);

		/* reset timestamping mode */
		i40e_ptp_set_timestamp_mode(pf, &pf->tstamp_config);

		/* Set the clock value. */
		ts = ktime_to_timespec64(ktime_get_real());
		i40e_ptp_settime(&pf->ptp_caps, &ts);
	}
}

/**
 * i40e_ptp_stop - Disable the driver/hardware support and unregister the PHC
 * @pf: Board private structure
 *
 * This function handles the cleanup work required from the initialization by
 * clearing out the important information and unregistering the PHC.
 **/
void i40e_ptp_stop(struct i40e_pf *pf)
{
	pf->flags &= ~I40E_FLAG_PTP;
	pf->ptp_tx = false;
	pf->ptp_rx = false;

	if (pf->ptp_tx_skb) {
		dev_kfree_skb_any(pf->ptp_tx_skb);
		pf->ptp_tx_skb = NULL;
		clear_bit_unlock(__I40E_PTP_TX_IN_PROGRESS, &pf->state);
	}

	if (pf->ptp_clock) {
		ptp_clock_unregister(pf->ptp_clock);
		pf->ptp_clock = NULL;
		dev_info(&pf->pdev->dev, "%s: removed PHC on %s\n", __func__,
			 pf->vsi[pf->lan_vsi]->netdev->name);
	}
}
