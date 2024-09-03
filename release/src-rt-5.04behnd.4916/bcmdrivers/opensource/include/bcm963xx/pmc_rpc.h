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
/****************************************************************************
 * PMC RPC Driver
 *
 * Author: Samyon Furman <samyon.furman@broadcom.com>
*****************************************************************************/

#ifndef _PMC_RPC_SVC_H_
#define _PMC_RPC_SVC_H_

#include <itc_rpc.h>

typedef uint8_t(*get_retcode_t)(rpc_msg *);

int pmc_svc_request(rpc_msg *msg, get_retcode_t cb);
int avs_svc_request(rpc_msg *msg, get_retcode_t cb);

#endif


