/*
 * Copyright 2015, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <net/if.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <syslog.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/mii.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>

#include <bcmnvram.h>
#include <shutils.h>
#include <bcmdevs.h>
#include <net/route.h>
#include <bcmdevs.h>

#include <linux/if_ether.h>
#include <linux/sockios.h>

#if defined(RTCONFIG_QCA)
#include <qca.h>
#endif

#include "shared.h"

#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2) || \
    defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2) || \
		defined(RTCONFIG_EXT_RTL8370MB)
/**
 * @ifname:
 * @location:
 * @return:
 *  < 0:	invalid parameter or failed.
 *  >= 0:	success, register content
 */
int mdio_read(char *ifname, int location)
{
	int fd, ret = 0;
	struct ifreq ifr;
	struct mii_ioctl_data *mii = (struct mii_ioctl_data *)&ifr.ifr_data;

	if (!ifname || *ifname == '\0') {
		_dprintf("%s: Invalid ifname.\n", __func__);
		return -1;
	}
	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		return -2;
	memset(&ifr, 0, sizeof(ifr));
	strlcpy(ifr.ifr_name, ifname, IFNAMSIZ);
	mii->reg_num = location;
	if (ioctl(fd, SIOCGMIIPHY, &ifr) < 0) {
		ret = -3;
		goto exit_mdio_read;
	}
#if defined(RTCONFIG_WIFI_QCA9990_QCA9990)
	mii->reg_num = location;
	if (ioctl(fd, SIOCGMIIREG, &ifr) < 0) {
		ret = -4;
		goto exit_mdio_read;
	}
#endif
	ret = mii->val_out;

exit_mdio_read:
	close(fd);

	return ret;
}

/**
 * @ifname:
 * @location:
 * @return:
 *  < 0:	invalid parameter or failed.
 *  = 0:	success
 */
int mdio_write(char *ifname, int location, int value)
{
	int fd, ret = 0;
	struct ifreq ifr;
	struct mii_ioctl_data *mii = (struct mii_ioctl_data *)&ifr.ifr_data;

	if (!ifname || *ifname == '\0' || value < 0) {
		_dprintf("%s: Invalid parameter.\n", __func__);
		return -1;
	}
	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		return -2;
	memset(&ifr, 0, sizeof(ifr));
	strlcpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(fd, SIOCGMIIPHY, &ifr) < 0) {
		ret = -3;
		goto exit_mdio_write;
	}
	mii->reg_num = location;
	mii->val_in = value;
	if (ioctl(fd, SIOCSMIIREG, &ifr) < 0) {
		ret = -4;
		goto exit_mdio_write;
	}

exit_mdio_write:
	close(fd);

	return ret;
}

int mdio_phy_speed(char *ifname)
{
	int bmcr, bmcr2, bmsr, advert, lkpar, lpa2;
	int mask, mask2;

	if (!ifname || *ifname == '\0')
		return 0;

	bmcr = mdio_read(ifname, MII_BMCR);
	bmcr2 = mdio_read(ifname, MII_CTRL1000);
	bmsr = mdio_read(ifname, MII_BMSR);
	advert = mdio_read(ifname, MII_ADVERTISE);
	lkpar = mdio_read(ifname, MII_LPA);
	lpa2 = mdio_read(ifname, MII_STAT1000);

	if (!(bmcr & BMCR_ANENABLE)) {
		if (bmcr & BMCR_SPEED1000)
			return 1000;
		else if (bmcr & BMCR_SPEED100)
			return 100;
		else
			return 10;
	}

	if (!(bmsr & BMSR_ANEGCOMPLETE)) {
		// _dprintf("%s: %s autonegotiation restarted\n", __func__, ifname);
		return 0;
	}

	if (!(advert & lkpar)) {
		// _dprintf("%s: %s autonegotiation failed\n", __func__, ifname);
		return 0;
	}

	mask = advert & lkpar;
	mask2 = bmcr2 & (lpa2>>2);
	if (mask2 & (ADVERTISE_1000FULL | ADVERTISE_1000HALF))
		return 1000;
	else if (mask & (ADVERTISE_100FULL | ADVERTISE_100HALF))
		return 100;
	else if (mask & (ADVERTISE_10FULL | ADVERTISE_10HALF))
		return 10;

	return 0;
}
#endif

#if defined(RTCONFIG_QCA) && defined(RTCONFIG_SWITCH_QCA8075_QCA8337_PHY_AQR107_AR8035_QCA8033)
/**
 * Parse ssdk_sh result.
 * NOTE:	Caller have to specify switch id in command-line.
 * @return:
 * 	0:	success
 *  otherwise:	fail
 */
int parse_ssdk_sh(const char *cmd, const char *fmt, int cnt, ...)
{
	int r, ret = 1;
	char line[256];
	FILE *fp;
	va_list args;

	if (!cmd || cnt <= 0 || !fmt)
		return -1;

	if (!(fp = popen(cmd, "r"))) {
		dbg("%s: can't execute [%s]\n", __func__, cmd);
		return -2;
	}

	va_start(args, cnt);
	while (ret && fgets(line, sizeof(line), fp)) {
		if (!strstr(line, "SSDK Init OK!["))
			continue;
		if ((r = vsscanf(line, fmt, args)) != cnt) {
			dbg("%s: Unknown output: [%s] of cmd [%s], fmt [%s], cnt %d, r %d\n",
				__func__, line, cmd, fmt, cnt, r);
			continue;
		}
		ret = 0;
	}
	va_end(args);
	pclose(fp);

	return ret;
}

/**
 * Read register of PHY via "ssdk_sh debug phy get" command.
 * @phy:	PHY address
 * @reg:	register address
 * @return:
 *    < 0:	error
 *    >=0:	lowest 16-bits are register value.
 */
int read_phy_reg(unsigned int phy, unsigned int reg)
{
	FILE *fp;
	int r = 0;
	unsigned int val;
	char cmd[sizeof("ssdk_sh swX debug phy get 0xPP 0xFFFFFFFFXXXXXX")], line[256];

	snprintf(cmd, sizeof(cmd), "ssdk_sh %s debug phy get 0x%x 0x%x", SWID_IPQ807X, phy, reg);
	if (!(fp = popen(cmd, "r"))) {
		dbg("%s: Run [%s] failed! errno %d (%s)\n",
			__func__, cmd, errno, strerror(errno));
		return -1;
	}

	/* Example:
	 * / # ssdk_sh debug phy get 9 1
	 *
	 *  SSDK Init OK![Data]:0x7949
	 * operation done.
	 *
	 */
	while (fgets(line, sizeof(line), fp)) {
		if (!strstr(line, "SSDK Init OK"))
			continue;
		if ((r = sscanf(line, "%*[^:]:%x", &val)) != 1) {
			_dprintf("%s: Unknown output: [%s]\n", __func__, line);
			continue;
		}
		r = val & 0xFFFF;
		break;
	}
	pclose(fp);

	return r;
}

/**
 * Write to register of PHY via "ssdk_sh debug phy set" command.
 * @phy:	PHY address
 * @reg:	register address
 * @return:
 * 	0:	fail
 *      1:	success
 */
int write_phy_reg(unsigned int phy, unsigned int reg, unsigned int value)
{
	FILE *fp;
	int r = 0;
	char cmd[sizeof("ssdk_sh swX debug phy set 0xPP 0xFFFFFFFFXXXXXX")], line[256];

	snprintf(cmd, sizeof(cmd), "ssdk_sh %s debug phy set 0x%x 0x%x 0x%x", SWID_IPQ807X, phy, reg, value);
	if (!(fp = popen(cmd, "r"))) {
		dbg("%s: Run [%s] failed! errno %d (%s)\n",
			__func__, cmd, errno, strerror(errno));
		return 0;
	}

	/* Example:
	 * / # ssdk_sh debug phy set 6 0 0x3900
	 *
	 *  SSDK Init OK!
	 * operation done.
	 */
	while (fgets(line, sizeof(line), fp)) {
		if (!strstr(line, "SSDK Init OK"))
			continue;
		r = 1;
		break;
	}
	pclose(fp);

	return r;
}

/**
 * Return speed of @phy.
 * @phy:	PHY address
 * @return:	speed of @phy
 *  1000:	1000Mbps
 *   100:	100Mbps
 *    10:	10Mbps
 *     0:	no link or error.
 */
int mdio_phy_speed(unsigned int phy)
{
	int bmcr, bmcr2, bmsr, advert, lkpar, lpa2;
	int mask, mask2;

	if ((bmsr = read_phy_reg(phy, MII_BMSR)) < 0)
		bmsr = 0;
	if (!(bmsr & BMSR_LSTATUS))
		return 0;

	if ((bmcr = read_phy_reg(phy, MII_BMCR)) < 0)
		bmcr = 0;
	if (!(bmcr & BMCR_ANENABLE)) {
		if (bmcr & BMCR_SPEED1000)
			return 1000;
		else if (bmcr & BMCR_SPEED100)
			return 100;
		else
			return 10;
	}

	if (!(bmsr & BMSR_ANEGCOMPLETE)) {
		dbg("%s: phy 0x%x: autonegotiation restarted\n", __func__, phy);
		return 0;
	}

	if ((advert = read_phy_reg(phy, MII_ADVERTISE)) < 0)
		advert = 0;
	if ((lkpar = read_phy_reg(phy, MII_LPA)) < 0)
		lkpar = 0;
	if (!(advert & lkpar)) {
		dbg("%s: phy 0x%x: autonegotiation failed\n", __func__, phy);
		return 0;
	}

	if ((bmcr2 = read_phy_reg(phy, MII_CTRL1000)) < 0)
		bmcr2 = 0;
	if ((lpa2 = read_phy_reg(phy, MII_STAT1000)) < 0)
		lpa2 = 0;
	mask = advert & lkpar;
	mask2 = bmcr2 & (lpa2>>2);
	if (mask2 & (ADVERTISE_1000FULL | ADVERTISE_1000HALF))
		return 1000;
	else if (mask & (ADVERTISE_100FULL | ADVERTISE_100HALF))
		return 100;
	else if (mask & (ADVERTISE_10FULL | ADVERTISE_10HALF))
		return 10;

	return 0;
}

/**
 * Helper function of qca8337_port_speed() and ipq8074_port_speed()
 * @port:	port of QCA8337
 * @return:	speed of @phy
 *  1000:	1000Mbps
 *   100:	100Mbps
 *    10:	10Mbps
 *     0:	no link or error.
 *    -1:	auto negotiation is not completed.
 */
static int __ssdk_sh_port_speed(const char *swid, unsigned int port)
{
	int r, speed = 0;
	char linkstatus_cmd[sizeof("ssdk_sh sw1 port linkstatus get XYYYYYY")], linkstatus[sizeof("DISABLEXXX")] = { 0 };
	char speed_cmd[sizeof("ssdk_sh sw1 port speed get XYYYYYY")];

	if (!swid)
		return 0;

	snprintf(linkstatus_cmd, sizeof(linkstatus_cmd), "ssdk_sh %s port linkstatus get %d", swid, port);
	/* Example:
	 * / # ssdk_sh sw1 port linkstatus get 5
	 *
	 *  SSDK Init OK![Status]:DISABLE
	 * operation done.
	 *
	 * / # ssdk_sh sw1 port linkstatus get 6
	 *
	 *  SSDK Init OK![Status]:ENABLE
	 * operation done.
	 */
	if ((r = parse_ssdk_sh(linkstatus_cmd, "%*[^:]:%7s", 1, linkstatus)) != 0)
		return 0;
	if (strcmp(linkstatus, "ENABLE"))
		return 0;

	snprintf(speed_cmd, sizeof(speed_cmd), "ssdk_sh %s port speed get %d", swid, port);
	/* Example:
	 * / # ssdk_sh sw1 port speed get 6
	 *
	 *  SSDK Init OK![speed]:1000(Mbps)
	 * operation done.
	 */
	if ((r = parse_ssdk_sh(speed_cmd, "%*[^:]:%d(Mbps)", 1, &speed)) != 0)
		return 0;
	if (speed != 10 && speed != 100 && speed != 1000)
		return 0;

	return speed;
}

/**
 * Return speed of IPQ8074 port
 * @port:	port of IPQ8074
 * @return:	speed of @phy
 *  1000:	1000Mbps
 *   100:	100Mbps
 *    10:	10Mbps
 *     0:	no link or error.
 *    -1:	auto negotiation is not completed.
 */
int ipq8074_port_speed(unsigned int port)
{
#if defined(RTCONFIG_SWITCH_QCA8075_QCA8337_PHY_AQR107_AR8035_QCA8033)
	const int min_port_nr = 2, max_port_nr = 4;
#else
	const int min_port_nr = 1, max_port_nr = 5;
#endif

	if (port < min_port_nr ||  port > max_port_nr) {
		dbg("%s: invalid port %d\n", __func__, port);
		return 0;
	}

	return __ssdk_sh_port_speed(SWID_IPQ807X, port);
}

/**
 * Return speed of QCA8337 port
 * @port:	port of QCA8337
 * @return:	speed of @phy
 *  1000:	1000Mbps
 *   100:	100Mbps
 *    10:	10Mbps
 *     0:	no link or error.
 *    -1:	auto negotiation is not completed.
 */
int qca8337_port_speed(unsigned int port)
{
#if defined(RTCONFIG_SWITCH_QCA8075_QCA8337_PHY_AQR107_AR8035_QCA8033)
	/* P6 is connected to AR8033 PHY and it's speed is always 1G.
	 * Don't use this function to check P6 of QCA8337.
	 */
	const int max_port_nr = 5;
#else
	const int max_port_nr = 6;
#endif

	if (!port || port > max_port_nr) {
		dbg("%s: invalid port %d\n", __func__, port);
		return 0;
	}

	return __ssdk_sh_port_speed(SWID_QCA8337, port);
}

/**
 * Return speed of Aquentia AQR107 PHY.
 * @phy:	PHY address
 * @return:	speed of @phy
 * 10000:	10Gbps
 *  5000:	5Gbps
 *  2500:	2.5Gbps
 *  1000:	1000Mbps
 *   100:	100Mbps
 *    10:	10Mbps
 *     0:	no link or error.
 *    -1:	auto negotiation is not completed.
 */
int aqr_phy_speed(unsigned int phy)
{
	int xs_status, prvs1, avs1, ret = 0;
	int speed, aneg_st;

	if ((xs_status = read_phy_reg(phy, 0x4004E812)) < 0)
		xs_status = 0;
	aneg_st = (xs_status >> 0xE) & 3;

	if (aneg_st != 2) {
		/* If auto negotiation status (AQR PHY and IPQ807X XGMAC) is not completed,
		 * network is not ok even link up.  Notifiy this case and assume it is not link up.
		 */
		dbg("%s: AQR PHY 0x%x: Auto negotiation status is not completed! (status %d)\n",
			__func__, phy, aneg_st);
		return -1;
	}

	/* PMA Receive Vendor State 1: 1.E800 */
	if ((prvs1 = read_phy_reg(phy, 0x4001E800)) < 0)
		prvs1 = 0;

	/* Check link status */
	if (!(prvs1 & 1))
		return 0;

	/* Autonegotiation Vendor Status 1: 7.C800 */
	if ((avs1 = read_phy_reg(phy, 0x4007C800)) < 0)
		avs1 = 0;
	speed = (avs1 >> 1) & 7;

	switch (speed) {
		case 0:
			ret = 10;
			break;
		case 1:
			ret = 100;
			break;
		case 2:
			ret = 1000;
			break;
		case 3:
			ret = 10000;
			break;
		case 4:
			ret = 2500;
			break;
		case 5:
			ret = 5000;
			break;
		default:
			dbg("%s: AQR PHY 0x%x: unknown speed 0x%x\n", __func__, speed);
			break;
	}

	return ret;
}
#endif	/* RTCONFIG_QCA && RTCONFIG_SWITCH_QCA8075_QCA8337_PHY_AQR107_AR8035_QCA8033 */
