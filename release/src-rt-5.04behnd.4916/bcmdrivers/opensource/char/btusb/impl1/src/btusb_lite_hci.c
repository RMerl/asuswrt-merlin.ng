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
#define BTUSB_LITE_HCI_NUM_CMD      0x01  /* HCI Num Command */

/* HCI Definitions for the NumberOfCompletePacket Event */
#define BTUSB_LITE_HCI_NOCP_HCI_LEN             7   /* HCI Length of the NumOfCpltPacket Event */
#define BTUSB_A2DP_NOCP_LEN                 5   /* Length of the NumOfCpltPacket Param */

/*
 * Local functions
 */
static UINT8 *btusb_lite_hci_write_acl_header(UINT8 *p_data, UINT16 con_hdl, UINT16 length);
static UINT8 *btusb_lite_hci_write_evt_header(UINT8 *p_data, UINT8 event, UINT8 length);

static int btusb_lite_hci_transport_pause_hndl(struct btusb *p_dev, UINT8 *p_msg);
static int btusb_lite_hci_transport_resume_hndl(struct btusb *p_dev, UINT8 *p_msg);
static void btusb_lite_hci_cmd_cplt_evt_send(struct btusb *p_dev,
        UINT16 opcode, UINT8 *p_param, UINT8 param_len);
static int btusb_lite_hci_nocp_event_hdlr(struct btusb *p_dev, UINT8 *p_data, int length);


/*******************************************************************************
 **
 ** Function         btusb_lite_hci_acl_send
 **
 ** Description      Send an ACL packet to HCI
 **
 ** Returns          Void
 **
 *******************************************************************************/
int btusb_lite_hci_acl_send(struct btusb *p_dev, BT_HDR *p_msg, UINT16 con_hdl)
{
    UINT8 *p_data;

    /* Sanity */
    if (p_msg->offset < BTUSB_LITE_HCI_ACL_HDR_SIZE)
    {
        BTUSB_ERR("offset too small=%d\n", p_msg->offset);
        GKI_freebuf(p_msg); /* Free this ACL buffer */
        return -1;
    }

    /* Decrement offset to add headers */
    p_msg->offset -= BTUSB_LITE_HCI_ACL_HDR_SIZE;

    /* Get address of the HCI Header */
    p_data = (UINT8 *)(p_msg + 1) + p_msg->offset;

    /* Write L2CAP Header (length field is SBC Frames + RTP/A2DP/Media Header) */
    p_data = btusb_lite_hci_write_acl_header(p_data, con_hdl, p_msg->len);

    /* Increment length */
    p_msg->len += BTUSB_LITE_HCI_ACL_HDR_SIZE;

    p_data = (UINT8 *)(p_msg + 1) + p_msg->offset;
    btusb_submit_acl(p_dev, p_data, p_msg->len);

    GKI_freebuf(p_msg); /* Free this ACL buffer */
    return 0;
}

/*******************************************************************************
 **
 ** Function        btusb_lite_hci_cmd_filter
 **
 ** Description     Check if the Sent HCI Command need to be handled/caught (not
 **                 sent to BT controller).
 **
 ** Returns         status: <> 0 if the command must be send to BT controller
 **                         0 if the command is handled
 **
 *******************************************************************************/
int btusb_lite_hci_cmd_filter(struct btusb *p_dev, UINT8 *p_msg)
{
    UINT8 *p;
    UINT8 hci_type;
    UINT16 opcode;
    int rv = 1; /* HCI command not handled by default */

    p = p_msg;

    STREAM_TO_UINT8(hci_type, p);           /* Extract HCI Type */

    if (hci_type != HCIT_TYPE_COMMAND)
    {
        /* This is not an HCI Command */
        return rv;                          /* Send it to BT Controller */
    }

    STREAM_TO_UINT16(opcode, p);            /* Extract HCI Command OpCode */

    switch(opcode)
    {
    case HCI_BRCM_PAUSE_TRANSPORT:
        rv = btusb_lite_hci_transport_pause_hndl(p_dev, p_msg);
        break;

    case HCI_BRCM_TRANSPORT_RESUME:
        /* Call the function in charge of filtering UIPC Over HCI VSC */
        rv = btusb_lite_hci_transport_resume_hndl(p_dev, p_msg);
        break;

    /* Add here other HCI Command OpCodes to filter */
    default:
        break;
        /* Do not filter other HCI Command OpCodes */
    }
    return rv;
}

/*******************************************************************************
 **
 ** Function        btusb_lite_hci_transport_pause_hndl
 **
 ** Description     Handles the HCI Transport Pause VSC.
 **
 ** Returns         status: <> 0 if the command must be send to BT controller
 **                         0 if the command is handled
 **
 *******************************************************************************/
static int btusb_lite_hci_transport_pause_hndl(struct btusb *p_dev, UINT8 *p_msg)
{
    UINT8 param[sizeof(UINT8)];
    UINT8 *p_param = param;

    BTUSB_INFO("HCI_TransportPause VSC caught\n");

    UINT8_TO_STREAM(p_param, HCI_SUCCESS);

    btusb_lite_hci_cmd_cplt_evt_send(p_dev, HCI_BRCM_PAUSE_TRANSPORT, param,
            p_param - param);

    return 0;
}

/*******************************************************************************
 **
 ** Function        btusb_lite_hci_transport_resume_hndl
 **
 ** Description     Handles the HCI Transport Pause VSC.
 **
 ** Returns         status: <> 0 if the command must be send to BT controller
 **                         0 if the command is handled
 **
 *******************************************************************************/
static int btusb_lite_hci_transport_resume_hndl(struct btusb *p_dev, UINT8 *p_msg)
{
    UINT8 param[sizeof(UINT8)];
    UINT8 *p_param = param;

    BTUSB_INFO("HCI_TransportResume VSC caught\n");

    UINT8_TO_STREAM(p_param, HCI_SUCCESS);

    btusb_lite_hci_cmd_cplt_evt_send(p_dev, HCI_BRCM_TRANSPORT_RESUME, param,
            p_param - param);

    return 0;
}

/*******************************************************************************
 **
 ** Function        btusb_lite_hci_event_filter
 **
 ** Description     Filter HCI Events received from BT Controller.
 **
 ** Returns         status: <> 0 if the event must be send to user space (BSA)
 **                         0 if the event is handled
 **
 *******************************************************************************/
int btusb_lite_hci_event_filter(struct btusb *p_dev, UINT8 *p_data, int length)
{
#if 0
    BT_HDR *p_msg;
    UINT8 *p;
    UINT16 size;
#endif

    /* Check if HCI is over IPC */
    if (btusb_lite_is_hci_over_ipc(p_dev) == 0)
    {
        /* If it is not, the event have to be sent through regular HCI */
        return 1;
    }

    /* Check if the Event is a NumberOfCompletePacket Event */
    if (btusb_lite_hci_nocp_event_hdlr(p_dev, p_data, length) == 0)
    {
        return 0;  /* Do not Send this event to user space (we handled it) */
    }

    /* TODO: check if CSB VSE */

    return 1;

#if 0

    /* Add size of both UIPC Length and Event header */
    size = length + sizeof(UINT16) + sizeof(UINT16);

    /* Get a buffer from the pool */
    p_msg = (BT_HDR *)GKI_getbuf(sizeof(BT_HDR) + size);
    if(unlikely(p_msg == NULL))
    {
        BTUSB_ERR("Unable to get GKI buffer\n");
        return 0;
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

    p = (UINT8 *)(p_msg + 1) + p_msg->offset;

    UINT16_TO_STREAM(p, length + sizeof(UINT16));   /* UIPC Length */
    UINT16_TO_STREAM(p, BT_EVT_TO_BTU_HCI_EVT);     /* UIPC Event */
    UINT8_TO_STREAM(p, hci_event);  /* Write back the HCI Event (we already read it) */
    ARRAY_TO_STREAM(p, p_data, length - 1);         /* Copy Event data */

    /* Send message to User Space */
    btusb_lite_ipc_sent_to_user(p_dev, p_msg);

    return 0;   /* Event handled by the Stack Lite. No need to send it to HCI */
#endif
}

/*******************************************************************************
 **
 ** Function        btusb_lite_hci_nocp_event_hdlr
 **
 ** Description     Check if the received HCI Event is A NumberOfdComplete Evt
 **                 sub opcode for a Started BAV stream.
 **
 ** Returns         status: <> 0 if the event must be send to user space (BSA)
 **                         0 if the event is handled
 **
 *******************************************************************************/
static int btusb_lite_hci_nocp_event_hdlr(struct btusb *p_dev, UINT8 *p_data, int length)
{
    UINT8 nb_handle;
    UINT16 con_hdl;
    UINT16 num_cplt_pck;
    UINT8 byte;
    UINT8 *p_save;
    int send_to_user;
    UINT16 num_cplt_pck_caugth;

    /* We are waiting for an Event of, at least, 7 bytes */
    if (length < BTUSB_LITE_HCI_NOCP_HCI_LEN)
    {
        return 1;   /* This is not a NOCP. Send this event to user space */
    }

    /* Extract Event */
    STREAM_TO_UINT8(byte, p_data);

    /* Check if it's a NumberOfCompletePacket Event */
    if (byte != HCI_NUM_COMPL_DATA_PKTS_EVT)
    {
        return 1;   /* This is not a NOCP. Send this event to user space */
    }

    /* Extract Parameter Length */
    STREAM_TO_UINT8(byte, p_data);

    /* Extract Number Of Handle */
    STREAM_TO_UINT8(nb_handle, p_data);

    /* Sanity */
    if (byte != (1 + (2 + 2) * nb_handle))
    {
        BTUSB_ERR("Unexpected Evt Size=%d vs.NumberOfHandle=%d\n", byte, nb_handle);
        return 1;   /* This is not a NOCP. Send this event to user space */
    }

    send_to_user = 0;         /* For the moment, no Complete Packet sent to user */

    /* For every Handle */
    while(nb_handle--)
    {
        /* Extract the Connection Handle */
        STREAM_TO_UINT16(con_hdl, p_data);

        /* Save the current pointer position (to overwrite number of packet) */
        p_save = p_data;

        /* Extract the Number Of Complete Packet */
        STREAM_TO_UINT16(num_cplt_pck, p_data);

        /* Call the L2CAP NumberOfcompletePacket Handler */
        num_cplt_pck_caugth = btusb_lite_l2c_nocp_hdlr(p_dev, con_hdl, num_cplt_pck);

        /* If L2CAP "caught"at least one nocp packet */
        if (num_cplt_pck_caugth)
        {
            /* Overwrite the Number Of Complete Packet */
            UINT16_TO_STREAM(p_save, num_cplt_pck - num_cplt_pck_caugth);

            /* If at least one Number Of Complete Packet remains */
            if (num_cplt_pck - num_cplt_pck_caugth)
            {
                /* Send the event to user space */
                send_to_user = 1;
            }
        }
        else
        {
            /* Don't update the number but send the event to user space */
            send_to_user = 1;
        }
    }

    return send_to_user;
}
/*******************************************************************************
 **
 ** Function        btusb_lite_hci_cmd_cplt_evt_send
 **
 ** Description     Send an HCI VSC Cmd Complete.
 **
 ** Returns         Void
 **
 *******************************************************************************/
static void btusb_lite_hci_cmd_cplt_evt_send(struct btusb *p_dev,
        UINT16 opcode, UINT8 *p_param, UINT8 param_len)
{
    BT_HDR *p_msg;
    UINT16 size = param_len + 5;
    UINT8 *p;

    /* Get a buffer from the pool */
    p_msg = (BT_HDR *)GKI_getbuf(sizeof(BT_HDR) + size);
    if(unlikely(!p_msg))
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
    p_msg->event = HCIT_TYPE_EVENT;
    p_msg->len = size;
    p_msg->layer_specific = BTUSB_LS_GKI_BUFFER;

    p = (UINT8 *)(p_msg + 1) + p_msg->offset;

    p = btusb_lite_hci_write_evt_header(p, HCI_COMMAND_COMPLETE_EVT, size - 2);

    UINT8_TO_STREAM(p, BTUSB_LITE_HCI_NUM_CMD);  /* HCI Num Command */
    UINT16_TO_STREAM(p, opcode);  /* HCI OpCode */

    if (p_param)
    {
        ARRAY_TO_STREAM(p, p_param, param_len)
    }

    /* InQ for user-space to read */
    GKI_enqueue(&p_dev->rx_queue, p_msg);

    /* if the process is polling, indicate RX event */
    wake_up_interruptible(&p_dev->rx_wait_q);
}

/*******************************************************************************
 **
 ** Function         btusb_lite_hci_write_acl_header
 **
 ** Description      Write HCI ACL Header (HCI_Type, Connection Handle, Length)
 **
 ** Returns          New buffer location
 **
 *******************************************************************************/
static UINT8 *btusb_lite_hci_write_acl_header(UINT8 *p_data, UINT16 con_hdl, UINT16 length)
{
    UINT16_TO_STREAM(p_data, con_hdl);              /* Connection Handle */
    UINT16_TO_STREAM(p_data, length);               /* Length */
    return p_data;
}

/*******************************************************************************
 **
 ** Function         btusb_lite_hci_write_evt_header
 **
 ** Description      Write HCI Event Header (HCI_Type, Connection Handle, Length)
 **
 ** Returns          New buffer location
 **
 *******************************************************************************/
static UINT8 *btusb_lite_hci_write_evt_header(UINT8 *p_data, UINT8 event, UINT8 length)
{
    UINT8_TO_STREAM(p_data, event);
    UINT8_TO_STREAM(p_data, length);  /* Param Length */
    return p_data;
}

