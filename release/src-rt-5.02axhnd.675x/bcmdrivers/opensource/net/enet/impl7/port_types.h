/*
   <:copyright-BRCM:2015:DUAL/GPL:standard
   
      Copyright (c) 2015 Broadcom 
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

