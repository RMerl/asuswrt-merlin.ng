/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#ifndef _NUVOTON_NCT6102D_H_
#define _NUVOTON_NCT6102D_H_

/* I/O address of Nuvoton Super IO chip */
#define NCT6102D_IO_PORT	0x4e

/* Extended Function Enable Registers */
#define NCT_EFER (NCT6102D_IO_PORT + 0)
/* Extended Function Index Register (same as EFER) */
#define NCT_EFIR (NCT6102D_IO_PORT + 0)
/* Extended Function Data Register */
#define NCT_EFDR (NCT_EFIR + 1)

#define NCT_LD_SELECT_REG	0x07

/* Logical device number */
#define NCT6102D_LD_UARTA	0x02
#define NCT6102D_LD_WDT		0x08

#define NCT6102D_UARTA_ENABLE	0x30
#define NCT6102D_WDT_TIMEOUT	0xf1

#define NCT_ENTRY_KEY		0x87
#define NCT_EXIT_KEY		0xaa

int nct6102d_wdt_disable(void);

#endif /* _NUVOTON_NCT6102D_H_ */
