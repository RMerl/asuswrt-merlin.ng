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

#include <bcmnvram.h>
#include "cms_msg.h"
#include "cms_util.h"
#include "cms_mdm.h"
#include "nvram_debug.h"
#include "wlmdm_lib.h"
#include "nvram_common.h"
#include "nvram_file.h"
#include "nvram_hash.h"
#include "staged.h"
#include "nvram_api.h"

#define NVRAM_LOCK_TIMEOUT 3000 // ms
#define PRE_COMMIT_NVRAM_FILE "/tmp/prec_nvf_staged"

static void *msgHandle = NULL;
static UBOOL8 cmsInitHere = FALSE;

static char *_nvram_get(const char *name, UBOOL8 use_hash);
static char *_nvram_kget(const char *name, UBOOL8 use_hash);
static int _cms_init(CmsEntityId eid);
static int _cms_destroy();

static int print_nvram_entry(const char *entry, void *data)
{
    if (entry == NULL)
    {
        log_error("invalid parameter!");
    }
    else
    {
        if(data)
        {
            /* if data is specified, it would be the stream for writing to a buffer */
            FILE *stream = (FILE*) data;

            fprintf(stream, "%s\n", entry);
        }
        else
        {
            printf("%s\n", entry);
        }
    }
    return 0;
}

static int kernel_nvrams_dump(void *to_where)
{
    int ret = -1;
    char *buf, *entry;

    buf = malloc(MAX_NVRAM_SPACE);
    if(!buf) {
        log_error("Could not allocate memory");
        return ret;
    }

    /* get whole nvrams from RB-tree */
    if((ret = common_nvram_kernel_getall(buf, MAX_NVRAM_SPACE)) == 0)
    {
        for (entry = buf; *entry; entry += strlen(entry) + 1)
        {
            /* each entry is a formatted nvram data like "nvname=value" */
            print_nvram_entry(entry, to_where);
        }
    }
    free(buf);
    return ret;
}

static int kernel_nvrams_save(const FILE *fp, void *data __attribute__ ((unused)))
{
    return kernel_nvrams_dump((void*) fp);
}

int nvram_kcommit(void)
{
    int ret = -1;

    /* get all of the kernel nvrams and dump to kernel nvram file */
    if(nvram_file_init(KERNEL_NVRAM_FILE_NAME) == 0)
    {
        ret = nvram_file_save_raw(kernel_nvrams_save, NULL);
        nvram_file_deinit();
    }

    return ret;
}

int nvram_commit(void)
{
    int ret = -1;
    if (0 == _cms_init(EID_WLNVRAM))
    {
        if (WLMDM_OK == wlmdm_init())
        {
            wlmdm_nvram_commit();
            ret = 0;
        }
    }
#ifdef KERNEL_NVRAM_AUTO_SET
    ret = nvram_kcommit();
#endif
    return ret;
}

char *nvram_get(const char *name)
{
    return _nvram_get(name, TRUE);
}

char *nvram_unf_get(const char *name)
{
    return _nvram_get(name, FALSE);
}

static char *_nvram_get(const char *name, UBOOL8 use_hash)
{
    int ret = -1;
    char value[MAX_NVRAM_VALUE_SIZE];
    char *result = NULL;

#ifdef KERNEL_NVRAM_AUTO_SET
    if (match_kernel_nvram(name) == 1)
    {
       return _nvram_kget(name, use_hash);
    }
#endif

    if (0 == _cms_init(EID_WLNVRAM))
    {
        if (WLMDM_OK != wlmdm_init())
        {
            return NULL;
        }
        ret = (wlmdm_nvram_get(name, (char *)&value, sizeof(value)) == WLMDM_OK) ? 0 : -1;
    }

    if (ret == 0)
    {
        if (use_hash)
        {
            nvram_hash_init();
            result = nvram_hash_get(name);
            if ((result == NULL) || (strcmp(result, (char *)&value) != 0))
            {
                nvram_hash_update(name, (char *)&value);
                result = nvram_hash_get(name);
            }
        }
        else
        {
            result = strdup((char *)&value);
        }
    }
    else
    {
        result = NULL;
    }

    return result;
}

char *nvram_get_bitflag(const char *name, const int bit)
{
    return common_nvram_get_bitflag(name, bit);
}

int nvram_set_bitflag(const char *name, const int bit, const int value)
{
    return common_nvram_set_bitflag(name, bit, value);
}

int nvram_set(const char *name, const char *value)
{
    int ret = -1;

#ifdef KERNEL_NVRAM_AUTO_SET
    if (match_kernel_nvram(name) == 1)
    {
        return nvram_kset(name, value);
    }
#endif

    if (0 == _cms_init(EID_WLNVRAM))
    {
        if (WLMDM_OK == wlmdm_init())
        {
            if (WLMDM_OK == wlmdm_nvram_set(name, value))
            {
                ret = 0;
            }
        }
    }
    return ret;
}

int nvram_dump(void)
{
    int ret = -1;
    if (0 == _cms_init(EID_WLNVRAM))
    {
        if (WLMDM_OK == wlmdm_init())
        {
            printf("/---------------------------------/\n");
            printf("/|       UserSpace NVRAMs        |/\n");
            printf("/---------------------------------/\n");
            wlmdm_nvram_dump();
            ret = 0;
        }
    }

    /* print out all of the kernel nvrams */
    ret =  nvram_kdump();

    return ret;
}

int nvram_kdump(void)
{
    int ret = -1;

    /* print out all of the kernel nvrams */
    printf("\n/---------------------------------/\n");
    printf("/|      KernelSpace NVRAMs       |/\n");
    printf("/---------------------------------/\n");
    ret = kernel_nvrams_dump(NULL);

    return ret;
}

int nvram_ugetall(char *buf, int count)
{
    int ret = -1;
    if (0 == _cms_init(EID_WLNVRAM))
    {
        if (WLMDM_OK == wlmdm_init())
        {
            wlmdm_nvram_getall(buf, (size_t) count);
            ret = 0;
        }
    }
    return ret;
}

int nvram_getall(char *buf, int count)
{
    int ret = -1;
    if (0 == _cms_init(EID_WLNVRAM))
    {
        if (WLMDM_OK == wlmdm_init())
        {
            wlmdm_nvram_getall(buf, (size_t) count);
            ret = 0;
        }
    }

    /* get all of the kernel nvrams and copy it to buf */
    {
        FILE *stream = NULL;
        size_t buff_size;
        char *buff_ptr = NULL;

        stream = open_memstream(&buff_ptr, &buff_size);
        if (stream == NULL)
        {
            log_error("open_memstream() error!!");
            return -1;
        }

        ret = kernel_nvrams_dump((void*) stream);

        fflush(stream);
        fclose(stream);

        if(ret == 0)
        {
            int size_used = strlen(buf);
            int size_remain = count - size_used - 1;

            if(size_remain > 0)
                strncat(buf, buff_ptr, (buff_size > size_remain) ? size_remain : buff_size);
        }
        free(buff_ptr);
    }
    return ret;
}

int nvram_unset(const char *name)
{
    int ret = -1;

#ifdef KERNEL_NVRAM_AUTO_SET
    if(match_kernel_nvram(name) == 1)
    {
        return nvram_kunset(name);
    }
#endif

    if (0 == _cms_init(EID_WLNVRAM))
    {
        if (WLMDM_OK == wlmdm_init())
        {
            ret = (wlmdm_nvram_unset(name) == WLMDM_OK) ? 0 : -1;
        }
    }
    return ret;
}

char *nvram_kget(const char *name)
{
    return _nvram_kget(name, TRUE);
}

char *nvram_unf_kget(const char *name)
{
    return _nvram_kget(name, FALSE);
}

static char *_nvram_kget(const char *name, UBOOL8 use_hash)
{
    int ret = -1;
    char value[MAX_NVRAM_VALUE_SIZE]={0};
    char *result = NULL;

    /* retrieve from RB-tree in kernel */
    ret = common_nvram_kernel_get(name, (char *)&value, sizeof(value));

    if (ret == 0)
    {
        if (use_hash)
        {
            nvram_hash_init();
            result = nvram_hash_get(name);
            if ((result == NULL) || (strcmp(result, (char *)&value) != 0))
            {
                nvram_hash_update(name, (char *)&value);
                result = nvram_hash_get(name);
            }
        }
        else
        {
            result = strdup((char *)&value);
        }
    }

    return result;
}

int nvram_kset(const char *name, const char *value)
{
    int ret = -1;

    if (name == NULL)
    {
        log_error("Invalid parameter!");
        return ret;
    }

    /* write to RB-tree in kernel */
    ret = common_nvram_kernel_set(name, value);

    return ret;
}

int nvram_kunset(const char *name)
{
    return nvram_kset(name, NULL);
}

static int _cms_init(CmsEntityId eid)
{
    int shmId = 0;
    CmsRet cret = CMSRET_SUCCESS;

    if (cmsInitHere == TRUE)
    {
        return 0;
    }

    if (FALSE == cmsMsg_isServiceReady())
    {
        return -1;
    }

    if (TRUE == cmsFil_isFilePresent(SMD_SHUTDOWN_IN_PROGRESS))
    {
        return -1;
    }

    if (!cmsMdm_isInitialized())
    {
        if ((cret = cmsMsg_initWithFlags(eid, EIF_MULTIPLE_INSTANCES, &msgHandle)) != CMSRET_SUCCESS)
        {
            return WLMDM_GENERIC_ERROR;
        }

        if ((cret = cmsMdm_initWithAcc(eid, 0x1, msgHandle, &shmId)) != CMSRET_SUCCESS)
        {
            log_error("cmsMdm_init failed, cret=%d", cret);
            cmsMsg_cleanup(&msgHandle);
            return -1;
        }
        cmsInitHere = TRUE;
    }
    return 0;
}

static int _cms_destroy()
{
    if (cmsInitHere == TRUE)
    {
        cmsMdm_cleanup();
        cmsMsg_cleanup(&msgHandle);
        cmsInitHere = FALSE;
    }
    return 0;
}
