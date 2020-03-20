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

#ifndef _CIPHER_H
#define _CIPHER_H

#include <linux/atomic.h>
#include <linux/mailbox_client.h>
#include <crypto/aes.h>
#include <crypto/sha.h>
#include <crypto/internal/hash.h>
#include <linux/netdevice.h>

#include "spu.h"

//#define SPU_DEBUG_STATS 

#define ARC4_MIN_KEY_SIZE   1
#define ARC4_MAX_KEY_SIZE   256
#define ARC4_BLOCK_SIZE     1
#define ARC4_STATE_SIZE     4

#define CCM_AES_IV_SIZE    16
#define GCM_AES_IV_SIZE    12

#define GCM_ESP_IV_SIZE     8
#define CCM_ESP_IV_SIZE     8
#define RFC4543_ICV_SIZE   16


/* size of salt value for AES-GCM-ESP and AES-CCM-ESP */
#define GCM_ESP_SALT_SIZE   4
#define CCM_ESP_SALT_SIZE   3
#define MAX_SALT_SIZE       GCM_ESP_SALT_SIZE
#define GCM_ESP_SALT_OFFSET 0
#define CCM_ESP_SALT_OFFSET 1

#define GCM_ESP_DIGESTSIZE 16

#define CCM_ESP_L_VALUE     4

#define MAX_KEY_SIZE	ARC4_MAX_KEY_SIZE
#define MAX_IV_SIZE	(AES_BLOCK_SIZE + MAX_SALT_SIZE)
#define MAX_DIGEST_SIZE	SHA512_DIGEST_SIZE
#define MAX_BLOCK_SIZE	SHA512_BLOCK_SIZE
#define MAX_ASSOC_SIZE	512
#define MAX_AUTH_KEY_SIZE	MAX_DIGEST_SIZE

/* Maximum number of bytes from a non-final hash request that can
 * be deferred until more data is available.
 */
#define HASH_CARRY_MAX  2048

struct brcm_message {
	struct scatterlist *src;
	struct scatterlist *dst;
	int error;
	void (*rx_callback)(struct brcm_message *msg);
	struct list_head list;
};

/* op_counts[] indexes */
enum op_type {
	SPU_OP_CIPHER,
	SPU_OP_HASH,
	SPU_OP_HMAC,
	SPU_OP_AEAD,
	SPU_OP_NUM
};

enum spu_stream_type {
	SPU_STREAM_US,
	SPU_STREAM_DS,
	SPU_STREAM_MAX
};

struct cipher_op {
	enum spu_cipher_alg alg;
	enum spu_cipher_mode mode;
};

struct auth_op {
	enum hash_alg alg;
	enum hash_mode mode;
};

struct iproc_alg_s {
	u32 type;
	union {
		struct crypto_alg crypto;
		struct ahash_alg hash;
	} alg;
	struct cipher_op cipher_info;
	struct auth_op auth_info;
	bool auth_first;
	bool dtls_hmac;
	int max_payload;
};

/* Context for the current SPU request message. A set of buffers
 * dynamically allocated for a single SPU request. Allocated at
 * cra init time. The set of buffers needed depends on the type
 * of SPU and the type of request.
 */
struct iproc_pdc_ctx {
	u16 spu_req_hdr_len;
	u8 *bcm_spu_req_hdr;	/* SPU request message header */
	u8 *spu_resp_hdr;	/* SPU response message header */

	u8 *spu_resp_assoc_data;/* SPU response data to be discarded */
	u8 *datapad;             /* SPU response pad for GCM or CCM data */
	u8 *digest;		/* SPU response msg digest */
	u8 *supdt;		/* SPU RC4 SUPDT field */
	u8 *spu_req_aad_pad;    /* SPU request msg padding for GCM AAD */
	u8 *spu_req_pad;	/* SPU request message padding */
	u8 *tx_stat;            /* SPU request message STATUS field */
	u8 *rx_stat_pad;	/* SPU response message STATUS field padding */
	u8 *rx_stat;            /* SPU response message STATUS field */
};

struct iproc_ctx_s {

#if defined(CONFIG_BLOG)
	struct list_head     entry;
	unsigned int         ipsaddr;
	unsigned int         ipdaddr;
	unsigned int         spi;
	enum spu_stream_type stream;
#endif

	u8 chan_idx; /* mailbox channel used to submit requests */

	unsigned int digestsize;
	unsigned int blocksize;

	struct iproc_alg_s *alg;

	struct cipher_op cipher;
	enum spu_cipher_type cipher_type;

	struct auth_op auth;
	bool auth_first;

	u8 enckey[MAX_KEY_SIZE + ARC4_STATE_SIZE];
	unsigned int enckeylen;

	u8 authkey[MAX_AUTH_KEY_SIZE];
	unsigned int authkeylen;

	bool is_esp;
	bool is_rfc4543;

	/*** variables below this line are less frequently used ***/

	u8 iv[MAX_IV_SIZE];
	u8 salt[MAX_SALT_SIZE];
	unsigned int salt_len;
	unsigned int salt_offset;

	unsigned max_payload;

	struct crypto_aead *fallback_cipher;

	/* auth_type is determined during processing of request */

	u8 ipad[MAX_BLOCK_SIZE];
	u8 opad[MAX_BLOCK_SIZE];

	u8 bcm_spu_req_hdr[SPU_HEADER_ALLOC_LEN];

	/* Length of SPU request header */
	u16 spu_req_hdr_len;

};

#define MAX_STATIC_FRAGS 8
#define REQ_DATA_SIZE (ALIGN(SPU_HEADER_ALLOC_LEN, 4) + \
                       ALIGN(SPU_RESP_HDR_LEN, 4) + \
                       ALIGN(MAX_ASSOC_SIZE + MAX_IV_SIZE, 4) + \
                       ALIGN(AES_BLOCK_SIZE, 4) + \
                       ALIGN(MAX_DIGEST_SIZE, 4) + \
                       ALIGN(SPU_SUPDT_LEN, 4) + \
                       ALIGN(AES_BLOCK_SIZE, 4) + \
                       ALIGN(SPU_PAD_LEN_MAX, 4) + \
                       ALIGN(SPU_STATUS_LEN, 4) + \
                       ALIGN(SPU_STAT_PAD_MAX, 4) + \
                       ALIGN(SPU_STATUS_LEN, 4) )

struct iproc_reqctx_s {
	/* general context */
	struct crypto_async_request *parent;
	struct iproc_ctx_s *ctx;

#if defined(CONFIG_BLOG)
	pNBuff_t          pNBuf;
	struct sec_path  *sp;
	struct dst_entry *dst;
#endif

	/* total todo, rx'd, and sent for this request */
	unsigned total_todo;
	unsigned total_received;	/* only valid for ablkcipher */
	unsigned total_sent;

	unsigned hmac_offset;

	/* scatterlist entry and offset to start of data for next chunk */
	struct scatterlist *src_sg;
	int src_nents;		/* Number of src entries with data */
	u32 src_skip;           /* bytes of entry already used */

	/* Same for destination */
	struct scatterlist *dst_sg;
	int dst_nents;		/* Number of dst entries with data */
	u32 dst_skip;           /* bytes of entry already used */

	/* scatterlist entry for association */
	struct scatterlist *assoc_sg;
	unsigned int assoc_len;

	/* message used to send this request to PDC driver */
	struct scatterlist msg_src[MAX_STATIC_FRAGS];
	struct scatterlist msg_dst[MAX_STATIC_FRAGS];
	struct brcm_message mb_mssg;

	struct iproc_pdc_ctx pctx;
	unsigned char pctx_data[REQ_DATA_SIZE];

	/* cipher context */
	bool is_encrypt;

	/* CBC mode: IV.  CTR mode: counter.  Else empty. Used as a DMA
	 * buffer for AEAD requests. So allocate as DMAable memory.
	 */
	u8 iv_ctr[MAX_IV_SIZE];
	/* block_size if either an IV or CTR is present, else 0 */
	unsigned iv_ctr_len;

	/*** variables below this line are less frequently used ***/

	/* aead iv for GCM-GMAC (RFC 4543) */
	u8 aead_iv[MAX_IV_SIZE];
	unsigned aead_iv_len;

	/* Hash requests can be of any size, whether initial, update, or final.
	 * A non-final request must be submitted to the SPU as an integral
	 * number of blocks. This may leave data at the end of the request
	 * that is not a full block. We could submit this remainder as its
	 * on small SPU request message, but doing so would be inefficient.
	 * So, we write the remainder to this hash_carry buffer and hold it
	 * until the next request arrives. The carry data is then submitted
	 * at the beginning of the data in the next SPU msg. hash_carry_len
	 * is the number of bytes currently in hash_carry. These fields are
	 * only used for ahash requests.
	 */
	u8 hash_carry[HASH_CARRY_MAX];
	unsigned hash_carry_len;
	unsigned is_final;	/* is this the final for the hash op? */

	/* hmac context */
	bool is_sw_hmac;

	struct aead_request subreq;

	bool bd_suppress;	/* suppress BD field in SPU response? */
};

struct spu_private {

#if defined(CONFIG_BLOG)
	rwlock_t                ctxListLock[SPU_STREAM_MAX];
	struct list_head        ctxList[SPU_STREAM_MAX];
	struct net_device      *spu_dev_ds;
	struct net_device      *spu_dev_us;
	struct net_device_ops   spu_dev_ops_us;
	struct net_device_ops   spu_dev_ops_ds;
	struct header_ops       spu_dev_header_ops;
#endif

	struct platform_device *pdev;

	/* The number of SPUs on this platform */
	u8 num_spu;

	struct dentry *debugfs_dir;
	struct dentry *debugfs_stats;
	struct file_operations debugfs_fo;

	/* Number of request bytes processed and result bytes returned */
	atomic_long_t bytes_in;
	atomic_long_t bytes_out;

	/* Number of message send failures */
	atomic_t mb_send_fail;

	/* Number of ICV check failures for AEAD messages */
	atomic_t bad_icv;

	/* Number of aead operations requiring fallback  */
	atomic_t aead_fallback_cnt;

#ifdef SPU_DEBUG_STATS 
	/* Number of operations of each type */
	atomic_t op_counts[SPU_OP_NUM];

	atomic_t session_count;	/* number of streams active */
	atomic_t stream_count;	/* monotonic counter for streamID's */

	/* Number of calls to setkey() for each operation type */
	atomic_t setkey_cnt[SPU_OP_NUM];

#endif
};

int handle_aead_req(struct iproc_reqctx_s *rctx);
void handle_aead_resp(struct iproc_reqctx_s *rctx);

#endif /* _CIPHER_H */
