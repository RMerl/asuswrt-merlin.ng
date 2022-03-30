/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Marek Behun <marek.behun@nic.cz>
 */

#ifndef _BOARD_CZNIC_TURRIS_MOX_MOX_SP_H_
#define _BOARD_CZNIC_TURRIS_MOX_MOX_SP_H_

#include <common.h>

const char *mox_sp_get_ecdsa_public_key(void);
int mbox_sp_get_board_info(u64 *sn, u8 *mac1, u8 *mac2, int *bv,
			   int *ram);

#endif /* _BOARD_CZNIC_TURRIS_MOX_MOX_SP_H_ */
