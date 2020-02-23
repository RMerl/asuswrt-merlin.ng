/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2013:proprietary:standard
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
 * :>
 *
 * $Change: 111969 $
 ***********************************************************************/

#if defined(WIRELESS)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <net/if.h>

#include "i5ctl.h"
#include "i5ctl_wlcfg.h"
#include "ieee1905_wlcfg.h"
#include "ieee1905_utils.h"
#include "ieee1905_socket.h"
#include "ieee1905_glue.h"

#include <wps_1905.h>

void convertMac(char *macstr,unsigned char *macs)
{
    int macsBytes=(strlen(macstr)+1)/18 *6;
    char *ptr=strtok(macstr,":");
    char *end;
    int i=0;
    while(ptr)
    {
        if(i>=macsBytes) break;
        macs[i++]=strtol(ptr,&end,16);
        ptr=strtok(NULL,":");
    }
}

t_I5_CTL_WLCFG_CMD i5CtlWlCfgCmds[] =
{
    {"wscstatus", "Check wsc status", I5_CTL_WLCFG_WSCSTATUS},
    {"wsccancel", "Stop current wsc session", I5_CTL_WLCFG_WSCCANCEL},
    {"wscenabled", "[interface] Check if wsc is enabled", I5_CTL_WLCFG_WSCENABLED},
    {"apconfigured", "[interface] Check if AP is configured", I5_CTL_WLCFG_APISCONFIGURED},
    {"wlinstances", "Retrieve the number of wireless interfaces", I5_CTL_WLCFG_WLINSTANCE},
    {"getwllinks", "[interface] Retrieve wireless links", I5_CTL_WLMETRIC_GETWLLINKS},
    {"macsmetric", "<maclist xx:xx:xx:xx:xx:xx> [interface]", I5_CTL_WLMETRIC_LINKMETRIC},

#if defined(DSLCPE_WLCSM_EXT)
    {"nvram", "nvram [get|set|unset|commit|trace]", I5_CTL_WLCFG_NVRAM}
#endif // endif
};

static t_I5_CTL_WLCFG_CMD *i5CtlWlCfgCmdLookup(const char *cmdName)
{

    int i;
    for (i = 0; i < (sizeof(i5CtlWlCfgCmds)/sizeof(i5CtlWlCfgCmds[0])); i++)
    {
        if (!strcmp(cmdName, i5CtlWlCfgCmds[i].cmdstr))
        {
            return &i5CtlWlCfgCmds[i];
        }
    }
    return NULL;
}

#if defined(DSLCPE_WLCSM_EXT)
t_I5_CTL_WLCFG_CMD i5CtlNvramCmds[] = {
    {"get",    "nvram get",     I5_CTL_WLCFG_NVRAM_GET},
    {"set",    "nvram set",     I5_CTL_WLCFG_NVRAM_SET},
    {"unset",  "nvram set",     I5_CTL_WLCFG_NVRAM_UNSET},
    {"commit", "nvram commit",  I5_CTL_WLCFG_NVRAM_COMMIT},
    {"trace",  "nvram trace",   I5_CTL_WLCFG_NVRAM_TRACE}
};

static t_I5_CTL_WLCFG_CMD *i5CtlWlcfgNvramCmdLookup(const char *cmdName)
{
    int i;
    for (i = 0; i < (sizeof(i5CtlNvramCmds)/sizeof(i5CtlNvramCmds[0])); i++) {
        if (!strcmp(cmdName, i5CtlNvramCmds[i].cmdstr)) {
            return &i5CtlNvramCmds[i];
        }
    }
    return NULL;
}
#endif // endif

static void i5CtlWlCfgShowUsage()
{
    int i;
    printf("Usage: wlcfg <option>\n" );
    for (i = 0; i < (sizeof(i5CtlWlCfgCmds)/sizeof(i5CtlWlCfgCmds[0])); i++)
    {
        printf("        %s  -%s\n", i5CtlWlCfgCmds[i].cmdstr, i5CtlWlCfgCmds[i].description);
    }
}

int i5CtlWlCfgCmdHandler(void *pCmd, int argc, char *argv[])
{
    t_I5_CTL_CMD          *pI5Cmd = (t_I5_CTL_CMD *)pCmd;
    t_I5_API_WLCFG_MSG     msg;
    t_I5_API_WLCFG_MSG    *pMsg;
    int                    sd;
    int                    rc;
    char                  *pBuf=NULL;
    t_I5_CTL_WLCFG_CMD    *wlcmd=NULL;
    int                    len;
    int                    count;
    int                    i;
#if defined(DSLCPE_WLCSM_EXT)
    int                    dataLen;
#endif // endif

    if(argc<3)
    {
        i5CtlWlCfgShowUsage();
        return 0;
    }

    wlcmd = i5CtlWlCfgCmdLookup(argv[2]);
    if ( NULL == wlcmd )
    {
        i5CtlWlCfgShowUsage();
        return 0;
    }

    len = sizeof(t_I5_API_WLCFG_MSG);
    msg.ifname[0] = '\0';
    msg.status = 0;
    msg.subcmd = wlcmd->cmd;
    pMsg=&msg;

    switch ( wlcmd->cmd ) {
    case I5_CTL_WLCFG_WSCSTATUS:
    case I5_CTL_WLCFG_WSCCANCEL:
    case I5_CTL_WLCFG_WLINSTANCE:
        break;

    case I5_CTL_WLCFG_WSCENABLED:
    case I5_CTL_WLCFG_APISCONFIGURED:
    case I5_CTL_WLMETRIC_GETWLLINKS:
        if ( argc == 3 ) {
            snprintf(msg.ifname, I5_MAX_IFNAME, "%s0", I5_GLUE_WLCFG_WL_NAME_STRING);
        }
        else {
            strncpy(msg.ifname, argv[3], I5_MAX_IFNAME-1);
            msg.ifname[I5_MAX_IFNAME-1] = '\0';
        }
        break;

    case I5_CTL_WLMETRIC_LINKMETRIC:
    {
        int index;
        if(argc < 4)
        {
            i5CtlWlCfgShowUsage();
            return 0;
        }

        /* check if the last argument is an interface */
        rc = if_nametoindex(argv[argc-1]);
        if ( 0 == rc ) {
            count = argc - 3;
        }
        else {
            count = argc - 4;
        }
        len = sizeof(t_I5_API_WLCFG_MSG)+(count*MAC_ADDR_LEN)+sizeof(unsigned int);
        pBuf = malloc(len);
        if ( pBuf == NULL ) {
            return 0;
        }
        pMsg = (t_I5_API_WLCFG_MSG*)pBuf;
        pMsg->status = 0;
        pMsg->subcmd = wlcmd->cmd;
        if ( 0 == rc ) {
            snprintf(pMsg->ifname, I5_MAX_IFNAME, "%s0", I5_GLUE_WLCFG_WL_NAME_STRING);
        }
        else {
            strncpy(pMsg->ifname, argv[argc-1], I5_MAX_IFNAME-1);
            pMsg->ifname[I5_MAX_IFNAME-1] = '\0';
        }
        *(unsigned int *)(pBuf+sizeof(t_I5_API_WLCFG_MSG)) = count;
        index = sizeof(unsigned int)+sizeof(t_I5_API_WLCFG_MSG);
        for(i = 3; i < (count + 3); i++)
        {
           if (strlen(argv[i]) < (I5_MAC_STR_BUF_LEN-1))
           {
               continue;
           }
           convertMac(argv[i], (unsigned char *)(pBuf+index));
           index += MAC_ADDR_LEN;
        }
        break;
    }

#if defined(DSLCPE_WLCSM_EXT)
    case I5_CTL_WLCFG_NVRAM:
        if ( argc < 5 )
        {
            i5CtlWlCfgShowUsage();
            return 0;
        }

        wlcmd = i5CtlWlcfgNvramCmdLookup(argv[3]);
        if ( NULL == wlcmd )
        {
            i5CtlWlCfgShowUsage();
            return 0;
        }
        else
        {
            msg.subcmd = wlcmd->cmd;
            switch ( wlcmd->cmd ) {
            case I5_CTL_WLCFG_NVRAM_SET:
            case I5_CTL_WLCFG_NVRAM_UNSET:
            case I5_CTL_WLCFG_NVRAM_GET:
            case I5_CTL_WLCFG_NVRAM_TRACE:
                len=strlen(argv[4])+1;
                pBuf = malloc(sizeof(t_I5_API_WLCFG_MSG)+len);
                if ( pBuf == NULL ) {
                    return 0;
                }
                memset(pBuf,'\0',len+sizeof(t_I5_API_WLCFG_MSG));
                len += sizeof(t_I5_API_WLCFG_MSG);
                memcpy(pBuf,&msg,sizeof(t_I5_API_WLCFG_MSG));
                pMsg = (t_I5_API_WLCFG_MSG*)pBuf;
                memcpy(pMsg+1,argv[4],strlen(argv[4]));
                break;
            case I5_CTL_WLCFG_NVRAM_COMMIT:
                break;
            default:
                return 0;
            }
        }
        break;
#endif // endif
    default:
        return 0;
    }

    sd = i5apiOpen();
    if (-1 == sd)
    {
        if (pBuf != NULL) {
          free(pBuf);
        }
        return sd;
    }

    rc = i5apiSendMessage(sd, pI5Cmd->cmd, pMsg, len);
    if (pBuf != NULL) {
      free(pBuf);
      pBuf = NULL;
    }
    if ( -1 == rc )
    {
        printf("Failed to send data to daemon\n");
    }
    else
    {
        /* wait for a response from the Daemon */
        rc = i5apiWait(sd, 2);
        if ( -1 == rc )
        {
            printf("failed to get response from 1905 Daemon\n");
        }
        else
        {
            rc = i5apiRecvResponse(sd, (void **)&pBuf, 0);
            if ( -1 == rc ) {
              printf("failed to receive data from 1905 Daemon\n");
            }
            else {
              pMsg = (t_I5_API_WLCFG_MSG*)pBuf;
              if(pMsg->status<0)
              {
                  printf("Cmd:%d execution failure:%d\n", pMsg->subcmd, pMsg->status);
                  free(pBuf);
                  return pMsg->status;
              }
#if defined(DSLCPE_WLCSM_EXT)
              dataLen = rc - sizeof(t_I5_API_WLCFG_MSG);
#endif // endif
              switch(pMsg->subcmd)
              {
              case  I5_CTL_WLCFG_WSCSTATUS:
                  if(pMsg->status==WPS_1905_WSCAP_SESSIONONGOING)
                  {
                      printf("WSC session ongoing.....\n");
                  }
                  else if(pMsg->status==WPS_1905_WSCAP_UNCONFIGURED)
                  {
                      printf(" WSC is unconfigured and no session\n");
                  }
                  else
                  {
                      printf(" WSC is configured and no session\n");
                  }
                  break;
              case  I5_CTL_WLCFG_WSCCANCEL:
                  if(pMsg->status)
                      printf("WSC session cancelled\n");
                  else
                      printf("Failed to cancel WSC session\n");
                  break;
              case I5_CTL_WLCFG_WSCENABLED:
                  if(pMsg->status==WPS_1905_WSCAP_DISABLED)
                  {
                      printf("WSC is disabled\n");
                  }
                  else
                  {
                      printf("WSC is enabled\n");
                  }
                  break;
              case  I5_CTL_WLCFG_WLINSTANCE:
                  if(pMsg->status>0)
                      printf(" There are %d wireless instances\r\n",pMsg->status);
                  else
                      printf(" There in no any wireless instances\r\n");
                  break;
              case  I5_CTL_WLCFG_APISCONFIGURED:
                  if(pMsg->status>0)
                      printf(" AP is configured\r\n");
                  else
                      printf(" AP is not configured\r\n");
                  break;
                  break;
              case I5_CTL_WLMETRIC_GETWLLINKS:
              case I5_CTL_WLMETRIC_LINKMETRIC:
                  printf("%s\r\n",(char *)(pMsg+1));
                  break;

  #if defined(DSLCPE_WLCSM_EXT)
              case I5_CTL_WLCFG_NVRAM_GET:
                  if(dataLen)
                      printf("%s\r\n",(char *)(pMsg+1));
                  else
                      printf("null\r\n");
                  break;
              case I5_CTL_WLCFG_NVRAM_SET:
              case I5_CTL_WLCFG_NVRAM_UNSET:
              case I5_CTL_WLCFG_NVRAM_TRACE:
              case I5_CTL_WLCFG_NVRAM_COMMIT:
                  break;
  #endif
              default:
                  printf("Unsupported wlcfg subcommand:%d\n",pMsg->subcmd);
                  break;
              }
              free(pBuf);
            }
        }
    }
    i5apiClose(sd);

    return 0;
}
#endif // endif
