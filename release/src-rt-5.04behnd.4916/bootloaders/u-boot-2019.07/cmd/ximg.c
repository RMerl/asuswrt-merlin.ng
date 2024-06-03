// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2003
 * Kai-Uwe Bloem, Auerswald GmbH & Co KG, <linux-development@auerswald.de>
 */


/*
 * Multi Image extract
 */
#include <common.h>
#include <command.h>
#include <image.h>
#include <mapmem.h>
#include <watchdog.h>
#if defined(CONFIG_BZIP2)
#include <bzlib.h>
#endif
#include <asm/byteorder.h>
#include <asm/io.h>

#ifndef CONFIG_SYS_XIMG_LEN
/* use 8MByte as default max gunzip size */
#define CONFIG_SYS_XIMG_LEN	0x800000
#endif

static int
do_imgextract(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	ulong		addr = load_addr;
	ulong		dest = 0;
	ulong		data, len;
	int		verify;
	int		part = 0;
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
	ulong		count;
	image_header_t	*hdr = NULL;
#endif
#if defined(CONFIG_FIT)
	const char	*uname = NULL;
	const void*	fit_hdr;
	int		noffset;
	const void	*fit_data;
	size_t		fit_len;
#endif
#ifdef CONFIG_GZIP
	uint		unc_len = CONFIG_SYS_XIMG_LEN;
#endif
	uint8_t		comp;

	verify = env_get_yesno("verify");

	if (argc > 1) {
		addr = simple_strtoul(argv[1], NULL, 16);
	}
	if (argc > 2) {
		part = simple_strtoul(argv[2], NULL, 16);
#if defined(CONFIG_FIT)
		uname = argv[2];
#endif
	}
	if (argc > 3) {
		dest = simple_strtoul(argv[3], NULL, 16);
	}

	switch (genimg_get_format((void *)addr)) {
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
	case IMAGE_FORMAT_LEGACY:

		printf("## Copying part %d from legacy image "
			"at %08lx ...\n", part, addr);

		hdr = (image_header_t *)addr;
		if (!image_check_magic(hdr)) {
			printf("Bad Magic Number\n");
			return 1;
		}

		if (!image_check_hcrc(hdr)) {
			printf("Bad Header Checksum\n");
			return 1;
		}
#ifdef DEBUG
		image_print_contents(hdr);
#endif

		if (!image_check_type(hdr, IH_TYPE_MULTI) &&
		    !image_check_type(hdr, IH_TYPE_SCRIPT)) {
			printf("Wrong Image Type for %s command\n",
					cmdtp->name);
			return 1;
		}

		comp = image_get_comp(hdr);
		if ((comp != IH_COMP_NONE) && (argc < 4)) {
			printf("Must specify load address for %s command "
					"with compressed image\n",
					cmdtp->name);
			return 1;
		}

		if (verify) {
			printf("   Verifying Checksum ... ");
			if (!image_check_dcrc(hdr)) {
				printf("Bad Data CRC\n");
				return 1;
			}
			printf("OK\n");
		}

		count = image_multi_count(hdr);
		if (part >= count) {
			printf("Bad Image Part\n");
			return 1;
		}

		image_multi_getimg(hdr, part, &data, &len);
		break;
#endif
#if defined(CONFIG_FIT)
	case IMAGE_FORMAT_FIT:
		if (uname == NULL) {
			puts("No FIT subimage unit name\n");
			return 1;
		}

		printf("## Copying '%s' subimage from FIT image "
			"at %08lx ...\n", uname, addr);

		fit_hdr = (const void *)addr;
		if (!fit_check_format(fit_hdr)) {
			puts("Bad FIT image format\n");
			return 1;
		}

		/* get subimage node offset */
		noffset = fit_image_get_node(fit_hdr, uname);
		if (noffset < 0) {
			printf("Can't find '%s' FIT subimage\n", uname);
			return 1;
		}

		if (!fit_image_check_comp(fit_hdr, noffset, IH_COMP_NONE)
		    && (argc < 4)) {
			printf("Must specify load address for %s command "
				"with compressed image\n",
				cmdtp->name);
			return 1;
		}

		/* verify integrity */
		if (verify) {
			if (!fit_image_verify(fit_hdr, noffset)) {
				puts("Bad Data Hash\n");
				return 1;
			}
		}

		/* get subimage/external data address and length */
		if (fit_image_get_data_and_size(fit_hdr, noffset,
					       &fit_data, &fit_len)) {
			puts("Could not find script subimage data\n");
			return 1;
		}

		if (fit_image_get_comp(fit_hdr, noffset, &comp)) {
			puts("Could not find script subimage "
				"compression type\n");
			return 1;
		}

		data = (ulong)fit_data;
		len = (ulong)fit_len;
		break;
#endif
	default:
		puts("Invalid image type for imxtract\n");
		return 1;
	}

	if (argc > 3) {
		switch (comp) {
		case IH_COMP_NONE:
#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
			{
				size_t l = len;
				size_t tail;
				void *to = (void *) dest;
				void *from = (void *)data;

				printf("   Loading part %d ... ", part);

				while (l > 0) {
					tail = (l > CHUNKSZ) ? CHUNKSZ : l;
					WATCHDOG_RESET();
					memmove(to, from, tail);
					to += tail;
					from += tail;
					l -= tail;
				}
			}
#else	/* !(CONFIG_HW_WATCHDOG || CONFIG_WATCHDOG) */
			printf("   Loading part %d ... ", part);
			memmove((char *) dest, (char *)data, len);
#endif	/* CONFIG_HW_WATCHDOG || CONFIG_WATCHDOG */
			break;
#ifdef CONFIG_GZIP
		case IH_COMP_GZIP:
			printf("   Uncompressing part %d ... ", part);
			if (gunzip((void *) dest, unc_len,
				   (uchar *) data, &len) != 0) {
				puts("GUNZIP ERROR - image not loaded\n");
				return 1;
			}
			break;
#endif
#if defined(CONFIG_BZIP2) && defined(CONFIG_IMAGE_FORMAT_LEGACY)
		case IH_COMP_BZIP2:
			{
				int i;

				printf("   Uncompressing part %d ... ", part);
				/*
				 * If we've got less than 4 MB of malloc()
				 * space, use slower decompression algorithm
				 * which requires at most 2300 KB of memory.
				 */
				i = BZ2_bzBuffToBuffDecompress(
					map_sysmem(ntohl(hdr->ih_load), 0),
					&unc_len, (char *)data, len,
					CONFIG_SYS_MALLOC_LEN < (4096 * 1024),
					0);
				if (i != BZ_OK) {
					printf("BUNZIP2 ERROR %d - "
						"image not loaded\n", i);
					return 1;
				}
			}
			break;
#endif /* CONFIG_BZIP2 */
		default:
			printf("Unimplemented compression type %d\n", comp);
			return 1;
		}
		puts("OK\n");
	}

	flush_cache(dest, ALIGN(len, ARCH_DMA_MINALIGN));

	env_set_hex("fileaddr", data);
	env_set_hex("filesize", len);

	return 0;
}

#ifdef CONFIG_SYS_LONGHELP
static char imgextract_help_text[] =
	"addr part [dest]\n"
	"    - extract <part> from legacy image at <addr> and copy to <dest>"
#if defined(CONFIG_FIT)
	"\n"
	"addr uname [dest]\n"
	"    - extract <uname> subimage from FIT image at <addr> and copy to <dest>"
#endif
	"";
#endif

U_BOOT_CMD(
	imxtract, 4, 1, do_imgextract,
	"extract a part of a multi-image", imgextract_help_text
);
