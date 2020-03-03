/*******************************************************************************
  Specialised functions for managing Chained mode

  Copyright(C) 2011  STMicroelectronics Ltd

  It defines all the functions used to handle the normal/enhanced
  descriptors in case of the DMA is configured to work in chained or
  in ring mode.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Author: Giuseppe Cavallaro <peppe.cavallaro@st.com>
*******************************************************************************/

#include "stmmac.h"

static int stmmac_jumbo_frm(void *p, struct sk_buff *skb, int csum)
{
	struct stmmac_priv *priv = (struct stmmac_priv *)p;
	unsigned int txsize = priv->dma_tx_size;
	unsigned int entry = priv->cur_tx % txsize;
	struct dma_desc *desc = priv->dma_tx + entry;
	unsigned int nopaged_len = skb_headlen(skb);
	unsigned int bmax;
	unsigned int i = 1, len;

	if (priv->plat->enh_desc)
		bmax = BUF_SIZE_8KiB;
	else
		bmax = BUF_SIZE_2KiB;

	len = nopaged_len - bmax;

	desc->des2 = dma_map_single(priv->device, skb->data,
				    bmax, DMA_TO_DEVICE);
	if (dma_mapping_error(priv->device, desc->des2))
		return -1;
	priv->tx_skbuff_dma[entry].buf = desc->des2;
	priv->hw->desc->prepare_tx_desc(desc, 1, bmax, csum, STMMAC_CHAIN_MODE);

	while (len != 0) {
		priv->tx_skbuff[entry] = NULL;
		entry = (++priv->cur_tx) % txsize;
		desc = priv->dma_tx + entry;

		if (len > bmax) {
			desc->des2 = dma_map_single(priv->device,
						    (skb->data + bmax * i),
						    bmax, DMA_TO_DEVICE);
			if (dma_mapping_error(priv->device, desc->des2))
				return -1;
			priv->tx_skbuff_dma[entry].buf = desc->des2;
			priv->hw->desc->prepare_tx_desc(desc, 0, bmax, csum,
							STMMAC_CHAIN_MODE);
			priv->hw->desc->set_tx_owner(desc);
			len -= bmax;
			i++;
		} else {
			desc->des2 = dma_map_single(priv->device,
						    (skb->data + bmax * i), len,
						    DMA_TO_DEVICE);
			if (dma_mapping_error(priv->device, desc->des2))
				return -1;
			priv->tx_skbuff_dma[entry].buf = desc->des2;
			priv->hw->desc->prepare_tx_desc(desc, 0, len, csum,
							STMMAC_CHAIN_MODE);
			priv->hw->desc->set_tx_owner(desc);
			len = 0;
		}
	}
	return entry;
}

static unsigned int stmmac_is_jumbo_frm(int len, int enh_desc)
{
	unsigned int ret = 0;

	if ((enh_desc && (len > BUF_SIZE_8KiB)) ||
	    (!enh_desc && (len > BUF_SIZE_2KiB))) {
		ret = 1;
	}

	return ret;
}

static void stmmac_init_dma_chain(void *des, dma_addr_t phy_addr,
				  unsigned int size, unsigned int extend_desc)
{
	/*
	 * In chained mode the des3 points to the next element in the ring.
	 * The latest element has to point to the head.
	 */
	int i;
	dma_addr_t dma_phy = phy_addr;

	if (extend_desc) {
		struct dma_extended_desc *p = (struct dma_extended_desc *)des;
		for (i = 0; i < (size - 1); i++) {
			dma_phy += sizeof(struct dma_extended_desc);
			p->basic.des3 = (unsigned int)dma_phy;
			p++;
		}
		p->basic.des3 = (unsigned int)phy_addr;

	} else {
		struct dma_desc *p = (struct dma_desc *)des;
		for (i = 0; i < (size - 1); i++) {
			dma_phy += sizeof(struct dma_desc);
			p->des3 = (unsigned int)dma_phy;
			p++;
		}
		p->des3 = (unsigned int)phy_addr;
	}
}

static void stmmac_refill_desc3(void *priv_ptr, struct dma_desc *p)
{
	struct stmmac_priv *priv = (struct stmmac_priv *)priv_ptr;

	if (priv->hwts_rx_en && !priv->extend_desc)
		/* NOTE: Device will overwrite des3 with timestamp value if
		 * 1588-2002 time stamping is enabled, hence reinitialize it
		 * to keep explicit chaining in the descriptor.
		 */
		p->des3 = (unsigned int)(priv->dma_rx_phy +
					 (((priv->dirty_rx) + 1) %
					  priv->dma_rx_size) *
					 sizeof(struct dma_desc));
}

static void stmmac_clean_desc3(void *priv_ptr, struct dma_desc *p)
{
	struct stmmac_priv *priv = (struct stmmac_priv *)priv_ptr;

	if (priv->hw->desc->get_tx_ls(p) && !priv->extend_desc)
		/* NOTE: Device will overwrite des3 with timestamp value if
		 * 1588-2002 time stamping is enabled, hence reinitialize it
		 * to keep explicit chaining in the descriptor.
		 */
		p->des3 = (unsigned int)(priv->dma_tx_phy +
					 (((priv->dirty_tx + 1) %
					   priv->dma_tx_size) *
					  sizeof(struct dma_desc)));
}

const struct stmmac_mode_ops chain_mode_ops = {
	.init = stmmac_init_dma_chain,
	.is_jumbo_frm = stmmac_is_jumbo_frm,
	.jumbo_frm = stmmac_jumbo_frm,
	.refill_desc3 = stmmac_refill_desc3,
	.clean_desc3 = stmmac_clean_desc3,
};
