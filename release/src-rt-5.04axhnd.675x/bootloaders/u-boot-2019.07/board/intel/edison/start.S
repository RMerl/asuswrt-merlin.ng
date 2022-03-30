/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
 */

/* board early intialization */
.globl early_board_init
early_board_init:
	/* No 32-bit board specific initialisation */
	jmp	early_board_init_ret
