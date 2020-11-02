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
 * $Id: bsd_util.c $
 */

#include "bsd.h"
#ifdef BCM_WBD
#include <bcmendian.h>
#endif /* BCM_WBD */

/* Default threshold value to measure special STA idle traffic for load-balance case */
#define BSD_LB_IDLE_RATE_DEFAULT	512

typedef sta_info_v7_t sta_info_t;

bsd_sta_info_t *bsd_sta_by_addr(bsd_info_t *info, bsd_bssinfo_t *bssinfo,
	struct ether_addr *addr, bool enable);

void bsd_remove_assoclist(bsd_bssinfo_t *bssinfo, struct ether_addr *addr);

static int bsd_check_sta_assoc_any(bsd_info_t *info, struct ether_addr *addr);

bsd_sta_info_t *bsd_check_sta_assoc_other_bss(bsd_info_t *info, bsd_bssinfo_t *except_bss,
	struct ether_addr *addr);

/* Update BSS capability for this BSS */
static void bsd_update_bss_capability(bsd_bssinfo_t *bssinfo);

extern bsd_info_t *bsd_info;

/* IOCTL swapping mode for Big Endian host with Little Endian dongle.  Default to off */
/* The below macros handle endian mis-matches between wl utility and wl driver. */
bool g_swap = FALSE;
#define htod64(i) (g_swap?bcmswap64(i):(uint64)(i))
#define htod32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define htod16(i) (g_swap?bcmswap16(i):(uint16)(i))
#define dtoh64(i) (g_swap?bcmswap64(i):(uint64)(i))
#define dtoh32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define dtoh16(i) (g_swap?bcmswap16(i):(uint16)(i))

void bsd_rpc_dump(char *ptr, int len, int enab)
{
	int i;
	char ch;

	if (!enab)
		return;

	for (i = 0; i < len; i++) {
		ch = ptr[i];
		BSD_PRINT_PLAIN("%02x[%c] ", ch, isprint((int)(ch & 0xff))? ch : ' ');
		if ((i+1)%16 == 0)
			BSD_PRINT_PLAIN("\n");
	}
	return;
}

/* coverity[ -tainted_data_argument : arg-2 ] */
int bsd_rpc_send(bsd_rpc_pkt_t *rpc_pkt, int len, bsd_rpc_pkt_t *resp)
{
	int	sockfd;
	int ret;
	struct sockaddr_in	servaddr;
	struct timeval tv;
	char tcmd[BSD_IOCTL_MAXLEN];

	BSD_ENTER();
	BSD_RPC("bsd_info=%p\n", bsd_info);

	if (len <= 0)
		return BSD_FAIL;

	BSD_RPC("raw Send buff[sock:%d]: id:%d cmd:%d name:%s len:%d\n",
		bsd_info->rpc_ioctlfd, rpc_pkt->id, rpc_pkt->cmd.cmd,
		rpc_pkt->cmd.name, rpc_pkt->cmd.len);
	bsd_rpc_dump((char *)rpc_pkt, 64, BSD_RPC_ENAB);

	if (bsd_info->rpc_ioctlfd == BSD_DFLT_FD) {
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0) {
			BSD_ERROR("Socket fails.\n");
			return BSD_FAIL;
		}

		tv.tv_sec = 5;
		tv.tv_usec = 0;
		if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
			(char *)&tv, sizeof(struct timeval)) < 0) {
			BSD_ERROR("SetSockoption fails.\n");
			close(sockfd);
			return BSD_FAIL;
		}

		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(bsd_info->hport);

		servaddr.sin_addr.s_addr = inet_addr(bsd_info->helper_addr);

		if (connect(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
			BSD_ERROR("Connect to help fails: Peer: %s[%d]\n",
				bsd_info->helper_addr, bsd_info->hport);
			close(sockfd);
			return BSD_FAIL;
		}
		bsd_info->rpc_ioctlfd = sockfd;
		BSD_RPCD("ioctl new sockfd:%d is created\n", sockfd);
	}

	ret = write(bsd_info->rpc_ioctlfd, (void *)rpc_pkt, len);
	BSD_RPC("Sending:%d sent:%d\n", len, ret);
	if (ret != len) {
		/* tcp conn broken */
		BSD_RPCD("Err: Socket: tcp sock[%d] broken. sending:%d, sent:%d. Close to reopen\n",
			bsd_info->rpc_ioctlfd, len, ret);

		close(bsd_info->rpc_ioctlfd);
		bsd_info->rpc_ioctlfd = BSD_DFLT_FD;
		return BSD_FAIL;
	}

	memset(tcmd, 0, BSD_IOCTL_MAXLEN);
	ret = read(bsd_info->rpc_ioctlfd, tcmd, sizeof(tcmd));

	if (ret <= 0) {
		/* tcp conn broken */
		BSD_RPCD("Err: Socket: sock[%d] broken. Reading ret:%d."
			"Close to reopen, errno:%d\n",
			bsd_info->rpc_ioctlfd, ret, errno);

		close(bsd_info->rpc_ioctlfd);
		bsd_info->rpc_ioctlfd = BSD_DFLT_FD;
		return BSD_FAIL;
	}

	if ((ret > 0) && (ret < BSD_IOCTL_MAXLEN)) {
		int delta = BSD_IOCTL_MAXLEN - ret;
		int pos = ret;
		BSD_ERROR("ERR++: ioctl len=%d, remaining:%d, pos:%d\n", ret, delta, pos);

		while (delta > 0) {
			ret = read(bsd_info->rpc_ioctlfd, (void *)(tcmd + pos), delta);
			if ((ret > 0) && (ret <= delta)) {
				delta = delta - ret;
				pos += ret;
				BSD_RPCD("Assemble:ioctl len=%d, remaining:%d, pos:%d\n",
					ret, delta, pos);
			}
			else {
				BSD_RPCD("Err: Socket: sock[%d] broken. Reading frag ret:%d."
					"Close to reopen, errno:%d\n",
					bsd_info->rpc_ioctlfd, ret, errno);

				close(bsd_info->rpc_ioctlfd);
				bsd_info->rpc_ioctlfd = BSD_DFLT_FD;
				return BSD_FAIL;
			}
		}
	}
	memcpy((char *)resp, tcmd, len);

	BSD_RPC("Raw recv buff[sock:%d] ret=%d: cmd:%d name:%s len:%d\n",
		bsd_info->rpc_ioctlfd, ret, resp->cmd.cmd, resp->cmd.name, resp->cmd.len);

	if ((resp->cmd.cmd != rpc_pkt->cmd.cmd) ||
		(resp->id != rpc_pkt->id) ||
		strcmp(resp->cmd.name, rpc_pkt->cmd.name)) {

		BSD_RPCD("+++++++++ERR: rpc_pkt:id[%d] cmd[%d] name[%s] "
			"resp:id[%d] cmd[%d] name[%s] ",
			rpc_pkt->id, rpc_pkt->cmd.cmd, rpc_pkt->cmd.name,
			resp->id, resp->cmd.cmd, resp->cmd.name);

		BSD_RPCD("Err: Socket: tcp sock[%d] broken. Close to reopen\n",
			bsd_info->rpc_ioctlfd);

		close(bsd_info->rpc_ioctlfd);
		bsd_info->rpc_ioctlfd = BSD_DFLT_FD;

		return BSD_FAIL;
	}

	BSD_EXIT();
	return BSD_OK;
}

char *bsd_nvram_get(bool rpc, const char *name, int *status)
{
	bsd_rpc_pkt_t *pkt = (bsd_rpc_pkt_t *)cmd_buf;
	bsd_rpc_pkt_t *resp = (bsd_rpc_pkt_t *)ret_buf;
	char *ptr;
	char *str = NULL;
	int ret = BSD_OK;
#define BSD_COMM_RETRY_LIMIT	10
	int cnt;

	BSD_ENTER();
	BSD_RPC("rpc[%d] nvram[%s]\n", rpc, name);

	if (!rpc) {
		str = nvram_get(name);
		if (str == NULL) {
			BSD_WARNING("Fail to read nvram %s\n", name);
			ret = BSD_FAIL;
		}
	}
	else {
		/* call rpc nvram */
		cnt = 0;
		BSD_RPC("\n\n\n nvram[%s]\n", name);

		while (cnt++ < BSD_COMM_RETRY_LIMIT) {
			memset(cmd_buf, 0, sizeof(cmd_buf));
			memset(ret_buf, 0, sizeof(ret_buf));

			pkt->id = BSD_RPC_ID_NVRAM;
			pkt->cmd.len = strlen(name) + 4;
			strcpy(pkt->cmd.name, "bsd");
			ptr = (char *)(pkt + 1);
			BSDSTRNCPY(ptr, name, 128);

			ret = bsd_rpc_send(pkt, pkt->cmd.len + sizeof(bsd_rpc_pkt_t), resp);
			bsd_rpc_dump((char *)resp, 64, BSD_RPC_ENAB);
			if (ret == BSD_OK)
				break;

			BSD_RPC("++retry cnt=%d\n", cnt);
		}
		if (ret == BSD_OK) {
			str = (char *)(resp + 1);
			BSD_RPCD("nvram:%s=%s\n", name, str);
		}
	}
	if (status)
		*status = ret;
	BSD_EXIT();
	return str;
}

static INLINE char *bsd_nvram_safe_get(bool rpc, const char *name, int *status)
{
	char *p = bsd_nvram_get(rpc, name, status);
	if (status && *status != BSD_OK)
		return "";
	else
		return p ? p : "";
}

static INLINE int bsd_nvram_match(bool rpc, const char *name, const char *match)
{
	int status;
	const char *value = bsd_nvram_get(rpc, name, &status);
	if (status == BSD_OK)
		return (value && !strcmp(value, match));
	else
		return 0;
}

/* Wrapper API for existing ioctl calls. */
int
bsd_wl_ioctl(bsd_bssinfo_t *bssinfo, int cmd, void *buf, int len)
{
	int ret;

	BSD_ENTER();

	BSD_RPC("\n\n\n ifname:%s idx:%d: Remote:%d  cmd:%d len:%d\n",
		bssinfo->ifnames, bssinfo->idx, bssinfo->intf_info->remote, cmd, len);

	ret = bsd_do_wl_ioctl(bssinfo->ifnames, cmd, buf, len, bssinfo->intf_info->remote);

	BSD_EXIT();
	return ret;
}

/* Function for ioctl calls. */
int
bsd_do_wl_ioctl(char *ifname, int cmd, void *buf, int len, uint8 remote)
{
	int ret = -1;
	bsd_rpc_pkt_t *pkt = (bsd_rpc_pkt_t *)cmd_buf;
	char *ptr;
	bsd_rpc_pkt_t *resp = (bsd_rpc_pkt_t *)ret_buf;

	BSD_ENTER();

	if (remote) {
		memset(cmd_buf, 0, sizeof(cmd_buf));
		memset(ret_buf, 0, sizeof(ret_buf));
		pkt->id = BSD_RPC_ID_IOCTL;
		pkt->cmd.cmd = cmd;
		BSDSTRNCPY(pkt->cmd.name, ifname, sizeof(pkt->cmd.name) - 1);
		pkt->cmd.len = len;
		ptr = (char *)(pkt + 1);
		memcpy(ptr, buf, len);
		BSD_RPCD("ioctl: cmd:%d len:%d name:%s\n", cmd, len, ifname);

		ret = bsd_rpc_send(pkt, pkt->cmd.len + sizeof(bsd_rpc_pkt_t), resp);
		bsd_rpc_dump((char *)resp, 64, BSD_RPC_ENAB);

		if (ret == BSD_OK) {
			ret = resp->cmd.ret;
			BSD_RPCD("ret: %d ioctl: cmd:%d len:%d name:%s\n",
				ret, resp->cmd.cmd, resp->cmd.len, ifname);
			memcpy(buf, (char *)(resp + 1), len);
		}
	} else {
		ret = wl_ioctl(ifname, cmd, buf, len);
	}

	BSD_EXIT();
	return ret;
}

/* remove disassoc STA from list */
bsd_bssinfo_t *bsd_bssinfo_by_ifname(bsd_info_t *info, char *ifname, uint8 remote)
{
	bsd_intf_info_t *intf_info;
	bsd_bssinfo_t *bssinfo = NULL;
	bool found = FALSE;
	int idx, bssidx;

	BSD_ENTER();

	for (idx = 0; (!found) && (idx < info->max_ifnum); idx++) {
		intf_info = &(info->intf_info[idx]);
		if (intf_info->remote != remote) {
			BSD_INFO("intf_info:[%d] remote[%d] != %d\n",
				idx, intf_info->remote, remote);
			continue;
		}
		for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
			bssinfo = &(intf_info->bsd_bssinfo[bssidx]);
			if (!(bssinfo->valid) || !BSD_BSS_BSD_OR_WBD_ENABLED(bssinfo))
				continue;

			BSD_ALL("idx=%d bssidx=%d ifname=[%s][%s]\n",
				idx, bssidx, ifname, bssinfo->ifnames);
			if (!strcmp(ifname, bssinfo->ifnames)) {
				found = TRUE;
				BSD_ALL("idx=%d bssidx=%d\n", idx, bssidx);
				break;
			}
		}
	}

	BSD_EXIT();
	if (found)
		return bssinfo;
	else
		return NULL;
}

/*
 * config info->staprio list
 * sta_config format: xx:xx:xx:xx:xx:xx,prio[,steerflag]".
 */

void bsd_retrieve_staprio_config(bsd_info_t *info)
{
	struct ether_addr ea;
	char var[80], *next, *tmp;
	char *addr, *p, *s;
	bsd_staprio_config_t *ptr;
	char *endptr = NULL;

	BSD_ENTER();
	foreach(var, nvram_safe_get("sta_config"), next) {
		if (strlen(var) < 21) {
			BSD_ERROR("bsd_stprio format error: %s\n", var);
			break;
		}
		tmp = var;

		BSD_INFO("var:%s\n", tmp);
		addr = strsep(&tmp, ",");
		p = strsep(&tmp, ",");
		s = tmp;

		BSD_INFO("addr:%s p:%s s:%s\n", addr, p, s);
		if (ether_atoe(addr, (unsigned char *)(&(ea.octet)))) {
			ptr = malloc(sizeof(bsd_staprio_config_t));
			if (!ptr) {
				BSD_ERROR("Malloc Err:%s\n", __FUNCTION__);
				break;
			}
			memset(ptr, 0, sizeof(bsd_staprio_config_t));
			memcpy(&ptr->addr, &ea, sizeof(struct ether_addr));
			ptr->prio = p ? (uint8)strtol(p, &endptr, 0) : 0;
			ptr->steerflag = s ? (uint8)strtol(s, &endptr, 0) : 1;
			ptr->next = info->staprio;
			info->staprio = ptr;
			BSD_INFO("Mac:"MACF" prio:%d steerflag:%d \n",
				ETHER_TO_MACF(ptr->addr), ptr->prio, ptr->steerflag);
		}
	}
	BSD_EXIT();
	return;
}

/* Set sta priority */
void bsd_set_staprio(bsd_info_t *info)
{
	bsd_staprio_config_t *ptr = NULL;
	char bsd_ifnames[80] = "";
	char var_intf[BSD_IFNAME_SIZE];
	char *next_intf;
	bool rpc = FALSE;

	BSDSTRNCPY(bsd_ifnames, nvram_safe_get("bsd_ifnames"), sizeof(bsd_ifnames) - 1);

	foreach(var_intf, bsd_ifnames, next_intf) {
		if ((info->role == BSD_ROLE_PRIMARY) && !strncmp(var_intf, "rpc:", 4)) {
			rpc = TRUE;
		} else {
			rpc = FALSE;
		}

		ptr = info->staprio;
		while (ptr) {
			wl_staprio_cfg_t *staprio_arg = NULL;
			uint32 buflen = 0;

			/* Set the priority of the sta */
			memset(ioctl_buf, 0, sizeof(ioctl_buf));
			strncpy(ioctl_buf, "staprio", strlen("staprio"));

			buflen = strlen(ioctl_buf) + 1;
			staprio_arg = (wl_staprio_cfg_t *)(ioctl_buf + buflen);

			memcpy(&staprio_arg->ea, &ptr->addr, sizeof(staprio_arg->ea));
			staprio_arg->prio = ptr->prio;

			bsd_do_wl_ioctl(var_intf, WLC_SET_VAR, ioctl_buf, sizeof(ioctl_buf), rpc);

			ptr = ptr->next;
		}
	}
}

/* config info->video_at_ratio */
void bsd_retrieve_video_at_ratio_config(bsd_info_t *info)
{
	char var[80], *next;
	char *endptr = NULL;
	int cnt;

	BSD_ENTER();
	for (cnt = 0; cnt < BSD_MAX_AT_SCB; cnt++)
		info->video_at_ratio[cnt] = 100 - BSD_VIDEO_AT_RATIO_BASE * (cnt + 1);

	cnt = 0;

	foreach(var, nvram_safe_get("bsd_video_at_ratio"), next) {
		info->video_at_ratio[cnt] = strtol(var, &endptr, 0);
		if (++cnt > (BSD_MAX_AT_SCB - 1))
			break;
	}

	for (cnt = 0; cnt < BSD_MAX_AT_SCB; cnt++) {
		BSD_INFO("video_at_ratio[%d]:%d\n", cnt, info->video_at_ratio[cnt]);
	}

	BSD_EXIT();
	return;
}

/* config info->staprio list */
int bsd_retrieve_static_maclist(bsd_bssinfo_t *bssinfo)
{

	int ret = BSD_OK, size;
	struct maclist *maclist = (struct maclist *) maclist_buf;

	BSD_ENTER();

	BSD_INFO("bssinfo[%p] prefix[%s], idx[%d]\n", bssinfo, bssinfo->prefix, bssinfo->idx);

	BSD_RPC("RPC name:%s cmd: %d(WLC_GET_MACMODE)\n", bssinfo->ifnames, WLC_GET_MACMODE);
	ret = bsd_wl_ioctl(bssinfo, WLC_GET_MACMODE,
		&(bssinfo->static_macmode), sizeof(bssinfo->static_macmode));

	if (ret < 0) {
		bssinfo->static_macmode = WLC_MACMODE_DISABLED;
		BSD_ERROR("Err: get macmode fails\n");
		ret = BSD_FAIL;
		goto done;
	}

	BSD_INFO("macmode=%d\n", bssinfo->static_macmode);

	BSD_RPC("RPC name:%s cmd: %d(WLC_GET_MACLIST)\n", bssinfo->ifnames, WLC_GET_MACLIST);
	if (bsd_wl_ioctl(bssinfo, WLC_GET_MACLIST, (void *)maclist,
		sizeof(maclist_buf)-BSD_RPC_HEADER_LEN) < 0) {
		BSD_ERROR("Err: get %s maclist fails\n", bssinfo->ifnames);
		ret = BSD_FAIL;
		goto done;
	}

	if (maclist->count > 0 && maclist->count < 128) {
		size = sizeof(uint) + sizeof(struct ether_addr) * (maclist->count + 1);

		BSD_INFO("count[%d] size[%d]\n", maclist->count, size);

		bssinfo->static_maclist = (struct maclist *)malloc(size);
		if (!(bssinfo->static_maclist)) {
			BSD_ERROR("%s malloc [%d] failure... \n", __FUNCTION__, size);
			ret = BSD_FAIL;
			goto done;
		}
		memcpy(bssinfo->static_maclist, maclist, size);
		maclist = bssinfo->static_maclist;
		if (BSD_DUMP_ENAB) {
			for (size = 0; size < maclist->count; size++) {
				BSD_PRINT("[%d]mac:"MACF"\n",
					size, ETHER_TO_MACF(maclist->ea[size]));
			}
		}
	} else if (maclist->count != 0) {
		BSD_ERROR("Err: %s maclist cnt [%d] too large\n",
			bssinfo->ifnames, maclist->count);
		ret = BSD_FAIL;
		goto done;
	}

done:
	BSD_EXIT();
	return ret;
}

/* Retrieve nvram setting */
void bsd_retrieve_config(bsd_info_t *info)
{
	char *str, *endptr = NULL;

	BSD_ENTER();

	if ((str = nvram_get("bsd_mode"))) {
		info->mode = (uint8)strtol(str, &endptr, 0);
		if (info->mode >= BSD_MODE_MAX)
			info->mode = BSD_MODE_DISABLE;
	}

	info->prefer_5g = BSD_BAND_5G;
	if ((str = nvram_get("bsd_prefer_5g"))) {
		info->prefer_5g = (uint8)strtol(str, &endptr, 0);
	}

	if ((str = nvram_get("bsd_status_poll"))) {
		info->status_poll = (uint8)strtol(str, &endptr, 0);
		if (info->status_poll == 0)
			info->status_poll = BSD_STATUS_POLL_INTV;
	}

	info->counter_poll = BSD_COUNTER_POLL_INTV;
	if ((str = nvram_get("bsd_counter_poll"))) {
		info->counter_poll = (uint8)strtol(str, &endptr, 0);
		if (info->counter_poll == 0)
			info->counter_poll = BSD_COUNTER_POLL_INTV;
	}

	info->idle_rate = 10;
	if ((str = nvram_get("bsd_idle_rate"))) {
		info->idle_rate = (uint8)strtol(str, &endptr, 0);
		if (info->idle_rate == 0)
			info->idle_rate = 10;
	}

	info->lb_idle_rate = BSD_LB_IDLE_RATE_DEFAULT;
	if ((str = nvram_get("bsd_lb_idle_rate")))
		info->lb_idle_rate = strtoul(str, &endptr, 0);

	info->slowest_at_ratio = BSD_SLOWEST_AT_RATIO;
	if ((str = nvram_get("bsd_slowest_at_ratio"))) {
		info->slowest_at_ratio = (uint8)strtol(str, &endptr, 0);
		if (info->slowest_at_ratio == 0)
			info->slowest_at_ratio = BSD_SLOWEST_AT_RATIO;
	}

	info->phyrate_delta = BSD_PHYRATE_DELTA;
	if ((str = nvram_get("bsd_phyrate_delta"))) {
		info->phyrate_delta = (uint8)strtol(str, &endptr, 0);
		if (info->phyrate_delta == 0)
			info->phyrate_delta = BSD_PHYRATE_DELTA;
	}
	bsd_retrieve_video_at_ratio_config(info);

	if ((str = nvram_get("bsd_poll_interval")))
		info->poll_interval = strtol(str, &endptr, 0);

	info->probe_timeout = BSD_PROBE_TIMEOUT;
	if ((str = nvram_get("bsd_probe_timeout")))
		info->probe_timeout = strtol(str, &endptr, 0);

	info->probe_gap = BSD_PROBE_GAP;
	if ((str = nvram_get("bsd_probe_gap")))
		info->probe_gap = strtol(str, &endptr, 0);

	info->maclist_timeout = BSD_MACLIST_TIMEOUT;
	if ((str = nvram_get("bsd_aclist_timeout")))
		info->maclist_timeout = strtol(str, &endptr, 0);

	info->steer_timeout = BSD_STEER_TIMEOUT;
	if ((str = nvram_get("bsd_steer_timeout")))
		info->steer_timeout = strtol(str, &endptr, 0);

	info->sta_timeout = BSD_STA_TIMEOUT;
	if ((str = nvram_get("bsd_sta_timeout")))
		info->sta_timeout = strtol(str, &endptr, 0);

	info->steer_no_deauth = 0;
	if ((str = nvram_get("bsd_steer_no_deauth"))) {
		info->steer_no_deauth = (uint8)strtol(str, &endptr, 0);
	}

	info->block_sta_timeout = BSD_BLOCK_STA_TIMEOUT;
	if ((str = nvram_get("bsd_block_sta_timeout"))) {
		info->block_sta_timeout = (int)strtol(str, &endptr, 0);
	}

	bsd_retrieve_staprio_config(info);

	if (BSD_DUMP_ENAB)
		bsd_dump_info(info);
	BSD_EXIT();
}

/* dump bsd config summary by "bsd -i" */
void bsd_dump_config_info(bsd_info_t *info)
{
	bsd_intf_info_t *intf_info;
	bsd_bssinfo_t *bssinfo;
	int idx, bssidx;
	bsd_steering_policy_t *steering_cfg;
	bsd_if_qualify_policy_t *qualify_cfg;
	bsd_if_bssinfo_list_t *list;
	FILE *out;

	if ((out = fopen(BSD_OUTPUT_FILE_INFO_TMP, "w")) == NULL) {
		printf("Err: Open info file.\n");
		return;
	}

	fprintf(out, "=== Basic info ===\n");
	fprintf(out, "max_ifnum: %d\n", info->max_ifnum);
	fprintf(out, "mode: %d\n", info->mode);
	fprintf(out, "role: %d\n", info->role);
	fprintf(out, "helper: %s[%d]\n", info->helper_addr, info->hport);
	fprintf(out, "primary: %s[%d]\n", info->primary_addr, info->pport);
	fprintf(out, "status_poll: %d\n", info->status_poll);
	fprintf(out, "counter_poll: %d\n", info->counter_poll);
	fprintf(out, "idle_rate: %d\n", info->idle_rate);
	fprintf(out, "prefer_5g: %d\n", info->prefer_5g);
	fprintf(out, "scheme: %d[%d]\n", info->scheme, bsd_get_max_scheme(info));
	fprintf(out, "steer_timeout: %d\n", info->steer_timeout);
	fprintf(out, "sta_timeout: %d\n", info->sta_timeout);
	fprintf(out, "maclist_timeout: %d\n", info->maclist_timeout);
	fprintf(out, "probe_timeout: %d\n", info->probe_timeout);
	fprintf(out, "probe_gap: %d\n", info->probe_gap);
	fprintf(out, "poll_interval: %d\n", info->poll_interval);
	fprintf(out, "slowest_at_ratio: %d\n", info->slowest_at_ratio);
	fprintf(out, "phyrate_delta: %d\n", info->phyrate_delta);
	fprintf(out, "block_sta_timeout: %d\n", info->block_sta_timeout);

	fprintf(out, "\n=== intf_info ===\n");
	for (idx = 0; idx < info->max_ifnum; idx++) {
		fprintf(out, "\nidx: %d\n", idx);
		intf_info = &(info->intf_info[idx]);
		steering_cfg = &intf_info->steering_cfg;
		qualify_cfg = &intf_info->qualify_cfg;

		fprintf(out, "idx=%d band=%d remote=%d enabled=%d steering_flags=0x%x\n",
			intf_info->idx, intf_info->band,
			intf_info->remote, intf_info->enabled,
			intf_info->steering_flags);

		fprintf(out, "Steer Policy:\n");
		fprintf(out, "max=%d period=%d cnt=%d "
			"rssi=%d phyrate=%d flags=0x%x state=%d\n",
			steering_cfg->chan_busy_max,
			steering_cfg->period, steering_cfg->cnt,
			steering_cfg->rssi, steering_cfg->phyrate,
			steering_cfg->flags,
			intf_info->state);

		fprintf(out, "Interface Qualify Policy:\n");
		fprintf(out, "min_bw=%d rssi=%d flags=0x%x\n",
			qualify_cfg->min_bw,
			qualify_cfg->rssi,
			qualify_cfg->flags);

		for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
			bssinfo = &(intf_info->bsd_bssinfo[bssidx]);
			if (bssinfo->valid && BSD_BSS_BSD_OR_WBD_ENABLED(bssinfo)) {
				fprintf(out, "ifidx=%d bssidx=%d ifnames=%s valid=%d:\n",
					idx, bssidx, bssinfo->ifnames, bssinfo->valid);

				fprintf(out, "prefix=%s "
					"ssid=%s idx=0x%x bssid="MACF" rclass=0x%x chanspec=0x%x "
					"prio=0x%x video_idle=%d\n",
					bssinfo->prefix,
					bssinfo->ssid, bssinfo->idx, ETHER_TO_MACF(bssinfo->bssid),
					bssinfo->rclass, bssinfo->chanspec,
					bssinfo->prio, bssinfo->video_idle);

				if (bssinfo->steer_bssinfo) {
					fprintf(out, "steer_prefix=%s [%d][%d]\n",
						bssinfo->steer_prefix,
						((bssinfo->steer_bssinfo)->intf_info)->idx,
						(bssinfo->steer_bssinfo)->idx);
				}

				fprintf(out, "policy=%d[%d]\n",
					bssinfo->policy, bsd_get_max_policy(info));
				fprintf(out, "algo=%d[%d]\n", bssinfo->algo,
					bsd_get_max_algo(info));

				fprintf(out, "Sta Select Policy: defined=%s:\n",
					(bssinfo->sta_select_policy_defined == TRUE)?
					"YES":"NO");
				fprintf(out, "idle_rate=%d rssi=%d phyrate=%d "
					"wprio=%d wrssi=%d wphy_rate=%d "
					"wtx_failures=%d wtx_rate=%d "
					"wrx_rate=%d flags=0x%x\n",
					bssinfo->sta_select_cfg.idle_rate,
					bssinfo->sta_select_cfg.rssi,
					bssinfo->sta_select_cfg.phyrate,
					bssinfo->sta_select_cfg.wprio,
					bssinfo->sta_select_cfg.wrssi,
					bssinfo->sta_select_cfg.wphy_rate,
					bssinfo->sta_select_cfg.wtx_failures,
					bssinfo->sta_select_cfg.wtx_rate,
					bssinfo->sta_select_cfg.wrx_rate,
					bssinfo->sta_select_cfg.flags);

				if (bssinfo->intf_info) {
					fprintf(out, "bssinfo to list:\n");
					fprintf(out, "ifname=%s, intf_info->idx=%d\n",
						bssinfo->ifnames, bssinfo->intf_info->idx);
					list = bssinfo->to_if_bss_list;
					while (list) {
						if (list->bssinfo && list->bssinfo->intf_info) {
							fprintf(out, "ifidx=%d bssidx=%d "
								"to_ifidx=%d ifnames=%s "
								"prefix=%s\n",
								list->bssinfo->intf_info->idx,
								list->bssinfo->idx, list->to_ifidx,
								list->bssinfo->ifnames,
								list->bssinfo->prefix);
						}
						list = list->next;
					}
				}
			}
		}
	}

	fclose(out);
	if (rename(BSD_OUTPUT_FILE_INFO_TMP, BSD_OUTPUT_FILE_INFO) != 0) {
		perror("Err for info data");
		unlink(BSD_OUTPUT_FILE_INFO_TMP);
	}
}

/* Dump bsd DB */
void bsd_dump_info(bsd_info_t *info)
{

	bsd_intf_info_t *intf_info;
	bsd_bssinfo_t *bssinfo;
	int idx, bssidx, cnt;
	bsd_sta_info_t *assoclist;
	bsd_maclist_t *maclist, *prbsta;
	struct maclist *static_maclist;

	bsd_staprio_config_t *staprio;
	time_t now = time(NULL);

	BSD_ENTER();

	BSD_PRINT("-------------------------\n");
	BSD_PRINT("max_ifnum:%d\n", info->max_ifnum);
	BSD_PRINT("mode:%d role:%d now:%lu \n", info->mode, info->role, (unsigned long)now);
	BSD_PRINT("helper:%s[%d] primary:%s[%d]\n",
		info->helper_addr, info->hport,
		info->primary_addr, info->pport);
	BSD_PRINT("status_poll: %d\n", info->status_poll);
	BSD_PRINT("counter_poll: %d idle_rate:%d\n", info->counter_poll, info->idle_rate);
	BSD_PRINT("prefer_5g: %d\n", info->prefer_5g);
	BSD_PRINT("scheme:%d[%d]\n", info->scheme, bsd_get_max_scheme(info));
	BSD_PRINT("steer_timeout: %d\n", info->steer_timeout);
	BSD_PRINT("sta_timeout: %d\n", info->sta_timeout);
	BSD_PRINT("maclist_timeout: %d\n", info->maclist_timeout);
	BSD_PRINT("probe_timeout: %d\n", info->probe_timeout);
	BSD_PRINT("probe_gap: %d\n", info->probe_gap);
	BSD_PRINT("poll_interval: %d\n", info->poll_interval);
	BSD_PRINT("slowest_at_ratio: %d\n", info->slowest_at_ratio);
	BSD_PRINT("phyrate_delta: %d\n", info->phyrate_delta);

	BSD_PRINT_PLAIN("video_at_ratio:\n");
	for (cnt = 0; cnt < BSD_MAX_AT_SCB; cnt++) {
		BSD_PRINT_PLAIN("[%d]:%d\t", cnt, info->video_at_ratio[cnt]);
	}
	BSD_PRINT("\n");

	BSD_PRINT("ifidx=%d bssidx=%d\n", info->ifidx, info->bssidx);

	BSD_PRINT("staPrio List:\n");
	staprio = info->staprio;
	while (staprio) {
		BSD_PRINT("staPrio:"MACF" Prio:%d Steerflag:%d\n",
			ETHER_TO_MACF(staprio->addr), staprio->prio, staprio->steerflag);
		staprio = staprio->next;
	}

	BSD_PRINT("-------------------------\n");
	BSD_PRINT("Probe STA List:\n");
	for (idx = 0; idx < BSD_PROBE_STA_HASH; idx++) {
		prbsta = info->prbsta[idx];
		while (prbsta) {
			BSD_PRINT("sta[%p]:"MACF" timestamp[%lu] band[%d]\n",
				prbsta, ETHER_TO_MACF(prbsta->addr),
				(unsigned long)(prbsta->timestamp), prbsta->band);
			prbsta = prbsta->next;
		}
	}

	BSD_PRINT("-------------------------\n");
	BSD_PRINT("intf_info:\n");
	for (idx = 0; idx < info->max_ifnum; idx++) {
		intf_info = &(info->intf_info[idx]);

		BSD_PRINT("band:%d idx:%d remote[%d] enabled[%d], steering_flags:0x%x\n",
			intf_info->band, intf_info->idx,
			intf_info->remote, intf_info->enabled,
			intf_info->steering_flags);

		for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
			bssinfo = &(intf_info->bsd_bssinfo[bssidx]);
			if (bssinfo->valid && BSD_BSS_BSD_OR_WBD_ENABLED(bssinfo)) {
				BSD_PRINT("ifidx:%d bssidx:%d ifnames:%s valid:%d prefix:%s "
					"ssid:%s idx:0x%x bssid:"MACF" rclass:0x%x chanspec:0x%x"
					"prio:0x%x video_idle:%d\n",
					idx, bssidx,
					bssinfo->ifnames, bssinfo->valid, bssinfo->prefix,
					bssinfo->ssid, bssinfo->idx, ETHER_TO_MACF(bssinfo->bssid),
					bssinfo->rclass, bssinfo->chanspec,
					bssinfo->prio, bssinfo->video_idle);
				BSD_PRINT("steerflag:0x%x assoc_cnt:%d \n",
					bssinfo->steerflag, bssinfo->assoc_cnt);

				if (bssinfo->steer_bssinfo) {
					BSD_PRINT("steer_prefix:%s [%d][%d]\n",
						bssinfo->steer_prefix,
						((bssinfo->steer_bssinfo)->intf_info)->idx,
						(bssinfo->steer_bssinfo)->idx);
				}

				BSD_PRINT("policy:%d[%d]\n",
					bssinfo->policy, bsd_get_max_policy(info));
				BSD_PRINT("algo:%d[%d]\n", bssinfo->algo, bsd_get_max_algo(info));
				BSD_PRINT("Policy: idle_rate=%d rssi=%d phyrate=%d "
					"wprio=%d wrssi=%d wphy_rate=%d "
					"wtx_failures=%d wtx_rate=%d "
					"wrx_rate=%d flags=0x%x defined=%s\n",
					bssinfo->sta_select_cfg.idle_rate,
					bssinfo->sta_select_cfg.rssi,
					bssinfo->sta_select_cfg.phyrate,
					bssinfo->sta_select_cfg.wprio,
					bssinfo->sta_select_cfg.wrssi,
					bssinfo->sta_select_cfg.wphy_rate,
					bssinfo->sta_select_cfg.wtx_failures,
					bssinfo->sta_select_cfg.wtx_rate,
					bssinfo->sta_select_cfg.wrx_rate,
					bssinfo->sta_select_cfg.flags,
					(bssinfo->sta_select_policy_defined == TRUE)?"YES":"NO");

				/* assoclist */
				assoclist = bssinfo->assoclist;
				BSD_PRINT("assoclist[%p]:\n", assoclist);
				while (assoclist) {
					BSD_PRINT("STA[%p]:"MACF" paddr:"MACF"\n",
						assoclist,
						ETHER_TO_MACF(assoclist->addr),
						ETHER_TO_MACF(assoclist->paddr));
					BSD_PRINT("prio: 0x%x\n", assoclist->prio);
					BSD_PRINT("steerflag: 0x%x\n", assoclist->steerflag);
					BSD_PRINT("rssi: %d\n", assoclist->rssi);
					BSD_PRINT("phy_rate: %d\n", assoclist->phy_rate);

					BSD_PRINT("tx_rate:%d, rx_rate:%d\n",
						assoclist->tx_rate, assoclist->rx_rate);

					BSD_PRINT("tx_bps:%d, rx_bps:%d\n",
						assoclist->tx_bps, assoclist->rx_bps);

					BSD_PRINT("timestamp: %lu(%ld)\n",
						(unsigned long)(assoclist->timestamp),
						(unsigned long)(assoclist->active));

					BSD_PRINT("at_ratio:%d, phyrate:%d\n",
						assoclist->at_ratio, assoclist->phyrate);
					assoclist = assoclist->next;
				}

				/* maclist */
				BSD_PRINT("macmode: %d\n", bssinfo->macmode);
				maclist = bssinfo->maclist;
				while (maclist) {
					BSD_PRINT("maclist: "MACF"\n",
						ETHER_TO_MACF(maclist->addr));
					maclist = maclist->next;
				}

				if (bssinfo->static_maclist) {
					static_maclist = bssinfo->static_maclist;
					BSD_PRINT("static_mac: macmode[%d] cnt[%d]\n",
						bssinfo->static_macmode, static_maclist->count);
					for (cnt = 0; cnt < static_maclist->count; cnt++) {
						BSD_INFO("[%d] mac:"MACF"\n", cnt,
							ETHER_TO_MACF(static_maclist->ea[cnt]));
					}
				}
			}

			dump_if_bssinfo_list(bssinfo);
		}

		{
			bsd_steering_policy_t * steering_cfg = &intf_info->steering_cfg;
			bsd_if_qualify_policy_t *qualify_cfg = &intf_info->qualify_cfg;
			uint8 idx, num;
			bsd_chanim_stats_t *rec = &(intf_info->chan_util.rec[0]);

			BSD_PRINT("-------------------------\n");
			BSD_PRINT("chamin histo:\n");
			BSD_PRINT("idx[%d] min[%d] max[%d] period[%d] cnt[%d] "
				"rssi[%d] phyrate[%d] flags[0x%x] state[%d]\n",
				intf_info->chan_util.idx, qualify_cfg->min_bw,
				steering_cfg->chan_busy_max,
				steering_cfg->period,
				steering_cfg->cnt,
				steering_cfg->rssi, steering_cfg->phyrate,
				steering_cfg->flags,
				intf_info->state);

			BSD_PRINT("if_qualify_policy: min[%d] rssi[%d] flags[0x%x]\n",
				qualify_cfg->min_bw, qualify_cfg->rssi, qualify_cfg->flags);
			BSD_PRINT_PLAIN("chanspec    tx   inbss   obss   nocat   nopkt   "
				"doze     txop     goodtx  badtx   glitch   badplcp  "
				"knoise  timestamp     idle\n");
			if (intf_info->chan_util.chanim_version == WL_CHANIM_STATS_V2) {
				chanim_stats_v2_t *statsv2;
				for (num = 0; num < BSD_CHANIM_STATS_MAX; num++) {
					if (!(rec[num].valid))
						continue;

					statsv2 = (chanim_stats_v2_t *)&(rec[num].stats);

					BSD_PRINT_PLAIN("[%d]0x%4x\t",
						num, statsv2->chanspec);

					for (idx = 0; idx < CCASTATS_V2_MAX; idx++)
						BSD_PRINT_PLAIN("%d\t", statsv2->ccastats[idx]);
						BSD_PRINT_PLAIN("%d\t%d\t%d\t%d\t%d\n",
							statsv2->glitchcnt, statsv2->badplcp,
							statsv2->bgnoise, statsv2->timestamp,
							statsv2->chan_idle);
				}
			}
			else if (intf_info->chan_util.chanim_version == WL_CHANIM_STATS_VERSION) {
				chanim_stats_t *statsv3;
				for (num = 0; num < BSD_CHANIM_STATS_MAX; num++) {
					if (!(rec[num].valid))
						continue;

					statsv3 = (chanim_stats_t *)&(rec[num].stats);

					BSD_PRINT_PLAIN("[%d]0x%4x\t",
						num, statsv3->chanspec);

					for (idx = 0; idx < CCASTATS_MAX; idx++)
						BSD_PRINT_PLAIN("%d\t", statsv3->ccastats[idx]);
						BSD_PRINT_PLAIN("%d\t%d\t%d\t%d\t%d\n",
							statsv3->glitchcnt, statsv3->badplcp,
							statsv3->bgnoise, statsv3->timestamp,
							statsv3->chan_idle);
				}
			}
		}
		BSD_PRINT("-------------------------\n");
	}
	BSD_EXIT();
}

/* Cleanup bsd DB */
void bsd_bssinfo_cleanup(bsd_info_t *info)
{
	bsd_intf_info_t *intf_info;
	bsd_bssinfo_t *bssinfo;
	int idx, bssidx;
	bsd_sta_info_t *assoclist, *next;
	bsd_maclist_t *maclist, *next_mac;
	bsd_staprio_config_t *staprio, *next_staprio;

	BSD_ENTER();

	for (idx = 0; idx < info->max_ifnum; idx++) {
		intf_info = &(info->intf_info[idx]);

		for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
			bssinfo = &(intf_info->bsd_bssinfo[bssidx]);

			/* Cleanup wlif handle. */
			wl_wlif_deinit(bssinfo->wlif_hdl);

			BSD_INFO("free assoclist/maclist[bssidx:%d]...\n", bssidx);
			/* assoclist */
			assoclist = bssinfo->assoclist;
			while (assoclist) {
				next = assoclist->next;
				BSD_INFO("sta[%p]:"MACF"\n",
					assoclist, ETHER_TO_MACF(assoclist->addr));
				free(assoclist);
				assoclist = next;
			}

			/* maclist */
			maclist = bssinfo->maclist;
			while (maclist) {
				BSD_INFO("maclist"MACF"\n", ETHER_TO_MACF(maclist->addr));
				next_mac = maclist->next;
				free(maclist);
				maclist = next_mac;
			}

			if (bssinfo->static_maclist)
				free(bssinfo->static_maclist);

			clean_if_bssinfo_list(bssinfo->to_if_bss_list);
		}
	}

	for (idx = 0; idx < BSD_PROBE_STA_HASH; idx++) {
		/* cleanup prbsta list */
		maclist = info->prbsta[idx];
		while (maclist) {
			BSD_INFO("prbsta: "MACF"\n", ETHER_TO_MACF(maclist->addr));
			next_mac = maclist->next;
			free(maclist);
			maclist = next_mac;
		}
	}

	/* cleanup staprio list */
	staprio = info->staprio;
	while (staprio) {
		BSD_INFO("staprio: "MACF"\n", ETHER_TO_MACF(staprio->addr));
		next_staprio = staprio->next;
		free(staprio);
		staprio = next_staprio;
	}

	BSD_EXIT();
}

static int wifi_ifnum(void)
{
	char nvram_name[32], ifname[32];
	int index;
	int total = 0;
	int unit = -1;

	/* Find out the wl interface index for the specified interface. */
	for (index = 0; index < DEV_NUMIFS; ++index) {

		snprintf(nvram_name, sizeof(nvram_name), "wl%d_ifname", index);
		BSDSTRNCPY(ifname, nvram_safe_get(nvram_name), sizeof(ifname));

		BSD_INFO("wifi_ifnum: nvram_name=%s, ifname=%s\n", nvram_name, ifname);

		if (!wl_probe(ifname)) {
			if (!wl_ioctl(ifname, WLC_GET_INSTANCE, &unit, sizeof(unit))) {
				total ++;
				BSD_INFO("wifi_ifnum: unit=%d, total=%d\n", unit, total);
			}
		}
	}

	return total;
}

/* initialize bsd intf_info DB */
int bsd_intf_info_init(bsd_info_t *info)
{
	char bsd_ifnames[64];

	BSD_ENTER();

	if ((info->max_ifnum = wifi_ifnum()) == 0)
		info->max_ifnum = BSD_ATLAS_MAX_INTF;

	BSDSTRNCPY(bsd_ifnames, nvram_safe_get(BSD_IFNAMES_NVRAM), sizeof(bsd_ifnames));
	BSD_INFO("bsd_ifnames=%s, max_ifnum=%d\n", bsd_ifnames, info->max_ifnum);

	info->intf_info = (bsd_intf_info_t *) calloc(sizeof(bsd_intf_info_t), info->max_ifnum);
	if (info->intf_info == NULL) {
		BSD_ERROR("Err: intf_info %d calloc error\n", info->max_ifnum);
		return BSD_FAIL;
	}

	BSD_EXIT();
	return BSD_OK;
}

/*
 * Two CPU platform remote vs. local indexing mapping
 */
static int bsd_remote_to_idx_mapping(bsd_info_t *info,
	bsd_intf_info_t *intf_info, bsd_bssinfo_t *bssinfo, char *policy_str)
{
	int to_idx;

	/* the dual band 2 radio structure defines the following simple mapping */
	to_idx = (intf_info->idx + 1) % 2;

	return (to_idx);
}

/* convert real interface index to intf_info array index */
static int bsd_ifidx_to_array_mapping(bsd_info_t *info, int ifidx)
{
	int i;
	bsd_intf_info_t *intf_info;

	for (i = 0; i < info->max_ifnum; i++) {
		intf_info = &(info->intf_info[i]);
		if ((intf_info == NULL) || (intf_info->enabled != TRUE))
			continue;
		if (intf_info->unit == ifidx)
			break;
	}
	return i;
}

/* initialize bounce detect config, and multi-RF(more than 2 RFs) bsd info DB,
 * parse wlX_if_select_policy and sets the linked list accordingly
 */
static int bsd_info_init_ext(bsd_info_t *info)
{
	bsd_intf_info_t *intf_info;
	bsd_bssinfo_t *bssinfo = NULL;
	bsd_bssinfo_t *to_bssinfo;
	int idx, bssidx;
	char *str, *next;
	int num = 0;
	bsd_bounce_detect_t *bnc_detect = &(info->bounce_cfg);
	bsd_bounce_detect_t *non11v_detect = &(info->non11v_cfg);
	char nvname[80];
	char policy_str[128];
	char var[80];
	char nvifname[IFNAMSIZ];

	BSD_ENTER();

	strcpy(nvname, BSD_BOUNCE_DETECT_NVRAM);
	str = nvram_safe_get(nvname);
	if (str) {
		num = sscanf(str, "%d %d %d",
			&bnc_detect->window, &bnc_detect->cnt, &bnc_detect->dwell_time);
	}

	if (num != 3) {
		bnc_detect->window = BSD_BOUNCE_DETECT_WIN;
		bnc_detect->cnt = BSD_BOUNCE_DETECT_CNT;
		bnc_detect->dwell_time = BSD_BOUNCE_DETECT_DWELL;
	}

	str = NULL;
	str = nvram_safe_get(BSD_NON11V_DETECT_NVRAM);
	if (str) {
		num = sscanf(str, "%d %d %d",
			&non11v_detect->window, &non11v_detect->cnt, &non11v_detect->dwell_time);
	}

	if (num != 3) {
		non11v_detect->window = 0;
		non11v_detect->cnt = 0;
		non11v_detect->dwell_time = 0;
	}

	for (idx = 0; idx < info->max_ifnum; idx++) {
		intf_info = &(info->intf_info[idx]);

		if (intf_info->enabled != TRUE) {
			BSD_INFO("Skip: idx %d is not enabled\n", idx);
			continue;
		}

		for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
			bssinfo = &(intf_info->bsd_bssinfo[bssidx]);

			BSD_INFO("[%d][%d].ifnames=%s, prefix=%s\n",
				idx, bssidx, bssinfo->ifnames, bssinfo->prefix);

			BSDSTRNCPY(nvname, bssinfo->prefix, sizeof(nvname));
			strcat(nvname, BSD_IF_SELECT_POLICY_NVRAM);
			BSDSTRNCPY(policy_str,
				bsd_nvram_safe_get(intf_info->remote, nvname, NULL),
				sizeof(policy_str));
			BSD_INFO("[%d][%d] %s=%s\n", idx, bssidx, nvname, policy_str);

			bssinfo->to_if_bss_list = NULL;

			if (strlen(policy_str)) {
				int to_idx, to_bssidx;
				struct bsd_if_bssinfo_list *if_bss_entry;
				struct bsd_if_bssinfo_list *tail = NULL;

				/* mark this bss steerable */
				bssinfo->steerflag = 0;
				BSD_INFO("parsing %s\n", policy_str);

				foreach(var, policy_str, next) {
					/* a quick idx decode based on wl prefix, and ethX */
					int var_offset;
					char *cptr;

					var_offset = 0;
					if (!strncmp(var, "rpc:", 4)) {
						var_offset = 4;
					}

					cptr = &var[var_offset];

					to_bssidx = 0;
					if (!strncmp(cptr, "eth", 3)) {
						if (osifname_to_nvifname(cptr, nvifname,
							sizeof(nvifname)) != 0) {
							BSD_INFO("Fail to find name %s\n", cptr);
							continue;
						}
						to_idx = nvifname[2] - '0';
						BSD_INFO("ethX %s to_idx:%d\n", cptr, to_idx);
					} else if (!strncmp(cptr, "wl", 2)) {
						to_idx = cptr[2] - '0';
						if (cptr[3] == '.') {
							to_bssidx = atoi(&cptr[4]);
						}
					} else {
						continue;
					}

					/* integrate two CPU platforms indexing mapping */
					if (intf_info->remote || var_offset != 0) {
						to_idx = bsd_remote_to_idx_mapping(info, intf_info,
							bssinfo, policy_str);
					}

					BSD_INFO("var:%s cptr:%s idx=%d to_idx=%d, to_bssidx=%d\n",
						var, cptr, idx, to_idx, to_bssidx);

					/* map to_idx (real intf index) to intf_info array index */
					to_idx = bsd_ifidx_to_array_mapping(info, to_idx);

					BSD_INFO("After index mapping to_idx=%d, to_bssidx=%d\n",
						to_idx, to_bssidx);

					/* validate */
					if ((to_idx < info->max_ifnum) &&
						(to_bssidx < WL_MAXBSSCFG)) {
						bsd_intf_info_t *to_if;

						to_if = &info->intf_info[to_idx];
						to_bssinfo = &to_if->bsd_bssinfo[to_bssidx];

						/* insert this to bssinfo's to_bssinfo_list tail */
						if_bss_entry = (bsd_if_bssinfo_list_t *)
							calloc(1, sizeof(bsd_if_bssinfo_list_t));

						if (if_bss_entry == NULL) {
							BSD_ERROR("failed to calloc(), abort\n");
							return BSD_FAIL;
						}

						if_bss_entry->bssinfo = to_bssinfo;
						if_bss_entry->to_ifidx = to_bssinfo->intf_info->idx;
						if_bss_entry->next = NULL;

						/* tail end insertion to keep if steering order */
						if (tail == NULL) {
							bssinfo->to_if_bss_list = if_bss_entry;
						} else {
							tail->next = if_bss_entry;
						}
						tail = if_bss_entry;

						/* mark this bss steerable */
						to_bssinfo->steerflag = 0;
					}
				}
			}
		}
	}

	BSD_EXIT();

	return BSD_OK;
}

/* To get the BSSID of Interface */
int bsd_get_bssid(bsd_bssinfo_t *bssinfo, struct ether_addr *out_bssid)
{
	int ret = BSD_FAIL, wlret = 0;
	int ncount = 0;

	/* Validate arg */
	if (!out_bssid) {
		goto end;
	}

	while (ncount < BSD_WBD_GET_VALID_BSSID_RETRY_COUNT) {
		wlret = bsd_wl_ioctl(bssinfo, WLC_GET_BSSID, out_bssid, ETHER_ADDR_LEN);
		if ((wlret < 0) || (ETHER_ISNULLADDR(out_bssid))) {
			usleep(BSD_WBD_GET_VALID_BSSID_TIMEOUT);
			ncount++;
			continue;
		}
		ret = BSD_OK;
		break;
	}

	/* The adapter is associated */
	BSD_INFO("ifname : %s BSSID : "MACF"\n", bssinfo->ifnames, ETHER_TO_MACF((*out_bssid)));

end:
	return ret;
}

/* initialize bsd info DB */
int bsd_info_init(bsd_info_t *info)
{
	char name[BSD_IFNAME_SIZE], var_intf[BSD_IFNAME_SIZE], prefix[BSD_IFNAME_SIZE];
	char *next_intf;

	int idx_intf, total_intf;
	int ret;
	int band;
	wlc_ssid_t ssid = { 0, {0} };
	struct ether_addr ea;

	int idx, counter;
	char tmp[100];
	char *str;
	char *endptr = NULL;
	uint8 tmpu8;

	bsd_intf_info_t *intf_info;
	bsd_bssinfo_t *bssinfo;
	int num;
	bsd_sta_select_policy_t policy;
	int err = BSD_FAIL;
	char bsd_ifnames[80] = "";
	char bsd_ifnames_sorted[80] = "";
	bool rpc = FALSE, valid_bssid = FALSE;
	char *appname = "bsd";
#ifdef BCM_WBD
	int chspec = 0x00;
#endif /* BCM_WBD */

	BSD_ENTER();

	if (info->role == BSD_ROLE_HELPER) {
		BSD_INFO("No need to do init for helper\n");
		BSD_EXIT();
		return BSD_OK;
	}

	BSDSTRNCPY(bsd_ifnames, nvram_safe_get("bsd_ifnames"), sizeof(bsd_ifnames) - 1);
	BSD_INFO("nvram bsd_ifnames=%s, max_ifnum=%d\n",
		bsd_ifnames, info->max_ifnum);

	BSD_INFO("bsd_ifnames=%s, max_ifnum=%d\n", bsd_ifnames, info->max_ifnum);

	if (info->role == BSD_ROLE_PRIMARY) {
		do {
			str = bsd_nvram_safe_get(1, BSD_IFNAMES_NVRAM, &ret);
			if (ret == BSD_OK) {
				break;
			}
			sleep(5);
			BSD_RPCD("Waiting for Helper...\n");
		} while (TRUE);

		BSD_RPCD("Primary bsd_ifnames='%s' Helper bsd_ifnames='%s'\n", bsd_ifnames, str);
	}

	/* Sort bsd_ifnames first */
	{
		const int VAR_LEN = 16;
		int i = 0;
		char *bsd_if = malloc(VAR_LEN * info->max_ifnum);

		if (!bsd_if) {
				BSD_ERROR("malloc failed!\n");
				return BSD_FAIL;
		}

		memset(bsd_if, 0, VAR_LEN * info->max_ifnum);

		foreach(var_intf, bsd_ifnames, next_intf) {
			if (i++ == info->max_ifnum) {
				BSD_ERROR("Wrong bsd_ifnames format!\n");
				break;
			}

			if ((info->role == BSD_ROLE_PRIMARY) && !strncmp(var_intf, "rpc:", 4))
				rpc = TRUE;
			else
				rpc = FALSE;

			if (bsd_do_wl_ioctl(var_intf, WLC_GET_INSTANCE,
				&idx_intf, sizeof(idx_intf), rpc)) {
				BSD_ERROR("Err: failed to get WL instance of %s!\n", var_intf);
				continue;
			}

			snprintf(&bsd_if[idx_intf * VAR_LEN], VAR_LEN, "%s", var_intf);
		}

		for (i = 0; i < info->max_ifnum; i++) {
			if (bsd_if[i * VAR_LEN]) {
				if (strlen(bsd_ifnames_sorted) +
					strlen(&bsd_if[i * VAR_LEN]) + 1 < 80) {
					strcat(bsd_ifnames_sorted, &bsd_if[i * VAR_LEN]);
					strcat(bsd_ifnames_sorted, " ");
				} else {
					BSD_ERROR("Invalid interface names!\n");
					return BSD_FAIL;
				}
			}
		}

		if (bsd_if)
			free(bsd_if);

		BSD_INFO("sorted nvram bsd_ifnames=%s\n", bsd_ifnames_sorted);
#ifdef BCM_WBD
		/* if WBD is enabled */
		if (info->enable_flag & BSD_FLAG_WBD_ENABLED) {
			bsd_wbd_ifnames_append(info, bsd_ifnames_sorted,
				sizeof(bsd_ifnames_sorted));
		}
#endif /* BCM_WBD */
	}

	total_intf = 0;
	foreach(var_intf, bsd_ifnames_sorted, next_intf) {
		BSD_INFO("------- BSD Interface name:index [%s: %d] -------\n", var_intf, idx_intf);

		if ((info->role == BSD_ROLE_PRIMARY) && !strncmp(var_intf, "rpc:", 4)) {
			BSDSTRNCPY(name, &var_intf[4], sizeof(name) - 1);
			rpc = TRUE;
		} else {
			rpc = FALSE;
			BSDSTRNCPY(name, var_intf, sizeof(name) - 1);
		}

		BSD_RPC("RPC(%d) name:%s converted from %s\n", rpc, name, var_intf);

		if (bsd_do_wl_ioctl(var_intf, WLC_GET_INSTANCE, &idx_intf, sizeof(idx_intf), rpc)) {
			BSD_ERROR("Err: failed to get WL instance of %s!\n", var_intf);
			continue;
		}
		BSD_INFO("Get WL instance of %s: %d\n", var_intf, idx_intf);

		intf_info = &(info->intf_info[idx_intf]);
		intf_info->idx = idx_intf;
		intf_info->unit = idx_intf;
		intf_info->remote = (rpc) ? 1 : 0;

		if (strstr(var_intf, ".")) {
			if ((get_ifname_unit(var_intf, NULL, &idx) != 0) ||
				(idx >= WL_MAXBSSCFG)) {
				BSD_ERROR("Failed to decode interface format of %s!\n", var_intf);
				continue;
			}
		}
		else {
			idx = 0;
		}
		BSD_INFO("Get subunit of %s: %d\n", var_intf, idx);

		bssinfo = &(intf_info->bsd_bssinfo[idx]);
		bssinfo->intf_info = intf_info;

		BSDSTRNCPY(bssinfo->ifnames, name, BSD_IFNAME_SIZE);
		BSD_INFO("var_intf=%s\n", var_intf);

		/* Init wlif handle. */
		bssinfo->wlif_hdl = wl_wlif_init(NULL, bssinfo->ifnames,
			bsd_ioctl_callback_func, &bssinfo->intf_info->remote, appname);

		if (idx == 0)
			snprintf(prefix, sizeof(prefix), "wl%d_", idx_intf);
		else {
			snprintf(prefix, sizeof(prefix), "wl%d.%d_", idx_intf, idx);
			snprintf(tmp, sizeof(tmp), "wl%d_ifname", idx_intf);
			snprintf(intf_info->bsd_bssinfo[0].ifnames, BSD_IFNAME_SIZE,
				"%s", nvram_safe_get(tmp));
			intf_info->bsd_bssinfo[0].intf_info = intf_info;
		}
		BSD_INFO("bsd_bssinfo[0].ifnames = %s\n", intf_info->bsd_bssinfo[0].ifnames);

		BSDSTRNCPY(bssinfo->prefix, prefix, sizeof(bssinfo->prefix) - 1);
		snprintf(bssinfo->primary_prefix, sizeof(bssinfo->primary_prefix),
			"wl%d_", idx_intf);
		bssinfo->idx = idx;

		BSD_INFO("nvram %s=%s\n",
			strcat_r(bssinfo->primary_prefix, "radio", tmp),
			bsd_nvram_safe_get(rpc, strcat_r(bssinfo->primary_prefix, "radio", tmp),
				NULL));

		intf_info->enabled = TRUE;
		if (bsd_nvram_match(rpc, strcat_r(bssinfo->primary_prefix, "radio", tmp), "0")) {
			BSD_INFO("Skip intf:%s.  radio is off\n", name);
			memset(intf_info, 0, sizeof(bsd_intf_info_t));
			continue;
		}

		BSD_INFO("nvram %s=%s\n",
			strcat_r(prefix, "bss_enabled", tmp),
			bsd_nvram_safe_get(rpc, strcat_r(prefix, "bss_enabled", tmp), NULL));

		if (bsd_nvram_match(rpc, strcat_r(prefix, "bss_enabled", tmp), "1")) {
			bssinfo->valid = TRUE;
			BSD_INFO("Valid intf:%s(%s)\n", name, tmp);
		}

		BSD_INFO("nvram %s=%s\n",
			strcat_r(prefix, "bss_prio", tmp),
			nvram_safe_get(strcat_r(prefix, "bss_prio", tmp)));

		str = bsd_nvram_safe_get(rpc, strcat_r(prefix, "bss_prio", tmp), NULL);
		tmpu8 = (uint8)strtol(str, &endptr, 0);
		if (tmpu8 >= BSD_MAX_PRIO) {
			BSD_INFO("Err prio:%s=0x%x\n",
				strcat_r(prefix, "bss_prio", tmp), tmpu8);
			tmpu8 = BSD_BSS_PRIO_DISABLE;
		}
		bssinfo->prio = tmpu8;

		BSD_RPC("RPC name:%s cmd: %d(WLC_GET_BAND)\n", bssinfo->ifnames, WLC_GET_BAND);
		ret = bsd_wl_ioctl(bssinfo, WLC_GET_BAND, &band, sizeof(band));
		if (ret < 0) {
			BSD_ERROR("Err: get %s band error\n", name);
			continue;
		}

#ifdef BCM_WBD
		/* if WBD is enabled */
		if (info->enable_flag & BSD_FLAG_WBD_ENABLED) {
			BSD_RPC("RPC name:%s cmd: %d(WLC_GET_VAR for chanspec)\n",
				bssinfo->ifnames, WLC_GET_VAR);
			memset(ioctl_buf, 0, sizeof(ioctl_buf));
			strcpy(ioctl_buf, "chanspec");

			ret = bsd_wl_ioctl(bssinfo, WLC_GET_VAR,
				ioctl_buf, sizeof(ioctl_buf) - BSD_RPC_HEADER_LEN);
			chspec =  *((int*)&ioctl_buf[0]);

			bssinfo->chanspec = (chanspec_t)dtoh32(chspec);
		}
#endif /* BCM_WBD */
		intf_info->band = band;

		/* Get the BSSID, till we get the valid BSSID. Getting BSSID in loop because for
		 * DWDS AP BSS comes up little late, So getting in loop for some retry time period
		 */
		valid_bssid = FALSE;
		for (counter = 0; counter < BSD_GET_VALID_BSSID_RETRY_COUNT; counter++) {
			BSD_RPC("RPC name:%s cmd: %d(WLC_GET_SSID)\n", bssinfo->ifnames,
				WLC_GET_SSID);
			if (bsd_wl_ioctl(bssinfo, WLC_GET_SSID, &ssid, sizeof(ssid)) < 0) {
				BSD_ERROR("Err: ifnams[%s] get ssid failure\n", name);
				sleep(BSD_GET_VALID_BSSID_TIMEOUT);
				continue;
			}
			ssid.SSID[ssid.SSID_len] = '\0';
			BSDSTRNCPY(bssinfo->ssid, (char *)(ssid.SSID), sizeof(bssinfo->ssid) - 1);
			BSD_INFO("ifnams[%s] ssid[%s] band[%d]\n", name, ssid.SSID, band);

			BSD_RPC("RPC name:%s cmd: %d(WLC_GET_BSSID)\n", bssinfo->ifnames,
				WLC_GET_BSSID);

			if (bsd_get_bssid(bssinfo, &ea) != BSD_OK) {
				BSD_ERROR("Err: ifnams[%s] get bssid failure\n", name);
				sleep(BSD_GET_VALID_BSSID_TIMEOUT);
				continue;
			}
			if (ETHER_ISNULLADDR(&ea)) {
				BSD_INFO("Err: ifnams[%s] ssid[%s] BSSID is NULL. Try again...\n",
					name, ssid.SSID);
				sleep(BSD_GET_VALID_BSSID_TIMEOUT);
				continue;
			}
			memcpy(&bssinfo->bssid, &ea, ETHER_ADDR_LEN);
			BSD_INFO("bssid:"MACF"\n", ETHER_TO_MACF(bssinfo->bssid));
			valid_bssid = TRUE;
			break;
		}
		if (valid_bssid == FALSE) {
			BSD_INFO("Err: ifnams[%s] Even after %d attempts for %d seconds, failed to"
				"get BSSID\n", name, BSD_GET_VALID_BSSID_RETRY_COUNT,
				(BSD_GET_VALID_BSSID_RETRY_COUNT * BSD_GET_VALID_BSSID_TIMEOUT));
			continue;
		}

		BSD_INFO("ifname=%s idx_intf=%d prefix=%s idx=%d "
			"ssid=%s bssid:"MACF" "
			"band=%d prio=%d steerflag=0x%x, steer_prefix=%s\n",
			var_intf, idx_intf, prefix, idx,
			bssinfo->ssid, ETHER_TO_MACF(bssinfo->bssid),
			intf_info->band,
			bssinfo->prio, bssinfo->steerflag, bssinfo->steer_prefix);

		if (info->enable_flag & BSD_FLAG_ENABLED)
			bssinfo->steerflag = BSD_BSSCFG_NOTSTEER;
		str = bsd_nvram_safe_get(rpc, strcat_r(prefix, "bsd_steer_prefix", tmp), &ret);
		if (ret == BSD_OK) {
			BSDSTRNCPY(bssinfo->steer_prefix, str,
				sizeof(bssinfo->steer_prefix) - 1);
			bssinfo->steerflag = 0;
			if (intf_info->band == BSD_BAND_5G) {
				info->ifidx = idx_intf;
				info->bssidx = idx;
				BSD_INFO("Monitor intf[%s] [%d][%d]\n",
					bssinfo->ifnames, idx_intf, idx);
				err = BSD_OK;
			}
		}

		/* index to STA selection algorithm, impl. */
		str = bsd_nvram_safe_get(rpc, strcat_r(prefix, "bsd_algo", tmp), &ret);
		if (ret) {
			bssinfo->algo = (uint8)strtol(str, &endptr, 0);
			if (bssinfo->algo >= bsd_get_max_algo(info))
				bssinfo->algo = 0;
		}

		str = bsd_nvram_safe_get(rpc, strcat_r(prefix, "bsd_policy", tmp), &ret);
		if (ret == BSD_OK) {
			bssinfo->policy = (uint8)strtol(str, &endptr, 0);
			if (bssinfo->policy >= bsd_get_max_policy(info))
				bssinfo->policy = 0;
		} else {
			if (intf_info->band == BSD_BAND_5G)
				bssinfo->policy = BSD_POLICY_LOW_PHYRATE;
			else
				bssinfo->policy = BSD_POLICY_HIGH_PHYRATE;
		}

		memcpy(&bssinfo->sta_select_cfg, bsd_get_sta_select_cfg(bssinfo),
			sizeof(bsd_sta_select_policy_t));

		bssinfo->sta_select_policy_defined = FALSE;

		str = bsd_nvram_safe_get(rpc,
			strcat_r(prefix, BSD_STA_SELECT_POLICY_NVRAM, tmp), &ret);

		if (ret == BSD_OK) {
			num = sscanf(str, "%d %d %d %d %d %d %d %d %d %x",
				&policy.idle_rate, &policy.rssi, &policy.phyrate, &policy.wprio,
				&policy.wrssi, &policy.wphy_rate,
				&policy.wtx_failures, &policy.wtx_rate, &policy.wrx_rate,
				&policy.flags);
			if (num == 10) {
				memcpy(&bssinfo->sta_select_cfg, &policy,
					sizeof(bsd_sta_select_policy_t));
				bssinfo->sta_select_policy_defined = TRUE;
			}
			else {
				BSD_ERROR("intf[%s] bsd_sta_select[%s] format error\n",
					bssinfo->ifnames, str);
			}
		}

		BSD_INFO("Algo[%d] sta_select_policy[%d]: "
			"idle_rate=%d rssi=%d phyrate=%d "
			"wprio=%d wrssi=%d wphy_rate=%d "
			"wtx_failures=%d wtx_rate=%d wrx_rate=%d "
			"flags=0x%x defined=%s\n",
			bssinfo->algo, bssinfo->policy,
			bssinfo->sta_select_cfg.idle_rate,
			bssinfo->sta_select_cfg.rssi,
			bssinfo->sta_select_cfg.phyrate,
			bssinfo->sta_select_cfg.wprio,
			bssinfo->sta_select_cfg.wrssi,
			bssinfo->sta_select_cfg.wphy_rate,
			bssinfo->sta_select_cfg.wtx_failures,
			bssinfo->sta_select_cfg.wtx_rate,
			bssinfo->sta_select_cfg.wrx_rate,
			bssinfo->sta_select_cfg.flags,
			(bssinfo->sta_select_policy_defined == TRUE)?"YES":"NO");

		bsd_retrieve_static_maclist(bssinfo);

		/* Update BSS capability for this BSS */
		bsd_update_bss_capability(bssinfo);

		if (find_in_list(bsd_ifnames, var_intf))
		{
			bsd_steering_policy_t *steering_cfg = &intf_info->steering_cfg;
			bsd_if_qualify_policy_t *qualify_cfg = &intf_info->qualify_cfg;
			num = 0;

			bssinfo->flags |= BSD_FLAG_BSS_BSD_ENABLED;

			str = bsd_nvram_safe_get(rpc,
				strcat_r(prefix, BSD_STEERING_POLICY_NVRAM, tmp), &ret);
			if (ret == BSD_OK) {
				num = sscanf(str, "%d %d %d %d %d %x",
					&steering_cfg->chan_busy_max,
					&steering_cfg->period,
					&steering_cfg->cnt,
					&steering_cfg->rssi,
					&steering_cfg->phyrate,
					&steering_cfg->flags);
			}
			if (!str || (num != 6)) {
				steering_cfg->chan_busy_max = BSD_CHAN_BUSY_MAX;
				steering_cfg->period = BSD_CHAN_BUSY_PERIOD;
				steering_cfg->cnt = BSD_CHAN_BUSY_CNT;

				steering_cfg->rssi = 0;
				steering_cfg->phyrate = 0;
				steering_cfg->flags = 0x0;
			}

			BSD_INFO("%s idx[%d] max[%d] period[%d] cnt[%d] "
				"rssi[%d] phyrate[%d] flags[0x%x] state[%d]\n",
				prefix,
				idx,
				steering_cfg->chan_busy_max,
				steering_cfg->period, steering_cfg->cnt,
				steering_cfg->rssi, steering_cfg->phyrate,
				steering_cfg->flags,
				intf_info->state);

			/* read BSD_IF_QUALIFY_POLICY_NVRAM */
			num = 0;
			str = bsd_nvram_safe_get(rpc,
				strcat_r(prefix, BSD_IF_QUALIFY_POLICY_NVRAM, tmp), &ret);

			if (ret == BSD_OK) {
				num = sscanf(str, "%d %x %d",
					&qualify_cfg->min_bw,
					&qualify_cfg->flags,
					&qualify_cfg->rssi);
			}
			if (!str || (num != 3)) {
				if (num != 2) {
					qualify_cfg->min_bw = BSD_CHAN_BUSY_MIN;
					qualify_cfg->flags = 0x0;
				}
				qualify_cfg->rssi = BSD_QUALIFY_POLICY_RSSI_DEFAULT;
			}

			BSD_INFO("idx[%d] bsd_if_qualify_policy min_bw[%d] flags[0x%x] rssi[%d]\n",
				idx_intf,
				qualify_cfg->min_bw,
				qualify_cfg->flags,
				qualify_cfg->rssi);
		}

		total_intf++;
		if (total_intf >= (info->max_ifnum * WL_MAXBSSCFG)) {
			BSD_ERROR("break, intf max %d reached!\n", info->max_ifnum);
			break;
		}
	}

#ifdef BCM_WBD
	if (((info->enable_flag & BSD_FLAG_ENABLED) == 0) &&
		(info->enable_flag & BSD_FLAG_WBD_ENABLED)) {
		BSD_WBD("BSD disabled but WBD is enabled so skipping steer_prefix update init\n");
		BSD_EXIT();
		return BSD_OK;
	}
#endif /* BCM_WBD */

	BSD_INFO("-----------------------------------\n");

	/* update steer_prefix idx */
	for (idx_intf = 0; idx_intf < info->max_ifnum; idx_intf++) {
		intf_info = &(info->intf_info[idx_intf]);

		if (intf_info->enabled != TRUE) {
			BSD_INFO("Skip: idx_intf %d is not enabled\n", idx_intf);
			continue;
		}

		for (idx = 0; idx < WL_MAXBSSCFG; idx++) {
			bssinfo = &(intf_info->bsd_bssinfo[idx]);
			if (bssinfo->valid && BSD_BSS_BSD_ENABLED(bssinfo)) {
				int tifidx, tmp_idx;
				bsd_intf_info_t *tifinfo;
				bsd_bssinfo_t *tbssinfo;
				bool found = FALSE;
				char *prefix, *steer_prefix;

				BSD_INFO("bssinfo->steer_prefix:[%s][%s] [%d][%d]\n",
					bssinfo->steer_prefix, bssinfo->ssid, idx_intf, idx);
				for (tifidx = 0; !found && (tifidx < info->max_ifnum); tifidx++) {
					/* skip same band */
					if (idx_intf == tifidx)
						continue;

					/* following logic assumes two RFs only */
					tifinfo = &(info->intf_info[tifidx]);
					if (tifinfo->enabled != TRUE) {
						BSD_INFO("Skip: idx %d is not enabled\n", tifidx);
						continue;
					}

					for (tmp_idx = 0; tmp_idx < WL_MAXBSSCFG; tmp_idx++) {
						tbssinfo = &(tifinfo->bsd_bssinfo[tmp_idx]);
						BSD_INFO("tbssinfo->prefix[%s] ssid[%s]"
							"[%d][%d][v:%d]\n",
							tbssinfo->prefix, tbssinfo->ssid,
							tifidx, tmp_idx, tbssinfo->valid);

						if (!(tbssinfo->valid) ||
							!BSD_BSS_BSD_ENABLED(tbssinfo))
							continue;
						prefix = tbssinfo->prefix;
						steer_prefix = bssinfo->steer_prefix;
						if (!strcmp(tbssinfo->ssid, bssinfo->ssid) ||
							!strcmp(prefix, steer_prefix))
						{
							bssinfo->steer_bssinfo = tbssinfo;

							BSDSTRNCPY(bssinfo->steer_prefix,
								tbssinfo->prefix,
								sizeof(bssinfo->steer_prefix) - 1);
							bssinfo->steerflag = 0;
							if (intf_info->band == BSD_BAND_5G) {
								info->ifidx = idx_intf;
								info->bssidx = idx;
								BSD_INFO("Mon intf[%s] [%d][%d]\n",
									bssinfo->ifnames,
									idx_intf, idx);
							}

							found = TRUE;
							err = BSD_OK;
							break;
						}
					}
				}

				if (!found) {
					BSD_INFO("[%d][%d] %s [%s] cannot found steering match\n",
						idx_intf, idx, bssinfo->steer_prefix,
						bssinfo->ssid);
					bssinfo->steerflag = BSD_BSSCFG_NOTSTEER;
					BSD_INFO("Err: Set %s[%d][%d] to nosteer [%x] \n",
						bssinfo->prefix, idx_intf, idx, bssinfo->steerflag);
				}
				else {
					BSD_INFO("[%d][%d]:%s [%s] found steering match"
						"[%d][%d]:%s[%s]\n",
						idx_intf, idx, bssinfo->steer_prefix, bssinfo->ssid,
						tbssinfo->intf_info->idx, tbssinfo->idx,
						tbssinfo->prefix, tbssinfo->ssid);
				}
			}
		}
	}

	if (err == BSD_OK) {
		err = bsd_info_init_ext(bsd_info);
	}

	BSD_INFO("-----------------------------------\n");
	BSD_EXIT();
	return err;
}

/* enable = TRUE:  Add to assoclist
 * enable = FALSE: Query only
 * In the future this function should be split into 2 separate find/add functions
 */
bsd_sta_info_t *bsd_add_assoclist(bsd_bssinfo_t *bssinfo, struct ether_addr *addr, bool enable)
{
	bsd_sta_info_t *sta, *head;

	BSD_ENTER();

	sta = bssinfo->assoclist;

	BSD_INFO("enable=%d, ifname: %s[%p] sta[%p]:"MACF"\n",
		enable, bssinfo->ifnames, bssinfo, sta, ETHERP_TO_MACF(addr));

	if (enable) {
		int idx_intf, bssidx;
		bsd_intf_info_t *intf_info;
		bsd_bssinfo_t *other_bss;

		BSD_INFO("remove sta "MACF" from other interfaces if found\n",
			ETHERP_TO_MACF(addr));

		for (idx_intf = 0; idx_intf < bsd_info->max_ifnum; idx_intf++) {
			intf_info = &(bsd_info->intf_info[idx_intf]);
			for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
				other_bss = &(intf_info->bsd_bssinfo[bssidx]);
				if (other_bss == bssinfo)
					continue;
				if (other_bss->assoclist)
					bsd_remove_assoclist(other_bss, addr);
			}
		}
	}

	while (sta) {
		BSD_INFO("cmp: sta[%p]:"MACF"\n", sta, ETHER_TO_MACF(sta->addr));
		if (eacmp(&(sta->addr), addr) == 0) {
			break;
		}
		sta = sta->next;
	}

	if (enable && !sta) {
		sta = malloc(sizeof(bsd_sta_info_t));
		if (!sta) {
			BSD_INFO("%s@%d: Malloc failure\n", __FUNCTION__, __LINE__);
			return NULL;
		}

		memset(sta, 0, sizeof(bsd_sta_info_t));
		memcpy(&sta->addr, addr, sizeof(struct ether_addr));

		sta->timestamp = time(NULL);
		sta->active = time(NULL);
		sta->bssinfo = bssinfo;

		sta->prio = bssinfo->prio;
		sta->steerflag = bssinfo->steerflag;

		head = bssinfo->assoclist;
		if (head)
			head->prev = sta;

		sta->next = head;
		sta->prev = (struct bsd_sta_info *)&(bssinfo->assoclist);
		bssinfo->assoclist = sta;

		BSD_INFO("head[%p] sta[%p]:"MACF" prio:0x%x steerflag:0x%x\n",
			head, sta, ETHERP_TO_MACF(addr), sta->prio, sta->steerflag);

	}

	BSD_INFO("sta[%p]\n", sta);

	BSD_EXIT();
	return sta;
}

void bsd_clear_assoclist_bs_data(bsd_bssinfo_t *bssinfo)
{
	bsd_sta_info_t *sta;

	BSD_ENTER();

	sta = bssinfo->assoclist;

	while (sta) {
		BSD_AT("cmp: sta[%p]:"MACF"\n", sta, ETHER_TO_MACF(sta->addr));
		sta->at_ratio = 0;
		sta->phyrate = 0;
		sta->tx_rate = 0;
		sta->rx_rate = 0;
		sta->datarate = 0;
		sta = sta->next;
	}

	BSD_EXIT();
}

void bsd_remove_assoclist(bsd_bssinfo_t *bssinfo, struct ether_addr *addr)
{
	bool found = FALSE;
	bsd_sta_info_t *assoclist = NULL, *prev, *head;

	BSD_ENTER();

	assoclist = bssinfo->assoclist;

	if (assoclist == NULL) {
		BSD_INFO("%s: ifname:%s empty assoclist \n",
			__FUNCTION__, bssinfo->ifnames);
		BSD_INFO("%s Exiting....\n", __FUNCTION__);
		return;
	}

	BSD_INFO("sta[%p]:"MACF"[cmp]"MACF"\n",
		assoclist,
		ETHERP_TO_MACF(&assoclist->addr), ETHERP_TO_MACF(addr));

	if (eacmp(&(assoclist->addr), addr) == 0) {
		head = assoclist->next;
		bssinfo->assoclist = head;
		if (head)
			head->prev = (struct bsd_sta_info *)&(bssinfo->assoclist);
		found = TRUE;
	} else {
		prev = assoclist;
		assoclist = prev->next;

		while (assoclist) {
			BSD_INFO("sta[%p]:"MACF"[cmp]"MACF"\n",
				assoclist, ETHERP_TO_MACF(&assoclist->addr),
				ETHERP_TO_MACF(addr));

			if (eacmp(&(assoclist->addr), addr) == 0) {
				head = assoclist->next;
				prev->next = head;
				if (head)
					head->prev = prev;

				found = TRUE;
				break;
			}

			prev = assoclist;
			assoclist = prev->next;
		}
	}

	if (found) {
		bssinfo->assoc_cnt--;
		BSD_INFO("remove sta[%p]:"MACF" assoc_cnt:%d\n",
			assoclist, ETHERP_TO_MACF(addr), bssinfo->assoc_cnt);
		free(assoclist);
	}
	else {
		BSD_INFO("doesn't find sta:"MACF"\n", ETHERP_TO_MACF(addr));
	}

	BSD_EXIT();
	return;
}

/* remove disassoc STA from list */
void bsd_remove_sta_reason(bsd_info_t *info, char *ifname, uint8 remote, struct ether_addr *addr,
	bsd_sta_state_t reason)
{
	bsd_bssinfo_t *bssinfo;

	BSD_ENTER();

	bssinfo = bsd_bssinfo_by_ifname(info, ifname, remote);

/*	if (!bssinfo || (bssinfo->steerflag & BSD_BSSCFG_NOTSTEER) ||
		(bssinfo->steer_bssinfo == NULL)) {
*/
	if (!bssinfo) {
		BSD_INFO("%s: not found steerable ifname:%s\n", __FUNCTION__, ifname);
		BSD_EXIT();
		return;
	}

	bsd_remove_assoclist(bssinfo, addr);

	BSD_EXIT();
}

/* Find STA from list */
bsd_sta_info_t *bsd_sta_by_addr(bsd_info_t *info, bsd_bssinfo_t *bssinfo,
struct ether_addr *addr, bool enable)
{
	bsd_sta_info_t *sta;
	bsd_staprio_config_t *ptr;

	BSD_ENTER();

	sta = bsd_add_assoclist(bssinfo, addr, enable);
	if (sta) {
		/* update staprio from staprio list */
		ptr = info->staprio;
		while (ptr) {
			if (eacmp(&(ptr->addr), addr) == 0) {
				sta->prio = ptr->prio;
				sta->steerflag = ptr->steerflag;
				break;
			}
			ptr = ptr->next;
		}
	}

	BSD_EXIT();
	return sta;
}

/* add assoc STA from list */
void bsd_assoc_sta(bsd_info_t *info, char *ifname, uint8 remote, struct ether_addr *addr)
{
	bsd_bssinfo_t *bssinfo;
	bsd_sta_info_t *sta;

	BSD_ENTER();

	/* add to list */
	bssinfo = bsd_bssinfo_by_ifname(info, ifname, remote);
	if (!bssinfo) {
		BSD_INFO("Not found steerable ifname:%s\n", ifname);
		return;
	}

	sta = bsd_sta_by_addr(info, bssinfo, addr, TRUE);
	if (!sta || !(sta->bssinfo->steer_bssinfo)) {
		if (sta) {
			BSD_INFO("sta[%p] is not in steer bssinfo[%s][%d]\n",
				sta, sta->bssinfo->ifnames, sta->bssinfo->idx);
		}
		return;
	}

	bsd_update_sta_state_transition(info, bssinfo->intf_info,
		addr, BSD_STA_STATE_ASSOC);

	/* update steer result */
	bsd_check_steer_result(info, addr, bssinfo);

	/* update steered intf acl maclist */
	bsd_stamp_maclist(info, bssinfo->steer_bssinfo, addr);
	bsd_remove_maclist(bssinfo, addr);

	BSD_INFO("sta[%p]:"MACF" prio:0x%x steerflag:0x%x\n",
		sta, ETHERP_TO_MACF(addr), sta->prio, sta->steerflag);

	if (BSD_DUMP_ENAB)
		bsd_dump_info(info);

	BSD_EXIT();
}

bool bsd_is_sta_dualband(bsd_info_t *info, struct ether_addr *addr)
{
	bsd_maclist_t *mac;

	mac = bsd_prbsta_by_addr(info, addr, FALSE);

	if (mac && ((mac->band & BSD_BAND_ALL) == BSD_BAND_ALL)) {
		return TRUE;
	}

	return FALSE;
}

void bsd_auth_sta(bsd_info_t *info, char *ifname, uint8 remote, struct ether_addr *addr)
{
	bsd_bssinfo_t *bssinfo;
	bsd_maclist_t *sta;

	BSD_ENTER();

	/* add to list */
	bssinfo = bsd_bssinfo_by_ifname(info, ifname, remote);

	if (!bssinfo) {
		BSD_INFO("Not found steerable ifname:%s\n", ifname);
		return;
	}

	/* use TRUE for passive scan STA */
	sta = bsd_prbsta_by_addr(info, addr, TRUE);
	if (sta) {
		sta->band |= bssinfo->intf_info->band;
	}

	bsd_update_sta_state_transition(info, bssinfo->intf_info, addr,  BSD_STA_STATE_AUTH);

	if (BSD_DUMP_ENAB)
		bsd_dump_info(info);

	BSD_EXIT();
}

void bsd_deauth_sta(bsd_info_t *info, char *ifname, uint8 remote, struct ether_addr *addr)
{
	BSD_ENTER();

	bsd_remove_sta_reason(info, ifname, remote,
		(struct ether_addr *)addr, BSD_STA_DEAUTH);
	if (BSD_DUMP_ENAB)
		bsd_dump_info(info);

	BSD_EXIT();
}

void bsd_disassoc_sta(bsd_info_t *info, char *ifname, uint8 remote, struct ether_addr *addr)
{
	BSD_ENTER();

	bsd_remove_sta_reason(info, ifname, remote,
		(struct ether_addr *)addr, BSD_STA_DISASSOC);
	if (BSD_DUMP_ENAB)
		bsd_dump_info(info);

	BSD_EXIT();
}

void bsd_update_psta(bsd_info_t *info, char *ifname, uint8 remote,
	struct ether_addr *addr, struct ether_addr *paddr)
{
	bsd_bssinfo_t *bssinfo;
	bsd_sta_info_t *sta, *psta;

	BSD_ENTER();

	bssinfo = bsd_bssinfo_by_ifname(info, ifname, remote);

	if (!bssinfo) {
		BSD_INFO("%s: not found ifname:%s\n", __FUNCTION__, ifname);
		return;
	}

	sta = bsd_sta_by_addr(info, bssinfo, addr, FALSE);
	psta = bsd_sta_by_addr(info, bssinfo, paddr, FALSE);

	if (!sta) {
		BSD_ERROR("Not found sta:"MACF" @ [%s]\n", ETHERP_TO_MACF(addr), ifname);
		BSD_EXIT();
		return;
	}

	if (!psta) {
		BSD_ERROR("Not found psta:"MACF" @ [%s]\n", ETHERP_TO_MACF(paddr), ifname);
		BSD_EXIT();
		return;
	}
	else if (!eacmp((&psta->paddr), &ether_null)) {
		/* if paddr in psta primary sta is NULL:
		   set its own addr as paddr, just as PSTA flag
		*/
		BSD_STEER("set paddr "MACF" to psta primary:"MACF"\n",
			ETHERP_TO_MACF(paddr), ETHER_TO_MACF(psta->addr));
		memcpy(&psta->paddr, paddr, sizeof(struct ether_addr));
	}

	memcpy(&sta->paddr, paddr, sizeof(struct ether_addr));
	sta->steerflag = psta->steerflag;
	sta->timestamp = time(NULL);

	BSD_INFO("sta: "MACF " steerflag=%d\n", ETHERP_TO_MACF(addr), sta->steerflag);
	BSD_EXIT();
}

static void bsd_scb_bs_data_convert_v2(iov_bs_data_struct_t *v2)
{
	/* This only take care of endianess between driver and application */
	int argn;
	for (argn = 0; argn < v2->structure_count; ++argn) {
		iov_bs_data_record_t *rec;
		iov_bs_data_counters_t *ctr;

		rec = &v2->structure_record[argn];
		ctr = &rec->station_counters;

		rec->station_flags = dtoh16(rec->station_flags);

#define DEVICE_TO_HOST64(xyzzy) ctr->xyzzy = dtoh64(ctr->xyzzy)
#define DEVICE_TO_HOST32(xyzzy) ctr->xyzzy = dtoh32(ctr->xyzzy)
		DEVICE_TO_HOST64(throughput);
		DEVICE_TO_HOST64(txrate_main);
		DEVICE_TO_HOST64(txrate_succ);
		DEVICE_TO_HOST32(retry_drop);
		DEVICE_TO_HOST32(rtsfail);
		DEVICE_TO_HOST32(retry);
		DEVICE_TO_HOST32(acked);
		DEVICE_TO_HOST32(ru_acked);
		DEVICE_TO_HOST32(mu_acked);
		DEVICE_TO_HOST32(time_delta);
		DEVICE_TO_HOST32(airtime);
		DEVICE_TO_HOST32(txbw);
		DEVICE_TO_HOST32(txnss);
		DEVICE_TO_HOST32(txmcs);
#undef DEVICE_TO_HOST64
#undef DEVICE_TO_HOST32
	}
}

static void bsd_scb_bs_data_convert_v1(iov_bs_data_struct_v1_t *v1, iov_bs_data_struct_t *v2)
{
	int argn;
	int max_stations;

	v2->structure_version = v1->structure_version;
	v2->structure_count = v1->structure_count;

	/* Calculating the maximum number of stations that v2 can hold  */
	max_stations = (BSD_IOCTL_MAXLEN / sizeof(iov_bs_data_struct_t)) - 1;

	for (argn = 0; (argn < v1->structure_count) &&
		(argn < max_stations); ++argn) {
		iov_bs_data_record_v1_t *rec_v1;
		iov_bs_data_counters_v1_t *ctr_v1;
		iov_bs_data_record_t *rec_v2;
		iov_bs_data_counters_t *ctr_v2;

		rec_v2 = &v2->structure_record[argn];
		ctr_v2 = &rec_v2->station_counters;

		rec_v1 = &v1->structure_record[argn];
		ctr_v1 = &rec_v1->station_counters;

		memcpy(&rec_v2->station_address, &rec_v1->station_address, ETHER_ADDR_LEN);
		rec_v2->station_flags = rec_v1->station_flags;

		ctr_v2->throughput = (uint64)dtoh32(ctr_v1->throughput);
		ctr_v2->txrate_main = (uint64)dtoh32(ctr_v1->txrate_main);
		ctr_v2->txrate_succ = (uint64)dtoh32(ctr_v1->txrate_succ);
		ctr_v2->txrate_succ *= (PERF_LOG_RATE_FACTOR_500 / PERF_LOG_RATE_FACTOR_100);

		ctr_v2->retry_drop = dtoh32(ctr_v1->retry_drop);
		ctr_v2->rtsfail = dtoh32(ctr_v1->rtsfail);
		ctr_v2->retry = dtoh32(ctr_v1->retry);
		ctr_v2->acked = dtoh32(ctr_v1->acked);
		ctr_v2->time_delta = dtoh32(ctr_v1->time_delta);
		ctr_v2->airtime = dtoh32(ctr_v1->airtime);

		ctr_v2->txbw = 0;
		ctr_v2->txmcs = 0;
		ctr_v2->txnss = 0;
		ctr_v2->ru_acked = 0;
		ctr_v2->mu_acked = 0;
	}
}

void bsd_retrieve_bs_data(bsd_bssinfo_t *bssinfo)
{
	iov_bs_data_struct_t *data = (iov_bs_data_struct_t *)ioctl_buf;
	int argn;
	int ret;
	bsd_sta_info_t *sta = NULL;
	iov_bs_data_record_t *rec;
	iov_bs_data_counters_t *ctr;
	float datarate;
	float phyrate;
	float air, rtr;
	void *p = NULL;

	BSD_ATENTER();

	memset(ioctl_buf, 0, sizeof(ioctl_buf));
	strcpy(ioctl_buf, "bs_data");
	BSD_RPC("RPC name:%s cmd: %d(WLC_GET_VAR: bs_data)\n", bssinfo->ifnames, WLC_GET_VAR);
	ret = bsd_wl_ioctl(bssinfo, WLC_GET_VAR, ioctl_buf, sizeof(ioctl_buf) - BSD_RPC_HEADER_LEN);

	if (ret < 0) {
		BSD_ERROR("Err to read bs_data: %s\n", bssinfo->ifnames);
		BSD_ATEXIT();
		return;
	}

	BSD_AT("ifnames=%s[remote:%d] data->structure_count=%d\n",
		bssinfo->ifnames, bssinfo->intf_info->remote, data->structure_count);

	if (data->structure_count == 0) {
		BSD_ATEXIT();
	}

	/* Convert v1 format into v2 if needed */
	if (data->structure_version == SCB_BS_DATA_STRUCT_VERSION_v1) {
		/* Alloc some memory and convert all incoming v1 format
		 * into v2, and redirect data to it
		 */
		p = (void*)malloc(BSD_IOCTL_MAXLEN);
		if (!p) {
			BSD_ERROR("%s@%d: Malloc failure\n", __FUNCTION__, __LINE__);
			BSD_ATEXIT();
		}
		bsd_scb_bs_data_convert_v1((iov_bs_data_struct_v1_t *)data,
			(iov_bs_data_struct_t *)p);
		data = (iov_bs_data_struct_t*)p;
	} else if (data->structure_version == SCB_BS_DATA_STRUCT_VERSION) {
		bsd_scb_bs_data_convert_v2(data);
	} else {
		BSD_ERROR("blanket / wl driver mismatch, expect V%d format, got %d.\n",
			SCB_BS_DATA_STRUCT_VERSION, data->structure_version);
		BSD_ATEXIT();
	}

	for (argn = 0; argn < data->structure_count; ++argn) {
		rec = &data->structure_record[argn];
		ctr = &rec->station_counters;

		if (ctr->acked == 0) continue;

		BSD_AT("STA:"MACF"\t", ETHER_TO_MACF(rec->station_address));

		/* Calculate PHY rate */
		phyrate = (float)ctr->txrate_succ *
			(PERF_LOG_RATE_FACTOR_100 / 1000.0) / (float)ctr->acked;

		/* Calculate Data rate */
		datarate = (ctr->time_delta) ?
			(float)ctr->throughput * 8000.0 / (float)ctr->time_delta : 0.0;

		/* Calculate % airtime */
		air = (ctr->time_delta) ? ((float)ctr->airtime * 100.0 /
			(float) ctr->time_delta) : 0.0;
		if (air > 100)
			air = 100;

		/* Calculate retry percentage */
		rtr = (float)ctr->retry / (float)ctr->acked * 100;
		if (rtr > 100)
			rtr = 100;

		BSD_AT("phyrate[%10.1f] [datarate]%10.1f [air]%9.1f%% [retry]%9.1f%%\n",
			phyrate, datarate, air, rtr);
		sta = bsd_add_assoclist(bssinfo, &(rec->station_address), FALSE);
		if (sta) {
			BSD_AT("sta[%p] Mac:"MACF"\n", sta, ETHER_TO_MACF(sta->addr));
			sta->at_ratio = air;
			sta->phyrate = phyrate;
			sta->datarate = datarate;
		}
	}

	if (p) {
		free(p);
	}

	BSD_ATEXIT();
}

/* Update sta_info */
void bsd_update_stainfo(bsd_info_t *info)
{
	bsd_intf_info_t *intf_info;
	bsd_bssinfo_t *bssinfo;
	int idx, bssidx, cnt;
	bsd_sta_info_t *sta = NULL;
	struct ether_addr ea;

	struct maclist *maclist = (struct maclist *) maclist_buf;
	int count = 0;
	char *param;
	int buflen;
	int ret;
	time_t now = time(NULL);
	uint32 tx_tot_pkts = 0, rx_tot_pkts = 0;
	uint32	delta_txframe, delta_rxframe;
	txpwr_target_max_t *txpwr;
	int i, val = 0;

	BSD_ENTER();

	for (idx = 0; idx < info->max_ifnum; idx++) {
		intf_info = &(info->intf_info[idx]);
		BSD_INFO("Start For idx=%d\n", idx);

		if (intf_info->enabled != TRUE) {
			BSD_INFO("Skip: idx %d is not enabled\n", idx);
			continue;
		}

		for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
			bssinfo = &(intf_info->bsd_bssinfo[bssidx]);
			if (!(bssinfo->valid) || !BSD_BSS_BSD_OR_WBD_ENABLED(bssinfo))
				continue;
/*			if (bssinfo->steerflag & BSD_BSSCFG_NOTSTEER)
				continue;
*/
			BSD_INFO("bssidx=%d intf:%s\n", bssidx, bssinfo->ifnames);

			memset(ioctl_buf, 0, sizeof(ioctl_buf));
			strcpy(ioctl_buf, "chanspec");
			BSD_RPC("RPC name:%s cmd: %d(WLC_GET_VAR: chanspec)\n",
				bssinfo->ifnames, WLC_GET_VAR);
			ret = bsd_wl_ioctl(bssinfo, WLC_GET_VAR, (void *)ioctl_buf,
				sizeof(ioctl_buf) - BSD_RPC_HEADER_LEN);

			if (ret < 0) {
				BSD_ERROR("Err to read chanspec: %s\n", bssinfo->ifnames);
				continue;
			}
			bssinfo->chanspec = (chanspec_t)(*((uint32 *)ioctl_buf));
			BSD_INFO("chanspec: 0x%x\n", bssinfo->chanspec);

			memset(ioctl_buf, 0, sizeof(ioctl_buf));
			strcpy(ioctl_buf, "rclass");
			buflen = strlen(ioctl_buf) + 1;
			param = (char *)(ioctl_buf + buflen);
			memcpy(param, &bssinfo->chanspec, sizeof(chanspec_t));

			BSD_RPC("RPC name:%s cmd: %d(WLC_GET_VAR: rclass)\n",
				bssinfo->ifnames, WLC_GET_VAR);
			ret = bsd_wl_ioctl(bssinfo, WLC_GET_VAR, (void *)ioctl_buf,
				sizeof(ioctl_buf) - BSD_RPC_HEADER_LEN);

			if (ret < 0) {
				BSD_ERROR("Err to read rclass. ifname:%s chanspec:0x%x\n",
					bssinfo->ifnames, bssinfo->chanspec);
				continue;
			}
			bssinfo->rclass = (uint8)(*((uint32 *)ioctl_buf));
			BSD_INFO("rclass:0x%x\n", bssinfo->rclass);

			BSD_RPC("RPC name:%s cmd: %d(WLC_GET_BSSID)\n",
				bssinfo->ifnames, WLC_GET_BSSID);
			if (bsd_wl_ioctl(bssinfo, WLC_GET_BSSID, &ea, ETHER_ADDR_LEN) < 0) {
				BSD_ERROR("Err: ifnams[%s] get bssid failure\n", bssinfo->ifnames);
				continue;
			}
			memcpy(&bssinfo->bssid, &ea, ETHER_ADDR_LEN);
			BSD_INFO("bssid:"MACF"\n", ETHER_TO_MACF(bssinfo->bssid));

			ret = bsd_wl_ioctl(bssinfo, WLC_GET_UP, &val, sizeof(val));
			if (!val) {
				BSD_ERROR("Err: ifname[%s] is not up, val is [%d]\n",
					bssinfo->ifnames, val);
				continue;
			}
			memset(ioctl_buf, 0, sizeof(ioctl_buf));
			strcpy(ioctl_buf, "txpwr_target_max");

			BSD_RPC("RPC name:%s cmd: %d(WLC_GET_VAR: txpwr_target)\n",
				bssinfo->ifnames, WLC_GET_VAR);
			ret = bsd_wl_ioctl(bssinfo, WLC_GET_VAR, ioctl_buf,
				sizeof(ioctl_buf) - BSD_RPC_HEADER_LEN);

			if (ret < 0) {
				BSD_ERROR("Err to read txpwr_target. ifname:%s chanspec:0x%x\n",
					bssinfo->ifnames, bssinfo->chanspec);
				continue;
			}
			txpwr = (txpwr_target_max_t *)ioctl_buf;

			BSD_INFO("Maximum Tx Power Target (chanspec:0x%x), rf_cores:%d:\n",
				txpwr->chanspec, txpwr->rf_cores);

			for (i = 0; i < txpwr->rf_cores; i++)
				BSD_INFO("rf_core[%d] %2d.%02d\n",
					i,
					DIV_QUO(txpwr->txpwr[i], 4),
					DIV_REM(txpwr->txpwr[i], 4));

			memcpy(&bssinfo->txpwr, txpwr, sizeof(bssinfo->txpwr));

			bsd_clear_assoclist_bs_data(bssinfo);
			bsd_retrieve_bs_data(bssinfo);

			/* read assoclist */
			memset(maclist_buf, 0, sizeof(maclist_buf));
			maclist->count = ((sizeof(maclist_buf)- 300 - sizeof(int))/ETHER_ADDR_LEN);
			BSD_RPC("RPC name:%s cmd: %d(WLC_GET_ASSOCLIST)\n",
				bssinfo->ifnames, WLC_GET_ASSOCLIST);
			ret = bsd_wl_ioctl(bssinfo,  WLC_GET_ASSOCLIST,
				(void *)maclist,  sizeof(maclist_buf)-BSD_RPC_HEADER_LEN);

			if (ret < 0) {
				BSD_ERROR("Err: ifnams[%s] get assoclist\n", bssinfo->ifnames);
				continue;
			}
			count = maclist->count;
			bssinfo->assoc_cnt = 0;
			BSD_INFO("assoclist count = %d\n", count);

			if (count) {
				/* calculate throughput, just accumulate all STA's datarate */
				bssinfo->throughput = 0;
			}

			/* Parse assoc list and read all sta_info */
			for (cnt = 0; cnt < count; cnt++) {
				time_t gap;
				sta_info_t *sta_info;

				scb_val_t scb_val;
				int32 rssi;
				bsd_maclist_t *prbsta;
				uint32 rx_rate = 0;

				BSD_INFO("sta_info sta:"MACF"\n", ETHER_TO_MACF(maclist->ea[cnt]));

				/* skip the blocked sta */
				if (bsd_maclist_by_addr(bssinfo, &(maclist->ea[cnt]))) {
					BSD_INFO("Skipp STA:"MACF", found in maclist\n",
						ETHER_TO_MACF(maclist->ea[cnt]));
					continue;
				}

				/* refresh this STA on probe sta list */
				prbsta = bsd_prbsta_by_addr(info, &(maclist->ea[cnt]), FALSE);
				if (prbsta) {
					prbsta->active = time(NULL);
				} else {
					BSD_WARNING("Warning: probe list has no sta:"MACF"\n",
						ETHER_TO_MACF(maclist->ea[cnt]));
				}

				memset(&scb_val, 0, sizeof(scb_val));
				memcpy(&scb_val.ea, &maclist->ea[cnt], ETHER_ADDR_LEN);

				BSD_RPC("RPC name:%s cmd: %d(WLC_GET_RSSI)\n",
					bssinfo->ifnames, WLC_GET_RSSI);
				ret = bsd_wl_ioctl(bssinfo, WLC_GET_RSSI,
					&scb_val, sizeof(scb_val));

				if (ret < 0) {
					BSD_ERROR("Err: reading intf:%s STA:"MACF" RSSI\n",
						bssinfo->ifnames, ETHER_TO_MACF(maclist->ea[cnt]));
					continue;
				}
				rssi = scb_val.val;
				BSD_HISTO("STA:"MACF" RSSI=%d\n",
					ETHER_TO_MACF(maclist->ea[cnt]), rssi);

				strcpy(ioctl_buf, "sta_info");
				buflen = strlen(ioctl_buf) + 1;
				param = (char *)(ioctl_buf + buflen);
				memcpy(param, &maclist->ea[cnt], ETHER_ADDR_LEN);

				BSD_RPC("RPC name:%s cmd: %d(WLC_GET_VAR: sta_info)\n",
					bssinfo->ifnames, WLC_GET_VAR);
				ret = bsd_wl_ioctl(bssinfo, WLC_GET_VAR,
					ioctl_buf, sizeof(ioctl_buf) - BSD_RPC_HEADER_LEN);

				if (ret < 0) {
					BSD_ERROR("Err: intf:%s STA:"MACF" sta_info\n",
						bssinfo->ifnames, ETHER_TO_MACF(maclist->ea[cnt]));
					continue;
				}

				sta_info = (sta_info_t *)ioctl_buf;

				sta = bsd_sta_by_addr(info, bssinfo, &(maclist->ea[cnt]), FALSE);

				if (!sta) {
					/* consider the STA may be on dual wl radio assoclist case
					 * only add the sta to bsd assoclist:
					 * if the STA is not on any radio
					 * (not added by (re)assoc event)
					 * and txrate is greater than other on other bss.
					 */
					sta = bsd_check_sta_assoc_other_bss(info, bssinfo,
						&(maclist->ea[cnt]));
					if (sta && (sta->tx_rate > (sta_info->tx_rate / 1000))) {
						BSD_INFO("STA ["MACF"] is on %s assoclist with"
							" tx_rate[%d] > on interface[%s] with"
							" tx_rate[%d] \n",
							ETHER_TO_MACF(maclist->ea[cnt]),
							sta->bssinfo->ifnames, sta->tx_rate,
							bssinfo->ifnames,
							(sta_info->tx_rate / 1000));
						continue;
					}
					sta = bsd_sta_by_addr(info, bssinfo, &(maclist->ea[cnt]),
						TRUE);
					if (!sta) {
						BSD_ERROR("Exiting... Error update [%s] "
							"sta:"MACF"\n",
							bssinfo->ifnames,
							ETHER_TO_MACF(maclist->ea[cnt]));
						continue;
					}
				}

				sta->flags = sta_info->flags;
				sta->rx_rate = (sta_info->rx_rate / 1000);
				sta->tx_rate = (sta_info->tx_rate / 1000);
				sta->rssi = rssi;
				bsd_log_rssi(info, sta, FALSE);

				sta->wnm_cap = sta_info->wnm_cap;

				BSD_STEER("sta:"MACF" flags:0x%x capability: %s, RSSI:%d "
					"phyrate:%d tx_rate:%d\n",
					ETHER_TO_MACF(sta->addr), sta->flags,
					(sta->flags & WL_STA_VHT_CAP) ? "VHT" : "Non-VHT",
					sta->rssi,
					sta->phyrate,
					sta->tx_rate);

				/* adjust sta_info's rspec based tx_rate, avoid junk # */
				/* WAR: if tx_rate droped too much, don't update mcs_phyrate */
				if (sta->mcs_phyrate && ((sta->mcs_phyrate / 4) > sta->tx_rate)) {
					BSD_STEER("Warning: Strange tx_rate=%d for "MACF" on %s\n",
						sta->tx_rate, ETHERP_TO_MACF(&sta->addr),
						bssinfo->ifnames);
				}
				else {
					sta->mcs_phyrate =
						sta->tx_rate < BSD_MAX_DATA_RATE ?
						sta->tx_rate : 0;
				}

				BSD_WARNING("[%d] sta:"MACF" flags:0x%x "
					"phyrate:%d mcs_phyrate:%d %s rssi:%d\n",
					idx,
					ETHER_TO_MACF(sta->addr), sta->flags,
					sta->phyrate, sta->mcs_phyrate,
					(sta->flags & WL_STA_VHT_CAP) ? "VHT" : "Non-VHT",
					sta->rssi);

				if (now <= sta->active)
					gap = info->status_poll;
				else
					gap = now - sta->active;

				sta->rx_pkts = 0;
				sta->rx_bps = 0;
				if (sta_info->rx_tot_pkts > sta->rx_tot_pkts) {
					sta->rx_pkts = sta_info->rx_tot_pkts - sta->rx_tot_pkts;

					/* rx_rate shall be aggregated into datarate calculation */
					if (sta_info->rx_tot_bytes > sta->rx_tot_bytes) {
						rx_rate = (sta_info->rx_tot_bytes -
							sta->rx_tot_bytes)*8 / (gap * 1000);
						sta->datarate += rx_rate;
					}
					sta->rx_bps = sta->rx_pkts / (gap * 1000);
					BSD_INFO("Mac:"MACF" rx_bps[%d] = "
						"rx_pkts[%d](%d - %d)/gap[%lu] @ timestamp:[%lu] "
						"phyrate:%d, datarate:%d rx_rate:%d\n",
						ETHER_TO_MACF(maclist->ea[cnt]),
						sta->rx_bps, sta->rx_pkts,
						sta_info->rx_tot_pkts, sta->rx_tot_pkts,
						(unsigned long)gap, (unsigned long)now,
						sta->phyrate, sta->datarate, rx_rate);
				}

				sta->tx_pkts = 0;
				sta->tx_bps = 0;
				if (sta_info->tx_tot_pkts > sta->tx_tot_pkts) {
					sta->tx_pkts = sta_info->tx_tot_pkts - sta->tx_tot_pkts;
					sta->tx_bps = sta->tx_pkts / (gap * 1000);
					BSD_INFO("Mac:"MACF" tx_bps[%d] = "
						"tx_pkts[%d](%d - %d)/gap[%lu] @ timestamp:[%lu] "
						"phyrate:%d, datarate:%d\n",
						ETHER_TO_MACF(maclist->ea[cnt]),
						sta->tx_bps, sta->tx_pkts,
						sta_info->tx_tot_pkts, sta->tx_tot_pkts,
						(unsigned long)gap, (unsigned long)now,
						sta->phyrate, sta->datarate);
				}

				/* Tx failures between two intervals */
				if (sta_info->tx_failures > sta->tx_tot_failures) {
					sta->tx_failures =
						sta_info->tx_failures - sta->tx_tot_failures;
				} else {
					sta->tx_failures = 0;
				}

				sta->rx_tot_pkts = sta_info->rx_tot_pkts;
				sta->tx_tot_pkts = sta_info->tx_tot_pkts;
				sta->rx_tot_bytes = sta_info->rx_tot_bytes;
				sta->tx_tot_failures = sta_info->tx_failures;
				sta->in = sta_info->in;
				sta->idle = sta_info->idle;
				sta->active = time(NULL);
				BSD_HISTO("sta[%p]:"MACF" active=%lu\n",
					sta, ETHER_TO_MACF(sta->addr),
					(unsigned long)(sta->active));

#ifdef BCM_WBD
				/* Check for DWDS flag. If DWDS, set the STA status */
				if ((sta_info->flags & WL_STA_DWDS_CAP) &&
					(sta_info->flags & WL_STA_DWDS)) {
					sta->wbd_sta_status |= BSD_WBD_STA_DWDS;
					BSD_WBD("ifname : %s STA["MACF"] is DWDS STA[%d]\n",
						bssinfo->ifnames, ETHER_TO_MACF(sta->addr),
						sta->wbd_sta_status);
				}
#endif /* BCM_WBD */
				/* cale STB tx/rx */
				if (sta->steerflag & BSD_BSSCFG_NOTSTEER) {
					tx_tot_pkts += sta_info->tx_tot_pkts;
					rx_tot_pkts += sta_info->rx_tot_pkts;

					BSD_STEER("[%s] "MACF" tx_tot_pkts[%d] rx_tot_pkts[%d]\n",
						bssinfo->ifnames, ETHER_TO_MACF(maclist->ea[cnt]),
						sta_info->tx_tot_pkts, sta_info->rx_tot_pkts);
				}
				else {
					(bssinfo->assoc_cnt)++;
				}

				bssinfo->throughput += sta->datarate;
			}
			delta_rxframe = 0;
			delta_txframe = 0;

			if (tx_tot_pkts > bssinfo->tx_tot_pkts)
				delta_txframe = tx_tot_pkts - bssinfo->tx_tot_pkts;

			if (rx_tot_pkts > bssinfo->rx_tot_pkts)
				delta_rxframe = rx_tot_pkts - bssinfo->rx_tot_pkts;

			BSD_STEER("last: txframe[%d] rxframe[%d] cnt:txframe[%d] rxframe[%d]\n",
				bssinfo->tx_tot_pkts, bssinfo->rx_tot_pkts,
				tx_tot_pkts, rx_tot_pkts);

			BSD_STEER("delta[tx+rx]=%d threshold=%d\n",
				delta_txframe + delta_rxframe,
				info->counter_poll * info->idle_rate);

			bssinfo->video_idle = 0;
			if ((delta_txframe + delta_rxframe) <
				(info->counter_poll * info->idle_rate)) {
				bssinfo->video_idle = 1;
			}

			BSD_STEER("ifname: %s, video_idle=%d\n",
				bssinfo->ifnames, bssinfo->video_idle);

			bssinfo->tx_tot_pkts = tx_tot_pkts;
			bssinfo->rx_tot_pkts = rx_tot_pkts;
		}
	}

	BSD_EXIT();
}

/* Update video STA counters */
void bsd_update_stb_info(bsd_info_t *info)
{
	bsd_intf_info_t *intf_info;
	bsd_bssinfo_t *bssinfo;
	int idx, bssidx, cnt;

	struct maclist *maclist = (struct maclist *) maclist_buf;
	int count = 0;
	char *param;
	int buflen;
	int ret;
	uint32 tx_tot_pkts = 0, rx_tot_pkts = 0;
	uint32	delta_txframe, delta_rxframe;

	BSD_ENTER();

	for (idx = 0; idx < info->max_ifnum; idx++) {
		intf_info = &(info->intf_info[idx]);
		BSD_STEER("idx=%d\n", idx);
		if (idx != info->ifidx) {
			BSD_STEER("skip idx=%d\n", idx);
			continue;
		}

		for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
			bssinfo = &(intf_info->bsd_bssinfo[bssidx]);

			if (!(bssinfo->valid) || !BSD_BSS_BSD_OR_WBD_ENABLED(bssinfo))
				continue;

			if (bssidx == info->bssidx) {
				BSD_STEER("skip bssidx=%d\n", bssidx);
				continue;
			}

			BSD_STEER("Cal: ifnames[%s] [%d]{%d]\n", bssinfo->ifnames, idx, bssidx);

			bsd_retrieve_bs_data(bssinfo);

			/* read assoclist */
			memset(maclist_buf, 0, sizeof(maclist_buf));
			maclist->count = (sizeof(maclist_buf) - sizeof(int))/ETHER_ADDR_LEN;

			BSD_RPC("RPC name:%s cmd: %d(WLC_GET_ASSOCLIST)\n",
				bssinfo->ifnames, WLC_GET_ASSOCLIST);
			ret = bsd_wl_ioctl(bssinfo,  WLC_GET_ASSOCLIST,
				(void *)maclist,  sizeof(maclist_buf) - BSD_RPC_HEADER_LEN);

			if (ret < 0) {
				BSD_ERROR("Err: ifnams[%s] get assoclist\n", bssinfo->ifnames);
				continue;
			}
			count = maclist->count;
			bssinfo->assoc_cnt = count;
			BSD_STEER("assoclist count = %d\n", count);

			/* Parse assoc list and read all sta_info */
			for (cnt = 0; cnt < count; cnt++) {
				sta_info_t *sta_info;

				BSD_STEER("sta_info sta:"MACF"\n", ETHER_TO_MACF(maclist->ea[cnt]));

				strcpy(ioctl_buf, "sta_info");
				buflen = strlen(ioctl_buf) + 1;
				param = (char *)(ioctl_buf + buflen);
				memcpy(param, &maclist->ea[cnt], ETHER_ADDR_LEN);

				BSD_RPC("RPC name:%s cmd: %d(WLC_GET_VAR: sta_info)\n",
					bssinfo->ifnames, WLC_GET_VAR);
				ret = bsd_wl_ioctl(bssinfo, WLC_GET_VAR, ioctl_buf,
					sizeof(ioctl_buf) - BSD_RPC_HEADER_LEN);

				if (ret < 0) {
					BSD_ERROR("Err: intf:%s STA:"MACF" sta_info\n",
						bssinfo->ifnames, ETHER_TO_MACF(maclist->ea[cnt]));
					continue;
				}

				sta_info = (sta_info_t *)ioctl_buf;

				tx_tot_pkts += sta_info->tx_tot_pkts;
				rx_tot_pkts += sta_info->rx_tot_pkts;

				BSD_STEER("intf:%s STA:"MACF" tx_tot_pkts[%d] rx_tot_pkts[%d]\n",
					bssinfo->ifnames, ETHER_TO_MACF(maclist->ea[cnt]),
					sta_info->tx_tot_pkts, sta_info->rx_tot_pkts);

			}
			delta_rxframe = 0;
			delta_txframe = 0;

			if (tx_tot_pkts > bssinfo->tx_tot_pkts)
				delta_txframe = tx_tot_pkts - bssinfo->tx_tot_pkts;

			if (rx_tot_pkts > bssinfo->rx_tot_pkts)
				delta_rxframe = rx_tot_pkts - bssinfo->rx_tot_pkts;

			BSD_STEER("last: txframe[%d] rxframe[%d] cnt:txframe[%d] rxframe[%d]\n",
				bssinfo->tx_tot_pkts, bssinfo->rx_tot_pkts,
				tx_tot_pkts, rx_tot_pkts);

			BSD_STEER("delta[tx+rx]=%d threshold=%d\n",
				delta_txframe + delta_rxframe,
				info->counter_poll * info->idle_rate);

			bssinfo->video_idle = 0;
			if ((delta_txframe + delta_rxframe) <
				(info->counter_poll * info->idle_rate)) {
				bssinfo->video_idle = 1;
			}

			BSD_STEER("ifname: %s, video_idle=%d\n",
				bssinfo->ifnames, bssinfo->video_idle);
			bssinfo->tx_tot_pkts = tx_tot_pkts;
			bssinfo->rx_tot_pkts = rx_tot_pkts;
		}
	}

	BSD_EXIT();
}

/* remove dead STA from list */
void bsd_timeout_sta(bsd_info_t *info)
{
	bsd_intf_info_t *intf_info;
	bsd_bssinfo_t *bssinfo;
	int idx, bssidx;
	bsd_sta_info_t *sta, *next, *prev, *head;
	time_t now = time(NULL);

	BSD_ENTER();

	for (idx = 0; idx < info->max_ifnum; idx++) {
		intf_info = &(info->intf_info[idx]);
		BSD_INFO("idx=%d\n", idx);

		for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
			bssinfo = &(intf_info->bsd_bssinfo[bssidx]);

			if (!(bssinfo->valid) || !BSD_BSS_BSD_OR_WBD_ENABLED(bssinfo))
				continue;
			if (bssinfo->steerflag & BSD_BSSCFG_NOTSTEER)
				continue;

			BSD_INFO("bssidx=%d intf:%s\n", bssidx, bssinfo->ifnames);

			sta = bssinfo->assoclist;
			head = NULL;
			prev = NULL;

			while (sta) {
				BSD_INFO("sta[%p]:"MACF" active=%lu\n",
					sta, ETHER_TO_MACF(sta->addr),
					(unsigned long)(sta->active));
				if (now - sta->active > info->sta_timeout) {
					next = sta->next;
					BSD_TO("free(to) sta[%p]:"MACF" now[%lu] active[%lu]\n",
						sta, ETHER_TO_MACF(sta->addr),
						(unsigned long)now,
						(unsigned long)(sta->active));

					free(sta);
					sta = next;

					if (prev)
						prev->next = sta;

					continue;
				}

				if (head == NULL)
					head = sta;

				prev = sta;
				sta = sta->next;
			}
			bssinfo->assoclist = head;
		}
	}
	BSD_EXIT();
}

/* search probe list */
bsd_maclist_t *bsd_prbsta_by_addr(bsd_info_t *info, struct ether_addr *addr, bool enable)
{
	int idx;
	bsd_maclist_t *sta;

	BSD_PROB("Enter...\n");

	idx = BSD_MAC_HASH(*addr);
	sta = info->prbsta[idx];

	while (sta) {
		BSD_PROB("cmp: sta[%p]:"MACF" states:0x%x 0x%x 0x%x\n",
			sta, ETHER_TO_MACF(sta->addr),
			sta->states[0], sta->states[1], sta->states[2]);

		if (eacmp(&(sta->addr), addr) == 0) {
			break;
		}
		sta = sta->next;
	}

	if (!sta && enable) {
		sta = malloc(sizeof(bsd_maclist_t));
		if (!sta) {
			BSD_PROB("%s@%d: Malloc failure\n", __FUNCTION__, __LINE__);
			return NULL;
		}

		memset(sta, 0, sizeof(bsd_maclist_t));
		memcpy(&sta->addr, addr, sizeof(struct ether_addr));

		sta->active = sta->timestamp = time(NULL);

		sta->next = info->prbsta[idx];
		info->prbsta[idx] = sta;

		BSD_PROB("sta[%p]:"MACF"\n", sta, ETHERP_TO_MACF(addr));
	}

	BSD_PROB("Exit...\n");
	return sta;
}

/* add probe STA from list */
void bsd_add_prbsta(bsd_info_t *info, char *ifname, uint8 remote, struct ether_addr *addr)
{
	bsd_bssinfo_t *bssinfo;
	bsd_maclist_t *sta;

	BSD_PROB("Entering...\n");

	bssinfo = bsd_bssinfo_by_ifname(info, ifname, remote);

	if (!bssinfo) {
		BSD_PROB("%s: not found ifname:%s\n", __FUNCTION__, ifname);
		return;
	}

	sta = bsd_prbsta_by_addr(info, addr, TRUE);
	if (sta) {
		sta->band |= bssinfo->intf_info->band;

		bsd_update_sta_state_transition(info, bssinfo->intf_info,
			addr, BSD_STA_STATE_PROBE);
	}

	BSD_PROB("Exit...\n");
}

/* timeout probe-list */
void bsd_timeout_prbsta(bsd_info_t *info)
{
	int idx;
	bsd_maclist_t *sta, *next, *head, *prev;
	time_t now = time(NULL);

	BSD_ENTER();
	BSD_INFO("now[%lu]\n", (unsigned long)now);

	for (idx = 0; idx < BSD_PROBE_STA_HASH; idx++) {
		sta = info->prbsta[idx];
		head = NULL;
		prev = NULL;

		while (sta) {
			BSD_INFO("sta[%p]:"MACF" timestamp=%lu active=%lu\n",
				sta, ETHER_TO_MACF(sta->addr),
				(unsigned long)(sta->timestamp),
				(unsigned long)(sta->active));
			if (now - sta->active > info->probe_timeout) {
				next = sta->next;
				BSD_TO("sta[%p]:"MACF"now[%lu] timestamp[%lu]\n",
					sta, ETHER_TO_MACF(sta->addr),
					(unsigned long)now,
					(unsigned long)(sta->active));

				free(sta);
				sta = next;

				if (prev)
					prev->next = sta;

				continue;
			}

			if (head == NULL)
				head = sta;

			prev = sta;
			sta = sta->next;
		}
		info->prbsta[idx] = head;
	}

	BSD_EXIT();
}

/* check if a sta is being associated on any of interfaces */
static int bsd_check_sta_assoc_any(bsd_info_t *info, struct ether_addr *addr)
{
	uint8 remote;
	int if_cnt = 0;
	char bsd_ifnames[80] = "";
	char var_intf[BSD_IFNAME_SIZE];
	char *next_intf;
	bsd_bssinfo_t *bssinfo;

	BSDSTRNCPY(bsd_ifnames, nvram_safe_get(BSD_IFNAMES_NVRAM), sizeof(bsd_ifnames) - 1);

	foreach(var_intf, bsd_ifnames, next_intf) {
		if (if_cnt++ == info->max_ifnum) {
			BSD_ERROR("Wrong bsd_ifnames format!\n");
			break;
		}

		if ((info->role == BSD_ROLE_PRIMARY) && !strncmp(var_intf, "rpc:", 4))
			remote = 1;
		else
			remote = 0;

		bssinfo = bsd_bssinfo_by_ifname(info, var_intf, remote);

		if (bssinfo && bssinfo->valid && bssinfo->intf_info &&
			(bssinfo->intf_info->enabled == TRUE) &&
			BSD_BSS_BSD_ENABLED(bssinfo)) {
			if (bsd_add_assoclist(bssinfo, addr, FALSE) != NULL) {
				BSD_INFO("STA "MACF" found associated on %s\n",
					ETHERP_TO_MACF(addr), var_intf);
				return TRUE;
			}
		}
		else {
			BSD_ERROR("Wrong info for %s\n", var_intf);
		}
	}

	return FALSE;
}

/* timeout maclist  */
void bsd_timeout_maclist(bsd_info_t *info)
{
	bsd_intf_info_t *intf_info;
	bsd_bssinfo_t *bssinfo;
	int idx, bssidx;
	bsd_maclist_t *sta, *head, *prev, *next;
	time_t now = time(NULL);

	BSD_ENTER();

	for (idx = 0; idx < info->max_ifnum; idx++) {
		intf_info = &(info->intf_info[idx]);
		BSD_INFO("idx=%d\n", idx);

		for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
			bssinfo = &(intf_info->bsd_bssinfo[bssidx]);
			if (!(bssinfo->valid) || !BSD_BSS_BSD_ENABLED(bssinfo))
				continue;
			if (bssinfo->steerflag & BSD_BSSCFG_NOTSTEER)
				continue;

			BSD_INFO("maclist[bssidx:%d] ifname:%s...\n", bssidx, bssinfo->ifnames);

			sta = bssinfo->maclist;
			head = NULL;
			prev = NULL;

			while (sta) {
				BSD_INFO("sta[%p]:"MACF" timestamp=%lu\n"
					"steer_state=%d now=%lu active=%lu\n",
					sta, ETHER_TO_MACF(sta->addr),
					(unsigned long)(sta->timestamp),
					sta->steer_state, now, sta->active);
				if (now - sta->timestamp > info->maclist_timeout) {
					int remove = 1;
					if (sta->steer_state != BSD_STA_STEER_SUCC) {
						/* check if the steering failed before remove it */
						if (bsd_check_steer_fail(info, sta, bssinfo) ==
							BSD_STEERING_RESULT_SUCC) {
							sta->steer_state = BSD_STA_STEER_SUCC;
							remove = 0;
						}
					}
					else {
						int assoc;
						assoc = bsd_check_sta_assoc_any(info, &(sta->addr));
						if ((assoc == TRUE) || (sta->active == 0)) {
							/* sta is associated on some interface */
							sta->active = now;
							remove = 0;
						}
						else {
							if (now - sta->active <
								BSD_STA_TIMEOUT_AFTER_SUCC) {
								remove = 0;
							}
							else {
							BSD_INFO("sta[%p] "MACF" disappeared, "
								"remove it from maclist\n",
								 sta, ETHER_TO_MACF(sta->addr));
							}
						}
					}

					if (remove) {
						next = sta->next;
						BSD_INFO("sta[%p]:"MACF"now[%lu] timestamp[%lu]\n",
							sta, ETHER_TO_MACF(sta->addr),
							(unsigned long)now,
							(unsigned long)(sta->timestamp));

						/* Unblock mac before removing sta. */
						wl_wlif_unblock_mac(bssinfo->wlif_hdl,
							sta->addr, 0);

						free(sta);
						sta = next;

						if (prev)
							prev->next = sta;

						continue;
					}
				}
				if (head == NULL)
					head = sta;

				prev = sta;
				sta = sta->next;
			}
			bssinfo->maclist = head;
		}
	}

	BSD_EXIT();
}

void bsd_update_cca_stats(bsd_info_t *info)
{
	bsd_intf_info_t *intf_info;
	bsd_bssinfo_t *bssinfo;
	int ifidx;
	uint8 idx;
	int bssidx;

	int ret = 0;
	wl_chanim_stats_t *list;
	wl_chanim_stats_t param;
	int buflen = BSD_IOCTL_MAXLEN;

	char *ptr;
	int tlen;

	BSD_CCAENTER();

	for (ifidx = 0; ifidx < info->max_ifnum; ifidx++) {
		intf_info = &(info->intf_info[ifidx]);
		if (intf_info->enabled != TRUE) {
			BSD_INFO("Skip: ifidx %d is not enabled\n", ifidx);
			continue;
		}

		bssidx =  bsd_get_steerable_bss(info, intf_info);
		if (bssidx  == -1) {
			continue;
		}

		bssinfo = &(intf_info->bsd_bssinfo[bssidx]);

		BSD_CCA("tick[%d] ifidx[%d] idx[%d] intf_info[%p] bssinfo[%p]"
			" [%s] period[%d] cnt[%d]\n",
			intf_info->chan_util.ticks, ifidx, intf_info->chan_util.idx,
			intf_info, bssinfo, bssinfo->ifnames,
			intf_info->steering_cfg.period, intf_info->steering_cfg.cnt);

		intf_info->chan_util.ticks += 1;

		if (intf_info->steering_cfg.period) {
			intf_info->chan_util.ticks %= intf_info->steering_cfg.period;
			if (intf_info->chan_util.ticks != 0)
				continue;
		}

		intf_info->chan_util.ticks = 0;

		BSD_CCA("Read cca stats from %s[%d][%d]\n", bssinfo->ifnames,
			intf_info->idx, bssinfo->idx);

		list = (wl_chanim_stats_t *) ioctl_buf;

		memset(&param, 0, sizeof(param));
		param.buflen = buflen;
		param.count = WL_CHANIM_COUNT_ONE;
		param.version = WL_CHANIM_STATS_VERSION;

		memset(ioctl_buf, 0, sizeof(ioctl_buf));
		strcpy(ioctl_buf, "chanim_stats");
		tlen = strlen(ioctl_buf) + 1;
		ptr = (char *)(ioctl_buf + tlen);
		memcpy(ptr, &param, sizeof(wl_chanim_stats_t));

		BSD_RPC("---RPC name:%s cmd: %d(WLC_GET_VAR: chanim_stats) tlen=%d\n",
			bssinfo->ifnames, WLC_GET_VAR, tlen);
		bsd_rpc_dump((char *)ioctl_buf, 64, BSD_RPC_ENAB);

		ret = bsd_wl_ioctl(bssinfo, WLC_GET_VAR, ioctl_buf,
			sizeof(ioctl_buf)-BSD_RPC_HEADER_LEN);

		if (ret < 0) {
			BSD_ERROR("Err: intf:%s chanim_stats\n", bssinfo->ifnames);
			continue;
		}

		BSD_CCA("ret:%d buflen: %d, version: %d count: %d\n",
			ret, list->buflen, list->version, list->count);

		if (!((list->version == WL_CHANIM_STATS_VERSION) ||
			(list->version == WL_CHANIM_STATS_V2))) {
			BSD_ERROR("Err: chanim_stats version %d doesn't match %d nor %d\n",
				list->version, WL_CHANIM_STATS_VERSION, WL_CHANIM_STATS_V2);
			BSD_CCAEXIT();
			return;
		}
		if (list->version == WL_CHANIM_STATS_V2) {
			chanim_stats_v2_t *statsv2 = (chanim_stats_v2_t *)list->stats;

			BSD_CCA_PLAIN("chanspec   tx   inbss   obss   nocat   nopkt"
					"   doze     txop     goodtx  badtx   glitch   "
					"badplcp  knoise  timestamp   idle\n");
			BSD_CCA_PLAIN("0x%4x\t", statsv2->chanspec);

			for (idx = 0; idx < CCASTATS_MAX; idx++)
				BSD_CCA_PLAIN("%d\t", statsv2->ccastats[idx]);
				BSD_CCA_PLAIN("%d\t%d\t%d\t%d\t%d\n", statsv2->glitchcnt,
					statsv2->badplcp, statsv2->bgnoise, statsv2->timestamp,
					statsv2->chan_idle);

			idx = intf_info->chan_util.idx;
			memcpy(&(intf_info->chan_util.rec[idx].stats), list->stats,
				sizeof(chanim_stats_v2_t));
		}
		else if (list->version == WL_CHANIM_STATS_VERSION) {
			chanim_stats_t *statsv3 = (chanim_stats_t *)list->stats;

			BSD_CCA_PLAIN("chanspec   tx   inbss   obss   nocat   nopkt"
					"   doze     txop     goodtx  badtx   glitch   "
					"badplcp  knoise  timestamp   idle\n");
			BSD_CCA_PLAIN("0x%4x\t", statsv3->chanspec);

			for (idx = 0; idx < CCASTATS_MAX; idx++)
				BSD_CCA_PLAIN("%d\t", statsv3->ccastats[idx]);
				BSD_CCA_PLAIN("%d\t%d\t%d\t%d\t%d\n", statsv3->glitchcnt,
					statsv3->badplcp, statsv3->bgnoise, statsv3->timestamp,
					statsv3->chan_idle);

			idx = intf_info->chan_util.idx;
			memcpy(&(intf_info->chan_util.rec[idx].stats), list->stats,
				sizeof(chanim_stats_t));
		}
		intf_info->chan_util.rec[idx].valid = 1;
		intf_info->chan_util.chanim_version = list->version;
		intf_info->chan_util.idx =
			MODINC((intf_info->chan_util.idx), BSD_CHANIM_STATS_MAX);
	}

	BSD_CCAEXIT();
	return;
}

/* chan busy detection: may need to be a generic algo */
void bsd_reset_chan_busy(bsd_info_t *info, int ifidx)
{
	bsd_intf_info_t *intf_info;
	int bssidx;
	bsd_bssinfo_t *bssinfo;
	uint8 idx, cnt, num;

	bsd_steering_policy_t *steering_cfg;
	bsd_chanim_stats_t *rec;

	BSD_CCAENTER();

	intf_info = &(info->intf_info[ifidx]);
	bssidx = bsd_get_steerable_bss(info, intf_info);
	if (bssidx == -1) {
		return;
	}
	bssinfo = &(intf_info->bsd_bssinfo[bssidx]);

	steering_cfg = &intf_info->steering_cfg;
	cnt = steering_cfg->cnt;

	idx = MODDEC(intf_info->chan_util.idx, BSD_CHANIM_STATS_MAX);
	BSD_CCA("invalid ccs: ifname:%s[remote:%d] rec[%d] for %d\n",
		bssinfo->ifnames, intf_info->remote, idx, cnt);
	if (intf_info->chan_util.chanim_version == WL_CHANIM_STATS_V2) {
		chanim_stats_v2_t *statsv2;
		for (num = 0; num < cnt; num++) {
			rec = &(intf_info->chan_util.rec[idx]);
			statsv2 = (chanim_stats_v2_t *)&rec->stats;
			rec->valid = 0;
			idx = MODINC(idx, BSD_CHANIM_STATS_MAX);
			BSD_CCA("invalid: rec[%d] idle[%d]\n", idx,
				statsv2->ccastats[CCASTATS_TXOP]);
		}
	}
	else if (intf_info->chan_util.chanim_version == WL_CHANIM_STATS_VERSION) {
		chanim_stats_t *statsv3;
		for (num = 0; num < cnt; num++) {
			rec = &(intf_info->chan_util.rec[idx]);
			statsv3 = (chanim_stats_t *)&rec->stats;
			rec->valid = 0;
			idx = MODINC(idx, BSD_CHANIM_STATS_MAX);
			BSD_CCA("invalid: rec[%d] idle[%d]\n", idx,
				statsv3->ccastats[CCASTATS_TXOP]);
		}
	}
	BSD_CCAEXIT();
	return;
}

void dump_if_bssinfo_list(bsd_bssinfo_t *bssinfo)
{
	bsd_if_bssinfo_list_t* list;

	BSD_ENTER();

	if (bssinfo && bssinfo->intf_info) {
		BSD_PRINT("ifname=%s, ifidx=%d\n", bssinfo->ifnames, bssinfo->intf_info->idx);
	} else {
		BSD_EXIT();
		return;
	}

	list = bssinfo->to_if_bss_list;
	while (list) {
		BSD_PRINT("ifidx=%d bssidx=%d to_ifidx=%d ifnames=%s, prefix=%s\n",
			list->bssinfo->intf_info->idx, list->bssinfo->idx,
			list->to_ifidx, list->bssinfo->ifnames, list->bssinfo->prefix);
		list = list->next;
	}

	BSD_EXIT();
}

void clean_if_bssinfo_list(bsd_if_bssinfo_list_t* list)
{
	while (list) {
		bsd_if_bssinfo_list_t *curr = list;
		list = list->next;
		free(curr);
	}
}

/*
 * STA Steering Bouncing Prevention is globally controlled by
 * nvram var bsd_bounce_detect=<window time in sec> <cnts> <dwell time in sec>
 *
 * to prevent from moving STAs in a ping-pong, or cascading steering scenario
 * If a STA is steer'd <cnts> times within <window time> seconds,
 * the BSD engine will keep the STA in current interface, and not steer it for
 * a period of <dwell time> seconds
 *
 * The bouncing STA detection impl. is based on STA's bounce state transition
 * The periodic watchdog timer events, and steering STA event move the STA's
 * bounce state among following 4 states:
 *		BSD_BOUNCE_INIT
 *		BSD_BOUNCE_WINDOW_STATE
 *		BSD_BOUNCE_DWELL_STATE
 *		BSD_BOUNCE_CLEAN_STATE
 *
 *  The STA is declared as a bouncing STA if it is in BSD_BOUNCE_DWELL_STATE
 *
 */
void bsd_update_sta_bounce_table(bsd_info_t *info)
{
	int key;
	time_t now = time(NULL);
	time_t gap;
	bsd_sta_bounce_detect_t *entry;

	BSD_ENTER();

	for (key = 0; key < BSD_BOUNCE_HASH_SIZE; key++) {
		entry = info->sta_bounce_table[key];

		while (entry) {
			gap = now - entry->timestamp;
			BSD_BOUNCE("entry[%p]:"MACF" state:%d timestamp:%lu now:%lu [%lu]\n",
				entry, ETHER_TO_MACF(entry->addr),
				entry->state,
				(unsigned long)(entry->timestamp),
				(unsigned long)(now),
				gap);

			/* state transition */
			switch (entry->state) {
				case BSD_BOUNCE_INIT:
					if (entry->timestamp == 0) {
						break;
					}
					entry->run.window = now - entry->timestamp;
					entry->state = BSD_BOUNCE_WINDOW_STATE;
					break;
				case BSD_BOUNCE_WINDOW_STATE:
					entry->run.window = gap;
					if (entry->run.cnt >= info->bounce_cfg.cnt) {
						entry->timestamp = now;
						entry->state = BSD_BOUNCE_DWELL_STATE;
					}
					else if (entry->run.window >= info->bounce_cfg.window) {
						/* move to */
						entry->state = BSD_BOUNCE_CLEAN_STATE;
					}
					break;
				case BSD_BOUNCE_DWELL_STATE:
					entry->run.dwell_time = gap;
					if (entry->run.dwell_time > info->bounce_cfg.dwell_time) {
						entry->state = BSD_BOUNCE_CLEAN_STATE;
					}
					break;
				case BSD_BOUNCE_CLEAN_STATE:
					memset(&entry->run, 0, sizeof(bsd_bounce_detect_t));
					entry->timestamp = 0;
					entry->state = BSD_BOUNCE_INIT;
					break;
			}

			BSD_BOUNCE("update bounce entry[%p]:"MACF" state:%d timestamp=%lu\n",
				entry, ETHER_TO_MACF(entry->addr),
				entry->state,
				(unsigned long)(entry->timestamp));

			if (!info->steer_no_deauth || !info->non11v_cfg.cnt) {
				goto next;
			}

			gap = now - entry->non11v_timestamp;
			/* state transition for non11v sta */
			switch (entry->non11v_state) {
				case BSD_BOUNCE_INIT:
					if (entry->non11v_timestamp == 0) {
						goto next;
					}
					entry->non11v_run.window = now - entry->non11v_timestamp;
					entry->non11v_state = BSD_BOUNCE_WINDOW_STATE;
					break;
				case BSD_BOUNCE_WINDOW_STATE:
					entry->non11v_run.window = gap;
					if (entry->non11v_run.cnt >= info->non11v_cfg.cnt) {
						entry->non11v_timestamp = now;
						entry->non11v_state = BSD_BOUNCE_DWELL_STATE;
					}
					else if (entry->non11v_run.window >=
						info->non11v_cfg.window) {
						/* move to */
						entry->non11v_state = BSD_BOUNCE_CLEAN_STATE;
					}
					break;
				case BSD_BOUNCE_DWELL_STATE:
					entry->non11v_run.dwell_time = gap;
					if (entry->non11v_run.dwell_time >
						info->non11v_cfg.dwell_time) {
						entry->non11v_state = BSD_BOUNCE_CLEAN_STATE;
					}
					break;
				case BSD_BOUNCE_CLEAN_STATE:
					memset(&entry->non11v_run, 0, sizeof(bsd_bounce_detect_t));
					entry->non11v_timestamp = now;
					entry->non11v_state = BSD_BOUNCE_INIT;
					break;
			}

			BSD_STEER("update non11v entry [%p]:"MACF" state:%d timestamp=%lu\n",
				entry, ETHER_TO_MACF(entry->addr),
				entry->non11v_state,
				(unsigned long)(entry->non11v_timestamp));

next:
			entry = entry->next;
		}
	}

	BSD_EXIT();
}

void bsd_add_sta_to_bounce_table(bsd_info_t *info, struct ether_addr *addr)
{
	int hash_key;
	bsd_sta_bounce_detect_t *entry;
	time_t now = time(NULL);

	BSD_ENTER();

	hash_key = BSD_BOUNCE_MAC_HASH(*addr);
	entry = info->sta_bounce_table[hash_key];

	BSD_BOUNCE("hash_key:%d, entry:%p, addr:%p\n", hash_key, entry, addr);

	while (entry) {
		BSD_BOUNCE("cmp: mac addr:"MACF"\n", ETHERP_TO_MACF(addr));
		if (eacmp(&(entry->addr), addr) == 0) {
			break;
		}
		entry = entry->next;
	}

	BSD_BOUNCE("entry[%p]\n", entry);

	if (!entry) {
		entry = malloc(sizeof(bsd_sta_bounce_detect_t));
		if (!entry) {
			BSD_ERROR("%s@%d: Malloc failure\n", __FUNCTION__, __LINE__);
			return;
		}

		memset(entry, 0, sizeof(bsd_sta_bounce_detect_t));
		memcpy(&entry->addr, addr, sizeof(struct ether_addr));
		entry->next = info->sta_bounce_table[hash_key];
		info->sta_bounce_table[hash_key] = entry;
	}

	if (entry->timestamp == 0) {
		entry->state = BSD_BOUNCE_INIT;
		entry->timestamp = now;
	}

	if (entry->non11v_timestamp == 0) {
		entry->non11v_state = BSD_BOUNCE_INIT;
		entry->non11v_timestamp = now;
	}

	entry->run.cnt++;

	BSD_BOUNCE("entry[%p] window:%d, cnt:%d dwell_time:%d\n",
		entry, entry->run.window, entry->run.cnt, entry->run.dwell_time);

	BSD_EXIT();
}

/* check the same STA if associating to this AP */
bool bsd_check_bouncing_sta(bsd_info_t *info, struct ether_addr *addr)
{
	int hash_key;
	bsd_sta_bounce_detect_t *entry;

	BSD_ENTER();

	if (!addr) {
		return FALSE;
	}

	hash_key = BSD_BOUNCE_MAC_HASH(*addr);
	entry = info->sta_bounce_table[hash_key];

	BSD_BOUNCE("entry[%p]\n", entry);

	while (entry) {
		BSD_BOUNCE("cmp: mac addr:"MACF"\n", ETHERP_TO_MACF(addr));
		if (eacmp(&(entry->addr), addr) == 0) {
			break;
		}
		entry = entry->next;
	}

	if (entry) {
		BSD_BOUNCE("entry[%p] state:%d window:%d, cnt:%d dwell_time:%d\n",
			entry, entry->state,
			entry->run.window, entry->run.cnt, entry->run.dwell_time);

		if (entry->state == BSD_BOUNCE_DWELL_STATE) {
			BSD_BOUNCE("bouncing detected, sta:"MACF"\n", ETHERP_TO_MACF(addr));
			return TRUE;
		}
	}

	BSD_EXIT();

	return FALSE;
}

void bsd_cleanup_sta_bounce_table(bsd_info_t *info)
{
	int key;
	bsd_sta_bounce_detect_t *entry, *tmp;

	BSD_ENTER();

	for (key = 0; key < BSD_BOUNCE_HASH_SIZE; key++) {
		entry = info->sta_bounce_table[key];

		while (entry) {
			BSD_BOUNCE("free entry[%p]:"MACF"\n", entry, ETHER_TO_MACF(entry->addr));
			tmp = entry;
			entry = entry->next;

			free(tmp);
		}
	}
	BSD_EXIT();
}

/* check if STA has not responded to BSS Tansition request */
bool bsd_check_non11v_sta(bsd_info_t *info, struct ether_addr *addr)
{
	int hash_key;
	bsd_sta_bounce_detect_t *entry;

	BSD_ENTER();

	if (!addr) {
		return FALSE;
	}

	hash_key = BSD_BOUNCE_MAC_HASH(*addr);
	entry = info->sta_bounce_table[hash_key];

	BSD_BOUNCE("entry[%p]\n", entry);

	while (entry) {
		BSD_BOUNCE("cmp: mac addr:"MACF"\n", ETHERP_TO_MACF(addr));
		if (eacmp(&(entry->addr), addr) == 0) {
			break;
		}
		entry = entry->next;
	}

	if (entry) {
		BSD_BOUNCE("entry[%p] state:%d window:%d, cnt:%d dwell_time:%d\n",
			entry, entry->non11v_state,
			entry->non11v_run.window, entry->non11v_run.cnt,
			entry->non11v_run.dwell_time);

		if (entry->non11v_state != BSD_BOUNCE_WINDOW_STATE) {
			BSD_BOUNCE("non 11v sta detected,sta:"MACF"\n", ETHERP_TO_MACF(addr));
			return TRUE;
		}
	}

	BSD_EXIT();

	return FALSE;
}

void bsd_update_sta_state_transition(bsd_info_t *info,
	bsd_intf_info_t *intf_info, struct ether_addr *addr, uint32 state)
{
	int ifidx;
	bsd_maclist_t *sta;

	ifidx = intf_info->idx;
	BSD_PROB("Enter... ifidx:%d "MACF" state:0x%x\n",
		ifidx, ETHERP_TO_MACF(addr),  state);

	sta = bsd_prbsta_by_addr(info, addr, FALSE);
	if (sta) {
		int enable = 1;
		if (state == BSD_STA_STATE_STEER_FAIL) {
			/* sta->steer_state in prbsta is used as steer failure counter */
			/* consider as picky sta if steering failed multiple times */
			if (sta->steer_state >= BSD_STA_STEER_FAIL_PICKY_CNT) {
				BSD_INFO("STA "MACF" Steering failed %d times on ifidx [%d]\n",
					ETHERP_TO_MACF(addr), sta->steer_state, ifidx);
			}
			else {
				sta->steer_state++;
				enable = 0;
			}
		}

		if (enable)
			sta->states[ifidx] |= state;

		BSD_PROB("Exit with states:0x%x 0x%x 0x%x...\n",
			sta->states[0], sta->states[1], sta->states[2]);
	} else {
		BSD_PROB("STA not in\n");
	}
}

bool bsd_check_picky_sta(bsd_info_t *info, struct ether_addr *addr)
{
	bool ret = FALSE;
	bsd_maclist_t *sta;
	uint32 states = 0;

	sta = bsd_prbsta_by_addr(info, addr, FALSE);
	if (sta) {
		int i;

		for (i = 0; i < info->max_ifnum; i++) {
			states |= sta->states[i];
		}

		if ((((states & BSD_STA_STATE_PROBE) == 0) &&
			((states & BSD_STA_STATE_AUTH) != 0)) ||
			(states & BSD_STA_STATE_STEER_FAIL)) {
			/* no need to check timestamp */
			ret = TRUE;
		}
	}

	return (ret);
}

/* The AP's STA detected RF channels based on STA's probe/auth/assoc/reassoc
 * frames on wl intf
 */
bool bsd_qualify_sta_rf(bsd_info_t *info,
	bsd_bssinfo_t *bss, bsd_bssinfo_t *to_bss, struct ether_addr *addr)
{
	bool ret = FALSE;
	bsd_maclist_t *sta;

	sta = bsd_prbsta_by_addr(info, addr, FALSE);
	if (sta &&
		((sta->states[to_bss->intf_info->idx])) != 0) {
		/* no need to check timestamp */
		BSD_PROB("good chan sta:"MACF"\n", ETHERP_TO_MACF(addr));
		ret = TRUE;
	}

	return (ret);
}

/* when sta asociated on to_bssinfo, check maclist on src interface */
void bsd_check_steer_result(bsd_info_t *info, struct ether_addr *addr, bsd_bssinfo_t *to_bssinfo)
{
	bsd_maclist_t *sta;
	bsd_bssinfo_t *bssinfo;
	bsd_intf_info_t *intf_info;
	int intfidx, bssidx;

	if (to_bssinfo == NULL)
		return;

	BSD_INFO("Check steer result after sta "MACF" on %s\n",
		ETHERP_TO_MACF(addr), to_bssinfo->ifnames);

	for (intfidx = 0; intfidx < info->max_ifnum; intfidx++) {
		intf_info = &(info->intf_info[intfidx]);

		for (bssidx = 0; bssidx < WL_MAXBSSCFG; bssidx++) {
			bssinfo = &intf_info->bsd_bssinfo[bssidx];
			if (!(bssinfo->valid) ||
				(bssinfo->steerflag & BSD_BSSCFG_NOTSTEER) ||
				(bssinfo == to_bssinfo) ||
				!BSD_BSS_BSD_OR_WBD_ENABLED(bssinfo))
					continue;

			sta = bssinfo->maclist;
			while (sta) {
				BSD_INFO("sta[%p]:"MACF" timestamp=%lu\n",
					sta, ETHER_TO_MACF(sta->addr),
					(unsigned long)(sta->timestamp));

				if ((eacmp(&(sta->addr), addr) == 0) &&
					(sta->steer_state == BSD_STA_STEERING) &&
					(sta->to_bssinfo == to_bssinfo)) {
					BSD_INFO("sta "MACF" is associated to target %s.\n",
						ETHER_TO_MACF(sta->addr), sta->to_bssinfo->ifnames);
					bsd_update_steering_record(info, &(sta->addr), bssinfo,
						sta->to_bssinfo, BSD_STEERING_RESULT_SUCC);
					sta->steer_state = BSD_STA_STEER_SUCC;

					bsd_steer_appevent(info, &(sta->addr),
						APP_E_BSD_STEER_END, APP_E_BSD_STATUS_STEER_SUCC);
					return;
				}

				sta = sta->next;
			}
		}
	}
}

int bsd_check_steer_fail(bsd_info_t *info, bsd_maclist_t *sta, bsd_bssinfo_t *bssinfo)
{
	bsd_sta_info_t *sta_assoc = NULL;
	uint32 result = BSD_STEERING_RESULT_FAIL;

	if ((sta->steer_state == BSD_STA_STEERING) && (sta->to_bssinfo)) {
		int status; /* for appevent */

		if ((sta_assoc = bsd_add_assoclist(sta->to_bssinfo, &(sta->addr), FALSE))) {
			result = BSD_STEERING_RESULT_SUCC;
			status = APP_E_BSD_STATUS_STEER_SUCC;
		}
		else {
			result = BSD_STEERING_RESULT_FAIL;
			/* add the sta to picky list */
			bsd_update_sta_state_transition(info, bssinfo->intf_info, &(sta->addr),
				BSD_STA_STATE_STEER_FAIL);
			status = APP_E_BSD_STATUS_STEER_FAIL;
			bsd_update_sta_stats(info, &(sta->addr), NULL, NULL, 1, 0, 0, 0, 0, 0);
		}

		BSD_INFO("sta "MACF" %s target %s (timeout)\n", ETHER_TO_MACF(sta->addr),
			sta_assoc?"on":"not on", sta->to_bssinfo->ifnames);

		bsd_update_steering_record(info, &(sta->addr), bssinfo, sta->to_bssinfo, result);
		bsd_steer_appevent(info, &(sta->addr), APP_E_BSD_STEER_END, status);
	}
	return result;
}

/* check whether channel is passive */
bool bsd_is_chan_passive(bsd_bssinfo_t *bssinfo)
{
	uint32 bitmap = 0, buflen;
	uint32 chanspec_arg;
	int ret;
	char *param;
	uint8 sub_channel;

	FOREACH_20_SB(bssinfo->chanspec, sub_channel) {
		memset(ioctl_buf, 0, sizeof(ioctl_buf));
		strncpy(ioctl_buf, "per_chan_info", strlen("per_chan_info"));

		buflen = strlen(ioctl_buf) + 1;
		param = (char *)(ioctl_buf + buflen);

		chanspec_arg = CH20MHZ_CHSPEC(sub_channel);
		memcpy(param, &chanspec_arg, sizeof(chanspec_arg));

		ret = bsd_wl_ioctl(bssinfo, WLC_GET_VAR, ioctl_buf,
			sizeof(ioctl_buf) - BSD_RPC_HEADER_LEN);
		if (ret < 0) {
			return TRUE;
		}

		bitmap = dtoh32(*(uint32 *)ioctl_buf);

		if (bitmap & WL_CHAN_PASSIVE) {
			BSD_STEER("channel %d passive Dont steer to it \n", chanspec_arg);
			return TRUE;
		}
	}
	return FALSE;
}

/* Check if FBT is Enabled on this BSS */
static bool
bsd_is_fbt_enabled(bsd_bssinfo_t *bssinfo)
{
	int ret = BSD_OK, data_len = sizeof(ioctl_buf) - BSD_RPC_HEADER_LEN;
	uint32 fbt = 0;

	BSD_RPC("RPC name:%s cmd: %d(WLC_GET_VAR for fbt)\n",
		bssinfo->ifnames, WLC_GET_VAR);
	memset(ioctl_buf, 0, sizeof(ioctl_buf));
	strcpy(ioctl_buf, "fbt");

	wl_endian_probe(bssinfo->ifnames);

	/* Try getting FBT IOVAR value */
	ret = bsd_wl_ioctl(bssinfo, WLC_GET_VAR, (void *)ioctl_buf, data_len);
	if (ret < 0) {
		BSD_ERROR("Err:%d ifname[%s] WLC_GET_VAR fbt\n",
			ret, bssinfo->ifnames);
		return FALSE;
	}

	fbt = dtoh32(*(uint32 *)ioctl_buf);
	BSD_INFO("IF[%s] FBT[%d]\n", bssinfo->ifnames, fbt);

	return ((fbt > 0) ? TRUE : FALSE);
}

/* To get the BSS information of BSS */
static int
bsd_get_bss_info(bsd_bssinfo_t *bssinfo, wl_bss_info_t **out_bss_info)
{
	int ret = BSD_OK, data_len = sizeof(ioctl_buf) - BSD_RPC_HEADER_LEN;
	wl_bss_info_t *bi;

	BSD_RPC("RPC name:%s cmd: %d(WLC_GET_BSS_INFO)\n",
		bssinfo->ifnames, WLC_GET_BSS_INFO);
	memset(ioctl_buf, 0, sizeof(ioctl_buf));
	*(uint32*)ioctl_buf = htod32(data_len);

	wl_endian_probe(bssinfo->ifnames);

	/* Try getting BSS Info */
	ret = bsd_wl_ioctl(bssinfo, WLC_GET_BSS_INFO, (void *)ioctl_buf, data_len);
	if (ret < 0) {
		BSD_ERROR("Err:%d ifname[%s] WLC_GET_BSS_INFO\n",
			ret, bssinfo->ifnames);
		return BSD_FAIL;
	}

	bi = (wl_bss_info_t*)(ioctl_buf + sizeof(uint32));

	if (out_bss_info) {
		*out_bss_info = bi;
	}

	return BSD_OK;
}

/* To get the phytype of BSS */
static int
bsd_get_phy_type(bsd_bssinfo_t *bssinfo, wl_bss_info_t *in_bi, uint8 *out_phy_type)
{
	int ret = BSD_FAIL;
	wl_bss_info_t *bi;

	/* Validate arg */
	if (!out_phy_type) {
		goto end;
	}

	/* If bss_info is not passed extract it, else use the given bss_info */
	if (!in_bi) {
		/* Extract BSS Info for this Interface */
		ret = bsd_get_bss_info(bssinfo, &bi);
		if (ret != BSD_OK) {
			goto end;
		}
	} else {
		bi = in_bi;
	}

	if (bi->vht_cap) {
		*out_phy_type = (uint8) BSD_DOT11_PHYTYPE_VHT;
	} else if (bi->n_cap) {
		*out_phy_type = (uint8) BSD_DOT11_PHYTYPE_HT;
	} else {
		*out_phy_type = 0;
	}

	BSD_INFO("IF[%s] out_phy_type[0x%x]\n", bssinfo->ifnames, *out_phy_type);
	ret = BSD_OK;

end:
	return ret;
}

/* To get the bssid_info of BSS */
static int
bsd_get_bssid_info_field(bsd_bssinfo_t *bssinfo, wl_bss_info_t *in_bi, uint32 *out_bssid_info)
{
	int ret = BSD_FAIL;
	wl_bss_info_t *bi;

	/* Validate arg */
	if (!out_bssid_info) {
		goto end;
	}

	/* If bss_info is not passed extract it, else use the given bss_info */
	if (!in_bi) {
		/* Extract BSS Info for this Interface */
		ret = bsd_get_bss_info(bssinfo, &bi);
		if (ret != BSD_OK) {
			goto end;
		}
	} else {
		bi = in_bi;
	}

	bi->capability = dtoh16(bi->capability);

	*out_bssid_info =  DOT11_NGBR_BI_REACHABILTY;
	*out_bssid_info |= DOT11_NGBR_BI_SEC;
	*out_bssid_info |= DOT11_NGBR_BI_KEY_SCOPE;
	*out_bssid_info |= (bi->capability & DOT11_CAP_QOS) ? DOT11_NGBR_BI_CAP_QOS : 0;
	*out_bssid_info |= (bi->capability & DOT11_CAP_APSD) ? DOT11_NGBR_BI_CAP_APSD : 0;
	*out_bssid_info |= (bi->capability & DOT11_CAP_RRM) ? DOT11_NGBR_BI_CAP_RDIO_MSMT : 0;
	*out_bssid_info |= (bi->capability & DOT11_CAP_DELAY_BA) ? DOT11_NGBR_BI_CAP_DEL_BA : 0;
	*out_bssid_info |= (bi->capability & DOT11_CAP_IMMEDIATE_BA) ? DOT11_NGBR_BI_CAP_IMM_BA : 0;
	*out_bssid_info |= (bi->n_cap) ? DOT11_NGBR_BI_HT : 0;
	*out_bssid_info |= (bi->vht_cap) ? DOT11_NGBR_BI_VHT : 0;

	/* Look for FBT Enabled */
	if (bsd_is_fbt_enabled(bssinfo) > 0) {

		/* If FBT Enabled, Set BSS capability with FBT Capable */
		*out_bssid_info |= DOT11_NGBR_BI_MOBILITY;
	}

	BSD_INFO("IF[%s] out_bssid_info[0x%x]\n", bssinfo->ifnames, *out_bssid_info);
	ret = BSD_OK;

end:
	return ret;
}

/* Update BSS capability for this BSS */
static void
bsd_update_bss_capability(bsd_bssinfo_t *bssinfo)
{
	int ret = BSD_OK;
	wl_bss_info_t *bss_info = NULL;
	uint8 phytype = 0;
	uint32 bssid_info = 0;

	/* Validate arg */
	if (!bssinfo) {
		return;
	}

	/* Get BSS Info of BSS */
	ret = bsd_get_bss_info(bssinfo, &bss_info);
	if (ret != BSD_OK) {
		return;
	}

	/* Get the PHY Type of BSS */
	ret = bsd_get_phy_type(bssinfo, bss_info, &phytype);
	if (ret == BSD_OK) {
		bssinfo->phytype = phytype;
	}

	/* Get the BSSID Information of BSS */
	ret = bsd_get_bssid_info_field(bssinfo, bss_info, &bssid_info);
	if (ret == BSD_OK) {
		bssinfo->bssid_info = bssid_info;
	}

	BSD_INFO("IF[%s] BSSID["MACF"] BSSID_Info[0x%x] PHY_Type[0x%x]\n",
		bssinfo->ifnames, ETHER_TO_MACF(bssinfo->bssid),
		bssinfo->bssid_info, bssinfo->phytype);

	return;
}

/* Check if sta is associated on any other bss */
bsd_sta_info_t* bsd_check_sta_assoc_other_bss(bsd_info_t *info, bsd_bssinfo_t *except_bss,
	struct ether_addr *addr)
{
	uint8 remote;
	int if_cnt = 0;
	char bsd_ifnames[80] = "";
	char var_intf[BSD_IFNAME_SIZE];
	char *next_intf;
	bsd_bssinfo_t *bssinfo;
	bsd_sta_info_t *sta = NULL;

	BSDSTRNCPY(bsd_ifnames, nvram_safe_get(BSD_IFNAMES_NVRAM), sizeof(bsd_ifnames) - 1);

	foreach(var_intf, bsd_ifnames, next_intf) {
		if (if_cnt++ == info->max_ifnum) {
			BSD_ERROR("Wrong bsd_ifnames format!\n");
			break;
		}

		if ((info->role == BSD_ROLE_PRIMARY) && !strncmp(var_intf, "rpc:", 4))
			remote = 1;
		else
			remote = 0;

		bssinfo = bsd_bssinfo_by_ifname(info, var_intf, remote);
		if (bssinfo == except_bss)
			continue;

		if (bssinfo && bssinfo->valid && bssinfo->intf_info &&
			(bssinfo->intf_info->enabled == TRUE) &&
			BSD_BSS_BSD_OR_WBD_ENABLED(bssinfo)) {
			if ((sta = bsd_add_assoclist(bssinfo, addr, FALSE)) != NULL) {
				BSD_INFO("STA "MACF" found associated on %s\n",
					ETHERP_TO_MACF(addr), var_intf);
				return sta;
			}
		}
	}

	return NULL;
}
