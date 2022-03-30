/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2008
 * Sergei Poselenov, Emcraft Systems, sposelenov@emcraft.com.
 *
 * Copyright 2004, 2007 Freescale Semiconductor, Inc.
 * (C) Copyright 2003 Motorola Inc.
 * Xianghua Xiao, (X.Xiao@motorola.com)
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef __UPM_TABLE_H
#define __UPM_TABLE_H

/* UPM Table Configuration Code for FPGA access */
static const unsigned int UPMTableA[] =
{
	0x00fcec00,  0x00fcec00,  0x00fcec00,  0x00fcec00, /* Words 0 to 3	*/
	0x00fcec00,  0x00fcfc00,  0x00fcfc00,  0x00fcec05, /* Words 4 to 7	*/
	0x00fcec00,  0x00fcec00,  0x00fcec04,  0x00fcec04, /* Words 8 to 11	*/
	0x00fcec04,  0x00fcec04,  0x00fcec04,  0x00fcec04, /* Words 12 to 15	*/
	0x00fcec04,  0x00fcec04,  0x0fffec00,  0xffffec00, /* Words 16 to 19	*/
	0xffffec00,  0xffffec00,  0xffffec00,  0xffffec01, /* Words 20 to 23	*/
	0x00ffec00,  0x00ffec00,  0x00f3ec00,  0x0fffec00, /* Words 24 to 27	*/
	0x0ffffc04,  0xffffec00,  0xffffec00,  0xffffec01, /* Words 28 to 31	*/
	0x00ffec00,  0x00ffec00,  0x00f3ec04,  0x00f3ec04, /* Words 32 to 35	*/
	0x00f3ec04,  0x00f3ec04,  0x00f3ec04,  0x00f3ec04, /* Words 36 to 39	*/
	0x00f3ec04,  0x00f3ec04,  0x0fffec00,  0xffffec00, /* Words 40 to 43	*/
	0xffffec00,  0xffffec00,  0xffffec00,  0xffffec01, /* Words 44 to 47	*/
	0xffffec00,  0xffffec00,  0xffffec00,  0xffffec00, /* Words 48 to 51	*/
	0xffffec00,  0xffffec00,  0xffffec00,  0xffffec00, /* Words 52 to 55	*/
	0xffffec00,  0xffffec00,  0xffffec00,  0xffffec01, /* Words 56 to 59	*/
	0xffffec00,  0xffffec00,  0xffffec00,  0xffffec01  /* Words 60 to 63	*/
};

/* LIME UPM B Table Configuration Code */
static unsigned int UPMTableB[] =
{
	0x0ffefc00,  0x0ffcfc00,  0x0ffcfc00,  0x0ffcfc00, /* Words 0 to 3	*/
	0x0ffcfc00,  0x0ffcfc00,  0x0ffcfc04,  0x0ffffc01, /* Words 4 to 7	*/
	0x0ffefc00,  0x0ffcfc00,  0x0ffcfc00,  0x0ffcfc00, /* Words 8 to 11	*/
	0x0ffcfc00,  0x0ffcfc00,  0x0ffcfc04,  0x0ffcfc04, /* Words 12 to 15	*/
	0x0ffcfc04,  0x0ffcfc04,  0x0ffcfc04,  0x0ffcfc04, /* Words 16 to 19	*/
	0x0ffcfc04,  0x0ffcfc04,  0x0ffffc00,  0xfffffc01, /* Words 20 to 23	*/
	0x0cfffc00,  0x00fffc00,  0x00fffc00,  0x00fffc00, /* Words 24 to 27	*/
	0x00fffc00,  0x00fffc00,  0x00fffc04,  0x0ffffc01, /* Words 28 to 31	*/
	0x0cfffc00,  0x00fffc00,  0x00fffc00,  0x00fffc00, /* Words 32 to 35	*/
	0x00fffc00,  0x00fffc00,  0x00fffc04,  0x00fffc04, /* Words 36 to 39	*/
	0x00fffc04,  0x00fffc04,  0x00fffc04,  0x00fffc04, /* Words 40 to 43	*/
	0x00fffc04,  0x00fffc04,  0x0ffffc00,  0xfffffc01, /* Words 44 to 47	*/
	0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc00, /* Words 48 to 51	*/
	0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc00, /* Words 52 to 55	*/
	0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc01, /* Words 56 to 59	*/
	0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc01  /* Words 60 to 63	*/
};
#endif
