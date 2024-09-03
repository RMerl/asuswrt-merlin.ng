#ifndef __BCM_NETDEV_PATH_H_INCLUDED__
#define __BCM_NETDEV_PATH_H_INCLUDED__


/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

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

/* Forward declaration */
struct net_device;

#if defined(CONFIG_BCM_GPON_MODULE)
#define NETDEV_PATH_HW_SUBPORTS_MAX  CONFIG_BCM_MAX_GEM_PORTS
#else
#define NETDEV_PATH_HW_SUBPORTS_MAX  0
#endif
struct netdev_path
{
        /* this reference counter indicates the number of interfaces
           referencing this interface */
        int refcount;
        /* indicates the RX hardware port number associated to the
           interface */
        unsigned int hw_port;
        /* indicates the TX hardware port number associated to the
           interface */
        unsigned int hw_tx_port;
        /* hardware port type, must be set to one of the types defined in
           BlogPhy_t  */
        unsigned int hw_port_type;
        /* some device drivers support virtual subports within a hardware
		   port. hw_subport_mcast is used to map a multicast hw subport
		   to a hw port. */
        unsigned int hw_subport_mcast_idx;
};

/* Returns TRUE when _dev is a member of a path, otherwise FALSE */
#define netdev_path_is_linked(_dev) ( netdev_path_next_dev(_dev) != NULL )

/* Returns TRUE when _dev is the leaf in a path, otherwise FALSE */
#define netdev_path_is_leaf(_dev) ( (_dev)->bcm_nd_ext.path.refcount == 0 )

/* Returns TRUE when _dev is the root of a path, otherwise FALSE. The root
   device is the physical device */
#define netdev_path_is_root(_dev) ( netdev_path_next_dev(_dev) == NULL )

#define netdev_path_set_hw_port(_dev, _hw_port, _hw_port_type)  \
    do {                                                        \
        (_dev)->bcm_nd_ext.path.hw_port = (_hw_port);                      \
        (_dev)->bcm_nd_ext.path.hw_tx_port = (_hw_port);                   \
        (_dev)->bcm_nd_ext.path.hw_port_type = (_hw_port_type);            \
    } while(0)

#define netdev_path_set_hw_port_only(_dev, _hw_port)            \
    do {                                                        \
        (_dev)->bcm_nd_ext.path.hw_port = (_hw_port);                      \
    } while(0)

#define netdev_path_set_hw_tx_port_only(_dev, _hw_port)         \
    do {                                                        \
        (_dev)->bcm_nd_ext.path.hw_tx_port = (_hw_port);                   \
    } while(0)

#define netdev_path_get_hw_port(_dev) ( (_dev)->bcm_nd_ext.path.hw_port )

#define netdev_path_get_hw_tx_port(_dev) ( (_dev)->bcm_nd_ext.path.hw_tx_port )

#define netdev_path_get_hw_port_type(_dev) ( (_dev)->bcm_nd_ext.path.hw_port_type )
#define netdev_path_set_hw_port_type(_dev, _hwt) do { (_dev)->bcm_nd_ext.path.hw_port_type = _hwt; } while (0)

#define netdev_path_get_hw_subport_mcast_idx(_dev) ( (_dev)->bcm_nd_ext.path.hw_subport_mcast_idx )

/* Returns a pointer to the next device in a path, towards the root
   (physical) device */
struct net_device *netdev_path_next_dev(struct net_device *dev);

struct net_device *netdev_path_get_root(struct net_device *dev);

int netdev_path_set_hw_subport_mcast_idx(struct net_device *dev, unsigned int subport_idx);

int netdev_path_add(struct net_device *new_dev, struct net_device *next_dev);

int netdev_path_remove(struct net_device *dev);

void netdev_path_dump(struct net_device *dev);


#endif /* __BCM_NETDEV_PATH_H_INCLUDED__ */
