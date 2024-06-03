/*
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
/****************************************************************************
 * Broadcom virtual flash block IO RPC interface.
 *
 * Author: Tim Ross <tim.ross@broadcom.com>
 *****************************************************************************/
#ifndef _VFBIO_RPC_H_
#define _VFBIO_RPC_H_

#ifdef __KERNEL__    /* Linux kernel */
#include <itc_rpc.h>
#endif

#define VFBIO_LUN_MAX        (1 << 8)
#define VFBIO_LUN_INFO_NAME_MAX    16

enum vfbio_func_idx {
    VFBIO_FUNC_ALLOC = 0,
    VFBIO_FUNC_FREE,
    VFBIO_FUNC_LUN_ID,
    VFBIO_FUNC_LUN_INFO,
    VFBIO_FUNC_READ,
    VFBIO_FUNC_WRITE,
    VFBIO_FUNC_DISCARD,
    VFBIO_FUNC_FLUSH,
    VFBIO_FUNC_SG_READ,
    VFBIO_FUNC_SG_WRITE,
    VFBIO_FUNC_ASYNC_DISCARD,
    VFBIO_FUNC_ASYNC_FLUSH,
    VFBIO_FUNC_ASYNC_COMPLETE,
    VFBIO_FUNC_DEVICE_HEALTH,
    VFBIO_FUNC_FINISH_FIRST_BOOT,
    VFBIO_FUNC_LVM_LUN_CREATE,
    VFBIO_FUNC_LVM_LUN_DELETE,
    VFBIO_FUNC_LVM_LUN_RESIZE,
    VFBIO_FUNC_LVM_LUN_RENAME,
    VFBIO_FUNC_LVM_LUN_CHMOD,
    VFBIO_FUNC_LVM_DEFRAGMENT,
    VFBIO_FUNC_LVM_DEFRAGMENT_INFO,
    VFBIO_FUNC_LVM_DEVICE_INFO,
    VFBIO_FUNC_MAX
};

static inline u8 vfbio_msg_get_retcode(rpc_msg *msg)
{
    return ((msg->data[0] >> 24) & 0xff);
}
static inline void vfbio_msg_set_retcode(rpc_msg *msg, u8 v)
{
    msg->data[0] = (msg->data[0] & ~(0xff << 24)) | (v << 24);
}

static inline u8 vfbio_msg_get_lun(rpc_msg *msg)
{
    return (msg->data[0] & 0xff);
}
static inline void vfbio_msg_set_lun(rpc_msg *msg, u8 v)
{
    msg->data[0] = (msg->data[0] & ~0xff) | v;
}
static inline void vfbio_msg_set_begin(rpc_msg *msg, bool v)
{
    msg->data[0] = (msg->data[0] & ~0x01) | v;
}

/*
 * vfbio_open, vfbio_close
 * request & reply encoding
 * W1: |     31    | 30..8 | 7..0 |
 *     | Exclusive | RSVD  |  LUN |
 *     |(open only)|       |      |
 *
 * W2: | 31..0 |
 *     |  RSVD |
 *
 * W3: | 31..0 |
 *     |  RSVD |
 *
 * Exclusive:    Must be 1
 * RSVD:    Reserved
 * LUN:        Logical unit #
 */
static inline u8 vfbio_msg_get_exclusive(rpc_msg *msg)
{
    return (msg->data[0] >> 31) & 0x1;
}
static inline void vfbio_msg_set_exclusive(rpc_msg *msg, u8 v)
{
    msg->data[0] = (msg->data[0] & ~(0x1 << 31)) | (v << 31);
}

/*
 * vfbio_lun_info
 * request encoding
 * W1: |       31..24        | 23..8 | 7..0 |
 *     | LUN_INFO_BUF[39:32] | RSVD  |  LUN |
 *
 * W2: |         31..0       |
 *     |  LUN_INFO_BUF[31:0] |
 *
 * W3: | 31..0 |
 *     |  RSVD |
 *
 * reply encoding
 * W1: | 31..24 | 23..8  | 7..0 |
 *     |   RC   |  RSVD  |  LUN |
 *
 * W2: | 31..0  |
 *     |  RSVD  |
 *
 * W3: | 31..0 |
 *     |  RSVD |
 *
 * LUN_INFO_BUF[39..32]
 *     Upper 8 bits of 40-bit reply buffer physical address
 * LUN_INFO_BUF[31..0]
 *     Lower 32 bits of 40-bit reply buffer physical address
 * RSVD:    Reserved
 * LUN:        Logical unit #
 */
struct vfbio_lun_info {
    /*
     * W0: |  31..8 |  7..0  |
     *     |  RSVD  | BLK_SZ |
     *
     * W1: | 31..0  |
     *     | N_BLKS |
     *
     * W2..W5: | 31..24  | 23..16  | 15..8   |  7..0   |
     *         | NAME[3] | NAME[2] | NAME[1] | NAME[0] |
     */
    /*
     * blk_sz:
     * 0 = 512B
     * 1 = 1KB
     * 2 = 2KB
     * 3 = 4KB
     * 4..7 = RSVD
     */
    u8 blk_sz;
    u8 flags;
#define VFBIO_LUN_INFO_FLAG_READ_ONLY  0x01
#define VFBIO_LUN_INFO_FLAG_DYNAMIC    0x02
    u8 rsvd1[2];
    u32 n_blks;
    u8 name[VFBIO_LUN_INFO_NAME_MAX]; /* NULL-terminated ASCII string */
};
static inline u8 vfbio_lun_info_get_blk_sz(struct vfbio_lun_info *info)
{
    return info->blk_sz & 0xf;
}
static inline u8 vfbio_lun_info_set_blk_sz(struct vfbio_lun_info *info, u8 v)
{
    return info->blk_sz = (info->blk_sz & ~0xf) | v;
}

/*
 * vfbio_read, vfbio_write
 * request encoding
 * W1: |       31..24     | 23..8  | 7..0 |
 *     | DMA ADDR[39..32] | N_BLKS |  LUN |
 *
 * W2: |      31..0      |
 *     | DMA ADDR[31..0] |
 *
 * W3: | 31..0 |
 *     |  BLK  |
 *
 * DMA ADDR[39..32]:    Upper 8 bits of 40-bit DMA address
 * N_BLKS:        # of blocks to read/write
 * LUN:            Logical unit #
 * DMA ADDR[31..0]:    Lower 32 bits of 40-bit DMA address
 * BLK:            Starting block in LUN
 *
 * reply encoding same as request except
 * N_BLKS:    # of blocks actually read/written
 */
static inline u32 vfbio_msg_get_n_blks(rpc_msg *msg)
{
    return (msg->data[0] >> 8) & 0xffff;
}
static inline void vfbio_msg_set_n_blks(rpc_msg *msg, u32 v)
{
    msg->data[0] = (msg->data[0] & ~(0xffff << 8)) |
        ((v & 0xffff) << 8);
}
static inline u64 vfbio_msg_get_addr(rpc_msg *msg)
{
    return ((u64)(msg->data[0] & (0xff << 24)) << 8) | msg->data[1];
}
static inline void vfbio_msg_set_addr(rpc_msg *msg, u64 v)
{
    msg->data[0] = (msg->data[0] & ~(0xff << 24)) |
        (u32)((v >> 8) & (0xff << 24));
    msg->data[1] = (u32)(v & 0xffffffff);
}
static inline u32 vfbio_msg_get_blk(rpc_msg *msg)
{
    return msg->data[2] ;
}
static inline void vfbio_msg_set_blk(rpc_msg *msg, u32 v)
{
    msg->data[2]  = v;
}

/*
 * vfbio_discard
 * request encoding
 * W1: | 31..24 | 23..8  | 7..0 |
 *     | RSVD   | N_BLKS |  LUN |
 *
 * W2: | 31..0 |
 *     | RSVD  |
 *
 * W3: | 31..0 |
 *     |  BLK  |
 *
 * RSVD:        Reserved
 * N_BLKS:        # of blocks to discard
 * LUN:            Logical unit #
 * BLK:            Starting block in LUN
 *
 * reply encoding same as request except
 * N_BLKS:    # of blocks actually discarded
 */

/*
 * vfbio_sg_read, vfbio_sg_write
 * message encoding
 * W1: | 31..16 |       15..8      | 7..0 |
 *     | RSVD   | DMA ADDR[39..32] |  LUN |
 *
 * W2: |      31..0      |
 *     | DMA ADDR[31..0] |
 *
 * W3: | 31..0 |
 *     |  RSVD |
 *
 * RSVD:        Reserved
 * DMA ADDR[39..32]:    Upper 8 bits of 40-bit DMA address
 * LUN:            Logical unit #
 * DMA ADDR[31..0]:    Lower 32 bits of 40-bit DMA address
 */
/*
 * See vfbio_msg_get_addr() and vfbio_msg_set_addr() above for
 * manipulation of msg.
 */
struct vfbio_sg_segment {
    /*
     * Each SG segment encoding
     * W0: |      31..24      |  23..8  | 7..0 |
     *     | DMA ADDR[39..32] | N_BLKS  | RSVD |
     *
     * W1: |       31..0     |
     *     | DMA ADDR[31..0] |
     *
     * DMA ADDR[39..32]:    Upper 8 bits of 40-bit DMA address
     * N_BLKS:        # of blocks to transfer
     * RSVD:        Reserved
     * DMA ADDR[31..0]:    Lower 32 bits of 40-bit DMA address
     */
    u32 w[2];
};
#define VFBIO_SG_SEGMENT_MAX_BLKS    (1 << 16)

/*
 * It is imperative that this struct size remains a multiple of the
 * cache line size. This buffer is passed to the SMC so we must
 * perform cache operations on it. To keep these operations from
 * becoming complex we must ensure the buffer starts and ends on cache
 * line boundaries.
 */
#define VFBIO_SG_LIST_SEGMENTS    30    /* struct size = 256B */
struct vfbio_hw_sgl {
    /*
     * SG list header encoding
     * W0: | 31..16 | 7..0  |
     *     |  RSVD  | N_SEG |
     *
     * W1: | 31..0 |
     *     |  BLK  |
     *
     * W2: | 31..0 |
     *     |  TAG  |
     *
     * W2: | 31..0 |
     *     | RSVD  |
     *
     * RSVD:    Reserved
     * N_SEG:    # of SG segments
     * BLK:        starting block #
     * TAG:        32-bit unique transaction ID
     */
    u32 hdr[4];
    struct vfbio_sg_segment seg[VFBIO_SG_LIST_SEGMENTS];
};
static inline u8 vfbio_sgl_get_n_segs(struct vfbio_hw_sgl *sgl)
{
    return sgl->hdr[0] & 0xff;
}
static inline void vfbio_sgl_set_n_segs(struct vfbio_hw_sgl *sgl, u8 v)
{
    sgl->hdr[0] = (sgl->hdr[0] & ~0xff) | v;
}
static inline u32 vfbio_sgl_get_blk(struct vfbio_hw_sgl *sgl)
{
    return (sgl->hdr[1]);
}
static inline void vfbio_sgl_set_blk(struct vfbio_hw_sgl *sgl, u32 v)
{
    sgl->hdr[1] = v;
}
static inline u32 vfbio_sgl_get_tag(struct vfbio_hw_sgl *sgl)
{
    return (sgl->hdr[2]);
}
static inline void vfbio_sgl_set_tag(struct vfbio_hw_sgl *sgl, u32 v)
{
    sgl->hdr[2] = v;
}
static inline u16 vfbio_sgl_seg_get_n_blks(struct vfbio_hw_sgl *sgl, u8 seg)
{
    return (sgl->seg[seg].w[0] >> 8) & 0xffff;
}
static inline void vfbio_sgl_seg_set_n_blks(struct vfbio_hw_sgl *sgl, u8 seg,
                        u16 v)
{
    sgl->seg[seg].w[0] = (sgl->seg[seg].w[0] & ~(0xffff << 8)) | (v << 8);
}
static inline u64 vfbio_sgl_seg_get_addr(struct vfbio_hw_sgl *sgl, u8 seg)
{
    return (((u64)(sgl->seg[seg].w[0] & (0xff << 24))) << 8) | sgl->seg[seg].w[1];
}
static inline void vfbio_sgl_seg_set_addr(struct vfbio_hw_sgl *sgl, u8 seg, u64 v)
{
    sgl->seg[seg].w[0] = (sgl->seg[seg].w[0] & ~(0xff << 24)) |
        (u32)((v >> 8) & (0xff << 24));
    sgl->seg[seg].w[1] = (u32)(v & 0xffffffff);
}

/*
 * vfbio_async_discard
 * request encoding
 * W1: | 31..24 | 23..8  | 7..0 |
 *     | RSVD   | N_BLKS |  LUN |
 *
 * W2: | 31..0 |
 *     | TAG   |
 *
 * W3: | 31..0 |
 *     |  BLK  |
 *
 * RSVD:        Reserved
 * N_BLKS:        # of blocks to discard
 * LUN:            Logical unit #
 * TAG:            32-bit tag for use only by client
 * BLK:            Starting block in LUN
 *
 */
static inline u32 vfbio_msg_get_tag(rpc_msg *msg)
{
    return msg->data[1];
}
static inline void vfbio_msg_set_tag(rpc_msg *msg, u32 v)
{
    msg->data[1] = v;
}

/*
 * vfbio_get_device_health
 * request encoding
 * W1: | 31..24 | 23..1  |  0  |
 *     |   RC   |  RSVD  | END |
 *
 * W2: | 31..24 | 23..16 | 15..8 | 7..0  |
 *     |  RSVD  |  EST-B | EST-A | P-EOL |
 *
 * W3: | 31..0  |
 *     |  RSVD  |
 *
 * RSVD:        Reserved
 * EST-B:        Device Life Time estimate(EST-B)
 * EST-A:        Device Life Time estimate(EST-A)
 * P-EOL:        Pre EOL Life Time Estimation Values
 *
 */
static inline bool vfbio_msg_get_dh_end(rpc_msg *msg)
{
    return (msg->data[0]) & 0x1;
}
static inline bool vfbio_msg_get_dh_pre_eol(rpc_msg *msg)
{
    return (msg->data[1]) & 0xFF;
}
static inline bool vfbio_msg_get_dh_est_a(rpc_msg *msg)
{
    return (msg->data[1] >> 8) & 0xFF;
}
static inline bool vfbio_msg_get_dh_est_b(rpc_msg *msg)
{
    return (msg->data[1] >> 16) & 0xFF;
}

/*
 * VFBIO LVM support
 */

#define VFBIO_GET_FIELD(word, field) \
    (((word) & (field##_MASK)) >> (field##_SHIFT))
#define VFBIO_SET_FIELD(word, field, val) \
    ((word) = (((word) & ~(field##_MASK)) | \
    (((val) << (field##_SHIFT)) & (field##_MASK))))

/*
 * Message access functions
 */

/*
 * vfbio_lun_create
 * request encoding
 * W1: | 31..24                   | 23..0 |
 *     | REQUEST_BUF_ADDR[39:32]  | RERV  |
 *
 * W2: | 31..0                 |
 *     | REQUEST_BUF_ADDR[31:0]|
 *
 * W3: | 31..0 |
 *     |  RSVD |
 *
 * reply encoding
 * W1: | 31..24 | 23..8 | 7..0 |
 *     | RC     | RSVD  |  LUN |
 *
 * W2: | 31..0 |
 *     |  RSVD |
 *
 * W3: | 31..0 |
 *     |  RSVD |
 *
 * REQUEST_BUF : VFBIO_LUN_CREATE request buffer
 * RSVD:      Reserved
 * LUN:       Logical unit #
 * RC         vfbio service return code
 *
 */
#define LUN_LVM_REQ_ADDR_0_MASK      (0xFF000000) /* W1[39:32] */
#define LUN_LVM_REQ_ADDR_0_SHIFT     (24)
#define LUN_LVM_REQ_ADDR_1_MASK      (0xFFFFFFFF) /* W2[31:0] */
#define LUN_LVM_REQ_ADDR_1_SHIFT     (0)
#define LUN_LVM_LUN_MASK             (0x000000FF) /* W1[7:0] */
#define LUN_LVM_LUN_SHIFT            (0)
#define LUN_LVM_RC_MASK              (0xFF000000) /* W1[31:24] */
#define LUN_LVM_RC_SHIFT             (24)

struct vfbio_lun_create_request
{
    char lun_name[VFBIO_LUN_INFO_NAME_MAX];  /* Unique lun name */
    uint32_t lun_size;  /* Volume size in 4KB blocks (in Little Endian format) */
};

/*
 * vfbio_lun_delete request encoding
 * W1: | 31..8 | 7..0 |
 *     | RSVD  |  LUN |
 *
 * W2: | 31..0 |
 *     |  RSVD |
 *
 * W3: | 31..0 |
 *     |  RSVD |
 *
 * reply encoding
 * W1: | 31..24 | 23..0 |
 *     | RC     | RSVD  |
 *
 * W2: | 31..0 |
 *     |  RSVD |
 *
 * W3: | 31..0 |
 *     |  RSVD |
 *
 * LUN:       Logical unit #
 * RC         vfbio service return code
 */

/*
 * vfbio_lun_resize request encoding
 * W1: | 31..8 | 7..0 |
 *     | RSVD  |  LUN |
 *
 * W2: | 31..0 |
 *     | BLKS  |
 *
 * W3: | 31..0 |
 *     |  RSVD |
 *
 * reply encoding
 * W1: | 31..24 | 23..0 |
 *     | RC     | RSVD  |
 *
 * W2: | 31..0 |
 *     |  RSVD |
 *
 * W3: | 31..0 |
 *     |  RSVD |
 *
 * LUN:       Logical unit #
 * BLKS       new size in 4KB byte blocks
 * RC         vfbio service return code
 */
#define LUN_LVM_SIZE_BLKS_MASK       (0xFFFFFFFF) /* W1[10:8] */
#define LUN_LVM_SIZE_BLKS_SHIFT      (0)

/*
 * vfbio_lun_rename
 * request encoding
 * W1: | 31..24                   | 23..8 | 7..0 |
 *     | REQUEST_BUF_ADDR[39:32]  | RERV  | LUN  |
 *
 * W2: | 31..0                 |
 *     | REQUEST_BUF_ADDR[31:0]|
 *
 * W3: | 31..0 |
 *     |  RSVD |
 *
 * reply encoding
 * W1: | 31..24 | 23..0 |
 *     | RC     | RSVD  |
 *
 * W2: | 31..0 |
 *     |  RSVD |
 *
 * W3: | 31..0 |
 *     |  RSVD |
 *
 * REQUEST_BUF : VFBIO_LUN_RENAME request buffer
 * RSVD:      Reserved
 * LUN:       Logical unit #
 * RC         vfbio service return code
 *
 */
struct vfbio_idx_name {
    uint8_t lun_idx;
    uint8_t reserved[3];
    char lun_name[VFBIO_LUN_INFO_NAME_MAX];  /* 0-terminated unique LUN name */
};

struct vfbio_lun_rename_request {
    uint8_t num_luns;/* Number of names in the structure */
    uint8_t reserved[3];
    /* num_names substructures below */
    struct vfbio_idx_name idx_name[0];
};

/*
 * vfbio_lun_set_access_type request encoding
 * W1: | 31..9 | 8    | 7..0 |
 *     | RSVD  | 0=RW | LUN  |
 *     |       | 1=RO |
 *
 * W2: | 31..0 |
 *     |  RSVD |
 *
 * W3: | 31..0 |
 *     |  RSVD |
 *
 * reply encoding
 * W1: | 31..24 | 23..0 |
 *     | RC     | RSVD  |
 *
 * W2: | 31..0 |
 *     |  RSVD |
 *
 * W3: | 31..0 |
 *     |  RSVD |
 *
 * LUN:       Logical unit #
 * RC         vfbio service return code
 */
#define LUN_LVM_ACCESS_READ_ONLY_MASK      (0x00000100) /* W1[8:8] */
#define LUN_LVM_ACCESS_READ_ONLY_SHIFT     (8)

/*
 * vfbio_lun_device_info
 * request encoding
 * W1: | 31..0 |
 *     | RSVD  |
 *
 * W2: | 31..0                 |
 *     |  RSVD |
 *
 * W3: | 31..0 |
 *     |  RSVD |
 *
 * reply encoding
 * W1: | 31..24 | 23..0 |
 *     | RC     | RSVD  |
 *
 * W2: | 31..0       |
 *     | TOTAL_PAGES |
 *
 * W3: | 31..0       |
 *     | FREE_PAGES  |
 *
 * RSVD:      Reserved
 * RC         vfbio service return code
 *
 */

static inline uint8_t vfbio_lvm_msg_get_rc(rpc_msg *msg)
{
    uint8_t v = VFBIO_GET_FIELD(msg->data[0], LUN_LVM_RC);
    return (v);
}

static inline void vfbio_lvm_msg_set_rc(rpc_msg *msg, uint8_t v)
{
    VFBIO_SET_FIELD(msg->data[0], LUN_LVM_RC, v);
}

static inline uint8_t vfbio_lvm_msg_get_lun(rpc_msg *msg)
{
    uint8_t v = VFBIO_GET_FIELD(msg->data[0], LUN_LVM_LUN);
    return (v);
}

static inline void vfbio_lvm_msg_set_lun(rpc_msg *msg, uint8_t v)
{
    VFBIO_SET_FIELD(msg->data[0], LUN_LVM_LUN, v);
}

static inline uint32_t vfbio_lvm_msg_get_size(rpc_msg *msg)
{
    uint32_t v = VFBIO_GET_FIELD(msg->data[1], LUN_LVM_SIZE_BLKS);
    return (v);
}

static inline void vfbio_lvm_msg_set_size(rpc_msg *msg, uint32_t v)
{
    VFBIO_SET_FIELD(msg->data[1], LUN_LVM_SIZE_BLKS, v);
}

static inline uint8_t vfbio_lvm_msg_get_req_buf_addr_hi(rpc_msg *msg)
{
    uint8_t v = VFBIO_GET_FIELD(msg->data[0], LUN_LVM_REQ_ADDR_0);
    return (v);
}

static inline void vfbio_lvm_msg_set_req_buf_addr_hi(rpc_msg *msg, uint8_t v)
{
    VFBIO_SET_FIELD(msg->data[0], LUN_LVM_REQ_ADDR_0, v);
}

static inline uint32_t vfbio_lvm_msg_get_req_buf_addr_lo(rpc_msg *msg)
{
    uint32_t v = VFBIO_GET_FIELD(msg->data[1], LUN_LVM_REQ_ADDR_1);
    return (v);
}

static inline void vfbio_lvm_msg_set_req_buf_addr_lo(rpc_msg *msg, uint32_t v)
{
    VFBIO_SET_FIELD(msg->data[1], LUN_LVM_REQ_ADDR_1, v);
}

static inline void vfbio_lvm_msg_set_req_buf_addr(rpc_msg *msg, u64 v)
{
    vfbio_lvm_msg_set_req_buf_addr_hi(msg, (uint8_t)((v >> 32) & 0xff));
    vfbio_lvm_msg_set_req_buf_addr_lo(msg, (uint32_t)(v & 0xffffffff));
}

static inline bool vfbio_lvm_msg_chmod_get_mode(rpc_msg *msg)
{
    bool v = (bool)VFBIO_GET_FIELD(msg->data[0], LUN_LVM_ACCESS_READ_ONLY);
    return (v);
}

static inline void vfbio_lvm_msg_chmod_set_mode(rpc_msg *msg, bool v)
{
    VFBIO_SET_FIELD(msg->data[0], LUN_LVM_ACCESS_READ_ONLY, v);
}

static inline uint32_t vfbio_lvm_msg_get_device_info_total_blocks(rpc_msg *msg)
{
    return msg->data[1];
}

static inline uint32_t vfbio_lvm_msg_get_device_info_free_blocks(rpc_msg *msg)
{
    return msg->data[2];
}

#endif
