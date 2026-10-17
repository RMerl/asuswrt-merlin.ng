/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007
 * Eran Liberty, Extricom, eran.liberty@gmail.com
 */
#ifndef _STRATIXII_H_
#define _STRATIXII_H_

extern int StratixII_load (Altera_desc * desc, void *image, size_t size);
extern int StratixII_dump (Altera_desc * desc, void *buf, size_t bsize);
extern int StratixII_info (Altera_desc * desc);

#endif				/* _STRATIXII_H_ */
