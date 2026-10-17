// SPDX-License-Identifier: GPL-2.0
/*
 * ifdtool - Manage Intel Firmware Descriptor information
 *
 * Copyright 2014 Google, Inc
 *
 * From Coreboot project, but it got a serious code clean-up
 * and a few new features
 */

#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/libfdt.h>
#include "ifdtool.h"

#undef DEBUG

#ifdef DEBUG
#define debug(fmt, args...)	printf(fmt, ##args)
#else
#define debug(fmt, args...)
#endif

#define FD_SIGNATURE		0x0FF0A55A
#define FLREG_BASE(reg)		((reg & 0x00000fff) << 12);
#define FLREG_LIMIT(reg)	(((reg & 0x0fff0000) >> 4) | 0xfff);

struct input_file {
	char *fname;
	unsigned int addr;
};

/**
 * find_fd() - Find the flash description in the ROM image
 *
 * @image:	Pointer to image
 * @size:	Size of image in bytes
 * Return: pointer to structure, or NULL if not found
 */
static struct fdbar_t *find_fd(char *image, int size)
{
	uint32_t *ptr, *end;

	/* Scan for FD signature */
	for (ptr = (uint32_t *)image, end = ptr + size / 4; ptr < end; ptr++) {
		if (*ptr == FD_SIGNATURE)
			break;
	}

	if (ptr == end) {
		printf("No Flash Descriptor found in this image\n");
		return NULL;
	}

	debug("Found Flash Descriptor signature at 0x%08lx\n",
	      (char *)ptr - image);

	return (struct fdbar_t *)ptr;
}

/**
 * get_region() - Get information about the selected region
 *
 * @frba:		Flash region list
 * @region_type:	Type of region (0..MAX_REGIONS-1)
 * @region:		Region information is written here
 * Return: 0 if OK, else -ve
 */
static int get_region(struct frba_t *frba, int region_type,
		      struct region_t *region)
{
	if (region_type >= MAX_REGIONS) {
		fprintf(stderr, "Invalid region type.\n");
		return -1;
	}

	region->base = FLREG_BASE(frba->flreg[region_type]);
	region->limit = FLREG_LIMIT(frba->flreg[region_type]);
	region->size = region->limit - region->base + 1;

	return 0;
}

static const char *region_name(int region_type)
{
	static const char *const regions[] = {
		"Flash Descriptor",
		"BIOS",
		"Intel ME",
		"GbE",
		"Platform Data"
	};

	assert(region_type < MAX_REGIONS);

	return regions[region_type];
}

static const char *region_filename(int region_type)
{
	static const char *const region_filenames[] = {
		"flashregion_0_flashdescriptor.bin",
		"flashregion_1_bios.bin",
		"flashregion_2_intel_me.bin",
		"flashregion_3_gbe.bin",
		"flashregion_4_platform_data.bin"
	};

	assert(region_type < MAX_REGIONS);

	return region_filenames[region_type];
}

static int dump_region(int num, struct frba_t *frba)
{
	struct region_t region;
	int ret;

	ret = get_region(frba, num, &region);
	if (ret)
		return ret;

	printf("  Flash Region %d (%s): %08x - %08x %s\n",
	       num, region_name(num), region.base, region.limit,
	       region.size < 1 ? "(unused)" : "");

	return ret;
}

static void dump_frba(struct frba_t *frba)
{
	int i;

	printf("Found Region Section\n");
	for (i = 0; i < MAX_REGIONS; i++) {
		printf("FLREG%d:    0x%08x\n", i, frba->flreg[i]);
		dump_region(i, frba);
	}
}

static void decode_spi_frequency(unsigned int freq)
{
	switch (freq) {
	case SPI_FREQUENCY_20MHZ:
		printf("20MHz");
		break;
	case SPI_FREQUENCY_33MHZ:
		printf("33MHz");
		break;
	case SPI_FREQUENCY_50MHZ:
		printf("50MHz");
		break;
	default:
		printf("unknown<%x>MHz", freq);
	}
}

static void decode_component_density(unsigned int density)
{
	switch (density) {
	case COMPONENT_DENSITY_512KB:
		printf("512KiB");
		break;
	case COMPONENT_DENSITY_1MB:
		printf("1MiB");
		break;
	case COMPONENT_DENSITY_2MB:
		printf("2MiB");
		break;
	case COMPONENT_DENSITY_4MB:
		printf("4MiB");
		break;
	case COMPONENT_DENSITY_8MB:
		printf("8MiB");
		break;
	case COMPONENT_DENSITY_16MB:
		printf("16MiB");
		break;
	default:
		printf("unknown<%x>MiB", density);
	}
}

static void dump_fcba(struct fcba_t *fcba)
{
	printf("\nFound Component Section\n");
	printf("FLCOMP     0x%08x\n", fcba->flcomp);
	printf("  Dual Output Fast Read Support:       %ssupported\n",
	       (fcba->flcomp & (1 << 30)) ? "" : "not ");
	printf("  Read ID/Read Status Clock Frequency: ");
	decode_spi_frequency((fcba->flcomp >> 27) & 7);
	printf("\n  Write/Erase Clock Frequency:         ");
	decode_spi_frequency((fcba->flcomp >> 24) & 7);
	printf("\n  Fast Read Clock Frequency:           ");
	decode_spi_frequency((fcba->flcomp >> 21) & 7);
	printf("\n  Fast Read Support:                   %ssupported",
	       (fcba->flcomp & (1 << 20)) ? "" : "not ");
	printf("\n  Read Clock Frequency:                ");
	decode_spi_frequency((fcba->flcomp >> 17) & 7);
	printf("\n  Component 2 Density:                 ");
	decode_component_density((fcba->flcomp >> 3) & 7);
	printf("\n  Component 1 Density:                 ");
	decode_component_density(fcba->flcomp & 7);
	printf("\n");
	printf("FLILL      0x%08x\n", fcba->flill);
	printf("  Invalid Instruction 3: 0x%02x\n",
	       (fcba->flill >> 24) & 0xff);
	printf("  Invalid Instruction 2: 0x%02x\n",
	       (fcba->flill >> 16) & 0xff);
	printf("  Invalid Instruction 1: 0x%02x\n",
	       (fcba->flill >> 8) & 0xff);
	printf("  Invalid Instruction 0: 0x%02x\n",
	       fcba->flill & 0xff);
	printf("FLPB       0x%08x\n", fcba->flpb);
	printf("  Flash Partition Boundary Address: 0x%06x\n\n",
	       (fcba->flpb & 0xfff) << 12);
}

static void dump_fpsba(struct fpsba_t *fpsba)
{
	int i;

	printf("Found PCH Strap Section\n");
	for (i = 0; i < MAX_STRAPS; i++)
		printf("PCHSTRP%-2d:  0x%08x\n", i, fpsba->pchstrp[i]);
}

static const char *get_enabled(int flag)
{
	return flag ? "enabled" : "disabled";
}

static void decode_flmstr(uint32_t flmstr)
{
	printf("  Platform Data Region Write Access: %s\n",
	       get_enabled(flmstr & (1 << 28)));
	printf("  GbE Region Write Access:           %s\n",
	       get_enabled(flmstr & (1 << 27)));
	printf("  Intel ME Region Write Access:      %s\n",
	       get_enabled(flmstr & (1 << 26)));
	printf("  Host CPU/BIOS Region Write Access: %s\n",
	       get_enabled(flmstr & (1 << 25)));
	printf("  Flash Descriptor Write Access:     %s\n",
	       get_enabled(flmstr & (1 << 24)));

	printf("  Platform Data Region Read Access:  %s\n",
	       get_enabled(flmstr & (1 << 20)));
	printf("  GbE Region Read Access:            %s\n",
	       get_enabled(flmstr & (1 << 19)));
	printf("  Intel ME Region Read Access:       %s\n",
	       get_enabled(flmstr & (1 << 18)));
	printf("  Host CPU/BIOS Region Read Access:  %s\n",
	       get_enabled(flmstr & (1 << 17)));
	printf("  Flash Descriptor Read Access:      %s\n",
	       get_enabled(flmstr & (1 << 16)));

	printf("  Requester ID:                      0x%04x\n\n",
	       flmstr & 0xffff);
}

static void dump_fmba(struct fmba_t *fmba)
{
	printf("Found Master Section\n");
	printf("FLMSTR1:   0x%08x (Host CPU/BIOS)\n", fmba->flmstr1);
	decode_flmstr(fmba->flmstr1);
	printf("FLMSTR2:   0x%08x (Intel ME)\n", fmba->flmstr2);
	decode_flmstr(fmba->flmstr2);
	printf("FLMSTR3:   0x%08x (GbE)\n", fmba->flmstr3);
	decode_flmstr(fmba->flmstr3);
}

static void dump_fmsba(struct fmsba_t *fmsba)
{
	int i;

	printf("Found Processor Strap Section\n");
	for (i = 0; i < 4; i++)
		printf("????:      0x%08x\n", fmsba->data[0]);
}

static void dump_jid(uint32_t jid)
{
	printf("    SPI Component Device ID 1:          0x%02x\n",
	       (jid >> 16) & 0xff);
	printf("    SPI Component Device ID 0:          0x%02x\n",
	       (jid >> 8) & 0xff);
	printf("    SPI Component Vendor ID:            0x%02x\n",
	       jid & 0xff);
}

static void dump_vscc(uint32_t vscc)
{
	printf("    Lower Erase Opcode:                 0x%02x\n",
	       vscc >> 24);
	printf("    Lower Write Enable on Write Status: 0x%02x\n",
	       vscc & (1 << 20) ? 0x06 : 0x50);
	printf("    Lower Write Status Required:        %s\n",
	       vscc & (1 << 19) ? "Yes" : "No");
	printf("    Lower Write Granularity:            %d bytes\n",
	       vscc & (1 << 18) ? 64 : 1);
	printf("    Lower Block / Sector Erase Size:    ");
	switch ((vscc >> 16) & 0x3) {
	case 0:
		printf("256 Byte\n");
		break;
	case 1:
		printf("4KB\n");
		break;
	case 2:
		printf("8KB\n");
		break;
	case 3:
		printf("64KB\n");
		break;
	}

	printf("    Upper Erase Opcode:                 0x%02x\n",
	       (vscc >> 8) & 0xff);
	printf("    Upper Write Enable on Write Status: 0x%02x\n",
	       vscc & (1 << 4) ? 0x06 : 0x50);
	printf("    Upper Write Status Required:        %s\n",
	       vscc & (1 << 3) ? "Yes" : "No");
	printf("    Upper Write Granularity:            %d bytes\n",
	       vscc & (1 << 2) ? 64 : 1);
	printf("    Upper Block / Sector Erase Size:    ");
	switch (vscc & 0x3) {
	case 0:
		printf("256 Byte\n");
		break;
	case 1:
		printf("4KB\n");
		break;
	case 2:
		printf("8KB\n");
		break;
	case 3:
		printf("64KB\n");
		break;
	}
}

static void dump_vtba(struct vtba_t *vtba, int vtl)
{
	int i;
	int num = (vtl >> 1) < 8 ? (vtl >> 1) : 8;

	printf("ME VSCC table:\n");
	for (i = 0; i < num; i++) {
		printf("  JID%d:  0x%08x\n", i, vtba->entry[i].jid);
		dump_jid(vtba->entry[i].jid);
		printf("  VSCC%d: 0x%08x\n", i, vtba->entry[i].vscc);
		dump_vscc(vtba->entry[i].vscc);
	}
	printf("\n");
}

static void dump_oem(uint8_t *oem)
{
	int i, j;
	printf("OEM Section:\n");
	for (i = 0; i < 4; i++) {
		printf("%02x:", i << 4);
		for (j = 0; j < 16; j++)
			printf(" %02x", oem[(i<<4)+j]);
		printf("\n");
	}
	printf("\n");
}

/**
 * dump_fd() - Display a dump of the full flash description
 *
 * @image:	Pointer to image
 * @size:	Size of image in bytes
 * Return: 0 if OK, -1 on error
 */
static int dump_fd(char *image, int size)
{
	struct fdbar_t *fdb = find_fd(image, size);

	if (!fdb)
		return -1;

	printf("FLMAP0:    0x%08x\n", fdb->flmap0);
	printf("  NR:      %d\n", (fdb->flmap0 >> 24) & 7);
	printf("  FRBA:    0x%x\n", ((fdb->flmap0 >> 16) & 0xff) << 4);
	printf("  NC:      %d\n", ((fdb->flmap0 >> 8) & 3) + 1);
	printf("  FCBA:    0x%x\n", ((fdb->flmap0) & 0xff) << 4);

	printf("FLMAP1:    0x%08x\n", fdb->flmap1);
	printf("  ISL:     0x%02x\n", (fdb->flmap1 >> 24) & 0xff);
	printf("  FPSBA:   0x%x\n", ((fdb->flmap1 >> 16) & 0xff) << 4);
	printf("  NM:      %d\n", (fdb->flmap1 >> 8) & 3);
	printf("  FMBA:    0x%x\n", ((fdb->flmap1) & 0xff) << 4);

	printf("FLMAP2:    0x%08x\n", fdb->flmap2);
	printf("  PSL:     0x%04x\n", (fdb->flmap2 >> 8) & 0xffff);
	printf("  FMSBA:   0x%x\n", ((fdb->flmap2) & 0xff) << 4);

	printf("FLUMAP1:   0x%08x\n", fdb->flumap1);
	printf("  Intel ME VSCC Table Length (VTL):        %d\n",
	       (fdb->flumap1 >> 8) & 0xff);
	printf("  Intel ME VSCC Table Base Address (VTBA): 0x%06x\n\n",
	       (fdb->flumap1 & 0xff) << 4);
	dump_vtba((struct vtba_t *)
			(image + ((fdb->flumap1 & 0xff) << 4)),
			(fdb->flumap1 >> 8) & 0xff);
	dump_oem((uint8_t *)image + 0xf00);
	dump_frba((struct frba_t *)(image + (((fdb->flmap0 >> 16) & 0xff)
			<< 4)));
	dump_fcba((struct fcba_t *)(image + (((fdb->flmap0) & 0xff) << 4)));
	dump_fpsba((struct fpsba_t *)
			(image + (((fdb->flmap1 >> 16) & 0xff) << 4)));
	dump_fmba((struct fmba_t *)(image + (((fdb->flmap1) & 0xff) << 4)));
	dump_fmsba((struct fmsba_t *)(image + (((fdb->flmap2) & 0xff) << 4)));

	return 0;
}

/**
 * write_regions() - Write each region from an image to its own file
 *
 * The filename to use in each case is fixed - see region_filename()
 *
 * @image:	Pointer to image
 * @size:	Size of image in bytes
 * Return: 0 if OK, -ve on error
 */
static int write_regions(char *image, int size)
{
	struct fdbar_t *fdb;
	struct frba_t *frba;
	int ret = 0;
	int i;

	fdb =  find_fd(image, size);
	if (!fdb)
		return -1;

	frba = (struct frba_t *)(image + (((fdb->flmap0 >> 16) & 0xff) << 4));

	for (i = 0; i < MAX_REGIONS; i++) {
		struct region_t region;
		int region_fd;

		ret = get_region(frba, i, &region);
		if (ret)
			return ret;
		dump_region(i, frba);
		if (region.size <= 0)
			continue;
		region_fd = open(region_filename(i),
				 O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR |
				 S_IWUSR | S_IRGRP | S_IROTH);
		if (write(region_fd, image + region.base, region.size) !=
				region.size) {
			perror("Error while writing");
			ret = -1;
		}
		close(region_fd);
	}

	return ret;
}

static int perror_fname(const char *fmt, const char *fname)
{
	char msg[strlen(fmt) + strlen(fname) + 1];

	sprintf(msg, fmt, fname);
	perror(msg);

	return -1;
}

/**
 * write_image() - Write the image to a file
 *
 * @filename:	Filename to use for the image
 * @image:	Pointer to image
 * @size:	Size of image in bytes
 * Return: 0 if OK, -ve on error
 */
static int write_image(char *filename, char *image, int size)
{
	int new_fd;

	debug("Writing new image to %s\n", filename);

	new_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR |
		      S_IWUSR | S_IRGRP | S_IROTH);
	if (new_fd < 0)
		return perror_fname("Could not open file '%s'", filename);
	if (write(new_fd, image, size) != size)
		return perror_fname("Could not write file '%s'", filename);
	close(new_fd);

	return 0;
}

/**
 * set_spi_frequency() - Set the SPI frequency to use when booting
 *
 * Several frequencies are supported, some of which work with fast devices.
 * For SPI emulators, the slowest (SPI_FREQUENCY_20MHZ) is often used. The
 * Intel boot system uses this information somehow on boot.
 *
 * The image is updated with the supplied value
 *
 * @image:	Pointer to image
 * @size:	Size of image in bytes
 * @freq:	SPI frequency to use
 */
static void set_spi_frequency(char *image, int size, enum spi_frequency freq)
{
	struct fdbar_t *fdb = find_fd(image, size);
	struct fcba_t *fcba;

	fcba = (struct fcba_t *)(image + (((fdb->flmap0) & 0xff) << 4));

	/* clear bits 21-29 */
	fcba->flcomp &= ~0x3fe00000;
	/* Read ID and Read Status Clock Frequency */
	fcba->flcomp |= freq << 27;
	/* Write and Erase Clock Frequency */
	fcba->flcomp |= freq << 24;
	/* Fast Read Clock Frequency */
	fcba->flcomp |= freq << 21;
}

/**
 * set_em100_mode() - Set a SPI frequency that will work with Dediprog EM100
 *
 * @image:	Pointer to image
 * @size:	Size of image in bytes
 */
static void set_em100_mode(char *image, int size)
{
	struct fdbar_t *fdb = find_fd(image, size);
	struct fcba_t *fcba;

	fcba = (struct fcba_t *)(image + (((fdb->flmap0) & 0xff) << 4));
	fcba->flcomp &= ~(1 << 30);
	set_spi_frequency(image, size, SPI_FREQUENCY_20MHZ);
}

/**
 * lock_descriptor() - Lock the NE descriptor so it cannot be updated
 *
 * @image:	Pointer to image
 * @size:	Size of image in bytes
 */
static void lock_descriptor(char *image, int size)
{
	struct fdbar_t *fdb = find_fd(image, size);
	struct fmba_t *fmba;

	/*
	 * TODO: Dynamically take Platform Data Region and GbE Region into
	 * account.
	 */
	fmba = (struct fmba_t *)(image + (((fdb->flmap1) & 0xff) << 4));
	fmba->flmstr1 = 0x0a0b0000;
	fmba->flmstr2 = 0x0c0d0000;
	fmba->flmstr3 = 0x08080118;
}

/**
 * unlock_descriptor() - Lock the NE descriptor so it can be updated
 *
 * @image:	Pointer to image
 * @size:	Size of image in bytes
 */
static void unlock_descriptor(char *image, int size)
{
	struct fdbar_t *fdb = find_fd(image, size);
	struct fmba_t *fmba;

	fmba = (struct fmba_t *)(image + (((fdb->flmap1) & 0xff) << 4));
	fmba->flmstr1 = 0xffff0000;
	fmba->flmstr2 = 0xffff0000;
	fmba->flmstr3 = 0x08080118;
}

/**
 * open_for_read() - Open a file for reading
 *
 * @fname:	Filename to open
 * @sizep:	Returns file size in bytes
 * Return: 0 if OK, -1 on error
 */
int open_for_read(const char *fname, int *sizep)
{
	int fd = open(fname, O_RDONLY);
	struct stat buf;

	if (fd == -1)
		return perror_fname("Could not open file '%s'", fname);
	if (fstat(fd, &buf) == -1)
		return perror_fname("Could not stat file '%s'", fname);
	*sizep = buf.st_size;
	debug("File %s is %d bytes\n", fname, *sizep);

	return fd;
}

/**
 * inject_region() - Add a file to an image region
 *
 * This puts a file into a particular region of the flash. Several pre-defined
 * regions are used.
 *
 * @image:		Pointer to image
 * @size:		Size of image in bytes
 * @region_type:	Region where the file should be added
 * @region_fname:	Filename to add to the image
 * Return: 0 if OK, -ve on error
 */
int inject_region(char *image, int size, int region_type, char *region_fname)
{
	struct fdbar_t *fdb = find_fd(image, size);
	struct region_t region;
	struct frba_t *frba;
	int region_size;
	int offset = 0;
	int region_fd;
	int ret;

	if (!fdb)
		exit(EXIT_FAILURE);
	frba = (struct frba_t *)(image + (((fdb->flmap0 >> 16) & 0xff) << 4));

	ret = get_region(frba, region_type, &region);
	if (ret)
		return -1;
	if (region.size <= 0xfff) {
		fprintf(stderr, "Region %s is disabled in target. Not injecting.\n",
			region_name(region_type));
		return -1;
	}

	region_fd = open_for_read(region_fname, &region_size);
	if (region_fd < 0)
		return region_fd;

	if ((region_size > region.size) ||
	    ((region_type != 1) && (region_size > region.size))) {
		fprintf(stderr, "Region %s is %d(0x%x) bytes. File is %d(0x%x)  bytes. Not injecting.\n",
			region_name(region_type), region.size,
			region.size, region_size, region_size);
		return -1;
	}

	if ((region_type == 1) && (region_size < region.size)) {
		fprintf(stderr, "Region %s is %d(0x%x) bytes. File is %d(0x%x) bytes. Padding before injecting.\n",
			region_name(region_type), region.size,
			region.size, region_size, region_size);
		offset = region.size - region_size;
		memset(image + region.base, 0xff, offset);
	}

	if (size < region.base + offset + region_size) {
		fprintf(stderr, "Output file is too small. (%d < %d)\n",
			size, region.base + offset + region_size);
		return -1;
	}

	if (read(region_fd, image + region.base + offset, region_size)
							!= region_size) {
		perror("Could not read file");
		return -1;
	}

	close(region_fd);

	debug("Adding %s as the %s section\n", region_fname,
	      region_name(region_type));

	return 0;
}

/**
 * write_data() - Write some raw data into a region
 *
 * This puts a file into a particular place in the flash, ignoring the
 * regions. Be careful not to overwrite something important.
 *
 * @image:		Pointer to image
 * @size:		Size of image in bytes
 * @addr:		x86 ROM address to put file. The ROM ends at
 *			0xffffffff so use an address relative to that. For an
 *			8MB ROM the start address is 0xfff80000.
 * @write_fname:	Filename to add to the image
 * @offset_uboot_top:	Offset of the top of U-Boot
 * @offset_uboot_start:	Offset of the start of U-Boot
 * Return: number of bytes written if OK, -ve on error
 */
static int write_data(char *image, int size, unsigned int addr,
		      const char *write_fname, int offset_uboot_top,
		      int offset_uboot_start)
{
	int write_fd, write_size;
	int offset;

	write_fd = open_for_read(write_fname, &write_size);
	if (write_fd < 0)
		return write_fd;

	offset = (uint32_t)(addr + size);
	if (offset_uboot_top) {
		if (offset_uboot_start < offset &&
		    offset_uboot_top >= offset) {
			fprintf(stderr, "U-Boot image overlaps with region '%s'\n",
				write_fname);
			fprintf(stderr,
				"U-Boot finishes at offset %x, file starts at %x\n",
				offset_uboot_top, offset);
			return -EXDEV;
		}
		if (offset_uboot_start > offset &&
		    offset_uboot_start <= offset + write_size) {
			fprintf(stderr, "U-Boot image overlaps with region '%s'\n",
				write_fname);
			fprintf(stderr,
				"U-Boot starts at offset %x, file finishes at %x\n",
				offset_uboot_start, offset + write_size);
			return -EXDEV;
		}
	}
	debug("Writing %s to offset %#x\n", write_fname, offset);

	if (offset < 0 || offset + write_size > size) {
		fprintf(stderr, "Output file is too small. (%d < %d)\n",
			size, offset + write_size);
		return -1;
	}

	if (read(write_fd, image + offset, write_size) != write_size) {
		perror("Could not read file");
		return -1;
	}

	close(write_fd);

	return write_size;
}

static void print_version(void)
{
	printf("ifdtool v%s -- ", IFDTOOL_VERSION);
	printf("Copyright (C) 2014 Google Inc.\n\n");
	printf("SPDX-License-Identifier: GPL-2.0+\n");
}

static void print_usage(const char *name)
{
	printf("usage: %s [-vhdix?] <filename> [<outfile>]\n", name);
	printf("\n"
	       "   -d | --dump:                      dump intel firmware descriptor\n"
	       "   -x | --extract:                   extract intel fd modules\n"
	       "   -i | --inject <region>:<module>   inject file <module> into region <region>\n"
	       "   -w | --write <addr>:<file>        write file to appear at memory address <addr>\n"
	       "                                     multiple files can be written simultaneously\n"
	       "   -s | --spifreq <20|33|50>         set the SPI frequency\n"
	       "   -e | --em100                      set SPI frequency to 20MHz and disable\n"
	       "                                     Dual Output Fast Read Support\n"
	       "   -l | --lock                       Lock firmware descriptor and ME region\n"
	       "   -u | --unlock                     Unlock firmware descriptor and ME region\n"
	       "   -r | --romsize                    Specify ROM size\n"
	       "   -D | --write-descriptor <file>    Write descriptor at base\n"
	       "   -c | --create                     Create a new empty image\n"
	       "   -v | --version:                   print the version\n"
	       "   -h | --help:                      print this help\n\n"
	       "<region> is one of Descriptor, BIOS, ME, GbE, Platform\n"
	       "\n");
}

/**
 * get_two_words() - Convert a string into two words separated by :
 *
 * The supplied string is split at ':', two substrings are allocated and
 * returned.
 *
 * @str:	String to split
 * @firstp:	Returns first string
 * @secondp:	Returns second string
 * Return: 0 if OK, -ve if @str does not have a :
 */
static int get_two_words(const char *str, char **firstp, char **secondp)
{
	const char *p;

	p = strchr(str, ':');
	if (!p)
		return -1;
	*firstp = strdup(str);
	(*firstp)[p - str] = '\0';
	*secondp = strdup(p + 1);

	return 0;
}

int main(int argc, char *argv[])
{
	int opt, option_index = 0;
	int mode_dump = 0, mode_extract = 0, mode_inject = 0;
	int mode_spifreq = 0, mode_em100 = 0, mode_locked = 0;
	int mode_unlocked = 0, mode_write = 0, mode_write_descriptor = 0;
	int create = 0;
	char *region_type_string = NULL, *inject_fname = NULL;
	char *desc_fname = NULL, *addr_str = NULL;
	int region_type = -1, inputfreq = 0;
	enum spi_frequency spifreq = SPI_FREQUENCY_20MHZ;
	struct input_file input_file[WRITE_MAX], *ifile, *fdt = NULL;
	unsigned char wr_idx, wr_num = 0;
	int rom_size = -1;
	bool write_it;
	char *filename;
	char *outfile = NULL;
	struct stat buf;
	int size = 0;
	bool have_uboot = false;
	int bios_fd;
	char *image;
	int ret;
	static struct option long_options[] = {
		{"create", 0, NULL, 'c'},
		{"dump", 0, NULL, 'd'},
		{"descriptor", 1, NULL, 'D'},
		{"em100", 0, NULL, 'e'},
		{"extract", 0, NULL, 'x'},
		{"fdt", 1, NULL, 'f'},
		{"inject", 1, NULL, 'i'},
		{"lock", 0, NULL, 'l'},
		{"romsize", 1, NULL, 'r'},
		{"spifreq", 1, NULL, 's'},
		{"unlock", 0, NULL, 'u'},
		{"uboot", 1, NULL, 'U'},
		{"write", 1, NULL, 'w'},
		{"version", 0, NULL, 'v'},
		{"help", 0, NULL, 'h'},
		{0, 0, 0, 0}
	};

	while ((opt = getopt_long(argc, argv, "cdD:ef:hi:lr:s:uU:vw:x?",
				  long_options, &option_index)) != EOF) {
		switch (opt) {
		case 'c':
			create = 1;
			break;
		case 'd':
			mode_dump = 1;
			break;
		case 'D':
			mode_write_descriptor = 1;
			desc_fname = optarg;
			break;
		case 'e':
			mode_em100 = 1;
			break;
		case 'i':
			if (get_two_words(optarg, &region_type_string,
					  &inject_fname)) {
				print_usage(argv[0]);
				exit(EXIT_FAILURE);
			}
			if (!strcasecmp("Descriptor", region_type_string))
				region_type = 0;
			else if (!strcasecmp("BIOS", region_type_string))
				region_type = 1;
			else if (!strcasecmp("ME", region_type_string))
				region_type = 2;
			else if (!strcasecmp("GbE", region_type_string))
				region_type = 3;
			else if (!strcasecmp("Platform", region_type_string))
				region_type = 4;
			if (region_type == -1) {
				fprintf(stderr, "No such region type: '%s'\n\n",
					region_type_string);
				print_usage(argv[0]);
				exit(EXIT_FAILURE);
			}
			mode_inject = 1;
			break;
		case 'l':
			mode_locked = 1;
			break;
		case 'r':
			rom_size = strtol(optarg, NULL, 0);
			debug("ROM size %d\n", rom_size);
			break;
		case 's':
			/* Parse the requested SPI frequency */
			inputfreq = strtol(optarg, NULL, 0);
			switch (inputfreq) {
			case 20:
				spifreq = SPI_FREQUENCY_20MHZ;
				break;
			case 33:
				spifreq = SPI_FREQUENCY_33MHZ;
				break;
			case 50:
				spifreq = SPI_FREQUENCY_50MHZ;
				break;
			default:
				fprintf(stderr, "Invalid SPI Frequency: %d\n",
					inputfreq);
				print_usage(argv[0]);
				exit(EXIT_FAILURE);
			}
			mode_spifreq = 1;
			break;
		case 'u':
			mode_unlocked = 1;
			break;
		case 'v':
			print_version();
			exit(EXIT_SUCCESS);
			break;
		case 'w':
		case 'U':
		case 'f':
			ifile = &input_file[wr_num];
			mode_write = 1;
			if (wr_num < WRITE_MAX) {
				if (get_two_words(optarg, &addr_str,
						  &ifile->fname)) {
					print_usage(argv[0]);
					exit(EXIT_FAILURE);
				}
				ifile->addr = strtoll(optarg, NULL, 0);
				wr_num++;
			} else {
				fprintf(stderr,
					"The number of files to write simultaneously exceeds the limitation (%d)\n",
					WRITE_MAX);
			}
			break;
		case 'x':
			mode_extract = 1;
			break;
		case 'h':
		case '?':
		default:
			print_usage(argv[0]);
			exit(EXIT_SUCCESS);
			break;
		}
	}

	if (mode_locked == 1 && mode_unlocked == 1) {
		fprintf(stderr, "Locking/Unlocking FD and ME are mutually exclusive\n");
		exit(EXIT_FAILURE);
	}

	if (mode_inject == 1 && mode_write == 1) {
		fprintf(stderr, "Inject/Write are mutually exclusive\n");
		exit(EXIT_FAILURE);
	}

	if ((mode_dump + mode_extract + mode_inject +
		(mode_spifreq | mode_em100 | mode_unlocked |
		 mode_locked)) > 1) {
		fprintf(stderr, "You may not specify more than one mode.\n\n");
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	if ((mode_dump + mode_extract + mode_inject + mode_spifreq +
	     mode_em100 + mode_locked + mode_unlocked + mode_write +
	     mode_write_descriptor) == 0 && !create) {
		fprintf(stderr, "You need to specify a mode.\n\n");
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	if (create && rom_size == -1) {
		fprintf(stderr, "You need to specify a rom size when creating.\n\n");
		exit(EXIT_FAILURE);
	}

	if (optind + 1 != argc) {
		fprintf(stderr, "You need to specify a file.\n\n");
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	if (have_uboot && !fdt) {
		fprintf(stderr,
			"You must supply a device tree file for U-Boot\n\n");
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	filename = argv[optind];
	if (optind + 2 != argc)
		outfile = argv[optind + 1];

	if (create)
		bios_fd = open(filename, O_WRONLY | O_CREAT, 0666);
	else
		bios_fd = open(filename, outfile ? O_RDONLY : O_RDWR);

	if (bios_fd == -1) {
		perror("Could not open file");
		exit(EXIT_FAILURE);
	}

	if (!create) {
		if (fstat(bios_fd, &buf) == -1) {
			perror("Could not stat file");
			exit(EXIT_FAILURE);
		}
		size = buf.st_size;
	}

	debug("File %s is %d bytes\n", filename, size);

	if (rom_size == -1)
		rom_size = size;

	image = malloc(rom_size);
	if (!image) {
		printf("Out of memory.\n");
		exit(EXIT_FAILURE);
	}

	memset(image, '\xff', rom_size);
	if (!create && read(bios_fd, image, size) != size) {
		perror("Could not read file");
		exit(EXIT_FAILURE);
	}
	if (size != rom_size) {
		debug("ROM size changed to %d bytes\n", rom_size);
		size = rom_size;
	}

	write_it = true;
	ret = 0;
	if (mode_dump) {
		ret = dump_fd(image, size);
		write_it = false;
	}

	if (mode_extract) {
		ret = write_regions(image, size);
		write_it = false;
	}

	if (mode_write_descriptor)
		ret = write_data(image, size, -size, desc_fname, 0, 0);

	if (mode_inject)
		ret = inject_region(image, size, region_type, inject_fname);

	if (mode_write) {
		int offset_uboot_top = 0;
		int offset_uboot_start = 0;

		for (wr_idx = 0; wr_idx < wr_num; wr_idx++) {
			ifile = &input_file[wr_idx];
			ret = write_data(image, size, ifile->addr,
					 ifile->fname, offset_uboot_top,
					 offset_uboot_start);
			if (ret < 0)
				break;
		}
	}

	if (mode_spifreq)
		set_spi_frequency(image, size, spifreq);

	if (mode_em100)
		set_em100_mode(image, size);

	if (mode_locked)
		lock_descriptor(image, size);

	if (mode_unlocked)
		unlock_descriptor(image, size);

	if (write_it) {
		if (outfile) {
			ret = write_image(outfile, image, size);
		} else {
			if (lseek(bios_fd, 0, SEEK_SET)) {
				perror("Error while seeking");
				ret = -1;
			}
			if (write(bios_fd, image, size) != size) {
				perror("Error while writing");
				ret = -1;
			}
		}
	}

	free(image);
	close(bios_fd);

	return ret < 0 ? 1 : 0;
}
