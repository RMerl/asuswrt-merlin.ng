// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Charles Manning <cdhmanning@gmail.com>
 *
 * Reference documents:
 *   Cyclone V SoC: https://www.altera.com/content/dam/altera-www/global/en_US/pdfs/literature/hb/cyclone-v/cv_5400a.pdf
 *   Arria V SoC:   https://www.altera.com/content/dam/altera-www/global/en_US/pdfs/literature/hb/arria-v/av_5400a.pdf
 *   Arria 10 SoC:  https://www.altera.com/content/dam/altera-www/global/en_US/pdfs/literature/hb/arria-10/a10_5400a.pdf
 *
 * Bootable SoCFPGA image requires a structure of the following format
 * positioned at offset 0x40 of the bootable image. Endian is LSB.
 *
 * There are two versions of the SoCFPGA header format, v0 and v1.
 * The version 0 is used by Cyclone V SoC and Arria V SoC, while
 * the version 1 is used by the Arria 10 SoC.
 *
 * Version 0:
 * Offset   Length   Usage
 * -----------------------
 *   0x40        4   Validation word (0x31305341)
 *   0x44        1   Version (0x0)
 *   0x45        1   Flags (unused, zero is fine)
 *   0x46        2   Length (in units of u32, including the end checksum).
 *   0x48        2   Zero (0x0)
 *   0x4A        2   Checksum over the header. NB Not CRC32
 *
 * Version 1:
 * Offset   Length   Usage
 * -----------------------
 *   0x40        4   Validation word (0x31305341)
 *   0x44        1   Version (0x1)
 *   0x45        1   Flags (unused, zero is fine)
 *   0x46        2   Header length (in units of u8).
 *   0x48        4   Length (in units of u8).
 *   0x4C        4   Image entry offset from standard of header
 *   0x50        2   Zero (0x0)
 *   0x52        2   Checksum over the header. NB Not CRC32
 *
 * At the end of the code we have a 32-bit CRC checksum over whole binary
 * excluding the CRC.
 *
 * Note that the CRC used here is **not** the zlib/Adler crc32. It is the
 * CRC-32 used in bzip2, ethernet and elsewhere.
 *
 * The Image entry offset in version 1 image is relative the the start of
 * the header, 0x40, and must not be a negative number. Therefore, it is
 * only possible to make the SoCFPGA jump forward. The U-Boot bootloader
 * places a trampoline instruction at offset 0x5c, 0x14 bytes from the
 * start of the SoCFPGA header, which jumps to the reset vector.
 *
 * The image is padded out to 64k, because that is what is
 * typically used to write the image to the boot medium.
 */

#include "pbl_crc32.h"
#include "imagetool.h"
#include "mkimage.h"

#include <image.h>

#define HEADER_OFFSET	0x40
#define VALIDATION_WORD	0x31305341

static uint8_t buffer_v0[0x10000];
static uint8_t buffer_v1[0x40000];

struct socfpga_header_v0 {
	uint32_t	validation;
	uint8_t		version;
	uint8_t		flags;
	uint16_t	length_u32;
	uint16_t	zero;
	uint16_t	checksum;
};

struct socfpga_header_v1 {
	uint32_t	validation;
	uint8_t		version;
	uint8_t		flags;
	uint16_t	header_u8;
	uint32_t	length_u8;
	uint32_t	entry_offset;
	uint16_t	zero;
	uint16_t	checksum;
};

static unsigned int sfp_hdr_size(uint8_t ver)
{
	if (ver == 0)
		return sizeof(struct socfpga_header_v0);
	if (ver == 1)
		return sizeof(struct socfpga_header_v1);
	return 0;
}

static unsigned int sfp_pad_size(uint8_t ver)
{
	if (ver == 0)
		return sizeof(buffer_v0);
	if (ver == 1)
		return sizeof(buffer_v1);
	return 0;
}

/*
 * The header checksum is just a very simple checksum over
 * the header area.
 * There is still a crc32 over the whole lot.
 */
static uint16_t sfp_hdr_checksum(uint8_t *buf, unsigned char ver)
{
	uint16_t ret = 0;
	int len = sfp_hdr_size(ver) - sizeof(ret);

	while (--len)
		ret += *buf++;

	return ret;
}

static void sfp_build_header(uint8_t *buf, uint8_t ver, uint8_t flags,
			     uint32_t length_bytes)
{
	struct socfpga_header_v0 header_v0 = {
		.validation	= cpu_to_le32(VALIDATION_WORD),
		.version	= 0,
		.flags		= flags,
		.length_u32	= cpu_to_le16(length_bytes / 4),
		.zero		= 0,
	};

	struct socfpga_header_v1 header_v1 = {
		.validation	= cpu_to_le32(VALIDATION_WORD),
		.version	= 1,
		.flags		= flags,
		.header_u8	= cpu_to_le16(sizeof(header_v1)),
		.length_u8	= cpu_to_le32(length_bytes),
		.entry_offset	= cpu_to_le32(0x14),	/* Trampoline offset */
		.zero		= 0,
	};

	uint16_t csum;

	if (ver == 0) {
		csum = sfp_hdr_checksum((uint8_t *)&header_v0, 0);
		header_v0.checksum = cpu_to_le16(csum);
		memcpy(buf, &header_v0, sizeof(header_v0));
	} else {
		csum = sfp_hdr_checksum((uint8_t *)&header_v1, 1);
		header_v1.checksum = cpu_to_le16(csum);
		memcpy(buf, &header_v1, sizeof(header_v1));
	}
}

/*
 * Perform a rudimentary verification of header and return
 * size of image.
 */
static int sfp_verify_header(const uint8_t *buf, uint8_t *ver)
{
	struct socfpga_header_v0 header_v0;
	struct socfpga_header_v1 header_v1;
	uint16_t hdr_csum, sfp_csum;
	uint32_t img_len;

	/*
	 * Header v0 is always smaller than Header v1 and the validation
	 * word and version field is at the same place, so use Header v0
	 * to check for version during verifiction and upgrade to Header
	 * v1 if needed.
	 */
	memcpy(&header_v0, buf, sizeof(header_v0));

	if (le32_to_cpu(header_v0.validation) != VALIDATION_WORD)
		return -1;

	if (header_v0.version == 0) {
		hdr_csum = le16_to_cpu(header_v0.checksum);
		sfp_csum = sfp_hdr_checksum((uint8_t *)&header_v0, 0);
		img_len = le16_to_cpu(header_v0.length_u32) * 4;
	} else if (header_v0.version == 1) {
		memcpy(&header_v1, buf, sizeof(header_v1));
		hdr_csum = le16_to_cpu(header_v1.checksum);
		sfp_csum = sfp_hdr_checksum((uint8_t *)&header_v1, 1);
		img_len = le32_to_cpu(header_v1.length_u8);
	} else {	/* Invalid version */
		return -EINVAL;
	}

	/* Verify checksum */
	if (hdr_csum != sfp_csum)
		return -EINVAL;

	*ver = header_v0.version;
	return img_len;
}

/* Sign the buffer and return the signed buffer size */
static int sfp_sign_buffer(uint8_t *buf, uint8_t ver, uint8_t flags,
			   int len, int pad_64k)
{
	uint32_t calc_crc;

	/* Align the length up */
	len = (len + 3) & ~3;

	/* Build header, adding 4 bytes to length to hold the CRC32. */
	sfp_build_header(buf + HEADER_OFFSET, ver, flags, len + 4);

	/* Calculate and apply the CRC */
	calc_crc = ~pbl_crc32(0, (char *)buf, len);

	*((uint32_t *)(buf + len)) = cpu_to_le32(calc_crc);

	if (!pad_64k)
		return len + 4;

	return sfp_pad_size(ver);
}

/* Verify that the buffer looks sane */
static int sfp_verify_buffer(const uint8_t *buf)
{
	int len; /* Including 32bit CRC */
	uint32_t calc_crc;
	uint32_t buf_crc;
	uint8_t ver = 0;

	len = sfp_verify_header(buf + HEADER_OFFSET, &ver);
	if (len < 0) {
		debug("Invalid header\n");
		return -1;
	}

	if (len < HEADER_OFFSET || len > sfp_pad_size(ver)) {
		debug("Invalid header length (%i)\n", len);
		return -1;
	}

	/*
	 * Adjust length to the base of the CRC.
	 * Check the CRC.
	*/
	len -= 4;

	calc_crc = ~pbl_crc32(0, (const char *)buf, len);

	buf_crc = le32_to_cpu(*((uint32_t *)(buf + len)));

	if (buf_crc != calc_crc) {
		fprintf(stderr, "CRC32 does not match (%08x != %08x)\n",
			buf_crc, calc_crc);
		return -1;
	}

	return 0;
}

/* mkimage glue functions */
static int socfpgaimage_verify_header(unsigned char *ptr, int image_size,
				      struct image_tool_params *params)
{
	if (image_size < 0x80)
		return -1;

	return sfp_verify_buffer(ptr);
}

static void socfpgaimage_print_header(const void *ptr)
{
	if (sfp_verify_buffer(ptr) == 0)
		printf("Looks like a sane SOCFPGA preloader\n");
	else
		printf("Not a sane SOCFPGA preloader\n");
}

static int socfpgaimage_check_params(struct image_tool_params *params)
{
	/* Not sure if we should be accepting fflags */
	return	(params->dflag && (params->fflag || params->lflag)) ||
		(params->fflag && (params->dflag || params->lflag)) ||
		(params->lflag && (params->dflag || params->fflag));
}

static int socfpgaimage_check_image_types_v0(uint8_t type)
{
	if (type == IH_TYPE_SOCFPGAIMAGE)
		return EXIT_SUCCESS;
	return EXIT_FAILURE;
}

static int socfpgaimage_check_image_types_v1(uint8_t type)
{
	if (type == IH_TYPE_SOCFPGAIMAGE_V1)
		return EXIT_SUCCESS;
	return EXIT_FAILURE;
}

/*
 * To work in with the mkimage framework, we do some ugly stuff...
 *
 * First, socfpgaimage_vrec_header() is called.
 * We prepend a fake header big enough to make the file sfp_pad_size().
 * This gives us enough space to do what we want later.
 *
 * Next, socfpgaimage_set_header() is called.
 * We fix up the buffer by moving the image to the start of the buffer.
 * We now have some room to do what we need (add CRC and padding).
 */

static int data_size;

static int sfp_fake_header_size(unsigned int size, uint8_t ver)
{
	return sfp_pad_size(ver) - size;
}

static int sfp_vrec_header(struct image_tool_params *params,
			   struct image_type_params *tparams, uint8_t ver)
{
	struct stat sbuf;

	if (params->datafile &&
	    stat(params->datafile, &sbuf) == 0 &&
	    sbuf.st_size <= (sfp_pad_size(ver) - sizeof(uint32_t))) {
		data_size = sbuf.st_size;
		tparams->header_size = sfp_fake_header_size(data_size, ver);
	}
	return 0;

}

static int socfpgaimage_vrec_header_v0(struct image_tool_params *params,
				       struct image_type_params *tparams)
{
	return sfp_vrec_header(params, tparams, 0);
}

static int socfpgaimage_vrec_header_v1(struct image_tool_params *params,
				       struct image_type_params *tparams)
{
	return sfp_vrec_header(params, tparams, 1);
}

static void sfp_set_header(void *ptr, unsigned char ver)
{
	uint8_t *buf = (uint8_t *)ptr;

	/*
	 * This function is called after vrec_header() has been called.
	 * At this stage we have the sfp_fake_header_size() dummy bytes
	 * followed by data_size image bytes. Total = sfp_pad_size().
	 * We need to fix the buffer by moving the image bytes back to
	 * the beginning of the buffer, then actually do the signing stuff...
	 */
	memmove(buf, buf + sfp_fake_header_size(data_size, ver), data_size);
	memset(buf + data_size, 0, sfp_fake_header_size(data_size, ver));

	sfp_sign_buffer(buf, ver, 0, data_size, 0);
}

static void socfpgaimage_set_header_v0(void *ptr, struct stat *sbuf, int ifd,
				       struct image_tool_params *params)
{
	sfp_set_header(ptr, 0);
}

static void socfpgaimage_set_header_v1(void *ptr, struct stat *sbuf, int ifd,
				       struct image_tool_params *params)
{
	sfp_set_header(ptr, 1);
}

U_BOOT_IMAGE_TYPE(
	socfpgaimage,
	"Altera SoCFPGA Cyclone V / Arria V image support",
	0, /* This will be modified by vrec_header() */
	(void *)buffer_v0,
	socfpgaimage_check_params,
	socfpgaimage_verify_header,
	socfpgaimage_print_header,
	socfpgaimage_set_header_v0,
	NULL,
	socfpgaimage_check_image_types_v0,
	NULL,
	socfpgaimage_vrec_header_v0
);

U_BOOT_IMAGE_TYPE(
	socfpgaimage_v1,
	"Altera SoCFPGA Arria10 image support",
	0, /* This will be modified by vrec_header() */
	(void *)buffer_v1,
	socfpgaimage_check_params,
	socfpgaimage_verify_header,
	socfpgaimage_print_header,
	socfpgaimage_set_header_v1,
	NULL,
	socfpgaimage_check_image_types_v1,
	NULL,
	socfpgaimage_vrec_header_v1
);
