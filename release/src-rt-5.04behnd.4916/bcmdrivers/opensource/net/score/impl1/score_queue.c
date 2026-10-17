/*
    <:copyright-BRCM:2024:DUAL/GPL:standard

    Copyright (c) 2024 Broadcom
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
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/dma-map-ops.h>
#include "score_int.h"
#include "bcm_score.h"

static struct device *p_alloc_device;

static int score_dcp_mmap(void *data, struct file *filep, struct vm_area_struct *vma);
static int score_ctrl_release(void *data);
static int score_ctrl_mmap(void *data, struct file *filep, struct vm_area_struct *vma);

#define SCORE_CONTROL_RING_DEPTH (1024)
#define SCORE_CONTROL_RING_ENTRY_SIZE (sizeof(score_ctrl_msg_t))

typedef int (*score_drv_obj_mmap_cb)(void *data, struct file *filep, struct vm_area_struct *vma);
typedef int (*score_drv_obj_release_cb)(void *data);

typedef struct {
    void *data;
    score_drv_obj_release_cb release;
    score_drv_obj_mmap_cb    mmap;
    // event notification
    wait_queue_head_t wqh;
    // memory mapping
    void    *mapped_data;
    uint32_t mapped_size;
    dma_addr_t phy_addr;
} score_drv_object_t;

typedef struct
{
    score_drv_object_t  score_object;
    uint32_t            pid;         // pid
    bcm_async_queue_t  *async_queue; // queue for control ring
} score_application_info_t;

#define MAX_APPLICATION_NUM (8)


score_application_info_t *dcp_2_application[SCORE_MAX_DCP_NUM];
score_application_info_t  application_info[MAX_APPLICATION_NUM];
score_drv_object_t        dcp_objects[SCORE_MAX_DCP_NUM];

static void score_extend_file(struct file *filep,        // open file to extend
                       score_drv_object_t * obj,    // obj to extend with
                       void * data,               // opaque for following callbacks
                       score_drv_obj_release_cb release,  // called by .release
                       score_drv_obj_mmap_cb mmap)        // called by .mmap
{
    obj->data = data;
    obj->release = release;
    obj->mmap = mmap;
    init_waitqueue_head(&obj->wqh);
    filep->private_data = obj;
}

static void score_signal(score_drv_object_t * obj)
{
        unsigned long flags;

        spin_lock_irqsave(&obj->wqh.lock, flags);
        if (waitqueue_active(&obj->wqh))
                wake_up_locked_poll(&obj->wqh, EPOLLIN);
        spin_unlock_irqrestore(&obj->wqh.lock, flags);
}

void score_signal_dcp(int dcp_id)
{
    if (dcp_id < 0 || dcp_id >= SCORE_MAX_DCP_NUM)
    {
        __logError("bad dcp_id %i\n",dcp_id);
        return;
    }
    score_signal(&dcp_objects[dcp_id]);
}

static int score_post_ctrl_message(uint32_t cmd, void *data, int size, score_application_info_t *app, int dcp_id)
{
    volatile score_ctrl_msg_t *msg;

    if (app == NULL)
        return -1;

    if (app->pid == 0xFFFFFFFF)
    {
        __logError("app not configured\n");
        return -1;
    }

    if (app->async_queue->entry_size < size + 8)
    {
        __logError("message too big %i < %i\n", app->async_queue->entry_size,  size + 8);
        return -1;
    }

    msg = (volatile score_ctrl_msg_t *)bcm_async_queue_entry_write(app->async_queue);
    msg->msg_type = cmd;

    // copy the data to the union field after msg_type
    memcpy((void *)&msg->msg_data, data, size);

    if (bcm_async_queue_not_full(app->async_queue))
    {
        bcm_async_queue_entry_enqueue(app->async_queue);
        app->async_queue->stats.writes++;
        score_procfs_data_g.ctrl_sent_app_total++;
        score_procfs_data_g.ctrl_sent_app[dcp_id]++;
    }
    else
    {
        app->async_queue->stats.discards++;
        score_procfs_data_g.ctrl_dropped_app_total++;
        score_procfs_data_g.ctrl_dropped_app[dcp_id]++;
        return -1;
    }

    __logDebug("new ctrl async queue write %i read %i\n",app->async_queue->write, app->async_queue->read);

    score_signal(&app->score_object);
 
    return 0;
}

int score_post_ctrl_message_dcp(uint32_t cmd, void *data, int size, int dcp_id) 
{
    return score_post_ctrl_message(cmd, data, size, dcp_2_application[dcp_id], dcp_id);
}

static int score_ctrl_create(score_application_info_t *app, score_queue_info_t *queue_info)
{
    /* allocated SDR ring */
    uint32_t alloc_size, entry_size;
    uint32_t entries = SCORE_CONTROL_RING_DEPTH;
    void *queue_mem;
    bcm_async_queue_t *async_queue;

    entry_size = SCORE_CONTROL_RING_ENTRY_SIZE;
    alloc_size = entries * entry_size + sizeof(bcm_async_queue_t);
    alloc_size = (alloc_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    queue_mem = kmalloc(alloc_size, GFP_KERNEL);
    if (!queue_mem)
        return -1;

    async_queue = (bcm_async_queue_t *)(queue_mem + alloc_size) - 1;

    bcm_async_queue_init_from(async_queue, (uint8_t *)queue_mem, entries, entry_size);

    __logDebug("queue initialized [%li] %p / %p", sizeof(queue_mem), queue_mem, (void*)(uintptr_t)virt_to_phys(queue_mem));

    // score object needs the base pointer for future mmap
    app->score_object.mapped_data = queue_mem;
    app->score_object.mapped_size = alloc_size;

    // set ioctl info
    queue_info->num_entries = entries;
    queue_info->entry_size = entry_size;
    queue_info->alloc_size = alloc_size;

    // keep working data
    app->async_queue = async_queue;

    return 0;
}

static int score_alloc_application_entry(void)
{
    int i;
    int free_entry = -1;
    uint32_t pid = current->pid;
    for (i = 0; i < MAX_APPLICATION_NUM; i++)
    {
        if (application_info[i].pid == pid)
        {
            __logError("Ctrl queue already created for application with pid %i",pid);
            return -1;
        }
        else if (application_info[i].pid == 0xFFFFFFFF && free_entry == -1)
            free_entry = i; 
    }
    return free_entry;
}

static score_application_info_t *score_find_application(uint32_t pid)
{
    int i;
    for (i = 0; i < MAX_APPLICATION_NUM; i++)
    {
        if (application_info[i].pid == pid)
            return &application_info[i];
    }
    return 0;
}


int score_ctrl_init(struct file *filep, score_cfg_data_t *cfg)
{
    int free_entry = score_alloc_application_entry();
    score_application_info_t *app_info;
    uint32_t pid = current->pid;
    __logDebug("Initialize");

    if (free_entry < 0)
    {
        __logError("No free ctrl queue. Application limit reached");
        return -1;
    }

    app_info = &application_info[free_entry];

    /* allocate control async queue */
    if (score_ctrl_create(app_info, &cfg->queue_info))
    {
        __logError("score_ctrl_create failed\n");
        return -1;
    }

    app_info->pid = pid;

    score_extend_file(filep,
                      &app_info->score_object,
                      (void *)app_info,
                      score_ctrl_release,
                      score_ctrl_mmap);

    __logDebug("init entry %i\n", free_entry);

    return free_entry;
}

int score_queue_init(struct device * alloc_device)
{
    int i;
    memset(dcp_2_application, 0, sizeof(dcp_2_application));
    memset(application_info, 0, sizeof(score_application_info_t));
    for (i = 0; i < MAX_APPLICATION_NUM; i++)
        application_info[i].pid = 0xFFFFFFFF;

#ifdef CONFIG_BCM_GLB_COHERENCY
    arch_setup_dma_ops(alloc_device, 0, 0, NULL, true);
#else
    arch_setup_dma_ops(alloc_device, 0, 0, NULL, false);
#endif

#if defined(CONFIG_BCM96858)
    /* need to confirm how many bits we support in 6858 runner */
    dma_set_coherent_mask(alloc_device, DMA_BIT_MASK(40));
#else
    dma_coerce_mask_and_coherent(alloc_device, DMA_BIT_MASK(32));
#endif
    p_alloc_device = alloc_device;
    return 0;
}

static uint32_t score_get_data_size_from_collection_profile(score_dcp_cfg_data_t *score_dcp_cfg_data)
{
    return score_dcp_cfg_data->info.report_size;
}

int score_sdr_create(struct file *filep, score_dcp_cfg_data_t *score_dcp_cfg_data, score_sdr_cfg_t *sdr)
{
    /* allocated SDR ring */
    uint32_t alloc_size, entry_size;
    uint32_t entries = score_dcp_cfg_data->queue_info.num_entries;
    uint32_t dcp_id = score_dcp_cfg_data->info.dcp_id;
    score_drv_object_t *dcp_object = &dcp_objects[dcp_id];
    void *score_rx_queue_p;
    bcm_async_queue_t *async_queue;

    if (entries & (entries - 1)) /* not a power of 2 */
    {
        __logError("bad num_entries %i not a power of 2\n", entries);
        return -1;
    }

    if (dcp_id >= SCORE_MAX_DCP_NUM)
        return -1;

    dcp_2_application[dcp_id] = score_find_application(current->pid);
    if (!dcp_2_application[dcp_id])
    {
        __logError("cannot find application entry\n");
        return -1;
    }

    entry_size = score_get_data_size_from_collection_profile(score_dcp_cfg_data);
    alloc_size = entries * entry_size + sizeof(bcm_async_queue_t);
    alloc_size = (alloc_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    score_rx_queue_p = dma_alloc_coherent(p_alloc_device, alloc_size, &dcp_object->phy_addr, __GFP_ZERO);
    if (!score_rx_queue_p)
    {
        __logError("failed creating mm queue\n");
        return -1;
    }

    async_queue = (bcm_async_queue_t *)(score_rx_queue_p + alloc_size) - 1;

    bcm_async_queue_init_from(async_queue, (uint8_t *)score_rx_queue_p, entries, entry_size);

    __logDebug("queue initialized [%li] %p / %p", sizeof(score_rx_queue_p), score_rx_queue_p, (void*)(uintptr_t)virt_to_phys(score_rx_queue_p));
    __logDebug("phy queue initialized [%li] %p", sizeof(dma_addr_t), (void*)(uintptr_t)dcp_object->phy_addr);

    // keep this pointer to be able to mmap later on
    dcp_object->mapped_data = score_rx_queue_p;
    dcp_object->mapped_size = alloc_size;

    // feedback info to ioctrl
    score_dcp_cfg_data->queue_info.entry_size = entry_size;
    score_dcp_cfg_data->queue_info.alloc_size = alloc_size;

    // setup bdmf request data
    sdr->sdr_addr = score_rx_queue_p;
    sdr->offset_to_read_write = (uint32_t)((uintptr_t)&(async_queue->write) - (uintptr_t)score_rx_queue_p);
    sdr->num_entries = entries;
    sdr->entry_size = entry_size;

    score_extend_file(filep,
                      dcp_object,
                      (void *)(uintptr_t) dcp_id,
                      score_dcp_release,
                      score_dcp_mmap);


    return 0;
}

void score_sdr_destroy(uint8_t dcp_id)
{
    if (dcp_2_application[dcp_id])
    {
        dma_free_coherent(p_alloc_device,
                          dcp_objects[dcp_id].mapped_size,
                          dcp_objects[dcp_id].mapped_data,
                          dcp_objects[dcp_id].phy_addr);

        dcp_2_application[dcp_id] = 0;

        __debug("queue removed");
    }
}

static int score_common_mmap(score_drv_object_t *obj, struct vm_area_struct *vma)
{
    struct page *page = NULL;
    unsigned long size = (unsigned long)(vma->vm_end - vma->vm_start);
    int ret;

    if (size > obj->mapped_size)
    {
        __logError("size %lu > max_size %u", size, obj->mapped_size);
        ret = -EINVAL;
        return ret;
    }

    page = virt_to_page((uintptr_t)obj->mapped_data + (vma->vm_pgoff << PAGE_SHIFT));

    ret = remap_pfn_range(vma, vma->vm_start, page_to_pfn(page), size, vma->vm_page_prot);
    if (ret != 0)
    {   
        __logError("Could not remap_pfn_range");
        return ret;
    }

    //WMB(); // UNdO ME 
    __logDebug("ok\n");
    return 0;
}

static bcm_async_queue_t * _score_get_dcp_async_queue(uint32_t dcp_id)
{
    uint32_t mapped_size = dcp_objects[dcp_id].mapped_size;
    uint8_t * mapped_data = dcp_objects[dcp_id].mapped_data;
    return (bcm_async_queue_t *)(mapped_data + mapped_size) - 1;
}

static int score_dcp_mmap(void *data, struct file *filep, struct vm_area_struct *vma)
{
    uint32_t idx = (uint32_t) (uintptr_t) data;
    bcm_async_queue_t *queue_p = _score_get_dcp_async_queue(idx);

    __logDebug("index %li, filep = %p ref_cnt = %d\n\n",idx, filep, filep->f_count);
    queue_p->read = 0;
    queue_p->write = 0;

    return score_common_mmap(&dcp_objects[idx], vma);
}

static int score_ctrl_release(void *data)
{
    score_application_info_t * app_info = (score_application_info_t *) data;
    if (!data)
        return -1;
    app_info->pid = 0xFFFFFFFF;
    kfree(app_info->score_object.mapped_data);
    return 0;
}

static int score_ctrl_mmap(void *data, struct file *filep, struct vm_area_struct *vma)
{
    score_application_info_t * app_info = (score_application_info_t *) data;

    __logDebug("pid %i, filep = %p ref_cnt = %d\n",app_info->pid, filep, filep->f_count);

    // application called this function to map this queue memory, reset read and write indices
    app_info->async_queue->read = 0;
    app_info->async_queue->write = 0;

    return score_common_mmap(&app_info->score_object, vma);
}

int score_release(struct inode *inodep, struct file *filep)
{
    __logDebug("filep %p %i %p, ref_cnt = %d\n",filep,current->pid, filep->private_data, filep->f_count);
    if (filep->private_data)
    {
        score_drv_object_t *prv = (score_drv_object_t *) filep->private_data; 
        if (prv->release)
            prv->release(prv->data);
        filep->private_data = NULL;
    }
    return 0;
}

int score_open(struct inode *inodep, struct file *filep)
{
    __logDebug("filep = %p pid = %i, ref_cnt = %d\n",filep,current->pid, filep->f_count);
    filep->private_data = 0;
    return 0;
}

int score_mmap(struct file *filep, struct vm_area_struct *vma)
{
    __logDebug("start %lx end %lx offset %lx pid %i, filep = %p ref_cnt = %d\n",
               (uintptr_t) vma->vm_start, (uintptr_t) vma->vm_end, (uintptr_t) vma->vm_pgoff, current->pid,
               filep, filep->f_count);
    if (filep->private_data)
    {
        score_drv_object_t *prv = (score_drv_object_t *) filep->private_data; 
        if (prv->mmap)
            prv->mmap(prv->data, filep, vma);
    }
    return 0;
}

__poll_t score_poll(struct file *file, poll_table *wait)
{
        score_drv_object_t *prv = (score_drv_object_t *) file->private_data; 
        if (prv == 0)
        {
           __logError("make sure oject is configured before issuing epoll_ctl\n");
           return EPOLLERR;
        }
        poll_wait(file, &prv->wqh, wait);
        return EPOLLIN;
}

