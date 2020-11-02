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

#ifndef BTUSB_H
#define BTUSB_H

#include <linux/version.h>
#include <linux/usb.h>
#include <linux/tty.h>
#include <linux/time.h>

#include "bt_types.h"
#include "btusb_cq.h"
#include "gki_int.h"
#include "btusbext.h"

#ifdef BTUSB_LITE
#include "btusb_lite.h"
#endif

extern const char bsa_version_string[];

/* Linux kernel compatibility abstraction */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 34)
#define BTUSB_USHRT_MAX USHORT_MAX
#else
#define BTUSB_USHRT_MAX USHRT_MAX
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
#define BTUSB_PDE_DATA(inode) PDE(inode)->data
#define BTUSB_PDE_PARENT_DATA(inode) PDE(inode)->parent->data
#else
#define BTUSB_PDE_DATA(inode) PDE_DATA(inode)
#define BTUSB_PDE_PARENT_DATA(inode) proc_get_parent_data(inode)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 34)
#define BTUSB_BUFFER_ALLOC usb_buffer_alloc
#define BTUSB_BUFFER_FREE usb_buffer_free
#else
#define BTUSB_BUFFER_ALLOC usb_alloc_coherent
#define BTUSB_BUFFER_FREE usb_free_coherent
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 30)
#define BTUSB_EP_TYPE(ep) (ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
#define BTUSB_EP_DIR_IN(ep) ((ep->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)
#define BTUSB_EP_DIR_OUT(ep) ((ep->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT)
#else
#define BTUSB_EP_TYPE(ep) usb_endpoint_type(ep)
#define BTUSB_EP_DIR_IN(ep) usb_endpoint_dir_in(ep)
#define BTUSB_EP_DIR_OUT(ep) usb_endpoint_dir_out(ep)
#endif

/* debug information flags. one-hot encoded:
   bit0 : enable printk(KERN_DEBUG)
   bit1 : enable printk(KERN_INFO)
   bit16 : enable all message dumps in printk
   bit17 : enable timestamp calculations on voice RX packets
   bit18 : enable timestamp calculations on voice TX packets
   bit19 : enable GKI buffer check */
#define BTUSB_DBG_MSG           0x0001
#define BTUSB_INFO_MSG          0x0002
#define BTUSB_DUMP_MSG          0x0100
#define BTUSB_VOICERX_TIME      0x0200
#define BTUSB_VOICETX_TIME      0x0400
#define BTUSB_GKI_CHK_MSG       0x0800
/* #define BTUSB_DBGFLAGS (BTUSB_DBG_MSG | BTUSB_INFO_MSG | BTUSB_DUMP_MSG | BTUSB_GKI_CHK_MSG) */
#define BTUSB_DBGFLAGS 0

extern int dbgflags;

#define BTUSB_DBG(fmt, ...) if (unlikely(dbgflags & BTUSB_DBG_MSG)) \
    printk(KERN_DEBUG "BTUSB %s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#define BTUSB_INFO(fmt, ...) if (unlikely(dbgflags & BTUSB_INFO_MSG))\
    printk(KERN_INFO "BTUSB %s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#define BTUSB_ERR(fmt, ...) \
    printk(KERN_ERR "BTUSB %s: " fmt, __FUNCTION__, ##__VA_ARGS__)

/* Get a minor range for your devices from the usb maintainer */
#define BTUSB_MINOR_BASE 194

/* 1025 = size(con_hdl) + size(acl_len) + size(3-dh5) = 2 + 2 + 1021 */
#define BTUSB_HCI_MAX_ACL_SIZE 1025
/* Maximum size of command and events packets (events = 255 + 2, commands = 255 + 3) */
#define BTUSB_HCI_MAX_CMD_SIZE 258
#define BTUSB_HCI_MAX_EVT_SIZE 257

/* Maximum HCI H4 size = HCI type + ACL packet */
#define BTUSB_H4_MAX_SIZE (1 + BTUSB_HCI_MAX_ACL_SIZE)

#define BTUSB_NUM_OF_ACL_RX_BUFFERS   12
#define BTUSB_NUM_OF_ACL_TX_BUFFERS   12
#define BTUSB_NUM_OF_DIAG_RX_BUFFERS   2 /* must not be less than 2 */
#define BTUSB_NUM_OF_DIAG_TX_BUFFERS   2
#define BTUSB_NUM_OF_EVENT_BUFFERS     8 /* must not be less than 2 */
#define BTUSB_NUM_OF_CMD_BUFFERS       8
#define BTUSB_MAXIMUM_TX_VOICE_SIZE  192
#define BTUSB_NUM_OF_VOICE_RX_BUFFERS  2 /* must not be less than 2 */
#define BTUSB_NUM_OF_VOICE_RX_PACKETS  8 /* maximum voice packets pending for user */
#define BTUSB_NUM_OF_VOICE_TX_BUFFERS 16

#define BTUSB_VOICE_BURST_SIZE 48
#define BTUSB_VOICE_HEADER_SIZE 3
#define BTUSB_VOICE_FRAMES_PER_URB 9
#define BTUSB_VOICE_BUFFER_MAXSIZE (BTUSB_VOICE_FRAMES_PER_URB * \
        ALIGN(BTUSB_VOICE_BURST_SIZE + BTUSB_VOICE_HEADER_SIZE, 4))

/* size of the SCO packets sent to user application (5 * 48 is the max below 256) */
#define BTUSB_SCO_RX_LEN (BTUSB_VOICE_BURST_SIZE * 5)
#if BTUSB_SCO_RX_LEN > 256
#error SCO RX packet length is larger than the format supports
#endif

#ifndef UINT8_TO_STREAM
#define UINT8_TO_STREAM(p, u8)   {*(p)++ = (UINT8)(u8);}
#define UINT16_TO_STREAM(p, u16) {*(p)++ = (UINT8)(u16); *(p)++ = (UINT8)((u16) >> 8);}
#define BDADDR_TO_STREAM(p, a)   {register int ijk; for (ijk = 0; ijk < BD_ADDR_LEN;  ijk++) *(p)++ = (UINT8) a[BD_ADDR_LEN - 1 - ijk];}
#define ARRAY_TO_STREAM(p, a, len) {register int ijk; for (ijk = 0; ijk < len; ijk++) *(p)++ = (UINT8) a[ijk];}

#define STREAM_TO_UINT8(u8, p)   {u8 = (UINT8)(*(p)); (p) += 1;}
#define STREAM_TO_UINT16(u16, p) {u16 = (UINT16)((*(p)) + ((*((p) + 1)) << 8)); (p) += 2;}
#define STREAM_TO_UINT32(u32, p) {u32 = (((UINT32)(*(p))) + ((((UINT32)(*((p) + 1)))) << 8) + ((((UINT32)(*((p) + 2)))) << 16) + ((((UINT32)(*((p) + 3)))) << 24)); (p) += 4;}
#define STREAM_TO_BDADDR(a, p)   {register int ijk; register UINT8 *pbda = (UINT8 *)a + BD_ADDR_LEN - 1; for (ijk = 0; ijk < BD_ADDR_LEN; ijk++) *pbda-- = *p++;}
#define STREAM_TO_ARRAY(a, p, len) {register int ijk; for (ijk = 0; ijk < len; ijk++) ((UINT8 *) a)[ijk] = *p++;}
#endif

#define UINT32_TO_BE_STREAM(p, u32) {*(p)++ = (UINT8)((u32) >> 24);  *(p)++ = (UINT8)((u32) >> 16); *(p)++ = (UINT8)((u32) >> 8); *(p)++ = (UINT8)(u32); }
#define UINT24_TO_BE_STREAM(p, u24) {*(p)++ = (UINT8)((u24) >> 16); *(p)++ = (UINT8)((u24) >> 8); *(p)++ = (UINT8)(u24);}
#define UINT16_TO_BE_STREAM(p, u16) {*(p)++ = (UINT8)((u16) >> 8); *(p)++ = (UINT8)(u16);}
#define UINT8_TO_BE_STREAM(p, u8)   {*(p)++ = (UINT8)(u8);}

/* macro that helps parsing arrays */
#define BTUSB_ARRAY_FOR_EACH_TX_TRANS(__a) \
    for (idx = 0, p_tx_trans = &__a[0]; idx < ARRAY_SIZE(__a); idx++, p_tx_trans = &__a[idx])

#define BTUSB_ARRAY_FOR_EACH_RX_TRANS(__a) \
    for (idx = 0, p_rx_trans = &__a[0]; idx < ARRAY_SIZE(__a); idx++, p_rx_trans = &__a[idx])

/* Layer Specific field: Used to send packet to User Space */
#define BTUSB_LS_H4_TYPE_SENT   (1<<0)  /* H4 HCI Type already sent */
#define BTUSB_LS_GKI_BUFFER     (1<<1)  /* Locally allocated buffer-not to resubmit */

struct btusb;

/* Container used to copy the URB to sniff the SCO */
struct btusb_scosniff
{
    struct list_head lh;                    /* to add element to a list */
    int s;                                  /* start frame */
    unsigned int n;                         /* number of descriptors */
    unsigned int l;                         /* buffer length */
    struct usb_iso_packet_descriptor d[0];  /* descriptors */
};

/* USB TX transaction */
struct btusb_tx_trans
{
    bool in_use;
    /* Pointer to the location where the data is stored */
    UINT8 *dma_buffer;
    /* DMA structure */
    dma_addr_t dma;
    /* URB for this transaction */
    struct urb *p_urb;
    /* pointer to the device information */
    struct btusb *p_dev;
    void (*complete)(struct btusb *p_dev, struct btusb_tx_trans *p_tx_trans, struct urb *p_urb);
};

/* USB RX transaction */
struct btusb_rx_trans
{
    /* This is mapped to a GKI buffer to allow queuing */
    BUFFER_HDR_T gki_hdr;
    /* Sharing queue with other packets -> needs BT header to multiplex */
    BT_HDR bt_hdr;
    /* Pointer to the location where the data is stored */
    UINT8 *dma_buffer;
    /* DMA structure */
    dma_addr_t dma;
    /* URB for this transaction */
    struct urb *p_urb;
    /* pointer to the device information */
    struct btusb *p_dev;
#if (GKI_ENABLE_BUF_CORRUPTION_CHECK == TRUE)
    /* Magic number */
    UINT32 magic;
#endif
};

/* Voice packet */
struct btusb_voice_pkt
{
    /* This is mapped to a GKI buffer to allow queuing */
    BUFFER_HDR_T gki_hdr;
    /* Sharing queue with other packets -> needs BT header to multiplex */
    BT_HDR bt_hdr;
    UINT8 data[BTUSB_VOICE_HEADER_SIZE + BTUSB_SCO_RX_LEN];
#if (GKI_ENABLE_BUF_CORRUPTION_CHECK == TRUE)
    /* Magic number */
    UINT32 magic;
#endif
};

/* Voice channel descriptor */
struct btusb_voice_channel
{
    bool used;
    unsigned short handle;
    unsigned char burst;
    /* pointer to HCI packet being reconstructed */
    struct btusb_voice_pkt *p_pkt;
};

/* hardware quirks required for some BT controllers */
#define BTUSB_QUIRK_ZLP_TX_REQ (1 << 0)
#define BTUSB_QUIRK_EXAMPLE    (1 << 1)

/* Define the main structure */
struct btusb
{
    struct usb_device *p_udev;              /* usb device for this device */
    const struct usb_device_id *p_id;       /* device id from probe */
    struct usb_interface *p_main_intf;      /* main interface reference */
    struct usb_interface *p_voice_intf;     /* voice interface reference */
    struct usb_interface *p_dfu_intf;       /* DFU interface reference */
    struct usb_interface *p_diag_intf;      /* diag interface reference */

    struct usb_host_endpoint *p_acl_in;     /* acl bulk in endpoint */
    struct usb_host_endpoint *p_acl_out;    /* acl bulk out endpoint */
    struct usb_host_endpoint *p_diag_in;    /* diag bulk in endpoint */
    struct usb_host_endpoint *p_diag_out;   /* diag bulk out endpoint */
    struct usb_host_endpoint *p_event_in;   /* event interrupt in endpoint */
    struct usb_host_endpoint *p_voice_out;  /* isoc voice out endpoint */
    struct usb_host_endpoint *p_voice_in;   /* isoc voice in endpoint */
    struct ktermios kterm;                  /* TTY emulation */
    struct kref kref;
    tBTUSB_STATS stats;
    bool issharedusb;
    unsigned int quirks;

    /* reception queue */
    wait_queue_head_t rx_wait_q;

    /* proc filesystem entry to retrieve info from driver environment */
    struct proc_dir_entry *p_pde;
    bool scosniff_active;
    struct list_head scosniff_list;
    struct completion scosniff_completion;

    /* Command transmit path */
    struct btusb_tx_trans cmd_array[BTUSB_NUM_OF_CMD_BUFFERS];
    struct usb_ctrlrequest cmd_req_array[BTUSB_NUM_OF_CMD_BUFFERS];
    struct usb_anchor cmd_submitted;

    /* Event receive path */
    struct btusb_rx_trans event_array[BTUSB_NUM_OF_EVENT_BUFFERS];
    struct usb_anchor event_submitted;

    /* ACL receive path */
    struct btusb_rx_trans acl_rx_array[BTUSB_NUM_OF_ACL_RX_BUFFERS];
    struct usb_anchor acl_rx_submitted;

    /* ACL transmit path */
    struct btusb_tx_trans acl_tx_array[BTUSB_NUM_OF_ACL_TX_BUFFERS];
    struct usb_anchor acl_tx_submitted;

    /* Diagnostics receive path */
    struct btusb_rx_trans diag_rx_array[BTUSB_NUM_OF_DIAG_RX_BUFFERS];
    struct usb_anchor diag_rx_submitted;

    /* Diagnostics transmit path */
    struct btusb_tx_trans diag_tx_array[BTUSB_NUM_OF_DIAG_TX_BUFFERS];
    struct usb_anchor diag_tx_submitted;

    /* Voice receive path */
    struct btusb_rx_trans voice_rx_array[BTUSB_NUM_OF_VOICE_RX_BUFFERS];
    struct usb_anchor voice_rx_submitted;
    struct btusb_voice_pkt voice_rx_pkts[BTUSB_NUM_OF_VOICE_RX_PACKETS];
    DECLARE_BTUSB_CQ(voice_rx_list, struct btusb_voice_pkt *, BTUSB_NUM_OF_VOICE_RX_PACKETS);

    struct
    {
        struct btusb_voice_channel channels[3];
        unsigned int remaining;
        struct btusb_voice_pkt **pp_pkt;
        unsigned char hdr[BTUSB_VOICE_HEADER_SIZE];
        unsigned int hdr_size;
    } voice_rx;

    /* Voice transmit path */
    struct btusb_tx_trans voice_tx_array[BTUSB_NUM_OF_VOICE_TX_BUFFERS];
    struct usb_anchor voice_tx_submitted;
    atomic_t voice_tx_active;

    char write_msg[BTUSB_H4_MAX_SIZE];
    size_t write_msg_len;

    BUFFER_Q rx_queue;
    BT_HDR *p_rx_msg;

#ifdef BTUSB_LITE
    struct btusb_lite_cb lite_cb;
#endif
};

/* Function prototypes */
void btusb_delete(struct kref *kref);
void btusb_cancel_voice(struct btusb *p_dev);
void btusb_voice_stats(unsigned long *p_max, unsigned long *p_min,
        struct timeval *p_result, struct timeval *p_last_time);
int btusb_submit_cmd(struct btusb *p_dev, char *packet, unsigned long length);
int btusb_submit_acl(struct btusb *p_dev, char *packet, unsigned long length);
int btusb_submit_voice_tx(struct btusb *p_dev, char *p_data, unsigned long lenth);
int btusb_submit_diag(struct btusb *p_dev, char *packet, unsigned long length);

/* URB submit */
int btusb_submit(struct btusb *p_dev, struct usb_anchor *p_anchor, struct urb *p_urb, int mem_flags);
void btusb_submit_voice_rx(struct btusb *p_dev, struct urb *p_urb, int mem_flags);

/* BT controller to host routines */
void btusb_rx_enqueue(struct btusb *p_dev, struct btusb_rx_trans *p_rx_trans, UINT8 hcitype);
void btusb_rx_enqueue_voice(struct btusb *p_dev, struct btusb_voice_pkt *p_pkt);
void btusb_rx_dequeued(struct btusb *p_dev, BT_HDR *p_msg);

void btusb_dump_data(const UINT8 *p, int len, const char *p_title);

void btusb_cmd_complete(struct btusb *p_dev, struct btusb_tx_trans *p_tx_trans, struct urb *p_urb);
void btusb_voicetx_complete(struct btusb *p_dev, struct btusb_tx_trans *p_tx_trans, struct urb *p_urb);
void btusb_urb_out_complete(struct urb *p_urb);
void btusb_voicerx_complete(struct urb *p_urb);

/* Voice commands */
int btusb_add_voice_channel(struct btusb *p_dev, unsigned short sco_handle, unsigned char burst);
int btusb_remove_voice_channel(struct btusb *p_dev, unsigned short sco_handle);

/* USB device interface */
int btusb_open(struct inode *inode, struct file *file);
int btusb_release(struct inode *inode, struct file *file);
ssize_t btusb_read(struct file *file, char __user *buffer, size_t count, loff_t *ppos);
ssize_t btusb_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *ppos);
unsigned int btusb_poll(struct file *file, struct poll_table_struct *p_pt);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
long btusb_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#else
int btusb_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
#endif

int btusb_suspend(struct usb_interface *p_interface, pm_message_t message);
int btusb_resume(struct usb_interface *p_interface);

/* Globals */
extern struct usb_driver btusb_driver;
extern bool autopm;

#endif /* BTUSB_H */
