#ifndef _NETLINK_EVENT_H
#define _NETLINK_EVENT_H

#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */

/*---------------------------------->
consistent in both kernel prog and user prog
*/

#define MAP_NETLINK 26
#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

enum _1905_NETLINK_CMD_TYPE {
	GET_SWITCH_PORT_STATUS,
};

/*common net link data structure, all 1905 netlink event should align this structure*/
struct GNU_PACKED _1905_netlink_message {
	unsigned char type;
	unsigned short len;
	unsigned char event[0];
};

enum DEVICE_TYPE {
	AP = 0,
	APCLI,
	ETH,
};

#define INF_UNKNOWN		0x00
#define INF_PRIMARY		0x01
#define INF_NONPRIMARY	0x02

#define _24G 0x01
#define _5GL 0x02
#define _5GH 0x04
#define _5G	0x06

enum MAP_NETLINK_EVENT_TYPE {
	UPDATE_MAP_NET_DEVICE = 0,
	SET_PRIMARY_INTERFACE,
	SET_UPLINK_PATH_ENTRY,
	DUMP_DEBUG_INFO,
	UPDATE_APCLI_LINK_STATUS,
	UPDATE_CHANNEL_UTILIZATION,
	DYNAMIC_LOAD_BALANCE,
	SET_DROP_SPECIFIC_IP_PACKETS_STATUS,
#ifdef MAP_R2
	SET_TRAFFIC_SEPARATION_DEFAULT_8021Q,
	SET_TRAFFIC_SEPARATION_POLICY,
	SET_TRANSPARENT_VID,
	UPDATE_CLIENT_VID,
#endif
};


struct GNU_PACKED map_netlink_message {
	unsigned char type;
	unsigned short len;
	unsigned char event[0];
};

struct GNU_PACKED local_interface {
	char name[IFNAMSIZ];
	unsigned char mac[ETH_ALEN];
	enum DEVICE_TYPE dev_type;
	unsigned char band;
};

struct GNU_PACKED local_itfs {
	unsigned char num;
	struct local_interface inf[0];
};

struct GNU_PACKED primary_itf_setting {
	struct local_interface inf;
	unsigned char primary;
};

struct GNU_PACKED up_link_path_setting {
	struct local_interface in;
	struct local_interface out;
};

struct GNU_PACKED apcli_link_status {
	struct local_interface in;
	unsigned char link_status;
};

struct GNU_PACKED radio_utilization {
	unsigned char radio_id[ETH_ALEN];
	unsigned char band;
	unsigned char util;
};

struct GNU_PACKED drop_specific_dest_ip_status {
	unsigned char drop_flag;
};

/*
 * mapfilter_netlink_init -This function is used to initialize the netlink between user space 
 * application and mapfilter
 * @pid: current process identity
 * return value should be larger than 0 if succeed, otherwise fail
*/
int mapfilter_netlink_init(unsigned int pid);

/*
 * mapfilter_netlink_deinit - This function is used to deinitialize the netlink between user
 * space application and mapfilter
*/
int mapfilter_netlink_deinit();

/*
 * mapfilter_set_all_interface -This function is used to update all local interfaces to
 * mapfilter
 * @itf: all local interfaces
 * return 0 if succed, otherwise fail.
*/
int mapfilter_set_all_interface(struct local_itfs *itf);

/*
 * mapfilter_set_primary_interface -This function is used to set a local interface as
 * a (non)primary interface to mapfilter
 * @itf: all local interfaces
 * @primary: 1-set the specific interface as primary interface. 0-set the specific 
 * interface as non-primary interface
 * return 0 if succed, otherwise fail.
*/
int mapfilter_set_primary_interface(struct local_interface *itf, unsigned char primary);

/*
 * mapfilter_set_uplink_path -This function is used control the mapfilter function  that
 * is used to forward a data packet to the @out local interface which is received from
 * @in local interface
 * @in: the local interface from which a packet is received
 * @out: the local interface to which a packet should be forwarded.
 * return 0 if succed, otherwise fail.
*/
int mapfilter_set_uplink_path(struct local_interface *in, struct local_interface *out);

/*
 * mapfilter_set_apcli_link_status -This function is used notify the apcli link status
 * to mapfilter
 * @apcli_inf: apcli interface
 * @link_status:1-link is up; 0-link is down
 * return 0 if succed, otherwise fail.
*/
int mapfilter_set_apcli_link_status(struct local_interface *apcli_inf, unsigned char link_status);

/*
 * mapfilter_set_channel_utilization -This function is used notify the channel
 * utilizations to mapfilter
 * @radio_id: radio identity
 * @band:band of current radio
 * @util: channel utilzation
 * return 0 if succed, otherwise fail.
*/
int mapfilter_set_channel_utilization(unsigned char *radio_id, unsigned char band, unsigned char util);

/*
 * mapfilter_set_channel_utilization -This function is used enable/disable
 * dynamic loading balance in mapfilter
 * @enable: 1-enable dynamic loading balance; 0-disable dynamic loading balance
 * return 0 if succed, otherwise fail.
*/
int mapfilter_enable_dynamic_load_balance(unsigned char enable);

/*
 * mapfilter_drop_specific_dest_ip_status -This function is used to drop specific ip packets to avoid two controllers.
 * drop specific ip packets in mapfilter
 * @drop_flag: 1-start drop the drop specific ip packets; 0-stop drop the drop specific ip packets
 * return 0 if succed, otherwise fail.
*/
int mapfilter_set_drop_specific_dest_ip_status(unsigned char drop_flag);


#endif

