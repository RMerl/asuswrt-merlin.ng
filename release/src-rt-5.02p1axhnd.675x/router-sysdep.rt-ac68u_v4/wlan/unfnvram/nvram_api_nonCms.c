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

#include <sys/socket.h>
#include <linux/netlink.h>
#include <errno.h>
#include <bcmnvram.h>

#include "os_defs.h"
#include "nvram_debug.h"
#include "staged.h"
#include "nvram_common.h"
#include "nvram_file.h"
#include "nvram_hash.h"
#include "nvram_api.h"

/*
  * PRE_COMMIT_NVRAM_FILE(in memory) is the file to store userspace nvram settings 
  * temporarily while doing nvram set or unset.
  * When "nvram commit" is issued, the settings in PRE_COMMIT_NVRAM_FILE would be
  * merged into COMMITTED_NVRAM_FILE(in memory), and then save back the committed
  * result to USER_NVRAM_FILE(in flash)
*/
#define PRE_COMMIT_NVRAM_FILE "/tmp/prec_nvf_staged"
#define COMMITTED_NVRAM_FILE "/tmp/commit_nvf_staged"

struct priv_data { void *mapped_data; void *fp; };

static char *_nvram_get(const char *name, UBOOL8 use_hash);
static char *_nvram_kget(const char *name, UBOOL8 use_hash);
static int print_nvram_entry(const char *entry, void *data);

static int print_nvram(const char *nvname, const char *value, void *data)
{
    if (nvname == NULL)
    {
        log_error("invalid nvname");
    }
    else
    {
        if(value)
        {
            int max_len = MAX_NVRAM_NAME_SIZE + MAX_NVRAM_VALUE_SIZE + 1;
            char entry[max_len];

            memset(entry, 0, sizeof(entry));
            snprintf(entry, max_len, "%s=%s", nvname, value);
            print_nvram_entry((char *)&entry, data);
        }
    }
    return 0;
}

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

static int filter_print_nvram(const char *nvname, const char *value, void *data)
{
    struct priv_data *pd = NULL;

    if(data == NULL)
        return 0;

    pd = (struct priv_data*) data;

    if (FALSE == nvc_list_exist((char*) pd->mapped_data, nvname))
    {
        return print_nvram(nvname, value, (void*) pd->fp);
    }
    return 0;
}

static int list_add(const char *nvname, const char *value, void *data)
{
    if(data)
    {
        json_object *list = (json_object*) data;
        nvc_list_update_object(list, nvname, value);
    }
    return 0;
}

static int load_nvf(char* mapped_data, void *data __attribute__((unused)))
{
    char *result = NULL;
    json_object *list = NULL;

    if(mapped_data && strlen(mapped_data) == 0)
    {
        if(nvram_file_init(USER_NVRAM_FILE_NAME) == 0)
        {
            list = json_object_new_object();
            nvram_file_load(list_add, list);
            result = nvc_list_object_to_string(list);
            if(result)
            {
                strcpy(mapped_data, result);
                free(result);
            }
            json_object_put(list);
            nvram_file_deinit();
        }
    }
    return 0;
}

static int save_nvf(char* mapped_data, void *data __attribute__((unused)))
{
    int ret = -1;

    if(mapped_data && strlen(mapped_data) != 0)
    {
        if(nvram_file_init(USER_NVRAM_FILE_NAME) == 0)
        {
            ret = nvram_file_save(mapped_data);
            nvram_file_deinit();
        }
    }
    return ret;
}

static int dup_mapped_data(char* mapped_data, void *data)
{
    if(mapped_data && strlen(mapped_data) != 0)
    {
        /* copy mapped data */
        if(data)
        {
            FILE *stream = (FILE*)data;
            fprintf(stream, "%s", mapped_data);
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

static int commit_func(const char *nvname, const char *value, void *data __attribute__((unused)))
{
    StagedRet ret = STAGED_OK;
    StagedInfo *commit_nvf_si;

    commit_nvf_si = staged_init(COMMITTED_NVRAM_FILE);
    if(commit_nvf_si == NULL)
    {
        log_error("staged_init() error!!");
        return -1;
    }

    /* check if COMMITTED_NVRAM_FILE is empty? if so, load the nvram settings as the initial data from NVRAM_FILE
         * to COMMITTED_NVRAM_FILE.
         */
    staged_raw_execute(commit_nvf_si, load_nvf, NULL);

    if (value == NULL)
    {
        ret = staged_delete(commit_nvf_si, nvname);
    }
    else
    {
        ret = staged_set(commit_nvf_si, nvname, value);
    }

    staged_free(commit_nvf_si);

    return (ret == STAGED_OK) ? 0 : -1;
}

int nvram_commit(void)
{
    int ret = 0;
    StagedInfo *prec_nvf_si;
    StagedInfo *commit_nvf_si;

    prec_nvf_si = staged_init(PRE_COMMIT_NVRAM_FILE);
    if(prec_nvf_si == NULL)
    {
        log_error("staged_init() error!!");
        return -1;
    }
    staged_commit(prec_nvf_si, &commit_func, NULL, NULL);
    staged_free(prec_nvf_si);

    /* sync to user nvram file */
    commit_nvf_si = staged_init(COMMITTED_NVRAM_FILE);
    if(commit_nvf_si == NULL)
    {
        log_error("staged_init() error!!");
        return -1;
    }
    if (STAGED_OK != staged_raw_execute(commit_nvf_si, save_nvf, NULL))
    {
        ret = -1;
    }
    staged_free(commit_nvf_si);

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
    StagedRet ret;
    char value[MAX_NVRAM_VALUE_SIZE];
    char *result = NULL;
    StagedInfo *prec_nvf_si;

    if (name == NULL)
    {
         log_error("Invalid parameter!");
         return result;
    }

#ifdef KERNEL_NVRAM_AUTO_SET
    if (match_kernel_nvram(name) == 1)
    {
       return _nvram_kget(name, use_hash);
    }
#endif

    prec_nvf_si = staged_init(PRE_COMMIT_NVRAM_FILE);
    if(prec_nvf_si == NULL)
    {
        log_error("staged_init() error!!");
        return result;
    }

    /* 1. Search PRE_COMMIT_NVRAM_FILE for uncommitted changes first. */
    ret = staged_get(prec_nvf_si, name, (char *)&value, sizeof(value));

    if (ret == STAGED_OK)
    {
         result = strdup((char *)&value);
    }
    else if (ret == STAGED_NOT_FOUND)
    {
        /* 2. if not found, then search COMMITTED_NVRAM_FILE instead. */
        StagedInfo *commit_nvf_si;

        commit_nvf_si = staged_init(COMMITTED_NVRAM_FILE);
        if(commit_nvf_si == NULL)
        {
            log_error("staged_init() error!!");
            return result;
        }

        /* check if COMMITTED_NVRAM_FILE is empty? if so, load the nvram settings as the initial data from NVRAM_FILE
                * to COMMITTED_NVRAM_FILE.
              */
        staged_raw_execute(commit_nvf_si, load_nvf, NULL);

        ret = staged_get(commit_nvf_si, name, (char *)&value, sizeof(value));
        if (ret == STAGED_OK)
        {
            result = strdup((char *)&value);
        }
        else
        {
            log_debug("#### nvname not found in nvram file. ####");
        }

        staged_free(commit_nvf_si);
    }

    staged_free(prec_nvf_si);

    if (use_hash == TRUE)
    {
        if (result != NULL)
        {
            nvram_hash_init();
            nvram_hash_update(name, (const char*)&value);
            free(result);
            result = nvram_hash_get(name);
        }
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
    int ret = 0;
    StagedInfo *prec_nvf_si;

    if (name == NULL)
    {
        log_error("Invalid parameter!");
        return -1;
    }

#ifdef KERNEL_NVRAM_AUTO_SET
    if (match_kernel_nvram(name) == 1)
    {
        return nvram_kset(name, value);
    }
#endif

    /* 
     * Workaround for BTEST build. Once we support "nvram kset" to write into RB-tree,
     * the workaround can be removed and should replace "nvram set" with "nvram kset" in wl_ko.sh
     */
#ifdef BTEST
    common_nvram_kernel_set(name, value);
#endif

    prec_nvf_si = staged_init(PRE_COMMIT_NVRAM_FILE);
    if(prec_nvf_si == NULL)
    {
        log_error("staged_init() error!!");
        return -1;
    }

    if (STAGED_OK != staged_set(prec_nvf_si, name, value))
    {
        ret = -1;
    }

    staged_free(prec_nvf_si);

    return ret;
}

int nvram_getall(char *buf, int count)
{
    int ret = 0;
    StagedInfo *prec_nvf_si;
    StagedInfo *commit_nvf_si;
    struct priv_data *pd;
    FILE *stream = NULL;
    size_t buff_size = 0;
    char *buff_mapped_data = NULL;
    char *buff_out = NULL;

    /* 1. Dump out all "set" NVRAM configurations in PRE_COMMIT_NVRAM_FILE. */
    prec_nvf_si = staged_init(PRE_COMMIT_NVRAM_FILE);
    if(prec_nvf_si == NULL)
    {
        log_error("staged_init() error!!");
        return -1;
    }

    stream = open_memstream(&buff_mapped_data, &buff_size);
    if (stream == NULL)
    {
        log_error("open_memstream() error!!");
        staged_free(prec_nvf_si);
        return -1;
    }

    /* just copy nvram data into buff_mapped_data */
    staged_raw_execute(prec_nvf_si, &dup_mapped_data, (void*)stream);

    fflush(stream);
    fclose(stream);
    staged_free(prec_nvf_si);

    buff_size = 0;
    stream = open_memstream(&buff_out, &buff_size);
    if (stream == NULL)
    {
        log_error("open_memstream() error!!");
        free(buff_mapped_data);
        return -1;
    }

    /* print out nvrams from buff_mapped_data to buff_out in different format */
    nvc_list_for_each(buff_mapped_data, print_nvram, (void*)stream);


    /* 2. Dump out NVRAM configurations in COMMITTED_NVRAM_FILE. */
    commit_nvf_si = staged_init(COMMITTED_NVRAM_FILE);
    if(commit_nvf_si == NULL)
    {
        log_error("staged_init() error!!");
        fclose(stream);
        free(buff_mapped_data);
        free(buff_out);
        return -1;
    }

    /* this is the private data we have to pass for filter_print_nvram() */
    pd = malloc(sizeof(struct priv_data));
    if(pd != NULL)
    {
        /* contain the copied nvram data in step1 */
        pd->mapped_data = (void*) buff_mapped_data;
        pd->fp = (void*) stream;

        staged_dump(commit_nvf_si, &filter_print_nvram, (void*) pd);

        free(pd);
    }

    staged_free(commit_nvf_si);
    free(buff_mapped_data);


    /* 3. Dump out NVRAM configurations in kernel */
    ret = kernel_nvrams_dump((void*) stream);

    fflush(stream);
    fclose(stream);

    log_debug("count=%d, size=%lu", count, (unsigned long) buff_size);
    strncpy(buf, buff_out, (buff_size > count) ? count : buff_size);

    free(buff_out);

    return ret;
}

int nvram_unset(const char *name)
{
    return nvram_set(name, NULL);
}

int nvram_ugetall(char *buf, int count)
{
    log_error("nvram_ugetall() is not implemented for nonCMS !!");
    return -1;
}

int nvram_dump(void)
{
    int ret = 0;
    StagedInfo *prec_nvf_si;
    StagedInfo *commit_nvf_si;
    struct priv_data *pd = NULL;
    FILE *stream = NULL;
    size_t buff_size;
    char *buff_mapped_data = NULL;

    printf("/---------------------------------/\n");
    printf("/|       UserSpace NVRAMs        |/\n");
    printf("/---------------------------------/\n");

    /* 1. Dump out all "set" NVRAM configurations in PRE_COMMIT_NVRAM_FILE. */
    prec_nvf_si = staged_init(PRE_COMMIT_NVRAM_FILE);
    if(prec_nvf_si == NULL)
    {
        log_error("staged_init() error!!");
        return -1;
    }

    stream = open_memstream(&buff_mapped_data, &buff_size);
    if (stream == NULL)
    {
        log_error("open_memstream() error!!");
        staged_free(prec_nvf_si);
        return -1;
    }

    /* just copy nvram data into buff_mapped_data */
    staged_raw_execute(prec_nvf_si, &dup_mapped_data, (void*)stream);

    fflush(stream);
    fclose(stream);
    staged_free(prec_nvf_si);

    /* print out nvrams from buff_mapped_data to stdout */
    nvc_list_for_each(buff_mapped_data, &print_nvram, NULL);


    /* 2. Dump out NVRAM configurations in COMMITTED_NVRAM_FILE. */
    commit_nvf_si = staged_init(COMMITTED_NVRAM_FILE);
    if(commit_nvf_si == NULL)
    {
        log_error("staged_init() error!!");
        free(buff_mapped_data);
        return -1;
    }

    /*
         * check if COMMITTED_NVRAM_FILE is empty? if so, load the nvram settings as the initial data from USER_NVRAM_FILE_NAME
         * to COMMITTED_NVRAM_FILE.
       */
    staged_raw_execute(commit_nvf_si, load_nvf, NULL);

    /* this is the private data we have to pass into filter_print_nvram() */
    pd = malloc(sizeof(struct priv_data));
    if(pd != NULL)
    {
        /* contain the copied nvram data in step1 */
        pd->mapped_data = (void*) buff_mapped_data;
        pd->fp = NULL;

        staged_dump(commit_nvf_si, &filter_print_nvram, (void*) pd);

        free(pd);        
    }
    staged_free(commit_nvf_si);
    free(buff_mapped_data);


    /* 3. Dump out NVRAM configurations in kernel */
    ret = nvram_kdump();

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

/*
  * write nvram to RB-tree without saving to staged list
  */
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
