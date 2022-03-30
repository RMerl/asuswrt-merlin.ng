/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Multicore Navigator definitions
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef _KEYSTONE_NAV_H_
#define _KEYSTONE_NAV_H_

#include <asm/arch/hardware.h>
#include <asm/io.h>

#define QM_OK                    0
#define QM_ERR                  -1
#define QM_DESC_TYPE_HOST        0
#define QM_DESC_PSINFO_IN_DESCR  0
#define QM_DESC_DEFAULT_DESCINFO    (QM_DESC_TYPE_HOST << 30) | \
					(QM_DESC_PSINFO_IN_DESCR << 22)

/* Packet Info */
#define QM_DESC_PINFO_EPIB              1
#define QM_DESC_PINFO_RETURN_OWN        1
#define QM_DESC_DEFAULT_PINFO           (QM_DESC_PINFO_EPIB << 31) | \
					(QM_DESC_PINFO_RETURN_OWN << 15)

struct qm_cfg_reg {
	u32	revision;
	u32	__pad1;
	u32	divert;
	u32	link_ram_base0;
	u32	link_ram_size0;
	u32	link_ram_base1;
	u32	link_ram_size1;
	u32	link_ram_base2;
	u32	starvation[0];
};

struct	descr_mem_setup_reg {
	u32	base_addr;
	u32	start_idx;
	u32	desc_reg_size;
	u32	_res0;
};

struct qm_reg_queue {
	u32	entry_count;
	u32	byte_count;
	u32	packet_size;
	u32	ptr_size_thresh;
};

struct qm_config {
	/* QM module addresses */
	u32	stat_cfg;	/* status and config		*/
	struct qm_reg_queue *queue;	/* management region	*/
	u32	mngr_vbusm;	/* management region (VBUSM)	*/
	u32	i_lram;		/* internal linking RAM		*/
	struct qm_reg_queue *proxy;
	u32	status_ram;
	struct qm_cfg_reg *mngr_cfg;
				/* Queue manager config region	*/
	u32	intd_cfg;	/* QMSS INTD config region	*/
	struct	descr_mem_setup_reg *desc_mem;
				/* descritor memory setup region*/
	u32	region_num;
	u32	pdsp_cmd;	/* PDSP1 command interface	*/
	u32	pdsp_ctl;	/* PDSP1 control registers	*/
	u32	pdsp_iram;
	/* QM configuration parameters */

	u32	qpool_num;	/* */
};

struct qm_host_desc {
	u32 desc_info;
	u32 tag_info;
	u32 packet_info;
	u32 buff_len;
	u32 buff_ptr;
	u32 next_bdptr;
	u32 orig_buff_len;
	u32 orig_buff_ptr;
	u32 timestamp;
	u32 swinfo[3];
	u32 ps_data[20];
};

#define HDESC_NUM        256

int	qm_init(void);
void	qm_close(void);
void	qm_push(struct qm_host_desc *hd, u32 qnum);
struct qm_host_desc *qm_pop(u32 qnum);

void	qm_buff_push(struct qm_host_desc *hd, u32 qnum,
		     void *buff_ptr, u32 buff_len);

struct	qm_host_desc *qm_pop_from_free_pool(void);
void	queue_close(u32 qnum);

/*
 * DMA API
 */
#define CPDMA_REG_VAL_MAKE_RX_FLOW_A(einfo, psinfo, rxerr, desc, \
				     psloc, sopoff, qmgr, qnum) \
	(((einfo & 1) << 30)  | \
	 ((psinfo & 1) << 29) | \
	 ((rxerr & 1) << 28)  | \
	 ((desc & 3) << 26)   | \
	 ((psloc & 1) << 25)  | \
	 ((sopoff & 0x1ff) << 16) | \
	 ((qmgr & 3) << 12)   | \
	 ((qnum & 0xfff) << 0))

#define CPDMA_REG_VAL_MAKE_RX_FLOW_D(fd0qm, fd0qnum, fd1qm, fd1qnum) \
	(((fd0qm & 3) << 28)  | \
	 ((fd0qnum & 0xfff) << 16) | \
	 ((fd1qm & 3) << 12)  | \
	 ((fd1qnum & 0xfff) <<  0))

#define CPDMA_CHAN_A_ENABLE ((u32)1 << 31)
#define CPDMA_CHAN_A_TDOWN  (1 << 30)
#define TDOWN_TIMEOUT_COUNT  100

struct global_ctl_regs {
	u32	revision;
	u32	perf_control;
	u32	emulation_control;
	u32	priority_control;
	u32	qm_base_addr[4];
};

struct tx_chan_regs {
	u32	cfg_a;
	u32	cfg_b;
	u32	res[6];
};

struct rx_chan_regs {
	u32	cfg_a;
	u32	res[7];
};

struct rx_flow_regs {
	u32	control;
	u32	tags;
	u32	tag_sel;
	u32	fdq_sel[2];
	u32	thresh[3];
};

struct pktdma_cfg {
	struct global_ctl_regs	*global;
	struct tx_chan_regs	*tx_ch;
	u32			tx_ch_num;
	struct rx_chan_regs	*rx_ch;
	u32			rx_ch_num;
	u32			*tx_sched;
	struct rx_flow_regs	*rx_flows;
	u32			rx_flow_num;

	u32			rx_free_q;
	u32			rx_rcv_q;
	u32			tx_snd_q;

	u32			rx_flow; /* flow that is used for RX */
};

extern struct pktdma_cfg netcp_pktdma;

/*
 * packet dma user allocates memory for rx buffers
 * and describe it in the following structure
 */
struct rx_buff_desc {
	u8	*buff_ptr;
	u32	num_buffs;
	u32	buff_len;
	u32	rx_flow;
};

int ksnav_close(struct pktdma_cfg *pktdma);
int ksnav_init(struct pktdma_cfg *pktdma, struct rx_buff_desc *rx_buffers);
int ksnav_send(struct pktdma_cfg *pktdma, u32 *pkt, int num_bytes, u32 swinfo2);
void *ksnav_recv(struct pktdma_cfg *pktdma, u32 **pkt, int *num_bytes);
void ksnav_release_rxhd(struct pktdma_cfg *pktdma, void *hd);

#endif  /* _KEYSTONE_NAV_H_ */
