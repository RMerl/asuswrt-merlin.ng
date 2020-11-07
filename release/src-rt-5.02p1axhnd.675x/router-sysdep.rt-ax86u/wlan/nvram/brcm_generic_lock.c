/***********************************************************************
 *
 *  Copyright (c) 2005  Broadcom Corporation
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <typedefs.h>
#include <bcmendian.h>
#include <bcmnvram.h>
#include <bcmutils.h>

/* for file locking */
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#if 0
#define DBG_INFO(fmt, arg...) \
        do { printf("%s@%d: "fmt , __FUNCTION__ , __LINE__, ##arg); } while(0)

#define DBG_ERROR(fmt, arg...) \
        do { printf("%s@%d: "fmt , __FUNCTION__ , __LINE__, ##arg); } while(0)

#else
#define DBG_INFO(fmt, arg...)
#define DBG_ERROR(fmt, arg...)
#endif

static int _generic_lock(char *name)
{
	char lockfile[30];
	int fd;
	snprintf(lockfile, sizeof(lockfile), "/var/%s_lock",name);
	fd = open(lockfile,O_WRONLY|O_CREAT|O_EXCL,0644);
	if (fd < 0 && errno == EEXIST) {
		DBG_INFO("%s is already locked\n",lockfile);
		return 0;		
	} else if (fd < 0){
		DBG_ERROR("unexpected error checking lock");			
		return 0;
	}
	DBG_INFO(" nvram : %s created\n",lockfile);
	close(fd);
	return 1;
}

int brcm_get_lock(char *name, int timeout) 
{
	int loop=0;
	while(!_generic_lock(name))
	{
		usleep(1000000);
		if(++loop  >timeout) {
		 DBG_INFO("%s:%d pid:%d timeouted,but still did not get lock:%s  \r\n",__FUNCTION__,__LINE__,getpid(),name );
		 return 0;
		}
		DBG_INFO("%s:%d pid:%d is waiting for %s lock  \r\n",__FUNCTION__,__LINE__,getpid(),name );
	}
	DBG_INFO("%s:%d  process:%d get %s lock done######  \r\n",__FUNCTION__,__LINE__,getpid(),name);
	return 1;
}

int brcm_release_lock(char* name)
{
	char lockfile[30];
	snprintf(lockfile, sizeof(lockfile), "/var/%s_lock",name);
	if (unlink(lockfile) < 0) {
		
		DBG_ERROR("III cannot remove lock file,%s",lockfile);
		DBG_INFO(":%s:%d  process:%d release %s lock failed######  \r\n",__FUNCTION__,__LINE__,getpid(),name);
		return 0;
	}		
	DBG_INFO(":%s:%d  process:%d release %s	 lock ok!!!!!!!!!!!!!!  \r\n",__FUNCTION__,__LINE__,getpid(),name);
	return 1;
}


