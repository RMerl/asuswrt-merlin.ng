/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#ifndef __SOC_H__
#define __SOC_H__

/* Product PARTNUM */
#define CN81XX	0xA2
#define CN83XX	0xA3
#define CN96XX	0xB2
#define CN95XX	0xB3

#define otx_is_altpkg()	read_alt_pkg()
#define otx_is_soc(soc)	(read_partnum() == (soc))
#define otx_is_board(model) (!strcmp(read_board_name(), model))
#define otx_is_platform(platform) (read_platform() == (platform))

enum platform {
	PLATFORM_HW = 0,
	PLATFORM_EMULATOR = 1,
	PLATFORM_ASIM = 3,
};

int read_platform(void);
u8 read_partnum(void);
const char *read_board_name(void);
bool read_alt_pkg(void);

#endif /* __SOC_H */
