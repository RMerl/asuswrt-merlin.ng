/* SPDX-License-Identifier:    GPL-2.0
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#ifndef __OCTEONTX2_COMMON_H__
#define __OCTEONTX2_COMMON_H__

/** Maximum size of image supported for bootm (and bootable FIT images) */

/** Memory base address */
#define CFG_SYS_SDRAM_BASE		CONFIG_TEXT_BASE

/** Stack starting address */

/** Extra environment settings */
#define CFG_EXTRA_ENV_SETTINGS	\
					"loadaddr=20080000\0"	\
					"ethrotate=yes\0"

#if defined(CONFIG_MMC_OCTEONTX)
#define MMC_SUPPORTS_TUNING
/** EMMC specific defines */
#endif

#endif /* __OCTEONTX2_COMMON_H__ */
