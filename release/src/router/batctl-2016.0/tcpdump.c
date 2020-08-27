/*
 * Copyright (C) 2007-2016  B.A.T.M.A.N. contributors:
 *
 * Andreas Langer <an.langer@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/select.h>
#include <sys/socket.h>

#include "tcpdump.h"
#include "packet.h"
#include "bat-hosts.h"
#include "functions.h"

#ifndef ETH_P_BATMAN
#define ETH_P_BATMAN	0x4305
#endif /* ETH_P_BATMAN */

#define IPV6_MIN_MTU	1280

#define LEN_CHECK(buff_len, check_len, desc) \
if ((size_t)(buff_len) < (check_len)) { \
	fprintf(stderr, "Warning - dropping received %s packet as it is smaller than expected (%zu): %zu\n", \
		desc, (check_len), (size_t)(buff_len)); \
	return; \
}

static unsigned short dump_level_all = DUMP_TYPE_BATOGM | DUMP_TYPE_BATICMP | DUMP_TYPE_BATUCAST |
		DUMP_TYPE_BATBCAST | DUMP_TYPE_BATUTVLV | DUMP_TYPE_BATFRAG | DUMP_TYPE_NONBAT;
static unsigned short dump_level;

static void parse_eth_hdr(unsigned char *packet_buff, ssize_t buff_len, int read_opt, int time_printed);

static void tcpdump_usage(void)
{
	fprintf(stderr, "Usage: batctl tcpdump [parameters] interface [interface]\n");
	fprintf(stderr, "parameters:\n");
	fprintf(stderr, " \t -c compat filter - only display packets matching own compat version (%i)\n", BATADV_COMPAT_VERSION);
	fprintf(stderr, " \t -h print this help\n");
	fprintf(stderr, " \t -n don't convert addresses to bat-host names\n");
	fprintf(stderr, " \t -p dump specific packet type\n");
	fprintf(stderr, " \t -x dump all packet types except specified\n");
	fprintf(stderr, "packet types:\n");
	fprintf(stderr, " \t\t%3d - batman ogm packets\n", DUMP_TYPE_BATOGM);
	fprintf(stderr, " \t\t%3d - batman icmp packets\n", DUMP_TYPE_BATICMP);
	fprintf(stderr, " \t\t%3d - batman unicast packets\n", DUMP_TYPE_BATUCAST);
	fprintf(stderr, " \t\t%3d - batman broadcast packets\n", DUMP_TYPE_BATBCAST);
	fprintf(stderr, " \t\t%3d - batman fragmented packets\n", DUMP_TYPE_BATFRAG);
	fprintf(stderr, " \t\t%3d - batman unicast tvlv packets\n", DUMP_TYPE_BATUTVLV);
	fprintf(stderr, " \t\t%3d - non batman packets\n", DUMP_TYPE_NONBAT);
	fprintf(stderr, " \t\t%3d - batman ogm & non batman packets\n", DUMP_TYPE_BATOGM | DUMP_TYPE_NONBAT);
}

static int print_time(void)
{
	struct timeval tv;
	struct tm *tm;

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	printf("%02d:%02d:%02d.%06ld ", tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec);
	return 1;
}

static void batctl_tvlv_parse_gw_v1(void *buff, ssize_t buff_len)
{
	struct batadv_tvlv_gateway_data *tvlv = buff;
	uint32_t down, up;

	if (buff_len != sizeof(*tvlv)) {
		fprintf(stderr, "Warning - dropping received %s packet as it is not the correct size (%zu): %zu\n",
			"TVLV GWv1", sizeof(*tvlv), buff_len);
		return;
	}

	down = ntohl(tvlv->bandwidth_down);
	up = ntohl(tvlv->bandwidth_up);

	printf("\tTVLV GWv1: down %d.%.1dMbps, up %d.%1dMbps\n",
	       down / 10, down % 10, up / 10, up % 10);
}

static void batctl_tvlv_parse_dat_v1(void (*buff)__attribute__((unused)),
				     ssize_t buff_len)
{
	if (buff_len != 0) {
		fprintf(stderr, "Warning - dropping received %s packet as it is not the correct size (0): %zu\n",
			"TVLV DATv1", buff_len);
		return;
	}

	printf("\tTVLV DATv1: enabled\n");
}

static void batctl_tvlv_parse_nc_v1(void (*buff)__attribute__((unused)),
				    ssize_t buff_len)
{
	if (buff_len != 0) {
		fprintf(stderr, "Warning - dropping received %s packet as it is not the correct size (0): %zu\n",
			"TVLV NCv1", buff_len);
		return;
	}

	printf("\tTVLV NCv1: enabled\n");
}

static void batctl_tvlv_parse_tt_v1(void *buff, ssize_t buff_len)
{
	struct batadv_tvlv_tt_data *tvlv = buff;
	struct batadv_tvlv_tt_vlan_data *vlan;
	int i, num_vlan, num_entry;
	const char *type;
	size_t vlan_len;

	LEN_CHECK(buff_len, sizeof(*tvlv), "TVLV TTv1")

	if (tvlv->flags & BATADV_TT_OGM_DIFF)
		type = "OGM DIFF";
	else if (tvlv->flags & BATADV_TT_REQUEST)
		type = "TT REQUEST";
	else if (tvlv->flags & BATADV_TT_RESPONSE)
		type = "TT RESPONSE";
	else
		type = "UNKNOWN";

	num_vlan = ntohs(tvlv->num_vlan);
	vlan_len = sizeof(*tvlv) + sizeof(*vlan) * num_vlan;
	LEN_CHECK(buff_len, vlan_len, "TVLV TTv1 VLAN")

	buff_len -= vlan_len;
	num_entry = buff_len / sizeof(struct batadv_tvlv_tt_change);

	printf("\tTVLV TTv1: %s [%c] ttvn=%hhu vlan_num=%hu entry_num=%hu\n",
	       type, tvlv->flags & BATADV_TT_FULL_TABLE ? 'F' : '.',
	       tvlv->ttvn, num_vlan, num_entry);

	vlan = (struct batadv_tvlv_tt_vlan_data *)(tvlv + 1);
	for (i = 0; i < num_vlan; i++) {
		printf("\t\tVLAN ID %hd, crc %#.8x\n",
		       BATADV_PRINT_VID(ntohs(vlan->vid)),
		       ntohl(vlan->crc));
		vlan++;
	}
}

static void batctl_tvlv_parse_roam_v1(void *buff, ssize_t buff_len)
{
	struct batadv_tvlv_roam_adv *tvlv = buff;

	if (buff_len != sizeof(*tvlv)) {
		fprintf(stderr, "Warning - dropping received %s packet as it is not the correct size (%zu): %zu\n",
			"TVLV ROAMv1", sizeof(*tvlv), buff_len);
		return;
	}

	printf("\tTVLV ROAMv1: client %s, VLAN ID %d\n",
	       get_name_by_macaddr((struct ether_addr *)tvlv->client, NO_FLAGS),
	       BATADV_PRINT_VID(ntohs(tvlv->vid)));
}

typedef void (*batctl_tvlv_parser_t)(void *buff, ssize_t buff_len);

static batctl_tvlv_parser_t tvlv_parser_get(uint8_t type, uint8_t version)
{
	switch (type) {
	case BATADV_TVLV_GW:
		switch (version) {
		case 1:
			return batctl_tvlv_parse_gw_v1;
		default:
			return NULL;
		}

	case BATADV_TVLV_DAT:
		switch (version) {
		case 1:
			return batctl_tvlv_parse_dat_v1;
		default:
			return NULL;
		}

	case BATADV_TVLV_NC:
		switch (version) {
		case 1:
			return batctl_tvlv_parse_nc_v1;
		default:
			return NULL;
		}

	case BATADV_TVLV_TT:
		switch (version) {
		case 1:
			return batctl_tvlv_parse_tt_v1;
		default:
			return NULL;
		}

	case BATADV_TVLV_ROAM:
		switch (version) {
		case 1:
			return batctl_tvlv_parse_roam_v1;
		default:
			return NULL;
		}

	default:
		return NULL;
	}
}

static void dump_batman_ucast_tvlv(unsigned char *packet_buff, ssize_t buff_len,
				   int read_opt, int time_printed)
{
	struct batadv_unicast_tvlv_packet *tvlv_packet;
	struct batadv_tvlv_hdr *tvlv_hdr;
	struct ether_header *ether_header;
	struct ether_addr *src, *dst;
	batctl_tvlv_parser_t parser;
	ssize_t check_len, tvlv_len, len;
	uint8_t *ptr;

	check_len = (size_t)buff_len - sizeof(struct ether_header);

	LEN_CHECK(check_len, sizeof(*tvlv_packet), "BAT TVLV");
	check_len -= sizeof(*tvlv_packet);

	ether_header = (struct ether_header *)packet_buff;
	tvlv_packet = (struct batadv_unicast_tvlv_packet *)(ether_header + 1);

	LEN_CHECK(check_len, (size_t)ntohs(tvlv_packet->tvlv_len),
		  "BAT TVLV (containers)");

	if (!time_printed)
		time_printed = print_time();

	src = (struct ether_addr *)tvlv_packet->src;
	printf("BAT %s > ", get_name_by_macaddr(src, read_opt));

	dst = (struct ether_addr *)tvlv_packet->dst;
	tvlv_len = ntohs(tvlv_packet->tvlv_len);
	printf("%s: TVLV, len %zu, tvlv_len %zu, ttl %hhu\n",
	       get_name_by_macaddr(dst, read_opt),
	       buff_len - sizeof(struct ether_header), tvlv_len,
	       tvlv_packet->ttl);

	ptr = (uint8_t *)(tvlv_packet + 1);

	while (tvlv_len >= (ssize_t)sizeof(*tvlv_hdr)) {
		tvlv_hdr = (struct batadv_tvlv_hdr *)ptr;

		/* data after TVLV header */
		ptr = (uint8_t *)(tvlv_hdr + 1);
		tvlv_len -= sizeof(*tvlv_hdr);

		len = ntohs(tvlv_hdr->len);
		LEN_CHECK(tvlv_len, (size_t)len, "BAT UCAST TVLV");

		parser = tvlv_parser_get(tvlv_hdr->type, tvlv_hdr->version);
		if (parser)
			parser(ptr, len);

		/* go to the next container */
		ptr += len;
		tvlv_len -= len;
	}
}

static int dump_bla2_claim(struct ether_header *eth_hdr,
			   struct ether_arp *arphdr, int read_opt)
{
	uint8_t bla_claim_magic[3] = {0xff, 0x43, 0x05};
	struct batadv_bla_claim_dst *bla_dst;
	int arp_is_bla2_claim = 0;
	uint8_t *hw_src, *hw_dst;

	if (arphdr->ea_hdr.ar_hrd != htons(ARPHRD_ETHER))
		goto out;

	if (arphdr->ea_hdr.ar_pro != htons(ETH_P_IP))
		goto out;

	if (arphdr->ea_hdr.ar_hln != ETH_ALEN)
		goto out;

	if (arphdr->ea_hdr.ar_pln != 4)
		goto out;

	hw_src = arphdr->arp_sha;
	hw_dst = arphdr->arp_tha;
	bla_dst = (struct batadv_bla_claim_dst *)hw_dst;

	if (memcmp(bla_dst->magic, bla_claim_magic, sizeof(bla_claim_magic)) != 0)
		goto out;

	switch (bla_dst->type) {
	case BATADV_CLAIM_TYPE_CLAIM:
		printf("BLA CLAIM, backbone %s, ",
		       get_name_by_macaddr((struct ether_addr *)hw_src, read_opt));
		printf("client %s, bla group %04x\n",
		       get_name_by_macaddr((struct ether_addr *)eth_hdr->ether_shost, read_opt),
		       ntohs(bla_dst->group));
		break;
	case BATADV_CLAIM_TYPE_UNCLAIM:
		printf("BLA UNCLAIM, backbone %s, ",
		       get_name_by_macaddr((struct ether_addr *)eth_hdr->ether_shost, read_opt));
		printf("client %s, bla group %04x\n",
		       get_name_by_macaddr((struct ether_addr *)hw_src, read_opt),
		       ntohs(bla_dst->group));
		break;
	case BATADV_CLAIM_TYPE_ANNOUNCE:
		printf("BLA ANNOUNCE, backbone %s, bla group %04x, crc %04x\n",
		       get_name_by_macaddr((struct ether_addr *)eth_hdr->ether_shost, read_opt),
		       ntohs(bla_dst->group), ntohs(*((uint16_t *)(&hw_src[4]))));
		break;
	case BATADV_CLAIM_TYPE_REQUEST:
		printf("BLA REQUEST, src backbone %s, ",
		       get_name_by_macaddr((struct ether_addr *)eth_hdr->ether_shost, read_opt));
		printf("dst backbone %s\n",
		       get_name_by_macaddr((struct ether_addr *)eth_hdr->ether_dhost, read_opt));
		break;
	default:
		printf("BLA UNKNOWN, type %hhu\n", bla_dst->type);
		break;
	}

	arp_is_bla2_claim = 1;

out:
	return arp_is_bla2_claim;
}

static void dump_arp(unsigned char *packet_buff, ssize_t buff_len,
		     struct ether_header *eth_hdr, int read_opt, int time_printed)
{
	struct ether_arp *arphdr;
	int arp_is_bla2_claim;

	LEN_CHECK((size_t)buff_len, sizeof(struct ether_arp), "ARP");

	if (!time_printed)
		print_time();

	arphdr = (struct ether_arp *)packet_buff;

	switch (ntohs(arphdr->arp_op)) {
	case ARPOP_REQUEST:
		printf("ARP, Request who-has %s", inet_ntoa(*(struct in_addr *)&arphdr->arp_tpa));
		printf(" tell %s (%s), length %zd\n", inet_ntoa(*(struct in_addr *)&arphdr->arp_spa),
			ether_ntoa_long((struct ether_addr *)&arphdr->arp_sha), buff_len);
		break;
	case ARPOP_REPLY:
		arp_is_bla2_claim = dump_bla2_claim(eth_hdr, arphdr, read_opt);
		if (arp_is_bla2_claim)
			break;

		printf("ARP, Reply %s is-at %s, length %zd\n", inet_ntoa(*(struct in_addr *)&arphdr->arp_spa),
			ether_ntoa_long((struct ether_addr *)&arphdr->arp_sha), buff_len);
		break;
	default:
		printf("ARP, unknown op code: %i\n", ntohs(arphdr->arp_op));
		break;
	}
}

static void dump_tcp(const char ip_string[], unsigned char *packet_buff,
		     ssize_t buff_len, size_t ip6_header_len, char *src_addr,
		     char *dst_addr)
{
	uint16_t tcp_header_len;
	struct tcphdr *tcphdr;

	LEN_CHECK((size_t)buff_len - ip6_header_len,
		  sizeof(struct tcphdr), "TCP");
	tcphdr = (struct tcphdr *)(packet_buff + ip6_header_len);
	tcp_header_len = tcphdr->doff * 4;
	printf("%s %s.%i > ", ip_string, src_addr, ntohs(tcphdr->source));
	printf("%s.%i: TCP, Flags [%c%c%c%c%c%c], length %zu\n",
		dst_addr, ntohs(tcphdr->dest),
		(tcphdr->fin ? 'F' : '.'), (tcphdr->syn ? 'S' : '.'),
		(tcphdr->rst ? 'R' : '.'), (tcphdr->psh ? 'P' : '.'),
		(tcphdr->ack ? 'A' : '.'), (tcphdr->urg ? 'U' : '.'),
		(size_t)buff_len - ip6_header_len - tcp_header_len);
}

static void dump_udp(const char ip_string[], unsigned char *packet_buff,
		     ssize_t buff_len, size_t ip6_header_len, char *src_addr,
		     char *dst_addr)
{
	struct udphdr *udphdr;

	LEN_CHECK((size_t)buff_len - ip6_header_len, sizeof(struct udphdr),
		  "UDP");
	udphdr = (struct udphdr *)(packet_buff + ip6_header_len);
	printf("%s %s.%i > ", ip_string, src_addr, ntohs(udphdr->source));

	switch (ntohs(udphdr->dest)) {
	case 67:
		LEN_CHECK((size_t)buff_len - ip6_header_len -
			  sizeof(struct udphdr), (size_t) 44, "DHCP");
		printf("%s.67: BOOTP/DHCP, Request from %s, length %zu\n",
		       dst_addr,
		       ether_ntoa_long((struct ether_addr *)(((char *)udphdr) +
				       sizeof(struct udphdr) + 28)),
		       (size_t)buff_len - ip6_header_len -
		       sizeof(struct udphdr));
		break;
	case 68:
		printf("%s.68: BOOTP/DHCP, Reply, length %zu\n", dst_addr,
		       (size_t)buff_len - ip6_header_len -
		       sizeof(struct udphdr));
		break;
	default:
		printf("%s.%i: UDP, length %zu\n", dst_addr,
		       ntohs(udphdr->dest),
		       (size_t)buff_len - ip6_header_len -
		       sizeof(struct udphdr));
		break;
	}
}

static void dump_ipv6(unsigned char *packet_buff, ssize_t buff_len,
		      int time_printed)
{
	struct ip6_hdr *iphdr;
	struct icmp6_hdr *icmphdr;

	char ipsrc[INET6_ADDRSTRLEN], ipdst[INET6_ADDRSTRLEN];
	struct nd_neighbor_solicit *nd_neigh_sol;
	struct nd_neighbor_advert *nd_advert;
	char nd_nas_target[INET6_ADDRSTRLEN];
	const char ip_string[] = "IP6";

	iphdr = (struct ip6_hdr *)packet_buff;
	LEN_CHECK((size_t)buff_len, (size_t)(sizeof(struct ip6_hdr)), ip_string);

	if (!time_printed)
		print_time();

	if (!inet_ntop(AF_INET6, &iphdr->ip6_src, ipsrc, sizeof(ipsrc))) {
		fprintf(stderr, "Cannot decode source IPv6\n");
		return;
	}

	if (!inet_ntop(AF_INET6, &iphdr->ip6_dst, ipdst, sizeof(ipdst))) {
		fprintf(stderr, "Cannot decode destination IPv6\n");
		return;
	}

	switch (iphdr->ip6_nxt) {
	case IPPROTO_ICMPV6:
		LEN_CHECK((size_t)buff_len - (size_t)(sizeof(struct ip6_hdr)),
			  sizeof(struct icmp6_hdr), "ICMPv6");
		icmphdr = (struct icmp6_hdr *)(packet_buff +
					       sizeof(struct ip6_hdr));

		printf("%s %s > %s ", ip_string, ipsrc, ipdst);
		if (icmphdr->icmp6_type < ICMP6_INFOMSG_MASK &&
		    (size_t)(buff_len) > IPV6_MIN_MTU) {
			fprintf(stderr,
				"Warning - dropping received 'ICMPv6 destination unreached' packet as it is bigger than maximum allowed size (%u): %zu\n",
				IPV6_MIN_MTU, (size_t)(buff_len));
			return;
		}

		printf("ICMP6");
		switch (icmphdr->icmp6_type) {
		case ICMP6_DST_UNREACH:
			switch (icmphdr->icmp6_code) {
			case ICMP6_DST_UNREACH_NOROUTE:
				printf(", unreachable route\n");
				break;
			case ICMP6_DST_UNREACH_ADMIN:
				printf(", unreachable prohibited\n");
				break;
			case ICMP6_DST_UNREACH_ADDR:
				printf(", unreachable address\n");
				break;
			case ICMP6_DST_UNREACH_BEYONDSCOPE:
				printf(", beyond scope\n");
				break;
			case ICMP6_DST_UNREACH_NOPORT:
				printf(", unreachable port\n");
				break;
			default:
				printf(", unknown unreach code (%u)\n",
				       icmphdr->icmp6_code);
			}
			break;
		case ICMP6_ECHO_REQUEST:
			printf(" echo request, id: %d, seq: %d, length: %hu\n",
			       icmphdr->icmp6_id, icmphdr->icmp6_seq,
			       iphdr->ip6_plen);
			break;
		case ICMP6_ECHO_REPLY:
			printf(" echo reply, id: %d, seq: %d, length: %hu\n",
			       icmphdr->icmp6_id, icmphdr->icmp6_seq,
			       iphdr->ip6_plen);
			break;
		case ICMP6_TIME_EXCEEDED:
			printf(" time exceeded in-transit, length %zu\n",
			       (size_t)buff_len - sizeof(struct icmp6_hdr));
			break;
		case ND_NEIGHBOR_SOLICIT:
			nd_neigh_sol = (struct nd_neighbor_solicit *)icmphdr;
			inet_ntop(AF_INET6, &(nd_neigh_sol->nd_ns_target),
				  nd_nas_target, 40);
			printf(" neighbor solicitation, who has %s, length %zd\n",
			       nd_nas_target, buff_len);
			break;
		case ND_NEIGHBOR_ADVERT:
			nd_advert = (struct nd_neighbor_advert *)icmphdr;
			inet_ntop(AF_INET6, &(nd_advert->nd_na_target),
				  nd_nas_target, 40);
			printf(" neighbor advertisement, tgt is %s, length %zd\n",
			       nd_nas_target, buff_len);
			break;
		default:
			printf(", destination unreachable, unknown icmp6 type (%u)\n",
			       icmphdr->icmp6_type);
			break;
		}
		break;
	case IPPROTO_TCP:
		dump_tcp(ip_string, packet_buff, buff_len,
			 sizeof(struct ip6_hdr), ipsrc, ipdst);
		break;
	case IPPROTO_UDP:
		dump_udp(ip_string, packet_buff, buff_len,
			 sizeof(struct ip6_hdr), ipsrc, ipdst);
		break;
	default:
		printf(" IPv6 unknown protocol: %i\n", iphdr->ip6_nxt);
	}
}

static void dump_ip(unsigned char *packet_buff, ssize_t buff_len,
		    int time_printed)
{
	char ipsrc[INET_ADDRSTRLEN], ipdst[INET_ADDRSTRLEN];
	struct iphdr *iphdr, *tmp_iphdr;
	const char ip_string[] = "IP";
	struct udphdr *tmp_udphdr;
	struct icmphdr *icmphdr;

	iphdr = (struct iphdr *)packet_buff;
	LEN_CHECK((size_t)buff_len, (size_t)(iphdr->ihl * 4), ip_string);

	if (!time_printed)
		print_time();

	if (!inet_ntop(AF_INET, &iphdr->saddr, ipsrc, sizeof(ipsrc))) {
		fprintf(stderr, "Cannot decode source IP\n");
		return;
	}

	if (!inet_ntop(AF_INET, &iphdr->daddr, ipdst, sizeof(ipdst))) {
		fprintf(stderr, "Cannot decode destination IP\n");
		return;
	}

	switch (iphdr->protocol) {
	case IPPROTO_ICMP:
		LEN_CHECK((size_t)buff_len - (iphdr->ihl * 4), sizeof(struct icmphdr), "ICMP");

		icmphdr = (struct icmphdr *)(packet_buff + (iphdr->ihl * 4));
		printf("%s %s > ", ip_string, ipsrc);

		switch (icmphdr->type) {
		case ICMP_ECHOREPLY:
			printf("%s: ICMP echo reply, id %hu, seq %hu, length %zu\n",
				ipdst, ntohs(icmphdr->un.echo.id),
				ntohs(icmphdr->un.echo.sequence),
				(size_t)buff_len - (iphdr->ihl * 4));
			break;
		case ICMP_DEST_UNREACH:
			LEN_CHECK((size_t)buff_len - (iphdr->ihl * 4) - sizeof(struct icmphdr),
				sizeof(struct iphdr) + 8, "ICMP DEST_UNREACH");

			switch (icmphdr->code) {
			case ICMP_PORT_UNREACH:
				tmp_iphdr = (struct iphdr *)(((char *)icmphdr) + sizeof(struct icmphdr));
				tmp_udphdr = (struct udphdr *)(((char *)tmp_iphdr) + (tmp_iphdr->ihl * 4));

				printf("%s: ICMP ", ipdst);
				printf("%s udp port %hu unreachable, length %zu\n",
					ipdst, ntohs(tmp_udphdr->dest),
					(size_t)buff_len - (iphdr->ihl * 4));
				break;
			default:
				printf("%s: ICMP unreachable %hhu, length %zu\n",
					ipdst, icmphdr->code,
					(size_t)buff_len - (iphdr->ihl * 4));
				break;
			}

			break;
		case ICMP_ECHO:
			printf("%s: ICMP echo request, id %hu, seq %hu, length %zu\n",
				ipdst, ntohs(icmphdr->un.echo.id),
				ntohs(icmphdr->un.echo.sequence),
				(size_t)buff_len - (iphdr->ihl * 4));
			break;
		case ICMP_TIME_EXCEEDED:
			printf("%s: ICMP time exceeded in-transit, length %zu\n",
				ipdst, (size_t)buff_len - (iphdr->ihl * 4));
			break;
		default:
			printf("%s: ICMP type %hhu, length %zu\n",
				ipdst, icmphdr->type,
				(size_t)buff_len - (iphdr->ihl * 4));
			break;
		}
		break;
	case IPPROTO_TCP:
		dump_tcp(ip_string, packet_buff, buff_len, iphdr->ihl * 4,
			 ipsrc, ipdst);
		break;
	case IPPROTO_UDP:
		dump_udp(ip_string, packet_buff, buff_len, iphdr->ihl * 4,
			 ipsrc, ipdst);
		break;
	default:
		printf("IP unknown protocol: %i\n", iphdr->protocol);
		break;
	}
}

static void dump_vlan(unsigned char *packet_buff, ssize_t buff_len, int read_opt, int time_printed)
{
	struct vlanhdr *vlanhdr;

	vlanhdr = (struct vlanhdr *)(packet_buff + sizeof(struct ether_header));
	LEN_CHECK((size_t)buff_len, sizeof(struct ether_header) + sizeof(struct vlanhdr), "VLAN");

	if (!time_printed)
		time_printed = print_time();

	vlanhdr->vid = ntohs(vlanhdr->vid);
	printf("vlan %u, p %u, ", vlanhdr->vid, vlanhdr->vid >> 12);

	/* overwrite vlan tags */
	memmove(packet_buff + 4, packet_buff, 2 * ETH_ALEN);

	parse_eth_hdr(packet_buff + 4, buff_len - 4, read_opt, time_printed);
}

static void dump_batman_iv_ogm(unsigned char *packet_buff, ssize_t buff_len, int read_opt, int time_printed)
{
	struct ether_header *ether_header;
	struct batadv_ogm_packet *batman_ogm_packet;
	struct batadv_tvlv_hdr *tvlv_hdr;
	ssize_t tvlv_len, len, check_len;
	batctl_tvlv_parser_t parser;
	uint8_t *ptr;

	check_len = (size_t)buff_len - sizeof(struct ether_header);
	LEN_CHECK(check_len, sizeof(struct batadv_ogm_packet), "BAT IV OGM");

	ether_header = (struct ether_header *)packet_buff;
	batman_ogm_packet = (struct batadv_ogm_packet *)(packet_buff + sizeof(struct ether_header));

	if (!time_printed)
		print_time();

	printf("BAT %s: ",
	       get_name_by_macaddr((struct ether_addr *)batman_ogm_packet->orig, read_opt));

	tvlv_len = ntohs(batman_ogm_packet->tvlv_len);
	printf("OGM IV via neigh %s, seq %u, tq %3d, ttl %2d, v %d, flags [%c%c%c], length %zu, tvlv_len %zu\n",
	       get_name_by_macaddr((struct ether_addr *)ether_header->ether_shost, read_opt),
	       ntohl(batman_ogm_packet->seqno), batman_ogm_packet->tq,
	       batman_ogm_packet->ttl, batman_ogm_packet->version,
	       (batman_ogm_packet->flags & BATADV_NOT_BEST_NEXT_HOP ? 'N' : '.'),
	       (batman_ogm_packet->flags & BATADV_DIRECTLINK ? 'D' : '.'),
	       (batman_ogm_packet->flags & BATADV_PRIMARIES_FIRST_HOP ? 'F' : '.'),
	       check_len, tvlv_len);

	check_len -= sizeof(struct batadv_ogm_packet);
	LEN_CHECK(check_len, (size_t)tvlv_len, "BAT OGM TVLV (containers)");

	ptr = (uint8_t *)(batman_ogm_packet + 1);

	while (tvlv_len >= (ssize_t)sizeof(*tvlv_hdr)) {
		tvlv_hdr = (struct batadv_tvlv_hdr *)ptr;

		/* data after TVLV header */
		ptr = (uint8_t *)(tvlv_hdr + 1);
		tvlv_len -= sizeof(*tvlv_hdr);

		len = ntohs(tvlv_hdr->len);
		LEN_CHECK(tvlv_len, (size_t)len, "BAT IV OGM TVLV");

		parser = tvlv_parser_get(tvlv_hdr->type, tvlv_hdr->version);
		if (parser)
			parser(ptr, len);

		/* go to the next container */
		ptr += len;
		tvlv_len -= len;
	}
}

static void dump_batman_icmp(unsigned char *packet_buff, ssize_t buff_len, int read_opt, int time_printed)
{
	struct batadv_icmp_packet *icmp_packet;
	char *name;

	LEN_CHECK((size_t)buff_len - sizeof(struct ether_header), sizeof(struct batadv_icmp_packet), "BAT ICMP");

	icmp_packet = (struct batadv_icmp_packet *)(packet_buff + sizeof(struct ether_header));

	if (!time_printed)
		print_time();

	printf("BAT %s > ",
	       get_name_by_macaddr((struct ether_addr *)icmp_packet->orig, read_opt));

	name = get_name_by_macaddr((struct ether_addr *)icmp_packet->dst, read_opt);

	switch (icmp_packet->msg_type) {
	case BATADV_ECHO_REPLY:
		printf("%s: ICMP echo reply, id %hhu, seq %hu, ttl %2d, v %d, length %zu\n",
			name, icmp_packet->uid, ntohs(icmp_packet->seqno),
			icmp_packet->ttl, icmp_packet->version,
			(size_t)buff_len - sizeof(struct ether_header));
		break;
	case BATADV_ECHO_REQUEST:
		printf("%s: ICMP echo request, id %hhu, seq %hu, ttl %2d, v %d, length %zu\n",
			name, icmp_packet->uid, ntohs(icmp_packet->seqno),
			icmp_packet->ttl, icmp_packet->version,
			(size_t)buff_len - sizeof(struct ether_header));
		break;
	case BATADV_TTL_EXCEEDED:
		printf("%s: ICMP time exceeded in-transit, id %hhu, seq %hu, ttl %2d, v %d, length %zu\n",
			name, icmp_packet->uid, ntohs(icmp_packet->seqno),
			icmp_packet->ttl, icmp_packet->version,
			(size_t)buff_len - sizeof(struct ether_header));
		break;
	default:
		printf("%s: ICMP type %hhu, length %zu\n",
			name, icmp_packet->msg_type,
			(size_t)buff_len - sizeof(struct ether_header));
		break;
	}
}

static void dump_batman_ucast(unsigned char *packet_buff, ssize_t buff_len, int read_opt, int time_printed)
{
	struct ether_header *ether_header;
	struct batadv_unicast_packet *unicast_packet;

	LEN_CHECK((size_t)buff_len - sizeof(struct ether_header), sizeof(struct batadv_unicast_packet), "BAT UCAST");
	LEN_CHECK((size_t)buff_len - sizeof(struct ether_header) - sizeof(struct batadv_unicast_packet),
		sizeof(struct ether_header), "BAT UCAST (unpacked)");

	ether_header = (struct ether_header *)packet_buff;
	unicast_packet = (struct batadv_unicast_packet *)(packet_buff + sizeof(struct ether_header));

	if (!time_printed)
		time_printed = print_time();

	printf("BAT %s > ",
	       get_name_by_macaddr((struct ether_addr *)ether_header->ether_shost, read_opt));

	printf("%s: UCAST, ttvn %d, ttl %hhu, ",
	       get_name_by_macaddr((struct ether_addr *)unicast_packet->dest, read_opt),
	       unicast_packet->ttvn, unicast_packet->ttl);

	parse_eth_hdr(packet_buff + ETH_HLEN + sizeof(struct batadv_unicast_packet),
		      buff_len - ETH_HLEN - sizeof(struct batadv_unicast_packet),
		      read_opt, time_printed);
}

static void dump_batman_bcast(unsigned char *packet_buff, ssize_t buff_len, int read_opt, int time_printed)
{
	struct ether_header *ether_header;
	struct batadv_bcast_packet *bcast_packet;

	LEN_CHECK((size_t)buff_len - sizeof(struct ether_header), sizeof(struct batadv_bcast_packet), "BAT BCAST");
	LEN_CHECK((size_t)buff_len - sizeof(struct ether_header) - sizeof(struct batadv_bcast_packet),
	          sizeof(struct ether_header), "BAT BCAST (unpacked)");

	ether_header = (struct ether_header *)packet_buff;
	bcast_packet = (struct batadv_bcast_packet *)(packet_buff + sizeof(struct ether_header));

	if (!time_printed)
		time_printed = print_time();

	printf("BAT %s: ",
	       get_name_by_macaddr((struct ether_addr *)ether_header->ether_shost, read_opt));

	printf("BCAST, orig %s, seq %u, ",
	       get_name_by_macaddr((struct ether_addr *)bcast_packet->orig, read_opt),
	       ntohl(bcast_packet->seqno));

	parse_eth_hdr(packet_buff + ETH_HLEN + sizeof(struct batadv_bcast_packet),
		      buff_len - ETH_HLEN - sizeof(struct batadv_bcast_packet),
		      read_opt, time_printed);
}

static void dump_batman_4addr(unsigned char *packet_buff, ssize_t buff_len, int read_opt, int time_printed)
{
	struct ether_header *ether_header;
	struct batadv_unicast_4addr_packet *unicast_4addr_packet;

	LEN_CHECK((size_t)buff_len - sizeof(struct ether_header), sizeof(struct batadv_unicast_4addr_packet), "BAT 4ADDR");
	LEN_CHECK((size_t)buff_len - sizeof(struct ether_header) - sizeof(struct batadv_unicast_4addr_packet),
		sizeof(struct ether_header), "BAT 4ADDR (unpacked)");

	ether_header = (struct ether_header *)packet_buff;
	unicast_4addr_packet = (struct batadv_unicast_4addr_packet *)(packet_buff + sizeof(struct ether_header));

	if (!time_printed)
		time_printed = print_time();

	printf("BAT %s > ",
	       get_name_by_macaddr((struct ether_addr *)ether_header->ether_shost, read_opt));

	printf("%s: 4ADDR, subtybe %hhu, ttvn %d, ttl %hhu, ",
	       get_name_by_macaddr((struct ether_addr *)unicast_4addr_packet->u.dest, read_opt),
	       unicast_4addr_packet->subtype, unicast_4addr_packet->u.ttvn,
	       unicast_4addr_packet->u.ttl);

	parse_eth_hdr(packet_buff + ETH_HLEN + sizeof(struct batadv_unicast_4addr_packet),
		      buff_len - ETH_HLEN - sizeof(struct batadv_unicast_4addr_packet),
		      read_opt, time_printed);
}

static void parse_eth_hdr(unsigned char *packet_buff, ssize_t buff_len, int read_opt, int time_printed)
{
	struct batadv_ogm_packet *batman_ogm_packet;
	struct ether_header *eth_hdr;

	eth_hdr = (struct ether_header *)packet_buff;

	switch (ntohs(eth_hdr->ether_type)) {
	case ETH_P_ARP:
		if ((dump_level & DUMP_TYPE_NONBAT) || (time_printed))
			dump_arp(packet_buff + ETH_HLEN, buff_len - ETH_HLEN,
				 eth_hdr, read_opt, time_printed);
		break;
	case ETH_P_IP:
		if ((dump_level & DUMP_TYPE_NONBAT) || (time_printed))
			dump_ip(packet_buff + ETH_HLEN, buff_len - ETH_HLEN, time_printed);
		break;
	case ETH_P_IPV6:
		if ((dump_level & DUMP_TYPE_NONBAT) || (time_printed))
			dump_ipv6(packet_buff + ETH_HLEN, buff_len - ETH_HLEN,
				  time_printed);
		break;
	case ETH_P_8021Q:
		if ((dump_level & DUMP_TYPE_NONBAT) || (time_printed))
			dump_vlan(packet_buff, buff_len, read_opt, time_printed);
		break;
	case ETH_P_BATMAN:
		batman_ogm_packet = (struct batadv_ogm_packet *)(packet_buff + ETH_HLEN);

		if ((read_opt & COMPAT_FILTER) &&
		    (batman_ogm_packet->version != BATADV_COMPAT_VERSION))
			return;

		switch (batman_ogm_packet->packet_type) {
		case BATADV_IV_OGM:
			if (dump_level & DUMP_TYPE_BATOGM)
				dump_batman_iv_ogm(packet_buff, buff_len, read_opt, time_printed);
			break;
		case BATADV_ICMP:
			if (dump_level & DUMP_TYPE_BATICMP)
				dump_batman_icmp(packet_buff, buff_len, read_opt, time_printed);
			break;
		case BATADV_UNICAST:
			if (dump_level & DUMP_TYPE_BATUCAST)
				dump_batman_ucast(packet_buff, buff_len, read_opt, time_printed);
			break;
		case BATADV_BCAST:
			if (dump_level & DUMP_TYPE_BATBCAST)
				dump_batman_bcast(packet_buff, buff_len, read_opt, time_printed);
			break;
		case BATADV_UNICAST_4ADDR:
			if (dump_level & DUMP_TYPE_BATUCAST)
				dump_batman_4addr(packet_buff, buff_len, read_opt, time_printed);
			break;
		case BATADV_UNICAST_TVLV:
			if ((dump_level & DUMP_TYPE_BATUCAST) ||
			    (dump_level & DUMP_TYPE_BATUTVLV))
				dump_batman_ucast_tvlv(packet_buff, buff_len,
						       read_opt, time_printed);
			break;
		default:
			fprintf(stderr, "Warning - packet contains unknown batman packet type: 0x%02x\n", batman_ogm_packet->packet_type);
			break;
		}

		break;

	default:
		fprintf(stderr, "Warning - packet contains unknown ether type: 0x%04x\n", ntohs(eth_hdr->ether_type));
		break;
	}
}

static int monitor_header_length(unsigned char *packet_buff, ssize_t buff_len, int32_t hw_type)
{
	struct radiotap_header *radiotap_hdr;
	switch (hw_type) {
	case ARPHRD_IEEE80211_PRISM:
		if (buff_len <= (ssize_t)PRISM_HEADER_LEN)
			return -1;
		else
			return PRISM_HEADER_LEN;

	case ARPHRD_IEEE80211_RADIOTAP:
		if (buff_len <= (ssize_t)RADIOTAP_HEADER_LEN)
			return -1;

		radiotap_hdr = (struct radiotap_header*)packet_buff;
		if (buff_len <= radiotap_hdr->it_len)
			return -1;
		else
			return radiotap_hdr->it_len;
	}

	return -1;
}

static void parse_wifi_hdr(unsigned char *packet_buff, ssize_t buff_len, int read_opt, int time_printed)
{
	struct ether_header *eth_hdr;
	struct ieee80211_hdr *wifi_hdr;
	unsigned char *shost, *dhost;
	uint16_t fc;
	int hdr_len;

	/* we assume a minimum size of 38 bytes
	 * (802.11 data frame + LLC)
	 * before we calculate the real size */
	if (buff_len <= 38)
		return;

	wifi_hdr = (struct ieee80211_hdr *)packet_buff;
	fc = ntohs(wifi_hdr->frame_control);

	/* not carrying payload */
	if ((fc & IEEE80211_FCTL_FTYPE) != IEEE80211_FTYPE_DATA)
		return;

	/* encrypted packet */
	if (fc & IEEE80211_FCTL_PROTECTED)
		return;

	shost = wifi_hdr->addr2;
	if (fc & IEEE80211_FCTL_FROMDS)
		shost = wifi_hdr->addr3;
	else if (fc & IEEE80211_FCTL_TODS)
		shost = wifi_hdr->addr4;

	dhost = wifi_hdr->addr1;
	if (fc & IEEE80211_FCTL_TODS)
		dhost = wifi_hdr->addr3;

	hdr_len = 24;
	if ((fc & IEEE80211_FCTL_FROMDS) && (fc & IEEE80211_FCTL_TODS))
		hdr_len = 30;

	if (fc & IEEE80211_STYPE_QOS_DATA)
		hdr_len += 2;

	/* LLC */
	hdr_len += 8;
	hdr_len -= sizeof(struct ether_header);

	if (buff_len <= hdr_len)
		return;

	buff_len -= hdr_len;
	packet_buff += hdr_len;

	eth_hdr = (struct ether_header *)packet_buff;
	memmove(eth_hdr->ether_shost, shost, ETH_ALEN);
	memmove(eth_hdr->ether_dhost, dhost, ETH_ALEN);

	 /* printf("parse_wifi_hdr(): ether_type: 0x%04x\n", ntohs(eth_hdr->ether_type));
	printf("parse_wifi_hdr(): shost: %s\n", ether_ntoa_long((struct ether_addr *)eth_hdr->ether_shost));
	printf("parse_wifi_hdr(): dhost: %s\n", ether_ntoa_long((struct ether_addr *)eth_hdr->ether_dhost)); */

	parse_eth_hdr(packet_buff, buff_len, read_opt, time_printed);
}

int tcpdump(int argc, char **argv)
{
	struct ifreq req;
	struct timeval tv;
	struct dump_if *dump_if, *dump_if_tmp;
	struct list_head_first dump_if_list;
	fd_set wait_sockets, tmp_wait_sockets;
	ssize_t read_len;
	int ret = EXIT_FAILURE, res, optchar, found_args = 1, max_sock = 0, tmp;
	int read_opt = USE_BAT_HOSTS;
	unsigned char packet_buff[2000];
	int monitor_header_len = -1;

	dump_level = dump_level_all;

	while ((optchar = getopt(argc, argv, "chnp:x:")) != -1) {
		switch (optchar) {
		case 'c':
			read_opt |= COMPAT_FILTER;
			found_args += 1;
			break;
		case 'h':
			tcpdump_usage();
			return EXIT_SUCCESS;
		case 'n':
			read_opt &= ~USE_BAT_HOSTS;
			found_args += 1;
			break;
		case 'p':
			tmp = strtol(optarg, NULL , 10);
			if ((tmp > 0) && (tmp <= dump_level_all))
				dump_level = tmp;
			found_args += ((*((char*)(optarg - 1)) == optchar ) ? 1 : 2);
			break;
		case 'x':
			tmp = strtol(optarg, NULL , 10);
			if ((tmp > 0) && (tmp <= dump_level_all))
				dump_level &= ~tmp;
			found_args += ((*((char*)(optarg - 1)) == optchar ) ? 1 : 2);
			break;
		default:
			tcpdump_usage();
			return EXIT_FAILURE;
		}
	}

	if (argc <= found_args) {
		fprintf(stderr, "Error - target interface not specified\n");
		tcpdump_usage();
		return EXIT_FAILURE;
	}

	bat_hosts_init(read_opt);

	/* init interfaces list */
	INIT_LIST_HEAD_FIRST(dump_if_list);
	FD_ZERO(&wait_sockets);

	while (argc > found_args) {

		dump_if = malloc(sizeof(struct dump_if));
		memset(dump_if, 0, sizeof(struct dump_if));
		dump_if->raw_sock = -1;
		INIT_LIST_HEAD(&dump_if->list);

		dump_if->dev = argv[found_args];

		if (strlen(dump_if->dev) > IFNAMSIZ - 1) {
			fprintf(stderr, "Error - interface name too long: %s\n", dump_if->dev);
			goto out;
		}

		dump_if->raw_sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

		if (dump_if->raw_sock < 0) {
			fprintf(stderr, "Error - can't create raw socket: %s\n", strerror(errno));
			goto out;
		}

		memset(&req, 0, sizeof (struct ifreq));
		strncpy(req.ifr_name, dump_if->dev, IFNAMSIZ);
		req.ifr_name[sizeof(req.ifr_name) - 1] = '\0';

		res = ioctl(dump_if->raw_sock, SIOCGIFHWADDR, &req);
		if (res < 0) {
			fprintf(stderr, "Error - can't create raw socket (SIOCGIFHWADDR): %s\n", strerror(errno));
			close(dump_if->raw_sock);
			goto out;
		}

		dump_if->hw_type = req.ifr_hwaddr.sa_family;

		switch (dump_if->hw_type) {
		case ARPHRD_ETHER:
		case ARPHRD_IEEE80211_PRISM:
		case ARPHRD_IEEE80211_RADIOTAP:
			break;
		default:
			fprintf(stderr, "Error - interface '%s' is of unknown type: %i\n", dump_if->dev, dump_if->hw_type);
			goto out;
		}

		memset(&req, 0, sizeof (struct ifreq));
		strncpy(req.ifr_name, dump_if->dev, IFNAMSIZ);
		req.ifr_name[sizeof(req.ifr_name) - 1] = '\0';

		res = ioctl(dump_if->raw_sock, SIOCGIFINDEX, &req);

		if (res < 0) {
			fprintf(stderr, "Error - can't create raw socket (SIOCGIFINDEX): %s\n", strerror(errno));
			close(dump_if->raw_sock);
			goto out;
		}

		dump_if->addr.sll_family   = AF_PACKET;
		dump_if->addr.sll_protocol = htons(ETH_P_ALL);
		dump_if->addr.sll_ifindex  = req.ifr_ifindex;

		res = bind(dump_if->raw_sock, (struct sockaddr *)&dump_if->addr, sizeof(struct sockaddr_ll));

		if (res < 0) {
			fprintf(stderr, "Error - can't bind raw socket: %s\n", strerror(errno));
			close(dump_if->raw_sock);
			goto out;
		}

		if (dump_if->raw_sock > max_sock)
			max_sock = dump_if->raw_sock;

		FD_SET(dump_if->raw_sock, &wait_sockets);
		list_add_tail(&dump_if->list, &dump_if_list);
		found_args++;
	}

	while (1) {

		memcpy(&tmp_wait_sockets, &wait_sockets, sizeof(fd_set));

		tv.tv_sec = 1;
		tv.tv_usec = 0;

		res = select(max_sock + 1, &tmp_wait_sockets, NULL, NULL, &tv);

		if (res == 0)
			continue;

		if (res < 0) {
			fprintf(stderr, "Error - can't select on raw socket: %s\n", strerror(errno));
			continue;
		}

		list_for_each_entry(dump_if, &dump_if_list, list) {
			if (!FD_ISSET(dump_if->raw_sock, &tmp_wait_sockets))
				continue;

			read_len = read(dump_if->raw_sock, packet_buff, sizeof(packet_buff));

			if (read_len < 0) {
				fprintf(stderr, "Error - can't read from interface '%s': %s\n", dump_if->dev, strerror(errno));
				continue;
			}

			if ((size_t)read_len < sizeof(struct ether_header)) {
				fprintf(stderr, "Warning - dropping received packet as it is smaller than expected (%zu): %zd\n",
					sizeof(struct ether_header), read_len);
				continue;
			}

			switch (dump_if->hw_type) {
			case ARPHRD_ETHER:
				parse_eth_hdr(packet_buff, read_len, read_opt, 0);
				break;
			case ARPHRD_IEEE80211_PRISM:
			case ARPHRD_IEEE80211_RADIOTAP:
				monitor_header_len = monitor_header_length(packet_buff, read_len, dump_if->hw_type);
				if (monitor_header_len >= 0)
					parse_wifi_hdr(packet_buff + monitor_header_len, read_len - monitor_header_len, read_opt, 0);
				break;
			default:
				/* should not happen */
				break;
			}

			fflush(stdout);
		}

	}

out:
	list_for_each_entry_safe(dump_if, dump_if_tmp, &dump_if_list, list) {
		if (dump_if->raw_sock >= 0)
			close(dump_if->raw_sock);

		list_del((struct list_head *)&dump_if_list, &dump_if->list, &dump_if_list);
		free(dump_if);
	}

	bat_hosts_free();
	return ret;
}
