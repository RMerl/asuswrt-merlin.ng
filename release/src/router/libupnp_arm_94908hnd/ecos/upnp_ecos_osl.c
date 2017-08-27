/*
 * Broadcom UPnP module eCos OS dependent implementation
 *
 * Copyright (C) 2015, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_ecos_osl.c 257213 2011-05-04 11:14:46Z $
 */

#include <upnp.h>

/*
 * The following functions are required by the
 * upnp engine, which the OSL has to implement.
 */
int
upnp_osl_ifaddr(const char *ifname, struct in_addr *inaddr)
{
	int sockfd;
	struct ifreq ifreq;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("%s:Cannot open socket\n", __func__);
		return -1;
	}

	strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
	ifreq.ifr_name[IFNAMSIZ-1] = '\0';
	if (ioctl(sockfd, SIOCGIFADDR, &ifreq) < 0) {
		close(sockfd);
		return -1;
	}
	else {
		memcpy(inaddr, &(((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr),
			sizeof(struct in_addr));
	}

	close(sockfd);
	return 0;
}

int
upnp_osl_netmask(const char *ifname, struct in_addr *inaddr)
{
	int sockfd;
	struct ifreq ifreq;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("%s:Cannot open socket\n", __func__);
		return -1;
	}

	strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
	ifreq.ifr_name[IFNAMSIZ-1] = '\0';
	if (ioctl(sockfd, SIOCGIFNETMASK, &ifreq) < 0) {
		close(sockfd);
		return -1;
	}
	else {
		memcpy(inaddr, &(((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr),
			sizeof(struct in_addr));
	}

	close(sockfd);
	return 0;
}

int
upnp_osl_hwaddr(const char *ifname, char *mac)
{
	int sockfd;
	struct ifreq  ifreq;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("%s:Cannot open socket\n", __func__);
		return -1;
	}

	strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
	ifreq.ifr_name[IFNAMSIZ-1] = '\0';
	if (ioctl(sockfd, SIOCGIFHWADDR, &ifreq) < 0) {
		close(sockfd);
		return -1;
	}
	else {
		memcpy(mac, ifreq.ifr_hwaddr.sa_data, 6);
	}

	close(sockfd);
	return 0;
}

/* Create a udp socket with a specific ip and port */
int
upnp_open_udp_socket(struct in_addr addr, unsigned short port)
{
	int s;
	int reuse = 1;
	struct sockaddr_in sin;

	/* create UDP socket */
	s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		printf("%s:Cannot open socket\n", __func__);
		return -1;
	}

	if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
		upnp_syslog(LOG_ERR, "Cannot set socket option (SO_REUSEPORT)");
	}

	/* bind socket to recive discovery */
	memset((char *)&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr = addr;

	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		upnp_syslog(LOG_ERR, "bind failed!");
		close(s);
		return -1;
	}

	return s;
}

/* Create a tcp socket with a specific ip and port */
int
upnp_open_tcp_socket(struct in_addr addr, unsigned short port)
{
	int s;
	int reuse = 1;
	struct sockaddr_in sin;

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		printf("%s:Cannot open socket\n", __func__);
		return -1;
	}

	if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
		upnp_syslog(LOG_ERR, "Cannot set socket option (SO_REUSEPORT)");
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr = addr;

	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0 ||
	    listen(s, MAX_WAITS) < 0) {
		close(s);
		return -1;
	}

	return s;
}
