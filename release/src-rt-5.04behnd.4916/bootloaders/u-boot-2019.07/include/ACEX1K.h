/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2003
 * Steven Scholz, imc Measurement & Control, steven.scholz@imc-berlin.de
 *
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 */

#ifndef _ACEX1K_H_
#define _ACEX1K_H_

#include <altera.h>

extern int ACEX1K_load(Altera_desc *desc, const void *image, size_t size);
extern int ACEX1K_dump(Altera_desc *desc, const void *buf, size_t bsize);
extern int ACEX1K_info(Altera_desc *desc);

extern int CYC2_load(Altera_desc *desc, const void *image, size_t size);
extern int CYC2_dump(Altera_desc *desc, const void *buf, size_t bsize);
extern int CYC2_info(Altera_desc *desc);

/* Slave Serial Implementation function table */
typedef struct {
	Altera_pre_fn		pre;
	Altera_config_fn	config;
	Altera_clk_fn		clk;
	Altera_status_fn	status;
	Altera_done_fn		done;
	Altera_data_fn		data;
	Altera_abort_fn		abort;
	Altera_post_fn		post;
} Altera_ACEX1K_Passive_Serial_fns;

/* Slave Serial Implementation function table */
typedef struct {
	Altera_pre_fn		pre;
	Altera_config_fn	config;
	Altera_status_fn	status;
	Altera_done_fn		done;
	Altera_write_fn		write;
	Altera_abort_fn		abort;
	Altera_post_fn		post;
} Altera_CYC2_Passive_Serial_fns;

/* Device Image Sizes
 *********************************************************************/
/* ACEX1K */
/* FIXME: Which size do we mean?
 * Datasheet says 1337000/8=167125Bytes,
 * Filesize of an *.rbf file is 166965 Bytes
 */
#if 0
#define Altera_EP1K100_SIZE	1337000/8	/* 167125 Bytes */
#endif
#define Altera_EP1K100_SIZE	(166965*8)

#define Altera_EP2C8_SIZE	247942
#define Altera_EP2C20_SIZE	586562
#define Altera_EP2C35_SIZE	883905
#define Altera_EP3C5_SIZE	368011		/* .rbf size in bytes */

/* Descriptor Macros
 *********************************************************************/
/* ACEX1K devices */
#define Altera_EP1K100_DESC(iface, fn_table, cookie) \
{ Altera_ACEX1K, iface, Altera_EP1K100_SIZE, fn_table, cookie }

#endif /* _ACEX1K_H_ */
