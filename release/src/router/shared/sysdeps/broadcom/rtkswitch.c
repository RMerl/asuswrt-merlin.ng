/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <bcmnvram.h>
#include <ethernet.h>

#include <utils.h>
#include <shutils.h>
#include <shared.h>

#define RTKSWITCH_DEV	"/dev/rtkswitch"

typedef struct {
	uint32	count;
	struct	ether_addr ea[256];
} mactable;

int rtkswitch_port_stat(int port);
int rtkswitch_port_mactable(int port);

int rtkswitch_ioctl(int val, int val2)
{
	int fd;
	int value = 0;
	int *p = NULL;

	fd = open(RTKSWITCH_DEV, O_RDONLY);
	if (fd < 0) {
		perror(RTKSWITCH_DEV);
		return -1;
	}

	switch (val) {
#if 0
	case 0:		/* Get LAN phy status of all LAN ports */
#endif
	case 1:		/* Dump all counters of the specified LAN port */
			return rtkswitch_port_stat(val2 + 1);
	case 100:	/* set reset pin to 0 */
	case 101:	/* set reset pin to 1 */
	case 7:		/* Dump L2 lookup table of specified LAN port */
			return rtkswitch_port_mactable(val2 + 1);
	case 2:		/* Dump MAC status of specified LAN port */
	case 3:		/* Get link status of the specified LAN port */
	case 4:		/* Get link status of LAN ports */
	case 9:		/* LAN / WAN partition */
	case 10:	/* power up LAN1 */
	case 11:	/* power down LAN1 */
	case 12:	/* power up specified LAN port */
	case 13:	/* power down specified LAN port */
	case 17:	/* disable jumbo frame */
	case 18:	/* enable jumbo frame */
#if 0
	case 22:
	case 23:
	case 24:
	case 25:
	case 34:	/* Set VoIP port. Cherry Cho added in 2011/6/30. */
	case 35:
	case 99:
	case 100:
#endif
	case 36:	/* Set Vlan VID. */
	case 37:	/* Set Vlan PRIO. */
	case 38:	/* Initialize VLAN. Cherry Cho added in 2011/7/15. */
	case 380:	/* Set port VlanFilter */
	case 381:	/* Get port VlanFilter */
	case 39:	/* Create VLAN. Cherry Cho added in 2011/7/15. need to specify vlanid first, by 36 */
	case 390:	/* Create VLAN w/o pvid. need to specify vlanid first, by 36 */
	case 3900:	/* Create VLAN w/o pvid, w/o cpu port, need to specify vlanid first, by 36 */
	case 3901:	/* reset VLAN. need to specify vlanid first, by 36 */
	case 3902:	/* Create VLAN w/o setting pvid, w/ cpu ports in mbr/untag , need to specify vlanid first, by 36 */
	case 391:	/* Set specified port PVID,PRIV, need to specify vlanid first, by 36 */
	case 3911:	/* Set specified ports PVID, need to specify vlanid first, by 36 */
	case 392:	/* Get specified port PVID */
	case 393:	/* Get all ports' PVID */
	case 395:	/* Reset all ports accept type as all */
	case 3951:	/* Reset ports accept type as all */
	case 3952:	/* Reset ports accept type as tag-only */
	case 3953:	/* Reset ports accept type as untag-only */
	case 396:	/* Dump all ports accept type */
	case 397:	/* Set port frame type */
	//case 398:	/* Get fwd/efid */
	case 399:	/* Dump vlan untag,fwd, need to specify vlanid first, by 36 */
	case 40:        /* set static is_singtel_mio */
	case 401:       /* set static rtk_led_group */
	case 4021:      /* Set Led operation mode */
	case 4031:      /* Set LED blinking rate */
	case 4041:      /* Set per group Led to congiuration mode */
	case 4051:      /* Set Led group to congiuration force mode */
	case 4350:      /* set specified led port group */
	case 4352:      /* set port_group-x's enabled mask */
	case 46:	/* power up specified LAN port */
	case 47:	/* power down specified LAN port */
	case 48:	/* dump serdes registers */
	case 51:	/* set FlowControlJumboMode */
	case 53:	/* set Jumbo threhsold(enable/disable) */
	case 55:	/* set Jumbo size for Jumbo mode flow control */
	case 61:	/* Set target iso_port */
	case 62:	/* Set permitted port(iso port) isolation portmask */
	case 621:	/* Set each port isolation in portmask */
	case 622:	/* Unset each port isolation in portmask */
	//case 64:	/* Set port(iso port) isolation EFID */
	case 701:       /* Set specified TxDelay */
	case 702:       /* Set specified RxDelay */
	case 900:       /* Set phy x in testmode */
	case 901:       /* Set phy testmode x(1, 4) */

		p = &value;
		value = (unsigned int)val2;
		break;
#if 0
	case 11:
	case 21:
	case 27:
#endif
	case 5:		/* power up LAN ports */
	case 6:		/* power down LAN ports */
	case 8:		/* reset per port MIB counter */
#if defined(RTBE58U) || defined(TUFBE3600)
	case 14:	/* tweak flow control behavior */
	case 15:	/* reset flow control behavior */
	case 16:	/* buffer status related register dumping */
#endif
	case 382:	/* reinit vlan */
	case 4020:      /* Get Led operation mode */
	case 4030:      /* Get LED blinking rate */
	case 4040:      /* Get Led group congiuration mode */
	case 4050:      /* Get Led group to congiuration force mode */
	case 41:	/* turn off all led */
	case 4119:	/* turn off all led, ebg19 case */
	case 4199:	/* turn off all led, ebg19 case */
	case 42:	/* turn on all led by force */
	case 4219:	/* turn on all led by force, ebg19 case */
	case 43:	/* turn on all led normally */
	case 4319:	/* turn on all led normally, ebg19 case */
	case 4351:	/* get port_group-x's enabled mask */
	case 44:	/* hardware reset */
	case 45:	/* software reset */
	case 451:	/* workaround serdes port link down */
	case 49:	/* disable l2 learning */
	case 491:	/* dump drop reason register pre */
	case 492:	/* dump drop reason register post */
	case 50:	/* get FlowControlJumboMode */
	case 52:	/* Get Jumbo threhsold(enable/disable) */
	case 54:	/* Get Jumbo size for Jumbo mode flow control*/
	case 60:	/* Get permitted port isolation portmask */
	//case 63:	/* Get all ports isolation EFID */
	case 70:        /* Get TxDelay, RxDelay */
	case 902:       /* Get phy testmode x(1, 4) */
		p = NULL;
#if defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(RTBE58U) || defined(TUFBE3600) || defined(GTBE19000) || defined(RTBE92U) || defined(RTBE95U)
		if (val == 44) {
#if defined(RTBE58U) || defined(TUFBE3600)
			f_write_string("/sys/class/leds/led_gpio_24/brightness", "255", 0, 0);
#elif defined(RTBE92U) || defined(RTBE95U)
			f_write_string("/sys/class/leds/led_gpio_11/brightness", "255", 0, 0);
#else
			f_write_string("/sys/class/leds/led_gpio_14/brightness", "255", 0, 0);
#endif
			usleep(40*1000);
#if defined(RTBE58U) || defined(TUFBE3600)
			f_write_string("/sys/class/leds/led_gpio_24/brightness", "0", 0, 0);
#elif defined(RTBE92U) || defined(RTBE95U)
			f_write_string("/sys/class/leds/led_gpio_11/brightness", "0", 0, 0);
#else
			f_write_string("/sys/class/leds/led_gpio_14/brightness", "0", 0, 0);
#endif
		}
#endif
		break;
	default:
		dbg("wrong ioctl cmd: %d\n", val);
		close(fd);
		return 0;
	}

	if (ioctl(fd, val, p) < 0) {
		perror("rtkswitch ioctl");
		close(fd);
		return -1;
	}

	if (val == 3 || val == 4)
		printf("return: %x\n", value);

	close(fd);
	return 0;
}

int config_rtkswitch(int argc, char *argv[])
{
	int val;
	int val2 = 0;
	char *cmd = NULL;
	char *cmd2 = NULL;

	if (hnd_boardid_cmp("GT-BE98_BCM") == 0)
		return 0;

	if (argc >= 2)
		cmd = argv[1];
	else
		return -1;
	if (argc >= 3)
		cmd2 = argv[2];

	val = (int) strtol(cmd, NULL, 0);
	if (cmd2)
		val2 = (int) strtol(cmd2, NULL, 0);

	return rtkswitch_ioctl(val, val2);
}

/* Get link status of LAN ports */
unsigned int rtkswitch_lanPorts_phyStatus(void)
{
	int fd;
	unsigned int value;

	fd = open(RTKSWITCH_DEV, O_RDONLY);
	if (fd < 0) {
		perror(RTKSWITCH_DEV);
		return -1;
	}

	if (ioctl(fd, 4, &value) < 0) {
		perror("ioctl");
		value = -1;
	}

	close(fd);
	return value;
}

int rtkswitch_LanPort_linkUp(void)
{
	eval("rtkswitch", "5");

	return 0;
}

int rtkswitch_LanPort_linkDown(void)
{
	eval("rtkswitch", "6");

	return 0;
}

#if defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(GTBE19000) || defined(RTBE92U) || defined(RTBE95U)
unsigned int rtkswitch_serdes_status(void)
{
	int fd;
	unsigned int value;

	fd = open(RTKSWITCH_DEV, O_RDONLY);
	if (fd < 0) {
		perror(RTKSWITCH_DEV);
		return -1;
	}

	if (ioctl(fd, 48, &value) < 0) {
		perror("ioctl");
		value = -1;
	}

	close(fd);
	return value;
}
#endif

#if 0
int rtkswitch_Reset_Storm_Control(void)
{
	eval("rtkswitch", "21");

	return 0;
}
#endif

typedef struct {
	unsigned int link[4];
	unsigned int speed[4];
	unsigned int duplex[4];
} phyState;

int rtkswitch_port_speed(int port)
{
	int fd;
	phyState pS;

	fd = open("/dev/rtkswitch", O_RDONLY);
	if (fd < 0) {
		perror("/dev/rtkswitch");
	} else {
		memset(&pS, 0, sizeof(pS));
		if (ioctl(fd, 0, &pS) < 0) {
			perror("rtkswitch ioctl");
			close(fd);
		}

		close(fd);
	}

#if defined(RTAX55) || defined(RTAX1800) || defined(RTAX58U_V2) || defined(RTAX3000N) || defined(BR63) || defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(RTBE58U) || defined(TUFBE3600) || defined(GTBE19000) || defined(RTBE92U) || defined(RTBE95U)
	if ((port > 0) && (port < 5) && pS.link[port - 1])
	{
		switch (pS.speed[port - 1]) {
#else
	if (pS.link[port])
	{
		switch (pS.speed[port]) {
#endif
			case 0:
				return 10;
			case 1:
				return 100;
			case 2:
				return 1000;
			case 5:
				return 2500;
			default:
				return 0;
		}
	}
	else
		return 0;
}

int rtkswitch_port_duplex(int port)
{
	int fd;
	phyState pS;

	fd = open("/dev/rtkswitch", O_RDONLY);
	if (fd < 0) {
		perror("/dev/rtkswitch");
	} else {
		memset(&pS, 0, sizeof(pS));
		if (ioctl(fd, 0, &pS) < 0) {
			perror("rtkswitch ioctl");
			close(fd);
		}

		close(fd);
	}

#if defined(RTAX55) || defined(RTAX1800) || defined(RTAX58U_V2) || defined(RTAX3000N) || defined(BR63) || defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(RTBE58U) || defined(TUFBE3600) || defined(GTBE19000) || defined(RTBE92U) || defined(RTBE95U)
	if ((port > 0) && (port < 5) && pS.link[port - 1])
		return pS.duplex[port - 1];
#else
	if (pS.link[port])
		return pS.duplex[port];
#endif
	else
		return -1;
}

void show_port_stat(rtk_stat_port_cntr_t *pPort_cntrs)
{
#if defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(GTBE19000) || defined(RTBE92U) || defined(RTBE95U)
	printf("ifInOctets_H: %d\n"
		"ifInOctets_L %d\n"
		"ifOutOctets_H %d\n"
		"ifOutOctets_L %d\n"
		"ifInUcastPkts_H %d\n"
		"ifInUcastPkts_L %d\n"
		"ifInMulticastPkts_H %d\n"
		"ifInMulticastPkts_L %d\n"
		"ifInBroadcastPkts_H %d\n"
		"ifInBroadcastPkts_L %d\n"
		"ifOutUcastPkts_H %d\n"
		"ifOutUcastPkts_L %d\n"
		"ifOutMulticastPkts_H %d\n"
		"ifOutMulticastPkts_L %d\n"
		"ifOutBroadcastPkts_H %d\n"
		"ifOutBroadcastPkts_L %d\n"
		"ifOutDiscards %d\n"
		"dot1dTpPortInDiscards %d\n"
		"dot3StatsSingleCollisionFrames %d\n"
		"dot3StatMultipleCollisionFrames %d\n"
		"dot3sDeferredTransmissions %d\n"
		"dot3StatsLateCollisions %d\n"
		"dot3StatsExcessiveCollisions %d\n"
		"dot3StatsSymbolErrors %d\n"
		"dot3ControlInUnknownOpcodes %d\n"
		"dot3InPauseFrames %d\n"
		"dot3OutPauseFrames %d\n"
		"etherStatsDropEvents %d\n"
		"tx_etherStatsBroadcastPkts %d\n"
		"tx_etherStatsMulticastPkts %d\n"
		"tx_etherStatsCRCAlignErrors %d\n"
		"rx_etherStatsCRCAlignErrors %d\n"
		"tx_etherStatsUndersizePkts %d\n"
		"rx_etherStatsUndersizePkts %d\n"
		"tx_etherStatsOversizePkts %d\n"
		"rx_etherStatsOversizePkts %d\n"
		"tx_etherStatsFragments %d\n"
		"rx_etherStatsFragments %d\n"
		"tx_etherStatsJabbers %d\n"
		"rx_etherStatsJabbers %d\n"
		"tx_etherStatsCollisions %d\n"
		"tx_etherStatsPkts64Octets %d\n"
		"rx_etherStatsPkts64Octets %d\n"
		"tx_etherStatsPkts65to127Octets %d\n"
		"rx_etherStatsPkts65to127Octets %d\n"
		"tx_etherStatsPkts128to255Octets %d\n"
		"rx_etherStatsPkts128to255Octets %d\n"
		"tx_etherStatsPkts256to511Octets %d\n"
		"rx_etherStatsPkts256to511Octets %d\n"
		"tx_etherStatsPkts512to1023Octets %d\n"
		"rx_etherStatsPkts512to1023Octets %d\n"
		"tx_etherStatsPkts1024to1518Octets %d\n"
		"rx_etherStatsPkts1024to1518Octets %d\n\n",
		pPort_cntrs->ifInOctets_H,
		pPort_cntrs->ifInOctets_L,
		pPort_cntrs->ifOutOctets_H,
		pPort_cntrs->ifOutOctets_L,
		pPort_cntrs->ifInUcastPkts_H,
		pPort_cntrs->ifInUcastPkts_L,
		pPort_cntrs->ifInMulticastPkts_H,
		pPort_cntrs->ifInMulticastPkts_L,
		pPort_cntrs->ifInBroadcastPkts_H,
		pPort_cntrs->ifInBroadcastPkts_L,
		pPort_cntrs->ifOutUcastPkts_H,
		pPort_cntrs->ifOutUcastPkts_L,
		pPort_cntrs->ifOutMulticastPkts_H,
		pPort_cntrs->ifOutMulticastPkts_L,
		pPort_cntrs->ifOutBroadcastPkts_H,
		pPort_cntrs->ifOutBroadcastPkts_L,
		pPort_cntrs->ifOutDiscards,
		pPort_cntrs->dot1dTpPortInDiscards,
		pPort_cntrs->dot3StatsSingleCollisionFrames,
		pPort_cntrs->dot3StatMultipleCollisionFrames,
		pPort_cntrs->dot3sDeferredTransmissions,
		pPort_cntrs->dot3StatsLateCollisions,
		pPort_cntrs->dot3StatsExcessiveCollisions,
		pPort_cntrs->dot3StatsSymbolErrors,
		pPort_cntrs->dot3ControlInUnknownOpcodes,
		pPort_cntrs->dot3InPauseFrames,
		pPort_cntrs->dot3OutPauseFrames,
		pPort_cntrs->etherStatsDropEvents,
		pPort_cntrs->tx_etherStatsBroadcastPkts,
		pPort_cntrs->tx_etherStatsMulticastPkts,
		pPort_cntrs->tx_etherStatsCRCAlignErrors,
		pPort_cntrs->rx_etherStatsCRCAlignErrors,
		pPort_cntrs->tx_etherStatsUndersizePkts,
		pPort_cntrs->rx_etherStatsUndersizePkts,
		pPort_cntrs->tx_etherStatsOversizePkts,
		pPort_cntrs->rx_etherStatsOversizePkts,
		pPort_cntrs->tx_etherStatsFragments,
		pPort_cntrs->rx_etherStatsFragments,
		pPort_cntrs->tx_etherStatsJabbers,
		pPort_cntrs->rx_etherStatsJabbers,
		pPort_cntrs->tx_etherStatsCollisions,
		pPort_cntrs->tx_etherStatsPkts64Octets,
		pPort_cntrs->rx_etherStatsPkts64Octets,
		pPort_cntrs->tx_etherStatsPkts65to127Octets,
		pPort_cntrs->rx_etherStatsPkts65to127Octets,
		pPort_cntrs->tx_etherStatsPkts128to255Octets,
		pPort_cntrs->rx_etherStatsPkts128to255Octets,
		pPort_cntrs->tx_etherStatsPkts256to511Octets,
		pPort_cntrs->rx_etherStatsPkts256to511Octets,
		pPort_cntrs->tx_etherStatsPkts512to1023Octets,
		pPort_cntrs->rx_etherStatsPkts512to1023Octets,
		pPort_cntrs->tx_etherStatsPkts1024to1518Octets,
		pPort_cntrs->rx_etherStatsPkts1024to1518Octets
	);

	printf("rx_etherStatsUndersizedropPkts: %d\n"
		"tx_etherStatsPkts1519toMaxOctets: %d\n"
		"rx_etherStatsPkts1519toMaxOctets: %d\n"
		"tx_etherStatsPktsOverMaxOctets: %d\n"
		"rx_etherStatsPktsOverMaxOctets: %d\n"
		"tx_etherStatsPktsFlexibleOctetsSET1: %d\n"
		"rx_etherStatsPktsFlexibleOctetsSET1: %d\n"
		"tx_etherStatsPktsFlexibleOctetsCRCSET1: %d\n"
		"rx_etherStatsPktsFlexibleOctetsCRCSET1: %d\n"
		"tx_etherStatsPktsFlexibleOctetsSET0: %d\n"
		"rx_etherStatsPktsFlexibleOctetsSET0: %d\n"
		"tx_etherStatsPktsFlexibleOctetsCRSET0C: %d\n"
		"rx_etherStatsPktsFlexibleOctetsCRSET0C: %d\n"
		"lengthFieldError: %d\n"
		"falseCarrieimes: %d\n"
		"underSizeOctets: %d\n"
		"framingErrors: %d\n"
		"rxMacDiscards: %d\n"
		"rxMacIPGShortDropRT: %d\n"
		"dot1dTpLearnedEntryDiscards: %d\n"
		"egrQueue7DropPktRT: %d\n"
		"egrQueue6DropPktRT: %d\n"
		"egrQueue5DropPktRT: %d\n"
		"egrQueue4DropPktRT: %d\n"
		"egrQueue3DropPktRT: %d\n"
		"egrQueue2DropPktRT: %d\n"
		"egrQueue1DropPktRT: %d\n"
		"egrQueue0DropPktRT: %d\n"
		"egrQueue7OutPktRT: %d\n"
		"egrQueue6OutPktRT: %d\n"
		"egrQueue5OutPktRT: %d\n"
		"egrQueue4OutPktRT: %d\n"
		"egrQueue3OutPktRT: %d\n"
		"egrQueue2OutPktRT: %d\n"
		"egrQueue1OutPktRT: %d\n"
		"egrQueue0OutPktRT: %d\n"
		"TxGoodCnt_H: %d\n"
		"TxGoodCnt_L: %d\n"
		"RxGoodCnt_H: %d\n"
		"RxGoodCnt_L: %d\n"
		"RxErrorCnt: %d\n"
		"TxErrorCnt: %d\n"
		"TxGoodCnt_phy_H: %d\n"
		"TxGoodCnt_phy_L: %d\n"
		"RxGoodCnt_phy_H: %d\n"
		"RxGoodCnt_phy_L: %d\n"
		"RxErrorCnt_phy: %d\n"
		"TxErrorCnt_phy: %d\n\n",
		pPort_cntrs->rx_etherStatsUndersizedropPkts,
		pPort_cntrs->tx_etherStatsPkts1519toMaxOctets,
		pPort_cntrs->rx_etherStatsPkts1519toMaxOctets,
		pPort_cntrs->tx_etherStatsPktsOverMaxOctets,
		pPort_cntrs->rx_etherStatsPktsOverMaxOctets,
		pPort_cntrs->tx_etherStatsPktsFlexibleOctetsSET1,
		pPort_cntrs->rx_etherStatsPktsFlexibleOctetsSET1,
		pPort_cntrs->tx_etherStatsPktsFlexibleOctetsCRCSET1,
		pPort_cntrs->rx_etherStatsPktsFlexibleOctetsCRCSET1,
		pPort_cntrs->tx_etherStatsPktsFlexibleOctetsSET0,
		pPort_cntrs->rx_etherStatsPktsFlexibleOctetsSET0,
		pPort_cntrs->tx_etherStatsPktsFlexibleOctetsCRSET0C,
		pPort_cntrs->rx_etherStatsPktsFlexibleOctetsCRSET0C,
		pPort_cntrs->lengthFieldError,
		pPort_cntrs->falseCarrieimes,
		pPort_cntrs->underSizeOctets,
		pPort_cntrs->framingErrors,
		pPort_cntrs->rxMacDiscards,
		pPort_cntrs->rxMacIPGShortDropRT,
		pPort_cntrs->dot1dTpLearnedEntryDiscards,
		pPort_cntrs->egrQueue7DropPktRT,
		pPort_cntrs->egrQueue6DropPktRT,
		pPort_cntrs->egrQueue5DropPktRT,
		pPort_cntrs->egrQueue4DropPktRT,
		pPort_cntrs->egrQueue3DropPktRT,
		pPort_cntrs->egrQueue2DropPktRT,
		pPort_cntrs->egrQueue1DropPktRT,
		pPort_cntrs->egrQueue0DropPktRT,
		pPort_cntrs->egrQueue7OutPktRT,
		pPort_cntrs->egrQueue6OutPktRT,
		pPort_cntrs->egrQueue5OutPktRT,
		pPort_cntrs->egrQueue4OutPktRT,
		pPort_cntrs->egrQueue3OutPktRT,
		pPort_cntrs->egrQueue2OutPktRT,
		pPort_cntrs->egrQueue1OutPktRT,
		pPort_cntrs->egrQueue0OutPktRT,
		pPort_cntrs->TxGoodCnt_H,
		pPort_cntrs->TxGoodCnt_L,
		pPort_cntrs->RxGoodCnt_H,
		pPort_cntrs->RxGoodCnt_L,
		pPort_cntrs->RxErrorCnt,
		pPort_cntrs->TxErrorCnt,
		pPort_cntrs->TxGoodCnt_phy_H,
		pPort_cntrs->TxGoodCnt_phy_L,
		pPort_cntrs->RxGoodCnt_phy_H,
		pPort_cntrs->RxGoodCnt_phy_L,
		pPort_cntrs->RxErrorCnt_phy,
		pPort_cntrs->TxErrorCnt_phy
	);
#else
	printf("ifInOctets: %lld\n"
		"dot3StatsFCSErrors: %d\n"
		"dot3StatsSymbolErrors: %d\n"
		"dot3InPauseFrames: %d\n"
		"dot3ControlInUnknownOpcodes: %d\n"
		"etherStatsFragments: %d\n"
		"etherStatsJabbers: %d\n"
		"ifInUcastPkts: %d\n"
		"etherStatsDropEvents: %d\n"
		"etherStatsOctets: %lld\n"
		"etherStatsUndersizePkts: %d\n"
		"etherStatsOversizePkts: %d\n"
		"etherStatsPkts64Octets: %d\n"
		"etherStatsPkts65to127Octets: %d\n"
		"etherStatsPkts128to255Octets: %d\n"
		"etherStatsPkts256to511Octets: %d\n"
		"etherStatsPkts512to1023Octets: %d\n"
		"etherStatsPkts1024toMaxOctets: %d\n"
		"etherStatsMcastPkts: %d\n"
		"etherStatsBcastPkts: %d\n"
		"ifOutOctets: %lld\n"
		"dot3StatsSingleCollisionFrames: %d\n"
		"dot3StatsMultipleCollisionFrames: %d\n"
		"dot3StatsDeferredTransmissions: %d\n"
		"dot3StatsLateCollisions: %d\n"
		"etherStatsCollisions: %d\n"
		"dot3StatsExcessiveCollisions: %d\n"
		"dot3OutPauseFrames: %d\n"
		"dot1dBasePortDelayExceededDiscards: %d\n"
		"dot1dTpPortInDiscards: %d\n"
		"ifOutUcastPkts: %d\n"
		"ifOutMulticastPkts: %d\n"
		"ifOutBrocastPkts: %d\n"
		"outOampduPkts: %d\n"
		"inOampduPkts: %d\n",
		pPort_cntrs->ifInOctets,
		pPort_cntrs->dot3StatsFCSErrors,
		pPort_cntrs->dot3StatsSymbolErrors,
		pPort_cntrs->dot3InPauseFrames,
		pPort_cntrs->dot3ControlInUnknownOpcodes,
		pPort_cntrs->etherStatsFragments,
		pPort_cntrs->etherStatsJabbers,
		pPort_cntrs->ifInUcastPkts,
		pPort_cntrs->etherStatsDropEvents,
		pPort_cntrs->etherStatsOctets,
		pPort_cntrs->etherStatsUndersizePkts,
		pPort_cntrs->etherStatsOversizePkts,
		pPort_cntrs->etherStatsPkts64Octets,
		pPort_cntrs->etherStatsPkts65to127Octets,
		pPort_cntrs->etherStatsPkts128to255Octets,
		pPort_cntrs->etherStatsPkts256to511Octets,
		pPort_cntrs->etherStatsPkts512to1023Octets,
		pPort_cntrs->etherStatsPkts1024toMaxOctets,
		pPort_cntrs->etherStatsMcastPkts,
		pPort_cntrs->etherStatsBcastPkts,
		pPort_cntrs->ifOutOctets,
		pPort_cntrs->dot3StatsSingleCollisionFrames,
		pPort_cntrs->dot3StatsMultipleCollisionFrames,
		pPort_cntrs->dot3StatsDeferredTransmissions,
		pPort_cntrs->dot3StatsLateCollisions,
		pPort_cntrs->etherStatsCollisions,
		pPort_cntrs->dot3StatsExcessiveCollisions,
		pPort_cntrs->dot3OutPauseFrames,
		pPort_cntrs->dot1dBasePortDelayExceededDiscards,
		pPort_cntrs->dot1dTpPortInDiscards,
		pPort_cntrs->ifOutUcastPkts,
		pPort_cntrs->ifOutMulticastPkts,
		pPort_cntrs->ifOutBrocastPkts,
		pPort_cntrs->outOampduPkts,
		pPort_cntrs->inOampduPkts
	);

	printf("pktgenPkts: %d\n"
		"inMldChecksumError: %d\n"
		"inIgmpChecksumError: %d\n"
		"inMldSpecificQuery: %d\n"
		"inMldGeneralQuery: %d\n"
		"inIgmpSpecificQuery: %d\n"
		"inIgmpGeneralQuery: %d\n"
		"inMldLeaves: %d\n"
		"inIgmpLeaves: %d\n"
		"inIgmpJoinsSuccess: %d\n"
		"inIgmpJoinsFail: %d\n"
		"inMldJoinsSuccess: %d\n"
		"inMldJoinsFail: %d\n"
		"inReportSuppressionDrop: %d\n"
		"inLeaveSuppressionDrop: %d\n"
		"outIgmpReports: %d\n"
		"outIgmpLeaves: %d\n"
		"outIgmpGeneralQuery: %d\n"
		"outIgmpSpecificQuery: %d\n"
		"outMldReports: %d\n"
		"outMldLeaves: %d\n"
		"outMldGeneralQuery: %d\n"
		"outMldSpecificQuery: %d\n"
		"inKnownMulticastPkts: %d\n"
		"ifInMulticastPkts: %d\n"
		"ifInBroadcastPkts: %d\n"
		"ifOutDiscards: %d\n\n",
		pPort_cntrs->pktgenPkts,
		pPort_cntrs->inMldChecksumError,
		pPort_cntrs->inIgmpChecksumError,
		pPort_cntrs->inMldSpecificQuery,
		pPort_cntrs->inMldGeneralQuery,
		pPort_cntrs->inIgmpSpecificQuery,
		pPort_cntrs->inIgmpGeneralQuery,
		pPort_cntrs->inMldLeaves,
		pPort_cntrs->inIgmpLeaves,
		pPort_cntrs->inIgmpJoinsSuccess,
		pPort_cntrs->inIgmpJoinsFail,
		pPort_cntrs->inMldJoinsSuccess,
		pPort_cntrs->inMldJoinsFail,
		pPort_cntrs->inReportSuppressionDrop,
		pPort_cntrs->inLeaveSuppressionDrop,
		pPort_cntrs->outIgmpReports,
		pPort_cntrs->outIgmpLeaves,
		pPort_cntrs->outIgmpGeneralQuery,
		pPort_cntrs->outIgmpSpecificQuery,
		pPort_cntrs->outMldReports,
		pPort_cntrs->outMldLeaves,
		pPort_cntrs->outMldGeneralQuery,
		pPort_cntrs->outMldSpecificQuery,
		pPort_cntrs->inKnownMulticastPkts,
		pPort_cntrs->ifInMulticastPkts,
		pPort_cntrs->ifInBroadcastPkts,
		pPort_cntrs->ifOutDiscards
	);
#endif
}

int rtkswitch_port_stat(int port)
{
	int fd;
	int *p = NULL;
	rtk_stat_port_cntr_t Port_cntrs;
#if defined(RTAX55) || defined(RTAX1800) || defined(RTAX58U_V2) || defined(RTAX3000N) || defined(BR63) || defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(RTBE58U) || defined(TUFBE3600) || defined(GTBE19000) || defined(RTBE92U) || defined(RTBE95U)
#if defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(GTBE19000) || defined(RTBE58U) || defined(TUFBE3600) || defined(RTBE92U) || defined(RTBE95U)
	if ((port < 1) || (port > 5))
#else
	if ((port < 1) || (port > 4))
#endif
		return -1;
#endif

	fd = open("/dev/rtkswitch", O_RDONLY);
	if (fd < 0) {
		perror("/dev/rtkswitch");
	} else {
		memset(&Port_cntrs, 0, sizeof(Port_cntrs));
		p = (int *) &Port_cntrs;
#if defined(RTAX55) || defined(RTAX1800) || defined(RTAX58U_V2) || defined(RTAX3000N) || defined(BR63) || defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(GTBE19000) || defined(RTBE58U) || defined(TUFBE3600) || defined(RTBE92U) || defined(RTBE95U)
		*p = port - 1;
#else
		*p = port;
#endif
		if (ioctl(fd, 1, &Port_cntrs) < 0) {
			perror("rtkswitch ioctl");
			close(fd);
		} else
			show_port_stat(&Port_cntrs);

		close(fd);
	}

	return 0;
}

void show_port_mactable(mactable *pPort_mactable)
{
	int i;
	char eabuf[18];

	if (pPort_mactable->count)
	for (i = 0; i < pPort_mactable->count && i < 256; i++)
		printf("%s\n", ether_etoa((void *)&pPort_mactable->ea[i], eabuf));
}

int rtkswitch_port_mactable(int port)
{
	int fd;
	int *p = NULL;
	mactable Port_mactable;

#if defined(RTAX55) || defined(RTAX1800) || defined(RTAX58U_V2) || defined(RTAX3000N) || defined(BR63) || defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(RTBE58U) || defined(TUFBE3600) || defined(GTBE19000) || defined(RTBE92U) || defined(RTBE95U)
	if ((port < 1) || (port > 4))
		return -1;
#endif

	fd = open("/dev/rtkswitch", O_RDONLY);
	if (fd < 0) {
		perror("/dev/rtkswitch");
	} else {
		memset(&Port_mactable, 0, sizeof(Port_mactable));
		p = (int *) &Port_mactable;
#if defined(RTAX55) || defined(RTAX1800) || defined(RTAX58U_V2) || defined(RTAX3000N) || defined(BR63) || defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(GTBE19000) || defined(RTBE58U) || defined(TUFBE3600) || defined(RTBE92U) || defined(RTBE95U)
		*p = port - 1;
#else
		*p = port;
#endif
		if (ioctl(fd, 7, &Port_mactable) < 0) {
			perror("rtkswitch ioctl");
			close(fd);
		} else
			show_port_mactable(&Port_mactable);

		close(fd);
	}

	return 0;
}
