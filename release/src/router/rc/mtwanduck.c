#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/prctl.h>

#include <rc.h>
#include <shared.h>
#include <multi_wan.h>

#define MTWANDUCK_SOCKET_PATH  "/var/run/mtwanduck_socket"

int stop = 0;

extern void  *mtwan_detect_internet(void *arg);

static int phy_status[MAX_MULTI_WAN_NUM] = {0};
static int wan_status[MAX_MULTI_WAN_NUM] = {0};
static int freeze_duck = 10;

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

static void _mtwanduck_recv_sock(int newsockfd)
{
	MTWANDUCK_SOCK_DATA_T data;
	int n;

	bzero(&data, sizeof(MTWANDUCK_SOCK_DATA_T));

	n = read(newsockfd, &data, sizeof(MTWANDUCK_SOCK_DATA_T));
	if( n < 0 ) {
		return;
	}

	if (data.d_type == MTWANDUCK_WAN_STAT) {
		int stat = 0;
		if (data.mtwan_unit >= MULTI_WAN_START_IDX && data.mtwan_unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM)
			stat = wan_status[data.mtwan_unit - MULTI_WAN_START_IDX];
		else
			stat = 0;
		n = write(newsockfd, (void*)(&stat), sizeof(stat));
		if (n < 0)
			perror("write");
	}
	else if (data.d_type == MTWANDUCK_PHY_STAT) {
		int stat = 0;
		if (data.mtwan_unit >= MULTI_WAN_START_IDX && data.mtwan_unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM)
			stat = phy_status[data.mtwan_unit - MULTI_WAN_START_IDX];
		else
			stat = 0;
		n = write(newsockfd, (void*)(&stat), sizeof(stat));
		if (n < 0)
			perror("write");
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

void mtwanduck_data_thread()
{
	struct sockaddr_un addr;
	int sockfd, newsockfd;

	if ( (sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket error");
		exit(-1);
	}

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

	while (1) {
		if ( (newsockfd = accept(sockfd, NULL, NULL)) == -1) {
			perror("accept error");
			continue;
		}
		_mtwanduck_recv_sock(newsockfd);
		close(newsockfd);
	}
}

static int _mtwanduck_cli_create()
{
	struct sockaddr_un addr;
	int sockfd;

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", MTWANDUCK_SOCKET_PATH);

	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		perror("connect");
		close(sockfd);
		return -1;
	}

	return sockfd;
}

static int _mtwanduck_get_data(MTWANDUCK_SOCK_DATA_T *s_data, void* data, size_t len)
{
	int fd;
	int ret = 0;
	size_t left = 0;
	ssize_t n = 0;
	uint8_t *p = NULL;

	fd = _mtwanduck_cli_create();
	if(fd < 0)
		return -1;

	left = sizeof(MTWANDUCK_SOCK_DATA_T);
	p = (uint8_t *)s_data;
	while(1) {
		n = write(fd, p, left);
		if (n < 0) {
			if (errno == EAGAIN)
				continue;
			else {
				perror("write");
				ret = -1;
				goto done;
			}
		}
		else if (n == left)
			break;
		else {
			left -= n;
			p += n;
		}
	}

	left = len;
	p = data;
	while(1) {
		n = read(fd, p, left);
		if (n < 0) {
			if (errno == EAGAIN)
				continue;
			else {
				perror("read");
				ret = -1;
				goto done;
			}
		}
		else if (n == 0)
			break;
		else {
			left -= n;
			p += n;
		}
	}

done:
	close(fd);
	return (ret);
}

static int _mtwanduck_set_data(MTWANDUCK_SOCK_DATA_T *s_data)
{
	int fd;
	int ret = 0;
	size_t left = 0;
	ssize_t n = 0;
	uint8_t *p = NULL;

	fd = _mtwanduck_cli_create();
	if(fd < 0)
		return -1;

	left = sizeof(MTWANDUCK_SOCK_DATA_T);
	p = (uint8_t *)s_data;
	while(1) {
		n = write(fd, p, left);
		if (n < 0) {
			if (errno == EAGAIN)
				continue;
			else {
				perror("write");
				ret = -1;
				goto set_done;
			}
		}
		else if (n == left)
			break;
		else {
			left -= n;
			p += n;
		}
	}

set_done:
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

	_mtwanduck_get_data(&data, &ret, sizeof(ret));
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

	_mtwanduck_get_data(&data, &ret, sizeof(ret));
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

static void _handle_signal(int sig)
{
	if(sig == SIGTERM)
	{
		stop = 1;
	}
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
	return (nvram_pf_get_int(wan_prefix, "link_internet") == 2) ? 1 : 0;
}

static void _init_wan_status(int wan_status[], size_t n)
{
	int i;
	int real_unit;
	char wan_prefix[16];

	for (i = 0; i < n; ++i)
	{
		real_unit = mtwan_get_real_wan(i + MULTI_WAN_START_IDX, wan_prefix, sizeof(wan_prefix));
		wan_status[i] = (phy_status[i] && nvram_pf_get_int(wan_prefix, "link_internet") == 2) ? 1 : 0;
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
	static int nat_redirect_enable_old;
	int nat_redirect_enable;
	static int nat_state;
	int link_internet = nvram_get_int("link_internet");

	nat_redirect_enable = nvram_get_int("nat_redirect_enable");
	if (nat_state == NAT_STATE_REDIRECT && link_internet == 2)
	{
		nat_state = start_nat_rules();
	}
	else if (nat_state == NAT_STATE_NORMAL && link_internet != 2)
	{
		if (nat_redirect_enable)
			nat_state = stop_nat_rules();
	}
	else if (nat_state == NAT_STATE_INITIALIZING)
	{
		nat_state = nvram_get_int("nat_state");
	}

	if (nat_redirect_enable != nat_redirect_enable_old)
	{
		if (!nat_redirect_enable && nat_state == NAT_STATE_REDIRECT)
			nat_state = start_nat_rules();
		nat_redirect_enable_old = nat_redirect_enable;
	}
}
#endif

int mtwanduck_main(int argc, char *argv[])
{
	int i, unit;
	char prefix[] = "wanXXXXX_", tmp[256];
	char link_wan[16] = {0};
	int wan_type;
	int cur_phy_status = 0;
	int cur_wan_status = 0;
	int res;
	pthread_t det_internet_thread;
	pthread_t local_data_thread;
	pthread_attr_t thread_attr;

	signal(SIGTERM, _handle_signal);

	//handle thread
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

		for (i = 0; i < MAX_MULTI_WAN_NUM; ++i)
		{
			unit = mtwan_get_real_wan(i + MULTI_WAN_START_IDX, prefix, sizeof(prefix));
			if (unit != -1)
			{
				//check phy state
#if !defined(RTCONFIG_MULTIWAN_PROFILE)
				snprintf(prefix, sizeof(prefix), "wan%d_", i + MULTI_WAN_START_IDX);
#endif
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
						mtwan_handle_wan_conn(unit, cur_wan_status);
						if (is_mtwan_primary(unit))
							mtwanduck_set_primary_link_internet(unit, 2, cur_phy_status);
#endif
					}
					else
					{
						_dprintf("\nmtwanduck: WAN %d Disconnected.\n", unit);
						logmessage("MTWAN", "WAN %d Disconnected.\n", unit);
#ifdef RTCONFIG_MULTIWAN_PROFILE
						mtwan_handle_wan_conn(unit, cur_wan_status);
						if (is_mtwan_primary(unit))
						{
							if (nvram_get_int(strlcat_r(prefix, "state_t", tmp, sizeof(tmp))) == WAN_STATE_CONNECTED)
								mtwanduck_set_primary_link_internet(unit, 1, cur_phy_status);
							else
								mtwanduck_set_primary_link_internet(unit, 0, cur_phy_status);
						}
#endif
					}
				}

			}
		}

#ifdef RTCONFIG_MULTIWAN_PROFILE
		/// nat redirect rule
		_handle_nat_redirect_rule();
#endif

		sleep(1);
	}
	return 0;
}

