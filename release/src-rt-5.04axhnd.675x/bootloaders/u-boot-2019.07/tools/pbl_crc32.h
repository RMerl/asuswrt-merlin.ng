/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2012 Freescale Semiconductor, Inc.
 */

#ifndef PBLCRC32_H
#define PBLCRC32_H

#include <stdint.h>
uint32_t pbl_crc32(uint32_t in_crc, const char *buf, uint32_t len);

#endif
