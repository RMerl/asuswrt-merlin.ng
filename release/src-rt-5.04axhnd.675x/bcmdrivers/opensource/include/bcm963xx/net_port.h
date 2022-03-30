/*
    Copyright 2007-2015 Broadcom Corporation

    <:label-BRCM:2015:DUAL/GPL:standard
    
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

/***********************************************************************/
/*                                                                     */
/*   MODULE:  net_port.h                                               */
/*   PURPOSE: Generic port name interface.                             */
/*                                                                     */
/***********************************************************************/
#ifndef _NET_PORT_H
#define _NET_PORT_H

typedef struct net_port_t
{
    int port;
    int sub_type;
    int is_wan;
    int speed;
    char ifname[32];
} net_port_t;

#define _FOREACH_NET_PORT_TYPE(TYPE, TYPE0) \
    TYPE0(NET_PORT_LAN_0) \
    TYPE(NET_PORT_LAN_1) \
    TYPE(NET_PORT_LAN_2) \
    TYPE(NET_PORT_LAN_3) \
    TYPE(NET_PORT_LAN_4) \
    TYPE(NET_PORT_LAN_5) \
    TYPE(NET_PORT_LAN_6) \
    TYPE(NET_PORT_LAN_7) \
    TYPE(NET_PORT_EPON_AE) \
    TYPE(NET_PORT_EPON) \
    TYPE(NET_PORT_GPON) \
    TYPE(NET_PORT_LAST) \
    TYPE(NET_PORT_NONE)

#define _FOREACH_NET_PORT_SUBTYPE_TYPE(TYPE, TYPE0) \
    TYPE0(NET_PORT_SUBTYPE_GPON) \
    TYPE(NET_PORT_SUBTYPE_XGPON) \
    TYPE(NET_PORT_SUBTYPE_XGS) \
    TYPE(NET_PORT_SUBTYPE_NGPON) \
    TYPE(NET_PORT_SUBTYPE_LAST) \
    TYPE(NET_PORT_SUBTYPE_NONE)
    
#define _FOREACH_NET_PORT_SPEED_TYPE(TYPE, TYPE0) \
    TYPE0(NET_PORT_SPEED_0101) \
    TYPE(NET_PORT_SPEED_0201) \
    TYPE(NET_PORT_SPEED_0202) \
    TYPE(NET_PORT_SPEED_2501) \
    TYPE(NET_PORT_SPEED_1001) \
    TYPE(NET_PORT_SPEED_1025) \
    TYPE(NET_PORT_SPEED_1010) \
    TYPE(NET_PORT_SPEED_LAST) \
    TYPE(NET_PORT_SPEED_NONE)

#define FOREACH_NET_PORT_ENUM \
    _FOREACH_NET_PORT_TYPE(GENERATE_ENUM, GENERATE_ENUM0) \
    _FOREACH_NET_PORT_SUBTYPE_TYPE(GENERATE_ENUM, GENERATE_ENUM0) \
    _FOREACH_NET_PORT_SPEED_TYPE(GENERATE_ENUM, GENERATE_ENUM0)

#define FOREACH_NET_PORT_STRING \
    _FOREACH_NET_PORT_TYPE(GENERATE_STRING, GENERATE_STRING) \
    _FOREACH_NET_PORT_SUBTYPE_TYPE(GENERATE_STRING, GENERATE_STRING) \
    _FOREACH_NET_PORT_SPEED_TYPE(GENERATE_STRING, GENERATE_STRING)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_ENUM0(ENUM) ENUM = 0,
#define GENERATE_STRING(STRING) #STRING,

enum
{
    FOREACH_NET_PORT_ENUM
};

#endif

