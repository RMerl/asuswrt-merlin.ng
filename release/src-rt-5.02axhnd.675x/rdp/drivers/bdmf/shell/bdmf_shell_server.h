/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */

/*******************************************************************
 * -mon_server.h
 *
 * BL framework - remote shell support
 *
 * This module is a back-end of remote shell support.
 * - multiple servers
 * - domain and TCP-based connections
 * - session access level - per server
 *******************************************************************/

#ifndef BDMF_MON_SERVER_H_
#define BDMF_MON_SERVER_H_

#include <bdmf_system.h>
#include <bdmf_session.h>
#include <bdmf_shell.h>

/** Shell server transport type
 */
typedef enum {
    BDMFMONS_TRANSPORT_DOMAIN_SOCKET,
    BDMFMONS_TRANSPORT_TCP_SOCKET,

    BDMFMONS_TRANSPORT__NUMBER_OF
} bdmfmons_transport_type_t;

/** Shell server parameters
 */
typedef struct bdmfmons_parm
{
    bdmf_access_right_t access;           /**< Access rights */
    bdmfmons_transport_type_t transport;  /**< Transport type */
    char *address;                      /**< Address in string form: domain socket file in local FS; port for TCP socket */
    int max_clients;                    /**< Max number of clients */
} bdmfmons_parm_t;


/** Create shell server.
 * Immediately after creation server is ready to accept client connections
 * \param[in]   parms   Server parameters
 * \param[out]  hs      Server handle
 * \return  0 - OK\n
 *         <0 - error code
 */
bdmf_error_t bdmfmons_server_create(const bdmfmons_parm_t *parms, int *hs);

/** Destroy shell server.
 * All client connections if any are closed
 * \param[in]   hs      Server handle
 * \return  0 - OK\n
 *         <0 - error code
 */
bdmf_error_t bdmfmons_server_destroy(int hs);


/* Create shell_server directory in root_dir
   Returns the "shell_server" directory handle
*/
bdmfmon_handle_t bdmfmons_server_mon_init(bdmfmon_handle_t root_dir);


/* Destroy shell_server directory
*/
void bdmfmons_server_mon_destroy(void);

#endif /* BDMF_MON_SERVER_H_ */
