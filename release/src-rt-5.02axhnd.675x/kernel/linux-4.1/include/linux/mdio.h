/*
 * linux/mdio.h: definitions for MDIO (clause 45) transceivers
 * Copyright 2006-2009 Solarflare Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, incorporated herein by reference.
 */
#ifndef __LINUX_MDIO_H__
#define __LINUX_MDIO_H__

#include <uapi/linux/mdio.h>


static inline bool mdio_phy_id_is_c45(int phy_id)
{
	return (phy_id & MDIO_PHY_ID_C45) && !(phy_id & ~MDIO_PHY_ID_C45_MASK);
}

static inline __u16 mdio_phy_id_prtad(int phy_id)
{
	return (phy_id & MDIO_PHY_ID_PRTAD) >> 5;
}

static inline __u16 mdio_phy_id_devad(int phy_id)
{
	return phy_id & MDIO_PHY_ID_DEVAD;
}

/**
 * struct mdio_if_info - Ethernet controller MDIO interface
 * @prtad: PRTAD of the PHY (%MDIO_PRTAD_NONE if not present/unknown)
 * @mmds: Mask of MMDs expected to be present in the PHY.  This must be
 *	non-zero unless @prtad = %MDIO_PRTAD_NONE.
 * @mode_support: MDIO modes supported.  If %MDIO_SUPPORTS_C22 is set then
 *	MII register access will be passed through with @devad =
 *	%MDIO_DEVAD_NONE.  If %MDIO_EMULATE_C22 is set then access to
 *	commonly used clause 22 registers will be translated into
 *	clause 45 registers.
 * @dev: Net device structure
 * @mdio_read: Register read function; returns value or negative error code
 * @mdio_write: Register write function; returns 0 or negative error code
 */
struct mdio_if_info {
	int prtad;
	u32 mmds;
	unsigned mode_support;

	struct net_device *dev;
	int (*mdio_read)(struct net_device *dev, int prtad, int devad,
			 u16 addr);
	int (*mdio_write)(struct net_device *dev, int prtad, int devad,
			  u16 addr, u16 val);
};

#define MDIO_PRTAD_NONE			(-1)
#define MDIO_DEVAD_NONE			(-1)
#define MDIO_SUPPORTS_C22		1
#define MDIO_SUPPORTS_C45		2
#define MDIO_EMULATE_C22		4

struct ethtool_cmd;
struct ethtool_pauseparam;
extern int mdio45_probe(struct mdio_if_info *mdio, int prtad);
extern int mdio_set_flag(const struct mdio_if_info *mdio,
			 int prtad, int devad, u16 addr, int mask,
			 bool sense);
extern int mdio45_links_ok(const struct mdio_if_info *mdio, u32 mmds);
extern int mdio45_nway_restart(const struct mdio_if_info *mdio);
extern void mdio45_ethtool_gset_npage(const struct mdio_if_info *mdio,
				      struct ethtool_cmd *ecmd,
				      u32 npage_adv, u32 npage_lpa);

/**
 * mdio45_ethtool_gset - get settings for ETHTOOL_GSET
 * @mdio: MDIO interface
 * @ecmd: Ethtool request structure
 *
 * Since the CSRs for auto-negotiation using next pages are not fully
 * standardised, this function does not attempt to decode them.  Use
 * mdio45_ethtool_gset_npage() to specify advertisement bits from next
 * pages.
 */
static inline void mdio45_ethtool_gset(const struct mdio_if_info *mdio,
				       struct ethtool_cmd *ecmd)
{
	mdio45_ethtool_gset_npage(mdio, ecmd, 0, 0);
}

extern int mdio_mii_ioctl(const struct mdio_if_info *mdio,
			  struct mii_ioctl_data *mii_data, int cmd);

/**
 * mmd_eee_cap_to_ethtool_sup_t
 * @eee_cap: value of the MMD EEE Capability register
 *
 * A small helper function that translates MMD EEE Capability (3.20) bits
 * to ethtool supported settings.
 */
static inline u32 mmd_eee_cap_to_ethtool_sup_t(u16 eee_cap)
{
	u32 supported = 0;

	if (eee_cap & MDIO_EEE_100TX)
		supported |= SUPPORTED_100baseT_Full;
	if (eee_cap & MDIO_EEE_1000T)
		supported |= SUPPORTED_1000baseT_Full;
	if (eee_cap & MDIO_EEE_10GT)
		supported |= SUPPORTED_10000baseT_Full;
	if (eee_cap & MDIO_EEE_1000KX)
		supported |= SUPPORTED_1000baseKX_Full;
	if (eee_cap & MDIO_EEE_10GKX4)
		supported |= SUPPORTED_10000baseKX4_Full;
	if (eee_cap & MDIO_EEE_10GKR)
		supported |= SUPPORTED_10000baseKR_Full;

	return supported;
}

/**
 * mmd_eee_adv_to_ethtool_adv_t
 * @eee_adv: value of the MMD EEE Advertisement/Link Partner Ability registers
 *
 * A small helper function that translates the MMD EEE Advertisment (7.60)
 * and MMD EEE Link Partner Ability (7.61) bits to ethtool advertisement
 * settings.
 */
static inline u32 mmd_eee_adv_to_ethtool_adv_t(u16 eee_adv)
{
	u32 adv = 0;

	if (eee_adv & MDIO_EEE_100TX)
		adv |= ADVERTISED_100baseT_Full;
	if (eee_adv & MDIO_EEE_1000T)
		adv |= ADVERTISED_1000baseT_Full;
	if (eee_adv & MDIO_EEE_10GT)
		adv |= ADVERTISED_10000baseT_Full;
	if (eee_adv & MDIO_EEE_1000KX)
		adv |= ADVERTISED_1000baseKX_Full;
	if (eee_adv & MDIO_EEE_10GKX4)
		adv |= ADVERTISED_10000baseKX4_Full;
	if (eee_adv & MDIO_EEE_10GKR)
		adv |= ADVERTISED_10000baseKR_Full;

	return adv;
}

/**
 * ethtool_adv_to_mmd_eee_adv_t
 * @adv: the ethtool advertisement settings
 *
 * A small helper function that translates ethtool advertisement settings
 * to EEE advertisements for the MMD EEE Advertisement (7.60) and
 * MMD EEE Link Partner Ability (7.61) registers.
 */
static inline u16 ethtool_adv_to_mmd_eee_adv_t(u32 adv)
{
	u16 reg = 0;

	if (adv & ADVERTISED_100baseT_Full)
		reg |= MDIO_EEE_100TX;
	if (adv & ADVERTISED_1000baseT_Full)
		reg |= MDIO_EEE_1000T;
	if (adv & ADVERTISED_10000baseT_Full)
		reg |= MDIO_EEE_10GT;
	if (adv & ADVERTISED_1000baseKX_Full)
		reg |= MDIO_EEE_1000KX;
	if (adv & ADVERTISED_10000baseKX4_Full)
		reg |= MDIO_EEE_10GKX4;
	if (adv & ADVERTISED_10000baseKR_Full)
		reg |= MDIO_EEE_10GKR;

	return reg;
}

#endif /* __LINUX_MDIO_H__ */
