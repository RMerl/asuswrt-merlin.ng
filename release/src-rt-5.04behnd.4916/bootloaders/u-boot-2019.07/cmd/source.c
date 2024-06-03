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
#include <image.h>
#include <malloc.h>
#include <mapmem.h>
#include <asm/byteorder.h>
#include <asm/io.h>

#if defined(CONFIG_FIT)
/**
 * get_default_image() - Return default property from /images
 *
 * Return: Pointer to value of default property (or NULL)
 */
static const char *get_default_image(const void *fit)
{
	int images_noffset;

	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images_noffset < 0)
		return NULL;

	return fdt_getprop(fit, images_noffset, FIT_DEFAULT_PROP, NULL);
}
#endif

int
source (ulong addr, const char *fit_uname)
{
	ulong		len;
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
	const image_header_t *hdr;
#endif
	u32		*data;
	int		verify;
	void *buf;
#if defined(CONFIG_FIT)
	const void*	fit_hdr;
	int		noffset;
	const void	*fit_data;
	size_t		fit_len;
#endif

	verify = env_get_yesno("verify");

	buf = map_sysmem(addr, 0);
	switch (genimg_get_format(buf)) {
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
	case IMAGE_FORMAT_LEGACY:
		hdr = buf;

		if (!image_check_magic (hdr)) {
			puts ("Bad magic number\n");
			return 1;
		}

		if (!image_check_hcrc (hdr)) {
			puts ("Bad header crc\n");
			return 1;
		}

		if (verify) {
			if (!image_check_dcrc (hdr)) {
				puts ("Bad data crc\n");
				return 1;
			}
		}

		if (!image_check_type (hdr, IH_TYPE_SCRIPT)) {
			puts ("Bad image type\n");
			return 1;
		}

		/* get length of script */
		data = (u32 *)image_get_data (hdr);

		if ((len = uimage_to_cpu (*data)) == 0) {
			puts ("Empty Script\n");
			return 1;
		}

		/*
		 * scripts are just multi-image files with one component, seek
		 * past the zero-terminated sequence of image lengths to get
		 * to the actual image data
		 */
		while (*data++);
		break;
#endif
#if defined(CONFIG_FIT)
	case IMAGE_FORMAT_FIT:
		fit_hdr = buf;
		if (!fit_check_format (fit_hdr)) {
			puts ("Bad FIT image format\n");
			return 1;
		}

		if (!fit_uname)
			fit_uname = get_default_image(fit_hdr);

		if (!fit_uname) {
			puts("No FIT subimage unit name\n");
			return 1;
		}

		/* get script component image node offset */
		noffset = fit_image_get_node (fit_hdr, fit_uname);
		if (noffset < 0) {
			printf ("Can't find '%s' FIT subimage\n", fit_uname);
			return 1;
		}

		if (!fit_image_check_type (fit_hdr, noffset, IH_TYPE_SCRIPT)) {
			puts ("Not a image image\n");
			return 1;
		}

		/* verify integrity */
		if (verify) {
			if (!fit_image_verify(fit_hdr, noffset)) {
				puts ("Bad Data Hash\n");
				return 1;
			}
		}

		/* get script subimage data address and length */
		if (fit_image_get_data (fit_hdr, noffset, &fit_data, &fit_len)) {
			puts ("Could not find script subimage data\n");
			return 1;
		}

		data = (u32 *)fit_data;
		len = (ulong)fit_len;
		break;
#endif
	default:
		puts ("Wrong image format for \"source\" command\n");
		return 1;
	}

	debug ("** Script length: %ld\n", len);
	return run_command_list((char *)data, len, 0);
}

/**************************************************/
#if defined(CONFIG_CMD_SOURCE)
static int do_source(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong addr;
	int rcode;
	const char *fit_uname = NULL;

	/* Find script image */
	if (argc < 2) {
		addr = CONFIG_SYS_LOAD_ADDR;
		debug ("*  source: default load address = 0x%08lx\n", addr);
#if defined(CONFIG_FIT)
	} else if (fit_parse_subimage (argv[1], load_addr, &addr, &fit_uname)) {
		debug ("*  source: subimage '%s' from FIT image at 0x%08lx\n",
				fit_uname, addr);
#endif
	} else {
		addr = simple_strtoul(argv[1], NULL, 16);
		debug ("*  source: cmdline image address = 0x%08lx\n", addr);
	}

	printf ("## Executing script at %08lx\n", addr);
	rcode = source (addr, fit_uname);
	return rcode;
}

#ifdef CONFIG_SYS_LONGHELP
static char source_help_text[] =
	"[addr]\n"
	"\t- run script starting at addr\n"
	"\t- A valid image header must be present"
#if defined(CONFIG_FIT)
	"\n"
	"For FIT format uImage addr must include subimage\n"
	"unit name in the form of addr:<subimg_uname>"
#endif
	"";
#endif

U_BOOT_CMD(
	source, 2, 0,	do_source,
	"run script from memory", source_help_text
);
#endif
