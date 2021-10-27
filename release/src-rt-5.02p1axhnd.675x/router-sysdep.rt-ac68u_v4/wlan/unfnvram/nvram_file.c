/***********************************************************************
 *
 *  Copyright (c) 2018  Broadcom
 *  All Rights Reserved
 *
<:label-BRCM:2018:proprietary:standard

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
#include <sys/file.h>
#include <errno.h>
#include <sys/stat.h>

#include "nvram_file.h"
#include "nvram_utils.h"
#include "nvram_debug.h"

#define NVRAM_LINE_MAX (MAX_NVRAM_NAME_SIZE+MAX_NVRAM_VALUE_SIZE+1)
static FILE *g_fp = NULL;

static int isFilePresent(const char *filename)
{
    struct stat statbuf;
    SINT32 rc;
 
    rc = stat(filename, &statbuf);
 
    if (rc == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int nvram_file_init(char *filename)
{
    int ret =0;

    if(!filename || strlen(filename) == 0)
    {
        log_error("filename is invalid!");
        return -1;
    }

    if (!isFilePresent(filename))
    {
        log_error("%s is not present!", filename);
        return -1;
    }

    g_fp = fopen(filename, "r+");

    if(!g_fp)
    {
        log_error("failed to fopen()! error=%s\n", strerror(errno));
        return -1;
    }

    lseek(fileno(g_fp), 0, SEEK_SET);

    /* to acquire file lock */
trylock:
    ret = lockf(fileno(g_fp), F_LOCK, 0L);
    if(ret == -1)
    {
        if(errno == EINTR)
        {
            log_error("lockf() failed to lock due to EINTR, so redo lockf()!");
            goto trylock;
        }
        else
            log_error("failed to lockf()! error=%s", strerror(errno));
        fclose(g_fp);
        g_fp = NULL;
        return -1;
    }
    return 0;
}

int nvram_file_deinit(void)
{
    if(g_fp)
    {
        /* release file lock */
        if(lockf(fileno(g_fp), F_ULOCK, 0L) == -1)
            log_error("failed to lockf()! error=%s", strerror(errno));

        fclose(g_fp);
        g_fp = NULL;
    }
    return 0;
}

static int _file_write(const char *nvname, const char *value, void *data)
{
    if (nvname == NULL)
    {
        log_error("invalid nvname");
    }
    else
    {
        if(data)
        {
            FILE *fp = (FILE *) data;
            fprintf(fp, "%s=%s\n", nvname, value);
        }
    }
    return 0;
}


/**  nvram_file_init()/_deinit() is required before/after invoking the following functions **/

/*
 *  retriveve each "name=value" from kernel NVRAM file (Default: /data/.KERNEL_NVRAM_FILE_NAME),
 *  and then invoke "foreach_func" and take the "name" and "value" as its input parameters by default.
 */
int nvram_file_load(nvc_for_each_func foreach_func, void *data)
{
    char *name,*value;
    char g_line_buffer[NVRAM_LINE_MAX];

    lseek(fileno(g_fp), 0, SEEK_SET);

    while(fgets(g_line_buffer, NVRAM_LINE_MAX, g_fp) != NULL) 
    {
        if(g_line_buffer[0 ]== '#') continue;

        value = g_line_buffer;
        name = strsep(&value, "=");

        if(name && value) {
            name = trim_str(name);
            value = trim_str(value);
            //printf("#### (%s:%d) [name:value]=%s:%s ####\n", __func__, __LINE__, name, value);
            foreach_func(name, value, data);
        }
    }
    return 0;
}

/*
 *  parse the input parameter "list_str"(JSON string) to get each ["nvname" & "value"] pair, 
 *  and then write each pair in "name=value" format into kernel NVRAM file.
 */
int nvram_file_save(const char *list_str)
{
    if(!g_fp)
    {
        log_error("invalid file pointer!");
        return -1;
    }

    if(list_str && strlen(list_str) == 0 )
        return -1;

    if(ftruncate(fileno(g_fp), 0) == -1)
    {
        log_error("file could not be truncated!");
        return -1;
    }

    lseek(fileno(g_fp), 0, SEEK_SET);

    nvc_list_for_each(list_str, _file_write, g_fp);

    fflush(g_fp);

    fsync(fileno(g_fp));

    return 0;
}

/*
 *  pass fp into the user defined save_func() to write data in nvram file
 */
int nvram_file_save_raw(save_raw_func save_func, void *data)
{
    if(!g_fp)
    {
        log_error("invalid file pointer!");
        return -1;
    }

    if(ftruncate(fileno(g_fp), 0) == -1)
    {
        log_error("file could not be truncated!");
        return -1;
    }

    lseek(fileno(g_fp), 0, SEEK_SET);

    save_func(g_fp, data);

    fflush(g_fp);

    fsync(fileno(g_fp));

    return 0;
}
