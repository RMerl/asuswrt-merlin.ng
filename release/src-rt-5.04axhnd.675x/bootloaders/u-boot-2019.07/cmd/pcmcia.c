// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 ********************************************************************
 *
 * Lots of code copied from:
 *
 * m8xx_pcmcia.c - Linux PCMCIA socket driver for the mpc8xx series.
 * (C) 1999-2000 Magnus Damm <damm@bitsmart.com>
 *
 * "The ExCA standard specifies that socket controllers should provide
 * two IO and five memory windows per socket, which can be independently
 * configured and positioned in the host address space and mapped to
 * arbitrary segments of card address space. " - David A Hinds. 1999
 *
 * This controller does _not_ meet the ExCA standard.
 *
 * m8xx pcmcia controller brief info:
 * + 8 windows (attrib, mem, i/o)
 * + up to two slots (SLOT_A and SLOT_B)
 * + inputpins, outputpins, event and mask registers.
 * - no offset register. sigh.
 *
 * Because of the lacking offset register we must map the whole card.
 * We assign each memory window PCMCIA_MEM_WIN_SIZE address space.
 * Make sure there is (PCMCIA_MEM_WIN_SIZE * PCMCIA_MEM_WIN_NO
 * * PCMCIA_SOCKETS_NO) bytes at PCMCIA_MEM_WIN_BASE.
 * The i/o windows are dynamically allocated at PCMCIA_IO_WIN_BASE.
 * They are maximum 64KByte each...
 */

/* #define DEBUG	1	*/

/*
 * PCMCIA support
 */
#include <common.h>
#include <command.h>
#include <config.h>
#include <pcmcia.h>
#include <asm/io.h>

/* -------------------------------------------------------------------- */

#if defined(CONFIG_CMD_PCMCIA)

extern int pcmcia_on (void);
extern int pcmcia_off (void);

int do_pinit (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int rcode = 0;

	if (argc != 2) {
		printf ("Usage: pinit {on | off}\n");
		return 1;
	}
	if (strcmp(argv[1],"on") == 0) {
		rcode = pcmcia_on ();
	} else if (strcmp(argv[1],"off") == 0) {
		rcode = pcmcia_off ();
	} else {
		printf ("Usage: pinit {on | off}\n");
		return 1;
	}

	return rcode;
}

U_BOOT_CMD(
	pinit,	2,	0,	do_pinit,
	"PCMCIA sub-system",
	"on  - power on PCMCIA socket\n"
	"pinit off - power off PCMCIA socket"
);

#endif

/* -------------------------------------------------------------------- */

#undef	CHECK_IDE_DEVICE

#if	defined(CONFIG_PXA_PCMCIA)
#define	CHECK_IDE_DEVICE
#endif

#ifdef	CHECK_IDE_DEVICE

int		ide_devices_found;
static uchar	*known_cards[] = {
	(uchar *)"ARGOSY PnPIDE D5",
	NULL
};

#define	MAX_TUPEL_SZ	512
#define MAX_FEATURES	4

#define MAX_IDENT_CHARS		64
#define	MAX_IDENT_FIELDS	4

#define	indent	"\t   "

static void print_funcid (int func)
{
	puts (indent);
	switch (func) {
		case CISTPL_FUNCID_MULTI:
			puts (" Multi-Function");
			break;
		case CISTPL_FUNCID_MEMORY:
			puts (" Memory");
			break;
		case CISTPL_FUNCID_SERIAL:
			puts (" Serial Port");
			break;
		case CISTPL_FUNCID_PARALLEL:
			puts (" Parallel Port");
			break;
		case CISTPL_FUNCID_FIXED:
			puts (" Fixed Disk");
			break;
		case CISTPL_FUNCID_VIDEO:
			puts (" Video Adapter");
			break;
		case CISTPL_FUNCID_NETWORK:
			puts (" Network Adapter");
			break;
		case CISTPL_FUNCID_AIMS:
			puts (" AIMS Card");
			break;
		case CISTPL_FUNCID_SCSI:
			puts (" SCSI Adapter");
			break;
		default:
			puts (" Unknown");
			break;
	}
	puts (" Card\n");
}

static void print_fixed (volatile uchar *p)
{
	if (p == NULL)
		return;

	puts(indent);

	switch (*p) {
		case CISTPL_FUNCE_IDE_IFACE:
		{   uchar iface = *(p+2);

		puts ((iface == CISTPL_IDE_INTERFACE) ? " IDE" : " unknown");
		puts (" interface ");
		break;
		}
		case CISTPL_FUNCE_IDE_MASTER:
		case CISTPL_FUNCE_IDE_SLAVE:
		{   uchar f1 = *(p+2);
		uchar f2 = *(p+4);

		puts ((f1 & CISTPL_IDE_SILICON) ? " [silicon]" : " [rotating]");

		if (f1 & CISTPL_IDE_UNIQUE)
			puts (" [unique]");

		puts ((f1 & CISTPL_IDE_DUAL) ? " [dual]" : " [single]");

		if (f2 & CISTPL_IDE_HAS_SLEEP)
			puts (" [sleep]");

		if (f2 & CISTPL_IDE_HAS_STANDBY)
			puts (" [standby]");

		if (f2 & CISTPL_IDE_HAS_IDLE)
			puts (" [idle]");

		if (f2 & CISTPL_IDE_LOW_POWER)
			puts (" [low power]");

		if (f2 & CISTPL_IDE_REG_INHIBIT)
			puts (" [reg inhibit]");

		if (f2 & CISTPL_IDE_HAS_INDEX)
			puts (" [index]");

		if (f2 & CISTPL_IDE_IOIS16)
			puts (" [IOis16]");

		break;
		}
	}
	putc ('\n');
}

static int identify  (volatile uchar *p)
{
	uchar id_str[MAX_IDENT_CHARS];
	uchar data;
	uchar *t;
	uchar **card;
	int i, done;

	if (p == NULL)
		return (0);	/* Don't know */

	t = id_str;
	done =0;

	for (i=0; i<=4 && !done; ++i, p+=2) {
		while ((data = *p) != '\0') {
			if (data == 0xFF) {
				done = 1;
				break;
			}
			*t++ = data;
			if (t == &id_str[MAX_IDENT_CHARS-1]) {
				done = 1;
				break;
			}
			p += 2;
		}
		if (!done)
			*t++ = ' ';
	}
	*t = '\0';
	while (--t > id_str) {
		if (*t == ' ')
			*t = '\0';
		else
			break;
	}
	puts ((char *)id_str);
	putc ('\n');

	for (card=known_cards; *card; ++card) {
		debug ("## Compare against \"%s\"\n", *card);
		if (strcmp((char *)*card, (char *)id_str) == 0) {	/* found! */
			debug ("## CARD FOUND ##\n");
			return (1);
		}
	}

	return (0);	/* don't know */
}

int check_ide_device (int slot)
{
	volatile uchar *ident = NULL;
	volatile uchar *feature_p[MAX_FEATURES];
	volatile uchar *p, *start, *addr;
	int n_features = 0;
	uchar func_id = ~0;
	uchar code, len;
	ushort config_base = 0;
	int found = 0;
	int i;

	addr = (volatile uchar *)(CONFIG_SYS_PCMCIA_MEM_ADDR +
				  CONFIG_SYS_PCMCIA_MEM_SIZE * (slot * 4));
	debug ("PCMCIA MEM: %08lX\n", (ulong)addr);

	start = p = (volatile uchar *) addr;

	while ((p - start) < MAX_TUPEL_SZ) {

		code = *p; p += 2;

		if (code == 0xFF) { /* End of chain */
			break;
		}

		len = *p; p += 2;
#if defined(DEBUG) && (DEBUG > 1)
		{ volatile uchar *q = p;
			printf ("\nTuple code %02x  length %d\n\tData:",
				code, len);

			for (i = 0; i < len; ++i) {
				printf (" %02x", *q);
				q+= 2;
			}
		}
#endif	/* DEBUG */
		switch (code) {
		case CISTPL_VERS_1:
			ident = p + 4;
			break;
		case CISTPL_FUNCID:
			/* Fix for broken SanDisk which may have 0x80 bit set */
			func_id = *p & 0x7F;
			break;
		case CISTPL_FUNCE:
			if (n_features < MAX_FEATURES)
				feature_p[n_features++] = p;
			break;
		case CISTPL_CONFIG:
			config_base = (*(p+6) << 8) + (*(p+4));
			debug ("\n## Config_base = %04x ###\n", config_base);
		default:
			break;
		}
		p += 2 * len;
	}

	found = identify (ident);

	if (func_id != ((uchar)~0)) {
		print_funcid (func_id);

		if (func_id == CISTPL_FUNCID_FIXED)
			found = 1;
		else
			return (1);	/* no disk drive */
	}

	for (i=0; i<n_features; ++i) {
		print_fixed (feature_p[i]);
	}

	if (!found) {
		printf ("unknown card type\n");
		return (1);
	}

	ide_devices_found |= (1 << slot);

	/* set I/O area in config reg -> only valid for ARGOSY D5!!! */
	*((uchar *)(addr + config_base)) = 1;
#if 0
	printf("\n## Config_base = %04x ###\n", config_base);
	printf("Configuration Option Register: %02x @ %x\n", readb(addr + config_base), addr + config_base);
	printf("Card Configuration and Status Register: %02x\n", readb(addr + config_base + 2));
	printf("Pin Replacement Register Register: %02x\n", readb(addr + config_base + 4));
	printf("Socket and Copy Register: %02x\n", readb(addr + config_base + 6));
#endif
	return (0);
}

#endif	/* CHECK_IDE_DEVICE */
