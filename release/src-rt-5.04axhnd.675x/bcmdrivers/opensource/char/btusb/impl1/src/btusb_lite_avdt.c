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
#include "bd.h"

/*
 * Definitions
 */
#define AVDT_MULTIPLEXING   FALSE


/*
 * Local functions
 */
static struct btusb_lite_avdt_scb *btusb_lite_avdt_allocate_scb(struct btusb *p_dev);
static void btusb_lite_avdt_free_scb(struct btusb *p_dev, struct btusb_lite_avdt_scb *p_scb_free);
static struct btusb_lite_avdt_ccb *btusb_lite_avdt_allocate_ccb(struct btusb *p_dev);
static void btusb_lite_avdt_free_ccb(struct btusb *p_dev, struct btusb_lite_avdt_ccb *p_ccb_free);
static UINT8 *btusb_lite_avdt_write_rtp_header(UINT8 *p_data, UINT8 m_pt, UINT16 seq_number, UINT32 timestamp, UINT32 ssrc);



/*******************************************************************************
**
** Function         btusb_lite_avdt_scb_by_hdl
**
** Description      Given an scb handle (or seid), return a pointer to the scb.
**
**
** Returns          Pointer to scb or NULL if index is out of range or scb
**                  is not allocated.
**
*******************************************************************************/
struct btusb_lite_avdt_scb *btusb_lite_avdt_scb_by_hdl(struct btusb *p_dev, UINT8 handle)
{
    struct btusb_lite_avdt_scb *p_scb;
    UINT8 scb;

    p_scb = &p_dev->lite_cb.s.avdt.scb[0];
    for(scb = 0; scb < AVDT_NUM_SEPS; scb++, p_scb++ )
    {
        if((p_scb->allocated) &&
           (p_scb->handle == handle))
            return(p_scb);
    }
    return(NULL);
}

/*******************************************************************************
**
** Function         btusb_lite_avdt_init_scb
**
** Description      allocate and initialize SCB/CCB with received sync info
**
** Returns          AVDT_SYNC_SUCCESS/AVDT_SYNC_FAILURE
**
*******************************************************************************/
UINT8 btusb_lite_avdt_init_scb(struct btusb *p_dev, tAVDT_SCB_SYNC_INFO *p_scb_info)
{
    struct btusb_lite_avdt_scb *p_scb;
    struct btusb_lite_avdt_ccb *p_ccb;

    if((p_scb = btusb_lite_avdt_allocate_scb(p_dev)) == NULL)
    {
        BTUSB_ERR("No SCB for handle %d\n", p_scb_info->handle);
        return AVDT_SYNC_FAILURE;
    }
    else
    {
        if((p_ccb = btusb_lite_avdt_allocate_ccb(p_dev)) == NULL)
        {
            BTUSB_ERR("No CCB for handle %d\n", p_scb_info->handle);
            p_scb->allocated = FALSE;
            return AVDT_SYNC_FAILURE;
        }
        else
        {
            memcpy(p_ccb->peer_addr, p_scb_info->peer_addr, BD_ADDR_LEN);
            p_ccb->lcid     = p_scb_info->local_cid;
            p_ccb->peer_mtu = p_scb_info->peer_mtu;
#if AVDT_MULTIPLEXING == TRUE
            GKI_init_q(&p_scb->frag_q);
#endif
            p_scb->handle           = p_scb_info->handle;
            p_scb->mux_tsid_media   = p_scb_info->mux_tsid_media;
            p_scb->media_seq        = p_scb_info->media_seq;
            p_scb->p_ccb            = p_ccb;
            BTUSB_INFO("Allocated SCB/CCB for handle %d\n", p_scb_info->handle);
        }
    }
    return AVDT_SYNC_SUCCESS;
}

/*******************************************************************************
**
** Function         btusb_lite_avdt_remove_scb
**
** Description      deallocate SCB and CCB
**
** Returns          AVDT_SYNC_SUCCESS/AVDT_SYNC_FAILURE
**
*******************************************************************************/
UINT8 btusb_lite_avdt_remove_scb(struct btusb *p_dev, UINT8 handle, tAVDT_SCB_SYNC_INFO *p_scb_info)
{
    struct btusb_lite_avdt_scb *p_scb;

    if((p_scb = btusb_lite_avdt_scb_by_hdl(p_dev, handle)) == NULL)
    {
        BTUSB_ERR("No SCB for handle %d\n", handle);
        return AVDT_SYNC_FAILURE;
    }
    else
    {
        if (p_scb_info)
        {
            p_scb_info->handle = p_scb->handle;
            p_scb_info->media_seq = p_scb->media_seq;
        }
        /* Free CCB first */
        btusb_lite_avdt_free_ccb(p_dev, p_scb->p_ccb);
        /* Free SCB */
        btusb_lite_avdt_free_scb(p_dev, p_scb);

        return AVDT_SYNC_SUCCESS;
    }
}

/*******************************************************************************
**
** Function         btusb_lite_avdt_allocate_ccb
**
** Description      allocate CCB in lite stack
**
** Returns          pointer of CCB
**
*******************************************************************************/
static struct btusb_lite_avdt_ccb *btusb_lite_avdt_allocate_ccb(struct btusb *p_dev)
{
    struct btusb_lite_avdt_ccb *p_ccb;
    UINT8 ccb;

    p_ccb = &p_dev->lite_cb.s.avdt.ccb[0];
    for(ccb = 0; ccb < AVDT_NUM_LINKS; ccb++, p_ccb++)
    {
        if (!p_ccb->allocated)
        {
            BTUSB_INFO("CCB=%d allocated\n", ccb);
            memset(p_ccb, 0, sizeof(struct btusb_lite_avdt_ccb));
            p_ccb->allocated = TRUE;
            return(p_ccb);
        }
    }
    return(NULL);
}

/*******************************************************************************
**
** Function         btusb_lite_avdt_free_ccb
**
** Description      Free CCB in lite stack
**
** Returns          None
**
*******************************************************************************/
static void btusb_lite_avdt_free_ccb(struct btusb *p_dev, struct btusb_lite_avdt_ccb *p_ccb_free)
{
    struct btusb_lite_avdt_ccb *p_ccb;
    UINT8 ccb;

    p_ccb = &p_dev->lite_cb.s.avdt.ccb[0];
    for(ccb = 0; ccb < AVDT_NUM_LINKS; ccb++, p_ccb++)
    {
        if (p_ccb == p_ccb_free)
        {
            /* Sanity */
            if (!p_ccb_free->allocated)
            {
                BTUSB_ERR("CCB=%d was not allocated\n", ccb);
            }
            BTUSB_INFO("CCB=%d freed\n", ccb);
            p_ccb_free->allocated = FALSE;
            return;
        }
    }
}

/*******************************************************************************
**
** Function         btusb_lite_avdt_allocate_scb
**
** Description      allocate SCB in lite stack
**
** Returns          pointer of SCB
**
*******************************************************************************/
static struct btusb_lite_avdt_scb *btusb_lite_avdt_allocate_scb(struct btusb *p_dev)
{
    struct btusb_lite_avdt_scb *p_scb;
    UINT8 scb;

    p_scb = &p_dev->lite_cb.s.avdt.scb[0];
    for(scb = 0; scb < AVDT_NUM_SEPS; scb++, p_scb++)
    {
        if(!p_scb->allocated)
        {
            BTUSB_INFO("SCB=%d allocated\n", scb);
            memset(p_scb, 0, sizeof(struct btusb_lite_avdt_scb));
            p_scb->allocated = TRUE;
            return(p_scb);
        }
    }
    return(NULL);
}

/*******************************************************************************
**
** Function         btusb_lite_avdt_free_scb
**
** Description      Free SCB in lite stack
**
** Returns          None
**
*******************************************************************************/
static void btusb_lite_avdt_free_scb(struct btusb *p_dev, struct btusb_lite_avdt_scb *p_scb_free)
{
    struct btusb_lite_avdt_scb *p_scb;
    UINT8 scb;

    p_scb = &p_dev->lite_cb.s.avdt.scb[0];
    for(scb = 0; scb < AVDT_NUM_SEPS; scb++, p_scb++)
    {
        if (p_scb == p_scb_free)
        {
            /* Sanity */
            if (!p_scb_free->allocated)
            {
                BTUSB_ERR("SCB=%d was not allocated\n", scb);
            }
            BTUSB_INFO("SCB=%d freed\n", scb);
            p_scb_free->allocated = FALSE;
            return;
        }
    }
}

/*******************************************************************************
**
** Function         btusb_lite_avdt_send
**
** Description      AVDT packet send
**
** Returns          None
**
*******************************************************************************/
void btusb_lite_avdt_send(struct btusb *p_dev, BT_HDR *p_msg, UINT8 avdt_handle,
        UINT8 m_pt, UINT8 option, UINT32 timestamp)
{
    UINT8 *p_data;
    struct btusb_lite_avdt_scb *p_avdt_scb;
    struct btusb_lite_avdt_ccb *p_avdt_ccb;

    if (p_dev == NULL)
    {
        BTUSB_ERR("p_dev is NULL\n");
        if (p_msg)
            GKI_freebuf(p_msg);
        return;
    }

    if (p_msg == NULL)
    {
        BTUSB_ERR("p_msg is NULL\n");
        return;
    }

    /* Find the AVDT SCB with this handle */
    p_avdt_scb = btusb_lite_avdt_scb_by_hdl(p_dev, avdt_handle);
    if (p_avdt_scb == NULL)
    {
        BTUSB_ERR("No AVDT SCB stream found\n");
        GKI_freebuf(p_msg); /* Free this ACL buffer */
        return;
    }

    /* Get the associated AVDT CCB */
    p_avdt_ccb = p_avdt_scb->p_ccb;
    if (p_avdt_ccb == NULL)
    {
        BTUSB_ERR("No AVDT CCB stream found\n");
        GKI_freebuf(p_msg); /* Free this ACL buffer */
        return;
    }

    /* Write the Media Payload Header if needed */
    if ((option & BTUSB_LITE_AVDT_OPT_NO_MPH) == 0)
    {
        if (p_msg->offset < BTUSB_LITE_MEDIA_SIZE)
        {
            BTUSB_ERR("Offset too small=%d for MediaPayloadHeader\n", p_msg->offset);
            GKI_freebuf(p_msg); /* Free this ACL buffer */
            return;
        }
        p_msg->offset -= BTUSB_LITE_MEDIA_SIZE;
        p_msg->len += BTUSB_LITE_MEDIA_SIZE;
        /* Get write address */
        p_data = (UINT8 *)(p_msg + 1) + p_msg->offset;
        /* Write Media Payload Header (Number of SBC Frames) */
        UINT8_TO_BE_STREAM(p_data, p_msg->layer_specific & A2D_SBC_HDR_NUM_MSK);
    }

    /* Write the SCMS content Protection Header if needed */
    if (p_avdt_scb->scms.enable)
    {
        if (p_msg->offset < BTUSB_LITE_SCMS_SIZE)
        {
            BTUSB_ERR("Offset too small=%d for CP Header\n", p_msg->offset);
            GKI_freebuf(p_msg); /* Free this ACL buffer */
            return;
        }
        p_msg->offset -= BTUSB_LITE_SCMS_SIZE;
        p_msg->len += BTUSB_LITE_SCMS_SIZE;
        /* Get write address */
        p_data = (UINT8 *)(p_msg + 1) + p_msg->offset;
        /* Write Media Payload Header (Number of SBC Frames) */
        UINT8_TO_BE_STREAM(p_data, p_avdt_scb->scms.header);
    }

    /* Write the RTP Header if needed */
    if ((option & BTUSB_LITE_AVDT_OPT_NO_RTP) == 0)
    {
        if (p_msg->offset < BTUSB_LITE_RTP_SIZE)
        {
            BTUSB_ERR("Offset too small=%d for RTP Header\n", p_msg->offset);
            GKI_freebuf(p_msg); /* Free this ACL buffer */
            return;
        }
        p_msg->offset -= BTUSB_LITE_RTP_SIZE;
        p_msg->len += BTUSB_LITE_RTP_SIZE;
        /* Get write address */
        p_data = (UINT8 *)(p_msg + 1) + p_msg->offset;
        /* Write RTP Header */
        p_data = btusb_lite_avdt_write_rtp_header(p_data, m_pt, p_avdt_scb->media_seq, timestamp, 0);
    }

    p_avdt_scb->media_seq++;    /* Increment Sequence number */

    /* Request L2CAP to send this packet */
    btusb_lite_l2c_send(p_dev, p_msg, p_avdt_ccb->lcid);
}

/*******************************************************************************
 **
 ** Function         btusb_lite_avdt_write_rtp_header
 **
 ** Description      Write A2DP RTP Header
 **
 ** Returns          New buffer location
 **
 *******************************************************************************/
static UINT8 *btusb_lite_avdt_write_rtp_header(UINT8 *p_data, UINT8 m_pt, UINT16 seq_number,
        UINT32 timestamp, UINT32 ssrc)
{
    /* Write RTP Header */
    UINT8_TO_BE_STREAM(p_data, AVDT_MEDIA_OCTET1);  /* Version, Padding, Ext, CSRC */
    UINT8_TO_BE_STREAM(p_data, m_pt);               /* Marker & Packet Type */
    UINT16_TO_BE_STREAM(p_data, seq_number);        /* Sequence number */
    UINT32_TO_BE_STREAM(p_data, timestamp);         /* TimeStamp */
    UINT32_TO_BE_STREAM(p_data, ssrc);              /* SSRC */
    return p_data;
}

/*******************************************************************************
**
** Function         btusb_lite_avdt_cp_set_scms
**
** Description      Set SCMS Content Protection for a channel
**
** Returns          AVDT_SYNC_SUCCESS/AVDT_SYNC_FAILURE
**
*******************************************************************************/
UINT8 btusb_lite_avdt_cp_set_scms(struct btusb *p_dev, UINT8 avdt_handle,
        BOOLEAN enable, UINT8 scms_hdr)
{
    struct btusb_lite_avdt_scb *p_avdt_scb;

    /* Find the AVDT SCB with this handle */
    p_avdt_scb = btusb_lite_avdt_scb_by_hdl(p_dev, avdt_handle);
    if (p_avdt_scb == NULL)
    {
        BTUSB_ERR("No AVDT SCB stream found\n");
        return AVDT_SYNC_FAILURE;
    }

    BTUSB_INFO("btusb_lite_avdt_cp_set_scms handle=0x%x enable=%d header=0x%x\n",
            avdt_handle, enable, scms_hdr);

    p_avdt_scb->scms.enable = enable;
    p_avdt_scb->scms.header = scms_hdr;

    return AVDT_SYNC_SUCCESS;

}
