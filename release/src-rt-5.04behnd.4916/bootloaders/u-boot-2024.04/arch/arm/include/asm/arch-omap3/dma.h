/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __SDMA_H
#define __SDMA_H

/* Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */

/* Functions */
void omap3_dma_init(void);
int omap3_dma_conf_transfer(uint32_t chan, uint32_t *src, uint32_t *dst,
		uint32_t sze);
int omap3_dma_start_transfer(uint32_t chan);
int omap3_dma_wait_for_transfer(uint32_t chan);
int omap3_dma_conf_chan(uint32_t chan, struct dma4_chan *config);
int omap3_dma_get_conf_chan(uint32_t chan, struct dma4_chan *config);

/* Register settings */
#define CSDP_DATA_TYPE_8BIT             0x0
#define CSDP_DATA_TYPE_16BIT            0x1
#define CSDP_DATA_TYPE_32BIT            0x2
#define CSDP_SRC_BURST_SINGLE           (0x0 << 7)
#define CSDP_SRC_BURST_EN_16BYTES       (0x1 << 7)
#define CSDP_SRC_BURST_EN_32BYTES       (0x2 << 7)
#define CSDP_SRC_BURST_EN_64BYTES       (0x3 << 7)
#define CSDP_DST_BURST_SINGLE           (0x0 << 14)
#define CSDP_DST_BURST_EN_16BYTES       (0x1 << 14)
#define CSDP_DST_BURST_EN_32BYTES       (0x2 << 14)
#define CSDP_DST_BURST_EN_64BYTES       (0x3 << 14)
#define CSDP_DST_ENDIAN_LOCK_ADAPT      (0x0 << 18)
#define CSDP_DST_ENDIAN_LOCK_LOCK       (0x1 << 18)
#define CSDP_DST_ENDIAN_LITTLE          (0x0 << 19)
#define CSDP_DST_ENDIAN_BIG             (0x1 << 19)
#define CSDP_SRC_ENDIAN_LOCK_ADAPT      (0x0 << 20)
#define CSDP_SRC_ENDIAN_LOCK_LOCK       (0x1 << 20)
#define CSDP_SRC_ENDIAN_LITTLE          (0x0 << 21)
#define CSDP_SRC_ENDIAN_BIG             (0x1 << 21)

#define CCR_READ_PRIORITY_LOW           (0x0 << 6)
#define CCR_READ_PRIORITY_HIGH          (0x1 << 6)
#define CCR_ENABLE_DISABLED             (0x0 << 7)
#define CCR_ENABLE_ENABLE               (0x1 << 7)
#define CCR_SRC_AMODE_CONSTANT          (0x0 << 12)
#define CCR_SRC_AMODE_POST_INC          (0x1 << 12)
#define CCR_SRC_AMODE_SINGLE_IDX        (0x2 << 12)
#define CCR_SRC_AMODE_DOUBLE_IDX        (0x3 << 12)
#define CCR_DST_AMODE_CONSTANT          (0x0 << 14)
#define CCR_DST_AMODE_POST_INC          (0x1 << 14)
#define CCR_DST_AMODE_SINGLE_IDX        (0x2 << 14)
#define CCR_DST_AMODE_SOUBLE_IDX        (0x3 << 14)

#define CCR_RD_ACTIVE_MASK              (1 << 9)
#define CCR_WR_ACTIVE_MASK              (1 << 10)

#define CSR_TRANS_ERR			(1 << 8)
#define CSR_SUPERVISOR_ERR		(1 << 10)
#define CSR_MISALIGNED_ADRS_ERR		(1 << 11)

/* others */
#define CHAN_NR_MIN			0
#define CHAN_NR_MAX			31

#endif /* __SDMA_H */
