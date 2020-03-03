/*
 * net/core/ethtool.c - Ethtool ioctl handler
 * Copyright (c) 2003 Matthew Wilcox <matthew@wil.cx>
 *
 * This file is where we call all the ethtool_ops commands to get
 * the information ethtool needs.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/capability.h>
#include <linux/errno.h>
#include <linux/ethtool.h>
#include <linux/netdevice.h>
#include <linux/net_tstamp.h>
#include <linux/phy.h>
#include <linux/bitops.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/rtnetlink.h>
#include <linux/sched.h>
#include <linux/net.h>

/*
 * Some useful ethtool_ops methods that're device independent.
 * If we find that all drivers want to do the same thing here,
 * we can turn these into dev_() function calls.
 */

u32 ethtool_op_get_link(struct net_device *dev)
{
	return netif_carrier_ok(dev) ? 1 : 0;
}
EXPORT_SYMBOL(ethtool_op_get_link);

int ethtool_op_get_ts_info(struct net_device *dev, struct ethtool_ts_info *info)
{
	info->so_timestamping =
		SOF_TIMESTAMPING_TX_SOFTWARE |
		SOF_TIMESTAMPING_RX_SOFTWARE |
		SOF_TIMESTAMPING_SOFTWARE;
	info->phc_index = -1;
	return 0;
}
EXPORT_SYMBOL(ethtool_op_get_ts_info);

/* Handlers for each ethtool command */

#define ETHTOOL_DEV_FEATURE_WORDS	((NETDEV_FEATURE_COUNT + 31) / 32)

static const char netdev_features_strings[NETDEV_FEATURE_COUNT][ETH_GSTRING_LEN] = {
	[NETIF_F_SG_BIT] =               "tx-scatter-gather",
	[NETIF_F_IP_CSUM_BIT] =          "tx-checksum-ipv4",
	[NETIF_F_HW_CSUM_BIT] =          "tx-checksum-ip-generic",
	[NETIF_F_IPV6_CSUM_BIT] =        "tx-checksum-ipv6",
	[NETIF_F_HIGHDMA_BIT] =          "highdma",
	[NETIF_F_FRAGLIST_BIT] =         "tx-scatter-gather-fraglist",
	[NETIF_F_HW_VLAN_CTAG_TX_BIT] =  "tx-vlan-hw-insert",

	[NETIF_F_HW_VLAN_CTAG_RX_BIT] =  "rx-vlan-hw-parse",
	[NETIF_F_HW_VLAN_CTAG_FILTER_BIT] = "rx-vlan-filter",
	[NETIF_F_HW_VLAN_STAG_TX_BIT] =  "tx-vlan-stag-hw-insert",
	[NETIF_F_HW_VLAN_STAG_RX_BIT] =  "rx-vlan-stag-hw-parse",
	[NETIF_F_HW_VLAN_STAG_FILTER_BIT] = "rx-vlan-stag-filter",
	[NETIF_F_VLAN_CHALLENGED_BIT] =  "vlan-challenged",
	[NETIF_F_GSO_BIT] =              "tx-generic-segmentation",
	[NETIF_F_LLTX_BIT] =             "tx-lockless",
	[NETIF_F_NETNS_LOCAL_BIT] =      "netns-local",
	[NETIF_F_GRO_BIT] =              "rx-gro",
	[NETIF_F_LRO_BIT] =              "rx-lro",

	[NETIF_F_TSO_BIT] =              "tx-tcp-segmentation",
	[NETIF_F_UFO_BIT] =              "tx-udp-fragmentation",
	[NETIF_F_GSO_ROBUST_BIT] =       "tx-gso-robust",
	[NETIF_F_TSO_ECN_BIT] =          "tx-tcp-ecn-segmentation",
	[NETIF_F_TSO6_BIT] =             "tx-tcp6-segmentation",
	[NETIF_F_FSO_BIT] =              "tx-fcoe-segmentation",
	[NETIF_F_GSO_GRE_BIT] =		 "tx-gre-segmentation",
	[NETIF_F_GSO_IPIP_BIT] =	 "tx-ipip-segmentation",
	[NETIF_F_GSO_SIT_BIT] =		 "tx-sit-segmentation",
	[NETIF_F_GSO_UDP_TUNNEL_BIT] =	 "tx-udp_tnl-segmentation",

	[NETIF_F_FCOE_CRC_BIT] =         "tx-checksum-fcoe-crc",
	[NETIF_F_SCTP_CSUM_BIT] =        "tx-checksum-sctp",
	[NETIF_F_FCOE_MTU_BIT] =         "fcoe-mtu",
	[NETIF_F_NTUPLE_BIT] =           "rx-ntuple-filter",
	[NETIF_F_RXHASH_BIT] =           "rx-hashing",
	[NETIF_F_RXCSUM_BIT] =           "rx-checksum",
	[NETIF_F_NOCACHE_COPY_BIT] =     "tx-nocache-copy",
	[NETIF_F_LOOPBACK_BIT] =         "loopback",
	[NETIF_F_RXFCS_BIT] =            "rx-fcs",
	[NETIF_F_RXALL_BIT] =            "rx-all",
	[NETIF_F_HW_L2FW_DOFFLOAD_BIT] = "l2-fwd-offload",
	[NETIF_F_BUSY_POLL_BIT] =        "busy-poll",
	[NETIF_F_HW_SWITCH_OFFLOAD_BIT] = "hw-switch-offload",
};

static const char
rss_hash_func_strings[ETH_RSS_HASH_FUNCS_COUNT][ETH_GSTRING_LEN] = {
	[ETH_RSS_HASH_TOP_BIT] =	"toeplitz",
	[ETH_RSS_HASH_XOR_BIT] =	"xor",
};

static int ethtool_get_features(struct net_device *dev, void __user *useraddr)
{
	struct ethtool_gfeatures cmd = {
		.cmd = ETHTOOL_GFEATURES,
		.size = ETHTOOL_DEV_FEATURE_WORDS,
	};
	struct ethtool_get_features_block features[ETHTOOL_DEV_FEATURE_WORDS];
	u32 __user *sizeaddr;
	u32 copy_size;
	int i;

	/* in case feature bits run out again */
	BUILD_BUG_ON(ETHTOOL_DEV_FEATURE_WORDS * sizeof(u32) > sizeof(netdev_features_t));

	for (i = 0; i < ETHTOOL_DEV_FEATURE_WORDS; ++i) {
		features[i].available = (u32)(dev->hw_features >> (32 * i));
		features[i].requested = (u32)(dev->wanted_features >> (32 * i));
		features[i].active = (u32)(dev->features >> (32 * i));
		features[i].never_changed =
			(u32)(NETIF_F_NEVER_CHANGE >> (32 * i));
	}

	sizeaddr = useraddr + offsetof(struct ethtool_gfeatures, size);
	if (get_user(copy_size, sizeaddr))
		return -EFAULT;

	if (copy_size > ETHTOOL_DEV_FEATURE_WORDS)
		copy_size = ETHTOOL_DEV_FEATURE_WORDS;

	if (copy_to_user(useraddr, &cmd, sizeof(cmd)))
		return -EFAULT;
	useraddr += sizeof(cmd);
	if (copy_to_user(useraddr, features, copy_size * sizeof(*features)))
		return -EFAULT;

	return 0;
}

static int ethtool_set_features(struct net_device *dev, void __user *useraddr)
{
	struct ethtool_sfeatures cmd;
	struct ethtool_set_features_block features[ETHTOOL_DEV_FEATURE_WORDS];
	netdev_features_t wanted = 0, valid = 0;
	int i, ret = 0;

	if (copy_from_user(&cmd, useraddr, sizeof(cmd)))
		return -EFAULT;
	useraddr += sizeof(cmd);

	if (cmd.size != ETHTOOL_DEV_FEATURE_WORDS)
		return -EINVAL;

	if (copy_from_user(features, useraddr, sizeof(features)))
		return -EFAULT;

	for (i = 0; i < ETHTOOL_DEV_FEATURE_WORDS; ++i) {
		valid |= (netdev_features_t)features[i].valid << (32 * i);
		wanted |= (netdev_features_t)features[i].requested << (32 * i);
	}

	if (valid & ~NETIF_F_ETHTOOL_BITS)
		return -EINVAL;

	if (valid & ~dev->hw_features) {
		valid &= dev->hw_features;
		ret |= ETHTOOL_F_UNSUPPORTED;
	}

	dev->wanted_features &= ~valid;
	dev->wanted_features |= wanted & valid;
	__netdev_update_features(dev);

	if ((dev->wanted_features ^ dev->features) & valid)
		ret |= ETHTOOL_F_WISH;

	return ret;
}

static int __ethtool_get_sset_count(struct net_device *dev, int sset)
{
	const struct ethtool_ops *ops = dev->ethtool_ops;

	if (sset == ETH_SS_FEATURES)
		return ARRAY_SIZE(netdev_features_strings);

	if (sset == ETH_SS_RSS_HASH_FUNCS)
		return ARRAY_SIZE(rss_hash_func_strings);

	if (ops->get_sset_count && ops->get_strings)
		return ops->get_sset_count(dev, sset);
	else
		return -EOPNOTSUPP;
}

static void __ethtool_get_strings(struct net_device *dev,
	u32 stringset, u8 *data)
{
	const struct ethtool_ops *ops = dev->ethtool_ops;

	if (stringset == ETH_SS_FEATURES)
		memcpy(data, netdev_features_strings,
			sizeof(netdev_features_strings));
	else if (stringset == ETH_SS_RSS_HASH_FUNCS)
		memcpy(data, rss_hash_func_strings,
		       sizeof(rss_hash_func_strings));
	else
		/* ops->get_strings is valid because checked earlier */
		ops->get_strings(dev, stringset, data);
}

static netdev_features_t ethtool_get_feature_mask(u32 eth_cmd)
{
	/* feature masks of legacy discrete ethtool ops */

	switch (eth_cmd) {
	case ETHTOOL_GTXCSUM:
	case ETHTOOL_STXCSUM:
		return NETIF_F_ALL_CSUM | NETIF_F_SCTP_CSUM;
	case ETHTOOL_GRXCSUM:
	case ETHTOOL_SRXCSUM:
		return NETIF_F_RXCSUM;
	case ETHTOOL_GSG:
	case ETHTOOL_SSG:
		return NETIF_F_SG;
	case ETHTOOL_GTSO:
	case ETHTOOL_STSO:
		return NETIF_F_ALL_TSO;
	case ETHTOOL_GUFO:
	case ETHTOOL_SUFO:
		return NETIF_F_UFO;
	case ETHTOOL_GGSO:
	case ETHTOOL_SGSO:
		return NETIF_F_GSO;
	case ETHTOOL_GGRO:
	case ETHTOOL_SGRO:
		return NETIF_F_GRO;
	default:
		BUG();
	}
}

static int ethtool_get_one_feature(struct net_device *dev,
	char __user *useraddr, u32 ethcmd)
{
	netdev_features_t mask = ethtool_get_feature_mask(ethcmd);
	struct ethtool_value edata = {
		.cmd = ethcmd,
		.data = !!(dev->features & mask),
	};

	if (copy_to_user(useraddr, &edata, sizeof(edata)))
		return -EFAULT;
	return 0;
}

static int ethtool_set_one_feature(struct net_device *dev,
	void __user *useraddr, u32 ethcmd)
{
	struct ethtool_value edata;
	netdev_features_t mask;

	if (copy_from_user(&edata, useraddr, sizeof(edata)))
		return -EFAULT;

	mask = ethtool_get_feature_mask(ethcmd);
	mask &= dev->hw_features;
	if (!mask)
		return -EOPNOTSUPP;

	if (edata.data)
		dev->wanted_features |= mask;
	else
		dev->wanted_features &= ~mask;

	__netdev_update_features(dev);

	return 0;
}

#define ETH_ALL_FLAGS    (ETH_FLAG_LRO | ETH_FLAG_RXVLAN | ETH_FLAG_TXVLAN | \
			  ETH_FLAG_NTUPLE | ETH_FLAG_RXHASH)
#define ETH_ALL_FEATURES (NETIF_F_LRO | NETIF_F_HW_VLAN_CTAG_RX | \
			  NETIF_F_HW_VLAN_CTAG_TX | NETIF_F_NTUPLE | \
			  NETIF_F_RXHASH)

static u32 __ethtool_get_flags(struct net_device *dev)
{
	u32 flags = 0;

	if (dev->features & NETIF_F_LRO)
		flags |= ETH_FLAG_LRO;
	if (dev->features & NETIF_F_HW_VLAN_CTAG_RX)
		flags |= ETH_FLAG_RXVLAN;
	if (dev->features & NETIF_F_HW_VLAN_CTAG_TX)
		flags |= ETH_FLAG_TXVLAN;
	if (dev->features & NETIF_F_NTUPLE)
		flags |= ETH_FLAG_NTUPLE;
	if (dev->features & NETIF_F_RXHASH)
		flags |= ETH_FLAG_RXHASH;

	return flags;
}

static int __ethtool_set_flags(struct net_device *dev, u32 data)
{
	netdev_features_t features = 0, changed;

	if (data & ~ETH_ALL_FLAGS)
		return -EINVAL;

	if (data & ETH_FLAG_LRO)
		features |= NETIF_F_LRO;
	if (data & ETH_FLAG_RXVLAN)
		features |= NETIF_F_HW_VLAN_CTAG_RX;
	if (data & ETH_FLAG_TXVLAN)
		features |= NETIF_F_HW_VLAN_CTAG_TX;
	if (data & ETH_FLAG_NTUPLE)
		features |= NETIF_F_NTUPLE;
	if (data & ETH_FLAG_RXHASH)
		features |= NETIF_F_RXHASH;

	/* allow changing only bits set in hw_features */
	changed = (features ^ dev->features) & ETH_ALL_FEATURES;
	if (changed & ~dev->hw_features)
		return (changed & dev->hw_features) ? -EINVAL : -EOPNOTSUPP;

	dev->wanted_features =
		(dev->wanted_features & ~changed) | (features & changed);

	__netdev_update_features(dev);

	return 0;
}

int __ethtool_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	ASSERT_RTNL();

	if (!dev->ethtool_ops->get_settings)
		return -EOPNOTSUPP;

	memset(cmd, 0, sizeof(struct ethtool_cmd));
	cmd->cmd = ETHTOOL_GSET;
	return dev->ethtool_ops->get_settings(dev, cmd);
}
EXPORT_SYMBOL(__ethtool_get_settings);

static int ethtool_get_settings(struct net_device *dev, void __user *useraddr)
{
	int err;
	struct ethtool_cmd cmd;

	err = __ethtool_get_settings(dev, &cmd);
	if (err < 0)
		return err;

	if (copy_to_user(useraddr, &cmd, sizeof(cmd)))
		return -EFAULT;
	return 0;
}

static int ethtool_set_settings(struct net_device *dev, void __user *useraddr)
{
	struct ethtool_cmd cmd;

	if (!dev->ethtool_ops->set_settings)
		return -EOPNOTSUPP;

	if (copy_from_user(&cmd, useraddr, sizeof(cmd)))
		return -EFAULT;

	return dev->ethtool_ops->set_settings(dev, &cmd);
}

static noinline_for_stack int ethtool_get_drvinfo(struct net_device *dev,
						  void __user *useraddr)
{
	struct ethtool_drvinfo info;
	const struct ethtool_ops *ops = dev->ethtool_ops;

	memset(&info, 0, sizeof(info));
	info.cmd = ETHTOOL_GDRVINFO;
	if (ops->get_drvinfo) {
		ops->get_drvinfo(dev, &info);
	} else if (dev->dev.parent && dev->dev.parent->driver) {
		strlcpy(info.bus_info, dev_name(dev->dev.parent),
			sizeof(info.bus_info));
		strlcpy(info.driver, dev->dev.parent->driver->name,
			sizeof(info.driver));
	} else {
		return -EOPNOTSUPP;
	}

	/*
	 * this method of obtaining string set info is deprecated;
	 * Use ETHTOOL_GSSET_INFO instead.
	 */
	if (ops->get_sset_count) {
		int rc;

		rc = ops->get_sset_count(dev, ETH_SS_TEST);
		if (rc >= 0)
			info.testinfo_len = rc;
		rc = ops->get_sset_count(dev, ETH_SS_STATS);
		if (rc >= 0)
			info.n_stats = rc;
		rc = ops->get_sset_count(dev, ETH_SS_PRIV_FLAGS);
		if (rc >= 0)
			info.n_priv_flags = rc;
	}
	if (ops->get_regs_len)
		info.regdump_len = ops->get_regs_len(dev);
	if (ops->get_eeprom_len)
		info.eedump_len = ops->get_eeprom_len(dev);

	if (copy_to_user(useraddr, &info, sizeof(info)))
		return -EFAULT;
	return 0;
}

static noinline_for_stack int ethtool_get_sset_info(struct net_device *dev,
						    void __user *useraddr)
{
	struct ethtool_sset_info info;
	u64 sset_mask;
	int i, idx = 0, n_bits = 0, ret, rc;
	u32 *info_buf = NULL;

	if (copy_from_user(&info, useraddr, sizeof(info)))
		return -EFAULT;

	/* store copy of mask, because we zero struct later on */
	sset_mask = info.sset_mask;
	if (!sset_mask)
		return 0;

	/* calculate size of return buffer */
	n_bits = hweight64(sset_mask);

	memset(&info, 0, sizeof(info));
	info.cmd = ETHTOOL_GSSET_INFO;

	info_buf = kzalloc(n_bits * sizeof(u32), GFP_USER);
	if (!info_buf)
		return -ENOMEM;

	/*
	 * fill return buffer based on input bitmask and successful
	 * get_sset_count return
	 */
	for (i = 0; i < 64; i++) {
		if (!(sset_mask & (1ULL << i)))
			continue;

		rc = __ethtool_get_sset_count(dev, i);
		if (rc >= 0) {
			info.sset_mask |= (1ULL << i);
			info_buf[idx++] = rc;
		}
	}

	ret = -EFAULT;
	if (copy_to_user(useraddr, &info, sizeof(info)))
		goto out;

	useraddr += offsetof(struct ethtool_sset_info, data);
	if (copy_to_user(useraddr, info_buf, idx * sizeof(u32)))
		goto out;

	ret = 0;

out:
	kfree(info_buf);
	return ret;
}

static noinline_for_stack int ethtool_set_rxnfc(struct net_device *dev,
						u32 cmd, void __user *useraddr)
{
	struct ethtool_rxnfc info;
	size_t info_size = sizeof(info);
	int rc;

	if (!dev->ethtool_ops->set_rxnfc)
		return -EOPNOTSUPP;

	/* struct ethtool_rxnfc was originally defined for
	 * ETHTOOL_{G,S}RXFH with only the cmd, flow_type and data
	 * members.  User-space might still be using that
	 * definition. */
	if (cmd == ETHTOOL_SRXFH)
		info_size = (offsetof(struct ethtool_rxnfc, data) +
			     sizeof(info.data));

	if (copy_from_user(&info, useraddr, info_size))
		return -EFAULT;

	rc = dev->ethtool_ops->set_rxnfc(dev, &info);
	if (rc)
		return rc;

	if (cmd == ETHTOOL_SRXCLSRLINS &&
	    copy_to_user(useraddr, &info, info_size))
		return -EFAULT;

	return 0;
}

static noinline_for_stack int ethtool_get_rxnfc(struct net_device *dev,
						u32 cmd, void __user *useraddr)
{
	struct ethtool_rxnfc info;
	size_t info_size = sizeof(info);
	const struct ethtool_ops *ops = dev->ethtool_ops;
	int ret;
	void *rule_buf = NULL;

	if (!ops->get_rxnfc)
		return -EOPNOTSUPP;

	/* struct ethtool_rxnfc was originally defined for
	 * ETHTOOL_{G,S}RXFH with only the cmd, flow_type and data
	 * members.  User-space might still be using that
	 * definition. */
	if (cmd == ETHTOOL_GRXFH)
		info_size = (offsetof(struct ethtool_rxnfc, data) +
			     sizeof(info.data));

	if (copy_from_user(&info, useraddr, info_size))
		return -EFAULT;

	if (info.cmd == ETHTOOL_GRXCLSRLALL) {
		if (info.rule_cnt > 0) {
			if (info.rule_cnt <= KMALLOC_MAX_SIZE / sizeof(u32))
				rule_buf = kzalloc(info.rule_cnt * sizeof(u32),
						   GFP_USER);
			if (!rule_buf)
				return -ENOMEM;
		}
	}

	ret = ops->get_rxnfc(dev, &info, rule_buf);
	if (ret < 0)
		goto err_out;

	ret = -EFAULT;
	if (copy_to_user(useraddr, &info, info_size))
		goto err_out;

	if (rule_buf) {
		useraddr += offsetof(struct ethtool_rxnfc, rule_locs);
		if (copy_to_user(useraddr, rule_buf,
				 info.rule_cnt * sizeof(u32)))
			goto err_out;
	}
	ret = 0;

err_out:
	kfree(rule_buf);

	return ret;
}

static int ethtool_copy_validate_indir(u32 *indir, void __user *useraddr,
					struct ethtool_rxnfc *rx_rings,
					u32 size)
{
	int i;

	if (copy_from_user(indir, useraddr, size * sizeof(indir[0])))
		return -EFAULT;

	/* Validate ring indices */
	for (i = 0; i < size; i++)
		if (indir[i] >= rx_rings->data)
			return -EINVAL;

	return 0;
}

u8 netdev_rss_key[NETDEV_RSS_KEY_LEN];

void netdev_rss_key_fill(void *buffer, size_t len)
{
	BUG_ON(len > sizeof(netdev_rss_key));
	net_get_random_once(netdev_rss_key, sizeof(netdev_rss_key));
	memcpy(buffer, netdev_rss_key, len);
}
EXPORT_SYMBOL(netdev_rss_key_fill);

static noinline_for_stack int ethtool_get_rxfh_indir(struct net_device *dev,
						     void __user *useraddr)
{
	u32 user_size, dev_size;
	u32 *indir;
	int ret;

	if (!dev->ethtool_ops->get_rxfh_indir_size ||
	    !dev->ethtool_ops->get_rxfh)
		return -EOPNOTSUPP;
	dev_size = dev->ethtool_ops->get_rxfh_indir_size(dev);
	if (dev_size == 0)
		return -EOPNOTSUPP;

	if (copy_from_user(&user_size,
			   useraddr + offsetof(struct ethtool_rxfh_indir, size),
			   sizeof(user_size)))
		return -EFAULT;

	if (copy_to_user(useraddr + offsetof(struct ethtool_rxfh_indir, size),
			 &dev_size, sizeof(dev_size)))
		return -EFAULT;

	/* If the user buffer size is 0, this is just a query for the
	 * device table size.  Otherwise, if it's smaller than the
	 * device table size it's an error.
	 */
	if (user_size < dev_size)
		return user_size == 0 ? 0 : -EINVAL;

	indir = kcalloc(dev_size, sizeof(indir[0]), GFP_USER);
	if (!indir)
		return -ENOMEM;

	ret = dev->ethtool_ops->get_rxfh(dev, indir, NULL, NULL);
	if (ret)
		goto out;

	if (copy_to_user(useraddr +
			 offsetof(struct ethtool_rxfh_indir, ring_index[0]),
			 indir, dev_size * sizeof(indir[0])))
		ret = -EFAULT;

out:
	kfree(indir);
	return ret;
}

static noinline_for_stack int ethtool_set_rxfh_indir(struct net_device *dev,
						     void __user *useraddr)
{
	struct ethtool_rxnfc rx_rings;
	u32 user_size, dev_size, i;
	u32 *indir;
	const struct ethtool_ops *ops = dev->ethtool_ops;
	int ret;
	u32 ringidx_offset = offsetof(struct ethtool_rxfh_indir, ring_index[0]);

	if (!ops->get_rxfh_indir_size || !ops->set_rxfh ||
	    !ops->get_rxnfc)
		return -EOPNOTSUPP;

	dev_size = ops->get_rxfh_indir_size(dev);
	if (dev_size == 0)
		return -EOPNOTSUPP;

	if (copy_from_user(&user_size,
			   useraddr + offsetof(struct ethtool_rxfh_indir, size),
			   sizeof(user_size)))
		return -EFAULT;

	if (user_size != 0 && user_size != dev_size)
		return -EINVAL;

	indir = kcalloc(dev_size, sizeof(indir[0]), GFP_USER);
	if (!indir)
		return -ENOMEM;

	rx_rings.cmd = ETHTOOL_GRXRINGS;
	ret = ops->get_rxnfc(dev, &rx_rings, NULL);
	if (ret)
		goto out;

	if (user_size == 0) {
		for (i = 0; i < dev_size; i++)
			indir[i] = ethtool_rxfh_indir_default(i, rx_rings.data);
	} else {
		ret = ethtool_copy_validate_indir(indir,
						  useraddr + ringidx_offset,
						  &rx_rings,
						  dev_size);
		if (ret)
			goto out;
	}

	ret = ops->set_rxfh(dev, indir, NULL, ETH_RSS_HASH_NO_CHANGE);

out:
	kfree(indir);
	return ret;
}

static noinline_for_stack int ethtool_get_rxfh(struct net_device *dev,
					       void __user *useraddr)
{
	int ret;
	const struct ethtool_ops *ops = dev->ethtool_ops;
	u32 user_indir_size, user_key_size;
	u32 dev_indir_size = 0, dev_key_size = 0;
	struct ethtool_rxfh rxfh;
	u32 total_size;
	u32 indir_bytes;
	u32 *indir = NULL;
	u8 dev_hfunc = 0;
	u8 *hkey = NULL;
	u8 *rss_config;

	if (!ops->get_rxfh)
		return -EOPNOTSUPP;

	if (ops->get_rxfh_indir_size)
		dev_indir_size = ops->get_rxfh_indir_size(dev);
	if (ops->get_rxfh_key_size)
		dev_key_size = ops->get_rxfh_key_size(dev);

	if (copy_from_user(&rxfh, useraddr, sizeof(rxfh)))
		return -EFAULT;
	user_indir_size = rxfh.indir_size;
	user_key_size = rxfh.key_size;

	/* Check that reserved fields are 0 for now */
	if (rxfh.rss_context || rxfh.rsvd8[0] || rxfh.rsvd8[1] ||
	    rxfh.rsvd8[2] || rxfh.rsvd32)
		return -EINVAL;

	rxfh.indir_size = dev_indir_size;
	rxfh.key_size = dev_key_size;
	if (copy_to_user(useraddr, &rxfh, sizeof(rxfh)))
		return -EFAULT;

	if ((user_indir_size && (user_indir_size != dev_indir_size)) ||
	    (user_key_size && (user_key_size != dev_key_size)))
		return -EINVAL;

	indir_bytes = user_indir_size * sizeof(indir[0]);
	total_size = indir_bytes + user_key_size;
	rss_config = kzalloc(total_size, GFP_USER);
	if (!rss_config)
		return -ENOMEM;

	if (user_indir_size)
		indir = (u32 *)rss_config;

	if (user_key_size)
		hkey = rss_config + indir_bytes;

	ret = dev->ethtool_ops->get_rxfh(dev, indir, hkey, &dev_hfunc);
	if (ret)
		goto out;

	if (copy_to_user(useraddr + offsetof(struct ethtool_rxfh, hfunc),
			 &dev_hfunc, sizeof(rxfh.hfunc))) {
		ret = -EFAULT;
	} else if (copy_to_user(useraddr +
			      offsetof(struct ethtool_rxfh, rss_config[0]),
			      rss_config, total_size)) {
		ret = -EFAULT;
	}
out:
	kfree(rss_config);

	return ret;
}

static noinline_for_stack int ethtool_set_rxfh(struct net_device *dev,
					       void __user *useraddr)
{
	int ret;
	const struct ethtool_ops *ops = dev->ethtool_ops;
	struct ethtool_rxnfc rx_rings;
	struct ethtool_rxfh rxfh;
	u32 dev_indir_size = 0, dev_key_size = 0, i;
	u32 *indir = NULL, indir_bytes = 0;
	u8 *hkey = NULL;
	u8 *rss_config;
	u32 rss_cfg_offset = offsetof(struct ethtool_rxfh, rss_config[0]);

	if (!ops->get_rxnfc || !ops->set_rxfh)
		return -EOPNOTSUPP;

	if (ops->get_rxfh_indir_size)
		dev_indir_size = ops->get_rxfh_indir_size(dev);
	if (ops->get_rxfh_key_size)
		dev_key_size = ops->get_rxfh_key_size(dev);

	if (copy_from_user(&rxfh, useraddr, sizeof(rxfh)))
		return -EFAULT;

	/* Check that reserved fields are 0 for now */
	if (rxfh.rss_context || rxfh.rsvd8[0] || rxfh.rsvd8[1] ||
	    rxfh.rsvd8[2] || rxfh.rsvd32)
		return -EINVAL;

	/* If either indir, hash key or function is valid, proceed further.
	 * Must request at least one change: indir size, hash key or function.
	 */
	if ((rxfh.indir_size &&
	     rxfh.indir_size != ETH_RXFH_INDIR_NO_CHANGE &&
	     rxfh.indir_size != dev_indir_size) ||
	    (rxfh.key_size && (rxfh.key_size != dev_key_size)) ||
	    (rxfh.indir_size == ETH_RXFH_INDIR_NO_CHANGE &&
	     rxfh.key_size == 0 && rxfh.hfunc == ETH_RSS_HASH_NO_CHANGE))
		return -EINVAL;

	if (rxfh.indir_size != ETH_RXFH_INDIR_NO_CHANGE)
		indir_bytes = dev_indir_size * sizeof(indir[0]);

	rss_config = kzalloc(indir_bytes + rxfh.key_size, GFP_USER);
	if (!rss_config)
		return -ENOMEM;

	rx_rings.cmd = ETHTOOL_GRXRINGS;
	ret = ops->get_rxnfc(dev, &rx_rings, NULL);
	if (ret)
		goto out;

	/* rxfh.indir_size == 0 means reset the indir table to default.
	 * rxfh.indir_size == ETH_RXFH_INDIR_NO_CHANGE means leave it unchanged.
	 */
	if (rxfh.indir_size &&
	    rxfh.indir_size != ETH_RXFH_INDIR_NO_CHANGE) {
		indir = (u32 *)rss_config;
		ret = ethtool_copy_validate_indir(indir,
						  useraddr + rss_cfg_offset,
						  &rx_rings,
						  rxfh.indir_size);
		if (ret)
			goto out;
	} else if (rxfh.indir_size == 0) {
		indir = (u32 *)rss_config;
		for (i = 0; i < dev_indir_size; i++)
			indir[i] = ethtool_rxfh_indir_default(i, rx_rings.data);
	}

	if (rxfh.key_size) {
		hkey = rss_config + indir_bytes;
		if (copy_from_user(hkey,
				   useraddr + rss_cfg_offset + indir_bytes,
				   rxfh.key_size)) {
			ret = -EFAULT;
			goto out;
		}
	}

	ret = ops->set_rxfh(dev, indir, hkey, rxfh.hfunc);

out:
	kfree(rss_config);
	return ret;
}

static int ethtool_get_regs(struct net_device *dev, char __user *useraddr)
{
	struct ethtool_regs regs;
	const struct ethtool_ops *ops = dev->ethtool_ops;
	void *regbuf;
	int reglen, ret;

	if (!ops->get_regs || !ops->get_regs_len)
		return -EOPNOTSUPP;

	if (copy_from_user(&regs, useraddr, sizeof(regs)))
		return -EFAULT;

	reglen = ops->get_regs_len(dev);
	if (regs.len > reglen)
		regs.len = reglen;

	regbuf = NULL;
	if (reglen) {
		regbuf = vzalloc(reglen);
		if (!regbuf)
			return -ENOMEM;
	}

	ops->get_regs(dev, &regs, regbuf);

	ret = -EFAULT;
	if (copy_to_user(useraddr, &regs, sizeof(regs)))
		goto out;
	useraddr += offsetof(struct ethtool_regs, data);
	if (regbuf && copy_to_user(useraddr, regbuf, regs.len))
		goto out;
	ret = 0;

 out:
	vfree(regbuf);
	return ret;
}

static int ethtool_reset(struct net_device *dev, char __user *useraddr)
{
	struct ethtool_value reset;
	int ret;

	if (!dev->ethtool_ops->reset)
		return -EOPNOTSUPP;

	if (copy_from_user(&reset, useraddr, sizeof(reset)))
		return -EFAULT;

	ret = dev->ethtool_ops->reset(dev, &reset.data);
	if (ret)
		return ret;

	if (copy_to_user(useraddr, &reset, sizeof(reset)))
		return -EFAULT;
	return 0;
}

static int ethtool_get_wol(struct net_device *dev, char __user *useraddr)
{
	struct ethtool_wolinfo wol = { .cmd = ETHTOOL_GWOL };

	if (!dev->ethtool_ops->get_wol)
		return -EOPNOTSUPP;

	dev->ethtool_ops->get_wol(dev, &wol);

	if (copy_to_user(useraddr, &wol, sizeof(wol)))
		return -EFAULT;
	return 0;
}

static int ethtool_set_wol(struct net_device *dev, char __user *useraddr)
{
	struct ethtool_wolinfo wol;

	if (!dev->ethtool_ops->set_wol)
		return -EOPNOTSUPP;

	if (copy_from_user(&wol, useraddr, sizeof(wol)))
		return -EFAULT;

	return dev->ethtool_ops->set_wol(dev, &wol);
}

static int ethtool_get_eee(struct net_device *dev, char __user *useraddr)
{
	struct ethtool_eee edata;
	int rc;

	if (!dev->ethtool_ops->get_eee)
		return -EOPNOTSUPP;

	memset(&edata, 0, sizeof(struct ethtool_eee));
	edata.cmd = ETHTOOL_GEEE;
	rc = dev->ethtool_ops->get_eee(dev, &edata);

	if (rc)
		return rc;

	if (copy_to_user(useraddr, &edata, sizeof(edata)))
		return -EFAULT;

	return 0;
}

static int ethtool_set_eee(struct net_device *dev, char __user *useraddr)
{
	struct ethtool_eee edata;

	if (!dev->ethtool_ops->set_eee)
		return -EOPNOTSUPP;

	if (copy_from_user(&edata, useraddr, sizeof(edata)))
		return -EFAULT;

	return dev->ethtool_ops->set_eee(dev, &edata);
}

static int ethtool_nway_reset(struct net_device *dev)
{
	if (!dev->ethtool_ops->nway_reset)
		return -EOPNOTSUPP;

	return dev->ethtool_ops->nway_reset(dev);
}

static int ethtool_get_link(struct net_device *dev, char __user *useraddr)
{
	struct ethtool_value edata = { .cmd = ETHTOOL_GLINK };

	if (!dev->ethtool_ops->get_link)
		return -EOPNOTSUPP;

	edata.data = netif_running(dev) && dev->ethtool_ops->get_link(dev);

	if (copy_to_user(useraddr, &edata, sizeof(edata)))
		return -EFAULT;
	return 0;
}

static int ethtool_get_any_eeprom(struct net_device *dev, void __user *useraddr,
				  int (*getter)(struct net_device *,
						struct ethtool_eeprom *, u8 *),
				  u32 total_len)
{
	struct ethtool_eeprom eeprom;
	void __user *userbuf = useraddr + sizeof(eeprom);
	u32 bytes_remaining;
	u8 *data;
	int ret = 0;

	if (copy_from_user(&eeprom, useraddr, sizeof(eeprom)))
		return -EFAULT;

	/* Check for wrap and zero */
	if (eeprom.offset + eeprom.len <= eeprom.offset)
		return -EINVAL;

	/* Check for exceeding total eeprom len */
	if (eeprom.offset + eeprom.len > total_len)
		return -EINVAL;

	data = kmalloc(PAGE_SIZE, GFP_USER);
	if (!data)
		return -ENOMEM;

	bytes_remaining = eeprom.len;
	while (bytes_remaining > 0) {
		eeprom.len = min(bytes_remaining, (u32)PAGE_SIZE);

		ret = getter(dev, &eeprom, data);
		if (ret)
			break;
		if (copy_to_user(userbuf, data, eeprom.len)) {
			ret = -EFAULT;
			break;
		}
		userbuf += eeprom.len;
		eeprom.offset += eeprom.len;
		bytes_remaining -= eeprom.len;
	}

	eeprom.len = userbuf - (useraddr + sizeof(eeprom));
	eeprom.offset -= eeprom.len;
	if (copy_to_user(useraddr, &eeprom, sizeof(eeprom)))
		ret = -EFAULT;

	kfree(data);
	return ret;
}

static int ethtool_get_eeprom(struct net_device *dev, void __user *useraddr)
{
	const struct ethtool_ops *ops = dev->ethtool_ops;

	if (!ops->get_eeprom || !ops->get_eeprom_len ||
	    !ops->get_eeprom_len(dev))
		return -EOPNOTSUPP;

	return ethtool_get_any_eeprom(dev, useraddr, ops->get_eeprom,
				      ops->get_eeprom_len(dev));
}

static int ethtool_set_eeprom(struct net_device *dev, void __user *useraddr)
{
	struct ethtool_eeprom eeprom;
	const struct ethtool_ops *ops = dev->ethtool_ops;
	void __user *userbuf = useraddr + sizeof(eeprom);
	u32 bytes_remaining;
	u8 *data;
	int ret = 0;

	if (!ops->set_eeprom || !ops->get_eeprom_len ||
	    !ops->get_eeprom_len(dev))
		return -EOPNOTSUPP;

	if (copy_from_user(&eeprom, useraddr, sizeof(eeprom)))
		return -EFAULT;

	/* Check for wrap and zero */
	if (eeprom.offset + eeprom.len <= eeprom.offset)
		return -EINVAL;

	/* Check for exceeding total eeprom len */
	if (eeprom.offset + eeprom.len > ops->get_eeprom_len(dev))
		return -EINVAL;

	data = kmalloc(PAGE_SIZE, GFP_USER);
	if (!data)
		return -ENOMEM;

	bytes_remaining = eeprom.len;
	while (bytes_remaining > 0) {
		eeprom.len = min(bytes_remaining, (u32)PAGE_SIZE);

		if (copy_from_user(data, userbuf, eeprom.len)) {
			ret = -EFAULT;
			break;
		}
		ret = ops->set_eeprom(dev, &eeprom, data);
		if (ret)
			break;
		userbuf += eeprom.len;
		eeprom.offset += eeprom.len;
		bytes_remaining -= eeprom.len;
	}

	kfree(data);
	return ret;
}

static noinline_for_stack int ethtool_get_coalesce(struct net_device *dev,
						   void __user *useraddr)
{
	struct ethtool_coalesce coalesce = { .cmd = ETHTOOL_GCOALESCE };

	if (!dev->ethtool_ops->get_coalesce)
		return -EOPNOTSUPP;

	dev->ethtool_ops->get_coalesce(dev, &coalesce);

	if (copy_to_user(useraddr, &coalesce, sizeof(coalesce)))
		return -EFAULT;
	return 0;
}

static noinline_for_stack int ethtool_set_coalesce(struct net_device *dev,
						   void __user *useraddr)
{
	struct ethtool_coalesce coalesce;

	if (!dev->ethtool_ops->set_coalesce)
		return -EOPNOTSUPP;

	if (copy_from_user(&coalesce, useraddr, sizeof(coalesce)))
		return -EFAULT;

	return dev->ethtool_ops->set_coalesce(dev, &coalesce);
}

static int ethtool_get_ringparam(struct net_device *dev, void __user *useraddr)
{
	struct ethtool_ringparam ringparam = { .cmd = ETHTOOL_GRINGPARAM };

	if (!dev->ethtool_ops->get_ringparam)
		return -EOPNOTSUPP;

	dev->ethtool_ops->get_ringparam(dev, &ringparam);

	if (copy_to_user(useraddr, &ringparam, sizeof(ringparam)))
		return -EFAULT;
	return 0;
}

static int ethtool_set_ringparam(struct net_device *dev, void __user *useraddr)
{
	struct ethtool_ringparam ringparam;

	if (!dev->ethtool_ops->set_ringparam)
		return -EOPNOTSUPP;

	if (copy_from_user(&ringparam, useraddr, sizeof(ringparam)))
		return -EFAULT;

	return dev->ethtool_ops->set_ringparam(dev, &ringparam);
}

static noinline_for_stack int ethtool_get_channels(struct net_device *dev,
						   void __user *useraddr)
{
	struct ethtool_channels channels = { .cmd = ETHTOOL_GCHANNELS };

	if (!dev->ethtool_ops->get_channels)
		return -EOPNOTSUPP;

	dev->ethtool_ops->get_channels(dev, &channels);

	if (copy_to_user(useraddr, &channels, sizeof(channels)))
		return -EFAULT;
	return 0;
}

static noinline_for_stack int ethtool_set_channels(struct net_device *dev,
						   void __user *useraddr)
{
	struct ethtool_channels channels;

	if (!dev->ethtool_ops->set_channels)
		return -EOPNOTSUPP;

	if (copy_from_user(&channels, useraddr, sizeof(channels)))
		return -EFAULT;

	return dev->ethtool_ops->set_channels(dev, &channels);
}

static int ethtool_get_pauseparam(struct net_device *dev, void __user *useraddr)
{
	struct ethtool_pauseparam pauseparam = { ETHTOOL_GPAUSEPARAM };

	if (!dev->ethtool_ops->get_pauseparam)
		return -EOPNOTSUPP;

	dev->ethtool_ops->get_pauseparam(dev, &pauseparam);

	if (copy_to_user(useraddr, &pauseparam, sizeof(pauseparam)))
		return -EFAULT;
	return 0;
}

static int ethtool_set_pauseparam(struct net_device *dev, void __user *useraddr)
{
	struct ethtool_pauseparam pauseparam;

	if (!dev->ethtool_ops->set_pauseparam)
		return -EOPNOTSUPP;

	if (copy_from_user(&pauseparam, useraddr, sizeof(pauseparam)))
		return -EFAULT;

	return dev->ethtool_ops->set_pauseparam(dev, &pauseparam);
}

static int ethtool_self_test(struct net_device *dev, char __user *useraddr)
{
	struct ethtool_test test;
	const struct ethtool_ops *ops = dev->ethtool_ops;
	u64 *data;
	int ret, test_len;

	if (!ops->self_test || !ops->get_sset_count)
		return -EOPNOTSUPP;

	test_len = ops->get_sset_count(dev, ETH_SS_TEST);
	if (test_len < 0)
		return test_len;
	WARN_ON(test_len == 0);

	if (copy_from_user(&test, useraddr, sizeof(test)))
		return -EFAULT;

	test.len = test_len;
	data = kmalloc(test_len * sizeof(u64), GFP_USER);
	if (!data)
		return -ENOMEM;

	ops->self_test(dev, &test, data);

	ret = -EFAULT;
	if (copy_to_user(useraddr, &test, sizeof(test)))
		goto out;
	useraddr += sizeof(test);
	if (copy_to_user(useraddr, data, test.len * sizeof(u64)))
		goto out;
	ret = 0;

 out:
	kfree(data);
	return ret;
}

static int ethtool_get_strings(struct net_device *dev, void __user *useraddr)
{
	struct ethtool_gstrings gstrings;
	u8 *data;
	int ret;

	if (copy_from_user(&gstrings, useraddr, sizeof(gstrings)))
		return -EFAULT;

	ret = __ethtool_get_sset_count(dev, gstrings.string_set);
	if (ret < 0)
		return ret;

	gstrings.len = ret;

	data = kcalloc(gstrings.len, ETH_GSTRING_LEN, GFP_USER);
	if (!data)
		return -ENOMEM;

	__ethtool_get_strings(dev, gstrings.string_set, data);

	ret = -EFAULT;
	if (copy_to_user(useraddr, &gstrings, sizeof(gstrings)))
		goto out;
	useraddr += sizeof(gstrings);
	if (copy_to_user(useraddr, data, gstrings.len * ETH_GSTRING_LEN))
		goto out;
	ret = 0;

out:
	kfree(data);
	return ret;
}

static int ethtool_phys_id(struct net_device *dev, void __user *useraddr)
{
	struct ethtool_value id;
	static bool busy;
	const struct ethtool_ops *ops = dev->ethtool_ops;
	int rc;

	if (!ops->set_phys_id)
		return -EOPNOTSUPP;

	if (busy)
		return -EBUSY;

	if (copy_from_user(&id, useraddr, sizeof(id)))
		return -EFAULT;

	rc = ops->set_phys_id(dev, ETHTOOL_ID_ACTIVE);
	if (rc < 0)
		return rc;

	/* Drop the RTNL lock while waiting, but prevent reentry or
	 * removal of the device.
	 */
	busy = true;
	dev_hold(dev);
	rtnl_unlock();

	if (rc == 0) {
		/* Driver will handle this itself */
		schedule_timeout_interruptible(
			id.data ? (id.data * HZ) : MAX_SCHEDULE_TIMEOUT);
	} else {
		/* Driver expects to be called at twice the frequency in rc */
		int n = rc * 2, i, interval = HZ / n;

		/* Count down seconds */
		do {
			/* Count down iterations per second */
			i = n;
			do {
				rtnl_lock();
				rc = ops->set_phys_id(dev,
				    (i & 1) ? ETHTOOL_ID_OFF : ETHTOOL_ID_ON);
				rtnl_unlock();
				if (rc)
					break;
				schedule_timeout_interruptible(interval);
			} while (!signal_pending(current) && --i != 0);
		} while (!signal_pending(current) &&
			 (id.data == 0 || --id.data != 0));
	}

	rtnl_lock();
	dev_put(dev);
	busy = false;

	(void) ops->set_phys_id(dev, ETHTOOL_ID_INACTIVE);
	return rc;
}

static int ethtool_get_stats(struct net_device *dev, void __user *useraddr)
{
	struct ethtool_stats stats;
	const struct ethtool_ops *ops = dev->ethtool_ops;
	u64 *data;
	int ret, n_stats;

	if (!ops->get_ethtool_stats || !ops->get_sset_count)
		return -EOPNOTSUPP;

	n_stats = ops->get_sset_count(dev, ETH_SS_STATS);
	if (n_stats < 0)
		return n_stats;
	WARN_ON(n_stats == 0);

	if (copy_from_user(&stats, useraddr, sizeof(stats)))
		return -EFAULT;

	stats.n_stats = n_stats;
	data = kmalloc(n_stats * sizeof(u64), GFP_USER);
	if (!data)
		return -ENOMEM;

	ops->get_ethtool_stats(dev, &stats, data);

	ret = -EFAULT;
	if (copy_to_user(useraddr, &stats, sizeof(stats)))
		goto out;
	useraddr += sizeof(stats);
	if (copy_to_user(useraddr, data, stats.n_stats * sizeof(u64)))
		goto out;
	ret = 0;

 out:
	kfree(data);
	return ret;
}

static int ethtool_get_perm_addr(struct net_device *dev, void __user *useraddr)
{
	struct ethtool_perm_addr epaddr;

	if (copy_from_user(&epaddr, useraddr, sizeof(epaddr)))
		return -EFAULT;

	if (epaddr.size < dev->addr_len)
		return -ETOOSMALL;
	epaddr.size = dev->addr_len;

	if (copy_to_user(useraddr, &epaddr, sizeof(epaddr)))
		return -EFAULT;
	useraddr += sizeof(epaddr);
	if (copy_to_user(useraddr, dev->perm_addr, epaddr.size))
		return -EFAULT;
	return 0;
}

static int ethtool_get_value(struct net_device *dev, char __user *useraddr,
			     u32 cmd, u32 (*actor)(struct net_device *))
{
	struct ethtool_value edata = { .cmd = cmd };

	if (!actor)
		return -EOPNOTSUPP;

	edata.data = actor(dev);

	if (copy_to_user(useraddr, &edata, sizeof(edata)))
		return -EFAULT;
	return 0;
}

static int ethtool_set_value_void(struct net_device *dev, char __user *useraddr,
			     void (*actor)(struct net_device *, u32))
{
	struct ethtool_value edata;

	if (!actor)
		return -EOPNOTSUPP;

	if (copy_from_user(&edata, useraddr, sizeof(edata)))
		return -EFAULT;

	actor(dev, edata.data);
	return 0;
}

static int ethtool_set_value(struct net_device *dev, char __user *useraddr,
			     int (*actor)(struct net_device *, u32))
{
	struct ethtool_value edata;

	if (!actor)
		return -EOPNOTSUPP;

	if (copy_from_user(&edata, useraddr, sizeof(edata)))
		return -EFAULT;

	return actor(dev, edata.data);
}

static noinline_for_stack int ethtool_flash_device(struct net_device *dev,
						   char __user *useraddr)
{
	struct ethtool_flash efl;

	if (copy_from_user(&efl, useraddr, sizeof(efl)))
		return -EFAULT;

	if (!dev->ethtool_ops->flash_device)
		return -EOPNOTSUPP;

	efl.data[ETHTOOL_FLASH_MAX_FILENAME - 1] = 0;

	return dev->ethtool_ops->flash_device(dev, &efl);
}

static int ethtool_set_dump(struct net_device *dev,
			void __user *useraddr)
{
	struct ethtool_dump dump;

	if (!dev->ethtool_ops->set_dump)
		return -EOPNOTSUPP;

	if (copy_from_user(&dump, useraddr, sizeof(dump)))
		return -EFAULT;

	return dev->ethtool_ops->set_dump(dev, &dump);
}

static int ethtool_get_dump_flag(struct net_device *dev,
				void __user *useraddr)
{
	int ret;
	struct ethtool_dump dump;
	const struct ethtool_ops *ops = dev->ethtool_ops;

	if (!ops->get_dump_flag)
		return -EOPNOTSUPP;

	if (copy_from_user(&dump, useraddr, sizeof(dump)))
		return -EFAULT;

	ret = ops->get_dump_flag(dev, &dump);
	if (ret)
		return ret;

	if (copy_to_user(useraddr, &dump, sizeof(dump)))
		return -EFAULT;
	return 0;
}

static int ethtool_get_dump_data(struct net_device *dev,
				void __user *useraddr)
{
	int ret;
	__u32 len;
	struct ethtool_dump dump, tmp;
	const struct ethtool_ops *ops = dev->ethtool_ops;
	void *data = NULL;

	if (!ops->get_dump_data || !ops->get_dump_flag)
		return -EOPNOTSUPP;

	if (copy_from_user(&dump, useraddr, sizeof(dump)))
		return -EFAULT;

	memset(&tmp, 0, sizeof(tmp));
	tmp.cmd = ETHTOOL_GET_DUMP_FLAG;
	ret = ops->get_dump_flag(dev, &tmp);
	if (ret)
		return ret;

	len = min(tmp.len, dump.len);
	if (!len)
		return -EFAULT;

	/* Don't ever let the driver think there's more space available
	 * than it requested with .get_dump_flag().
	 */
	dump.len = len;

	/* Always allocate enough space to hold the whole thing so that the
	 * driver does not need to check the length and bother with partial
	 * dumping.
	 */
	data = vzalloc(tmp.len);
	if (!data)
		return -ENOMEM;
	ret = ops->get_dump_data(dev, &dump, data);
	if (ret)
		goto out;

	/* There are two sane possibilities:
	 * 1. The driver's .get_dump_data() does not touch dump.len.
	 * 2. Or it may set dump.len to how much it really writes, which
	 *    should be tmp.len (or len if it can do a partial dump).
	 * In any case respond to userspace with the actual length of data
	 * it's receiving.
	 */
	WARN_ON(dump.len != len && dump.len != tmp.len);
	dump.len = len;

	if (copy_to_user(useraddr, &dump, sizeof(dump))) {
		ret = -EFAULT;
		goto out;
	}
	useraddr += offsetof(struct ethtool_dump, data);
	if (copy_to_user(useraddr, data, len))
		ret = -EFAULT;
out:
	vfree(data);
	return ret;
}

static int ethtool_get_ts_info(struct net_device *dev, void __user *useraddr)
{
	int err = 0;
	struct ethtool_ts_info info;
	const struct ethtool_ops *ops = dev->ethtool_ops;
	struct phy_device *phydev = dev->phydev;

	memset(&info, 0, sizeof(info));
	info.cmd = ETHTOOL_GET_TS_INFO;

	if (phydev && phydev->drv && phydev->drv->ts_info) {
		err = phydev->drv->ts_info(phydev, &info);
	} else if (ops->get_ts_info) {
		err = ops->get_ts_info(dev, &info);
	} else {
		info.so_timestamping =
			SOF_TIMESTAMPING_RX_SOFTWARE |
			SOF_TIMESTAMPING_SOFTWARE;
		info.phc_index = -1;
	}

	if (err)
		return err;

	if (copy_to_user(useraddr, &info, sizeof(info)))
		err = -EFAULT;

	return err;
}

static int __ethtool_get_module_info(struct net_device *dev,
				     struct ethtool_modinfo *modinfo)
{
	const struct ethtool_ops *ops = dev->ethtool_ops;
	struct phy_device *phydev = dev->phydev;

	if (phydev && phydev->drv && phydev->drv->module_info)
		return phydev->drv->module_info(phydev, modinfo);

	if (ops->get_module_info)
		return ops->get_module_info(dev, modinfo);

	return -EOPNOTSUPP;
}

static int ethtool_get_module_info(struct net_device *dev,
				   void __user *useraddr)
{
	int ret;
	struct ethtool_modinfo modinfo;

	if (copy_from_user(&modinfo, useraddr, sizeof(modinfo)))
		return -EFAULT;

	ret = __ethtool_get_module_info(dev, &modinfo);
	if (ret)
		return ret;

	if (copy_to_user(useraddr, &modinfo, sizeof(modinfo)))
		return -EFAULT;

	return 0;
}

static int __ethtool_get_module_eeprom(struct net_device *dev,
				       struct ethtool_eeprom *ee, u8 *data)
{
	const struct ethtool_ops *ops = dev->ethtool_ops;
	struct phy_device *phydev = dev->phydev;

	if (phydev && phydev->drv && phydev->drv->module_eeprom)
		return phydev->drv->module_eeprom(phydev, ee, data);

	if (ops->get_module_eeprom)
		return ops->get_module_eeprom(dev, ee, data);

	return -EOPNOTSUPP;
}

static int ethtool_get_module_eeprom(struct net_device *dev,
				     void __user *useraddr)
{
	int ret;
	struct ethtool_modinfo modinfo;

	ret = __ethtool_get_module_info(dev, &modinfo);
	if (ret)
		return ret;

	return ethtool_get_any_eeprom(dev, useraddr,
				      __ethtool_get_module_eeprom,
				      modinfo.eeprom_len);
}

static int ethtool_tunable_valid(const struct ethtool_tunable *tuna)
{
	switch (tuna->id) {
	case ETHTOOL_RX_COPYBREAK:
	case ETHTOOL_TX_COPYBREAK:
		if (tuna->len != sizeof(u32) ||
		    tuna->type_id != ETHTOOL_TUNABLE_U32)
			return -EINVAL;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int ethtool_get_tunable(struct net_device *dev, void __user *useraddr)
{
	int ret;
	struct ethtool_tunable tuna;
	const struct ethtool_ops *ops = dev->ethtool_ops;
	void *data;

	if (!ops->get_tunable)
		return -EOPNOTSUPP;
	if (copy_from_user(&tuna, useraddr, sizeof(tuna)))
		return -EFAULT;
	ret = ethtool_tunable_valid(&tuna);
	if (ret)
		return ret;
	data = kmalloc(tuna.len, GFP_USER);
	if (!data)
		return -ENOMEM;
	ret = ops->get_tunable(dev, &tuna, data);
	if (ret)
		goto out;
	useraddr += sizeof(tuna);
	ret = -EFAULT;
	if (copy_to_user(useraddr, data, tuna.len))
		goto out;
	ret = 0;

out:
	kfree(data);
	return ret;
}

static int ethtool_set_tunable(struct net_device *dev, void __user *useraddr)
{
	int ret;
	struct ethtool_tunable tuna;
	const struct ethtool_ops *ops = dev->ethtool_ops;
	void *data;

	if (!ops->set_tunable)
		return -EOPNOTSUPP;
	if (copy_from_user(&tuna, useraddr, sizeof(tuna)))
		return -EFAULT;
	ret = ethtool_tunable_valid(&tuna);
	if (ret)
		return ret;
	data = kmalloc(tuna.len, GFP_USER);
	if (!data)
		return -ENOMEM;
	useraddr += sizeof(tuna);
	ret = -EFAULT;
	if (copy_from_user(data, useraddr, tuna.len))
		goto out;
	ret = ops->set_tunable(dev, &tuna, data);

out:
	kfree(data);
	return ret;
}

/* The main entry point in this file.  Called from net/core/dev_ioctl.c */

int dev_ethtool(struct net *net, struct ifreq *ifr)
{
	struct net_device *dev = __dev_get_by_name(net, ifr->ifr_name);
	void __user *useraddr = ifr->ifr_data;
	u32 ethcmd;
	int rc;
	netdev_features_t old_features;

	if (!dev || !netif_device_present(dev))
		return -ENODEV;

	if (copy_from_user(&ethcmd, useraddr, sizeof(ethcmd)))
		return -EFAULT;

	/* Allow some commands to be done by anyone */
	switch (ethcmd) {
	case ETHTOOL_GSET:
	case ETHTOOL_GDRVINFO:
	case ETHTOOL_GMSGLVL:
	case ETHTOOL_GLINK:
	case ETHTOOL_GCOALESCE:
	case ETHTOOL_GRINGPARAM:
	case ETHTOOL_GPAUSEPARAM:
	case ETHTOOL_GRXCSUM:
	case ETHTOOL_GTXCSUM:
	case ETHTOOL_GSG:
	case ETHTOOL_GSSET_INFO:
	case ETHTOOL_GSTRINGS:
	case ETHTOOL_GSTATS:
	case ETHTOOL_GTSO:
	case ETHTOOL_GPERMADDR:
	case ETHTOOL_GUFO:
	case ETHTOOL_GGSO:
	case ETHTOOL_GGRO:
	case ETHTOOL_GFLAGS:
	case ETHTOOL_GPFLAGS:
	case ETHTOOL_GRXFH:
	case ETHTOOL_GRXRINGS:
	case ETHTOOL_GRXCLSRLCNT:
	case ETHTOOL_GRXCLSRULE:
	case ETHTOOL_GRXCLSRLALL:
	case ETHTOOL_GRXFHINDIR:
	case ETHTOOL_GRSSH:
	case ETHTOOL_GFEATURES:
	case ETHTOOL_GCHANNELS:
	case ETHTOOL_GET_TS_INFO:
	case ETHTOOL_GEEE:
	case ETHTOOL_GTUNABLE:
		break;
	default:
		if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
			return -EPERM;
	}

	if (dev->ethtool_ops->begin) {
		rc = dev->ethtool_ops->begin(dev);
		if (rc  < 0)
			return rc;
	}
	old_features = dev->features;

	switch (ethcmd) {
	case ETHTOOL_GSET:
		rc = ethtool_get_settings(dev, useraddr);
		break;
	case ETHTOOL_SSET:
		rc = ethtool_set_settings(dev, useraddr);
		break;
	case ETHTOOL_GDRVINFO:
		rc = ethtool_get_drvinfo(dev, useraddr);
		break;
	case ETHTOOL_GREGS:
		rc = ethtool_get_regs(dev, useraddr);
		break;
	case ETHTOOL_GWOL:
		rc = ethtool_get_wol(dev, useraddr);
		break;
	case ETHTOOL_SWOL:
		rc = ethtool_set_wol(dev, useraddr);
		break;
	case ETHTOOL_GMSGLVL:
		rc = ethtool_get_value(dev, useraddr, ethcmd,
				       dev->ethtool_ops->get_msglevel);
		break;
	case ETHTOOL_SMSGLVL:
		rc = ethtool_set_value_void(dev, useraddr,
				       dev->ethtool_ops->set_msglevel);
		break;
	case ETHTOOL_GEEE:
		rc = ethtool_get_eee(dev, useraddr);
		break;
	case ETHTOOL_SEEE:
		rc = ethtool_set_eee(dev, useraddr);
		break;
	case ETHTOOL_NWAY_RST:
		rc = ethtool_nway_reset(dev);
		break;
	case ETHTOOL_GLINK:
		rc = ethtool_get_link(dev, useraddr);
		break;
	case ETHTOOL_GEEPROM:
		rc = ethtool_get_eeprom(dev, useraddr);
		break;
	case ETHTOOL_SEEPROM:
		rc = ethtool_set_eeprom(dev, useraddr);
		break;
	case ETHTOOL_GCOALESCE:
		rc = ethtool_get_coalesce(dev, useraddr);
		break;
	case ETHTOOL_SCOALESCE:
		rc = ethtool_set_coalesce(dev, useraddr);
		break;
	case ETHTOOL_GRINGPARAM:
		rc = ethtool_get_ringparam(dev, useraddr);
		break;
	case ETHTOOL_SRINGPARAM:
		rc = ethtool_set_ringparam(dev, useraddr);
		break;
	case ETHTOOL_GPAUSEPARAM:
		rc = ethtool_get_pauseparam(dev, useraddr);
		break;
	case ETHTOOL_SPAUSEPARAM:
		rc = ethtool_set_pauseparam(dev, useraddr);
		break;
	case ETHTOOL_TEST:
		rc = ethtool_self_test(dev, useraddr);
		break;
	case ETHTOOL_GSTRINGS:
		rc = ethtool_get_strings(dev, useraddr);
		break;
	case ETHTOOL_PHYS_ID:
		rc = ethtool_phys_id(dev, useraddr);
		break;
	case ETHTOOL_GSTATS:
		rc = ethtool_get_stats(dev, useraddr);
		break;
	case ETHTOOL_GPERMADDR:
		rc = ethtool_get_perm_addr(dev, useraddr);
		break;
	case ETHTOOL_GFLAGS:
		rc = ethtool_get_value(dev, useraddr, ethcmd,
					__ethtool_get_flags);
		break;
	case ETHTOOL_SFLAGS:
		rc = ethtool_set_value(dev, useraddr, __ethtool_set_flags);
		break;
	case ETHTOOL_GPFLAGS:
		rc = ethtool_get_value(dev, useraddr, ethcmd,
				       dev->ethtool_ops->get_priv_flags);
		break;
	case ETHTOOL_SPFLAGS:
		rc = ethtool_set_value(dev, useraddr,
				       dev->ethtool_ops->set_priv_flags);
		break;
	case ETHTOOL_GRXFH:
	case ETHTOOL_GRXRINGS:
	case ETHTOOL_GRXCLSRLCNT:
	case ETHTOOL_GRXCLSRULE:
	case ETHTOOL_GRXCLSRLALL:
		rc = ethtool_get_rxnfc(dev, ethcmd, useraddr);
		break;
	case ETHTOOL_SRXFH:
	case ETHTOOL_SRXCLSRLDEL:
	case ETHTOOL_SRXCLSRLINS:
		rc = ethtool_set_rxnfc(dev, ethcmd, useraddr);
		break;
	case ETHTOOL_FLASHDEV:
		rc = ethtool_flash_device(dev, useraddr);
		break;
	case ETHTOOL_RESET:
		rc = ethtool_reset(dev, useraddr);
		break;
	case ETHTOOL_GSSET_INFO:
		rc = ethtool_get_sset_info(dev, useraddr);
		break;
	case ETHTOOL_GRXFHINDIR:
		rc = ethtool_get_rxfh_indir(dev, useraddr);
		break;
	case ETHTOOL_SRXFHINDIR:
		rc = ethtool_set_rxfh_indir(dev, useraddr);
		break;
	case ETHTOOL_GRSSH:
		rc = ethtool_get_rxfh(dev, useraddr);
		break;
	case ETHTOOL_SRSSH:
		rc = ethtool_set_rxfh(dev, useraddr);
		break;
	case ETHTOOL_GFEATURES:
		rc = ethtool_get_features(dev, useraddr);
		break;
	case ETHTOOL_SFEATURES:
		rc = ethtool_set_features(dev, useraddr);
		break;
	case ETHTOOL_GTXCSUM:
	case ETHTOOL_GRXCSUM:
	case ETHTOOL_GSG:
	case ETHTOOL_GTSO:
	case ETHTOOL_GUFO:
	case ETHTOOL_GGSO:
	case ETHTOOL_GGRO:
		rc = ethtool_get_one_feature(dev, useraddr, ethcmd);
		break;
	case ETHTOOL_STXCSUM:
	case ETHTOOL_SRXCSUM:
	case ETHTOOL_SSG:
	case ETHTOOL_STSO:
	case ETHTOOL_SUFO:
	case ETHTOOL_SGSO:
	case ETHTOOL_SGRO:
		rc = ethtool_set_one_feature(dev, useraddr, ethcmd);
		break;
	case ETHTOOL_GCHANNELS:
		rc = ethtool_get_channels(dev, useraddr);
		break;
	case ETHTOOL_SCHANNELS:
		rc = ethtool_set_channels(dev, useraddr);
		break;
	case ETHTOOL_SET_DUMP:
		rc = ethtool_set_dump(dev, useraddr);
		break;
	case ETHTOOL_GET_DUMP_FLAG:
		rc = ethtool_get_dump_flag(dev, useraddr);
		break;
	case ETHTOOL_GET_DUMP_DATA:
		rc = ethtool_get_dump_data(dev, useraddr);
		break;
	case ETHTOOL_GET_TS_INFO:
		rc = ethtool_get_ts_info(dev, useraddr);
		break;
	case ETHTOOL_GMODULEINFO:
		rc = ethtool_get_module_info(dev, useraddr);
		break;
	case ETHTOOL_GMODULEEEPROM:
		rc = ethtool_get_module_eeprom(dev, useraddr);
		break;
	case ETHTOOL_GTUNABLE:
		rc = ethtool_get_tunable(dev, useraddr);
		break;
	case ETHTOOL_STUNABLE:
		rc = ethtool_set_tunable(dev, useraddr);
		break;
	default:
		rc = -EOPNOTSUPP;
	}

	if (dev->ethtool_ops->complete)
		dev->ethtool_ops->complete(dev);

	if (old_features != dev->features)
		netdev_features_change(dev);

	return rc;
}
