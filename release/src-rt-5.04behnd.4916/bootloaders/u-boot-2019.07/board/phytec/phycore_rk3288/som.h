/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 PHYTEC Messtechnik GmbH
 * Author: Wadim Egorov <w.egorov@phytec.de>
 */

/*
 * rk3288_som struct represents the eeprom layout for PHYTEC RK3288 based SoMs
 */
struct rk3288_som {
	unsigned char api_version;	/* EEPROM layout API version */
	unsigned char mod_version;	/* PCM/PFL/PCA */
	unsigned char option[12];	/* coding for variants */
	unsigned char som_rev;		/* SOM revision */
	unsigned char mac[6];
	unsigned char ksp;		/* 1: KSP, 2: KSM */
	unsigned char kspno;		/* Number for KSP/KSM module */
	unsigned char reserved[8];	/* not used */
	unsigned char bs;		/* Bits set in previous bytes */
} __attribute__ ((__packed__));
