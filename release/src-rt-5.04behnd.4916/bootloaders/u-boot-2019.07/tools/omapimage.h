/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Linaro LTD, www.linaro.org
 * Author John Rigby <john.rigby@linaro.org>
 * Based on TI's signGP.c
 */

#ifndef _OMAPIMAGE_H_
#define _OMAPIMAGE_H_

struct ch_toc {
	uint32_t section_offset;
	uint32_t section_size;
	uint8_t unused[12];
	uint8_t section_name[12];
};

struct ch_settings {
	uint32_t section_key;
	uint8_t valid;
	uint8_t version;
	uint16_t reserved;
	uint32_t flags;
};

#define KEY_CHSETTINGS 0xC0C0C0C1
#endif /* _OMAPIMAGE_H_ */
