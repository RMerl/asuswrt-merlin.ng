/*
   Copyright (c) 2020 Broadcom Corporation
   All Rights Reserved

<:label-BRCM:2020:DUAL/GPL:standard

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

#ifndef _PON_DRV_NL_H_
#define _PON_DRV_NL_H_

#include "wan_drv.h" /* for serdes_wan_type_t */

#define NETLINK_SERDES_VI_GROUP 29
#define NETLINK_MSG_MAX_LEN 224

int serdes_vi_nl_create(void);
int netlink_invoke_serdes_job(const char *msg);
/* it is the job requester responsibility to make sure that the requested job doesn't exceed the memory allocation for results */
int netlink_invoke_serdes_job_to(const char *msg, uint32_t *results, const uint64_t job_timeout_ns);
void serdes_vi_nl_release(void);
#endif
