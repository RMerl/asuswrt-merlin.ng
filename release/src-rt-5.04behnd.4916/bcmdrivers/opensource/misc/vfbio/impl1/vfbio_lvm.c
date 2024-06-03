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
 * vFlash block IO driver - Logical LUN Management support
 *
 * Author: Igor Ternovsky <igor.ternovsky@broadcom.com>
*****************************************************************************/

#include <asm/cacheflush.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include "vfbio_priv.h"
#include "vfbio_lvm.h"

#define VFBIO_LVM_CONTROL0_DEV_NAME     "lvmctrlo"
#define VFBIO_LVM_CONTROLB_DEV_NAME     "lvmctrlb"
#define VFBIO_LVM_DEFAULT_NAME_FORMAT   "dynlun-%u"
#define VFBIO_WRITE_BUFFER_SIZE         (128 * 1024)

#define VFBIO_ROUND_UP(v, b) (((v)+(b)-1) & ~((b)-1))

DEFINE_MUTEX(vfdevs_lock);

static int vfbio_lun_delete_from_smc(struct vfbio_device *vfbio)
{
    rpc_msg msg;
    int status;

    rpc_msg_init(&msg, RPC_SERVICE_VFBIO, VFBIO_FUNC_LVM_LUN_DELETE, 0, 0, 0, 0);
    status = vfbio_lun_request_timeout_msg(vfbio, &msg);
    if (status) {
        dev_err(&vfbio->pdev->dev, RED("Delete dynamic lun request failed. status='%s'\n"),
            (status==-EIO) ? "EIO" : vfbio_error_str(status));
    }
    return status;
}

static struct vfbio_device *vfbio_get_lvm_control(void)
{
    struct vfbio_device *vfbio_lvm;
    /* Find block device for LVM control. Dynamic devices will be created
       as "disks" sharing the same basic parameters (tunnel, blk_size) */
    vfbio_lvm = vfbio_device_get_by_name(VFBIO_LVM_CONTROL0_DEV_NAME);
    if (!vfbio_lvm)
        vfbio_lvm = vfbio_device_get_by_name(VFBIO_LVM_CONTROLB_DEV_NAME);
    if (!vfbio_lvm || !vfbio_lvm->pdev) {
        printk(RED("LVM control device is not found. LVM support is off\n"));
        return NULL;
    }
    return vfbio_lvm;
}

/* Create a new dynamic lun */
int vfbio_lun_create(const char *name, uint64_t size, int *lun_id)
{
    struct vfbio_device *vfbio_lvm;
    struct vfbio_device *vfbio=NULL;
    struct vfbio_lun_create_request *create_request=NULL;
    struct device *dev;
    uint64_t rounded_size;
    char *lun_name = NULL;
    dma_addr_t dma_addr;
    rpc_msg msg;
    int status;

    /* Validation */
    if (!size || !lun_id || (*lun_id > 0xFF))
        return -EINVAL;

    /* Find block device for LVM control. Dynamic devices will be created
       as "disks" sharing the same basic parameters (tunnel, blk_size) */
    vfbio_lvm = vfbio_get_lvm_control();
    if (!vfbio_lvm)
        return -ENOTSUPP;

    dev = &vfbio_lvm->pdev->dev;
    vfbio = devm_kzalloc(dev, sizeof(*vfbio), GFP_KERNEL);
    if (!vfbio) {
        dev_err(dev, RED("Unable to allocate vfbio\n"));
        status = -ENOMEM;
        goto vfbio_err;
    }

    /* Automatic name assignment */
    lun_name = kmalloc(VFBIO_LVM_MAX_NAME_SIZE + 1, GFP_KERNEL);
    if (!lun_name) {
        dev_err(dev, RED("Unable to allocate lun_name\n"));
        status = -ENOMEM;
        goto vfbio_err;
    }

    if (!name || ! *name)
    {
        if (*lun_id < 0) {
            dev_err(dev, RED("automatic lun name and lun id assignment can't be used simultaneously\n"));
            status = -EINVAL;
            goto vfbio_err;
        }
        snprintf(lun_name, VFBIO_LVM_MAX_NAME_SIZE, VFBIO_LVM_DEFAULT_NAME_FORMAT, *lun_id);
    }
    else {
        strncpy(lun_name, name, VFBIO_LVM_MAX_NAME_SIZE);
    }
    lun_name[VFBIO_LVM_MAX_NAME_SIZE] = 0;

    if (vfbio_lvm->blk_sz > VFBIO_LVM_ALLOC_BLOCK_SIZE)
        rounded_size = VFBIO_ROUND_UP(size, vfbio_lvm->blk_sz);
    else
        rounded_size = VFBIO_ROUND_UP(size, VFBIO_LVM_ALLOC_BLOCK_SIZE);
    vfbio->pdev = vfbio_lvm->pdev;
    vfbio->lun = (*lun_id >= 0) ? (uint8_t)*lun_id : 0xff;
    vfbio->name = lun_name;
    vfbio->tunnel = vfbio_lvm->tunnel;
    vfbio->blk_sz = vfbio_lvm->blk_sz;
    vfbio->n_blks = rounded_size / vfbio->blk_sz;

    /* Try to create in SMC first */
    create_request = kzalloc(sizeof(*create_request), GFP_KERNEL);
    if (create_request == NULL) {
        dev_err(dev, RED("Failed to allocate create request buffer\n"));
        status = -ENOMEM;
        goto vfbio_err;
    }

    strncpy(create_request->lun_name, lun_name, sizeof(create_request->lun_name));
    create_request->lun_size = rounded_size / VFBIO_LVM_ALLOC_BLOCK_SIZE;
    rpc_msg_init(&msg, RPC_SERVICE_VFBIO, VFBIO_FUNC_LVM_LUN_CREATE, 0, 0, 0, 0);
    dma_addr = dma_map_single(dev, create_request, sizeof(*create_request), DMA_TO_DEVICE);
    if (dma_mapping_error(dev, dma_addr)) {
        dev_err(dev, RED("failure mapping LVM create request\n"));
        kfree(create_request);
        status = -ENOMEM;
        goto vfbio_err;
    }
    vfbio_lvm_msg_set_req_buf_addr(&msg, dma_addr);

    mutex_lock(&vfdevs_lock);
    /* Send create request to the SMC */
    status = vfbio_lun_request_timeout_msg(vfbio, &msg);

    dma_unmap_single(dev, dma_addr, sizeof(*create_request), DMA_TO_DEVICE);
    kfree(create_request);
    if (status) {
        dev_err(dev, RED("Create dynamic lun request failed. status='%s' (%d)\n"),
            (status==-EIO) ? "EIO" : vfbio_error_str(status), status);
        mutex_unlock(&vfdevs_lock);
        goto vfbio_err;
    }
    vfbio->lun = vfbio_lvm_msg_get_lun(&msg);

    /* Create a block device */
    status = vfbio_device_create(dev, vfbio);
    mutex_unlock(&vfdevs_lock);
    if (status) {
        dev_err(dev, RED("Failed to create dynamic lun %s-%u\n"), lun_name, vfbio->lun);
        vfbio_lun_delete_from_smc(vfbio);
        goto vfbio_err;
    }

    /* All good. lun info is already printed by vfbio_device_create */
    *lun_id = vfbio->lun;

    return 0;

    /* Cleanup in case of error */
vfbio_err:
    if (lun_name)
        kfree(lun_name);
    if (vfbio)
        devm_kfree(dev, vfbio);
    return status;
}

/* Delete dynamic lun created by vfbio_lun_create */
int vfbio_lun_delete(int lun_id)
{
    struct vfbio_device *vfbio_lvm = vfbio_get_lvm_control();
    struct vfbio_device *vfbio;
    int status;

    if (!vfbio_lvm)
        return -ENOTSUPP;

    /* Find block device for LVM control. Dynamic devices will be created
       as "disks" sharing the same basic parameters (tunnel, blk_size) */
    vfbio = vfbio_device_get(lun_id);
    if (!vfbio) {
        return -ENODEV;
    }
    dev_notice(&vfbio->pdev->dev, "Removing logical device %s-%d\n", vfbio->name, lun_id);
    mutex_lock(&vfdevs_lock);
    status = vfbio_lun_delete_from_smc(vfbio);
    if (!status) {
        vfbio_device_delete(vfbio);
        /* Free memory if device was created by vfbio_lun_create during the current session,
           rather than probe. */
        if (vfbio->pdev == vfbio_lvm->pdev) {
            kfree(vfbio->name);
            devm_kfree(&vfbio_lvm->pdev->dev, vfbio);
        }
    }
    mutex_unlock(&vfdevs_lock);
    return status;
}

/* Resize dynamic lun */
int vfbio_lun_resize(int lun_id, uint64_t size)
{
    struct vfbio_device *vfbio;
    uint64_t rounded_size;
    uint32_t n_blks;
    sector_t n_sects;
    rpc_msg msg;
    int status;

    /* Validation */
    if (!size)
        return -EINVAL;

    /* Find the relevant block device */
    vfbio = vfbio_device_get(lun_id);
    if (!vfbio) {
        return -ENODEV;
    }
    dev_notice(&vfbio->pdev->dev, "Resizing logical device %s-%d\n", vfbio->name, lun_id);
    if (vfbio->blk_sz > VFBIO_LVM_ALLOC_BLOCK_SIZE)
        rounded_size = VFBIO_ROUND_UP(size, vfbio->blk_sz);
    else
        rounded_size = VFBIO_ROUND_UP(size, VFBIO_LVM_ALLOC_BLOCK_SIZE);
    n_blks = rounded_size / vfbio->blk_sz;

    /* Send create request to the SMC */
    rpc_msg_init(&msg, RPC_SERVICE_VFBIO, VFBIO_FUNC_LVM_LUN_RESIZE, 0, 0, 0, 0);
    vfbio_lvm_msg_set_size(&msg, n_blks);
    status = vfbio_lun_request_timeout_msg(vfbio, &msg);
    if (!status) {
        vfbio->n_blks = n_blks;
        n_sects = ((sector_t)n_blks * vfbio->blk_sz) >> SECTOR_SHIFT;
        set_capacity(vfbio->disk, n_sects);
    }

    return status;
}

/* Rename 1 or multiple luns atomically (all or nothing) */
int vfbio_lun_rename(uint8_t num_luns, struct vfbio_lun_id_name id_name[])
{
    struct vfbio_device *vfbio_lvm;
    struct vfbio_lun_rename_request *rename_request = NULL;
    uint32_t request_size;
    struct device *dev;
    dma_addr_t dma_addr;
    rpc_msg msg;
    int status;
    int i;

    if (!num_luns)
        return -EINVAL;

    /* Find block device for LVM control. Dynamic devices will be created
       as "disks" sharing the same basic parameters (tunnel, blk_size) */
    vfbio_lvm = vfbio_get_lvm_control();
    if (!vfbio_lvm)
        return -ENOTSUPP;
    dev = &vfbio_lvm->pdev->dev;

    request_size = sizeof(*rename_request) + num_luns * sizeof(struct vfbio_idx_name);
    rename_request = kzalloc(request_size, GFP_KERNEL);
    if (rename_request == NULL) {
        dev_err(dev, RED("Failed to allocate the request buffer\n"));
        status = -ENOMEM;
        goto vfbio_req_done;
    }
    rename_request->num_luns = num_luns;
    for (i = 0; i < num_luns; i++) {
        rename_request->idx_name[i].lun_idx = id_name[i].lun_id;
        strncpy(rename_request->idx_name[i].lun_name, id_name[i].lun_name, VFBIO_LVM_MAX_NAME_SIZE);
    }

    dma_addr = dma_map_single(dev, rename_request, request_size, DMA_TO_DEVICE);
    if (dma_mapping_error(dev, dma_addr)) {
        dev_err(dev, RED("failure mapping LUN info request\n"));
        status = -ENOMEM;
        goto vfbio_req_done;
    }

    rpc_msg_init(&msg, RPC_SERVICE_VFBIO, VFBIO_FUNC_LVM_LUN_RENAME, 0, 0, 0, 0);
    vfbio_lvm_msg_set_req_buf_addr(&msg, dma_addr);

    /* Send rename request to the SMC */
    status = vfbio_lun_request_timeout_msg(vfbio_lvm, &msg);

vfbio_req_done:
    if (rename_request != NULL) {
        if (!dma_mapping_error(dev, dma_addr))
            dma_unmap_single(dev, dma_addr, request_size, DMA_TO_DEVICE);
        kfree(rename_request);
    }
    if (status) {
        dev_err(dev, RED("Rename request failed. status='%s' (%d)\n"),
            (status==-EIO) ? "EIO" : vfbio_error_str(status), status);
    }
    else {
        dev_notice(dev, RED("%u devices renamed. The new names will become active after reboot\n"), num_luns);
    }
    return status;
}

/* Set lun access type (read-write or read-only) */
int vfbio_lun_chmod(int lun_id, int read_only)
{
    struct vfbio_device *vfbio;
    rpc_msg msg;
    int status;

    /* Find the relevant block device */
    vfbio = vfbio_device_get(lun_id);
    if (!vfbio) {
        return -ENODEV;
    }
    dev_notice(&vfbio->pdev->dev, "Changing logical device %s-%d to %s\n",
        vfbio->name, lun_id, read_only ? "read-only" : "read-write");

    /* Send create request to the SMC */
    rpc_msg_init(&msg, RPC_SERVICE_VFBIO, VFBIO_FUNC_LVM_LUN_CHMOD, 0, 0, 0, 0);
    vfbio_lvm_msg_chmod_set_mode(&msg, read_only);
    status = vfbio_lun_request_timeout_msg(vfbio, &msg);
    if (!status) {
        set_disk_ro(vfbio->disk, read_only);
    }

    return status;
}

/* Internal version of 'Get lun info' */
int vfbio_get_info(struct vfbio_device *vfbio, struct vfbio_lun_descr *lun_descr)
{
    struct device *dev = &vfbio->pdev->dev;
    struct vfbio_lun_info *lun_info = NULL;
    dma_addr_t dma_addr;
    rpc_msg msg;
    int status;

    lun_info = kzalloc(sizeof(*lun_info), GFP_KERNEL);
    if (lun_info == NULL) {
        dev_err(dev, RED("Failed to allocate info request buffer\n"));
        status = -ENOMEM;
        goto vfbio_req_done;
    }
    dma_addr = dma_map_single(dev, lun_info, sizeof(*lun_info), DMA_FROM_DEVICE);
    if (dma_mapping_error(dev, dma_addr)) {
        dev_err(dev, RED("failure mapping LUN info request\n"));
        status = -ENOMEM;
        goto vfbio_req_done;
    }

    rpc_msg_init(&msg, RPC_SERVICE_VFBIO, VFBIO_FUNC_LUN_INFO, 0, 0, 0, 0);
    vfbio_lvm_msg_set_req_buf_addr(&msg, dma_addr);

    /* Send create request to the SMC */
    status = vfbio_lun_request_timeout_msg(vfbio, &msg);
    if (!status && !lun_info->n_blks)
        status = VFBIO_ERROR_LUN_INVALID;
    if (!status)
    {
        memset(lun_descr, 0, sizeof(*lun_descr));
        lun_descr->lun_id = vfbio->lun;
        strncpy(lun_descr->lun_name, lun_info->name, sizeof(lun_info->name));
        lun_descr->block_size = 512 * (1 << vfbio_lun_info_get_blk_sz(lun_info));
        lun_descr->size_in_blocks = lun_info->n_blks;
        lun_descr->read_only = (lun_info->flags & VFBIO_LUN_INFO_FLAG_READ_ONLY) != 0;
        lun_descr->dynamic = (lun_info->flags & VFBIO_LUN_INFO_FLAG_DYNAMIC) != 0;
    }

vfbio_req_done:
    if (lun_info != NULL) {
        if (!dma_mapping_error(dev, dma_addr))
            dma_unmap_single(dev, dma_addr, sizeof(*lun_info), DMA_TO_DEVICE);
        kfree(lun_info);
    }
    if (status) {
        dev_err(dev, RED("Get lun info request failed. status='%s' (%d)\n"),
            (status==-EIO) ? "EIO" : vfbio_error_str(status), status);
    }
    return status;

}

/* Get lun info */
int vfbio_lun_get_info(int lun_id, struct vfbio_lun_descr *lun_descr)
{
    struct vfbio_device *vfbio;

    /* Find the relevant block device */
    vfbio = vfbio_device_get(lun_id);
    if (!vfbio) {
        return -ENODEV;
    }
    return vfbio_get_info(vfbio, lun_descr);
}

/* Get device info */
int vfbio_device_get_info(uint64_t *total_size, uint64_t *free_size)
{
    struct vfbio_device *vfbio_lvm;
    rpc_msg msg;
    int status;

    /* Find block device for LVM control */
    vfbio_lvm = vfbio_get_lvm_control();
    if (!vfbio_lvm)
        return -ENOTSUPP;

    rpc_msg_init(&msg, RPC_SERVICE_VFBIO, VFBIO_FUNC_LVM_DEVICE_INFO, 0, 0, 0, 0);

    /* Send create request to the SMC */
    status = vfbio_lun_request_timeout_msg(vfbio_lvm, &msg);
    *total_size = (uint64_t)vfbio_lvm_msg_get_device_info_total_blocks(&msg) * VFBIO_LVM_ALLOC_BLOCK_SIZE;
    *free_size = (uint64_t)vfbio_lvm_msg_get_device_info_free_blocks(&msg) * VFBIO_LVM_ALLOC_BLOCK_SIZE;
    return status;
}

/* Get lun id by name */
int vfbio_lun_get_id(const char *name, int *id)
{
    struct vfbio_device *vfbio;

    if (name == NULL || id == NULL)
        return -EINVAL;

    /* Find block device for LVM control. Dynamic devices will be created
       as "disks" sharing the same basic parameters (tunnel, blk_size) */
    vfbio = vfbio_device_get_by_name(name);
    if (!vfbio)
        return VFBIO_ERROR_LUN_INVALID;
    *id = vfbio->lun;
    return 0;
}

/* LUN iterator */
int vfbio_lun_get_next(int prev, int *lun_id)
{
    struct vfbio_device *vfbio;

    if (lun_id == NULL)
        return -EINVAL;

    /* Validate prev */
    if (prev >= 0 && vfbio_device_get(prev) == NULL)
        return VFBIO_ERROR_LUN_INVALID;

    /* Get next */
    vfbio = vfbio_device_get_next(prev);
    if (vfbio == NULL)
        return VFBIO_ERROR_NO_MORE_LUNS;
    *lun_id = vfbio->lun;
    return 0;
}

/* Write to LUN using synchronous IO */
int vfbio_lun_write(int lun_id, void *data, uint32_t size)
{
    uint32_t num_blocks;
    uint8_t *write_buf;
    uint32_t transfer_size;
    uint32_t offset = 0;
    uint32_t blocks_per_transfer;
    uint32_t start_block = 0;
    struct vfbio_device *vfbio;
    struct device *dev;
    dma_addr_t dma_addr;
    int status = 0;
    rpc_msg msg;
    int kaddr = 1;

    /* Find the relevant block device */
    vfbio = vfbio_device_get(lun_id);
    if (!vfbio)
        return -ENODEV;

    num_blocks = (size + vfbio->blk_sz - 1) / vfbio->blk_sz;
    if (num_blocks > vfbio->n_blks)
        return VFBIO_ERROR_NO_ROOM;

    size = num_blocks * vfbio->blk_sz;
    transfer_size = (size > VFBIO_WRITE_BUFFER_SIZE) ? VFBIO_WRITE_BUFFER_SIZE : size;
    
    if (!virt_addr_valid(data))
    {
        kaddr = 0;
        write_buf = kmalloc(transfer_size, GFP_KERNEL);
        if (write_buf == NULL)
            return VFBIO_ERROR_NO_MEM;
    }
    

    dev = &vfbio->pdev->dev;
    blocks_per_transfer = transfer_size / vfbio->blk_sz;
    while (num_blocks)
    {
        if(kaddr)
        {
            write_buf = (char *)data + offset;
        }
        else
        {
            status = copy_from_user((char *)write_buf, (char *)data + offset, transfer_size);
            if (status < 0)
                break;
	}

        dma_addr = dma_map_single(dev, write_buf, transfer_size, DMA_TO_DEVICE);
        if (dma_mapping_error(dev, dma_addr)) 
        {
            dev_err(dev, RED("failure mapping hw_sgl\n"));
            status = VFBIO_ERROR_NO_MEM;
            break;
        }

        rpc_msg_init(&msg, RPC_SERVICE_VFBIO, VFBIO_FUNC_WRITE, 0, 0, 0, 0);
        vfbio_msg_set_lun(&msg, lun_id);
        vfbio_msg_set_n_blks(&msg, blocks_per_transfer);
        vfbio_msg_set_blk(&msg, start_block);
        vfbio_msg_set_addr(&msg, dma_addr);
        status = vfbio_lun_request_timeout_msg(vfbio, &msg);
        dma_unmap_single(dev, dma_addr, transfer_size, DMA_TO_DEVICE);
        if (status)
        {
            dev_err(dev, RED("LUN %s: rpc WRITE failure\n"), vfbio->name);
            break;
        }

        num_blocks -= blocks_per_transfer; 
        offset += transfer_size;
        start_block += blocks_per_transfer;
        if (blocks_per_transfer > num_blocks)
        {
            blocks_per_transfer = num_blocks;
            transfer_size = blocks_per_transfer * vfbio->blk_sz;
        }
    }

    if(!kaddr)
        kfree(write_buf);

    return status;
}

/* Get error text */
const char *vfbio_error_str(int err)
{
    const char *err_text;
    switch(err)
    {
        case VFBIO_ERROR_OK:
            err_text = "OK";
            break;
        case VFBIO_ERROR_PARMS:
            err_text = "Error in parameters";
            break;
        case VFBIO_ERROR_LUN_INVALID:
            err_text = "Invalid LUN index or name";
            break;
        case VFBIO_ERROR_LUN_NOT_OPENED:
            err_text = "LUN is not opened";
            break;
        case VFBIO_ERROR_LUN_IDX_EXISTS:
            err_text = "LUN with such index already exists";
            break;
        case VFBIO_ERROR_LUN_NAME_EXISTS:
            err_text = "LUN with such name already exists";
            break;
        case VFBIO_ERROR_LUN_IS_NOT_DYNAMIC:
            err_text = "LUN is not dynamic";
            break;
        case VFBIO_ERROR_NO_MORE_LUNS:
            err_text = "No more LUNs";
            break;
        case VFBIO_ERROR_TRANSLATION:
            err_text = "Address translation error";
            break;
        case VFBIO_ERROR_NO_MEM:
            err_text = "Dynamic memory allocation failed";
            break;
        case VFBIO_ERROR_NO_ROOM:
            err_text = "No room on flash";
            break;
        case VFBIO_ERROR_NO_WRITE_PERMISISON:
            err_text = "No write permission";
            break;
        case VFBIO_ERROR_NO_READ_PERMISISON:
            err_text = "No read permission";
            break;
        case VFBIO_ERROR_INTERNAL:
            err_text = "Internal error";
            break;
        case VFBIO_ERROR_INVALID_DEVICE:
            err_text = "Invalid device index";
            break;
        case VFBIO_ERROR_INVALID_DEVICE_PAGE:
            err_text = "Invalid device page";
            break;
        case VFBIO_ERROR_NO_LVM_LUN:
            err_text = "No LUN for for saving dynamic configuration";
            break;
        case VFBIO_ERROR_CANT_SAVE_CONFIG:
            err_text = "Can't save dynamic LUN configuration";
            break;
        case VFBIO_ERROR_CANT_LOAD_CONFIG:
            err_text = "Can't load dynamic LUN configuration";
            break;
        case VFBIO_ERROR_LVM_IS_NOT_SUPPORTED:
            err_text = "Logical LUN management is not supported";
            break;
        default:
            err_text = "Unknown error";
            break;
    }
    return err_text;
}
