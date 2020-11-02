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
 
#ifndef BTUSB_LITE_AVDT_H
#define BTUSB_LITE_AVDT_H

/* AVDT Option */
#define BTUSB_LITE_AVDT_OPT_NO_RTP          0x01    /* No RTP Header */
#define BTUSB_LITE_AVDT_OPT_NO_MPH          0x02    /* No Media Payload Header */

/* Definitions for A2DP packets */
#define BTUSB_LITE_RTP_SIZE                 12  /* RTP Header size */
#define BTUSB_LITE_MEDIA_SIZE               1   /* Media Header size */
#define BTUSB_LITE_SCMS_SIZE                1   /* SCMS Header size */

/* Some A2DP Definitions */
#define AVDT_MEDIA_OCTET1                   0x80 /* First byte of media packet header */

#define AVDT_MARKER_SET                     0x80
#define AVDT_RTP_PAYLOAD_TYPE               0x60 /* First Dynamic Payload type */

/* AVDT Header size */
#define BTUSB_LITE_AVDT_HDR_SIZE            (BTUSB_LITE_RTP_SIZE + BTUSB_LITE_MEDIA_SIZE)

/* A2DP Definitions */
#define A2D_SBC_HDR_NUM_MSK                 0x0F /* A2DP Media Header Frame Mask */


/* channel control block type */
struct btusb_lite_avdt_ccb
{
    BD_ADDR             peer_addr;      /* BD address of peer */
    BOOLEAN             allocated;      /* Whether ccb is allocated */
    UINT16              lcid;           /* local L2CAP channel ID */
    UINT16              peer_mtu;       /* L2CAP mtu of the peer device */
};

/* SCMS information */
struct btusb_lite_avdt_scms
{
    BOOLEAN             enable;         /* Indicates if SCMS is enabled */
    UINT8               header;         /* SCMS Header */
};


/* stream control block type */
struct btusb_lite_avdt_scb
{
    BT_HDR          *p_pkt;         /* packet waiting to be sent */
    struct btusb_lite_avdt_ccb *p_ccb;         /* ccb associated with this scb */
    UINT16          media_seq;      /* media packet sequence number */
    BOOLEAN         allocated;      /* whether scb is allocated or unused */
    BOOLEAN         in_use;         /* whether stream being used by peer */
    BOOLEAN         cong;           /* Whether media transport channel is congested */
    UINT8           handle;
    UINT8           mux_tsid_media; /* TSID for media transport session */
    struct btusb_lite_avdt_scms scms;
};

struct btusb_lite_avdt_cb
{
    struct btusb_lite_avdt_ccb ccb[AVDT_NUM_LINKS];    /* channel control blocks */
    struct btusb_lite_avdt_scb scb[AVDT_NUM_SEPS];     /* stream control blocks */

};
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
struct btusb_lite_avdt_scb *btusb_lite_avdt_scb_by_hdl(struct btusb *p_dev, UINT8 hdl);

/*******************************************************************************
**
** Function         btusb_lite_avdt_init_scb
**
** Description      allocate and initialize SCB/CCB with received sync info
**
** Returns          AVDT_SYNC_SUCCESS/AVDT_SYNC_FAILURE
**
*******************************************************************************/
UINT8 btusb_lite_avdt_init_scb(struct btusb *p_dev, tAVDT_SCB_SYNC_INFO *p_scb_info);

/*******************************************************************************
**
** Function         btusb_lite_avdt_remove_scb
**
** Description      deallocate SCB and CCB
**
** Returns          AVDT_SYNC_SUCCESS/AVDT_SYNC_FAILURE
**
*******************************************************************************/
UINT8 btusb_lite_avdt_remove_scb(struct btusb *p_dev, UINT8 handle, tAVDT_SCB_SYNC_INFO *p_scb_info);

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
        UINT8 m_pt, UINT8 option, UINT32 timestamp);

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
        BOOLEAN enable, UINT8 scms_hdr);

#endif /* BTUSB_LITE_AVDT_H*/

