/*
<:copyright-BRCM:2015:GPL/GPL:spu

   Copyright (c) 2015 Broadcom
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

#ifndef _UTIL_H
#define _UTIL_H

#include <linux/kernel.h>
#include <linux/delay.h>
#include <crypto/hash.h>
#include <crypto/scatterwalk.h>
#include <linux/dma-direction.h>
#include "cipher.h"
#include "spu.h"

extern int flow_debug_logging;
extern int packet_debug_logging;

extern int debug_logging_sleep;

//#define DEBUG

#ifdef DEBUG
#define flow_log(...)	                \
	do {	                              \
		if (flow_debug_logging) {	        \
			printk(__VA_ARGS__);	          \
			if (debug_logging_sleep)	      \
				msleep(debug_logging_sleep);	\
		}	                                \
	} while (0)
#define flow_dump(msg, var, var_len)	   \
	do {	                                 \
		if (flow_debug_logging) {	           \
			print_hex_dump(KERN_ALERT, msg, DUMP_PREFIX_NONE,  \
					16, 1, var, var_len, false); \
				if (debug_logging_sleep)	       \
					msleep(debug_logging_sleep);   \
		}                                    \
	} while (0)

#define packet_log(...)               \
	do {                                \
		if (packet_debug_logging) {       \
			printk(__VA_ARGS__);            \
			if (debug_logging_sleep)        \
				msleep(debug_logging_sleep);  \
		}                                 \
	} while (0)
#define packet_dump(msg, var, var_len)   \
	do {                                   \
		if (packet_debug_logging) {          \
			print_hex_dump(KERN_ALERT, msg, DUMP_PREFIX_NONE,  \
					16, 1, var, var_len, false); \
			if (debug_logging_sleep)           \
				msleep(debug_logging_sleep);     \
		}                                    \
	} while (0)

void __dump_sg(struct scatterlist *sg, unsigned skip, unsigned len);

#define dump_sg(sg, skip, len)     __dump_sg(sg, skip, len)

#else /* !DEBUG_ON */

#define flow_log(...) do {} while (0)
#define flow_dump(msg, var, var_len) do {} while (0)
#define packet_log(...) do {} while (0)
#define packet_dump(msg, var, var_len) do {} while (0)

#define dump_active_list() do {} while (0)
#define dump_sg(sg, skip, len) do {} while (0)

#endif /* DEBUG_ON */

/* Copy sg data, from skip, length len, to dest */
void sg_copy_part_to_buf(struct scatterlist *src, u8 *dest,
			 unsigned int len, unsigned skip);
/* Copy src into scatterlist from offset, length len */
void sg_copy_part_from_buf(struct scatterlist *dest, u8 *src,
			   unsigned len, unsigned skip);

int spu_sg_count(struct scatterlist *sg_list, int nbytes, int skip);
u32 spu_msg_sg_add(struct scatterlist **to_sg,
		   struct scatterlist **from_sg, u32 *skip,
		   u8 from_nents, u32 tot_len);

void add_to_ctr(uint8_t *ctr_pos, unsigned increment);

/* do a synchronous decrypt operation */
int do_decrypt(char *alg_name,
	       void *key_ptr, unsigned key_len,
	       void *iv_ptr, void *src_ptr, void *dst_ptr, unsigned block_len);

/* produce a message digest from data of length n bytes */
int do_shash(unsigned char *name, unsigned char *result,
	     const uint8_t *data1, unsigned data1_len,
	     const uint8_t *data2, unsigned data2_len);

char *spu_alg_name(enum spu_cipher_alg alg, enum spu_cipher_mode mode);

void spu_setup_debugfs(struct spu_private *iproc_priv);
void spu_free_debugfs(struct spu_private *iproc_priv);
void spu_free_debugfs_stats(struct spu_private *iproc_priv);
int spu_dma_bufs_alloc(struct iproc_reqctx_s *rctx);
void format_value_ccm(unsigned int val, u8 *buf, u8 len);
#endif /* _UTIL_H */
