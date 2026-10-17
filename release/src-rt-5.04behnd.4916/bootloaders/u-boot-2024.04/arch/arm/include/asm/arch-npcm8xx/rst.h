/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _NPCM_RST_H_
#define _NPCM_RST_H_

/* Watchdog Timer Controller Register */
#define WTCR0_REG	0xF000801C
#define WTCR_WTR	BIT(0)
#define WTCR_WTRE	BIT(1)
#define WTCR_WTE	BIT(7)

/* Reset status bits */
#define PORST		BIT(31)
#define CORST		BIT(30)
#define WD0RST		BIT(29)
#define SW1RST		BIT(28)
#define SW2RST		BIT(27)
#define SW3RST		BIT(26)
#define TIPRST		BIT(25)
#define WD1RST		BIT(24)
#define WD2RST		BIT(23)
#define RST_STS_MASK	GENMASK(31, 23)

int npcm_get_reset_status(void);

#endif
