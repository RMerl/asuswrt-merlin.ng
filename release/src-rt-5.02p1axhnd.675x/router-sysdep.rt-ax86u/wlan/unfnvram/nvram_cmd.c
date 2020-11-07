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

#include <bcmnvram.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <typedefs.h>
#include <unistd.h>

#include "nvram_debug.h"
#include "nvram_api.h"
#include "nvram_utils.h"

#define MFG_NVRAM_FILE_NAME_PATH "/mnt/nvram/nvram.nvm"
#define NVRAM_LINE_MAX (1024)
#define STR_MAX_LEN(a, b) (a > b ? a : b)
#define ARG_VAL (**p_argv)

#define foreachcolon(word, wordlist, next)                   \
    for (next = &wordlist[strspn(wordlist, ":")],            \
        strncpy(word, next, sizeof(word)),                   \
        word[strcspn(word, ":")] = '\0',                     \
        word[sizeof(word) - 1] = '\0',                       \
        next = strchr(next, ':');                            \
         strlen(word);                                       \
         next = next ? &next[strspn(next, ":")] : (char*)"", \
        strncpy(word, next, sizeof(word)),                   \
        word[strcspn(word, ":")] = '\0',                     \
        word[sizeof(word) - 1] = '\0',                       \
        next = strchr(next, ':'))

char g_line_buffer[NVRAM_LINE_MAX];
char* g_buf = NULL;

static int
isnumber(const char* src)
{
    char* iter = (char*)src;
    while (*iter)
    {
        if (!isdigit(*iter))
            return 0;
        iter++;
    }
    return 1;
}

#define NEXT_ARG(argc, argv)           \
    do                                 \
    {                                  \
        (argv)++;                      \
        if (--(argc) <= 0 || !*(argv)) \
            usage(NULL);               \
    } while (0)
#define ADVANCE_ARGCV(argc, argv, step) \
    do                                  \
    {                                   \
        (argv) += step;                 \
        (argc) -= step;                 \
    } while (0)
#define NEXT_IS_NUMBER(argc, argv)                               \
    do                                                           \
    {                                                            \
        if (((argc)-1 <= 0) || !*(argv) || !isnumber((argv)[1])) \
        {                                                        \
            usage(NULL);                                         \
        }                                                        \
    } while (0)
#define NEXT_IS_VALID(argc)  \
    do                       \
    {                        \
        if ((argc - 1 <= 0)) \
            usage(NULL);     \
    } while (0)

enum
{
    NVRAM_NORMAL_CMD = 0,
    NVRAM_HIDDEN_CMD
};

void usage(char* cmd);
typedef int (*NVRAM_CMD_FUNC)(int* argc, char*** p_argv);

typedef struct nvram_cmd_item
{
    char* cmd;
    char* prompt;
    char is_hidden_cmd;
    NVRAM_CMD_FUNC func;
    char align_tab;
} NVRAM_CMD_ITEM;

#ifdef NAND_SYS

/**** following functions are for kernel nvram handling ****
* The defaul kernel file * will be /data/.kernel_nvram.setting, when build image, the
* file kernel_nvram.setting under nvram will be burned to image and be available under
* /data directory after board bootup. When "nvram kset" is used, the variable will save
* to KERNEL_NVRAM_FILE_NAME directly, and the specified available would be removed when
* "nvram kunset" is issued.
*/

/** nvram kset wl0_ssid="12345" **/
static int nvram_cmd_kset(int* argc, char*** p_argv)
{
    char *name, *value = NULL;
    NEXT_ARG(*argc, *p_argv);
    strncpy(value = g_buf, ARG_VAL, strlen(ARG_VAL) + 1);
    name = strsep(&value, "=");
    return nvram_kset(name, value);
}

/** nvram kunset wl0_ssid **/
static int nvram_cmd_kunset(int* argc, char*** p_argv)
{
    NEXT_ARG(*argc, *p_argv);
    return nvram_kunset(ARG_VAL);
}

/** nvram kget wl0_ssid **/
static int nvram_cmd_kget(int* argc, char*** p_argv)
{
    char* value = NULL;
    NEXT_ARG(*argc, *p_argv);
    if ((value = nvram_unf_kget(ARG_VAL)))
    {
        puts(value);
        free(value);
    }
    return 0;
}

/** nvram kcommit **/
static int nvram_cmd_kcommit(int *argc, char ***p_argv) {
    ADVANCE_ARGCV(*argc,*p_argv,1);
    return nvram_kcommit();
}
#endif

/**
 * read from nvram files and populate it to the system
 */
int nvram_populate(char* file_name)
{
    int ret = 0;
    char *name, *value;
    if (file_name)
    {
        FILE* fp = fopen(file_name, "r");
        if (fp)
        {
            while (fgets(g_line_buffer, NVRAM_LINE_MAX, fp) != NULL)
            {
                if (g_line_buffer[0] == '#')
                    continue;
                value = g_line_buffer;
                name = strsep(&value, "=");
                if (name && value)
                {
                    name = trim_str(name);
                    value = trim_str(value);
                    nvram_set(name, value);
                }
            }
            fclose(fp);
        }
        else
        {
            printf("Failed to open file: %s\n", file_name);
            ret = -1;
        }
    }
    else
    {
        printf("invalid filename: %s\n", file_name);
        ret = -1;
    }
    return ret;
}


/**
 * read from nvram files and populate it to the system
 */
int kernel_nvram_populate(char* file_name)
{
    int ret = 0;
    char *name, *value;
    if (file_name)
    {
        FILE* fp = fopen(file_name, "r");
        if (fp)
        {
            while (fgets(g_line_buffer, NVRAM_LINE_MAX, fp) != NULL)
            {
                if (g_line_buffer[0] == '#')
                    continue;
                value = g_line_buffer;
                name = strsep(&value, "=");
                if (name && value)
                {
                    name = trim_str(name);
                    value = trim_str(value);
                    nvram_kset(name, value);
                }
            }
            fclose(fp);
        }
        else
        {
            printf("Failed to open file: %s\n", file_name);
            ret = -1;
        }
    }
    else
    {
        printf("invalid filename: %s\n", file_name);
        ret = -1;
    }
    return ret;
}

/**************END OF KERNEL NVRAM HANDLING SECTION **************/

/******* NVRAM_CMDS implementations *********/

/** nvram set wl0_ssid="12345" **/
static int nvram_cmd_set(int* argc, char*** p_argv)
{
    char *name, *value = NULL;
    NEXT_ARG(*argc, *p_argv);
    strncpy(value = g_buf, ARG_VAL, strlen(ARG_VAL) + 1);
    name = strsep(&value, "=");
    return nvram_set(name, value);
}

/** nvram setflag wl0_bititem 5=1 **/
static int nvram_cmd_setflag(int* argc, char*** p_argv)
{
    char *bit_value, *bit_pos, *value;
    char** argv;
    NEXT_ARG(*argc, *p_argv);
    NEXT_IS_VALID(*argc);
    argv = *p_argv;

    bit_value = argv[1];
    bit_pos = strsep(&bit_value, "=");
    if (bit_value && bit_pos && isnumber(bit_value) && isnumber(bit_pos))
    {
        if ((value = nvram_unf_get(argv[0])))
        {
            printf("value:%s->", value);
            free(value);
        }
        else
            printf("value:NULL->");

        nvram_set_bitflag(argv[0], atoi(bit_pos), atoi(bit_value));
        if ((value = nvram_unf_get(argv[0])))
        {
            printf("%s\n", value);
            free(value);
        }
    }
    ADVANCE_ARGCV(*argc, *p_argv, 1);
    return 0;
}

/** nvram unset wl0_ssid **/
static int nvram_cmd_unset(int* argc, char*** p_argv)
{
    NEXT_ARG(*argc, *p_argv);
    return nvram_unset(ARG_VAL);
}

/** nvram get wl0_ssid **/
static int nvram_cmd_get(int* argc, char*** p_argv)
{
    char* value = NULL;
    NEXT_ARG(*argc, *p_argv);
    if ((value = nvram_unf_get(ARG_VAL)))
    {
        puts(value);
        free(value);
    }
    return 0;
}

/** nvram getflag wl0_ssid 6 */
static int nvram_cmd_getflag(int* argc, char*** p_argv)
{
    char* value = NULL;
    char** argv;
    NEXT_ARG(*argc, *p_argv);
    NEXT_IS_NUMBER(*argc, *p_argv);
    argv = *p_argv;
    if ((value = nvram_get_bitflag(argv[0], atoi(argv[1]))))
        puts(value);
    ADVANCE_ARGCV(*argc, *p_argv, 1);
    return 0;
}

/** nvram getall|dump|show **/
static int nvram_cmd_getall(int* argc __attribute__((unused)),
                            char*** p_argv __attribute__((unused)))
{
    nvram_dump();
    return 0;
}

/** nvram kgetall|kdump|kshow **/
static int nvram_cmd_kgetall(int* argc __attribute__((unused)),
                             char*** p_argv __attribute__((unused)))
{
    nvram_kdump();
    return 0;
}


/** nvram restart **/
static int nvram_cmd_restart(int* argc, char*** p_argv)
{
    ADVANCE_ARGCV(*argc, *p_argv, 1);
    if (0 == system("killall -SIGUSR2 wlssk"))
    {
        return 0;
    }
    return -1;
}

/** nvram loadfile filename **/
/** populate nvram from a file on board **/
static int nvram_cmd_loadfile(int *argc, char*** p_argv) {
    int ret;
    char *file_name;

    ADVANCE_ARGCV(*argc, *p_argv, 1);
    file_name = (ARG_VAL ? ARG_VAL : "/data/nvramdefault.txt");

    if((ret = kernel_nvram_populate(file_name)) == 0)
        printf("populate nvram from %s done!\n", file_name);
    return ret;
}

/** nvram uloadfile filename **/
/** populate nvram from a file on board **/
static int nvram_cmd_uloadfile(int *argc, char*** p_argv) {
    int ret;
    char *file_name;

    ADVANCE_ARGCV(*argc, *p_argv, 1);
    file_name = (ARG_VAL ? ARG_VAL : "/data/nvramdefault.txt");

    if((ret = nvram_populate(file_name)) == 0)
        printf("populate nvram from %s done!\n", file_name);
    return ret;
}


/** nvram usavefile filename **/
/** save nvram to a file on board **/
int nvram_cmd_usavefile(int *argc, char ***p_argv) {
    char *file_name;
    FILE *ofp;

    ADVANCE_ARGCV(*argc,*p_argv,1);
    file_name=(ARG_VAL?ARG_VAL:"/data/nvramdefault.txt");
    ofp =fopen(file_name,"w");
    if(!ofp) {
        fprintf(stderr,"%s:%d open configuration file error \r\n",__FUNCTION__,__LINE__ );
        return -1;
    }

    if (0 == nvram_ugetall(g_buf, (size_t) MAX_NVRAM_SPACE))
    {
         fputs(g_buf,ofp);
         fputs("\n",ofp);
    }
    fclose(ofp);
    printf("save nvram to %s done!\n",file_name);
    return 0;
}

#ifdef NAND_SYS
int nvram_cmd_kernelset(int* argc, char*** p_argv)
{
    int ret;
    NEXT_ARG(*argc, *p_argv);
    if((ret = kernel_nvram_populate(ARG_VAL)) == 0)
        printf("restore done!\r\n");
    return ret;
}

/*
* -1 - error
*/
/*****************************************************************************
*  FUNCTION:  nvram_mfg_read_entry
*  PURPOSE:  Read next "name=value" entry from  from manufacturing NVRAM
*            file  (/mnt/nvram/nvram.nvm).
*  PARAMETERS:
*      f(IN) - manufacturing default NVRAM file handler.
*      ptr(IN) - buffer to read in.
*      len(IN) - max size of the buffer "ptr"
*  RETURNS:
*      strlen of string "name=value".
*      -1 if error occur.
*  NOTES:
*      Each couple "name=value" in /mnt/nvram/nvram.nvm is ending with '\0'.
*
*****************************************************************************/
static int nvram_mfg_read_entry(FILE* f, void* ptr, int len)
{
    int rv = -1;
    int i;
    char* p = (char*)ptr;

    for (i = 0; i < len; i++, p++)
    {
        if ((fread(p, 1, 1, f) != 1) || (*p == '\0'))
        {
            break;
        }
    }

    if (i < len)
        rv = i;

    return rv;
}

/*****************************************************************************
*  FUNCTION:  nvram_mfg_restore_default
*  PURPOSE:  Restore kernel NVRAM file (/data/.KERNEL_NVRAM_FILE_NAME)
*            from manufacturing default NVRAM setting (/mnt/nvram/nvram.nvm).
*  PARAMETERS:
*      pathname(IN) - the string of kernel NVRAM file name.
*  RETURNS:
*      0 - succeeded.
*      -1 error
*  NOTES:
*      /mnt/nvram/nvram.nvm is in binary format. Each "name=value" ending with '\0'
*
*****************************************************************************/
static int nvram_mfg_restore_default(char* pathname)
{
    FILE *f_mfg, *f_usr;
    int err = -1;

    if ((f_mfg = fopen(MFG_NVRAM_FILE_NAME_PATH, "rb")) != NULL)
    {
        clearerr(f_mfg);
        if ((f_usr = fopen(pathname, "a")) != NULL)
        {

            while ((err = nvram_mfg_read_entry(f_mfg, (void*)&g_line_buffer[0],
                        sizeof(g_line_buffer)))
                > 0)
            {
                fprintf(f_usr, "%s\n", g_line_buffer);
            }

            fclose(f_usr);
        }

        fclose(f_mfg);
    }

    return err;
}

int nvram_cmd_restore_mfg(int* argc, char*** p_argv)
{
    NEXT_ARG(*argc, *p_argv);
    printf("Restoring NVRAM to manufacturing default ... ");
    if (nvram_mfg_restore_default(ARG_VAL) == 0)
        printf("done.\r\n");
    else
        printf("fail.\r\n");
    return 0;
}

#endif

/** nvram commit [restart] **/
static int nvram_cmd_commit(int* argc, char*** p_argv)
{
    int ret = 0;
    ADVANCE_ARGCV(*argc, *p_argv, 1);
    nvram_commit();

    //sends message to wldaemo to covers all the work nvram_commit does.
    if (ARG_VAL && !strncmp(ARG_VAL, "restart", 7))
    {
        ret = system("killall -SIGUSR2 wlssk");
    }

    return ret;
}

/** nvram godefault **/
static int nvram_cmd_wl_godefault(int* argc, char*** p_argv)
{
    ADVANCE_ARGCV(*argc, *p_argv, 1);
    //sends message to wldaemo to covers all the work nvram_commit does.
    //and remove kernel nvram configu.
    unlink(KERNEL_NVRAM_FILE_NAME);
#ifdef NOCMS
    unlink(USER_NVRAM_FILE_NAME);
#endif
    sync();
    //wlcsm_mngr_wl_godefault();
    return 0;
}

NVRAM_CMD_ITEM nvram_cmd_items[] = {
    { "set", "set name with value", NVRAM_NORMAL_CMD, nvram_cmd_set, 2 },
    { "setflag", "set bit value", NVRAM_NORMAL_CMD, nvram_cmd_setflag, 1 },
    { "unset", "remove nvram entry", NVRAM_NORMAL_CMD, nvram_cmd_unset, 2 },
    { "get", "get nvram value with name", NVRAM_NORMAL_CMD, nvram_cmd_get, 2 },

    { "getflag", "get bit value", NVRAM_NORMAL_CMD, nvram_cmd_getflag, 1 },
    /* for cmd alias to use colon to seperate them */
    { "show:dump:getall", "show all nvrams", NVRAM_NORMAL_CMD, nvram_cmd_getall, 0 },
    { "kshow:kdump:kgetall", "show all kernel nvrams", NVRAM_NORMAL_CMD, nvram_cmd_kgetall, 0 },
    { "loadfile", "populate kernel nvram value from files",NVRAM_NORMAL_CMD,nvram_cmd_loadfile,1},
    { "uloadfile", "populate userspace nvram value from file",NVRAM_HIDDEN_CMD,nvram_cmd_uloadfile,1},
    { "usavefile", 	"save all userspace nvram value to file",NVRAM_HIDDEN_CMD,nvram_cmd_usavefile,1},
#ifdef NAND_SYS
    { "restore_mfg", "restore mfg nvrams", NVRAM_HIDDEN_CMD, nvram_cmd_restore_mfg, 0 },
    { "kernelset", "populate kernel nvram from kernel configuration file", NVRAM_HIDDEN_CMD, nvram_cmd_kernelset, 0 },
    { "kset", "set kernel nvram into runtime storage", NVRAM_NORMAL_CMD, nvram_cmd_kset, 2 },
    { "kunset", "remove kernel nvram from runtime storage", NVRAM_NORMAL_CMD, nvram_cmd_kunset, 1 },
    { "kget", "get kernel nvram from runtime storage", NVRAM_NORMAL_CMD, nvram_cmd_kget, 2 },
    { "kcommit", "only save kernel nvram", NVRAM_NORMAL_CMD, nvram_cmd_kcommit, 1},
#endif
    { "commit", "save nvram [optional] to restart wlan when following restart", NVRAM_NORMAL_CMD, nvram_cmd_commit, 1 },
    { "restart", "restart wlan", NVRAM_NORMAL_CMD, nvram_cmd_restart, 1 },
    { "godefault", "reset wireless nvram to default and restart", NVRAM_HIDDEN_CMD, nvram_cmd_wl_godefault, 1 }
};

#define NVRAM_CMDS_NUM (sizeof(nvram_cmd_items) / sizeof(NVRAM_CMD_ITEM))
#define FM_RESET "\033[0m"
#define FM_BOLDBLACK "\033[1m\033[30m"

void usage(char* cmd)
{

    unsigned int i = 0;
    int j = 0, line = 0;
    if (cmd)
        printf("\n=====\nInput cmd:%s is not supported \n======\n", cmd);
    printf("\n======== NVRAM CMDS ========\n");
    for (i = 0; i < NVRAM_CMDS_NUM; i++)
    {
        if (line % 2)
            printf(FM_BOLDBLACK);
        else
            printf(FM_RESET);
        if (!nvram_cmd_items[i].is_hidden_cmd)
        {
            printf("[%s]\t", nvram_cmd_items[i].cmd);
            for (j = 0; j < nvram_cmd_items[i].align_tab; j++)
                printf("\t");
            printf(": %s\n", nvram_cmd_items[i].prompt);
            line++;
        }
    }
    printf(FM_RESET);
    printf("============================\n");
    if (g_buf)
        free(g_buf);
    exit(0);
}

/* Main function to go through cmd list and execute cmd handler */

int main(int argc, char** argv)
{
    int ret = 0;
    unsigned int i = 0;
    char cmd_found = 0;
    char* next;

#ifdef BRCM_CMS_BUILD
    cmsLog_init(EID_WLNVRAM);
    cmsLog_setLevel(LOG_LEVEL_ERR);
#else
    bcmuLog_setLevel(BCMULOG_LEVEL_ERR);
#endif
    --argc;
    ++argv;
    if (!*argv)
    {
        usage(NULL);
    }
    else
    {
        g_buf = malloc(MAX_NVRAM_SPACE);
        if (!g_buf)
        {
            log_error("Could not allocate memory");
            return -1;
        }
        do
        {
            for (i = 0; i < NVRAM_CMDS_NUM; i++)
            {
                foreachcolon(g_line_buffer, nvram_cmd_items[i].cmd, next)
                {
                    if (!strncmp(g_line_buffer, *argv, strlen(g_line_buffer) + 1))
                    {
                        cmd_found = 1;
                    }
                }
                if (cmd_found)
                {
                    ret = nvram_cmd_items[i].func(&argc, &argv);
                    break;
                }
            }
            if (!cmd_found)
                usage(*argv);
            else
                cmd_found = 0;
            if (!*argv)
                break;
            ADVANCE_ARGCV(argc, argv, 1);
        } while (*argv);
    }
    free(g_buf);
#ifdef BRCM_CMS_BUILD
    cmsLog_cleanup();
#endif
    return ret;
}
