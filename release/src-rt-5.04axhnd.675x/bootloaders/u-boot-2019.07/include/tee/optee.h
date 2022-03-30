/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * OP-TEE related definitions
 *
 * (C) Copyright 2016 Linaro Limited
 * Andrew F. Davis <andrew.davis@linaro.org>
 */

#ifndef	_OPTEE_H
#define _OPTEE_H

#include <linux/errno.h>

#define OPTEE_MAGIC             0x4554504f
#define OPTEE_VERSION           1
#define OPTEE_ARCH_ARM32        0
#define OPTEE_ARCH_ARM64        1

struct optee_header {
	uint32_t magic;
	uint8_t version;
	uint8_t arch;
	uint16_t flags;
	uint32_t init_size;
	uint32_t init_load_addr_hi;
	uint32_t init_load_addr_lo;
	uint32_t init_mem_usage;
	uint32_t paged_size;
};

static inline uint32_t optee_image_get_entry_point(const image_header_t *hdr)
{
	struct optee_header *optee_hdr = (struct optee_header *)(hdr + 1);

	return optee_hdr->init_load_addr_lo;
}

static inline uint32_t optee_image_get_load_addr(const image_header_t *hdr)
{
	return optee_image_get_entry_point(hdr) - sizeof(struct optee_header);
}

#if defined(CONFIG_OPTEE)
int optee_verify_image(struct optee_header *hdr, unsigned long tzdram_start,
		       unsigned long tzdram_len, unsigned long image_len);
#else
static inline int optee_verify_image(struct optee_header *hdr,
				     unsigned long tzdram_start,
				     unsigned long tzdram_len,
				     unsigned long image_len)
{
	return -EPERM;
}

#endif

#if defined(CONFIG_OPTEE)
int optee_verify_bootm_image(unsigned long image_addr,
			     unsigned long image_load_addr,
			     unsigned long image_len);
#else
static inline int optee_verify_bootm_image(unsigned long image_addr,
					   unsigned long image_load_addr,
					   unsigned long image_len)
{
	return -EPERM;
}
#endif

#endif /* _OPTEE_H */
