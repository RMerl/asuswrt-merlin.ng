/*
   <:copyright-BRCM:2015:DUAL/GPL:standard
   
      Copyright (c) 2015 Broadcom 
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

/*
 *  Created on: Nov/2015
 *      Author: ido@broadcom.com
 */

#ifndef _PORT_TYPES_H_
#define _PORT_TYPES_H_

#define FOREACH_PORT_TYPE(TYPE) \
    TYPE(PORT_TYPE_RUNNER_SW) \
    TYPE(PORT_TYPE_RUNNER_PORT) \
    TYPE(PORT_TYPE_RUNNER_GPON) \
    TYPE(PORT_TYPE_RUNNER_EPON) \
    TYPE(PORT_TYPE_DETECT) \
    TYPE(PORT_TYPE_RUNNER_MAC) \
    TYPE(PORT_TYPE_RUNNER_WIFI) \
    TYPE(PORT_TYPE_G9991_SW) \
    TYPE(PORT_TYPE_G9991_PORT) \
    TYPE(PORT_TYPE_G9991_ES_PORT) /* Error sampling SID port, RX only */ \
    TYPE(PORT_TYPE_VLAN_SW) \
    TYPE(PORT_TYPE_VLAN_PORT) \
    TYPE(PORT_TYPE_DIRECT_RGMII) \
    TYPE(PORT_TYPE_GENERIC_DMA) \
    TYPE(PORT_TYPE_SF2_SW) \
    TYPE(PORT_TYPE_SF2_PORT) \
    TYPE(PORT_TYPE_SF2_MAC) \
    TYPE(PORT_TYPE_SYSP_SW) \
    TYPE(PORT_TYPE_SYSP_PORT) \
    TYPE(PORT_TYPE_SYSP_MAC)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum
{
    FOREACH_PORT_TYPE(GENERATE_ENUM)
} port_type_t;

#endif

