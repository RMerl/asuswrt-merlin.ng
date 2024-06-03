// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */

#include <common.h>
#include <command.h>
#include <image.h>
#include <u-boot/zlib.h>
#include <asm/byteorder.h>
#include <asm/bootm.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SETUP_MEMORY_TAGS) || \
	defined(CONFIG_CMDLINE_TAG) || \
	defined(CONFIG_INITRD_TAG) || \
	defined(CONFIG_SERIAL_TAG) || \
	defined(CONFIG_REVISION_TAG)
static void setup_start_tag(bd_t *bd);

# ifdef CONFIG_SETUP_MEMORY_TAGS
static void setup_memory_tags(bd_t *bd);
# endif
static void setup_commandline_tag(bd_t *bd, char *commandline);

# ifdef CONFIG_INITRD_TAG
static void setup_initrd_tag(bd_t *bd, ulong initrd_start, ulong initrd_end);
# endif
static void setup_end_tag(bd_t *bd);

static struct tag *params;
#endif /* CONFIG_SETUP_MEMORY_TAGS || CONFIG_CMDLINE_TAG || CONFIG_INITRD_TAG */

int do_bootm_linux(int flag, int argc, char *argv[], bootm_headers_t *images)
{
	bd_t	*bd = gd->bd;
	char	*s;
	int	machid = bd->bi_arch_number;
	void	(*theKernel)(int zero, int arch, uint params);

#ifdef CONFIG_CMDLINE_TAG
	char *commandline = env_get("bootargs");
#endif

	/*
	 * allow the PREP bootm subcommand, it is required for bootm to work
	 */
	if (flag & BOOTM_STATE_OS_PREP)
		return 0;

	if ((flag != 0) && (flag != BOOTM_STATE_OS_GO))
		return 1;

	theKernel = (void (*)(int, int, uint))images->ep;

	s = env_get("machid");
	if (s) {
		machid = simple_strtoul(s, NULL, 16);
		printf("Using machid 0x%x from environment\n", machid);
	}

	bootstage_mark(BOOTSTAGE_ID_RUN_OS);

	debug("## Transferring control to Linux (at address %08lx) ...\n",
	       (ulong)theKernel);

	if (IMAGE_ENABLE_OF_LIBFDT && images->ft_len) {
#ifdef CONFIG_OF_LIBFDT
		debug("using: FDT\n");
		if (image_setup_linux(images)) {
			printf("FDT creation failed! hanging...");
			hang();
		}
#endif
	} else if (BOOTM_ENABLE_TAGS) {
#if defined(CONFIG_SETUP_MEMORY_TAGS) || \
	defined(CONFIG_CMDLINE_TAG) || \
	defined(CONFIG_INITRD_TAG) || \
	defined(CONFIG_SERIAL_TAG) || \
	defined(CONFIG_REVISION_TAG)
	setup_start_tag(bd);
#ifdef CONFIG_SERIAL_TAG
	setup_serial_tag(&params);
#endif
#ifdef CONFIG_REVISION_TAG
	setup_revision_tag(&params);
#endif
#ifdef CONFIG_SETUP_MEMORY_TAGS
	setup_memory_tags(bd);
#endif
#ifdef CONFIG_CMDLINE_TAG
	setup_commandline_tag(bd, commandline);
#endif
#ifdef CONFIG_INITRD_TAG
	if (images->rd_start && images->rd_end)
		setup_initrd_tag(bd, images->rd_start, images->rd_end);
#endif
	setup_end_tag(bd);
#endif

	/* we assume that the kernel is in place */
	printf("\nStarting kernel ...\n\n");

#ifdef CONFIG_USB_DEVICE
	{
		extern void udc_disconnect(void);
		udc_disconnect();
	}
#endif
	}
	cleanup_before_linux();
	if (IMAGE_ENABLE_OF_LIBFDT && images->ft_len)
		theKernel(0, machid, (unsigned long)images->ft_addr);
	else
		theKernel(0, machid, bd->bi_boot_params);
	/* does not return */

	return 1;
}

#if defined(CONFIG_SETUP_MEMORY_TAGS) || \
	defined(CONFIG_CMDLINE_TAG) || \
	defined(CONFIG_INITRD_TAG) || \
	defined(CONFIG_SERIAL_TAG) || \
	defined(CONFIG_REVISION_TAG)
static void setup_start_tag(bd_t *bd)
{
	params = (struct tag *)bd->bi_boot_params;

	params->hdr.tag = ATAG_CORE;
	params->hdr.size = tag_size(tag_core);

	params->u.core.flags = 0;
	params->u.core.pagesize = 0;
	params->u.core.rootdev = 0;

	params = tag_next(params);
}

#ifdef CONFIG_SETUP_MEMORY_TAGS
static void setup_memory_tags(bd_t *bd)
{
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		params->hdr.tag = ATAG_MEM;
		params->hdr.size = tag_size(tag_mem32);

		params->u.mem.start = bd->bi_dram[i].start;
		params->u.mem.size = bd->bi_dram[i].size;

		params = tag_next(params);
	}
}
#endif /* CONFIG_SETUP_MEMORY_TAGS */

static void setup_commandline_tag(bd_t *bd, char *commandline)
{
	char *p;

	if (!commandline)
		return;

	/* eat leading white space */
	for (p = commandline; *p == ' '; p++)
		;

	/* skip non-existent command lines so the kernel will still
	 * use its default command line.
	 */
	if (*p == '\0')
		return;

	params->hdr.tag = ATAG_CMDLINE;
	params->hdr.size =
		(sizeof(struct tag_header) + strlen(p) + 1 + 4) >> 2;

	strcpy(params->u.cmdline.cmdline, p)
		;

	params = tag_next(params);
}

#ifdef CONFIG_INITRD_TAG
static void setup_initrd_tag(bd_t *bd, ulong initrd_start, ulong initrd_end)
{
	/* an ATAG_INITRD node tells the kernel where the compressed
	 * ramdisk can be found. ATAG_RDIMG is a better name, actually.
	 */
	params->hdr.tag = ATAG_INITRD2;
	params->hdr.size = tag_size(tag_initrd);

	params->u.initrd.start = initrd_start;
	params->u.initrd.size = initrd_end - initrd_start;

	params = tag_next(params);
}
#endif /* CONFIG_INITRD_TAG */

#ifdef CONFIG_SERIAL_TAG
void setup_serial_tag(struct tag **tmp)
{
	struct tag *params = *tmp;
	struct tag_serialnr serialnr;
	void get_board_serial(struct tag_serialnr *serialnr);

	get_board_serial(&serialnr);
	params->hdr.tag = ATAG_SERIAL;
	params->hdr.size = tag_size(tag_serialnr);
	params->u.serialnr.low = serialnr.low;
	params->u.serialnr.high = serialnr.high;
	params = tag_next(params);
	*tmp = params;
}
#endif

#ifdef CONFIG_REVISION_TAG
void setup_revision_tag(struct tag **in_params)
{
	u32 rev = 0;
	u32 get_board_rev(void);

	rev = get_board_rev();
	params->hdr.tag = ATAG_REVISION;
	params->hdr.size = tag_size(tag_revision);
	params->u.revision.rev = rev;
	params = tag_next(params);
}
#endif  /* CONFIG_REVISION_TAG */

static void setup_end_tag(bd_t *bd)
{
	params->hdr.tag = ATAG_NONE;
	params->hdr.size = 0;
}

#endif /* CONFIG_SETUP_MEMORY_TAGS || CONFIG_CMDLINE_TAG || CONFIG_INITRD_TAG */
