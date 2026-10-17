/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef AT91_SCK_H
#define AT91_SCK_H

/*
 * SCKCR flags
 */
#define AT91SAM9G45_SCKCR_RCEN	    (1 << 0)	/* RC Oscillator Enable */
#define AT91SAM9G45_SCKCR_OSC32EN   (1 << 1)	/* 32kHz Oscillator Enable */
#define AT91SAM9G45_SCKCR_OSC32BYP  (1 << 2)	/* 32kHz Oscillator Bypass */
#define AT91SAM9G45_SCKCR_OSCSEL    (1 << 3)	/* Slow Clock Selector */
#define		AT91SAM9G45_SCKCR_OSCSEL_RC	(0 << 3)
#define		AT91SAM9G45_SCKCR_OSCSEL_32	(1 << 3)

#endif
