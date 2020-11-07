#ifndef __BCM_DSLCPE_WLAN_INFO_H_
#define __BCM_DSLCPE_WLAN_INFO_H_
#include <linux/netdevice.h>
#include <linux/blog.h>
#define WLAN_CLIENT_INFO_OK (0)
#define WLAN_CLIENT_INFO_ERR (-1)
typedef enum {
    WLAN_CLIENT_TYPE_CPU,
    WLAN_CLIENT_TYPE_WFD,
    WLAN_CLIENT_TYPE_RUNNER,
    WLAN_CLIENT_TYPE_MAX
} wlan_client_type_t;

typedef struct  {
    wlan_client_type_t type;
    union {
        uint32_t        wl;
        BlogWfd_t       wfd;
        BlogRnr_t       rnr;
    };
} wlan_client_info_t;


typedef int (* wlan_client_get_info_t)(struct net_device *dev,char *mac_address_p,int priority, wlan_client_info_t *info_p);

#endif
