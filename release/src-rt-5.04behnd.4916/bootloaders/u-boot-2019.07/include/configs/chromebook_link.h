/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/x86-common.h>
#include <configs/x86-chromebook.h>

#define CONFIG_ENV_SECT_SIZE		0x1000
#define CONFIG_ENV_OFFSET		0x003f8000

#define CONFIG_SPL_BOARD_LOAD_IMAGE

#endif	/* __CONFIG_H */
