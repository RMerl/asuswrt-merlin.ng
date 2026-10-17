// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Kyle Harris, kharris@nexus-tech.net
 */

/*
 * The "source" command allows to define "script images", i. e. files
 * that contain command sequences that can be executed by the command
 * interpreter. It returns the exit status of the last command
 * executed from the script. This is very similar to running a shell
 * script in a UNIX shell, hence the name for the command.
 */

/* #define DEBUG */

#include <common.h>
#include <command.h>
#include <env.h>
#include <image.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <asm/byteorder.h>
#include <asm/io.h>

static int do_source(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	ulong addr;
	int rcode;
	const char *fit_uname = NULL, *confname = NULL;

	/* Find script image */
	if (argc < 2) {
		addr = CONFIG_SYS_LOAD_ADDR;
		debug("*  source: default load address = 0x%08lx\n", addr);
#if defined(CONFIG_FIT)
	} else if (fit_parse_subimage(argv[1], image_load_addr, &addr,
				      &fit_uname)) {
		debug("*  source: subimage '%s' from FIT image at 0x%08lx\n",
		      fit_uname, addr);
	} else if (fit_parse_conf(argv[1], image_load_addr, &addr, &confname)) {
		debug("*  source: config '%s' from FIT image at 0x%08lx\n",
		      confname, addr);
#endif
	} else {
		addr = hextoul(argv[1], NULL);
		debug("*  source: cmdline image address = 0x%08lx\n", addr);
	}

	printf ("## Executing script at %08lx\n", addr);
	rcode = cmd_source_script(addr, fit_uname, confname);
	return rcode;
}

U_BOOT_LONGHELP(source,
#if defined(CONFIG_FIT)
	"[<addr>][:[<image>]|#[<config>]]\n"
	"\t- Run script starting at addr\n"
	"\t- A FIT config name or subimage name may be specified with : or #\n"
	"\t  (like bootm). If the image or config name is omitted, the\n"
	"\t  default is used."
#else
	"[<addr>]\n"
	"\t- Run script starting at addr"
#endif
	);

U_BOOT_CMD(
	source, 2, 0,	do_source,
	"run script from memory", source_help_text
);
