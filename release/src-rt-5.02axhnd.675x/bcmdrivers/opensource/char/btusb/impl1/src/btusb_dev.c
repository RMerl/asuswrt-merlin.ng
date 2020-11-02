/*
 * <:copyright-BRCM:2015:GPL/GPL:standard
 * 
 *    Copyright (c) 2015 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 */

#include "btusb.h"
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/serial.h>

#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include "hcidefs.h"

#include "gki_int.h"

/* local functions declaration */
static int btusb_update_voice_channels(struct btusb *p_dev);
static int btusb_update_voice_trans(struct btusb *p_dev);

/*******************************************************************************
 **
 ** Function         btusb_alloc_tx_trans
 **
 ** Description      Allocate a transaction in the array of transactions.
 **
 ** Parameters       p_array: array of USB transactions
 **                  size: size of the array of transactions
 **
 ** Returns          Pointer to the next free transaction, NULL if not found
 **
 *******************************************************************************/
static struct btusb_tx_trans *btusb_alloc_tx_trans(struct btusb_tx_trans *p_array, size_t size)
{
    int idx;
    struct btusb_tx_trans *p_tx_trans;

    for (idx = 0; idx < size; idx++)
    {
        p_tx_trans = &p_array[idx];
        if (!p_tx_trans->in_use)
        {
            p_tx_trans->in_use = true;
            return p_tx_trans;
        }
    }
    return NULL;
}

/*******************************************************************************
 **
 ** Function         btusb_dump_data
 **
 ** Description      Print the data into the kernel messages
 **
 ** Parameters       p: pointer to the data to print
 **                  len: length of the data to print
 **                  p_title: title to print before the data
 **
 ** Returns          void
 **
 *******************************************************************************/
void btusb_dump_data(const UINT8 *p, int len, const char *p_title)
{
    int idx;

    if (likely((dbgflags & BTUSB_DBG_MSG) == 0))
    {
        return;
    }

    if (p_title)
    {
        printk("---------------------- %s ----------------------\n", p_title);
    }

    printk("%p: ", p);

    for(idx = 0; idx < len; idx++)
    {
        printk("%02x ", p[idx]);
    }
    printk("\n--------------------------------------------------------------------\n");
}

/*******************************************************************************
 **
 ** Function         btusb_voice_stats
 **
 ** Description
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void btusb_voice_stats(unsigned long *p_max, unsigned long *p_min,
        struct timeval *p_result, struct timeval *p_last_time)
{
    struct timeval current_time;

    do_gettimeofday(&current_time);
    /* perform the carry for the later subtraction by updating y */
    if (current_time.tv_usec < p_last_time->tv_usec)
    {
        int nsec = (p_last_time->tv_usec - current_time.tv_usec) / 1000000 + 1;
        p_last_time->tv_usec -= 1000000 * nsec;
        p_last_time->tv_sec += nsec;
    }
    if (current_time.tv_usec - p_last_time->tv_usec > 1000000)
    {
        int nsec = (current_time.tv_usec - p_last_time->tv_usec) / 1000000;
        p_last_time->tv_usec += 1000000 * nsec;
        p_last_time->tv_sec -= nsec;
    }
    /* compute the time remaining to wait */
    p_result->tv_sec = current_time.tv_sec - p_last_time->tv_sec;
    p_result->tv_usec = current_time.tv_usec - p_last_time->tv_usec;

    /* update the max except the first time where the calculation is wrong
       because of the initial value assuming p_last is zero initialized */
    if (p_max != NULL)
    {
        if (p_result->tv_usec > *p_max && (p_last_time->tv_sec) && (p_last_time->tv_usec))
        {
            *p_max = p_result->tv_usec;
        }
    }

    /* update the min except the first time where the calculation is wrong
       because of the initial value assuming *p_last and p_min are zero initialized */
    if (p_min != NULL)
    {
        if ((p_result->tv_usec < *p_min || (*p_min == 0)) &&
            (p_last_time->tv_sec) && (p_last_time->tv_usec))
        {
            *p_min = p_result->tv_usec;
        }
    }

    *p_last_time = current_time;
    BTUSB_DBG("btusb_voice_stats len: %lu\n", p_result->tv_usec);
}

/*******************************************************************************
 **
 ** Function         btusb_rx_enqueue
 **
 ** Description      Add a received USB message to the RX queue
 **
 ** Parameters       p_dev: device control block reference
 **                  p_rx_trans: transaction to forward
 **                  hcitype: type of HCI data in the USB transaction
 **
 ** Returns          void
 **
 *******************************************************************************/
void btusb_rx_enqueue(struct btusb *p_dev, struct btusb_rx_trans *p_rx_trans, UINT8 hcitype)
{
    BT_HDR *p_buf = &p_rx_trans->bt_hdr;
    struct urb *p_urb = p_rx_trans->p_urb;
    UINT8 *p = (UINT8 *)p_urb->transfer_buffer;
    UINT16 len;

    BTUSB_DBG("len = %d\n", p_urb->actual_length);

    /* calculate the HCI packet length, depending on the type */
    switch (hcitype)
    {
    case HCIT_TYPE_ACL_DATA:
        p += 2;
        STREAM_TO_UINT16(len, p);
        len += 4;
        break;

    case HCIT_TYPE_EVENT:
        p += 1;
        STREAM_TO_UINT8(len, p);
        len += 2;
        break;

    case HCIT_TYPE_LM_DIAG:
        len = HCIT_LM_DIAG_LENGTH;
        break;

    default:
        BTUSB_ERR("Unsupported HCI type: %d\n", hcitype);
        len = p_urb->actual_length;
        break;
    }

    /* check HCI packet length matches that of the USB transaction */
    if (unlikely(len != p_urb->actual_length))
    {
        /* firmware fix for specific HW that do not correctly support ZLP:
         * - 43242 and 43569 shared USB
         * - some USB hubs not supporting ZLP packets and requiring a workaround
         */
        if (unlikely(((len % 16) == 0) && (p_urb->actual_length == (len + 1))))
        {
            BTUSB_DBG("missing ZLP workaround: %d != %d\n", p_urb->actual_length, len);
            p_urb->actual_length = len;
        }
        else
        {
            BTUSB_ERR("URB data length does not match packet %d != %d\n", p_urb->actual_length, len);
        }
    }

    /* fill up the header of the message */
    p_buf->event = hcitype;
    p_buf->len = p_urb->actual_length;
    p_buf->offset = 0;
    p_buf->layer_specific &= ~BTUSB_LS_H4_TYPE_SENT;

    /* InQ for user-space to read */
    GKI_enqueue(&p_dev->rx_queue, p_buf);

    /* if the process is polling, indicate RX event */
    wake_up_interruptible(&p_dev->rx_wait_q);
}

/*******************************************************************************
 **
 ** Function         btusb_rx_enqueue_voice
 **
 ** Description      Add a full HCI voice packet to the RX queue
 **
 ** Parameters       p_dev: device control block reference
 **                  p_pkt: transaction to forward
 **
 ** Returns          void
 **
 *******************************************************************************/
void btusb_rx_enqueue_voice(struct btusb *p_dev, struct btusb_voice_pkt *p_pkt)
{
    /* InQ for user-space to read */
    GKI_enqueue(&p_dev->rx_queue, &p_pkt->bt_hdr);

    /* if the process is polling, indicate RX event */
    wake_up_interruptible(&p_dev->rx_wait_q);
}

/*******************************************************************************
 **
 ** Function         btusb_rx_dequeued
 **
 ** Description      Figure out what to do with a dequeued message
 **
 ** Parameters       p_dev: device control block reference
 **                  p_msg: dequeued message
 **
 ** Returns          void
 **
 *******************************************************************************/
void btusb_rx_dequeued(struct btusb *p_dev, BT_HDR *p_msg)
{
    struct btusb_rx_trans *p_rx_trans = container_of(p_msg, struct btusb_rx_trans, bt_hdr);
    struct btusb_voice_pkt *p_pkt = container_of(p_msg, struct btusb_voice_pkt, bt_hdr);

#if defined(BTUSB_LITE)
    if (unlikely(p_msg->layer_specific & BTUSB_LS_GKI_BUFFER))
    {
        GKI_freebuf(p_msg);
        return;
    }
#endif

    switch (p_msg->event)
    {
    case HCIT_TYPE_ACL_DATA:
        if (unlikely(btusb_submit(p_dev, &p_dev->acl_rx_submitted, p_rx_trans->p_urb, GFP_KERNEL)))
            p_dev->stats.acl_rx_submit_err++;
        else
            p_dev->stats.acl_rx_submit_ok++;
        break;

    case HCIT_TYPE_EVENT:
#if defined(BTUSB_LITE)
        /* when BTUSB_LITE is defined, EVT may be resubmitted from IRQ context */
        if (unlikely(btusb_submit(p_dev, &p_dev->event_submitted, p_rx_trans->p_urb, GFP_ATOMIC)))
#else
        if (unlikely(btusb_submit(p_dev, &p_dev->event_submitted, p_rx_trans->p_urb, GFP_KERNEL)))
#endif
            p_dev->stats.event_submit_err++;
        else
            p_dev->stats.event_submit_ok++;
        break;

    case HCIT_TYPE_LM_DIAG:
        if (unlikely(btusb_submit(p_dev, &p_dev->diag_rx_submitted, p_rx_trans->p_urb, GFP_KERNEL)))
            p_dev->stats.diag_rx_submit_err++;
        else
            p_dev->stats.diag_rx_submit_ok++;
        break;

    case HCIT_TYPE_SCO_DATA:
        if (!btusb_cq_put(&p_dev->voice_rx_list, p_pkt))
        {
            BTUSB_ERR("btusb_cq_put failed\n");
        }
        break;
    default:
        BTUSB_ERR("Unexpected buffer type\n");
        break;
    }
}

/*******************************************************************************
 **
 ** Function         btusb_open
 **
 ** Description      User mode open callback
 **
 *******************************************************************************/
int btusb_open(struct inode *inode, struct file *file)
{
    struct btusb *p_dev;
    struct usb_interface *interface;
    int subminor;
    int retval = 0;

    BTUSB_INFO("enter\n");

    /* retrieve the minor for this inode */
    subminor = iminor(inode);

    /* retrieve the USB interface attached to this minor */
    interface = usb_find_interface(&btusb_driver, subminor);
    if (!interface)
    {
        BTUSB_ERR("can't find interface for minor %d\n", subminor);
        retval = -ENODEV;
        goto exit;
    }
    BTUSB_INFO("minor %u\n", subminor);

    /* retrieve the device driver structure pointer */
    p_dev = usb_get_intfdata(interface);
    BTUSB_INFO("p_dev=%p\n", p_dev);
    if (!p_dev)
    {
        BTUSB_ERR("can't find device\n");
        retval = -ENODEV;
        goto exit;
    }

    /* save our object in the file's private structure */
    file->private_data = p_dev;

    /* increment our usage count for the device */
    BTUSB_INFO("kref_get -> &p_dev->kref 0x%p\n", &p_dev->kref);
    kref_get(&p_dev->kref);

exit:
    return retval;
}

/*******************************************************************************
 **
 ** Function         btusb_release
 **
 ** Description      User mode close callback
 **
 *******************************************************************************/
int btusb_release(struct inode *inode, struct file *file)
{
    struct btusb *p_dev;
    int idx;

    BTUSB_INFO("enter\n");

    p_dev = (struct btusb *) file->private_data;
    BTUSB_INFO("p_dev=%p\n", p_dev);
    if (unlikely(!p_dev))
    {
        BTUSB_ERR("can't find device\n");
        return -ENODEV;
    }

#ifdef BTUSB_LITE
    btusb_lite_stop_all(p_dev);
#endif

    /* ensure there is no process hanging on poll / select */
    wake_up_interruptible(&p_dev->rx_wait_q);

    for (idx = 0; idx < ARRAY_SIZE(p_dev->voice_rx.channels); idx++)
    {
        if (p_dev->voice_rx.channels[idx].used)
        {
            if (btusb_remove_voice_channel(p_dev, p_dev->voice_rx.channels[idx].handle))
            {
                BTUSB_ERR("btusb_remove_voice_channel failed\n");
            }
        }
    }

    /* decrement the usage count on our device */
    BTUSB_INFO("kref_put -> &p_dev->kref 0x%p\n", &p_dev->kref);
    kref_put(&p_dev->kref, btusb_delete);

    return 0;
}

/*******************************************************************************
 **
 ** Function         btusb_get_rx_packet
 **
 ** Description      Get (extract) next buffer to sent to User Space
 **
 *******************************************************************************/
static BT_HDR *btusb_get_rx_packet(struct btusb *p_dev)
{
    /* if there is a pending Rx buffer */
    if (p_dev->p_rx_msg)
    {
        return p_dev->p_rx_msg;
    }
#ifdef BTUSB_LITE
    /* If HCI is over IPC */
    if (btusb_lite_is_hci_over_ipc(p_dev))
    {
        return NULL;
    }
#endif

    /* dequeue the next buffer and store its reference */
    p_dev->p_rx_msg = GKI_dequeue(&p_dev->rx_queue);

    return p_dev->p_rx_msg;
}

/*******************************************************************************
 **
 ** Function         btusb_get_rx_packet_buffer
 **
 ** Description      Get pointer address of data to send to User Space
 **
 *******************************************************************************/
static UINT8 *btusb_get_rx_packet_buffer(struct btusb *p_dev, BT_HDR *p_msg)
{
    struct btusb_rx_trans *p_rx_trans;

    switch(p_msg->event)
    {
    case HCIT_TYPE_ACL_DATA:
    case HCIT_TYPE_EVENT:
    case HCIT_TYPE_LM_DIAG:
#ifdef BTUSB_LITE
        if (unlikely(p_msg->layer_specific & BTUSB_LS_GKI_BUFFER))
        {
            return ((char *)(p_msg + 1) + p_msg->offset);
        }
#endif
        p_rx_trans = container_of(p_msg, struct btusb_rx_trans, bt_hdr);
        return (p_rx_trans->dma_buffer + p_msg->offset);
    default: /* SCO etc... */
        return ((char *)(p_msg + 1) + p_msg->offset);
    }
}

/*******************************************************************************
 **
 ** Function         btusb_read
 **
 ** Description      User mode read
 **
 ** Returns          Number of bytes read, error number if negative
 **
 *******************************************************************************/
ssize_t btusb_read(struct file *file, char __user *buffer, size_t count, loff_t *ppos)
{
    struct btusb *p_dev = (struct btusb *)file->private_data;
    UINT16 len;
    size_t remainder = count;
    BT_HDR *p_buf;
    char *p_data;
    char type;
    int retval;

    BTUSB_DBG("p_dev=%p count=%zu\n", p_dev, count);
    if (unlikely(!p_dev))
    {
        BTUSB_ERR("can't find device\n");
        return -ENODEV;
    }

    if (unlikely(!p_dev->p_main_intf))
    {
        BTUSB_ERR("device unplugged\n");
        return -ENODEV;
    }

    /* if read is non blocking and there is no data */
    if (unlikely((file->f_flags & O_NONBLOCK) && (!btusb_get_rx_packet(p_dev))))
    {
        BTUSB_INFO("Non blocking read without any data\n");
        return -EAGAIN;
    }

    /* wait for an event */
    retval = wait_event_interruptible(p_dev->rx_wait_q, btusb_get_rx_packet(p_dev));
    if (unlikely(retval))
    {
        BTUSB_DBG("read wait interrupted\n");
        return retval;
    }

    p_buf = p_dev->p_rx_msg;
    BTUSB_DBG("buffer=%p len=%u ls=%u\n", p_buf, p_buf->len, p_buf->layer_specific);
    /* if the buffer is not empty and this is the first time buffer is picked,
     * add HCI header in user space */
    if (likely(p_buf->len && !(p_buf->layer_specific & BTUSB_LS_H4_TYPE_SENT)))
    {
        if (likely(remainder >= 1))
        {
            type = p_buf->event;
            /* add the H4 packet header */
            if (unlikely(copy_to_user(buffer, &type, 1)))
            {
                BTUSB_ERR("copy to user error\n");
                return -EINVAL;
            }
            p_buf->layer_specific |= BTUSB_LS_H4_TYPE_SENT;
            buffer += 1;
            remainder -= 1;
        }
        else
        {
            BTUSB_ERR("Not enough space to copy H4 ACL header\n");
            goto read_end;
        }
    }

    /* retrieve address of the next data to send (depends on event) */
    p_data = btusb_get_rx_packet_buffer(p_dev, p_buf);

    /* take the min of remainder and p_buf length */
    if (likely(p_buf->len < remainder))
    {
        len = p_buf->len;
    }
    else
    {
        len = remainder;
    }

    /* copy the message data to the available user space */
    if (unlikely(copy_to_user(buffer, p_data, len)))
    {
        BTUSB_ERR("copy to user error\n");
        return -EINVAL;
    }
    remainder -= len;
    p_buf->len -= len;
    p_buf->offset += len;

    /* free the first buffer if it is empty */
    if (likely(p_buf->len == 0))
    {
        /* free (fake event) or resubmit (real USB buffer) the buffer sent to app */
        btusb_rx_dequeued(p_dev, p_buf);
        p_dev->p_rx_msg = NULL;
    }

read_end:
    return (count - remainder);
}

/*******************************************************************************
 **
 ** Function         btusb_write
 **
 ** Description      User mode write
 **
 ** Returns          Number of bytes written, error number if negative
 **
 *******************************************************************************/
ssize_t btusb_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *ppos)
{
    struct btusb *p_dev;
    unsigned char *p_data;
    size_t remainder = count;
    int err;
    size_t appended, len;

    p_dev = (struct btusb *)file->private_data;

    BTUSB_DBG("p_dev=%p count=%zu\n", p_dev, count);

    if (unlikely(!p_dev))
    {
        BTUSB_ERR("can't find device\n");
        return -ENODEV;
    }

    if (unlikely(!p_dev->p_main_intf))
    {
        BTUSB_ERR("device unplugged\n");
        return -ENODEV;
    }

    if (unlikely(count == 0))
    {
        return 0;
    }

    while (remainder)
    {
        BTUSB_DBG("remain=%zu write_msg_len=%zu\n", remainder, p_dev->write_msg_len);

        /* append the entire data to the buffer (not exceeding buffer length) */
        if (likely(remainder < (sizeof(p_dev->write_msg) - p_dev->write_msg_len)))
            appended = remainder;
        else
            appended = sizeof(p_dev->write_msg) - p_dev->write_msg_len;
        if (unlikely(copy_from_user(p_dev->write_msg + p_dev->write_msg_len, (void *)user_buffer, appended)))
        {
            BTUSB_ERR("copy from user error\n");
            return -EINVAL;
        }
        BTUSB_DBG("write_msg_len=%zu appended=%zu\n", p_dev->write_msg_len, appended);

        /* update the size of the message buffer */
        p_dev->write_msg_len += appended;

        /* compute the real HCI packet length (by default 0xFFFF to mark incomplete) */
        len = 0xFFFF;
        /* bypass HCI type + opcode/connection handle */
        p_data = &p_dev->write_msg[3];
        switch(p_dev->write_msg[0])
        {
            case HCIT_TYPE_SCO_DATA:
            case HCIT_TYPE_COMMAND:
                if (likely(p_dev->write_msg_len >= 4))
                {
                    STREAM_TO_UINT8(len, p_data);
                    len += 4;
                    BTUSB_DBG("SCO/CMD len=%zu cur=%zu\n", len, p_dev->write_msg_len);
                }
                break;
            case HCIT_TYPE_ACL_DATA:
                if (likely(p_dev->write_msg_len >= 5))
                {
                    STREAM_TO_UINT16(len, p_data);
                    len += 5;
                    /* sanity check : ACL buffer should not be larger than supported */
                    if (unlikely(len > BTUSB_H4_MAX_SIZE))
                    {
                        BTUSB_ERR("ACL packet too long (%zu)\n", len);
                        p_dev->write_msg_len = 0;
                        return -EINVAL;
                    }
                    BTUSB_DBG("ACL len=%zu cur=%zu\n", len, p_dev->write_msg_len);
                }
                break;
            case HCIT_TYPE_LM_DIAG:
                /* this packet does not have a length, so just send everything */
                len = p_dev->write_msg_len;
                BTUSB_DBG("DIAG len=%zu cur=%zu\n", len, p_dev->write_msg_len);
                break;
            default:
                BTUSB_ERR("unsupported packet type\n");
                p_dev->write_msg_len = 0;
                return count;
                break;
        }
        /* check if the buffer length exceeds the packet length */
        if (likely(p_dev->write_msg_len >= len))
        {
            /* remove the extra data (belonging to next HCI packet) */
            if (unlikely(p_dev->write_msg_len > len))
            {
                /* remove exceeding data */
                appended -= p_dev->write_msg_len - len;
                /* set len to computed HCI packet length */
                p_dev->write_msg_len = len;
            }
            if (autopm)
            {
                err = usb_autopm_get_interface(p_dev->p_main_intf);
                if (unlikely(err < 0))
                {
                    BTUSB_ERR("autopm failed\n");
                    autopm = 0;
                }
            }
            switch(p_dev->write_msg[0])
            {
            case HCIT_TYPE_COMMAND:
#ifdef BTUSB_LITE
                if (btusb_lite_hci_cmd_filter(p_dev, &p_dev->write_msg[0]))
#endif
                {
                    err = btusb_submit_cmd(p_dev, &p_dev->write_msg[1], p_dev->write_msg_len - 1);
                }
                break;
            case HCIT_TYPE_ACL_DATA:
                err = btusb_submit_acl(p_dev, &p_dev->write_msg[1], p_dev->write_msg_len - 1);
                break;
            case HCIT_TYPE_SCO_DATA:
                err = btusb_submit_voice_tx(p_dev, &p_dev->write_msg[1], p_dev->write_msg_len - 1);
                break;
            case HCIT_TYPE_LM_DIAG:
                err = btusb_submit_diag(p_dev, &p_dev->write_msg[1], p_dev->write_msg_len - 1);
                break;
            default:
                BTUSB_ERR("impossible write path\n");
                return -EFAULT;
            }
            if (err)
            {
                BTUSB_ERR("submit failed\n");
                return err;
            }

            /* new write message */
            p_dev->write_msg_len = 0;
        }
        remainder -= appended;
        user_buffer += appended;
    }

    return count;
}

/*******************************************************************************
 **
 ** Function         btusb_poll
 **
 ** Description      Poll callback (to implement select)
 **
 ** Parameters       file : file structure of the opened instance
 **                  p_pt : poll table to which the local queue must be added
 **
 ** Returns          Mask of the type of polling supported, error number if negative
 **
 *******************************************************************************/
unsigned int btusb_poll(struct file *file, struct poll_table_struct *p_pt)
{
    struct btusb *p_dev;
    unsigned int mask;

    BTUSB_DBG("enter\n");

    /* retrieve the device from the file pointer */
    p_dev = (struct btusb *) file->private_data;
    BTUSB_DBG("p_dev=%p\n", p_dev);
    if (unlikely(!p_dev))
    {
        BTUSB_ERR("can't find device\n");
        return -ENODEV;
    }

    /* check if the device was disconnected */
    if (unlikely(!p_dev->p_main_intf))
    {
        BTUSB_ERR("device unplugged\n");
        return -ENODEV;
    }

    /* indicate to the system on which queue it should poll (non blocking call) */
    poll_wait(file, &p_dev->rx_wait_q, p_pt);

    /* enable WRITE (select/poll is not write blocking) */
    mask = POLLOUT | POLLWRNORM;

    /* enable READ only if data is queued from HCI */
    if (btusb_get_rx_packet(p_dev))
    {
        mask |= POLLIN | POLLRDNORM;
    }

    return mask;
}

#define BTUSB_RETURN_STR(__c) case __c: return #__c
static const char *btusb_ioctl_string(unsigned int cmd)
{
    switch(cmd)
    {
        BTUSB_RETURN_STR(IOCTL_BTWUSB_GET_STATS);
        BTUSB_RETURN_STR(IOCTL_BTWUSB_CLEAR_STATS);
        BTUSB_RETURN_STR(IOCTL_BTWUSB_ADD_VOICE_CHANNEL);
        BTUSB_RETURN_STR(IOCTL_BTWUSB_REMOVE_VOICE_CHANNEL);
        BTUSB_RETURN_STR(TCGETS);
        BTUSB_RETURN_STR(TCSETS);
        BTUSB_RETURN_STR(TCSETSW);
        BTUSB_RETURN_STR(TCSETSF);
        BTUSB_RETURN_STR(TCGETA);
        BTUSB_RETURN_STR(TCSETA);
        BTUSB_RETURN_STR(TCSETAW);
        BTUSB_RETURN_STR(TCSETAF);
        BTUSB_RETURN_STR(TCSBRK);
        BTUSB_RETURN_STR(TCXONC);
        BTUSB_RETURN_STR(TCFLSH);
        BTUSB_RETURN_STR(TIOCGSOFTCAR);
        BTUSB_RETURN_STR(TIOCSSOFTCAR);
        BTUSB_RETURN_STR(TIOCGLCKTRMIOS);
        BTUSB_RETURN_STR(TIOCSLCKTRMIOS);
#ifdef TIOCGETP
        BTUSB_RETURN_STR(TIOCGETP);
        BTUSB_RETURN_STR(TIOCSETP);
        BTUSB_RETURN_STR(TIOCSETN);
#endif
#ifdef TIOCGETC
        BTUSB_RETURN_STR(TIOCGETC);
        BTUSB_RETURN_STR(TIOCSETC);
#endif
#ifdef TIOCGLTC
        BTUSB_RETURN_STR(TIOCGLTC);
        BTUSB_RETURN_STR(TIOCSLTC);
#endif
#ifdef TCGETX
        BTUSB_RETURN_STR(TCGETX);
        BTUSB_RETURN_STR(TCSETX);
        BTUSB_RETURN_STR(TCSETXW);
        BTUSB_RETURN_STR(TCSETXF);
#endif
        BTUSB_RETURN_STR(TIOCMGET);
        BTUSB_RETURN_STR(TIOCMSET);
        BTUSB_RETURN_STR(TIOCGSERIAL);
        BTUSB_RETURN_STR(TIOCMIWAIT);
        BTUSB_RETURN_STR(TIOCMBIC);
        BTUSB_RETURN_STR(TIOCMBIS);
        default:
            return "unknwown";
    }
}

/*******************************************************************************
 **
 ** Function         btusb_ioctl
 **
 ** Description      User mode ioctl
 **
 ** Parameters
 **
 ** Returns          0 upon success or an error code.
 **
 *******************************************************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
long btusb_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
int btusb_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
    const char *s_cmd;
    struct btusb *p_dev;
    struct btusb_ioctl_sco_info sco_info;
    int retval = 0;

    s_cmd = btusb_ioctl_string(cmd);

    BTUSB_INFO("cmd=%s\n", s_cmd);

    p_dev = (struct btusb *) file->private_data;
    BTUSB_INFO("p_dev=%p\n", p_dev);
    if (unlikely(!p_dev))
    {
        BTUSB_ERR("can't find device\n");
        return -ENODEV;
    }

    /* check if the device was disconnected */
    if (unlikely(!p_dev->p_main_intf))
    {
        BTUSB_ERR("device unplugged\n");
        return -ENODEV;
    }

    switch (cmd)
    {
        case IOCTL_BTWUSB_GET_STATS:
            if (copy_to_user((void *) arg, &p_dev->stats, sizeof(tBTUSB_STATS)))
            {
                retval = -EFAULT;
                goto err_out;
            }
            break;

        case IOCTL_BTWUSB_CLEAR_STATS:
            memset(&p_dev->stats, 0, sizeof(tBTUSB_STATS));
            break;

        case IOCTL_BTWUSB_ADD_VOICE_CHANNEL:
            if (copy_from_user(&sco_info, (void *) arg, sizeof(sco_info)))
            {
                BTUSB_ERR("BTUSB_IOCTL_ADD_VOICE_CHANNEL failed getting 1st par\n");
                retval = -EFAULT;
                goto err_out;
            }
            return btusb_add_voice_channel(p_dev, sco_info.handle, sco_info.burst);

        case IOCTL_BTWUSB_REMOVE_VOICE_CHANNEL:
            if (copy_from_user(&sco_info, (void *) arg, sizeof(sco_info)))
            {
                BTUSB_ERR("BTUSB_IOCTL_ADD_VOICE_CHANNEL failed getting 1st par\n");
                retval = -EFAULT;
                goto err_out;
            }
            return btusb_remove_voice_channel(p_dev, sco_info.handle);

        case TCGETS:
            /* dummy support of TTY IOCTLs */
            if (!arg)
            {
                retval = -EFAULT;
                goto err_out;
            }
#ifndef TCGETS2
            if (kernel_termios_to_user_termios((struct termios __user *)arg, &p_dev->kterm))
#else
            if (kernel_termios_to_user_termios_1((struct termios __user *)arg, &p_dev->kterm))
#endif
            {
                BTUSB_ERR("failure copying termios\n");
                retval = -EFAULT;
                goto err_out;
            }
            break;

        case TCSETSW:
        case TCSETS:
            if (!arg)
            {
                retval = -EFAULT;
                goto err_out;
            }
#ifndef TCGETS2
            if (user_termios_to_kernel_termios(&p_dev->kterm, (struct termios __user *)arg))
#else
            if (user_termios_to_kernel_termios_1(&p_dev->kterm, (struct termios __user *)arg))
#endif
            {
                retval = -EFAULT;
                goto err_out;
            }
            break;

        case TCSETSF:
        case TCGETA:
        case TCSETA:
        case TCSETAW:
        case TCSETAF:
        case TCSBRK:
        case TCXONC:
        case TCFLSH:
        case TIOCGSOFTCAR:
        case TIOCSSOFTCAR:
        case TIOCGLCKTRMIOS:
        case TIOCSLCKTRMIOS:
#ifdef TIOCGETP
        case TIOCGETP:
        case TIOCSETP:
        case TIOCSETN:
#endif
#ifdef TIOCGETC
        case TIOCGETC:
        case TIOCSETC:
#endif
#ifdef TIOCGLTC
        case TIOCGLTC:
        case TIOCSLTC:
#endif
#ifdef TCGETX
        case TCGETX:
        case TCSETX:
        case TCSETXW:
        case TCSETXF:
#endif
        case TIOCMSET:
        case TIOCMBIC:
        case TIOCMBIS:
            /* dummy support of TTY IOCTLs */
            BTUSB_DBG("TTY ioctl not implemented\n");
            break;

        case TIOCGSERIAL:
        {
            struct serial_struct tmp;
            if (!arg)
            {
                retval = -EFAULT;
                goto err_out;
            }
            memset(&tmp, 0, sizeof(tmp));
            tmp.type = 0;
            tmp.line = 0;
            tmp.port = 0;
            tmp.irq = 0;
            tmp.flags = ASYNC_SKIP_TEST | ASYNC_AUTO_IRQ;
            tmp.xmit_fifo_size = 4096;
            tmp.baud_base = 115200;
            tmp.close_delay = 5*HZ;
            tmp.closing_wait = 30*HZ;
            tmp.custom_divisor = 0;
            tmp.hub6 = 0;
            tmp.io_type = 0;
            if (copy_to_user((void __user *)arg, &tmp, sizeof(tmp)))
            {
                retval = -EFAULT;
                goto err_out;
            }
            retval = 0;
            break;
        }

        case TIOCMGET:
        {
            int tiocm = TIOCM_RTS | TIOCM_CTS;
            if (!arg)
            {
                retval = -EFAULT;
                goto err_out;
            }

            if (copy_to_user((void __user *)arg, &tiocm, sizeof(tiocm)))
            {
                retval = -EFAULT;
                goto err_out;
            }
            retval = 0;
            break;
        }

        case TIOCMIWAIT:
        {
            DECLARE_WAITQUEUE(wait, current);

            BTUSB_DBG("arg = %lu\n", arg);
            while (1)
            {
                BTUSB_DBG("adding task to wait list\n");
                add_wait_queue(&p_dev->rx_wait_q, &wait);
                set_current_state(TASK_INTERRUPTIBLE);
                schedule();
                BTUSB_DBG("removing task from wait list\n");
                remove_wait_queue(&p_dev->rx_wait_q, &wait);
                /* see if a signal woke us up */
                if (signal_pending(current))
                {
                    BTUSB_ERR("signal was pending\n");
                    retval = -ERESTARTSYS;
                    goto err_out;
                }
                /* do not check the expected signals */
                retval = 0;
                break;
            }
            break;
        }
        default:
            BTUSB_ERR("unknown cmd %u\n", cmd);
            retval = -ENOIOCTLCMD;
            break;
    }

err_out:
    BTUSB_DBG("returning %d\n", retval);
    return retval;
}

/*******************************************************************************
 **
 ** Function         btusb_urb_out_complete
 **
 ** Description      Data write (bulk pipe) completion routine
 **
 ** Parameters       urb pointer
 **
 ** Returns          void
 **
 *******************************************************************************/
void btusb_urb_out_complete(struct urb *p_urb)
{
    struct btusb_tx_trans *p_tx_trans = p_urb->context;
    struct btusb *p_dev = p_tx_trans->p_dev;

    BTUSB_DBG("status %d length %u flags %x\n", p_urb->status,
            p_urb->transfer_buffer_length, p_urb->transfer_flags);

    p_tx_trans->in_use = false;

    p_dev->stats.urb_out_complete++;
    if (unlikely(p_urb->status))
    {
        /* this error can happen when unplugging */
        p_dev->stats.urb_out_complete_err++;
    }

    if (p_tx_trans->complete)
    {
        p_tx_trans->complete(p_dev, p_tx_trans, p_urb);
    }

    if (autopm)
    {
        usb_autopm_put_interface(p_dev->p_main_intf);
    }

    return;
}

/*******************************************************************************
 **
 ** Function         btusb_submit_acl
 **
 ** Parameters       p_dev: device control block reference
 **                  packet: HCI ACL packet to transmit
 **                  length: length of the HCI packet
 **
 ** Returns          0 upon success, negative value if error
 **
 *******************************************************************************/
int btusb_submit_acl(struct btusb *p_dev, char *packet, unsigned long length)
{
    int retval;
    struct btusb_tx_trans *p_tx_trans;

    BTUSB_DBG("%p[%lu]\n", packet, length);

    btusb_dump_data(packet, length, "OUTGOING DATA");

    p_tx_trans = btusb_alloc_tx_trans(p_dev->acl_tx_array, ARRAY_SIZE(p_dev->acl_tx_array));
    if (unlikely(!p_tx_trans))
    {
        return -ENOMEM;
    }

    if (unlikely(length > BTUSB_HCI_MAX_ACL_SIZE))
    {
        retval = -E2BIG;
        goto error;
    }

    memcpy(p_tx_trans->dma_buffer, packet, length);
    p_tx_trans->p_urb->transfer_buffer_length = length;

#if defined(BTUSB_LITE)
    retval = btusb_submit(p_dev, &p_dev->acl_tx_submitted, p_tx_trans->p_urb, GFP_ATOMIC);
#else
    retval = btusb_submit(p_dev, &p_dev->acl_tx_submitted, p_tx_trans->p_urb, GFP_KERNEL);
#endif
    if (unlikely(retval))
    {
        goto error;
    }
    return retval;

error:
    BTUSB_ERR("failed : %d\n", retval);
    p_tx_trans->in_use = false;
    return retval;
}

/*******************************************************************************
 **
 ** Function         btusb_submit_diag
 **
 ** Parameters       p_dev: device control block reference
 **                  packet: HCI DIAG packet to transmit
 **                  length: length of the packet
 **
 ** Returns          0 upon success, negative value if error
 **
 *******************************************************************************/
int btusb_submit_diag(struct btusb *p_dev, char *packet, unsigned long length)
{
    int retval;
    struct btusb_tx_trans *p_tx_trans;

    BTUSB_DBG("%p[%lu]\n", packet, length);

    btusb_dump_data(packet, length, "OUTGOING DIAG");
    if (unlikely(!p_dev->p_diag_out))
    {
        /* some controllers have no DIAG interface */
        return 0;
    }

    p_tx_trans = btusb_alloc_tx_trans(p_dev->diag_tx_array, ARRAY_SIZE(p_dev->diag_tx_array));
    if (unlikely(!p_tx_trans))
    {
        return -ENOMEM;
    }

    if (unlikely(length > BTUSB_HCI_MAX_ACL_SIZE))
    {
        retval = -E2BIG;
        goto error;
    }

    memcpy(p_tx_trans->dma_buffer, packet, length);
    p_tx_trans->p_urb->transfer_buffer_length = length;

    retval = btusb_submit(p_dev, &p_dev->diag_tx_submitted, p_tx_trans->p_urb, GFP_KERNEL);
    if (unlikely(retval))
    {
        goto error;
    }
    return retval;

error:
    BTUSB_ERR("failed : %d\n", retval);
    p_tx_trans->in_use = false;
    return retval;
}


/*******************************************************************************
 **
 ** Function         btusb_cmd_complete
 **
 ** Description      Command (control pipe) completion routine
 **
 ** Parameters       p_dev: device control block reference
 **
 ** Returns          void.
 **
 *******************************************************************************/
void btusb_cmd_complete(struct btusb *p_dev, struct btusb_tx_trans *p_tx_trans, struct urb *p_urb)
{
    p_dev->stats.cmd_complete++;
    if (unlikely(p_urb->status))
    {
        p_dev->stats.cmd_complete_err++;
    }
}

/*******************************************************************************
 **
 ** Function         btusb_submit_cmd
 **
 ** Parameters       p_dev: device control block reference
 **                  packet: HCI CMD packet to transmit
 **                  length: length of the HCI packet
 **
 ** Returns          0 upon success, negative value if error
 **
 *******************************************************************************/
int btusb_submit_cmd(struct btusb *p_dev, char *packet, unsigned long length)
{
    int retval;
    struct btusb_tx_trans *p_tx_trans;
    struct usb_ctrlrequest *p_dr = NULL;

    BTUSB_DBG("%p[%lu]\n", packet, length);

    btusb_dump_data(packet, length, "OUTGOING CMD");

    p_tx_trans = btusb_alloc_tx_trans(p_dev->cmd_array, ARRAY_SIZE(p_dev->cmd_array));
    if (unlikely(!p_tx_trans))
    {
        return -ENOMEM;
    }

    if (unlikely(length > BTUSB_HCI_MAX_CMD_SIZE))
    {
        retval = -E2BIG;
        goto error;
    }

    memcpy(p_tx_trans->dma_buffer, packet, length);
    p_dr = (void *)p_tx_trans->p_urb->setup_packet;
    p_dr->wLength = __cpu_to_le16(length);
    p_tx_trans->p_urb->transfer_buffer_length = length;

    retval = btusb_submit(p_dev, &p_dev->cmd_submitted, p_tx_trans->p_urb, GFP_KERNEL);
    if (unlikely(retval))
    {
        p_dev->stats.cmd_submit_err++;
        goto error;
    }
    else
    {
        p_dev->stats.cmd_submit_ok++;
    }
    return retval;

error:
    BTUSB_ERR("failed : %d\n", retval);
    p_tx_trans->in_use = false;
    return retval;
}

/*******************************************************************************
 **
 ** Function         btusb_voicetx_complete
 **
 ** Description      Voice write (iso pipe) completion routine.
 **
 ** Parameters       p_dev: device control block reference
 **                  p_tx_trans: pointer to the TX transaction
 **                  p_urb: pointer to the URB
 **
 ** Returns          void
 **
 *******************************************************************************/
void btusb_voicetx_complete(struct btusb *p_dev, struct btusb_tx_trans *p_tx_trans, struct urb *p_urb)
{
    p_dev->stats.voicetx_complete++;
    if (unlikely(p_urb->status))
    {
        p_dev->stats.voicetx_complete_err++;
    }

    if (unlikely(dbgflags & BTUSB_VOICETX_TIME))
    {
        btusb_voice_stats(&(p_dev->stats.voice_max_tx_done_delta_time), &(p_dev->stats.voice_min_tx_done_delta_time),
                &(p_dev->stats.voice_tx_done_delta_time), &(p_dev->stats.voice_last_tx_done_ts));
    }

    atomic_sub(p_urb->number_of_packets, &p_dev->voice_tx_active);
}

/*******************************************************************************
 **
 ** Function         btusb_submit_voice_tx
 **
 ** Description      Voice write submission
 **
 ** Parameters       p_dev: device control block reference
 **                  packet: HCI SCO packet to transmit
 **                  length: length of the HCI packet
 **
 ** Returns          0 upon success, negative value if error
 **
 *******************************************************************************/
int btusb_submit_voice_tx(struct btusb *p_dev, char *packet, unsigned long length)
{
    int i, retval;
    unsigned int packet_size, pkt_used, total_size;
    unsigned long remain;
    unsigned char to_send;
    struct btusb_tx_trans *p_tx_trans;
    struct usb_iso_packet_descriptor *p_ifd;
    char *p_cur;

    if (!p_dev->p_main_intf || !p_dev->p_voice_intf || !p_dev->p_voice_out)
    {
        BTUSB_DBG(" failing (p_dev removed or no voice intf)\n");
        return -ENODEV;
    }

    if (unlikely(dbgflags & BTUSB_VOICETX_TIME))
    {
        btusb_voice_stats(&(p_dev->stats.voice_max_rx_feeding_interval), &(p_dev->stats.voice_min_rx_feeding_interval),
                &(p_dev->stats.voice_rx_feeding_interval), &(p_dev->stats.voice_last_rx_feeding_ts));
    }

    packet_size = le16_to_cpu(p_dev->p_voice_out->desc.wMaxPacketSize);
    if (unlikely(!packet_size))
    {
        BTUSB_ERR("Max Packet Size = 0\n");
        return -EINVAL;
    }

    if (unlikely(length < BTUSB_VOICE_HEADER_SIZE))
    {
        BTUSB_ERR("SCO packet length is too short\n");
        return -EINVAL;
    }

    resend:
    to_send = 0;
    p_cur = packet + BTUSB_VOICE_HEADER_SIZE;
    remain = length - BTUSB_VOICE_HEADER_SIZE;
    while (remain)
    {
        /* find a free transaction */
        p_tx_trans = btusb_alloc_tx_trans(p_dev->voice_tx_array, ARRAY_SIZE(p_dev->voice_tx_array));
        if (unlikely(!p_tx_trans))
        {
            p_dev->stats.voicetx_nobuf++;
            BTUSB_ERR("No transaction available\n");
            return -ENOMEM;
        }

        total_size = 0;
        for (i = 0, p_ifd = &p_tx_trans->p_urb->iso_frame_desc[0];
                i < BTUSB_VOICE_FRAMES_PER_URB; i++, p_ifd++)
        {
            if (unlikely(!to_send))
            {
                if (!remain)
                {
                    break;
                }
                else
                {
                    if (remain >= BTUSB_VOICE_BURST_SIZE)
                    {
                        to_send = BTUSB_VOICE_BURST_SIZE;
                    }
                    else
                    {
                        BTUSB_DBG("Sending partial sco data len: %lu\n", remain);
                        to_send = remain;
                    }
                    /* remaining data computed before adding header*/
                    remain -= to_send;

                    /* rebuild a SCO header, copying the full packet header */
                    p_cur -= BTUSB_VOICE_HEADER_SIZE;
                    p_cur[0] = packet[0];
                    p_cur[1] = packet[1];
                    p_cur[2] = to_send;
                    to_send += BTUSB_VOICE_HEADER_SIZE;
                }
            }
            if (likely(to_send > packet_size))
            {
                pkt_used = packet_size;
            }
            else
            {
                pkt_used = to_send;
            }
            memcpy(&p_tx_trans->dma_buffer[p_ifd->offset], p_cur, pkt_used);
            p_cur += pkt_used;
            to_send -= pkt_used;
            p_ifd->length = pkt_used;
            total_size += pkt_used;
        }
        if (likely(i))
        {
            p_tx_trans->p_urb->number_of_packets = i;

            retval = btusb_submit(p_dev, &p_dev->voice_tx_submitted, p_tx_trans->p_urb, GFP_KERNEL);
            if (unlikely(retval))
            {
                p_tx_trans->in_use = false;
                p_dev->stats.voicetx_submit_err++;
            }
            else
            {
                atomic_add(i, &p_dev->voice_tx_active);
                p_dev->stats.voicetx_submit_ok++;
            }
        }
        else
        {
            p_tx_trans->in_use = false;
        }
    }

    /* if there are less than 30 ms of data, send the same data again */
    if (atomic_read(&p_dev->voice_tx_active) < 30)
    {
        BTUSB_DBG("less than 30 ms of data available, resending packet");
        goto resend;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function         btusb_fill_isoc_pkts
 **
 ** Description      Prepare the ISOC packet descriptors
 **
 ** Parameters       p_urb: pointer to the URB
 **                  len: total length of the URB buffer
 **                  pkt_size: ISOC transfer max packet size
 **
 ** Returns          void
 **
 *******************************************************************************/
static void btusb_fill_isoc_pkts(struct urb *p_urb, int len, int pkt_size)
{
    int off = 0, i;

    for (i = 0; len >= pkt_size; i++, off += ALIGN(pkt_size, 4), len -= ALIGN(pkt_size, 4))
    {
        p_urb->iso_frame_desc[i].offset = off;
        p_urb->iso_frame_desc[i].length = pkt_size;
        BTUSB_DBG("off=%d pkt_size=%d\n", off, pkt_size);
    }
    p_urb->number_of_packets = i;
}

/*******************************************************************************
 **
 ** Function         btusb_submit_voice_rx
 **
 ** Description      Submit a voice RX URB
 **
 ** Parameters       p_dev: device control block reference
 **                  p_urb: URB reference
 **                  mem_flags: memory protection flags
 **
 ** Returns
 **
 *******************************************************************************/
void btusb_submit_voice_rx(struct btusb *p_dev, struct urb *p_urb, int mem_flags)
{
    if (unlikely(btusb_submit(p_dev, &p_dev->voice_rx_submitted, p_urb, mem_flags)))
    {
        p_dev->stats.voicerx_submit_err++;
    }
    else
    {
        p_dev->stats.voicerx_submit_ok++;
    }
}

/*******************************************************************************
 **
 ** Function         btusb_update_voice_trans
 **
 ** Description      Finish filling the voice URB and submit them.  At this point
 **                  the URBs should NOT be submitted.
 **
 ** Parameters       p_dev: device control block reference
 **
 ** Returns          0 upon success, error code otherwise
 **
 *******************************************************************************/
static int btusb_update_voice_trans(struct btusb *p_dev)
{
    unsigned int idx;
    struct btusb_rx_trans *p_rx_trans;
    struct btusb_tx_trans *p_tx_trans;
    struct urb *p_urb;
    int packet_size;

    BTUSB_DBG("enter\n");

    if (!p_dev->p_main_intf || !p_dev->p_voice_intf || !p_dev->p_voice_in)
    {
        BTUSB_ERR("failing (p_dev removed or no voice intf)\n");
        return -EFAULT;
    }

    packet_size = le16_to_cpu(p_dev->p_voice_in->desc.wMaxPacketSize);
    if (!packet_size)
    {
        BTUSB_ERR("Max Packet Size = 0\n");
        return -EINVAL;
    }
    BTUSB_DBG("packet_size=%d\n", packet_size);

    BTUSB_ARRAY_FOR_EACH_RX_TRANS(p_dev->voice_rx_array)
    {
        p_urb = p_rx_trans->p_urb;
        p_urb->pipe = usb_rcvisocpipe(p_dev->p_udev, p_dev->p_voice_in->desc.bEndpointAddress);
        BTUSB_DBG("ep=%d\n", p_dev->p_voice_in->desc.bEndpointAddress);
        p_urb->interval = p_dev->p_voice_in->desc.bInterval;
        BTUSB_DBG("interval=%d\n", p_urb->interval);
        p_urb->transfer_buffer_length = ALIGN(packet_size, 4) * BTUSB_VOICE_FRAMES_PER_URB;
        BTUSB_DBG("transfer_buffer_length=%d\n", p_urb->transfer_buffer_length);
        if (p_urb->transfer_buffer_length > BTUSB_VOICE_BUFFER_MAXSIZE)
        {
            BTUSB_ERR("rx voice allocated insufficient for MaxPacketSize\n");
            p_urb->transfer_buffer_length = BTUSB_VOICE_BUFFER_MAXSIZE;
            return -ENOMEM;
        }
        btusb_fill_isoc_pkts(p_urb, p_urb->transfer_buffer_length, packet_size);

        btusb_submit_voice_rx(p_dev, p_urb, GFP_KERNEL);
    }

    packet_size = le16_to_cpu(p_dev->p_voice_out->desc.wMaxPacketSize);
    if (!packet_size)
    {
        BTUSB_ERR("Max Packet Size = 0\n");
        return -EINVAL;
    }
    BTUSB_ARRAY_FOR_EACH_TX_TRANS(p_dev->voice_tx_array)
    {
        p_urb = p_tx_trans->p_urb;
        p_urb->pipe = usb_sndisocpipe(p_dev->p_udev, p_dev->p_voice_out->desc.bEndpointAddress);
        BTUSB_DBG("ep=%d\n", p_dev->p_voice_out->desc.bEndpointAddress);
        p_urb->interval = p_dev->p_voice_out->desc.bInterval;
        BTUSB_DBG("interval=%d\n", p_urb->interval);
        p_urb->transfer_buffer_length = ALIGN(packet_size, 4) * BTUSB_VOICE_FRAMES_PER_URB;
        if (p_urb->transfer_buffer_length > BTUSB_VOICE_BUFFER_MAXSIZE)
        {
            BTUSB_ERR("tx voice allocated insufficient for MaxPacketSize\n");
            p_urb->transfer_buffer_length = BTUSB_VOICE_BUFFER_MAXSIZE;
            return -ENOMEM;
        }
        btusb_fill_isoc_pkts(p_urb, p_urb->transfer_buffer_length, packet_size);
    }

    /* at this point, initiate voice TX */
    for (idx = 0; idx < ARRAY_SIZE(p_dev->voice_rx.channels); idx++)
    {
        struct btusb_voice_channel *p_chan = &p_dev->voice_rx.channels[idx];
        if (p_chan->used)
        {
            UINT8 data[BTUSB_VOICE_HEADER_SIZE + BTUSB_VOICE_BURST_SIZE];
            UINT8 *p_data = data;
            memset(data, 0, sizeof(data));
            UINT16_TO_STREAM(p_data, p_chan->handle);
            UINT8_TO_STREAM(p_data, BTUSB_VOICE_BURST_SIZE);
            btusb_submit_voice_tx(p_dev, data, sizeof(data));
        }
    }
    return 0;
}


/*******************************************************************************
 **
 ** Function         btusb_set_voice
 **
 ** Description      Change voice interface setting processor
 **                  NOTE: Must be called at low execution level
 **
 ** Parameters       p_dev: device control block reference
 **                  setting_number: new voice interface setting number
 **                  submit_urb: true if the URBs must be submitted
 **
 ** Returns          0 upon success, error code otherwise
 **
 *******************************************************************************/
static int btusb_set_voice(struct btusb *p_dev, unsigned char setting_number,
        bool submit_urb)
{
    int idx;
    struct usb_host_interface *p_host_intf;
    struct usb_endpoint_descriptor *p_ep_desc;

    BTUSB_DBG("setting_number=%d submit_urb=%u\n", setting_number, submit_urb);

    /* cancel all voice requests before switching buffers */
    p_dev->p_voice_in = NULL;
    p_dev->p_voice_out = NULL;
    btusb_cancel_voice(p_dev);

    /* configure the voice interface to the proper setting */
    if (usb_set_interface(p_dev->p_udev, 1, setting_number))
    {
        BTUSB_ERR("failed to set iso intf to %u\n", setting_number);
        return -EFAULT;
    }

    /* find the endpoints */
    p_host_intf = p_dev->p_voice_intf->cur_altsetting;

    for (idx = 0; idx < p_host_intf->desc.bNumEndpoints; idx++)
    {
        p_ep_desc = &p_host_intf->endpoint[idx].desc;
        if (usb_endpoint_type(p_ep_desc) == USB_ENDPOINT_XFER_ISOC)
        {
            if (usb_endpoint_dir_in(p_ep_desc))
            {
                p_dev->p_voice_in = &p_host_intf->endpoint[idx];
            }
            else
            {
                p_dev->p_voice_out = &p_host_intf->endpoint[idx];
            }
        }
    }

    if (!p_dev->p_voice_in || !p_dev->p_voice_out)
    {
        BTUSB_ERR("no iso pipes found!\n");
        return -EFAULT;
    }

    if (submit_urb)
    {
        return btusb_update_voice_trans(p_dev);
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function         btusb_add_voice_channel
 **
 ** Description      Add a voice channel in the list of current channels
 **
 ** Parameters       p_dev: device control block reference
 **                  sco_handle: handle of the synchronous connection carrying voice
 **                  burst: size of the voice bursts
 **
 ** Returns          Return 0 upon success, error code otherwise
 **
 *******************************************************************************/
int btusb_add_voice_channel(struct btusb *p_dev, unsigned short sco_handle, unsigned char burst)
{
    int idx;
    struct btusb_voice_channel *p_chan;

    if (!p_dev->p_voice_intf)
    {
        BTUSB_ERR("No voice interface detected\n");
        return -EOPNOTSUPP;
    }

    /* kludge to support the backward compatibility with older BTKRNLs
       not supplying the packet size... */
    if (burst == 0)
    {
        BTUSB_INFO("fixing legacy req to 48\n");
        burst = BTUSB_VOICE_BURST_SIZE;
    }

    /* look for an available voice channel entry */
    for (idx = 0; idx < ARRAY_SIZE(p_dev->voice_rx.channels); idx++)
    {
        p_chan = &p_dev->voice_rx.channels[idx];
        if (!p_chan->used)
        {
            p_chan->handle = sco_handle;
            p_chan->burst = burst;
            p_chan->used = true;
            goto found;
        }
    }
    BTUSB_ERR("Could not find empty slot in internal tables\n");
    return -EMLINK;

found:
    if (btusb_update_voice_channels(p_dev))
    {
        BTUSB_ERR("Failed adding voice channel\n");
        /* failure, remove the channel just added */
        btusb_remove_voice_channel(p_dev, sco_handle);
        return -ENOMEM;
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function         btusb_remove_voice_channel
 **
 ** Description      Remove a voice channel from the list of current channels
 **
 ** Parameters       p_dev: device control block reference
 **                  sco_handle: handle of the synchronous connection carrying voice
 **
 ** Returns          Return 0 upon success, error code otherwise
 **
 *******************************************************************************/
int btusb_remove_voice_channel(struct btusb *p_dev, unsigned short sco_handle)
{
    int idx;
    struct btusb_voice_channel *p_chan;

    if (!p_dev->p_voice_intf)
    {
        BTUSB_ERR("No voice interface detected\n");
        return -EOPNOTSUPP;
    }

    /* find the channel to be removed */
    for (idx = 0; idx < ARRAY_SIZE(p_dev->voice_rx.channels); idx++)
    {
        p_chan = &p_dev->voice_rx.channels[idx];
        if (p_chan->used && (p_chan->handle == sco_handle))
        {
            p_chan->used = false;
            goto found;
        }
    }
    BTUSB_ERR("Could not find SCO handle in internal tables\n");
    return -ENOENT;

found:
    return btusb_update_voice_channels(p_dev);
}

/*******************************************************************************
 **
 ** Function         btusb_update_voice_channels
 **
 ** Description      Voice channels just updated, reconfigure
 **
 ** Parameters       p_dev: device control block reference
 **
 ** Returns          Return 0 upon success, error code otherwise
 **
 *******************************************************************************/
static int btusb_update_voice_channels(struct btusb *p_dev)
{
    int idx, jdx;
    unsigned char min_burst, max_burst, num_voice_chan, voice_setting;
    unsigned short desired_packet_size, packet_size;
    struct btusb_voice_channel *p_chan;
    struct usb_host_interface *p_host_intf;
    struct usb_endpoint_descriptor *p_ep_desc;

    BTUSB_DBG("\n");

    /* get the number of voice channels and the size information */
    num_voice_chan = 0;
    min_burst = 0xFF;
    max_burst = 0;
    for (idx = 0; idx < ARRAY_SIZE(p_dev->voice_rx.channels); idx++)
    {
        p_chan = &p_dev->voice_rx.channels[idx];
        if (p_chan->used)
        {
            num_voice_chan++;
            min_burst = min(min_burst, p_chan->burst);
            max_burst = max(max_burst, p_chan->burst);
        }
    }

    BTUSB_DBG("num_voice_chan=%d\n", num_voice_chan);
    /* now calculate a desired_packet_size */
    switch (num_voice_chan)
    {
    case 0:
        desired_packet_size = 0;
        break;

    case 1:
        /* single channel: we just need a third of the length
           (rounded up so we add 2 before dividing) */
        desired_packet_size = ((max_burst + BTUSB_VOICE_HEADER_SIZE) + 2) / 3;
        break;

    case 2:
        /* two channels: we need the smaller one to fit in completely
           and the larger one to fit in into two... */
        packet_size = (max_burst + BTUSB_VOICE_HEADER_SIZE + 1) / 2;

        desired_packet_size = min_burst + BTUSB_VOICE_HEADER_SIZE;
        if (packet_size > desired_packet_size)
            desired_packet_size = packet_size;
        break;

    case 3:
        /* three channels - we need all of them to fit into a single packet */
        desired_packet_size = max_burst + BTUSB_VOICE_HEADER_SIZE;
        break;

    default:
        /* this can not happen */
        BTUSB_ERR("invalid # (%d) of channels, failing...\n", num_voice_chan);
        return 0;
    }

    BTUSB_DBG("desired packet size is %u\n", desired_packet_size);

    /* now convert the desired_packet_size into the interface setting number */
    packet_size = BTUSB_USHRT_MAX;
    voice_setting = 0;
    for (idx = 0; idx < p_dev->p_voice_intf->num_altsetting; idx++)
    {
        p_host_intf = &p_dev->p_voice_intf->altsetting[idx];
        for (jdx = 0; jdx < p_host_intf->desc.bNumEndpoints; jdx++)
        {
            p_ep_desc = &p_host_intf->endpoint[jdx].desc;
            if ((usb_endpoint_type(p_ep_desc) == USB_ENDPOINT_XFER_ISOC) &&
                usb_endpoint_dir_in(p_ep_desc))
            {
                /* if the MaxPacketSize is large enough and if it is smaller
                   than the current setting */
                if ((desired_packet_size <= le16_to_cpu(p_ep_desc->wMaxPacketSize)) &&
                    (le16_to_cpu(p_ep_desc->wMaxPacketSize) < packet_size))
                {
                    packet_size = le16_to_cpu(p_ep_desc->wMaxPacketSize);
                    voice_setting = p_host_intf->desc.bAlternateSetting;
                }
            }
        }
    }
    if (packet_size == BTUSB_USHRT_MAX)
    {
        BTUSB_ERR("no appropriate ISO interface setting found, failing...\n");
        return -ERANGE;
    }

    BTUSB_DBG("desired_packet_size=%d voice_setting=%d\n", desired_packet_size, voice_setting);

    /* set the voice setting and only submit the URBs if there is a channel */
    return btusb_set_voice(p_dev, voice_setting, num_voice_chan != 0);
}
