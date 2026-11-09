/*
 * Copyright 2023, ASUSTeK Inc.
 * All Rights Reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <linux/rtnetlink.h>

#include <rc.h>
#include <shared.h>
#include <multi_wan.h>

#define MTWANDUCK_SOCKET_PATH  "/var/run/mtwanduck_socket"
#define MTWANDUCK_ALARM_SECS   15

int stop = 0;
int do_mtwanduck_check = 0;
pthread_mutex_t mtwanduck_chkrt, mtwanduck_chkaddr;

extern void  *mtwan_detect_internet(void *arg);

static int phy_status[MAX_MULTI_WAN_NUM] = {0};
static int wan_status[MAX_MULTI_WAN_NUM] = {0};
static int freeze_duck = 10;

static int _do_read(int fd, void* data, size_t len);
static int _do_write(int fd, void* data, size_t len);
static int _get_phy_status(int unit);
static int _get_wan_status(int unit);
static void _init_phy_status(int phy_status[], size_t n);
static void _init_wan_status(int wan_status[], size_t n);

typedef enum {
	MTWANDUCK_WAN_STAT=0,
	MTWANDUCK_PHY_STAT,
	MTWANDUCK_FREEZE,
	MTWANDUCK_UPDATE_PROFILE,
} MTWANDUCK_DATA_TYPE_T;

typedef struct __ptcsrv_sock_data_t_
{
	MTWANDUCK_DATA_TYPE_T d_type;	// Data type
	union {
	int mtwan_unit;
	int count;
	};

} MTWANDUCK_SOCK_DATA_T;

static int _do_write(int fd, void* data, size_t len)
{
	size_t left = len;
	uint8_t *p = data;
	ssize_t n = 0;
	int cnt = 100;

	while (cnt > 0) {
		n = write(fd, p, left);
		if (n < 0) {
			// _dprintf("%d: %s\n", __LINE__, strerror(errno));
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				usleep(10000);
				cnt--;
				continue;
			}
			else {
				perror("write");
				return -1;
			}
		}
		else if (n == left)
			break;
		else {
			left -= n;
			p += n;
		}
	}

	return 0;
}

static int _do_read(int fd, void* data, size_t len)
{
	size_t left = len;
	uint8_t *p = data;
	ssize_t n = 0;
	int cnt = 100;

	while (cnt > 0) {
		n = read(fd, p, left);
		if (n < 0) {
			// _dprintf("%d: %s\n", __LINE__, strerror(errno));
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				usleep(10000);
				cnt--;
				continue;
			}
			else {
				perror("read");
				return -1;
			}
		}
		else if (n == 0)
			break;
		else {
			left -= n;
			p += n;
		}
	}

	return 0;
}

static void _mtwanduck_recv_sock(int newsockfd)
{
	MTWANDUCK_SOCK_DATA_T data;

	bzero(&data, sizeof(MTWANDUCK_SOCK_DATA_T));

	if (_do_read(newsockfd, &data, sizeof(MTWANDUCK_SOCK_DATA_T))) {
		_dprintf("%s:%d\n", __FUNCTION__, __LINE__);
		return;
	}

	if (data.d_type == MTWANDUCK_WAN_STAT) {
		int stat = 0;
		if (data.mtwan_unit >= MULTI_WAN_START_IDX && data.mtwan_unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM)
			stat = wan_status[data.mtwan_unit - MULTI_WAN_START_IDX];
		else
			stat = 0;
		if (_do_write(newsockfd, (void*)(&stat), sizeof(stat))) {
			_dprintf("%s:%d\n", __FUNCTION__, __LINE__);
		}
	}
	else if (data.d_type == MTWANDUCK_PHY_STAT) {
		int stat = 0;
		if (data.mtwan_unit >= MULTI_WAN_START_IDX && data.mtwan_unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM)
			stat = phy_status[data.mtwan_unit - MULTI_WAN_START_IDX];
		else
			stat = 0;
		if (_do_write(newsockfd, (void*)(&stat), sizeof(stat))) {
			_dprintf("%s:%d\n", __FUNCTION__, __LINE__);
		}
	}
	else if (data.d_type == MTWANDUCK_FREEZE) {
		freeze_duck = data.count;
	}
#ifdef RTCONFIG_MULTIWAN_PROFILE
	else if (data.d_type == MTWANDUCK_UPDATE_PROFILE) {
		mtwan_init_profile();
	}
#endif
	else {
		_dprintf("Receive unknown message.\n");
	}
}

static void _mtwanduck_handle_ipv6_route(int sockfd)
{
	struct nlmsghdr *nlh;
	struct rtmsg *rtm;
	ssize_t len;
	char buf[4096] = {0};
	unsigned int ifindex;
	char ifname[IFNAMSIZ] = {0};
	char dst[INET6_ADDRSTRLEN] = {0}, gateway[INET6_ADDRSTRLEN] = {0};

	nlh = (struct nlmsghdr *)buf;

	do {
		len = recv(sockfd, nlh, sizeof(buf), 0);
		if (len < 0) {
			perror("recv chkrt");
			return;
		}

		for (; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
			if (nlh->nlmsg_type == RTM_NEWROUTE) {
				rtm = (struct rtmsg *)NLMSG_DATA(nlh);
				if (rtm->rtm_protocol == RTPROT_RA
				 && rtm->rtm_table == RT_TABLE_MAIN
				) {
					// _dprintf("New route by RA\n");
					struct rtattr *rta = (struct rtattr *)RTM_RTA(rtm);
					int rtattrlen = RTM_PAYLOAD(nlh);
					memset(dst, 0 , sizeof(dst));
					memset(gateway, 0 , sizeof(gateway));
					memset(ifname, 0 , sizeof(ifname));
					for (; RTA_OK(rta, rtattrlen); rta = RTA_NEXT(rta, rtattrlen)) {
						switch (rta->rta_type) {
							case RTA_DST:
								inet_ntop(AF_INET6, RTA_DATA(rta), dst, sizeof(dst));
								// _dprintf("dst: %s\n", dst);
								break;
							case RTA_GATEWAY:
								inet_ntop(AF_INET6, RTA_DATA(rta), gateway, sizeof(gateway));
								// _dprintf("gateway: %s\n", gateway);
								break;
							case RTA_OIF:
								ifindex = *(unsigned int *)RTA_DATA(rta);
								if_indextoname(ifindex, ifname);
								// _dprintf("if: %u, %s\n", ifindex, ifname);
								break;
							default:
								// _dprintf("rta type: %d\n", rta->rta_type);
								// dump_data(RTA_DATA(rta), RTA_PAYLOAD(rta), "");
								break;
						}
					}
					if (!*dst && *gateway && *ifname) {
						mtwan6_handle_new_default_route(ifname, gateway);
					}
				}
			}
		}
	} while (len > 0);

}

static void _mtwanduck_handle_ipv6_addr(int sockfd)
{
	struct nlmsghdr *nlh;
	struct ifaddrmsg *ifaddr;
	ssize_t len;
	char buf[4096] = {0};
	char ifname[IFNAMSIZ] = {0};
	char addr[INET6_ADDRSTRLEN] = {0};

	nlh = (struct nlmsghdr *)buf;

	do {
		len = recv(sockfd, nlh, sizeof(buf), 0);
		if (len < 0) {
			perror("recv chkaddr");
			return;
		}

		for (; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
			if (nlh->nlmsg_type == RTM_NEWADDR) {
				ifaddr = (struct ifaddrmsg *)NLMSG_DATA(nlh);
				if (ifaddr->ifa_scope == RT_SCOPE_UNIVERSE) {
					if_indextoname(ifaddr->ifa_index, ifname);
					// _dprintf("New Global addr of %s\n", ifname);
					struct rtattr *rta = (struct rtattr *)IFA_RTA(ifaddr);
					int rtattrlen = IFA_PAYLOAD(nlh);
					memset(addr, 0 , sizeof(addr));
					for (; RTA_OK(rta, rtattrlen); rta = RTA_NEXT(rta, rtattrlen)) {
						switch (rta->rta_type) {
							case IFA_ADDRESS:
								inet_ntop(AF_INET6, RTA_DATA(rta), addr, sizeof(addr));
								// _dprintf("addr: %s\n", addr);
								break;
							default:
								// _dprintf("rta type: %d\n", rta->rta_type);
								// dump_data(RTA_DATA(rta), RTA_PAYLOAD(rta), "");
								break;
						}
					}
					if (*addr) {
						mtwan6_handle_new_addr(ifname, addr, (int)ifaddr->ifa_prefixlen);
					}
				}
			}
		}
	} while (len > 0);

}

void mtwanduck_data_thread()
{
	struct sockaddr_un addr;
	int sockfd, newsockfd;
	fd_set readFds;
	int ret;
	struct timeval timeout = {1, 0};
	int max_fd;
#ifdef RTCONFIG_IPV6
	int chkrt_fd;
	struct sockaddr_nl chkrt_nladdr;
	int chkaddr_fd;
	struct sockaddr_nl chkaddr_nladdr;
#endif

	if ( (sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket error");
		exit(-1);
	}
	max_fd = sockfd;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, MTWANDUCK_SOCKET_PATH, sizeof(addr.sun_path)-1);

	unlink(MTWANDUCK_SOCKET_PATH);

	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("socket bind error");
		exit(-1);
	}

	if (listen(sockfd, 3) == -1) {
		perror("listen error");
		exit(-1);
	}

#ifdef RTCONFIG_IPV6
	//initial for check ipv6 default route
	if ((chkrt_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0) {
		perror("socket chkrt");
		exit(-2);
	}
	if (chkrt_fd > max_fd)
		max_fd = chkrt_fd;

	chkrt_nladdr.nl_family = AF_NETLINK;
	chkrt_nladdr.nl_groups = RTMGRP_IPV6_ROUTE;
	if (bind(chkrt_fd, (struct sockaddr *)&chkrt_nladdr, sizeof(chkrt_nladdr)) < 0) {
		perror("bind chkrt");
		exit(-2);
	}
	//initial for check ipv6 address
	if ((chkaddr_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0) {
		perror("socket chkaddr");
		exit(-3);
	}
	if (chkaddr_fd > max_fd)
		max_fd = chkaddr_fd;

	chkaddr_nladdr.nl_family = AF_NETLINK;
	chkaddr_nladdr.nl_groups = RTMGRP_IPV6_IFADDR;
	if (bind(chkaddr_fd, (struct sockaddr *)&chkaddr_nladdr, sizeof(chkaddr_nladdr)) < 0) {
		perror("bind chkaddr");
		exit(-3);
	}
#endif

	while (!stop) {
		FD_ZERO(&readFds);
		FD_SET(sockfd, &readFds);
		FD_SET(chkrt_fd, &readFds);
		FD_SET(chkaddr_fd, &readFds);
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		ret = select(max_fd + 1, &readFds, NULL, NULL, &timeout);
		if (ret == -1)  {
			if (errno == EINTR || errno == ENOENT)
				continue;
			else {
				_dprintf("%s: select: %s\n", __FUNCTION__, strerror(errno));
				break;
			}
		}
		else if (ret == 0) {
			continue;
		}

		if (FD_ISSET(sockfd, &readFds)) {
			if ((newsockfd = accept(sockfd, NULL, NULL)) == -1) {
				_dprintf("%s: accept: %s\n", __FUNCTION__, strerror(errno));
				continue;
			}
			_mtwanduck_recv_sock(newsockfd);
			close(newsockfd);
		}
		if (FD_ISSET(chkrt_fd, &readFds)) {
			_mtwanduck_handle_ipv6_route(chkrt_fd);
		}
		if (FD_ISSET(chkaddr_fd, &readFds)) {
			_mtwanduck_handle_ipv6_addr(chkaddr_fd);
		}
	}

	close(sockfd);
	close(chkrt_fd);
	close(chkaddr_fd);
	exit(0);
}

static int _mtwanduck_cli_create()
{
	struct sockaddr_un addr;
	int sockfd;
	fd_set fds;
	struct timeval timeout;
	int ret;

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		return -1;
	}

	fcntl(sockfd, F_SETFL, O_NONBLOCK);

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", MTWANDUCK_SOCKET_PATH);

	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		if (errno != EINPROGRESS) {
			perror("connect");
			close(sockfd);
			return -1;
		}

		FD_ZERO(&fds);
		FD_SET(sockfd, &fds);
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;
		ret = select(sockfd + 1, &fds, NULL, NULL, &timeout);
		if (ret <= 0)  {
			close(sockfd);
			return -1;
		}
	}

	return sockfd;
}

static int _mtwanduck_get_data(MTWANDUCK_SOCK_DATA_T *s_data, void* data, size_t len)
{
	int fd;
	int ret = 0;

	fd = _mtwanduck_cli_create();
	if(fd < 0)
		return -1;

	if (_do_write(fd, s_data, sizeof(MTWANDUCK_SOCK_DATA_T))) {
		ret = -1;
		goto done;
	}
	if (_do_read(fd, data, len)) {
		ret = -1;
		if (data)
			memset(data, 0, len);
		goto done;
	}

done:
	close(fd);
	return (ret);
}

static int _mtwanduck_set_data(MTWANDUCK_SOCK_DATA_T *s_data)
{
	int fd;
	int ret = 0;

	fd = _mtwanduck_cli_create();
	if(fd < 0)
		return -1;

	if (_do_write(fd, s_data, sizeof(MTWANDUCK_SOCK_DATA_T))) {
		ret = -1;
	}

	close(fd);
	return (ret);
}

int mtwanduck_get_mtwan_status(int mtwan_unit)
{
	MTWANDUCK_SOCK_DATA_T data;
	int ret = 0;
	char prc[16] = {0};

	prctl(PR_GET_NAME, prc);
	if (!strcmp(prc, "mtwanduck"))
	{
		if (mtwan_unit >= MULTI_WAN_START_IDX && mtwan_unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM)
			return wan_status[mtwan_unit - MULTI_WAN_START_IDX];
		else
			return 0;
	}

	bzero(&data, sizeof(MTWANDUCK_SOCK_DATA_T));

	data.d_type = MTWANDUCK_WAN_STAT;
	data.mtwan_unit = mtwan_unit;

	if (_mtwanduck_get_data(&data, &ret, sizeof(ret))) {
		int unit = mtwan_get_mapped_unit(mtwan_unit);
		ret = (get_phy_status(unit) && _get_wan_status(unit)) ? 1 : 0;
	}

	return (ret);
}

int mtwanduck_get_phy_status(int mtwan_unit)
{
	MTWANDUCK_SOCK_DATA_T data;
	int ret = 0;
	char prc[16] = {0};

	prctl(PR_GET_NAME, prc);
	if (!strcmp(prc, "mtwanduck"))
	{
		if (mtwan_unit >= MULTI_WAN_START_IDX && mtwan_unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM)
			return phy_status[mtwan_unit - MULTI_WAN_START_IDX];
		else
			return 0;
	}

	bzero(&data, sizeof(MTWANDUCK_SOCK_DATA_T));

	data.d_type = MTWANDUCK_PHY_STAT;
	data.mtwan_unit = mtwan_unit;

	if (_mtwanduck_get_data(&data, &ret, sizeof(ret))) {
		char link_wan[16];
		link_wan_nvname(mtwan_get_mapped_unit(mtwan_unit), link_wan, sizeof(link_wan));
		ret = nvram_get_int(link_wan);
	}

	return (ret);
}

void mtwanduck_freeze(int count)
{
	MTWANDUCK_SOCK_DATA_T data;

	bzero(&data, sizeof(MTWANDUCK_SOCK_DATA_T));

	data.d_type = MTWANDUCK_FREEZE;
	data.count = count;

	_mtwanduck_set_data(&data);
}

void mtwanduck_update_profile()
{
	MTWANDUCK_SOCK_DATA_T data;

	bzero(&data, sizeof(MTWANDUCK_SOCK_DATA_T));

	data.d_type = MTWANDUCK_UPDATE_PROFILE;

	_mtwanduck_set_data(&data);
}

static void _mtwanduck_check()
{
#ifdef RTCONFIG_IPV6
	mtwan6_check_route();
	mtwan6_check_addr();
	mtwan6_check_dns();
#endif
}

static void _handle_signal(int sig)
{
	if(sig == SIGTERM)
	{
		stop = 1;
	}
	else if (sig == SIGALRM)
	{
		do_mtwanduck_check = 1;
	}
	else
	{
		_dprintf("%s: sig %d\n", __FUNCTION__, sig);
	}
}

static int _get_phy_status(int unit)
{
	int wan_type;
	int cur_phy_status;

	wan_type = get_dualwan_by_unit(unit);
	if (wan_type == WANS_DUALWAN_IF_WAN || wan_type == WANS_DUALWAN_IF_LAN)
	{
		cur_phy_status = get_wanports_status(unit);
	}
#ifdef RTCONFIG_USB_MODEM
	else if (wan_type == WANS_DUALWAN_IF_USB || wan_type == WANS_DUALWAN_IF_USB2)
	{
		cur_phy_status = is_usb_modem_ready(wan_type);
	}
#endif
	else
	{
		cur_phy_status = 0;
	}

	return cur_phy_status;
}

static void _init_phy_status(int phy_status[], size_t n)
{
	int i, unit;
	char wan_prefix[16];
	int wan_type;
	char link_wan[16];

	for (i = 0; i < n; ++i)
	{
		unit = mtwan_get_real_wan(i + MULTI_WAN_START_IDX, wan_prefix, sizeof(wan_prefix));
		if (unit != -1)
		{
			wan_type = get_dualwan_by_unit(unit);
			if (wan_type == WANS_DUALWAN_IF_WAN || wan_type == WANS_DUALWAN_IF_LAN)
			{
				phy_status[i] = get_wanports_status(unit);
			}
#ifdef RTCONFIG_USB_MODEM
			else if (wan_type == WANS_DUALWAN_IF_USB || wan_type == WANS_DUALWAN_IF_USB2)
			{
				phy_status[i] = is_usb_modem_ready(wan_type);
			}
#endif
			link_wan_nvname(unit, link_wan, sizeof(link_wan));
			nvram_set_int(link_wan, phy_status[i]);
		}
	}
}

static int _get_wan_status(int unit)
{
	char wan_prefix[16];

	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", mtwan_get_mapped_unit(unit));

	if (nvram_pf_get_int(wan_prefix, "link_internet") == 2
	 || nvram_pf_get_int(wan_prefix, "sbstate_t") == WAN_STOPPED_REASON_PPP_LACK_ACTIVITY
	)
		return 1;
	else
		return 0;
}

static void _init_wan_status(int wan_status[], size_t n)
{
	int i;
	int real_unit;
	char wan_prefix[16];

	for (i = 0; i < n; ++i)
	{
		real_unit = mtwan_get_real_wan(i + MULTI_WAN_START_IDX, wan_prefix, sizeof(wan_prefix));
		wan_status[i] = (phy_status[i] && _get_wan_status(real_unit));
#ifdef RTCONFIG_MULTIWAN_PROFILE
		if (is_mtwan_primary(real_unit) && wan_status[i])
			mtwanduck_set_primary_link_internet(real_unit, 2, phy_status[i]);
#endif
	}
}

#ifdef RTCONFIG_MULTIWAN_PROFILE
void mtwanduck_set_primary_link_internet(int wan_unit, int link_internet, int phy_status)
{
	nvram_set_int("link_internet", link_internet);

	/// sysdeps LED control
	update_wan_leds(wan_unit, phy_status);
}

static void _handle_nat_redirect_rule()
{
	int nat_redirect_enable = nvram_get_int("nat_redirect_enable");
	int nat_state = nvram_get_int("nat_state");
	int link_internet = nvram_get_int("link_internet");

	if (nat_redirect_enable == 0)
		return;

	if (nat_state == NAT_STATE_REDIRECT && link_internet == 2)
	{
		start_nat_rules();
	}
	else if (nat_state == NAT_STATE_NORMAL && link_internet != 2)
	{
		stop_nat_rules();
	}
}

static void _check_nat_redirect()
{
	static int nat_redirect_enable_old;
	int nat_redirect_enable = nvram_get_int("nat_redirect_enable");

	if (nat_redirect_enable != nat_redirect_enable_old)
	{
		int nat_state = nvram_get_int("nat_state");
		if (nat_redirect_enable)
		{
			if (nat_state == NAT_STATE_NORMAL && nvram_get_int("link_internet") != 2)
				stop_nat_rules();
		}
		else
		{
			if (nat_state == NAT_STATE_REDIRECT)
				start_nat_rules();
		}
		nat_redirect_enable_old = nat_redirect_enable;
	}
}

static void _handle_same_subnet(int wan_unit)
{
	if (get_wan_state(wan_unit) == WAN_STATE_STOPPED
	 && get_wan_sbstate(wan_unit) == WAN_STOPPED_REASON_INVALID_IPADDR)
	{
		char lan_ipaddr[INET_ADDRSTRLEN], lan_netmask[INET_ADDRSTRLEN];
		struct in_addr addr;
		in_addr_t new_addr;

		nvram_safe_get_r("lan_ipaddr", lan_ipaddr, sizeof(lan_ipaddr));
		nvram_safe_get_r("lan_netmask", lan_netmask, sizeof(lan_netmask));
		if (inet_deconflict(lan_ipaddr, lan_netmask, lan_ipaddr, lan_netmask, &addr))
		{
			nvram_set("lan_ipaddr", inet_ntoa(addr));
			nvram_set("lan_ipaddr_rt", inet_ntoa(addr));

			new_addr = ntohl(addr.s_addr);
			addr.s_addr = htonl(new_addr + 1);
			nvram_set("dhcp_start", inet_ntoa(addr));
			addr.s_addr = htonl((new_addr | ~inet_network(lan_netmask)) & 0xfffffffe);
			nvram_set("dhcp_end", inet_ntoa(addr));

			notify_rc_and_wait("restart_net_and_phy");
		}
	}
}
#endif

int mtwanduck_main(int argc, char *argv[])
{
	int i, unit;
	char prefix[] = "wanXXXXX_", tmp[256];
	char link_wan[16] = {0};
	int cur_phy_status = 0;
	int cur_wan_status = 0;
	int res;
	pthread_t det_internet_thread;
	pthread_t local_data_thread;
	pthread_attr_t thread_attr;
	struct itimerval itval;
	sigset_t set;
	int x_Setting = nvram_get_int("x_Setting");

	// signal
	sigemptyset(&set);
	sigaddset(&set, SIGTERM);
	sigaddset(&set, SIGALRM);
	sigaddset(&set, SIGCHLD);
	sigprocmask(SIG_UNBLOCK, &set, NULL);

	itval.it_value.tv_sec = MTWANDUCK_ALARM_SECS;
	itval.it_value.tv_usec = 0;
	itval.it_interval = itval.it_value;
	setitimer(ITIMER_REAL, &itval, NULL);

	signal(SIGALRM, _handle_signal);
	signal(SIGTERM, _handle_signal);
	signal(SIGCHLD, chld_reap);

	//handle thread
	pthread_mutex_init(&mtwanduck_chkaddr, NULL);
	pthread_mutex_init(&mtwanduck_chkrt, NULL);
	/// detect internet
	res = pthread_attr_init(&thread_attr);
	if(!res)
	{
		res = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
		if(!res)
		{
			res = pthread_create(&det_internet_thread, &thread_attr, mtwan_detect_internet, NULL);
		}
		pthread_attr_destroy(&thread_attr);
	}
	if(!res)
		_dprintf("[%s]Create detect internet thread SUCCESS.\n", __FUNCTION__);
	else
		_dprintf("[%s]Create detect internet thread FAIL!\n", __FUNCTION__);
	/// get local data thread
	res = pthread_attr_init(&thread_attr);
	if(!res)
	{
		res = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
		if(!res)
		{
			res = pthread_create(&local_data_thread, &thread_attr, (void*)mtwanduck_data_thread, NULL);
		}
		pthread_attr_destroy(&thread_attr);
	}
	if(!res)
		_dprintf("[%s]Create local thread SUCCESS.\n", __FUNCTION__);
	else
		_dprintf("[%s]Create local thread FAIL!\n", __FUNCTION__);

	//initial wait
	while(!nvram_get_int("success_start_service"))
		sleep(1);

	while (freeze_duck)
	{
		_dprintf("\n mtwanduck first wait, %d left!\n", freeze_duck--);
		sleep(1);
		continue;
	}

	//update current status
	_init_phy_status(phy_status, MAX_MULTI_WAN_NUM);
#ifdef RTCONFIG_MULTIWAN_PROFILE
	_init_wan_status(wan_status, MAX_MULTI_WAN_NUM);
	mtwan_init_profile();
	mtwan_init_mtwan_group();
#endif

	//main loop to check
	while(!stop)
	{
		if (freeze_duck)
		{
			_dprintf("\nfreeze mtwanduck, %d left!\n", freeze_duck--);
			sleep(1);
			continue;
		}
		else if (nvram_get_int("freeze_duck") > 0)
		{
			_dprintf("\nfreeze mtwanduck by dualwan freeze_duck!\n");
			sleep(1);
			continue;
		}
		x_Setting = nvram_get_int("x_Setting");

		for (i = 0; i < MAX_MULTI_WAN_NUM; ++i)
		{
			unit = mtwan_get_real_wan(i + MULTI_WAN_START_IDX, prefix, sizeof(prefix));
			if (unit != -1)
			{
				//check phy state
#if !defined(RTCONFIG_MULTIWAN_PROFILE)
				snprintf(prefix, sizeof(prefix), "wan%d_", i + MULTI_WAN_START_IDX);
#endif
				cur_phy_status = _get_phy_status(unit);

				if (cur_phy_status != phy_status[i])
				{
#if !defined(RTCONFIG_MULTIWAN_PROFILE)
					if(is_mtwan_unit(unit))	//only handle non-dualwan unit for now.
#endif
					{
						snprintf(prefix, sizeof(prefix), "wan%d_", unit);
						link_wan_nvname(unit, link_wan, sizeof(link_wan));
						nvram_set_int(link_wan, cur_phy_status);
						if (cur_phy_status)
						{
							_dprintf("\nmtwanduck: Try to restart_wan_if %d.\n", unit);
							snprintf(tmp, sizeof(tmp), "restart_wan_if %d", unit);
							notify_rc(tmp);
						}
						else
						{
							_dprintf("\nmtwanduck: WAN %d down.\n", unit);
							nvram_set_int(strlcat_r(prefix, "state_t", tmp, sizeof(tmp)), WAN_STATE_DISCONNECTED);
							nvram_set_int(strlcat_r(prefix, "auxstate_t", tmp, sizeof(tmp)), WAN_AUXSTATE_NOPHY);
							nvram_set_int(strlcat_r(prefix, "link_internet", tmp, sizeof(tmp)), 1);
#ifdef RTCONFIG_AUTO_WANPORT
							if (unit == nvram_get_int("autowan_live_wanunit") && is_auto_wanport_enabled() == 2)
								restore_auto_wanport();
#endif
						}
					}
					phy_status[i] = cur_phy_status;
				}

				//check connection state
				cur_wan_status = (cur_phy_status && _get_wan_status(unit)) ? 1 : 0;
				if (cur_wan_status != wan_status[i])
				{
					wan_status[i] = cur_wan_status;
					if (cur_wan_status)
					{
						_dprintf("\nmtwanduck: WAN %d Connected.\n", unit);
						logmessage("MTWAN", "WAN %d Connected.\n", unit);
#ifdef RTCONFIG_MULTIWAN_PROFILE
						mtwan_handle_wan_conn(unit, cur_wan_status, cur_phy_status);
						if (is_mtwan_primary(unit))
						{
							mtwanduck_set_primary_link_internet(unit, 2, cur_phy_status);
							_handle_nat_redirect_rule();
						}
#endif
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK)
						if (nvram_get_int("x_Setting") == 0) {
							nvram_set("amas_bdl_wanstate", "2");
#if defined(RTCONFIG_BT_CONN)
							ble_rename_ssid();
#endif
						}
#endif
					}
					else
					{
						_dprintf("\nmtwanduck: WAN %d Disconnected.\n", unit);
						logmessage("MTWAN", "WAN %d Disconnected.\n", unit);
#ifdef RTCONFIG_AUTO_WANPORT
						if (unit == nvram_get_int("autowan_live_wanunit") && is_auto_wanport_enabled() > 0)
							restore_auto_wanport();
#endif
#ifdef RTCONFIG_MULTIWAN_PROFILE
						mtwan_handle_wan_conn(unit, cur_wan_status, cur_phy_status);
						if (is_mtwan_primary(unit))
						{
							if (nvram_get_int(strlcat_r(prefix, "state_t", tmp, sizeof(tmp))) == WAN_STATE_CONNECTED)
								mtwanduck_set_primary_link_internet(unit, 1, cur_phy_status);
							else
								mtwanduck_set_primary_link_internet(unit, 0, cur_phy_status);
							_handle_nat_redirect_rule();
						}
#endif
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK)
						if (nvram_get_int("x_Setting") == 0) {
							nvram_set("amas_bdl_wanstate", "0");
#if defined(RTCONFIG_BT_CONN)
							ble_rename_ssid();
#endif
						}
#endif
					}
				}

#ifdef RTCONFIG_MULTIWAN_PROFILE
				if (!x_Setting && !cur_wan_status && nvram_get_int("atcover_sip"))
					_handle_same_subnet(unit);
#endif
			}
		}

		if (do_mtwanduck_check)
		{
			_mtwanduck_check();
			do_mtwanduck_check = 0;
		}

#ifdef RTCONFIG_MULTIWAN_PROFILE
		/// check nat_redirect_enable change
		_check_nat_redirect();
#endif

		sleep(1);
	}

	pthread_mutex_destroy(&mtwanduck_chkaddr);
	pthread_mutex_destroy(&mtwanduck_chkrt);
	unlink(MTWANDUCK_SOCKET_PATH);
	return 0;
}

