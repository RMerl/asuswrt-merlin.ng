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
//**************************************************************************
// File Name  : spu.h
//
// Description: 
//               
//**************************************************************************
#ifndef __SPU_H__
#define __SPU_H__

#include <linux/device.h>
#include <linux/cdev.h>
#include <crypto/aead.h>

//#define SPU_PACKET_TRACE 1
//#define SPU_KEY_TRACE 1

extern struct spu_info *spuinfo;

//#define SPU_DEBUG 1

#ifdef SPU_DEBUG
#define SPU_TRACE(x)        printk x
#else
#define SPU_TRACE(x)
#endif

#define SPU_NUM_DEVICES 1
#define SPU_DEVICE_NAME "spu"

#define SPU_CRA_PRIORITY         3000

#define SPU_MAX_KEY_SIZE         32 /* AES256*/
#define SPU_MAX_AUTH_KEY_SIZE    SHA256_DIGEST_SIZE 
#define SPU_MAX_IV_LENGTH        16 /* max of AES_BLOCK_SIZE, DES3_EDE_BLOCK_SIZE */
#define SPU_MAX_ICV_LENGTH       12

#define SPU_MAX_PAYLOAD          ((1 << 14) - 1)

#define SPU_NUM_DESCR            128
#define SPU_NUM_TRANS_REQ        64 /*128*/
#define SPU_TRANS_REQ_BASE_IDX   0x1000

#define SPU_RX_BUDGET            32

#define SPU_DESC_ALIGN           8
#define SPU_ALIGN(addr)          (((unsigned int)addr + SPU_DESC_ALIGN - 1) & ~(SPU_DESC_ALIGN - 1))

#define BCM_DESC_ENCR_ALG_NONE      0x00000001
#define BCM_DESC_ENCR_ALG_NULL      0x00000002
#define BCM_DESC_ENCR_ALG_DES       0x00000004
#define BCM_DESC_ENCR_ALG_3DES      0x00000008
#define BCM_DESC_ENCR_ALG_AES       0x00000010

#define BCM_DESC_AUTH_ALG_NONE      0x00001000
#define BCM_DESC_AUTH_ALG_SHA1      0x00002000
#define BCM_DESC_AUTH_ALG_MD5       0x00004000
#define BCM_DESC_AUTH_ALG_SHA256    0x00008000

#define BCM_DESC_ENCR_KEYLEN_SHIFT  24
#define BCM_DESC_ENCR_KEYLEN_MASK   0xFF
#define BCM_DESC_AUTH_KEYLEN_SHIFT  16
#define BCM_DESC_AUTH_KEYLEN_MASK   0xFF

#define SPU_DIRECTION_INVALID       0xFF
#define SPU_DIRECTION_DS            0
#define SPU_DIRECTION_US            1

#define SPU_FALLBACK_METHOD_NONE   0
#define SPU_FALLBACK_METHOD_SKB    1
#define SPU_FALLBACK_METHOD_CIPHER 2

struct spu_alg_template
{
    struct crypto_alg alg;
    unsigned int      descAlg;
};

struct spu_crypto_alg
{
    struct list_head   entry;
    struct crypto_alg  cryptoAlg;
    unsigned int       descAlg;
};

struct spu_ctx
{
    struct spu_desc    *pDescr;
#if defined(CONFIG_BLOG)
    unsigned int        ipsaddr;
    unsigned int        ipdaddr;
    unsigned int        spi;
    struct list_head    entry;
#endif
    uint8_t             direction;
    uint8_t             update;
    uint8_t             next_hdr;
    uint32_t            descAlg;
    uint8_t             auth_key[SPU_MAX_AUTH_KEY_SIZE];
    uint8_t             encrypt_key[SPU_MAX_KEY_SIZE];
    uint8_t             decrypt_key[SPU_MAX_KEY_SIZE];
    struct crypto_aead *fallback_cipher;
};

struct spu_trans_req
{
    struct list_head   entry;
    unsigned int       index;
    void              *context;
    void             (*callback) (struct spu_trans_req *trans_req);
    struct spu_ctx    *pSpuCtx;
    unsigned int       err;
    struct sk_buff    *pAllocSkb;
    unsigned char     *giv;
    RecycleFuncP       recycle_hook;
    unsigned int       recycle_context;
    unsigned int       recycle_flags;
    uint32_t           ivsize;
    pNBuff_t           pNBuf;
    struct sec_path   *sp;
    struct dst_entry  *dst;
    struct net_device *dev;
};

struct spu_info
{
    struct list_head          algList;
    struct list_head          transReqList;
    struct list_head          descList;
    spinlock_t                spuListLock;
    spinlock_t                spuRxLock;
    struct mutex              spuIoctlMutex;
    void                     *descrBase;
    void                     *alignedDescrBase;
    void                     *transReqBase;
    struct task_struct       *rx_thread;
    wait_queue_head_t         rx_thread_wqh;
    int                       rx_work_avail;
    struct file_operations    spu_file_ops;
    SPU_STAT_PARMS            stats;
    int                       major;
    struct class             *spu_class;
    struct cdev               cdev;
    struct device            *device;
#if defined(CONFIG_BLOG)
    struct list_head          ctxList;
    struct net_device        *spu_dev_ds;
    struct net_device        *spu_dev_us;
    struct net_device_ops     spu_dev_ops_us;
    struct net_device_ops     spu_dev_ops_ds;
    struct header_ops         spu_dev_header_ops;
#endif
};

struct spu_trans_req *spu_alloc_trans_req(unsigned int cryptoflags);
void spu_free_trans_req(struct spu_trans_req *pTransReq);

#endif /* __SPU_H__ */
