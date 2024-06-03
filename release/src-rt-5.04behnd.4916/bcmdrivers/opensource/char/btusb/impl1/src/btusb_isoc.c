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

/* for kmalloc */
#include <linux/slab.h>
#include "btusb.h"
#include "hcidefs.h"

/*******************************************************************************
 **
 ** Function         btusb_isoc_check_hdr
 **
 ** Description      Check the packet header
 **
 ** Parameters       p_dev: device instance control block
 **
 ** Returns          void
 **
 *******************************************************************************/
static bool btusb_isoc_check_hdr(struct btusb *p_dev)
{
    unsigned char *p_data = p_dev->voice_rx.hdr;
    int idx;
    unsigned short sco_handle;
    unsigned char size;
    struct btusb_voice_pkt *p_pkt;
    BT_HDR *p_hdr;
    struct btusb_voice_channel *p_chan;

    STREAM_TO_UINT16(sco_handle, p_data);
    sco_handle &= 0x0fff;
    STREAM_TO_UINT8(size, p_data);

    for (idx = 0; idx < ARRAY_SIZE(p_dev->voice_rx.channels); idx++)
    {
        p_chan = &p_dev->voice_rx.channels[idx];
        if ((p_chan->used) &&
            (sco_handle == p_chan->handle) &&
            (size <= (2 * p_chan->burst)))
        {
            /* check if there is already a message being consolidated */
            if (unlikely(!p_chan->p_pkt))
            {
                if (!btusb_cq_get(&p_dev->voice_rx_list, &p_pkt))
                {
                    BTUSB_ERR("No buffer available for SCO defragmentation\n");
                    return false;
                }
                p_hdr = &p_pkt->bt_hdr;
                p_hdr->len = BTUSB_VOICE_HEADER_SIZE;
                p_hdr->offset = 0;
                p_hdr->layer_specific = 0;

                p_data = (unsigned char *) (p_hdr + 1);

                /* add sco handle and buffer size */
                UINT16_TO_STREAM(p_data, sco_handle);
                UINT8_TO_STREAM(p_data, BTUSB_SCO_RX_LEN);

                p_chan->p_pkt = p_pkt;
            }
            p_dev->voice_rx.remaining = size;
            p_dev->voice_rx.pp_pkt = &p_chan->p_pkt;

            return true;
        }
    }
    return false;
}

/*******************************************************************************
 **
 ** Function         btusb_isoc_check_pkt
 **
 ** Description      Check if the reconstructed HCI voice packet is large enough
 **
 ** Parameters       p_dev: device instance control block
 **
 ** Returns          void
 **
 *******************************************************************************/
static void btusb_isoc_check_pkt(struct btusb *p_dev)
{
    struct btusb_voice_pkt *p_pkt = *p_dev->voice_rx.pp_pkt;
    BT_HDR *p_hdr = &p_pkt->bt_hdr;

    /* if enough data was received */
    if (unlikely(p_hdr->len == sizeof(((struct btusb_voice_pkt *)0)->data)))
    {
        btusb_rx_enqueue_voice(p_dev, p_pkt);
        /* clear both references to the HCI packet */
        *p_dev->voice_rx.pp_pkt = NULL;
        p_dev->voice_rx.pp_pkt = NULL;
    }
}

/*******************************************************************************
 **
 ** Function         btusb_voicerx_complete
 **
 ** Description      Voice read (iso pipe) completion routine.
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void btusb_voicerx_complete(struct urb *p_urb)
{
    struct btusb_rx_trans *p_rx_trans = p_urb->context;
    struct btusb *p_dev = p_rx_trans->p_dev;
    BT_HDR *p_hdr;
    unsigned int length, packet_length;
    unsigned char *p_packet, *p_frame, *p_data;
    struct usb_iso_packet_descriptor *p_uipd, *p_end;

    if (unlikely(dbgflags & BTUSB_VOICERX_TIME))
    {
        btusb_voice_stats(&(p_dev->stats.voice_max_rx_rdy_delta_time), &(p_dev->stats.voice_min_rx_rdy_delta_time),
            &(p_dev->stats.voice_rx_rdy_delta_time), &(p_dev->stats.voice_last_rx_rdy_ts));
    }

    BTUSB_INFO("enter");

    p_dev->stats.voicerx_complete++;

    if (unlikely(!p_dev->p_main_intf || !p_dev->p_voice_in))
    {
        BTUSB_DBG("intf is down\n");
        return;
    }

    /* entire URB error? */
    if (unlikely(p_urb->status))
    {
        /* this error can happen when unplugging or updating channels */
        p_dev->stats.voicerx_complete_err++;
        return;
    }

    if (unlikely(p_dev->scosniff_active))
    {
        struct btusb_scosniff *bs;

        bs = kmalloc(sizeof(struct btusb_scosniff) +
                (p_urb->number_of_packets * sizeof(p_urb->iso_frame_desc[0])) +
                p_urb->transfer_buffer_length, GFP_ATOMIC);
        if (bs)
        {
            bs->s = p_urb->start_frame;
            bs->n = p_urb->number_of_packets;
            bs->l = p_urb->transfer_buffer_length;
            /* copy the descriptors */
            memcpy(bs->d, p_urb->iso_frame_desc, bs->n * sizeof(p_urb->iso_frame_desc[0]));
            /* then copy the content of the buffer */
            memcpy(&bs->d[bs->n], p_urb->transfer_buffer, bs->l);
            /* protection not required because callback invoked with IRQ disabled */
            list_add_tail(&bs->lh, &p_dev->scosniff_list);
            complete(&p_dev->scosniff_completion);
        }
        else
        {
            BTUSB_ERR("Failed allocating scosniff buffer");
        }
    }

    p_frame = p_urb->transfer_buffer;
    p_end = &p_urb->iso_frame_desc[p_urb->number_of_packets];
    for (p_uipd = p_urb->iso_frame_desc; p_uipd < p_end; p_uipd++)
    {
        if (unlikely(p_uipd->status))
        {
            p_dev->stats.voicerx_bad_frames++;
            /* should we do something if there is expected data? */
            continue;
        }

        p_packet = p_frame + p_uipd->offset;
        packet_length = p_uipd->actual_length;
        p_dev->stats.voicerx_raw_bytes += packet_length;

        /* waiting for data? */
        if (likely(p_dev->voice_rx.remaining))
        {
            fill_data:
            if (likely(p_dev->voice_rx.remaining >= packet_length))
            {
                length = packet_length;
            }
            else
            {
                length = p_dev->voice_rx.remaining;
            }
            p_hdr = &(*p_dev->voice_rx.pp_pkt)->bt_hdr;
            p_data = (void *)(p_hdr + 1) + p_hdr->len;

            if (unlikely((p_hdr->len + length) > sizeof(((struct btusb_voice_pkt *)0)->data)))
            {
                BTUSB_ERR("SCO message too large for buffer\n");
                p_dev->stats.voicerx_bad_size++;
                /* reset the length and pending bytes to end the current packet */
                p_dev->voice_rx.remaining = length =
                        sizeof(((struct btusb_voice_pkt *)0)->data) - p_hdr->len;
            }
            /* append data to the current message */
            memcpy(p_data, p_packet, length);
            p_hdr->len += length;

            /* decrement the number of bytes remaining */
            p_dev->voice_rx.remaining -= length;
            if (likely(p_dev->voice_rx.remaining))
            {
                /* data still needed -> next descriptor */
                continue;
            }
            /* no more pending bytes, check if packet is full */
            btusb_isoc_check_pkt(p_dev);
            packet_length -= length;
            /* speedup peep-hole */
            if (likely(!packet_length))
                continue;
            /* more bytes -> increment pointer */
            p_packet += length;
        }

        /* if there is still data in the packet */
        if (likely(packet_length))
        {
            /* at this point, there is NO SCO packet pending */
            if (likely(packet_length >= (BTUSB_VOICE_HEADER_SIZE - p_dev->voice_rx.hdr_size)))
            {
                length = BTUSB_VOICE_HEADER_SIZE - p_dev->voice_rx.hdr_size;
            }
            else
            {
                length = packet_length;
            }

            /* fill the hdr (in case header is split across descriptors) */
            memcpy(&p_dev->voice_rx.hdr[p_dev->voice_rx.hdr_size], p_packet, length);
            p_dev->voice_rx.hdr_size += length;

            if (likely(p_dev->voice_rx.hdr_size == BTUSB_VOICE_HEADER_SIZE))
            {
                /* reset the pending size */
                p_dev->voice_rx.hdr_size = 0;

                if (likely(btusb_isoc_check_hdr(p_dev)))
                {
                    p_packet += length;
                    packet_length -= length;
                    /* a correct header was found, get the data */
                    goto fill_data;
                }
                p_dev->stats.voicerx_bad_hdr++;
                p_dev->stats.voicerx_skipped_bytes += packet_length;
            }
            else
            {
                p_dev->stats.voicerx_split_hdr++;
            }
        }
    }

    btusb_submit_voice_rx(p_dev, p_urb, GFP_ATOMIC);
}

