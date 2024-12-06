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
 * vFlash rpmb  driver
 *
 * Author: Dima Mamut <dima.mamut@broadcom.com>
*****************************************************************************/
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/cdev.h>
#include <linux/mmc/ioctl.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/hdreg.h>
#include <linux/kdev_t.h>
#include <linux/blkdev.h>
#include <linux/mutex.h>
#include <linux/scatterlist.h>
#include <linux/string_helpers.h>
#include <linux/delay.h>
#include <linux/capability.h>
#include <linux/compat.h>
#include <linux/pm_runtime.h>
#include <linux/idr.h>
#include <linux/debugfs.h>
#include "vfbio_rpmb.h" 
#include "vfbio_priv.h"
 
//#define DEBUG
 
 
typedef struct 
{
    uint32_t write_counter;
    struct {
        uint16_t msg_type;
        uint16_t op_result;
        uint16_t address;
    } last_op;
    uint8_t nonce[16];
    uint8_t key_mac[32];
}rpmb_last_op_db_t;
 
#define MMC_READ_MULTIPLE_BLOCK                     18
#define MMC_WRITE_MULTIPLE_BLOCK                    25
 
/* Emulated rel_wr_sec_c value (reliable write size, *256 bytes) */
#define VFBIO_RPMB_REL_WR_SEC_C   1
/* Emulated rpmb_size_mult value (RPMB size, *128 kB) */
#define VFBIO_RPMB_SIZE_MULT  2
 
typedef struct  
{
    struct mmc_ioc_cmd ic;
    unsigned char *buf;
    u64 buf_bytes;
}rpmb_blk_ioc_data_t;
 
static rpmb_last_op_db_t rpmb_last_op_db; 
 
static inline void dump_buffer(uint8_t *pHead, uint32_t len)
{
    uint32_t i, n;
    uint8_t *c = pHead;
    uint8_t buf[60];
    for (i = 0, n = 0; i < len; ++i) 
    {
        if (i % 16 == 0) 
        {
            if (i) 
                printk("%s\n", buf);
            n = sprintf(buf, "%04x:", i);
        }
        if (i % 8 == 0)
            n += sprintf(&buf[n], "  %02x", *c++ );
        else
            n += sprintf(&buf[n], " %02x", *c++ );
    }
    printk("%s\n", buf);
}
 
static int rpc_rpbm_setkey(rpmb_data_frame_t *frm)
{
    int status = 0;
 
#ifdef DEBUG
    printk("[%s]:\n",__FUNCTION__);
    dump_buffer(frm->key_mac, 32);
#endif
 
    status = vfbio_rpmb_transfer_data(frm, sizeof(rpmb_data_frame_t), VFBIO_FUNC_RPMB_WRITE_KEY, VFBIO_RPMB_DMA_TO_DEVICE, 1);
    return status;
}
 
static int rpc_rpbm_read_ctr(rpmb_data_frame_t *frm)
{
    int status = 0;
 
#ifdef DEBUG
    printk("[%s]:\n",__FUNCTION__);
#endif
 
    status = vfbio_rpmb_transfer_data(frm, sizeof(rpmb_data_frame_t), VFBIO_FUNC_RPMB_READ_CNT, VFBIO_RPMB_DMA_FROM_DEVICE, 1);
 
#ifdef DEBUG
    printk("write_counter[%d]\n", frm->write_counter);
#endif
 
    return status;
}
 
static int rpc_rpbm_mem_write(rpmb_data_frame_t *frm, unsigned int nfrm)
{
    int status = 0;
 
#ifdef DEBUG
    printk("[%s]: address[0x%x] frm->block_count[%d] data:\n",__FUNCTION__, frm->address, frm->block_count);
    dump_buffer(frm->data, RPMB_BLOCK_SIZE);
#endif
 
    status = vfbio_rpmb_transfer_data(frm, sizeof(rpmb_data_frame_t), VFBIO_FUNC_RPMB_WRITE_DATA, VFBIO_RPMB_DMA_TO_DEVICE, nfrm);
    return status;
}
 
static int rpc_rpbm_mem_read(rpmb_data_frame_t *frm, unsigned int nfrm)
{
    int status = 0;
#ifdef DEBUG 
    int i;    
    rpmb_data_frame_t  *p_rpmb_buf;
#endif     
 
#ifdef DEBUG
    printk("[%s]:\n",__FUNCTION__);
#endif
 
    status = vfbio_rpmb_transfer_data(frm, (sizeof(rpmb_data_frame_t) * nfrm), VFBIO_FUNC_RPMB_READ_DATA, VFBIO_RPMB_DMA_FROM_DEVICE, nfrm);
 
#ifdef DEBUG
    for (i = 0; i < nfrm; i++) 
    {
        p_rpmb_buf = &frm[i];
        dump_buffer(p_rpmb_buf->data, 256);
        printk("END READ BUFFER[%d]]\n",i);
    }
#endif
 
    return status;
}
 
static int rpmb_blk_ioctl_copy_to_user(struct mmc_ioc_cmd __user *ic_ptr, rpmb_blk_ioc_data_t *idata)
{
    struct mmc_ioc_cmd *ic = &idata->ic;
 
    if (copy_to_user(&(ic_ptr->response), ic->response, sizeof(ic->response)))
    {
        printk("ERROR[%s:%d] size[%ld]\n",__FUNCTION__,__LINE__, sizeof(ic->response));
        return -EFAULT;
    }
    if (!idata->ic.write_flag) 
    {
        if (copy_to_user((void __user *)(unsigned long)ic->data_ptr,
                 idata->buf, idata->buf_bytes))
            return -EFAULT;
    }
 
    if (copy_to_user((void __user *)(unsigned long)ic->data_ptr, idata->buf, idata->buf_bytes))
    {
        printk("ERROR[%s:%d]  idata->buf_bytes[%lld]\n",__FUNCTION__,__LINE__, idata->buf_bytes);
        return -EFAULT;
    }
 
    return 0;
}
 
static rpmb_blk_ioc_data_t *rpmb_blk_ioctl_copy_from_user(struct mmc_ioc_cmd  *user)
{
    rpmb_blk_ioc_data_t *idata;
    int err;
 
    idata = kmalloc(sizeof(*idata), GFP_KERNEL);
    if (!idata) 
    {
        err = -ENOMEM;
        goto out;
    }
 
    if (copy_from_user(&idata->ic, user, sizeof(idata->ic))) 
    {
        err = -EFAULT;
        goto idata_err;
    }
 
    idata->buf_bytes = (u64) idata->ic.blksz * idata->ic.blocks;
 
    if (idata->buf_bytes > MMC_IOC_MAX_BYTES) 
    {
        err = -EOVERFLOW;
        goto idata_err;
    }
 
    if (!idata->buf_bytes) 
    {
        idata->buf = NULL;
        return idata;
    }
 
    idata->buf = memdup_user((void __user *)(unsigned long)idata->ic.data_ptr, idata->buf_bytes);
    if (IS_ERR(idata->buf)) 
    {
        err = PTR_ERR(idata->buf);
        goto idata_err;
    }
 
    return idata;
 
idata_err:
    kfree(idata);
out:
    return ERR_PTR(err);
}
 
static void ioctl_get_keyprog_result(rpmb_last_op_db_t *mem, rpmb_data_frame_t *frm)
{
    frm->msg_type = htons(RPMB_MSG_TYPE_RESP_AUTH_KEY_PROGRAM);
    frm->op_result = mem->last_op.op_result;
}
 
static void ioctl_get_write_result(rpmb_last_op_db_t *mem, rpmb_data_frame_t *frm)
{
    frm->msg_type = mem->last_op.msg_type;
    frm->op_result = mem->last_op.op_result;
    frm->address = mem->last_op.address;
    frm->write_counter = mem->write_counter;
    memcpy(frm->key_mac, mem->key_mac, 32);
}
 
static int rpmb_process_cmd(rpmb_blk_ioc_data_t *idata)
{
    struct mmc_ioc_cmd *cmd = &idata->ic;
    rpmb_data_frame_t *frm = NULL;
    uint16_t msg_type = 0;
    rpmb_last_op_db_t *mem = &rpmb_last_op_db;
    int result = 0;
    int i;
 
    switch (cmd->opcode) 
    {
        case MMC_WRITE_MULTIPLE_BLOCK:
            frm = (rpmb_data_frame_t *)(uintptr_t)idata->buf;
            msg_type = ntohs(frm->msg_type);
#ifdef DEBUG            
            printk("[%s] msg_type[%d]\n",__FUNCTION__, msg_type);
#endif            
            switch (msg_type) 
            {
                case VFBIO_RPMB_WRITE_KEY:
                    mem->last_op.msg_type = msg_type;
                    mem->last_op.op_result = rpc_rpbm_setkey(frm);
#ifdef DEBUG                    
                    printk("[%s:%d] msg_type[%d] op_result[%d]\n",__FUNCTION__,__LINE__,msg_type, mem->last_op.op_result);
#endif                    
                    break;
 
                case VFBIO_RPMB_WRITE:
#ifdef DEBUG                
                    printk("[%d] VFBIO_RPMB_WRITE: frm->address[%d] cmd->blocks[%d]\n",__LINE__,frm->address,cmd->blocks);
#endif                    
                    frm->address = frm->address;
                    mem->last_op.address = frm->address;
                    rpc_rpbm_mem_write(frm, cmd->blocks);
                    mem->last_op.op_result = frm->op_result;
                    mem->write_counter = frm->write_counter;
                    mem->last_op.msg_type = msg_type;
                    memcpy(mem->key_mac, frm->key_mac, 32);
                    break;
 
                case VFBIO_RPMB_RESP:
                    /* Optionally not used currently in mmc user space aaplication */
                    break;
                case VFBIO_RPMB_READ_CNT:
                case VFBIO_RPMB_READ:
                    mem->last_op.msg_type = msg_type;
                    mem->last_op.address = frm->address;
                    memcpy(mem->nonce, frm->nonce, 16);
                    break;        
                default:
                    printk("ERROR[%s:%d] - MMC_WRITE_MULTIPLE_BLOCK Unexpected msg_type[%d]\n",__FUNCTION__,__LINE__,msg_type);
                    return -1;
            }
            break;
 
        case MMC_READ_MULTIPLE_BLOCK:
            frm = (rpmb_data_frame_t *)(uintptr_t)idata->buf;
            msg_type = ntohs(frm->msg_type);
#ifdef DEBUG 
            printk("[%s] fmsg_type[%d]\n",__FUNCTION__, msg_type);
#endif
            switch (mem->last_op.msg_type)
            {
                case VFBIO_RPMB_WRITE_KEY:
                    ioctl_get_keyprog_result(mem, frm);
                    break;
 
                case VFBIO_RPMB_WRITE:
                    ioctl_get_write_result(mem, frm); // counter
                    break;
 
                case VFBIO_RPMB_READ_CNT:
                    memcpy(frm->nonce, mem->nonce, 16);
                    result = rpc_rpbm_read_ctr(frm);
                    break;
 
                case VFBIO_RPMB_READ:
                    for (i = 0; i < cmd->blocks; i++)
                        memcpy(frm[i].nonce, mem->nonce, 16);
                    frm->address = mem->last_op.address;
                    result = rpc_rpbm_mem_read(frm, cmd->blocks);
                    break;
 
                default:
                    printk("ERROR[%s:%d] - MMC_READ_MULTIPLE_BLOCK Unexpected msg_type[%d]\n",__FUNCTION__,__LINE__,msg_type);
                    return -1;    
            }
 
            break;
 
        default:
            printk("ERROR[%s:%d] - Unsupported ioctl opcode 0x%08x\n", __FUNCTION__,__LINE__,cmd->opcode);
            return -1;
    }
 
    return 0;
}
 
int vfbio_rpmb_ioctl_cmd(struct mmc_ioc_cmd *ic_ptr)
{
    int err = 0, ioc_err = 0;
 
    rpmb_blk_ioc_data_t *idata;
 
    idata = rpmb_blk_ioctl_copy_from_user(ic_ptr);
    if (IS_ERR(idata))
        return PTR_ERR(idata);
 
    ioc_err = rpmb_process_cmd(idata);
    if (ioc_err)
    {
        printk("ERROR[%s:%d] - Unexpected\n",__FUNCTION__,__LINE__);
        goto cmd_err;
    }
 
    err = rpmb_blk_ioctl_copy_to_user(ic_ptr, idata);
 
cmd_err:
    if (idata->buf != NULL)
        kfree(idata->buf);
    kfree(idata);
 
    return ioc_err ? ioc_err : err;
}
 
int vfbio_rpmb_ioctl_multi_cmd(struct mmc_ioc_multi_cmd *user)
{
    struct mmc_ioc_cmd __user *cmds = user->cmds;
    __u64 num_of_cmds;
    rpmb_blk_ioc_data_t **idata = NULL;
    int i;
    int err = 0, ioc_err = 0;
 
    if (copy_from_user(&num_of_cmds, &user->num_of_cmds, sizeof(num_of_cmds)))
        return -EFAULT;
 
    if (!num_of_cmds)
        return 0;
 
    if (num_of_cmds > MMC_IOC_MAX_CMDS)
        return -EINVAL;
 
    idata = kcalloc(num_of_cmds, sizeof(*idata), GFP_KERNEL);
    if (!idata)
        return -ENOMEM;
 
    for (i = 0; i < num_of_cmds; i++) 
    {
        idata[i] = rpmb_blk_ioctl_copy_from_user(&cmds[i]);
        if (IS_ERR(idata[i])) 
        {
            err = PTR_ERR(idata[i]);
            num_of_cmds = i;
            goto multi_cmd_err;
        }
        ioc_err = rpmb_process_cmd(idata[i]);
        if (ioc_err)
        {
            printk("ERROR[%s:%d] - Unexpected\n",__FUNCTION__,__LINE__);
            goto multi_cmd_err;
        }
    }
 
    /* copy to user if data and response */
    for (i = 0; i < num_of_cmds && !err; i++)
    {
        err = rpmb_blk_ioctl_copy_to_user(&cmds[i], idata[i]);
        if (err)
            printk("ERROR[%s:%d] cmnd[%d]- Unexpected\n",__FUNCTION__,__LINE__,i);
    }
 
multi_cmd_err:
    for (i = 0; i < num_of_cmds; i++) 
    {
        if (idata[i]->buf != NULL)
            kfree(idata[i]->buf);
        kfree(idata[i]);
    }
    kfree(idata);
 
    return ioc_err ? ioc_err : err;
}