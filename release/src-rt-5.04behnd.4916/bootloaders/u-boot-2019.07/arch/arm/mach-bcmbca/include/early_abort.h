/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _EARLY_ABORT_H
#define _EARLY_ABORT_H
typedef enum _spl_ea_status_e {
	SPL_EA_NONE = 0x0,
	SPL_EA_DDR3_SAFE_MODE = 0x1,
	SPL_EA_DDR4_SAFE_MODE = 0x2,
	SPL_EA_LPDDR4_SAFE_MODE = 0x4,
	SPL_EA_LPDDR5_SAFE_MODE = 0x8,
	SPL_EA_LPDDR4X_SAFE_MODE = 0x10,
	SPL_EA_LPDDR5X_SAFE_MODE = 0x20,
	SPL_EA_DDR_MCB_SEL = 0x40,
	SPL_EA_IMAGE_RECOV = 0x80,
	SPL_EA_IMAGE_FB = 0x100,
	SPL_EA_IGNORE_BOARDID = 0x200,
	SPL_EA_JTAG_UNLOCK = 0x400,
	SPL_EA_WDT_DISABLE = 0x800,
} spl_ea_status_t;

#define SPL_EA_DDR_SAFE_MODE_MASK					\
	(SPL_EA_DDR3_SAFE_MODE|SPL_EA_DDR4_SAFE_MODE|	\
	SPL_EA_LPDDR4_SAFE_MODE|SPL_EA_LPDDR5_SAFE_MODE|\
	SPL_EA_LPDDR4X_SAFE_MODE|SPL_EA_LPDDR5X_SAFE_MODE)

typedef struct e_abort_s {
	spl_ea_status_t status;
	uint32_t data;
} early_abort_t;

void early_abort(void);
early_abort_t* early_abort_info(void);
#define SPL_EA_TM_MS 60000
#define SPL_EA_CATCH_TM_MS 200
#endif
