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
	uint8_t *prepend_hdr;
};

struct spu_xrdp_offload_state
{
	void *ses_seq_base;
	dma_addr_t ses_seq_dma_base;
	void *key_buf_base;
	dma_addr_t key_buf_dma_base;
	void *prepend_base;
	dma_addr_t prepend_dma_base;

	struct spu_xrdp_session_info session_mem[MAX_SPU_OFFLOAD_SESSIONS];
};

static struct spu_xrdp_offload_state spu_xrdp_state_g;
static struct spu_offload_debug_info debug_info;

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

	if (cpu_spu_attrs)
		BDMF_MATTR_FREE(cpu_spu_attrs);
	if (rdpa_port_attrs)
		BDMF_MATTR_FREE(rdpa_port_attrs);

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

	if (cpu_spu_attrs)
		BDMF_MATTR_FREE(cpu_spu_attrs);
	if (rdpa_port_attrs)
		BDMF_MATTR_FREE(rdpa_port_attrs);

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
			debug_info.invalid_rx++;
			bdmf_sysb_databuf_free((uint8_t *)info.data, 0);
			continue;
		}

		count++;
		datap =  (uint8_t *)info.data + info.data_offset;

		fkb = fkb_init(datap, BCM_PKT_HEADROOM+info.data_offset, datap, info.size);
		fkb->recycle_hook = (RecycleFuncP)bdmf_sysb_recycle;
		fkb->recycle_context = 0;

		flow_log("%s(): fkb %px spu_session_id %d\n",
				__func__, fkb, info.spu_session_id);

		memset(&fcArgs, 0, sizeof(fcArgs));
		if (info.spu_session_id < MAX_SPU_OFFLOAD_SESSIONS/2) {
			/* DS */
			debug_info.slowpath_ds++;
			fcArgs.esp_inner_pkt = 1;
			fcArgs.esp_ivsize = s_info.iv_size;
			fcArgs.esp_icvsize = s_info.digestsize;
			blogAction = blog_finit(fkb, iproc_priv.spu_dev_ds, TYPE_IP, s_info.blog_chan_id, BLOG_SPU_DS, &fcArgs);
		} else {
			/* US */
			debug_info.slowpath_us++;
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

			skbuff_bcm_ext_iq_prio_set(skb, info.iq_prio);

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
    
				sp->xvec[sp->len++] = s_info.ptr;
#endif
				skb->dev = iproc_priv.spu_dev_ds;
				skb_shinfo(skb)->dirty_p = datap + info.size;
				if ((*datap & 0xF0) == 0x40)
					skb->protocol = htons(ETH_P_IP);
				else if ((*datap & 0xF0) == 0x60)
					skb->protocol = htons(ETH_P_IPV6);
				else
					pr_info("unknown pkt type 0x%x\n", *datap);

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
	uint8_t *pdata;
	uint32_t padlen, hdr_offset, fpi_tun_id, len;
	uint8_t nexthdr;
	int rc = 0;
	struct sk_buff *skb = NULL;
	FkBuff_t *fkb = NULL;
	struct spu_offload_prephdr_args prep = {};
	rdpa_cpu_tx_info_t info = {};
	uint8_t *tmp; /* tmp buffer holding MTU packet */

	if (IS_SKBUFF_PTR(pNBuf))
	{
		skb = (struct sk_buff *)PNBUFF_2_PBUF(pNBuf);
		pdata = skb->data;
		len = skb->len;

		if (skb_is_nonlinear(skb)) {
			if(skb_linearize(skb)) {
				debug_info.invalid_insert++;
				return -1;
			}
		}
	}
	else
	{
		fkb = (FkBuff_t *)PNBUFF_2_PBUF(pNBuf);
		pdata = fkb->data;
		len = fkb->len;
	}

	padlen = pdata[len - digestsize - 2];
	nexthdr = pdata[len - digestsize - 1];

	/* extract packet tos from outer header */
	spu_platform_offload_us_prepend(&prep, pdata, (nexthdr == BLOG_IPPROTO_IPV6));

	spu_offload_us_hdr_offset(session_id, &hdr_offset, &fpi_tun_id);
	len += (prep.prepend_size - (hdr_offset + padlen + BLOG_ESP_PADLEN_LEN + BLOG_ESP_NEXT_PROTO_LEN + digestsize));
	*payloadlen = len - prep.prepend_size;

	tmp = kmalloc(len, GFP_ATOMIC);
	if (tmp == NULL) {
		debug_info.invalid_insert++;
		return -1;
	}

	/* not to reduce buffer headroom, save the incoming packet to tmp buffer */
	memcpy(tmp, pdata + hdr_offset, len - prep.prepend_size);

	if (IS_SKBUFF_PTR(pNBuf)) {
		/* chop the ESP trailer */
		__skb_trim(skb, (skb->len - (padlen + BLOG_ESP_PADLEN_LEN + BLOG_ESP_NEXT_PROTO_LEN + digestsize)));
		/* push buffer down by outer headers to ensure no reduction in skb headroom (from enet) */
		__skb_grow(skb, skb->len + (hdr_offset - ETH_HLEN));
		/* push down 1 outer header to regain the lost skb headroom
		 * another outer header space is the reserve of the insertion done by offload FW */
		pdata = __skb_pull(skb, (2*hdr_offset - ETH_HLEN - prep.prepend_size));
		__skb_trim(skb, len);
	} else {
		pdata += (2*hdr_offset - prep.prepend_size - ETH_HLEN);
		fkb->len = len;
		fkb->data = pdata;
	}

	/* copy the prepend header */
	memcpy(pdata, &prep.prepend_word, prep.prepend_size);
	/* copy the packet data */
	memcpy(pdata + prep.prepend_size, tmp, len - prep.prepend_size);

	kfree(tmp);

	/* insert the packet for an offload session */
	info.method = rdpa_cpu_tx_egress;
	info.cpu_port = rdpa_cpu_spu;
	info.port_obj = spu_offload_inst.rdpa_port_priv;
	info.l3_packet = 1;
#if IS_ENABLED(CONFIG_BCM_FPI)
	info.crypto_session_id = fpi_tun_id;
#else
	info.crypto_session_id = session_id;
#endif
	info.flags = 0;
	
	rc = rdpa_cpu_send_sysb(pNBuf, &info);
	if (rc)
		pr_err("failed rdpa_cpu_send_sysb rc %d\n", rc);

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

int spu_platform_offload_ds_session(int session_id, struct xfrm_state *xfrm,
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

	if (xfrm->props.mode == XFRM_MODE_TRANSPORT) {
		type |= 1 << SPU_TRANSPORT_MODE_BIT;
		info.transport_mode = BLOG_ESP_MODE_TRANSPORT;
	}
	if (parm->is_esn) {
		type |= 1 << SPU_ESN_BIT;
		info.is_esn = 1;
	}
 	if (parm->is_gcm) {
		info.is_gcm = 1;
		type |= 1 << SPU_GCM_BIT;
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

	info.session_id = session_id;
	info.key_size = parm->key_size;
	info.digest_size = parm->digest_size;
	info.key_buf_dma_addr = session->key_buf_dma_addr;
	info.iv_size = parm->iv_size;
	rdpa_crypto_session_info_set(&info);

	/* create the prepend header XRDP FW needs */
	memcpy(session->prepend_hdr, parm->spu_header, FMD_SIZE);
	return 0;
}

void spu_platform_offload_us_session(int session_id, struct xfrm_state *xfrm,
					struct bcmspu_offload_parm *parm)
{
	rdpa_crypto_session_info_t info = {};
	uint32_t type = 0;
	struct spu_xrdp_session_info *session = &spu_xrdp_state_g.session_mem[session_id];
	uint8_t *data_p;

	session->taken = 1;
	/* clear the sequence number memory */
	memset(session->offload_seq, 0, sizeof(CRYPTO_SESSION_SEQ_INFO_STRUCT));

	/* for Runner platforms, the key buffers located in DDR separately */
	memcpy(session->key_bufp, &parm->spu_header[FMD_SIZE], parm->key_size);
	info.key_buf_dma_addr = session->key_buf_dma_addr;

	if (parm->is_esn) {
		info.is_esn = 1;
		type |= (1 << SPU_ESN_BIT);
		session->offload_seq->oseq_hi = cpu_to_be32(parm->seq_hi);
	}
	session->offload_seq->oseq_lo = cpu_to_be32(parm->seq_lo);

	if (xfrm->props.mode == XFRM_MODE_TRANSPORT) {
		info.transport_mode = BLOG_ESP_MODE_TRANSPORT;
	}

	session->offload_seq->block_size = cpu_to_be32((uint32_t)parm->blk_size);

	info.digest_size = parm->digest_size;
	info.session_id = session_id;
	info.key_size = parm->key_size;
	info.iv_size = parm->iv_size;
	info.is_gcm = parm->is_gcm;
	info.ipv6 = parm->ipv6;
	info.esp_o_udp = parm->esp_o_udp;
	info.ipv4_in_ipv6 = parm->ipv4_in_ipv6;

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
	session->offload_seq->type = cpu_to_be32(type);

	rdpa_crypto_session_info_set(&info);

	/* create the prepend header XRDP FW needs */
	data_p = session->prepend_hdr;
	memset(data_p, 0, SPU_PREPEND_STORAGE);
	memcpy(data_p, parm->spu_header, FMD_SIZE);

	data_p += FMD_SIZE;

	if (parm->is_gcm) {
		/* for GCM, IV not generated by HW, need to add as part of prepend header */
		memcpy(data_p, parm->gcm_iv_aad2, parm->iv_size);
		data_p += parm->iv_size;
		/* followed by assoc */
		memcpy(data_p, &parm->esp_spi, BLOG_ESP_SPI_LEN);
		data_p += (BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN);

		if (parm->is_esn) {
			/* GCM ESN case, seqhi is after the SPI, part of the assoc */
			data_p += BLOG_ESP_SEQNUM_HI_LEN;
			/* GCM ESN case, ESP header is not written by HW
			 * fill the SPI for the outgoing ESP header in prepend */
			memcpy(data_p + parm->outer_hdr_size, &parm->esp_spi, BLOG_ESP_SPI_LEN);
		}
	} else {
		/* for non GCM cases, seqhi is right after the keys, not part of assoc */
		if (parm->is_esn)
			data_p += BLOG_ESP_SEQNUM_HI_LEN;
		/* followed by assoc */
		memcpy(data_p, &parm->esp_spi, BLOG_ESP_SPI_LEN);
		data_p += (BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN);
	}
	/* outer IP header goes right after, note the outer IP header will not be part of SPU request */
	memcpy(data_p, &parm->outer_header, parm->outer_hdr_size);
}

void spu_platform_offload_us_prepend(struct spu_offload_prephdr_args *a, uint8_t *datap, uint8_t ipv6)
{
	uint32_t word32;
	a->prepend_size = 0;

	/* only the tos / ipv6 hdr word0 (tos + flow label) goes to the prepend header */
	if (ipv6) {
		/* extract word0 from outer header */
		if (datap)
			word32 = ((BlogIpv6Hdr_t *)datap)->u32[0];
		/* extract word0 from blog */
		else if (a->blog_p)
			word32 = a->blog_p->esp6tx_tuple_p->word0;
	} else {
		/* extract tos from outer header */
		if (datap)
			word32 = ((BlogIpv4Hdr_t *)datap)->tos;
		/* extract tos from blog */
		else if (a->blog_p)
			word32 = a->blog_p->esptx_tuple_p->tos;
		word32 = cpu_to_be32(word32);
	}
	a->prepend_word = word32;
	a->prepend_size = sizeof(uint32_t);
}

void spu_platform_offload_free_session(uint32_t session_id)
{
	struct spu_xrdp_session_info *session = &spu_xrdp_state_g.session_mem[session_id];

	if (!session->taken)
		flow_log("session %u is not taken\n", session_id);
	session->taken = 0;
}

void spu_platform_offloaded(struct spu_offload_debug_info *info_p)
{
	rdpa_spu_offload_stat_t offload_stat;
	rdpa_spu_offload_pkt(&offload_stat);

	debug_info.requests = offload_stat.num_request;
	debug_info.complete = offload_stat.num_completed;
	debug_info.icv_err = offload_stat.icv_error;
	debug_info.seqno_err = offload_stat.seqno_error;
        
	memcpy(info_p, &debug_info, sizeof(struct spu_offload_debug_info));
}

void spu_platform_offload_tun_map(uint32_t tun_id, int32_t session_id)
{
	rdpa_spu_offload_tunnel_mapping(tun_id, session_id);
}

void spu_platform_offload_copy_fwd(uint32_t session_id2, uint32_t session_id1)
{
	/* not applicable on XRDP */
}

void spu_platform_offload_register(void)
{
	int i, aligned_size;
	uint8_t *datap, *key_bufp, *prepend_p;
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

	statep->key_buf_base = dma_alloc_coherent(&iproc_priv.pdev->dev,
					MAX_OFFLOAD_KEY_SIZE * MAX_SPU_OFFLOAD_SESSIONS,
					&statep->key_buf_dma_base, GFP_KERNEL);
	key_bufp = statep->key_buf_base;
	key_bufp_dma = (uint64_t)statep->key_buf_dma_base;

	/* prepend headers */
	statep->prepend_base = dma_alloc_coherent(&iproc_priv.pdev->dev,
					SPU_PREPEND_STORAGE * MAX_SPU_OFFLOAD_SESSIONS,
					&statep->prepend_dma_base, GFP_KERNEL); 

	rdpa_spu_crypto_session_base_set((uint64_t)statep->ses_seq_dma_base, (uint64_t)statep->prepend_dma_base);

	prepend_p = statep->prepend_base;

	/* initializaing DS sessions */
	for (i=0; i < MAX_SPU_OFFLOAD_SESSIONS; i++) {
		session = &statep->session_mem[i];
		session->offload_seq = (CRYPTO_SESSION_SEQ_INFO_STRUCT *)datap;
		session->key_bufp = key_bufp;
		session->key_buf_dma_addr = key_bufp_dma;
		session->prepend_hdr = prepend_p;
		datap += aligned_size;
		key_bufp += MAX_OFFLOAD_KEY_SIZE;
		key_bufp_dma += MAX_OFFLOAD_KEY_SIZE;
		prepend_p += SPU_PREPEND_STORAGE;
	}
}

void spu_platform_offload_deregister(void)
{
	int aligned_size;
	struct spu_xrdp_offload_state *statep = &spu_xrdp_state_g;

	rdpa_spu_crypto_session_base_set((uint64_t)0, (uint64_t)0);

	aligned_size = BCM_DCACHE_ALIGN(sizeof(CRYPTO_SESSION_SEQ_INFO_STRUCT));
	dma_free_coherent(&iproc_priv.pdev->dev,
			  aligned_size * MAX_SPU_OFFLOAD_SESSIONS,
			  statep->ses_seq_base, statep->ses_seq_dma_base);

	dma_free_coherent(&iproc_priv.pdev->dev,
			  MAX_OFFLOAD_KEY_SIZE * MAX_SPU_OFFLOAD_SESSIONS,
			  statep->key_buf_base, statep->key_buf_dma_base);

	dma_free_coherent(&iproc_priv.pdev->dev,
			  SPU_PREPEND_STORAGE * MAX_SPU_OFFLOAD_SESSIONS,
			  statep->prepend_base, statep->prepend_dma_base);
			
}

