// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * Copyright (c) 2015 Sjoerd Simons <sjoerd.simons@collabora.co.uk>
 */

#include <bootm.h>
#include <bootstage.h>
#include <image.h>
#include <asm/io.h>

#define	LINUX_ARM_ZIMAGE_MAGIC	0x016f2818

struct arm_z_header {
	uint32_t	code[9];
	uint32_t	zi_magic;
	uint32_t	zi_start;
	uint32_t	zi_end;
} __attribute__ ((__packed__));

int bootz_setup(ulong image, ulong *start, ulong *end)
{
	uint8_t *zimage = map_sysmem(image, 0);
	struct arm_z_header *arm_hdr = (struct arm_z_header *)zimage;
	int ret = 0;

	if (memcmp(zimage + 0x202, "HdrS", 4) == 0) {
		uint8_t setup_sects = *(zimage + 0x1f1);
		uint32_t syssize =
			le32_to_cpu(*(uint32_t *)(zimage + 0x1f4));

		*start = 0;
		*end = (setup_sects + 1) * 512 + syssize * 16;

		printf("setting up X86 zImage [ %ld - %ld ]\n",
		       *start, *end);
	} else if (le32_to_cpu(arm_hdr->zi_magic) == LINUX_ARM_ZIMAGE_MAGIC) {
		*start = le32_to_cpu(arm_hdr->zi_start);
		*end = le32_to_cpu(arm_hdr->zi_end);

		printf("setting up ARM zImage [ %ld - %ld ]\n",
		       *start, *end);
	} else {
		printf("Unrecognized zImage\n");
		ret = 1;
	}

	unmap_sysmem((void *)image);

	return ret;
}

/* Subcommand: PREP */
static int boot_prep_linux(struct bootm_headers *images)
{
	int ret;

	if (IS_ENABLED(CONFIG_LMB)) {
		ret = image_setup_linux(images);
		if (ret)
			return ret;
	}

	return 0;
}

int do_bootm_linux(int flag, struct bootm_info *bmi)
{
	struct bootm_headers *images = bmi->images;

	if (flag & BOOTM_STATE_OS_PREP)
		return boot_prep_linux(images);

	if (flag & (BOOTM_STATE_OS_GO | BOOTM_STATE_OS_FAKE_GO)) {
		bootstage_mark(BOOTSTAGE_ID_RUN_OS);
		printf("## Transferring control to Linux (at address %08lx)...\n",
		       images->ep);
		printf("sandbox: continuing, as we cannot run Linux\n");
	}

	return 0;
}

/* used for testing 'booti' command */
int booti_setup(ulong image, ulong *relocated_addr, ulong *size,
		bool force_reloc)
{
	return 0;
}
