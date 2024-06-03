#include "includes.h"
#include "mapd_interface_ctrl.h"
#include "os.h"
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

#define CMD_SUC_SYNC_EVENT 0
#define CMD_SUC_ASYNC_EVENT 1
#define CMD_PROCESS_FAIL 2
#define dump_file_path "/tmp/dump.txt"
#define dump_de_file_path "/tmp/dump_de.txt"
#define dump_ch_scan_dump "/tmp/dump_ch_scan.txt"
#define dump_ch_score_dump "/tmp/dump_ch_score.txt"
#define max_file_size 15000
#define PRINT_MAC(a) a[0],a[1],a[2],a[3],a[4],a[5]

struct mapd_interface_ctrl * mapd_interface_ctrl_open(const char *ctrl_path)
{
    struct mapd_interface_ctrl *ctrl;
    static int counter = 0;
    int ret;
    size_t res;
    int tries = 0;
    int flags;

    if (ctrl_path == NULL)
        return NULL;

    ctrl = (struct mapd_interface_ctrl *)os_malloc(sizeof(*ctrl));
    if (ctrl == NULL)
    {
        printf("%s, alloc memory fail\n", __func__);
        return NULL;
    }
    os_memset(ctrl, 0, sizeof(*ctrl));

    ctrl->s = socket(PF_UNIX, SOCK_DGRAM, 0);
    if (ctrl->s < 0) {
        os_free(ctrl);
        return NULL;
    }

    ctrl->local.sun_family = AF_UNIX;
    counter++;
try_again:
    ret = os_snprintf(ctrl->local.sun_path,
            sizeof(ctrl->local.sun_path),
            "/tmp" "/"
            "mapd_ctrl" "%d-%d",
            (int) getpid(), counter);
    tries++;
    if (bind(ctrl->s, (struct sockaddr *) &ctrl->local,
                sizeof(ctrl->local)) < 0) {
        if (errno == EADDRINUSE && tries < 2) {
            /* getpid() returns unique identifier for this instance
             * of mapd_interface_ctrl, so the existing socket file must have
             * been left by unclean termination of an earlier run.
             * Remove the file and try again. */
            unlink(ctrl->local.sun_path);
            goto try_again;
        }
        close(ctrl->s);
        os_free(ctrl);
        return NULL;
    }
    ctrl->dest.sun_family = AF_UNIX;
    if (os_strncmp(ctrl_path, "@abstract:", 10) == 0) {
        ctrl->dest.sun_path[0] = '\0';
        os_strlcpy(ctrl->dest.sun_path + 1, ctrl_path + 10,
                sizeof(ctrl->dest.sun_path) - 1);
    } else {
        res = os_strlcpy(ctrl->dest.sun_path, ctrl_path,
                sizeof(ctrl->dest.sun_path));
        if (res >= sizeof(ctrl->dest.sun_path)) {
            close(ctrl->s);
            os_free(ctrl);
            return NULL;
        }
    }
    if (connect(ctrl->s, (struct sockaddr *) &ctrl->dest,
                sizeof(ctrl->dest)) < 0) {
        close(ctrl->s);
        unlink(ctrl->local.sun_path);
        os_free(ctrl);
        return NULL;
    }
    /*
     *      * Make socket non-blocking so that we don't hang forever if
     *           * target dies unexpectedly.
     *                */
    flags = fcntl(ctrl->s, F_GETFL);
    if (flags >= 0) {
        flags |= O_NONBLOCK;
        if (fcntl(ctrl->s, F_SETFL, flags) < 0) {
            perror("fcntl(ctrl->s, O_NONBLOCK)");
            /* Not fatal, continue on.*/
        }
    }

    return ctrl;
}

void mapd_interface_ctrl_close(struct mapd_interface_ctrl *ctrl)
{
	if (ctrl == NULL)
		return;
    unlink(ctrl->local.sun_path);
	if (ctrl->s >= 0)
		close(ctrl->s);
	os_free(ctrl);
}
void os_sleep(os_time_t sec, os_time_t usec)
{
    if (sec)
        sleep(sec);
    if (usec)
        usleep(usec);
}

int mapd_interface_ctrl_request(struct mapd_interface_ctrl *ctrl, const char *cmd, size_t cmd_len,
        char *reply, size_t *reply_len,
        void (*msg_cb)(char *msg, size_t len))
{
    struct timeval tv;
    struct os_reltime started_at;
    int res;
    fd_set rfds;
    const char *_cmd;
    char *cmd_buf = NULL;
    size_t _cmd_len;

    _cmd = cmd;
    _cmd_len = cmd_len;
    errno = 0;
    started_at.sec = 0;
    started_at.usec = 0;
retry_send:
    if (send(ctrl->s, _cmd, _cmd_len, 0) < 0) {
        if (errno == EAGAIN || errno == EBUSY || errno == EWOULDBLOCK)
        {
            /* Must be a non-blocking socket... Try for a bit
             * longer before giving up. */
            if (started_at.sec == 0)
                os_get_reltime(&started_at);
            else {
                struct os_reltime n;
                os_get_reltime(&n);
                /* Try for a few seconds. */
                if (os_reltime_expired(&n, &started_at, 5))
                    goto send_err;
            }
            os_sleep(1, 0);
            goto retry_send;
        }
send_err:
        os_free(cmd_buf);
        return -1;
    }
    os_free(cmd_buf);

    for (;;) {
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        FD_ZERO(&rfds);
        FD_SET(ctrl->s, &rfds);
        res = select(ctrl->s + 1, &rfds, NULL, NULL, &tv);
        if (res < 0 && errno == EINTR)
            continue;
        if (res < 0)
            return res;
        if (FD_ISSET(ctrl->s, &rfds)) {
            res = recv(ctrl->s, reply, *reply_len, 0);
            if (res < 0)
                return res;
            if (res > 0 && reply[0] == '<') {
                /* This is an unsolicited message from
                 * mapd, not the reply to the
                 * request. Use msg_cb to report this to the
                 * caller. */
                if (msg_cb) {
                    /* Make sure the message is nul terminated. */
                    if ((size_t) res == *reply_len)
                        res = (*reply_len) - 1;
                    reply[res] = '\0';
                    msg_cb(reply, res);
                }
                continue;
            }
            *reply_len = res;
            break;
        } else {
            return -2;
        }
    }
    return 0;
}

int mapd_interface_ctrl_pending(struct mapd_interface_ctrl *ctrl);

static int mapd_interface_ctrl_attach_helper(struct mapd_interface_ctrl *ctrl, int attach)
{
    char buf[10];
    int ret;
    size_t len = 10;

    ret = mapd_interface_ctrl_request(ctrl, attach ? "ATTACH" : "DETACH", 6,
            buf, &len, NULL);
    if (ret < 0)
        return ret;
    if (len == 3 && os_memcmp(buf, "OK\n", 3) == 0)
        return 0;
    return -1;
}

int mapd_interface_ctrl_attach(struct mapd_interface_ctrl *ctrl)
{
        return mapd_interface_ctrl_attach_helper(ctrl, 1);
}


int mapd_interface_ctrl_detach(struct mapd_interface_ctrl *ctrl)
{
        return mapd_interface_ctrl_attach_helper(ctrl, 0);
}


int mapd_interface_ctrl_recv(struct mapd_interface_ctrl *ctrl, char *reply, size_t *reply_len)
{
	int res;

	res = recv(ctrl->s, reply, *reply_len, 0);
	if (res < 0)
		return res;
	*reply_len = res;
	return 0;
}


int mapd_interface_ctrl_pending(struct mapd_interface_ctrl *ctrl)
{
    struct timeval tv;
    fd_set rfds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_SET(ctrl->s, &rfds);
    select(ctrl->s + 1, &rfds, NULL, NULL, &tv);
    return FD_ISSET(ctrl->s, &rfds);
}


int mapd_interface_ctrl_get_fd(struct mapd_interface_ctrl *ctrl)
{
	return ctrl->s;
}
#ifdef SUPPORT_MULTI_AP
int mapd_interface_get_topology(struct mapd_interface_ctrl *ctrl, char *topo_buf, size_t *buf_len, char *file_path_)
{
	char cmd[100] = {0};
	char reply[4096];
	size_t len = sizeof(reply);
	char *pos, *end;
	int ret = 0, index = 1;
	unsigned char found = 0;
	int i = 0, fsize;
	FILE *fptr = NULL;

	pos = topo_buf;
	end = topo_buf + *buf_len;

	if(file_path_ == NULL)
		strcpy(file_path, dump_file_path);
	else
		strcpy(file_path, (const char *)file_path_);
	os_memset(cmd, 0, sizeof(cmd));
	os_snprintf(cmd, sizeof(cmd), "dump_topology_v1 %d %s", index, file_path);
	len = sizeof(reply);
	if (mapd_interface_ctrl_request(ctrl, cmd, os_strlen(cmd), reply, &len, NULL) < 0) {
		return 0;
	}
	fptr = fopen(file_path, "r");
	if(fptr == NULL) {
		printf("Error!");
		//exit(1);
		return 0;
	}
	fseek(fptr, 0, SEEK_END);
	fsize = ftell(fptr);
	if (fsize >= 10240) {
		printf("topo_buf not enough space!!\n");
		fclose(fptr);
		return -1;
	}
	fseek(fptr, 0, SEEK_SET);
	fread(pos, fsize, 1, fptr);
	fclose(fptr);


	
	*buf_len = strlen(topo_buf);
	return 0;
}

int mapd_interface_set_enrollee_bh_info(struct mapd_interface_ctrl *ctrl, char *if_mac, unsigned char if_type)
{
	char cmd[128];
	char recv_buf[16];
	size_t recv_len = sizeof(recv_buf);

	os_memset(cmd, 0, sizeof(cmd));
	os_memset(recv_buf, 0, sizeof(recv_buf));
	os_snprintf(cmd, sizeof(cmd), "set_enrollee_bh %s %02x", if_mac, if_type);
//	os_snprintf(cmd, sizeof(cmd), "set_enrollee_bh %02x:%02x:%02x:%02x:%02x:%02x %02x", PRINT_MAC(if_mac), if_type);
	if(mapd_interface_ctrl_request(ctrl, cmd, os_strlen(cmd), recv_buf, &recv_len, NULL) < 0)
		return -1;

	if (os_memcmp(recv_buf, "OK", 2) != 0)
		return -1;

	return 0;
}
int mapd_interface_set_acl_block(struct mapd_interface_ctrl *ctrl, unsigned char type, int argc, char *argv[])
{
#define ETH_ALEN 6
	char cmd[256];
	char recv_buf[16];
	unsigned int dur;
	size_t recv_len = sizeof(recv_buf);
	unsigned char cli_mac[ETH_ALEN]={0};
	unsigned char bss_mac[ETH_ALEN]={0};
	unsigned char al_mac[ETH_ALEN]={0};

	hwaddr_aton(argv[2], cli_mac);
	if((type == 1 || type == 2) && argc >= 4)
	hwaddr_aton(argv[3], al_mac);
	if(type == 2 && argc == 5)
	hwaddr_aton(argv[4], bss_mac);

	os_memset(cmd, 0, sizeof(cmd));
	os_memset(recv_buf, 0, sizeof(recv_buf));
	os_snprintf(cmd, sizeof(cmd), "set_acl_block %02x %s %02x:%02x:%02x:%02x:%02x:%02x %02x:%02x:%02x:%02x:%02x:%02x %02x:%02x:%02x:%02x:%02x:%02x", type, argv[1], PRINT_MAC(cli_mac), PRINT_MAC(al_mac), PRINT_MAC(bss_mac));
	err("%s",cmd);
	if(mapd_interface_ctrl_request(ctrl, cmd, os_strlen(cmd), recv_buf, &recv_len, NULL) < 0)
		return -1;

	if (os_memcmp(recv_buf, "OK", 2) != 0)
		return -1;

	return 0;
}

int mapd_interface_set_bss_role(struct mapd_interface_ctrl *ctrl, unsigned char *bssid, unsigned char role)
{

	char cmd[128];
	char recv_buf[16];
	size_t recv_len = sizeof(recv_buf);

	os_memset(cmd, 0, sizeof(cmd));
	os_memset(recv_buf, 0, sizeof(recv_buf));
	os_snprintf(cmd, sizeof(cmd), "set_bss_role %02x:%02x:%02x:%02x:%02x:%02x %02x", PRINT_MAC(bssid), role);
	if (mapd_interface_ctrl_request(ctrl, cmd, os_strlen(cmd), recv_buf, &recv_len, NULL) < 0)
		return -1;

	if (os_memcmp(recv_buf, "OK", 2) != 0)
		return -1;

	return 0;
}

int mapd_interface_set_bh_priority(struct mapd_interface_ctrl *ctrl, unsigned char *_2g, unsigned char *_5gl, unsigned char *_5gh)
{
	char cmd[128];
	char recv_buf[16];
	int bands = 0;
	size_t recv_len = sizeof(recv_buf);
	int priority_2g, priority_5gl, priority_5gh;
	if(_2g == NULL || _5gl == NULL)
		return -1;
	if(_5gh == NULL)
		bands = 2;
	else
		bands = 3;
	priority_2g = atoi(_2g);
	priority_5gl = atoi(_5gl);
	if(bands == 3)
		priority_5gh = atoi(_5gh);
	if (priority_2g > 3 || priority_2g < 0)
		return -1;
	if (priority_5gl > 3 || priority_5gl < 0)
		return -1;
	if (bands == 3) {
		if (priority_5gh > 3 || priority_5gh < 0)
			return -1;
	}
	if (bands == 3)
		info("Setting BH priority...\nPriority 2G: %d\nPriority 5GL: %d Priority 5GH: %d\n",
				priority_2g, priority_5gl, priority_5gh);
	else
		info("Setting BH priority...\nPriority 2G: %d\nPriority 5G: %d\n",
				priority_2g, priority_5gl);

	os_memset(cmd, 0, sizeof(cmd));
	os_memset(recv_buf, 0, sizeof(recv_buf));
	if (bands == 3)
		os_snprintf(cmd, sizeof(cmd), "bh_priority %d %d %d %d", bands, priority_2g, priority_5gl, priority_5gh);
	else
		os_snprintf(cmd, sizeof(cmd), "bh_priority %d %d %d", bands, priority_2g, priority_5gl);
	if (mapd_interface_ctrl_request(ctrl, cmd, os_strlen(cmd), recv_buf, &recv_len, NULL) < 0)
		return -1;

	if (os_memcmp(recv_buf, "OK", 2) != 0)
		return -1;

	return 0;
}
int mapd_interface_trigger_map_wps(struct mapd_interface_ctrl *ctrl, char *mac_addr)
{
	char cmd[64];
	char recv_buf[16];
	size_t recv_len = sizeof(recv_buf);

	os_memset(cmd, 0, sizeof(cmd));
	os_memset(recv_buf, 0, sizeof(recv_buf));
	os_snprintf(cmd, sizeof(cmd), "trigger_wps %s PBC", mac_addr);
//	os_snprintf(cmd, sizeof(cmd), "trigger_wps %02x:%02x:%02x:%02x:%02x:%02x PBC", PRINT_MAC(mac_addr));
	if (mapd_interface_ctrl_request(ctrl, cmd, os_strlen(cmd), recv_buf, &recv_len, NULL) < 0)
		return -1;

	if (os_memcmp(recv_buf, "OK", 2) != 0)
		return -1;

	return 0;

}
int mapd_interface_trigger_onboarding(struct mapd_interface_ctrl *ctrl, char *int_name)
{
	if(*int_name=='1'){
		//printf("wifi interface select \n");
		system("wappctrl ra0 wps_pbc");
		return 0;
	}else if(*int_name=='0'){
		//printf("eth interface select \n");
		system("wappctrl ra0 map set_bh_type eth");
		return 0;
	}else{
		//printf("no interface select\n");
		return -1;
	}
}

int mapd_interface_trigger_bh_steer(struct mapd_interface_ctrl *ctrl, char *iface, char *bssid)
{
	char cmd[128];
	char recv_buf[16];
	size_t recv_len = sizeof(recv_buf);

	os_memset(cmd, 0, sizeof(cmd));
	os_memset(recv_buf, 0, sizeof(recv_buf));
	os_snprintf(cmd, sizeof(cmd), "ap_selection_bh_steer %s %s", iface, bssid);
	if (mapd_interface_ctrl_request(ctrl, cmd, os_strlen(cmd), recv_buf, &recv_len, NULL) < 0)
		return -1;

	if (os_memcmp(recv_buf, "OK", 2) != 0)
		return -1;

	return 0;
}
#endif /* #ifdef SUPPORT_MULTI_AP */
int mapd_interface_get_client_db(struct mapd_interface_ctrl *ctrl, struct client_db *dbs_buf, size_t *buf_len)
{	
	char cmd[20] = {"get_client_DB 1 200"};
	
	if (mapd_interface_ctrl_request(ctrl, cmd, os_strlen(cmd), (char *)dbs_buf, buf_len, NULL) < 0)
		return -1;

/*	
	if ((*buf_len) % sizeof(struct client_db) != 0)
		return -1;
*/
	return (*buf_len) / sizeof(struct client_db);
}

#ifdef SUPPORT_MULTI_AP
int mapd_interface_get_role(struct mapd_interface_ctrl *ctrl,int *role)
{
	char cmd[10];
	int res;
	res = os_snprintf(cmd, sizeof(cmd), "GET role");
	size_t len1=strlen(cmd);
	 char buf[10];
	 size_t len;
	 int ret;
	 len = sizeof(buf) - 1;
	 ret = mapd_interface_ctrl_request(ctrl, cmd, len1, buf, &len,NULL);
	if (ret == -2) {
        return -2;
    } else if (ret < 0) {
        return -1;
    }
	*role=buf[0];
    buf[len] = '\0';
    return 0;
}
int mapd_interface_get_conn_status(struct mapd_interface_ctrl *ctrl, int *fhbss_status, int *bhsta_status)
{
	char cmd[15], buf[15];
	int res, ret;
	size_t len;
	res = os_snprintf(cmd, sizeof(cmd), "CONN_STATUS");
	size_t len1 = strlen(cmd);
	len = sizeof(buf) - 1;
	ret = mapd_interface_ctrl_request(ctrl, cmd, len1, buf, &len, NULL);
	if (ret == -2) {
		return -2;
	} else if (ret < 0) {
		return -1;
	}
	*fhbss_status = buf[0];
	*bhsta_status = buf[4];
	printf("fhbss_status: %d, bhsta_status: %d\n", *fhbss_status, *bhsta_status);
	buf[len] = '\0';
	return 0;
}
int mapd_interface_select_best_ap(struct mapd_interface_ctrl *ctrl)
{
	char cmd[20];
	int res;
	res = os_snprintf(cmd, sizeof(cmd), "SELECT BEST AP");
    size_t len1=strlen(cmd);
	 char buf[30];
	 size_t len;
	 int ret;
	 len = sizeof(buf) - 1;
	 ret = mapd_interface_ctrl_request(ctrl, cmd, len1, buf, &len,NULL);
	if (ret == -2) {
        return -2;
    } else if (ret < 0) {
        return -1;
    }
    buf[len] = '\0';
    return 0;
}
int mapd_interface_set_rssi_thresh(struct mapd_interface_ctrl *ctrl, char *rssi_thresh )
{
	char cmd[50];
	int res;
	res = os_snprintf(cmd, sizeof(cmd), "SET rssith %s",rssi_thresh);
    size_t len1=strlen(cmd);
	 char buf[30];
	 size_t len;
	 int ret;
	 len = sizeof(buf) - 1;
	 ret = mapd_interface_ctrl_request(ctrl, cmd, len1, buf, &len,NULL);
	if (ret == -2) {
        return -2;
    } else if (ret < 0) {
        return -1;
    }
    buf[len] = '\0';
    return 0;
}
int mapd_interface_set_ChUtil_thresh(struct mapd_interface_ctrl *ctrl, char *cu_thresh2G , char *cu_thresh5GL, char *cu_thresh5GH)
{
	char cmd[50];
	int res;
	res = os_snprintf(cmd, sizeof(cmd), "SET CH_Utilth %s %s %s",cu_thresh2G,cu_thresh5GL, cu_thresh5GH);
    size_t len1=strlen(cmd);
	 char buf[30];
	 size_t len;
	 int ret;
	 len = sizeof(buf) - 1;
	 ret = mapd_interface_ctrl_request(ctrl, cmd, len1, buf, &len,NULL);
	if (ret == -2) {
        return -2;
    } else if (ret < 0) {
        return -1;
    }
    buf[len] = '\0';
    return 0;
}
int mapd_interface_mandate_steer(struct mapd_interface_ctrl *ctrl, char *mac_bh, char *bssid )
{
	char cmd[50];
	int res;
	res = os_snprintf(cmd, sizeof(cmd), "MANDATE_STEER %s %s", mac_bh, bssid);
    size_t len1=strlen(cmd);
	 char buf[30];
	 size_t len;
	 int ret;
	 len = sizeof(buf) - 1;
	 ret = mapd_interface_ctrl_request(ctrl, cmd, len1, buf, &len,NULL);
	if (ret == -2) {
        return -2;
    } else if (ret < 0) {
        return -1;
    }
    buf[len] = '\0';
    return 0;
}
int mapd_interface_bh_steer(struct mapd_interface_ctrl *ctrl, char *mac_bh, char *bssid )
{
	char cmd[50];
	int res;
	res = os_snprintf(cmd, sizeof(cmd), "BHSTEER %s %s", mac_bh, bssid);
    size_t len1=strlen(cmd);
	 char buf[30];
	 size_t len;
	 int ret;
	 len = sizeof(buf) - 1;
	 ret = mapd_interface_ctrl_request(ctrl, cmd, len1, buf, &len,NULL);
	if (ret == -2) {
        return -2;
    } else if (ret < 0) {
        return -1;
    }
    buf[len] = '\0';
    return 0;
}
int mapd_interface_bh_ConnectionStatus(struct mapd_interface_ctrl *ctrl,char *conn_status)
{
	char cmd[20];
	int res;
	res = os_snprintf(cmd, sizeof(cmd), "BH_CONN_STATUS");
	size_t len1=strlen(cmd);
	char buf[30];
	size_t len;
	int ret;
	len = sizeof(buf) - 1;
	ret = mapd_interface_ctrl_request(ctrl, cmd, len1, buf, &len,NULL);
	if (ret == -2){
		return -2;
	} else if (ret < 0){
		return -1;
	}
	*conn_status=buf[0];
	buf[len] = '\0';
	return 0;
}

int mapd_interface_bh_ConnectionType(struct mapd_interface_ctrl *ctrl,char *conn_type)
{
	char cmd[20];
	int res;
	res = os_snprintf(cmd, sizeof(cmd), "BH_CONN_TYPE");
	size_t len1=strlen(cmd);
	 char buf[30];
	 size_t len;
	 int ret;
	 len = sizeof(buf) - 1;
	 ret = mapd_interface_ctrl_request(ctrl, cmd, len1, buf, &len,NULL);
	if (ret == -2) {
        return -2;
     else if (ret < 0) {
        return -1;
    }
	*conn_type=buf[0];
	buf[len] = '\0';
    return 0;
}

#endif
int mapd_interface_Set_Steer(struct mapd_interface_ctrl *ctrl,char *set_steer)
{
	char cmd[20];
	int res;
	res = os_snprintf(cmd, sizeof(cmd), "STEER_EN_DIS %s",set_steer);
	size_t len1=strlen(cmd);
	 char buf[30];
	 size_t len;
	 int ret;
	 len = sizeof(buf) - 1;
	 ret = mapd_interface_ctrl_request(ctrl, cmd, len1, buf, &len,NULL);
	if (ret == -2) {
        return -2;
    } else if (ret < 0) {
        return -1;
    }	
	buf[len] = '\0';
    return 0;
}
#ifdef SUPPORT_MULTI_AP
int mapd_interface_config_renew(struct mapd_interface_ctrl *ctrl)
{
	char cmd[20];
	int res;
	res = os_snprintf(cmd, sizeof(cmd), "CONFIG_RENEW");
	size_t len1=strlen(cmd);
	 char buf[30];
	 size_t len;
	 int ret;
	len = sizeof(buf) - 1;
	 ret = mapd_interface_ctrl_request(ctrl, cmd, len1, buf, &len,NULL);
	if (ret == -2) {
        return -2;
    } else if (ret < 0) {
        return -1;
    }
    return 0;
}

int mapd_interface_set_renew(struct mapd_interface_ctrl *ctrl)
{
	char cmd[20];
	int res;
	res = os_snprintf(cmd, sizeof(cmd), "renew");
	size_t len1=strlen(cmd);
	 char buf[30];
	 size_t len;
	 int ret;
	 len = sizeof(buf) - 1;
	 ret = mapd_interface_ctrl_request(ctrl, cmd, len1, buf, &len,NULL);
	if (ret == -2) {
        return -2;
    } else if (ret < 0) {
        return -1;
    }
	buf[len] = '\0';
    return 0;
}

int mapd_interface_get_bh_ap(struct mapd_interface_ctrl *ctrl,char *list)
{
	char cmd[20];
	int res;
	res = os_snprintf(cmd, sizeof(cmd), "GET_BH_if_AP");
	size_t len1=strlen(cmd);
	char *buf=os_malloc(MAX_STR_SIZE_MAC);
	if(buf==NULL)
		return -1;
	 size_t len;
	 int ret;
	 len = MAX_STR_SIZE_MAC;
	 ret = mapd_interface_ctrl_request(ctrl, cmd, len1, buf, &len,NULL);
	if (ret == -2) {
        os_free(buf);
        return -2;
    } else if (ret < 0) {
        os_free(buf);
        return -1;
    }
	memcpy(list, buf, len);
	//printf("interface %s , length %d\n", list, len );
	os_free(buf);
    return 0;
}

int mapd_interface_get_fh_ap(struct mapd_interface_ctrl *ctrl,char *list)
{
	char cmd[20];
	int res;
	res = os_snprintf(cmd, sizeof(cmd), "GET_FH_if_AP");
	size_t len1=strlen(cmd);
	char *buf=os_malloc(MAX_STR_SIZE_MAC);
	if(buf==NULL)
		return -1;
	 size_t len;
	 int ret;
	 len = MAX_STR_SIZE_MAC;
	 ret = mapd_interface_ctrl_request(ctrl, cmd, len1, buf, &len,NULL);
	if (ret == -2) {
        os_free(buf);
        return -2;
    } else if (ret < 0) {
        os_free(buf);
        return -1;
    }
	memcpy(list, buf, len);
	//printf("interface %s , length %d\n", list, len );
	os_free(buf);
    return 0;
}
int mapd_interface_forceChSwitch(struct mapd_interface_ctrl *ctrl, char *almac, char *channel )
{
	char cmd[100];
	int res;
	res = os_snprintf(cmd, sizeof(cmd), "force_ch_switch %s %s", almac, channel);
	size_t len1=strlen(cmd);
	char buf[50];
	size_t len;
	int ret;
	len = sizeof(buf) - 1;
	ret = mapd_interface_ctrl_request(ctrl, cmd, len1, buf, &len,NULL);
	if (ret == -2) {
		return -2;
	} else if (ret < 0) {
		return -1;
	}
	buf[len] = '\0';
	return 0;
}
int mapd_interface_off_ch_scan_req(struct mapd_interface_ctrl *ctrl, char *almac, char *band)
{
	char cmd[100];
	int res;
	res = os_snprintf(cmd, sizeof(cmd), "off_ch_scan_req %s %s", almac, band);
	size_t len1=strlen(cmd);
	char buf[50];
	size_t len;
	int ret;
	len = sizeof(buf) - 1;
	ret = mapd_interface_ctrl_request(ctrl, cmd, len1, buf, &len,NULL);
	if (ret == -2) {
		return -2;
	} else if (ret < 0) {
		return -1;
	}
	buf[len] = '\0';
	return 0;
}
int mapd_interface_off_ch_scan_req_noise(struct mapd_interface_ctrl *ctrl, int argc, char *argv[])
{
	char cmd[200];
	int res;
	os_memset(cmd, 0, sizeof(cmd));
	res = os_snprintf(cmd, sizeof(cmd), "get_ch_scan_noise %s ", argv[0]);
	size_t len1=strlen(cmd);
	char buf[50];
	size_t len;
	int ret;
	int i = 0;
	len = sizeof(buf) - 1;

	for (i = 1; i < argc; i++) {
		res = os_snprintf(&cmd[len1], sizeof(cmd), "%s ", argv[i]);
		len1 = strlen(cmd);
	}
	ret = mapd_interface_ctrl_request(ctrl, cmd, len1, buf, &len,NULL);
	if (ret == -2) {
		return -2;
	} else if (ret < 0) {
		return -1;
	}
	buf[len] = '\0';
	return 0;
}

int mapd_interface_off_ch_scan_result(struct mapd_interface_ctrl *ctrl, char *almac)
{
	char cmd[100];
	int res;
	res = os_snprintf(cmd, sizeof(cmd), "off_ch_scan_result %s", almac);
	size_t len1=strlen(cmd);
	char buf[50];
	size_t len;
	int ret;
	len = sizeof(buf) - 1;
	ret = mapd_interface_ctrl_request(ctrl, cmd, len1, buf, &len,NULL);
	if (ret == -2) {
		return -2;
	} else if (ret < 0) {
		return -1;
	}
	buf[len] = '\0';
	return 0;
}

int mapd_interface_set_txpower_percentage(struct mapd_interface_ctrl *ctrl, char *almac, char *bandIdx, char* txpower_percentage)
{
	char cmd[100];
	int res;
	always("%s\n", __func__);
	res = os_snprintf(cmd, sizeof(cmd), "set_txpower_percentage %s %s %s", almac, bandIdx, txpower_percentage);
	size_t len1=strlen(cmd);
	char buf[50];
	size_t len;
	int ret;
	len = sizeof(buf) - 1;
	ret = mapd_interface_ctrl_request(ctrl, cmd, len1, buf, &len,NULL);
	if (ret == -2) {
		return -2;
	} else if (ret < 0) {
		return -1;
	}
	buf[len] = '\0';
	return 0;
}
int mapd_interface_get_de_dump(struct mapd_interface_ctrl *ctrl, char *topo_buf, size_t *buf_len, char *file_path_)
{
	char cmd[100] = {0};
	char reply[4096];
	size_t len = sizeof(reply);
	char *pos, *end;
	int ret = 0, index = 1;
	unsigned char found = 0;
	int i = 0, fsize;
	FILE *fptr = NULL;
	char file_path[30] = {0};

	pos = topo_buf;
	end = topo_buf + *buf_len;

	if(file_path_ == NULL)
		strcpy(file_path, dump_de_file_path);
	else
		strcpy(file_path, (const char *)file_path_);
	os_memset(cmd, 0, sizeof(cmd));
	os_snprintf(cmd, sizeof(cmd), "get_de_stats_dump %s", file_path);
	len = sizeof(reply);
	if (mapd_interface_ctrl_request(ctrl, cmd, os_strlen(cmd), reply, &len, NULL) < 0) {
		return 0;
	}
	fptr = fopen(file_path, "r");
	if(fptr == NULL) {
		printf("Error!");
		//exit(1);
		return 0;
	}
	fseek(fptr, 0, SEEK_END);
	fsize = ftell(fptr);
	if (fsize >= max_file_size) {
		printf("topo_buf not enough space!!\n");
		fclose(fptr);
		return -1;
	}
	fseek(fptr, 0, SEEK_SET);
	fread(pos, fsize, 1, fptr);
	fclose(fptr);

	*buf_len = strlen(topo_buf);
	return 0;
}

int mapd_interface_trigger_de_dump(struct mapd_interface_ctrl *ctrl, char *al_mac)
{
	char cmd[100] = {0};
	char reply[4096];
	size_t len = sizeof(reply);
	int ret = 0, index = 1;

	os_memset(cmd, 0, sizeof(cmd));
	os_snprintf(cmd, sizeof(cmd), "get_de_stats %s", al_mac);
	len = sizeof(reply);
	if (mapd_interface_ctrl_request(ctrl, cmd, os_strlen(cmd), reply, &len, NULL) < 0) {
		return -1;
	}
}

int mapd_interface_get_ch_score_dump(struct mapd_interface_ctrl *ctrl, char *topo_buf, size_t *buf_len, char *file_path_)
{
	char cmd[100] = {0};
	char reply[4096];
	size_t len = sizeof(reply);
	char *pos, *end;
	int ret = 0, index = 1;
	unsigned char found = 0;
	int i = 0, fsize;
	FILE *fptr = NULL;
	char file_path[30] = {0};

	pos = topo_buf;
	end = topo_buf + *buf_len;

	if(file_path_ == NULL)
		strcpy(file_path, dump_ch_score_dump);
	else
		strcpy(file_path, (const char *)file_path_);
	os_memset(cmd, 0, sizeof(cmd));
	os_snprintf(cmd, sizeof(cmd), "get_ch_plan_score_dump %s", file_path);
	len = sizeof(reply);
	if (mapd_interface_ctrl_request(ctrl, cmd, os_strlen(cmd), reply, &len, NULL) < 0) {
		return 0;
	}
	fptr = fopen(file_path, "r");
	if(fptr == NULL) {
		printf("Error!");
		//exit(1);
		return 0;
	}
	fseek(fptr, 0, SEEK_END);
	fsize = ftell(fptr);
	if (fsize >= max_file_size) {
		printf("topo_buf not enough space!!\n");
		fclose(fptr);
		return -1;
	}
	fseek(fptr, 0, SEEK_SET);
	fread(pos, fsize, 1, fptr);
	fclose(fptr);

	*buf_len = strlen(topo_buf);
	return 0;
}

int mapd_interface_get_ch_scan_dump(struct mapd_interface_ctrl *ctrl, char *topo_buf, size_t *buf_len, char *file_path_)
{
	char cmd[100] = {0};
	char reply[4096];
	size_t len = sizeof(reply);
	char *pos, *end;
	int ret = 0, index = 1;
	unsigned char found = 0;
	int i = 0, fsize;
	FILE *fptr = NULL;
	char file_path[30] = {0};

	pos = topo_buf;
	end = topo_buf + *buf_len;

	if(file_path_ == NULL)
		strcpy(file_path, dump_ch_scan_dump);
	else
		strcpy(file_path, (const char *)file_path_);
	os_memset(cmd, 0, sizeof(cmd));
	os_snprintf(cmd, sizeof(cmd), "get_ch_scan_stats_dump %s", file_path);
	len = sizeof(reply);
	if (mapd_interface_ctrl_request(ctrl, cmd, os_strlen(cmd), reply, &len, NULL) < 0) {
		return 0;
	}
	fptr = fopen(file_path, "r");
	if(fptr == NULL) {
		printf("Error!");
		//exit(1);
		return 0;
	}
	fseek(fptr, 0, SEEK_END);
	fsize = ftell(fptr);
	if (fsize >= max_file_size) {
		printf("topo_buf not enough space!!\n");
		fclose(fptr);
		return -1;
	}
	fseek(fptr, 0, SEEK_SET);
	fread(pos, fsize, 1, fptr);
	fclose(fptr);

	*buf_len = strlen(topo_buf);
	return 0;
}

int mapd_interface_trigger_ch_scan(struct mapd_interface_ctrl *ctrl, char *al_mac)
{
	char cmd[100] = {0};
	char reply[4096];
	size_t len = sizeof(reply);
	int ret = 0, index = 1;

	os_memset(cmd, 0, sizeof(cmd));
	os_snprintf(cmd, sizeof(cmd), "CH_SCAN %s", al_mac);
	len = sizeof(reply);
	if (mapd_interface_ctrl_request(ctrl, cmd, os_strlen(cmd), reply, &len, NULL) < 0) {
		return -1;
	}
}
int mapd_interface_trigger_ch_plan_R2(struct mapd_interface_ctrl *ctrl, char *band)
{
	char cmd[100] = {0};
	char reply[4096];
	size_t len = sizeof(reply);
	int ret = 0, index = 1;

	os_memset(cmd, 0, sizeof(cmd));
	os_snprintf(cmd, sizeof(cmd), "CH_PLAN_R2 %s", band);
	len = sizeof(reply);
	if (mapd_interface_ctrl_request(ctrl, cmd, os_strlen(cmd), reply, &len, NULL) < 0) {
		return -1;
	}
}


#endif
