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

int iomux_doenv(const int, const char *);
void iomux_printdevs(const int);
struct stdio_dev *search_device(int, const char *);

#endif /* _IO_MUX_H */
