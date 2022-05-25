/*********************************************************************
 * bcm63xx-i2stdm.h -- ALSA SoC Audio Layer - Broadcom I2S/TDM Controller driver
 * 
 * Copyright (c) 2020 Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2020:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 **********************************************************************/

#ifndef __BCM63XX_I2S_H
#define __BCM63XX_I2S_H

#define I2S_DESC_FIFO_DEPTH		8
#define I2S_MISC_CFG			(0x003C)
#define I2S_PAD_LVL_LOOP_DIS_MASK	(1 << 2)
#define I2S_PAD_LVL_LOOP_DIS_ENABLE	I2S_PAD_LVL_LOOP_DIS_MASK

#define I2S_INDEP_CLK_TXRX_MASK	(1 << 3)
#define I2S_INDEP_CLK_TXRX_EN	I2S_INDEP_CLK_TXRX_MASK

#define I2S_GEN_TX_LRCK_WODATA_MASK	(1 << 4)
#define I2S_GEN_TX_LRCK_WODATA_EN	I2S_GEN_TX_LRCK_WODATA_MASK

#define I2S_TX_ENABLE_MASK		(1 << 31)
#define I2S_TX_ENABLE			I2S_TX_ENABLE_MASK
#define I2S_TX_OUT_R			(1 << 19)
#define I2S_TX_DATA_ALIGNMENT		(1 << 2)
#define I2S_TX_DATA_ENABLE		(1 << 1)
#define I2S_TX_CLOCK_ENABLE		(1 << 0)

#define I2S_TX_DESC_OFF_LEVEL_SHIFT	12
#define I2S_TX_DESC_OFF_LEVEL_MASK	(0x0F << I2S_TX_DESC_OFF_LEVEL_SHIFT)
#define I2S_TX_DESC_IFF_LEVEL_SHIFT	8
#define I2S_TX_DESC_IFF_LEVEL_MASK	(0x0F << I2S_TX_DESC_IFF_LEVEL_SHIFT)
#define I2S_TX_DESC_OFF_INTR_EN_MSK	(1 << 1)
#define I2S_TX_DESC_OFF_INTR_EN		I2S_TX_DESC_OFF_INTR_EN_MSK

#define I2S_TX_CFG			(0x0000)
#define I2S_TX_IRQ_CTL			(0x0004)
#define I2S_TX_IRQ_EN			(0x0008)
#define I2S_TX_IRQ_IFF_THLD		(0x000c)
#define I2S_TX_IRQ_OFF_THLD		(0x0010)
#define I2S_TX_DESC_IFF_ADDR		(0x0014)
#define I2S_TX_DESC_IFF_LEN		(0x0018)
#define I2S_TX_DESC_OFF_ADDR		(0x001C)
#define I2S_TX_DESC_OFF_LEN		(0x0020)
#define I2S_TX_CFG_2			(0x0024)
#define I2S_TX_SLAVE_MODE_SHIFT		13
#define I2S_TX_SLAVE_MODE_MASK		(1 << I2S_TX_SLAVE_MODE_SHIFT)
#define I2S_TX_TDM_MODE_SHIFT		14
#define I2S_TX_TDM_MODE_MASK		(1 << I2S_TX_TDM_MODE_SHIFT)
#define I2S_TX_TDM_MODE (1 << I2S_TX_TDM_MODE_SHIFT)
#define I2S_TX_SLAVE_MODE		I2S_TX_SLAVE_MODE_MASK
#define I2S_TX_MASTER_MODE		0
#define I2S_TX_INTR_MASK		0x0F

#define I2S_RX_ENABLE_MASK		(1 << 31)
#define I2S_RX_ENABLE			I2S_RX_ENABLE_MASK
#define I2S_RX_IN_R			(1 << 19)
#define I2S_RX_DATA_ALIGNMENT		(1 << 2)
#define I2S_RX_CLOCK_ENABLE		(1 << 0)

#define I2S_RX_DESC_OFF_LEVEL_SHIFT	12
#define I2S_RX_DESC_OFF_LEVEL_MASK	(0x0F << I2S_RX_DESC_OFF_LEVEL_SHIFT)
#define I2S_RX_DESC_IFF_LEVEL_SHIFT	8
#define I2S_RX_DESC_IFF_LEVEL_MASK	(0x0F << I2S_RX_DESC_IFF_LEVEL_SHIFT)
#define I2S_RX_DESC_OFF_INTR_EN_MSK	(1 << 1)
#define I2S_RX_DESC_OFF_INTR_EN		I2S_RX_DESC_OFF_INTR_EN_MSK

#define I2S_RX_CFG			(0x0040) /* 20c0 */
#define I2S_RX_IRQ_CTL			(0x0044)
#define I2S_RX_IRQ_EN			(0x0048)
#define I2S_RX_IRQ_IFF_THLD		(0x004C)
#define I2S_RX_IRQ_OFF_THLD		(0x0050)
#define I2S_RX_DESC_IFF_ADDR		(0x0054)
#define I2S_RX_DESC_IFF_LEN		(0x0058)
#define I2S_RX_DESC_OFF_ADDR		(0x005C)
#define I2S_RX_DESC_OFF_LEN		(0x0060)
#define I2S_RX_CFG_2			(0x0064)
#define I2S_RX_SLAVE_MODE_SHIFT		13
#define I2S_RX_SLAVE_MODE_MASK		(1 << I2S_RX_SLAVE_MODE_SHIFT)
#define I2S_RX_TDM_MODE_SHIFT		14
#define I2S_RX_TDM_MODE_MASK		(1 << I2S_RX_TDM_MODE_SHIFT)
#define I2S_RX_TDM_MODE (1 << I2S_RX_TDM_MODE_SHIFT)
#define I2S_RX_SLAVE_MODE		I2S_RX_SLAVE_MODE_MASK
#define I2S_RX_MASTER_MODE		0
#define I2S_RX_VALID_SLOT_SHIFT		1
#define I2S_RX_VALID_SLOT_MASK		(0x0F << I2S_RX_VALID_SLOT_SHIFT)
#define I2S_TDM_VALID_SLOT_MASK I2S_RX_VALID_SLOT_MASK
#define I2S_RX_VALID_SLOT_8		(4 << I2S_RX_VALID_SLOT_SHIFT)
#define I2S_RX_BIT_PER_SLOT_SHIFT		0
#define I2S_RX_BIT_PER_SLOT_MASK		(0x01 << I2S_RX_BIT_PER_SLOT_SHIFT)
#define I2S_BIT_PER_SLOT_MASK	I2S_RX_BIT_PER_SLOT_MASK
#define I2S_RX_32BIT_PER_SLOT		(0 << I2S_RX_BIT_PER_SLOT_SHIFT)
#define I2S_BIT_PER_SLOT_SHIFT		0
#define I2S_TDM_32BIT_PER_SLOT	0
#define I2S_RX_INTR_MASK		0x0F

#define I2S_REG_MAX			0x007C

struct bcm_i2s_priv {
	struct device *dev;
	struct resource *r_irq;
	struct regmap *regmap_i2s;
	struct clk *i2s_tx_clk,*i2s_rx_clk;;
	struct snd_pcm_substream	*play_substream;
	struct snd_pcm_substream	*capture_substream;
	struct i2s_dma_desc *play_dma_desc;
	struct i2s_dma_desc *capture_dma_desc;
};

extern int bcm63xx_soc_platform_probe(struct platform_device *pdev,
				      struct bcm_i2s_priv *i2s_priv);
extern int bcm63xx_soc_platform_remove(struct platform_device *pdev);

#endif
