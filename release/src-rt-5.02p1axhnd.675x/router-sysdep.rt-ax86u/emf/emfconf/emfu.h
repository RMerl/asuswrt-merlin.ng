/*
 * Copyright 2019 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: emfu.h 465707 2014-03-28 03:56:34Z $
 */

#ifndef _EMFU_H_
#define _EMFU_H_

#define	EMF_ARGC_ENABLE_FWD             2
#define	EMF_ARGC_DISABLE_FWD            2
#define	EMF_ARGC_GET_FWD                2
#define	EMF_ARGC_ADD_BRIDGE             3
#define	EMF_ARGC_DEL_BRIDGE             3
#define	EMF_ARGC_LIST_BRIDGE            3
#define	EMF_ARGC_ADD_IF                 4
#define	EMF_ARGC_DEL_IF                 4
#define	EMF_ARGC_LIST_IF                3
#define	EMF_ARGC_ADD_UFFP               4
#define	EMF_ARGC_DEL_UFFP               4
#define	EMF_ARGC_LIST_UFFP              3
#define	EMF_ARGC_ADD_RTPORT             4
#define	EMF_ARGC_DEL_RTPORT             4
#define	EMF_ARGC_LIST_RTPORT            3
#define	EMF_ARGC_ADD_MFDB               5
#define	EMF_ARGC_DEL_MFDB               5
#define	EMF_ARGC_LIST_MFDB              3
#define	EMF_ARGC_CLEAR_MFDB             3
#define	EMF_ARGC_SHOW_STATS             3

#define EMF_USAGE \
"Usage: emf  start   <bridge>\n"\
"            stop    <bridge>\n"\
"            status  <bridge>\n"\
"            add     bridge  <bridge>\n"\
"            del     bridge  <bridge>\n"\
"            add     iface   <bridge>  <if-name>\n"\
"            del     iface   <bridge>  <if-name>\n"\
"            list    iface   <bridge>\n"\
"            add     uffp    <bridge>  <if-name>\n"\
"            del     uffp    <bridge>  <if-name>\n"\
"            list    uffp    <bridge>\n"\
"            add     rtport  <bridge>  <if-name>\n"\
"            del     rtport  <bridge>  <if-name>\n"\
"            list    rtport  <bridge>\n"\
"            add     mfdb    <bridge>  <group-ip>  <if-name>\n"\
"            del     mfdb    <bridge>  <group-ip>  <if-name>\n"\
"            list    mfdb    <bridge>\n"\
"            clear   mfdb    <bridge>\n"\
"            show    stats   <bridge>\n"

typedef struct emf_cmd_arg
{
	char *cmd_oper_str;             /* Operation type string */
	char *cmd_id_str;               /* Command id string */
	int  (*input)(char *[]);        /* Command process function */
	int  arg_count;                 /* Arguments count */
} emf_cmd_arg_t;

#endif /* _EMFU_H_ */
