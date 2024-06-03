/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#ifndef _NAND_SPL_H_
#define	_NAND_SPL_H_

#define SPL_EXPORT	(0x00000001)

#define SPL_EXPORT_FDT		(0x00000001)
#define SPL_EXPORT_ATAGS	(0x00000002)
#define SPL_EXPORT_LAST		SPL_EXPORT_ATAGS

#endif /* _NAND_SPL_H_ */
