/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  UI Command Dispatch			File: ui_cmddisp.c
    *  
    *  This module contains routines to maintain the command table,
    *  parse and execute commands
    *  
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *  
    *********************************************************************  
    *
    *  Copyright 2000,2001,2002,2003
    *  Broadcom Corporation. All rights reserved.
    *  
    *  This software is furnished under license and may be used and 
    *  copied only in accordance with the following terms and 
    *  conditions.  Subject to these conditions, you may download, 
    *  copy, install, use, modify and distribute modified or unmodified 
    *  copies of this software in source and/or binary form.  No title 
    *  or ownership is transferred hereby.
    *  
    *  1) Any source code used, modified or distributed must reproduce 
    *     and retain this copyright notice and list of conditions 
    *     as they appear in the source file.
    *  
    *  2) No right is granted to use any trade name, trademark, or 
    *     logo of Broadcom Corporation.  The "Broadcom Corporation" 
    *     name may not be used to endorse or promote products derived 
    *     from this software without the prior written permission of 
    *     Broadcom Corporation.
    *  
    *  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
    *     IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
    *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
    *     PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
    *     SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
    *     PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
    *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
    *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    *     GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    *     BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
    *     OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
    *     TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
    *     THE POSSIBILITY OF SUCH DAMAGE.
    ********************************************************************* */


#include <stdarg.h>

#include "lib_types.h"
#include "lib_string.h"
#include "lib_queue.h"
#include "lib_malloc.h"
#include "lib_printf.h"
#include "lib_setjmp.h"

#include "cfe_iocb.h"
#include "cfe_device.h"
#include "cfe_console.h"
#include "cfe_error.h"
#include "env_subr.h"
#include "cfe.h"
#include "ui_command.h"

/*  *********************************************************************
    *  Types
    ********************************************************************* */


/*  *********************************************************************
    *  Globals
    ********************************************************************* */

static jmp_buf ui_jmpbuf;		/* for getting control in exceptions */



void dumplist(queue_t *qb);
void dumplist(queue_t *qb)
{
    queue_t *q;
    ui_token_t *t;

    for (q = qb->q_next; q != qb; q = q->q_next) {

	t = (ui_token_t *) q;
	printf("[%s] ",&(t->token));
	}
    printf("\n");
}



const char *ui_errstring(int errcode)
{
    return cfe_errortext(errcode);
}


int ui_showerror(int errcode,char *tmplt,...)
{
    va_list marker;

    va_start(marker,tmplt);
    xvprintf(tmplt,marker);
    va_end(marker);
    xprintf(": %s\n",ui_errstring(errcode));

    return errcode;
}



static int ui_do_one_command(queue_t *head)
{
    int res;
    ui_cmdline_t cmd;

    res = cmd_lookup(head, &cmd);

    if (res == 0) {

	res = cmd_sw_validate(&cmd,cmd.switches);
	if (res != -1) {
	    xprintf("Invalid switch: %s\n",
		    cmd_sw_name(&cmd,res));
	    return CFE_ERR_INV_PARAM;
	    }

	if (lib_setjmp(ui_jmpbuf) != 0) return -1;
	res = (*cmd.func)(&cmd,cmd.argc-cmd.argidx,
		    &(cmd.argv[cmd.argidx]));
	}
    cmd_free(&cmd);
    return res;
}


void ui_restart(int arg)
{
    if (arg == 0) arg = -1;

    lib_longjmp(ui_jmpbuf,arg);
}

int ui_init_cmddisp(void)
{
    cmd_init();

    return 0;
}

int ui_showusage(ui_cmdline_t *cmd)
{
    cmd_showusage(cmd);

    return CFE_ERR_INV_COMMAND;
}


/*  *********************************************************************
    *  ui_docommands_internal(head)
    *  
    *  Process (possibly multiple) commands from a list of tokens
    *  
    *  Input parameters: 
    *  	   buf - buffer
    *  	   
    *  Return value:
    *  	   exit status of first command that failed, or null
    ********************************************************************* */
static int ui_docommands_internal(queue_t *head)
{
    queue_t cmdqueue;
    ui_command_t *cmd;
    int status = CMD_ERR_BLANK;
    int term;

    q_init(&cmdqueue);

    /*
     * Find all the individual commands
     */

    while ((cmd = cmd_readcommand(head))) {

	if (cmd == NULL) {
	    return CMD_ERR_BLANK;
	    }

	q_enqueue(&cmdqueue,(queue_t *) cmd);
	}

    /*
     * Do each command
     */

    while ((cmd = (ui_command_t *) q_deqnext(&(cmdqueue)))) {
	status = ui_do_one_command(&(cmd->head));
	term = cmd->term;
	KFREE(cmd);
	if (status == CMD_ERR_BLANK) continue;

	/*
	 * And causes us to stop at the first failure.
	 */
	if ((term == CMD_TERM_AND) && (status != 0)) break;

	/*
	 * OR causes us to stop at the first success.
	 */

	if ((term == CMD_TERM_OR) && (status == 0)) break;

	/*
	 * Neither AND nor OR causes us to keep chugging away.
	 */
	}

    /*
     * Free any remaining tokens and commands that we did not do
     */

    while ((cmd = (ui_command_t *) q_deqnext(&(cmdqueue)))) {
	cmd_free_tokens(&(cmd->head));
	KFREE(cmd);
	}

    return status;
}


int ui_docommands(char *str)
{
    queue_t cmd_list;
    int res;

    /* Convert the command into a token list */
    cmd_build_list(&cmd_list,str);

    /* Walk the list and expand environment variables */
    cmd_walk_and_expand(&cmd_list);

    /* Process each command.  This removes tokens from the list */
    res = ui_docommands_internal(&cmd_list);

    /* Free any leftover tokens.  There should not be any. */
    cmd_free_tokens(&cmd_list);

    return res;
}
