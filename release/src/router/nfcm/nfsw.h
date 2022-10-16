#ifndef __NFSW__H__
#define __NFSW__H__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>

#if 0
#include <bcmnvram.h>
#include <shared.h>

#if defined(RTCONFIG_EXT_RTL8365MB) || defined(RTCONFIG_EXT_RTL8370MB)
#include <linux/major.h>
#include <rtk_switch.h>
#include <rtk_types.h>
#endif

#if defined(CONFIG_ET)
#include <proto/ethernet.h>
#include <sys/ioctl.h>
#include <linux/ethtool.h>
#include <etioctl.h>
#include <etc.h>
#endif
#endif

#include "list.h"
#include "log.h"

#if defined(RTCONFIG_EXT_BCM53134) || defined(HND)
#include <bcmswapitypes.h>
#endif

typedef struct sw_node_s {
    bool is_wl;
    uint8_t port;
	char mac[ETHER_ADDR_LENGTH];

    // for wl guest network
    bool is_guest;
    char ifname[IFNAMESIZE];

	struct list_head list;
} sw_node_t;

extern sw_node_t *sw_node_new();
extern void sw_node_free(sw_node_t *sw);
extern void sw_list_free(struct list_head *list);
extern int sw_list_parse(struct list_head *list);

extern int nfcm_get_lan_ports(int port);

#if defined(CONFIG_ET)
extern int bcm_arl_dump_5301x(ethsw_mac_table *emt);
#endif

#if defined(RTCONFIG_EXT_RTL8365MB) || defined(RTCONFIG_EXT_RTL8370MB)
extern int rtkswitch_arl_dump_port_mac(int port, struct list_head *swlist);
#endif

#endif // __NFSW__H__
