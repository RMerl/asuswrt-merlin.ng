/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */


#include <stdio.h>

#include <bdmf_dev.h>
#include <bdmf_shell.h>
#include <errno.h>

static int pipe_mode;
static bdmf_session_handle mon_session;

#ifdef BDMF_EDITLINE
/* editline functionality */
#include <histedit.h>
static EditLine *el;
static History *myhistory;
static HistEvent myhistevent;
static FILE *stdin_initval;
static char * editline_prompt(EditLine *e)
{
    return "";
}

#endif

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
"\tinit_script - initial script\n"
    );
    return -EINVAL;
}

static int bdmf_exec_script(bdmf_session_handle session, const char *script)
{
    char buf[1024];
    FILE *f;
    f = fopen(script, "r");
    if (!f )
    {
        printf("Can't open file %s for reading\n", script);
        return -EINVAL;
    }
    while(!bdmfmon_is_stopped(session) &&
          !feof(f) &&
          fgets(buf, sizeof(buf)-1, f))
    {
        bdmf_session_print(session, "%s\n", buf);
        bdmfmon_parse(session, buf);
    }
    fclose(f);
    return 0;
}

/* quit monitor command handler
*/
static int bdmf_mon_quit(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t nParms)
{
    bdmfmon_stop(session);
    bdmf_session_print(session, "BDMF terminated by 'Quit' command\n");
    return 0;
}

/* exec monitor command handler
*/
static int bdmf_mon_exec(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t nParms)
{
    const char *script=parm[0].value.string;
    return bdmf_exec_script(session, script);
}

/** Session's input function. NULL=use gets */
static char *bdmf_gets(void *buffer, uint32_t size)
{
    char *line = NULL;
#ifdef BDMF_EDITLINE
    if (el && myhistory)
    {
        int line_len;
        line = (char *)el_gets(el, &line_len);
        if (!line)
            return NULL;
        if (line_len > size)
        {
            bdmf_print("%s: buffer is too short %u - got %d. Truncated\n",
                    __FUNCTION__, size, line_len);
        }
        strncpy(buffer, line, size);
        if (*line && *line != '\n' && *line != '#')
            history(myhistory, &myhistevent, H_ENTER, line);
    }
    else
#else
        line = fgets(buffer, size, stdin);
#endif
    if (line && pipe_mode)
        fputs(line, stdout);
    return line;
}


int main(int argc, char **argv)
{
    bdmf_session_parm_t mon_session_parm;
    bdmf_trace_level_t trace_level=bdmf_trace_level_error;
    const char *init_script=NULL;
    struct bdmf_init_config init_cfg;
    int noedit=0;
    int i;
    int rc;

    (void)noedit; /* prevent warning */
    memset(&init_cfg, 0, sizeof(init_cfg));
    memset(&mon_session_parm, 0, sizeof(mon_session_parm));
    mon_session_parm.access_right = BDMF_ACCESS_ADMIN;
    bdmfmon_session_open(&mon_session_parm, &mon_session);

    for(i=1; i<argc; i++)
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
            noedit=1;
        else if (init_script != NULL)
        {
            printf("Unknown option: %s\n", argv[i]);
            return command_line_help();
        }
        else
            init_script = argv[i];
    }

    /* Initialise bdmf library */
    init_cfg.trace_level = trace_level;
    rc = bdmf_init(&init_cfg);
    if (rc)
        return rc;

    /* Add exec command */
    {
        static bdmfmon_cmd_parm_t parms[]={
            BDMFMON_MAKE_PARM("script",   "Script file name", BDMFMON_PARM_STRING, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        bdmfmon_cmd_add(NULL, ".", bdmf_mon_exec,
                      "Execute script",
                      BDMF_ACCESS_GUEST, NULL, parms);
    }
    /* Add quit command */
    bdmfmon_cmd_add(NULL, "quit", bdmf_mon_quit,
                  "Quit simulation",
                  BDMF_ACCESS_GUEST, NULL, NULL);


#ifdef BDMF_EDITLINE
    if (!noedit)
    {
        /* Initialize editline library */
        el = el_init(argv[0], stdin, stdout, stderr);
        myhistory = history_init();
        if (el && myhistory)
        {
            stdin_initval = stdin;
            el_set(el, EL_EDITOR, "emacs");
            el_set(el, EL_PROMPT, &editline_prompt);
            el_set(el, EL_TERMINAL, "xterm");
            history(myhistory, &myhistevent, H_SETSIZE, 800);
            el_set(el, EL_HIST, history, myhistory);
        }
        else
            printf("Can't initialize editline library. Falling back\n");
    }
#endif

    if (init_script && bdmf_exec_script(mon_session, init_script))
        return -EINVAL;

    do
    {
        char buf[1024];
        while(!bdmfmon_is_stopped(mon_session) &&
              bdmf_gets(buf, sizeof(buf)-1))
        {
            bdmfmon_parse(mon_session, buf);
        }
        if (!bdmfmon_is_stopped(mon_session) && feof(stdin) && pipe_mode)
            stdin = freopen(NULL, "r", stdin);
    } while(!bdmfmon_is_stopped(mon_session));

    bdmf_exit();
    bdmfmon_session_close(mon_session);
    bdmfmon_token_destroy(NULL);

#ifdef BDMF_EDITLINE
    if (myhistory)
        history_end(myhistory);
    if (el)
        el_end(el);
#endif

    return 0;
}
