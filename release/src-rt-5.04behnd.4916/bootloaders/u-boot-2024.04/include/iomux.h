/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#ifndef _IO_MUX_H
#define _IO_MUX_H

#include <stdio_dev.h>

/*
 * Stuff required to support console multiplexing.
 */

/*
 * Pointers to devices used for each file type.  Defined in console.c
 * but storage is allocated in iomux.c.
 */
extern struct stdio_dev **console_devices[MAX_FILES];
/*
 * The count of devices assigned to each FILE.  Defined in console.c
 * and populated in iomux.c.
 */
extern int cd_count[MAX_FILES];

#define for_each_console_dev(i, file, dev)				\
	for (i = 0;							\
	     i < cd_count[file] && (dev = console_devices[file][i]);	\
	     i++)

int iomux_match_device(struct stdio_dev **, const int, struct stdio_dev *);
int iomux_doenv(const int, const char *);
int iomux_replace_device(const int, const char *, const char *);
void iomux_printdevs(const int);

#endif /* _IO_MUX_H */
