/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Marek Behún <kabel@kernel.org>
 */

#ifndef _BOARD_CZNIC_TURRIS_MOX_MOX_SP_H_
#define _BOARD_CZNIC_TURRIS_MOX_MOX_SP_H_

enum cznic_a3720_board {
	BOARD_UNDEFINED		= 0x0,
	BOARD_TURRIS_MOX	= 0x1,
	BOARD_RIPE_ATLAS	= 0x3,
};

const char *mox_sp_get_ecdsa_public_key(void);
int mbox_sp_get_board_info(u64 *sn, u8 *mac1, u8 *mac2, int *bv,
			   int *ram, enum cznic_a3720_board *board);

#endif /* _BOARD_CZNIC_TURRIS_MOX_MOX_SP_H_ */
