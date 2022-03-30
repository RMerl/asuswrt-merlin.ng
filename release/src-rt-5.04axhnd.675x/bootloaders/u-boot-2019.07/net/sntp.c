/*
 * SNTP support driver
 *
 * Masami Komiya <mkomiya@sonare.it> 2005
 *
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <net.h>
#include <rtc.h>

#include "sntp.h"

#define SNTP_TIMEOUT 10000UL

static int sntp_our_port;

static void sntp_send(void)
{
	struct sntp_pkt_t pkt;
	int pktlen = SNTP_PACKET_LEN;
	int sport;

	debug("%s\n", __func__);

	memset(&pkt, 0, sizeof(pkt));

	pkt.li = NTP_LI_NOLEAP;
	pkt.vn = NTP_VERSION;
	pkt.mode = NTP_MODE_CLIENT;

	memcpy((char *)net_tx_packet + net_eth_hdr_size() + IP_UDP_HDR_SIZE,
	       (char *)&pkt, pktlen);

	sntp_our_port = 10000 + (get_timer(0) % 4096);
	sport = NTP_SERVICE_PORT;

	net_send_udp_packet(net_server_ethaddr, net_ntp_server, sport,
			    sntp_our_port, pktlen);
}

static void sntp_timeout_handler(void)
{
	puts("Timeout\n");
	net_set_state(NETLOOP_FAIL);
	return;
}

static void sntp_handler(uchar *pkt, unsigned dest, struct in_addr sip,
			 unsigned src, unsigned len)
{
#ifdef CONFIG_TIMESTAMP
	struct sntp_pkt_t *rpktp = (struct sntp_pkt_t *)pkt;
	struct rtc_time tm;
	ulong seconds;
#endif

	debug("%s\n", __func__);

	if (dest != sntp_our_port)
		return;

#ifdef CONFIG_TIMESTAMP
	/*
	 * As the RTC's used in U-Boot support second resolution only
	 * we simply ignore the sub-second field.
	 */
	memcpy(&seconds, &rpktp->transmit_timestamp, sizeof(ulong));

	rtc_to_tm(ntohl(seconds) - 2208988800UL + net_ntp_time_offset, &tm);
#if defined(CONFIG_CMD_DATE)
#  ifdef CONFIG_DM_RTC
	struct udevice *dev;
	int ret;

	ret = uclass_get_device(UCLASS_RTC, 0, &dev);
	if (ret)
		printf("SNTP: cannot find RTC: err=%d\n", ret);
	else
		dm_rtc_set(dev, &tm);
#  else
	rtc_set(&tm);
#  endif
#endif
	printf("Date: %4d-%02d-%02d Time: %2d:%02d:%02d\n",
	       tm.tm_year, tm.tm_mon, tm.tm_mday,
	       tm.tm_hour, tm.tm_min, tm.tm_sec);
#endif

	net_set_state(NETLOOP_SUCCESS);
}

void sntp_start(void)
{
	debug("%s\n", __func__);

	net_set_timeout_handler(SNTP_TIMEOUT, sntp_timeout_handler);
	net_set_udp_handler(sntp_handler);
	memset(net_server_ethaddr, 0, sizeof(net_server_ethaddr));

	sntp_send();
}
