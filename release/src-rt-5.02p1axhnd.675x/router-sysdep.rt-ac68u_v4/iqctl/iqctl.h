/***********************************************************************
 *
 *  Copyright (c) 2010  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
************************************************************************/

/*
 *******************************************************************************
 * File Name  : iqctl.h
 * Description: Command line parsing for the Broadcom Ingress QoS 
 * Control Utility
 *******************************************************************************
 */

/*** Includes. ***/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
/*** Defines. ***/

/* Limit values */
#define CMD_NAME_LEN                            24
#define MAX_OPTS                                32
#define MAX_SUB_CMDS                            16
#define MAX_PARMS                               16

/* Argument type values. */
#define ARG_TYPE_COMMAND                        1
#define ARG_TYPE_OPTION                         2
#define ARG_TYPE_PARAMETER                      3

/* Return codes. */
#define IQCTL_GENERAL_ERROR                 100
#define IQCTL_NOT_FOUND                     101
#define IQCTL_ALLOC_ERROR                   102
#define IQCTL_INVALID_COMMAND               103

#define IQCTL_INVALID_OPTION                104
#define IQCTL_INVALID_PARAMETER             105
#define IQCTL_INVALID_NUMBER_OF_OPTIONS     106
#define IQCTL_INVALID_NUMBER_OF_PARAMETERS  107

/*** Typedefs. ***/

typedef struct
{
    char    * pOptName;
    char    * pParms[MAX_PARMS];
    int     nNumParms;
} OPTION_INFO, *POPTION_INFO;

typedef int (*FN_COMMAND_HANDLER) (POPTION_INFO pOptions, int nNumOptions);

typedef struct
{
    char    szCmdName[CMD_NAME_LEN];
    char    *pszOptionNames[MAX_OPTS];
    FN_COMMAND_HANDLER pfnCmdHandler;
} COMMAND_INFO, *PCOMMAND_INFO;

extern char g_PgmName[];
extern COMMAND_INFO g_Cmds[];

/*** Common Command Line Parsing Prototypes. ***/

extern void Usage(void);
extern int  GetArgType(char *pszArg, PCOMMAND_INFO pCmds, char **ppszOptions);
extern PCOMMAND_INFO GetCommand(char *pszArg, PCOMMAND_INFO pCmds);
extern int  ProcessCommand(PCOMMAND_INFO pCmd, int argc, char **argv,
                           PCOMMAND_INFO pCmds, int *pnArgNext);
extern void DumpOption(char *pszCmdName, POPTION_INFO pOptions,int nNumOptions);


