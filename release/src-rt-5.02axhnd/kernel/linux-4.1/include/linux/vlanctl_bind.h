#if defined(CONFIG_BCM_KF_VLANCTL_BIND)
/*
*    Copyright (c) 2003-2014 Broadcom Corporation
*    All Rights Reserved
*
<:label-BRCM:2014:DUAL/GPL:standard 

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/


#ifndef _VLANCTL_BIND_
#define _VLANCTL_BIND_

typedef enum {
#if defined(CONFIG_BCM_KF_FAP)
        VLANCTL_BIND_CLIENT_FAP,
#endif
#if defined(CONFIG_BCM_KF_RUNNER)
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
        VLANCTL_BIND_CLIENT_RUNNER,
#endif /* CONFIG_BCM_RUNNER */
#endif /* CONFIG_BCM_KF_RUNNER */
        VLANCTL_BIND_CLIENT_MAX
} vlanctl_bind_client_t;


/*
 * vlanctl_bind defines three(!) hooks:
 *  NotifHook: When blog_notify is invoked, the bound hook is invoked. Based on
 *           event type the bound Blog client may perform a custom action.
 *  SC Hook: If this hook is defined, blog_activate() will pass a blog with
 *           necessary information for statical configuration.
 *  SD Hook: If this hook is defined, blog_deactivate() will pass a pointer
 *           to a network object with BlogActivateKey information. The
 *           respective flow entry will be deleted.
 */
typedef union {
    struct {
        uint8_t         unused      : 5;
        uint8_t         SN_HOOK     : 1;
        uint8_t         SC_HOOK     : 1;
        uint8_t         SD_HOOK     : 1;
    } bmap;
    uint8_t             hook_info;
} vlanctl_bind_t;

typedef struct {
    struct net_device *vlan_dev;
    unsigned int vid;
    int enable;
} vlanctl_vlan_t;

typedef struct {
    uint8_t mac[6];
    int enable;
} vlanctl_route_mac_t;

typedef struct {
    struct net_device *aggregate_vlan_dev;
    struct net_device *deaggregate_vlan_dev;
} vlanctl_vlan_aggregate_t;

typedef enum {
        VLANCTL_BIND_NOTIFY_TPID,       /* set interface tpid */
        VLANCTL_BIND_NOTIFY_VLAN,       /* set vlan object */
        VLANCTL_BIND_NOTIFY_ROUTE_MAC,  /* route mac create and delete */
        VLANCTL_BIND_NOTIFY_VLAN_AGGREGATE,       /* set vlan aggregation */        
} vlanctl_bind_Notify_t;


typedef uint32_t (* vlanctl_bind_ScHook_t)(Blog_t * blog_p, BlogTraffic_t traffic);

typedef Blog_t * (* vlanctl_bind_SdHook_t)(uint32_t key, BlogTraffic_t traffic);

typedef void     (* vlanctl_bind_SnHook_t)(vlanctl_bind_Notify_t event, void *ptr);

void vlanctl_bind_config(vlanctl_bind_ScHook_t vlanctl_bind_sc, 
	                     vlanctl_bind_SdHook_t vlanctl_bind_sd,  
	                     vlanctl_bind_SnHook_t vlanctl_bind_sn,  
	                     vlanctl_bind_client_t client, 
                         vlanctl_bind_t bind);


int vlanctl_bind_activate(vlanctl_bind_client_t client);

int	vlanctl_notify(vlanctl_bind_Notify_t event, void *ptr, vlanctl_bind_client_t client);

/*
 *------------------------------------------------------------------------------
 *  vlanctl_activate(): static configuration function of blog application
 *             pass a filled blog to the hook for configuration
 *------------------------------------------------------------------------------
 */
extern uint32_t vlanctl_activate( Blog_t * blog_p,  vlanctl_bind_client_t client );

/*
 *------------------------------------------------------------------------------
 *  vlanctl_deactivate(): static deconfiguration function of blog application
 *------------------------------------------------------------------------------
 */
extern Blog_t * vlanctl_deactivate( uint32_t key,  vlanctl_bind_client_t client );


#endif /* ! _VLANCTL_BIND_ */
#endif
