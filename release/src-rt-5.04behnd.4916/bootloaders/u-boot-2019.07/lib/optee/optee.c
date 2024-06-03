// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Linaro
 * Bryan O'Donoghue <bryan.odonoghue@linaro.org>
 */

#include <common.h>
#include <tee/optee.h>

#define optee_hdr_err_msg \
	"OPTEE verification error:" \
	"\n\thdr=%p image=0x%08lx magic=0x%08x tzdram 0x%08lx-0x%08lx " \
	"\n\theader lo=0x%08x hi=0x%08x size=0x%08lx arch=0x%08x" \
	"\n\tuimage params 0x%08lx-0x%08lx\n"

int optee_verify_image(struct optee_header *hdr, unsigned long tzdram_start,
		       unsigned long tzdram_len, unsigned long image_len)
{
	unsigned long tzdram_end = tzdram_start + tzdram_len;
	uint32_t tee_file_size;

	tee_file_size = hdr->init_size + hdr->paged_size +
			sizeof(struct optee_header);

	if (hdr->magic != OPTEE_MAGIC ||
	    hdr->version != OPTEE_VERSION ||
	    hdr->init_load_addr_hi > tzdram_end ||
	    hdr->init_load_addr_lo < tzdram_start ||
	    tee_file_size > tzdram_len ||
	    tee_file_size != image_len ||
	    (hdr->init_load_addr_lo + tee_file_size) > tzdram_end) {
		return -EINVAL;
	}

	return 0;
}

int optee_verify_bootm_image(unsigned long image_addr,
			     unsigned long image_load_addr,
			     unsigned long image_len)
{
	struct optee_header *hdr = (struct optee_header *)image_addr;
	unsigned long tzdram_start = CONFIG_OPTEE_TZDRAM_BASE;
	unsigned long tzdram_len = CONFIG_OPTEE_TZDRAM_SIZE;

	int ret;

	ret = optee_verify_image(hdr, tzdram_start, tzdram_len, image_len);
	if (ret)
		goto error;

	if (image_load_addr + sizeof(*hdr) != hdr->init_load_addr_lo) {
		ret = -EINVAL;
		goto error;
	}

	return ret;
error:
	printf(optee_hdr_err_msg, hdr, image_addr, hdr->magic, tzdram_start,
	       tzdram_start + tzdram_len, hdr->init_load_addr_lo,
	       hdr->init_load_addr_hi, image_len, hdr->arch, image_load_addr,
	       image_load_addr + image_len);

	return ret;
}
