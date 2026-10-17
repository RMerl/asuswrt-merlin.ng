/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2015 The Chromium OS Authors. All rights reserved.
 */

/*
 * EMEM data may be accessed through port 62/66 or through LPC at 900h.
 */

Offset (0x00),
TIN0, 8,	// Temperature 0
TIN1, 8,	// Temperature 1
TIN2, 8,	// Temperature 2
TIN3, 8,	// Temperature 3
TIN4, 8,	// Temperature 4
TIN5, 8,	// Temperature 5
TIN6, 8,	// Temperature 6
TIN7, 8,	// Temperature 7
TIN8, 8,	// Temperature 8
TIN9, 8,	// Temperature 9
Offset (0x10),
FAN0, 16,	// Fan Speed 0
Offset (0x24),
BTVR, 8,	// Battery structure version
Offset (0x30),
LIDS, 1,	// Lid Switch State
PBTN, 1,	// Power Button Pressed
WPDI, 1,	// Write Protect Disabled
RECK, 1,	// Keyboard Initiated Recovery
RECD, 1,	// Dedicated Recovery Mode
Offset (0x40),
BTVO, 32,	// Battery Present Voltage
BTPR, 32,	// Battery Present Rate
BTRA, 32,	// Battery Remaining Capacity
ACEX, 1,	// AC Present
BTEX, 1,	// Battery Present
BFDC, 1,	// Battery Discharging
BFCG, 1,	// Battery Charging
BFCR, 1,	// Battery Level Critical
Offset (0x4d),
BTCN, 8,	// Battery Count
BTIX, 8,	// Battery index
Offset (0x50),
BTDA, 32,	// Battery Design Capacity
BTDV, 32,	// Battery Design Voltage
BTDF, 32,	// Battery Last Full Charge Capacity
BTCC, 32,	// Battery Cycle Count
BMFG, 64,	// Battery Manufacturer String
BMOD, 64,	// Battery Model String
BSER, 64,	// Battery Serial String
BTYP, 64,	// Battery Type String
Offset (0x80),
ALS0, 16,	// ALS reading 0 in lux
