// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 NXP
 */

#include <common.h>
#include <console.h>
#include <errno.h>
#include <fuse.h>
#include <asm/arch/sci/sci.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

#define FSL_ECC_WORD_START_1	 0x10
#define FSL_ECC_WORD_END_1	 0x10F

#define FSL_ECC_WORD_START_2	 0x220
#define FSL_ECC_WORD_END_2	 0x31F

#define FSL_QXP_FUSE_GAP_START	 0x110
#define FSL_QXP_FUSE_GAP_END	 0x21F

#define FSL_SIP_OTP_READ             0xc200000A
#define FSL_SIP_OTP_WRITE            0xc200000B

int fuse_read(u32 bank, u32 word, u32 *val)
{
	return fuse_sense(bank, word, val);
}

int fuse_sense(u32 bank, u32 word, u32 *val)
{
	unsigned long ret = 0, value = 0;

	if (bank != 0) {
		printf("Invalid bank argument, ONLY bank 0 is supported\n");
		return -EINVAL;
	}

	ret = call_imx_sip_ret2(FSL_SIP_OTP_READ, (unsigned long)word, &value,
				0, 0);
	*val = (u32)value;

	return ret;
}

int fuse_prog(u32 bank, u32 word, u32 val)
{
	if (bank != 0) {
		printf("Invalid bank argument, ONLY bank 0 is supported\n");
		return -EINVAL;
	}

	if (IS_ENABLED(CONFIG_IMX8QXP)) {
		if (word >= FSL_QXP_FUSE_GAP_START &&
		    word <= FSL_QXP_FUSE_GAP_END) {
			printf("Invalid word argument for this SoC\n");
			return -EINVAL;
		}
	}

	if ((word >= FSL_ECC_WORD_START_1 && word <= FSL_ECC_WORD_END_1) ||
	    (word >= FSL_ECC_WORD_START_2 && word <= FSL_ECC_WORD_END_2)) {
		puts("Warning: Words in this index range have ECC protection\n"
		     "and can only be programmed once per word. Individual bit\n"
		     "operations will be rejected after the first one.\n"
		     "\n\n Really program this word? <y/N>\n");

		if (!confirm_yesno()) {
			puts("Word programming aborted\n");
			return -EPERM;
		}
	}

	return call_imx_sip(FSL_SIP_OTP_WRITE, (unsigned long)word,
			    (unsigned long)val, 0);
}

int fuse_override(u32 bank, u32 word, u32 val)
{
	printf("Override fuse to i.MX8 in u-boot is forbidden\n");
	return -EPERM;
}
