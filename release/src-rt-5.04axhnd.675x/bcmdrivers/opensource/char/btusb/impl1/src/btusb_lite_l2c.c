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

/*
 * Definitions
 */

/*
 * Local functions
 */
static UINT8 *btusb_lite_l2c_write_header(UINT8 *p_data, UINT16 length, UINT16 cid);

/*******************************************************************************
**
** Function         btusb_lite_l2c_add
**
** Description      Synchronize (Add) L2CAP Stream
**
** Returns          Status.
**
*******************************************************************************/
int btusb_lite_l2c_add(struct btusb *p_dev, tL2C_STREAM_INFO *p_l2c_stream)
{
    int idx;
    struct btusb_lite_l2c_cb *p_l2c = &p_dev->lite_cb.s.l2c;
    struct btusb_lite_l2c_ccb *p_l2c_ccb;

    /* Check if this L2CAP Stream exists */
    for (idx = 0, p_l2c_ccb = p_l2c->ccb ; idx < ARRAY_SIZE(p_l2c->ccb) ; idx++, p_l2c_ccb++)
    {
        if ((p_l2c_ccb->in_use) &&
            (p_l2c_ccb->handle == p_l2c_stream->handle))
        {
            BTUSB_INFO("CCB=%d was already allocated. Update it.\n", idx);
            break;
        }
    }
    /* If Not found */
    if (idx == BTM_SYNC_INFO_NUM_STR)
    {
        /* Look for a free CCB */
        for (idx = 0, p_l2c_ccb = p_l2c->ccb ; idx < ARRAY_SIZE(p_l2c->ccb) ; idx++, p_l2c_ccb++)
        {
            if (p_l2c_ccb->in_use == FALSE)
            {
                BTUSB_INFO("CCB=%d allocated\n", idx);
                memset(p_l2c_ccb, 0, sizeof(*p_l2c_ccb));
                p_l2c_ccb->in_use = TRUE;
                p_l2c_ccb->local_cid = p_l2c_stream->local_cid;
                p_l2c_ccb->remote_cid = p_l2c_stream->remote_cid;
                p_l2c_ccb->out_mtu = p_l2c_stream->out_mtu;
                p_l2c_ccb->handle = p_l2c_stream->handle;
                p_l2c_ccb->is_flushable = p_l2c_stream->is_flushable;
                break;
            }
        }
    }

    /* If Not found ot not allocated */
    if (idx == BTM_SYNC_INFO_NUM_STR)
    {
        BTUSB_ERR("No Free L2C CCB found (handle=0x%x)\n", p_l2c_stream->handle);
        return -1;
    }

    /* Update Transmit Quota */
    p_l2c_ccb->link_xmit_quota = p_l2c_stream->link_xmit_quota;

    return 0;
}

/*******************************************************************************
**
** Function         btusb_lite_l2c_remove
**
** Description      Synchronize (Remove) L2CAP Stream
**
** Returns          Status.
**
*******************************************************************************/
int btusb_lite_l2c_remove(struct btusb *p_dev, UINT16 local_cid)
{
    int idx;
    struct btusb_lite_l2c_cb *p_l2c = &p_dev->lite_cb.s.l2c;
    struct btusb_lite_l2c_ccb *p_l2c_ccb;

    /* Check if this L2CAP Stream exists */
    for (idx = 0, p_l2c_ccb = p_l2c->ccb ; idx < ARRAY_SIZE(p_l2c->ccb) ; idx++, p_l2c_ccb++)
    {
        if ((p_l2c_ccb->in_use) &&
            (p_l2c_ccb->local_cid == local_cid))
        {
            break;
        }
    }
    /* If Not found */
    if (idx == BTM_SYNC_INFO_NUM_STR)
    {
        BTUSB_ERR("L2C CCB found (lcid=0x%x)\n",local_cid);
        return -1;
    }

    BTUSB_INFO("CCB=%d freed\n", idx);

    /* Reset (Free) the L2CAP Stream */
    memset(p_l2c_ccb, 0, sizeof(*p_l2c_ccb));

    return 0;
}

/*******************************************************************************
**
** Function         btusb_lite_l2c_send
**
** Description      Send L2CAP packet
**
** Returns          Status.
**
*******************************************************************************/
int btusb_lite_l2c_send(struct btusb *p_dev, BT_HDR *p_msg, UINT16 local_cid)
{
    int idx;
    struct btusb_lite_l2c_cb *p_l2c;
    struct btusb_lite_l2c_ccb *p_l2c_ccb;
    UINT8 *p_data;

    /* Look for the first AV stream Started */
    p_l2c = &p_dev->lite_cb.s.l2c;
    for (idx = 0, p_l2c_ccb = p_l2c->ccb ; idx < BTM_SYNC_INFO_NUM_STR ; idx++, p_l2c_ccb++)
    {
        if (p_l2c_ccb->local_cid == local_cid)
        {
            break;
        }
    }
    if (idx == BTM_SYNC_INFO_NUM_STR)
    {
        BTUSB_ERR("No L2C CCB found (lcid=0x%x)\n", local_cid);
        GKI_freebuf(p_msg); /* Free this ACL buffer */
        return -1;
    }

    /* Check if the Tx Quota has been reached for this channel */
    if (p_l2c_ccb->tx_pending >= p_l2c_ccb->link_xmit_quota)
    {
        BTUSB_ERR("Tx Quota reached for L2CAP channel (lcid=0x%x). Drop buffer\n", local_cid);
        GKI_freebuf(p_msg); /* Free this ACL buffer */
        return -1;
    }

    /* Sanity */
    if (p_msg->offset < BTUSB_LITE_L2CAP_HDR_SIZE)
    {
        BTUSB_ERR("offset too small=%d\n", p_msg->offset);
        GKI_freebuf(p_msg); /* Free this ACL buffer */
        return-1;
    }

    /* Decrement offset to add headers */
    p_msg->offset -= BTUSB_LITE_L2CAP_HDR_SIZE;

    /* Get address of the HCI Header */
    p_data = (UINT8 *)(p_msg + 1) + p_msg->offset;

    /* Write L2CAP Header (length field is SBC Frames + RTP/A2DP/Media Header) */
    p_data = btusb_lite_l2c_write_header(p_data, p_msg->len, p_l2c_ccb->remote_cid);

    /* Increment length */
    p_msg->len += BTUSB_LITE_L2CAP_HDR_SIZE;


    GKI_disable();      /* tx_pending field can be updated by another context */
    p_l2c_ccb->tx_pending++;            /* One more A2DP L2CAP packet pending */
    BTUSB_DBG("L2C Tx Pending=%d\n", p_l2c_ccb->tx_pending);
    GKI_enable();

    if (btusb_lite_hci_acl_send(p_dev, p_msg, p_l2c_ccb->handle) < 0)
    {
        GKI_disable();      /* tx_pending field can be updated by another context */
        p_l2c_ccb->tx_pending--;        /* Remove A2DP L2CAP packet pending */
        GKI_enable();
        return -1;
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function         btusb_lite_l2c_write_header
 **
 ** Description      Write L2CAP ACL Header (Length, Channel Id)
 **
 ** Returns          New buffer location
 **
 *******************************************************************************/
static UINT8 *btusb_lite_l2c_write_header(UINT8 *p_data, UINT16 length, UINT16 cid)
{
    UINT16_TO_STREAM(p_data, length);               /* Length */
    UINT16_TO_STREAM(p_data, cid);                  /* Channel Id */
    return p_data;
}

/*******************************************************************************
 **
 ** Function         btusb_lite_l2c_nocp_hdlr
 **
 ** Description      L2CAP NumberOfcompletePacket Handler function
 **
 ** Returns          Number Of Complete Packet caught
 **
 *******************************************************************************/
UINT16 btusb_lite_l2c_nocp_hdlr(struct btusb *p_dev, UINT16 con_hdl, UINT16 num_cplt_pck)
{
    struct btusb_lite_l2c_cb *p_l2c;
    struct btusb_lite_l2c_ccb *p_l2c_ccb;
    int i;
    UINT16 num_cplt_pck_caugth;

    /* Look for the L2CAP channel matching the Connection Handle */
    p_l2c = &p_dev->lite_cb.s.l2c;
    for (i = 0, p_l2c_ccb = p_l2c->ccb ; i < BTM_SYNC_INFO_NUM_STR ; i++, p_l2c_ccb++)
    {
        if (p_l2c_ccb->handle == con_hdl)
        {
            break;
        }
    }
    /* If L2CAP channel not found/known */
    if (i == BTM_SYNC_INFO_NUM_STR)
    {
        return 0;
    }

    /* If no Tx Pending */
    if (p_l2c_ccb->tx_pending == 0)
    {
        return 0;
    }

    GKI_disable();      /* tx_pending field can be updated by another context */

    /* Take the min between the number of pending packet and the number of acked packet */
    num_cplt_pck_caugth = min(p_l2c_ccb->tx_pending, num_cplt_pck);

    /* Update the number of pending packet */
    p_l2c_ccb->tx_pending-= num_cplt_pck_caugth;

    BTUSB_DBG("L2C NOCP Tx Pending=%d\n", p_l2c_ccb->tx_pending);

    GKI_enable();

    return num_cplt_pck_caugth;
}
