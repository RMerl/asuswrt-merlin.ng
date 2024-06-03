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
 * vFlash block IO driver
 *
 * Author: Tim Ross <tim.ross@broadcom.com>
 *****************************************************************************/

#include <asm/cacheflush.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/dma-mapping.h>
#include <linux/sched/sysctl.h>
#include <linux/cpumask.h>
#include <linux/preempt.h>
#include <linux/version.h>
#include "vfbio_priv.h"
#include "vfbio_proc.h"
#include "blk.h"

#define MODULE_NAME    "brcm-vfbio"
#define MODULE_VER    "1.0"

#define VFBIO_MINORS            16
#define VFBIO_MAX_PENDING_ASYNC_OPS    16
#define VFBIO_MAX_DISCARD_BLOCKS    1024
#define VFBIO_MAX_SEGMENT_SIZE        (16*1024*1024)
#define VFBIO_IO_TIMEOUT_DEFAULT    120 /* sec */
#define VFBIO_RETRY_DELAY_DEFAULT    200 /* msec */
#define VFBIO_CRASH_RPC_TIMEOUT        2000 /* msec */
#define SECTOR_SHIFT        9
#define SECTOR_SIZE        (1 << SECTOR_SHIFT)

/* limit CHS to size of geometry struct members as well as fdisk limits */
#define MAX_SECTORS \
    min((1 << (sizeof_field(struct hd_geometry, sectors) << 3)) - 1, 63)
#define MAX_HEADS \
    min((1 << (sizeof_field(struct hd_geometry, heads) << 3)) - 1, 255)
#define MAX_CYLINDERS \
    min((1 << (sizeof_field(struct hd_geometry, cylinders) << 3)) - 1, 1048576)

struct vfbio_sgls_entry {
    /*
     * Since hw_sgl is passed to the SMC and it requires 8-byte
     * alignement of buffers, the hw_sgl has to be the first member.
     * Since the containing struct is > 8 bytes it will be aligned
     * on a boundary >= it's size by the slab allocator and thus hw_sgl
     * will too.
     */
    struct vfbio_hw_sgl    hw_sgl;
    struct list_head    list;
    dma_addr_t        hw_sgl_dma_addr;
    int            n_hw_sgl_segs;
    struct scatterlist     sw_sgl[VFBIO_SG_LIST_SEGMENTS];
    int            n_sw_sgl_segs;
};

#ifdef CONFIG_BCM_VFBIO_MQ
struct vfbio_req_pdu {
    struct vfbio_sgls_entry    *sgls_entry;
};
#endif

static int vfbio_major;

static struct vfbio_device *vfdevs[VFBIO_LUN_MAX];
struct list_head vfdevs_list;
static spinlock_t vfdevs_list_lock;

static struct list_head vfbio_sgls;
static spinlock_t vfbio_sgls_lock;

static struct vfbio_hw_sgl *vfbio_crash_hw_sgl;

static atomic_t pending_ops = ATOMIC_INIT(0);

static unsigned io_timeout;
ssize_t io_timeout_show(struct device_driver *driver, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%u\n", io_timeout);
}
ssize_t io_timeout_store(struct device_driver *driver, const char *buf,
               size_t count)
{
    int i;
    struct vfbio_device *vfbio;
    i = sscanf(buf, "%u", &io_timeout);
    if (count > 0 && i == 1) {
        for (i = 0; i < VFBIO_LUN_MAX; i++) {
            vfbio = vfdevs[i];
            if (!vfbio)
                continue;
            blk_queue_rq_timeout(vfbio->q, io_timeout * HZ);
        }
        return count;
    }
    return -EINVAL;
}
DRIVER_ATTR_RW(io_timeout);

static unsigned retry_delay;
ssize_t retry_delay_show(struct device_driver *driver, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%u\n", retry_delay);
}
ssize_t retry_delay_store(struct device_driver *driver, const char *buf,
               size_t count)
{
    int i;
    struct vfbio_device *vfbio;
    i = sscanf(buf, "%u", &retry_delay);
    if (count > 0 && i == 1) {
        for (i = 0; i < VFBIO_LUN_MAX; i++) {
            vfbio = vfdevs[i];
            if (!vfbio)
                continue;
            blk_queue_rq_timeout(vfbio->q, retry_delay * HZ);
        }
        return count;
    }
    return -EINVAL;
}
DRIVER_ATTR_RW(retry_delay);

static char *req_op_str(struct request *req)
{
    switch (req_op(req)) {
    case REQ_OP_READ:
        return "read";
    case REQ_OP_WRITE:
        return "write";
    case REQ_OP_DISCARD:
        return "discard";
    case REQ_OP_FLUSH:
        return "flush";
    case REQ_OP_SECURE_ERASE:
        return "secure erase";
    case REQ_OP_WRITE_SAME:
        return "write same";
    default:
        return "unknown";
    }
}

static char *req_op_str_short(struct request *req, bool upper)
{
    switch (req_op(req)) {
    case REQ_OP_READ:
        return upper ? "R" : "r";
    case REQ_OP_WRITE:
        return upper ? "W" : "w";
    case REQ_OP_DISCARD:
        return upper ? "D" : "d";
    case REQ_OP_FLUSH:
        return upper ? "F" : "f";
    case REQ_OP_SECURE_ERASE:
        return upper ? "SE" : "se";
    case REQ_OP_WRITE_SAME:
        return upper ? "WS" : "ws";
    default:
        return upper ? "U" : "u";
    }
}

static void vfbio_dump_buf(struct device *dev, u8 *buf, unsigned int len)
{
    unsigned int offset = 0;
    unsigned int line_len;
    u8 line[10 + 32 * 3 + 2 + 32 + 1];
    bool repeat;
    bool print_repeat = true;

    while (len) {
        line_len = len < 16 ? len : 16;
        repeat = offset >= 16 ?
            memcmp(buf, buf - 16, line_len) == 0 : false;
        if (!repeat || len == line_len) {
            snprintf(line, 11, "%8x: ", offset);
            hex_dump_to_buffer(buf, line_len, 16, 1,
                       &line[10], sizeof(line)-10, true);
            pr_debug("%s\n", line);
            print_repeat = true;
        } else if (print_repeat) {
            pr_debug("*\n");
            print_repeat = false;
        }
        len -= line_len;
        buf += line_len;
        offset += line_len;
    }
}

int vfbio_lun_request_timeout_msg(struct vfbio_device *vfbio, rpc_msg *msg)
{
    int status = 0;
    struct device *dev = &vfbio->pdev->dev;

    BUG_ON(!vfbio);
    BUG_ON(!msg);
    vfbio_msg_set_lun(msg, vfbio->lun);
    /*
     * Prevent block IO layer timeout on sync req by setting RPC
     * timeout 1s shorter.
     */
    status = rpc_send_request_timeout(vfbio->tunnel, msg, io_timeout - 1);
    if (unlikely(status)) {
        dev_err(dev, RED("rpc_send_request failure (%d)\n"),
            status);
        rpc_dump_msg(msg);
        return status;
    }
    status = (int8_t)vfbio_msg_get_retcode(msg);

    return status;
}

static int vfbio_async_discard(struct vfbio_device *vfbio, struct request *req)
{
    int status = 0;
    struct device *dev = &vfbio->pdev->dev;
    rpc_msg msg;
    sector_t sect = blk_rq_pos(req);
    unsigned int sects = blk_rq_sectors(req);
    u32 blk = sect >> (vfbio->blk_sz_shift - SECTOR_SHIFT);
    u32 n_blks = sects >> (vfbio->blk_sz_shift - SECTOR_SHIFT);
    u32 sect_per_blk_mask = (vfbio->blk_sz >> SECTOR_SHIFT) - 1;

    /* BUG if start or length is not multiple of flash block size */
    BUG_ON(sect & sect_per_blk_mask);
    BUG_ON(sects & sect_per_blk_mask);

    rpc_msg_init(&msg, RPC_SERVICE_VFBIO, VFBIO_FUNC_ASYNC_DISCARD, 0,
             0, 0, 0);
    vfbio_msg_set_lun(&msg, vfbio->lun);
    vfbio_msg_set_blk(&msg, blk);
    vfbio_msg_set_n_blks(&msg, n_blks);
#ifdef CONFIG_BCM_VFBIO_MQ
    vfbio_msg_set_tag(&msg, blk_mq_unique_tag(req));
#else
    vfbio_msg_set_tag(&msg, (u32)req->tag);
#endif
    pr_debug(KERN_CONT GRN("D%d:%d#%d|"), vfbio->lun, n_blks,
         atomic_read(&pending_ops));
    status = rpc_send_message(vfbio->tunnel, &msg, false);
    if (unlikely(status)) {
        dev_err(dev, RED("rpc_send_message failure (%d)\n"),
            status);
        rpc_dump_msg(&msg);
    }
    return status;
}

static int vfbio_async_flush(struct vfbio_device *vfbio, struct request *req)
{
    int status = 0;
    struct device *dev = &vfbio->pdev->dev;
    rpc_msg msg;

    rpc_msg_init(&msg, RPC_SERVICE_VFBIO, VFBIO_FUNC_ASYNC_FLUSH, 0,
             0, 0, 0);
    vfbio_msg_set_lun(&msg, vfbio->lun);
#ifdef CONFIG_BCM_VFBIO_MQ
    vfbio_msg_set_tag(&msg, blk_mq_unique_tag(req));
#else
    vfbio_msg_set_tag(&msg, (u32)req->tag);
#endif
    pr_debug(KERN_CONT GRN("F%d#%d|"), vfbio->lun,
         atomic_read(&pending_ops));
    status = rpc_send_message(vfbio->tunnel, &msg, false);
    if (unlikely(status)) {
        dev_err(dev, RED("rpc_send_message failure (%d)\n"),
            status);
        rpc_dump_msg(&msg);
    }
    return status;
}

static int vfbio_async_complete(int tunnel, rpc_msg *msg)
{
    int status = 0;
    struct vfbio_device *vfbio;
    struct device *dev;
    u8 lun;
    u32 tag;
    struct request *req;
    u8 rc;
    sector_t blk;
    u32 n_blks, req_n_blks;
    struct vfbio_sgls_entry *sgls_entry;
    enum req_opf op;
    int dma_dir;
#ifdef CONFIG_BCM_VFBIO_MQ
    u16 hwq;
    struct vfbio_req_pdu *pdu;
#endif

    pr_debug("-->\n");
    lun = vfbio_msg_get_lun(msg);
    if (!vfdevs[lun]) {
        pr_err(RED("unexpected completion msg for closed LUN %d\n"),
               lun);
        goto done;
    }
    vfbio = vfdevs[lun];
    dev = &vfbio->pdev->dev;
    tag = vfbio_msg_get_tag(msg);
#ifdef CONFIG_BCM_VFBIO_MQ
    hwq = blk_mq_unique_tag_to_hwq(tag);
    if (hwq >= vfbio->tset.nr_hw_queues) {
        dev_err(dev, RED("tag 0x%08x for LUN %d "
            "(%s) hwq is out of bounds\n"), tag, lun,
            vfbio->disk->disk_name);
        goto done;
    }
    req = blk_mq_tag_to_rq(vfbio->tset.tags[hwq],
                   blk_mq_unique_tag_to_tag(tag));
#else
    req = blk_queue_find_tag(vfbio->q, tag);
#endif
    if (!req) {
        dev_err(dev, RED("tag 0x%08x doesn't exist for LUN %d "
            "(%s)\n"), tag, lun, vfbio->disk->disk_name);
        goto done;
    }
#ifdef CONFIG_BCM_VFBIO_MQ
    if (!blk_mq_request_started(req)) {
        dev_err(dev, RED("request with tag 0x%08x for LUN %d "
            "(%s) wasn't started\n"), tag, lun,
            vfbio->disk->disk_name);
        goto done;
    }
#endif
    op = req_op(req);
    rc = vfbio_msg_get_retcode(msg);

    if (op == REQ_OP_FLUSH || op == REQ_OP_DISCARD)
        goto flush_discard;

    dma_dir = op == REQ_OP_READ ? DMA_FROM_DEVICE : DMA_TO_DEVICE;
    n_blks = vfbio_msg_get_n_blks(msg);
    blk = blk_rq_pos(req) >> (vfbio->blk_sz_shift - SECTOR_SHIFT);
    req_n_blks = blk_rq_sectors(req) >> (vfbio->blk_sz_shift - SECTOR_SHIFT);
    if (rc || n_blks != req_n_blks) {
        dev_err(dev, RED("%s request failure @ block %lld on "
            "LUN %d (%s), rc: %d, n_blks: %d\n"),
            req_op_str(req), (u64)blk, lun, vfbio->disk->disk_name,
            (s8)rc, n_blks);
        status = -EIO;
    }
    dev_dbg(dev, "%s tag 0x%08x, %d blks @ 0x%08llx\n",
        op == REQ_OP_READ ? "completed sg read op" :
        "completed sg write op", tag, n_blks, (u64)blk);
#ifdef CONFIG_BCM_VFBIO_MQ
    pdu = (struct vfbio_req_pdu *)blk_mq_rq_to_pdu(req);
    sgls_entry = pdu->sgls_entry;
#else
    sgls_entry = req->special;
#endif
    if (sgls_entry) {
        dma_unmap_single(dev, sgls_entry->hw_sgl_dma_addr,
                 sizeof(struct vfbio_hw_sgl), DMA_TO_DEVICE);
        vfbio_dump_buf(dev, (u8 *)&sgls_entry->hw_sgl,
                 sizeof(struct vfbio_hw_sgl));
        dma_unmap_sg(dev, sgls_entry->sw_sgl,
                 sgls_entry->n_hw_sgl_segs, dma_dir);
        spin_lock_irq(&vfbio_sgls_lock);
        list_add_tail(&sgls_entry->list, &vfbio_sgls);
        spin_unlock_irq(&vfbio_sgls_lock);
    } else {
        dev_err(dev, RED("Error: NULL private data ptr in request\n"));
        status = -EIO;
    }

flush_discard:
    atomic_dec(&pending_ops);
    pr_debug(KERN_CONT GRN("%s%d#%d|"), req_op_str_short(req, false),
         vfbio->lun, atomic_read(&pending_ops));
#ifdef CONFIG_BCM_VFBIO_MQ
    blk_mq_end_request(req, errno_to_blk_status(status));
#else
    /* q must be unlocked when calling blk_end_request */
    blk_end_request_all(req, errno_to_blk_status(status));
#endif

    if (atomic_dec_and_test(&vfbio->pending_ops) &&
        vfbio->waiting_on_sync) {
        dev_dbg(dev, "Restarting LUN %d request queue\n",
            vfbio->lun);
        pr_debug(KERN_CONT GRN("s%d|"), vfbio->lun);
        vfbio->waiting_on_sync = false;
#ifdef CONFIG_BCM_VFBIO_MQ
        blk_mq_start_stopped_hw_queues(vfbio->q, true);
#else
        spin_lock_irq(vfbio->q->queue_lock);
        blk_start_queue_async(vfbio->q);
        spin_unlock_irq(vfbio->q->queue_lock);
#endif
    }

done:
    pr_debug("<--\n");
    return status;
}

static rpc_function vfbio_services_tbl[VFBIO_FUNC_MAX] =
{
    [VFBIO_FUNC_ASYNC_COMPLETE] = { vfbio_async_complete,    0 },
};

static int vfbio_sg_rw(struct vfbio_device *vfbio, struct request *req)
{
    int status = 0;
    struct device *dev = &vfbio->pdev->dev;
    sector_t sect = blk_rq_pos(req);
    u32 sects = blk_rq_sectors(req);
    u32 blk = sect >> (vfbio->blk_sz_shift - SECTOR_SHIFT);
    u32 n_blks;
    u32 sect_per_blk_mask = (vfbio->blk_sz >> SECTOR_SHIFT) - 1;
    struct vfbio_sgls_entry *sgls_entry;
    struct vfbio_hw_sgl *hw_sgl;
    struct scatterlist *sw_sgl;
    struct scatterlist *sw_sgl_seg;
    int n_sw_sgl_segs, n_hw_sgl_segs;
    u32 len;
    int i;
    rpc_msg msg;
    enum req_opf op = req_op(req);
    int dma_dir = op == REQ_OP_READ ? DMA_FROM_DEVICE : DMA_TO_DEVICE;
    dma_addr_t dma_addr;
#ifdef CONFIG_BCM_VFBIO_MQ
    struct vfbio_req_pdu *pdu;
#endif

    pr_debug("-->\n");
    /* BUG if start or len is not multiple of flash block size */
    BUG_ON(sect & sect_per_blk_mask);
    BUG_ON(sects & sect_per_blk_mask);

    dev_dbg(dev, "sg %s op: tag 0x%08x\n", op == REQ_OP_READ ? "read" :
        "write", req->tag);
    dev_dbg(dev,  "%d sectors @ sect %lld\n", sects, (u64)sect);
    dev_dbg(dev,  "%d blocks @ block %d, block size: %d\n",
        blk_rq_sectors(req) >> (vfbio->blk_sz_shift - SECTOR_SHIFT),
        blk, vfbio->blk_sz);

    spin_lock_irq(&vfbio_sgls_lock);
    if (list_empty(&vfbio_sgls)) {
        dev_dbg(dev, "Max operations pending.\n");
        spin_unlock_irq(&vfbio_sgls_lock);
        status = -ENOMEM;
        goto done;
    }
    sgls_entry = list_first_entry(&vfbio_sgls,
                      struct vfbio_sgls_entry, list);
    list_del(&sgls_entry->list);
    spin_unlock_irq(&vfbio_sgls_lock);
#ifdef CONFIG_BCM_VFBIO_MQ
    pdu = (struct vfbio_req_pdu *)blk_mq_rq_to_pdu(req);
    pdu->sgls_entry = sgls_entry;
#else
    req->special = sgls_entry;
#endif
    hw_sgl = &sgls_entry->hw_sgl;
    sw_sgl = sgls_entry->sw_sgl;

    n_sw_sgl_segs = blk_rq_map_sg(req->q, req, sw_sgl);
    sgls_entry->n_sw_sgl_segs = n_sw_sgl_segs;
    n_hw_sgl_segs = dma_map_sg(&vfbio->pdev->dev, sw_sgl, n_sw_sgl_segs, dma_dir);
    sgls_entry->n_hw_sgl_segs = n_hw_sgl_segs;
    if (unlikely(!n_hw_sgl_segs)) {
        dev_err(dev, RED("Failure mapping page.\n"));
        status = -EIO;
        goto err_return_entry_to_list;
    }
    vfbio_sgl_set_n_segs(hw_sgl, n_hw_sgl_segs);
    vfbio_sgl_set_blk(hw_sgl, blk);
#ifdef CONFIG_BCM_VFBIO_MQ
    vfbio_sgl_set_tag(hw_sgl, blk_mq_unique_tag(req));
#else
    vfbio_sgl_set_tag(hw_sgl, (u32)req->tag);
#endif
    for_each_sg(sw_sgl, sw_sgl_seg, n_hw_sgl_segs, i) {
        len = sg_dma_len(sw_sgl_seg);
        BUG_ON(len & (vfbio->blk_sz - 1));
        n_blks = len >> vfbio->blk_sz_shift;
        BUG_ON(!n_blks);
        vfbio_sgl_seg_set_n_blks(hw_sgl, i, n_blks);
        dev_dbg(dev, "sectors: %d, blocks: %d\n",
            len >> SECTOR_SHIFT, n_blks);
        dma_addr = sg_dma_address(sw_sgl_seg);
        BUG_ON(dma_addr & 0xfff); /* should be 4k aligned page buffer */
        vfbio_sgl_seg_set_addr(hw_sgl, i, (u64)dma_addr);
        dev_dbg(dev, CYN("dma addr: 0x%llx\n"), dma_addr);
    }
    vfbio_dump_buf(dev, (u8 *)hw_sgl, sizeof(struct vfbio_hw_sgl));

    if (op == REQ_OP_READ)
        rpc_msg_init(&msg, RPC_SERVICE_VFBIO, VFBIO_FUNC_SG_READ, 0,
                 0, 0, 0);
    else {
        rpc_msg_init(&msg, RPC_SERVICE_VFBIO, VFBIO_FUNC_SG_WRITE, 0,
                 0, 0, 0);
    }
    vfbio_msg_set_lun(&msg, vfbio->lun);
    dma_addr = dma_map_single(dev, hw_sgl, sizeof(struct vfbio_hw_sgl),
                  DMA_TO_DEVICE);
    if (dma_mapping_error(dev, dma_addr)) {
        dev_err(dev, RED("failure mapping hw_sgl\n"));
        goto err_unmap_sgl;
    }
    dev_dbg(dev, MAG("dma addr: 0x%llx\n"), dma_addr);
    sgls_entry->hw_sgl_dma_addr = dma_addr;
    BUG_ON(dma_addr & 0x7);    /* buffers must be 8-byte aligned */
    vfbio_msg_set_addr(&msg, (u64)dma_addr);
    status = rpc_send_message(vfbio->tunnel, &msg, false);
    if (unlikely(status)) {
        dev_err(dev, RED("rpc_send_message failure (%d)\n"),
            status);
        rpc_dump_msg(&msg);
        goto err_unmap_cmd_buf;
    }
    pr_debug(KERN_CONT GRN("%s%d:%d@%d#%d|"), req_op_str_short(req, true),
         vfbio->lun, blk_rq_sectors(req) >> (vfbio->blk_sz_shift - SECTOR_SHIFT),
         blk, atomic_read(&pending_ops));
    goto done;

err_unmap_cmd_buf:
    dma_unmap_single(dev, sgls_entry->hw_sgl_dma_addr,
             sizeof(struct vfbio_hw_sgl), DMA_TO_DEVICE);
err_unmap_sgl:
    dma_unmap_sg(dev, sgls_entry->sw_sgl,
             sgls_entry->n_hw_sgl_segs, dma_dir);
err_return_entry_to_list:
    spin_lock_irq(&vfbio_sgls_lock);
    list_add_tail(&sgls_entry->list, &vfbio_sgls);
    spin_unlock_irq(&vfbio_sgls_lock);
done:
    pr_debug("<--\n");
    return status;
}

#ifdef CONFIG_BCM_VFBIO_MQ
static enum blk_eh_timer_return vfbio_req_timed_out(struct request *req,
                            bool reserved)
#else
static enum blk_eh_timer_return vfbio_req_timed_out(struct request *req)
#endif
{
    struct vfbio_device *vfbio = req->q->queuedata;
    struct device *dev = &vfbio->pdev->dev;
    sector_t blk;
    u32 n_blks;
    enum req_opf op;
    int dma_dir;
    struct vfbio_sgls_entry *sgls_entry;
#ifdef CONFIG_BCM_VFBIO_MQ
    struct vfbio_req_pdu *pdu;
#else
    /* q is locked on entry. */
#endif

    op = req_op(req);
    if (op == REQ_OP_FLUSH || op == REQ_OP_DISCARD)
        goto flush_discard;

    dma_dir = op == REQ_OP_READ ? DMA_FROM_DEVICE : DMA_TO_DEVICE;
    blk = blk_rq_pos(req) >> (vfbio->blk_sz_shift - SECTOR_SHIFT);
    n_blks = blk_rq_sectors(req) >> (vfbio->blk_sz_shift - SECTOR_SHIFT);
    dev_err(dev, RED("%s request timeout @ block %lld on LUN %d "
        "(%s), tag: 0x%08x, n_blks: %d\n"), req_op_str(req),
        (u64)blk, vfbio->lun, vfbio->disk->disk_name, req->tag,
        n_blks);
#ifdef CONFIG_BCM_VFBIO_MQ
    pdu = (struct vfbio_req_pdu *)blk_mq_rq_to_pdu(req);
    sgls_entry = pdu->sgls_entry;
#else
    sgls_entry = req->special;
#endif
    dma_unmap_single(dev, sgls_entry->hw_sgl_dma_addr,
             sizeof(struct vfbio_hw_sgl), DMA_TO_DEVICE);
    vfbio_dump_buf(dev, (u8 *)&sgls_entry->hw_sgl,
             sizeof(struct vfbio_hw_sgl));
    dma_unmap_sg(dev, sgls_entry->sw_sgl, sgls_entry->n_hw_sgl_segs,
             dma_dir);
    spin_lock_irq(&vfbio_sgls_lock);
    list_add_tail(&sgls_entry->list, &vfbio_sgls);
    spin_unlock_irq(&vfbio_sgls_lock);

flush_discard:
    atomic_dec(&pending_ops);
    pr_debug(KERN_CONT RED("T%s%d#%d|"), req_op_str_short(req, false),
         vfbio->lun, atomic_read(&pending_ops));
#ifdef CONFIG_BCM_VFBIO_MQ
    __blk_mq_end_request(req, errno_to_blk_status(-ETIME));
#else
    __blk_end_request_all(req, errno_to_blk_status(-ETIME));
#endif

    if (atomic_dec_and_test(&vfbio->pending_ops) &&
        vfbio->waiting_on_sync) {
        dev_dbg(dev, "Restarting LUN %d request queue\n",
            vfbio->lun);
        pr_debug(KERN_CONT GRN("s%d|"), vfbio->lun);
        vfbio->waiting_on_sync = false;
#ifdef CONFIG_BCM_VFBIO_MQ
        blk_mq_start_stopped_hw_queues(vfbio->q, true);
    }

    return BLK_EH_DONE;
#else
        blk_start_queue_async(vfbio->q);
    }

    /* Leave with q locked as it was on entry. */
    return BLK_EH_RESET_TIMER;
#endif

}

#ifdef CONFIG_BCM_VFBIO_MQ
static blk_status_t vfbio_request(struct blk_mq_hw_ctx *hctx,
              const struct blk_mq_queue_data *bd)
{
    int status = 0;
    struct vfbio_device *vfbio = hctx->queue->queuedata;
    struct device *dev = &vfbio->pdev->dev;
    struct request *req = bd->rq;
    struct request_queue *q = vfbio->q;
    unsigned int op;
    sector_t sect, cap;
    unsigned int sects;
    bool sync = false;
/*     bool delay_and_retry = false; */
    struct vfbio_req_pdu *pdu;

    pr_debug("-->\n");
    pdu = (struct vfbio_req_pdu *)blk_mq_rq_to_pdu(req);
    memset(pdu, 0, sizeof(*pdu));

    dev_dbg(dev, "LUN: %d, op: %s, cmd_flags: 0x%x\n",
        vfbio->lun, req_op_str(req), req->cmd_flags);
    blk_mq_start_request(req);
    op = req_op(req);
    sect = blk_rq_pos(req);
    sects = blk_rq_sectors(req);
    if (op != REQ_OP_FLUSH) {
        cap = get_capacity(req->rq_disk);
        if (sect + sects > cap) {
            dev_err(dev, RED("ignoring request of %d 512B "
                "sectors @ sector %lld as it is beyond "
                " device limit of %lld sectors\n"),
                sects, (u64)sect, (u64)cap);
            status = BLK_STS_IOERR;
            goto done;
        }
    }

    atomic_inc(&pending_ops);
    atomic_inc(&vfbio->pending_ops);
    switch (op) {
    case REQ_OP_FLUSH:
        /*
         * We tell the block layer that our device
         * has a cache on it and it does not natively handle
         * the REQ_FUA operation. This causes the block layer
         * to translate all ops with the REQ_PREFLUSH flag set
         * into a sequence of REQ_OP_FLUSH followed by the
         * R/W request. It also translates a write with the
         * REQ_FUA flag into a REQ_OP_WRITE followed by a
         * REQ_OP_FLUSH.
         */
        dev_dbg(dev, "Flush\n");
        status = vfbio_async_flush(vfbio, req);
        break;
    case REQ_OP_DISCARD:
        dev_dbg(dev, "Discard %d sectors @ %lld\n",
            sects, (u64)sect);
        status = vfbio_async_discard(vfbio, req);
        break;
    case REQ_OP_READ:
    case REQ_OP_WRITE:
        dev_dbg(dev, "%s %d sectors @ %lld\n",
            op == REQ_OP_READ ? "Read" : "Write",
            sects, (u64)sect);
        status = vfbio_sg_rw(vfbio, req);
        break;
    default:
        dev_warn(dev, YLW("Unhandled op: %s\n"),
             req_op_str(req));
        pr_debug(KERN_CONT YLW("UO%d|"), vfbio->lun);
        break;
    }

    switch (status) {
    /* success */
    case 0:
        status = BLK_STS_OK;
        break;

    /* no sgl entries available */
    case -ENOMEM:
        dev_dbg(dev, YLW("Out of SG list entries.\n"));
        pr_debug(KERN_CONT YLW("OE|"));
        atomic_dec(&pending_ops);
        atomic_dec(&vfbio->pending_ops);
/*         delay_and_retry = true; */
        status = BLK_STS_RESOURCE;
        break;

    /* RPC TX FIFO full */
    case -EAGAIN:
        dev_dbg(dev, YLW("RPC tunnel full.\n"));
        pr_debug(KERN_CONT YLW("TF|"));
        atomic_dec(&pending_ops);
        atomic_dec(&vfbio->pending_ops);
/*         delay_and_retry = true; */
        status = BLK_STS_DEV_RESOURCE;
        break;

    /* RPC tunnel link down */
    case -EBUSY:
        dev_dbg(dev, RED("RPC tunnel link down.\n"));
        pr_debug(KERN_CONT RED("LD|"));
        atomic_dec(&pending_ops);
        atomic_dec(&vfbio->pending_ops);
/*         delay_and_retry = true; */
        status = BLK_STS_TRANSPORT;
        break;

    /* any other error */
    default:
        pr_debug(KERN_CONT RED("ER|"));
        atomic_dec(&pending_ops);
        atomic_dec(&vfbio->pending_ops);
        status = BLK_STS_IOERR;
        __blk_mq_end_request(req, status);
        break;
    }

/*     if (delay_and_retry) {                        */
/*         dev_dbg(dev, "Delay and retry again.\n");       */
/*         pr_debug(KERN_CONT YLW("A|"));                */
/*         blk_mq_requeue_request(req);                */
/*         blk_mq_delay_kick_requeue_list(q, retry_delay); */
/*         goto done;                        */
/*     }                                */

    /*
     * Synchronous requests require it and all previous requests
     * complete before any new requests are started. Since SMC could
     * complete requests out of order we have to stop accepting new
     * requests until all outstanding requests complete for this LUN.
     * Synchronous RPC requests cannot be used as they will block and
     * blocking in the request function is not allowed. Flush
     * requests are also considered synchronous.
     */
    sync = /*req->cmd_flags & REQ_SYNC ||*/ op == REQ_OP_FLUSH;
    if (sync) {
        dev_dbg(dev, "Sync request. Stopping LUN %d "
                 "request queue\n",
            vfbio->lun);
        pr_debug(KERN_CONT GRN("S%d|"), vfbio->lun);
        blk_mq_stop_hw_queues(q);
        vfbio->waiting_on_sync = true;
    }

done:
    /* Check to make sure sync did not finish already. */
    if (vfbio->waiting_on_sync && atomic_read(&vfbio->pending_ops) == 0) {
        pr_debug(KERN_CONT GRN("s%d|"), vfbio->lun);
        vfbio->waiting_on_sync = false;
        blk_mq_start_stopped_hw_queues(q, true);
    }

    return status;
    pr_debug("<--\n");
}

#else

static void vfbio_request(struct request_queue *q)
{
    int status = 0;
    struct vfbio_device *vfbio = q->queuedata;
    struct device *dev = &vfbio->pdev->dev;
    struct request *req;
    unsigned int op;
    sector_t sect, cap;
    unsigned int sects;
    bool sync = false;
    bool delay_and_retry = false;

    pr_debug("-->\n");
    /*
     * q is locked on entry. Need to leave it locked until after
     * request is removed from q.
     */
    req = blk_peek_request(q);
    while (req) {
        req->special = NULL;

        dev_dbg(dev, "LUN: %d, op: %s, cmd_flags: 0x%x\n",
            vfbio->lun, req_op_str(req), req->cmd_flags);
        status = blk_queue_start_tag(q, req);
        if (unlikely(status)) {
            dev_dbg(dev, YLW("Unable to start tagged request.\n"));
            dev_dbg(dev, "Delay and retry again.\n");
            pr_debug(KERN_CONT YLW("US%d|"), vfbio->lun);
            pr_debug(KERN_CONT YLW("A|"));
            blk_delay_queue(q, retry_delay);
            break;
        }
        spin_unlock_irq(q->queue_lock);

        op = req_op(req);
        sect = blk_rq_pos(req);
        sects = blk_rq_sectors(req);
        if (op != REQ_OP_FLUSH) {
            cap = get_capacity(req->rq_disk);
            if (sect + sects > cap) {
                dev_err(dev, RED("ignoring request of %d 512B "
                    "sectors @ sector %lld as it is beyond "
                    " device limit of %lld sectors\n"),
                    sects, (u64)sect, (u64)cap);
                status = -EINVAL;
                break;
            }
        }

        atomic_inc(&pending_ops);
        atomic_inc(&vfbio->pending_ops);
        switch (op) {
        case REQ_OP_FLUSH:
            /*
             * We tell the block layer that our device
             * has a cache on it and it does not natively handle
             * the REQ_FUA operation. This causes the block layer
             * to translate all ops with the REQ_PREFLUSH flag set
             * into a sequence of REQ_OP_FLUSH followed by the
             * R/W request. It also translates a write with the
             * REQ_FUA flag into a REQ_OP_WRITE followed by a
             * REQ_OP_FLUSH.
             */
            dev_dbg(dev, "Flush\n");
            status = vfbio_async_flush(vfbio, req);
            break;
        case REQ_OP_DISCARD:
            dev_dbg(dev, "Discard %d sectors @ %lld\n",
                sects, (u64)sect);
            status = vfbio_async_discard(vfbio, req);
            break;
        case REQ_OP_READ:
        case REQ_OP_WRITE:
            dev_dbg(dev, "%s %d sectors @ %lld\n",
                op == REQ_OP_READ ? "Read" : "Write",
                sects, (u64)sect);
            status = vfbio_sg_rw(vfbio, req);
            break;
        default:
            dev_warn(dev, YLW("Unhandled op: %s\n"),
                 req_op_str(req));
            pr_debug(KERN_CONT YLW("UO%d|"), vfbio->lun);
            break;
        }

        switch (status) {
        /* success */
        case 0:
            break;

        /* no sgl entries available */
        case -ENOMEM:
            dev_dbg(dev, YLW("Out of SG list entries.\n"));
            pr_debug(KERN_CONT YLW("OE|"));
            atomic_dec(&pending_ops);
            atomic_dec(&vfbio->pending_ops);
            delay_and_retry = true;
            break;

        /* RPC TX FIFO full */
        case -EAGAIN:
            dev_dbg(dev, YLW("RPC tunnel full.\n"));
            pr_debug(KERN_CONT YLW("TF|"));
            atomic_dec(&pending_ops);
            atomic_dec(&vfbio->pending_ops);
            delay_and_retry = true;
            break;

        /* RPC tunnel link down */
        case -EBUSY:
            dev_dbg(dev, RED("RPC tunnel link down.\n"));
            pr_debug(KERN_CONT RED("LD|"));
            atomic_dec(&pending_ops);
            atomic_dec(&vfbio->pending_ops);
            delay_and_retry = true;
            break;

        /* any other error */
        default:
            pr_debug(KERN_CONT RED("ER|"));
            atomic_dec(&pending_ops);
            atomic_dec(&vfbio->pending_ops);
            spin_lock_irq(q->queue_lock);
            __blk_end_request_all(req, errno_to_blk_status(status));
            spin_unlock_irq(q->queue_lock);
            break;
        }

        spin_lock_irq(q->queue_lock);

        if (delay_and_retry) {
            dev_dbg(dev, "Delay and retry again.\n");
            pr_debug(KERN_CONT YLW("A|"));
            blk_requeue_request(q, req);
            blk_delay_queue(q, retry_delay);
            break;
        }

        /*
         * Synchronous requests require it and all previous requests
         * complete before any new requests are started. Since SMC could
         * complete requests out of order we have to stop accepting new
         * requests until all outstanding requests complete for this LUN.
         * Synchronous RPC requests cannot be used as they will
         * blocking in the request function is not allowed. Flush
         * requests are also considered synchronous.
         */
        sync = req->cmd_flags & REQ_SYNC || op == REQ_OP_FLUSH;
        if (sync) {
            dev_dbg(dev, "Sync request. Stopping LUN %d "
                     "request queue\n",
                vfbio->lun);
            pr_debug(KERN_CONT GRN("S%d|"), vfbio->lun);
            blk_stop_queue(q);
            vfbio->waiting_on_sync = true;
            break;
        }

        req = blk_peek_request(q);

        /* Leave q locked for return to top of loop. */
    }
    /* Always exit while loop with q locked. */

    /* Check to make sure sync did not finish already. */
    if (vfbio->waiting_on_sync && atomic_read(&vfbio->pending_ops) == 0) {
        pr_debug(KERN_CONT GRN("s%d|"), vfbio->lun);
        vfbio->waiting_on_sync = false;
        blk_start_queue_async(q);
    }

    /* Return with q locked as it was when we entered. */
    pr_debug("<--\n");
}
#endif

#define VFBIO_CRASH_TAG 0x48535243 /* "CRSH" */
/*
 * vfbio_crash_write() must only be called when there is only one CPU
 * running that cannot be preempted. We need to be certain we get
 * to run to completion and the block layer will not send any more requests.
 * Since we don't want any other code to run from here on we can do whatever
 * necessary to prevent that, but we must be sure not to do anything that
 * relies on interrupts or the scheduler.
 *
 * Since we could have up to MAX_PENDING_ASYNC_OPS outstanding for which
 * the SMC will send async RPC response messages we have to be sure we don't
 * allow the incoming RPC DQM to fill up or this could cause undesired SMC
 * behavior.
 *
 */
int vfbio_crash_write(struct block_device *bdev, u32 start_blk, void *buf,
              u32 n_blks)
{
    int status = 0;
    struct vfbio_device *vfbio;
    struct device *dev;
    struct vfbio_hw_sgl *hw_sgl = vfbio_crash_hw_sgl;
    rpc_msg msg;
    dma_addr_t buf_dma_addr, hw_sgl_dma_addr;
    u8 serv, func;
    u32 tag;

    pr_debug("-->\n");

    vfbio = (struct vfbio_device *)bdev->bd_disk->private_data;
    dev = &vfbio->pdev->dev;

    if (num_online_cpus() > 1 || preemptible()) {
        dev_err(dev, RED("%s() called from non-atomic "
                 "context!\n"), __func__);
        status = -EINVAL;
        goto done;
    }

    if (start_blk > vfbio->n_blks) {
        dev_err(dev, RED("start block beyond LUN size!\n"));
        status = -EINVAL;
        goto done;
    }
    if ((start_blk + n_blks) > vfbio->n_blks) {
        status = -EINVAL;
        goto done;
    }

    dev_dbg(dev, "%s start_blk %d n_blks %d\n", vfbio->name,
         start_blk, n_blks);

    /*
     * Poll RPC tunnel tossing undesired messages until the # of
     * pending ops is below the allowable outstanding level.
     */
    while (atomic_read(&pending_ops) >= VFBIO_MAX_PENDING_ASYNC_OPS) {
        status = rpc_receive_message_crash(vfbio->tunnel, &msg,
                           VFBIO_CRASH_RPC_TIMEOUT);
        if (status < 0)
            return -EIO;
        serv = rpc_msg_service(&msg);
        func = rpc_msg_function(&msg);
        if (serv == RPC_SERVICE_VFBIO &&
            func == VFBIO_FUNC_ASYNC_COMPLETE)
            atomic_dec(&pending_ops);
    }

    buf_dma_addr = dma_map_single(dev, buf, n_blks * vfbio->blk_sz,
                  DMA_TO_DEVICE);
    if (dma_mapping_error(dev, buf_dma_addr)) {
        dev_err(dev, RED("failure mapping data buf\n"));
        status = -EFAULT;
        goto done;
    }
    dev_dbg(dev, MAG("buf_dma addr: 0x%llx\n"), buf_dma_addr);
    vfbio_sgl_set_n_segs(hw_sgl, 1);
    vfbio_sgl_set_blk(hw_sgl, start_blk);
    vfbio_sgl_set_tag(hw_sgl, (u32)VFBIO_CRASH_TAG);
    vfbio_sgl_seg_set_n_blks(hw_sgl, 0, n_blks);
    vfbio_sgl_seg_set_addr(hw_sgl, 0, (u64)buf_dma_addr);

    rpc_msg_init(&msg, RPC_SERVICE_VFBIO, VFBIO_FUNC_SG_WRITE, 0,
             0, 0, 0);
    vfbio_msg_set_lun(&msg, vfbio->lun);
    hw_sgl_dma_addr = dma_map_single(dev, hw_sgl,
                sizeof(struct vfbio_hw_sgl),
                DMA_TO_DEVICE);
    if (dma_mapping_error(dev, hw_sgl_dma_addr)) {
        dev_err(dev, RED("failure mapping cmd buf\n"));
        status = -EFAULT;
        goto err_unmap_data_buf;
    }
    dev_dbg(dev, MAG("hw_sgl_dma addr: 0x%llx\n"), hw_sgl_dma_addr);
    BUG_ON(hw_sgl_dma_addr & 0x7);    /* buffers must be 8-byte aligned */
    vfbio_msg_set_addr(&msg, (u64)hw_sgl_dma_addr);
    status = rpc_send_message(vfbio->tunnel, &msg, false);
    if (unlikely(status)) {
        dev_err(dev, RED("rpc_send_message failure (%d)\n"),
            status);
        rpc_dump_msg(&msg);
        goto err_unmap_cmd_buf;
    }

    /*
     * Poll RPC tunnel tossing undesired messages until our write
     * completes.
     */
    for (serv = RPC_MAX_SERVICES, func = VFBIO_FUNC_MAX, tag = 0;
         !(serv == RPC_SERVICE_VFBIO &&
           func == VFBIO_FUNC_ASYNC_COMPLETE &&
           tag == VFBIO_CRASH_TAG);
         serv = rpc_msg_service(&msg),
         func = rpc_msg_function(&msg),
         tag = vfbio_msg_get_tag(&msg)) {
        status = rpc_receive_message_crash(vfbio->tunnel, &msg,
                           VFBIO_CRASH_RPC_TIMEOUT);
        if (status < 0)
            break;
    }

err_unmap_cmd_buf:
    dma_unmap_single(dev, hw_sgl_dma_addr, sizeof(struct vfbio_hw_sgl),
             DMA_TO_DEVICE);
err_unmap_data_buf:
    dma_unmap_single(dev, buf_dma_addr, n_blks * vfbio->blk_sz,
             DMA_TO_DEVICE);
done:
    pr_debug("<--\n");
    return status;
}
EXPORT_SYMBOL(vfbio_crash_write);

static int vfbio_getgeo(struct block_device *bd, struct hd_geometry *geo)
{
    struct vfbio_device *vfbio = bd->bd_disk->private_data;
    sector_t n_512_sects, n_sects;
    u32 sects, hds, cyls;
    u32 rem;
    u64 hds_x_cyls, tmp;

    /*
     * Pick a geometry to equal total capacity. However, we have to pick it
     * so that we still fit within the CHS limits imposed by fdisk.
     */
    n_512_sects = get_capacity(bd->bd_disk);
    n_sects = n_512_sects >> __ffs(vfbio->blk_sz >> SECTOR_SHIFT);
    pr_debug("n_512_sects: %lld, n_sects: %lld\n", (u64)n_512_sects, (u64)n_sects);
    if (vfbio->geo.sectors == 0) {
        for (rem = 1, tmp = n_sects, sects = MAX_SECTORS;
              rem != 0;
              tmp = n_sects, sects--)
            rem = do_div(tmp, sects);
        sects++;
        pr_debug("sects: %d\n", sects);
        do_div(tmp, sects);
        hds_x_cyls = tmp;
        for (rem = 1, hds = MAX_HEADS; rem != 0;
              tmp = hds_x_cyls, hds--)
            rem = do_div(tmp, hds);
        hds++;
        pr_debug("heads: %d\n", hds);
        do_div(tmp, hds);
        cyls = tmp;
        pr_debug("cylinders: %d\n", cyls);
        if (cyls > MAX_CYLINDERS) {
            pr_err(RED("Unable to calculate CHS for LUN %s "
                   "that will make use of all %lld bytes "
                   "available.\n"),
                   bd->bd_disk->disk_name, (u64)n_sects << SECTOR_SHIFT);
            sects = MAX_SECTORS;
            hds = MAX_HEADS;
            tmp = n_sects;
            rem = do_div(tmp, hds * sects);
            cyls = tmp > MAX_CYLINDERS ? MAX_CYLINDERS : tmp;
            pr_err(RED("Using CHS %d : %d : %d resulting in "
                   "%d lost bytes.\n"),
                   cyls, hds, sects, rem << SECTOR_SHIFT);
        } else if (cyls < 32) {
            /*
             * Since LUN is small, swap heads & cylinders so that
             * fdisk can be used to create several partitions.
             */
            tmp = cyls;
            cyls = hds;
            hds = tmp;
        }
        vfbio->geo.sectors = sects;
        vfbio->geo.heads = hds;
        vfbio->geo.cylinders = cyls;
    }

    *geo = vfbio->geo;
    pr_debug("LUN %s geometry (CHS): %d : %d : %d, %lu\n", bd->bd_disk->disk_name,
        geo->cylinders, geo->heads, geo->sectors, geo->start);
    return 0;
}

static const struct block_device_operations vfbio_fops = {
    .owner =    THIS_MODULE,
    .getgeo =    vfbio_getgeo,
};

#ifdef CONFIG_BCM_VFBIO_MQ
static struct blk_mq_ops vfbio_mq_ops = {
    .queue_rq =    vfbio_request,
    .timeout =    vfbio_req_timed_out,
};
#endif

/*
 * Helper functions that are used by the static probe as well as LVM
 */

/* Create vfbio device. */
int vfbio_device_create(struct device *dev, struct vfbio_device *vfbio)
{
    int status = 0;
    struct gendisk *disk = NULL;
    sector_t n_sects;
    u64 n_bytes, n_kb, n_mb, n_gb;
#ifdef CONFIG_BCM_VFBIO_MQ
    struct blk_mq_tag_set *tset;
#endif

    pr_debug("-->\n");
    if (vfbio_device_get_by_name(vfbio->name)) {
        dev_err(dev, RED("Duplicate LUN name %s invalid."),
            vfbio->name);
        goto done;
    }

    atomic_set(&vfbio->pending_ops, 0);
    vfbio->blk_sz_shift = __ffs(vfbio->blk_sz);

#ifdef CONFIG_BCM_VFBIO_MQ
    tset = &vfbio->tset;
    tset->ops = &vfbio_mq_ops;
    tset->nr_hw_queues = 1;
/*     tset->nr_maps = 1; */
    tset->queue_depth = VFBIO_MAX_PENDING_ASYNC_OPS;
    tset->numa_node = NUMA_NO_NODE;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
    tset->flags = BLK_MQ_F_SHOULD_MERGE;
#else
    tset->flags = BLK_MQ_F_SHOULD_MERGE | BLK_MQ_F_SG_MERGE;
#endif
#else
    spin_lock_init(&vfbio->q_lock);
    vfbio->q = blk_init_queue(vfbio_request, &vfbio->q_lock);
    if (!vfbio->q) {
        dev_err(dev, RED("Unable to init block request queue.\n"));
        status = -ENOMEM;
        goto done;
    }
#endif
    /*
     * Have the tags be recycled in a round-robin fashion instead of a
     * FIFO fashion. We do this to make it unlikely that an unexpected
     * completion received from the SMC for a cancelled request (e.g.
     * it timed out) will have a tag that matches a pending request
     * submitted after the cancelled request. If we don't do this we
     * could get confused.
     */
#ifdef CONFIG_BCM_VFBIO_MQ
    tset->flags |= BLK_ALLOC_POLICY_TO_MQ_FLAG(BLK_TAG_ALLOC_RR);
    tset->timeout = io_timeout * HZ;
    tset->cmd_size = sizeof(struct vfbio_req_pdu);
    status = blk_mq_alloc_tag_set(tset);
    if (status) {
        dev_err(dev, RED("Unable to init tag set.\n"));
        goto done;
    }
    vfbio->q = blk_mq_init_queue(tset);
    if (IS_ERR_OR_NULL(vfbio->q)) {
        dev_err(dev, RED("Unable to init block request queue.\n"));
        blk_mq_free_tag_set(tset);
        status = -ENOMEM;
        goto done;
    }

#else
    status = blk_queue_init_tags(vfbio->q, VFBIO_MAX_PENDING_ASYNC_OPS,
                     NULL, BLK_TAG_ALLOC_RR);
    if (status) {
        dev_err(dev, RED("Unable to allocate block q tags.\n"));
        goto err_cleanup_q;
    }
#endif
    blk_queue_write_cache(vfbio->q, true, false);
    vfbio->q->queuedata = vfbio;
    blk_queue_physical_block_size(vfbio->q, (unsigned int)vfbio->blk_sz);
    blk_queue_logical_block_size(vfbio->q, (unsigned short)vfbio->blk_sz);
    blk_queue_io_min(vfbio->q, (unsigned int)vfbio->blk_sz);
    vfbio->q->limits.max_dev_sectors =
        (int)((u64)vfbio->blk_sz * VFBIO_SG_SEGMENT_MAX_BLKS *
         VFBIO_SG_LIST_SEGMENTS) >> SECTOR_SHIFT;
    blk_queue_max_hw_sectors(vfbio->q, vfbio->q->limits.max_dev_sectors);
    blk_queue_max_segments(vfbio->q, VFBIO_SG_LIST_SEGMENTS);
    blk_queue_max_segment_size(vfbio->q,
        VFBIO_SG_SEGMENT_MAX_BLKS * vfbio->blk_sz);
    blk_queue_dma_alignment(vfbio->q, vfbio->blk_sz - 1);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
    blk_queue_bounce_limit(vfbio->q, BLK_BOUNCE_NONE);
    dma_set_mask(dev, BLK_BOUNCE_NONE);
    blk_queue_flag_set(QUEUE_FLAG_DISCARD, vfbio->q);
#else
    blk_queue_bounce_limit(vfbio->q, BLK_BOUNCE_ANY);
    dma_set_mask(dev, BLK_BOUNCE_ANY);
    queue_flag_set_unlocked(QUEUE_FLAG_DISCARD, vfbio->q);
#endif
    vfbio->q->limits.discard_alignment = vfbio->blk_sz;
    vfbio->q->limits.discard_granularity = vfbio->blk_sz;
    blk_queue_max_discard_sectors(vfbio->q,
        VFBIO_MAX_DISCARD_BLOCKS * vfbio->blk_sz / SECTOR_SIZE);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
    blk_queue_flag_set(QUEUE_FLAG_NONROT, vfbio->q);
#else
    queue_flag_set_unlocked(QUEUE_FLAG_NONROT, vfbio->q);
#endif
#ifndef CONFIG_BCM_VFBIO_MQ
    blk_queue_rq_timed_out(vfbio->q, vfbio_req_timed_out);
    blk_queue_rq_timeout(vfbio->q, io_timeout * HZ);
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
    disk = blk_alloc_disk(NUMA_NO_NODE);
    disk->minors = 32;
#else
    disk = alloc_disk(VFBIO_MINORS);
#endif
    if (!disk) {
        dev_err(dev, RED("Unable to allocate gendisk.\n"));
        status = -ENOMEM;
        goto err_cleanup_q;
    }
    vfbio->disk = disk;
    disk->major        = vfbio_major;
    disk->first_minor    = VFBIO_MINORS * vfbio->lun;
    disk->fops        = &vfbio_fops;
    disk->private_data    = vfbio;
    disk->queue        = vfbio->q;
    disk->flags        = GENHD_FL_EXT_DEVT;
    snprintf(disk->disk_name, DISK_NAME_LEN, "flash-%s", vfbio->name);
    disk->disk_name[DISK_NAME_LEN - 1] = '\0';
    n_sects = ((sector_t)vfbio->n_blks * vfbio->blk_sz) >> SECTOR_SHIFT;
    set_capacity(disk, n_sects);
    if (vfbio->read_only)
        set_disk_ro(disk, 1);

    n_bytes = n_sects << SECTOR_SHIFT;
    n_kb = n_bytes >> 10;
    n_mb = n_kb >> 10;
    n_gb = n_mb >> 10;
    pr_info("%-25.25s LUN %3d %3lld%s\n", disk->disk_name, vfbio->lun,
        n_gb ? n_gb : n_mb ? n_mb : n_kb ? n_kb : n_bytes,
        n_gb ? "GB" : n_mb ? "MB" : n_kb ? "KB" : "B");
    pr_debug("    block size:     %d\n", vfbio->blk_sz);
    pr_debug("    blocks:         %d\n", vfbio->n_blks);

    vfdevs[vfbio->lun] = vfbio;
    spin_lock_irq(&vfdevs_list_lock);
    list_add_tail(&vfbio->list, &vfdevs_list);
    spin_unlock_irq(&vfdevs_list_lock);
    device_add_disk(dev, disk, NULL);
    goto done;

err_cleanup_q:
    if (vfbio->q)
        blk_cleanup_queue(vfbio->q);
    if (disk)
        put_disk(disk);

done:
    pr_debug("<--\n");
    return status;
}

/* Delete vfbio device */
void vfbio_device_delete(struct vfbio_device *vfbio)
{
    del_gendisk(vfbio->disk);
    put_disk(vfbio->disk);
    blk_cleanup_queue(vfbio->q);
    while (atomic_read(&vfbio->pending_ops));
    vfdevs[vfbio->lun] = NULL;
    spin_lock_irq(&vfdevs_list_lock);
    list_del(&vfbio->list);
    spin_unlock_irq(&vfdevs_list_lock);
}

/* Get vfbio device by lun */
struct vfbio_device *vfbio_device_get(int lun)
{
    struct vfbio_device *vfbio = NULL;
    struct list_head *pos;

    spin_lock_irq(&vfdevs_list_lock);
    list_for_each(pos, &vfdevs_list) {
        struct vfbio_device *vfdev = list_entry(pos, struct vfbio_device, list);
        if (vfdev->lun == lun) {
            vfbio = vfdev;
            break;
        }
    }
    spin_unlock_irq(&vfdevs_list_lock);
    return vfbio;
}

/* Get vfbio device by name */
struct vfbio_device *vfbio_device_get_by_name(const char *name)
{
    struct vfbio_device *vfbio = NULL;
    struct list_head *pos;

    spin_lock_irq(&vfdevs_list_lock);
    list_for_each(pos, &vfdevs_list) {
        struct vfbio_device *vfdev = list_entry(pos, struct vfbio_device, list);
        if (!strcmp(name, vfdev->name)) {
            vfbio = vfdev;
            break;
        }
    }
    spin_unlock_irq(&vfdevs_list_lock);
    return vfbio;
}

/* Get vfbio device by lun */
struct vfbio_device *vfbio_device_get_next(int lun)
{
    struct vfbio_device *vfbio = NULL;
    struct list_head *pos;

    spin_lock_irq(&vfdevs_list_lock);
    if (lun < 0) {
        vfbio = list_first_entry_or_null(&vfdevs_list, struct vfbio_device, list);
    }
    else {
        list_for_each(pos, &vfdevs_list) {
            struct vfbio_device *vfdev = list_entry(pos, struct vfbio_device, list);
            if (vfdev->lun == lun) {
                vfbio = vfdev;
                break;
            }
        }
        if (vfbio != NULL && vfbio != list_last_entry(&vfdevs_list, struct vfbio_device, list))
            vfbio = list_next_entry(vfbio, list);
        else
            vfbio = NULL;
    }
    spin_unlock_irq(&vfdevs_list_lock);
    return vfbio;
}

static int vfbio_probe(struct platform_device *pdev)
{
    int status = 0;
    struct device *dev = &pdev->dev;
    struct device_node *np = dev->of_node;
    struct device_node *rnp;
    char *str;
    struct vfbio_device *vfbio;
    struct vfbio_lun_descr lun_descr;
    u32 tmp;

    pr_debug("-->\n");
    vfbio = devm_kzalloc(dev, sizeof(*vfbio), GFP_KERNEL);
    if (!vfbio) {
        dev_err(dev, RED("Unable to allocate vfbio.\n"));
        status = -ENOMEM;
        goto err_kfree_vfbio;
    }

    /* Fetch basic properties from the DT */
    status = of_property_read_u32(np, "reg", &tmp);
    if (status) {
        dev_err(dev, RED("reg property missing or malformed.\n"));
        goto err_kfree_vfbio;
    }
    vfbio->lun = (u8)(tmp & 0xff);
    status = of_property_read_string(np, "lun-name",
                     (const char **)&vfbio->name);
    if (status) {
        dev_err(dev, RED("LUN name property missing or "
                 "malformed.\n"));
        goto err_kfree_vfbio;
    }
    rnp = of_parse_phandle(np, "rpc-channel", 0);
    if (!rnp) {
        dev_err(dev, RED("Unable to retrieve rpc-channel "
            "phandle.\n"));
        status = -EINVAL;
        goto err_kfree_vfbio;
    }
    status = of_property_read_string(rnp, "dev-name", (const char **)&str);
    of_node_put(rnp);
    if (status) {
        dev_err(dev, RED("RPC node dev-name property missing or "
                 "malformed.\n"));
        goto err_kfree_vfbio;
    }
    vfbio->tunnel = rpc_get_fifo_tunnel_id(str);
    if (vfbio->tunnel < 0) {
        dev_err(dev, RED("Unable to obtain RPC tunnel ID.\n"));
        status = -EIO;
        goto err_kfree_vfbio;
    }
    pr_debug("    tunnel          %s (ID: %d)\n", str, vfbio->tunnel);

    /* Fetch the remaining info from the SMC */
    platform_set_drvdata(pdev, vfbio);
    vfbio->pdev = pdev;
    status = vfbio_get_info(vfbio, &lun_descr);
    if (status) {
        dev_err(dev, RED("%s:%u Failed to fetch the device info from the master.\n"),
            vfbio->name, vfbio->lun);
        goto err_kfree_vfbio;
    }
    vfbio->blk_sz = lun_descr.block_size;
    vfbio->n_blks = lun_descr.size_in_blocks;
    vfbio->read_only = lun_descr.read_only;

    /* Overrides, if any */
    status = of_property_read_u32(np, "block-size", &vfbio->blk_sz);
    if (!status && !vfbio->blk_sz) {
        dev_err(dev, RED("block-size property is malformed.\n"));
        goto err_kfree_vfbio;
    }
    status = of_property_read_u32(np, "blocks", &vfbio->n_blks);
    if (!status && !vfbio->n_blks) {
        dev_err(dev, RED("blocks property is malformed.\n"));
        goto err_kfree_vfbio;
    }
    pr_debug("    access          %s\n", vfbio->read_only ? "read-only" : "read-write");

    /* Create device */
    status = vfbio_device_create(dev, vfbio);
    if (status)
        goto err_kfree_vfbio;

    pr_debug("<--\n");
    return 0;

err_kfree_vfbio:
    pr_debug("<-- error\n");
    if (vfbio)
        kfree(vfbio);
    platform_set_drvdata(pdev, NULL);
    return status;
}

static int vfbio_remove(struct platform_device *pdev)
{
    struct vfbio_device *vfbio = platform_get_drvdata(pdev);
    vfbio_device_delete(vfbio);
    platform_set_drvdata(pdev, NULL);
    return 0;
}

static const struct of_device_id vfbio_of_match[] = {
    {.compatible = "brcm,vfbio"},
    {}
};
MODULE_DEVICE_TABLE(of, vfbio_of_match);
static struct platform_driver vfbio_driver = {
    .probe        = vfbio_probe,
    .remove        = vfbio_remove,
    .driver        = {
        .owner    = THIS_MODULE,
        .name    = MODULE_NAME,
        .of_match_table    = vfbio_of_match
    },
};

static int __init vfbio_init(void)
{
    int status = 0;
    struct vfbio_sgls_entry *entry;
    int i;
    struct list_head *pos;
#ifdef CONFIG_BCM_VFBIO_MQ
    const char *mode = "multi-queue";
#else
    const char *mode = "single-queue";
#endif

    pr_info("%s driver v%s, %s mode\n", MODULE_NAME, MODULE_VER, mode);

    /*
     * Make sure that struct vfbio_hw_sgl that we pass to the
     * SMC is an integral # of cache lines.
     */
    BUG_ON(sizeof(struct vfbio_hw_sgl) % cache_line_size());

    spin_lock_init(&vfdevs_list_lock);
    INIT_LIST_HEAD(&vfdevs_list);
    spin_lock_init(&vfbio_sgls_lock);
    INIT_LIST_HEAD(&vfbio_sgls);
    spin_lock_irq(&vfbio_sgls_lock);
    for (i = 0; i < VFBIO_MAX_PENDING_ASYNC_OPS; i++) {
        /*
         * Allocate each entry individually to ensure that the 1st
         * member (struct vfbio_hw_sgl) that we pass to the SMC is
         * cache-line aligned. The slab allocator will allocate
         * on boundaries of the next largest size block from what
         * is requested. Since we already ensured above that
         * this struct size is an integral multiple of cache lines
         * this will ensure that the struct is cache line aligned.
         */
        entry = kzalloc(sizeof(struct vfbio_sgls_entry), GFP_KERNEL);
        if (!entry) {
            pr_err(RED("Unable to allocate sgls entry.\n"));
            status = -ENOMEM;
            spin_unlock_irq(&vfbio_sgls_lock);
            goto err_free_sgls;
        }
        list_add_tail(&entry->list, &vfbio_sgls);
    }
    spin_unlock_irq(&vfbio_sgls_lock);
    vfbio_crash_hw_sgl = kzalloc(sizeof(struct vfbio_hw_sgl), GFP_KERNEL);
    if (!vfbio_crash_hw_sgl) {
        pr_err(RED("Unable to allocate vfbio_crash_hw_sgl.\n"));
        status = -ENOMEM;
        goto err_free_sgls;
    }
    io_timeout = VFBIO_IO_TIMEOUT_DEFAULT;
    retry_delay = VFBIO_RETRY_DELAY_DEFAULT;

    vfbio_major = register_blkdev(0, "vflash");
    if (vfbio_major < 0) {
        pr_err(RED("Failed to register block device.\n"));
        status = vfbio_major;
        goto err_free_sgls;
    }
    status = rpc_register_functions(RPC_SERVICE_VFBIO, vfbio_services_tbl,
                    VFBIO_FUNC_MAX);
    if (status) {
        pr_err(RED("Failed to register RPC function(s).\n"));
        goto err_unreg_blkdev;
    }
    vfbio_proc_init();
    status = platform_driver_register(&vfbio_driver);
    if (status)
        goto err_unreg_rpc;
    status = driver_create_file(&vfbio_driver.driver,
                    &driver_attr_io_timeout);
    if (status)
        goto err_unreg_driver;
    status = driver_create_file(&vfbio_driver.driver,
                    &driver_attr_retry_delay);
    if (status)
        goto err_remove_file;

    /* Initialize LVM ioctl interface */
    status = vfbio_lvm_ioctl_init();
    if (status)
        goto err_remove_file2;
    goto done;

err_remove_file2:
    driver_remove_file(&vfbio_driver.driver, &driver_attr_retry_delay);
err_remove_file:
    driver_remove_file(&vfbio_driver.driver, &driver_attr_io_timeout);
err_unreg_driver:
    platform_driver_unregister(&vfbio_driver);
err_unreg_rpc:
    rpc_unregister_functions(RPC_SERVICE_VFBIO);
err_unreg_blkdev:
    unregister_blkdev(vfbio_major, "vflash");
err_free_sgls:
    if (vfbio_crash_hw_sgl)
        kfree(vfbio_crash_hw_sgl);
    spin_lock_irq(&vfbio_sgls_lock);
    list_for_each(pos, &vfbio_sgls) {
        entry = list_entry(pos, struct vfbio_sgls_entry, list);
        kfree(entry);
        list_del(pos);
    }
    spin_unlock_irq(&vfbio_sgls_lock);
done:
    return status;
}
late_initcall_sync(vfbio_init);

static void vfbio_exit(void)
{
    struct vfbio_sgls_entry *entry;
    struct list_head *pos;

    vfbio_lvm_ioctl_exit();
    driver_remove_file(&vfbio_driver.driver, &driver_attr_retry_delay);
    driver_remove_file(&vfbio_driver.driver, &driver_attr_io_timeout);
    platform_driver_unregister(&vfbio_driver);
    vfbio_proc_exit();
    rpc_unregister_functions(RPC_SERVICE_VFBIO);
    unregister_blkdev(vfbio_major, "vflash");
    if (vfbio_crash_hw_sgl)
        kfree(vfbio_crash_hw_sgl);
    spin_lock_irq(&vfbio_sgls_lock);
    list_for_each(pos, &vfbio_sgls) {
        entry = list_entry(pos, struct vfbio_sgls_entry, list);
        kfree(entry);
        list_del(pos);
    }
    spin_unlock_irq(&vfbio_sgls_lock);
}
module_exit(vfbio_exit);

MODULE_AUTHOR("Tim Ross <tim.ross@broadcom.com>");
MODULE_LICENSE("GPL v2");
