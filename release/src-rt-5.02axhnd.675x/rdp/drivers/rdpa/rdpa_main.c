/*
* <:copyright-BRCM:2013-2015:proprietary:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
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
* :>
*/

#include <stdio.h>
#ifdef XRDP_EMULATION
#include "rdpa_emu.h"
#endif
#include <bdmf_dev.h>
#include <bdmf_shell.h>
#include <errno.h>

static int pipe_mode;
static bdmf_session_handle mon_session;
extern int rdpa_module_init(void);
extern void rdpa_module_exit(void);
static int ut_mode;

static int command_line_help(void)
{
    fprintf(stderr,
        "bdmf [options] [init_script]\n"
        "\t-t <trace_level> - set initial tracing level\n"
        "\t\t whereas trace_level is one of the following:\n"
        "\t\t\t none | error | info | debug\n"
        "\t-p - pipe mode\n"
        "\t\tThis mode is used for integration with external shell\n"
        "\t\trunning in a separate process\n\n"
        "\t-noedit - disable line edit mode\n"
        "\t-ut - ut mode\n"
        "\t\tExit after non-successful command\n"
        "\tinit_script - initial script\n"
    );
    return -EINVAL;
}

static int bdmf_exec_script(bdmf_session_handle session, const char *script)
{
    char buf[2048];
    FILE *f;
    int rc = 0;

    f = fopen(script, "r");
    if (!f)
    {
        printf("Can't open file %s for reading\n", script);
        return -EINVAL;
    }
    while (!bdmfmon_is_stopped(session) && !feof(f) &&
        fgets(buf, sizeof(buf)-1, f))
    {
        bdmf_session_print(session, "%s", buf);
        rc |= bdmfmon_parse(session, buf);
        if (rc && ut_mode)
        	break;
    }
    fclose(f);
    return rc;
}

/* quit monitor command handler */
static int bdmf_mon_quit(bdmf_session_handle session,
    const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmfmon_stop(session);
    bdmf_session_print(session, "BDMF terminated by 'Quit' command\n");
    return 0;
}

/* exec monitor command handler */
static int bdmf_mon_exec(bdmf_session_handle session,
    const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    const char *script = parm[0].value.string;

    return bdmf_exec_script(session, script);
}

/* reinit monitor command handler */
static int bdmf_mon_reinit(bdmf_session_handle session,
    const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int rc;

    rdpa_module_exit();
    rc = rdpa_module_init();
    printf("RDPA module re-loaded. rc=%d\n", rc);
    return rc;
}

/* unload monitor command handler */
static int bdmf_mon_unload(bdmf_session_handle session,
    const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    rdpa_module_exit();
    printf("RDPA module unloaded.\n");
    return 0;
}

#ifdef XRDP_EMULATION
int rdpa_main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    bdmf_session_parm_t mon_session_parm;
    bdmf_trace_level_t trace_level = bdmf_trace_level_error;
    const char *init_script = NULL;
    struct bdmf_init_config init_cfg;
    int noedit = 0;
    int i, rc;

    (void)noedit; /* prevent warning */

    ut_mode = 0;
    for (i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-p"))
            pipe_mode = 1;
        else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
            return command_line_help();
        else if (!strcmp(argv[i], "-t"))
        {
            const char *level;
            ++i;
            if (i == argc)
                return command_line_help();
            level = argv[i];
            if (!strcmp(level, "none"))
                trace_level = bdmf_trace_level_none;
            else if (!strcmp(level, "error"))
                trace_level = bdmf_trace_level_error;
            else if (!strcmp(level, "info"))
                trace_level = bdmf_trace_level_info;
            else if (!strcmp(level, "debug"))
                trace_level = bdmf_trace_level_debug;
            else
                return command_line_help();
        }
        else if (!strcmp(argv[i], "-noedit"))
            noedit = 1;
        else if (!strcmp(argv[i], "-ut"))
        	ut_mode = 1;
        else if (init_script != NULL)
        {
            printf("Unknown option: %s\n", argv[i]);
            return command_line_help();
        }
        else
            init_script = argv[i];
    }
#if defined(XRDP) && defined(RDP_SIM)
    if (!bdmf_rsv_mem_init())
        return BDMF_ERR_INTERNAL;
#endif
    memset(&init_cfg, 0, sizeof(init_cfg));
    memset(&mon_session_parm, 0, sizeof(mon_session_parm));
    mon_session_parm.access_right = BDMF_ACCESS_ADMIN;
    if (noedit)
    {
        mon_session_parm.line_edit_mode = BDMF_LINE_EDIT_DISABLE;
    }
    bdmfmon_session_open(&mon_session_parm, &mon_session);

    /* Initialise bdmf library */
    init_cfg.trace_level = trace_level;
    rc = bdmf_init(&init_cfg);
    if (rc)
        goto exit;

    /* Add exec command */
    {
        static bdmfmon_cmd_parm_t parms[] = {
            BDMFMON_MAKE_PARM("script",   "Script file name",
                BDMFMON_PARM_STRING, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        bdmfmon_cmd_add(NULL, ".", bdmf_mon_exec, "Execute script",
            BDMF_ACCESS_GUEST, NULL, parms);
    }

    /* Add re-init command */
    {
        bdmfmon_cmd_add(NULL, "reinit", bdmf_mon_reinit, "Unload, Re-load rdpa",
            BDMF_ACCESS_ADMIN, NULL, NULL);
    }

    /* Add unload command */
    {
        bdmfmon_cmd_add(NULL, "unload", bdmf_mon_unload, "Unload rdpa",
            BDMF_ACCESS_ADMIN, NULL, NULL);
    }

    /* Add quit command */
    bdmfmon_cmd_add(NULL, "quit", bdmf_mon_quit, "Quit simulation",
        BDMF_ACCESS_GUEST, NULL, NULL);

    /*
     * Init RDPA module
     */
    rc = rdpa_module_init();
    if (rc)
        goto exit;

    if (init_script && bdmf_exec_script(mon_session, init_script))
    {
        rc = -EINVAL;
    }

    if (rc && ut_mode)
    	goto exit;

    do
    {
        /* Process user input until EOF or quit command */
        bdmfmon_driver(mon_session);
        if (!bdmfmon_is_stopped(mon_session) && feof(stdin) && pipe_mode)
            stdin = freopen(NULL, "r", stdin);
    } while (!bdmfmon_is_stopped(mon_session));
#ifndef XRDP_EMULATION
    bdmf_exit();
    bdmfmon_session_close(mon_session);
    bdmfmon_token_destroy(NULL);
#endif
exit:
    if (rc)
        printf("RDPA module init failed. rc=%d\n", rc);
#if defined(XRDP) && defined(RDP_SIM)
    bdmf_rsv_mem_destroy();
#endif
    return rc;
}
