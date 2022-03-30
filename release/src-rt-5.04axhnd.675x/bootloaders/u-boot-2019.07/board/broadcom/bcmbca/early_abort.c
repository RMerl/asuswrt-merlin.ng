/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <linux/ctype.h>
#include <linux/io.h>
#include <spl.h>
#include "early_abort.h"
#include "bcm_secure.h"
#include "spl_ddrinit.h"

static  early_abort_t ea __attribute__((section(".data")));

static char _get_char(void)
{
	return tstc() ? getc():'\0';
}

static void _flush_chars(void)
{
	mdelay(1500);
	while(_get_char() != '\0');
}

static int catch_early_abort(void)
{
	/* listen for .5 s to catch */ 
	uint32_t tm = SPL_EA_CATCH_TM_MS;
	do {
		if( _get_char() == 'a') {
			return 0;
		}
		mdelay(1);
	} while(--tm);
	return 1;
}

static void early_abort_menu(void)
{
	const char *menu = "Use enter to confirm input menu selection\r\n"
		"go - continue\r\n"
		"ddr - mcb override(hex) or DDR safe mode.\r\n"
		"\tddr (3|4) safe modes;\r\n" 
		"\tddr <hex num>  passes selector value \r\n"
		"r - Boot Fallback; 1 - recovery\r\n"
		"bid - Ignore boardid while booting\r\n";
	const char *menu_cont =
		"mcb - List all the mcb selectors\r\n"
#ifdef CONFIG_BCMBCA_EARLY_ABORT_JTAG_UNLOCK
		"jtag - Unlock JTAG\r\n"
#endif
		"h - halt\r\n";

	printf("%s", menu);
	printf("%s", menu_cont);
}

static int _get_chars(char brk, char b[128])
{
	char *s =  b;
	do {
		char c = _get_char();
		if (c == '\0' || (!isalpha(c) && !isdigit(c) && c != brk && !isspace(c))) {
			continue;
		}
		if (c == brk) {
			break;
		}
		putc(c);
		*s++ = c;
	} while(s - b < 127);
	*s = '\0';
	return s-b;
}


static inline char* _skpchr(const char* s, char c)
{
	char* p = (char*)s;
	while(*p == c ) { p++; }
	return p;
}
/* given the char c and tok - search for the pattern:
 *  <tok|$>c<(tok)|\0*>
 *
 * */
static char* _strchr_not_tok(const char* s, char c, char tok) 
{
	char *p = (char*)s;
	if (*p == c) {
		goto done ;
	}
	p = _skpchr(p, tok);
	if (p == s) {	
		return  NULL;
	}
done:
	return (*p == c && (*(p+1) =='\0' || isspace(*(p+1))))? p : NULL;
}

__weak void early_abort(void)
{
	unsigned long tm = SPL_EA_TM_MS; 
	spl_ea_status_t ddr_flags = SPL_EA_NONE,
			ignore_boardid = SPL_EA_NONE,
			image_flags = SPL_EA_NONE;
	int chno = 0;
	/* This function called early so avoiding usage of BS as much as possible.
 	* Stack MUST BE available  
 	* Using global pointer to convei
 	* early abort status
 	* */
	if (catch_early_abort()) {
		return;
	}
	/* print menu */
	memset(&ea, 0, sizeof(ea));
	early_abort_menu();
	_flush_chars();
	do {
		char cbuf[128];
		char* sel = cbuf;
		mdelay(1);
		if (!serial_tstc()) {
			continue;
		}
		chno =  _get_chars('\r', sel);
		sel = _skpchr(sel, ' ');
		if (!strncmp(sel, "ddr", 3)) {
			/* grab hex for selector */
			char * p = _strchr_not_tok(sel + 3, '3', ' ');
			if (p) {
				ddr_flags =  SPL_EA_DDR3_SAFE_MODE;
						printf("\r\nSafe Mode - DDR%c\n",*p);
			} else {
				p = _strchr_not_tok(sel + 3, '4', ' ');
				if (p) {
					ddr_flags =  SPL_EA_DDR4_SAFE_MODE;
					printf("\r\nSafe Mode - DDR%c\n",*p);
				} else {
					char * p = sel + 3;
					unsigned long val;
					p = _skpchr(p, ' ');
					val = simple_strtoul(p, NULL, 16);
					if (val != -EINVAL) {
						ddr_flags = SPL_EA_DDR_MCB_SEL;
						ea.data = val;
						printf("\r\nMCB selector 0x%lx\r\n", val);
					} else {
						ddr_flags =  SPL_EA_DDR3_SAFE_MODE;
						printf("\r\nSafe Mode - DDR3\r\n");
					}
				}
			}
		} else if (!strncmp(sel,"go", 2)) {
			/*done get out*/
			break;
		} else if (*sel == 'r') {
			char * p = _strchr_not_tok(sel + 1 , '1', ' '); 
			image_flags = p && *p=='1'?
				SPL_EA_IMAGE_RECOV : SPL_EA_IMAGE_FB;
			printf("\r\nBoot Image: %s \r\n",
				(image_flags&SPL_EA_IMAGE_RECOV)?"Recovery":"Fallback");
		} else if (*sel == 'h') {
			/*resetting timeout; 
 * 				started polling almost infinitely*/
			tm = -1;
			printf("\r\nHalted\r\n");
#ifdef CONFIG_BCMBCA_EARLY_ABORT_JTAG_UNLOCK
		} else if (*sel == 'j'|| !strncmp(sel,"jtag", 4)) {
			bcm_sec_cb_arg_t cb_args[SEC_CTRL_ARG_MAX] = {0}; 
			cb_args[SEC_CTRL_ARG_SOTP].arg[0].ctrl = SEC_CTRL_SOTP_LOCK_ALL;
			cb_args[SEC_CTRL_ARG_KEY].arg[0].ctrl = SEC_CTRL_KEY_CLEAN_ALL;
			bcm_sec_do(SEC_SET, cb_args);
			BCM_SEC_UNLOCK_JTAG;
			ea.status |= SPL_EA_JTAG_UNLOCK;
			printf("\r\nUnlocked\r\n");
#endif
		} else if (!strncmp(sel, "bid", 3)) {
			ignore_boardid = SPL_EA_IGNORE_BOARDID;
			printf("\r\nIgnore boardid while booting\r\n");
		} else if (!strncmp(sel, "mcb", 3)) {
#ifdef CONFIG_BCMBCA_DDRC
			spl_list_mcb_sel();
#endif
		} else if (chno){
			 
			putc('\r');
			do {
				putc(' ');
			} while(--chno);
			putc('\r');
		} 

	} while(--tm);
	if (ddr_flags || image_flags || ignore_boardid) {
		ea.status |= (ddr_flags|image_flags|ignore_boardid);
	}
	/*printf("Time out counter at %lu\n",tm);*/
}

early_abort_t* early_abort_info()
{
	return &ea;
}
