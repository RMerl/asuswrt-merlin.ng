/*
 * Linux-specific portion of EAPD
 * (OS dependent file)
 *
 * Copyright 2019 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: eapd_linux.c 773571 2019-03-25 18:50:50Z $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <ethernet.h>
#include <eapol.h>
#include <eap.h>
#include <wlutils.h>
#include <bcmnvram.h>
#include <eapd.h>
#include <wlif_utils.h>
#include <UdpLib.h>

#define EAPD_DRV_SOCKET_BUFSIZE	(512*1024)

static eapd_wksp_t *eapd_nwksp = NULL;
static char EAPD_OPT_LIST[] = "hHFi:M";

static void
eapd_hup_hdlr(int sig)
{
	if (eapd_nwksp)
		eapd_nwksp->flags |= EAPD_WKSP_FLAG_SHUTDOWN;

	return;
}

static void
eapd_chld_hdlr(int sig)
{
	pid_t pid;

	while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
		EAPD_INFO("Reaped %d\n", pid);
}

#ifdef EAPDDUMP
static void
eapd_dump_hdlr(int sig)
{
	if (eapd_nwksp)
		eapd_nwksp->flags |= EAPD_WKSP_FLAG_DUMP;

	return;
}
#endif // endif

static int
eapd_send(eapd_wksp_t *nwksp, int drvSocket, struct iovec *frags, int nfrags)
{
	struct msghdr mh;

	memset(&mh, 0, sizeof(mh));
	mh.msg_name = (caddr_t) NULL;
	mh.msg_namelen = 0;
	mh.msg_iov = frags;
	mh.msg_iovlen = nfrags;

	if (sendmsg(drvSocket, &mh, 0) < 0) {
		EAPD_ERROR("send error %d to drvSocket %d\n", errno, drvSocket);
		return errno;
	}
	else {
		EAPD_INFO("send successful on drvSocket %d\n", drvSocket);
	}

	return 0;
}

/* Send a canned EAPOL packet */
/* If BCM_CEVENT is defined, then copy this EAPOL pkt and forward it to cevent */
void
eapd_eapol_canned_send(eapd_wksp_t *nwksp, struct eapd_socket *Socket, eapd_sta_t *sta,
                                                    unsigned char code, unsigned char type)
{
	eapol_header_t eapol;
	eap_header_t eap;
	struct iovec frags[2];

#ifdef BCM_CEVENT
	uint32 src_app = 0;
#endif /* BCM_CEVENT */

	memcpy(&eapol.eth.ether_dhost, &sta->ea, ETHER_ADDR_LEN);
	memcpy(&eapol.eth.ether_shost, &sta->bssid, ETHER_ADDR_LEN);

	eapol.eth.ether_type = htons(ETHER_TYPE_802_1X);
	eapol.version = sta->eapol_version;
	eapol.type = EAP_PACKET;
	eapol.length = htons(type ? (EAP_HEADER_LEN + 1) : EAP_HEADER_LEN);

	eap.code = code;
	eap.id = sta->pae_id;
	eap.length = eapol.length;
	eap.type = type;

#ifdef BCM_CEVENT
	if (eapd_ceventd_enable) {
		/* EAP Request Identity packet built and sent by EAPD to driver.
		 * Make a copy of this packet and forward to cevent
		 */
		src_app |= CEVENT_EAPD;
		cevent_copy_eapol_and_forward(nwksp, sta->ifname, &eapol, &eap, src_app);
	}
#endif /* BCM_CEVENT */

	frags[0].iov_base = (caddr_t) &eapol;
	frags[0].iov_len = EAPOL_HEADER_LEN;
	frags[1].iov_base = (caddr_t) &eap;
	frags[1].iov_len = ntohs(eapol.length);

	eapd_send(nwksp, Socket->drvSocket, frags, 2);
}

void
eapd_message_send(eapd_wksp_t *nwksp, struct eapd_socket *Socket, uint8 *pData, int pLen)
{
	struct iovec frags[1];

	frags[0].iov_base = (caddr_t) pData;
	frags[0].iov_len = pLen;

	eapd_send(nwksp, Socket->drvSocket, frags, 1);
}

int
eapd_brcm_open(eapd_wksp_t *nwksp, eapd_brcm_socket_t *sock)
{
	struct ifreq ifr;
	struct sockaddr_ll ll;
	size_t rcv_buf_size = EAPD_DRV_SOCKET_BUFSIZE;
	size_t snd_buf_size = EAPD_DRV_SOCKET_BUFSIZE;
	socklen_t buf_size = sizeof(rcv_buf_size);

	if (nwksp == NULL || sock == NULL) {
		EAPD_ERROR("Wrong arguments...\n");
		return -1;
	}

	sock->drvSocket = socket(PF_PACKET, SOCK_RAW, htons(ETHER_TYPE_BRCM));
	if (sock->drvSocket < 0) {
		EAPD_ERROR("open socket error!!\n");
		return -1;
	}

	/* Let's bump up the sock buffer sizes for the driver socket
	 * to make sure we don't loose any events from the driver.
	 */
	if ((setsockopt(sock->drvSocket, SOL_SOCKET, SO_RCVBUF, &rcv_buf_size, buf_size)) < 0) {
		EAPD_ERROR("%s: Error setting SO_RCVBUF errno=%d\n", __FUNCTION__, errno);
	}
	if ((setsockopt(sock->drvSocket, SOL_SOCKET, SO_SNDBUF, &snd_buf_size, buf_size)) < 0) {
		EAPD_ERROR("%s: Error setting SO_SNDBUF errno=%d\n", __FUNCTION__, errno);
	}

	/* Read back and log the values for reference */
	rcv_buf_size = snd_buf_size = 0;
	if ((getsockopt(sock->drvSocket, SOL_SOCKET, SO_RCVBUF, &rcv_buf_size, &buf_size)) < 0) {
		EAPD_ERROR("%s: Error getting SO_RCVBUF errno=%d\n", __FUNCTION__, errno);
	}
	if ((getsockopt(sock->drvSocket, SOL_SOCKET, SO_SNDBUF, &snd_buf_size, &buf_size)) < 0) {
		EAPD_ERROR("%s: Error getting SO_SNDBUF errno=%d\n", __FUNCTION__, errno);
	}
	EAPD_PRINT("%s: Sock buffer size  SO_RCVBUF=%zu  SO_SNDBUF=%zu\n",
			__FUNCTION__, rcv_buf_size, snd_buf_size);

	memset(&ifr, 0, sizeof(ifr));

	strcpy(ifr.ifr_name, sock->ifname);
	if (ioctl(sock->drvSocket, SIOCGIFINDEX, &ifr) != 0) {
		EAPD_ERROR("%s, ioctl(SIOCGIFINDEX), close drvSocket %d\n",
			sock->ifname, sock->drvSocket);
		close(sock->drvSocket);
		sock->drvSocket = -1;
		return -1;
	}

	sock->ifindex = ifr.ifr_ifindex;
	memset(&ll, 0, sizeof(ll));
	ll.sll_family = AF_PACKET;
	ll.sll_protocol = htons(ETHER_TYPE_BRCM);
	ll.sll_ifindex = sock->ifindex;

	if (bind(sock->drvSocket, (struct sockaddr *) &ll, sizeof(ll)) < 0) {
		EAPD_ERROR("%s, bind fail, close drvSocket %d!!\n",
			sock->ifname, sock->drvSocket);
		close(sock->drvSocket);
		sock->drvSocket = -1;
		return -1;
	}
	/* at least one use it */
	sock->inuseCount = 1;

	EAPD_INFO("%s: BRCM socket %d opened\n", ifr.ifr_name, sock->drvSocket);

	return 0;
}

int
eapd_brcm_close(int drvSocket)
{
	close(drvSocket);
	return 0;
}

int
eapd_preauth_open(eapd_wksp_t *nwksp, eapd_preauth_socket_t *sock)
{
	struct ifreq ifr;
	struct sockaddr_ll ll;

	if (nwksp == NULL || sock == NULL) {
		EAPD_ERROR("Wrong arguments...\n");
		return -1;
	}

	sock->drvSocket = socket(PF_PACKET, SOCK_RAW, htons(ETHER_TYPE_802_1X_PREAUTH));
	if (sock->drvSocket < 0) {
		EAPD_ERROR("open socket error!!\n");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));

	strcpy(ifr.ifr_name, sock->ifname);
	if (ioctl(sock->drvSocket, SIOCGIFINDEX, &ifr) != 0) {
		EAPD_ERROR("%s, ioctl(SIOCGIFINDEX), close drvSocket %d\n",
			sock->ifname, sock->drvSocket);
		close(sock->drvSocket);
		sock->drvSocket = -1;
		return -1;
	}

	sock->ifindex = ifr.ifr_ifindex;
	memset(&ll, 0, sizeof(ll));
	ll.sll_family = AF_PACKET;
	ll.sll_protocol = htons(ETHER_TYPE_802_1X_PREAUTH);
	ll.sll_ifindex = sock->ifindex;

	if (bind(sock->drvSocket, (struct sockaddr *) &ll, sizeof(ll)) < 0) {
		EAPD_ERROR("%s, bind fail, close drvSocket %d!!\n", sock->ifname, sock->drvSocket);
		close(sock->drvSocket);
		sock->drvSocket = -1;
		return -1;
	}
	/* at least one use it */
	sock->inuseCount = 1;

	EAPD_INFO("%s: preauth socket %d opened\n", ifr.ifr_name, sock->drvSocket);

	return 0;
}

int
eapd_preauth_close(int drvSocket)
{
	close(drvSocket);
	return 0;
}

/*
 * Configuration APIs
 */
int
eapd_safe_get_conf(char *outval, int outval_size, char *name)
{
	char *val;

	if (name == NULL || outval == NULL) {
		if (outval)
			memset(outval, 0, outval_size);
		return -1;
	}

	val = nvram_safe_get(name);
	if (!strcmp(val, ""))
		memset(outval, 0, outval_size);
	else
		snprintf(outval, outval_size, "%s", val);
	return 0;
}

int main(int argc, char* argv[])
{
	int opt;
	int auto_config = 1;
	char * nv_ceventd_enable;
#ifdef BCMDBG
	char *dbg;
#endif // endif

#ifdef BCMDBG
	/* get eapd_msg_level from nvram */
	if ((dbg = nvram_get("eapd_dbg"))) {
		eapd_msg_level = (uint)strtoul(dbg, NULL, 0);
	}
#endif // endif

	nv_ceventd_enable = nvram_safe_get("ceventd_enable");

	if (nv_ceventd_enable && nv_ceventd_enable[0] && nv_ceventd_enable[0] == '1' &&
			nv_ceventd_enable[1] == '\0') {
		eapd_ceventd_enable = TRUE;
	}

	EAPD_INFO("EAP Dispatch Start...\n");
	/* alloc eapd work space */
	if (!(eapd_nwksp = eapd_wksp_alloc_workspace())) {
		EAPD_ERROR("Unable to allocate wksp memory. Quitting...\n");
		return -1;
	}

	/* assign loopback address as default */
	eapd_nwksp->s_addr = htonl(INADDR_LOOPBACK);

	while ((opt = getopt(argc, argv, EAPD_OPT_LIST)) != EOF) {
		switch (opt) {
#ifdef BCMDBG
		case 'h':
		case 'H':
			/* display usage if nothing is specified */
			if (argc == 2) {
				eapd_wksp_display_usage();
				return 0;
			}
			break;
#endif // endif
		case 'F':
			eapd_nwksp->foreground = 1;
			break;
		case 'i':
			eapd_nwksp->s_addr = inet_addr(optarg);
			break;
		case 'M':
			/* Manual config eapd */
			auto_config = 0;
			break;
		default:
			break;
		}
	}

	EAPD_INFO("eapd listen to s_addr %#lx\n", eapd_nwksp->s_addr);

#if EAPD_WKSP_AUTO_CONFIG
	/* auto config */
	if (auto_config) {
		if (eapd_wksp_auto_config(eapd_nwksp)) {
			EAPD_ERROR("Unable to auto config. Quitting...\n");
			eapd_wksp_cleanup(eapd_nwksp);
			return -1;
		}
	}
	else
#endif	/* EAPD_WKSP_AUTO_CONFIG */
	if (eapd_wksp_parse_cmd(argc, argv, eapd_nwksp)) {
		EAPD_ERROR("Command line parsing error. Quitting...\n");
		eapd_wksp_cleanup(eapd_nwksp);
		return -1;
	}

	/* establish a handler to handle SIGTERM. */
	signal(SIGTERM, eapd_hup_hdlr);

#ifdef EAPDDUMP
	signal(SIGUSR1, eapd_dump_hdlr);
#endif // endif

	signal(SIGCHLD, eapd_chld_hdlr);

	/* run main loop to dispatch messages */
	eapd_wksp_main_loop(eapd_nwksp);

	EAPD_INFO("EAP Dispatcher Stopped...\n");

	return 0;
}

size_t
eapd_message_read(int fd, void *buf, size_t nbytes)
{
	return (read(fd, buf, nbytes));
}
