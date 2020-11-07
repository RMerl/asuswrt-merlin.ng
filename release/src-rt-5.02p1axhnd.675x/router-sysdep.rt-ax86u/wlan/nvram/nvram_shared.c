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

/*
 * NVRAM variable manipulation (common)
 *
 * Copyright Open Broadcom Corporation
 *
 * NVRAM emulation interface
 *
 * $Id:$
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <shared.h>
#include <wlcsm_lib_api.h>

/* for file locking */
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

char *nvram_xfr(const char *buf);
#ifdef DUMP_PREV_OOPS_MSG
int dump_prev_oops(void);
#endif

int nvram_commit(void)
{
    FILE *fp = NULL;
 
    if (nvram_get(ASUS_STOP_COMMIT) != NULL)
    {
	printf("# skip nvram commit #\n");
	return 0;
    }

    int ret = wlcsm_nvram_commit();
    sync();
    if( WLCSM_SUCCESS == ret &&
        (fp = fopen("/var/log/commit_ret", "w")) !=NULL)
    {
	fprintf(fp,"commit: OK\n");
	fclose(fp);
    }
    return ret;
}

#ifdef BCMDBG
static int debug_nvram_level =0xfffffff;

#define DBG_SET(fmt, arg...) \
        do { if (debug_nvram_level & DBG_NVRAM_SET) \
                printf("%s@%d: "fmt , __FUNCTION__ , __LINE__, ##arg); } while(0)

#define DBG_GET(fmt, arg...) \
        do { if (debug_nvram_level & DBG_NVRAM_GET) \
                printf("%s@%d: "fmt , __FUNCTION__ , __LINE__,##arg); } while(0)

#define DBG_GETALL(fmt, arg...) \
        do { if (debug_nvram_level & DBG_NVRAM_GETALL) \
                printf("%s@%d: "fmt , __FUNCTION__ , __LINE__,##arg); } while(0)

#define DBG_UNSET(fmt, arg...) \
        do { if (debug_nvram_level & DBG_NVRAM_UNSET) \
                printf("%s@%d: "fmt , __FUNCTION__ , __LINE__,##arg); } while(0)

#define DBG_INFO(fmt, arg...) \
        do { if (debug_nvram_level & DBG_NVRAM_INFO) \
                printf("%s@%d: "fmt , __FUNCTION__ , __LINE__, ##arg); } while(0)

#define DBG_ERROR(fmt, arg...) \
        do { if (debug_nvram_level & DBG_NVRAM_ERROR) \
                printf("%s@%d: "fmt , __FUNCTION__ , __LINE__, ##arg); } while(0)

#else
#define DBG_SET(fmt, arg...)
#define DBG_GET(fmt, arg...)
#define DBG_GETALL(fmt, arg...)
#define DBG_UNSET(fmt, arg...)
#define DBG_INFO(fmt, arg...)
#define DBG_ERROR(fmt, arg...)
#endif

#define LOCK_FILE      "/var/nvram.lock"
#define MAX_LOCK_WAIT  10

static int _lock()
{
       int fd;
       fd = open(LOCK_FILE,O_WRONLY|O_CREAT|O_EXCL,0644);
       if (fd < 0 && errno == EEXIST) {
               DBG_INFO("%s is already locked\n",LOCK_FILE);
               return 0;
       } else if (fd < 0){
               DBG_ERROR("unexpected error checking lock");
               return 0;
       }
       DBG_INFO(" nvram : %s created\n",LOCK_FILE);
       close(fd);
       return 1;
}

static int _unlock()
{
       if (unlink(LOCK_FILE) < 0) {
               DBG_ERROR("cannot remove lock file");
               return 0;
       }
       DBG_INFO(" nvram : %s deleted\n",LOCK_FILE);
       return 1;
}

static int _nvram_lock()
{
       int i=0;

       while (i++ < MAX_LOCK_WAIT) {
               if(_lock())
                       return 1;
               else
                       usleep(500000);
       }
       return 0;
}
/*nvram file unlock*/
static int _nvram_unlock()
{
       int i=0;

       while (i++ < MAX_LOCK_WAIT) {
               if(_unlock())
                       return 1;
               else
                       usleep(500000);
       }
       return 0;
}

char *nvram_get(const char *name)
{

#ifdef RTCONFIG_JFFS_NVRAM
       char *ret = NULL;
       if (large_nvram(name)) {
               DBG_GET("==>nvram_get\n");

               if (!_nvram_lock())
                       return NULL;

               ret = jffs_nvram_get(name);

               _nvram_unlock();

               DBG_GET("%s=%s\n",name, ret);
               DBG_GET("<==nvram_get\n");

               return ret;
       }
#endif

    return wlcsm_nvram_get((char *)name);
}

char *
nvram_get_bitflag(const char *name, const int bit)
{
    return wlcsm_nvram_get_bitflag((char *)name,(int)bit);
}

int
nvram_set_bitflag(const char *name, const int bit, const int value)
{
    return wlcsm_nvram_set_bitflag((char *)name,(int )bit,(int)value);
}

int nvram_set(const char *name, const char *value)
{
#ifdef RTCONFIG_JFFS_NVRAM
	int ret = 0;
	if (large_nvram(name)) {
		wlcsm_nvram_set((char *)name, "");

		DBG_SET("===>nvram_set[%s]=[%s]\n", name, value?value:"NULL");

		if (!_nvram_lock()) {
                       DBG_SET("lock failure");
                       ret = -1;
                       goto fail_set;
               }

               ret = jffs_nvram_set(name, value);

fail_set:
               _nvram_unlock();

               DBG_SET("<==nvram_set\n");

               return ret;
       }
#endif
    return wlcsm_nvram_set((char *)name,(char *)value);
}

int nvram_getall(char *buf, int count)
{
#ifdef RTCONFIG_JFFS_NVRAM
	int len;

	DBG_GETALL("==>nvram_getall\n");

	len = wlcsm_nvram_getall(buf,count);

	if (!_nvram_lock())
		return -1;

	len = jffs_nvram_getall(len,buf,count);

	_nvram_unlock();

	DBG_GETALL("<==nvram_getall\n");

	return len;
#else
    return wlcsm_nvram_getall(buf,count);
#endif
}

char *nvram_xfr(const char *buf)
{
#ifdef DSLCPE_WLCSM_EXT
	return wlcsm_nvram_xfr((char *)buf);
#else
	return NULL;
#endif
}

#ifdef DUMP_PREV_OOPS_MSG
int dump_prev_oops(void)
{
#ifdef DSLCPE_WLCSM_EXT
	return wlcsm_dump_prev_oops();
#else
	return 0;
#endif
}
#endif

int nvram_unset(const char *name)
{
#ifdef RTCONFIG_JFFS_NVRAM
       int ret;

       if (large_nvram(name)) {
               DBG_UNSET("==>nvram_unset\n");

               if (!_nvram_lock())
                       return NULL;

               ret = jffs_nvram_unset((char *)name);

               _nvram_unlock();

               DBG_UNSET("<==nvram_unset\n");

               return ret;
       }
#endif
    return wlcsm_nvram_unset((char *)name);
}
