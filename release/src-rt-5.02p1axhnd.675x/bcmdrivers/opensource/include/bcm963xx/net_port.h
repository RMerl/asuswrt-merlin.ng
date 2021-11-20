/*
    Copyright 2007-2015 Broadcom Corporation

    <:label-BRCM:2015:DUAL/GPL:standard
    
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

