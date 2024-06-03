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

#include "btusb_proc.h"

#include <linux/slab.h> /* for kmalloc/kfree */
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h> /* GREG */

#include "btusb.h"

static struct proc_dir_entry *btusb_proc_dir = NULL;

/* Definition of the proc interface */
struct btusb_proc_file
{
    const char *name;
    umode_t mode;
    const struct seq_operations seq_ops;
};


/*******************************************************************************
 **
 ** Function         btusb_version_show
 **
 ** Description      Version display function
 **
 *******************************************************************************/
int btusb_version_show(struct seq_file *s, void *unused)
{
    /* s->private points to unknown data */
    seq_printf(s, "version : %s\n", bsa_version_string);
    return 0;
}

/*******************************************************************************
 **
 ** Function         btusb_info_show
 **
 ** Description      Information display function
 **
 *******************************************************************************/
int btusb_info_show(struct seq_file *s, void *unused)
{
    struct btusb *p_dev = s->private;
    const struct usb_device_id *p_id = p_dev->p_id;
    struct usb_device *udev = p_dev->p_udev;
    struct usb_host_interface *p_host_intf;
    struct usb_endpoint_descriptor *p_ep_desc;
    int idx, jdx;

    BTUSB_DBG("p_dev=%p\n", p_dev);

    seq_printf(s, "USB device :\n");
    seq_printf(s, "  - Match info:\n");
    if (p_id->match_flags & USB_DEVICE_ID_MATCH_VENDOR)
        seq_printf(s, "    * USB_DEVICE_ID_MATCH_VENDOR (0x%02X)\n", p_id->idVendor);
    if (p_id->match_flags & USB_DEVICE_ID_MATCH_PRODUCT)
        seq_printf(s, "    * USB_DEVICE_ID_MATCH_PRODUCT 0x%02X)\n", p_id->idProduct);
    if (p_id->match_flags & USB_DEVICE_ID_MATCH_DEV_LO)
        seq_printf(s, "    * USB_DEVICE_ID_MATCH_DEV_LO (%u)\n", p_id->bcdDevice_lo);
    if (p_id->match_flags & USB_DEVICE_ID_MATCH_DEV_HI)
        seq_printf(s, "    * USB_DEVICE_ID_MATCH_DEV_HI (%u)\n", p_id->bcdDevice_hi);
    if (p_id->match_flags & USB_DEVICE_ID_MATCH_DEV_CLASS)
        seq_printf(s, "    * USB_DEVICE_ID_MATCH_DEV_CLASS (0x%02X)\n", p_id->bDeviceClass);
    if (p_id->match_flags & USB_DEVICE_ID_MATCH_DEV_SUBCLASS)
        seq_printf(s, "    * USB_DEVICE_ID_MATCH_DEV_SUBCLASS (0x%02X)\n", p_id->bDeviceSubClass);
    if (p_id->match_flags & USB_DEVICE_ID_MATCH_DEV_PROTOCOL)
        seq_printf(s, "    * USB_DEVICE_ID_MATCH_DEV_PROTOCOL (0x%02X)\n", p_id->bDeviceProtocol);
    if (p_id->match_flags & USB_DEVICE_ID_MATCH_INT_CLASS)
        seq_printf(s, "    * USB_DEVICE_ID_MATCH_INT_CLASS (0x%02X)\n", p_id->bInterfaceClass);
    if (p_id->match_flags & USB_DEVICE_ID_MATCH_INT_SUBCLASS)
        seq_printf(s, "    * USB_DEVICE_ID_MATCH_INT_SUBCLASS (0x%02X)\n", p_id->bInterfaceSubClass);
    if (p_id->match_flags & USB_DEVICE_ID_MATCH_INT_PROTOCOL)
        seq_printf(s, "    * USB_DEVICE_ID_MATCH_INT_PROTOCOL (0x%02X)\n", p_id->bInterfaceProtocol);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0)
    if (p_id->match_flags & USB_DEVICE_ID_MATCH_INT_NUMBER)
        seq_printf(s, "    * USB_DEVICE_ID_MATCH_INT_NUMBER (%u)\n", p_id->bInterfaceNumber);
#endif

    seq_printf(s, "  - Address = %d\n", udev->devnum);
    seq_printf(s, "  - VendorId = %04x\n", le16_to_cpu(udev->descriptor.idVendor));
    seq_printf(s, "  - ProductId = %04x\n", le16_to_cpu(udev->descriptor.idProduct));
    seq_printf(s, "  - Manufacturer String = %s\n", udev->manufacturer);
    seq_printf(s, "  - Product String = %s\n", udev->product);
    seq_printf(s, "  - USB bus number = %d\n", udev->bus->busnum);
    seq_printf(s, "  - USB devpath = %s\n", udev->devpath);
    seq_printf(s, "  - USB devnum = %d\n", udev->devnum);
    seq_printf(s, "  - USB ttport = %d\n", udev->ttport);
    seq_printf(s, "  - Interfaces :\n");
    seq_printf(s, "    * MAIN : ");
    if (p_dev->p_main_intf)
    {
        seq_printf(s, "intf = %d (nb alt settings = %d, ",
                p_dev->p_main_intf->cur_altsetting->desc.bInterfaceNumber,
                p_dev->p_main_intf->num_altsetting);
        seq_printf(s, "cur alt setting = %d)\n", p_dev->p_main_intf->cur_altsetting->desc.bAlternateSetting);
        seq_printf(s, "      * HCI EVENT : ");
        if (p_dev->p_event_in)
        {
            seq_printf(s, "ep = 0x%02x\n", p_dev->p_event_in->desc.bEndpointAddress);
        }
        else
        {
            seq_printf(s, "ERROR (endpoint not found)\n");
        }
        seq_printf(s, "      * ACL RX : ");
        if (p_dev->p_acl_in)
        {
            seq_printf(s, "ep = 0x%02x\n", p_dev->p_acl_in->desc.bEndpointAddress);
        }
        else
        {
            seq_printf(s, "ERROR (endpoint not found)\n");
        }
        seq_printf(s, "      * ACL TX : ");
        if (p_dev->p_acl_out)
        {
            seq_printf(s, "ep = 0x%02x\n", p_dev->p_acl_out->desc.bEndpointAddress);
        }
        else
        {
            seq_printf(s, "ERROR (endpoint not found)\n");
        }
    }
    else
    {
        seq_printf(s, "Not present\n");
    }
    seq_printf(s, "    * VOICE :");
    if (p_dev->p_voice_intf)
    {
        seq_printf(s, " intf = %d (nb alt setting = %d, ", p_dev->p_voice_intf->cur_altsetting->desc.bInterfaceNumber, p_dev->p_voice_intf->num_altsetting);
        seq_printf(s, "cur alt setting = %d)\n", p_dev->p_voice_intf->cur_altsetting->desc.bAlternateSetting);
        for (idx = 0; idx < p_dev->p_voice_intf->num_altsetting; idx++)
        {
            p_host_intf = &p_dev->p_voice_intf->altsetting[idx];
            seq_printf(s, "      * alt setting %d (idx %d) : %d enpoints\n",
                    p_host_intf->desc.bAlternateSetting, idx, p_host_intf->desc.bNumEndpoints);
            for (jdx = 0; jdx < p_host_intf->desc.bNumEndpoints; jdx++)
            {
                p_ep_desc = &p_host_intf->endpoint[jdx].desc;
                seq_printf(s, "        *  ep = 0x%02x : ", p_ep_desc->bEndpointAddress);
                if (usb_endpoint_type(p_ep_desc) == USB_ENDPOINT_XFER_ISOC)
                {
                    seq_printf(s, "Isoch ");
                    if (usb_endpoint_dir_out(p_ep_desc))
                    {
                        seq_printf(s, "(OUT) ");
                    }
                    else
                    {
                        seq_printf(s, "(IN)  ");
                    }
                    seq_printf(s, "wMaxPacketSize = %d\n", le16_to_cpu(p_ep_desc->wMaxPacketSize));
                }
                else
                {
                    seq_printf(s, "not isochronous endpoint\n");
                }
            }
        }
    }
    else
    {
        seq_printf(s, "Not present\n");
    }
    seq_printf(s, "    * DIAG RX : ");
    if (p_dev->p_diag_in)
    {
        seq_printf(s, "intf = %d ep = 0x%02x\n", p_dev->p_diag_intf->cur_altsetting->desc.bInterfaceNumber, p_dev->p_diag_in->desc.bEndpointAddress);
    }
    else
    {
        seq_printf(s, "Not present\n");
    }
    seq_printf(s, "    * DIAG TX : ");
    if (p_dev->p_diag_out)
    {
        seq_printf(s, "intf = %d ep = 0x%02x\n", p_dev->p_diag_intf->cur_altsetting->desc.bInterfaceNumber, p_dev->p_diag_out->desc.bEndpointAddress);
    }
    else
    {
        seq_printf(s, "Not present\n");
    }
    seq_printf(s, "    * DFU : ");
    if (p_dev->p_dfu_intf)
    {
        seq_printf(s, "intf = %d\n", p_dev->p_dfu_intf->cur_altsetting->desc.bInterfaceNumber);
    }
    else
    {
        seq_printf(s, "Not present\n");
    }

    seq_printf(s, "Memory usage :\n");
    seq_printf(s, "  - p_dev = %p\n", p_dev);
    seq_printf(s, "  - size = %zd\n", sizeof(*p_dev));
    seq_printf(s, "    * CMD = off:%zd/size=%zd\n", offsetof(struct btusb, cmd_array), sizeof(p_dev->cmd_array));
    seq_printf(s, "    * EVENT = off:%zd/size=%zd\n", offsetof(struct btusb, event_array), sizeof(p_dev->event_array));
    seq_printf(s, "    * ACL RX = off:%zd/size=%zd\n", offsetof(struct btusb, acl_rx_array), sizeof(p_dev->acl_rx_array));
    seq_printf(s, "    * ACL TX = off:%zd/size=%zd\n", offsetof(struct btusb, acl_tx_array), sizeof(p_dev->acl_tx_array));
    seq_printf(s, "    * DIAG RX = off:%zd/size=%zd\n", offsetof(struct btusb, diag_rx_array), sizeof(p_dev->diag_rx_array));
    seq_printf(s, "    * DIAG TX = off:%zd/size=%zd\n", offsetof(struct btusb, diag_tx_array), sizeof(p_dev->diag_tx_array));
    seq_printf(s, "    * VOICE = off:%zd/size=%zd\n", offsetof(struct btusb, voice_rx.channels), offsetof(struct btusb, rx_queue)-offsetof(struct btusb, voice_rx.channels));

    seq_printf(s, "Config :\n");
    seq_printf(s, "  - issharedusb = %u\n", p_dev->issharedusb);
    seq_printf(s, "  - quirks = %u\n", p_dev->quirks);
    return 0;
}

/*******************************************************************************
 **
 ** Function         btusb_status_show
 **
 ** Description      Status display function
 **
 *******************************************************************************/
int btusb_status_show(struct seq_file *s, void *unused)
{
    struct btusb *p_dev = s->private;
    struct btusb_voice_channel *p_chan;
    struct btusb_scosniff *p, *n;
    int idx, jdx;

    BTUSB_DBG("p_dev=%p\n", p_dev);

    seq_printf(s, "Voice :\n");
    jdx = 0;
    for (idx = 0; idx < ARRAY_SIZE(p_dev->voice_rx.channels); idx++)
    {
        p_chan = &p_dev->voice_rx.channels[idx];
        if (p_chan->used)
        {
            seq_printf(s, "  - channel %d : SCO handle = %d(0x%02x) burst = %d\n", idx,
                    p_chan->handle, p_chan->handle, p_chan->burst);
            jdx = 1;
        }
    }
    if (!jdx)
    {
        seq_printf(s, "  - No active channels\n");
    }
    seq_printf(s, "SCO sniffing :\n");
    seq_printf(s, "  - list: %p\n", &p_dev->scosniff_list);
    seq_printf(s, "  - list.next: %p\n", p_dev->scosniff_list.next);
    seq_printf(s, "  - list.prev: %p\n", p_dev->scosniff_list.prev);
    seq_printf(s, "  - whole list:\n");
    list_for_each_entry_safe(p, n, &p_dev->scosniff_list, lh)
    {
        seq_printf(s, "    >  %p\n", p);
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function         btusb_stats_show
 **
 ** Description      Statistics display function
 **
 *******************************************************************************/
#define BTUSB_STATS(__n) \
    seq_printf(s, #__n " = %lu\n", p_dev->stats.__n)
int btusb_stats_show(struct seq_file *s, void *unused)
{
    struct btusb *p_dev = s->private;

    BTUSB_STATS(urb_submit_ok);
    BTUSB_STATS(urb_submit_err);
    BTUSB_STATS(acl_rx_submit_ok);
    BTUSB_STATS(acl_rx_submit_err);
    BTUSB_STATS(acl_rx_complete);
    BTUSB_STATS(acl_rx_complete_err);
    BTUSB_STATS(acl_rx_resubmit);
    BTUSB_STATS(acl_rx_bytes);
    BTUSB_STATS(event_submit_ok);
    BTUSB_STATS(event_submit_err);
    BTUSB_STATS(event_complete);
    BTUSB_STATS(event_complete_err);
    BTUSB_STATS(event_resubmit);
    BTUSB_STATS(event_bytes);
    BTUSB_STATS(diag_rx_submit_ok);
    BTUSB_STATS(diag_rx_submit_err);
    BTUSB_STATS(diag_rx_complete);
    BTUSB_STATS(diag_rx_complete_err);
    BTUSB_STATS(diag_rx_resubmit);
    BTUSB_STATS(diag_rx_bytes);
    BTUSB_STATS(diag_rx_bytes);
    BTUSB_STATS(urb_out_complete);
    BTUSB_STATS(urb_out_complete_err);
    BTUSB_STATS(cmd_submit_ok);
    BTUSB_STATS(cmd_submit_err);
    BTUSB_STATS(cmd_complete);
    BTUSB_STATS(cmd_complete_err);
    BTUSB_STATS(voicerx_submit_ok);
    BTUSB_STATS(voicerx_submit_err);
    BTUSB_STATS(voicerx_complete);
    BTUSB_STATS(voicerx_complete_err);
    BTUSB_STATS(voicerx_bad_frames);
    BTUSB_STATS(voicerx_bad_hdr);
    BTUSB_STATS(voicerx_split_hdr);
    BTUSB_STATS(voicerx_raw_bytes);
    BTUSB_STATS(voicerx_skipped_bytes);
    BTUSB_STATS(voicerx_bad_size);
    BTUSB_STATS(voicetx_submit_ok);
    BTUSB_STATS(voicetx_submit_err);
    BTUSB_STATS(voicetx_nobuf);
    BTUSB_STATS(voicetx_complete);
    BTUSB_STATS(voicetx_complete_err);
    return 0;
}

/*******************************************************************************
 **
 ** Function         btusb_scosniff_start
 **
 ** Description      SCO sniffing sequence start function
 **
 *******************************************************************************/
void *btusb_scosniff_start(struct seq_file *s, loff_t *pos)
{
    struct btusb *p_dev = s->private;
    struct btusb_scosniff *bs;
    int rv;

    BTUSB_INFO("waiting %p\n", p_dev);
    rv = wait_for_completion_interruptible(&p_dev->scosniff_completion);
    if (rv < 0)
        return NULL;

    BTUSB_INFO("triggered\n");

    if (!list_empty(&p_dev->scosniff_list))
    {
        bs = list_first_entry(&p_dev->scosniff_list, struct btusb_scosniff, lh);

        /* remove the element from the list */
        list_del(&bs->lh);
        BTUSB_INFO("receiving %p\n", bs);

        return bs;
    }
    return NULL;
}

/*******************************************************************************
 **
 ** Function         btusb_scosniff_next
 **
 ** Description      SCO sniffing sequence next function
 **
 *******************************************************************************/
void *btusb_scosniff_next(struct seq_file *s, void *v, loff_t *pos)
{
    struct btusb_scosniff *bs = v;
    BTUSB_INFO("next\n");

    kfree(bs);

    (*pos)++;

    /* if you do not want to buffer the data, just return NULL, otherwise, call start
     * again in order to use as much of the allocated PAGE in seq_read
     *
     * optional:
     *     return btusb_scosniff_start(s, pos);
     */
    return NULL;
}

/*******************************************************************************
 **
 ** Function         btusb_scosniff_stop
 **
 ** Description      SCO sniffing sequence stop function
 **
 *******************************************************************************/
void btusb_scosniff_stop(struct seq_file *s, void *v)
{
    BTUSB_INFO("stop\n");
}

/*******************************************************************************
 **
 ** Function         btusb_scosniff_show
 **
 ** Description      SCO sniffing display function
 **
 *******************************************************************************/
int btusb_scosniff_show(struct seq_file *s, void *v)
{
    struct btusb_scosniff *bs = v;
    unsigned int i, j;
    unsigned char *p_buf, *p_c;
    unsigned char c;
    struct usb_iso_packet_descriptor *p_uipd;
    const char hexdigit[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

    seq_printf(s, "%u %u %d \n", bs->n, bs->l, bs->s);
    p_buf = (unsigned char *)&bs->d[bs->n];
    for (i = 0; i < bs->n; i++)
    {
        p_uipd = &bs->d[i];
        seq_printf(s, "  %d %u %u %u\n", p_uipd->status, p_uipd->actual_length, p_uipd->length, p_uipd->offset);
        for (j = 0, p_c = &p_buf[p_uipd->offset]; j < p_uipd->actual_length; j++, p_c++)
        {
            c = *p_c;
            seq_putc(s, hexdigit[c >> 4]);
            seq_putc(s, hexdigit[c & 0xF]);
            seq_putc(s, ' ');
        }
        seq_putc(s, '\n');
    }

    return 0;
}

const struct seq_operations btusb_scosniff_seq_ops =
{
    .start = btusb_scosniff_start,
    .next  = btusb_scosniff_next,
    .stop  = btusb_scosniff_stop,
    .show  = btusb_scosniff_show,
};

/*******************************************************************************
 **
 ** Function         btusb_proc_open
 **
 ** Description      Open handler of the proc interface
 **
 *******************************************************************************/
int btusb_proc_open(struct inode *inode, struct file *file)
{
    int rv;
    struct btusb_proc_file *p_file = BTUSB_PDE_DATA(inode);
    /* in case of proc_files, there is no p_dev */
    struct btusb *p_dev = BTUSB_PDE_PARENT_DATA(inode);

    BTUSB_DBG("p_dev=%p\n", p_dev);
    /* check if it is a single sequence */
    if (!p_file->seq_ops.start)
    {
        return single_open(file, p_file->seq_ops.show, p_dev);
    }
    else
    {
        rv = seq_open(file, &p_file->seq_ops);
        if (!rv)
        {
            p_dev->scosniff_active = true;
            ((struct seq_file *)file->private_data)->private = p_dev;
        }
        return rv;
    }
    return -1;
}

/*******************************************************************************
 **
 ** Function         btusb_proc_release
 **
 ** Description      Close handler of the proc interface
 **
 *******************************************************************************/
int btusb_proc_release(struct inode *inode, struct file *file)
{
    struct btusb_proc_file *p_file = BTUSB_PDE_DATA(inode);
    struct btusb *p_dev = BTUSB_PDE_PARENT_DATA(inode);
    struct btusb_scosniff *p, *n;

    BTUSB_DBG("p_dev=%p\n", p_dev);
    if (!p_file->seq_ops.start)
    {
        return single_release(inode, file);
    }
    else
    {
        p_dev->scosniff_active = false;
        list_for_each_entry_safe(p, n, &p_dev->scosniff_list, lh)
        {
            list_del(&p->lh);
            kfree(p);
        }
        return seq_release(inode, file);
    }
    return -1;
}

/*******************************************************************************
 **
 ** Function         btusb_proc_write
 **
 ** Description      Write handler of the proc interface
 **
 *******************************************************************************/
ssize_t btusb_proc_write(struct file *file, const char *buf,
        size_t count, loff_t *pos)
{
    struct seq_file *s = file->private_data;
    struct btusb *p_dev = s->private;
    unsigned char cmd;
    pm_message_t tmp;

    /* copy the first byte from the data written */
    if (copy_from_user(&cmd, buf, 1))
    {
        return -EFAULT;
    }

    /* unconditional print on purpose */
    BTUSB_INFO("'%c'\n", cmd);

    switch (cmd)
    {
    case '0':
        /* reset the stats */
        memset(&p_dev->stats, 0, sizeof(p_dev->stats));
        break;
    case '1':
        btusb_add_voice_channel(p_dev, 6, BTUSB_VOICE_BURST_SIZE);
        break;

    case '2':
        btusb_remove_voice_channel(p_dev, 6);
        break;

    case '3':
        if (p_dev->scosniff_active)
        {
            struct btusb_scosniff *bs;
            bs = kmalloc(sizeof(*bs), GFP_ATOMIC);
            if (bs)
            {
                BTUSB_INFO("SCOSNIFF: adding %p\n", bs);
                bs->n = 0;
                list_add_tail(&bs->lh, &p_dev->scosniff_list);
                complete(&p_dev->scosniff_completion);
            }
        }
        break;

#ifdef BTUSB_LITE
    case '4':
        BTUSB_INFO("Mute PCM0\n");
        pcm0_mute = 1;
        break;

    case '5':
        BTUSB_INFO("Unmute PCM0\n");
        pcm0_mute = 0;
        break;
#endif

    case '6':
        BTUSB_INFO("BTUSB_SUSPEND\n");
        btusb_suspend(p_dev->p_main_intf, tmp);
        break;

    case '7':
        BTUSB_INFO("BTUSB_RESUME\n");
        btusb_resume(p_dev->p_main_intf);
        break;

    default:
        break;
    }

    return count;
}

static struct btusb_proc_file btusb_proc_files[] =
{
    {
        "version", S_IRUGO,
        {
            .show=btusb_version_show,
            .start=NULL
        }
    },
    {NULL, 0, {.start=NULL}}
};

static struct btusb_proc_file btusb_proc_dev_files[] =
{
    {
        "info", S_IRUGO,
        {
            .show=btusb_info_show,
            .start=NULL
        }
    },
    {
        "stats", S_IRUGO | S_IWUGO,
        {
            .show=btusb_stats_show,
            .start=NULL
        }
    },
    {
        "status", S_IRUGO,
        {
            .show=btusb_status_show,
            .start=NULL
        }
    },
    {
        "scosniff", S_IRUGO,
        {
            .show=btusb_scosniff_show,
            .start=btusb_scosniff_start,
            .stop=btusb_scosniff_stop,
            .next=btusb_scosniff_next
        }
    },
    {NULL, 0, {.start=NULL}}
};

static const struct file_operations btusb_proc_fops =
{
    .open    = btusb_proc_open,
    .read    = seq_read,
    .write   = btusb_proc_write,
    .llseek  = seq_lseek,
    .release = btusb_proc_release,
};


/*******************************************************************************
 **
 ** Function         btusb_proc_create_files
 **
 ** Description      Proc interface file creation
 **
 *******************************************************************************/
static void btusb_proc_create_files(struct btusb_proc_file proc_files[],
        struct proc_dir_entry *p_parent)
{
    struct proc_dir_entry *p_pde;
    struct btusb_proc_file *p_file;

    if (p_parent)
    {
        for (p_file = proc_files; p_file->name; p_file++)
        {
            p_pde = proc_create_data(p_file->name, p_file->mode,
                    p_parent, &btusb_proc_fops, p_file);
            if (!p_pde)
            {
                BTUSB_ERR("failed creating %s\n", p_file->name);
            }
        }
    }
}

/*******************************************************************************
 **
 ** Function         btusb_proc_remove_files
 **
 ** Description      Proc interface file removal
 **
 *******************************************************************************/
static void btusb_proc_remove_files(struct btusb_proc_file proc_files[],
        struct proc_dir_entry *p_parent)
{
    struct btusb_proc_file *p_file;
    if (p_parent)
    {
        for (p_file = proc_files; p_file->name; p_file++)
        {
            remove_proc_entry(p_file->name, p_parent);
        }
    }
}

/*******************************************************************************
 **
 ** Function         btusb_proc_init
 **
 ** Description      Proc interface init
 **
 *******************************************************************************/
void btusb_proc_init(void)
{
    btusb_proc_dir = proc_mkdir("driver/btusb", NULL);
    btusb_proc_create_files(btusb_proc_files, btusb_proc_dir);
}

/*******************************************************************************
 **
 ** Function         btusb_proc_exit
 **
 ** Description      Proc interface exit
 **
 *******************************************************************************/
void btusb_proc_exit(void)
{
    btusb_proc_remove_files(btusb_proc_files, btusb_proc_dir);
    remove_proc_entry("driver/btusb", NULL);
    btusb_proc_dir = NULL;
}

/*******************************************************************************
 **
 ** Function         btusb_proc_add
 **
 ** Description      Add a device to the proc interface
 **
 *******************************************************************************/
void btusb_proc_add(struct btusb *p_dev, const char *name)
{
    btusb_proc_dir = proc_mkdir("driver/btusb", NULL);
    btusb_proc_create_files(btusb_proc_files, btusb_proc_dir);

    if (btusb_proc_dir)
    {
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
        p_dev->p_pde = create_proc_entry(name, S_IFDIR | S_IRUGO | S_IWUGO | S_IXUGO, btusb_proc_dir);
        if (p_dev->p_pde)
            p_dev->p_pde->data = p_dev;
#elif LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
        p_dev->p_pde = proc_mkdir_mode(name, S_IRUGO | S_IWUGO | S_IXUGO, btusb_proc_dir);
        if (p_dev->p_pde)
            p_dev->p_pde->data = p_dev;
#else
        p_dev->p_pde = proc_mkdir_data(name, S_IRUGO | S_IWUGO | S_IXUGO, btusb_proc_dir, p_dev);
#endif
        if (!p_dev->p_pde)
        {
            BTUSB_ERR("Couldn't create proc entry %s\n", name);
            return;
        }

        btusb_proc_create_files(btusb_proc_dev_files, p_dev->p_pde);
    }
}

/*******************************************************************************
 **
 ** Function         btusb_proc_remove
 **
 ** Description      Remove a device from the proc interface
 **
 *******************************************************************************/
void btusb_proc_remove(struct btusb *p_dev, const char *name)
{
    btusb_proc_remove_files(btusb_proc_dev_files, p_dev->p_pde);
    remove_proc_entry(name, btusb_proc_dir);

    btusb_proc_remove_files(btusb_proc_files, btusb_proc_dir);
    remove_proc_entry("driver/btusb", NULL);
    btusb_proc_dir = NULL;
}

