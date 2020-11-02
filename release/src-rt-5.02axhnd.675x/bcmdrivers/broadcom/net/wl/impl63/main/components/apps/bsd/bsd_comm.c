/*
 * bsd deamon (Linux)
 *
 * Copyright 2020 Broadcom
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
 * $Id: bsd_main.c $
 */
#include "bsd.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/* open a UDP packet to event dispatcher for receiving/sending data */
/* bss=1: create new special socket for bss trsansit response event */
static int bsd_open_eventfd_sub(bsd_info_t*info, int bss)
{
	int reuse = 1;
	struct sockaddr_in sockaddr;
	int fd = BSD_DFLT_FD;

	BSD_ENTER();
	/* open loopback socket to communicate with event dispatcher */
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	if (bss)
		sockaddr.sin_port = htons(EAPD_WKSP_BSD_UDP_MPORT);
	else
		sockaddr.sin_port = htons(EAPD_WKSP_BSD_UDP_SPORT);

	if ((fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		BSD_ERROR("Unable to create loopback socket\n");
		goto error;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0) {
		BSD_ERROR("Unable to setsockopt to loopback socket %d.\n", fd);
		goto error;
	}

	if (bind(fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0) {
		BSD_ERROR("Unable to bind to loopback socket %d\n", fd);
		goto error;
	}

	BSD_INFO("opened loopback socket %d\n", fd);
	if (bss)
		info->event_fd2 = fd;
	else
		info->event_fd = fd;

	BSD_EXIT();
	return BSD_OK;

	/* error handling */
error:
	if (fd != BSD_DFLT_FD)
		close(fd);
	BSD_EXIT();
	return BSD_FAIL;
}

int bsd_open_eventfd(bsd_info_t*info)
{
	if (bsd_open_eventfd_sub(info, 0) == BSD_FAIL) {
		BSD_ERROR("Fail to init event socket\n");
		return BSD_FAIL;
	}

	if (bsd_open_eventfd_sub(info, 1) == BSD_FAIL) {
		BSD_ERROR("Fail to init bss event socket\n");
		return BSD_FAIL;
	}

	return BSD_OK;
}

/* open TCP socket to receive rpc event */
int bsd_open_rpc_eventfd(bsd_info_t*info)
{
	int reuse = 1;
	int	listenfd = BSD_DFLT_FD;
	struct sockaddr_in	sockaddr;

	BSD_ENTER();

	if ((info->role != BSD_ROLE_PRIMARY) &&
		(info->role != BSD_ROLE_HELPER)) {
		BSD_INFO("no rpc socket created for standalone mode\n");
		goto done;
	}

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		BSD_ERROR("Unable to create rpc listen socket\n");
		goto error;
	}

	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0) {
		BSD_ERROR("Unable to setsockopt to rpc listen socket %d.\n", listenfd);
		goto error;
	}

	bzero(&sockaddr, sizeof(sockaddr));
	sockaddr.sin_family      = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	switch (info->role) {
		case BSD_ROLE_PRIMARY:
			sockaddr.sin_port = htons(info->pport);
			break;
		case BSD_ROLE_HELPER:
			sockaddr.sin_port = htons(info->hport);
			break;
		default:
			BSD_INFO("no rpc socket created\n");
			break;
	}

	if (bind(listenfd, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) < 0) {
		BSD_ERROR("Unable to bind socket %d.\n", listenfd);
		goto error;
	}

	if (listen(listenfd, 2) < 0) {
		BSD_ERROR("listen() fails.\n");
		goto error;
	}
	info->rpc_listenfd = listenfd;

done:
	BSD_EXIT();
	return BSD_OK;

error:
	if (listenfd != BSD_DFLT_FD)
		close(listenfd);
	BSD_EXIT();
	return BSD_FAIL;
}

void bsd_close_rpc_eventfd(bsd_info_t*info)
{
	BSD_ENTER();
	if (info->rpc_listenfd != BSD_DFLT_FD) {
		BSD_INFO("close rpc_listenfd %d\n", info->rpc_listenfd);
		close(info->rpc_listenfd);
		info->rpc_listenfd = BSD_DFLT_FD;
	}
	if (info->rpc_eventfd != BSD_DFLT_FD) {
		BSD_INFO("close rpc_eventfd %d\n", info->rpc_eventfd);
		close(info->rpc_eventfd);
		info->rpc_eventfd = BSD_DFLT_FD;
	}
	if (info->rpc_ioctlfd != BSD_DFLT_FD) {
		BSD_INFO("close rpc_ioctlfd %d\n", info->rpc_ioctlfd);
		close(info->rpc_ioctlfd);
		info->rpc_ioctlfd = BSD_DFLT_FD;
	}
	BSD_EXIT();
	return;
}

void bsd_close_eventfd(bsd_info_t*info)
{
	BSD_ENTER();
	/* close event dispatcher socket */
	if (info->event_fd != BSD_DFLT_FD) {
		BSD_INFO("close loopback event_fd %d\n", info->event_fd);
		close(info->event_fd);
		info->event_fd = BSD_DFLT_FD;
	}
	/* close bss event dispatcher socket */
	if (info->event_fd2 != BSD_DFLT_FD) {
		BSD_INFO("close loopback event_fd2 %d\n", info->event_fd2);
		close(info->event_fd2);
		info->event_fd2 = BSD_DFLT_FD;
	}
	BSD_EXIT();
	return;
}

/* Msg dispatch */
static int bsd_validate_message(int bytes, uint8 *dpkt)
{
	bcm_event_t *pvt_data;

	BSD_EVTENTER();
	/* the message should be at least the header to even look at it */
	if (bytes < sizeof(bcm_event_t) + 2) {
		BSD_ERROR("Invalid length of message\n");
		return BSD_FAIL;
	}
	pvt_data  = (bcm_event_t *)dpkt;
	if (ntohs(pvt_data->bcm_hdr.subtype) != BCMILCP_SUBTYPE_VENDOR_LONG) {
		BSD_ERROR("%s: not vendor specifictype\n",
		       pvt_data->event.ifname);
		return BSD_FAIL;
	}
	if (pvt_data->bcm_hdr.version != BCMILCP_BCM_SUBTYPEHDR_VERSION) {
		BSD_ERROR("%s: subtype header version mismatch\n",
			pvt_data->event.ifname);
		return BSD_FAIL;
	}
	if (ntohs(pvt_data->bcm_hdr.length) < BCMILCP_BCM_SUBTYPEHDR_MINLENGTH) {
		BSD_ERROR("%s: subtype hdr length not even minimum\n",
			pvt_data->event.ifname);
		return BSD_FAIL;
	}
	if (bcmp(&pvt_data->bcm_hdr.oui[0], BRCM_OUI, DOT11_OUI_LEN) != 0) {
		BSD_ERROR("%s: bsd_validate_wlpvt_message: not BRCM OUI\n",
			pvt_data->event.ifname);
		return BSD_FAIL;
	}
	/* check for wl dcs message types */
	switch (ntohs(pvt_data->bcm_hdr.usr_subtype)) {
		case BCMILCP_BCM_SUBTYPE_EVENT:
			BSD_EVENT("subtype: event\n");
			break;
		default:
			return BSD_FAIL;
	}
	BSD_EVTEXIT();
	return BSD_OK; /* good packet may be this is destined to us */
}

static int bsd_proc_event(bsd_info_t*info, uint8 remote, char *pkt, int bytes)
{
	bcm_event_t *pvt_data;
	struct ether_addr *addr;
	char *ifname;
	struct ether_header *eth_hdr;
	uint16 ether_type;
	uint32 evt_type;
	int err;
	wl_psta_primary_intf_event_t *event;
	bsd_intf_info_t *intf_info = NULL;
	bsd_bssinfo_t *bssinfo = NULL;
	int intfidx, bssidx;

	BSD_EVTENTER();

	ifname = (char *)pkt;
	eth_hdr = (struct ether_header *)(ifname + IFNAMSIZ);

	BSD_EVENT("recved %d bytes from eventfd, ifname: %s\n",	bytes, ifname);

	if ((ether_type = ntohs(eth_hdr->ether_type) != ETHER_TYPE_BRCM)) {
		BSD_EVENT("recved ether type %x\n", ether_type);
		return BSD_FAIL;
	}

	if ((err = bsd_validate_message(bytes - IFNAMSIZ, (uint8 *)eth_hdr))) {
		BSD_EVENT("Err msg\n");
		return BSD_FAIL;
	}

	pvt_data = (bcm_event_t *)(ifname + IFNAMSIZ);
	evt_type = ntoh32(pvt_data->event.event_type);

	addr = (struct ether_addr *)(&(pvt_data->event.addr));

	if ((evt_type != WLC_E_PROBREQ_MSG) ||
		((evt_type == WLC_E_PROBREQ_MSG) && BSD_PROB_ENAB)) {
		/* too many probe. only handle proble for dump level */
		BSD_EVENT("Evttype:%d info:%s Mac:"MACF"bsscfgidx=0x%x ifidx=0x%x\n",
			evt_type, ifname, ETHERP_TO_MACF(addr),
			pvt_data->event.bsscfgidx, pvt_data->event.ifidx);
	}

	switch (evt_type) {
		case WLC_E_DEAUTH:
		case WLC_E_DEAUTH_IND:
			BSD_EVENT("Deauth_ind\n");
			bsd_deauth_sta(info, ifname, remote, addr);
			break;
		case WLC_E_DISASSOC_IND:
			/* update sta info list */
			BSD_EVENT("Disassoc\n");
			bsd_disassoc_sta(info, ifname,  remote, addr);
			break;
		case WLC_E_AUTH_IND:
			BSD_EVENT("WLC_E_AUTH_IND\n");
			/* some STA (e.g. iphone6 with random mac) may send auth without probe */
			bsd_add_prbsta(info, ifname, remote, addr);
			bsd_auth_sta(info, ifname,  remote, addr);
			break;
		case WLC_E_REASSOC_IND:
			BSD_EVENT("ReAssoc\n");
			bsd_assoc_sta(info, ifname,  remote, addr);
			break;
		case WLC_E_ASSOC_IND:
			BSD_EVENT("Assoc\n");
			bsd_assoc_sta(info, ifname,  remote, addr);
			break;

		case WLC_E_PSTA_PRIMARY_INTF_IND:
			event = (wl_psta_primary_intf_event_t *)(pvt_data + 1);

			BSD_EVENT("p-Mac:"MACF"\n", ETHER_TO_MACF(event->prim_ea));

			bsd_update_psta(info, ifname,  remote, addr, &event->prim_ea);

			if (BSD_DUMP_ENAB)
				bsd_dump_info(info);
			break;

		case WLC_E_PROBREQ_MSG:
			BSD_EVENT("Probe-req...\n");

			/* Loop through all interfaces */
			for (intfidx = 0; intfidx < info->max_ifnum; intfidx++) {
				intf_info = &(info->intf_info[intfidx]);
				bssinfo = &(intf_info->bsd_bssinfo[0]);

				/* skip if primary ifname doesn't match the event ifname */
				if (strncmp(ifname, bssinfo->ifnames, strlen(ifname))) {
					continue;
				}

				/* bsd_add_prbsta() on first valid bsd bss */
				for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
					bssinfo = &(intf_info->bsd_bssinfo[bssidx]);

					BSD_EVENT("bssinfo->ifnames:%s bssidx=%d\n",
						bssinfo->ifnames, bssidx);

					if (bssinfo->valid && BSD_BSS_BSD_ENABLED(bssinfo)) {
						bsd_add_prbsta(info, bssinfo->ifnames,
								remote, addr);
						break;
					}
				}
				break;
			}
			break;

		case WLC_E_BSSTRANS_RESP:
			BSD_EVENT("BSS Transit Response: "MACF"\n", ETHERP_TO_MACF(addr));
			break;
#ifdef BCM_WBD
		case WLC_E_DFS_AP_RESUME:
			if ((info->enable_flag & BSD_FLAG_WBD_ENABLED) && (info->wbd_info)) {
				BSD_EVENT("DFS_AP_RESUME event ..\n");
				bsd_wbd_update_bss_info(info, ifname);
			} else {
				BSD_EVENT("DFS_AP_RESUME event skip, WBD is not enabled..\n");
			}
			break;
#endif /* BCM_WBD */
		default:
			BSD_INFO("recved event type: 0x%x\n", evt_type);
			break;
	}

	BSD_EVTEXIT();
	return BSD_OK; /* good packet may be this is destined to us */
}

static int bsd_helper_proc_rpc(bsd_info_t*info)
{
	bsd_rpc_pkt_t *pkt = (bsd_rpc_pkt_t *)ret_buf;
	char *ptr;
	int bytes;

	char name[128], val[128];
	int size, resp_len;
	int status = BSD_OK, ret;

	BSD_ENTER();

	memset(ret_buf, 0, sizeof(ret_buf));
	if ((bytes = read(info->rpc_ioctlfd, ret_buf, sizeof(ret_buf))) <= 0) {
		BSD_RPCD("Err: Socket: Recv rpc ioctl. close rpc_ioctlfd[%d]"
			", bytes:%d. errno=%d\n", info->rpc_ioctlfd, bytes, errno);
		close(info->rpc_ioctlfd);
		info->rpc_ioctlfd = BSD_DFLT_FD;

		BSD_RPCD("Err: Socket: Also close Event[%d] socket.\n",
			info->rpc_eventfd);
		close(info->rpc_eventfd);
		info->rpc_eventfd = BSD_DFLT_FD;
		status = BSD_FAIL;
		goto done;
	}

	BSD_RPCD("\n\nRecv ioctl rpc_ioctlfd[%d]: bytes:%d"
		"(sizeof(ret_buf)=%zu) id:%d \n",
		info->rpc_ioctlfd, bytes, sizeof(ret_buf), pkt->id);
	BSD_RPC("raw Rcv buff[sock:%d]: cmd:%d name:%s len:%d\n",
		info->rpc_ioctlfd, pkt->cmd.cmd, pkt->cmd.name, pkt->cmd.len);
	bsd_rpc_dump((char *)pkt, 64, BSD_RPC_ENAB);

	if (pkt->cmd.name[0] == '\0') {
		BSD_ERROR("null intf name skipped\n");
		status = BSD_FAIL;
		goto done;
	}

	switch (pkt->id) {
		case BSD_RPC_ID_NVRAM:
		{
			BSDSTRNCPY(name, (char *)(pkt+1), sizeof(name));
			BSDSTRNCPY(val, nvram_safe_get(name), sizeof(val) - 1);

			BSD_RPCD("nvram:%s=%s\n", name, val);

			strcpy((char *)(pkt + 1), val);
			pkt->cmd.len = strlen(val) + 4;
			resp_len = sizeof(ret_buf);

			size = write(info->rpc_ioctlfd, (void *)pkt, resp_len);
			BSD_RPC("nvram writing: size=%d sent:%d\n",
				resp_len, size);

			if (size != resp_len) {
				BSD_RPCD("Err: Socket: sending[%d] Sent[%d] "
					"close rpc_ioctlfd[%d]\n",
					resp_len, size, info->rpc_ioctlfd);
				status = BSD_FAIL;
				/*
				close(info->rpc_ioctlfd);
				info->rpc_ioctlfd = BSD_DFLT_FD;
				*/
			}
			break;
		}
		case BSD_RPC_ID_IOCTL:
		{
			BSD_RPCD("BSD_RPC_ID_IOCTL: id:%d name:%s cmd:%d len:%d"
				" bytes:%d (sizeof(ret_buf))=%zu \n",
				pkt->id, pkt->cmd.name, pkt->cmd.cmd, pkt->cmd.len,
				bytes, sizeof(ret_buf));
			ptr = (char *)(pkt+1);

			BSD_RPC("raw Rcv ioctl buff:\n");
			bsd_rpc_dump(ptr, 64, BSD_RPC_ENAB);

			ret = wl_ioctl(pkt->cmd.name, pkt->cmd.cmd,
				ptr, pkt->cmd.len);
			BSD_RPCD("ret=%d\n", ret);
			if (ret < 0) {
				BSD_ERROR("wl_ioctl fails: cmd:%d name:%s len:%d\n",
					pkt->cmd.cmd, pkt->cmd.name, pkt->cmd.len);
				status = BSD_FAIL;
				break;
			}

			BSD_RPC("raw Send buff[sock:%d: cmd:%d name:%s len:%d\n",
				info->rpc_ioctlfd, pkt->cmd.cmd,
				pkt->cmd.name, pkt->cmd.len);
			bsd_rpc_dump((char *)pkt, 64, BSD_RPC_ENAB);

			pkt->cmd.ret = ret;
			resp_len = bytes;

			size = write(info->rpc_ioctlfd, (void *)pkt,
				sizeof(ret_buf));
			BSD_RPC("ioctl writing: size=%zu, sent:%d\n",
				sizeof(ret_buf), size);

			if (size != sizeof(ret_buf)) {
				BSD_RPCD("Err: Socket: sending[%d] Sent[%d]"
					" close rpc_ioctlfd[%d]\n",
					resp_len, size, info->rpc_ioctlfd);
				status = BSD_FAIL;
				/*
				close(info->rpc_ioctlfd);
				info->rpc_ioctlfd = BSD_DFLT_FD;
				*/
			}
			break;
		}
		default:
			BSD_ERROR("Wrong cmd id. Ignose\n");
			break;
	}

done:
	BSD_EXIT();
	return status;
}

/* coverity[ -tainted_string_sanitize_content : arg-1 ]  */
static int bsd_helper_proc_event(bsd_info_t*info, char *pkt, int bytes)
{
	bcm_event_t *pvt_data;
	unsigned char *addr;
	char *ifname;
	struct ether_header *eth_hdr;
	uint16 ether_type = 0;
	uint32 evt_type;
	int err;

	int	sockfd = BSD_DFLT_FD;
	struct sockaddr_in	servaddr;
	int size;
	int status = BSD_OK;

	BSD_EVTENTER();

	ifname = (char *)pkt;
	eth_hdr = (struct ether_header *)(ifname + IFNAMSIZ);

	BSD_EVENT("recved %d bytes from eventfd, ifname: %s\n",	bytes, ifname);
	BSD_EVENT("[1]time=%lu\n", (unsigned long)time(NULL));

	if ((ether_type = ntohs(eth_hdr->ether_type) != ETHER_TYPE_BRCM)) {
		BSD_EVENT("recved ether type %x\n", ether_type);
		status = BSD_FAIL;
		goto done;
	}

	if ((err = bsd_validate_message(bytes - IFNAMSIZ, (uint8 *)eth_hdr))) {
		BSD_EVENT("Err msg\n");
		status = BSD_FAIL;
		goto done;
	}

	pvt_data = (bcm_event_t *)(ifname + IFNAMSIZ);
	evt_type = ntoh32(pvt_data->event.event_type);

	addr = (unsigned char *)(&(pvt_data->event.addr));

	if ((evt_type != WLC_E_PROBREQ_MSG) ||
		((evt_type == WLC_E_PROBREQ_MSG) && BSD_PROB_ENAB)) {
		/* too many probe. only handle proble for dump level */
		BSD_EVENT("Evt:%d ifname:%s Mac:"MACF" bsscfgidx:0x%x "
			"ifidx:0x%x\n",
			evt_type, ifname, ETHERP_TO_MACF(addr),
			pvt_data->event.bsscfgidx, pvt_data->event.ifidx);
	}

	BSD_RPCEVT("Forward event to %s[%d]\n", info->primary_addr, info->pport);

	BSD_RPCEVT("Raw Event[sock:%d] [ifname:%s]\n",
		info->rpc_eventfd, (char *)pkt);
	bsd_rpc_dump((char *)pkt, 64, BSD_RPCEVT_ENAB);

	if (info->rpc_eventfd < 0) {
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		BSD_RPCEVT("Create sock to forward event%d\n", sockfd);
		if (sockfd < 0) {
			BSD_ERROR("Err: open socket=%d\n", sockfd);
			status = BSD_FAIL;
			goto done;
		}

		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(info->pport);

		servaddr.sin_addr.s_addr = inet_addr(info->primary_addr);
		if (connect(sockfd, (const struct sockaddr *)&servaddr,
			sizeof(servaddr)) < 0) {
			BSD_ERROR("Err: Cannot connect to Primary: %s[%d]\n",
				info->primary_addr, info->pport);
			status = BSD_FAIL;
			if (info->rpc_eventfd != BSD_DFLT_FD) {
				close(info->rpc_eventfd);
				info->rpc_eventfd = BSD_DFLT_FD;
			}
			if (sockfd !=  BSD_DFLT_FD) {
				close(sockfd);
			}
			goto done;
		}

		info->rpc_eventfd = sockfd;
	}

	if (info->rpc_eventfd > 0) {
		BSD_RPCEVT("RPCEVENT[%s] : Writing:[%d]\n",
			(char *)pkt, bytes);
		size = write(info->rpc_eventfd, pkt, bytes);
		BSD_RPCEVT("RPCEVENT[%s] : Wrote:[%d]\n", (char *)pkt, size);

		if (size != bytes) {
			BSD_RPCEVT("Err: Socket: close rpc_eventfd[%d]: writing:%d"
				"Wrote:%d Close to reopen\n",
				info->rpc_eventfd, bytes, size);
			if (info->rpc_eventfd != BSD_DFLT_FD) {
				close(info->rpc_eventfd);
				info->rpc_eventfd = BSD_DFLT_FD;
			}
			status = BSD_FAIL;
		}
	}

	BSD_RPCEVT("[2]time=%lu\n", (unsigned long)time(NULL));

done:
	BSD_EVTEXIT();
	return status;
}

/*
 * process_response() - common function to process the client requests.
 *
 */
static int
bsd_proc_cmd(uint8 cmd_id)
{
	int ret = BSD_OK;

	BSD_ENTER();
	/* Check if we have command and data in the expected order */
	switch (cmd_id) {
		case BSD_CMD_CONFIG_INFO:
			bsd_info_hdlr();
			break;
		case BSD_CMD_STA_INFO:
			bsd_sta_hdlr();
			break;
		case BSD_CMD_STEER_LOG:
			bsd_log_hdlr();
			break;
		case BSD_CMD_STA_CONFIG_HDLR:
			bsd_sta_config_hdlr();
			break;
		case BSD_CMD_STA_STATS:
			bsd_query_sta_stats_hdlr();
			break;
		case BSD_CMD_RADIO_STATS:
			bsd_query_radio_stats_hdlr();
			break;
		case BSD_CMD_CHANGE_TTY_CONSOLE:
			bsd_tty_hdlr();
			break;
		default :
			BSD_INFO("Invalid  BSD command ID\n");
			break;
	}
	BSD_EXIT();
	return ret;
}

/*
 * Receives and processes the commands from client
 * o Wait for connection from client
 * o Process the command and respond back to client
 * o close connection with client
 */
static int
bsd_proc_client_req(bsd_info_t *info)
{
	int ret = BSD_OK;
	int fd = -1;
	uint8 cmd_id;
	struct sockaddr_in cliaddr;
	socklen_t len; /* need initialize here to avoid EINVAL */

	BSD_ENTER();

	len = sizeof(cliaddr);
	fd = accept(info->cli_listenfd, (struct sockaddr *)&cliaddr, &len);
	if (fd < 0) {
		if (errno == EINTR) return 0;
		else {
			BSD_ERROR("accept failed: errno: %d - %s\n", errno, strerror(errno));
			return -1;
		}
	}
	/* get command from client */
	if (read(fd, &(cmd_id), sizeof(cmd_id)) < 0) {
		BSD_ERROR("Failed reading message from client: %s\n", strerror(errno));
		ret = -1;
		goto done;
	}
	/* Process the response */
	bsd_proc_cmd(cmd_id);

done:
	BSD_EXIT();
	bsd_close_socket(&fd);
	return ret;
}

/* listen to sockets and call handlers to process packets */
void bsd_proc_socket(bsd_info_t*info, struct timeval *tv)
{
	fd_set fdset;
	int fdmax;
	int width, status = 0, bytes;
	uint8 pkt[BSD_BUFSIZE_4K];

	BSD_ENTER();

	/* init file descriptor set */
	FD_ZERO(&fdset);
	fdmax = -1;

	/* build file descriptor set now to save time later */
	if (info->event_fd != BSD_DFLT_FD) {
		FD_SET(info->event_fd, &fdset);
		fdmax = info->event_fd;
	}

	/* build file descriptor set now to save time later */
	if (info->rpc_listenfd != BSD_DFLT_FD) {
		FD_SET(info->rpc_listenfd, &fdset);
		if (fdmax < info->rpc_listenfd)
			fdmax = info->rpc_listenfd;
	}

	if (info->role == BSD_ROLE_PRIMARY) {
		/* build file descriptor set now to save time later */
		if (info->rpc_eventfd != BSD_DFLT_FD) {
			FD_SET(info->rpc_eventfd, &fdset);
			if (fdmax < info->rpc_eventfd)
				fdmax = info->rpc_eventfd;
		}
	}

	if (info->role == BSD_ROLE_HELPER) {
		/* build file descriptor set now to save time later */
		if (info->rpc_ioctlfd != BSD_DFLT_FD) {
			FD_SET(info->rpc_ioctlfd, &fdset);
			if (fdmax < info->rpc_ioctlfd)
				fdmax = info->rpc_ioctlfd;
		}
	}
	/* build file descriptor set now to save time later */
	if (info->cli_listenfd != BSD_DFLT_FD) {
		FD_SET(info->cli_listenfd, &fdset);
		if (info->cli_listenfd > fdmax) {
			fdmax = info->cli_listenfd;
		}
	}

	width = fdmax + 1;

	/* listen to data availible on all sockets */
	status = select(width, &fdset, NULL, NULL, tv);

	if ((status == -1 && errno == EINTR) || (status == 0)) {
		BSD_EVENT("No event\n");
		goto done;
	}

	if (status <= 0) {
		BSD_ERROR("err from select: %s", strerror(errno));
		goto done;
	}

		/* handle rpc brcm event */
	if (info->rpc_listenfd !=  BSD_DFLT_FD && FD_ISSET(info->rpc_listenfd, &fdset)) {
		int	 connfd = -1;
		socklen_t			clilen;
		struct sockaddr_in	cliaddr;
		struct timeval ltv;

		clilen = sizeof(cliaddr);
		connfd = accept(info->rpc_listenfd, (struct sockaddr *)&cliaddr, &clilen);
		if (connfd < 0) {
			BSD_ERROR("Err: accept error[%d]\n", connfd);
			goto done;
		}
		ltv.tv_sec = 5;
		ltv.tv_usec = 0;
		if (setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&ltv,
			sizeof(struct timeval)) < 0) {
			close(connfd);
			BSD_ERROR("Err: setsockopt error[%d]\n", connfd);
			goto done;
		}

		BSD_RPCD("New rpc from addr:%s, port:%d connfd:%d\n",
			inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), connfd);

		switch (info->role) {
			case BSD_ROLE_PRIMARY:
				BSD_RPCD("Err: Socket: close pervious rpc_event_fd:%d\n",
					info->rpc_eventfd);
				if (info->rpc_eventfd !=  BSD_DFLT_FD)
					close(info->rpc_eventfd);

				info->rpc_eventfd = connfd;
				break;
			case BSD_ROLE_HELPER:
				BSD_RPCD("Err: Socket: close pervious rpc_ioctlfd:%d\n",
					info->rpc_ioctlfd);
				if (info->rpc_ioctlfd !=  BSD_DFLT_FD)
					close(info->rpc_ioctlfd);

				info->rpc_ioctlfd = connfd;
				break;
			default:
				BSD_ERROR("Err: Error role:%d\n", info->role);
				break;
		}
	}

	if (info->role == BSD_ROLE_HELPER) {
		if (info->rpc_ioctlfd !=  BSD_DFLT_FD && FD_ISSET(info->rpc_ioctlfd, &fdset)) {
			bsd_helper_proc_rpc(info);
		}
	}

	if (info->role == BSD_ROLE_PRIMARY &&
		(info->rpc_eventfd !=  BSD_DFLT_FD) && FD_ISSET(info->rpc_eventfd, &fdset)) {
		bsd_rpc_pkt_t *rpc_pkt = (bsd_rpc_pkt_t *)ret_buf;
		memset(ret_buf, 0, sizeof(ret_buf));
		bytes = read(info->rpc_eventfd, ret_buf, sizeof(ret_buf));

		if (bytes <= 0) {
			BSD_RPCD("Err: Socket: Recv rpc event. rpc_eventfd[%d]"
				" bytes=%d errno=%d\n",
				info->rpc_eventfd, bytes, errno);
			close(info->rpc_eventfd);
			info->rpc_eventfd = BSD_DFLT_FD;
			goto done;
		}
		else {
			BSD_RPCEVT("Recv rpc event: %d [%s]\n", bytes, (char *)rpc_pkt);

			/*
				if(rpc_pkt.id != BSD_RPC_ID_EVENT) {
					return;
				}
			*/
			bsd_proc_event(info, 1, (char *)rpc_pkt, bytes);
			BSD_RPCEVT("Done rpc event\n");
		}
	}

	/* handle brcm event */
	if (info->event_fd !=  BSD_DFLT_FD && FD_ISSET(info->event_fd, &fdset)) {

		memset(pkt, 0, sizeof(pkt));
		if ((bytes = recv(info->event_fd, pkt, sizeof(pkt), 0)) <= 0) {
			goto done;
		}

		BSD_EVENT("Recv Local event: %d\n", bytes);

		if ((info->role == BSD_ROLE_PRIMARY) || (info->role == BSD_ROLE_STANDALONE)) {
			bsd_proc_event(info, 0, (char *)pkt, bytes);
		}

		if (info->role == BSD_ROLE_HELPER) {
			 bsd_helper_proc_event(info, (char *)pkt, bytes);
		}

		BSD_EVENT("Done Local Event\n");
	}

	/* Process CLI commands */
	if (info->cli_listenfd !=  BSD_DFLT_FD && FD_ISSET(info->cli_listenfd, &fdset)) {
		bsd_proc_client_req(info);
	}

done:
	BSD_EXIT();
}

/* Closes the socket */
void
bsd_close_socket(int *sockfd)
{
	if (*sockfd < 0) {
		return;
	}
	close(*sockfd);
	*sockfd = BSD_DFLT_FD;
}

/* Open a TCP socket for getting requests from client */
int
bsd_open_server_cli_fd(bsd_info_t *info)
{
	int sockfd = BSD_DFLT_FD, optval = 1;
	struct sockaddr_in sockaddr;
	int cli_port = EAPD_WKSP_BSD_CLI_PORT;

	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sockaddr.sin_port = htons(cli_port);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		BSD_ERROR("portno[%d]. socket error is : %s\n", cli_port, strerror(errno));
		goto error;
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
		BSD_ERROR("sockfd[%d] portno[%d]. setsockopt error is : %s\n",
				sockfd, cli_port, strerror(errno));
		goto error;
	}

	if (bind(sockfd, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) < 0) {
		BSD_ERROR("sockfd[%d] portno[%d]. bind error is : %s\n", sockfd, cli_port,
			strerror(errno));
		goto error;
	}

	if (listen(sockfd, 10) < 0) {
		BSD_ERROR("sockfd[%d] portno[%d]. listen error is : %s\n", sockfd, cli_port,
			strerror(errno));
		goto error;
	}
	info->cli_listenfd = sockfd;
	return  BSD_OK;

error:
	bsd_close_socket(&sockfd);
	return BSD_DFLT_FD;
}

/*
 * do_command_response() - set up and send a command to the server, read and process the response.
 */
static int
bsd_do_command_response(bsd_cmdargs_t *ctx)
{
	int ret = BCME_OK;
	if (ctx->socket < 0)
		return BCME_ERROR;

	BSD_ENTER();
	/* Send it */
	if (write(ctx->socket, &(ctx->cmd_id), sizeof(ctx->cmd_id)) < 0) {
		fprintf(stderr, "Failed to send command to server: %s\n", strerror(errno));
		return BCME_ERROR;
	}

	/* Help server get the data till EOF */
	shutdown(ctx->socket, SHUT_WR);

	BSD_EXIT();
	return ret;
}

/*
 * connect_to_server() - Establish a TCP connection to the BSD server command port.
 *
 * On success, the context socket is updated and BCME_OK is returned.
 *
 */
static int
bsd_connect_to_server(bsd_cmdargs_t *ctx)
{
	int sock = BSD_DFLT_FD;
	struct sockaddr_in sockaddr;
	int cli_port = EAPD_WKSP_BSD_CLI_PORT;

	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = inet_addr(ctx->server_host);
	sockaddr.sin_port = htons(cli_port);

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		BSD_ERROR("portno[%d]. socket error is : %s\n", cli_port, strerror(errno));
		goto error;
	}
	if (connect(sock, (const struct sockaddr *)&sockaddr,	sizeof(sockaddr)) < 0) {
		fprintf(stderr, "Could not connect to %s port %d.\n",
			ctx->server_host, cli_port);
		goto error;
	}

	ctx->socket = sock;
	BSD_EXIT();
	return BCME_OK;
error:
	bsd_close_socket(&sock);
	return BSD_DFLT_FD;
}

/* Open a TCP socket as client for sending requests to server */
int
bsd_open_cli_fd(uint8 cmd_id)
{
	int ret = BSD_OK;
	bsd_cmdargs_t cmdarg;

	BSD_ENTER();
	/* Initalise our context object */
	memset(&cmdarg, 0, sizeof(cmdarg));
	cmdarg.server_host = BSD_DEFAULT_SERVER_HOST;
	cmdarg.socket = -1;
	cmdarg.cmd_id = cmd_id;

	ret = bsd_connect_to_server(&cmdarg);
	if (ret == BCME_OK) {
		ret = bsd_do_command_response(&cmdarg);
		bsd_close_socket(&(cmdarg.socket));
	}
	BSD_EXIT();
	return ret;
}
