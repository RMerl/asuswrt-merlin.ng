/*******************************************************************************
 *
 * Intel Ethernet Controller XL710 Family Linux Driver
 * Copyright(c) 2013 - 2015 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 *
 * Contact Information:
 * e1000-devel Mailing List <e1000-devel@lists.sourceforge.net>
 * Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497
 *
 ******************************************************************************/

#include "i40e.h"

/*********************notification routines***********************/

/**
 * i40e_vc_vf_broadcast
 * @pf: pointer to the PF structure
 * @opcode: operation code
 * @retval: return value
 * @msg: pointer to the msg buffer
 * @msglen: msg length
 *
 * send a message to all VFs on a given PF
 **/
static void i40e_vc_vf_broadcast(struct i40e_pf *pf,
				 enum i40e_virtchnl_ops v_opcode,
				 i40e_status v_retval, u8 *msg,
				 u16 msglen)
{
	struct i40e_hw *hw = &pf->hw;
	struct i40e_vf *vf = pf->vf;
	int i;

	for (i = 0; i < pf->num_alloc_vfs; i++, vf++) {
		int abs_vf_id = vf->vf_id + hw->func_caps.vf_base_id;
		/* Not all vfs are enabled so skip the ones that are not */
		if (!test_bit(I40E_VF_STAT_INIT, &vf->vf_states) &&
		    !test_bit(I40E_VF_STAT_ACTIVE, &vf->vf_states))
			continue;

		/* Ignore return value on purpose - a given VF may fail, but
		 * we need to keep going and send to all of them
		 */
		i40e_aq_send_msg_to_vf(hw, abs_vf_id, v_opcode, v_retval,
				       msg, msglen, NULL);
	}
}

/**
 * i40e_vc_notify_link_state
 * @vf: pointer to the VF structure
 *
 * send a link status message to a single VF
 **/
static void i40e_vc_notify_vf_link_state(struct i40e_vf *vf)
{
	struct i40e_virtchnl_pf_event pfe;
	struct i40e_pf *pf = vf->pf;
	struct i40e_hw *hw = &pf->hw;
	struct i40e_link_status *ls = &pf->hw.phy.link_info;
	int abs_vf_id = vf->vf_id + hw->func_caps.vf_base_id;

	pfe.event = I40E_VIRTCHNL_EVENT_LINK_CHANGE;
	pfe.severity = I40E_PF_EVENT_SEVERITY_INFO;
	if (vf->link_forced) {
		pfe.event_data.link_event.link_status = vf->link_up;
		pfe.event_data.link_event.link_speed =
			(vf->link_up ? I40E_LINK_SPEED_40GB : 0);
	} else {
		pfe.event_data.link_event.link_status =
			ls->link_info & I40E_AQ_LINK_UP;
		pfe.event_data.link_event.link_speed = ls->link_speed;
	}
	i40e_aq_send_msg_to_vf(hw, abs_vf_id, I40E_VIRTCHNL_OP_EVENT,
			       0, (u8 *)&pfe, sizeof(pfe), NULL);
}

/**
 * i40e_vc_notify_link_state
 * @pf: pointer to the PF structure
 *
 * send a link status message to all VFs on a given PF
 **/
void i40e_vc_notify_link_state(struct i40e_pf *pf)
{
	int i;

	for (i = 0; i < pf->num_alloc_vfs; i++)
		i40e_vc_notify_vf_link_state(&pf->vf[i]);
}

/**
 * i40e_vc_notify_reset
 * @pf: pointer to the PF structure
 *
 * indicate a pending reset to all VFs on a given PF
 **/
void i40e_vc_notify_reset(struct i40e_pf *pf)
{
	struct i40e_virtchnl_pf_event pfe;

	pfe.event = I40E_VIRTCHNL_EVENT_RESET_IMPENDING;
	pfe.severity = I40E_PF_EVENT_SEVERITY_CERTAIN_DOOM;
	i40e_vc_vf_broadcast(pf, I40E_VIRTCHNL_OP_EVENT, 0,
			     (u8 *)&pfe, sizeof(struct i40e_virtchnl_pf_event));
}

/**
 * i40e_vc_notify_vf_reset
 * @vf: pointer to the VF structure
 *
 * indicate a pending reset to the given VF
 **/
void i40e_vc_notify_vf_reset(struct i40e_vf *vf)
{
	struct i40e_virtchnl_pf_event pfe;
	int abs_vf_id;

	/* validate the request */
	if (!vf || vf->vf_id >= vf->pf->num_alloc_vfs)
		return;

	/* verify if the VF is in either init or active before proceeding */
	if (!test_bit(I40E_VF_STAT_INIT, &vf->vf_states) &&
	    !test_bit(I40E_VF_STAT_ACTIVE, &vf->vf_states))
		return;

	abs_vf_id = vf->vf_id + vf->pf->hw.func_caps.vf_base_id;

	pfe.event = I40E_VIRTCHNL_EVENT_RESET_IMPENDING;
	pfe.severity = I40E_PF_EVENT_SEVERITY_CERTAIN_DOOM;
	i40e_aq_send_msg_to_vf(&vf->pf->hw, abs_vf_id, I40E_VIRTCHNL_OP_EVENT,
			       0, (u8 *)&pfe,
			       sizeof(struct i40e_virtchnl_pf_event), NULL);
}
/***********************misc routines*****************************/

/**
 * i40e_vc_disable_vf
 * @pf: pointer to the PF info
 * @vf: pointer to the VF info
 *
 * Disable the VF through a SW reset
 **/
static inline void i40e_vc_disable_vf(struct i40e_pf *pf, struct i40e_vf *vf)
{
	struct i40e_hw *hw = &pf->hw;
	u32 reg;

	reg = rd32(hw, I40E_VPGEN_VFRTRIG(vf->vf_id));
	reg |= I40E_VPGEN_VFRTRIG_VFSWR_MASK;
	wr32(hw, I40E_VPGEN_VFRTRIG(vf->vf_id), reg);
	i40e_flush(hw);
}

/**
 * i40e_vc_isvalid_vsi_id
 * @vf: pointer to the VF info
 * @vsi_id: VF relative VSI id
 *
 * check for the valid VSI id
 **/
static inline bool i40e_vc_isvalid_vsi_id(struct i40e_vf *vf, u16 vsi_id)
{
	struct i40e_pf *pf = vf->pf;
	struct i40e_vsi *vsi = i40e_find_vsi_from_id(pf, vsi_id);

	return (vsi && (vsi->vf_id == vf->vf_id));
}

/**
 * i40e_vc_isvalid_queue_id
 * @vf: pointer to the VF info
 * @vsi_id: vsi id
 * @qid: vsi relative queue id
 *
 * check for the valid queue id
 **/
static inline bool i40e_vc_isvalid_queue_id(struct i40e_vf *vf, u16 vsi_id,
					    u8 qid)
{
	struct i40e_pf *pf = vf->pf;
	struct i40e_vsi *vsi = i40e_find_vsi_from_id(pf, vsi_id);

	return (vsi && (qid < vsi->alloc_queue_pairs));
}

/**
 * i40e_vc_isvalid_vector_id
 * @vf: pointer to the VF info
 * @vector_id: VF relative vector id
 *
 * check for the valid vector id
 **/
static inline bool i40e_vc_isvalid_vector_id(struct i40e_vf *vf, u8 vector_id)
{
	struct i40e_pf *pf = vf->pf;

	return vector_id < pf->hw.func_caps.num_msix_vectors_vf;
}

/***********************vf resource mgmt routines*****************/

/**
 * i40e_vc_get_pf_queue_id
 * @vf: pointer to the VF info
 * @vsi_id: id of VSI as provided by the FW
 * @vsi_queue_id: vsi relative queue id
 *
 * return PF relative queue id
 **/
static u16 i40e_vc_get_pf_queue_id(struct i40e_vf *vf, u16 vsi_id,
				   u8 vsi_queue_id)
{
	struct i40e_pf *pf = vf->pf;
	struct i40e_vsi *vsi = i40e_find_vsi_from_id(pf, vsi_id);
	u16 pf_queue_id = I40E_QUEUE_END_OF_LIST;

	if (!vsi)
		return pf_queue_id;

	if (le16_to_cpu(vsi->info.mapping_flags) &
	    I40E_AQ_VSI_QUE_MAP_NONCONTIG)
		pf_queue_id =
			le16_to_cpu(vsi->info.queue_mapping[vsi_queue_id]);
	else
		pf_queue_id = le16_to_cpu(vsi->info.queue_mapping[0]) +
			      vsi_queue_id;

	return pf_queue_id;
}

/**
 * i40e_config_irq_link_list
 * @vf: pointer to the VF info
 * @vsi_id: id of VSI as given by the FW
 * @vecmap: irq map info
 *
 * configure irq link list from the map
 **/
static void i40e_config_irq_link_list(struct i40e_vf *vf, u16 vsi_id,
				      struct i40e_virtchnl_vector_map *vecmap)
{
	unsigned long linklistmap = 0, tempmap;
	struct i40e_pf *pf = vf->pf;
	struct i40e_hw *hw = &pf->hw;
	u16 vsi_queue_id, pf_queue_id;
	enum i40e_queue_type qtype;
	u16 next_q, vector_id;
	u32 reg, reg_idx;
	u16 itr_idx = 0;

	vector_id = vecmap->vector_id;
	/* setup the head */
	if (0 == vector_id)
		reg_idx = I40E_VPINT_LNKLST0(vf->vf_id);
	else
		reg_idx = I40E_VPINT_LNKLSTN(
		     ((pf->hw.func_caps.num_msix_vectors_vf - 1) * vf->vf_id) +
		     (vector_id - 1));

	if (vecmap->rxq_map == 0 && vecmap->txq_map == 0) {
		/* Special case - No queues mapped on this vector */
		wr32(hw, reg_idx, I40E_VPINT_LNKLST0_FIRSTQ_INDX_MASK);
		goto irq_list_done;
	}
	tempmap = vecmap->rxq_map;
	for_each_set_bit(vsi_queue_id, &tempmap, I40E_MAX_VSI_QP) {
		linklistmap |= (1 <<
				(I40E_VIRTCHNL_SUPPORTED_QTYPES *
				 vsi_queue_id));
	}

	tempmap = vecmap->txq_map;
	for_each_set_bit(vsi_queue_id, &tempmap, I40E_MAX_VSI_QP) {
		linklistmap |= (1 <<
				(I40E_VIRTCHNL_SUPPORTED_QTYPES * vsi_queue_id
				 + 1));
	}

	next_q = find_first_bit(&linklistmap,
				(I40E_MAX_VSI_QP *
				 I40E_VIRTCHNL_SUPPORTED_QTYPES));
	vsi_queue_id = next_q/I40E_VIRTCHNL_SUPPORTED_QTYPES;
	qtype = next_q%I40E_VIRTCHNL_SUPPORTED_QTYPES;
	pf_queue_id = i40e_vc_get_pf_queue_id(vf, vsi_id, vsi_queue_id);
	reg = ((qtype << I40E_VPINT_LNKLSTN_FIRSTQ_TYPE_SHIFT) | pf_queue_id);

	wr32(hw, reg_idx, reg);

	while (next_q < (I40E_MAX_VSI_QP * I40E_VIRTCHNL_SUPPORTED_QTYPES)) {
		switch (qtype) {
		case I40E_QUEUE_TYPE_RX:
			reg_idx = I40E_QINT_RQCTL(pf_queue_id);
			itr_idx = vecmap->rxitr_idx;
			break;
		case I40E_QUEUE_TYPE_TX:
			reg_idx = I40E_QINT_TQCTL(pf_queue_id);
			itr_idx = vecmap->txitr_idx;
			break;
		default:
			break;
		}

		next_q = find_next_bit(&linklistmap,
				       (I40E_MAX_VSI_QP *
					I40E_VIRTCHNL_SUPPORTED_QTYPES),
				       next_q + 1);
		if (next_q <
		    (I40E_MAX_VSI_QP * I40E_VIRTCHNL_SUPPORTED_QTYPES)) {
			vsi_queue_id = next_q / I40E_VIRTCHNL_SUPPORTED_QTYPES;
			qtype = next_q % I40E_VIRTCHNL_SUPPORTED_QTYPES;
			pf_queue_id = i40e_vc_get_pf_queue_id(vf, vsi_id,
							      vsi_queue_id);
		} else {
			pf_queue_id = I40E_QUEUE_END_OF_LIST;
			qtype = 0;
		}

		/* format for the RQCTL & TQCTL regs is same */
		reg = (vector_id) |
		    (qtype << I40E_QINT_RQCTL_NEXTQ_TYPE_SHIFT) |
		    (pf_queue_id << I40E_QINT_RQCTL_NEXTQ_INDX_SHIFT) |
		    (1 << I40E_QINT_RQCTL_CAUSE_ENA_SHIFT) |
		    (itr_idx << I40E_QINT_RQCTL_ITR_INDX_SHIFT);
		wr32(hw, reg_idx, reg);
	}

irq_list_done:
	i40e_flush(hw);
}

/**
 * i40e_config_vsi_tx_queue
 * @vf: pointer to the VF info
 * @vsi_id: id of VSI as provided by the FW
 * @vsi_queue_id: vsi relative queue index
 * @info: config. info
 *
 * configure tx queue
 **/
static int i40e_config_vsi_tx_queue(struct i40e_vf *vf, u16 vsi_id,
				    u16 vsi_queue_id,
				    struct i40e_virtchnl_txq_info *info)
{
	struct i40e_pf *pf = vf->pf;
	struct i40e_hw *hw = &pf->hw;
	struct i40e_hmc_obj_txq tx_ctx;
	struct i40e_vsi *vsi;
	u16 pf_queue_id;
	u32 qtx_ctl;
	int ret = 0;

	pf_queue_id = i40e_vc_get_pf_queue_id(vf, vsi_id, vsi_queue_id);
	vsi = i40e_find_vsi_from_id(pf, vsi_id);

	/* clear the context structure first */
	memset(&tx_ctx, 0, sizeof(struct i40e_hmc_obj_txq));

	/* only set the required fields */
	tx_ctx.base = info->dma_ring_addr / 128;
	tx_ctx.qlen = info->ring_len;
	tx_ctx.rdylist = le16_to_cpu(vsi->info.qs_handle[0]);
	tx_ctx.rdylist_act = 0;
	tx_ctx.head_wb_ena = info->headwb_enabled;
	tx_ctx.head_wb_addr = info->dma_headwb_addr;

	/* clear the context in the HMC */
	ret = i40e_clear_lan_tx_queue_context(hw, pf_queue_id);
	if (ret) {
		dev_err(&pf->pdev->dev,
			"Failed to clear VF LAN Tx queue context %d, error: %d\n",
			pf_queue_id, ret);
		ret = -ENOENT;
		goto error_context;
	}

	/* set the context in the HMC */
	ret = i40e_set_lan_tx_queue_context(hw, pf_queue_id, &tx_ctx);
	if (ret) {
		dev_err(&pf->pdev->dev,
			"Failed to set VF LAN Tx queue context %d error: %d\n",
			pf_queue_id, ret);
		ret = -ENOENT;
		goto error_context;
	}

	/* associate this queue with the PCI VF function */
	qtx_ctl = I40E_QTX_CTL_VF_QUEUE;
	qtx_ctl |= ((hw->pf_id << I40E_QTX_CTL_PF_INDX_SHIFT)
		    & I40E_QTX_CTL_PF_INDX_MASK);
	qtx_ctl |= (((vf->vf_id + hw->func_caps.vf_base_id)
		     << I40E_QTX_CTL_VFVM_INDX_SHIFT)
		    & I40E_QTX_CTL_VFVM_INDX_MASK);
	wr32(hw, I40E_QTX_CTL(pf_queue_id), qtx_ctl);
	i40e_flush(hw);

error_context:
	return ret;
}

/**
 * i40e_config_vsi_rx_queue
 * @vf: pointer to the VF info
 * @vsi_id: id of VSI  as provided by the FW
 * @vsi_queue_id: vsi relative queue index
 * @info: config. info
 *
 * configure rx queue
 **/
static int i40e_config_vsi_rx_queue(struct i40e_vf *vf, u16 vsi_id,
				    u16 vsi_queue_id,
				    struct i40e_virtchnl_rxq_info *info)
{
	struct i40e_pf *pf = vf->pf;
	struct i40e_hw *hw = &pf->hw;
	struct i40e_hmc_obj_rxq rx_ctx;
	u16 pf_queue_id;
	int ret = 0;

	pf_queue_id = i40e_vc_get_pf_queue_id(vf, vsi_id, vsi_queue_id);

	/* clear the context structure first */
	memset(&rx_ctx, 0, sizeof(struct i40e_hmc_obj_rxq));

	/* only set the required fields */
	rx_ctx.base = info->dma_ring_addr / 128;
	rx_ctx.qlen = info->ring_len;

	if (info->splithdr_enabled) {
		rx_ctx.hsplit_0 = I40E_RX_SPLIT_L2      |
				  I40E_RX_SPLIT_IP      |
				  I40E_RX_SPLIT_TCP_UDP |
				  I40E_RX_SPLIT_SCTP;
		/* header length validation */
		if (info->hdr_size > ((2 * 1024) - 64)) {
			ret = -EINVAL;
			goto error_param;
		}
		rx_ctx.hbuff = info->hdr_size >> I40E_RXQ_CTX_HBUFF_SHIFT;

		/* set splitalways mode 10b */
		rx_ctx.dtype = 0x2;
	}

	/* databuffer length validation */
	if (info->databuffer_size > ((16 * 1024) - 128)) {
		ret = -EINVAL;
		goto error_param;
	}
	rx_ctx.dbuff = info->databuffer_size >> I40E_RXQ_CTX_DBUFF_SHIFT;

	/* max pkt. length validation */
	if (info->max_pkt_size >= (16 * 1024) || info->max_pkt_size < 64) {
		ret = -EINVAL;
		goto error_param;
	}
	rx_ctx.rxmax = info->max_pkt_size;

	/* enable 32bytes desc always */
	rx_ctx.dsize = 1;

	/* default values */
	rx_ctx.lrxqthresh = 2;
	rx_ctx.crcstrip = 1;
	rx_ctx.prefena = 1;
	rx_ctx.l2tsel = 1;

	/* clear the context in the HMC */
	ret = i40e_clear_lan_rx_queue_context(hw, pf_queue_id);
	if (ret) {
		dev_err(&pf->pdev->dev,
			"Failed to clear VF LAN Rx queue context %d, error: %d\n",
			pf_queue_id, ret);
		ret = -ENOENT;
		goto error_param;
	}

	/* set the context in the HMC */
	ret = i40e_set_lan_rx_queue_context(hw, pf_queue_id, &rx_ctx);
	if (ret) {
		dev_err(&pf->pdev->dev,
			"Failed to set VF LAN Rx queue context %d error: %d\n",
			pf_queue_id, ret);
		ret = -ENOENT;
		goto error_param;
	}

error_param:
	return ret;
}

/**
 * i40e_alloc_vsi_res
 * @vf: pointer to the VF info
 * @type: type of VSI to allocate
 *
 * alloc VF vsi context & resources
 **/
static int i40e_alloc_vsi_res(struct i40e_vf *vf, enum i40e_vsi_type type)
{
	struct i40e_mac_filter *f = NULL;
	struct i40e_pf *pf = vf->pf;
	struct i40e_vsi *vsi;
	int ret = 0;

	vsi = i40e_vsi_setup(pf, type, pf->vsi[pf->lan_vsi]->seid, vf->vf_id);

	if (!vsi) {
		dev_err(&pf->pdev->dev,
			"add vsi failed for VF %d, aq_err %d\n",
			vf->vf_id, pf->hw.aq.asq_last_status);
		ret = -ENOENT;
		goto error_alloc_vsi_res;
	}
	if (type == I40E_VSI_SRIOV) {
		u8 brdcast[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
		vf->lan_vsi_idx = vsi->idx;
		vf->lan_vsi_id = vsi->id;
		/* If the port VLAN has been configured and then the
		 * VF driver was removed then the VSI port VLAN
		 * configuration was destroyed.  Check if there is
		 * a port VLAN and restore the VSI configuration if
		 * needed.
		 */
		if (vf->port_vlan_id)
			i40e_vsi_add_pvid(vsi, vf->port_vlan_id);
		f = i40e_add_filter(vsi, vf->default_lan_addr.addr,
				    vf->port_vlan_id, true, false);
		if (!f)
			dev_info(&pf->pdev->dev,
				 "Could not allocate VF MAC addr\n");
		f = i40e_add_filter(vsi, brdcast, vf->port_vlan_id,
				    true, false);
		if (!f)
			dev_info(&pf->pdev->dev,
				 "Could not allocate VF broadcast filter\n");
	}

	/* program mac filter */
	ret = i40e_sync_vsi_filters(vsi);
	if (ret)
		dev_err(&pf->pdev->dev, "Unable to program ucast filters\n");

	/* Set VF bandwidth if specified */
	if (vf->tx_rate) {
		ret = i40e_aq_config_vsi_bw_limit(&pf->hw, vsi->seid,
						  vf->tx_rate / 50, 0, NULL);
		if (ret)
			dev_err(&pf->pdev->dev, "Unable to set tx rate, VF %d, error code %d.\n",
				vf->vf_id, ret);
	}

error_alloc_vsi_res:
	return ret;
}

/**
 * i40e_enable_vf_mappings
 * @vf: pointer to the VF info
 *
 * enable VF mappings
 **/
static void i40e_enable_vf_mappings(struct i40e_vf *vf)
{
	struct i40e_pf *pf = vf->pf;
	struct i40e_hw *hw = &pf->hw;
	u32 reg, total_queue_pairs = 0;
	int j;

	/* Tell the hardware we're using noncontiguous mapping. HW requires
	 * that VF queues be mapped using this method, even when they are
	 * contiguous in real life
	 */
	wr32(hw, I40E_VSILAN_QBASE(vf->lan_vsi_id),
	     I40E_VSILAN_QBASE_VSIQTABLE_ENA_MASK);

	/* enable VF vplan_qtable mappings */
	reg = I40E_VPLAN_MAPENA_TXRX_ENA_MASK;
	wr32(hw, I40E_VPLAN_MAPENA(vf->vf_id), reg);

	/* map PF queues to VF queues */
	for (j = 0; j < pf->vsi[vf->lan_vsi_idx]->alloc_queue_pairs; j++) {
		u16 qid = i40e_vc_get_pf_queue_id(vf, vf->lan_vsi_id, j);
		reg = (qid & I40E_VPLAN_QTABLE_QINDEX_MASK);
		wr32(hw, I40E_VPLAN_QTABLE(total_queue_pairs, vf->vf_id), reg);
		total_queue_pairs++;
	}

	/* map PF queues to VSI */
	for (j = 0; j < 7; j++) {
		if (j * 2 >= pf->vsi[vf->lan_vsi_idx]->alloc_queue_pairs) {
			reg = 0x07FF07FF;	/* unused */
		} else {
			u16 qid = i40e_vc_get_pf_queue_id(vf, vf->lan_vsi_id,
							  j * 2);
			reg = qid;
			qid = i40e_vc_get_pf_queue_id(vf, vf->lan_vsi_id,
						      (j * 2) + 1);
			reg |= qid << 16;
		}
		wr32(hw, I40E_VSILAN_QTABLE(j, vf->lan_vsi_id), reg);
	}

	i40e_flush(hw);
}

/**
 * i40e_disable_vf_mappings
 * @vf: pointer to the VF info
 *
 * disable VF mappings
 **/
static void i40e_disable_vf_mappings(struct i40e_vf *vf)
{
	struct i40e_pf *pf = vf->pf;
	struct i40e_hw *hw = &pf->hw;
	int i;

	/* disable qp mappings */
	wr32(hw, I40E_VPLAN_MAPENA(vf->vf_id), 0);
	for (i = 0; i < I40E_MAX_VSI_QP; i++)
		wr32(hw, I40E_VPLAN_QTABLE(i, vf->vf_id),
		     I40E_QUEUE_END_OF_LIST);
	i40e_flush(hw);
}

/**
 * i40e_free_vf_res
 * @vf: pointer to the VF info
 *
 * free VF resources
 **/
static void i40e_free_vf_res(struct i40e_vf *vf)
{
	struct i40e_pf *pf = vf->pf;
	struct i40e_hw *hw = &pf->hw;
	u32 reg_idx, reg;
	int i, msix_vf;

	/* free vsi & disconnect it from the parent uplink */
	if (vf->lan_vsi_idx) {
		i40e_vsi_release(pf->vsi[vf->lan_vsi_idx]);
		vf->lan_vsi_idx = 0;
		vf->lan_vsi_id = 0;
	}
	msix_vf = pf->hw.func_caps.num_msix_vectors_vf;

	/* disable interrupts so the VF starts in a known state */
	for (i = 0; i < msix_vf; i++) {
		/* format is same for both registers */
		if (0 == i)
			reg_idx = I40E_VFINT_DYN_CTL0(vf->vf_id);
		else
			reg_idx = I40E_VFINT_DYN_CTLN(((msix_vf - 1) *
						      (vf->vf_id))
						     + (i - 1));
		wr32(hw, reg_idx, I40E_VFINT_DYN_CTLN_CLEARPBA_MASK);
		i40e_flush(hw);
	}

	/* clear the irq settings */
	for (i = 0; i < msix_vf; i++) {
		/* format is same for both registers */
		if (0 == i)
			reg_idx = I40E_VPINT_LNKLST0(vf->vf_id);
		else
			reg_idx = I40E_VPINT_LNKLSTN(((msix_vf - 1) *
						      (vf->vf_id))
						     + (i - 1));
		reg = (I40E_VPINT_LNKLSTN_FIRSTQ_TYPE_MASK |
		       I40E_VPINT_LNKLSTN_FIRSTQ_INDX_MASK);
		wr32(hw, reg_idx, reg);
		i40e_flush(hw);
	}
	/* reset some of the state varibles keeping
	 * track of the resources
	 */
	vf->num_queue_pairs = 0;
	vf->vf_states = 0;
}

/**
 * i40e_alloc_vf_res
 * @vf: pointer to the VF info
 *
 * allocate VF resources
 **/
static int i40e_alloc_vf_res(struct i40e_vf *vf)
{
	struct i40e_pf *pf = vf->pf;
	int total_queue_pairs = 0;
	int ret;

	/* allocate hw vsi context & associated resources */
	ret = i40e_alloc_vsi_res(vf, I40E_VSI_SRIOV);
	if (ret)
		goto error_alloc;
	total_queue_pairs += pf->vsi[vf->lan_vsi_idx]->alloc_queue_pairs;
	set_bit(I40E_VIRTCHNL_VF_CAP_PRIVILEGE, &vf->vf_caps);

	/* store the total qps number for the runtime
	 * VF req validation
	 */
	vf->num_queue_pairs = total_queue_pairs;

	/* VF is now completely initialized */
	set_bit(I40E_VF_STAT_INIT, &vf->vf_states);

error_alloc:
	if (ret)
		i40e_free_vf_res(vf);

	return ret;
}

#define VF_DEVICE_STATUS 0xAA
#define VF_TRANS_PENDING_MASK 0x20
/**
 * i40e_quiesce_vf_pci
 * @vf: pointer to the VF structure
 *
 * Wait for VF PCI transactions to be cleared after reset. Returns -EIO
 * if the transactions never clear.
 **/
static int i40e_quiesce_vf_pci(struct i40e_vf *vf)
{
	struct i40e_pf *pf = vf->pf;
	struct i40e_hw *hw = &pf->hw;
	int vf_abs_id, i;
	u32 reg;

	vf_abs_id = vf->vf_id + hw->func_caps.vf_base_id;

	wr32(hw, I40E_PF_PCI_CIAA,
	     VF_DEVICE_STATUS | (vf_abs_id << I40E_PF_PCI_CIAA_VF_NUM_SHIFT));
	for (i = 0; i < 100; i++) {
		reg = rd32(hw, I40E_PF_PCI_CIAD);
		if ((reg & VF_TRANS_PENDING_MASK) == 0)
			return 0;
		udelay(1);
	}
	return -EIO;
}

/**
 * i40e_reset_vf
 * @vf: pointer to the VF structure
 * @flr: VFLR was issued or not
 *
 * reset the VF
 **/
void i40e_reset_vf(struct i40e_vf *vf, bool flr)
{
	struct i40e_pf *pf = vf->pf;
	struct i40e_hw *hw = &pf->hw;
	bool rsd = false;
	int i;
	u32 reg;

	if (test_and_set_bit(__I40E_VF_DISABLE, &pf->state))
		return;

	/* warn the VF */
	clear_bit(I40E_VF_STAT_ACTIVE, &vf->vf_states);

	/* In the case of a VFLR, the HW has already reset the VF and we
	 * just need to clean up, so don't hit the VFRTRIG register.
	 */
	if (!flr) {
		/* reset VF using VPGEN_VFRTRIG reg */
		reg = rd32(hw, I40E_VPGEN_VFRTRIG(vf->vf_id));
		reg |= I40E_VPGEN_VFRTRIG_VFSWR_MASK;
		wr32(hw, I40E_VPGEN_VFRTRIG(vf->vf_id), reg);
		i40e_flush(hw);
	}

	if (i40e_quiesce_vf_pci(vf))
		dev_err(&pf->pdev->dev, "VF %d PCI transactions stuck\n",
			vf->vf_id);

	/* poll VPGEN_VFRSTAT reg to make sure
	 * that reset is complete
	 */
	for (i = 0; i < 10; i++) {
		/* VF reset requires driver to first reset the VF and then
		 * poll the status register to make sure that the reset
		 * completed successfully. Due to internal HW FIFO flushes,
		 * we must wait 10ms before the register will be valid.
		 */
		usleep_range(10000, 20000);
		reg = rd32(hw, I40E_VPGEN_VFRSTAT(vf->vf_id));
		if (reg & I40E_VPGEN_VFRSTAT_VFRD_MASK) {
			rsd = true;
			break;
		}
	}

	if (flr)
		usleep_range(10000, 20000);

	if (!rsd)
		dev_err(&pf->pdev->dev, "VF reset check timeout on VF %d\n",
			vf->vf_id);
	wr32(hw, I40E_VFGEN_RSTAT1(vf->vf_id), I40E_VFR_COMPLETED);
	/* clear the reset bit in the VPGEN_VFRTRIG reg */
	reg = rd32(hw, I40E_VPGEN_VFRTRIG(vf->vf_id));
	reg &= ~I40E_VPGEN_VFRTRIG_VFSWR_MASK;
	wr32(hw, I40E_VPGEN_VFRTRIG(vf->vf_id), reg);

	/* On initial reset, we won't have any queues */
	if (vf->lan_vsi_idx == 0)
		goto complete_reset;

	i40e_vsi_control_rings(pf->vsi[vf->lan_vsi_idx], false);
complete_reset:
	/* reallocate VF resources to reset the VSI state */
	i40e_free_vf_res(vf);
	i40e_alloc_vf_res(vf);
	i40e_enable_vf_mappings(vf);
	set_bit(I40E_VF_STAT_ACTIVE, &vf->vf_states);

	/* tell the VF the reset is done */
	wr32(hw, I40E_VFGEN_RSTAT1(vf->vf_id), I40E_VFR_VFACTIVE);
	i40e_flush(hw);
	clear_bit(__I40E_VF_DISABLE, &pf->state);
}

/**
 * i40e_free_vfs
 * @pf: pointer to the PF structure
 *
 * free VF resources
 **/
void i40e_free_vfs(struct i40e_pf *pf)
{
	struct i40e_hw *hw = &pf->hw;
	u32 reg_idx, bit_idx;
	int i, tmp, vf_id;

	if (!pf->vf)
		return;
	while (test_and_set_bit(__I40E_VF_DISABLE, &pf->state))
		usleep_range(1000, 2000);

	for (i = 0; i < pf->num_alloc_vfs; i++)
		if (test_bit(I40E_VF_STAT_INIT, &pf->vf[i].vf_states))
			i40e_vsi_control_rings(pf->vsi[pf->vf[i].lan_vsi_idx],
					       false);

	/* Disable IOV before freeing resources. This lets any VF drivers
	 * running in the host get themselves cleaned up before we yank
	 * the carpet out from underneath their feet.
	 */
	if (!pci_vfs_assigned(pf->pdev))
		pci_disable_sriov(pf->pdev);
	else
		dev_warn(&pf->pdev->dev, "VFs are assigned - not disabling SR-IOV\n");

	msleep(20); /* let any messages in transit get finished up */

	/* free up VF resources */
	tmp = pf->num_alloc_vfs;
	pf->num_alloc_vfs = 0;
	for (i = 0; i < tmp; i++) {
		if (test_bit(I40E_VF_STAT_INIT, &pf->vf[i].vf_states))
			i40e_free_vf_res(&pf->vf[i]);
		/* disable qp mappings */
		i40e_disable_vf_mappings(&pf->vf[i]);
	}

	kfree(pf->vf);
	pf->vf = NULL;

	/* This check is for when the driver is unloaded while VFs are
	 * assigned. Setting the number of VFs to 0 through sysfs is caught
	 * before this function ever gets called.
	 */
	if (!pci_vfs_assigned(pf->pdev)) {
		/* Acknowledge VFLR for all VFS. Without this, VFs will fail to
		 * work correctly when SR-IOV gets re-enabled.
		 */
		for (vf_id = 0; vf_id < tmp; vf_id++) {
			reg_idx = (hw->func_caps.vf_base_id + vf_id) / 32;
			bit_idx = (hw->func_caps.vf_base_id + vf_id) % 32;
			wr32(hw, I40E_GLGEN_VFLRSTAT(reg_idx), (1 << bit_idx));
		}
	}
	clear_bit(__I40E_VF_DISABLE, &pf->state);
}

#ifdef CONFIG_PCI_IOV
/**
 * i40e_alloc_vfs
 * @pf: pointer to the PF structure
 * @num_alloc_vfs: number of VFs to allocate
 *
 * allocate VF resources
 **/
int i40e_alloc_vfs(struct i40e_pf *pf, u16 num_alloc_vfs)
{
	struct i40e_vf *vfs;
	int i, ret = 0;

	/* Disable interrupt 0 so we don't try to handle the VFLR. */
	i40e_irq_dynamic_disable_icr0(pf);

	/* Check to see if we're just allocating resources for extant VFs */
	if (pci_num_vf(pf->pdev) != num_alloc_vfs) {
		ret = pci_enable_sriov(pf->pdev, num_alloc_vfs);
		if (ret) {
			dev_err(&pf->pdev->dev,
				"Failed to enable SR-IOV, error %d.\n", ret);
			pf->num_alloc_vfs = 0;
			goto err_iov;
		}
	}
	/* allocate memory */
	vfs = kcalloc(num_alloc_vfs, sizeof(struct i40e_vf), GFP_KERNEL);
	if (!vfs) {
		ret = -ENOMEM;
		goto err_alloc;
	}
	pf->vf = vfs;

	/* apply default profile */
	for (i = 0; i < num_alloc_vfs; i++) {
		vfs[i].pf = pf;
		vfs[i].parent_type = I40E_SWITCH_ELEMENT_TYPE_VEB;
		vfs[i].vf_id = i;

		/* assign default capabilities */
		set_bit(I40E_VIRTCHNL_VF_CAP_L2, &vfs[i].vf_caps);
		vfs[i].spoofchk = true;
		/* VF resources get allocated during reset */
		i40e_reset_vf(&vfs[i], false);

		/* enable VF vplan_qtable mappings */
		i40e_enable_vf_mappings(&vfs[i]);
	}
	pf->num_alloc_vfs = num_alloc_vfs;

err_alloc:
	if (ret)
		i40e_free_vfs(pf);
err_iov:
	/* Re-enable interrupt 0. */
	i40e_irq_dynamic_enable_icr0(pf);
	return ret;
}

#endif
/**
 * i40e_pci_sriov_enable
 * @pdev: pointer to a pci_dev structure
 * @num_vfs: number of VFs to allocate
 *
 * Enable or change the number of VFs
 **/
static int i40e_pci_sriov_enable(struct pci_dev *pdev, int num_vfs)
{
#ifdef CONFIG_PCI_IOV
	struct i40e_pf *pf = pci_get_drvdata(pdev);
	int pre_existing_vfs = pci_num_vf(pdev);
	int err = 0;

	dev_info(&pdev->dev, "Allocating %d VFs.\n", num_vfs);
	if (pre_existing_vfs && pre_existing_vfs != num_vfs)
		i40e_free_vfs(pf);
	else if (pre_existing_vfs && pre_existing_vfs == num_vfs)
		goto out;

	if (num_vfs > pf->num_req_vfs) {
		err = -EPERM;
		goto err_out;
	}

	err = i40e_alloc_vfs(pf, num_vfs);
	if (err) {
		dev_warn(&pdev->dev, "Failed to enable SR-IOV: %d\n", err);
		goto err_out;
	}

out:
	return num_vfs;

err_out:
	return err;
#endif
	return 0;
}

/**
 * i40e_pci_sriov_configure
 * @pdev: pointer to a pci_dev structure
 * @num_vfs: number of VFs to allocate
 *
 * Enable or change the number of VFs. Called when the user updates the number
 * of VFs in sysfs.
 **/
int i40e_pci_sriov_configure(struct pci_dev *pdev, int num_vfs)
{
	struct i40e_pf *pf = pci_get_drvdata(pdev);

	if (num_vfs) {
		if (!(pf->flags & I40E_FLAG_VEB_MODE_ENABLED)) {
			pf->flags |= I40E_FLAG_VEB_MODE_ENABLED;
			i40e_do_reset_safe(pf,
					   BIT_ULL(__I40E_PF_RESET_REQUESTED));
		}
		return i40e_pci_sriov_enable(pdev, num_vfs);
	}

	if (!pci_vfs_assigned(pf->pdev)) {
		i40e_free_vfs(pf);
		pf->flags &= ~I40E_FLAG_VEB_MODE_ENABLED;
		i40e_do_reset_safe(pf, BIT_ULL(__I40E_PF_RESET_REQUESTED));
	} else {
		dev_warn(&pdev->dev, "Unable to free VFs because some are assigned to VMs.\n");
		return -EINVAL;
	}
	return 0;
}

/***********************virtual channel routines******************/

/**
 * i40e_vc_send_msg_to_vf
 * @vf: pointer to the VF info
 * @v_opcode: virtual channel opcode
 * @v_retval: virtual channel return value
 * @msg: pointer to the msg buffer
 * @msglen: msg length
 *
 * send msg to VF
 **/
static int i40e_vc_send_msg_to_vf(struct i40e_vf *vf, u32 v_opcode,
				  u32 v_retval, u8 *msg, u16 msglen)
{
	struct i40e_pf *pf;
	struct i40e_hw *hw;
	int abs_vf_id;
	i40e_status aq_ret;

	/* validate the request */
	if (!vf || vf->vf_id >= vf->pf->num_alloc_vfs)
		return -EINVAL;

	pf = vf->pf;
	hw = &pf->hw;
	abs_vf_id = vf->vf_id + hw->func_caps.vf_base_id;

	/* single place to detect unsuccessful return values */
	if (v_retval) {
		vf->num_invalid_msgs++;
		dev_err(&pf->pdev->dev, "Failed opcode %d Error: %d\n",
			v_opcode, v_retval);
		if (vf->num_invalid_msgs >
		    I40E_DEFAULT_NUM_INVALID_MSGS_ALLOWED) {
			dev_err(&pf->pdev->dev,
				"Number of invalid messages exceeded for VF %d\n",
				vf->vf_id);
			dev_err(&pf->pdev->dev, "Use PF Control I/F to enable the VF\n");
			set_bit(I40E_VF_STAT_DISABLED, &vf->vf_states);
		}
	} else {
		vf->num_valid_msgs++;
	}

	aq_ret = i40e_aq_send_msg_to_vf(hw, abs_vf_id,	v_opcode, v_retval,
					msg, msglen, NULL);
	if (aq_ret) {
		dev_err(&pf->pdev->dev,
			"Unable to send the message to VF %d aq_err %d\n",
			vf->vf_id, pf->hw.aq.asq_last_status);
		return -EIO;
	}

	return 0;
}

/**
 * i40e_vc_send_resp_to_vf
 * @vf: pointer to the VF info
 * @opcode: operation code
 * @retval: return value
 *
 * send resp msg to VF
 **/
static int i40e_vc_send_resp_to_vf(struct i40e_vf *vf,
				   enum i40e_virtchnl_ops opcode,
				   i40e_status retval)
{
	return i40e_vc_send_msg_to_vf(vf, opcode, retval, NULL, 0);
}

/**
 * i40e_vc_get_version_msg
 * @vf: pointer to the VF info
 *
 * called from the VF to request the API version used by the PF
 **/
static int i40e_vc_get_version_msg(struct i40e_vf *vf)
{
	struct i40e_virtchnl_version_info info = {
		I40E_VIRTCHNL_VERSION_MAJOR, I40E_VIRTCHNL_VERSION_MINOR
	};

	return i40e_vc_send_msg_to_vf(vf, I40E_VIRTCHNL_OP_VERSION,
				      I40E_SUCCESS, (u8 *)&info,
				      sizeof(struct
					     i40e_virtchnl_version_info));
}

/**
 * i40e_vc_get_vf_resources_msg
 * @vf: pointer to the VF info
 * @msg: pointer to the msg buffer
 * @msglen: msg length
 *
 * called from the VF to request its resources
 **/
static int i40e_vc_get_vf_resources_msg(struct i40e_vf *vf)
{
	struct i40e_virtchnl_vf_resource *vfres = NULL;
	struct i40e_pf *pf = vf->pf;
	i40e_status aq_ret = 0;
	struct i40e_vsi *vsi;
	int i = 0, len = 0;
	int num_vsis = 1;
	int ret;

	if (!test_bit(I40E_VF_STAT_INIT, &vf->vf_states)) {
		aq_ret = I40E_ERR_PARAM;
		goto err;
	}

	len = (sizeof(struct i40e_virtchnl_vf_resource) +
	       sizeof(struct i40e_virtchnl_vsi_resource) * num_vsis);

	vfres = kzalloc(len, GFP_KERNEL);
	if (!vfres) {
		aq_ret = I40E_ERR_NO_MEMORY;
		len = 0;
		goto err;
	}

	vfres->vf_offload_flags = I40E_VIRTCHNL_VF_OFFLOAD_L2;
	vsi = pf->vsi[vf->lan_vsi_idx];
	if (!vsi->info.pvid)
		vfres->vf_offload_flags |= I40E_VIRTCHNL_VF_OFFLOAD_VLAN;

	vfres->num_vsis = num_vsis;
	vfres->num_queue_pairs = vf->num_queue_pairs;
	vfres->max_vectors = pf->hw.func_caps.num_msix_vectors_vf;
	if (vf->lan_vsi_idx) {
		vfres->vsi_res[i].vsi_id = vf->lan_vsi_id;
		vfres->vsi_res[i].vsi_type = I40E_VSI_SRIOV;
		vfres->vsi_res[i].num_queue_pairs =
		    pf->vsi[vf->lan_vsi_idx]->alloc_queue_pairs;
		memcpy(vfres->vsi_res[i].default_mac_addr,
		       vf->default_lan_addr.addr, ETH_ALEN);
		i++;
	}
	set_bit(I40E_VF_STAT_ACTIVE, &vf->vf_states);

err:
	/* send the response back to the VF */
	ret = i40e_vc_send_msg_to_vf(vf, I40E_VIRTCHNL_OP_GET_VF_RESOURCES,
				     aq_ret, (u8 *)vfres, len);

	kfree(vfres);
	return ret;
}

/**
 * i40e_vc_reset_vf_msg
 * @vf: pointer to the VF info
 * @msg: pointer to the msg buffer
 * @msglen: msg length
 *
 * called from the VF to reset itself,
 * unlike other virtchnl messages, PF driver
 * doesn't send the response back to the VF
 **/
static void i40e_vc_reset_vf_msg(struct i40e_vf *vf)
{
	if (test_bit(I40E_VF_STAT_ACTIVE, &vf->vf_states))
		i40e_reset_vf(vf, false);
}

/**
 * i40e_vc_config_promiscuous_mode_msg
 * @vf: pointer to the VF info
 * @msg: pointer to the msg buffer
 * @msglen: msg length
 *
 * called from the VF to configure the promiscuous mode of
 * VF vsis
 **/
static int i40e_vc_config_promiscuous_mode_msg(struct i40e_vf *vf,
					       u8 *msg, u16 msglen)
{
	struct i40e_virtchnl_promisc_info *info =
	    (struct i40e_virtchnl_promisc_info *)msg;
	struct i40e_pf *pf = vf->pf;
	struct i40e_hw *hw = &pf->hw;
	struct i40e_vsi *vsi;
	bool allmulti = false;
	i40e_status aq_ret;

	vsi = i40e_find_vsi_from_id(pf, info->vsi_id);
	if (!test_bit(I40E_VF_STAT_ACTIVE, &vf->vf_states) ||
	    !test_bit(I40E_VIRTCHNL_VF_CAP_PRIVILEGE, &vf->vf_caps) ||
	    !i40e_vc_isvalid_vsi_id(vf, info->vsi_id) ||
	    (vsi->type != I40E_VSI_FCOE)) {
		aq_ret = I40E_ERR_PARAM;
		goto error_param;
	}
	if (info->flags & I40E_FLAG_VF_MULTICAST_PROMISC)
		allmulti = true;
	aq_ret = i40e_aq_set_vsi_multicast_promiscuous(hw, vsi->seid,
						       allmulti, NULL);

error_param:
	/* send the response to the VF */
	return i40e_vc_send_resp_to_vf(vf,
				       I40E_VIRTCHNL_OP_CONFIG_PROMISCUOUS_MODE,
				       aq_ret);
}

/**
 * i40e_vc_config_queues_msg
 * @vf: pointer to the VF info
 * @msg: pointer to the msg buffer
 * @msglen: msg length
 *
 * called from the VF to configure the rx/tx
 * queues
 **/
static int i40e_vc_config_queues_msg(struct i40e_vf *vf, u8 *msg, u16 msglen)
{
	struct i40e_virtchnl_vsi_queue_config_info *qci =
	    (struct i40e_virtchnl_vsi_queue_config_info *)msg;
	struct i40e_virtchnl_queue_pair_info *qpi;
	struct i40e_pf *pf = vf->pf;
	u16 vsi_id, vsi_queue_id;
	i40e_status aq_ret = 0;
	int i;

	if (!test_bit(I40E_VF_STAT_ACTIVE, &vf->vf_states)) {
		aq_ret = I40E_ERR_PARAM;
		goto error_param;
	}

	vsi_id = qci->vsi_id;
	if (!i40e_vc_isvalid_vsi_id(vf, vsi_id)) {
		aq_ret = I40E_ERR_PARAM;
		goto error_param;
	}
	for (i = 0; i < qci->num_queue_pairs; i++) {
		qpi = &qci->qpair[i];
		vsi_queue_id = qpi->txq.queue_id;
		if ((qpi->txq.vsi_id != vsi_id) ||
		    (qpi->rxq.vsi_id != vsi_id) ||
		    (qpi->rxq.queue_id != vsi_queue_id) ||
		    !i40e_vc_isvalid_queue_id(vf, vsi_id, vsi_queue_id)) {
			aq_ret = I40E_ERR_PARAM;
			goto error_param;
		}

		if (i40e_config_vsi_rx_queue(vf, vsi_id, vsi_queue_id,
					     &qpi->rxq) ||
		    i40e_config_vsi_tx_queue(vf, vsi_id, vsi_queue_id,
					     &qpi->txq)) {
			aq_ret = I40E_ERR_PARAM;
			goto error_param;
		}
	}
	/* set vsi num_queue_pairs in use to num configured by VF */
	pf->vsi[vf->lan_vsi_idx]->num_queue_pairs = qci->num_queue_pairs;

error_param:
	/* send the response to the VF */
	return i40e_vc_send_resp_to_vf(vf, I40E_VIRTCHNL_OP_CONFIG_VSI_QUEUES,
				       aq_ret);
}

/**
 * i40e_vc_config_irq_map_msg
 * @vf: pointer to the VF info
 * @msg: pointer to the msg buffer
 * @msglen: msg length
 *
 * called from the VF to configure the irq to
 * queue map
 **/
static int i40e_vc_config_irq_map_msg(struct i40e_vf *vf, u8 *msg, u16 msglen)
{
	struct i40e_virtchnl_irq_map_info *irqmap_info =
	    (struct i40e_virtchnl_irq_map_info *)msg;
	struct i40e_virtchnl_vector_map *map;
	u16 vsi_id, vsi_queue_id, vector_id;
	i40e_status aq_ret = 0;
	unsigned long tempmap;
	int i;

	if (!test_bit(I40E_VF_STAT_ACTIVE, &vf->vf_states)) {
		aq_ret = I40E_ERR_PARAM;
		goto error_param;
	}

	for (i = 0; i < irqmap_info->num_vectors; i++) {
		map = &irqmap_info->vecmap[i];

		vector_id = map->vector_id;
		vsi_id = map->vsi_id;
		/* validate msg params */
		if (!i40e_vc_isvalid_vector_id(vf, vector_id) ||
		    !i40e_vc_isvalid_vsi_id(vf, vsi_id)) {
			aq_ret = I40E_ERR_PARAM;
			goto error_param;
		}

		/* lookout for the invalid queue index */
		tempmap = map->rxq_map;
		for_each_set_bit(vsi_queue_id, &tempmap, I40E_MAX_VSI_QP) {
			if (!i40e_vc_isvalid_queue_id(vf, vsi_id,
						      vsi_queue_id)) {
				aq_ret = I40E_ERR_PARAM;
				goto error_param;
			}
		}

		tempmap = map->txq_map;
		for_each_set_bit(vsi_queue_id, &tempmap, I40E_MAX_VSI_QP) {
			if (!i40e_vc_isvalid_queue_id(vf, vsi_id,
						      vsi_queue_id)) {
				aq_ret = I40E_ERR_PARAM;
				goto error_param;
			}
		}

		i40e_config_irq_link_list(vf, vsi_id, map);
	}
error_param:
	/* send the response to the VF */
	return i40e_vc_send_resp_to_vf(vf, I40E_VIRTCHNL_OP_CONFIG_IRQ_MAP,
				       aq_ret);
}

/**
 * i40e_vc_enable_queues_msg
 * @vf: pointer to the VF info
 * @msg: pointer to the msg buffer
 * @msglen: msg length
 *
 * called from the VF to enable all or specific queue(s)
 **/
static int i40e_vc_enable_queues_msg(struct i40e_vf *vf, u8 *msg, u16 msglen)
{
	struct i40e_virtchnl_queue_select *vqs =
	    (struct i40e_virtchnl_queue_select *)msg;
	struct i40e_pf *pf = vf->pf;
	u16 vsi_id = vqs->vsi_id;
	i40e_status aq_ret = 0;

	if (!test_bit(I40E_VF_STAT_ACTIVE, &vf->vf_states)) {
		aq_ret = I40E_ERR_PARAM;
		goto error_param;
	}

	if (!i40e_vc_isvalid_vsi_id(vf, vsi_id)) {
		aq_ret = I40E_ERR_PARAM;
		goto error_param;
	}

	if ((0 == vqs->rx_queues) && (0 == vqs->tx_queues)) {
		aq_ret = I40E_ERR_PARAM;
		goto error_param;
	}

	if (i40e_vsi_control_rings(pf->vsi[vf->lan_vsi_idx], true))
		aq_ret = I40E_ERR_TIMEOUT;
error_param:
	/* send the response to the VF */
	return i40e_vc_send_resp_to_vf(vf, I40E_VIRTCHNL_OP_ENABLE_QUEUES,
				       aq_ret);
}

/**
 * i40e_vc_disable_queues_msg
 * @vf: pointer to the VF info
 * @msg: pointer to the msg buffer
 * @msglen: msg length
 *
 * called from the VF to disable all or specific
 * queue(s)
 **/
static int i40e_vc_disable_queues_msg(struct i40e_vf *vf, u8 *msg, u16 msglen)
{
	struct i40e_virtchnl_queue_select *vqs =
	    (struct i40e_virtchnl_queue_select *)msg;
	struct i40e_pf *pf = vf->pf;
	i40e_status aq_ret = 0;

	if (!test_bit(I40E_VF_STAT_ACTIVE, &vf->vf_states)) {
		aq_ret = I40E_ERR_PARAM;
		goto error_param;
	}

	if (!i40e_vc_isvalid_vsi_id(vf, vqs->vsi_id)) {
		aq_ret = I40E_ERR_PARAM;
		goto error_param;
	}

	if ((0 == vqs->rx_queues) && (0 == vqs->tx_queues)) {
		aq_ret = I40E_ERR_PARAM;
		goto error_param;
	}

	if (i40e_vsi_control_rings(pf->vsi[vf->lan_vsi_idx], false))
		aq_ret = I40E_ERR_TIMEOUT;

error_param:
	/* send the response to the VF */
	return i40e_vc_send_resp_to_vf(vf, I40E_VIRTCHNL_OP_DISABLE_QUEUES,
				       aq_ret);
}

/**
 * i40e_vc_get_stats_msg
 * @vf: pointer to the VF info
 * @msg: pointer to the msg buffer
 * @msglen: msg length
 *
 * called from the VF to get vsi stats
 **/
static int i40e_vc_get_stats_msg(struct i40e_vf *vf, u8 *msg, u16 msglen)
{
	struct i40e_virtchnl_queue_select *vqs =
	    (struct i40e_virtchnl_queue_select *)msg;
	struct i40e_pf *pf = vf->pf;
	struct i40e_eth_stats stats;
	i40e_status aq_ret = 0;
	struct i40e_vsi *vsi;

	memset(&stats, 0, sizeof(struct i40e_eth_stats));

	if (!test_bit(I40E_VF_STAT_ACTIVE, &vf->vf_states)) {
		aq_ret = I40E_ERR_PARAM;
		goto error_param;
	}

	if (!i40e_vc_isvalid_vsi_id(vf, vqs->vsi_id)) {
		aq_ret = I40E_ERR_PARAM;
		goto error_param;
	}

	vsi = pf->vsi[vf->lan_vsi_idx];
	if (!vsi) {
		aq_ret = I40E_ERR_PARAM;
		goto error_param;
	}
	i40e_update_eth_stats(vsi);
	stats = vsi->eth_stats;

error_param:
	/* send the response back to the VF */
	return i40e_vc_send_msg_to_vf(vf, I40E_VIRTCHNL_OP_GET_STATS, aq_ret,
				      (u8 *)&stats, sizeof(stats));
}

/**
 * i40e_check_vf_permission
 * @vf: pointer to the VF info
 * @macaddr: pointer to the MAC Address being checked
 *
 * Check if the VF has permission to add or delete unicast MAC address
 * filters and return error code -EPERM if not.  Then check if the
 * address filter requested is broadcast or zero and if so return
 * an invalid MAC address error code.
 **/
static inline int i40e_check_vf_permission(struct i40e_vf *vf, u8 *macaddr)
{
	struct i40e_pf *pf = vf->pf;
	int ret = 0;

	if (is_broadcast_ether_addr(macaddr) ||
		   is_zero_ether_addr(macaddr)) {
		dev_err(&pf->pdev->dev, "invalid VF MAC addr %pM\n", macaddr);
		ret = I40E_ERR_INVALID_MAC_ADDR;
	} else if (vf->pf_set_mac && !is_multicast_ether_addr(macaddr) &&
		   !ether_addr_equal(macaddr, vf->default_lan_addr.addr)) {
		/* If the host VMM administrator has set the VF MAC address
		 * administratively via the ndo_set_vf_mac command then deny
		 * permission to the VF to add or delete unicast MAC addresses.
		 * The VF may request to set the MAC address filter already
		 * assigned to it so do not return an error in that case.
		 */
		dev_err(&pf->pdev->dev,
			"VF attempting to override administratively set MAC address\nPlease reload the VF driver to resume normal operation\n");
		ret = -EPERM;
	}
	return ret;
}

/**
 * i40e_vc_add_mac_addr_msg
 * @vf: pointer to the VF info
 * @msg: pointer to the msg buffer
 * @msglen: msg length
 *
 * add guest mac address filter
 **/
static int i40e_vc_add_mac_addr_msg(struct i40e_vf *vf, u8 *msg, u16 msglen)
{
	struct i40e_virtchnl_ether_addr_list *al =
	    (struct i40e_virtchnl_ether_addr_list *)msg;
	struct i40e_pf *pf = vf->pf;
	struct i40e_vsi *vsi = NULL;
	u16 vsi_id = al->vsi_id;
	i40e_status ret = 0;
	int i;

	if (!test_bit(I40E_VF_STAT_ACTIVE, &vf->vf_states) ||
	    !test_bit(I40E_VIRTCHNL_VF_CAP_PRIVILEGE, &vf->vf_caps) ||
	    !i40e_vc_isvalid_vsi_id(vf, vsi_id)) {
		ret = I40E_ERR_PARAM;
		goto error_param;
	}

	for (i = 0; i < al->num_elements; i++) {
		ret = i40e_check_vf_permission(vf, al->list[i].addr);
		if (ret)
			goto error_param;
	}
	vsi = pf->vsi[vf->lan_vsi_idx];

	/* add new addresses to the list */
	for (i = 0; i < al->num_elements; i++) {
		struct i40e_mac_filter *f;

		f = i40e_find_mac(vsi, al->list[i].addr, true, false);
		if (!f) {
			if (i40e_is_vsi_in_vlan(vsi))
				f = i40e_put_mac_in_vlan(vsi, al->list[i].addr,
							 true, false);
			else
				f = i40e_add_filter(vsi, al->list[i].addr, -1,
						    true, false);
		}

		if (!f) {
			dev_err(&pf->pdev->dev,
				"Unable to add VF MAC filter\n");
			ret = I40E_ERR_PARAM;
			goto error_param;
		}
	}

	/* program the updated filter list */
	if (i40e_sync_vsi_filters(vsi))
		dev_err(&pf->pdev->dev, "Unable to program VF MAC filters\n");

error_param:
	/* send the response to the VF */
	return i40e_vc_send_resp_to_vf(vf, I40E_VIRTCHNL_OP_ADD_ETHER_ADDRESS,
				       ret);
}

/**
 * i40e_vc_del_mac_addr_msg
 * @vf: pointer to the VF info
 * @msg: pointer to the msg buffer
 * @msglen: msg length
 *
 * remove guest mac address filter
 **/
static int i40e_vc_del_mac_addr_msg(struct i40e_vf *vf, u8 *msg, u16 msglen)
{
	struct i40e_virtchnl_ether_addr_list *al =
	    (struct i40e_virtchnl_ether_addr_list *)msg;
	struct i40e_pf *pf = vf->pf;
	struct i40e_vsi *vsi = NULL;
	u16 vsi_id = al->vsi_id;
	i40e_status ret = 0;
	int i;

	if (!test_bit(I40E_VF_STAT_ACTIVE, &vf->vf_states) ||
	    !test_bit(I40E_VIRTCHNL_VF_CAP_PRIVILEGE, &vf->vf_caps) ||
	    !i40e_vc_isvalid_vsi_id(vf, vsi_id)) {
		ret = I40E_ERR_PARAM;
		goto error_param;
	}

	for (i = 0; i < al->num_elements; i++) {
		if (is_broadcast_ether_addr(al->list[i].addr) ||
		    is_zero_ether_addr(al->list[i].addr)) {
			dev_err(&pf->pdev->dev, "invalid VF MAC addr %pM\n",
				al->list[i].addr);
			ret = I40E_ERR_INVALID_MAC_ADDR;
			goto error_param;
		}
	}
	vsi = pf->vsi[vf->lan_vsi_idx];

	/* delete addresses from the list */
	for (i = 0; i < al->num_elements; i++)
		i40e_del_filter(vsi, al->list[i].addr,
				I40E_VLAN_ANY, true, false);

	/* program the updated filter list */
	if (i40e_sync_vsi_filters(vsi))
		dev_err(&pf->pdev->dev, "Unable to program VF MAC filters\n");

error_param:
	/* send the response to the VF */
	return i40e_vc_send_resp_to_vf(vf, I40E_VIRTCHNL_OP_DEL_ETHER_ADDRESS,
				       ret);
}

/**
 * i40e_vc_add_vlan_msg
 * @vf: pointer to the VF info
 * @msg: pointer to the msg buffer
 * @msglen: msg length
 *
 * program guest vlan id
 **/
static int i40e_vc_add_vlan_msg(struct i40e_vf *vf, u8 *msg, u16 msglen)
{
	struct i40e_virtchnl_vlan_filter_list *vfl =
	    (struct i40e_virtchnl_vlan_filter_list *)msg;
	struct i40e_pf *pf = vf->pf;
	struct i40e_vsi *vsi = NULL;
	u16 vsi_id = vfl->vsi_id;
	i40e_status aq_ret = 0;
	int i;

	if (!test_bit(I40E_VF_STAT_ACTIVE, &vf->vf_states) ||
	    !test_bit(I40E_VIRTCHNL_VF_CAP_PRIVILEGE, &vf->vf_caps) ||
	    !i40e_vc_isvalid_vsi_id(vf, vsi_id)) {
		aq_ret = I40E_ERR_PARAM;
		goto error_param;
	}

	for (i = 0; i < vfl->num_elements; i++) {
		if (vfl->vlan_id[i] > I40E_MAX_VLANID) {
			aq_ret = I40E_ERR_PARAM;
			dev_err(&pf->pdev->dev,
				"invalid VF VLAN id %d\n", vfl->vlan_id[i]);
			goto error_param;
		}
	}
	vsi = pf->vsi[vf->lan_vsi_idx];
	if (vsi->info.pvid) {
		aq_ret = I40E_ERR_PARAM;
		goto error_param;
	}

	i40e_vlan_stripping_enable(vsi);
	for (i = 0; i < vfl->num_elements; i++) {
		/* add new VLAN filter */
		int ret = i40e_vsi_add_vlan(vsi, vfl->vlan_id[i]);
		if (ret)
			dev_err(&pf->pdev->dev,
				"Unable to add VF vlan filter %d, error %d\n",
				vfl->vlan_id[i], ret);
	}

error_param:
	/* send the response to the VF */
	return i40e_vc_send_resp_to_vf(vf, I40E_VIRTCHNL_OP_ADD_VLAN, aq_ret);
}

/**
 * i40e_vc_remove_vlan_msg
 * @vf: pointer to the VF info
 * @msg: pointer to the msg buffer
 * @msglen: msg length
 *
 * remove programmed guest vlan id
 **/
static int i40e_vc_remove_vlan_msg(struct i40e_vf *vf, u8 *msg, u16 msglen)
{
	struct i40e_virtchnl_vlan_filter_list *vfl =
	    (struct i40e_virtchnl_vlan_filter_list *)msg;
	struct i40e_pf *pf = vf->pf;
	struct i40e_vsi *vsi = NULL;
	u16 vsi_id = vfl->vsi_id;
	i40e_status aq_ret = 0;
	int i;

	if (!test_bit(I40E_VF_STAT_ACTIVE, &vf->vf_states) ||
	    !test_bit(I40E_VIRTCHNL_VF_CAP_PRIVILEGE, &vf->vf_caps) ||
	    !i40e_vc_isvalid_vsi_id(vf, vsi_id)) {
		aq_ret = I40E_ERR_PARAM;
		goto error_param;
	}

	for (i = 0; i < vfl->num_elements; i++) {
		if (vfl->vlan_id[i] > I40E_MAX_VLANID) {
			aq_ret = I40E_ERR_PARAM;
			goto error_param;
		}
	}

	vsi = pf->vsi[vf->lan_vsi_idx];
	if (vsi->info.pvid) {
		aq_ret = I40E_ERR_PARAM;
		goto error_param;
	}

	for (i = 0; i < vfl->num_elements; i++) {
		int ret = i40e_vsi_kill_vlan(vsi, vfl->vlan_id[i]);
		if (ret)
			dev_err(&pf->pdev->dev,
				"Unable to delete VF vlan filter %d, error %d\n",
				vfl->vlan_id[i], ret);
	}

error_param:
	/* send the response to the VF */
	return i40e_vc_send_resp_to_vf(vf, I40E_VIRTCHNL_OP_DEL_VLAN, aq_ret);
}

/**
 * i40e_vc_validate_vf_msg
 * @vf: pointer to the VF info
 * @msg: pointer to the msg buffer
 * @msglen: msg length
 * @msghndl: msg handle
 *
 * validate msg
 **/
static int i40e_vc_validate_vf_msg(struct i40e_vf *vf, u32 v_opcode,
				   u32 v_retval, u8 *msg, u16 msglen)
{
	bool err_msg_format = false;
	int valid_len;

	/* Check if VF is disabled. */
	if (test_bit(I40E_VF_STAT_DISABLED, &vf->vf_states))
		return I40E_ERR_PARAM;

	/* Validate message length. */
	switch (v_opcode) {
	case I40E_VIRTCHNL_OP_VERSION:
		valid_len = sizeof(struct i40e_virtchnl_version_info);
		break;
	case I40E_VIRTCHNL_OP_RESET_VF:
	case I40E_VIRTCHNL_OP_GET_VF_RESOURCES:
		valid_len = 0;
		break;
	case I40E_VIRTCHNL_OP_CONFIG_TX_QUEUE:
		valid_len = sizeof(struct i40e_virtchnl_txq_info);
		break;
	case I40E_VIRTCHNL_OP_CONFIG_RX_QUEUE:
		valid_len = sizeof(struct i40e_virtchnl_rxq_info);
		break;
	case I40E_VIRTCHNL_OP_CONFIG_VSI_QUEUES:
		valid_len = sizeof(struct i40e_virtchnl_vsi_queue_config_info);
		if (msglen >= valid_len) {
			struct i40e_virtchnl_vsi_queue_config_info *vqc =
			    (struct i40e_virtchnl_vsi_queue_config_info *)msg;
			valid_len += (vqc->num_queue_pairs *
				      sizeof(struct
					     i40e_virtchnl_queue_pair_info));
			if (vqc->num_queue_pairs == 0)
				err_msg_format = true;
		}
		break;
	case I40E_VIRTCHNL_OP_CONFIG_IRQ_MAP:
		valid_len = sizeof(struct i40e_virtchnl_irq_map_info);
		if (msglen >= valid_len) {
			struct i40e_virtchnl_irq_map_info *vimi =
			    (struct i40e_virtchnl_irq_map_info *)msg;
			valid_len += (vimi->num_vectors *
				      sizeof(struct i40e_virtchnl_vector_map));
			if (vimi->num_vectors == 0)
				err_msg_format = true;
		}
		break;
	case I40E_VIRTCHNL_OP_ENABLE_QUEUES:
	case I40E_VIRTCHNL_OP_DISABLE_QUEUES:
		valid_len = sizeof(struct i40e_virtchnl_queue_select);
		break;
	case I40E_VIRTCHNL_OP_ADD_ETHER_ADDRESS:
	case I40E_VIRTCHNL_OP_DEL_ETHER_ADDRESS:
		valid_len = sizeof(struct i40e_virtchnl_ether_addr_list);
		if (msglen >= valid_len) {
			struct i40e_virtchnl_ether_addr_list *veal =
			    (struct i40e_virtchnl_ether_addr_list *)msg;
			valid_len += veal->num_elements *
			    sizeof(struct i40e_virtchnl_ether_addr);
			if (veal->num_elements == 0)
				err_msg_format = true;
		}
		break;
	case I40E_VIRTCHNL_OP_ADD_VLAN:
	case I40E_VIRTCHNL_OP_DEL_VLAN:
		valid_len = sizeof(struct i40e_virtchnl_vlan_filter_list);
		if (msglen >= valid_len) {
			struct i40e_virtchnl_vlan_filter_list *vfl =
			    (struct i40e_virtchnl_vlan_filter_list *)msg;
			valid_len += vfl->num_elements * sizeof(u16);
			if (vfl->num_elements == 0)
				err_msg_format = true;
		}
		break;
	case I40E_VIRTCHNL_OP_CONFIG_PROMISCUOUS_MODE:
		valid_len = sizeof(struct i40e_virtchnl_promisc_info);
		break;
	case I40E_VIRTCHNL_OP_GET_STATS:
		valid_len = sizeof(struct i40e_virtchnl_queue_select);
		break;
	/* These are always errors coming from the VF. */
	case I40E_VIRTCHNL_OP_EVENT:
	case I40E_VIRTCHNL_OP_UNKNOWN:
	default:
		return -EPERM;
		break;
	}
	/* few more checks */
	if ((valid_len != msglen) || (err_msg_format)) {
		i40e_vc_send_resp_to_vf(vf, v_opcode, I40E_ERR_PARAM);
		return -EINVAL;
	} else {
		return 0;
	}
}

/**
 * i40e_vc_process_vf_msg
 * @pf: pointer to the PF structure
 * @vf_id: source VF id
 * @msg: pointer to the msg buffer
 * @msglen: msg length
 * @msghndl: msg handle
 *
 * called from the common aeq/arq handler to
 * process request from VF
 **/
int i40e_vc_process_vf_msg(struct i40e_pf *pf, u16 vf_id, u32 v_opcode,
			   u32 v_retval, u8 *msg, u16 msglen)
{
	struct i40e_hw *hw = &pf->hw;
	unsigned int local_vf_id = vf_id - hw->func_caps.vf_base_id;
	struct i40e_vf *vf;
	int ret;

	pf->vf_aq_requests++;
	if (local_vf_id >= pf->num_alloc_vfs)
		return -EINVAL;
	vf = &(pf->vf[local_vf_id]);
	/* perform basic checks on the msg */
	ret = i40e_vc_validate_vf_msg(vf, v_opcode, v_retval, msg, msglen);

	if (ret) {
		dev_err(&pf->pdev->dev, "Invalid message from VF %d, opcode %d, len %d\n",
			local_vf_id, v_opcode, msglen);
		return ret;
	}

	switch (v_opcode) {
	case I40E_VIRTCHNL_OP_VERSION:
		ret = i40e_vc_get_version_msg(vf);
		break;
	case I40E_VIRTCHNL_OP_GET_VF_RESOURCES:
		ret = i40e_vc_get_vf_resources_msg(vf);
		break;
	case I40E_VIRTCHNL_OP_RESET_VF:
		i40e_vc_reset_vf_msg(vf);
		ret = 0;
		break;
	case I40E_VIRTCHNL_OP_CONFIG_PROMISCUOUS_MODE:
		ret = i40e_vc_config_promiscuous_mode_msg(vf, msg, msglen);
		break;
	case I40E_VIRTCHNL_OP_CONFIG_VSI_QUEUES:
		ret = i40e_vc_config_queues_msg(vf, msg, msglen);
		break;
	case I40E_VIRTCHNL_OP_CONFIG_IRQ_MAP:
		ret = i40e_vc_config_irq_map_msg(vf, msg, msglen);
		break;
	case I40E_VIRTCHNL_OP_ENABLE_QUEUES:
		ret = i40e_vc_enable_queues_msg(vf, msg, msglen);
		i40e_vc_notify_vf_link_state(vf);
		break;
	case I40E_VIRTCHNL_OP_DISABLE_QUEUES:
		ret = i40e_vc_disable_queues_msg(vf, msg, msglen);
		break;
	case I40E_VIRTCHNL_OP_ADD_ETHER_ADDRESS:
		ret = i40e_vc_add_mac_addr_msg(vf, msg, msglen);
		break;
	case I40E_VIRTCHNL_OP_DEL_ETHER_ADDRESS:
		ret = i40e_vc_del_mac_addr_msg(vf, msg, msglen);
		break;
	case I40E_VIRTCHNL_OP_ADD_VLAN:
		ret = i40e_vc_add_vlan_msg(vf, msg, msglen);
		break;
	case I40E_VIRTCHNL_OP_DEL_VLAN:
		ret = i40e_vc_remove_vlan_msg(vf, msg, msglen);
		break;
	case I40E_VIRTCHNL_OP_GET_STATS:
		ret = i40e_vc_get_stats_msg(vf, msg, msglen);
		break;
	case I40E_VIRTCHNL_OP_UNKNOWN:
	default:
		dev_err(&pf->pdev->dev, "Unsupported opcode %d from VF %d\n",
			v_opcode, local_vf_id);
		ret = i40e_vc_send_resp_to_vf(vf, v_opcode,
					      I40E_ERR_NOT_IMPLEMENTED);
		break;
	}

	return ret;
}

/**
 * i40e_vc_process_vflr_event
 * @pf: pointer to the PF structure
 *
 * called from the vlfr irq handler to
 * free up VF resources and state variables
 **/
int i40e_vc_process_vflr_event(struct i40e_pf *pf)
{
	u32 reg, reg_idx, bit_idx, vf_id;
	struct i40e_hw *hw = &pf->hw;
	struct i40e_vf *vf;

	if (!test_bit(__I40E_VFLR_EVENT_PENDING, &pf->state))
		return 0;

	/* re-enable vflr interrupt cause */
	reg = rd32(hw, I40E_PFINT_ICR0_ENA);
	reg |= I40E_PFINT_ICR0_ENA_VFLR_MASK;
	wr32(hw, I40E_PFINT_ICR0_ENA, reg);
	i40e_flush(hw);

	clear_bit(__I40E_VFLR_EVENT_PENDING, &pf->state);
	for (vf_id = 0; vf_id < pf->num_alloc_vfs; vf_id++) {
		reg_idx = (hw->func_caps.vf_base_id + vf_id) / 32;
		bit_idx = (hw->func_caps.vf_base_id + vf_id) % 32;
		/* read GLGEN_VFLRSTAT register to find out the flr VFs */
		vf = &pf->vf[vf_id];
		reg = rd32(hw, I40E_GLGEN_VFLRSTAT(reg_idx));
		if (reg & (1 << bit_idx)) {
			/* clear the bit in GLGEN_VFLRSTAT */
			wr32(hw, I40E_GLGEN_VFLRSTAT(reg_idx), (1 << bit_idx));

			if (!test_bit(__I40E_DOWN, &pf->state))
				i40e_reset_vf(vf, true);
		}
	}

	return 0;
}

/**
 * i40e_ndo_set_vf_mac
 * @netdev: network interface device structure
 * @vf_id: VF identifier
 * @mac: mac address
 *
 * program VF mac address
 **/
int i40e_ndo_set_vf_mac(struct net_device *netdev, int vf_id, u8 *mac)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_vsi *vsi = np->vsi;
	struct i40e_pf *pf = vsi->back;
	struct i40e_mac_filter *f;
	struct i40e_vf *vf;
	int ret = 0;

	/* validate the request */
	if (vf_id >= pf->num_alloc_vfs) {
		dev_err(&pf->pdev->dev,
			"Invalid VF Identifier %d\n", vf_id);
		ret = -EINVAL;
		goto error_param;
	}

	vf = &(pf->vf[vf_id]);
	vsi = pf->vsi[vf->lan_vsi_idx];
	if (!test_bit(I40E_VF_STAT_INIT, &vf->vf_states)) {
		dev_err(&pf->pdev->dev,
			"Uninitialized VF %d\n", vf_id);
		ret = -EINVAL;
		goto error_param;
	}

	if (!is_valid_ether_addr(mac)) {
		dev_err(&pf->pdev->dev,
			"Invalid VF ethernet address\n");
		ret = -EINVAL;
		goto error_param;
	}

	/* delete the temporary mac address */
	i40e_del_filter(vsi, vf->default_lan_addr.addr, vf->port_vlan_id,
			true, false);

	/* Delete all the filters for this VSI - we're going to kill it
	 * anyway.
	 */
	list_for_each_entry(f, &vsi->mac_filter_list, list)
		i40e_del_filter(vsi, f->macaddr, f->vlan, true, false);

	dev_info(&pf->pdev->dev, "Setting MAC %pM on VF %d\n", mac, vf_id);
	/* program mac filter */
	if (i40e_sync_vsi_filters(vsi)) {
		dev_err(&pf->pdev->dev, "Unable to program ucast filters\n");
		ret = -EIO;
		goto error_param;
	}
	ether_addr_copy(vf->default_lan_addr.addr, mac);
	vf->pf_set_mac = true;
	/* Force the VF driver stop so it has to reload with new MAC address */
	i40e_vc_disable_vf(pf, vf);
	dev_info(&pf->pdev->dev, "Reload the VF driver to make this change effective.\n");

error_param:
	return ret;
}

/**
 * i40e_ndo_set_vf_port_vlan
 * @netdev: network interface device structure
 * @vf_id: VF identifier
 * @vlan_id: mac address
 * @qos: priority setting
 *
 * program VF vlan id and/or qos
 **/
int i40e_ndo_set_vf_port_vlan(struct net_device *netdev,
			      int vf_id, u16 vlan_id, u8 qos)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_pf *pf = np->vsi->back;
	struct i40e_vsi *vsi;
	struct i40e_vf *vf;
	int ret = 0;

	/* validate the request */
	if (vf_id >= pf->num_alloc_vfs) {
		dev_err(&pf->pdev->dev, "Invalid VF Identifier %d\n", vf_id);
		ret = -EINVAL;
		goto error_pvid;
	}

	if ((vlan_id > I40E_MAX_VLANID) || (qos > 7)) {
		dev_err(&pf->pdev->dev, "Invalid VF Parameters\n");
		ret = -EINVAL;
		goto error_pvid;
	}

	vf = &(pf->vf[vf_id]);
	vsi = pf->vsi[vf->lan_vsi_idx];
	if (!test_bit(I40E_VF_STAT_INIT, &vf->vf_states)) {
		dev_err(&pf->pdev->dev, "Uninitialized VF %d\n", vf_id);
		ret = -EINVAL;
		goto error_pvid;
	}

	if (vsi->info.pvid == 0 && i40e_is_vsi_in_vlan(vsi)) {
		dev_err(&pf->pdev->dev,
			"VF %d has already configured VLAN filters and the administrator is requesting a port VLAN override.\nPlease unload and reload the VF driver for this change to take effect.\n",
			vf_id);
		/* Administrator Error - knock the VF offline until he does
		 * the right thing by reconfiguring his network correctly
		 * and then reloading the VF driver.
		 */
		i40e_vc_disable_vf(pf, vf);
	}

	/* Check for condition where there was already a port VLAN ID
	 * filter set and now it is being deleted by setting it to zero.
	 * Additionally check for the condition where there was a port
	 * VLAN but now there is a new and different port VLAN being set.
	 * Before deleting all the old VLAN filters we must add new ones
	 * with -1 (I40E_VLAN_ANY) or otherwise we're left with all our
	 * MAC addresses deleted.
	 */
	if ((!(vlan_id || qos) ||
	    (vlan_id | qos) != le16_to_cpu(vsi->info.pvid)) &&
	    vsi->info.pvid)
		ret = i40e_vsi_add_vlan(vsi, I40E_VLAN_ANY);

	if (vsi->info.pvid) {
		/* kill old VLAN */
		ret = i40e_vsi_kill_vlan(vsi, (le16_to_cpu(vsi->info.pvid) &
					       VLAN_VID_MASK));
		if (ret) {
			dev_info(&vsi->back->pdev->dev,
				 "remove VLAN failed, ret=%d, aq_err=%d\n",
				 ret, pf->hw.aq.asq_last_status);
		}
	}
	if (vlan_id || qos)
		ret = i40e_vsi_add_pvid(vsi,
				vlan_id | (qos << I40E_VLAN_PRIORITY_SHIFT));
	else
		i40e_vsi_remove_pvid(vsi);

	if (vlan_id) {
		dev_info(&pf->pdev->dev, "Setting VLAN %d, QOS 0x%x on VF %d\n",
			 vlan_id, qos, vf_id);

		/* add new VLAN filter */
		ret = i40e_vsi_add_vlan(vsi, vlan_id);
		if (ret) {
			dev_info(&vsi->back->pdev->dev,
				 "add VF VLAN failed, ret=%d aq_err=%d\n", ret,
				 vsi->back->hw.aq.asq_last_status);
			goto error_pvid;
		}
		/* Kill non-vlan MAC filters - ignore error return since
		 * there might not be any non-vlan MAC filters.
		 */
		i40e_vsi_kill_vlan(vsi, I40E_VLAN_ANY);
	}

	if (ret) {
		dev_err(&pf->pdev->dev, "Unable to update VF vsi context\n");
		goto error_pvid;
	}
	/* The Port VLAN needs to be saved across resets the same as the
	 * default LAN MAC address.
	 */
	vf->port_vlan_id = le16_to_cpu(vsi->info.pvid);
	ret = 0;

error_pvid:
	return ret;
}

#define I40E_BW_CREDIT_DIVISOR 50     /* 50Mbps per BW credit */
#define I40E_MAX_BW_INACTIVE_ACCUM 4  /* device can accumulate 4 credits max */
/**
 * i40e_ndo_set_vf_bw
 * @netdev: network interface device structure
 * @vf_id: VF identifier
 * @tx_rate: Tx rate
 *
 * configure VF Tx rate
 **/
int i40e_ndo_set_vf_bw(struct net_device *netdev, int vf_id, int min_tx_rate,
		       int max_tx_rate)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_pf *pf = np->vsi->back;
	struct i40e_vsi *vsi;
	struct i40e_vf *vf;
	int speed = 0;
	int ret = 0;

	/* validate the request */
	if (vf_id >= pf->num_alloc_vfs) {
		dev_err(&pf->pdev->dev, "Invalid VF Identifier %d.\n", vf_id);
		ret = -EINVAL;
		goto error;
	}

	if (min_tx_rate) {
		dev_err(&pf->pdev->dev, "Invalid min tx rate (%d) (greater than 0) specified for VF %d.\n",
			min_tx_rate, vf_id);
		return -EINVAL;
	}

	vf = &(pf->vf[vf_id]);
	vsi = pf->vsi[vf->lan_vsi_idx];
	if (!test_bit(I40E_VF_STAT_INIT, &vf->vf_states)) {
		dev_err(&pf->pdev->dev, "Uninitialized VF %d.\n", vf_id);
		ret = -EINVAL;
		goto error;
	}

	switch (pf->hw.phy.link_info.link_speed) {
	case I40E_LINK_SPEED_40GB:
		speed = 40000;
		break;
	case I40E_LINK_SPEED_10GB:
		speed = 10000;
		break;
	case I40E_LINK_SPEED_1GB:
		speed = 1000;
		break;
	default:
		break;
	}

	if (max_tx_rate > speed) {
		dev_err(&pf->pdev->dev, "Invalid max tx rate %d specified for VF %d.",
			max_tx_rate, vf->vf_id);
		ret = -EINVAL;
		goto error;
	}

	if ((max_tx_rate < 50) && (max_tx_rate > 0)) {
		dev_warn(&pf->pdev->dev, "Setting max Tx rate to minimum usable value of 50Mbps.\n");
		max_tx_rate = 50;
	}

	/* Tx rate credits are in values of 50Mbps, 0 is disabled*/
	ret = i40e_aq_config_vsi_bw_limit(&pf->hw, vsi->seid,
					  max_tx_rate / I40E_BW_CREDIT_DIVISOR,
					  I40E_MAX_BW_INACTIVE_ACCUM, NULL);
	if (ret) {
		dev_err(&pf->pdev->dev, "Unable to set max tx rate, error code %d.\n",
			ret);
		ret = -EIO;
		goto error;
	}
	vf->tx_rate = max_tx_rate;
error:
	return ret;
}

/**
 * i40e_ndo_get_vf_config
 * @netdev: network interface device structure
 * @vf_id: VF identifier
 * @ivi: VF configuration structure
 *
 * return VF configuration
 **/
int i40e_ndo_get_vf_config(struct net_device *netdev,
			   int vf_id, struct ifla_vf_info *ivi)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_vsi *vsi = np->vsi;
	struct i40e_pf *pf = vsi->back;
	struct i40e_vf *vf;
	int ret = 0;

	/* validate the request */
	if (vf_id >= pf->num_alloc_vfs) {
		dev_err(&pf->pdev->dev, "Invalid VF Identifier %d\n", vf_id);
		ret = -EINVAL;
		goto error_param;
	}

	vf = &(pf->vf[vf_id]);
	/* first vsi is always the LAN vsi */
	vsi = pf->vsi[vf->lan_vsi_idx];
	if (!test_bit(I40E_VF_STAT_INIT, &vf->vf_states)) {
		dev_err(&pf->pdev->dev, "Uninitialized VF %d\n", vf_id);
		ret = -EINVAL;
		goto error_param;
	}

	ivi->vf = vf_id;

	memcpy(&ivi->mac, vf->default_lan_addr.addr, ETH_ALEN);

	ivi->max_tx_rate = vf->tx_rate;
	ivi->min_tx_rate = 0;
	ivi->vlan = le16_to_cpu(vsi->info.pvid) & I40E_VLAN_MASK;
	ivi->qos = (le16_to_cpu(vsi->info.pvid) & I40E_PRIORITY_MASK) >>
		   I40E_VLAN_PRIORITY_SHIFT;
	if (vf->link_forced == false)
		ivi->linkstate = IFLA_VF_LINK_STATE_AUTO;
	else if (vf->link_up == true)
		ivi->linkstate = IFLA_VF_LINK_STATE_ENABLE;
	else
		ivi->linkstate = IFLA_VF_LINK_STATE_DISABLE;
	ivi->spoofchk = vf->spoofchk;
	ret = 0;

error_param:
	return ret;
}

/**
 * i40e_ndo_set_vf_link_state
 * @netdev: network interface device structure
 * @vf_id: VF identifier
 * @link: required link state
 *
 * Set the link state of a specified VF, regardless of physical link state
 **/
int i40e_ndo_set_vf_link_state(struct net_device *netdev, int vf_id, int link)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_pf *pf = np->vsi->back;
	struct i40e_virtchnl_pf_event pfe;
	struct i40e_hw *hw = &pf->hw;
	struct i40e_vf *vf;
	int abs_vf_id;
	int ret = 0;

	/* validate the request */
	if (vf_id >= pf->num_alloc_vfs) {
		dev_err(&pf->pdev->dev, "Invalid VF Identifier %d\n", vf_id);
		ret = -EINVAL;
		goto error_out;
	}

	vf = &pf->vf[vf_id];
	abs_vf_id = vf->vf_id + hw->func_caps.vf_base_id;

	pfe.event = I40E_VIRTCHNL_EVENT_LINK_CHANGE;
	pfe.severity = I40E_PF_EVENT_SEVERITY_INFO;

	switch (link) {
	case IFLA_VF_LINK_STATE_AUTO:
		vf->link_forced = false;
		pfe.event_data.link_event.link_status =
			pf->hw.phy.link_info.link_info & I40E_AQ_LINK_UP;
		pfe.event_data.link_event.link_speed =
			pf->hw.phy.link_info.link_speed;
		break;
	case IFLA_VF_LINK_STATE_ENABLE:
		vf->link_forced = true;
		vf->link_up = true;
		pfe.event_data.link_event.link_status = true;
		pfe.event_data.link_event.link_speed = I40E_LINK_SPEED_40GB;
		break;
	case IFLA_VF_LINK_STATE_DISABLE:
		vf->link_forced = true;
		vf->link_up = false;
		pfe.event_data.link_event.link_status = false;
		pfe.event_data.link_event.link_speed = 0;
		break;
	default:
		ret = -EINVAL;
		goto error_out;
	}
	/* Notify the VF of its new link state */
	i40e_aq_send_msg_to_vf(hw, abs_vf_id, I40E_VIRTCHNL_OP_EVENT,
			       0, (u8 *)&pfe, sizeof(pfe), NULL);

error_out:
	return ret;
}

/**
 * i40e_ndo_set_vf_spoofchk
 * @netdev: network interface device structure
 * @vf_id: VF identifier
 * @enable: flag to enable or disable feature
 *
 * Enable or disable VF spoof checking
 **/
int i40e_ndo_set_vf_spoofchk(struct net_device *netdev, int vf_id, bool enable)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_vsi *vsi = np->vsi;
	struct i40e_pf *pf = vsi->back;
	struct i40e_vsi_context ctxt;
	struct i40e_hw *hw = &pf->hw;
	struct i40e_vf *vf;
	int ret = 0;

	/* validate the request */
	if (vf_id >= pf->num_alloc_vfs) {
		dev_err(&pf->pdev->dev, "Invalid VF Identifier %d\n", vf_id);
		ret = -EINVAL;
		goto out;
	}

	vf = &(pf->vf[vf_id]);

	if (enable == vf->spoofchk)
		goto out;

	vf->spoofchk = enable;
	memset(&ctxt, 0, sizeof(ctxt));
	ctxt.seid = pf->vsi[vf->lan_vsi_idx]->seid;
	ctxt.pf_num = pf->hw.pf_id;
	ctxt.info.valid_sections = cpu_to_le16(I40E_AQ_VSI_PROP_SECURITY_VALID);
	if (enable)
		ctxt.info.sec_flags |= (I40E_AQ_VSI_SEC_FLAG_ENABLE_VLAN_CHK |
					I40E_AQ_VSI_SEC_FLAG_ENABLE_MAC_CHK);
	ret = i40e_aq_update_vsi_params(hw, &ctxt, NULL);
	if (ret) {
		dev_err(&pf->pdev->dev, "Error %d updating VSI parameters\n",
			ret);
		ret = -EIO;
	}
out:
	return ret;
}
