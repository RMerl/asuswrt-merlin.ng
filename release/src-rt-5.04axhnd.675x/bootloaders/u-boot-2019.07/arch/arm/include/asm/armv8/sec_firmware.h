/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 NXP Semiconductor, Inc.
 */

#ifndef __SEC_FIRMWARE_H_
#define __SEC_FIRMWARE_H_

#define PSCI_INVALID_VER		0xffffffff
#define SEC_JR3_OFFSET			0x40000
#define WORD_MASK			0xffffffff
#define WORD_SHIFT			32

int sec_firmware_init(const void *, u32 *, u32 *, u32 *, u32 *);
int _sec_firmware_entry(const void *, u32 *, u32 *);
bool sec_firmware_is_valid(const void *);
bool sec_firmware_support_hwrng(void);
int sec_firmware_get_random(uint8_t *rand, int bytes);
int fdt_fixup_kaslr(void *fdt);
#ifdef CONFIG_SEC_FIRMWARE_ARMV8_PSCI
unsigned int sec_firmware_support_psci_version(void);
unsigned int _sec_firmware_support_psci_version(void);
#else
static inline unsigned int sec_firmware_support_psci_version(void)
{
	return PSCI_INVALID_VER;
}
#endif

static inline unsigned int sec_firmware_used_jobring_offset(void)
{
	return SEC_JR3_OFFSET;
}

#endif /* __SEC_FIRMWARE_H_ */
