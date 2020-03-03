/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 ******************************************************************************/
#define _USB_OPS_LINUX_C_

#include <drv_types.h>
#include <recv_osdep.h>
#include <rtw_sreset.h>

static void interrupt_handler_8188eu(struct adapter *adapt, u16 pkt_len, u8 *pbuf)
{
	struct hal_data_8188e	*haldata = GET_HAL_DATA(adapt);

	if (pkt_len != INTERRUPT_MSG_FORMAT_LEN) {
		DBG_88E("%s Invalid interrupt content length (%d)!\n", __func__, pkt_len);
		return;
	}

	/*  HISR */
	memcpy(&(haldata->IntArray[0]), &(pbuf[USB_INTR_CONTENT_HISR_OFFSET]), 4);
	memcpy(&(haldata->IntArray[1]), &(pbuf[USB_INTR_CONTENT_HISRE_OFFSET]), 4);

	/*  C2H Event */
	if (pbuf[0] != 0)
		memcpy(&(haldata->C2hArray[0]), &(pbuf[USB_INTR_CONTENT_C2H_OFFSET]), 16);
}

static int recvbuf2recvframe(struct adapter *adapt, struct sk_buff *pskb)
{
	u8	*pbuf;
	u8	shift_sz = 0;
	u16	pkt_cnt;
	u32	pkt_offset, skb_len, alloc_sz;
	s32	transfer_len;
	struct recv_stat	*prxstat;
	struct phy_stat	*pphy_status = NULL;
	struct sk_buff *pkt_copy = NULL;
	struct recv_frame	*precvframe = NULL;
	struct rx_pkt_attrib	*pattrib = NULL;
	struct hal_data_8188e	*haldata = GET_HAL_DATA(adapt);
	struct recv_priv	*precvpriv = &adapt->recvpriv;
	struct __queue *pfree_recv_queue = &precvpriv->free_recv_queue;

	transfer_len = (s32)pskb->len;
	pbuf = pskb->data;

	prxstat = (struct recv_stat *)pbuf;
	pkt_cnt = (le32_to_cpu(prxstat->rxdw2) >> 16) & 0xff;

	do {
		RT_TRACE(_module_rtl871x_recv_c_, _drv_info_,
			 ("recvbuf2recvframe: rxdesc=offsset 0:0x%08x, 4:0x%08x, 8:0x%08x, C:0x%08x\n",
			  prxstat->rxdw0, prxstat->rxdw1, prxstat->rxdw2, prxstat->rxdw4));

		prxstat = (struct recv_stat *)pbuf;

		precvframe = rtw_alloc_recvframe(pfree_recv_queue);
		if (precvframe == NULL) {
			RT_TRACE(_module_rtl871x_recv_c_, _drv_err_, ("recvbuf2recvframe: precvframe==NULL\n"));
			DBG_88E("%s()-%d: rtw_alloc_recvframe() failed! RX Drop!\n", __func__, __LINE__);
			goto _exit_recvbuf2recvframe;
		}

		INIT_LIST_HEAD(&precvframe->list);
		precvframe->len = 0;

		update_recvframe_attrib_88e(precvframe, prxstat);

		pattrib = &precvframe->attrib;

		if ((pattrib->crc_err) || (pattrib->icv_err)) {
			DBG_88E("%s: RX Warning! crc_err=%d icv_err=%d, skip!\n", __func__, pattrib->crc_err, pattrib->icv_err);

			rtw_free_recvframe(precvframe, pfree_recv_queue);
			goto _exit_recvbuf2recvframe;
		}

		if ((pattrib->physt) && (pattrib->pkt_rpt_type == NORMAL_RX))
			pphy_status = (struct phy_stat *)(pbuf + RXDESC_OFFSET);

		pkt_offset = RXDESC_SIZE + pattrib->drvinfo_sz + pattrib->shift_sz + pattrib->pkt_len;

		if ((pattrib->pkt_len <= 0) || (pkt_offset > transfer_len)) {
			RT_TRACE(_module_rtl871x_recv_c_, _drv_info_, ("recvbuf2recvframe: pkt_len<=0\n"));
			DBG_88E("%s()-%d: RX Warning!,pkt_len<=0 or pkt_offset> transfoer_len\n", __func__, __LINE__);
			rtw_free_recvframe(precvframe, pfree_recv_queue);
			goto _exit_recvbuf2recvframe;
		}

		/*	Modified by Albert 20101213 */
		/*	For 8 bytes IP header alignment. */
		if (pattrib->qos)	/*	Qos data, wireless lan header length is 26 */
			shift_sz = 6;
		else
			shift_sz = 0;

		skb_len = pattrib->pkt_len;

		/*  for first fragment packet, driver need allocate 1536+drvinfo_sz+RXDESC_SIZE to defrag packet. */
		/*  modify alloc_sz for recvive crc error packet by thomas 2011-06-02 */
		if ((pattrib->mfrag == 1) && (pattrib->frag_num == 0)) {
			if (skb_len <= 1650)
				alloc_sz = 1664;
			else
				alloc_sz = skb_len + 14;
		} else {
			alloc_sz = skb_len;
			/*	6 is for IP header 8 bytes alignment in QoS packet case. */
			/*	8 is for skb->data 4 bytes alignment. */
			alloc_sz += 14;
		}

		pkt_copy = netdev_alloc_skb(adapt->pnetdev, alloc_sz);
		if (pkt_copy) {
			pkt_copy->dev = adapt->pnetdev;
			precvframe->pkt = pkt_copy;
			precvframe->rx_head = pkt_copy->data;
			precvframe->rx_end = pkt_copy->data + alloc_sz;
			skb_reserve(pkt_copy, 8 - ((size_t)(pkt_copy->data) & 7));/* force pkt_copy->data at 8-byte alignment address */
			skb_reserve(pkt_copy, shift_sz);/* force ip_hdr at 8-byte alignment address according to shift_sz. */
			memcpy(pkt_copy->data, (pbuf + pattrib->drvinfo_sz + RXDESC_SIZE), skb_len);
			precvframe->rx_tail = pkt_copy->data;
			precvframe->rx_data = pkt_copy->data;
		} else {
			if ((pattrib->mfrag == 1) && (pattrib->frag_num == 0)) {
				DBG_88E("recvbuf2recvframe: alloc_skb fail , drop frag frame\n");
				rtw_free_recvframe(precvframe, pfree_recv_queue);
				goto _exit_recvbuf2recvframe;
			}
			precvframe->pkt = skb_clone(pskb, GFP_ATOMIC);
			if (precvframe->pkt) {
				precvframe->rx_tail = pbuf + pattrib->drvinfo_sz + RXDESC_SIZE;
				precvframe->rx_head = precvframe->rx_tail;
				precvframe->rx_data = precvframe->rx_tail;
				precvframe->rx_end =  pbuf + pattrib->drvinfo_sz + RXDESC_SIZE + alloc_sz;
			} else {
				DBG_88E("recvbuf2recvframe: skb_clone fail\n");
				rtw_free_recvframe(precvframe, pfree_recv_queue);
				goto _exit_recvbuf2recvframe;
			}
		}

		recvframe_put(precvframe, skb_len);

		switch (haldata->UsbRxAggMode) {
		case USB_RX_AGG_DMA:
		case USB_RX_AGG_MIX:
			pkt_offset = (u16)round_up(pkt_offset, 128);
			break;
		case USB_RX_AGG_USB:
			pkt_offset = (u16)round_up(pkt_offset, 4);
			break;
		case USB_RX_AGG_DISABLE:
		default:
			break;
		}
		if (pattrib->pkt_rpt_type == NORMAL_RX) { /* Normal rx packet */
			if (pattrib->physt)
				update_recvframe_phyinfo_88e(precvframe, (struct phy_stat *)pphy_status);
			if (rtw_recv_entry(precvframe) != _SUCCESS) {
				RT_TRACE(_module_rtl871x_recv_c_, _drv_err_,
					("recvbuf2recvframe: rtw_recv_entry(precvframe) != _SUCCESS\n"));
			}
		} else {
			/* enqueue recvframe to txrtp queue */
			if (pattrib->pkt_rpt_type == TX_REPORT1) {
				/* CCX-TXRPT ack for xmit mgmt frames. */
				handle_txrpt_ccx_88e(adapt, precvframe->rx_data);
			} else if (pattrib->pkt_rpt_type == TX_REPORT2) {
				ODM_RA_TxRPT2Handle_8188E(
							&haldata->odmpriv,
							precvframe->rx_data,
							pattrib->pkt_len,
							pattrib->MacIDValidEntry[0],
							pattrib->MacIDValidEntry[1]
							);
			} else if (pattrib->pkt_rpt_type == HIS_REPORT) {
				interrupt_handler_8188eu(adapt, pattrib->pkt_len, precvframe->rx_data);
			}
			rtw_free_recvframe(precvframe, pfree_recv_queue);
		}
		pkt_cnt--;
		transfer_len -= pkt_offset;
		pbuf += pkt_offset;
		precvframe = NULL;
		pkt_copy = NULL;

		if (transfer_len > 0 && pkt_cnt == 0)
			pkt_cnt = (le32_to_cpu(prxstat->rxdw2)>>16) & 0xff;

	} while ((transfer_len > 0) && (pkt_cnt > 0));

_exit_recvbuf2recvframe:

	return _SUCCESS;
}

unsigned int ffaddr2pipehdl(struct dvobj_priv *pdvobj, u32 addr)
{
	unsigned int pipe = 0, ep_num = 0;
	struct usb_device *pusbd = pdvobj->pusbdev;

	if (addr == RECV_BULK_IN_ADDR) {
		pipe = usb_rcvbulkpipe(pusbd, pdvobj->RtInPipe[0]);
	} else if (addr == RECV_INT_IN_ADDR) {
		pipe = usb_rcvbulkpipe(pusbd, pdvobj->RtInPipe[1]);
	} else if (addr < HW_QUEUE_ENTRY) {
		ep_num = pdvobj->Queue2Pipe[addr];
		pipe = usb_sndbulkpipe(pusbd, ep_num);
	}

	return pipe;
}

static int usbctrl_vendorreq(struct adapter *adapt, u8 request, u16 value, u16 index, void *pdata, u16 len, u8 requesttype)
{
	struct dvobj_priv  *dvobjpriv = adapter_to_dvobj(adapt);
	struct usb_device *udev = dvobjpriv->pusbdev;
	unsigned int pipe;
	int status = 0;
	u8 reqtype;
	u8 *pIo_buf;
	int vendorreq_times = 0;

	if ((adapt->bSurpriseRemoved) || (adapt->pwrctrlpriv.pnp_bstop_trx)) {
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("usbctrl_vendorreq:(adapt->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)!!!\n"));
		status = -EPERM;
		goto exit;
	}

	if (len > MAX_VENDOR_REQ_CMD_SIZE) {
		DBG_88E("[%s] Buffer len error ,vendor request failed\n", __func__);
		status = -EINVAL;
		goto exit;
	}

	_enter_critical_mutex(&dvobjpriv->usb_vendor_req_mutex, NULL);

	/*  Acquire IO memory for vendorreq */
	pIo_buf = dvobjpriv->usb_vendor_req_buf;

	if (pIo_buf == NULL) {
		DBG_88E("[%s] pIo_buf == NULL\n", __func__);
		status = -ENOMEM;
		goto release_mutex;
	}

	while (++vendorreq_times <= MAX_USBCTRL_VENDORREQ_TIMES) {
		memset(pIo_buf, 0, len);

		if (requesttype == 0x01) {
			pipe = usb_rcvctrlpipe(udev, 0);/* read_in */
			reqtype =  REALTEK_USB_VENQT_READ;
		} else {
			pipe = usb_sndctrlpipe(udev, 0);/* write_out */
			reqtype =  REALTEK_USB_VENQT_WRITE;
			memcpy(pIo_buf, pdata, len);
		}

		status = usb_control_msg(udev, pipe, request, reqtype, value, index, pIo_buf, len, RTW_USB_CONTROL_MSG_TIMEOUT);

		if (status == len) {   /*  Success this control transfer. */
			if (requesttype == 0x01)
				memcpy(pdata, pIo_buf,  len);
		} else { /*  error cases */
			DBG_88E("reg 0x%x, usb %s %u fail, status:%d value=0x%x, vendorreq_times:%d\n",
				value, (requesttype == 0x01) ? "read" : "write",
				len, status, *(u32 *)pdata, vendorreq_times);

			if (status < 0) {
				if (status == (-ESHUTDOWN) || status == -ENODEV) {
					adapt->bSurpriseRemoved = true;
				} else {
					struct hal_data_8188e	*haldata = GET_HAL_DATA(adapt);
					haldata->srestpriv.Wifi_Error_Status = USB_VEN_REQ_CMD_FAIL;
				}
			} else { /*  status != len && status >= 0 */
				if (status > 0) {
					if (requesttype == 0x01) {
						/*  For Control read transfer, we have to copy the read data from pIo_buf to pdata. */
						memcpy(pdata, pIo_buf,  len);
					}
				}
			}

		}

		/*  firmware download is checksumed, don't retry */
		if ((value >= FW_8188E_START_ADDRESS && value <= FW_8188E_END_ADDRESS) || status == len)
			break;
	}
release_mutex:
	mutex_unlock(&dvobjpriv->usb_vendor_req_mutex);
exit:
	return status;
}

u8 usb_read8(struct adapter *adapter, u32 addr)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u8 data = 0;


	request = 0x05;
	requesttype = 0x01;/* read_in */
	index = 0;/* n/a */

	wvalue = (u16)(addr&0x0000ffff);
	len = 1;

	usbctrl_vendorreq(adapter, request, wvalue, index, &data, len, requesttype);


	return data;

}

u16 usb_read16(struct adapter *adapter, u32 addr)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	__le32 data;

	request = 0x05;
	requesttype = 0x01;/* read_in */
	index = 0;/* n/a */
	wvalue = (u16)(addr&0x0000ffff);
	len = 2;
	usbctrl_vendorreq(adapter, request, wvalue, index, &data, len, requesttype);

	return (u16)(le32_to_cpu(data)&0xffff);
}

u32 usb_read32(struct adapter *adapter, u32 addr)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	__le32 data;


	request = 0x05;
	requesttype = 0x01;/* read_in */
	index = 0;/* n/a */

	wvalue = (u16)(addr&0x0000ffff);
	len = 4;

	usbctrl_vendorreq(adapter, request, wvalue, index, &data, len, requesttype);


	return le32_to_cpu(data);
}

static void usb_read_port_complete(struct urb *purb, struct pt_regs *regs)
{
	struct recv_buf	*precvbuf = (struct recv_buf *)purb->context;
	struct adapter	*adapt = (struct adapter *)precvbuf->adapter;
	struct recv_priv *precvpriv = &adapt->recvpriv;

	RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("usb_read_port_complete!!!\n"));

	precvpriv->rx_pending_cnt--;

	if (adapt->bSurpriseRemoved || adapt->bDriverStopped || adapt->bReadPortCancel) {
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_,
			 ("usb_read_port_complete:bDriverStopped(%d) OR bSurpriseRemoved(%d)\n",
			 adapt->bDriverStopped, adapt->bSurpriseRemoved));

		precvbuf->reuse = true;
		DBG_88E("%s() RX Warning! bDriverStopped(%d) OR bSurpriseRemoved(%d) bReadPortCancel(%d)\n",
			__func__, adapt->bDriverStopped,
			adapt->bSurpriseRemoved, adapt->bReadPortCancel);
		return;
	}

	if (purb->status == 0) { /* SUCCESS */
		if ((purb->actual_length > MAX_RECVBUF_SZ) || (purb->actual_length < RXDESC_SIZE)) {
			RT_TRACE(_module_hci_ops_os_c_, _drv_err_,
				 ("usb_read_port_complete: (purb->actual_length > MAX_RECVBUF_SZ) || (purb->actual_length < RXDESC_SIZE)\n"));
			precvbuf->reuse = true;
			usb_read_port(adapt, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
			DBG_88E("%s()-%d: RX Warning!\n", __func__, __LINE__);
		} else {
			skb_put(precvbuf->pskb, purb->actual_length);
			skb_queue_tail(&precvpriv->rx_skb_queue, precvbuf->pskb);

			if (skb_queue_len(&precvpriv->rx_skb_queue) <= 1)
				tasklet_schedule(&precvpriv->recv_tasklet);

			precvbuf->pskb = NULL;
			precvbuf->reuse = false;
			usb_read_port(adapt, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
		}
	} else {
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("usb_read_port_complete : purb->status(%d) != 0\n", purb->status));

		DBG_88E("###=> usb_read_port_complete => urb status(%d)\n", purb->status);
		skb_put(precvbuf->pskb, purb->actual_length);
		precvbuf->pskb = NULL;

		switch (purb->status) {
		case -EINVAL:
		case -EPIPE:
		case -ENODEV:
		case -ESHUTDOWN:
			adapt->bSurpriseRemoved = true;
		case -ENOENT:
			adapt->bDriverStopped = true;
			RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("usb_read_port_complete:bDriverStopped=true\n"));
			break;
		case -EPROTO:
		case -EOVERFLOW:
			{
				struct hal_data_8188e	*haldata = GET_HAL_DATA(adapt);
				haldata->srestpriv.Wifi_Error_Status = USB_READ_PORT_FAIL;
			}
			precvbuf->reuse = true;
			usb_read_port(adapt, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
			break;
		case -EINPROGRESS:
			DBG_88E("ERROR: URB IS IN PROGRESS!\n");
			break;
		default:
			break;
		}
	}
}

u32 usb_read_port(struct adapter *adapter, u32 addr, u32 cnt, u8 *rmem)
{
	struct urb *purb = NULL;
	struct recv_buf	*precvbuf = (struct recv_buf *)rmem;
	struct dvobj_priv	*pdvobj = adapter_to_dvobj(adapter);
	struct recv_priv	*precvpriv = &adapter->recvpriv;
	struct usb_device	*pusbd = pdvobj->pusbdev;
	int err;
	unsigned int pipe;
	size_t tmpaddr = 0;
	size_t alignment = 0;
	u32 ret = _SUCCESS;


	if (adapter->bDriverStopped || adapter->bSurpriseRemoved ||
	    adapter->pwrctrlpriv.pnp_bstop_trx) {
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_,
			 ("usb_read_port:(adapt->bDriverStopped ||adapt->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)!!!\n"));
		return _FAIL;
	}

	if (!precvbuf) {
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_,
			 ("usb_read_port:precvbuf==NULL\n"));
		return _FAIL;
	}

	if ((!precvbuf->reuse) || (precvbuf->pskb == NULL)) {
		precvbuf->pskb = skb_dequeue(&precvpriv->free_recv_skb_queue);
		if (NULL != precvbuf->pskb)
			precvbuf->reuse = true;
	}

	/* re-assign for linux based on skb */
	if ((!precvbuf->reuse) || (precvbuf->pskb == NULL)) {
		precvbuf->pskb = netdev_alloc_skb(adapter->pnetdev, MAX_RECVBUF_SZ + RECVBUFF_ALIGN_SZ);
		if (precvbuf->pskb == NULL) {
			RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("init_recvbuf(): alloc_skb fail!\n"));
			DBG_88E("#### usb_read_port() alloc_skb fail!#####\n");
			return _FAIL;
		}

		tmpaddr = (size_t)precvbuf->pskb->data;
		alignment = tmpaddr & (RECVBUFF_ALIGN_SZ-1);
		skb_reserve(precvbuf->pskb, (RECVBUFF_ALIGN_SZ - alignment));
	} else { /* reuse skb */
		precvbuf->reuse = false;
	}

	precvpriv->rx_pending_cnt++;

	purb = precvbuf->purb;

	/* translate DMA FIFO addr to pipehandle */
	pipe = ffaddr2pipehdl(pdvobj, addr);

	usb_fill_bulk_urb(purb, pusbd, pipe,
			  precvbuf->pskb->data,
			  MAX_RECVBUF_SZ,
			  usb_read_port_complete,
			  precvbuf);/* context is precvbuf */

	err = usb_submit_urb(purb, GFP_ATOMIC);
	if ((err) && (err != (-EPERM))) {
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_,
			 ("cannot submit rx in-token(err=0x%.8x), URB_STATUS =0x%.8x",
			 err, purb->status));
		DBG_88E("cannot submit rx in-token(err = 0x%08x),urb_status = %d\n",
			err, purb->status);
		ret = _FAIL;
	}

	return ret;
}

void usb_read_port_cancel(struct adapter *padapter)
{
	int i;
	struct recv_buf *precvbuf;

	precvbuf = (struct recv_buf *)padapter->recvpriv.precv_buf;

	DBG_88E("%s\n", __func__);

	padapter->bReadPortCancel = true;

	for (i = 0; i < NR_RECVBUFF; i++) {
		precvbuf->reuse = true;
		if (precvbuf->purb)
			usb_kill_urb(precvbuf->purb);
		precvbuf++;
	}
}

int usb_write8(struct adapter *adapter, u32 addr, u8 val)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u8 data;

	request = 0x05;
	requesttype = 0x00;/* write_out */
	index = 0;/* n/a */
	wvalue = (u16)(addr&0x0000ffff);
	len = 1;
	data = val;
	return usbctrl_vendorreq(adapter, request, wvalue,
				 index, &data, len, requesttype);
}

int usb_write16(struct adapter *adapter, u32 addr, u16 val)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	__le32 data;


	request = 0x05;
	requesttype = 0x00;/* write_out */
	index = 0;/* n/a */

	wvalue = (u16)(addr&0x0000ffff);
	len = 2;

	data = cpu_to_le32(val & 0x0000ffff);

	return usbctrl_vendorreq(adapter, request, wvalue,
				 index, &data, len, requesttype);


}

int usb_write32(struct adapter *adapter, u32 addr, u32 val)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	__le32 data;


	request = 0x05;
	requesttype = 0x00;/* write_out */
	index = 0;/* n/a */

	wvalue = (u16)(addr&0x0000ffff);
	len = 4;
	data = cpu_to_le32(val);

	return usbctrl_vendorreq(adapter, request, wvalue,
				 index, &data, len, requesttype);


}

static void usb_write_port_complete(struct urb *purb, struct pt_regs *regs)
{
	struct xmit_buf *pxmitbuf = (struct xmit_buf *)purb->context;
	struct adapter	*padapter = pxmitbuf->padapter;
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;

	switch (pxmitbuf->flags) {
	case VO_QUEUE_INX:
		pxmitpriv->voq_cnt--;
		break;
	case VI_QUEUE_INX:
		pxmitpriv->viq_cnt--;
		break;
	case BE_QUEUE_INX:
		pxmitpriv->beq_cnt--;
		break;
	case BK_QUEUE_INX:
		pxmitpriv->bkq_cnt--;
		break;
	case HIGH_QUEUE_INX:
#ifdef CONFIG_88EU_AP_MODE
		rtw_chk_hi_queue_cmd(padapter);
#endif
		break;
	default:
		break;
	}

	if (padapter->bSurpriseRemoved || padapter->bDriverStopped ||
	    padapter->bWritePortCancel) {
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_,
			 ("usb_write_port_complete:bDriverStopped(%d) OR bSurpriseRemoved(%d)",
			 padapter->bDriverStopped, padapter->bSurpriseRemoved));
		DBG_88E("%s(): TX Warning! bDriverStopped(%d) OR bSurpriseRemoved(%d) bWritePortCancel(%d) pxmitbuf->ext_tag(%x)\n",
			__func__, padapter->bDriverStopped,
			padapter->bSurpriseRemoved, padapter->bReadPortCancel,
			pxmitbuf->ext_tag);

		goto check_completion;
	}

	if (purb->status) {
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("usb_write_port_complete : purb->status(%d) != 0\n", purb->status));
		DBG_88E("###=> urb_write_port_complete status(%d)\n", purb->status);
		if ((purb->status == -EPIPE) || (purb->status == -EPROTO)) {
			sreset_set_wifi_error_status(padapter, USB_WRITE_PORT_FAIL);
		} else if (purb->status == -EINPROGRESS) {
			RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("usb_write_port_complete: EINPROGESS\n"));
			goto check_completion;
		} else if (purb->status == -ENOENT) {
			DBG_88E("%s: -ENOENT\n", __func__);
			goto check_completion;
		} else if (purb->status == -ECONNRESET) {
			DBG_88E("%s: -ECONNRESET\n", __func__);
			goto check_completion;
		} else if (purb->status == -ESHUTDOWN) {
			RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("usb_write_port_complete: ESHUTDOWN\n"));
			padapter->bDriverStopped = true;
			RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("usb_write_port_complete:bDriverStopped = true\n"));
			goto check_completion;
		} else {
			padapter->bSurpriseRemoved = true;
			DBG_88E("bSurpriseRemoved = true\n");
			RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("usb_write_port_complete:bSurpriseRemoved = true\n"));

			goto check_completion;
		}
	}

check_completion:
	rtw_sctx_done_err(&pxmitbuf->sctx,
			  purb->status ? RTW_SCTX_DONE_WRITE_PORT_ERR :
			  RTW_SCTX_DONE_SUCCESS);

	rtw_free_xmitbuf(pxmitpriv, pxmitbuf);

	tasklet_hi_schedule(&pxmitpriv->xmit_tasklet);
}

u32 usb_write_port(struct adapter *padapter, u32 addr, u32 cnt, u8 *wmem)
{
	unsigned long irqL;
	unsigned int pipe;
	int status;
	u32 ret = _FAIL;
	struct urb *purb = NULL;
	struct dvobj_priv	*pdvobj = adapter_to_dvobj(padapter);
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;
	struct xmit_buf *pxmitbuf = (struct xmit_buf *)wmem;
	struct xmit_frame *pxmitframe = (struct xmit_frame *)pxmitbuf->priv_data;
	struct usb_device *pusbd = pdvobj->pusbdev;


	RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("+usb_write_port\n"));

	if ((padapter->bDriverStopped) || (padapter->bSurpriseRemoved) ||
	    (padapter->pwrctrlpriv.pnp_bstop_trx)) {
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_,
			 ("usb_write_port:( padapter->bDriverStopped ||padapter->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)!!!\n"));
		rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_TX_DENY);
		goto exit;
	}

	spin_lock_irqsave(&pxmitpriv->lock, irqL);

	switch (addr) {
	case VO_QUEUE_INX:
		pxmitpriv->voq_cnt++;
		pxmitbuf->flags = VO_QUEUE_INX;
		break;
	case VI_QUEUE_INX:
		pxmitpriv->viq_cnt++;
		pxmitbuf->flags = VI_QUEUE_INX;
		break;
	case BE_QUEUE_INX:
		pxmitpriv->beq_cnt++;
		pxmitbuf->flags = BE_QUEUE_INX;
		break;
	case BK_QUEUE_INX:
		pxmitpriv->bkq_cnt++;
		pxmitbuf->flags = BK_QUEUE_INX;
		break;
	case HIGH_QUEUE_INX:
		pxmitbuf->flags = HIGH_QUEUE_INX;
		break;
	default:
		pxmitbuf->flags = MGT_QUEUE_INX;
		break;
	}

	spin_unlock_irqrestore(&pxmitpriv->lock, irqL);

	purb	= pxmitbuf->pxmit_urb[0];

	/* translate DMA FIFO addr to pipehandle */
	pipe = ffaddr2pipehdl(pdvobj, addr);

	usb_fill_bulk_urb(purb, pusbd, pipe,
			  pxmitframe->buf_addr, /*  pxmitbuf->pbuf */
			  cnt,
			  usb_write_port_complete,
			  pxmitbuf);/* context is pxmitbuf */

	status = usb_submit_urb(purb, GFP_ATOMIC);
	if (status) {
		rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_WRITE_PORT_ERR);
		DBG_88E("usb_write_port, status =%d\n", status);
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("usb_write_port(): usb_submit_urb, status =%x\n", status));

		switch (status) {
		case -ENODEV:
			padapter->bDriverStopped = true;
			break;
		default:
			break;
		}
		goto exit;
	}

	ret = _SUCCESS;

/*    We add the URB_ZERO_PACKET flag to urb so that the host will send the zero packet automatically. */

	RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("-usb_write_port\n"));

exit:
	if (ret != _SUCCESS)
		rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
	return ret;
}

void usb_write_port_cancel(struct adapter *padapter)
{
	int i, j;
	struct xmit_buf *pxmitbuf = (struct xmit_buf *)padapter->xmitpriv.pxmitbuf;

	DBG_88E("%s\n", __func__);

	padapter->bWritePortCancel = true;

	for (i = 0; i < NR_XMITBUFF; i++) {
		for (j = 0; j < 8; j++) {
			if (pxmitbuf->pxmit_urb[j])
				usb_kill_urb(pxmitbuf->pxmit_urb[j]);
		}
		pxmitbuf++;
	}

	pxmitbuf = (struct xmit_buf *)padapter->xmitpriv.pxmit_extbuf;
	for (i = 0; i < NR_XMIT_EXTBUFF; i++) {
		for (j = 0; j < 8; j++) {
			if (pxmitbuf->pxmit_urb[j])
				usb_kill_urb(pxmitbuf->pxmit_urb[j]);
		}
		pxmitbuf++;
	}
}

void rtl8188eu_recv_tasklet(void *priv)
{
	struct sk_buff *pskb;
	struct adapter *adapt = priv;
	struct recv_priv *precvpriv = &adapt->recvpriv;

	while (NULL != (pskb = skb_dequeue(&precvpriv->rx_skb_queue))) {
		if ((adapt->bDriverStopped) || (adapt->bSurpriseRemoved)) {
			DBG_88E("recv_tasklet => bDriverStopped or bSurpriseRemoved\n");
			dev_kfree_skb_any(pskb);
			break;
		}
		recvbuf2recvframe(adapt, pskb);
		skb_reset_tail_pointer(pskb);
		pskb->len = 0;
		skb_queue_tail(&precvpriv->free_recv_skb_queue, pskb);
	}
}

void rtl8188eu_xmit_tasklet(void *priv)
{
	int ret = false;
	struct adapter *adapt = priv;
	struct xmit_priv *pxmitpriv = &adapt->xmitpriv;

	if (check_fwstate(&adapt->mlmepriv, _FW_UNDER_SURVEY))
		return;

	while (1) {
		if ((adapt->bDriverStopped) ||
		    (adapt->bSurpriseRemoved) ||
		    (adapt->bWritePortCancel)) {
			DBG_88E("xmit_tasklet => bDriverStopped or bSurpriseRemoved or bWritePortCancel\n");
			break;
		}

		ret = rtl8188eu_xmitframe_complete(adapt, pxmitpriv, NULL);

		if (!ret)
			break;
	}
}
