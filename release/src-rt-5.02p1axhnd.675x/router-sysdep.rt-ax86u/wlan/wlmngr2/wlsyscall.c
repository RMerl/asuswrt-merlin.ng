/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <netinet/if_ether.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "board.h"

#include "cms_util.h"
#include "mdm.h"
#include "mdm_private.h"
#include "odl.h"
#include "cms_boardcmds.h"
#include "cms_boardioctl.h"
#include "cms_core.h"
#include <bcmnvram.h>

 #include "wlsyscall.h"

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_packet.h>

#include <pwd.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <wlcsm_lib_api.h>
#define READ_BUF_SIZE        128



/* Tmp function moved from syscall.c */


int bcmSystemEx (char *command, int printFlag) 
{
   
#ifdef DSLCPE_WLCSM_EXT
int i=0;
	i=system(command);
	return i;
#else
   int pid = 0, status = 0;
      char *argv[4];

   if ( command == 0 )
      return 1;

   pid = fork();
   if ( pid == -1 )
      return -1;

   if ( pid == 0 ) {
      argv[0] = "sh";
      argv[1] = "-c";
      argv[2] = command;
      argv[3] = 0;
#ifdef BRCM_DEBUG
      if (printFlag)
         printf("app: %s\r\n", command);
#endif
      execv("/bin/sh", argv);
      exit(127);
   }

   /* wait for child process return */
   do {
      if ( waitpid(pid, &status, 0) == -1 ) {
         if ( errno != EINTR )
            return -1;
      } else
         return status;
   } while ( 1 );

   return status;
#endif
}


//**************************************************************************
// Function Name: bcmIsMarkStrChar
// Description  : verify the given character is used to mark the begining or
//                ending of string or not.
// Parameters   : c -- the given character.
// Returns      : TRUE or FALSE.
//**************************************************************************
int bcmIsMarkStrChar(char c) {
   // need to add '\0' as termination character to speChars[]
   char specChars[] = { '\'', '"', '\\', '\0' };
   int len = strlen(specChars);
   int i = 0;
   int ret = FALSE;

   for ( i = 0; i < len; i++ )
      if ( c == specChars[i] )
         break;

   if ( i < len )
      ret = TRUE;

   return ret;
}
//**************************************************************************
// Function Name: bcmProcessMarkStrChars
// Description  : use backslash in front one of the escape codes to process
//                marked string characters.
//                (a'b"c => a\'b\"c)
// Parameters   : str - the string that needs to process its special chars.
// Returns      : none.
//**************************************************************************
void bcmProcessMarkStrChars(char *str) {
   if ( str == NULL ) return;
   if ( str[0] == '\0' ) return;

   char buf[BUFLEN_256];
   int len = strlen(str);
   int i = 0, j = 0;

   for ( i = 0; i < len; i++ ) {
      if ( bcmIsMarkStrChar(str[i]) == TRUE )
         buf[j++] = '\\';
      buf[j++] = str[i];
   }

   buf[j] = '\0';
   strcpy(str, buf);
}


char *bcmGetWlName(int idx, int ssid_idx, char *ifcName)
{
   if ( !ifcName ) 
   	return NULL;

   if (idx == 0)
         sprintf(ifcName, "wl%d", idx);
   else
         sprintf(ifcName, "wl%d.%d",idx, ssid_idx);

   return ifcName;
}

int bcmGetWlIdxByName(char *ifcName) 
{
   struct ifreq ifr;
   int s = 0;

   if ( !ifcName ) 
   	return -1;

   if ( (s = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
   	return -1;

   strncpy(ifr.ifr_name, ifcName, sizeof(ifr.ifr_name));
   
   if ( ioctl(s, SIOCGIFINDEX, &ifr) < 0 ) {
      close(s);
      return -1;
   }

   close(s);

   return ifr.ifr_ifindex;
}

int bcmIsValidWlName(char *ifcName) 
{
   int ret = FALSE;

   if ( bcmGetWlIdxByName(ifcName)!= -1 )
      ret = TRUE;
   return ret;
}

int bcmMacNumToStr(char *macAddr, char *str) {
   if ( macAddr == NULL ) return FALSE;
   if ( str == NULL ) return FALSE;

   sprintf(str, "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
           (UINT8) macAddr[0], (UINT8) macAddr[1], (UINT8) macAddr[2],
           (UINT8) macAddr[3], (UINT8) macAddr[4], (UINT8) macAddr[5]);

   return TRUE;
}

int bcmMacStrToNum(char *macAddr, char *str) {
   char *pToken = NULL, *pLast = NULL;
   char *buf;
   UINT16 i = 1;
   int len;
   
   if ( macAddr == NULL ) return FALSE;
   if ( str == NULL ) return FALSE;

   len = strlen(str) + 1;
   if (len > 20)
     len = 20;
   buf = (char*)malloc(len);
   memset(buf,0,len);

   if ( buf == NULL ) return FALSE;

   /* need to copy since strtok_r updates string */
   strncpy(buf, str,len-1);

   /* Mac address has the following format
       xx:xx:xx:xx:xx:xx where x is hex number */
   pToken = strtok_r(buf, ":", &pLast);
   macAddr[0] = (char) strtol(pToken, (char **)NULL, 16);
   for ( i = 1; i < 6; i++ ) {
      pToken = strtok_r(NULL, ":", &pLast);
      macAddr[i] = (char) strtol(pToken, (char **)NULL, 16);
   }
   free(buf);
   return TRUE;
}


void bcmConvertStrToShellStr(char *str, char *buf) {
   if ( buf == NULL ) return;

   int len = strlen(str);
   int i = 0, j = 0;

   for ( i = 0; i < len; i++ ) {
      if ( str[i] != '\'' ) {
         buf[j++] = '\'';
         buf[j++] = str[i];
         buf[j++] = '\'';
      } else {
         buf[j++] = '"';
         buf[j++] = str[i];
         buf[j++] = '"';
      }
   }

   buf[j]  = '\0';
}


void remove_delimitor( char *s)
{
    char *p1, *p2;

    p1 = p2 = s;
    while ( *p1 != '\0' || *(p1+1) != '\0') {
        if (*p1 != '\0') {
           *p2 = *p1;
           p2++;
         }
         p1++;
    }
    *p2='\0';

}
/* find_pid_by_name()
 *
 *  This finds the pid of the specified process.
 *  Currently, it's implemented by rummaging through
 *  the proc filesystem.
 *
 *  Returns a list of all matching PIDs
 */
pid_t* find_pid_by_name( char* pidName)
{
        DIR *dir;
        struct dirent *next;
        pid_t* pidList=NULL;
        int i=0;

        /*FILE *status */
        FILE *cmdline;
        char filename[NAME_MAX+16];
        char buffer[READ_BUF_SIZE];
        /* char name[READ_BUF_SIZE]; */
                
        dir = opendir("/proc");
        if (!dir) {
                printf("cfm:Cannot open /proc");
                return NULL;
        }

        while ((next = readdir(dir)) != NULL) {
                /* re-initialize buffers */
                memset(filename, 0, sizeof(filename));
                memset(buffer, 0, sizeof(buffer));  

                /* Must skip ".." since that is outside /proc */
                if (WLCSM_STRCMP(next->d_name, "..") == 0)
                        continue;

                /* If it isn't a number, we don't want it */
                if (!isdigit(*next->d_name))
                        continue;

                /* snprintf(filename, sizeof(filename), "/proc/%s/status", next->d_name); */
                /* read /porc/<pid>/cmdline instead to get full cmd line */
                snprintf(filename, sizeof(filename), "/proc/%s/cmdline", next->d_name);
                if (! (cmdline = fopen(filename, "r")) ) {
                        continue;
                }
                if (fgets(buffer, READ_BUF_SIZE-1, cmdline) == NULL) {
                        fclose(cmdline);
                        continue;
                }
                fclose(cmdline);

                /* Buffer should contain a string like "Name:   binary_name" */
                /*sscanf(buffer, "%*s %s", name);*/
                /* buffer contains full commandline params separted by '\0' */
                remove_delimitor(buffer);
                if (strstr(buffer, pidName) != NULL) {
                        pidList=realloc( pidList, sizeof(pid_t) * (i+2));
                        if (!pidList) {
                                printf("cfm: Out of memeory!\n");
				closedir(dir);
                                return NULL;
                        }
                        pidList[i++]=strtol(next->d_name, NULL, 0);
                }
        }
        closedir(dir);

        if (pidList)
                pidList[i]=0;
        else if ( WLCSM_STRCMP(pidName, "init")==0) {
                /* If we found nothing and they were trying to kill "init",
                 * guess PID 1 and call it good...  Perhaps we should simply
                 * exit if /proc isn't mounted, but this will do for now. */
                pidList=realloc( pidList, sizeof(pid_t));
                if (!pidList) {
                        printf("cfm: Out of memeory!\n");
                        return NULL;
                }
                pidList[0]=1;
        } else {
                pidList=realloc( pidList, sizeof(pid_t));
                if (!pidList) {
                        printf("cfm: Out of memeory!\n");
                        return NULL;
                }
                pidList[0]=-1;
        }
        return pidList;
}



int bcmGetPid(char * command)
{
    char cmdline[128], *p1, *p2;
    pid_t *pid = NULL;
    int ret = 0;

    p1 = command;
    p2 = cmdline;
    while ( *p1 != '\0') {
        if (*p1 != ' ') {
           *p2 = *p1;
           p2++;
         }
         p1++;
    }
    *p2='\0';

    pid = find_pid_by_name(cmdline);
    if ( pid != NULL ) {
       ret = (int)(*pid);
       free(pid);
    }

    return ret;
}

int boardIoctl(int board_ioctl, BOARD_IOCTL_ACTION action, char *string, int strLen, int offset, char *buf)
{
    BOARD_IOCTL_PARMS IoctlParms;
    int boardFd = 0;

    boardFd = open("/dev/brcmboard", O_RDWR);
    if ( boardFd != -1 ) {
        IoctlParms.string = string;
        IoctlParms.strLen = strLen;
        IoctlParms.offset = offset;
        IoctlParms.action = action;
        IoctlParms.buf    = buf;
        ioctl(boardFd, board_ioctl, &IoctlParms);
        close(boardFd);
        boardFd = IoctlParms.result;
    } else
        printf("Unable to open device /dev/brcmboard.\n");

    return boardFd;
}

int sysGetChipId( void )
{
    return( boardIoctl(BOARD_IOCTL_GET_CHIP_ID, 0, "", 0, 0, "") );
}

//endof wsyscall.c

