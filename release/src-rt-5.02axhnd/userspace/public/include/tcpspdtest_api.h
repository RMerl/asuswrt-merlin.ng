/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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
