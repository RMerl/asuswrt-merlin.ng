/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2000,2001,2002,2003,2004  Free Software Foundation, Inc.
 */
/*
 * Copyright 2010 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */
#ifndef	_SYS_SA_IMPL_H
#define	_SYS_SA_IMPL_H

typedef struct sa_hdr_phys {
	uint32_t sa_magic;
	uint16_t sa_layout_info;
	uint16_t sa_lengths[1];
} sa_hdr_phys_t;

#define	SA_HDR_SIZE(hdr)	BF32_GET_SB(hdr->sa_layout_info, 10, 16, 3, 0)
#define	SA_SIZE_OFFSET	0x8

#endif	/* _SYS_SA_IMPL_H */
