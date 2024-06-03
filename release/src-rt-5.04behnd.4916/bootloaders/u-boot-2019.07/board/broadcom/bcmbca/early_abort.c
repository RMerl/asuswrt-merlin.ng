/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <linux/ctype.h>
#include <linux/io.h>
#include <spl.h>
#include <xyzModem.h>
#ifdef CONFIG_BCMBCA_EARLY_ABORT_JTAG_UNLOCK
#include <asm/arch/brom.h>
#endif
#include "early_abort.h"
#include "bcm_secure.h"
#include "spl_ddrinit.h"
#include "boot_flash.h"

DECLARE_GLOBAL_DATA_PTR;

static early_abort_t ea __attribute__((section(".data")));
#ifdef CONFIG_BCMBCA_EARLY_ABORT_LOADB
static uint32_t* lb_addr __attribute__((section(".data"))) = NULL;
typedef void (*image_entry_t)(void);
#endif

#ifdef CONFIG_BCMBCA_EARLY_ABORT_DDRINIT
void spl_board_ddrinit(early_abort_t*);
#endif

static char _get_char(void)
{
	return (char)(tstc()&0xff) ? (char)(getc()&0xff):'\0';
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
#ifdef CONFIG_BCMBCA_EARLY_ABORT_LOADB
		"go - continue or run from uart loaded binary\r\n"
#else
		"go - continue\r\n"
#endif
		"ddr - mcb override(hex) or DDR safe mode.\r\n"
		"\tddr (3"
#if defined(CONFIG_BCMBCA_DDR4)
		"|4"
#endif
#if defined(CONFIG_BCMBCA_LPDDR4)
		"|lp4|lp4x"
#endif
#if defined(CONFIG_BCMBCA_LPDDR5)
		"|lp5|lp5x"
#endif
		") safe modes;\r\n"
		"\tddr <hex num>  passes selector value \r\n"
		"r - Boot Fallback; 1 - recovery\r\n"
		"bid - Ignore boardid while booting\r\n";
	const char *menu_cont =
#ifdef CONFIG_BCMBCA_DDRC	  
		"mcb - List all the mcb selectors\r\n"
#ifdef CONFIG_BCMBCA_EARLY_ABORT_DDRINIT
		"id - Initialize DDR\r\n"
#endif
#endif
#ifdef CONFIG_BCMBCA_EARLY_ABORT_LOADB
		"lb <hex addr> <dec baudrate> - Load binary using Ymodem\r\n"
#endif
#ifdef CONFIG_BCMBCA_EARLY_ABORT_JTAG_UNLOCK
		"jtag - Unlock JTAG\r\n"
#endif
#if defined(CONFIG_SPL_WATCHDOG_SUPPORT) && defined(CONFIG_WDT)
		/* Option 2 - disable spl wdt on user selection */
		"wdt - Disable spl wdt\r\n"
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
		if (c == '\0' || (c > 0 && !isalpha(c) && !isdigit(c) && c != brk && !isspace(c))) {
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

#ifdef CONFIG_BCMBCA_EARLY_ABORT_LOADB
static int getcxmodem(void) {
	if (tstc())
		return (getc());
	return -1;
}

static int load_binary_ymodem(uint32_t* lb_addr, int load_baudrate)
{
	int size;
	int err;
	int res;
	connection_info_t info;
	char* buf = (char*)lb_addr;
	int current_baudrate;

	current_baudrate = gd->baudrate;
	if (load_baudrate != current_baudrate) {
		printf("## Switch baudrate to %d bps and press ENTER ...\n",
			load_baudrate);
		udelay(50000);
		gd->baudrate = load_baudrate;
		serial_setbrg();
		udelay(50000);
		for (;;) {
			if (getc() == '\r')
				break;
		}
	}
	printf("## Ready for binary (ymodem) download to 0x%px at %d bps...\n",
		lb_addr, load_baudrate);

	size = 0;
	info.mode = xyzModem_ymodem;
	res = xyzModem_stream_open(&info, &err);
	if (!res) {
		while ((res =
			xyzModem_stream_read(buf, 1024, &err)) > 0) {
			size += res;
			buf += res;
		}
	} else {
		printf("yModem error %s\n", xyzModem_error(err));
	}

	xyzModem_stream_close(&err);
	xyzModem_stream_terminate(false, &getcxmodem);

	printf("Ymodem load total Size %d Bytes at 0x%px\n", size, lb_addr);

	if (load_baudrate != current_baudrate) {
		printf("## Switch baudrate back to %d bps and press ESC ...\n",
			current_baudrate);
		udelay(50000);
		gd->baudrate = current_baudrate;
		serial_setbrg();
		udelay(50000);
		for (;;) {
			if (getc() == 0x1B) /* ESC */
				break;
		}
		printf("Done\n");
	}

	return 0;
}

static void early_abort_load_binary(char* sel)
{  
	char * paddr = sel + 2;
	char * pbaud = NULL;
	unsigned long val;
	int baud_rate;

	if(bcm_sec_state() != SEC_STATE_UNSEC) {
#if !defined(CONFIG_BCM6878)
		printf("load binary not supported in secure mode!\n");
#endif
		return;
	}

	/*
	 * MMU is not enabled in early abort. Need to enable i-cache 
	 * to speed up binary load. Otherwise on slow v7 platform, 
	 * Y-modem always time out.
	 */
	icache_enable();

	baud_rate = gd->baudrate;
	lb_addr = (uint32_t*)0x01000000;
	
	paddr = _skpchr(paddr, ' ');
	if (paddr) {
		pbaud = strchr(paddr, ' ');
		if(pbaud != NULL)
			pbaud = _skpchr(pbaud, ' ');
	}

	if (paddr && pbaud) {
		if (strict_strtoul(paddr, 16, &val) != -EINVAL) {
			lb_addr = (uint32_t*)val;
		}
		if (strict_strtoul(pbaud, 10, &val) != -EINVAL) {
			baud_rate = (int)val;
		}
	}

	printf("\nPlease use YMODME program to send the binary now\n");
	load_binary_ymodem(lb_addr, baud_rate);

	invalidate_icache_all();
}
#endif

#ifdef CONFIG_BCMBCA_EARLY_ABORT_DDRINIT
static void early_abort_clear_bss(void)
{
	char *cp = __bss_start;

	/* Zero out BSS */
	while (cp < __bss_end)
		*cp++ = 0;
}

static void early_abort_ddr_init(early_abort_t* ea)
{
	/* 
	 * early abort is called before the c env is ready, need to 
	 * manually clear bss and call other necessary init code 
	 * that ddr init need. This also mean the normal execution
	 * after ddr init will not work because u-boot will clear bss
	 * again so we don't support such senario.  The only cmd will
	 * work is 'lb' and 'go' to run the loaded binary.
	 */
	early_abort_clear_bss();
	/*
	 * MMU is not enabled in early abort. But we can enable i-cache 
	 * to speed up ddr init 
	 */
	icache_enable();
	boot_flash_init();
	jumptable_init();
	spl_board_ddrinit(ea);
}
#endif

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
		if (!strncmp(sel, "ddr ", 4)) {
			/* skip all the spaces */
			char * p = _skpchr(sel + 3, ' ');
			if (p) {
				/* clear ddr flag first */
			  	ea.status &= ~(SPL_EA_DDR_SAFE_MODE_MASK|SPL_EA_DDR_MCB_SEL);
				if (!strcmp(p, "3")) {
					ddr_flags =  SPL_EA_DDR3_SAFE_MODE;
					printf("\r\nSafe Mode - DDR3\n");
				}
#if defined(CONFIG_BCMBCA_DDR4)
				else if (!strcmp(p, "4")) {
					ddr_flags =  SPL_EA_DDR4_SAFE_MODE;
					printf("\r\nSafe Mode - DDR4\n");
				}
#endif
#if defined(CONFIG_BCMBCA_LPDDR4)
				else if (!strcmp(p, "lp4")) {
					ddr_flags =  SPL_EA_LPDDR4_SAFE_MODE;
					printf("\r\nSafe Mode - LPDDR4\n");
				}
				else if (!strcmp(p, "lp4x")) {
					ddr_flags =  SPL_EA_LPDDR4X_SAFE_MODE;
					printf("\r\nSafe Mode - LPDDR4X\n");
				}
#endif
#if defined(CONFIG_BCMBCA_LPDDR5)
				else if (!strcmp(p, "lp5")) {
					ddr_flags =  SPL_EA_LPDDR5_SAFE_MODE;
					printf("\r\nSafe Mode - LPDDR5\n");
				}
				else if (!strcmp(p, "lp5x")) {
					ddr_flags =  SPL_EA_LPDDR5X_SAFE_MODE;
					printf("\r\nSafe Mode - LPDDR5X\n");
				}
#endif
				else {
					unsigned long val;
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
				ea.status |= ddr_flags;
			}
		} else if (!strncmp(sel,"go", 2)) {
#ifdef CONFIG_BCMBCA_EARLY_ABORT_LOADB
			if (lb_addr) {
				image_entry_t image_entry = (image_entry_t)lb_addr;
				image_entry();
			}
#endif
			/*done get out*/
			break;
		} else if (*sel == 'r') {
			char * p = _strchr_not_tok(sel + 1 , '1', ' '); 
			image_flags = p && *p=='1'?
				SPL_EA_IMAGE_RECOV : SPL_EA_IMAGE_FB;
			printf("\r\nBoot Image: %s \r\n",
				(image_flags&SPL_EA_IMAGE_RECOV)?"Recovery":"Fallback");
		} else if (*sel == 'h') {
			/* resetting timeout; 
			 * started polling almost infinitely*/
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
#if defined(CONFIG_SPL_WATCHDOG_SUPPORT) && defined(CONFIG_WDT)
		} else if (*sel == 'w' || !strncmp(sel,"wdt", 3)) {
			/* Option 2 - disable spl wdt on user selection */
			ea.status |= SPL_EA_WDT_DISABLE;
			printf("\r\nWDT disabled in SPL\r\n");
#endif		
		} else if (!strncmp(sel, "bid", 3)) {
			ignore_boardid = SPL_EA_IGNORE_BOARDID;
			printf("\r\nIgnore boardid while booting\r\n");
#ifdef CONFIG_BCMBCA_DDRC
		} else if (!strncmp(sel, "mcb", 3)) {
			spl_list_mcb_sel();
#ifdef CONFIG_BCMBCA_EARLY_ABORT_DDRINIT
		} else if (!strncmp(sel, "id", 2)) {
		  early_abort_ddr_init(&ea);
#endif
#endif
#ifdef CONFIG_BCMBCA_EARLY_ABORT_LOADB
		} else if (!strncmp(sel, "lb", 2)) {
			early_abort_load_binary(sel);
#endif
		} else if (chno) {
			 
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
