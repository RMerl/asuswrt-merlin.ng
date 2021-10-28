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

/*
*******************************************************************************
* File Name  : tcpspdtest_defs.h
*
* Description: This file contains the specification of some common definitions
*      and interfaces to other modules. This file may be included by both
*      Kernel and userapp (C only).
*
*******************************************************************************
*/

#ifndef __TCPSPDTEST_DEFS_INCLUDED_H__
#define __TCPSPDTEST_DEFS_INCLUDED_H__

#include <linux/socket.h>
#include <linux/types.h>
#include "spdt_defs.h"

/******************************************** Defines ********************************************/
#define TCPSPDTEST_GENL_MAX_FILE_NAME_LEN  256

#define TCPSPDTEST_GENL_FAMILY_NAME        "TCPSPDTEST" /* Max Family name length is 16 chars including \0 */
#define TCPSPDTEST_GENL_MAX_MSG_LEN        768
#define TCPSPDTEST_DEF_STREAM_ID           0 /* Used when multi-stream mode is not in use */

/******************************************** Types **********************************************/

/* Tcp Speed Test GENL */

typedef enum
{
    /* Request + Response commands */
    TCPSPDTEST_GENL_CMD_CONNECT,
    TCPSPDTEST_GENL_CMD_DISCONNECT,
    TCPSPDTEST_GENL_CMD_RELEASE,
    TCPSPDTEST_GENL_CMD_PING,
    TCPSPDTEST_GENL_CMD_DNLD,
    TCPSPDTEST_GENL_CMD_UPLOAD,
    TCPSPDTEST_GENL_CMD_STATS,
    TCPSPDTEST_GENL_CMD_SPEED_REPORT,
    TCPSPDTEST_GENL_CMD_OOB_SEND,
    TCPSPDTEST_GENL_CMD_STREAM_IDX,
    TCPSPDTEST_GENL_CMD_PROTOCOL,
    TCPSPDTEST_GENL_CMD_STREAM_PARAMS,
    TCPSPDTEST_GENL_CMD_NUM_STREAMS,
    UDPSPDTEST_GENL_CMD_INIT,
    UDPSPDTEST_GENL_CMD_UNINIT,
} spdtest_genl_cmd_t;

typedef enum
{
    /* Commad parameter */
    TCPSPDTEST_GENL_CMD_PARAM_SET,
    TCPSPDTEST_GENL_CMD_PARAM_GET,
    TCPSPDTEST_GENL_CMD_PARAM_ALLOC,
    TCPSPDTEST_GENL_CMD_PARAM_FREE
} tcpspdtest_genl_cmd_param_t;

typedef spdtest_genl_cmd_t tcpspdtest_genl_cmd_t;

typedef enum
{
    TCPSPDTEST_GENL_CMD_STATUS_OK = 0,
    TCPSPDTEST_GENL_CMD_STATUS_IN_PROCESS,
    TCPSPDTEST_GENL_CMD_STATUS_ERR,
} tcpspdtest_genl_cmd_status_t;

/* TCPSPDTEST GENL families */
typedef enum
{
    TCPSPDTEST_GENL_FAMILY_MSG_UNSPEC,
    TCPSPDTEST_GENL_FAMILY_MSG_OWN_MSG,
    TCPSPDTEST_GENL_FAMILY_MSG_MAX,
} tcpspdtest_genl_family_t;

/* TCPSPDTEST GENL Policies */
typedef enum
{
    TCPSPDTEST_GENL_POLICY_UNSPEC,
    TCPSPDTEST_GENL_POLICY_OWN_MSG,
    TCPSPDTEST_GENL_POLICY_MAX,
} tcpspdtest_genl_policy_t;

typedef struct
{
    uint8_t                      stream_idx;
    tcpspdtest_genl_cmd_t        cmd;
    tcpspdtest_genl_cmd_param_t  cmd_param;
    spdt_stream_params_t         stream_params;
    uint64_t                     dn_up_size;
    char                         file_name[TCPSPDTEST_GENL_MAX_FILE_NAME_LEN];
} __attribute__ ((packed)) tcpspdtest_genl_req_msg_t;

typedef struct
{
    uint8_t                      stream_idx;
    uint8_t                      num_streams;
    uint8_t                      num_udp_streams;
    tcpspdtest_genl_cmd_t        cmd;
    union
    {
        tcp_spdt_rep_t           spd_report;
        spdt_stream_params_t     stream_params;
    } msg;
    tcpspdtest_genl_cmd_status_t status;
}  __attribute__ ((packed)) tcpspdtest_genl_resp_msg_t;

#endif /* __TCPSPDTEST_DEFS_INCLUDED_H__ */
