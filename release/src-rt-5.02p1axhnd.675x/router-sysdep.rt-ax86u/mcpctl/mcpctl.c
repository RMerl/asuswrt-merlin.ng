/*
* <:copyright-BRCM:2011:proprietary:standard
* 
*    Copyright (c) 2011 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
:>
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <asm/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include "mcpctl.h"

static int mcpdCtlGenericHandler(void *pCmd, int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    t_MCPD_CTL_CMD *pMcpdCmd = (t_MCPD_CTL_CMD *)pCmd;
    int             sd;
    int             rc = 0;

    sd = mcpd_api_open();
    if (sd < 0) {
      return sd;
    }
    rc = mcpd_api_send_message(sd, pMcpdCmd->cmd, 0, 0);
    mcpd_api_close(sd);

    return rc;
}

static int mcpdCtlGrpModeHandler(void *pCmd, int argc __attribute__((unused)), char *argv[])
{
    t_MCPD_CTL_CMD      *pMcpdCmd = (t_MCPD_CTL_CMD *)pCmd;
    t_MCPD_CTL_GRP_MODE  msg;
    int                  sd;
    int                  rc = 0;

    if (strcmp(argv[2], "firstin") == 0)
    {
        msg.mode = MCPD_CTL_CMD_MCGRPMODE_FIRST_IN;
    }
    else if (strcmp(argv[2], "iana") == 0)
    {
        msg.mode = MCPD_CTL_CMD_MCGRPMODE_IANA;
    }
    else
    {
        return -1;
    }

    sd = mcpd_api_open();
    if (sd < 0) {
      return sd;
    }
    rc = mcpd_api_send_message(sd, pMcpdCmd->cmd, &msg, sizeof(msg));
    mcpd_api_close(sd);

    return rc;
}

static int mcpdCtlAdmissionHandler(void *pCmd, int argc __attribute__((unused)), char *argv[])
{
    t_MCPD_CTL_CMD      *pMcpdCmd = (t_MCPD_CTL_CMD *)pCmd;
    t_MCPD_CTL_ADMISSION msg;
    int                  sd;
    int                  rc = 0;

    if (strcmp(argv[2], "on") == 0)
    {
        msg.enable = 1;
    }
    else if (strcmp(argv[2], "off") == 0)
    {
        msg.enable = 0;
    }
    else
    {
        return -1;
    }

    sd = mcpd_api_open();
    if (sd < 0) {
      return sd;
    }
    rc = mcpd_api_send_message(sd, pMcpdCmd->cmd, &msg, sizeof(msg));
    mcpd_api_close(sd);

    return rc;
}

t_MCPD_CTL_CMD mcpctl_cmds[] = {
    {"objinfo",   "Display mcpd object tree information.", MCPD_CTL_CMD_OBJINFO, mcpdCtlGenericHandler, 0},
    {"meminfo",   "Display mcpd object memory usage information.", MCPD_CTL_CMD_MEMINFO, mcpdCtlGenericHandler, 0},
    {"mcgrpmode", "Set Multicast Group rules for categorizing ASM vs SSM. (iana|firstin)", MCPD_CTL_CMD_MCGRPMODE, mcpdCtlGrpModeHandler, 1},
    {"admission", "Set Admission Filter on for Bridging interfaces (off|on)", MCPD_CTL_CMD_ADMISSION, mcpdCtlAdmissionHandler, 1},
    {"configinfo","Display mcpd configuration.", MCPD_CTL_CMD_CONFIGINFO, mcpdCtlGenericHandler, 0},
    {"reload",    "Reload mcpd configuration.", MCPD_CTL_CMD_RELOAD, mcpdCtlGenericHandler, 0},
    {"allinfo",   "Display all information", MCPD_CTL_CMD_ALLINFO, mcpdCtlGenericHandler, 0},
};

void usage(char *prog_name)
{
    unsigned int i;

    printf("Usage: %s <option>\n", prog_name);
    for (i = 0; i < (sizeof(mcpctl_cmds) / sizeof(t_MCPD_CTL_CMD)); i++)
    {
        printf("        %-12s  -%s\n", mcpctl_cmds[i].cmdstr, mcpctl_cmds[i].description);
    }
}

t_MCPD_CTL_CMD *mcpdCtlCmdLookup(const char *cmdName)
{
    unsigned int i;
    for (i = 0; i < (sizeof(mcpctl_cmds)/sizeof(mcpctl_cmds[0])); i++) {
        if (!strcmp(cmdName, mcpctl_cmds[i].cmdstr)) {
            return &mcpctl_cmds[i];
        }
    }

    return NULL;
}

/***************************************************************************
 * Function Name: main
 * Description  : This utility sends user selectable control messages to
 *                mcpd for diagnostic purposes.
 ***************************************************************************/
#ifdef BUILD_STATIC
int mcpctl_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    t_MCPD_CTL_CMD *pCmd;
    if (argc == 1) 
    {
        usage(argv[0]);
        return -EINVAL;
    }

    pCmd = mcpdCtlCmdLookup(argv[1]);
    if (pCmd == NULL) {
        printf("unknown command [%s]\n", argv[1]);
        usage(argv[0]);
        return -EINVAL;
    }

    if ( (unsigned int) argc < (pCmd->nargs + 1)) {
        printf("incorrect number of arguments\n");
        usage(argv[0]);
        return -EINVAL;
    }

    /* found a match - call handler */
    pCmd->func((void *)pCmd, argc, &argv[0]);
  
    return 0;

}

