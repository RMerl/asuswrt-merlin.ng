/*
   <:copyright-BRCM:2022:DUAL/GPL:standard
   
      Copyright (c) 2022 Broadcom 
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
/**
 * @file enet_defs.h
 * @author Yoel Tulatov (yoel.tulatov@broadcom.com)
 * @brief Inter space common enet definitions
 * @date 2023-03-29
 */

#ifndef ___ENETDEFS_H___
#define ___ENETDEFS_H___

#include <linux/netlink.h>

#ifndef __KERNEL__
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#endif

#define NETLINK_UNSUBSCTIBE_GPC_PID ((uint32_t)-1)

#define GENL_GPC_FAMILY_NAME "genl_gpc_family"
#define GENL_GPC_MCGRP0_NAME "genl_gpc_mcgrp0"

#define GENL_GPC_ATTR_MSG_MAX 256

enum
{
  GENL_GPC_C_UNSPEC,
  GENL_GPC_C_MSG,
};

enum genl_test_attrs
{
  GENL_GPC_ATTR_UNSPEC,
  GENL_GPC_ATTR_MSG,
  __GENL_GPC_ATTR__MAX,
};

#define GENL_GPC_ATTR_MAX (__GENL_GPC_ATTR__MAX - 1)

struct nla_policy genl_gpc_policy[GENL_GPC_ATTR_MAX + 1] = {
    [GENL_GPC_ATTR_MSG] = {
        .type = NLA_STRING,
#ifdef __KERNEL__
        .len = GENL_GPC_ATTR_MSG_MAX
#else
        .maxlen = GENL_GPC_ATTR_MSG_MAX
#endif
    },
};

#endif /*___ENETDEFS_H___*/