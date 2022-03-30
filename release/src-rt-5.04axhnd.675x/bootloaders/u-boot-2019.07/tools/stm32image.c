// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <image.h>
#include "imagetool.h"

/* magic ='S' 'T' 'M' 0x32 */
#define HEADER_MAGIC be32_to_cpu(0x53544D32)
#define VER_MAJOR_IDX	2
#define VER_MINOR_IDX	1
#define VER_VARIANT_IDX	0
#define HEADER_VERSION_V1	0x1
/* default option : bit0 => no signature */
#define HEADER_DEFAULT_OPTION	(cpu_to_le32(0x00000001))
/* default binary type for U-Boot */
#define HEADER_TYPE_UBOOT	(cpu_to_le32(0x00000000))

struct stm32_header {
	uint32_t magic_number;
	uint32_t image_signature[64 / 4];
	uint32_t image_checksum;
	uint8_t  header_version[4];
	uint32_t image_length;
	uint32_t image_entry_point;
	uint32_t reserved1;
	uint32_t load_address;
	uint32_t reserved2;
	uint32_t version_number;
	uint32_t option_flags;
	uint32_t ecdsa_algorithm;
	uint32_t ecdsa_public_key[64 / 4];
	uint32_t padding[83 / 4];
	uint32_t binary_type;
};

static struct stm32_header stm32image_header;

static void stm32image_default_header(struct stm32_header *ptr)
{
	if (!ptr)
		return;

	ptr->magic_number = HEADER_MAGIC;
	ptr->header_version[VER_MAJOR_IDX] = HEADER_VERSION_V1;
	ptr->option_flags = HEADER_DEFAULT_OPTION;
	ptr->ecdsa_algorithm = 1;
	ptr->binary_type = HEADER_TYPE_UBOOT;
}

static uint32_t stm32image_checksum(void *start, uint32_t len)
{
	uint32_t csum = 0;
	uint32_t hdr_len = sizeof(struct stm32_header);
	uint8_t *p;

	if (len < hdr_len)
		return 0;

	p = start + hdr_len;
	len -= hdr_len;

	while (len > 0) {
		csum += *p;
		p++;
		len--;
	}

	return csum;
}

static int stm32image_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_STM32IMAGE)
		return EXIT_SUCCESS;
	return EXIT_FAILURE;
}

static int stm32image_verify_header(unsigned char *ptr, int image_size,
				    struct image_tool_params *params)
{
	struct stm32_header *stm32hdr = (struct stm32_header *)ptr;
	int i;

	if (image_size < sizeof(struct stm32_header))
		return -1;
	if (stm32hdr->magic_number != HEADER_MAGIC)
		return -1;
	if (stm32hdr->header_version[VER_MAJOR_IDX] != HEADER_VERSION_V1)
		return -1;
	if (stm32hdr->reserved1 || stm32hdr->reserved2)
		return -1;
	for (i = 0; i < (sizeof(stm32hdr->padding) / 4); i++) {
		if (stm32hdr->padding[i] != 0)
			return -1;
	}

	return 0;
}

static void stm32image_print_header(const void *ptr)
{
	struct stm32_header *stm32hdr = (struct stm32_header *)ptr;

	printf("Image Type   : STMicroelectronics STM32 V%d.%d\n",
	       stm32hdr->header_version[VER_MAJOR_IDX],
	       stm32hdr->header_version[VER_MINOR_IDX]);
	printf("Image Size   : %lu bytes\n",
	       (unsigned long)le32_to_cpu(stm32hdr->image_length));
	printf("Image Load   : 0x%08x\n",
	       le32_to_cpu(stm32hdr->load_address));
	printf("Entry Point  : 0x%08x\n",
	       le32_to_cpu(stm32hdr->image_entry_point));
	printf("Checksum     : 0x%08x\n",
	       le32_to_cpu(stm32hdr->image_checksum));
	printf("Option     : 0x%08x\n",
	       le32_to_cpu(stm32hdr->option_flags));
	printf("BinaryType : 0x%08x\n",
	       le32_to_cpu(stm32hdr->binary_type));
}

static void stm32image_set_header(void *ptr, struct stat *sbuf, int ifd,
				  struct image_tool_params *params)
{
	struct stm32_header *stm32hdr = (struct stm32_header *)ptr;

	stm32image_default_header(stm32hdr);

	stm32hdr->load_address = cpu_to_le32(params->addr);
	stm32hdr->image_entry_point = cpu_to_le32(params->ep);
	stm32hdr->image_length = cpu_to_le32((uint32_t)sbuf->st_size -
					     sizeof(struct stm32_header));
	stm32hdr->image_checksum = stm32image_checksum(ptr, sbuf->st_size);
}

/*
 * stm32image parameters
 */
U_BOOT_IMAGE_TYPE(
	stm32image,
	"STMicroelectronics STM32MP Image support",
	sizeof(struct stm32_header),
	(void *)&stm32image_header,
	NULL,
	stm32image_verify_header,
	stm32image_print_header,
	stm32image_set_header,
	NULL,
	stm32image_check_image_types,
	NULL,
	NULL
);
