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
 
#ifndef BTUSB_LITE_L2C_H
#define BTUSB_LITE_L2C_H

/* L2CAP Header Definition (Length, CID) */
#define BTUSB_LITE_L2CAP_HDR_SIZE (sizeof(UINT16) + sizeof(UINT16))

struct btusb_lite_l2c_ccb
{
    BOOLEAN in_use;
    UINT16 local_cid;
    UINT16 remote_cid;
    UINT16 out_mtu;
    UINT16 handle;
    UINT16 link_xmit_quota;
    UINT8 is_flushable;

    UINT16 tx_pending;
};

struct btusb_lite_l2c_cb
{
    /* Req */
    UINT16 light_xmit_quota;
    UINT16 acl_data_size;
    UINT16 non_flushable_pbf;
    UINT8 multi_av_data_cong_start;
    UINT8 multi_av_data_cong_end;
    UINT8 multi_av_data_cong_discard;
    struct btusb_lite_l2c_ccb ccb[BTM_SYNC_INFO_NUM_STR];

    /* Status */
    UINT16 light_xmit_unacked;
};

/*******************************************************************************
**
** Function         btusb_lite_l2c_add
**
** Description      Synchronize (Add) L2CAP Stream
**
** Returns          Status.
**
*******************************************************************************/
int btusb_lite_l2c_add(struct btusb *p_dev, tL2C_STREAM_INFO *p_l2c_stream);

/*******************************************************************************
**
** Function         btusb_lite_l2c_remove
**
** Description      Synchronize (Remove) L2CAP Stream
**
** Returns          Status.
**
*******************************************************************************/
int btusb_lite_l2c_remove(struct btusb *p_dev, UINT16 local_cid);

/*******************************************************************************
**
** Function         btusb_lite_l2c_send
**
** Description      Send L2CAP packet
**
** Returns          Status
**
*******************************************************************************/
int btusb_lite_l2c_send(struct btusb *p_dev, BT_HDR *p_msg, UINT16 local_cid);

/*******************************************************************************
 **
 ** Function         btusb_lite_l2c_nocp_hdlr
 **
 ** Description      L2CAP NumberOfcompletePacket Handler function
 **
 ** Returns          Number Of Complete Packet caught
 **
 *******************************************************************************/
UINT16 btusb_lite_l2c_nocp_hdlr(struct btusb *p_dev, UINT16 con_hdl, UINT16 num_cplt_pck);

#endif /* BTUSB_LITE_L2C_H*/

