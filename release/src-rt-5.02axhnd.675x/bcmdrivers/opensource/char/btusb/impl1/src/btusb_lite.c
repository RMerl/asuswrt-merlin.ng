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
 
#include <linux/proc_fs.h>
#include <linux/poll.h>

#include "btusb.h"
#include "btpcm_api.h"
#include "bd.h"


/*
 * Defines
 */

#define BTUSB_LITE_IPC_AVDT_SYNC_INFO_RSP_LEN    (1 + BTM_SYNC_INFO_NUM_STR * (1+6+2+2+1+2))

/*
 * Local functions
 */
static void btusb_lite_reinit(struct btusb *p_dev);
static int btusb_lite_open(struct inode *inode, struct file *file);
static int btusb_lite_close(struct inode *inode, struct file *file);
static ssize_t btusb_lite_write(struct file *file, const char *buf, size_t count,
        loff_t *p_off);
static ssize_t btusb_lite_read(struct file *file, char __user *buffer, size_t count,
        loff_t *p_off);
static unsigned int btusb_lite_poll(struct file *file, struct poll_table_struct *p_pt);

static BT_HDR *btusb_lite_msg_to_app_get(struct btusb *p_dev);
static void btusb_lite_msg_to_app_free(struct btusb *p_dev, BT_HDR *p_msg);
static UINT8 *btusb_lite_msg_to_app_get_data_addr(struct btusb *p_dev, BT_HDR *p_msg);

static char *btusb_lite_ipc_event_desc(UINT16 event);

static void btusb_lite_ipc_hndl(struct btusb *p_dev, BT_HDR *p_msg);
static void btusb_lite_ipc_hci_cmd_hndl(struct btusb *p_dev, BT_HDR *p_msg);
static void btusb_lite_ipc_hci_acl_hndl(struct btusb *p_dev, BT_HDR *p_msg);
static void btusb_lite_ipc_mgt_hndl(struct btusb *p_dev, BT_HDR *p_msg);
static void btusb_lite_ipc_btu_hndl(struct btusb *p_dev, BT_HDR *p_msg);
static void btusb_lite_ipc_btm_hndl(struct btusb *p_dev, BT_HDR *p_msg);
static void btusb_lite_ipc_l2c_hndl(struct btusb *p_dev, BT_HDR *p_msg);
static void btusb_lite_ipc_avdt_hndl(struct btusb *p_dev, BT_HDR *p_msg);

static void btusb_lite_ipc_rsp_send(struct btusb *p_dev, UINT16 event, UINT8 op_code,
        UINT8 *p_param, UINT8 param_len);
static void btusb_lite_ipc_cmd_cplt_evt_send(struct btusb *p_dev,
        UINT16 opcode, UINT8 *p_param, UINT8 param_len);
static void btusb_lite_ipc_avdt_sync_info_send(struct btusb *p_dev,
        tAVDT_SYNC_INFO *p_sync_rsp);

static void btusb_lite_ipc_sent_to_user(struct btusb *p_dev, BT_HDR *p_msg);

/*
 * Globals
 */
static const struct file_operations btusb_lite_fops =
{
    .open = btusb_lite_open,
    .read = btusb_lite_read,
    .poll = btusb_lite_poll,
    .write = btusb_lite_write,
    .release = btusb_lite_close,
};

/*******************************************************************************
 **
 ** Function        btusb_lite_reinit
 **
 ** Description     Init BTUSB Lite interface
 **
 ** Returns         None
 **
 *******************************************************************************/
static void btusb_lite_reinit(struct btusb *p_dev)
{
    memset(&p_dev->lite_cb.s, 0, sizeof(p_dev->lite_cb.s));
}

/*******************************************************************************
 **
 ** Function        btusb_lite_open
 **
 ** Description     Open BTUSB Lite interface
 **
 ** Returns         status (< 0 if error)
 **
 *******************************************************************************/
static int btusb_lite_open(struct inode *inode, struct file *file)
{
    int rv;
    struct btusb *p_dev = BTUSB_PDE_DATA(inode);

    BTUSB_INFO("btusb_lite_open\n");

    if (!p_dev)
    {
        BTUSB_ERR("Unable to find p_dev reference\n");
        rv = -ENODEV;
        goto out;
    }

    if (p_dev->lite_cb.s.opened)
    {
        BTUSB_ERR("Lite interface already opened\n");
        rv = -EBUSY;;
        goto out;
    }

    file->private_data = p_dev;     /* Save our private p_dev */

    btusb_lite_reinit(p_dev);
    p_dev->lite_cb.s.opened = true;

    rv = 0;

out:
    return rv;
}

/*******************************************************************************
 **
 ** Function        btusb_lite_close
 **
 ** Description     Close BTUSB Lite interface
 **
 ** Returns         status (< 0 if error)
 **
 *******************************************************************************/
static int btusb_lite_close(struct inode *inode, struct file *file)
{
    int rv;
    struct btusb *p_dev = BTUSB_PDE_DATA(inode);

    BTUSB_INFO("btusb_lite_close\n");

    if (!p_dev)
    {
        BTUSB_ERR("Unable to find p_dev reference\n");
        rv = -ENODEV;
        goto out;
    }

    if (!p_dev->lite_cb.s.opened)
    {
        BTUSB_ERR("Lite interface was not opened\n");
        rv = -EBUSY;;
        goto out;
    }

    btusb_lite_reinit(p_dev);

    rv = 0;

out:
    return rv;
}

/*******************************************************************************
 **
 ** Function        btusb_lite_write
 **
 ** Description     Write handler of the BTUSB Lite interface
 **
 ** Returns         Nb bytes written
 **
 *******************************************************************************/
static ssize_t btusb_lite_write(struct file *file, const char *p_user_buffer,
        size_t count, loff_t *p_off)
{
    struct btusb *p_dev = file->private_data;
    size_t remainder;
    struct btusb_lite_cb *p_lite_cb;
    unsigned long copy_len;
    ssize_t copied_len;
    UINT8 *p;
    UINT16 rx_event;
    BT_HDR *p_msg;

    BTUSB_DBG("btusb_lite_write count=%zu\n", count);

    if (!p_dev)
    {
        BTUSB_ERR("Unable to find p_dev reference\n");
        return -ENODEV;
    }

    if (!p_dev->lite_cb.s.opened)
    {
        BTUSB_ERR("Lite interface was not opened\n");
        return -EBUSY;
    }

    if (unlikely(count == 0))
    {
        return 0;
    }

    /* check that the incoming data is good */
    if (unlikely(!access_ok(VERIFY_READ, (void *)p_user_buffer, count)))
    {
        BTUSB_ERR("buffer access verification failed\n");
        return -EFAULT;
    }

    p_lite_cb = &p_dev->lite_cb;

    copied_len = 0;
    remainder = count;

    while(remainder)
    {
        /* If a full Header has not yet been received */
        if (p_lite_cb->s.from_app.rx_header_len < BTUSB_LITE_IPC_HDR_SIZE)
        {
            /* How many bytes are needed (1 to 4) */
            copy_len = BTUSB_LITE_IPC_HDR_SIZE - p_lite_cb->s.from_app.rx_header_len;

            /* If less bytes are available */
            if (remainder < copy_len)
            {
                copy_len = remainder;
            }

            /* Copy the header (or a part of it) */
            if (copy_from_user(&p_lite_cb->s.from_app.rx_header[p_lite_cb->s.from_app.rx_header_len],
                    (void *)p_user_buffer, copy_len))
            {
                BTUSB_ERR("Copy header from user error\n");
                return -EINVAL;
            }
            remainder -= copy_len;
            p_lite_cb->s.from_app.rx_header_len += copy_len;
            p_user_buffer += copy_len;
            copied_len += copy_len;

            /* If the buffer has been read */
            if (p_lite_cb->s.from_app.rx_header_len == BTUSB_LITE_IPC_HDR_SIZE)
            {
                p = p_lite_cb->s.from_app.rx_header;
                STREAM_TO_UINT16(p_lite_cb->s.from_app.rx_len, p);
                STREAM_TO_UINT16(rx_event, p);

                BTUSB_DBG("Rx Len=%d RxEvent=0x%X\n", p_lite_cb->s.from_app.rx_len, rx_event);

                p_lite_cb->s.from_app.rx_len -= sizeof(UINT16);    /* Do not count Event Field */

                /* get a buffer from the pool (add one byte for HCI_Type) */
                if (unlikely((p_msg = (BT_HDR *)GKI_getbuf(sizeof(BT_HDR) + p_lite_cb->s.from_app.rx_len + 1)) == NULL))
                {
                    BTUSB_ERR("unable to get GKI buffer - write failed\n");
                    return -EINVAL;
                }
                p_msg->event = rx_event;
                p_msg->layer_specific = 0;
                p_msg->offset = 1;
                p_msg->len = 0;

                p_lite_cb->s.from_app.p_rx_msg = p_msg;
            }
        }
        /* If Header already received */
        else
        {
            p_msg = p_lite_cb->s.from_app.p_rx_msg;

            if (!p_msg)
            {
                BTUSB_ERR("no Rx buffer\n");
                return EINVAL;
            }
            p = (UINT8 *)(p_msg + 1) + p_msg->len + p_msg->offset;

            /* How many payload bytes are expected */
            copy_len = p_lite_cb->s.from_app.rx_len;

            /* If less bytes are available */
            if (remainder < copy_len)
            {
                copy_len = remainder;
            }

            /* Copy the Payload (or a part of it) */
            if (copy_from_user(p, (void *)p_user_buffer, copy_len))
            {
                BTUSB_ERR("Copy payload from user error\n");
                return -EINVAL;
            }
            remainder -= copy_len;
            p_user_buffer += copy_len;
            copied_len += copy_len;
            p_lite_cb->s.from_app.rx_len -= copy_len;
            p_msg->len += copy_len;

            if (p_lite_cb->s.from_app.rx_len == 0)
            {
                /* Handle the received message */
                btusb_lite_ipc_hndl(p_dev, p_msg);

                p_lite_cb->s.from_app.p_rx_msg = NULL;
                p_lite_cb->s.from_app.rx_header_len = 0; /* Ready to receive a new header */
            }
        }
    }

    return copied_len;
}

/*******************************************************************************
 **
 ** Function        btusb_lite_msg_to_app_get
 **
 ** Description     Get next message to send to IPC
 **
 ** Returns         GKI buffer to send (or Null)
 **
 *******************************************************************************/
static BT_HDR *btusb_lite_msg_to_app_get(struct btusb *p_dev)
{
    BT_HDR *p_msg;
    BT_HDR *p_hdr_msg;
    UINT8 *p_data;

    /* First, check if a locally generated Event (IPC) message is pending */
    p_msg = p_dev->lite_cb.s.to_app.p_ipc_msg;
    if (p_msg)
    {
        return p_msg;
    }

    /* Secondly, check if a locally generated IPC Header message is pending */
    p_msg = p_dev->lite_cb.s.to_app.p_hdr_msg;
    if (p_msg)
    {
        return p_msg;
    }

    /* Thirdly, check if an HCI message is pending */
    p_msg = p_dev->lite_cb.s.to_app.p_hci_msg;
    if (p_msg)
    {
        return p_msg;
    }

    /*
     * No pending message. Check queues now
     */

    /* First, check if a locally generated Event (UIPC) message is enqueued */
    p_msg = GKI_dequeue(&p_dev->lite_cb.s.to_app.ipc_queue);
    if (p_msg)
    {
        p_dev->lite_cb.s.to_app.p_ipc_msg = p_msg;
        return p_msg;
    }

    /* If HCI is not over IPC */
    if (!btusb_lite_is_hci_over_ipc(p_dev))
    {
        return NULL;    /* Nothing more to send on IPC */
    }

    /* Check if an HCI message (from BT controller) is ready */
    p_msg = GKI_getfirst(&p_dev->rx_queue);
    if (p_msg)
    {
        /* We need to Build an IPC Header to send the HCI packet */
        p_hdr_msg = GKI_getbuf(sizeof(BT_HDR) + BTUSB_LITE_IPC_HDR_SIZE);
        if (p_hdr_msg == NULL)
        {
            /* Leave the Received HCI packet in the queue (for next time) */
            BTUSB_ERR("No more buffer\n");
            p_dev->lite_cb.s.to_app.p_hci_msg = NULL;
            return NULL;
        }
        p_hdr_msg->len = BTUSB_LITE_IPC_HDR_SIZE;
        p_hdr_msg->offset = 0;
        p_hdr_msg->layer_specific = 0;
        p_hdr_msg->event = 0;
        p_data = (UINT8 *)(p_hdr_msg + 1) + p_hdr_msg->offset;

        /* Write Length */
        UINT16_TO_STREAM(p_data, p_msg->len + BTUSB_LITE_IPC_HDR_EVT_SIZE);

        /* Write Event code */
        switch(p_msg->event)
        {
        case HCIT_TYPE_EVENT:
            {
                struct btusb_rx_trans *p_rx_trans;
                p_rx_trans = container_of(p_msg, struct btusb_rx_trans, bt_hdr);
                BTUSB_INFO("Event=0x%02X Received from BT Controller\n",
                        *(p_rx_trans->dma_buffer + p_msg->offset));
                BTUSB_INFO("IPC Buffer TxEvent=BT_EVT_TO_BTU_HCI_EVT(%X) Length=%d\n",
                        BT_EVT_TO_BTU_HCI_EVT, p_msg->len);
            }
            UINT16_TO_STREAM(p_data, BT_EVT_TO_BTU_HCI_EVT);
            break;

        case HCIT_TYPE_ACL_DATA:
            BTUSB_INFO("IPC Buffer TxEvent=BT_EVT_BTU_IPC_ACL_EVT(%X) Length=%d\n", BT_EVT_BTU_IPC_ACL_EVT, p_msg->len);
            UINT16_TO_STREAM(p_data, BT_EVT_BTU_IPC_ACL_EVT);
            break;

        case HCIT_TYPE_LM_DIAG:
            BTUSB_INFO("IPC Buffer TxEvent=BT_EVT_TO_LM_DIAG(%X) Length=%d\n", BT_EVT_TO_LM_DIAG, p_msg->len);
            UINT16_TO_STREAM(p_data, BT_EVT_TO_LM_DIAG);
            break;

        default:
            /* Should not append. Set Event to 0xFFFF for debug */
            BTUSB_ERR("Unknown event=0x%x\n", p_msg->event);
            UINT16_TO_STREAM(p_data, 0xFFFF);
            break;
        }

        /* Store Header */
        p_dev->lite_cb.s.to_app.p_hdr_msg = p_hdr_msg;

        /* Dequeue HCI message */
        p_msg = GKI_dequeue(&p_dev->rx_queue);
        p_dev->lite_cb.s.to_app.p_hci_msg = p_msg;

        return p_hdr_msg;                   /* Return pointer on Header */
    }

    return p_msg;
}

/*******************************************************************************
 **
 ** Function        btusb_lite_msg_to_app_get_data_addr
 **
 ** Description     Retrieve the data from a GKI Buffer pointer
 **
 ** Returns         None
 **
 *******************************************************************************/
static UINT8 *btusb_lite_msg_to_app_get_data_addr(struct btusb *p_dev, BT_HDR *p_msg)
{
    struct btusb_rx_trans *p_rx_trans;

    /* If the message is a "real" GKI buffer */
    if ((p_dev->lite_cb.s.to_app.p_ipc_msg == p_msg) ||
        (p_dev->lite_cb.s.to_app.p_hdr_msg == p_msg))
    {
        return (UINT8 *)(p_msg + 1) + p_msg->offset;
    }

    /* If the message is an HCI message, the data is located in the USB transaction */

    /* Check if the message is an HCI message */
    if (p_dev->lite_cb.s.to_app.p_hci_msg == p_msg)
    {
        p_rx_trans = container_of(p_msg, struct btusb_rx_trans, bt_hdr);
        return (p_rx_trans->dma_buffer + p_msg->offset);
    }

    BTUSB_ERR("Unknown buffer=%p\n", p_msg);

    return NULL;
}

/*******************************************************************************
 **
 ** Function        btusb_lite_msg_to_app_free
 **
 ** Description     Free a message which has been sent to IPC
 **
 ** Returns         None
 **
 *******************************************************************************/
static void btusb_lite_msg_to_app_free(struct btusb *p_dev, BT_HDR *p_msg)
{
    /* Check if the message is a locally generated Event (IPC) message */
    if (p_dev->lite_cb.s.to_app.p_ipc_msg == p_msg)
    {
        GKI_freebuf(p_msg);
        p_dev->lite_cb.s.to_app.p_ipc_msg = NULL;
        return;
    }

    /* Check if the message is a locally generated Header (IPC) message */
    if (p_dev->lite_cb.s.to_app.p_hdr_msg == p_msg)
    {
        GKI_freebuf(p_msg);
        p_dev->lite_cb.s.to_app.p_hdr_msg = NULL;
        return;
    }

    /* Check if the message is an HCI message */
    if (p_dev->lite_cb.s.to_app.p_hci_msg == p_msg)
    {
        p_dev->lite_cb.s.to_app.p_hci_msg = NULL;
        /* The buffer must not be freed. It has to be re-enqueue in USB core */
        btusb_rx_dequeued(p_dev, p_msg);
        return;
    }

    BTUSB_ERR("Unknown buffer=%p\n", p_msg);
}

/*******************************************************************************
 **
 ** Function        btusb_lite_read
 **
 ** Description     Read handler of the BTUSB Lite interface
 **
 ** Returns         Nb bytes written
 **
 *******************************************************************************/
static ssize_t btusb_lite_read(struct file *file, char __user *p_user_buffer, size_t count, loff_t *p_off)
{
    struct btusb *p_dev = file->private_data;
    ssize_t size;
    UINT8 *p_data;
    unsigned long copy_len;
    BT_HDR *p_msg;
    int rv;

    BTUSB_DBG("btusb_lite_read\n");

    if (!p_dev)
    {
        BTUSB_ERR("Unable to find p_dev reference\n");
        size = -ENODEV;
        goto out;
    }

    if (!p_dev->lite_cb.s.opened)
    {
        BTUSB_ERR("Lite interface was not opened\n");
        size = -EBUSY;;
        goto out;
    }

    if (count == 0)
    {
        size = 0;
        goto out;
    }

    /* Check that the user buffer is good */
    if (unlikely(!access_ok(VERIFY_WRITE, (void *)p_user_buffer, count)))
    {
        BTUSB_ERR("buffer access verification failed\n");
        size = -EFAULT;
        goto out;
    }

    /* Sleep while nothing for the application */
    rv = wait_event_interruptible(p_dev->rx_wait_q,
            ((p_msg = btusb_lite_msg_to_app_get(p_dev)) != NULL));

    if (unlikely(rv))
    {
        BTUSB_DBG("read wait interrupted");
        return rv;
    }

    if (p_msg)
    {
        if (p_msg->len < count)
        {
            copy_len = p_msg->len;
        }
        else
        {
            copy_len = count; /* User asks for count bytes */
        }

        BTUSB_DBG("p_msg=%p  msg->len=%d count=%zu copy_len=%lu\n", p_msg, p_msg->len, count, copy_len);

        p_data = btusb_lite_msg_to_app_get_data_addr(p_dev, p_msg);
        if (p_data == NULL)
        {
            size = -EFAULT;
            goto out;
        }

        /* copy the message data to the available user space */
        if (unlikely(copy_to_user(p_user_buffer, p_data, copy_len)))
        {
            BTUSB_ERR("copy to user error\n");
            size = -EINVAL;
            goto out;
        }

        BTUSB_DBG("copied %ld bytes\n", copy_len);

        p_msg->offset += copy_len;
        p_msg->len -= copy_len;
        size = copy_len;

        if (p_msg->len == 0)
        {
            BTUSB_DBG("free gki buffer\n");
            btusb_lite_msg_to_app_free(p_dev, p_msg);
        }
        goto out;
    }
    else
    {
        BTUSB_ERR("No Buffer\n");
    }

    size = 0; /* should not happen */

out:
    return size;
}

/*******************************************************************************
 **
 ** Function         btusb_lite_poll
 **
 ** Description      Poll callback (to implement select)
 **
 ** Parameters       file : file structure of the opened instance
 **                  p_pt : poll table to which the local queue must be added
 **
 ** Returns          Mask of the type of polling supported, error number if negative
 **
 *******************************************************************************/
static unsigned int btusb_lite_poll(struct file *file, struct poll_table_struct *p_pt)
{
    struct btusb *p_dev = file->private_data;
    unsigned int mask;

    BTUSB_DBG("enter\n");

    /* retrieve the device from the file pointer*/
    BTUSB_DBG("p_dev=%p\n", p_dev);
    if (unlikely(p_dev == NULL))
    {
        BTUSB_ERR("can't find device\n");
        return -ENODEV;
    }

    // check if the device was disconnected
    if (unlikely(!p_dev->p_main_intf))
    {
        BTUSB_ERR("device unplugged\n");
        return -ENODEV;
    }

    if (unlikely(!p_dev->lite_cb.s.opened))
    {
        BTUSB_ERR("Lite interface was not opened\n");
        return -EINVAL;
    }

    /* indicate to the system on which queue it should poll (non blocking call) */
    poll_wait(file, &p_dev->rx_wait_q, p_pt);

    /* enable WRITE (select/poll is not write blocking) */
    mask = POLLOUT | POLLWRNORM;

    /* enable READ only if data is queued from HCI */
    if (btusb_lite_msg_to_app_get(p_dev))
    {
        mask |= POLLIN | POLLRDNORM;
    }

    return mask;
}

/*******************************************************************************
 **
 ** Function        btusb_lite_create
 **
 ** Description     Create BTUSB Lite interface
 **
 ** Returns         status (< 0 if error)
 **
 *******************************************************************************/
int btusb_lite_create(struct btusb *p_dev, struct usb_interface *p_interface)
{
    int rv = -1;

    p_dev->lite_cb.p_btpcm = btpcm_init(kobject_name(&p_dev->p_main_intf->usb_dev->kobj));
    if (!p_dev->lite_cb.p_btpcm)
    {
        BTUSB_ERR("btpcm_init failed\n");
    }

    if (p_dev->p_pde)
    {
        p_dev->lite_cb.p_lite_pde = proc_create_data("lite", S_IRUGO | S_IWUGO, p_dev->p_pde, &btusb_lite_fops, p_dev);
        if (!p_dev->lite_cb.p_lite_pde)
        {
            BTUSB_ERR("Couldn't create proc lite entry\n");
        }
        else
        {
            BTUSB_INFO("Created proc lite entry\n");
            rv = 0;
        }
    }

    return rv;
}

/*******************************************************************************
 **
 ** Function        btusb_lite_delete
 **
 ** Description     Delete BTUSB Lite interface
 **
 ** Returns         none
 **
 *******************************************************************************/
void btusb_lite_delete(struct btusb *p_dev, struct usb_interface *p_interface)
{
    if (p_dev->p_pde)
    {
        remove_proc_entry("lite", p_dev->p_pde);
        BTUSB_INFO("proc lite removed\n");
    }

    if (btpcm_exit(p_dev->lite_cb.p_btpcm))
    {
        BTUSB_ERR("btpcm_exit failed\n");
    }
}

/*******************************************************************************
 **
 ** Function        btusb_lite_is_hci_over_ipc
 **
 ** Description     Check if HCI is over IPC (Lite Interface).
 **
 ** Returns         int (1 if HCI is over IPC otherwise 0)
 **
 *******************************************************************************/
int btusb_lite_is_hci_over_ipc(struct btusb *p_dev)
{
    if ((p_dev->lite_cb.s.opened) &&      /* User Space opened the Lite interface */
        (p_dev->lite_cb.s.mgt.opened) &&  /* IPC MGT Opened */
        /* Lite Transport is opened */
        ((p_dev->lite_cb.s.btu.transport_state == BTU_LITE_STACK_ACTIVE) ||
         (p_dev->lite_cb.s.btu.transport_state == BTU_LITE_TRANSPORT_ACTIVE)))
    {
        return 1;
    }
    return 0;
}


/*******************************************************************************
 **
 ** Function        btusb_lite_stop_all
 **
 ** Description     Stop all sound streams
 **
 ** Returns         void
 **
 *******************************************************************************/
void btusb_lite_stop_all(struct btusb *p_dev)
{
    int i;

    BTUSB_INFO("");

    for (i = 0 ; i < BTA_AV_NUM_STRS ; i++)
    {
        btusb_lite_av_stop(p_dev, i, 0);
        btusb_lite_av_remove(p_dev, i, 0, 0);
    }
}

/*******************************************************************************
 **
 ** Function        btusb_lite_ipc_hndl
 **
 ** Description     Handle received message from BTUSB Lite interface
 **
 ** Returns         none
 **
 *******************************************************************************/
static void btusb_lite_ipc_hndl(struct btusb *p_dev, BT_HDR *p_msg)
{
    BTUSB_INFO("IPC Buffer RxEvent=%s(0x%X) Length=%d\n",
            btusb_lite_ipc_event_desc(p_msg->event), p_msg->event, p_msg->len);

    if (unlikely(dbgflags & BTUSB_DBG_MSG) && p_msg->len)
    {
        UINT8 *p = (UINT8 *)(p_msg + 1) + p_msg->offset;
        int len = p_msg->len>20?20:p_msg->len;
        int i;

        for (i = 0 ; i < len ; i++, p++)
        {
            printk("%02X ", *p);
        }
        if (p_msg->len>20)
            printk("...\n");
        else
            printk("\n");
    }

    switch(p_msg->event)
    {
    case BT_EVT_TO_LM_HCI_CMD:
        btusb_lite_ipc_hci_cmd_hndl(p_dev, p_msg);
        /* NO GKI_freebuf here */
        break;

    case BT_EVT_TO_LM_HCI_ACL:
    case BT_EVT_TO_LM_HCI_ACL | LOCAL_BLE_CONTROLLER_ID:
        btusb_lite_ipc_hci_acl_hndl(p_dev, p_msg);
        /* NO GKI_freebuf here */
        break;

    case BT_EVT_BTU_IPC_MGMT_EVT:
        btusb_lite_ipc_mgt_hndl(p_dev, p_msg);
        GKI_freebuf(p_msg);
        break;

    case BT_EVT_BTU_IPC_BTU_EVT:
        btusb_lite_ipc_btu_hndl(p_dev, p_msg);
        GKI_freebuf(p_msg);
        break;

    case BT_EVT_BTU_IPC_L2C_EVT:
        btusb_lite_ipc_l2c_hndl(p_dev, p_msg);
        GKI_freebuf(p_msg);
        break;

    case BT_EVT_BTU_IPC_BTM_EVT:
        btusb_lite_ipc_btm_hndl(p_dev, p_msg);
        GKI_freebuf(p_msg);
        break;

    case BT_EVT_BTU_IPC_AVDT_EVT:
        btusb_lite_ipc_avdt_hndl(p_dev, p_msg);
        GKI_freebuf(p_msg);
        break;

    case BT_EVT_BTU_IPC_SLIP_EVT:
    case BT_EVT_BTU_IPC_BTTRC_EVT:
    case BT_EVT_BTU_IPC_BURST_EVT:
    case BT_EVT_BTU_IPC_ACL_EVT:
    case BT_EVT_BTU_IPC_L2C_MSG_EVT:
        BTUSB_INFO("Event=0x%X Not expected\n", p_msg->event);
        GKI_freebuf(p_msg);
        break;

    default:
        BTUSB_INFO("Event=0x%X Unknown\n", p_msg->event);
        GKI_freebuf(p_msg);
        break;
    }
}

/*******************************************************************************
 **
 ** Function        btusb_lite_ipc_hci_cmd_hndl
 **
 ** Description     Handle HCI CMD packet received from Lite interface
 **
 ** Returns         none
 **
 *******************************************************************************/
static void btusb_lite_ipc_hci_cmd_hndl(struct btusb *p_dev, BT_HDR *p_msg)
{
    UINT8 *p;
    UINT16 hci_opcode;
    UINT8 hci_status;

    /* First check if this HCI command must be caught by BTUSB */
    p = (UINT8 *)(p_msg + 1) + p_msg->offset;

    STREAM_TO_UINT16(hci_opcode, p);        /* Extract HCI Opcode */

    /* If the HCI Opcode must be caught */
    switch(hci_opcode)
    {
    case HCI_BRCM_PAUSE_TRANSPORT:
        BTUSB_INFO("HCI TransportPause VSC (%X) caught\n", hci_opcode);
        hci_status = HCI_SUCCESS;
        btusb_lite_ipc_cmd_cplt_evt_send(p_dev, hci_opcode,
                &hci_status, sizeof(hci_status));
        GKI_freebuf(p_msg);
        return;

    case HCI_BRCM_TRANSPORT_RESUME:
        BTUSB_INFO("HCI TransportResume VSC (%X) caught\n", hci_opcode);

        hci_status = HCI_SUCCESS;
        btusb_lite_ipc_cmd_cplt_evt_send(p_dev, hci_opcode,
                &hci_status, sizeof(hci_status));
        GKI_freebuf(p_msg);
        return;

    default:
        break;
    }

    BTUSB_INFO("HCI OpCode=0x%04X Sent to BT Controller\n", hci_opcode);

    if (p_msg->offset < 1)
    {
        BTUSB_ERR("Offset=%d too small\n", p_msg->offset);
        GKI_freebuf(p_msg);
        return;
    }

    p = (UINT8 *)(p_msg + 1) + p_msg->offset;
    btusb_submit_cmd(p_dev, p, p_msg->len);

    GKI_freebuf(p_msg);
}

/*******************************************************************************
 **
 ** Function        btusb_lite_ipc_hci_acl_hndl
 **
 ** Description     Handle HCI ACL packet received from Lite interface
 **
 ** Returns         none
 **
 *******************************************************************************/
static void btusb_lite_ipc_hci_acl_hndl(struct btusb *p_dev, BT_HDR *p_msg)
{
    UINT8 *p;

    if (p_msg->offset < 1)
    {
        BTUSB_ERR("Offset=%d too small\n", p_msg->offset);
        GKI_freebuf(p_msg);
        return;
    }

    p = (UINT8 *)(p_msg + 1) + p_msg->offset;
    btusb_submit_acl(p_dev, p, p_msg->len);

    GKI_freebuf(p_msg);
}

/*******************************************************************************
 **
 ** Function        btusb_lite_ipc_mgt_hndl
 **
 ** Description     Handle Mgt messages received from Lite interface
 **
 ** Returns         none
 **
 *******************************************************************************/
static void btusb_lite_ipc_mgt_hndl(struct btusb *p_dev, BT_HDR *p_msg)
{
    UINT8 cmd;
    UINT8 *p = (UINT8 *)(p_msg + 1) + p_msg->offset;
    UINT8 response[6];
    UINT8 *p_response = response;

    STREAM_TO_UINT8(cmd, p);           /* Extract UIPC_MGMT Request */

    switch(cmd)
    {
    case UIPC_OPEN_REQ:
        BTUSB_INFO("IPC_MGT:OpenReq received\n");
        /* If Mgt already opened */
        if (p_dev->lite_cb.s.mgt.opened)
        {
            BTUSB_ERR("IPC_MGT Was already opened\n");
            UINT8_TO_STREAM(p_response, UIPC_STATUS_FAIL);  /* Status */
        }
        else
        {
            p_dev->lite_cb.s.mgt.opened = true;
            UINT8_TO_STREAM(p_response, UIPC_STATUS_SUCCESS);  /* Status */
        }
        UINT16_TO_STREAM(p_response, UIPC_VERSION_MAJOR);    /* version_major */
        UINT16_TO_STREAM(p_response, UIPC_VERSION_MINOR);    /* version_minor */
        UINT8_TO_STREAM(p_response, BTM_SYNC_INFO_NUM_STR);  /* Number of simultaneous streams supported by the light stack */
        btusb_lite_ipc_rsp_send(p_dev, BT_EVT_BTU_IPC_MGMT_EVT, UIPC_OPEN_RSP,
                response, p_response - response);
        break;

    case UIPC_CLOSE_REQ:
        BTUSB_INFO("IPC_MGT:CloseReq received\n");
        p_dev->lite_cb.s.mgt.opened = false;
        btusb_lite_ipc_rsp_send(p_dev, BT_EVT_BTU_IPC_MGMT_EVT, UIPC_CLOSE_RSP, NULL, 0);
        break;

    default:
        BTUSB_INFO("Unknown IPC_MGT command=%d\n", cmd);
        break;
    }
}

/*******************************************************************************
 **
 ** Function        btusb_lite_ipc_btu_hndl
 **
 ** Description     Handle BTU messages received from Lite interface
 **
 ** Returns         none
 **
 *******************************************************************************/
static void btusb_lite_ipc_btu_hndl(struct btusb *p_dev, BT_HDR *p_msg)
{
    UINT8 cmd;
    UINT8 byte;
    UINT8 *p = (UINT8 *)(p_msg + 1) + p_msg->offset;

    STREAM_TO_UINT8(cmd, p);           /* Extract UIPC_BTU Request */

    switch(cmd)
    {
    case BTU_IPC_CMD_SET_TRANSPORT_STATE:
        STREAM_TO_UINT8(byte, p);           /* Extract Param */
        BTUSB_INFO("IPC_BTU:SetTransportState (%d) received\n", byte);
        p_dev->lite_cb.s.btu.transport_state = byte;
        break;

    case BTU_IPC_CMD_DISABLE_TRANSPORT:
        STREAM_TO_UINT8(byte, p);           /* Extract Param */
        BTUSB_INFO("IPC_BTU:DisableTransport (%d) received\n", byte);
        p_dev->lite_cb.s.btu.transport_disabled = byte;
        break;

    default:
        BTUSB_INFO("Unknown IPC_BTU command=%d\n", cmd);
        break;
    }
}

/*******************************************************************************
 **
 ** Function        btusb_lite_ipc_btm_hndl
 **
 ** Description     Handle BTM messages received from Lite interface
 **
 ** Returns         none
 **
 *******************************************************************************/
static void btusb_lite_ipc_btm_hndl(struct btusb *p_dev, BT_HDR *p_msg)
{
    UINT8 cmd;
    UINT8 *p = (UINT8 *)(p_msg + 1) + p_msg->offset;
    tBTA_AV_SYNC_INFO sync_info;
    UINT8   scb_idx;
    tBTA_AV_AUDIO_CODEC_INFO codec_cfg;
    UINT8 start_stop_flag;
    UINT8 multi_av_supported;
    UINT16 curr_mtu;
    UINT8 audio_open_cnt;
    UINT8 response[1];
    UINT8 *p_response = response;
    UINT8 streaming_type;

    STREAM_TO_UINT8(cmd, p);           /* Extract UIPC_BTU Request */

    switch(cmd)
    {
    case BTA_AV_SYNC_TO_LITE_REQ:
        BTUSB_INFO("IPC_BTM:BtaAvSyncToLiteReq (%d)\n", cmd);
        STREAM_TO_UINT8(sync_info.avdt_handle, p);
        STREAM_TO_UINT8(sync_info.chnl, p);
        STREAM_TO_UINT8(sync_info.codec_type, p);
        STREAM_TO_UINT8(sync_info.cong, p);
        STREAM_TO_UINT8(sync_info.hdi, p);
        STREAM_TO_UINT8(sync_info.hndl, p);
        STREAM_TO_UINT8(sync_info.l2c_bufs, p);
        STREAM_TO_UINT16(sync_info.l2c_cid, p);
        STREAM_TO_ARRAY (sync_info.peer_addr, p, BD_ADDR_LEN);
        STREAM_TO_UINT8(multi_av_supported, p);
        STREAM_TO_UINT16(curr_mtu, p);
        BTUSB_INFO("avdt_hdl=0x%x chnl=0x%x codec_type=0x%x cong=%d hdi=%d hndl=0x%x\n",
                sync_info.avdt_handle, sync_info.chnl, sync_info.codec_type,
                sync_info.cong, sync_info.hdi, sync_info.hndl);
        BTUSB_INFO("l2c_bufs=%d l2c_cid=0x%x multi_av_sup=%d curr_mtu=%d\n",
                sync_info.l2c_bufs, sync_info.l2c_cid, multi_av_supported, curr_mtu);
        BTUSB_INFO("BdAddr=%02X-%02X-%02X-%02X-%02X-%02X\n",
                sync_info.peer_addr[0], sync_info.peer_addr[1],sync_info.peer_addr[2],
                sync_info.peer_addr[3], sync_info.peer_addr[4],sync_info.peer_addr[5]);

        /* Add AV channel */
        btusb_lite_av_add(p_dev, &sync_info, multi_av_supported, curr_mtu);

        UINT8_TO_STREAM(p_response, sync_info.hdi);
        btusb_lite_ipc_rsp_send(p_dev, BT_EVT_BTU_IPC_BTM_EVT, BTA_AV_SYNC_TO_LITE_RESP,
                response, p_response - response);
        break;

    case BTA_AV_STR_START_TO_LITE_REQ:
        BTUSB_INFO("IPC_BTM:BtaAvStrStartToLiteReq (%d)\n", cmd);
        STREAM_TO_UINT8(scb_idx, p);
        STREAM_TO_UINT8(audio_open_cnt, p);
        STREAM_TO_UINT16 (codec_cfg.bit_rate, p);
        STREAM_TO_UINT16 (codec_cfg.bit_rate_busy, p);
        STREAM_TO_UINT16 (codec_cfg.bit_rate_swampd, p);
        STREAM_TO_UINT8 (codec_cfg.busy_level, p);
        STREAM_TO_ARRAY (codec_cfg.codec_info, p, AVDT_CODEC_SIZE);
        STREAM_TO_UINT8 (codec_cfg.codec_type, p);
        STREAM_TO_UINT8 (start_stop_flag, p);
        STREAM_TO_UINT8 (streaming_type, p);

        BTUSB_INFO("  scb_idx=%d audio_open_cnt=%d busy_level=%d start_stop_flag=%d streaming_type=%d\n",
                scb_idx, audio_open_cnt,
                codec_cfg.busy_level, start_stop_flag, streaming_type);
        BTUSB_INFO("  codec_type=0x%x bit_rate=%d busy_level=%d bit_rate_busy=%d bit_rate_swampd=%d\n",
                codec_cfg.codec_type, codec_cfg.bit_rate, codec_cfg.busy_level,
                codec_cfg.bit_rate_busy, codec_cfg.bit_rate_swampd);
        BTUSB_INFO("  CodecInfo:%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X",
                codec_cfg.codec_info[0], codec_cfg.codec_info[1], codec_cfg.codec_info[2],
                codec_cfg.codec_info[3], codec_cfg.codec_info[4], codec_cfg.codec_info[5],
                codec_cfg.codec_info[6], codec_cfg.codec_info[7], codec_cfg.codec_info[8],
                codec_cfg.codec_info[9]);

        /* Start AV */
        btusb_lite_av_start(p_dev, scb_idx, start_stop_flag, audio_open_cnt, &codec_cfg, streaming_type);

        /* Send the response */
        UINT8_TO_STREAM(p_response, scb_idx);
        btusb_lite_ipc_rsp_send(p_dev, BT_EVT_BTU_IPC_BTM_EVT, BTA_AV_STR_START_TO_LITE_RESP,
                response, p_response - response);
        break;

    case BTA_AV_STR_STOP_TO_LITE_REQ:
        BTUSB_INFO("IPC_BTM:BtaAvStrStopToLiteReq (%d)\n", cmd);
        STREAM_TO_UINT8(scb_idx, p);
        STREAM_TO_UINT8(audio_open_cnt, p);
        BTUSB_INFO("  scb_idx=%d audio_open_cnt=%d\n", scb_idx, audio_open_cnt);

        btusb_lite_av_stop(p_dev, scb_idx, audio_open_cnt);

        UINT8_TO_STREAM(p_response, scb_idx);
        btusb_lite_ipc_rsp_send(p_dev, BT_EVT_BTU_IPC_BTM_EVT, BTA_AV_STR_STOP_TO_LITE_RESP,
                response, p_response - response);
        break;

    case BTA_AV_STR_SUSPEND_TO_LITE_REQ:
        BTUSB_INFO("IPC_BTM:BtaAvStrSuspendToLiteReq (%d)\n", cmd);
        STREAM_TO_UINT8(scb_idx, p);
        STREAM_TO_UINT8(audio_open_cnt, p);
        BTUSB_INFO("  scb_idx=%d audio_open_cnt=%d\n", scb_idx, audio_open_cnt);
        btusb_lite_av_suspend(p_dev, scb_idx, audio_open_cnt);

        UINT8_TO_STREAM(p_response, scb_idx);
        btusb_lite_ipc_rsp_send(p_dev, BT_EVT_BTU_IPC_BTM_EVT, BTA_AV_STR_SUSPEND_TO_LITE_RESP,
                response, p_response - response);
        break;

    case BTA_AV_STR_CLEANUP_TO_LITE_REQ:
        BTUSB_INFO("IPC_BTM:BtaAvStrCleanupToLiteReq (%d)\n", cmd);
        STREAM_TO_UINT8(scb_idx, p);
        STREAM_TO_UINT8(audio_open_cnt, p);
        STREAM_TO_UINT16(curr_mtu, p);
        BTUSB_INFO("  scb_idx=%d audio_open_cnt=%d curr_mtu=%d\n", scb_idx, audio_open_cnt,
                curr_mtu);

        btusb_lite_av_remove(p_dev, scb_idx, audio_open_cnt, curr_mtu);

        UINT8_TO_STREAM(p_response, scb_idx);
        btusb_lite_ipc_rsp_send(p_dev, BT_EVT_BTU_IPC_BTM_EVT, BTA_AV_STR_CLEANUP_TO_LITE_RESP,
                response, p_response - response);
        break;

    /* BTC commands (unexpected) */
    case A2DP_START_REQ:
    case A2DP_START_RESP:
    case A2DP_STOP_REQ:
    case A2DP_STOP_RESP:
    case A2DP_CLEANUP_REQ:
    case A2DP_CLEANUP_RESP:
    case A2DP_SUSPEND_REQ:
    case A2DP_SUSPEND_RESP:
    case A2DP_JITTER_DONE_IND:

    /* BTA AV Response commands (unexpected) */
    case BTA_AV_SYNC_TO_LITE_RESP:
    case BTA_AV_STR_START_TO_LITE_RESP:
    case BTA_AV_STR_STOP_TO_LITE_RESP:
    case BTA_AV_STR_CLEANUP_TO_LITE_RESP:
    case BTA_AV_STR_SUSPEND_TO_LITE_RESP:
    case BTA_AV_SYNC_ERROR_RESP:
        BTUSB_INFO("IPC_BTM: Unexpected Cmd=%d received\n", cmd);
        break;

    default:
        BTUSB_INFO("IPC_BTM: Unknown Cmd=%d received\n", cmd);
        break;
    }
}

/*******************************************************************************
 **
 ** Function        btusb_lite_ipc_avdt_hndl
 **
 ** Description     Handle AVDT messages received from Lite interface
 **
 ** Returns         none
 **
 *******************************************************************************/
static void btusb_lite_ipc_avdt_hndl(struct btusb *p_dev, BT_HDR *p_msg)
{
    UINT8 *p = (UINT8 *)(p_msg + 1) + p_msg->offset;
    tAVDT_SYNC_INFO sync_req;
    tAVDT_SYNC_INFO sync_rsp;
    int stream;
    struct btusb_lite_avdt_scb *p_scb;
    UINT8 avdt_status;

    STREAM_TO_UINT8(sync_req.op_code, p); /* Extract UIPC_AVDT Request */

    switch(sync_req.op_code)
    {
    case AVDT_SYNC_TO_LITE_REQ:
        BTUSB_INFO("IPC_AVDT:AvdtSyncToLiteReq (%d) received\n", sync_req.op_code);
        break;

    case AVDT_RESYNC_TO_LITE_REQ:
        BTUSB_INFO("IPC_AVDT:AvdtReSyncToLiteReq (%d) received\n", sync_req.op_code);
        break;

    case AVDT_SYNC_TO_FULL_REQ:
        BTUSB_INFO("IPC_AVDT:AvdtSyncToFullReq (%d) received\n", sync_req.op_code);
        break;

    case AVDT_REMOVE_TO_LITE_REQ:
        BTUSB_INFO("IPC_AVDT:AvdtRemoveToLiteReq (%d)\n", sync_req.op_code);
        break;

    case AVDT_SYNC_CP_TO_LITE_REQ:
        BTUSB_INFO("IPC_AVDT:AvdtSyncCpToLiteReq (%d)\n", sync_req.op_code);
        break;

    case AVDT_SYNC_TO_LITE_RESP:
    case AVDT_RESYNC_TO_LITE_RESP:
    case AVDT_SYNC_TO_FULL_RESP:
    case AVDT_REMOVE_TO_LITE_RESP:
    case AVDT_SYNC_CP_TO_LITE_RESP:
        BTUSB_INFO("IPC_AVDT: Unexpected Cmd=%d received\n", sync_req.op_code);
        sync_rsp.op_code = 0xFF;
        sync_rsp.status  = AVDT_SYNC_FAILURE;
        btusb_lite_ipc_avdt_sync_info_send(p_dev, &sync_rsp);
        return;

    default:
        BTUSB_INFO("IPC_AVDT: Unknown Cmd=%d received\n", sync_req.op_code);
        sync_rsp.op_code = 0xFE;
        sync_rsp.status  = AVDT_SYNC_FAILURE;
        btusb_lite_ipc_avdt_sync_info_send(p_dev, &sync_rsp);
        return;
    }

    /* Extract parameters */
    STREAM_TO_UINT8(sync_req.status, p);

    /* Decode Sync CP parameters */
    if (sync_req.op_code == AVDT_SYNC_CP_TO_LITE_REQ)
    {
        for(stream = 0; stream < BTM_SYNC_INFO_NUM_STR; stream++ )
        {
            STREAM_TO_UINT8(sync_req.scb_info[stream].handle, p);
            STREAM_TO_UINT16(sync_req.scb_info[stream].cp.id, p);
            STREAM_TO_UINT8(sync_req.scb_info[stream].cp.scms_hdr, p);
            if (sync_req.scb_info[stream].handle)
            {
                BTUSB_INFO("  stream[%d]: handle=0x%x cp_id=0x%04X scms_hdr=0x%x\n",
                        stream, sync_req.scb_info[stream].handle,
                        sync_req.scb_info[stream].cp.id,
                        sync_req.scb_info[stream].cp.scms_hdr);
            }
            else
            {
                BTUSB_INFO("  stream[%d]: No Data\n", stream);
            }
        }
    }
    /* Decode other Sync message parameters */
    else
    {
        for(stream = 0; stream < BTM_SYNC_INFO_NUM_STR; stream++ )
        {
            STREAM_TO_UINT8(sync_req.scb_info[stream].handle, p);
            STREAM_TO_BDADDR(sync_req.scb_info[stream].peer_addr, p)
            STREAM_TO_UINT16(sync_req.scb_info[stream].local_cid, p);
            STREAM_TO_UINT16(sync_req.scb_info[stream].peer_mtu, p);
            STREAM_TO_UINT8(sync_req.scb_info[stream].mux_tsid_media, p);
            STREAM_TO_UINT16(sync_req.scb_info[stream].media_seq, p);
            if (sync_req.scb_info[stream].handle)
            {
                BTUSB_INFO("  stream[%d]:\n", stream);
                BTUSB_INFO("  BdAddr=%02X-%02X-%02X-%02X-%02X-%02X\n",
                        sync_req.scb_info[stream].peer_addr[0],
                        sync_req.scb_info[stream].peer_addr[1],
                        sync_req.scb_info[stream].peer_addr[2],
                        sync_req.scb_info[stream].peer_addr[3],
                        sync_req.scb_info[stream].peer_addr[4],
                        sync_req.scb_info[stream].peer_addr[5]);
                BTUSB_INFO("  handle=0x%x local_cid=0x%x peer_mtu=%d mux_tsid_media=%d media_seq=%d\n",
                        sync_req.scb_info[stream].handle,
                        sync_req.scb_info[stream].local_cid,
                        sync_req.scb_info[stream].peer_mtu,
                        sync_req.scb_info[stream].mux_tsid_media,
                        sync_req.scb_info[stream].media_seq);
            }
            else
            {
                BTUSB_INFO("  stream[%d]: No data\n", stream);
            }
        }
    }
    memset(&sync_rsp, 0, sizeof(sync_rsp));

    switch(sync_req.op_code)
    {
    case AVDT_SYNC_TO_LITE_REQ:
        sync_rsp.op_code = AVDT_SYNC_TO_LITE_RESP;

        for(stream = 0; stream < BTM_SYNC_INFO_NUM_STR; stream++)
        {
            if(sync_req.scb_info[stream].handle == 0)
                continue;

            if(btusb_lite_avdt_init_scb(p_dev, &(sync_req.scb_info[stream])) != AVDT_SYNC_SUCCESS)
            {
                sync_rsp.status  = AVDT_SYNC_FAILURE;
                btusb_lite_ipc_avdt_sync_info_send(p_dev, &sync_rsp);
                return;
            }
        }
        sync_rsp.status  = AVDT_SYNC_SUCCESS;
        btusb_lite_ipc_avdt_sync_info_send(p_dev, &sync_rsp);
        break;

    case AVDT_RESYNC_TO_LITE_REQ:
        sync_rsp.op_code = AVDT_RESYNC_TO_LITE_RESP;

        for(stream = 0; stream < BTM_SYNC_INFO_NUM_STR; stream++)
        {
            if(sync_req.scb_info[stream].handle == 0)
                continue;

            if((p_scb = btusb_lite_avdt_scb_by_hdl(p_dev, sync_req.scb_info[stream].handle)) != NULL)
            {
                memcpy(p_scb->p_ccb->peer_addr, sync_req.scb_info[stream].peer_addr, BD_ADDR_LEN);
                p_scb->p_ccb->lcid = sync_req.scb_info[stream].local_cid;
                p_scb->p_ccb->peer_mtu = sync_req.scb_info[stream].peer_mtu;
                p_scb->mux_tsid_media = sync_req.scb_info[stream].mux_tsid_media;
                p_scb->media_seq = sync_req.scb_info[stream].media_seq;
            }
            else if(btusb_lite_avdt_init_scb(p_dev, &(sync_req.scb_info[stream])) != AVDT_SYNC_SUCCESS)
            {
                sync_rsp.status  = AVDT_SYNC_FAILURE;
                btusb_lite_ipc_avdt_sync_info_send(p_dev, &sync_rsp);
                return;
            }
        }
        sync_rsp.status  = AVDT_SYNC_SUCCESS;
        btusb_lite_ipc_avdt_sync_info_send(p_dev, &sync_rsp);
        break;

    case AVDT_SYNC_TO_FULL_REQ:
        sync_rsp.op_code = AVDT_SYNC_TO_FULL_RESP;

        for(stream = 0; stream < BTM_SYNC_INFO_NUM_STR; stream++)
        {
            if(sync_req.scb_info[stream].handle == 0)
            {
                sync_rsp.scb_info[stream].handle = 0;
                continue;
            }

            if(btusb_lite_avdt_remove_scb(p_dev, sync_req.scb_info[stream].handle,
                    &sync_rsp.scb_info[stream]) != AVDT_SYNC_SUCCESS)
            {
                sync_rsp.status  = AVDT_SYNC_FAILURE;
                btusb_lite_ipc_avdt_sync_info_send(p_dev, &sync_rsp);
                return;
            }
        }
        sync_rsp.status  = AVDT_SYNC_SUCCESS;
        btusb_lite_ipc_avdt_sync_info_send(p_dev, &sync_rsp);
        break;

    case AVDT_REMOVE_TO_LITE_REQ:
        sync_rsp.op_code = AVDT_REMOVE_TO_LITE_RESP;
        for(stream = 0; stream < BTM_SYNC_INFO_NUM_STR; stream++)
        {
            if(sync_req.scb_info[stream].handle == 0)
                continue;

            if(btusb_lite_avdt_remove_scb(p_dev, sync_req.scb_info[stream].handle,
                    &sync_rsp.scb_info[stream]) != AVDT_SYNC_SUCCESS)
            {
                sync_rsp.status  = AVDT_SYNC_FAILURE;
                btusb_lite_ipc_avdt_sync_info_send(p_dev, &sync_rsp);
                return;
            }
        }
        sync_rsp.status  = AVDT_SYNC_SUCCESS;
        btusb_lite_ipc_avdt_sync_info_send(p_dev, &sync_rsp);
        break;

    case AVDT_SYNC_CP_TO_LITE_REQ:
        sync_rsp.op_code = AVDT_SYNC_CP_TO_LITE_RESP;
        for(stream = 0; stream < BTM_SYNC_INFO_NUM_STR; stream++)
        {
            if(sync_req.scb_info[stream].handle == 0)
                continue;

            switch(sync_req.scb_info[stream].cp.id)
            {
            case AVDT_SYNC_CP_ID_NONE:
                avdt_status = btusb_lite_avdt_cp_set_scms(p_dev,
                        sync_req.scb_info[stream].handle, FALSE, 0x00);
                break;

            case AVDT_SYNC_CP_ID_SCMS:
                avdt_status = btusb_lite_avdt_cp_set_scms(p_dev,
                        sync_req.scb_info[stream].handle, TRUE,
                        sync_req.scb_info[stream].cp.scms_hdr);
                break;

            default:
                avdt_status = AVDT_SYNC_FAILURE;
                break;
            }
            if (avdt_status != AVDT_SYNC_SUCCESS)
            {
                sync_rsp.status  = AVDT_SYNC_FAILURE;
                btusb_lite_ipc_avdt_sync_info_send(p_dev, &sync_rsp);
                return;
            }
        }
        sync_rsp.status  = AVDT_SYNC_SUCCESS;
        btusb_lite_ipc_avdt_sync_info_send(p_dev, &sync_rsp);
        break;
    }
}

/*******************************************************************************
 **
 ** Function        btusb_lite_ipc_avdt_sync_info_send
 **
 ** Description     Build and send an IPC AVDT Sync Info Response
 **
 ** Returns         none
 **
 *******************************************************************************/
static void btusb_lite_ipc_avdt_sync_info_send(struct btusb *p_dev, tAVDT_SYNC_INFO *p_sync_rsp)
{
    int stream;
    UINT8 response[BTUSB_LITE_IPC_AVDT_SYNC_INFO_RSP_LEN];
    UINT8 *p_response = response;

    UINT8_TO_STREAM(p_response, p_sync_rsp->status);

    if (p_sync_rsp->op_code != AVDT_SYNC_CP_TO_LITE_RESP)
    {
        for(stream = 0; stream < BTM_SYNC_INFO_NUM_STR; stream++)
        {
            UINT8_TO_STREAM(p_response, p_sync_rsp->scb_info[stream].handle);
            BDADDR_TO_STREAM(p_response, p_sync_rsp->scb_info[stream].peer_addr)
            UINT16_TO_STREAM(p_response, p_sync_rsp->scb_info[stream].local_cid);
            UINT16_TO_STREAM(p_response, p_sync_rsp->scb_info[stream].peer_mtu);
            UINT8_TO_STREAM(p_response, p_sync_rsp->scb_info[stream].mux_tsid_media);
            UINT16_TO_STREAM(p_response, p_sync_rsp->scb_info[stream].media_seq);
        }
    }
    btusb_lite_ipc_rsp_send(p_dev, BT_EVT_BTU_IPC_AVDT_EVT, p_sync_rsp->op_code,
            response, p_response - response);
}

/*******************************************************************************
 **
 ** Function        btusb_lite_ipc_l2c_hndl
 **
 ** Description     Handle L2C messages received from Lite interface
 **
 ** Returns         none
 **
 *******************************************************************************/
static void btusb_lite_ipc_l2c_hndl(struct btusb *p_dev, BT_HDR *p_msg)
{
    UINT8 cmd;
    UINT8 *p = (UINT8 *)(p_msg + 1) + p_msg->offset;
    UINT8 response[3];
    UINT8 *p_response = response;
    struct btusb_lite_l2c_cb *p_l2c = &p_dev->lite_cb.s.l2c;
    int stream;
    tL2C_STREAM_INFO l2c_stream;
    UINT16 local_cid;
    UINT8 num_stream;

    STREAM_TO_UINT8(cmd, p);           /* Extract UIPC_MGMT Request */

    switch(cmd)
    {
    case L2C_SYNC_TO_LITE_REQ:
        BTUSB_INFO("IPC_L2C:L2cSyncToLiteReq (%d) received\n", L2C_SYNC_TO_LITE_REQ);
        STREAM_TO_UINT16(p_l2c->light_xmit_quota, p);
        STREAM_TO_UINT16(p_l2c->acl_data_size, p);
        STREAM_TO_UINT16(p_l2c->non_flushable_pbf, p);
        STREAM_TO_UINT8(p_l2c->multi_av_data_cong_start, p);
        STREAM_TO_UINT8(p_l2c->multi_av_data_cong_end, p);
        STREAM_TO_UINT8(p_l2c->multi_av_data_cong_discard, p);
        STREAM_TO_UINT8(num_stream, p);
        BTUSB_INFO("Xquota=%d AclSize=%d NFpbf=%d congStart=%d congEnd=%d congDisc=%d NbStr=%d\n",
                p_l2c->light_xmit_quota, p_l2c->acl_data_size, p_l2c->non_flushable_pbf,
                p_l2c->multi_av_data_cong_start, p_l2c->multi_av_data_cong_end,
                p_l2c->multi_av_data_cong_discard, num_stream);

        /* Start building the response */
        UINT16_TO_STREAM(p_response, p_l2c->light_xmit_unacked);
        UINT8_TO_STREAM(p_response, num_stream);

        for(stream = 0; stream < num_stream; stream++)
        {
            STREAM_TO_UINT16(l2c_stream.local_cid, p);
            STREAM_TO_UINT16(l2c_stream.remote_cid, p);
            STREAM_TO_UINT16(l2c_stream.out_mtu, p);
            STREAM_TO_UINT16(l2c_stream.handle, p);
            STREAM_TO_UINT16(l2c_stream.link_xmit_quota, p);
            STREAM_TO_UINT8(l2c_stream.is_flushable, p);
            BTUSB_INFO("  Stream[%d]:lcid=0x%X rcid=0x%X mtu=%d handle=0x%X xmit_quota=%d flushable=%d\n",
                    stream, l2c_stream.local_cid, l2c_stream.remote_cid,
                    l2c_stream.out_mtu, l2c_stream.handle,
                    l2c_stream.link_xmit_quota, l2c_stream.is_flushable);

            /* Resume building the response */
            UINT16_TO_STREAM(p_response, l2c_stream.local_cid);

            /* Synchronize (add) this L2CAP Stream */
            if (btusb_lite_l2c_add(p_dev, &l2c_stream) < 0)
            {
                UINT8_TO_STREAM(p_response, L2C_SYNC_FAILURE);
            }
            else
            {
                UINT8_TO_STREAM(p_response, L2C_SYNC_SUCCESS);
            }
        }
        btusb_lite_ipc_rsp_send(p_dev, BT_EVT_BTU_IPC_L2C_EVT, L2C_SYNC_TO_LITE_RESP,
                response, p_response - response);
        break;

    case L2C_REMOVE_TO_LITE_REQ:
        BTUSB_INFO("IPC_L2C:L2cRemoveToLiteReq (%d) received\n", L2C_REMOVE_TO_LITE_REQ);
        STREAM_TO_UINT16(p_l2c->light_xmit_quota, p);
        STREAM_TO_UINT8(num_stream, p);
        BTUSB_INFO("Xquota=%d NbStr=%d\n", p_l2c->light_xmit_quota, num_stream);

        /* Start building the response */
        UINT16_TO_STREAM(p_response, p_l2c->light_xmit_unacked);
        UINT8_TO_STREAM(p_response, num_stream);

        for(stream = 0; stream < num_stream; stream++)
        {
            STREAM_TO_UINT16(local_cid, p);
            BTUSB_INFO("  Stream[%d]:lcid=0x%X\n", stream, p_l2c->ccb[stream].local_cid);

            /* Resume building the response */
            UINT16_TO_STREAM(p_response, local_cid);

            /* Synchronize (remove) this L2CAP Stream */
            if (btusb_lite_l2c_remove(p_dev, local_cid) < 0)
            {
                UINT8_TO_STREAM(p_response, L2C_SYNC_FAILURE);
            }
            else
            {
                UINT8_TO_STREAM(p_response, L2C_SYNC_SUCCESS);
            }
        }

        /* Send the response to the full stack */
        btusb_lite_ipc_rsp_send(p_dev, BT_EVT_BTU_IPC_L2C_EVT, L2C_REMOVE_TO_LITE_RESP,
                response, p_response - response);
        break;

    default:
        BTUSB_INFO("Unknown IPC_MGT command=%d\n", cmd);
        break;
    }
}

/*******************************************************************************
 **
 ** Function        btusb_lite_ipc_rsp_send
 **
 ** Description     Send an Response over Lite interface.
 **
 ** Returns         Void
 **
 *******************************************************************************/
static void btusb_lite_ipc_rsp_send(struct btusb *p_dev,
        UINT16 event, UINT8 op_code, UINT8 *p_param, UINT8 param_len)
{
    BT_HDR *p_msg;
    UINT16 size = param_len + BTUSB_LITE_IPC_HDR_SIZE + sizeof(UINT8);
    UINT8 *p;

    BTUSB_INFO("Event=%s(0x%X), opcode=%d len=%d\n", btusb_lite_ipc_event_desc(event),
            event, op_code, param_len);

    /* Get a buffer from the pool */
    p_msg = (BT_HDR *)GKI_getbuf(sizeof(BT_HDR) + size);
    if(unlikely(p_msg == NULL))
    {
        BTUSB_ERR("Unable to get GKI buffer\n");
        return;
    }

    if (unlikely(dbgflags & BTUSB_GKI_CHK_MSG) &&
        unlikely(GKI_buffer_status(p_msg) != BUF_STATUS_UNLINKED))
    {
        BTUSB_ERR("buffer != BUF_STATUS_UNLINKED 0x%p\n", p_msg);
        return;
    }

    p_msg->offset = 0;
    p_msg->event = 0;
    p_msg->len = size;

    p = (UINT8 *)(p_msg + 1);

    UINT16_TO_STREAM(p, param_len + BTUSB_LITE_IPC_HDR_EVT_SIZE + sizeof(UINT8));  /* Length */
    UINT16_TO_STREAM(p, event);  /* Event */
    UINT8_TO_STREAM(p, op_code);  /* Opcode */
    if (p_param)
    {
        ARRAY_TO_STREAM(p, p_param, param_len)
    }

    /* Send message to User Space */
    btusb_lite_ipc_sent_to_user(p_dev, p_msg);
}

/*******************************************************************************
 **
 ** Function        btusb_lite_ipc_cmd_cplt_evt_send
 **
 ** Description     Send an UIPC_Over_HCI VSC Cmd Complete.
 **
 ** Returns         Void
 **
 *******************************************************************************/
static void btusb_lite_ipc_cmd_cplt_evt_send(struct btusb *p_dev,
        UINT16 opcode, UINT8 *p_param, UINT8 param_len)
{
    BT_HDR *p_msg;
    UINT16 size = param_len + BTUSB_LITE_IPC_HDR_SIZE + 5;
    UINT8 *p;

    /* Get a buffer from the pool */
    p_msg = (BT_HDR *)GKI_getbuf(sizeof(BT_HDR) + size);
    if(unlikely(p_msg == NULL))
    {
        BTUSB_ERR("Unable to get GKI buffer\n");
        return;
    }

    if (unlikely(dbgflags & BTUSB_GKI_CHK_MSG) &&
        unlikely(GKI_buffer_status(p_msg) != BUF_STATUS_UNLINKED))
    {
        BTUSB_ERR("buffer != BUF_STATUS_UNLINKED 0x%p\n", p_msg);
        return;
    }

    p_msg->offset = 0;
    p_msg->event = 0;
    p_msg->len = size;

    p = (UINT8 *)(p_msg + 1);

    UINT16_TO_STREAM(p, param_len + BTUSB_LITE_IPC_HDR_EVT_SIZE + 5);  /* length */
    UINT16_TO_STREAM(p, BT_EVT_TO_BTU_HCI_EVT);  /* IPC OpCode */
    UINT8_TO_STREAM(p, HCI_COMMAND_COMPLETE_EVT);  /* Command Complete Evt */
    UINT8_TO_STREAM(p, param_len + 3);  /* Param Length (param + NumCmd + OpCode) */
    UINT8_TO_STREAM(p, 0x01);  /* HCI Num Command */
    UINT16_TO_STREAM(p, opcode);  /* HCI OpCode */

    if (p_param)
    {
        ARRAY_TO_STREAM(p, p_param, param_len)
    }

    /* Send message to User Space */
    btusb_lite_ipc_sent_to_user(p_dev, p_msg);
}

/*******************************************************************************
 **
 ** Function        btusb_lite_ipc_sent_to_user
 **
 ** Description     Send message to User Space (via IPC Interface).
 **
 ** Returns         status: <> 0 if the event must be send to user space (BSA)
 **                         0 if the event is handled
 **
 *******************************************************************************/
static void btusb_lite_ipc_sent_to_user(struct btusb *p_dev, BT_HDR *p_msg)
{
    /* Update Lite Statistics */
    p_dev->lite_cb.s.stat.event_bytes += p_msg->len;
    p_dev->lite_cb.s.stat.event_completed++;

    /* Enqueue message in IPC queue */
    GKI_enqueue(&p_dev->lite_cb.s.to_app.ipc_queue, p_msg);

    /* WakeUp IPC read  */
    wake_up_interruptible(&p_dev->rx_wait_q);
}

/*******************************************************************************
 **
 ** Function        btusb_lite_ipc_event_desc
 **
 ** Description     Get IPC Event description
 **
 ** Returns         status: <> 0 if the event must be send to user space (BSA)
 **                         0 if the event is handled
 **
 *******************************************************************************/
static char *btusb_lite_ipc_event_desc(UINT16 event)
{
    switch(event)
    {
    case BT_EVT_TO_LM_HCI_CMD:
        return "BT_EVT_TO_LM_HCI_CMD";
    case BT_EVT_TO_LM_HCI_ACL:
        return "BT_EVT_TO_LM_HCI_ACL";
    case BT_EVT_BTU_IPC_MGMT_EVT:
        return "BT_EVT_BTU_IPC_MGMT_EVT";
    case BT_EVT_BTU_IPC_BTU_EVT:
        return "BT_EVT_BTU_IPC_BTU_EVT";
    case BT_EVT_BTU_IPC_L2C_EVT:
        return "BT_EVT_BTU_IPC_L2C_EVT";
    case BT_EVT_BTU_IPC_ACL_EVT:
        return "BT_EVT_BTU_IPC_ACL_EVT";
    case BT_EVT_BTU_IPC_BTM_EVT:
        return "BT_EVT_BTU_IPC_BTM_EVT";
    case BT_EVT_BTU_IPC_L2C_MSG_EVT:
        return "BT_EVT_BTU_IPC_L2C_MSG_EVT";
    case BT_EVT_BTU_IPC_AVDT_EVT:
        return "BT_EVT_BTU_IPC_AVDT_EVT";
    case BT_EVT_BTU_IPC_SLIP_EVT:
        return "BT_EVT_BTU_IPC_SLIP_EVT";
    case BT_EVT_BTU_IPC_BTTRC_EVT:
        return "BT_EVT_BTU_IPC_BTTRC_EVT";
    case BT_EVT_BTU_IPC_BURST_EVT:
        return "BT_EVT_BTU_IPC_BURST_EVT";
    default:
        return "Unknown Event";
    }
}

