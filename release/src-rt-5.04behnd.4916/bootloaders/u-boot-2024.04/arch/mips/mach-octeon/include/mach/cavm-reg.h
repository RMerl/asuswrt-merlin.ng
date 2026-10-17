/* SPDX-License-Identifier:    GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __CAVM_REG_H__

/* Register offsets */
#define CAVM_CIU_FUSE			0x00010100000001a0
#define CAVM_MIO_BOOT_REG_CFG0		0x0001180000000000
#define CAVM_RST_BOOT			0x0001180006001600

/* Register bits */
#define RST_BOOT_C_MUL			GENMASK_ULL(36, 30)
#define RST_BOOT_PNR_MUL		GENMASK_ULL(29, 24)

#endif /* __CAVM_REG_H__ */
