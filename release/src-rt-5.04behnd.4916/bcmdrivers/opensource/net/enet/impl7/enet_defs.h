/*
   <:copyright-BRCM:2022:DUAL/GPL:standard

     Copyright (c) 2022 Broadcom
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