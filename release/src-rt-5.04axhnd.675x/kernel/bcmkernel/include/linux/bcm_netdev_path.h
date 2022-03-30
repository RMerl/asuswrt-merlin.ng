#ifndef __BCM_NETDEV_PATH_H_INCLUDED__
#define __BCM_NETDEV_PATH_H_INCLUDED__


/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

/* Forward declaration */
struct net_device;

#define NETDEV_PATH_HW_SUBPORTS_MAX  CONFIG_BCM_MAX_GEM_PORTS
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
