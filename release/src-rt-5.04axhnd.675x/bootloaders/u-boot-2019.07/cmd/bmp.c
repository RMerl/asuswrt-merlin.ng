// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de.
 */

/*
 * BMP handling routines
 */

#include <common.h>
#include <dm.h>
#include <lcd.h>
#include <mapmem.h>
#include <bmp_layout.h>
#include <command.h>
#include <asm/byteorder.h>
#include <malloc.h>
#include <mapmem.h>
#include <splash.h>
#include <video.h>

static int bmp_info (ulong addr);

/*
 * Allocate and decompress a BMP image using gunzip().
 *
 * Returns a pointer to the decompressed image data. This pointer is
 * aligned to 32-bit-aligned-address + 2.
 * See doc/README.displaying-bmps for explanation.
 *
 * The allocation address is passed to 'alloc_addr' and must be freed
 * by the caller after use.
 *
 * Returns NULL if decompression failed, or if the decompressed data
 * didn't contain a valid BMP signature.
 */
#ifdef CONFIG_VIDEO_BMP_GZIP
struct bmp_image *gunzip_bmp(unsigned long addr, unsigned long *lenp,
			     void **alloc_addr)
{
	void *dst;
	unsigned long len;
	struct bmp_image *bmp;

	/*
	 * Decompress bmp image
	 */
	len = CONFIG_SYS_VIDEO_LOGO_MAX_SIZE;
	/* allocate extra 3 bytes for 32-bit-aligned-address + 2 alignment */
	dst = malloc(CONFIG_SYS_VIDEO_LOGO_MAX_SIZE + 3);
	if (dst == NULL) {
		puts("Error: malloc in gunzip failed!\n");
		return NULL;
	}

	bmp = dst;

	/* align to 32-bit-aligned-address + 2 */
	bmp = (struct bmp_image *)((((uintptr_t)dst + 1) & ~3) + 2);

	if (gunzip(bmp, CONFIG_SYS_VIDEO_LOGO_MAX_SIZE, map_sysmem(addr, 0),
		   &len) != 0) {
		free(dst);
		return NULL;
	}
	if (len == CONFIG_SYS_VIDEO_LOGO_MAX_SIZE)
		puts("Image could be truncated"
				" (increase CONFIG_SYS_VIDEO_LOGO_MAX_SIZE)!\n");

	/*
	 * Check for bmp mark 'BM'
	 */
	if (!((bmp->header.signature[0] == 'B') &&
	      (bmp->header.signature[1] == 'M'))) {
		free(dst);
		return NULL;
	}

	debug("Gzipped BMP image detected!\n");

	*alloc_addr = dst;
	return bmp;
}
#else
struct bmp_image *gunzip_bmp(unsigned long addr, unsigned long *lenp,
			     void **alloc_addr)
{
	return NULL;
}
#endif

static int do_bmp_info(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	ulong addr;

	switch (argc) {
	case 1:		/* use load_addr as default address */
		addr = load_addr;
		break;
	case 2:		/* use argument */
		addr = simple_strtoul(argv[1], NULL, 16);
		break;
	default:
		return CMD_RET_USAGE;
	}

	return (bmp_info(addr));
}

static int do_bmp_display(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	ulong addr;
	int x = 0, y = 0;

	splash_get_pos(&x, &y);

	switch (argc) {
	case 1:		/* use load_addr as default address */
		addr = load_addr;
		break;
	case 2:		/* use argument */
		addr = simple_strtoul(argv[1], NULL, 16);
		break;
	case 4:
		addr = simple_strtoul(argv[1], NULL, 16);
		if (!strcmp(argv[2], "m"))
			x = BMP_ALIGN_CENTER;
		else
			x = simple_strtoul(argv[2], NULL, 10);
		if (!strcmp(argv[3], "m"))
			y = BMP_ALIGN_CENTER;
		else
			y = simple_strtoul(argv[3], NULL, 10);
		break;
	default:
		return CMD_RET_USAGE;
	}

	 return (bmp_display(addr, x, y));
}

static cmd_tbl_t cmd_bmp_sub[] = {
	U_BOOT_CMD_MKENT(info, 3, 0, do_bmp_info, "", ""),
	U_BOOT_CMD_MKENT(display, 5, 0, do_bmp_display, "", ""),
};

#ifdef CONFIG_NEEDS_MANUAL_RELOC
void bmp_reloc(void) {
	fixup_cmdtable(cmd_bmp_sub, ARRAY_SIZE(cmd_bmp_sub));
}
#endif

/*
 * Subroutine:  do_bmp
 *
 * Description: Handler for 'bmp' command..
 *
 * Inputs:	argv[1] contains the subcommand
 *
 * Return:      None
 *
 */
static int do_bmp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *c;

	/* Strip off leading 'bmp' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_bmp_sub[0], ARRAY_SIZE(cmd_bmp_sub));

	if (c)
		return  c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

U_BOOT_CMD(
	bmp,	5,	1,	do_bmp,
	"manipulate BMP image data",
	"info <imageAddr>          - display image info\n"
	"bmp display <imageAddr> [x y] - display image at x,y"
);

/*
 * Subroutine:  bmp_info
 *
 * Description: Show information about bmp file in memory
 *
 * Inputs:	addr		address of the bmp file
 *
 * Return:      None
 *
 */
static int bmp_info(ulong addr)
{
	struct bmp_image *bmp = (struct bmp_image *)map_sysmem(addr, 0);
	void *bmp_alloc_addr = NULL;
	unsigned long len;

	if (!((bmp->header.signature[0]=='B') &&
	      (bmp->header.signature[1]=='M')))
		bmp = gunzip_bmp(addr, &len, &bmp_alloc_addr);

	if (bmp == NULL) {
		printf("There is no valid bmp file at the given address\n");
		return 1;
	}

	printf("Image size    : %d x %d\n", le32_to_cpu(bmp->header.width),
	       le32_to_cpu(bmp->header.height));
	printf("Bits per pixel: %d\n", le16_to_cpu(bmp->header.bit_count));
	printf("Compression   : %d\n", le32_to_cpu(bmp->header.compression));

	if (bmp_alloc_addr)
		free(bmp_alloc_addr);

	return(0);
}

/*
 * Subroutine:  bmp_display
 *
 * Description: Display bmp file located in memory
 *
 * Inputs:	addr		address of the bmp file
 *
 * Return:      None
 *
 */
int bmp_display(ulong addr, int x, int y)
{
#ifdef CONFIG_DM_VIDEO
	struct udevice *dev;
#endif
	int ret;
	struct bmp_image *bmp = map_sysmem(addr, 0);
	void *bmp_alloc_addr = NULL;
	unsigned long len;

	if (!((bmp->header.signature[0]=='B') &&
	      (bmp->header.signature[1]=='M')))
		bmp = gunzip_bmp(addr, &len, &bmp_alloc_addr);

	if (!bmp) {
		printf("There is no valid bmp file at the given address\n");
		return 1;
	}
	addr = map_to_sysmem(bmp);

#ifdef CONFIG_DM_VIDEO
	ret = uclass_first_device_err(UCLASS_VIDEO, &dev);
	if (!ret) {
		bool align = false;

		if (CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) ||
		    x == BMP_ALIGN_CENTER ||
		    y == BMP_ALIGN_CENTER)
			align = true;

		ret = video_bmp_display(dev, addr, x, y, align);
	}
#elif defined(CONFIG_LCD)
	ret = lcd_display_bitmap(addr, x, y);
#elif defined(CONFIG_VIDEO)
	ret = video_display_bitmap(addr, x, y);
#else
# error bmp_display() requires CONFIG_LCD or CONFIG_VIDEO
#endif

	if (bmp_alloc_addr)
		free(bmp_alloc_addr);

	return ret ? CMD_RET_FAILURE : 0;
}
