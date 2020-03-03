/*
 * QLogic Fibre Channel HBA Driver
 * Copyright (c)  2003-2014 QLogic Corporation
 *
 * See LICENSE.qla2xxx for copyright and licensing details.
 */
#include "qla_def.h"
#include "qla_target.h"

#include <linux/blkdev.h>
#include <linux/delay.h>

#include <scsi/scsi_tcq.h>

static void qla25xx_set_que(srb_t *, struct rsp_que **);
/**
 * qla2x00_get_cmd_direction() - Determine control_flag data direction.
 * @cmd: SCSI command
 *
 * Returns the proper CF_* direction based on CDB.
 */
static inline uint16_t
qla2x00_get_cmd_direction(srb_t *sp)
{
	uint16_t cflags;
	struct scsi_cmnd *cmd = GET_CMD_SP(sp);
	struct scsi_qla_host *vha = sp->fcport->vha;

	cflags = 0;

	/* Set transfer direction */
	if (cmd->sc_data_direction == DMA_TO_DEVICE) {
		cflags = CF_WRITE;
		vha->qla_stats.output_bytes += scsi_bufflen(cmd);
		vha->qla_stats.output_requests++;
	} else if (cmd->sc_data_direction == DMA_FROM_DEVICE) {
		cflags = CF_READ;
		vha->qla_stats.input_bytes += scsi_bufflen(cmd);
		vha->qla_stats.input_requests++;
	}
	return (cflags);
}

/**
 * qla2x00_calc_iocbs_32() - Determine number of Command Type 2 and
 * Continuation Type 0 IOCBs to allocate.
 *
 * @dsds: number of data segment decriptors needed
 *
 * Returns the number of IOCB entries needed to store @dsds.
 */
uint16_t
qla2x00_calc_iocbs_32(uint16_t dsds)
{
	uint16_t iocbs;

	iocbs = 1;
	if (dsds > 3) {
		iocbs += (dsds - 3) / 7;
		if ((dsds - 3) % 7)
			iocbs++;
	}
	return (iocbs);
}

/**
 * qla2x00_calc_iocbs_64() - Determine number of Command Type 3 and
 * Continuation Type 1 IOCBs to allocate.
 *
 * @dsds: number of data segment decriptors needed
 *
 * Returns the number of IOCB entries needed to store @dsds.
 */
uint16_t
qla2x00_calc_iocbs_64(uint16_t dsds)
{
	uint16_t iocbs;

	iocbs = 1;
	if (dsds > 2) {
		iocbs += (dsds - 2) / 5;
		if ((dsds - 2) % 5)
			iocbs++;
	}
	return (iocbs);
}

/**
 * qla2x00_prep_cont_type0_iocb() - Initialize a Continuation Type 0 IOCB.
 * @ha: HA context
 *
 * Returns a pointer to the Continuation Type 0 IOCB packet.
 */
static inline cont_entry_t *
qla2x00_prep_cont_type0_iocb(struct scsi_qla_host *vha)
{
	cont_entry_t *cont_pkt;
	struct req_que *req = vha->req;
	/* Adjust ring index. */
	req->ring_index++;
	if (req->ring_index == req->length) {
		req->ring_index = 0;
		req->ring_ptr = req->ring;
	} else {
		req->ring_ptr++;
	}

	cont_pkt = (cont_entry_t *)req->ring_ptr;

	/* Load packet defaults. */
	*((uint32_t *)(&cont_pkt->entry_type)) =
	    __constant_cpu_to_le32(CONTINUE_TYPE);

	return (cont_pkt);
}

/**
 * qla2x00_prep_cont_type1_iocb() - Initialize a Continuation Type 1 IOCB.
 * @ha: HA context
 *
 * Returns a pointer to the continuation type 1 IOCB packet.
 */
static inline cont_a64_entry_t *
qla2x00_prep_cont_type1_iocb(scsi_qla_host_t *vha, struct req_que *req)
{
	cont_a64_entry_t *cont_pkt;

	/* Adjust ring index. */
	req->ring_index++;
	if (req->ring_index == req->length) {
		req->ring_index = 0;
		req->ring_ptr = req->ring;
	} else {
		req->ring_ptr++;
	}

	cont_pkt = (cont_a64_entry_t *)req->ring_ptr;

	/* Load packet defaults. */
	*((uint32_t *)(&cont_pkt->entry_type)) = IS_QLAFX00(vha->hw) ?
	    __constant_cpu_to_le32(CONTINUE_A64_TYPE_FX00) :
	    __constant_cpu_to_le32(CONTINUE_A64_TYPE);

	return (cont_pkt);
}

static inline int
qla24xx_configure_prot_mode(srb_t *sp, uint16_t *fw_prot_opts)
{
	struct scsi_cmnd *cmd = GET_CMD_SP(sp);
	uint8_t	guard = scsi_host_get_guard(cmd->device->host);

	/* We always use DIFF Bundling for best performance */
	*fw_prot_opts = 0;

	/* Translate SCSI opcode to a protection opcode */
	switch (scsi_get_prot_op(cmd)) {
	case SCSI_PROT_READ_STRIP:
		*fw_prot_opts |= PO_MODE_DIF_REMOVE;
		break;
	case SCSI_PROT_WRITE_INSERT:
		*fw_prot_opts |= PO_MODE_DIF_INSERT;
		break;
	case SCSI_PROT_READ_INSERT:
		*fw_prot_opts |= PO_MODE_DIF_INSERT;
		break;
	case SCSI_PROT_WRITE_STRIP:
		*fw_prot_opts |= PO_MODE_DIF_REMOVE;
		break;
	case SCSI_PROT_READ_PASS:
	case SCSI_PROT_WRITE_PASS:
		if (guard & SHOST_DIX_GUARD_IP)
			*fw_prot_opts |= PO_MODE_DIF_TCP_CKSUM;
		else
			*fw_prot_opts |= PO_MODE_DIF_PASS;
		break;
	default:	/* Normal Request */
		*fw_prot_opts |= PO_MODE_DIF_PASS;
		break;
	}

	return scsi_prot_sg_count(cmd);
}

/*
 * qla2x00_build_scsi_iocbs_32() - Build IOCB command utilizing 32bit
 * capable IOCB types.
 *
 * @sp: SRB command to process
 * @cmd_pkt: Command type 2 IOCB
 * @tot_dsds: Total number of segments to transfer
 */
void qla2x00_build_scsi_iocbs_32(srb_t *sp, cmd_entry_t *cmd_pkt,
    uint16_t tot_dsds)
{
	uint16_t	avail_dsds;
	uint32_t	*cur_dsd;
	scsi_qla_host_t	*vha;
	struct scsi_cmnd *cmd;
	struct scatterlist *sg;
	int i;

	cmd = GET_CMD_SP(sp);

	/* Update entry type to indicate Command Type 2 IOCB */
	*((uint32_t *)(&cmd_pkt->entry_type)) =
	    __constant_cpu_to_le32(COMMAND_TYPE);

	/* No data transfer */
	if (!scsi_bufflen(cmd) || cmd->sc_data_direction == DMA_NONE) {
		cmd_pkt->byte_count = __constant_cpu_to_le32(0);
		return;
	}

	vha = sp->fcport->vha;
	cmd_pkt->control_flags |= cpu_to_le16(qla2x00_get_cmd_direction(sp));

	/* Three DSDs are available in the Command Type 2 IOCB */
	avail_dsds = 3;
	cur_dsd = (uint32_t *)&cmd_pkt->dseg_0_address;

	/* Load data segments */
	scsi_for_each_sg(cmd, sg, tot_dsds, i) {
		cont_entry_t *cont_pkt;

		/* Allocate additional continuation packets? */
		if (avail_dsds == 0) {
			/*
			 * Seven DSDs are available in the Continuation
			 * Type 0 IOCB.
			 */
			cont_pkt = qla2x00_prep_cont_type0_iocb(vha);
			cur_dsd = (uint32_t *)&cont_pkt->dseg_0_address;
			avail_dsds = 7;
		}

		*cur_dsd++ = cpu_to_le32(sg_dma_address(sg));
		*cur_dsd++ = cpu_to_le32(sg_dma_len(sg));
		avail_dsds--;
	}
}

/**
 * qla2x00_build_scsi_iocbs_64() - Build IOCB command utilizing 64bit
 * capable IOCB types.
 *
 * @sp: SRB command to process
 * @cmd_pkt: Command type 3 IOCB
 * @tot_dsds: Total number of segments to transfer
 */
void qla2x00_build_scsi_iocbs_64(srb_t *sp, cmd_entry_t *cmd_pkt,
    uint16_t tot_dsds)
{
	uint16_t	avail_dsds;
	uint32_t	*cur_dsd;
	scsi_qla_host_t	*vha;
	struct scsi_cmnd *cmd;
	struct scatterlist *sg;
	int i;

	cmd = GET_CMD_SP(sp);

	/* Update entry type to indicate Command Type 3 IOCB */
	*((uint32_t *)(&cmd_pkt->entry_type)) =
	    __constant_cpu_to_le32(COMMAND_A64_TYPE);

	/* No data transfer */
	if (!scsi_bufflen(cmd) || cmd->sc_data_direction == DMA_NONE) {
		cmd_pkt->byte_count = __constant_cpu_to_le32(0);
		return;
	}

	vha = sp->fcport->vha;
	cmd_pkt->control_flags |= cpu_to_le16(qla2x00_get_cmd_direction(sp));

	/* Two DSDs are available in the Command Type 3 IOCB */
	avail_dsds = 2;
	cur_dsd = (uint32_t *)&cmd_pkt->dseg_0_address;

	/* Load data segments */
	scsi_for_each_sg(cmd, sg, tot_dsds, i) {
		dma_addr_t	sle_dma;
		cont_a64_entry_t *cont_pkt;

		/* Allocate additional continuation packets? */
		if (avail_dsds == 0) {
			/*
			 * Five DSDs are available in the Continuation
			 * Type 1 IOCB.
			 */
			cont_pkt = qla2x00_prep_cont_type1_iocb(vha, vha->req);
			cur_dsd = (uint32_t *)cont_pkt->dseg_0_address;
			avail_dsds = 5;
		}

		sle_dma = sg_dma_address(sg);
		*cur_dsd++ = cpu_to_le32(LSD(sle_dma));
		*cur_dsd++ = cpu_to_le32(MSD(sle_dma));
		*cur_dsd++ = cpu_to_le32(sg_dma_len(sg));
		avail_dsds--;
	}
}

/**
 * qla2x00_start_scsi() - Send a SCSI command to the ISP
 * @sp: command to send to the ISP
 *
 * Returns non-zero if a failure occurred, else zero.
 */
int
qla2x00_start_scsi(srb_t *sp)
{
	int		ret, nseg;
	unsigned long   flags;
	scsi_qla_host_t	*vha;
	struct scsi_cmnd *cmd;
	uint32_t	*clr_ptr;
	uint32_t        index;
	uint32_t	handle;
	cmd_entry_t	*cmd_pkt;
	uint16_t	cnt;
	uint16_t	req_cnt;
	uint16_t	tot_dsds;
	struct device_reg_2xxx __iomem *reg;
	struct qla_hw_data *ha;
	struct req_que *req;
	struct rsp_que *rsp;

	/* Setup device pointers. */
	ret = 0;
	vha = sp->fcport->vha;
	ha = vha->hw;
	reg = &ha->iobase->isp;
	cmd = GET_CMD_SP(sp);
	req = ha->req_q_map[0];
	rsp = ha->rsp_q_map[0];
	/* So we know we haven't pci_map'ed anything yet */
	tot_dsds = 0;

	/* Send marker if required */
	if (vha->marker_needed != 0) {
		if (qla2x00_marker(vha, req, rsp, 0, 0, MK_SYNC_ALL) !=
		    QLA_SUCCESS) {
			return (QLA_FUNCTION_FAILED);
		}
		vha->marker_needed = 0;
	}

	/* Acquire ring specific lock */
	spin_lock_irqsave(&ha->hardware_lock, flags);

	/* Check for room in outstanding command list. */
	handle = req->current_outstanding_cmd;
	for (index = 1; index < req->num_outstanding_cmds; index++) {
		handle++;
		if (handle == req->num_outstanding_cmds)
			handle = 1;
		if (!req->outstanding_cmds[handle])
			break;
	}
	if (index == req->num_outstanding_cmds)
		goto queuing_error;

	/* Map the sg table so we have an accurate count of sg entries needed */
	if (scsi_sg_count(cmd)) {
		nseg = dma_map_sg(&ha->pdev->dev, scsi_sglist(cmd),
		    scsi_sg_count(cmd), cmd->sc_data_direction);
		if (unlikely(!nseg))
			goto queuing_error;
	} else
		nseg = 0;

	tot_dsds = nseg;

	/* Calculate the number of request entries needed. */
	req_cnt = ha->isp_ops->calc_req_entries(tot_dsds);
	if (req->cnt < (req_cnt + 2)) {
		cnt = RD_REG_WORD_RELAXED(ISP_REQ_Q_OUT(ha, reg));
		if (req->ring_index < cnt)
			req->cnt = cnt - req->ring_index;
		else
			req->cnt = req->length -
			    (req->ring_index - cnt);
		/* If still no head room then bail out */
		if (req->cnt < (req_cnt + 2))
			goto queuing_error;
	}

	/* Build command packet */
	req->current_outstanding_cmd = handle;
	req->outstanding_cmds[handle] = sp;
	sp->handle = handle;
	cmd->host_scribble = (unsigned char *)(unsigned long)handle;
	req->cnt -= req_cnt;

	cmd_pkt = (cmd_entry_t *)req->ring_ptr;
	cmd_pkt->handle = handle;
	/* Zero out remaining portion of packet. */
	clr_ptr = (uint32_t *)cmd_pkt + 2;
	memset(clr_ptr, 0, REQUEST_ENTRY_SIZE - 8);
	cmd_pkt->dseg_count = cpu_to_le16(tot_dsds);

	/* Set target ID and LUN number*/
	SET_TARGET_ID(ha, cmd_pkt->target, sp->fcport->loop_id);
	cmd_pkt->lun = cpu_to_le16(cmd->device->lun);
	cmd_pkt->control_flags = __constant_cpu_to_le16(CF_SIMPLE_TAG);

	/* Load SCSI command packet. */
	memcpy(cmd_pkt->scsi_cdb, cmd->cmnd, cmd->cmd_len);
	cmd_pkt->byte_count = cpu_to_le32((uint32_t)scsi_bufflen(cmd));

	/* Build IOCB segments */
	ha->isp_ops->build_iocbs(sp, cmd_pkt, tot_dsds);

	/* Set total data segment count. */
	cmd_pkt->entry_count = (uint8_t)req_cnt;
	wmb();

	/* Adjust ring index. */
	req->ring_index++;
	if (req->ring_index == req->length) {
		req->ring_index = 0;
		req->ring_ptr = req->ring;
	} else
		req->ring_ptr++;

	sp->flags |= SRB_DMA_VALID;

	/* Set chip new ring index. */
	WRT_REG_WORD(ISP_REQ_Q_IN(ha, reg), req->ring_index);
	RD_REG_WORD_RELAXED(ISP_REQ_Q_IN(ha, reg));	/* PCI Posting. */

	/* Manage unprocessed RIO/ZIO commands in response queue. */
	if (vha->flags.process_response_queue &&
	    rsp->ring_ptr->signature != RESPONSE_PROCESSED)
		qla2x00_process_response_queue(rsp);

	spin_unlock_irqrestore(&ha->hardware_lock, flags);
	return (QLA_SUCCESS);

queuing_error:
	if (tot_dsds)
		scsi_dma_unmap(cmd);

	spin_unlock_irqrestore(&ha->hardware_lock, flags);

	return (QLA_FUNCTION_FAILED);
}

/**
 * qla2x00_start_iocbs() - Execute the IOCB command
 */
void
qla2x00_start_iocbs(struct scsi_qla_host *vha, struct req_que *req)
{
	struct qla_hw_data *ha = vha->hw;
	device_reg_t __iomem *reg = ISP_QUE_REG(ha, req->id);

	if (IS_P3P_TYPE(ha)) {
		qla82xx_start_iocbs(vha);
	} else {
		/* Adjust ring index. */
		req->ring_index++;
		if (req->ring_index == req->length) {
			req->ring_index = 0;
			req->ring_ptr = req->ring;
		} else
			req->ring_ptr++;

		/* Set chip new ring index. */
		if (ha->mqenable || IS_QLA83XX(ha) || IS_QLA27XX(ha)) {
			WRT_REG_DWORD(req->req_q_in, req->ring_index);
			RD_REG_DWORD_RELAXED(&ha->iobase->isp24.hccr);
		} else if (IS_QLAFX00(ha)) {
			WRT_REG_DWORD(&reg->ispfx00.req_q_in, req->ring_index);
			RD_REG_DWORD_RELAXED(&reg->ispfx00.req_q_in);
			QLAFX00_SET_HST_INTR(ha, ha->rqstq_intr_code);
		} else if (IS_FWI2_CAPABLE(ha)) {
			WRT_REG_DWORD(&reg->isp24.req_q_in, req->ring_index);
			RD_REG_DWORD_RELAXED(&reg->isp24.req_q_in);
		} else {
			WRT_REG_WORD(ISP_REQ_Q_IN(ha, &reg->isp),
				req->ring_index);
			RD_REG_WORD_RELAXED(ISP_REQ_Q_IN(ha, &reg->isp));
		}
	}
}

/**
 * qla2x00_marker() - Send a marker IOCB to the firmware.
 * @ha: HA context
 * @loop_id: loop ID
 * @lun: LUN
 * @type: marker modifier
 *
 * Can be called from both normal and interrupt context.
 *
 * Returns non-zero if a failure occurred, else zero.
 */
static int
__qla2x00_marker(struct scsi_qla_host *vha, struct req_que *req,
			struct rsp_que *rsp, uint16_t loop_id,
			uint64_t lun, uint8_t type)
{
	mrk_entry_t *mrk;
	struct mrk_entry_24xx *mrk24 = NULL;

	struct qla_hw_data *ha = vha->hw;
	scsi_qla_host_t *base_vha = pci_get_drvdata(ha->pdev);

	req = ha->req_q_map[0];
	mrk = (mrk_entry_t *)qla2x00_alloc_iocbs(vha, NULL);
	if (mrk == NULL) {
		ql_log(ql_log_warn, base_vha, 0x3026,
		    "Failed to allocate Marker IOCB.\n");

		return (QLA_FUNCTION_FAILED);
	}

	mrk->entry_type = MARKER_TYPE;
	mrk->modifier = type;
	if (type != MK_SYNC_ALL) {
		if (IS_FWI2_CAPABLE(ha)) {
			mrk24 = (struct mrk_entry_24xx *) mrk;
			mrk24->nport_handle = cpu_to_le16(loop_id);
			int_to_scsilun(lun, (struct scsi_lun *)&mrk24->lun);
			host_to_fcp_swap(mrk24->lun, sizeof(mrk24->lun));
			mrk24->vp_index = vha->vp_idx;
			mrk24->handle = MAKE_HANDLE(req->id, mrk24->handle);
		} else {
			SET_TARGET_ID(ha, mrk->target, loop_id);
			mrk->lun = cpu_to_le16((uint16_t)lun);
		}
	}
	wmb();

	qla2x00_start_iocbs(vha, req);

	return (QLA_SUCCESS);
}

int
qla2x00_marker(struct scsi_qla_host *vha, struct req_que *req,
		struct rsp_que *rsp, uint16_t loop_id, uint64_t lun,
		uint8_t type)
{
	int ret;
	unsigned long flags = 0;

	spin_lock_irqsave(&vha->hw->hardware_lock, flags);
	ret = __qla2x00_marker(vha, req, rsp, loop_id, lun, type);
	spin_unlock_irqrestore(&vha->hw->hardware_lock, flags);

	return (ret);
}

/*
 * qla2x00_issue_marker
 *
 * Issue marker
 * Caller CAN have hardware lock held as specified by ha_locked parameter.
 * Might release it, then reaquire.
 */
int qla2x00_issue_marker(scsi_qla_host_t *vha, int ha_locked)
{
	if (ha_locked) {
		if (__qla2x00_marker(vha, vha->req, vha->req->rsp, 0, 0,
					MK_SYNC_ALL) != QLA_SUCCESS)
			return QLA_FUNCTION_FAILED;
	} else {
		if (qla2x00_marker(vha, vha->req, vha->req->rsp, 0, 0,
					MK_SYNC_ALL) != QLA_SUCCESS)
			return QLA_FUNCTION_FAILED;
	}
	vha->marker_needed = 0;

	return QLA_SUCCESS;
}

static inline int
qla24xx_build_scsi_type_6_iocbs(srb_t *sp, struct cmd_type_6 *cmd_pkt,
	uint16_t tot_dsds)
{
	uint32_t *cur_dsd = NULL;
	scsi_qla_host_t	*vha;
	struct qla_hw_data *ha;
	struct scsi_cmnd *cmd;
	struct	scatterlist *cur_seg;
	uint32_t *dsd_seg;
	void *next_dsd;
	uint8_t avail_dsds;
	uint8_t first_iocb = 1;
	uint32_t dsd_list_len;
	struct dsd_dma *dsd_ptr;
	struct ct6_dsd *ctx;

	cmd = GET_CMD_SP(sp);

	/* Update entry type to indicate Command Type 3 IOCB */
	*((uint32_t *)(&cmd_pkt->entry_type)) =
		__constant_cpu_to_le32(COMMAND_TYPE_6);

	/* No data transfer */
	if (!scsi_bufflen(cmd) || cmd->sc_data_direction == DMA_NONE) {
		cmd_pkt->byte_count = __constant_cpu_to_le32(0);
		return 0;
	}

	vha = sp->fcport->vha;
	ha = vha->hw;

	/* Set transfer direction */
	if (cmd->sc_data_direction == DMA_TO_DEVICE) {
		cmd_pkt->control_flags =
		    __constant_cpu_to_le16(CF_WRITE_DATA);
		vha->qla_stats.output_bytes += scsi_bufflen(cmd);
		vha->qla_stats.output_requests++;
	} else if (cmd->sc_data_direction == DMA_FROM_DEVICE) {
		cmd_pkt->control_flags =
		    __constant_cpu_to_le16(CF_READ_DATA);
		vha->qla_stats.input_bytes += scsi_bufflen(cmd);
		vha->qla_stats.input_requests++;
	}

	cur_seg = scsi_sglist(cmd);
	ctx = GET_CMD_CTX_SP(sp);

	while (tot_dsds) {
		avail_dsds = (tot_dsds > QLA_DSDS_PER_IOCB) ?
		    QLA_DSDS_PER_IOCB : tot_dsds;
		tot_dsds -= avail_dsds;
		dsd_list_len = (avail_dsds + 1) * QLA_DSD_SIZE;

		dsd_ptr = list_first_entry(&ha->gbl_dsd_list,
		    struct dsd_dma, list);
		next_dsd = dsd_ptr->dsd_addr;
		list_del(&dsd_ptr->list);
		ha->gbl_dsd_avail--;
		list_add_tail(&dsd_ptr->list, &ctx->dsd_list);
		ctx->dsd_use_cnt++;
		ha->gbl_dsd_inuse++;

		if (first_iocb) {
			first_iocb = 0;
			dsd_seg = (uint32_t *)&cmd_pkt->fcp_data_dseg_address;
			*dsd_seg++ = cpu_to_le32(LSD(dsd_ptr->dsd_list_dma));
			*dsd_seg++ = cpu_to_le32(MSD(dsd_ptr->dsd_list_dma));
			cmd_pkt->fcp_data_dseg_len = cpu_to_le32(dsd_list_len);
		} else {
			*cur_dsd++ = cpu_to_le32(LSD(dsd_ptr->dsd_list_dma));
			*cur_dsd++ = cpu_to_le32(MSD(dsd_ptr->dsd_list_dma));
			*cur_dsd++ = cpu_to_le32(dsd_list_len);
		}
		cur_dsd = (uint32_t *)next_dsd;
		while (avail_dsds) {
			dma_addr_t	sle_dma;

			sle_dma = sg_dma_address(cur_seg);
			*cur_dsd++ = cpu_to_le32(LSD(sle_dma));
			*cur_dsd++ = cpu_to_le32(MSD(sle_dma));
			*cur_dsd++ = cpu_to_le32(sg_dma_len(cur_seg));
			cur_seg = sg_next(cur_seg);
			avail_dsds--;
		}
	}

	/* Null termination */
	*cur_dsd++ =  0;
	*cur_dsd++ = 0;
	*cur_dsd++ = 0;
	cmd_pkt->control_flags |= CF_DATA_SEG_DESCR_ENABLE;
	return 0;
}

/*
 * qla24xx_calc_dsd_lists() - Determine number of DSD list required
 * for Command Type 6.
 *
 * @dsds: number of data segment decriptors needed
 *
 * Returns the number of dsd list needed to store @dsds.
 */
inline uint16_t
qla24xx_calc_dsd_lists(uint16_t dsds)
{
	uint16_t dsd_lists = 0;

	dsd_lists = (dsds/QLA_DSDS_PER_IOCB);
	if (dsds % QLA_DSDS_PER_IOCB)
		dsd_lists++;
	return dsd_lists;
}


/**
 * qla24xx_build_scsi_iocbs() - Build IOCB command utilizing Command Type 7
 * IOCB types.
 *
 * @sp: SRB command to process
 * @cmd_pkt: Command type 3 IOCB
 * @tot_dsds: Total number of segments to transfer
 */
inline void
qla24xx_build_scsi_iocbs(srb_t *sp, struct cmd_type_7 *cmd_pkt,
    uint16_t tot_dsds)
{
	uint16_t	avail_dsds;
	uint32_t	*cur_dsd;
	scsi_qla_host_t	*vha;
	struct scsi_cmnd *cmd;
	struct scatterlist *sg;
	int i;
	struct req_que *req;

	cmd = GET_CMD_SP(sp);

	/* Update entry type to indicate Command Type 3 IOCB */
	*((uint32_t *)(&cmd_pkt->entry_type)) =
	    __constant_cpu_to_le32(COMMAND_TYPE_7);

	/* No data transfer */
	if (!scsi_bufflen(cmd) || cmd->sc_data_direction == DMA_NONE) {
		cmd_pkt->byte_count = __constant_cpu_to_le32(0);
		return;
	}

	vha = sp->fcport->vha;
	req = vha->req;

	/* Set transfer direction */
	if (cmd->sc_data_direction == DMA_TO_DEVICE) {
		cmd_pkt->task_mgmt_flags =
		    __constant_cpu_to_le16(TMF_WRITE_DATA);
		vha->qla_stats.output_bytes += scsi_bufflen(cmd);
		vha->qla_stats.output_requests++;
	} else if (cmd->sc_data_direction == DMA_FROM_DEVICE) {
		cmd_pkt->task_mgmt_flags =
		    __constant_cpu_to_le16(TMF_READ_DATA);
		vha->qla_stats.input_bytes += scsi_bufflen(cmd);
		vha->qla_stats.input_requests++;
	}

	/* One DSD is available in the Command Type 3 IOCB */
	avail_dsds = 1;
	cur_dsd = (uint32_t *)&cmd_pkt->dseg_0_address;

	/* Load data segments */

	scsi_for_each_sg(cmd, sg, tot_dsds, i) {
		dma_addr_t	sle_dma;
		cont_a64_entry_t *cont_pkt;

		/* Allocate additional continuation packets? */
		if (avail_dsds == 0) {
			/*
			 * Five DSDs are available in the Continuation
			 * Type 1 IOCB.
			 */
			cont_pkt = qla2x00_prep_cont_type1_iocb(vha, vha->req);
			cur_dsd = (uint32_t *)cont_pkt->dseg_0_address;
			avail_dsds = 5;
		}

		sle_dma = sg_dma_address(sg);
		*cur_dsd++ = cpu_to_le32(LSD(sle_dma));
		*cur_dsd++ = cpu_to_le32(MSD(sle_dma));
		*cur_dsd++ = cpu_to_le32(sg_dma_len(sg));
		avail_dsds--;
	}
}

struct fw_dif_context {
	uint32_t ref_tag;
	uint16_t app_tag;
	uint8_t ref_tag_mask[4];	/* Validation/Replacement Mask*/
	uint8_t app_tag_mask[2];	/* Validation/Replacement Mask*/
};

/*
 * qla24xx_set_t10dif_tags_from_cmd - Extract Ref and App tags from SCSI command
 *
 */
static inline void
qla24xx_set_t10dif_tags(srb_t *sp, struct fw_dif_context *pkt,
    unsigned int protcnt)
{
	struct scsi_cmnd *cmd = GET_CMD_SP(sp);

	switch (scsi_get_prot_type(cmd)) {
	case SCSI_PROT_DIF_TYPE0:
		/*
		 * No check for ql2xenablehba_err_chk, as it would be an
		 * I/O error if hba tag generation is not done.
		 */
		pkt->ref_tag = cpu_to_le32((uint32_t)
		    (0xffffffff & scsi_get_lba(cmd)));

		if (!qla2x00_hba_err_chk_enabled(sp))
			break;

		pkt->ref_tag_mask[0] = 0xff;
		pkt->ref_tag_mask[1] = 0xff;
		pkt->ref_tag_mask[2] = 0xff;
		pkt->ref_tag_mask[3] = 0xff;
		break;

	/*
	 * For TYPE 2 protection: 16 bit GUARD + 32 bit REF tag has to
	 * match LBA in CDB + N
	 */
	case SCSI_PROT_DIF_TYPE2:
		pkt->app_tag = __constant_cpu_to_le16(0);
		pkt->app_tag_mask[0] = 0x0;
		pkt->app_tag_mask[1] = 0x0;

		pkt->ref_tag = cpu_to_le32((uint32_t)
		    (0xffffffff & scsi_get_lba(cmd)));

		if (!qla2x00_hba_err_chk_enabled(sp))
			break;

		/* enable ALL bytes of the ref tag */
		pkt->ref_tag_mask[0] = 0xff;
		pkt->ref_tag_mask[1] = 0xff;
		pkt->ref_tag_mask[2] = 0xff;
		pkt->ref_tag_mask[3] = 0xff;
		break;

	/* For Type 3 protection: 16 bit GUARD only */
	case SCSI_PROT_DIF_TYPE3:
		pkt->ref_tag_mask[0] = pkt->ref_tag_mask[1] =
			pkt->ref_tag_mask[2] = pkt->ref_tag_mask[3] =
								0x00;
		break;

	/*
	 * For TYpe 1 protection: 16 bit GUARD tag, 32 bit REF tag, and
	 * 16 bit app tag.
	 */
	case SCSI_PROT_DIF_TYPE1:
		pkt->ref_tag = cpu_to_le32((uint32_t)
		    (0xffffffff & scsi_get_lba(cmd)));
		pkt->app_tag = __constant_cpu_to_le16(0);
		pkt->app_tag_mask[0] = 0x0;
		pkt->app_tag_mask[1] = 0x0;

		if (!qla2x00_hba_err_chk_enabled(sp))
			break;

		/* enable ALL bytes of the ref tag */
		pkt->ref_tag_mask[0] = 0xff;
		pkt->ref_tag_mask[1] = 0xff;
		pkt->ref_tag_mask[2] = 0xff;
		pkt->ref_tag_mask[3] = 0xff;
		break;
	}
}

struct qla2_sgx {
	dma_addr_t		dma_addr;	/* OUT */
	uint32_t		dma_len;	/* OUT */

	uint32_t		tot_bytes;	/* IN */
	struct scatterlist	*cur_sg;	/* IN */

	/* for book keeping, bzero on initial invocation */
	uint32_t		bytes_consumed;
	uint32_t		num_bytes;
	uint32_t		tot_partial;

	/* for debugging */
	uint32_t		num_sg;
	srb_t			*sp;
};

static int
qla24xx_get_one_block_sg(uint32_t blk_sz, struct qla2_sgx *sgx,
	uint32_t *partial)
{
	struct scatterlist *sg;
	uint32_t cumulative_partial, sg_len;
	dma_addr_t sg_dma_addr;

	if (sgx->num_bytes == sgx->tot_bytes)
		return 0;

	sg = sgx->cur_sg;
	cumulative_partial = sgx->tot_partial;

	sg_dma_addr = sg_dma_address(sg);
	sg_len = sg_dma_len(sg);

	sgx->dma_addr = sg_dma_addr + sgx->bytes_consumed;

	if ((cumulative_partial + (sg_len - sgx->bytes_consumed)) >= blk_sz) {
		sgx->dma_len = (blk_sz - cumulative_partial);
		sgx->tot_partial = 0;
		sgx->num_bytes += blk_sz;
		*partial = 0;
	} else {
		sgx->dma_len = sg_len - sgx->bytes_consumed;
		sgx->tot_partial += sgx->dma_len;
		*partial = 1;
	}

	sgx->bytes_consumed += sgx->dma_len;

	if (sg_len == sgx->bytes_consumed) {
		sg = sg_next(sg);
		sgx->num_sg++;
		sgx->cur_sg = sg;
		sgx->bytes_consumed = 0;
	}

	return 1;
}

int
qla24xx_walk_and_build_sglist_no_difb(struct qla_hw_data *ha, srb_t *sp,
	uint32_t *dsd, uint16_t tot_dsds, struct qla_tgt_cmd *tc)
{
	void *next_dsd;
	uint8_t avail_dsds = 0;
	uint32_t dsd_list_len;
	struct dsd_dma *dsd_ptr;
	struct scatterlist *sg_prot;
	uint32_t *cur_dsd = dsd;
	uint16_t	used_dsds = tot_dsds;

	uint32_t	prot_int; /* protection interval */
	uint32_t	partial;
	struct qla2_sgx sgx;
	dma_addr_t	sle_dma;
	uint32_t	sle_dma_len, tot_prot_dma_len = 0;
	struct scsi_cmnd *cmd;
	struct scsi_qla_host *vha;

	memset(&sgx, 0, sizeof(struct qla2_sgx));
	if (sp) {
		vha = sp->fcport->vha;
		cmd = GET_CMD_SP(sp);
		prot_int = cmd->device->sector_size;

		sgx.tot_bytes = scsi_bufflen(cmd);
		sgx.cur_sg = scsi_sglist(cmd);
		sgx.sp = sp;

		sg_prot = scsi_prot_sglist(cmd);
	} else if (tc) {
		vha = tc->vha;
		prot_int      = tc->blk_sz;
		sgx.tot_bytes = tc->bufflen;
		sgx.cur_sg    = tc->sg;
		sg_prot	      = tc->prot_sg;
	} else {
		BUG();
		return 1;
	}

	while (qla24xx_get_one_block_sg(prot_int, &sgx, &partial)) {

		sle_dma = sgx.dma_addr;
		sle_dma_len = sgx.dma_len;
alloc_and_fill:
		/* Allocate additional continuation packets? */
		if (avail_dsds == 0) {
			avail_dsds = (used_dsds > QLA_DSDS_PER_IOCB) ?
					QLA_DSDS_PER_IOCB : used_dsds;
			dsd_list_len = (avail_dsds + 1) * 12;
			used_dsds -= avail_dsds;

			/* allocate tracking DS */
			dsd_ptr = kzalloc(sizeof(struct dsd_dma), GFP_ATOMIC);
			if (!dsd_ptr)
				return 1;

			/* allocate new list */
			dsd_ptr->dsd_addr = next_dsd =
			    dma_pool_alloc(ha->dl_dma_pool, GFP_ATOMIC,
				&dsd_ptr->dsd_list_dma);

			if (!next_dsd) {
				/*
				 * Need to cleanup only this dsd_ptr, rest
				 * will be done by sp_free_dma()
				 */
				kfree(dsd_ptr);
				return 1;
			}

			if (sp) {
				list_add_tail(&dsd_ptr->list,
				    &((struct crc_context *)
					    sp->u.scmd.ctx)->dsd_list);

				sp->flags |= SRB_CRC_CTX_DSD_VALID;
			} else {
				list_add_tail(&dsd_ptr->list,
				    &(tc->ctx->dsd_list));
				tc->ctx_dsd_alloced = 1;
			}


			/* add new list to cmd iocb or last list */
			*cur_dsd++ = cpu_to_le32(LSD(dsd_ptr->dsd_list_dma));
			*cur_dsd++ = cpu_to_le32(MSD(dsd_ptr->dsd_list_dma));
			*cur_dsd++ = dsd_list_len;
			cur_dsd = (uint32_t *)next_dsd;
		}
		*cur_dsd++ = cpu_to_le32(LSD(sle_dma));
		*cur_dsd++ = cpu_to_le32(MSD(sle_dma));
		*cur_dsd++ = cpu_to_le32(sle_dma_len);
		avail_dsds--;

		if (partial == 0) {
			/* Got a full protection interval */
			sle_dma = sg_dma_address(sg_prot) + tot_prot_dma_len;
			sle_dma_len = 8;

			tot_prot_dma_len += sle_dma_len;
			if (tot_prot_dma_len == sg_dma_len(sg_prot)) {
				tot_prot_dma_len = 0;
				sg_prot = sg_next(sg_prot);
			}

			partial = 1; /* So as to not re-enter this block */
			goto alloc_and_fill;
		}
	}
	/* Null termination */
	*cur_dsd++ = 0;
	*cur_dsd++ = 0;
	*cur_dsd++ = 0;
	return 0;
}

int
qla24xx_walk_and_build_sglist(struct qla_hw_data *ha, srb_t *sp, uint32_t *dsd,
	uint16_t tot_dsds, struct qla_tgt_cmd *tc)
{
	void *next_dsd;
	uint8_t avail_dsds = 0;
	uint32_t dsd_list_len;
	struct dsd_dma *dsd_ptr;
	struct scatterlist *sg, *sgl;
	uint32_t *cur_dsd = dsd;
	int	i;
	uint16_t	used_dsds = tot_dsds;
	struct scsi_cmnd *cmd;
	struct scsi_qla_host *vha;

	if (sp) {
		cmd = GET_CMD_SP(sp);
		sgl = scsi_sglist(cmd);
		vha = sp->fcport->vha;
	} else if (tc) {
		sgl = tc->sg;
		vha = tc->vha;
	} else {
		BUG();
		return 1;
	}


	for_each_sg(sgl, sg, tot_dsds, i) {
		dma_addr_t	sle_dma;

		/* Allocate additional continuation packets? */
		if (avail_dsds == 0) {
			avail_dsds = (used_dsds > QLA_DSDS_PER_IOCB) ?
					QLA_DSDS_PER_IOCB : used_dsds;
			dsd_list_len = (avail_dsds + 1) * 12;
			used_dsds -= avail_dsds;

			/* allocate tracking DS */
			dsd_ptr = kzalloc(sizeof(struct dsd_dma), GFP_ATOMIC);
			if (!dsd_ptr)
				return 1;

			/* allocate new list */
			dsd_ptr->dsd_addr = next_dsd =
			    dma_pool_alloc(ha->dl_dma_pool, GFP_ATOMIC,
				&dsd_ptr->dsd_list_dma);

			if (!next_dsd) {
				/*
				 * Need to cleanup only this dsd_ptr, rest
				 * will be done by sp_free_dma()
				 */
				kfree(dsd_ptr);
				return 1;
			}

			if (sp) {
				list_add_tail(&dsd_ptr->list,
				    &((struct crc_context *)
					    sp->u.scmd.ctx)->dsd_list);

				sp->flags |= SRB_CRC_CTX_DSD_VALID;
			} else {
				list_add_tail(&dsd_ptr->list,
				    &(tc->ctx->dsd_list));
				tc->ctx_dsd_alloced = 1;
			}

			/* add new list to cmd iocb or last list */
			*cur_dsd++ = cpu_to_le32(LSD(dsd_ptr->dsd_list_dma));
			*cur_dsd++ = cpu_to_le32(MSD(dsd_ptr->dsd_list_dma));
			*cur_dsd++ = dsd_list_len;
			cur_dsd = (uint32_t *)next_dsd;
		}
		sle_dma = sg_dma_address(sg);

		*cur_dsd++ = cpu_to_le32(LSD(sle_dma));
		*cur_dsd++ = cpu_to_le32(MSD(sle_dma));
		*cur_dsd++ = cpu_to_le32(sg_dma_len(sg));
		avail_dsds--;

	}
	/* Null termination */
	*cur_dsd++ = 0;
	*cur_dsd++ = 0;
	*cur_dsd++ = 0;
	return 0;
}

int
qla24xx_walk_and_build_prot_sglist(struct qla_hw_data *ha, srb_t *sp,
	uint32_t *dsd, uint16_t tot_dsds, struct qla_tgt_cmd *tc)
{
	void *next_dsd;
	uint8_t avail_dsds = 0;
	uint32_t dsd_list_len;
	struct dsd_dma *dsd_ptr;
	struct scatterlist *sg, *sgl;
	int	i;
	struct scsi_cmnd *cmd;
	uint32_t *cur_dsd = dsd;
	uint16_t used_dsds = tot_dsds;
	struct scsi_qla_host *vha;

	if (sp) {
		cmd = GET_CMD_SP(sp);
		sgl = scsi_prot_sglist(cmd);
		vha = sp->fcport->vha;
	} else if (tc) {
		vha = tc->vha;
		sgl = tc->prot_sg;
	} else {
		BUG();
		return 1;
	}

	ql_dbg(ql_dbg_tgt, vha, 0xe021,
		"%s: enter\n", __func__);

	for_each_sg(sgl, sg, tot_dsds, i) {
		dma_addr_t	sle_dma;

		/* Allocate additional continuation packets? */
		if (avail_dsds == 0) {
			avail_dsds = (used_dsds > QLA_DSDS_PER_IOCB) ?
						QLA_DSDS_PER_IOCB : used_dsds;
			dsd_list_len = (avail_dsds + 1) * 12;
			used_dsds -= avail_dsds;

			/* allocate tracking DS */
			dsd_ptr = kzalloc(sizeof(struct dsd_dma), GFP_ATOMIC);
			if (!dsd_ptr)
				return 1;

			/* allocate new list */
			dsd_ptr->dsd_addr = next_dsd =
			    dma_pool_alloc(ha->dl_dma_pool, GFP_ATOMIC,
				&dsd_ptr->dsd_list_dma);

			if (!next_dsd) {
				/*
				 * Need to cleanup only this dsd_ptr, rest
				 * will be done by sp_free_dma()
				 */
				kfree(dsd_ptr);
				return 1;
			}

			if (sp) {
				list_add_tail(&dsd_ptr->list,
				    &((struct crc_context *)
					    sp->u.scmd.ctx)->dsd_list);

				sp->flags |= SRB_CRC_CTX_DSD_VALID;
			} else {
				list_add_tail(&dsd_ptr->list,
				    &(tc->ctx->dsd_list));
				tc->ctx_dsd_alloced = 1;
			}

			/* add new list to cmd iocb or last list */
			*cur_dsd++ = cpu_to_le32(LSD(dsd_ptr->dsd_list_dma));
			*cur_dsd++ = cpu_to_le32(MSD(dsd_ptr->dsd_list_dma));
			*cur_dsd++ = dsd_list_len;
			cur_dsd = (uint32_t *)next_dsd;
		}
		sle_dma = sg_dma_address(sg);

		*cur_dsd++ = cpu_to_le32(LSD(sle_dma));
		*cur_dsd++ = cpu_to_le32(MSD(sle_dma));
		*cur_dsd++ = cpu_to_le32(sg_dma_len(sg));

		avail_dsds--;
	}
	/* Null termination */
	*cur_dsd++ = 0;
	*cur_dsd++ = 0;
	*cur_dsd++ = 0;
	return 0;
}

/**
 * qla24xx_build_scsi_crc_2_iocbs() - Build IOCB command utilizing Command
 *							Type 6 IOCB types.
 *
 * @sp: SRB command to process
 * @cmd_pkt: Command type 3 IOCB
 * @tot_dsds: Total number of segments to transfer
 */
static inline int
qla24xx_build_scsi_crc_2_iocbs(srb_t *sp, struct cmd_type_crc_2 *cmd_pkt,
    uint16_t tot_dsds, uint16_t tot_prot_dsds, uint16_t fw_prot_opts)
{
	uint32_t		*cur_dsd, *fcp_dl;
	scsi_qla_host_t		*vha;
	struct scsi_cmnd	*cmd;
	int			sgc;
	uint32_t		total_bytes = 0;
	uint32_t		data_bytes;
	uint32_t		dif_bytes;
	uint8_t			bundling = 1;
	uint16_t		blk_size;
	uint8_t			*clr_ptr;
	struct crc_context	*crc_ctx_pkt = NULL;
	struct qla_hw_data	*ha;
	uint8_t			additional_fcpcdb_len;
	uint16_t		fcp_cmnd_len;
	struct fcp_cmnd		*fcp_cmnd;
	dma_addr_t		crc_ctx_dma;

	cmd = GET_CMD_SP(sp);

	sgc = 0;
	/* Update entry type to indicate Command Type CRC_2 IOCB */
	*((uint32_t *)(&cmd_pkt->entry_type)) =
	    __constant_cpu_to_le32(COMMAND_TYPE_CRC_2);

	vha = sp->fcport->vha;
	ha = vha->hw;

	/* No data transfer */
	data_bytes = scsi_bufflen(cmd);
	if (!data_bytes || cmd->sc_data_direction == DMA_NONE) {
		cmd_pkt->byte_count = __constant_cpu_to_le32(0);
		return QLA_SUCCESS;
	}

	cmd_pkt->vp_index = sp->fcport->vha->vp_idx;

	/* Set transfer direction */
	if (cmd->sc_data_direction == DMA_TO_DEVICE) {
		cmd_pkt->control_flags =
		    __constant_cpu_to_le16(CF_WRITE_DATA);
	} else if (cmd->sc_data_direction == DMA_FROM_DEVICE) {
		cmd_pkt->control_flags =
		    __constant_cpu_to_le16(CF_READ_DATA);
	}

	if ((scsi_get_prot_op(cmd) == SCSI_PROT_READ_INSERT) ||
	    (scsi_get_prot_op(cmd) == SCSI_PROT_WRITE_STRIP) ||
	    (scsi_get_prot_op(cmd) == SCSI_PROT_READ_STRIP) ||
	    (scsi_get_prot_op(cmd) == SCSI_PROT_WRITE_INSERT))
		bundling = 0;

	/* Allocate CRC context from global pool */
	crc_ctx_pkt = sp->u.scmd.ctx =
	    dma_pool_alloc(ha->dl_dma_pool, GFP_ATOMIC, &crc_ctx_dma);

	if (!crc_ctx_pkt)
		goto crc_queuing_error;

	/* Zero out CTX area. */
	clr_ptr = (uint8_t *)crc_ctx_pkt;
	memset(clr_ptr, 0, sizeof(*crc_ctx_pkt));

	crc_ctx_pkt->crc_ctx_dma = crc_ctx_dma;

	sp->flags |= SRB_CRC_CTX_DMA_VALID;

	/* Set handle */
	crc_ctx_pkt->handle = cmd_pkt->handle;

	INIT_LIST_HEAD(&crc_ctx_pkt->dsd_list);

	qla24xx_set_t10dif_tags(sp, (struct fw_dif_context *)
	    &crc_ctx_pkt->ref_tag, tot_prot_dsds);

	cmd_pkt->crc_context_address[0] = cpu_to_le32(LSD(crc_ctx_dma));
	cmd_pkt->crc_context_address[1] = cpu_to_le32(MSD(crc_ctx_dma));
	cmd_pkt->crc_context_len = CRC_CONTEXT_LEN_FW;

	/* Determine SCSI command length -- align to 4 byte boundary */
	if (cmd->cmd_len > 16) {
		additional_fcpcdb_len = cmd->cmd_len - 16;
		if ((cmd->cmd_len % 4) != 0) {
			/* SCSI cmd > 16 bytes must be multiple of 4 */
			goto crc_queuing_error;
		}
		fcp_cmnd_len = 12 + cmd->cmd_len + 4;
	} else {
		additional_fcpcdb_len = 0;
		fcp_cmnd_len = 12 + 16 + 4;
	}

	fcp_cmnd = &crc_ctx_pkt->fcp_cmnd;

	fcp_cmnd->additional_cdb_len = additional_fcpcdb_len;
	if (cmd->sc_data_direction == DMA_TO_DEVICE)
		fcp_cmnd->additional_cdb_len |= 1;
	else if (cmd->sc_data_direction == DMA_FROM_DEVICE)
		fcp_cmnd->additional_cdb_len |= 2;

	int_to_scsilun(cmd->device->lun, &fcp_cmnd->lun);
	memcpy(fcp_cmnd->cdb, cmd->cmnd, cmd->cmd_len);
	cmd_pkt->fcp_cmnd_dseg_len = cpu_to_le16(fcp_cmnd_len);
	cmd_pkt->fcp_cmnd_dseg_address[0] = cpu_to_le32(
	    LSD(crc_ctx_dma + CRC_CONTEXT_FCPCMND_OFF));
	cmd_pkt->fcp_cmnd_dseg_address[1] = cpu_to_le32(
	    MSD(crc_ctx_dma + CRC_CONTEXT_FCPCMND_OFF));
	fcp_cmnd->task_management = 0;
	fcp_cmnd->task_attribute = TSK_SIMPLE;

	cmd_pkt->fcp_rsp_dseg_len = 0; /* Let response come in status iocb */

	/* Compute dif len and adjust data len to incude protection */
	dif_bytes = 0;
	blk_size = cmd->device->sector_size;
	dif_bytes = (data_bytes / blk_size) * 8;

	switch (scsi_get_prot_op(GET_CMD_SP(sp))) {
	case SCSI_PROT_READ_INSERT:
	case SCSI_PROT_WRITE_STRIP:
	    total_bytes = data_bytes;
	    data_bytes += dif_bytes;
	    break;

	case SCSI_PROT_READ_STRIP:
	case SCSI_PROT_WRITE_INSERT:
	case SCSI_PROT_READ_PASS:
	case SCSI_PROT_WRITE_PASS:
	    total_bytes = data_bytes + dif_bytes;
	    break;
	default:
	    BUG();
	}

	if (!qla2x00_hba_err_chk_enabled(sp))
		fw_prot_opts |= 0x10; /* Disable Guard tag checking */
	/* HBA error checking enabled */
	else if (IS_PI_UNINIT_CAPABLE(ha)) {
		if ((scsi_get_prot_type(GET_CMD_SP(sp)) == SCSI_PROT_DIF_TYPE1)
		    || (scsi_get_prot_type(GET_CMD_SP(sp)) ==
			SCSI_PROT_DIF_TYPE2))
			fw_prot_opts |= BIT_10;
		else if (scsi_get_prot_type(GET_CMD_SP(sp)) ==
		    SCSI_PROT_DIF_TYPE3)
			fw_prot_opts |= BIT_11;
	}

	if (!bundling) {
		cur_dsd = (uint32_t *) &crc_ctx_pkt->u.nobundling.data_address;
	} else {
		/*
		 * Configure Bundling if we need to fetch interlaving
		 * protection PCI accesses
		 */
		fw_prot_opts |= PO_ENABLE_DIF_BUNDLING;
		crc_ctx_pkt->u.bundling.dif_byte_count = cpu_to_le32(dif_bytes);
		crc_ctx_pkt->u.bundling.dseg_count = cpu_to_le16(tot_dsds -
							tot_prot_dsds);
		cur_dsd = (uint32_t *) &crc_ctx_pkt->u.bundling.data_address;
	}

	/* Finish the common fields of CRC pkt */
	crc_ctx_pkt->blk_size = cpu_to_le16(blk_size);
	crc_ctx_pkt->prot_opts = cpu_to_le16(fw_prot_opts);
	crc_ctx_pkt->byte_count = cpu_to_le32(data_bytes);
	crc_ctx_pkt->guard_seed = __constant_cpu_to_le16(0);
	/* Fibre channel byte count */
	cmd_pkt->byte_count = cpu_to_le32(total_bytes);
	fcp_dl = (uint32_t *)(crc_ctx_pkt->fcp_cmnd.cdb + 16 +
	    additional_fcpcdb_len);
	*fcp_dl = htonl(total_bytes);

	if (!data_bytes || cmd->sc_data_direction == DMA_NONE) {
		cmd_pkt->byte_count = __constant_cpu_to_le32(0);
		return QLA_SUCCESS;
	}
	/* Walks data segments */

	cmd_pkt->control_flags |=
	    __constant_cpu_to_le16(CF_DATA_SEG_DESCR_ENABLE);

	if (!bundling && tot_prot_dsds) {
		if (qla24xx_walk_and_build_sglist_no_difb(ha, sp,
			cur_dsd, tot_dsds, NULL))
			goto crc_queuing_error;
	} else if (qla24xx_walk_and_build_sglist(ha, sp, cur_dsd,
			(tot_dsds - tot_prot_dsds), NULL))
		goto crc_queuing_error;

	if (bundling && tot_prot_dsds) {
		/* Walks dif segments */
		cmd_pkt->control_flags |=
			__constant_cpu_to_le16(CF_DIF_SEG_DESCR_ENABLE);
		cur_dsd = (uint32_t *) &crc_ctx_pkt->u.bundling.dif_address;
		if (qla24xx_walk_and_build_prot_sglist(ha, sp, cur_dsd,
				tot_prot_dsds, NULL))
			goto crc_queuing_error;
	}
	return QLA_SUCCESS;

crc_queuing_error:
	/* Cleanup will be performed by the caller */

	return QLA_FUNCTION_FAILED;
}

/**
 * qla24xx_start_scsi() - Send a SCSI command to the ISP
 * @sp: command to send to the ISP
 *
 * Returns non-zero if a failure occurred, else zero.
 */
int
qla24xx_start_scsi(srb_t *sp)
{
	int		ret, nseg;
	unsigned long   flags;
	uint32_t	*clr_ptr;
	uint32_t        index;
	uint32_t	handle;
	struct cmd_type_7 *cmd_pkt;
	uint16_t	cnt;
	uint16_t	req_cnt;
	uint16_t	tot_dsds;
	struct req_que *req = NULL;
	struct rsp_que *rsp = NULL;
	struct scsi_cmnd *cmd = GET_CMD_SP(sp);
	struct scsi_qla_host *vha = sp->fcport->vha;
	struct qla_hw_data *ha = vha->hw;

	/* Setup device pointers. */
	ret = 0;

	qla25xx_set_que(sp, &rsp);
	req = vha->req;

	/* So we know we haven't pci_map'ed anything yet */
	tot_dsds = 0;

	/* Send marker if required */
	if (vha->marker_needed != 0) {
		if (qla2x00_marker(vha, req, rsp, 0, 0, MK_SYNC_ALL) !=
		    QLA_SUCCESS)
			return QLA_FUNCTION_FAILED;
		vha->marker_needed = 0;
	}

	/* Acquire ring specific lock */
	spin_lock_irqsave(&ha->hardware_lock, flags);

	/* Check for room in outstanding command list. */
	handle = req->current_outstanding_cmd;
	for (index = 1; index < req->num_outstanding_cmds; index++) {
		handle++;
		if (handle == req->num_outstanding_cmds)
			handle = 1;
		if (!req->outstanding_cmds[handle])
			break;
	}
	if (index == req->num_outstanding_cmds)
		goto queuing_error;

	/* Map the sg table so we have an accurate count of sg entries needed */
	if (scsi_sg_count(cmd)) {
		nseg = dma_map_sg(&ha->pdev->dev, scsi_sglist(cmd),
		    scsi_sg_count(cmd), cmd->sc_data_direction);
		if (unlikely(!nseg))
			goto queuing_error;
	} else
		nseg = 0;

	tot_dsds = nseg;
	req_cnt = qla24xx_calc_iocbs(vha, tot_dsds);
	if (req->cnt < (req_cnt + 2)) {
		cnt = IS_SHADOW_REG_CAPABLE(ha) ? *req->out_ptr :
		    RD_REG_DWORD_RELAXED(req->req_q_out);
		if (req->ring_index < cnt)
			req->cnt = cnt - req->ring_index;
		else
			req->cnt = req->length -
				(req->ring_index - cnt);
		if (req->cnt < (req_cnt + 2))
			goto queuing_error;
	}

	/* Build command packet. */
	req->current_outstanding_cmd = handle;
	req->outstanding_cmds[handle] = sp;
	sp->handle = handle;
	cmd->host_scribble = (unsigned char *)(unsigned long)handle;
	req->cnt -= req_cnt;

	cmd_pkt = (struct cmd_type_7 *)req->ring_ptr;
	cmd_pkt->handle = MAKE_HANDLE(req->id, handle);

	/* Zero out remaining portion of packet. */
	/*    tagged queuing modifier -- default is TSK_SIMPLE (0). */
	clr_ptr = (uint32_t *)cmd_pkt + 2;
	memset(clr_ptr, 0, REQUEST_ENTRY_SIZE - 8);
	cmd_pkt->dseg_count = cpu_to_le16(tot_dsds);

	/* Set NPORT-ID and LUN number*/
	cmd_pkt->nport_handle = cpu_to_le16(sp->fcport->loop_id);
	cmd_pkt->port_id[0] = sp->fcport->d_id.b.al_pa;
	cmd_pkt->port_id[1] = sp->fcport->d_id.b.area;
	cmd_pkt->port_id[2] = sp->fcport->d_id.b.domain;
	cmd_pkt->vp_index = sp->fcport->vha->vp_idx;

	int_to_scsilun(cmd->device->lun, &cmd_pkt->lun);
	host_to_fcp_swap((uint8_t *)&cmd_pkt->lun, sizeof(cmd_pkt->lun));

	cmd_pkt->task = TSK_SIMPLE;

	/* Load SCSI command packet. */
	memcpy(cmd_pkt->fcp_cdb, cmd->cmnd, cmd->cmd_len);
	host_to_fcp_swap(cmd_pkt->fcp_cdb, sizeof(cmd_pkt->fcp_cdb));

	cmd_pkt->byte_count = cpu_to_le32((uint32_t)scsi_bufflen(cmd));

	/* Build IOCB segments */
	qla24xx_build_scsi_iocbs(sp, cmd_pkt, tot_dsds);

	/* Set total data segment count. */
	cmd_pkt->entry_count = (uint8_t)req_cnt;
	/* Specify response queue number where completion should happen */
	cmd_pkt->entry_status = (uint8_t) rsp->id;
	wmb();
	/* Adjust ring index. */
	req->ring_index++;
	if (req->ring_index == req->length) {
		req->ring_index = 0;
		req->ring_ptr = req->ring;
	} else
		req->ring_ptr++;

	sp->flags |= SRB_DMA_VALID;

	/* Set chip new ring index. */
	WRT_REG_DWORD(req->req_q_in, req->ring_index);
	RD_REG_DWORD_RELAXED(&ha->iobase->isp24.hccr);

	/* Manage unprocessed RIO/ZIO commands in response queue. */
	if (vha->flags.process_response_queue &&
		rsp->ring_ptr->signature != RESPONSE_PROCESSED)
		qla24xx_process_response_queue(vha, rsp);

	spin_unlock_irqrestore(&ha->hardware_lock, flags);
	return QLA_SUCCESS;

queuing_error:
	if (tot_dsds)
		scsi_dma_unmap(cmd);

	spin_unlock_irqrestore(&ha->hardware_lock, flags);

	return QLA_FUNCTION_FAILED;
}

/**
 * qla24xx_dif_start_scsi() - Send a SCSI command to the ISP
 * @sp: command to send to the ISP
 *
 * Returns non-zero if a failure occurred, else zero.
 */
int
qla24xx_dif_start_scsi(srb_t *sp)
{
	int			nseg;
	unsigned long		flags;
	uint32_t		*clr_ptr;
	uint32_t		index;
	uint32_t		handle;
	uint16_t		cnt;
	uint16_t		req_cnt = 0;
	uint16_t		tot_dsds;
	uint16_t		tot_prot_dsds;
	uint16_t		fw_prot_opts = 0;
	struct req_que		*req = NULL;
	struct rsp_que		*rsp = NULL;
	struct scsi_cmnd	*cmd = GET_CMD_SP(sp);
	struct scsi_qla_host	*vha = sp->fcport->vha;
	struct qla_hw_data	*ha = vha->hw;
	struct cmd_type_crc_2	*cmd_pkt;
	uint32_t		status = 0;

#define QDSS_GOT_Q_SPACE	BIT_0

	/* Only process protection or >16 cdb in this routine */
	if (scsi_get_prot_op(cmd) == SCSI_PROT_NORMAL) {
		if (cmd->cmd_len <= 16)
			return qla24xx_start_scsi(sp);
	}

	/* Setup device pointers. */

	qla25xx_set_que(sp, &rsp);
	req = vha->req;

	/* So we know we haven't pci_map'ed anything yet */
	tot_dsds = 0;

	/* Send marker if required */
	if (vha->marker_needed != 0) {
		if (qla2x00_marker(vha, req, rsp, 0, 0, MK_SYNC_ALL) !=
		    QLA_SUCCESS)
			return QLA_FUNCTION_FAILED;
		vha->marker_needed = 0;
	}

	/* Acquire ring specific lock */
	spin_lock_irqsave(&ha->hardware_lock, flags);

	/* Check for room in outstanding command list. */
	handle = req->current_outstanding_cmd;
	for (index = 1; index < req->num_outstanding_cmds; index++) {
		handle++;
		if (handle == req->num_outstanding_cmds)
			handle = 1;
		if (!req->outstanding_cmds[handle])
			break;
	}

	if (index == req->num_outstanding_cmds)
		goto queuing_error;

	/* Compute number of required data segments */
	/* Map the sg table so we have an accurate count of sg entries needed */
	if (scsi_sg_count(cmd)) {
		nseg = dma_map_sg(&ha->pdev->dev, scsi_sglist(cmd),
		    scsi_sg_count(cmd), cmd->sc_data_direction);
		if (unlikely(!nseg))
			goto queuing_error;
		else
			sp->flags |= SRB_DMA_VALID;

		if ((scsi_get_prot_op(cmd) == SCSI_PROT_READ_INSERT) ||
		    (scsi_get_prot_op(cmd) == SCSI_PROT_WRITE_STRIP)) {
			struct qla2_sgx sgx;
			uint32_t	partial;

			memset(&sgx, 0, sizeof(struct qla2_sgx));
			sgx.tot_bytes = scsi_bufflen(cmd);
			sgx.cur_sg = scsi_sglist(cmd);
			sgx.sp = sp;

			nseg = 0;
			while (qla24xx_get_one_block_sg(
			    cmd->device->sector_size, &sgx, &partial))
				nseg++;
		}
	} else
		nseg = 0;

	/* number of required data segments */
	tot_dsds = nseg;

	/* Compute number of required protection segments */
	if (qla24xx_configure_prot_mode(sp, &fw_prot_opts)) {
		nseg = dma_map_sg(&ha->pdev->dev, scsi_prot_sglist(cmd),
		    scsi_prot_sg_count(cmd), cmd->sc_data_direction);
		if (unlikely(!nseg))
			goto queuing_error;
		else
			sp->flags |= SRB_CRC_PROT_DMA_VALID;

		if ((scsi_get_prot_op(cmd) == SCSI_PROT_READ_INSERT) ||
		    (scsi_get_prot_op(cmd) == SCSI_PROT_WRITE_STRIP)) {
			nseg = scsi_bufflen(cmd) / cmd->device->sector_size;
		}
	} else {
		nseg = 0;
	}

	req_cnt = 1;
	/* Total Data and protection sg segment(s) */
	tot_prot_dsds = nseg;
	tot_dsds += nseg;
	if (req->cnt < (req_cnt + 2)) {
		cnt = IS_SHADOW_REG_CAPABLE(ha) ? *req->out_ptr :
		    RD_REG_DWORD_RELAXED(req->req_q_out);
		if (req->ring_index < cnt)
			req->cnt = cnt - req->ring_index;
		else
			req->cnt = req->length -
				(req->ring_index - cnt);
		if (req->cnt < (req_cnt + 2))
			goto queuing_error;
	}

	status |= QDSS_GOT_Q_SPACE;

	/* Build header part of command packet (excluding the OPCODE). */
	req->current_outstanding_cmd = handle;
	req->outstanding_cmds[handle] = sp;
	sp->handle = handle;
	cmd->host_scribble = (unsigned char *)(unsigned long)handle;
	req->cnt -= req_cnt;

	/* Fill-in common area */
	cmd_pkt = (struct cmd_type_crc_2 *)req->ring_ptr;
	cmd_pkt->handle = MAKE_HANDLE(req->id, handle);

	clr_ptr = (uint32_t *)cmd_pkt + 2;
	memset(clr_ptr, 0, REQUEST_ENTRY_SIZE - 8);

	/* Set NPORT-ID and LUN number*/
	cmd_pkt->nport_handle = cpu_to_le16(sp->fcport->loop_id);
	cmd_pkt->port_id[0] = sp->fcport->d_id.b.al_pa;
	cmd_pkt->port_id[1] = sp->fcport->d_id.b.area;
	cmd_pkt->port_id[2] = sp->fcport->d_id.b.domain;

	int_to_scsilun(cmd->device->lun, &cmd_pkt->lun);
	host_to_fcp_swap((uint8_t *)&cmd_pkt->lun, sizeof(cmd_pkt->lun));

	/* Total Data and protection segment(s) */
	cmd_pkt->dseg_count = cpu_to_le16(tot_dsds);

	/* Build IOCB segments and adjust for data protection segments */
	if (qla24xx_build_scsi_crc_2_iocbs(sp, (struct cmd_type_crc_2 *)
	    req->ring_ptr, tot_dsds, tot_prot_dsds, fw_prot_opts) !=
		QLA_SUCCESS)
		goto queuing_error;

	cmd_pkt->entry_count = (uint8_t)req_cnt;
	/* Specify response queue number where completion should happen */
	cmd_pkt->entry_status = (uint8_t) rsp->id;
	cmd_pkt->timeout = __constant_cpu_to_le16(0);
	wmb();

	/* Adjust ring index. */
	req->ring_index++;
	if (req->ring_index == req->length) {
		req->ring_index = 0;
		req->ring_ptr = req->ring;
	} else
		req->ring_ptr++;

	/* Set chip new ring index. */
	WRT_REG_DWORD(req->req_q_in, req->ring_index);
	RD_REG_DWORD_RELAXED(&ha->iobase->isp24.hccr);

	/* Manage unprocessed RIO/ZIO commands in response queue. */
	if (vha->flags.process_response_queue &&
	    rsp->ring_ptr->signature != RESPONSE_PROCESSED)
		qla24xx_process_response_queue(vha, rsp);

	spin_unlock_irqrestore(&ha->hardware_lock, flags);

	return QLA_SUCCESS;

queuing_error:
	if (status & QDSS_GOT_Q_SPACE) {
		req->outstanding_cmds[handle] = NULL;
		req->cnt += req_cnt;
	}
	/* Cleanup will be performed by the caller (queuecommand) */

	spin_unlock_irqrestore(&ha->hardware_lock, flags);
	return QLA_FUNCTION_FAILED;
}


static void qla25xx_set_que(srb_t *sp, struct rsp_que **rsp)
{
	struct scsi_cmnd *cmd = GET_CMD_SP(sp);
	struct qla_hw_data *ha = sp->fcport->vha->hw;
	int affinity = cmd->request->cpu;

	if (ha->flags.cpu_affinity_enabled && affinity >= 0 &&
		affinity < ha->max_rsp_queues - 1)
		*rsp = ha->rsp_q_map[affinity + 1];
	 else
		*rsp = ha->rsp_q_map[0];
}

/* Generic Control-SRB manipulation functions. */

/* hardware_lock assumed to be held. */
void *
qla2x00_alloc_iocbs_ready(scsi_qla_host_t *vha, srb_t *sp)
{
	if (qla2x00_reset_active(vha))
		return NULL;

	return qla2x00_alloc_iocbs(vha, sp);
}

void *
qla2x00_alloc_iocbs(scsi_qla_host_t *vha, srb_t *sp)
{
	struct qla_hw_data *ha = vha->hw;
	struct req_que *req = ha->req_q_map[0];
	device_reg_t __iomem *reg = ISP_QUE_REG(ha, req->id);
	uint32_t index, handle;
	request_t *pkt;
	uint16_t cnt, req_cnt;

	pkt = NULL;
	req_cnt = 1;
	handle = 0;

	if (!sp)
		goto skip_cmd_array;

	/* Check for room in outstanding command list. */
	handle = req->current_outstanding_cmd;
	for (index = 1; index < req->num_outstanding_cmds; index++) {
		handle++;
		if (handle == req->num_outstanding_cmds)
			handle = 1;
		if (!req->outstanding_cmds[handle])
			break;
	}
	if (index == req->num_outstanding_cmds) {
		ql_log(ql_log_warn, vha, 0x700b,
		    "No room on outstanding cmd array.\n");
		goto queuing_error;
	}

	/* Prep command array. */
	req->current_outstanding_cmd = handle;
	req->outstanding_cmds[handle] = sp;
	sp->handle = handle;

	/* Adjust entry-counts as needed. */
	if (sp->type != SRB_SCSI_CMD)
		req_cnt = sp->iocbs;

skip_cmd_array:
	/* Check for room on request queue. */
	if (req->cnt < req_cnt + 2) {
		if (ha->mqenable || IS_QLA83XX(ha) || IS_QLA27XX(ha))
			cnt = RD_REG_DWORD(&reg->isp25mq.req_q_out);
		else if (IS_P3P_TYPE(ha))
			cnt = RD_REG_DWORD(&reg->isp82.req_q_out);
		else if (IS_FWI2_CAPABLE(ha))
			cnt = RD_REG_DWORD(&reg->isp24.req_q_out);
		else if (IS_QLAFX00(ha))
			cnt = RD_REG_DWORD(&reg->ispfx00.req_q_out);
		else
			cnt = qla2x00_debounce_register(
			    ISP_REQ_Q_OUT(ha, &reg->isp));

		if  (req->ring_index < cnt)
			req->cnt = cnt - req->ring_index;
		else
			req->cnt = req->length -
			    (req->ring_index - cnt);
	}
	if (req->cnt < req_cnt + 2)
		goto queuing_error;

	/* Prep packet */
	req->cnt -= req_cnt;
	pkt = req->ring_ptr;
	memset(pkt, 0, REQUEST_ENTRY_SIZE);
	if (IS_QLAFX00(ha)) {
		WRT_REG_BYTE((void __iomem *)&pkt->entry_count, req_cnt);
		WRT_REG_WORD((void __iomem *)&pkt->handle, handle);
	} else {
		pkt->entry_count = req_cnt;
		pkt->handle = handle;
	}

queuing_error:
	return pkt;
}

static void
qla24xx_login_iocb(srb_t *sp, struct logio_entry_24xx *logio)
{
	struct srb_iocb *lio = &sp->u.iocb_cmd;

	logio->entry_type = LOGINOUT_PORT_IOCB_TYPE;
	logio->control_flags = cpu_to_le16(LCF_COMMAND_PLOGI);
	if (lio->u.logio.flags & SRB_LOGIN_COND_PLOGI)
		logio->control_flags |= cpu_to_le16(LCF_COND_PLOGI);
	if (lio->u.logio.flags & SRB_LOGIN_SKIP_PRLI)
		logio->control_flags |= cpu_to_le16(LCF_SKIP_PRLI);
	logio->nport_handle = cpu_to_le16(sp->fcport->loop_id);
	logio->port_id[0] = sp->fcport->d_id.b.al_pa;
	logio->port_id[1] = sp->fcport->d_id.b.area;
	logio->port_id[2] = sp->fcport->d_id.b.domain;
	logio->vp_index = sp->fcport->vha->vp_idx;
}

static void
qla2x00_login_iocb(srb_t *sp, struct mbx_entry *mbx)
{
	struct qla_hw_data *ha = sp->fcport->vha->hw;
	struct srb_iocb *lio = &sp->u.iocb_cmd;
	uint16_t opts;

	mbx->entry_type = MBX_IOCB_TYPE;
	SET_TARGET_ID(ha, mbx->loop_id, sp->fcport->loop_id);
	mbx->mb0 = cpu_to_le16(MBC_LOGIN_FABRIC_PORT);
	opts = lio->u.logio.flags & SRB_LOGIN_COND_PLOGI ? BIT_0 : 0;
	opts |= lio->u.logio.flags & SRB_LOGIN_SKIP_PRLI ? BIT_1 : 0;
	if (HAS_EXTENDED_IDS(ha)) {
		mbx->mb1 = cpu_to_le16(sp->fcport->loop_id);
		mbx->mb10 = cpu_to_le16(opts);
	} else {
		mbx->mb1 = cpu_to_le16((sp->fcport->loop_id << 8) | opts);
	}
	mbx->mb2 = cpu_to_le16(sp->fcport->d_id.b.domain);
	mbx->mb3 = cpu_to_le16(sp->fcport->d_id.b.area << 8 |
	    sp->fcport->d_id.b.al_pa);
	mbx->mb9 = cpu_to_le16(sp->fcport->vha->vp_idx);
}

static void
qla24xx_logout_iocb(srb_t *sp, struct logio_entry_24xx *logio)
{
	logio->entry_type = LOGINOUT_PORT_IOCB_TYPE;
	logio->control_flags =
	    cpu_to_le16(LCF_COMMAND_LOGO|LCF_IMPL_LOGO);
	if (!sp->fcport->tgt_session ||
	    !sp->fcport->tgt_session->keep_nport_handle)
		logio->control_flags |= cpu_to_le16(LCF_FREE_NPORT);
	logio->nport_handle = cpu_to_le16(sp->fcport->loop_id);
	logio->port_id[0] = sp->fcport->d_id.b.al_pa;
	logio->port_id[1] = sp->fcport->d_id.b.area;
	logio->port_id[2] = sp->fcport->d_id.b.domain;
	logio->vp_index = sp->fcport->vha->vp_idx;
}

static void
qla2x00_logout_iocb(srb_t *sp, struct mbx_entry *mbx)
{
	struct qla_hw_data *ha = sp->fcport->vha->hw;

	mbx->entry_type = MBX_IOCB_TYPE;
	SET_TARGET_ID(ha, mbx->loop_id, sp->fcport->loop_id);
	mbx->mb0 = cpu_to_le16(MBC_LOGOUT_FABRIC_PORT);
	mbx->mb1 = HAS_EXTENDED_IDS(ha) ?
	    cpu_to_le16(sp->fcport->loop_id):
	    cpu_to_le16(sp->fcport->loop_id << 8);
	mbx->mb2 = cpu_to_le16(sp->fcport->d_id.b.domain);
	mbx->mb3 = cpu_to_le16(sp->fcport->d_id.b.area << 8 |
	    sp->fcport->d_id.b.al_pa);
	mbx->mb9 = cpu_to_le16(sp->fcport->vha->vp_idx);
	/* Implicit: mbx->mbx10 = 0. */
}

static void
qla24xx_adisc_iocb(srb_t *sp, struct logio_entry_24xx *logio)
{
	logio->entry_type = LOGINOUT_PORT_IOCB_TYPE;
	logio->control_flags = cpu_to_le16(LCF_COMMAND_ADISC);
	logio->nport_handle = cpu_to_le16(sp->fcport->loop_id);
	logio->vp_index = sp->fcport->vha->vp_idx;
}

static void
qla2x00_adisc_iocb(srb_t *sp, struct mbx_entry *mbx)
{
	struct qla_hw_data *ha = sp->fcport->vha->hw;

	mbx->entry_type = MBX_IOCB_TYPE;
	SET_TARGET_ID(ha, mbx->loop_id, sp->fcport->loop_id);
	mbx->mb0 = cpu_to_le16(MBC_GET_PORT_DATABASE);
	if (HAS_EXTENDED_IDS(ha)) {
		mbx->mb1 = cpu_to_le16(sp->fcport->loop_id);
		mbx->mb10 = cpu_to_le16(BIT_0);
	} else {
		mbx->mb1 = cpu_to_le16((sp->fcport->loop_id << 8) | BIT_0);
	}
	mbx->mb2 = cpu_to_le16(MSW(ha->async_pd_dma));
	mbx->mb3 = cpu_to_le16(LSW(ha->async_pd_dma));
	mbx->mb6 = cpu_to_le16(MSW(MSD(ha->async_pd_dma)));
	mbx->mb7 = cpu_to_le16(LSW(MSD(ha->async_pd_dma)));
	mbx->mb9 = cpu_to_le16(sp->fcport->vha->vp_idx);
}

static void
qla24xx_tm_iocb(srb_t *sp, struct tsk_mgmt_entry *tsk)
{
	uint32_t flags;
	uint64_t lun;
	struct fc_port *fcport = sp->fcport;
	scsi_qla_host_t *vha = fcport->vha;
	struct qla_hw_data *ha = vha->hw;
	struct srb_iocb *iocb = &sp->u.iocb_cmd;
	struct req_que *req = vha->req;

	flags = iocb->u.tmf.flags;
	lun = iocb->u.tmf.lun;

	tsk->entry_type = TSK_MGMT_IOCB_TYPE;
	tsk->entry_count = 1;
	tsk->handle = MAKE_HANDLE(req->id, tsk->handle);
	tsk->nport_handle = cpu_to_le16(fcport->loop_id);
	tsk->timeout = cpu_to_le16(ha->r_a_tov / 10 * 2);
	tsk->control_flags = cpu_to_le32(flags);
	tsk->port_id[0] = fcport->d_id.b.al_pa;
	tsk->port_id[1] = fcport->d_id.b.area;
	tsk->port_id[2] = fcport->d_id.b.domain;
	tsk->vp_index = fcport->vha->vp_idx;

	if (flags == TCF_LUN_RESET) {
		int_to_scsilun(lun, &tsk->lun);
		host_to_fcp_swap((uint8_t *)&tsk->lun,
			sizeof(tsk->lun));
	}
}

static void
qla24xx_els_iocb(srb_t *sp, struct els_entry_24xx *els_iocb)
{
	struct fc_bsg_job *bsg_job = sp->u.bsg_job;

        els_iocb->entry_type = ELS_IOCB_TYPE;
        els_iocb->entry_count = 1;
        els_iocb->sys_define = 0;
        els_iocb->entry_status = 0;
        els_iocb->handle = sp->handle;
        els_iocb->nport_handle = cpu_to_le16(sp->fcport->loop_id);
        els_iocb->tx_dsd_count = __constant_cpu_to_le16(bsg_job->request_payload.sg_cnt);
	els_iocb->vp_index = sp->fcport->vha->vp_idx;
        els_iocb->sof_type = EST_SOFI3;
        els_iocb->rx_dsd_count = __constant_cpu_to_le16(bsg_job->reply_payload.sg_cnt);

	els_iocb->opcode =
	    sp->type == SRB_ELS_CMD_RPT ?
	    bsg_job->request->rqst_data.r_els.els_code :
	    bsg_job->request->rqst_data.h_els.command_code;
        els_iocb->port_id[0] = sp->fcport->d_id.b.al_pa;
        els_iocb->port_id[1] = sp->fcport->d_id.b.area;
        els_iocb->port_id[2] = sp->fcport->d_id.b.domain;
        els_iocb->control_flags = 0;
        els_iocb->rx_byte_count =
            cpu_to_le32(bsg_job->reply_payload.payload_len);
        els_iocb->tx_byte_count =
            cpu_to_le32(bsg_job->request_payload.payload_len);

        els_iocb->tx_address[0] = cpu_to_le32(LSD(sg_dma_address
            (bsg_job->request_payload.sg_list)));
        els_iocb->tx_address[1] = cpu_to_le32(MSD(sg_dma_address
            (bsg_job->request_payload.sg_list)));
        els_iocb->tx_len = cpu_to_le32(sg_dma_len
            (bsg_job->request_payload.sg_list));

        els_iocb->rx_address[0] = cpu_to_le32(LSD(sg_dma_address
            (bsg_job->reply_payload.sg_list)));
        els_iocb->rx_address[1] = cpu_to_le32(MSD(sg_dma_address
            (bsg_job->reply_payload.sg_list)));
        els_iocb->rx_len = cpu_to_le32(sg_dma_len
            (bsg_job->reply_payload.sg_list));

	sp->fcport->vha->qla_stats.control_requests++;
}

static void
qla2x00_ct_iocb(srb_t *sp, ms_iocb_entry_t *ct_iocb)
{
	uint16_t        avail_dsds;
	uint32_t        *cur_dsd;
	struct scatterlist *sg;
	int index;
	uint16_t tot_dsds;
	scsi_qla_host_t *vha = sp->fcport->vha;
	struct qla_hw_data *ha = vha->hw;
	struct fc_bsg_job *bsg_job = sp->u.bsg_job;
	int loop_iterartion = 0;
	int cont_iocb_prsnt = 0;
	int entry_count = 1;

	memset(ct_iocb, 0, sizeof(ms_iocb_entry_t));
	ct_iocb->entry_type = CT_IOCB_TYPE;
	ct_iocb->entry_status = 0;
	ct_iocb->handle1 = sp->handle;
	SET_TARGET_ID(ha, ct_iocb->loop_id, sp->fcport->loop_id);
	ct_iocb->status = __constant_cpu_to_le16(0);
	ct_iocb->control_flags = __constant_cpu_to_le16(0);
	ct_iocb->timeout = 0;
	ct_iocb->cmd_dsd_count =
	    __constant_cpu_to_le16(bsg_job->request_payload.sg_cnt);
	ct_iocb->total_dsd_count =
	    __constant_cpu_to_le16(bsg_job->request_payload.sg_cnt + 1);
	ct_iocb->req_bytecount =
	    cpu_to_le32(bsg_job->request_payload.payload_len);
	ct_iocb->rsp_bytecount =
	    cpu_to_le32(bsg_job->reply_payload.payload_len);

	ct_iocb->dseg_req_address[0] = cpu_to_le32(LSD(sg_dma_address
	    (bsg_job->request_payload.sg_list)));
	ct_iocb->dseg_req_address[1] = cpu_to_le32(MSD(sg_dma_address
	    (bsg_job->request_payload.sg_list)));
	ct_iocb->dseg_req_length = ct_iocb->req_bytecount;

	ct_iocb->dseg_rsp_address[0] = cpu_to_le32(LSD(sg_dma_address
	    (bsg_job->reply_payload.sg_list)));
	ct_iocb->dseg_rsp_address[1] = cpu_to_le32(MSD(sg_dma_address
	    (bsg_job->reply_payload.sg_list)));
	ct_iocb->dseg_rsp_length = ct_iocb->rsp_bytecount;

	avail_dsds = 1;
	cur_dsd = (uint32_t *)ct_iocb->dseg_rsp_address;
	index = 0;
	tot_dsds = bsg_job->reply_payload.sg_cnt;

	for_each_sg(bsg_job->reply_payload.sg_list, sg, tot_dsds, index) {
		dma_addr_t       sle_dma;
		cont_a64_entry_t *cont_pkt;

		/* Allocate additional continuation packets? */
		if (avail_dsds == 0) {
			/*
			* Five DSDs are available in the Cont.
			* Type 1 IOCB.
			       */
			cont_pkt = qla2x00_prep_cont_type1_iocb(vha,
			    vha->hw->req_q_map[0]);
			cur_dsd = (uint32_t *) cont_pkt->dseg_0_address;
			avail_dsds = 5;
			cont_iocb_prsnt = 1;
			entry_count++;
		}

		sle_dma = sg_dma_address(sg);
		*cur_dsd++   = cpu_to_le32(LSD(sle_dma));
		*cur_dsd++   = cpu_to_le32(MSD(sle_dma));
		*cur_dsd++   = cpu_to_le32(sg_dma_len(sg));
		loop_iterartion++;
		avail_dsds--;
	}
	ct_iocb->entry_count = entry_count;

	sp->fcport->vha->qla_stats.control_requests++;
}

static void
qla24xx_ct_iocb(srb_t *sp, struct ct_entry_24xx *ct_iocb)
{
	uint16_t        avail_dsds;
	uint32_t        *cur_dsd;
	struct scatterlist *sg;
	int index;
	uint16_t tot_dsds;
        scsi_qla_host_t *vha = sp->fcport->vha;
	struct qla_hw_data *ha = vha->hw;
	struct fc_bsg_job *bsg_job = sp->u.bsg_job;
	int loop_iterartion = 0;
	int cont_iocb_prsnt = 0;
	int entry_count = 1;

	ct_iocb->entry_type = CT_IOCB_TYPE;
        ct_iocb->entry_status = 0;
        ct_iocb->sys_define = 0;
        ct_iocb->handle = sp->handle;

	ct_iocb->nport_handle = cpu_to_le16(sp->fcport->loop_id);
	ct_iocb->vp_index = sp->fcport->vha->vp_idx;
        ct_iocb->comp_status = __constant_cpu_to_le16(0);

	ct_iocb->cmd_dsd_count =
            __constant_cpu_to_le16(bsg_job->request_payload.sg_cnt);
        ct_iocb->timeout = 0;
        ct_iocb->rsp_dsd_count =
            __constant_cpu_to_le16(bsg_job->reply_payload.sg_cnt);
        ct_iocb->rsp_byte_count =
            cpu_to_le32(bsg_job->reply_payload.payload_len);
        ct_iocb->cmd_byte_count =
            cpu_to_le32(bsg_job->request_payload.payload_len);
        ct_iocb->dseg_0_address[0] = cpu_to_le32(LSD(sg_dma_address
            (bsg_job->request_payload.sg_list)));
        ct_iocb->dseg_0_address[1] = cpu_to_le32(MSD(sg_dma_address
           (bsg_job->request_payload.sg_list)));
        ct_iocb->dseg_0_len = cpu_to_le32(sg_dma_len
            (bsg_job->request_payload.sg_list));

	avail_dsds = 1;
	cur_dsd = (uint32_t *)ct_iocb->dseg_1_address;
	index = 0;
	tot_dsds = bsg_job->reply_payload.sg_cnt;

	for_each_sg(bsg_job->reply_payload.sg_list, sg, tot_dsds, index) {
		dma_addr_t       sle_dma;
		cont_a64_entry_t *cont_pkt;

		/* Allocate additional continuation packets? */
		if (avail_dsds == 0) {
			/*
			* Five DSDs are available in the Cont.
			* Type 1 IOCB.
			       */
			cont_pkt = qla2x00_prep_cont_type1_iocb(vha,
			    ha->req_q_map[0]);
			cur_dsd = (uint32_t *) cont_pkt->dseg_0_address;
			avail_dsds = 5;
			cont_iocb_prsnt = 1;
			entry_count++;
		}

		sle_dma = sg_dma_address(sg);
		*cur_dsd++   = cpu_to_le32(LSD(sle_dma));
		*cur_dsd++   = cpu_to_le32(MSD(sle_dma));
		*cur_dsd++   = cpu_to_le32(sg_dma_len(sg));
		loop_iterartion++;
		avail_dsds--;
	}
        ct_iocb->entry_count = entry_count;
}

/*
 * qla82xx_start_scsi() - Send a SCSI command to the ISP
 * @sp: command to send to the ISP
 *
 * Returns non-zero if a failure occurred, else zero.
 */
int
qla82xx_start_scsi(srb_t *sp)
{
	int		ret, nseg;
	unsigned long   flags;
	struct scsi_cmnd *cmd;
	uint32_t	*clr_ptr;
	uint32_t        index;
	uint32_t	handle;
	uint16_t	cnt;
	uint16_t	req_cnt;
	uint16_t	tot_dsds;
	struct device_reg_82xx __iomem *reg;
	uint32_t dbval;
	uint32_t *fcp_dl;
	uint8_t additional_cdb_len;
	struct ct6_dsd *ctx;
	struct scsi_qla_host *vha = sp->fcport->vha;
	struct qla_hw_data *ha = vha->hw;
	struct req_que *req = NULL;
	struct rsp_que *rsp = NULL;

	/* Setup device pointers. */
	ret = 0;
	reg = &ha->iobase->isp82;
	cmd = GET_CMD_SP(sp);
	req = vha->req;
	rsp = ha->rsp_q_map[0];

	/* So we know we haven't pci_map'ed anything yet */
	tot_dsds = 0;

	dbval = 0x04 | (ha->portnum << 5);

	/* Send marker if required */
	if (vha->marker_needed != 0) {
		if (qla2x00_marker(vha, req,
			rsp, 0, 0, MK_SYNC_ALL) != QLA_SUCCESS) {
			ql_log(ql_log_warn, vha, 0x300c,
			    "qla2x00_marker failed for cmd=%p.\n", cmd);
			return QLA_FUNCTION_FAILED;
		}
		vha->marker_needed = 0;
	}

	/* Acquire ring specific lock */
	spin_lock_irqsave(&ha->hardware_lock, flags);

	/* Check for room in outstanding command list. */
	handle = req->current_outstanding_cmd;
	for (index = 1; index < req->num_outstanding_cmds; index++) {
		handle++;
		if (handle == req->num_outstanding_cmds)
			handle = 1;
		if (!req->outstanding_cmds[handle])
			break;
	}
	if (index == req->num_outstanding_cmds)
		goto queuing_error;

	/* Map the sg table so we have an accurate count of sg entries needed */
	if (scsi_sg_count(cmd)) {
		nseg = dma_map_sg(&ha->pdev->dev, scsi_sglist(cmd),
		    scsi_sg_count(cmd), cmd->sc_data_direction);
		if (unlikely(!nseg))
			goto queuing_error;
	} else
		nseg = 0;

	tot_dsds = nseg;

	if (tot_dsds > ql2xshiftctondsd) {
		struct cmd_type_6 *cmd_pkt;
		uint16_t more_dsd_lists = 0;
		struct dsd_dma *dsd_ptr;
		uint16_t i;

		more_dsd_lists = qla24xx_calc_dsd_lists(tot_dsds);
		if ((more_dsd_lists + ha->gbl_dsd_inuse) >= NUM_DSD_CHAIN) {
			ql_dbg(ql_dbg_io, vha, 0x300d,
			    "Num of DSD list %d is than %d for cmd=%p.\n",
			    more_dsd_lists + ha->gbl_dsd_inuse, NUM_DSD_CHAIN,
			    cmd);
			goto queuing_error;
		}

		if (more_dsd_lists <= ha->gbl_dsd_avail)
			goto sufficient_dsds;
		else
			more_dsd_lists -= ha->gbl_dsd_avail;

		for (i = 0; i < more_dsd_lists; i++) {
			dsd_ptr = kzalloc(sizeof(struct dsd_dma), GFP_ATOMIC);
			if (!dsd_ptr) {
				ql_log(ql_log_fatal, vha, 0x300e,
				    "Failed to allocate memory for dsd_dma "
				    "for cmd=%p.\n", cmd);
				goto queuing_error;
			}

			dsd_ptr->dsd_addr = dma_pool_alloc(ha->dl_dma_pool,
				GFP_ATOMIC, &dsd_ptr->dsd_list_dma);
			if (!dsd_ptr->dsd_addr) {
				kfree(dsd_ptr);
				ql_log(ql_log_fatal, vha, 0x300f,
				    "Failed to allocate memory for dsd_addr "
				    "for cmd=%p.\n", cmd);
				goto queuing_error;
			}
			list_add_tail(&dsd_ptr->list, &ha->gbl_dsd_list);
			ha->gbl_dsd_avail++;
		}

sufficient_dsds:
		req_cnt = 1;

		if (req->cnt < (req_cnt + 2)) {
			cnt = (uint16_t)RD_REG_DWORD_RELAXED(
				&reg->req_q_out[0]);
			if (req->ring_index < cnt)
				req->cnt = cnt - req->ring_index;
			else
				req->cnt = req->length -
					(req->ring_index - cnt);
			if (req->cnt < (req_cnt + 2))
				goto queuing_error;
		}

		ctx = sp->u.scmd.ctx =
		    mempool_alloc(ha->ctx_mempool, GFP_ATOMIC);
		if (!ctx) {
			ql_log(ql_log_fatal, vha, 0x3010,
			    "Failed to allocate ctx for cmd=%p.\n", cmd);
			goto queuing_error;
		}

		memset(ctx, 0, sizeof(struct ct6_dsd));
		ctx->fcp_cmnd = dma_pool_alloc(ha->fcp_cmnd_dma_pool,
			GFP_ATOMIC, &ctx->fcp_cmnd_dma);
		if (!ctx->fcp_cmnd) {
			ql_log(ql_log_fatal, vha, 0x3011,
			    "Failed to allocate fcp_cmnd for cmd=%p.\n", cmd);
			goto queuing_error;
		}

		/* Initialize the DSD list and dma handle */
		INIT_LIST_HEAD(&ctx->dsd_list);
		ctx->dsd_use_cnt = 0;

		if (cmd->cmd_len > 16) {
			additional_cdb_len = cmd->cmd_len - 16;
			if ((cmd->cmd_len % 4) != 0) {
				/* SCSI command bigger than 16 bytes must be
				 * multiple of 4
				 */
				ql_log(ql_log_warn, vha, 0x3012,
				    "scsi cmd len %d not multiple of 4 "
				    "for cmd=%p.\n", cmd->cmd_len, cmd);
				goto queuing_error_fcp_cmnd;
			}
			ctx->fcp_cmnd_len = 12 + cmd->cmd_len + 4;
		} else {
			additional_cdb_len = 0;
			ctx->fcp_cmnd_len = 12 + 16 + 4;
		}

		cmd_pkt = (struct cmd_type_6 *)req->ring_ptr;
		cmd_pkt->handle = MAKE_HANDLE(req->id, handle);

		/* Zero out remaining portion of packet. */
		/*    tagged queuing modifier -- default is TSK_SIMPLE (0). */
		clr_ptr = (uint32_t *)cmd_pkt + 2;
		memset(clr_ptr, 0, REQUEST_ENTRY_SIZE - 8);
		cmd_pkt->dseg_count = cpu_to_le16(tot_dsds);

		/* Set NPORT-ID and LUN number*/
		cmd_pkt->nport_handle = cpu_to_le16(sp->fcport->loop_id);
		cmd_pkt->port_id[0] = sp->fcport->d_id.b.al_pa;
		cmd_pkt->port_id[1] = sp->fcport->d_id.b.area;
		cmd_pkt->port_id[2] = sp->fcport->d_id.b.domain;
		cmd_pkt->vp_index = sp->fcport->vha->vp_idx;

		/* Build IOCB segments */
		if (qla24xx_build_scsi_type_6_iocbs(sp, cmd_pkt, tot_dsds))
			goto queuing_error_fcp_cmnd;

		int_to_scsilun(cmd->device->lun, &cmd_pkt->lun);
		host_to_fcp_swap((uint8_t *)&cmd_pkt->lun, sizeof(cmd_pkt->lun));

		/* build FCP_CMND IU */
		memset(ctx->fcp_cmnd, 0, sizeof(struct fcp_cmnd));
		int_to_scsilun(cmd->device->lun, &ctx->fcp_cmnd->lun);
		ctx->fcp_cmnd->additional_cdb_len = additional_cdb_len;

		if (cmd->sc_data_direction == DMA_TO_DEVICE)
			ctx->fcp_cmnd->additional_cdb_len |= 1;
		else if (cmd->sc_data_direction == DMA_FROM_DEVICE)
			ctx->fcp_cmnd->additional_cdb_len |= 2;

		/* Populate the FCP_PRIO. */
		if (ha->flags.fcp_prio_enabled)
			ctx->fcp_cmnd->task_attribute |=
			    sp->fcport->fcp_prio << 3;

		memcpy(ctx->fcp_cmnd->cdb, cmd->cmnd, cmd->cmd_len);

		fcp_dl = (uint32_t *)(ctx->fcp_cmnd->cdb + 16 +
		    additional_cdb_len);
		*fcp_dl = htonl((uint32_t)scsi_bufflen(cmd));

		cmd_pkt->fcp_cmnd_dseg_len = cpu_to_le16(ctx->fcp_cmnd_len);
		cmd_pkt->fcp_cmnd_dseg_address[0] =
		    cpu_to_le32(LSD(ctx->fcp_cmnd_dma));
		cmd_pkt->fcp_cmnd_dseg_address[1] =
		    cpu_to_le32(MSD(ctx->fcp_cmnd_dma));

		sp->flags |= SRB_FCP_CMND_DMA_VALID;
		cmd_pkt->byte_count = cpu_to_le32((uint32_t)scsi_bufflen(cmd));
		/* Set total data segment count. */
		cmd_pkt->entry_count = (uint8_t)req_cnt;
		/* Specify response queue number where
		 * completion should happen
		 */
		cmd_pkt->entry_status = (uint8_t) rsp->id;
	} else {
		struct cmd_type_7 *cmd_pkt;
		req_cnt = qla24xx_calc_iocbs(vha, tot_dsds);
		if (req->cnt < (req_cnt + 2)) {
			cnt = (uint16_t)RD_REG_DWORD_RELAXED(
			    &reg->req_q_out[0]);
			if (req->ring_index < cnt)
				req->cnt = cnt - req->ring_index;
			else
				req->cnt = req->length -
					(req->ring_index - cnt);
		}
		if (req->cnt < (req_cnt + 2))
			goto queuing_error;

		cmd_pkt = (struct cmd_type_7 *)req->ring_ptr;
		cmd_pkt->handle = MAKE_HANDLE(req->id, handle);

		/* Zero out remaining portion of packet. */
		/* tagged queuing modifier -- default is TSK_SIMPLE (0).*/
		clr_ptr = (uint32_t *)cmd_pkt + 2;
		memset(clr_ptr, 0, REQUEST_ENTRY_SIZE - 8);
		cmd_pkt->dseg_count = cpu_to_le16(tot_dsds);

		/* Set NPORT-ID and LUN number*/
		cmd_pkt->nport_handle = cpu_to_le16(sp->fcport->loop_id);
		cmd_pkt->port_id[0] = sp->fcport->d_id.b.al_pa;
		cmd_pkt->port_id[1] = sp->fcport->d_id.b.area;
		cmd_pkt->port_id[2] = sp->fcport->d_id.b.domain;
		cmd_pkt->vp_index = sp->fcport->vha->vp_idx;

		int_to_scsilun(cmd->device->lun, &cmd_pkt->lun);
		host_to_fcp_swap((uint8_t *)&cmd_pkt->lun,
		    sizeof(cmd_pkt->lun));

		/* Populate the FCP_PRIO. */
		if (ha->flags.fcp_prio_enabled)
			cmd_pkt->task |= sp->fcport->fcp_prio << 3;

		/* Load SCSI command packet. */
		memcpy(cmd_pkt->fcp_cdb, cmd->cmnd, cmd->cmd_len);
		host_to_fcp_swap(cmd_pkt->fcp_cdb, sizeof(cmd_pkt->fcp_cdb));

		cmd_pkt->byte_count = cpu_to_le32((uint32_t)scsi_bufflen(cmd));

		/* Build IOCB segments */
		qla24xx_build_scsi_iocbs(sp, cmd_pkt, tot_dsds);

		/* Set total data segment count. */
		cmd_pkt->entry_count = (uint8_t)req_cnt;
		/* Specify response queue number where
		 * completion should happen.
		 */
		cmd_pkt->entry_status = (uint8_t) rsp->id;

	}
	/* Build command packet. */
	req->current_outstanding_cmd = handle;
	req->outstanding_cmds[handle] = sp;
	sp->handle = handle;
	cmd->host_scribble = (unsigned char *)(unsigned long)handle;
	req->cnt -= req_cnt;
	wmb();

	/* Adjust ring index. */
	req->ring_index++;
	if (req->ring_index == req->length) {
		req->ring_index = 0;
		req->ring_ptr = req->ring;
	} else
		req->ring_ptr++;

	sp->flags |= SRB_DMA_VALID;

	/* Set chip new ring index. */
	/* write, read and verify logic */
	dbval = dbval | (req->id << 8) | (req->ring_index << 16);
	if (ql2xdbwr)
		qla82xx_wr_32(ha, ha->nxdb_wr_ptr, dbval);
	else {
		WRT_REG_DWORD(
			(unsigned long __iomem *)ha->nxdb_wr_ptr,
			dbval);
		wmb();
		while (RD_REG_DWORD((void __iomem *)ha->nxdb_rd_ptr) != dbval) {
			WRT_REG_DWORD(
				(unsigned long __iomem *)ha->nxdb_wr_ptr,
				dbval);
			wmb();
		}
	}

	/* Manage unprocessed RIO/ZIO commands in response queue. */
	if (vha->flags.process_response_queue &&
	    rsp->ring_ptr->signature != RESPONSE_PROCESSED)
		qla24xx_process_response_queue(vha, rsp);

	spin_unlock_irqrestore(&ha->hardware_lock, flags);
	return QLA_SUCCESS;

queuing_error_fcp_cmnd:
	dma_pool_free(ha->fcp_cmnd_dma_pool, ctx->fcp_cmnd, ctx->fcp_cmnd_dma);
queuing_error:
	if (tot_dsds)
		scsi_dma_unmap(cmd);

	if (sp->u.scmd.ctx) {
		mempool_free(sp->u.scmd.ctx, ha->ctx_mempool);
		sp->u.scmd.ctx = NULL;
	}
	spin_unlock_irqrestore(&ha->hardware_lock, flags);

	return QLA_FUNCTION_FAILED;
}

static void
qla24xx_abort_iocb(srb_t *sp, struct abort_entry_24xx *abt_iocb)
{
	struct srb_iocb *aio = &sp->u.iocb_cmd;
	scsi_qla_host_t *vha = sp->fcport->vha;
	struct req_que *req = vha->req;

	memset(abt_iocb, 0, sizeof(struct abort_entry_24xx));
	abt_iocb->entry_type = ABORT_IOCB_TYPE;
	abt_iocb->entry_count = 1;
	abt_iocb->handle = cpu_to_le32(MAKE_HANDLE(req->id, sp->handle));
	abt_iocb->nport_handle = cpu_to_le16(sp->fcport->loop_id);
	abt_iocb->handle_to_abort =
	    cpu_to_le32(MAKE_HANDLE(req->id, aio->u.abt.cmd_hndl));
	abt_iocb->port_id[0] = sp->fcport->d_id.b.al_pa;
	abt_iocb->port_id[1] = sp->fcport->d_id.b.area;
	abt_iocb->port_id[2] = sp->fcport->d_id.b.domain;
	abt_iocb->vp_index = vha->vp_idx;
	abt_iocb->req_que_no = cpu_to_le16(req->id);
	/* Send the command to the firmware */
	wmb();
}

int
qla2x00_start_sp(srb_t *sp)
{
	int rval;
	struct qla_hw_data *ha = sp->fcport->vha->hw;
	void *pkt;
	unsigned long flags;

	rval = QLA_FUNCTION_FAILED;
	spin_lock_irqsave(&ha->hardware_lock, flags);
	pkt = qla2x00_alloc_iocbs(sp->fcport->vha, sp);
	if (!pkt) {
		ql_log(ql_log_warn, sp->fcport->vha, 0x700c,
		    "qla2x00_alloc_iocbs failed.\n");
		goto done;
	}

	rval = QLA_SUCCESS;
	switch (sp->type) {
	case SRB_LOGIN_CMD:
		IS_FWI2_CAPABLE(ha) ?
		    qla24xx_login_iocb(sp, pkt) :
		    qla2x00_login_iocb(sp, pkt);
		break;
	case SRB_LOGOUT_CMD:
		IS_FWI2_CAPABLE(ha) ?
		    qla24xx_logout_iocb(sp, pkt) :
		    qla2x00_logout_iocb(sp, pkt);
		break;
	case SRB_ELS_CMD_RPT:
	case SRB_ELS_CMD_HST:
		qla24xx_els_iocb(sp, pkt);
		break;
	case SRB_CT_CMD:
		IS_FWI2_CAPABLE(ha) ?
		    qla24xx_ct_iocb(sp, pkt) :
		    qla2x00_ct_iocb(sp, pkt);
		break;
	case SRB_ADISC_CMD:
		IS_FWI2_CAPABLE(ha) ?
		    qla24xx_adisc_iocb(sp, pkt) :
		    qla2x00_adisc_iocb(sp, pkt);
		break;
	case SRB_TM_CMD:
		IS_QLAFX00(ha) ?
		    qlafx00_tm_iocb(sp, pkt) :
		    qla24xx_tm_iocb(sp, pkt);
		break;
	case SRB_FXIOCB_DCMD:
	case SRB_FXIOCB_BCMD:
		qlafx00_fxdisc_iocb(sp, pkt);
		break;
	case SRB_ABT_CMD:
		IS_QLAFX00(ha) ?
			qlafx00_abort_iocb(sp, pkt) :
			qla24xx_abort_iocb(sp, pkt);
		break;
	default:
		break;
	}

	wmb();
	qla2x00_start_iocbs(sp->fcport->vha, ha->req_q_map[0]);
done:
	spin_unlock_irqrestore(&ha->hardware_lock, flags);
	return rval;
}

static void
qla25xx_build_bidir_iocb(srb_t *sp, struct scsi_qla_host *vha,
				struct cmd_bidir *cmd_pkt, uint32_t tot_dsds)
{
	uint16_t avail_dsds;
	uint32_t *cur_dsd;
	uint32_t req_data_len = 0;
	uint32_t rsp_data_len = 0;
	struct scatterlist *sg;
	int index;
	int entry_count = 1;
	struct fc_bsg_job *bsg_job = sp->u.bsg_job;

	/*Update entry type to indicate bidir command */
	*((uint32_t *)(&cmd_pkt->entry_type)) =
		__constant_cpu_to_le32(COMMAND_BIDIRECTIONAL);

	/* Set the transfer direction, in this set both flags
	 * Also set the BD_WRAP_BACK flag, firmware will take care
	 * assigning DID=SID for outgoing pkts.
	 */
	cmd_pkt->wr_dseg_count = cpu_to_le16(bsg_job->request_payload.sg_cnt);
	cmd_pkt->rd_dseg_count = cpu_to_le16(bsg_job->reply_payload.sg_cnt);
	cmd_pkt->control_flags =
			__constant_cpu_to_le16(BD_WRITE_DATA | BD_READ_DATA |
							BD_WRAP_BACK);

	req_data_len = rsp_data_len = bsg_job->request_payload.payload_len;
	cmd_pkt->wr_byte_count = cpu_to_le32(req_data_len);
	cmd_pkt->rd_byte_count = cpu_to_le32(rsp_data_len);
	cmd_pkt->timeout = cpu_to_le16(qla2x00_get_async_timeout(vha) + 2);

	vha->bidi_stats.transfer_bytes += req_data_len;
	vha->bidi_stats.io_count++;

	vha->qla_stats.output_bytes += req_data_len;
	vha->qla_stats.output_requests++;

	/* Only one dsd is available for bidirectional IOCB, remaining dsds
	 * are bundled in continuation iocb
	 */
	avail_dsds = 1;
	cur_dsd = (uint32_t *)&cmd_pkt->fcp_data_dseg_address;

	index = 0;

	for_each_sg(bsg_job->request_payload.sg_list, sg,
				bsg_job->request_payload.sg_cnt, index) {
		dma_addr_t sle_dma;
		cont_a64_entry_t *cont_pkt;

		/* Allocate additional continuation packets */
		if (avail_dsds == 0) {
			/* Continuation type 1 IOCB can accomodate
			 * 5 DSDS
			 */
			cont_pkt = qla2x00_prep_cont_type1_iocb(vha, vha->req);
			cur_dsd = (uint32_t *) cont_pkt->dseg_0_address;
			avail_dsds = 5;
			entry_count++;
		}
		sle_dma = sg_dma_address(sg);
		*cur_dsd++   = cpu_to_le32(LSD(sle_dma));
		*cur_dsd++   = cpu_to_le32(MSD(sle_dma));
		*cur_dsd++   = cpu_to_le32(sg_dma_len(sg));
		avail_dsds--;
	}
	/* For read request DSD will always goes to continuation IOCB
	 * and follow the write DSD. If there is room on the current IOCB
	 * then it is added to that IOCB else new continuation IOCB is
	 * allocated.
	 */
	for_each_sg(bsg_job->reply_payload.sg_list, sg,
				bsg_job->reply_payload.sg_cnt, index) {
		dma_addr_t sle_dma;
		cont_a64_entry_t *cont_pkt;

		/* Allocate additional continuation packets */
		if (avail_dsds == 0) {
			/* Continuation type 1 IOCB can accomodate
			 * 5 DSDS
			 */
			cont_pkt = qla2x00_prep_cont_type1_iocb(vha, vha->req);
			cur_dsd = (uint32_t *) cont_pkt->dseg_0_address;
			avail_dsds = 5;
			entry_count++;
		}
		sle_dma = sg_dma_address(sg);
		*cur_dsd++   = cpu_to_le32(LSD(sle_dma));
		*cur_dsd++   = cpu_to_le32(MSD(sle_dma));
		*cur_dsd++   = cpu_to_le32(sg_dma_len(sg));
		avail_dsds--;
	}
	/* This value should be same as number of IOCB required for this cmd */
	cmd_pkt->entry_count = entry_count;
}

int
qla2x00_start_bidir(srb_t *sp, struct scsi_qla_host *vha, uint32_t tot_dsds)
{

	struct qla_hw_data *ha = vha->hw;
	unsigned long flags;
	uint32_t handle;
	uint32_t index;
	uint16_t req_cnt;
	uint16_t cnt;
	uint32_t *clr_ptr;
	struct cmd_bidir *cmd_pkt = NULL;
	struct rsp_que *rsp;
	struct req_que *req;
	int rval = EXT_STATUS_OK;

	rval = QLA_SUCCESS;

	rsp = ha->rsp_q_map[0];
	req = vha->req;

	/* Send marker if required */
	if (vha->marker_needed != 0) {
		if (qla2x00_marker(vha, req,
			rsp, 0, 0, MK_SYNC_ALL) != QLA_SUCCESS)
			return EXT_STATUS_MAILBOX;
		vha->marker_needed = 0;
	}

	/* Acquire ring specific lock */
	spin_lock_irqsave(&ha->hardware_lock, flags);

	/* Check for room in outstanding command list. */
	handle = req->current_outstanding_cmd;
	for (index = 1; index < req->num_outstanding_cmds; index++) {
		handle++;
	if (handle == req->num_outstanding_cmds)
		handle = 1;
	if (!req->outstanding_cmds[handle])
		break;
	}

	if (index == req->num_outstanding_cmds) {
		rval = EXT_STATUS_BUSY;
		goto queuing_error;
	}

	/* Calculate number of IOCB required */
	req_cnt = qla24xx_calc_iocbs(vha, tot_dsds);

	/* Check for room on request queue. */
	if (req->cnt < req_cnt + 2) {
		cnt = IS_SHADOW_REG_CAPABLE(ha) ? *req->out_ptr :
		    RD_REG_DWORD_RELAXED(req->req_q_out);
		if  (req->ring_index < cnt)
			req->cnt = cnt - req->ring_index;
		else
			req->cnt = req->length -
				(req->ring_index - cnt);
	}
	if (req->cnt < req_cnt + 2) {
		rval = EXT_STATUS_BUSY;
		goto queuing_error;
	}

	cmd_pkt = (struct cmd_bidir *)req->ring_ptr;
	cmd_pkt->handle = MAKE_HANDLE(req->id, handle);

	/* Zero out remaining portion of packet. */
	/* tagged queuing modifier -- default is TSK_SIMPLE (0).*/
	clr_ptr = (uint32_t *)cmd_pkt + 2;
	memset(clr_ptr, 0, REQUEST_ENTRY_SIZE - 8);

	/* Set NPORT-ID  (of vha)*/
	cmd_pkt->nport_handle = cpu_to_le16(vha->self_login_loop_id);
	cmd_pkt->port_id[0] = vha->d_id.b.al_pa;
	cmd_pkt->port_id[1] = vha->d_id.b.area;
	cmd_pkt->port_id[2] = vha->d_id.b.domain;

	qla25xx_build_bidir_iocb(sp, vha, cmd_pkt, tot_dsds);
	cmd_pkt->entry_status = (uint8_t) rsp->id;
	/* Build command packet. */
	req->current_outstanding_cmd = handle;
	req->outstanding_cmds[handle] = sp;
	sp->handle = handle;
	req->cnt -= req_cnt;

	/* Send the command to the firmware */
	wmb();
	qla2x00_start_iocbs(vha, req);
queuing_error:
	spin_unlock_irqrestore(&ha->hardware_lock, flags);
	return rval;
}
