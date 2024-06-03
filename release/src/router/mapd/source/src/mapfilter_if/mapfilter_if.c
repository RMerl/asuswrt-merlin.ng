#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <net/if.h>

#include "os.h"
#include "mapfilter_if.h"

static int netlink_sock = 0;
static int netlink_pid = 0;

unsigned char cmd_buf[1024];

int mapfilter_netlink_init(unsigned int pid)
{
    int sock;
    struct sockaddr_nl addr;

    sock = socket(AF_NETLINK, SOCK_RAW, MAP_NETLINK);
    if (sock < 0) {
        printf("sock < 0.\n");
        return -1;
    }

    memset((void *) &addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = pid;
	addr.nl_groups = 0;
    /* This doesn't work for some reason. See the setsockopt() below. */
    /* addr.nl_groups = MYMGRP; */

    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        printf("bind < 0.\n");
	    close(sock);
        return -1;
    }

	netlink_sock = sock;
	netlink_pid = pid;
    return sock;
}

int mapfilter_netlink_deinit(int sock)
{
	close(netlink_sock);
	
	return 0;
}

static int mapfilter_netlink_msg_send(const unsigned char *message, int len, unsigned int dst_group)
{
	struct nlmsghdr *nlh = NULL;
	struct sockaddr_nl dest_addr;
	struct iovec iov;
	struct msghdr msg;

	if(!message ) {
		return -1;
	}

	//create message
	nlh = (struct nlmsghdr *)os_malloc(NLMSG_SPACE(len));
	if (!nlh) {
		printf("allocate nlmsghdr fail\n");
		return -1;
	}
	nlh->nlmsg_len = NLMSG_SPACE(len);
	nlh->nlmsg_pid = netlink_pid;
	nlh->nlmsg_flags = 0;
	os_memcpy(NLMSG_DATA(nlh), message, len);

	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	os_memset(&dest_addr, 0, sizeof(struct sockaddr_nl));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;
	dest_addr.nl_groups = dst_group;

	os_memset(&msg, 0, sizeof(struct msghdr));
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(struct sockaddr_nl);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	//send message
	if (sendmsg(netlink_sock, &msg, 0) < 0) {
		printf("netlink msg send fail\n");
		os_free(nlh);
		return -3;
	}

	os_free(nlh);
	return 0;
}

int mapfilter_set_all_interface(struct local_itfs *itf)
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;
	int len = sizeof(struct local_itfs) + itf->num * sizeof(struct local_interface);
	int ret = 0;

	msg->type = UPDATE_MAP_NET_DEVICE;
	msg->len = len;
	os_memcpy(msg->event, (unsigned char*)itf, len);
	
	ret = mapfilter_netlink_msg_send(cmd_buf, sizeof(struct map_netlink_message) + len, 0);
	return ret;
}

int mapfilter_set_primary_interface(struct local_interface *itf, unsigned char primary)
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;
	struct primary_itf_setting *setting = (struct primary_itf_setting*)msg->event;
	int len = sizeof(struct primary_itf_setting);
	int ret = 0;
	
	if(!itf) {
		printf("itf is NULL");
		return 0;
	}
	printf("itf is %s", itf->name);
	msg->type = SET_PRIMARY_INTERFACE;
	os_memcpy(&setting->inf, itf, sizeof(struct local_interface));
	setting->primary = primary;
	msg->len = len;
	
	ret = mapfilter_netlink_msg_send(cmd_buf, sizeof(struct map_netlink_message) + len, 0);
	return ret;
}

int mapfilter_set_uplink_path(struct local_interface *in, struct local_interface *out)
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;
	struct up_link_path_setting *setting = (struct up_link_path_setting*)msg->event;
	int len = 0, ret = 0;

	printf("IN=%s ", in->name);
	if (out)
		printf("OUT=%s\n", out->name);
	else
		printf("OUT=NULL\n");

	os_memset(setting, 0, sizeof(struct up_link_path_setting));
	msg->type = SET_UPLINK_PATH_ENTRY;
	os_memcpy(&setting->in, in, sizeof(struct local_interface));
	if (out)
		os_memcpy(&setting->out, out, sizeof(struct local_interface));
	
	len = sizeof(struct up_link_path_setting);
	msg->len = len;

	ret = mapfilter_netlink_msg_send(cmd_buf, sizeof(struct map_netlink_message) + len, 0);
	return ret;
}

int mapfilter_dump_debug_info()
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;

	int ret = 0;
	
	msg->type = DUMP_DEBUG_INFO;

	ret = mapfilter_netlink_msg_send(cmd_buf, sizeof(struct map_netlink_message), 0);
	return ret;
}

int mapfilter_set_apcli_link_status(struct local_interface *apcli_inf, unsigned char link_status)
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;
	struct apcli_link_status *status= (struct apcli_link_status*)msg->event;
	int len = 0, ret = 0;

	os_memset(status, 0, sizeof(struct apcli_link_status));
	msg->type = UPDATE_APCLI_LINK_STATUS;
	os_memcpy(&status->in, apcli_inf, sizeof(struct local_interface));
	status->link_status = link_status;
	
	len = sizeof(struct apcli_link_status);
	msg->len = len;

	ret = mapfilter_netlink_msg_send(cmd_buf, sizeof(struct map_netlink_message) + len, 0);
	return ret;
}

int mapfilter_set_channel_utilization(unsigned char *radio_id, unsigned char band, unsigned char util)
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;
	struct radio_utilization *ch_util= (struct radio_utilization*)msg->event;
	int len = 0, ret = 0;

	os_memset(ch_util, 0, sizeof(struct radio_utilization));
	msg->type = UPDATE_CHANNEL_UTILIZATION;
	os_memcpy(&ch_util->radio_id, radio_id, ETH_ALEN);
	ch_util->band = band;
	ch_util->util = util;
	
	len = sizeof(struct radio_utilization);
	msg->len = len;

	ret = mapfilter_netlink_msg_send(cmd_buf, sizeof(struct map_netlink_message) + len, 0);
	return ret;
}

int mapfilter_enable_dynamic_load_balance(unsigned char enable)
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;
	int len = 0, ret = 0;

	msg->type = DYNAMIC_LOAD_BALANCE;
	msg->event[0] = enable;
	
	len = 1;
	msg->len = len;

	ret = mapfilter_netlink_msg_send(cmd_buf, sizeof(struct map_netlink_message) + len, 0);
	return ret;
}

int mapfilter_set_drop_specific_dest_ip_status(unsigned char drop_flag)
{
	struct map_netlink_message *msg = (struct map_netlink_message*)cmd_buf;
	struct drop_specific_dest_ip_status *dp_dest_ip= (struct drop_specific_dest_ip_status*)msg->event;
	int len = 0, ret = 0;

	os_memset(dp_dest_ip, 0, sizeof(struct drop_specific_dest_ip_status));
	msg->type = SET_DROP_SPECIFIC_IP_PACKETS_STATUS;
	dp_dest_ip->drop_flag = drop_flag;

	len = sizeof(struct drop_specific_dest_ip_status);
	msg->len = len;
	
	ret = mapfilter_netlink_msg_send(cmd_buf, sizeof(struct map_netlink_message) + len, 0);
	return ret;
}

