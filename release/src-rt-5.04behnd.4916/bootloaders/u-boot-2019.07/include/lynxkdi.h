/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2003
 * Orbacom Systems, Inc.
 */

#ifndef __LYNXKDI_H__
#define __LYNXKDI_H__


/* Boot parameter struct passed to kernel
 */
typedef struct lynxos_bootparms_t {
	uint8_t		rsvd1[2];	/* Reserved			*/
	uint8_t		ethaddr[6];	/* Ethernet address		*/
	uint16_t	flags;		/* Boot flags			*/
	uint32_t	rate;		/* System frequency		*/
	uint32_t	clock_ref;	/* Time reference		*/
	uint32_t	dramsz;		/* DRAM size			*/
	uint32_t	rsvd2;		/* Reserved			*/
} lynxos_bootparms_t;


#endif	/* __LYNXKDI_H__ */
