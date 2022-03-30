/*
<:copyright-BRCM:2016:GPL/GPL:spu

   Copyright (c) 2016 Broadcom
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

#include "spu.h"

extern int flow_debug_logging;
extern int packet_debug_logging;
extern int debug_logging_sleep;

//#define DEBUG

/* SPU_TEST_RAW_PERF enables the standalone performance testing capability.
 * Only SPU2 equipped SoCs support standalone performance testing. The driver
 * will compile properly even if this flag is defined in SPU1 devices but those
 * testing related debugfs will not be exposed */
//#define SPU_TEST_RAW_PERF

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

void __dump_sg(struct scatterlist *sg, unsigned int skip, unsigned int len);

#define dump_sg(sg, skip, len)     __dump_sg(sg, skip, len)

#else /* !DEBUG_ON */

#define flow_log(...) do {} while (0)
#define flow_dump(msg, var, var_len) do {} while (0)
#define packet_log(...) do {} while (0)
#define packet_dump(msg, var, var_len) do {} while (0)

#define dump_sg(sg, skip, len) do {} while (0)

#endif /* DEBUG_ON */

#ifdef SPU_TEST_RAW_PERF
struct spu_test_vector_t {
	char *name;
	uint8_t inbound;
	uint8_t *enc_key;
	uint8_t *iv;
	uint8_t *hash_key;
	uint8_t *assoc;
	uint8_t *src;
	uint8_t *result;
	uint32_t enc_key_len;
	uint32_t iv_len;
	uint32_t hash_key_len;
	uint32_t src_size;
	uint32_t digest_size;

	enum spu_cipher_alg calg;
	enum spu_cipher_mode cmode;
	enum spu_cipher_type ctype;
	enum hash_alg halg;
	enum hash_mode hmode;
	enum hash_type htype;
	enum aead_type atype;
};
#endif

int spu_sg_at_offset(struct scatterlist *sg, unsigned int skip,
		     struct scatterlist **sge, unsigned int *sge_offset);

/* Copy sg data, from skip, length len, to dest */
void sg_copy_part_to_buf(struct scatterlist *src, u8 *dest,
			 unsigned int len, unsigned int skip);
/* Copy src into scatterlist from offset, length len */
void sg_copy_part_from_buf(struct scatterlist *dest, u8 *src,
			   unsigned int len, unsigned int skip);

int spu_sg_count(struct scatterlist *sg_list, unsigned int skip, int nbytes);
u32 spu_msg_sg_add(struct scatterlist **to_sg,
		   struct scatterlist **from_sg, u32 *skip,
		   u8 from_nents, u32 tot_len);

void add_to_ctr(u8 *ctr_pos, unsigned int increment);

/* produce a message digest from data of length n bytes */
int do_shash(unsigned char *name, unsigned char *result,
	     const u8 *data1, unsigned int data1_len,
	     const u8 *data2, unsigned int data2_len,
	     const u8 *key, unsigned int key_len);

char *spu_alg_name(enum spu_cipher_alg alg, enum spu_cipher_mode mode);

void spu_setup_debugfs(void);
void spu_free_debugfs(void);
void format_value_ccm(unsigned int val, u8 *buf, u8 len);

#endif
