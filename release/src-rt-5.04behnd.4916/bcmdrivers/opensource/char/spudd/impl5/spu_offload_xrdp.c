/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.
:> 
*/
#include <linux/kthread.h>
#include <linux/sched.h>
#include <net/dst.h>
#include <net/xfrm.h>
#include <net/gro_cells.h>
#include "rdpa_api.h"
#include "rdp_cpu_ring_defs.h"
#include "rdp_cpu_ring.h"
#include "spu_blog.h"
#include "linux/bcm_log.h"
#include <linux/platform_device.h>

#define SPU_INTERRUPT_COALESCING_TIMEOUT_US 500
#define SPU_INTERRUPT_COALESCING_MAX_PKT_CNT 32
#define SPU_RX_HANDLER_BUDGET 16

#define MAX_OFFLOAD_KEY_SIZE  128

struct spu_offload_info
{
	wait_queue_head_t   rx_thread_wqh;
	struct task_struct *rx_thread;
	int work_avail;
	struct gro_cells gro_cells;
	void *rdpa_port_priv;
};

struct spu_xrdp_session_info {
	uint8_t taken;
	uint64_t key_buf_dma_addr;
	CRYPTO_SESSION_SEQ_INFO_STRUCT *offload_seq;
	uint8_t *key_bufp;
};

struct spu_xrdp_offload_state
{
	void *ses_seq_base;
	dma_addr_t ses_seq_dma_base;
	void *key_buf_base;
	dma_addr_t key_buf_dma_base;

	struct spu_xrdp_session_info session_mem[MAX_SPU_OFFLOAD_SESSIONS];
};

static struct spu_xrdp_offload_state spu_xrdp_state_g;

static struct spu_offload_info spu_offload_inst;
extern void bdmf_sysb_databuf_recycle(void *pBuf);

#define SPU_WAKEUP_RXWORKER() do { \
            wake_up_interruptible(&spu_offload_inst.rx_thread_wqh); \
          } while (0)

static void spu_offload_isr_callback(long priv)
{
	/* handle interrupt */
	rdpa_cpu_int_disable(rdpa_cpu_spu, 0);
	rdpa_cpu_int_clear(rdpa_cpu_spu, 0);

	spu_offload_inst.work_avail = 1;
	/* wake up the packet handler */
	SPU_WAKEUP_RXWORKER();
}

static int spu_offload_config_tc_to_queue(void)
{
    bdmf_object_handle rdpa_cpu_obj = NULL;
    bdmf_object_handle rdpa_sys_obj = NULL;
    int rc = 0;
    int tc_idx, tc_idx_start, tc_idx_end;
    rdpa_cpu_tc  cpu_tc_threshold = rdpa_cpu_tc0;

    rc = rdpa_system_get(&rdpa_sys_obj);
    rc = rc ? rc : rdpa_system_high_prio_tc_threshold_get(rdpa_sys_obj, &cpu_tc_threshold);
    rc = rc ? rc : rdpa_cpu_get(rdpa_cpu_spu, &rdpa_cpu_obj);

    if (rc)
        goto tc2queue_exit;

    tc_idx_start = rdpa_cpu_tc0;
    tc_idx_end = rdpa_cpu_tc7;

    for (tc_idx = tc_idx_start; tc_idx <= tc_idx_end;  tc_idx ++)
        rdpa_cpu_tc_to_rxq_set(rdpa_cpu_obj, tc_idx, 0);

tc2queue_exit:
    if (rdpa_sys_obj)
        bdmf_put(rdpa_sys_obj);

    if (rdpa_cpu_obj)
        bdmf_put(rdpa_cpu_obj);

    return rc;
}

static int spu_offload_config_rx_queue(void)
{
	rdpa_cpu_rxq_cfg_t rxq_cfg;
	bdmf_object_handle rdpa_cpu_obj;
	int rc = 0;

	if (rdpa_cpu_get(rdpa_cpu_spu, &rdpa_cpu_obj))
		return -1;
	/* Read current config, set new thresholds and ISR */
	rc = rdpa_cpu_rxq_cfg_get(rdpa_cpu_obj, 0, &rxq_cfg);
	if (rc)
		goto unlock_exit;

	rxq_cfg.isr_priv = 0;
	rxq_cfg.size = 1024;
	rxq_cfg.rx_isr = spu_offload_isr_callback;
	rxq_cfg.ring_head = NULL;
	rxq_cfg.ic_cfg.ic_enable = true;
	rxq_cfg.ic_cfg.ic_timeout_us = SPU_INTERRUPT_COALESCING_TIMEOUT_US;
	rxq_cfg.ic_cfg.ic_max_pktcnt = SPU_INTERRUPT_COALESCING_MAX_PKT_CNT;
	rxq_cfg.rxq_stat = NULL;

	rc = rdpa_cpu_rxq_cfg_set(rdpa_cpu_obj, 0, &rxq_cfg);
	rc |= spu_offload_config_tc_to_queue();

        rdpa_spu_set_cpu_irq(rdpa_cpu_obj, 0);

unlock_exit:
	bdmf_put(rdpa_cpu_obj);
	return rc;

}

/* Both spu_us_dummy and spu_ds_dummy devices use the same port object in rdpa */
#define SPU_PORT_NAME "spu"

static int spu_offload_rdpa_init(void)
{
	int rc;
	bdmf_object_handle rdpa_cpu_obj, rdpa_port_obj;
	rdpa_port_dp_cfg_t port_cfg = {};

	BDMF_MATTR_ALLOC(cpu_spu_attrs, rdpa_cpu_drv());
	BDMF_MATTR_ALLOC(rdpa_port_attrs, rdpa_port_drv());

	/* create cpu */
	rdpa_cpu_index_set(cpu_spu_attrs, rdpa_cpu_spu);
	/* there is only 1 queue from runner to host */
	rdpa_cpu_num_queues_set(cpu_spu_attrs, 1);

	if ((rc = bdmf_new_and_set(rdpa_cpu_drv(), NULL, cpu_spu_attrs, &rdpa_cpu_obj)))
	{
		pr_err("%s:Failed to create xrdp spu cpu object rc(%d)\n", __func__, rc);
		goto exit_err;
	}
	if ((rc = rdpa_cpu_int_connect_set(rdpa_cpu_obj, true)) && rc != BDMF_ERR_ALREADY)
	{
		pr_err("%s:Failed to connect xrdp spu cpu interrupts rc(%d)\n", __func__, rc);
		goto exit_err;
	}

	rdpa_port_index_set(rdpa_port_attrs, rdpa_cpu_spu);
	rdpa_port_type_set(rdpa_port_attrs, rdpa_port_cpu);
	rdpa_port_name_set(rdpa_port_attrs, SPU_PORT_NAME);
	rdpa_port_handle_set(rdpa_port_attrs, RDPA_PORT_CPU_HANDLE);
	rdpa_port_cpu_obj_set(rdpa_port_attrs, rdpa_cpu_obj);
	rc = bdmf_new_and_set(rdpa_port_drv(), NULL, rdpa_port_attrs, &rdpa_port_obj);
	if (rc)
	{
		pr_err("%s:Failed to create rdpa port object rc(%d)\n", __func__, rc);
		goto exit_err;
	}
	spu_offload_inst.rdpa_port_priv = rdpa_port_obj;

	if ((rc = rdpa_port_cfg_get(spu_offload_inst.rdpa_port_priv, &port_cfg)))
	{
		pr_err("%s:Failed to get rdpa port config rc(%d)\n", __func__, rc);
		goto exit_err;
	}
	if ((rc = rdpa_port_cfg_set(spu_offload_inst.rdpa_port_priv, &port_cfg)))
	{
		pr_err("%s:Failed to set rdpa port config rc(%d)\n", __func__, rc);
		goto exit_err;
	}

	/* configure the rxq */
	if (spu_offload_config_rx_queue())
	{
		pr_err("%s:Failed to config rdpa rxq rx(%d)\n", __func__, rc);
		goto exit_err;
	}
	
	rdpa_cpu_int_enable(rdpa_cpu_spu, 0);
	return 0;

exit_err:
	if (spu_offload_inst.rdpa_port_priv != NULL)
	{
		bdmf_destroy(spu_offload_inst.rdpa_port_priv);
		spu_offload_inst.rdpa_port_priv = NULL;
	}
	if (!rdpa_cpu_get(rdpa_cpu_spu, &rdpa_cpu_obj))
	{
		bdmf_put(rdpa_cpu_obj);
		bdmf_destroy(rdpa_cpu_obj);
	}
	return -1;
}


static void spu_offload_rdpa_uninit(void)
{
	bdmf_object_handle rdpa_cpu_obj;

	rdpa_cpu_int_disable(rdpa_cpu_spu, 0);

	if (spu_offload_inst.rdpa_port_priv != NULL)
	{
		bdmf_destroy(spu_offload_inst.rdpa_port_priv);
		spu_offload_inst.rdpa_port_priv = NULL;
	}
	if (!rdpa_cpu_get(rdpa_cpu_spu, &rdpa_cpu_obj))
	{
		bdmf_put(rdpa_cpu_obj);
		bdmf_destroy(rdpa_cpu_obj);
	}
}

static inline int rx_pkt_from_q(int budget)
{
	int rc, count = 0;
	FkBuff_t *fkb = NULL;
	rdpa_cpu_rx_info_t info = {};
	BlogAction_t blogAction;
	BlogFcArgs_t fcArgs = {};
	struct spu_offload_rx_info s_info = {};
	uint8_t * datap;
	struct sk_buff *skb;

	do
	{
		rc = rdpa_cpu_packet_get(rdpa_cpu_spu, 0, &info);
		if (rc)
			continue;

		if (spu_offload_get_rx_info(info.spu_session_id, &s_info)) {
			flow_log("invalid session information for %d\n", info.spu_session_id);
			bdmf_sysb_databuf_free((uint8_t *)info.data, 0);
			continue;
		}

		count++;
		datap =  (uint8_t *)info.data + info.data_offset;

		fkb = fkb_init(datap, BCM_PKT_HEADROOM+info.data_offset, datap, info.size);
		fkb->recycle_hook = (RecycleFuncP)bdmf_sysb_recycle;
		fkb->recycle_context = 0;

		if (info.spu_session_id < MAX_SPU_OFFLOAD_SESSIONS/2) {
			/* DS */
			fcArgs.esp_inner_pkt = 1;
			blogAction = blog_finit(fkb, iproc_priv.spu_dev_ds, TYPE_IP, s_info.blog_chan_id, BLOG_SPU_DS, &fcArgs);
		} else {
			/* US */
			fcArgs.esp_over_udp = s_info.esp_over_udp;
			fcArgs.esp_spi = s_info.spi;
			fcArgs.esp_ivsize = s_info.iv_size;
			fcArgs.esp_icvsize = s_info.digestsize;
			blogAction = blog_finit(fkb, iproc_priv.spu_dev_us, TYPE_IP, s_info.blog_chan_id, BLOG_SPU_US, &fcArgs);
		}

		if (blogAction != PKT_DONE)
		{
			if (blogAction == PKT_DROP)
			{
				bdmf_sysb_databuf_recycle(fkb);
				goto release_ptrs;
			}
			if (blogAction == PKT_NORM)
			{
				fkb = fkb_init(datap, BCM_PKT_HEADROOM+info.data_offset, datap, info.size);
				fkb->recycle_hook = (RecycleFuncP)bdmf_sysb_recycle;
				fkb->recycle_context = 0;
			}

			skb = nbuff_xlate(FKBUFF_2_PNBUFF(fkb));

			if (info.spu_session_id < MAX_SPU_OFFLOAD_SESSIONS/2) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
				skb->sp = secpath_get((struct sec_path *)s_info.sp);
#else
				struct sec_path *sp = secpath_set(skb);
				if (!sp) {
					pr_err("%s:%d - failed to set secpath for skb %p\n",
					__func__, __LINE__, skb);
					bdmf_sysb_databuf_recycle(fkb);
					goto release_ptrs;
				}
				xfrm_state_hold(s_info.ptr);
				flow_log("%s:%d held xfrm=%p refcnt=%d\n",
					__func__, __LINE__, s_info.ptr,
					refcount_read(&((struct xfrm_state *)s_info.ptr)->refcnt));
    
				sp->xvec[sp->len++] = s_info.ptr
#endif
				skb->dev = iproc_priv.spu_dev_ds;
				skb_shinfo(skb)->dirty_p = datap + info.size;
				skb->protocol = 8;
				gro_cells_receive(&spu_offload_inst.gro_cells, skb);
			} else {
				skb_dst_set(skb, dst_clone(s_info.dst_p));
				skb_reset_network_header(skb);
				skb->dev = iproc_priv.spu_dev_us;
				xfrm_output_resume(skb, 0);
			}
		}
release_ptrs:
		if (info.spu_session_id < MAX_SPU_OFFLOAD_SESSIONS/2) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
			secpath_put(s_info.sp);
#else
			xfrm_state_put(s_info.ptr);
#endif
		} else {
			dst_release(s_info.dst_p);
		}

	} while(count < budget && likely(!rc));
	return count;
}

static void spu_xrdp_handle_exception(void)
{
	int i;
	uint32_t exception;
	struct spu_xrdp_session_info *session;

	for (i = 0; i < MAX_SPU_OFFLOAD_SESSIONS; i++) {
		session = &spu_xrdp_state_g.session_mem[i];
		if (session->taken == 0)
			continue;

		cache_invalidate_len(session->offload_seq, sizeof(CRYPTO_SESSION_SEQ_INFO_STRUCT));
		if (session->offload_seq->exception) {
			uint8_t data_limit = 0;
			uint8_t overflow = 0;

			exception = be32_to_cpu(session->offload_seq->exception);
			if (exception & ((1 << SPU_HARD_LIMIT_BIT) | (1 << SPU_SOFT_LIMIT_BIT)))
				data_limit = 1;
			if (exception & (1 << SPU_OVERFLOW_BIT))
				overflow = 1;
			spu_offload_handle_exception(i, data_limit, overflow);
			/* clear all exception for this session */
			session->offload_seq->exception = 0;
			wmb();
		}
	}
}

static int spu_xrdp_rx_handler(void *context)
{
	int work =  0;

	while (1)
	{
		wait_event_interruptible(spu_offload_inst.rx_thread_wqh,
					 spu_offload_inst.work_avail ||
					 kthread_should_stop());
		if (kthread_should_stop())
		{
			pr_err("%s kthread_should_stop detected\n", __func__);
			break;
		}
		spu_xrdp_handle_exception();

		/* handle rx packet */
		work += rx_pkt_from_q(SPU_RX_HANDLER_BUDGET);

		if (rdpa_cpu_queue_not_empty(rdpa_cpu_spu, 0))
		{
			if (work >= SPU_RX_HANDLER_BUDGET)
			{
				schedule();
			}
		}
		else
		{
			spu_offload_inst.work_avail = 0;
			work = 0;
			rdpa_cpu_int_enable(rdpa_cpu_spu, 0);
		}

	}
	return 0;
}

int spu_platform_offload_init(void)
{
	char threadname[15]={0};
	struct task_struct *thread;

	if (spu_offload_rdpa_init())
	{
		pr_err("Failed to start spu offloading on rdpa\n");
		return -1;
	}

	init_waitqueue_head(&spu_offload_inst.rx_thread_wqh);

	sprintf(threadname, "spu_xrdp_rx");
	thread = kthread_create(spu_xrdp_rx_handler, NULL, threadname);

	if (IS_ERR(thread))
	{
		pr_err("Failed to create %s kthread\n", threadname);
		return -1;
	}
	spu_offload_inst.rx_thread = thread;
	spu_offload_inst.work_avail = 0;
	wake_up_process(thread);

	return 0;
}

int spu_offload_postinit(void)
{
	int ret = gro_cells_init(&spu_offload_inst.gro_cells, iproc_priv.spu_dev_ds);
	if (ret)
		spu_offload_inst.gro_cells.cells = NULL;
	return ret;
}

void spu_offload_deinit(void)
{
	gro_cells_destroy(&spu_offload_inst.gro_cells);
	spu_offload_inst.gro_cells.cells = NULL;
	spu_offload_rdpa_uninit();
}

static inline int _spu_netdev_to_rdpa_port_obj(struct net_device *dev, bcm_netdev_priv_info_out_t *info_out)
{
	info_out->bcm_netdev_to_rdpa_port_obj.rdpa_port_obj = spu_offload_inst.rdpa_port_priv;
	return 0;
}

int spu_offload_priv_info_get(struct net_device *dev, bcm_netdev_priv_info_type_t info_type, bcm_netdev_priv_info_out_t *info_out)
{
	int rc = -1;
	switch (info_type)
	{
		case BCM_NETDEV_TO_RDPA_PORT_OBJ:
			rc = _spu_netdev_to_rdpa_port_obj(dev, info_out);
		break;
		default:
		break;
	}
	return rc;
}

int spu_offload_insert_us_pkt(pNBuff_t pNBuf, int session_id, uint32_t digestsize, uint32_t *payloadlen)
{
	uint8_t *datap, *hdr, *pdata;
	uint8_t hdr_size;
	uint32_t padlen, hdr_offset;
	int rc = 0;
	rdpa_cpu_tx_info_t info = {};

	spu_offload_get_fixed_hdr(session_id, &hdr_offset, &hdr_size, &hdr);

	/* revert the outer IP header and padding done by the Blog layer
	 * Runner will handle that */
        if (IS_SKBUFF_PTR(pNBuf))
        {	
		struct sk_buff *skb = (struct sk_buff *)PNBUFF_2_PBUF(pNBuf);

		pdata = skb->data;
		padlen  = pdata[skb->len - digestsize - 2];

		skb->data = &pdata[hdr_offset];
		skb->len -= (hdr_offset + padlen + BLOG_ESP_PADLEN_LEN + BLOG_ESP_NEXT_PROTO_LEN + digestsize);
		*payloadlen = skb->len;
        }
        else
        {
		FkBuff_t *fkb = (FkBuff_t *)PNBUFF_2_PBUF(pNBuf);

		pdata = fkb->data;
		padlen  = pdata[fkb->len - digestsize - 2];

		fkb->data = &pdata[hdr_offset];
		fkb->len -= (hdr_offset + padlen + BLOG_ESP_PADLEN_LEN + BLOG_ESP_NEXT_PROTO_LEN + digestsize);
		*payloadlen = fkb->len;
        }


	/* insert the fixed header in the header, mimic the flow match handling in Runner */
	if (IS_SKBUFF_PTR(pNBuf)) {
		struct sk_buff *skb = (struct sk_buff *)PNBUFF_2_PBUF(pNBuf);
		datap = skb->data - hdr_size;
		skb->data = datap;
		skb->len += hdr_size;

		memcpy(datap, hdr, hdr_size);
	}
	else {
		FkBuff_t *fkb = (FkBuff_t *)PNBUFF_2_PBUF(pNBuf);
		datap = fkb->data - hdr_size;
		fkb->data = datap;
		fkb->len += hdr_size;

		memcpy(datap, hdr, hdr_size);
	}

	/* found an offload session */
	info.method = rdpa_cpu_tx_egress;
	info.cpu_port = rdpa_cpu_spu;
	info.port_obj = spu_offload_inst.rdpa_port_priv;
	info.l3_packet = 1;
	info.crypto_session_id = session_id;
	info.flags = 0;
	
	rc = rdpa_cpu_send_sysb(pNBuf, &info);
	if (rc)
		printk("something wrong rc %d\n", rc);
	return rc;
}

void spu_platform_offload_stats(uint32_t session_id, struct spu_offload_tracker *curr,
				uint32_t *bitmap, uint8_t long_bitmap)
{
	CRYPTO_SESSION_SEQ_INFO_STRUCT local;
	struct spu_xrdp_session_info *session = &spu_xrdp_state_g.session_mem[session_id];
	uint32_t type;
	uint32_t *offload_bitmap;
	int i;

	cache_invalidate_len(session->offload_seq, sizeof(CRYPTO_SESSION_SEQ_INFO_STRUCT));
	memcpy(&local, session->offload_seq, sizeof(CRYPTO_SESSION_SEQ_INFO_STRUCT));

	curr->lft_bytes = ((uint64_t)be32_to_cpu(local.curlft_bytes_hi) << 32) |
			be32_to_cpu(local.curlft_bytes_lo);
	curr->lft_pkts = ((uint64_t)be32_to_cpu(local.curlft_pkts_hi) << 32) |
			be32_to_cpu(local.curlft_pkts_lo);

	curr->replay_window = be32_to_cpu(local.err_repl_window);
	curr->replay = be32_to_cpu(local.err_replay);
	curr->seq_lo = be32_to_cpu(local.last_seqno_lo);
	curr->seq_hi = be32_to_cpu(local.last_seqno_hi);

	type = be32_to_cpu(local.type_replay_window);
	if (type & (1 << SPU_DATA_LIMIT_BIT)) {
		offload_bitmap = &local.bitmap_2;
	}
	else {
		offload_bitmap = &local.bitmap_1;
	}

	if (long_bitmap) {
		for (i = 0; i < (MAX_REPLAY_WIN_SIZE / 32); i++) {
			bitmap[i] = be32_to_cpu(offload_bitmap[i]);
		} 
	} else {
		bitmap[0] = be32_to_cpu(offload_bitmap[0]);
	}
}

int spu_platform_offload_session_ds_parm(int session_id, struct xfrm_state *xfrm,
					  struct bcmspu_offload_parm *parm)
{
	rdpa_crypto_session_info_t info = {};
	uint32_t type = 0, replay_window, limit = MAX_REPLAY_WIN_SIZE;
	uint32_t *bitmap;
	struct spu_xrdp_session_info *session = &spu_xrdp_state_g.session_mem[session_id];

	/* clear the seqence number memory */
	memset(session->offload_seq, 0, sizeof(CRYPTO_SESSION_SEQ_INFO_STRUCT));

	/* for Runner platforms, the key buffers located in DDR separately */
	memcpy(session->key_bufp, &parm->spu_header[FMD_SIZE], parm->key_size);

	/* parm->fixed_hdr_size is platform specific */
	parm->fixed_hdr_size = FMD_SIZE;
	if (parm->is_esn) {
		parm->fixed_hdr_size += BLOG_ESP_SEQNUM_HI_LEN;
		memset(&parm->spu_header[FMD_SIZE], 0xFFFFFFFF, 4);
		type |= 1 << SPU_ESN_BIT;
		info.is_esn = 1;
	}
	bitmap = &session->offload_seq->bitmap_1;
	session->offload_seq->last_seqno_lo = cpu_to_be32(parm->seq_lo);
	session->offload_seq->last_seqno_hi = cpu_to_be32(parm->seq_hi);

	spin_lock_bh(&xfrm->lock);
	if (parm->data_limit) {
		type |= (1 << SPU_DATA_LIMIT_BIT);
		session->offload_seq->hard_byte_limit_hi = cpu_to_be32(xfrm->lft.hard_byte_limit >> 32);
		session->offload_seq->hard_byte_limit_lo = cpu_to_be32((uint32_t)xfrm->lft.hard_byte_limit);
		session->offload_seq->hard_pkt_limit_hi = cpu_to_be32(xfrm->lft.hard_packet_limit >> 32);
		session->offload_seq->hard_pkt_limit_lo = cpu_to_be32((uint32_t)xfrm->lft.hard_packet_limit);
		session->offload_seq->soft_byte_limit_hi = cpu_to_be32(xfrm->lft.soft_byte_limit >> 32);
		session->offload_seq->soft_byte_limit_lo = cpu_to_be32((uint32_t)xfrm->lft.soft_byte_limit);
		session->offload_seq->soft_pkt_limit_hi = cpu_to_be32(xfrm->lft.soft_packet_limit >> 32);
		session->offload_seq->soft_pkt_limit_lo = cpu_to_be32((uint32_t)xfrm->lft.soft_packet_limit);
		bitmap = &session->offload_seq->bitmap_2;
		limit = MAX_REPLAY_WIN_SIZE / 2;
	}
	/* copy the transmitted bytes to xrdp */
	session->offload_seq->curlft_bytes_hi = cpu_to_be32(xfrm->curlft.bytes >> 32);
	session->offload_seq->curlft_bytes_lo = cpu_to_be32((uint32_t)xfrm->curlft.bytes);
	session->offload_seq->curlft_pkts_hi = cpu_to_be32(xfrm->curlft.packets >> 32);
	session->offload_seq->curlft_pkts_lo = cpu_to_be32((uint32_t)xfrm->curlft.packets);

	/* copy the xfrm bitmap to xrdp */
	if (xfrm->replay_esn) {
		uint32_t i, nr;

		replay_window = xfrm->replay_esn->replay_window;
		nr = replay_window >> 5;
		for (i = 0; i < nr; i++) {
			bitmap[i] = cpu_to_be32(xfrm->replay_esn->bmp[i]);
		}
	} else {
		replay_window = xfrm->props.replay_window;
		*bitmap = cpu_to_be32(xfrm->replay.bitmap);
	}
	spin_unlock_bh(&xfrm->lock);

	if (replay_window > limit)
		return -1;
	
	session->taken = 1;
	session->offload_seq->type_replay_window = cpu_to_be32(type | replay_window);

	info.session_id = parm->session_id;
	info.key_size = parm->key_size;
	info.digest_size = parm->digest_size;
	info.key_buf_dma_addr = session->key_buf_dma_addr;
	rdpa_crypto_session_info_set(&info);

	return 0;
}

int spu_platform_offload_session_us_parm(int session_id, struct xfrm_state *xfrm,
					  struct bcmspu_offload_parm *parm)
{
	rdpa_crypto_session_info_t info = {};
	uint32_t type = 0;
	uint16_t base_chksm;
	uint8_t *data_p;

	struct spu_xrdp_session_info *session = &spu_xrdp_state_g.session_mem[session_id];

	session->taken = 1;
	/* clear the seqence number memory */
	memset(session->offload_seq, 0, sizeof(CRYPTO_SESSION_SEQ_INFO_STRUCT));

	/* for Runner platforms, the key buffers located in DDR separately */
	memcpy(session->key_bufp, &parm->spu_header[FMD_SIZE], parm->key_size);
	info.key_buf_dma_addr = session->key_buf_dma_addr;

	if (parm->is_esn) {
		CRYPTO_SESSION_SEQ_HI_INFO_STRUCT *seqhi_info = (CRYPTO_SESSION_SEQ_HI_INFO_STRUCT *)&session->offload_seq->seq_hi_info;

		seqhi_info->key_size = cpu_to_be32(parm->key_size);
		/* copy keys to hi buffer as well, note seqhi is after the keys */
		memcpy(&session->key_bufp[parm->key_size], &parm->spu_header[FMD_SIZE + parm->key_size], BLOG_ESP_SEQNUM_HI_LEN);

		/* adjust the parm->keysize to include seqhi, for SPU PD */
		parm->key_size += BLOG_ESP_SEQNUM_HI_LEN;
		memcpy(&session->key_bufp[MAX_OFFLOAD_KEY_SIZE], session->key_bufp, parm->key_size);
		if (parm->seq_hi & 1) {
			info.key_buf_dma_addr = session->key_buf_dma_addr + MAX_OFFLOAD_KEY_SIZE;
		}
		seqhi_info->key_buff_lo0 = (uint32_t)(session->key_buf_dma_addr);
		seqhi_info->key_buff_lo1 = (uint32_t)(session->key_buf_dma_addr + MAX_OFFLOAD_KEY_SIZE);
		seqhi_info->key_buff_hi = cpu_to_be32((uint32_t)(session->key_buf_dma_addr >> 32));

		info.is_esn = 1;
		type |= (1 << SPU_ESN_BIT);
		session->offload_seq->oseq_hi = cpu_to_be32(parm->seq_hi);
	}
	session->offload_seq->oseq_lo = cpu_to_be32(parm->seq_lo);

	/* parm->fixed_hdr_size is platform specific */
	parm->fixed_hdr_size = FMD_SIZE + BLOG_IPV4_HDR_LEN + BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN + parm->iv_size;

	/* shuffle the rest of the SPU header for XRDP FW needs, outer header is right after FMD */
	data_p = &parm->spu_header[FMD_SIZE];
	memcpy(data_p, &parm->outer_header, BLOG_IPV4_HDR_LEN);
	base_chksm = ((BlogIpv4Hdr_t *)data_p)->chkSum;
	((BlogIpv4Hdr_t *)data_p)->chkSum = 0;

	data_p += BLOG_IPV4_HDR_LEN;
	if (parm->esp_o_udp) {
		SPU_ESPOUDP_SCRATCH_STRUCT *espoudp_scratch = (SPU_ESPOUDP_SCRATCH_STRUCT *)&session->offload_seq->filler[RDD_CRYPTO_SESSION_SEQ_INFO_FILLER_NUMBER - 3];

		memcpy(data_p, &parm->outer_header[BLOG_IPV4_HDR_LEN], BLOG_UDP_HDR_LEN);
		data_p += BLOG_UDP_HDR_LEN;
		info.esp_o_udp = 1;
		parm->fixed_hdr_size += BLOG_UDP_HDR_LEN;

		espoudp_scratch->spi = parm->esp_spi;
	}
	/* add the ESP header after the Outer header */
	*((uint32_t *)data_p) = parm->esp_spi;
        data_p += BLOG_ESP_SPI_LEN;
	*((uint32_t *)data_p) = 0;
	data_p += BLOG_ESP_SEQNUM_LEN;
	/* leave empty space for IV generation */
        memset(data_p, 0, parm->iv_size);

	info.digest_size = parm->digest_size;
	info.session_id = session_id;
	info.key_size = parm->key_size;

	/* if this is a data limiting session */
	spin_lock_bh(&xfrm->lock);
	if (parm->data_limit) {
		type |= (1 << SPU_DATA_LIMIT_BIT);
		session->offload_seq->hard_byte_limit_hi = cpu_to_be32(xfrm->lft.hard_byte_limit >> 32);
		session->offload_seq->hard_byte_limit_lo = cpu_to_be32((uint32_t)xfrm->lft.hard_byte_limit);
		session->offload_seq->hard_pkt_limit_hi = cpu_to_be32(xfrm->lft.hard_packet_limit >> 32);
		session->offload_seq->hard_pkt_limit_lo = cpu_to_be32((uint32_t)xfrm->lft.hard_packet_limit);
		session->offload_seq->soft_byte_limit_hi = cpu_to_be32(xfrm->lft.soft_byte_limit >> 32);
		session->offload_seq->soft_byte_limit_lo = cpu_to_be32((uint32_t)xfrm->lft.soft_byte_limit);
		session->offload_seq->soft_pkt_limit_hi = cpu_to_be32(xfrm->lft.soft_packet_limit >> 32);
		session->offload_seq->soft_pkt_limit_lo = cpu_to_be32((uint32_t)xfrm->lft.soft_packet_limit);
	}
	/* copy the transmitted bytes to xrdp */
	session->offload_seq->curlft_bytes_hi = cpu_to_be32(xfrm->curlft.bytes >> 32);
	session->offload_seq->curlft_bytes_lo = cpu_to_be32((uint32_t)xfrm->curlft.bytes);
	session->offload_seq->curlft_pkts_hi = cpu_to_be32(xfrm->curlft.packets >> 32);
	session->offload_seq->curlft_pkts_lo = cpu_to_be32((uint32_t)xfrm->curlft.packets);
	spin_unlock_bh(&xfrm->lock);

	session->offload_seq->type_chksm = cpu_to_be32(type | cpu_to_be16(base_chksm));

	rdpa_crypto_session_info_set(&info);

	return 0;
}

void spu_platform_offload_free_session(uint32_t session_id)
{
	struct spu_xrdp_session_info *session = &spu_xrdp_state_g.session_mem[session_id];

	if (!session->taken)
		flow_log("session %u is not taken\n", session_id);
	session->taken = 0;
}

void spu_platform_offload_register(void)
{
	int i, aligned_size;
	uint8_t *datap, *key_bufp;
	uint64_t key_bufp_dma;
	struct spu_xrdp_session_info *session;
	struct spu_xrdp_offload_state *statep = &spu_xrdp_state_g;

	/* initialize platform specific offload memory */
	memset(statep, 0, sizeof(struct spu_xrdp_offload_state));
	aligned_size = BCM_DCACHE_ALIGN(sizeof(CRYPTO_SESSION_SEQ_INFO_STRUCT));

	statep->ses_seq_base = dma_alloc_coherent(&iproc_priv.pdev->dev,
						  aligned_size * MAX_SPU_OFFLOAD_SESSIONS,
						  &statep->ses_seq_dma_base, GFP_KERNEL);
	datap = statep->ses_seq_base;

	rdpa_spu_crypto_session_base_set((uint64_t)statep->ses_seq_dma_base);

	/* DS sessions uses 1 key buffer, US sessions has 2 key buffers */
	statep->key_buf_base = dma_alloc_coherent(&iproc_priv.pdev->dev,
					MAX_OFFLOAD_KEY_SIZE * (MAX_SPU_OFFLOAD_SESSIONS * 3 / 2),
					&statep->key_buf_dma_base, GFP_KERNEL);
	key_bufp = statep->key_buf_base;
	key_bufp_dma = (uint64_t)statep->key_buf_dma_base;

	/* initializaing DS sessions */
	for (i=0; i < MAX_SPU_OFFLOAD_SESSIONS/2; i++) {
		session = &statep->session_mem[i];
		session->offload_seq = (CRYPTO_SESSION_SEQ_INFO_STRUCT *)datap;
		session->key_bufp = key_bufp;
		session->key_buf_dma_addr = key_bufp_dma;
		datap += aligned_size;
		key_bufp += MAX_OFFLOAD_KEY_SIZE;
		key_bufp_dma += MAX_OFFLOAD_KEY_SIZE;
	}
	/* initializaing US sessions */
	for (i=MAX_SPU_OFFLOAD_SESSIONS/2; i < MAX_SPU_OFFLOAD_SESSIONS; i++) {
		session = &statep->session_mem[i];
		session->offload_seq = (CRYPTO_SESSION_SEQ_INFO_STRUCT *)datap;
		session->key_bufp = key_bufp;
		session->key_buf_dma_addr = key_bufp_dma;
		datap += aligned_size;
		key_bufp += MAX_OFFLOAD_KEY_SIZE * 2;
		key_bufp_dma += MAX_OFFLOAD_KEY_SIZE * 2;
	}
}

void spu_platform_offload_deregister(void)
{
	int aligned_size;
	struct spu_xrdp_offload_state *statep = &spu_xrdp_state_g;

	rdpa_spu_crypto_session_base_set((uint64_t)0);


	aligned_size = BCM_DCACHE_ALIGN(sizeof(CRYPTO_SESSION_SEQ_INFO_STRUCT));
	dma_free_coherent(&iproc_priv.pdev->dev,
			  aligned_size * MAX_SPU_OFFLOAD_SESSIONS,
			  statep->ses_seq_base, statep->ses_seq_dma_base);

	dma_free_coherent(&iproc_priv.pdev->dev,
			  MAX_OFFLOAD_KEY_SIZE * (MAX_SPU_OFFLOAD_SESSIONS * 3 / 2),
			  statep->key_buf_base, statep->key_buf_dma_base);
}

