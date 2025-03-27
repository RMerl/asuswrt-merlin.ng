/*
   Copyright (c) 2020 Broadcom Corporation
   All Rights Reserved

<:label-BRCM:2020:DUAL/GPL:standard

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


#define NETLINK_EYESCOPE_GROUP 30

int eyescope_nl_create(void);
void eyescope_nl_release(void);
int eyescope_nl_send_messgae(const char *msg);
#endif
