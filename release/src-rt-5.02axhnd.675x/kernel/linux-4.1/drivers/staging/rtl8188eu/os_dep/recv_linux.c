/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
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
 *
 ******************************************************************************/
#define _RECV_OSDEP_C_

#include <osdep_service.h>
#include <drv_types.h>

#include <wifi.h>
#include <recv_osdep.h>

#include <osdep_intf.h>
#include <usb_ops_linux.h>

/* alloc os related resource in struct recv_frame */
int rtw_os_recv_resource_alloc(struct adapter *padapter,
			       struct recv_frame *precvframe)
{
	precvframe->pkt_newalloc = NULL;
	precvframe->pkt = NULL;
	return _SUCCESS;
}

/* alloc os related resource in struct recv_buf */
int rtw_os_recvbuf_resource_alloc(struct adapter *padapter,
				  struct recv_buf *precvbuf)
{
	int res = _SUCCESS;

	precvbuf->purb = usb_alloc_urb(0, GFP_KERNEL);
	if (precvbuf->purb == NULL)
		res = _FAIL;
	precvbuf->pskb = NULL;
	precvbuf->reuse = false;
	return res;
}

void rtw_handle_tkip_mic_err(struct adapter *padapter, u8 bgroup)
{
	union iwreq_data wrqu;
	struct iw_michaelmicfailure    ev;
	struct mlme_priv *pmlmepriv  = &padapter->mlmepriv;
	struct security_priv	*psecuritypriv = &padapter->securitypriv;
	u32 cur_time = 0;

	if (psecuritypriv->last_mic_err_time == 0) {
		psecuritypriv->last_mic_err_time = jiffies;
	} else {
		cur_time = jiffies;

		if (cur_time - psecuritypriv->last_mic_err_time < 60*HZ) {
			psecuritypriv->btkip_countermeasure = true;
			psecuritypriv->last_mic_err_time = 0;
			psecuritypriv->btkip_countermeasure_time = cur_time;
		} else {
			psecuritypriv->last_mic_err_time = jiffies;
		}
	}

	memset(&ev, 0x00, sizeof(ev));
	if (bgroup)
		ev.flags |= IW_MICFAILURE_GROUP;
	else
		ev.flags |= IW_MICFAILURE_PAIRWISE;

	ev.src_addr.sa_family = ARPHRD_ETHER;
	memcpy(ev.src_addr.sa_data, &pmlmepriv->assoc_bssid[0], ETH_ALEN);
	memset(&wrqu, 0x00, sizeof(wrqu));
	wrqu.data.length = sizeof(ev);
	wireless_send_event(padapter->pnetdev, IWEVMICHAELMICFAILURE,
			    &wrqu, (char *)&ev);
}

int rtw_recv_indicatepkt(struct adapter *padapter,
			 struct recv_frame *precv_frame)
{
	struct recv_priv *precvpriv;
	struct __queue *pfree_recv_queue;
	struct sk_buff *skb;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;


	precvpriv = &(padapter->recvpriv);
	pfree_recv_queue = &(precvpriv->free_recv_queue);

	skb = precv_frame->pkt;
	if (skb == NULL) {
		RT_TRACE(_module_recv_osdep_c_, _drv_err_,
			 ("rtw_recv_indicatepkt():skb == NULL something wrong!!!!\n"));
		goto _recv_indicatepkt_drop;
	}

	RT_TRACE(_module_recv_osdep_c_, _drv_info_,
		 ("rtw_recv_indicatepkt():skb != NULL !!!\n"));
	RT_TRACE(_module_recv_osdep_c_, _drv_info_,
		 ("rtw_recv_indicatepkt():precv_frame->rx_head =%p  precv_frame->hdr.rx_data =%p\n",
		 precv_frame->rx_head, precv_frame->rx_data));
	RT_TRACE(_module_recv_osdep_c_, _drv_info_,
		 ("precv_frame->hdr.rx_tail =%p precv_frame->rx_end =%p precv_frame->hdr.len =%d\n",
		 precv_frame->rx_tail, precv_frame->rx_end,
		 precv_frame->len));

	skb->data = precv_frame->rx_data;

	skb_set_tail_pointer(skb, precv_frame->len);

	skb->len = precv_frame->len;

	RT_TRACE(_module_recv_osdep_c_, _drv_info_,
		 ("skb->head =%p skb->data =%p skb->tail =%p skb->end =%p skb->len =%d\n",
		 skb->head, skb->data, skb_tail_pointer(skb),
		 skb_end_pointer(skb), skb->len));

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE)) {
		struct sk_buff *pskb2 = NULL;
		struct sta_info *psta = NULL;
		struct sta_priv *pstapriv = &padapter->stapriv;
		struct rx_pkt_attrib *pattrib = &precv_frame->attrib;
		int bmcast = IS_MCAST(pattrib->dst);

		if (memcmp(pattrib->dst, myid(&padapter->eeprompriv),
			   ETH_ALEN)) {
			if (bmcast) {
				psta = rtw_get_bcmc_stainfo(padapter);
				pskb2 = skb_clone(skb, GFP_ATOMIC);
			} else {
				psta = rtw_get_stainfo(pstapriv, pattrib->dst);
			}

			if (psta) {
				struct net_device *pnetdev;

				pnetdev = (struct net_device *)padapter->pnetdev;
				skb->dev = pnetdev;
				skb_set_queue_mapping(skb, rtw_recv_select_queue(skb));

				rtw_xmit_entry(skb, pnetdev);

				if (bmcast)
					skb = pskb2;
				else
					goto _recv_indicatepkt_end;
			}
		}
	}

	rcu_read_lock();
	rcu_dereference(padapter->pnetdev->rx_handler_data);
	rcu_read_unlock();

	skb->ip_summed = CHECKSUM_NONE;
	skb->dev = padapter->pnetdev;
	skb->protocol = eth_type_trans(skb, padapter->pnetdev);

	netif_rx(skb);

_recv_indicatepkt_end:

	/*  pointers to NULL before rtw_free_recvframe() */
	precv_frame->pkt = NULL;

	rtw_free_recvframe(precv_frame, pfree_recv_queue);

	RT_TRACE(_module_recv_osdep_c_, _drv_info_,
		 ("\n rtw_recv_indicatepkt :after netif_rx!!!!\n"));


	return _SUCCESS;

_recv_indicatepkt_drop:

	 /* enqueue back to free_recv_queue */
	rtw_free_recvframe(precv_frame, pfree_recv_queue);

	 return _FAIL;
}

void rtw_init_recv_timer(struct recv_reorder_ctrl *preorder_ctrl)
{

	setup_timer(&preorder_ctrl->reordering_ctrl_timer,
		    rtw_reordering_ctrl_timeout_handler,
		    (unsigned long)preorder_ctrl);
}
