/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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

/*!\file tcpspdtest_api.h
 * \brief Header file for Tcp Speed Test activation.
 *
 * Main API functions for Tcp Speed Test activation.
 *
 */

#ifndef __TCPSPDTEST_API_H__
#define __TCPSPDTEST_API_H__

#include <stdbool.h>
#include "tcpspdtest_defs.h"

/******************************************** Functions ******************************************/
int tcpspdtest_init(int *nl_fd);
int tcpspdtest_shutdown(int nl_fd);
int tcpspdtest_connect(int nl_fd, tcpspdtest_genl_protocol_t protocol, tcpspd_connection_params_t *params);
int tcpspdtest_disconnect(int nl_fd);
int tcpspdtest_download(int nl_fd, tcpspdtest_genl_protocol_t protocol, uint64_t size, char *file_name);
int tcpspdtest_upload(int nl_fd, tcpspdtest_genl_protocol_t protocol, uint64_t size);
int tcpspdtest_get_speed(int nl_fd, bool req_spd_report, tcpspdtest_spd_report_t *spd_report, tcpspdtest_genl_cmd_status_t *spd_status);

#endif /* __TCPSPDTEST_API_H__ */
