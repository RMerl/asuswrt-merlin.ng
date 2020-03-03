/*******************************************************************************
 *
 * Intel Ethernet Controller XL710 Family Linux Driver
 * Copyright(c) 2013 - 2014 Intel Corporation.
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

#include <linux/prefetch.h>
#include <net/busy_poll.h>
#include "i40e.h"
#include "i40e_prototype.h"

static inline __le64 build_ctob(u32 td_cmd, u32 td_offset, unsigned int size,
				u32 td_tag)
{
	return cpu_to_le64(I40E_TX_DESC_DTYPE_DATA |
			   ((u64)td_cmd  << I40E_TXD_QW1_CMD_SHIFT) |
			   ((u64)td_offset << I40E_TXD_QW1_OFFSET_SHIFT) |
			   ((u64)size  << I40E_TXD_QW1_TX_BUF_SZ_SHIFT) |
			   ((u64)td_tag  << I40E_TXD_QW1_L2TAG1_SHIFT));
}

#define I40E_TXD_CMD (I40E_TX_DESC_CMD_EOP | I40E_TX_DESC_CMD_RS)
#define I40E_FD_CLEAN_DELAY 10
/**
 * i40e_program_fdir_filter - Program a Flow Director filter
 * @fdir_data: Packet data that will be filter parameters
 * @raw_packet: the pre-allocated packet buffer for FDir
 * @pf: The PF pointer
 * @add: True for add/update, False for remove
 **/
int i40e_program_fdir_filter(struct i40e_fdir_filter *fdir_data, u8 *raw_packet,
			     struct i40e_pf *pf, bool add)
{
	struct i40e_filter_program_desc *fdir_desc;
	struct i40e_tx_buffer *tx_buf, *first;
	struct i40e_tx_desc *tx_desc;
	struct i40e_ring *tx_ring;
	unsigned int fpt, dcc;
	struct i40e_vsi *vsi;
	struct device *dev;
	dma_addr_t dma;
	u32 td_cmd = 0;
	u16 delay = 0;
	u16 i;

	/* find existing FDIR VSI */
	vsi = NULL;
	for (i = 0; i < pf->num_alloc_vsi; i++)
		if (pf->vsi[i] && pf->vsi[i]->type == I40E_VSI_FDIR)
			vsi = pf->vsi[i];
	if (!vsi)
		return -ENOENT;

	tx_ring = vsi->tx_rings[0];
	dev = tx_ring->dev;

	/* we need two descriptors to add/del a filter and we can wait */
	do {
		if (I40E_DESC_UNUSED(tx_ring) > 1)
			break;
		msleep_interruptible(1);
		delay++;
	} while (delay < I40E_FD_CLEAN_DELAY);

	if (!(I40E_DESC_UNUSED(tx_ring) > 1))
		return -EAGAIN;

	dma = dma_map_single(dev, raw_packet,
			     I40E_FDIR_MAX_RAW_PACKET_SIZE, DMA_TO_DEVICE);
	if (dma_mapping_error(dev, dma))
		goto dma_fail;

	/* grab the next descriptor */
	i = tx_ring->next_to_use;
	fdir_desc = I40E_TX_FDIRDESC(tx_ring, i);
	first = &tx_ring->tx_bi[i];
	memset(first, 0, sizeof(struct i40e_tx_buffer));

	tx_ring->next_to_use = ((i + 1) < tx_ring->count) ? i + 1 : 0;

	fpt = (fdir_data->q_index << I40E_TXD_FLTR_QW0_QINDEX_SHIFT) &
	      I40E_TXD_FLTR_QW0_QINDEX_MASK;

	fpt |= (fdir_data->flex_off << I40E_TXD_FLTR_QW0_FLEXOFF_SHIFT) &
	       I40E_TXD_FLTR_QW0_FLEXOFF_MASK;

	fpt |= (fdir_data->pctype << I40E_TXD_FLTR_QW0_PCTYPE_SHIFT) &
	       I40E_TXD_FLTR_QW0_PCTYPE_MASK;

	/* Use LAN VSI Id if not programmed by user */
	if (fdir_data->dest_vsi == 0)
		fpt |= (pf->vsi[pf->lan_vsi]->id) <<
		       I40E_TXD_FLTR_QW0_DEST_VSI_SHIFT;
	else
		fpt |= ((u32)fdir_data->dest_vsi <<
			I40E_TXD_FLTR_QW0_DEST_VSI_SHIFT) &
		       I40E_TXD_FLTR_QW0_DEST_VSI_MASK;

	dcc = I40E_TX_DESC_DTYPE_FILTER_PROG;

	if (add)
		dcc |= I40E_FILTER_PROGRAM_DESC_PCMD_ADD_UPDATE <<
		       I40E_TXD_FLTR_QW1_PCMD_SHIFT;
	else
		dcc |= I40E_FILTER_PROGRAM_DESC_PCMD_REMOVE <<
		       I40E_TXD_FLTR_QW1_PCMD_SHIFT;

	dcc |= (fdir_data->dest_ctl << I40E_TXD_FLTR_QW1_DEST_SHIFT) &
	       I40E_TXD_FLTR_QW1_DEST_MASK;

	dcc |= (fdir_data->fd_status << I40E_TXD_FLTR_QW1_FD_STATUS_SHIFT) &
	       I40E_TXD_FLTR_QW1_FD_STATUS_MASK;

	if (fdir_data->cnt_index != 0) {
		dcc |= I40E_TXD_FLTR_QW1_CNT_ENA_MASK;
		dcc |= ((u32)fdir_data->cnt_index <<
			I40E_TXD_FLTR_QW1_CNTINDEX_SHIFT) &
			I40E_TXD_FLTR_QW1_CNTINDEX_MASK;
	}

	fdir_desc->qindex_flex_ptype_vsi = cpu_to_le32(fpt);
	fdir_desc->rsvd = cpu_to_le32(0);
	fdir_desc->dtype_cmd_cntindex = cpu_to_le32(dcc);
	fdir_desc->fd_id = cpu_to_le32(fdir_data->fd_id);

	/* Now program a dummy descriptor */
	i = tx_ring->next_to_use;
	tx_desc = I40E_TX_DESC(tx_ring, i);
	tx_buf = &tx_ring->tx_bi[i];

	tx_ring->next_to_use = ((i + 1) < tx_ring->count) ? i + 1 : 0;

	memset(tx_buf, 0, sizeof(struct i40e_tx_buffer));

	/* record length, and DMA address */
	dma_unmap_len_set(tx_buf, len, I40E_FDIR_MAX_RAW_PACKET_SIZE);
	dma_unmap_addr_set(tx_buf, dma, dma);

	tx_desc->buffer_addr = cpu_to_le64(dma);
	td_cmd = I40E_TXD_CMD | I40E_TX_DESC_CMD_DUMMY;

	tx_buf->tx_flags = I40E_TX_FLAGS_FD_SB;
	tx_buf->raw_buf = (void *)raw_packet;

	tx_desc->cmd_type_offset_bsz =
		build_ctob(td_cmd, 0, I40E_FDIR_MAX_RAW_PACKET_SIZE, 0);

	/* set the timestamp */
	tx_buf->time_stamp = jiffies;

	/* Force memory writes to complete before letting h/w
	 * know there are new descriptors to fetch.
	 */
	wmb();

	/* Mark the data descriptor to be watched */
	first->next_to_watch = tx_desc;

	writel(tx_ring->next_to_use, tx_ring->tail);
	return 0;

dma_fail:
	return -1;
}

#define IP_HEADER_OFFSET 14
#define I40E_UDPIP_DUMMY_PACKET_LEN 42
/**
 * i40e_add_del_fdir_udpv4 - Add/Remove UDPv4 filters
 * @vsi: pointer to the targeted VSI
 * @fd_data: the flow director data required for the FDir descriptor
 * @add: true adds a filter, false removes it
 *
 * Returns 0 if the filters were successfully added or removed
 **/
static int i40e_add_del_fdir_udpv4(struct i40e_vsi *vsi,
				   struct i40e_fdir_filter *fd_data,
				   bool add)
{
	struct i40e_pf *pf = vsi->back;
	struct udphdr *udp;
	struct iphdr *ip;
	bool err = false;
	u8 *raw_packet;
	int ret;
	static char packet[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x08, 0,
		0x45, 0, 0, 0x1c, 0, 0, 0x40, 0, 0x40, 0x11, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	raw_packet = kzalloc(I40E_FDIR_MAX_RAW_PACKET_SIZE, GFP_KERNEL);
	if (!raw_packet)
		return -ENOMEM;
	memcpy(raw_packet, packet, I40E_UDPIP_DUMMY_PACKET_LEN);

	ip = (struct iphdr *)(raw_packet + IP_HEADER_OFFSET);
	udp = (struct udphdr *)(raw_packet + IP_HEADER_OFFSET
	      + sizeof(struct iphdr));

	ip->daddr = fd_data->dst_ip[0];
	udp->dest = fd_data->dst_port;
	ip->saddr = fd_data->src_ip[0];
	udp->source = fd_data->src_port;

	fd_data->pctype = I40E_FILTER_PCTYPE_NONF_IPV4_UDP;
	ret = i40e_program_fdir_filter(fd_data, raw_packet, pf, add);
	if (ret) {
		dev_info(&pf->pdev->dev,
			 "PCTYPE:%d, Filter command send failed for fd_id:%d (ret = %d)\n",
			 fd_data->pctype, fd_data->fd_id, ret);
		err = true;
	} else if (I40E_DEBUG_FD & pf->hw.debug_mask) {
		if (add)
			dev_info(&pf->pdev->dev,
				 "Filter OK for PCTYPE %d loc = %d\n",
				 fd_data->pctype, fd_data->fd_id);
		else
			dev_info(&pf->pdev->dev,
				 "Filter deleted for PCTYPE %d loc = %d\n",
				 fd_data->pctype, fd_data->fd_id);
	}
	return err ? -EOPNOTSUPP : 0;
}

#define I40E_TCPIP_DUMMY_PACKET_LEN 54
/**
 * i40e_add_del_fdir_tcpv4 - Add/Remove TCPv4 filters
 * @vsi: pointer to the targeted VSI
 * @fd_data: the flow director data required for the FDir descriptor
 * @add: true adds a filter, false removes it
 *
 * Returns 0 if the filters were successfully added or removed
 **/
static int i40e_add_del_fdir_tcpv4(struct i40e_vsi *vsi,
				   struct i40e_fdir_filter *fd_data,
				   bool add)
{
	struct i40e_pf *pf = vsi->back;
	struct tcphdr *tcp;
	struct iphdr *ip;
	bool err = false;
	u8 *raw_packet;
	int ret;
	/* Dummy packet */
	static char packet[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x08, 0,
		0x45, 0, 0, 0x28, 0, 0, 0x40, 0, 0x40, 0x6, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x80, 0x11,
		0x0, 0x72, 0, 0, 0, 0};

	raw_packet = kzalloc(I40E_FDIR_MAX_RAW_PACKET_SIZE, GFP_KERNEL);
	if (!raw_packet)
		return -ENOMEM;
	memcpy(raw_packet, packet, I40E_TCPIP_DUMMY_PACKET_LEN);

	ip = (struct iphdr *)(raw_packet + IP_HEADER_OFFSET);
	tcp = (struct tcphdr *)(raw_packet + IP_HEADER_OFFSET
	      + sizeof(struct iphdr));

	ip->daddr = fd_data->dst_ip[0];
	tcp->dest = fd_data->dst_port;
	ip->saddr = fd_data->src_ip[0];
	tcp->source = fd_data->src_port;

	if (add) {
		pf->fd_tcp_rule++;
		if (pf->flags & I40E_FLAG_FD_ATR_ENABLED) {
			dev_info(&pf->pdev->dev, "Forcing ATR off, sideband rules for TCP/IPv4 flow being applied\n");
			pf->flags &= ~I40E_FLAG_FD_ATR_ENABLED;
		}
	} else {
		pf->fd_tcp_rule = (pf->fd_tcp_rule > 0) ?
				  (pf->fd_tcp_rule - 1) : 0;
		if (pf->fd_tcp_rule == 0) {
			pf->flags |= I40E_FLAG_FD_ATR_ENABLED;
			dev_info(&pf->pdev->dev, "ATR re-enabled due to no sideband TCP/IPv4 rules\n");
		}
	}

	fd_data->pctype = I40E_FILTER_PCTYPE_NONF_IPV4_TCP;
	ret = i40e_program_fdir_filter(fd_data, raw_packet, pf, add);

	if (ret) {
		dev_info(&pf->pdev->dev,
			 "PCTYPE:%d, Filter command send failed for fd_id:%d (ret = %d)\n",
			 fd_data->pctype, fd_data->fd_id, ret);
		err = true;
	} else if (I40E_DEBUG_FD & pf->hw.debug_mask) {
		if (add)
			dev_info(&pf->pdev->dev, "Filter OK for PCTYPE %d loc = %d)\n",
				 fd_data->pctype, fd_data->fd_id);
		else
			dev_info(&pf->pdev->dev,
				 "Filter deleted for PCTYPE %d loc = %d\n",
				 fd_data->pctype, fd_data->fd_id);
	}

	return err ? -EOPNOTSUPP : 0;
}

/**
 * i40e_add_del_fdir_sctpv4 - Add/Remove SCTPv4 Flow Director filters for
 * a specific flow spec
 * @vsi: pointer to the targeted VSI
 * @fd_data: the flow director data required for the FDir descriptor
 * @add: true adds a filter, false removes it
 *
 * Always returns -EOPNOTSUPP
 **/
static int i40e_add_del_fdir_sctpv4(struct i40e_vsi *vsi,
				    struct i40e_fdir_filter *fd_data,
				    bool add)
{
	return -EOPNOTSUPP;
}

#define I40E_IP_DUMMY_PACKET_LEN 34
/**
 * i40e_add_del_fdir_ipv4 - Add/Remove IPv4 Flow Director filters for
 * a specific flow spec
 * @vsi: pointer to the targeted VSI
 * @fd_data: the flow director data required for the FDir descriptor
 * @add: true adds a filter, false removes it
 *
 * Returns 0 if the filters were successfully added or removed
 **/
static int i40e_add_del_fdir_ipv4(struct i40e_vsi *vsi,
				  struct i40e_fdir_filter *fd_data,
				  bool add)
{
	struct i40e_pf *pf = vsi->back;
	struct iphdr *ip;
	bool err = false;
	u8 *raw_packet;
	int ret;
	int i;
	static char packet[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x08, 0,
		0x45, 0, 0, 0x14, 0, 0, 0x40, 0, 0x40, 0x10, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0};

	for (i = I40E_FILTER_PCTYPE_NONF_IPV4_OTHER;
	     i <= I40E_FILTER_PCTYPE_FRAG_IPV4;	i++) {
		raw_packet = kzalloc(I40E_FDIR_MAX_RAW_PACKET_SIZE, GFP_KERNEL);
		if (!raw_packet)
			return -ENOMEM;
		memcpy(raw_packet, packet, I40E_IP_DUMMY_PACKET_LEN);
		ip = (struct iphdr *)(raw_packet + IP_HEADER_OFFSET);

		ip->saddr = fd_data->src_ip[0];
		ip->daddr = fd_data->dst_ip[0];
		ip->protocol = 0;

		fd_data->pctype = i;
		ret = i40e_program_fdir_filter(fd_data, raw_packet, pf, add);

		if (ret) {
			dev_info(&pf->pdev->dev,
				 "PCTYPE:%d, Filter command send failed for fd_id:%d (ret = %d)\n",
				 fd_data->pctype, fd_data->fd_id, ret);
			err = true;
		} else if (I40E_DEBUG_FD & pf->hw.debug_mask) {
			if (add)
				dev_info(&pf->pdev->dev,
					 "Filter OK for PCTYPE %d loc = %d\n",
					 fd_data->pctype, fd_data->fd_id);
			else
				dev_info(&pf->pdev->dev,
					 "Filter deleted for PCTYPE %d loc = %d\n",
					 fd_data->pctype, fd_data->fd_id);
		}
	}

	return err ? -EOPNOTSUPP : 0;
}

/**
 * i40e_add_del_fdir - Build raw packets to add/del fdir filter
 * @vsi: pointer to the targeted VSI
 * @cmd: command to get or set RX flow classification rules
 * @add: true adds a filter, false removes it
 *
 **/
int i40e_add_del_fdir(struct i40e_vsi *vsi,
		      struct i40e_fdir_filter *input, bool add)
{
	struct i40e_pf *pf = vsi->back;
	int ret;

	switch (input->flow_type & ~FLOW_EXT) {
	case TCP_V4_FLOW:
		ret = i40e_add_del_fdir_tcpv4(vsi, input, add);
		break;
	case UDP_V4_FLOW:
		ret = i40e_add_del_fdir_udpv4(vsi, input, add);
		break;
	case SCTP_V4_FLOW:
		ret = i40e_add_del_fdir_sctpv4(vsi, input, add);
		break;
	case IPV4_FLOW:
		ret = i40e_add_del_fdir_ipv4(vsi, input, add);
		break;
	case IP_USER_FLOW:
		switch (input->ip4_proto) {
		case IPPROTO_TCP:
			ret = i40e_add_del_fdir_tcpv4(vsi, input, add);
			break;
		case IPPROTO_UDP:
			ret = i40e_add_del_fdir_udpv4(vsi, input, add);
			break;
		case IPPROTO_SCTP:
			ret = i40e_add_del_fdir_sctpv4(vsi, input, add);
			break;
		default:
			ret = i40e_add_del_fdir_ipv4(vsi, input, add);
			break;
		}
		break;
	default:
		dev_info(&pf->pdev->dev, "Could not specify spec type %d\n",
			 input->flow_type);
		ret = -EINVAL;
	}

	/* The buffer allocated here is freed by the i40e_clean_tx_ring() */
	return ret;
}

/**
 * i40e_fd_handle_status - check the Programming Status for FD
 * @rx_ring: the Rx ring for this descriptor
 * @rx_desc: the Rx descriptor for programming Status, not a packet descriptor.
 * @prog_id: the id originally used for programming
 *
 * This is used to verify if the FD programming or invalidation
 * requested by SW to the HW is successful or not and take actions accordingly.
 **/
static void i40e_fd_handle_status(struct i40e_ring *rx_ring,
				  union i40e_rx_desc *rx_desc, u8 prog_id)
{
	struct i40e_pf *pf = rx_ring->vsi->back;
	struct pci_dev *pdev = pf->pdev;
	u32 fcnt_prog, fcnt_avail;
	u32 error;
	u64 qw;

	qw = le64_to_cpu(rx_desc->wb.qword1.status_error_len);
	error = (qw & I40E_RX_PROG_STATUS_DESC_QW1_ERROR_MASK) >>
		I40E_RX_PROG_STATUS_DESC_QW1_ERROR_SHIFT;

	if (error == (0x1 << I40E_RX_PROG_STATUS_DESC_FD_TBL_FULL_SHIFT)) {
		if ((rx_desc->wb.qword0.hi_dword.fd_id != 0) ||
		    (I40E_DEBUG_FD & pf->hw.debug_mask))
			dev_warn(&pdev->dev, "ntuple filter loc = %d, could not be added\n",
				 rx_desc->wb.qword0.hi_dword.fd_id);

		/* Check if the programming error is for ATR.
		 * If so, auto disable ATR and set a state for
		 * flush in progress. Next time we come here if flush is in
		 * progress do nothing, once flush is complete the state will
		 * be cleared.
		 */
		if (test_bit(__I40E_FD_FLUSH_REQUESTED, &pf->state))
			return;

		pf->fd_add_err++;
		/* store the current atr filter count */
		pf->fd_atr_cnt = i40e_get_current_atr_cnt(pf);

		if ((rx_desc->wb.qword0.hi_dword.fd_id == 0) &&
		    (pf->auto_disable_flags & I40E_FLAG_FD_SB_ENABLED)) {
			pf->auto_disable_flags |= I40E_FLAG_FD_ATR_ENABLED;
			set_bit(__I40E_FD_FLUSH_REQUESTED, &pf->state);
		}

		/* filter programming failed most likely due to table full */
		fcnt_prog = i40e_get_global_fd_count(pf);
		fcnt_avail = pf->fdir_pf_filter_count;
		/* If ATR is running fcnt_prog can quickly change,
		 * if we are very close to full, it makes sense to disable
		 * FD ATR/SB and then re-enable it when there is room.
		 */
		if (fcnt_prog >= (fcnt_avail - I40E_FDIR_BUFFER_FULL_MARGIN)) {
			if ((pf->flags & I40E_FLAG_FD_SB_ENABLED) &&
			    !(pf->auto_disable_flags &
				     I40E_FLAG_FD_SB_ENABLED)) {
				dev_warn(&pdev->dev, "FD filter space full, new ntuple rules will not be added\n");
				pf->auto_disable_flags |=
							I40E_FLAG_FD_SB_ENABLED;
			}
		} else {
			dev_info(&pdev->dev,
				"FD filter programming failed due to incorrect filter parameters\n");
		}
	} else if (error ==
			  (0x1 << I40E_RX_PROG_STATUS_DESC_NO_FD_ENTRY_SHIFT)) {
		if (I40E_DEBUG_FD & pf->hw.debug_mask)
			dev_info(&pdev->dev, "ntuple filter fd_id = %d, could not be removed\n",
				 rx_desc->wb.qword0.hi_dword.fd_id);
	}
}

/**
 * i40e_unmap_and_free_tx_resource - Release a Tx buffer
 * @ring:      the ring that owns the buffer
 * @tx_buffer: the buffer to free
 **/
static void i40e_unmap_and_free_tx_resource(struct i40e_ring *ring,
					    struct i40e_tx_buffer *tx_buffer)
{
	if (tx_buffer->skb) {
		if (tx_buffer->tx_flags & I40E_TX_FLAGS_FD_SB)
			kfree(tx_buffer->raw_buf);
		else
			dev_kfree_skb_any(tx_buffer->skb);

		if (dma_unmap_len(tx_buffer, len))
			dma_unmap_single(ring->dev,
					 dma_unmap_addr(tx_buffer, dma),
					 dma_unmap_len(tx_buffer, len),
					 DMA_TO_DEVICE);
	} else if (dma_unmap_len(tx_buffer, len)) {
		dma_unmap_page(ring->dev,
			       dma_unmap_addr(tx_buffer, dma),
			       dma_unmap_len(tx_buffer, len),
			       DMA_TO_DEVICE);
	}
	tx_buffer->next_to_watch = NULL;
	tx_buffer->skb = NULL;
	dma_unmap_len_set(tx_buffer, len, 0);
	/* tx_buffer must be completely set up in the transmit path */
}

/**
 * i40e_clean_tx_ring - Free any empty Tx buffers
 * @tx_ring: ring to be cleaned
 **/
void i40e_clean_tx_ring(struct i40e_ring *tx_ring)
{
	unsigned long bi_size;
	u16 i;

	/* ring already cleared, nothing to do */
	if (!tx_ring->tx_bi)
		return;

	/* Free all the Tx ring sk_buffs */
	for (i = 0; i < tx_ring->count; i++)
		i40e_unmap_and_free_tx_resource(tx_ring, &tx_ring->tx_bi[i]);

	bi_size = sizeof(struct i40e_tx_buffer) * tx_ring->count;
	memset(tx_ring->tx_bi, 0, bi_size);

	/* Zero out the descriptor ring */
	memset(tx_ring->desc, 0, tx_ring->size);

	tx_ring->next_to_use = 0;
	tx_ring->next_to_clean = 0;

	if (!tx_ring->netdev)
		return;

	/* cleanup Tx queue statistics */
	netdev_tx_reset_queue(netdev_get_tx_queue(tx_ring->netdev,
						  tx_ring->queue_index));
}

/**
 * i40e_free_tx_resources - Free Tx resources per queue
 * @tx_ring: Tx descriptor ring for a specific queue
 *
 * Free all transmit software resources
 **/
void i40e_free_tx_resources(struct i40e_ring *tx_ring)
{
	i40e_clean_tx_ring(tx_ring);
	kfree(tx_ring->tx_bi);
	tx_ring->tx_bi = NULL;

	if (tx_ring->desc) {
		dma_free_coherent(tx_ring->dev, tx_ring->size,
				  tx_ring->desc, tx_ring->dma);
		tx_ring->desc = NULL;
	}
}

/**
 * i40e_get_head - Retrieve head from head writeback
 * @tx_ring:  tx ring to fetch head of
 *
 * Returns value of Tx ring head based on value stored
 * in head write-back location
 **/
static inline u32 i40e_get_head(struct i40e_ring *tx_ring)
{
	void *head = (struct i40e_tx_desc *)tx_ring->desc + tx_ring->count;

	return le32_to_cpu(*(volatile __le32 *)head);
}

/**
 * i40e_get_tx_pending - how many tx descriptors not processed
 * @tx_ring: the ring of descriptors
 *
 * Since there is no access to the ring head register
 * in XL710, we need to use our local copies
 **/
static u32 i40e_get_tx_pending(struct i40e_ring *ring)
{
	u32 head, tail;

	head = i40e_get_head(ring);
	tail = readl(ring->tail);

	if (head != tail)
		return (head < tail) ?
			tail - head : (tail + ring->count - head);

	return 0;
}

/**
 * i40e_check_tx_hang - Is there a hang in the Tx queue
 * @tx_ring: the ring of descriptors
 **/
static bool i40e_check_tx_hang(struct i40e_ring *tx_ring)
{
	u32 tx_done = tx_ring->stats.packets;
	u32 tx_done_old = tx_ring->tx_stats.tx_done_old;
	u32 tx_pending = i40e_get_tx_pending(tx_ring);
	struct i40e_pf *pf = tx_ring->vsi->back;
	bool ret = false;

	clear_check_for_tx_hang(tx_ring);

	/* Check for a hung queue, but be thorough. This verifies
	 * that a transmit has been completed since the previous
	 * check AND there is at least one packet pending. The
	 * ARMED bit is set to indicate a potential hang. The
	 * bit is cleared if a pause frame is received to remove
	 * false hang detection due to PFC or 802.3x frames. By
	 * requiring this to fail twice we avoid races with
	 * PFC clearing the ARMED bit and conditions where we
	 * run the check_tx_hang logic with a transmit completion
	 * pending but without time to complete it yet.
	 */
	if ((tx_done_old == tx_done) && tx_pending) {
		/* make sure it is true for two checks in a row */
		ret = test_and_set_bit(__I40E_HANG_CHECK_ARMED,
				       &tx_ring->state);
	} else if (tx_done_old == tx_done &&
		   (tx_pending < I40E_MIN_DESC_PENDING) && (tx_pending > 0)) {
		if (I40E_DEBUG_FLOW & pf->hw.debug_mask)
			dev_info(tx_ring->dev, "HW needs some more descs to do a cacheline flush. tx_pending %d, queue %d",
				 tx_pending, tx_ring->queue_index);
		pf->tx_sluggish_count++;
	} else {
		/* update completed stats and disarm the hang check */
		tx_ring->tx_stats.tx_done_old = tx_done;
		clear_bit(__I40E_HANG_CHECK_ARMED, &tx_ring->state);
	}

	return ret;
}

#define WB_STRIDE 0x3

/**
 * i40e_clean_tx_irq - Reclaim resources after transmit completes
 * @tx_ring:  tx ring to clean
 * @budget:   how many cleans we're allowed
 *
 * Returns true if there's any budget left (e.g. the clean is finished)
 **/
static bool i40e_clean_tx_irq(struct i40e_ring *tx_ring, int budget)
{
	u16 i = tx_ring->next_to_clean;
	struct i40e_tx_buffer *tx_buf;
	struct i40e_tx_desc *tx_head;
	struct i40e_tx_desc *tx_desc;
	unsigned int total_packets = 0;
	unsigned int total_bytes = 0;

	tx_buf = &tx_ring->tx_bi[i];
	tx_desc = I40E_TX_DESC(tx_ring, i);
	i -= tx_ring->count;

	tx_head = I40E_TX_DESC(tx_ring, i40e_get_head(tx_ring));

	do {
		struct i40e_tx_desc *eop_desc = tx_buf->next_to_watch;

		/* if next_to_watch is not set then there is no work pending */
		if (!eop_desc)
			break;

		/* prevent any other reads prior to eop_desc */
		smp_rmb();

		/* we have caught up to head, no work left to do */
		if (tx_head == tx_desc)
			break;

		/* clear next_to_watch to prevent false hangs */
		tx_buf->next_to_watch = NULL;

		/* update the statistics for this packet */
		total_bytes += tx_buf->bytecount;
		total_packets += tx_buf->gso_segs;

		/* free the skb */
		dev_consume_skb_any(tx_buf->skb);

		/* unmap skb header data */
		dma_unmap_single(tx_ring->dev,
				 dma_unmap_addr(tx_buf, dma),
				 dma_unmap_len(tx_buf, len),
				 DMA_TO_DEVICE);

		/* clear tx_buffer data */
		tx_buf->skb = NULL;
		dma_unmap_len_set(tx_buf, len, 0);

		/* unmap remaining buffers */
		while (tx_desc != eop_desc) {

			tx_buf++;
			tx_desc++;
			i++;
			if (unlikely(!i)) {
				i -= tx_ring->count;
				tx_buf = tx_ring->tx_bi;
				tx_desc = I40E_TX_DESC(tx_ring, 0);
			}

			/* unmap any remaining paged data */
			if (dma_unmap_len(tx_buf, len)) {
				dma_unmap_page(tx_ring->dev,
					       dma_unmap_addr(tx_buf, dma),
					       dma_unmap_len(tx_buf, len),
					       DMA_TO_DEVICE);
				dma_unmap_len_set(tx_buf, len, 0);
			}
		}

		/* move us one more past the eop_desc for start of next pkt */
		tx_buf++;
		tx_desc++;
		i++;
		if (unlikely(!i)) {
			i -= tx_ring->count;
			tx_buf = tx_ring->tx_bi;
			tx_desc = I40E_TX_DESC(tx_ring, 0);
		}

		prefetch(tx_desc);

		/* update budget accounting */
		budget--;
	} while (likely(budget));

	i += tx_ring->count;
	tx_ring->next_to_clean = i;
	u64_stats_update_begin(&tx_ring->syncp);
	tx_ring->stats.bytes += total_bytes;
	tx_ring->stats.packets += total_packets;
	u64_stats_update_end(&tx_ring->syncp);
	tx_ring->q_vector->tx.total_bytes += total_bytes;
	tx_ring->q_vector->tx.total_packets += total_packets;

	/* check to see if there are any non-cache aligned descriptors
	 * waiting to be written back, and kick the hardware to force
	 * them to be written back in case of napi polling
	 */
	if (budget &&
	    !((i & WB_STRIDE) == WB_STRIDE) &&
	    !test_bit(__I40E_DOWN, &tx_ring->vsi->state) &&
	    (I40E_DESC_UNUSED(tx_ring) != tx_ring->count))
		tx_ring->arm_wb = true;
	else
		tx_ring->arm_wb = false;

	if (check_for_tx_hang(tx_ring) && i40e_check_tx_hang(tx_ring)) {
		/* schedule immediate reset if we believe we hung */
		dev_info(tx_ring->dev, "Detected Tx Unit Hang\n"
			 "  VSI                  <%d>\n"
			 "  Tx Queue             <%d>\n"
			 "  next_to_use          <%x>\n"
			 "  next_to_clean        <%x>\n",
			 tx_ring->vsi->seid,
			 tx_ring->queue_index,
			 tx_ring->next_to_use, i);
		dev_info(tx_ring->dev, "tx_bi[next_to_clean]\n"
			 "  time_stamp           <%lx>\n"
			 "  jiffies              <%lx>\n",
			 tx_ring->tx_bi[i].time_stamp, jiffies);

		netif_stop_subqueue(tx_ring->netdev, tx_ring->queue_index);

		dev_info(tx_ring->dev,
			 "tx hang detected on queue %d, reset requested\n",
			 tx_ring->queue_index);

		/* do not fire the reset immediately, wait for the stack to
		 * decide we are truly stuck, also prevents every queue from
		 * simultaneously requesting a reset
		 */

		/* the adapter is about to reset, no point in enabling polling */
		budget = 1;
	}

	netdev_tx_completed_queue(netdev_get_tx_queue(tx_ring->netdev,
						      tx_ring->queue_index),
				  total_packets, total_bytes);

#define TX_WAKE_THRESHOLD (DESC_NEEDED * 2)
	if (unlikely(total_packets && netif_carrier_ok(tx_ring->netdev) &&
		     (I40E_DESC_UNUSED(tx_ring) >= TX_WAKE_THRESHOLD))) {
		/* Make sure that anybody stopping the queue after this
		 * sees the new next_to_clean.
		 */
		smp_mb();
		if (__netif_subqueue_stopped(tx_ring->netdev,
					     tx_ring->queue_index) &&
		   !test_bit(__I40E_DOWN, &tx_ring->vsi->state)) {
			netif_wake_subqueue(tx_ring->netdev,
					    tx_ring->queue_index);
			++tx_ring->tx_stats.restart_queue;
		}
	}

	return !!budget;
}

/**
 * i40e_force_wb - Arm hardware to do a wb on noncache aligned descriptors
 * @vsi: the VSI we care about
 * @q_vector: the vector  on which to force writeback
 *
 **/
static void i40e_force_wb(struct i40e_vsi *vsi, struct i40e_q_vector *q_vector)
{
	u32 val = I40E_PFINT_DYN_CTLN_INTENA_MASK |
		  I40E_PFINT_DYN_CTLN_ITR_INDX_MASK | /* set noitr */
		  I40E_PFINT_DYN_CTLN_SWINT_TRIG_MASK |
		  I40E_PFINT_DYN_CTLN_SW_ITR_INDX_ENA_MASK;
		  /* allow 00 to be written to the index */

	wr32(&vsi->back->hw,
	     I40E_PFINT_DYN_CTLN(q_vector->v_idx + vsi->base_vector - 1),
	     val);
}

/**
 * i40e_set_new_dynamic_itr - Find new ITR level
 * @rc: structure containing ring performance data
 *
 * Stores a new ITR value based on packets and byte counts during
 * the last interrupt.  The advantage of per interrupt computation
 * is faster updates and more accurate ITR for the current traffic
 * pattern.  Constants in this function were computed based on
 * theoretical maximum wire speed and thresholds were set based on
 * testing data as well as attempting to minimize response time
 * while increasing bulk throughput.
 **/
static void i40e_set_new_dynamic_itr(struct i40e_ring_container *rc)
{
	enum i40e_latency_range new_latency_range = rc->latency_range;
	u32 new_itr = rc->itr;
	int bytes_per_int;

	if (rc->total_packets == 0 || !rc->itr)
		return;

	/* simple throttlerate management
	 *   0-10MB/s   lowest (100000 ints/s)
	 *  10-20MB/s   low    (20000 ints/s)
	 *  20-1249MB/s bulk   (8000 ints/s)
	 */
	bytes_per_int = rc->total_bytes / rc->itr;
	switch (rc->itr) {
	case I40E_LOWEST_LATENCY:
		if (bytes_per_int > 10)
			new_latency_range = I40E_LOW_LATENCY;
		break;
	case I40E_LOW_LATENCY:
		if (bytes_per_int > 20)
			new_latency_range = I40E_BULK_LATENCY;
		else if (bytes_per_int <= 10)
			new_latency_range = I40E_LOWEST_LATENCY;
		break;
	case I40E_BULK_LATENCY:
		if (bytes_per_int <= 20)
			rc->latency_range = I40E_LOW_LATENCY;
		break;
	}

	switch (new_latency_range) {
	case I40E_LOWEST_LATENCY:
		new_itr = I40E_ITR_100K;
		break;
	case I40E_LOW_LATENCY:
		new_itr = I40E_ITR_20K;
		break;
	case I40E_BULK_LATENCY:
		new_itr = I40E_ITR_8K;
		break;
	default:
		break;
	}

	if (new_itr != rc->itr) {
		/* do an exponential smoothing */
		new_itr = (10 * new_itr * rc->itr) /
			  ((9 * new_itr) + rc->itr);
		rc->itr = new_itr & I40E_MAX_ITR;
	}

	rc->total_bytes = 0;
	rc->total_packets = 0;
}

/**
 * i40e_update_dynamic_itr - Adjust ITR based on bytes per int
 * @q_vector: the vector to adjust
 **/
static void i40e_update_dynamic_itr(struct i40e_q_vector *q_vector)
{
	u16 vector = q_vector->vsi->base_vector + q_vector->v_idx;
	struct i40e_hw *hw = &q_vector->vsi->back->hw;
	u32 reg_addr;
	u16 old_itr;

	reg_addr = I40E_PFINT_ITRN(I40E_RX_ITR, vector - 1);
	old_itr = q_vector->rx.itr;
	i40e_set_new_dynamic_itr(&q_vector->rx);
	if (old_itr != q_vector->rx.itr)
		wr32(hw, reg_addr, q_vector->rx.itr);

	reg_addr = I40E_PFINT_ITRN(I40E_TX_ITR, vector - 1);
	old_itr = q_vector->tx.itr;
	i40e_set_new_dynamic_itr(&q_vector->tx);
	if (old_itr != q_vector->tx.itr)
		wr32(hw, reg_addr, q_vector->tx.itr);
}

/**
 * i40e_clean_programming_status - clean the programming status descriptor
 * @rx_ring: the rx ring that has this descriptor
 * @rx_desc: the rx descriptor written back by HW
 *
 * Flow director should handle FD_FILTER_STATUS to check its filter programming
 * status being successful or not and take actions accordingly. FCoE should
 * handle its context/filter programming/invalidation status and take actions.
 *
 **/
static void i40e_clean_programming_status(struct i40e_ring *rx_ring,
					  union i40e_rx_desc *rx_desc)
{
	u64 qw;
	u8 id;

	qw = le64_to_cpu(rx_desc->wb.qword1.status_error_len);
	id = (qw & I40E_RX_PROG_STATUS_DESC_QW1_PROGID_MASK) >>
		  I40E_RX_PROG_STATUS_DESC_QW1_PROGID_SHIFT;

	if (id == I40E_RX_PROG_STATUS_DESC_FD_FILTER_STATUS)
		i40e_fd_handle_status(rx_ring, rx_desc, id);
#ifdef I40E_FCOE
	else if ((id == I40E_RX_PROG_STATUS_DESC_FCOE_CTXT_PROG_STATUS) ||
		 (id == I40E_RX_PROG_STATUS_DESC_FCOE_CTXT_INVL_STATUS))
		i40e_fcoe_handle_status(rx_ring, rx_desc, id);
#endif
}

/**
 * i40e_setup_tx_descriptors - Allocate the Tx descriptors
 * @tx_ring: the tx ring to set up
 *
 * Return 0 on success, negative on error
 **/
int i40e_setup_tx_descriptors(struct i40e_ring *tx_ring)
{
	struct device *dev = tx_ring->dev;
	int bi_size;

	if (!dev)
		return -ENOMEM;

	bi_size = sizeof(struct i40e_tx_buffer) * tx_ring->count;
	tx_ring->tx_bi = kzalloc(bi_size, GFP_KERNEL);
	if (!tx_ring->tx_bi)
		goto err;

	/* round up to nearest 4K */
	tx_ring->size = tx_ring->count * sizeof(struct i40e_tx_desc);
	/* add u32 for head writeback, align after this takes care of
	 * guaranteeing this is at least one cache line in size
	 */
	tx_ring->size += sizeof(u32);
	tx_ring->size = ALIGN(tx_ring->size, 4096);
	tx_ring->desc = dma_alloc_coherent(dev, tx_ring->size,
					   &tx_ring->dma, GFP_KERNEL);
	if (!tx_ring->desc) {
		dev_info(dev, "Unable to allocate memory for the Tx descriptor ring, size=%d\n",
			 tx_ring->size);
		goto err;
	}

	tx_ring->next_to_use = 0;
	tx_ring->next_to_clean = 0;
	return 0;

err:
	kfree(tx_ring->tx_bi);
	tx_ring->tx_bi = NULL;
	return -ENOMEM;
}

/**
 * i40e_clean_rx_ring - Free Rx buffers
 * @rx_ring: ring to be cleaned
 **/
void i40e_clean_rx_ring(struct i40e_ring *rx_ring)
{
	struct device *dev = rx_ring->dev;
	struct i40e_rx_buffer *rx_bi;
	unsigned long bi_size;
	u16 i;

	/* ring already cleared, nothing to do */
	if (!rx_ring->rx_bi)
		return;

	if (ring_is_ps_enabled(rx_ring)) {
		int bufsz = ALIGN(rx_ring->rx_hdr_len, 256) * rx_ring->count;

		rx_bi = &rx_ring->rx_bi[0];
		if (rx_bi->hdr_buf) {
			dma_free_coherent(dev,
					  bufsz,
					  rx_bi->hdr_buf,
					  rx_bi->dma);
			for (i = 0; i < rx_ring->count; i++) {
				rx_bi = &rx_ring->rx_bi[i];
				rx_bi->dma = 0;
				rx_bi->hdr_buf = NULL;
			}
		}
	}
	/* Free all the Rx ring sk_buffs */
	for (i = 0; i < rx_ring->count; i++) {
		rx_bi = &rx_ring->rx_bi[i];
		if (rx_bi->dma) {
			dma_unmap_single(dev,
					 rx_bi->dma,
					 rx_ring->rx_buf_len,
					 DMA_FROM_DEVICE);
			rx_bi->dma = 0;
		}
		if (rx_bi->skb) {
			dev_kfree_skb(rx_bi->skb);
			rx_bi->skb = NULL;
		}
		if (rx_bi->page) {
			if (rx_bi->page_dma) {
				dma_unmap_page(dev,
					       rx_bi->page_dma,
					       PAGE_SIZE / 2,
					       DMA_FROM_DEVICE);
				rx_bi->page_dma = 0;
			}
			__free_page(rx_bi->page);
			rx_bi->page = NULL;
			rx_bi->page_offset = 0;
		}
	}

	bi_size = sizeof(struct i40e_rx_buffer) * rx_ring->count;
	memset(rx_ring->rx_bi, 0, bi_size);

	/* Zero out the descriptor ring */
	memset(rx_ring->desc, 0, rx_ring->size);

	rx_ring->next_to_clean = 0;
	rx_ring->next_to_use = 0;
}

/**
 * i40e_free_rx_resources - Free Rx resources
 * @rx_ring: ring to clean the resources from
 *
 * Free all receive software resources
 **/
void i40e_free_rx_resources(struct i40e_ring *rx_ring)
{
	i40e_clean_rx_ring(rx_ring);
	kfree(rx_ring->rx_bi);
	rx_ring->rx_bi = NULL;

	if (rx_ring->desc) {
		dma_free_coherent(rx_ring->dev, rx_ring->size,
				  rx_ring->desc, rx_ring->dma);
		rx_ring->desc = NULL;
	}
}

/**
 * i40e_alloc_rx_headers - allocate rx header buffers
 * @rx_ring: ring to alloc buffers
 *
 * Allocate rx header buffers for the entire ring. As these are static,
 * this is only called when setting up a new ring.
 **/
void i40e_alloc_rx_headers(struct i40e_ring *rx_ring)
{
	struct device *dev = rx_ring->dev;
	struct i40e_rx_buffer *rx_bi;
	dma_addr_t dma;
	void *buffer;
	int buf_size;
	int i;

	if (rx_ring->rx_bi[0].hdr_buf)
		return;
	/* Make sure the buffers don't cross cache line boundaries. */
	buf_size = ALIGN(rx_ring->rx_hdr_len, 256);
	buffer = dma_alloc_coherent(dev, buf_size * rx_ring->count,
				    &dma, GFP_KERNEL);
	if (!buffer)
		return;
	for (i = 0; i < rx_ring->count; i++) {
		rx_bi = &rx_ring->rx_bi[i];
		rx_bi->dma = dma + (i * buf_size);
		rx_bi->hdr_buf = buffer + (i * buf_size);
	}
}

/**
 * i40e_setup_rx_descriptors - Allocate Rx descriptors
 * @rx_ring: Rx descriptor ring (for a specific queue) to setup
 *
 * Returns 0 on success, negative on failure
 **/
int i40e_setup_rx_descriptors(struct i40e_ring *rx_ring)
{
	struct device *dev = rx_ring->dev;
	int bi_size;

	bi_size = sizeof(struct i40e_rx_buffer) * rx_ring->count;
	rx_ring->rx_bi = kzalloc(bi_size, GFP_KERNEL);
	if (!rx_ring->rx_bi)
		goto err;

	u64_stats_init(&rx_ring->syncp);

	/* Round up to nearest 4K */
	rx_ring->size = ring_is_16byte_desc_enabled(rx_ring)
		? rx_ring->count * sizeof(union i40e_16byte_rx_desc)
		: rx_ring->count * sizeof(union i40e_32byte_rx_desc);
	rx_ring->size = ALIGN(rx_ring->size, 4096);
	rx_ring->desc = dma_alloc_coherent(dev, rx_ring->size,
					   &rx_ring->dma, GFP_KERNEL);

	if (!rx_ring->desc) {
		dev_info(dev, "Unable to allocate memory for the Rx descriptor ring, size=%d\n",
			 rx_ring->size);
		goto err;
	}

	rx_ring->next_to_clean = 0;
	rx_ring->next_to_use = 0;

	return 0;
err:
	kfree(rx_ring->rx_bi);
	rx_ring->rx_bi = NULL;
	return -ENOMEM;
}

/**
 * i40e_release_rx_desc - Store the new tail and head values
 * @rx_ring: ring to bump
 * @val: new head index
 **/
static inline void i40e_release_rx_desc(struct i40e_ring *rx_ring, u32 val)
{
	rx_ring->next_to_use = val;
	/* Force memory writes to complete before letting h/w
	 * know there are new descriptors to fetch.  (Only
	 * applicable for weak-ordered memory model archs,
	 * such as IA-64).
	 */
	wmb();
	writel(val, rx_ring->tail);
}

/**
 * i40e_alloc_rx_buffers_ps - Replace used receive buffers; packet split
 * @rx_ring: ring to place buffers on
 * @cleaned_count: number of buffers to replace
 **/
void i40e_alloc_rx_buffers_ps(struct i40e_ring *rx_ring, u16 cleaned_count)
{
	u16 i = rx_ring->next_to_use;
	union i40e_rx_desc *rx_desc;
	struct i40e_rx_buffer *bi;

	/* do nothing if no valid netdev defined */
	if (!rx_ring->netdev || !cleaned_count)
		return;

	while (cleaned_count--) {
		rx_desc = I40E_RX_DESC(rx_ring, i);
		bi = &rx_ring->rx_bi[i];

		if (bi->skb) /* desc is in use */
			goto no_buffers;
		if (!bi->page) {
			bi->page = alloc_page(GFP_ATOMIC);
			if (!bi->page) {
				rx_ring->rx_stats.alloc_page_failed++;
				goto no_buffers;
			}
		}

		if (!bi->page_dma) {
			/* use a half page if we're re-using */
			bi->page_offset ^= PAGE_SIZE / 2;
			bi->page_dma = dma_map_page(rx_ring->dev,
						    bi->page,
						    bi->page_offset,
						    PAGE_SIZE / 2,
						    DMA_FROM_DEVICE);
			if (dma_mapping_error(rx_ring->dev,
					      bi->page_dma)) {
				rx_ring->rx_stats.alloc_page_failed++;
				bi->page_dma = 0;
				goto no_buffers;
			}
		}

		dma_sync_single_range_for_device(rx_ring->dev,
						 bi->dma,
						 0,
						 rx_ring->rx_hdr_len,
						 DMA_FROM_DEVICE);
		/* Refresh the desc even if buffer_addrs didn't change
		 * because each write-back erases this info.
		 */
		rx_desc->read.pkt_addr = cpu_to_le64(bi->page_dma);
		rx_desc->read.hdr_addr = cpu_to_le64(bi->dma);
		i++;
		if (i == rx_ring->count)
			i = 0;
	}

no_buffers:
	if (rx_ring->next_to_use != i)
		i40e_release_rx_desc(rx_ring, i);
}

/**
 * i40e_alloc_rx_buffers_1buf - Replace used receive buffers; single buffer
 * @rx_ring: ring to place buffers on
 * @cleaned_count: number of buffers to replace
 **/
void i40e_alloc_rx_buffers_1buf(struct i40e_ring *rx_ring, u16 cleaned_count)
{
	u16 i = rx_ring->next_to_use;
	union i40e_rx_desc *rx_desc;
	struct i40e_rx_buffer *bi;
	struct sk_buff *skb;

	/* do nothing if no valid netdev defined */
	if (!rx_ring->netdev || !cleaned_count)
		return;

	while (cleaned_count--) {
		rx_desc = I40E_RX_DESC(rx_ring, i);
		bi = &rx_ring->rx_bi[i];
		skb = bi->skb;

		if (!skb) {
			skb = netdev_alloc_skb_ip_align(rx_ring->netdev,
							rx_ring->rx_buf_len);
			if (!skb) {
				rx_ring->rx_stats.alloc_buff_failed++;
				goto no_buffers;
			}
			/* initialize queue mapping */
			skb_record_rx_queue(skb, rx_ring->queue_index);
			bi->skb = skb;
		}

		if (!bi->dma) {
			bi->dma = dma_map_single(rx_ring->dev,
						 skb->data,
						 rx_ring->rx_buf_len,
						 DMA_FROM_DEVICE);
			if (dma_mapping_error(rx_ring->dev, bi->dma)) {
				rx_ring->rx_stats.alloc_buff_failed++;
				bi->dma = 0;
				goto no_buffers;
			}
		}

		rx_desc->read.pkt_addr = cpu_to_le64(bi->dma);
		rx_desc->read.hdr_addr = 0;
		i++;
		if (i == rx_ring->count)
			i = 0;
	}

no_buffers:
	if (rx_ring->next_to_use != i)
		i40e_release_rx_desc(rx_ring, i);
}

/**
 * i40e_receive_skb - Send a completed packet up the stack
 * @rx_ring:  rx ring in play
 * @skb: packet to send up
 * @vlan_tag: vlan tag for packet
 **/
static void i40e_receive_skb(struct i40e_ring *rx_ring,
			     struct sk_buff *skb, u16 vlan_tag)
{
	struct i40e_q_vector *q_vector = rx_ring->q_vector;
	struct i40e_vsi *vsi = rx_ring->vsi;
	u64 flags = vsi->back->flags;

	if (vlan_tag & VLAN_VID_MASK)
		__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), vlan_tag);

	if (flags & I40E_FLAG_IN_NETPOLL)
		netif_rx(skb);
	else
		napi_gro_receive(&q_vector->napi, skb);
}

/**
 * i40e_rx_checksum - Indicate in skb if hw indicated a good cksum
 * @vsi: the VSI we care about
 * @skb: skb currently being received and modified
 * @rx_status: status value of last descriptor in packet
 * @rx_error: error value of last descriptor in packet
 * @rx_ptype: ptype value of last descriptor in packet
 **/
static inline void i40e_rx_checksum(struct i40e_vsi *vsi,
				    struct sk_buff *skb,
				    u32 rx_status,
				    u32 rx_error,
				    u16 rx_ptype)
{
	struct i40e_rx_ptype_decoded decoded = decode_rx_desc_ptype(rx_ptype);
	bool ipv4 = false, ipv6 = false;
	bool ipv4_tunnel, ipv6_tunnel;
	__wsum rx_udp_csum;
	struct iphdr *iph;
	__sum16 csum;

	ipv4_tunnel = (rx_ptype >= I40E_RX_PTYPE_GRENAT4_MAC_PAY3) &&
		     (rx_ptype <= I40E_RX_PTYPE_GRENAT4_MACVLAN_IPV6_ICMP_PAY4);
	ipv6_tunnel = (rx_ptype >= I40E_RX_PTYPE_GRENAT6_MAC_PAY3) &&
		     (rx_ptype <= I40E_RX_PTYPE_GRENAT6_MACVLAN_IPV6_ICMP_PAY4);

	skb->ip_summed = CHECKSUM_NONE;

	/* Rx csum enabled and ip headers found? */
	if (!(vsi->netdev->features & NETIF_F_RXCSUM))
		return;

	/* did the hardware decode the packet and checksum? */
	if (!(rx_status & (1 << I40E_RX_DESC_STATUS_L3L4P_SHIFT)))
		return;

	/* both known and outer_ip must be set for the below code to work */
	if (!(decoded.known && decoded.outer_ip))
		return;

	if (decoded.outer_ip == I40E_RX_PTYPE_OUTER_IP &&
	    decoded.outer_ip_ver == I40E_RX_PTYPE_OUTER_IPV4)
		ipv4 = true;
	else if (decoded.outer_ip == I40E_RX_PTYPE_OUTER_IP &&
		 decoded.outer_ip_ver == I40E_RX_PTYPE_OUTER_IPV6)
		ipv6 = true;

	if (ipv4 &&
	    (rx_error & ((1 << I40E_RX_DESC_ERROR_IPE_SHIFT) |
			 (1 << I40E_RX_DESC_ERROR_EIPE_SHIFT))))
		goto checksum_fail;

	/* likely incorrect csum if alternate IP extension headers found */
	if (ipv6 &&
	    rx_status & (1 << I40E_RX_DESC_STATUS_IPV6EXADD_SHIFT))
		/* don't increment checksum err here, non-fatal err */
		return;

	/* there was some L4 error, count error and punt packet to the stack */
	if (rx_error & (1 << I40E_RX_DESC_ERROR_L4E_SHIFT))
		goto checksum_fail;

	/* handle packets that were not able to be checksummed due
	 * to arrival speed, in this case the stack can compute
	 * the csum.
	 */
	if (rx_error & (1 << I40E_RX_DESC_ERROR_PPRS_SHIFT))
		return;

	/* If VXLAN traffic has an outer UDPv4 checksum we need to check
	 * it in the driver, hardware does not do it for us.
	 * Since L3L4P bit was set we assume a valid IHL value (>=5)
	 * so the total length of IPv4 header is IHL*4 bytes
	 * The UDP_0 bit *may* bet set if the *inner* header is UDP
	 */
	if (ipv4_tunnel) {
		skb->transport_header = skb->mac_header +
					sizeof(struct ethhdr) +
					(ip_hdr(skb)->ihl * 4);

		/* Add 4 bytes for VLAN tagged packets */
		skb->transport_header += (skb->protocol == htons(ETH_P_8021Q) ||
					  skb->protocol == htons(ETH_P_8021AD))
					  ? VLAN_HLEN : 0;

		if ((ip_hdr(skb)->protocol == IPPROTO_UDP) &&
		    (udp_hdr(skb)->check != 0)) {
			rx_udp_csum = udp_csum(skb);
			iph = ip_hdr(skb);
			csum = csum_tcpudp_magic(
					iph->saddr, iph->daddr,
					(skb->len - skb_transport_offset(skb)),
					IPPROTO_UDP, rx_udp_csum);

			if (udp_hdr(skb)->check != csum)
				goto checksum_fail;

		} /* else its GRE and so no outer UDP header */
	}

	skb->ip_summed = CHECKSUM_UNNECESSARY;
	skb->csum_level = ipv4_tunnel || ipv6_tunnel;

	return;

checksum_fail:
	vsi->back->hw_csum_rx_error++;
}

/**
 * i40e_rx_hash - returns the hash value from the Rx descriptor
 * @ring: descriptor ring
 * @rx_desc: specific descriptor
 **/
static inline u32 i40e_rx_hash(struct i40e_ring *ring,
			       union i40e_rx_desc *rx_desc)
{
	const __le64 rss_mask =
		cpu_to_le64((u64)I40E_RX_DESC_FLTSTAT_RSS_HASH <<
			    I40E_RX_DESC_STATUS_FLTSTAT_SHIFT);

	if ((ring->netdev->features & NETIF_F_RXHASH) &&
	    (rx_desc->wb.qword1.status_error_len & rss_mask) == rss_mask)
		return le32_to_cpu(rx_desc->wb.qword0.hi_dword.rss);
	else
		return 0;
}

/**
 * i40e_ptype_to_hash - get a hash type
 * @ptype: the ptype value from the descriptor
 *
 * Returns a hash type to be used by skb_set_hash
 **/
static inline enum pkt_hash_types i40e_ptype_to_hash(u8 ptype)
{
	struct i40e_rx_ptype_decoded decoded = decode_rx_desc_ptype(ptype);

	if (!decoded.known)
		return PKT_HASH_TYPE_NONE;

	if (decoded.outer_ip == I40E_RX_PTYPE_OUTER_IP &&
	    decoded.payload_layer == I40E_RX_PTYPE_PAYLOAD_LAYER_PAY4)
		return PKT_HASH_TYPE_L4;
	else if (decoded.outer_ip == I40E_RX_PTYPE_OUTER_IP &&
		 decoded.payload_layer == I40E_RX_PTYPE_PAYLOAD_LAYER_PAY3)
		return PKT_HASH_TYPE_L3;
	else
		return PKT_HASH_TYPE_L2;
}

/**
 * i40e_clean_rx_irq_ps - Reclaim resources after receive; packet split
 * @rx_ring:  rx ring to clean
 * @budget:   how many cleans we're allowed
 *
 * Returns true if there's any budget left (e.g. the clean is finished)
 **/
static int i40e_clean_rx_irq_ps(struct i40e_ring *rx_ring, int budget)
{
	unsigned int total_rx_bytes = 0, total_rx_packets = 0;
	u16 rx_packet_len, rx_header_len, rx_sph, rx_hbo;
	u16 cleaned_count = I40E_DESC_UNUSED(rx_ring);
	const int current_node = numa_node_id();
	struct i40e_vsi *vsi = rx_ring->vsi;
	u16 i = rx_ring->next_to_clean;
	union i40e_rx_desc *rx_desc;
	u32 rx_error, rx_status;
	u8 rx_ptype;
	u64 qword;

	if (budget <= 0)
		return 0;

	do {
		struct i40e_rx_buffer *rx_bi;
		struct sk_buff *skb;
		u16 vlan_tag;
		/* return some buffers to hardware, one at a time is too slow */
		if (cleaned_count >= I40E_RX_BUFFER_WRITE) {
			i40e_alloc_rx_buffers_ps(rx_ring, cleaned_count);
			cleaned_count = 0;
		}

		i = rx_ring->next_to_clean;
		rx_desc = I40E_RX_DESC(rx_ring, i);
		qword = le64_to_cpu(rx_desc->wb.qword1.status_error_len);
		rx_status = (qword & I40E_RXD_QW1_STATUS_MASK) >>
			I40E_RXD_QW1_STATUS_SHIFT;

		if (!(rx_status & (1 << I40E_RX_DESC_STATUS_DD_SHIFT)))
			break;

		/* This memory barrier is needed to keep us from reading
		 * any other fields out of the rx_desc until we know the
		 * DD bit is set.
		 */
		dma_rmb();
		if (i40e_rx_is_programming_status(qword)) {
			i40e_clean_programming_status(rx_ring, rx_desc);
			I40E_RX_INCREMENT(rx_ring, i);
			continue;
		}
		rx_bi = &rx_ring->rx_bi[i];
		skb = rx_bi->skb;
		if (likely(!skb)) {
			skb = netdev_alloc_skb_ip_align(rx_ring->netdev,
							rx_ring->rx_hdr_len);
			if (!skb) {
				rx_ring->rx_stats.alloc_buff_failed++;
				break;
			}

			/* initialize queue mapping */
			skb_record_rx_queue(skb, rx_ring->queue_index);
			/* we are reusing so sync this buffer for CPU use */
			dma_sync_single_range_for_cpu(rx_ring->dev,
						      rx_bi->dma,
						      0,
						      rx_ring->rx_hdr_len,
						      DMA_FROM_DEVICE);
		}
		rx_packet_len = (qword & I40E_RXD_QW1_LENGTH_PBUF_MASK) >>
				I40E_RXD_QW1_LENGTH_PBUF_SHIFT;
		rx_header_len = (qword & I40E_RXD_QW1_LENGTH_HBUF_MASK) >>
				I40E_RXD_QW1_LENGTH_HBUF_SHIFT;
		rx_sph = (qword & I40E_RXD_QW1_LENGTH_SPH_MASK) >>
			 I40E_RXD_QW1_LENGTH_SPH_SHIFT;

		rx_error = (qword & I40E_RXD_QW1_ERROR_MASK) >>
			   I40E_RXD_QW1_ERROR_SHIFT;
		rx_hbo = rx_error & (1 << I40E_RX_DESC_ERROR_HBO_SHIFT);
		rx_error &= ~(1 << I40E_RX_DESC_ERROR_HBO_SHIFT);

		rx_ptype = (qword & I40E_RXD_QW1_PTYPE_MASK) >>
			   I40E_RXD_QW1_PTYPE_SHIFT;
		prefetch(rx_bi->page);
		rx_bi->skb = NULL;
		cleaned_count++;
		if (rx_hbo || rx_sph) {
			int len;
			if (rx_hbo)
				len = I40E_RX_HDR_SIZE;
			else
				len = rx_header_len;
			memcpy(__skb_put(skb, len), rx_bi->hdr_buf, len);
		} else if (skb->len == 0) {
			int len;

			len = (rx_packet_len > skb_headlen(skb) ?
				skb_headlen(skb) : rx_packet_len);
			memcpy(__skb_put(skb, len),
			       rx_bi->page + rx_bi->page_offset,
			       len);
			rx_bi->page_offset += len;
			rx_packet_len -= len;
		}

		/* Get the rest of the data if this was a header split */
		if (rx_packet_len) {
			skb_fill_page_desc(skb, skb_shinfo(skb)->nr_frags,
					   rx_bi->page,
					   rx_bi->page_offset,
					   rx_packet_len);

			skb->len += rx_packet_len;
			skb->data_len += rx_packet_len;
			skb->truesize += rx_packet_len;

			if ((page_count(rx_bi->page) == 1) &&
			    (page_to_nid(rx_bi->page) == current_node))
				get_page(rx_bi->page);
			else
				rx_bi->page = NULL;

			dma_unmap_page(rx_ring->dev,
				       rx_bi->page_dma,
				       PAGE_SIZE / 2,
				       DMA_FROM_DEVICE);
			rx_bi->page_dma = 0;
		}
		I40E_RX_INCREMENT(rx_ring, i);

		if (unlikely(
		    !(rx_status & (1 << I40E_RX_DESC_STATUS_EOF_SHIFT)))) {
			struct i40e_rx_buffer *next_buffer;

			next_buffer = &rx_ring->rx_bi[i];
			next_buffer->skb = skb;
			rx_ring->rx_stats.non_eop_descs++;
			continue;
		}

		/* ERR_MASK will only have valid bits if EOP set */
		if (unlikely(rx_error & (1 << I40E_RX_DESC_ERROR_RXE_SHIFT))) {
			dev_kfree_skb_any(skb);
			/* TODO: shouldn't we increment a counter indicating the
			 * drop?
			 */
			continue;
		}

		skb_set_hash(skb, i40e_rx_hash(rx_ring, rx_desc),
			     i40e_ptype_to_hash(rx_ptype));
		if (unlikely(rx_status & I40E_RXD_QW1_STATUS_TSYNVALID_MASK)) {
			i40e_ptp_rx_hwtstamp(vsi->back, skb, (rx_status &
					   I40E_RXD_QW1_STATUS_TSYNINDX_MASK) >>
					   I40E_RXD_QW1_STATUS_TSYNINDX_SHIFT);
			rx_ring->last_rx_timestamp = jiffies;
		}

		/* probably a little skewed due to removing CRC */
		total_rx_bytes += skb->len;
		total_rx_packets++;

		skb->protocol = eth_type_trans(skb, rx_ring->netdev);

		i40e_rx_checksum(vsi, skb, rx_status, rx_error, rx_ptype);

		vlan_tag = rx_status & (1 << I40E_RX_DESC_STATUS_L2TAG1P_SHIFT)
			 ? le16_to_cpu(rx_desc->wb.qword0.lo_dword.l2tag1)
			 : 0;
#ifdef I40E_FCOE
		if (!i40e_fcoe_handle_offload(rx_ring, rx_desc, skb)) {
			dev_kfree_skb_any(skb);
			continue;
		}
#endif
		skb_mark_napi_id(skb, &rx_ring->q_vector->napi);
		i40e_receive_skb(rx_ring, skb, vlan_tag);

		rx_ring->netdev->last_rx = jiffies;
		rx_desc->wb.qword1.status_error_len = 0;

	} while (likely(total_rx_packets < budget));

	u64_stats_update_begin(&rx_ring->syncp);
	rx_ring->stats.packets += total_rx_packets;
	rx_ring->stats.bytes += total_rx_bytes;
	u64_stats_update_end(&rx_ring->syncp);
	rx_ring->q_vector->rx.total_packets += total_rx_packets;
	rx_ring->q_vector->rx.total_bytes += total_rx_bytes;

	return total_rx_packets;
}

/**
 * i40e_clean_rx_irq_1buf - Reclaim resources after receive; single buffer
 * @rx_ring:  rx ring to clean
 * @budget:   how many cleans we're allowed
 *
 * Returns number of packets cleaned
 **/
static int i40e_clean_rx_irq_1buf(struct i40e_ring *rx_ring, int budget)
{
	unsigned int total_rx_bytes = 0, total_rx_packets = 0;
	u16 cleaned_count = I40E_DESC_UNUSED(rx_ring);
	struct i40e_vsi *vsi = rx_ring->vsi;
	union i40e_rx_desc *rx_desc;
	u32 rx_error, rx_status;
	u16 rx_packet_len;
	u8 rx_ptype;
	u64 qword;
	u16 i;

	do {
		struct i40e_rx_buffer *rx_bi;
		struct sk_buff *skb;
		u16 vlan_tag;
		/* return some buffers to hardware, one at a time is too slow */
		if (cleaned_count >= I40E_RX_BUFFER_WRITE) {
			i40e_alloc_rx_buffers_1buf(rx_ring, cleaned_count);
			cleaned_count = 0;
		}

		i = rx_ring->next_to_clean;
		rx_desc = I40E_RX_DESC(rx_ring, i);
		qword = le64_to_cpu(rx_desc->wb.qword1.status_error_len);
		rx_status = (qword & I40E_RXD_QW1_STATUS_MASK) >>
			I40E_RXD_QW1_STATUS_SHIFT;

		if (!(rx_status & (1 << I40E_RX_DESC_STATUS_DD_SHIFT)))
			break;

		/* This memory barrier is needed to keep us from reading
		 * any other fields out of the rx_desc until we know the
		 * DD bit is set.
		 */
		dma_rmb();

		if (i40e_rx_is_programming_status(qword)) {
			i40e_clean_programming_status(rx_ring, rx_desc);
			I40E_RX_INCREMENT(rx_ring, i);
			continue;
		}
		rx_bi = &rx_ring->rx_bi[i];
		skb = rx_bi->skb;
		prefetch(skb->data);

		rx_packet_len = (qword & I40E_RXD_QW1_LENGTH_PBUF_MASK) >>
				I40E_RXD_QW1_LENGTH_PBUF_SHIFT;

		rx_error = (qword & I40E_RXD_QW1_ERROR_MASK) >>
			   I40E_RXD_QW1_ERROR_SHIFT;
		rx_error &= ~(1 << I40E_RX_DESC_ERROR_HBO_SHIFT);

		rx_ptype = (qword & I40E_RXD_QW1_PTYPE_MASK) >>
			   I40E_RXD_QW1_PTYPE_SHIFT;
		rx_bi->skb = NULL;
		cleaned_count++;

		/* Get the header and possibly the whole packet
		 * If this is an skb from previous receive dma will be 0
		 */
		skb_put(skb, rx_packet_len);
		dma_unmap_single(rx_ring->dev, rx_bi->dma, rx_ring->rx_buf_len,
				 DMA_FROM_DEVICE);
		rx_bi->dma = 0;

		I40E_RX_INCREMENT(rx_ring, i);

		if (unlikely(
		    !(rx_status & (1 << I40E_RX_DESC_STATUS_EOF_SHIFT)))) {
			rx_ring->rx_stats.non_eop_descs++;
			continue;
		}

		/* ERR_MASK will only have valid bits if EOP set */
		if (unlikely(rx_error & (1 << I40E_RX_DESC_ERROR_RXE_SHIFT))) {
			dev_kfree_skb_any(skb);
			/* TODO: shouldn't we increment a counter indicating the
			 * drop?
			 */
			continue;
		}

		skb_set_hash(skb, i40e_rx_hash(rx_ring, rx_desc),
			     i40e_ptype_to_hash(rx_ptype));
		if (unlikely(rx_status & I40E_RXD_QW1_STATUS_TSYNVALID_MASK)) {
			i40e_ptp_rx_hwtstamp(vsi->back, skb, (rx_status &
					   I40E_RXD_QW1_STATUS_TSYNINDX_MASK) >>
					   I40E_RXD_QW1_STATUS_TSYNINDX_SHIFT);
			rx_ring->last_rx_timestamp = jiffies;
		}

		/* probably a little skewed due to removing CRC */
		total_rx_bytes += skb->len;
		total_rx_packets++;

		skb->protocol = eth_type_trans(skb, rx_ring->netdev);

		i40e_rx_checksum(vsi, skb, rx_status, rx_error, rx_ptype);

		vlan_tag = rx_status & (1 << I40E_RX_DESC_STATUS_L2TAG1P_SHIFT)
			 ? le16_to_cpu(rx_desc->wb.qword0.lo_dword.l2tag1)
			 : 0;
#ifdef I40E_FCOE
		if (!i40e_fcoe_handle_offload(rx_ring, rx_desc, skb)) {
			dev_kfree_skb_any(skb);
			continue;
		}
#endif
		i40e_receive_skb(rx_ring, skb, vlan_tag);

		rx_ring->netdev->last_rx = jiffies;
		rx_desc->wb.qword1.status_error_len = 0;
	} while (likely(total_rx_packets < budget));

	u64_stats_update_begin(&rx_ring->syncp);
	rx_ring->stats.packets += total_rx_packets;
	rx_ring->stats.bytes += total_rx_bytes;
	u64_stats_update_end(&rx_ring->syncp);
	rx_ring->q_vector->rx.total_packets += total_rx_packets;
	rx_ring->q_vector->rx.total_bytes += total_rx_bytes;

	return total_rx_packets;
}

/**
 * i40e_napi_poll - NAPI polling Rx/Tx cleanup routine
 * @napi: napi struct with our devices info in it
 * @budget: amount of work driver is allowed to do this pass, in packets
 *
 * This function will clean all queues associated with a q_vector.
 *
 * Returns the amount of work done
 **/
int i40e_napi_poll(struct napi_struct *napi, int budget)
{
	struct i40e_q_vector *q_vector =
			       container_of(napi, struct i40e_q_vector, napi);
	struct i40e_vsi *vsi = q_vector->vsi;
	struct i40e_ring *ring;
	bool clean_complete = true;
	bool arm_wb = false;
	int budget_per_ring;
	int cleaned;

	if (test_bit(__I40E_DOWN, &vsi->state)) {
		napi_complete(napi);
		return 0;
	}

	/* Since the actual Tx work is minimal, we can give the Tx a larger
	 * budget and be more aggressive about cleaning up the Tx descriptors.
	 */
	i40e_for_each_ring(ring, q_vector->tx) {
		clean_complete &= i40e_clean_tx_irq(ring, vsi->work_limit);
		arm_wb |= ring->arm_wb;
	}

	/* We attempt to distribute budget to each Rx queue fairly, but don't
	 * allow the budget to go below 1 because that would exit polling early.
	 */
	budget_per_ring = max(budget/q_vector->num_ringpairs, 1);

	i40e_for_each_ring(ring, q_vector->rx) {
		if (ring_is_ps_enabled(ring))
			cleaned = i40e_clean_rx_irq_ps(ring, budget_per_ring);
		else
			cleaned = i40e_clean_rx_irq_1buf(ring, budget_per_ring);
		/* if we didn't clean as many as budgeted, we must be done */
		clean_complete &= (budget_per_ring != cleaned);
	}

	/* If work not completed, return budget and polling will return */
	if (!clean_complete) {
		if (arm_wb)
			i40e_force_wb(vsi, q_vector);
		return budget;
	}

	/* Work is done so exit the polling mode and re-enable the interrupt */
	napi_complete(napi);
	if (ITR_IS_DYNAMIC(vsi->rx_itr_setting) ||
	    ITR_IS_DYNAMIC(vsi->tx_itr_setting))
		i40e_update_dynamic_itr(q_vector);

	if (!test_bit(__I40E_DOWN, &vsi->state)) {
		if (vsi->back->flags & I40E_FLAG_MSIX_ENABLED) {
			i40e_irq_dynamic_enable(vsi,
					q_vector->v_idx + vsi->base_vector);
		} else {
			struct i40e_hw *hw = &vsi->back->hw;
			/* We re-enable the queue 0 cause, but
			 * don't worry about dynamic_enable
			 * because we left it on for the other
			 * possible interrupts during napi
			 */
			u32 qval = rd32(hw, I40E_QINT_RQCTL(0));
			qval |= I40E_QINT_RQCTL_CAUSE_ENA_MASK;
			wr32(hw, I40E_QINT_RQCTL(0), qval);

			qval = rd32(hw, I40E_QINT_TQCTL(0));
			qval |= I40E_QINT_TQCTL_CAUSE_ENA_MASK;
			wr32(hw, I40E_QINT_TQCTL(0), qval);

			i40e_irq_dynamic_enable_icr0(vsi->back);
		}
	}

	return 0;
}

/**
 * i40e_atr - Add a Flow Director ATR filter
 * @tx_ring:  ring to add programming descriptor to
 * @skb:      send buffer
 * @flags:    send flags
 * @protocol: wire protocol
 **/
static void i40e_atr(struct i40e_ring *tx_ring, struct sk_buff *skb,
		     u32 flags, __be16 protocol)
{
	struct i40e_filter_program_desc *fdir_desc;
	struct i40e_pf *pf = tx_ring->vsi->back;
	union {
		unsigned char *network;
		struct iphdr *ipv4;
		struct ipv6hdr *ipv6;
	} hdr;
	struct tcphdr *th;
	unsigned int hlen;
	u32 flex_ptype, dtype_cmd;
	u16 i;

	/* make sure ATR is enabled */
	if (!(pf->flags & I40E_FLAG_FD_ATR_ENABLED))
		return;

	if ((pf->auto_disable_flags & I40E_FLAG_FD_ATR_ENABLED))
		return;

	/* if sampling is disabled do nothing */
	if (!tx_ring->atr_sample_rate)
		return;

	/* snag network header to get L4 type and address */
	hdr.network = skb_network_header(skb);

	/* Currently only IPv4/IPv6 with TCP is supported */
	if (protocol == htons(ETH_P_IP)) {
		if (hdr.ipv4->protocol != IPPROTO_TCP)
			return;

		/* access ihl as a u8 to avoid unaligned access on ia64 */
		hlen = (hdr.network[0] & 0x0F) << 2;
	} else if (protocol == htons(ETH_P_IPV6)) {
		if (hdr.ipv6->nexthdr != IPPROTO_TCP)
			return;

		hlen = sizeof(struct ipv6hdr);
	} else {
		return;
	}

	th = (struct tcphdr *)(hdr.network + hlen);

	/* Due to lack of space, no more new filters can be programmed */
	if (th->syn && (pf->auto_disable_flags & I40E_FLAG_FD_ATR_ENABLED))
		return;

	tx_ring->atr_count++;

	/* sample on all syn/fin/rst packets or once every atr sample rate */
	if (!th->fin &&
	    !th->syn &&
	    !th->rst &&
	    (tx_ring->atr_count < tx_ring->atr_sample_rate))
		return;

	tx_ring->atr_count = 0;

	/* grab the next descriptor */
	i = tx_ring->next_to_use;
	fdir_desc = I40E_TX_FDIRDESC(tx_ring, i);

	i++;
	tx_ring->next_to_use = (i < tx_ring->count) ? i : 0;

	flex_ptype = (tx_ring->queue_index << I40E_TXD_FLTR_QW0_QINDEX_SHIFT) &
		      I40E_TXD_FLTR_QW0_QINDEX_MASK;
	flex_ptype |= (protocol == htons(ETH_P_IP)) ?
		      (I40E_FILTER_PCTYPE_NONF_IPV4_TCP <<
		       I40E_TXD_FLTR_QW0_PCTYPE_SHIFT) :
		      (I40E_FILTER_PCTYPE_NONF_IPV6_TCP <<
		       I40E_TXD_FLTR_QW0_PCTYPE_SHIFT);

	flex_ptype |= tx_ring->vsi->id << I40E_TXD_FLTR_QW0_DEST_VSI_SHIFT;

	dtype_cmd = I40E_TX_DESC_DTYPE_FILTER_PROG;

	dtype_cmd |= (th->fin || th->rst) ?
		     (I40E_FILTER_PROGRAM_DESC_PCMD_REMOVE <<
		      I40E_TXD_FLTR_QW1_PCMD_SHIFT) :
		     (I40E_FILTER_PROGRAM_DESC_PCMD_ADD_UPDATE <<
		      I40E_TXD_FLTR_QW1_PCMD_SHIFT);

	dtype_cmd |= I40E_FILTER_PROGRAM_DESC_DEST_DIRECT_PACKET_QINDEX <<
		     I40E_TXD_FLTR_QW1_DEST_SHIFT;

	dtype_cmd |= I40E_FILTER_PROGRAM_DESC_FD_STATUS_FD_ID <<
		     I40E_TXD_FLTR_QW1_FD_STATUS_SHIFT;

	dtype_cmd |= I40E_TXD_FLTR_QW1_CNT_ENA_MASK;
	dtype_cmd |=
		((u32)pf->fd_atr_cnt_idx << I40E_TXD_FLTR_QW1_CNTINDEX_SHIFT) &
		I40E_TXD_FLTR_QW1_CNTINDEX_MASK;

	fdir_desc->qindex_flex_ptype_vsi = cpu_to_le32(flex_ptype);
	fdir_desc->rsvd = cpu_to_le32(0);
	fdir_desc->dtype_cmd_cntindex = cpu_to_le32(dtype_cmd);
	fdir_desc->fd_id = cpu_to_le32(0);
}

/**
 * i40e_tx_prepare_vlan_flags - prepare generic TX VLAN tagging flags for HW
 * @skb:     send buffer
 * @tx_ring: ring to send buffer on
 * @flags:   the tx flags to be set
 *
 * Checks the skb and set up correspondingly several generic transmit flags
 * related to VLAN tagging for the HW, such as VLAN, DCB, etc.
 *
 * Returns error code indicate the frame should be dropped upon error and the
 * otherwise  returns 0 to indicate the flags has been set properly.
 **/
#ifdef I40E_FCOE
int i40e_tx_prepare_vlan_flags(struct sk_buff *skb,
			       struct i40e_ring *tx_ring,
			       u32 *flags)
#else
static int i40e_tx_prepare_vlan_flags(struct sk_buff *skb,
				      struct i40e_ring *tx_ring,
				      u32 *flags)
#endif
{
	__be16 protocol = skb->protocol;
	u32  tx_flags = 0;

	if (protocol == htons(ETH_P_8021Q) &&
	    !(tx_ring->netdev->features & NETIF_F_HW_VLAN_CTAG_TX)) {
		/* When HW VLAN acceleration is turned off by the user the
		 * stack sets the protocol to 8021q so that the driver
		 * can take any steps required to support the SW only
		 * VLAN handling.  In our case the driver doesn't need
		 * to take any further steps so just set the protocol
		 * to the encapsulated ethertype.
		 */
		skb->protocol = vlan_get_protocol(skb);
		goto out;
	}

	/* if we have a HW VLAN tag being added, default to the HW one */
	if (skb_vlan_tag_present(skb)) {
		tx_flags |= skb_vlan_tag_get(skb) << I40E_TX_FLAGS_VLAN_SHIFT;
		tx_flags |= I40E_TX_FLAGS_HW_VLAN;
	/* else if it is a SW VLAN, check the next protocol and store the tag */
	} else if (protocol == htons(ETH_P_8021Q)) {
		struct vlan_hdr *vhdr, _vhdr;
		vhdr = skb_header_pointer(skb, ETH_HLEN, sizeof(_vhdr), &_vhdr);
		if (!vhdr)
			return -EINVAL;

		protocol = vhdr->h_vlan_encapsulated_proto;
		tx_flags |= ntohs(vhdr->h_vlan_TCI) << I40E_TX_FLAGS_VLAN_SHIFT;
		tx_flags |= I40E_TX_FLAGS_SW_VLAN;
	}

	if (!(tx_ring->vsi->back->flags & I40E_FLAG_DCB_ENABLED))
		goto out;

	/* Insert 802.1p priority into VLAN header */
	if ((tx_flags & (I40E_TX_FLAGS_HW_VLAN | I40E_TX_FLAGS_SW_VLAN)) ||
	    (skb->priority != TC_PRIO_CONTROL)) {
		tx_flags &= ~I40E_TX_FLAGS_VLAN_PRIO_MASK;
		tx_flags |= (skb->priority & 0x7) <<
				I40E_TX_FLAGS_VLAN_PRIO_SHIFT;
		if (tx_flags & I40E_TX_FLAGS_SW_VLAN) {
			struct vlan_ethhdr *vhdr;
			int rc;

			rc = skb_cow_head(skb, 0);
			if (rc < 0)
				return rc;
			vhdr = (struct vlan_ethhdr *)skb->data;
			vhdr->h_vlan_TCI = htons(tx_flags >>
						 I40E_TX_FLAGS_VLAN_SHIFT);
		} else {
			tx_flags |= I40E_TX_FLAGS_HW_VLAN;
		}
	}

out:
	*flags = tx_flags;
	return 0;
}

/**
 * i40e_tso - set up the tso context descriptor
 * @tx_ring:  ptr to the ring to send
 * @skb:      ptr to the skb we're sending
 * @tx_flags: the collected send information
 * @protocol: the send protocol
 * @hdr_len:  ptr to the size of the packet header
 * @cd_tunneling: ptr to context descriptor bits
 *
 * Returns 0 if no TSO can happen, 1 if tso is going, or error
 **/
static int i40e_tso(struct i40e_ring *tx_ring, struct sk_buff *skb,
		    u32 tx_flags, __be16 protocol, u8 *hdr_len,
		    u64 *cd_type_cmd_tso_mss, u32 *cd_tunneling)
{
	u32 cd_cmd, cd_tso_len, cd_mss;
	struct ipv6hdr *ipv6h;
	struct tcphdr *tcph;
	struct iphdr *iph;
	u32 l4len;
	int err;

	if (!skb_is_gso(skb))
		return 0;

	err = skb_cow_head(skb, 0);
	if (err < 0)
		return err;

	iph = skb->encapsulation ? inner_ip_hdr(skb) : ip_hdr(skb);
	ipv6h = skb->encapsulation ? inner_ipv6_hdr(skb) : ipv6_hdr(skb);

	if (iph->version == 4) {
		tcph = skb->encapsulation ? inner_tcp_hdr(skb) : tcp_hdr(skb);
		iph->tot_len = 0;
		iph->check = 0;
		tcph->check = ~csum_tcpudp_magic(iph->saddr, iph->daddr,
						 0, IPPROTO_TCP, 0);
	} else if (ipv6h->version == 6) {
		tcph = skb->encapsulation ? inner_tcp_hdr(skb) : tcp_hdr(skb);
		ipv6h->payload_len = 0;
		tcph->check = ~csum_ipv6_magic(&ipv6h->saddr, &ipv6h->daddr,
					       0, IPPROTO_TCP, 0);
	}

	l4len = skb->encapsulation ? inner_tcp_hdrlen(skb) : tcp_hdrlen(skb);
	*hdr_len = (skb->encapsulation
		    ? (skb_inner_transport_header(skb) - skb->data)
		    : skb_transport_offset(skb)) + l4len;

	/* find the field values */
	cd_cmd = I40E_TX_CTX_DESC_TSO;
	cd_tso_len = skb->len - *hdr_len;
	cd_mss = skb_shinfo(skb)->gso_size;
	*cd_type_cmd_tso_mss |= ((u64)cd_cmd << I40E_TXD_CTX_QW1_CMD_SHIFT) |
				((u64)cd_tso_len <<
				 I40E_TXD_CTX_QW1_TSO_LEN_SHIFT) |
				((u64)cd_mss << I40E_TXD_CTX_QW1_MSS_SHIFT);
	return 1;
}

/**
 * i40e_tsyn - set up the tsyn context descriptor
 * @tx_ring:  ptr to the ring to send
 * @skb:      ptr to the skb we're sending
 * @tx_flags: the collected send information
 *
 * Returns 0 if no Tx timestamp can happen and 1 if the timestamp will happen
 **/
static int i40e_tsyn(struct i40e_ring *tx_ring, struct sk_buff *skb,
		     u32 tx_flags, u64 *cd_type_cmd_tso_mss)
{
	struct i40e_pf *pf;

	if (likely(!(skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP)))
		return 0;

	/* Tx timestamps cannot be sampled when doing TSO */
	if (tx_flags & I40E_TX_FLAGS_TSO)
		return 0;

	/* only timestamp the outbound packet if the user has requested it and
	 * we are not already transmitting a packet to be timestamped
	 */
	pf = i40e_netdev_to_pf(tx_ring->netdev);
	if (!(pf->flags & I40E_FLAG_PTP))
		return 0;

	if (pf->ptp_tx &&
	    !test_and_set_bit_lock(__I40E_PTP_TX_IN_PROGRESS, &pf->state)) {
		skb_shinfo(skb)->tx_flags |= SKBTX_IN_PROGRESS;
		pf->ptp_tx_skb = skb_get(skb);
	} else {
		return 0;
	}

	*cd_type_cmd_tso_mss |= (u64)I40E_TX_CTX_DESC_TSYN <<
				I40E_TXD_CTX_QW1_CMD_SHIFT;

	return 1;
}

/**
 * i40e_tx_enable_csum - Enable Tx checksum offloads
 * @skb: send buffer
 * @tx_flags: Tx flags currently set
 * @td_cmd: Tx descriptor command bits to set
 * @td_offset: Tx descriptor header offsets to set
 * @cd_tunneling: ptr to context desc bits
 **/
static void i40e_tx_enable_csum(struct sk_buff *skb, u32 tx_flags,
				u32 *td_cmd, u32 *td_offset,
				struct i40e_ring *tx_ring,
				u32 *cd_tunneling)
{
	struct ipv6hdr *this_ipv6_hdr;
	unsigned int this_tcp_hdrlen;
	struct iphdr *this_ip_hdr;
	u32 network_hdr_len;
	u8 l4_hdr = 0;
	u32 l4_tunnel = 0;

	if (skb->encapsulation) {
		switch (ip_hdr(skb)->protocol) {
		case IPPROTO_UDP:
			l4_tunnel = I40E_TXD_CTX_UDP_TUNNELING;
			break;
		default:
			return;
		}
		network_hdr_len = skb_inner_network_header_len(skb);
		this_ip_hdr = inner_ip_hdr(skb);
		this_ipv6_hdr = inner_ipv6_hdr(skb);
		this_tcp_hdrlen = inner_tcp_hdrlen(skb);

		if (tx_flags & I40E_TX_FLAGS_IPV4) {

			if (tx_flags & I40E_TX_FLAGS_TSO) {
				*cd_tunneling |= I40E_TX_CTX_EXT_IP_IPV4;
				ip_hdr(skb)->check = 0;
			} else {
				*cd_tunneling |=
					 I40E_TX_CTX_EXT_IP_IPV4_NO_CSUM;
			}
		} else if (tx_flags & I40E_TX_FLAGS_IPV6) {
			*cd_tunneling |= I40E_TX_CTX_EXT_IP_IPV6;
			if (tx_flags & I40E_TX_FLAGS_TSO)
				ip_hdr(skb)->check = 0;
		}

		/* Now set the ctx descriptor fields */
		*cd_tunneling |= (skb_network_header_len(skb) >> 2) <<
				   I40E_TXD_CTX_QW0_EXT_IPLEN_SHIFT      |
				   l4_tunnel                             |
				   ((skb_inner_network_offset(skb) -
					skb_transport_offset(skb)) >> 1) <<
				   I40E_TXD_CTX_QW0_NATLEN_SHIFT;
		if (this_ip_hdr->version == 6) {
			tx_flags &= ~I40E_TX_FLAGS_IPV4;
			tx_flags |= I40E_TX_FLAGS_IPV6;
		}
	} else {
		network_hdr_len = skb_network_header_len(skb);
		this_ip_hdr = ip_hdr(skb);
		this_ipv6_hdr = ipv6_hdr(skb);
		this_tcp_hdrlen = tcp_hdrlen(skb);
	}

	/* Enable IP checksum offloads */
	if (tx_flags & I40E_TX_FLAGS_IPV4) {
		l4_hdr = this_ip_hdr->protocol;
		/* the stack computes the IP header already, the only time we
		 * need the hardware to recompute it is in the case of TSO.
		 */
		if (tx_flags & I40E_TX_FLAGS_TSO) {
			*td_cmd |= I40E_TX_DESC_CMD_IIPT_IPV4_CSUM;
			this_ip_hdr->check = 0;
		} else {
			*td_cmd |= I40E_TX_DESC_CMD_IIPT_IPV4;
		}
		/* Now set the td_offset for IP header length */
		*td_offset = (network_hdr_len >> 2) <<
			      I40E_TX_DESC_LENGTH_IPLEN_SHIFT;
	} else if (tx_flags & I40E_TX_FLAGS_IPV6) {
		l4_hdr = this_ipv6_hdr->nexthdr;
		*td_cmd |= I40E_TX_DESC_CMD_IIPT_IPV6;
		/* Now set the td_offset for IP header length */
		*td_offset = (network_hdr_len >> 2) <<
			      I40E_TX_DESC_LENGTH_IPLEN_SHIFT;
	}
	/* words in MACLEN + dwords in IPLEN + dwords in L4Len */
	*td_offset |= (skb_network_offset(skb) >> 1) <<
		       I40E_TX_DESC_LENGTH_MACLEN_SHIFT;

	/* Enable L4 checksum offloads */
	switch (l4_hdr) {
	case IPPROTO_TCP:
		/* enable checksum offloads */
		*td_cmd |= I40E_TX_DESC_CMD_L4T_EOFT_TCP;
		*td_offset |= (this_tcp_hdrlen >> 2) <<
			       I40E_TX_DESC_LENGTH_L4_FC_LEN_SHIFT;
		break;
	case IPPROTO_SCTP:
		/* enable SCTP checksum offload */
		*td_cmd |= I40E_TX_DESC_CMD_L4T_EOFT_SCTP;
		*td_offset |= (sizeof(struct sctphdr) >> 2) <<
			       I40E_TX_DESC_LENGTH_L4_FC_LEN_SHIFT;
		break;
	case IPPROTO_UDP:
		/* enable UDP checksum offload */
		*td_cmd |= I40E_TX_DESC_CMD_L4T_EOFT_UDP;
		*td_offset |= (sizeof(struct udphdr) >> 2) <<
			       I40E_TX_DESC_LENGTH_L4_FC_LEN_SHIFT;
		break;
	default:
		break;
	}
}

/**
 * i40e_create_tx_ctx Build the Tx context descriptor
 * @tx_ring:  ring to create the descriptor on
 * @cd_type_cmd_tso_mss: Quad Word 1
 * @cd_tunneling: Quad Word 0 - bits 0-31
 * @cd_l2tag2: Quad Word 0 - bits 32-63
 **/
static void i40e_create_tx_ctx(struct i40e_ring *tx_ring,
			       const u64 cd_type_cmd_tso_mss,
			       const u32 cd_tunneling, const u32 cd_l2tag2)
{
	struct i40e_tx_context_desc *context_desc;
	int i = tx_ring->next_to_use;

	if ((cd_type_cmd_tso_mss == I40E_TX_DESC_DTYPE_CONTEXT) &&
	    !cd_tunneling && !cd_l2tag2)
		return;

	/* grab the next descriptor */
	context_desc = I40E_TX_CTXTDESC(tx_ring, i);

	i++;
	tx_ring->next_to_use = (i < tx_ring->count) ? i : 0;

	/* cpu_to_le32 and assign to struct fields */
	context_desc->tunneling_params = cpu_to_le32(cd_tunneling);
	context_desc->l2tag2 = cpu_to_le16(cd_l2tag2);
	context_desc->rsvd = cpu_to_le16(0);
	context_desc->type_cmd_tso_mss = cpu_to_le64(cd_type_cmd_tso_mss);
}

/**
 * __i40e_maybe_stop_tx - 2nd level check for tx stop conditions
 * @tx_ring: the ring to be checked
 * @size:    the size buffer we want to assure is available
 *
 * Returns -EBUSY if a stop is needed, else 0
 **/
static inline int __i40e_maybe_stop_tx(struct i40e_ring *tx_ring, int size)
{
	netif_stop_subqueue(tx_ring->netdev, tx_ring->queue_index);
	/* Memory barrier before checking head and tail */
	smp_mb();

	/* Check again in a case another CPU has just made room available. */
	if (likely(I40E_DESC_UNUSED(tx_ring) < size))
		return -EBUSY;

	/* A reprieve! - use start_queue because it doesn't call schedule */
	netif_start_subqueue(tx_ring->netdev, tx_ring->queue_index);
	++tx_ring->tx_stats.restart_queue;
	return 0;
}

/**
 * i40e_maybe_stop_tx - 1st level check for tx stop conditions
 * @tx_ring: the ring to be checked
 * @size:    the size buffer we want to assure is available
 *
 * Returns 0 if stop is not needed
 **/
#ifdef I40E_FCOE
int i40e_maybe_stop_tx(struct i40e_ring *tx_ring, int size)
#else
static int i40e_maybe_stop_tx(struct i40e_ring *tx_ring, int size)
#endif
{
	if (likely(I40E_DESC_UNUSED(tx_ring) >= size))
		return 0;
	return __i40e_maybe_stop_tx(tx_ring, size);
}

/**
 * i40e_chk_linearize - Check if there are more than 8 fragments per packet
 * @skb:      send buffer
 * @tx_flags: collected send information
 *
 * Note: Our HW can't scatter-gather more than 8 fragments to build
 * a packet on the wire and so we need to figure out the cases where we
 * need to linearize the skb.
 **/
static bool i40e_chk_linearize(struct sk_buff *skb, u32 tx_flags)
{
	struct skb_frag_struct *frag;
	bool linearize = false;
	unsigned int size = 0;
	u16 num_frags;
	u16 gso_segs;

	num_frags = skb_shinfo(skb)->nr_frags;
	gso_segs = skb_shinfo(skb)->gso_segs;

	if (tx_flags & (I40E_TX_FLAGS_TSO | I40E_TX_FLAGS_FSO)) {
		u16 j = 0;

		if (num_frags < (I40E_MAX_BUFFER_TXD))
			goto linearize_chk_done;
		/* try the simple math, if we have too many frags per segment */
		if (DIV_ROUND_UP((num_frags + gso_segs), gso_segs) >
		    I40E_MAX_BUFFER_TXD) {
			linearize = true;
			goto linearize_chk_done;
		}
		frag = &skb_shinfo(skb)->frags[0];
		/* we might still have more fragments per segment */
		do {
			size += skb_frag_size(frag);
			frag++; j++;
			if ((size >= skb_shinfo(skb)->gso_size) &&
			    (j < I40E_MAX_BUFFER_TXD)) {
				size = (size % skb_shinfo(skb)->gso_size);
				j = (size) ? 1 : 0;
			}
			if (j == I40E_MAX_BUFFER_TXD) {
				linearize = true;
				break;
			}
			num_frags--;
		} while (num_frags);
	} else {
		if (num_frags >= I40E_MAX_BUFFER_TXD)
			linearize = true;
	}

linearize_chk_done:
	return linearize;
}

/**
 * i40e_tx_map - Build the Tx descriptor
 * @tx_ring:  ring to send buffer on
 * @skb:      send buffer
 * @first:    first buffer info buffer to use
 * @tx_flags: collected send information
 * @hdr_len:  size of the packet header
 * @td_cmd:   the command field in the descriptor
 * @td_offset: offset for checksum or crc
 **/
#ifdef I40E_FCOE
void i40e_tx_map(struct i40e_ring *tx_ring, struct sk_buff *skb,
		 struct i40e_tx_buffer *first, u32 tx_flags,
		 const u8 hdr_len, u32 td_cmd, u32 td_offset)
#else
static void i40e_tx_map(struct i40e_ring *tx_ring, struct sk_buff *skb,
			struct i40e_tx_buffer *first, u32 tx_flags,
			const u8 hdr_len, u32 td_cmd, u32 td_offset)
#endif
{
	unsigned int data_len = skb->data_len;
	unsigned int size = skb_headlen(skb);
	struct skb_frag_struct *frag;
	struct i40e_tx_buffer *tx_bi;
	struct i40e_tx_desc *tx_desc;
	u16 i = tx_ring->next_to_use;
	u32 td_tag = 0;
	dma_addr_t dma;
	u16 gso_segs;

	if (tx_flags & I40E_TX_FLAGS_HW_VLAN) {
		td_cmd |= I40E_TX_DESC_CMD_IL2TAG1;
		td_tag = (tx_flags & I40E_TX_FLAGS_VLAN_MASK) >>
			 I40E_TX_FLAGS_VLAN_SHIFT;
	}

	if (tx_flags & (I40E_TX_FLAGS_TSO | I40E_TX_FLAGS_FSO))
		gso_segs = skb_shinfo(skb)->gso_segs;
	else
		gso_segs = 1;

	/* multiply data chunks by size of headers */
	first->bytecount = skb->len - hdr_len + (gso_segs * hdr_len);
	first->gso_segs = gso_segs;
	first->skb = skb;
	first->tx_flags = tx_flags;

	dma = dma_map_single(tx_ring->dev, skb->data, size, DMA_TO_DEVICE);

	tx_desc = I40E_TX_DESC(tx_ring, i);
	tx_bi = first;

	for (frag = &skb_shinfo(skb)->frags[0];; frag++) {
		if (dma_mapping_error(tx_ring->dev, dma))
			goto dma_error;

		/* record length, and DMA address */
		dma_unmap_len_set(tx_bi, len, size);
		dma_unmap_addr_set(tx_bi, dma, dma);

		tx_desc->buffer_addr = cpu_to_le64(dma);

		while (unlikely(size > I40E_MAX_DATA_PER_TXD)) {
			tx_desc->cmd_type_offset_bsz =
				build_ctob(td_cmd, td_offset,
					   I40E_MAX_DATA_PER_TXD, td_tag);

			tx_desc++;
			i++;
			if (i == tx_ring->count) {
				tx_desc = I40E_TX_DESC(tx_ring, 0);
				i = 0;
			}

			dma += I40E_MAX_DATA_PER_TXD;
			size -= I40E_MAX_DATA_PER_TXD;

			tx_desc->buffer_addr = cpu_to_le64(dma);
		}

		if (likely(!data_len))
			break;

		tx_desc->cmd_type_offset_bsz = build_ctob(td_cmd, td_offset,
							  size, td_tag);

		tx_desc++;
		i++;
		if (i == tx_ring->count) {
			tx_desc = I40E_TX_DESC(tx_ring, 0);
			i = 0;
		}

		size = skb_frag_size(frag);
		data_len -= size;

		dma = skb_frag_dma_map(tx_ring->dev, frag, 0, size,
				       DMA_TO_DEVICE);

		tx_bi = &tx_ring->tx_bi[i];
	}

	/* Place RS bit on last descriptor of any packet that spans across the
	 * 4th descriptor (WB_STRIDE aka 0x3) in a 64B cacheline.
	 */
	if (((i & WB_STRIDE) != WB_STRIDE) &&
	    (first <= &tx_ring->tx_bi[i]) &&
	    (first >= &tx_ring->tx_bi[i & ~WB_STRIDE])) {
		tx_desc->cmd_type_offset_bsz =
			build_ctob(td_cmd, td_offset, size, td_tag) |
			cpu_to_le64((u64)I40E_TX_DESC_CMD_EOP <<
					 I40E_TXD_QW1_CMD_SHIFT);
	} else {
		tx_desc->cmd_type_offset_bsz =
			build_ctob(td_cmd, td_offset, size, td_tag) |
			cpu_to_le64((u64)I40E_TXD_CMD <<
					 I40E_TXD_QW1_CMD_SHIFT);
	}

	netdev_tx_sent_queue(netdev_get_tx_queue(tx_ring->netdev,
						 tx_ring->queue_index),
			     first->bytecount);

	/* set the timestamp */
	first->time_stamp = jiffies;

	/* Force memory writes to complete before letting h/w
	 * know there are new descriptors to fetch.  (Only
	 * applicable for weak-ordered memory model archs,
	 * such as IA-64).
	 */
	wmb();

	/* set next_to_watch value indicating a packet is present */
	first->next_to_watch = tx_desc;

	i++;
	if (i == tx_ring->count)
		i = 0;

	tx_ring->next_to_use = i;

	i40e_maybe_stop_tx(tx_ring, DESC_NEEDED);
	/* notify HW of packet */
	if (!skb->xmit_more ||
	    netif_xmit_stopped(netdev_get_tx_queue(tx_ring->netdev,
						   tx_ring->queue_index)))
		writel(i, tx_ring->tail);

	return;

dma_error:
	dev_info(tx_ring->dev, "TX DMA map failed\n");

	/* clear dma mappings for failed tx_bi map */
	for (;;) {
		tx_bi = &tx_ring->tx_bi[i];
		i40e_unmap_and_free_tx_resource(tx_ring, tx_bi);
		if (tx_bi == first)
			break;
		if (i == 0)
			i = tx_ring->count;
		i--;
	}

	tx_ring->next_to_use = i;
}

/**
 * i40e_xmit_descriptor_count - calculate number of tx descriptors needed
 * @skb:     send buffer
 * @tx_ring: ring to send buffer on
 *
 * Returns number of data descriptors needed for this skb. Returns 0 to indicate
 * there is not enough descriptors available in this ring since we need at least
 * one descriptor.
 **/
#ifdef I40E_FCOE
int i40e_xmit_descriptor_count(struct sk_buff *skb,
			       struct i40e_ring *tx_ring)
#else
static int i40e_xmit_descriptor_count(struct sk_buff *skb,
				      struct i40e_ring *tx_ring)
#endif
{
	unsigned int f;
	int count = 0;

	/* need: 1 descriptor per page * PAGE_SIZE/I40E_MAX_DATA_PER_TXD,
	 *       + 1 desc for skb_head_len/I40E_MAX_DATA_PER_TXD,
	 *       + 4 desc gap to avoid the cache line where head is,
	 *       + 1 desc for context descriptor,
	 * otherwise try next time
	 */
	for (f = 0; f < skb_shinfo(skb)->nr_frags; f++)
		count += TXD_USE_COUNT(skb_shinfo(skb)->frags[f].size);

	count += TXD_USE_COUNT(skb_headlen(skb));
	if (i40e_maybe_stop_tx(tx_ring, count + 4 + 1)) {
		tx_ring->tx_stats.tx_busy++;
		return 0;
	}
	return count;
}

/**
 * i40e_xmit_frame_ring - Sends buffer on Tx ring
 * @skb:     send buffer
 * @tx_ring: ring to send buffer on
 *
 * Returns NETDEV_TX_OK if sent, else an error code
 **/
static netdev_tx_t i40e_xmit_frame_ring(struct sk_buff *skb,
					struct i40e_ring *tx_ring)
{
	u64 cd_type_cmd_tso_mss = I40E_TX_DESC_DTYPE_CONTEXT;
	u32 cd_tunneling = 0, cd_l2tag2 = 0;
	struct i40e_tx_buffer *first;
	u32 td_offset = 0;
	u32 tx_flags = 0;
	__be16 protocol;
	u32 td_cmd = 0;
	u8 hdr_len = 0;
	int tsyn;
	int tso;
	if (0 == i40e_xmit_descriptor_count(skb, tx_ring))
		return NETDEV_TX_BUSY;

	/* prepare the xmit flags */
	if (i40e_tx_prepare_vlan_flags(skb, tx_ring, &tx_flags))
		goto out_drop;

	/* obtain protocol of skb */
	protocol = vlan_get_protocol(skb);

	/* record the location of the first descriptor for this packet */
	first = &tx_ring->tx_bi[tx_ring->next_to_use];

	/* setup IPv4/IPv6 offloads */
	if (protocol == htons(ETH_P_IP))
		tx_flags |= I40E_TX_FLAGS_IPV4;
	else if (protocol == htons(ETH_P_IPV6))
		tx_flags |= I40E_TX_FLAGS_IPV6;

	tso = i40e_tso(tx_ring, skb, tx_flags, protocol, &hdr_len,
		       &cd_type_cmd_tso_mss, &cd_tunneling);

	if (tso < 0)
		goto out_drop;
	else if (tso)
		tx_flags |= I40E_TX_FLAGS_TSO;

	tsyn = i40e_tsyn(tx_ring, skb, tx_flags, &cd_type_cmd_tso_mss);

	if (tsyn)
		tx_flags |= I40E_TX_FLAGS_TSYN;

	if (i40e_chk_linearize(skb, tx_flags))
		if (skb_linearize(skb))
			goto out_drop;

	skb_tx_timestamp(skb);

	/* always enable CRC insertion offload */
	td_cmd |= I40E_TX_DESC_CMD_ICRC;

	/* Always offload the checksum, since it's in the data descriptor */
	if (skb->ip_summed == CHECKSUM_PARTIAL) {
		tx_flags |= I40E_TX_FLAGS_CSUM;

		i40e_tx_enable_csum(skb, tx_flags, &td_cmd, &td_offset,
				    tx_ring, &cd_tunneling);
	}

	i40e_create_tx_ctx(tx_ring, cd_type_cmd_tso_mss,
			   cd_tunneling, cd_l2tag2);

	/* Add Flow Director ATR if it's enabled.
	 *
	 * NOTE: this must always be directly before the data descriptor.
	 */
	i40e_atr(tx_ring, skb, tx_flags, protocol);

	i40e_tx_map(tx_ring, skb, first, tx_flags, hdr_len,
		    td_cmd, td_offset);

	return NETDEV_TX_OK;

out_drop:
	dev_kfree_skb_any(skb);
	return NETDEV_TX_OK;
}

/**
 * i40e_lan_xmit_frame - Selects the correct VSI and Tx queue to send buffer
 * @skb:    send buffer
 * @netdev: network interface device structure
 *
 * Returns NETDEV_TX_OK if sent, else an error code
 **/
netdev_tx_t i40e_lan_xmit_frame(struct sk_buff *skb, struct net_device *netdev)
{
	struct i40e_netdev_priv *np = netdev_priv(netdev);
	struct i40e_vsi *vsi = np->vsi;
	struct i40e_ring *tx_ring = vsi->tx_rings[skb->queue_mapping];

	/* hardware can't handle really short frames, hardware padding works
	 * beyond this point
	 */
	if (skb_put_padto(skb, I40E_MIN_TX_LEN))
		return NETDEV_TX_OK;

	return i40e_xmit_frame_ring(skb, tx_ring);
}
