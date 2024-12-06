/*********************************************************************
 * bcm63xx-i2s.h -- Broadcom I2S Controller driver header file
 *
 * Author: Kevin Li <kevin-ke.li@broadcom.com>
 * 
 * Copyright (c) 2018 Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2018:DUAL/GPL:standard
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
*********************************************************************/
#ifndef __BCM63XX_SQUAMISH_H
#define __BCM63XX_SQUAMISH_H

#define DMA_RX_CH_ID 4
#define DMA_TX_CH_ID 5
#define DMA_GLB_BIT_CH(x) ( 1<<(x) )

#define DMA_RX_IRQ_STAT		DMA_CH4_IRQ
#define DMA_RX_IRQ_MASK		DMA_CH4_IRQ_MASK
#define DMA_RX_BURST		DMA_CH4_IRQ_BURST
#define DMA_RX_CH_CFG		DMA_CH4_CFG
#define DMA_RX_CH_STATE_RAM1	DMA_CH4_STATE_RAM1
#define DMA_RX_CH_STATE_RAM2	DMA_CH4_STATE_RAM2
#define DMA_GLB_RX_IRQMASK	DMA_GLB_CH4_IRQMASK
#define DMA_GLB_RX_IRQEN	DMA_GLB_CH4_IRQEN

#define DMA_TX_IRQ_STAT		DMA_CH5_IRQ
#define DMA_TX_IRQ_MASK		DMA_CH5_IRQ_MASK
#define DMA_TX_BURST		DMA_CH5_IRQ_BURST
#define DMA_TX_CH_CFG		DMA_CH5_CFG
#define DMA_TX_CH_STATE_RAM1	DMA_CH5_STATE_RAM1
#define DMA_TX_CH_STATE_RAM2	DMA_CH5_STATE_RAM2
#define DMA_GLB_TX_IRQMASK	DMA_GLB_CH5_IRQMASK
#define DMA_GLB_TX_IRQEN	DMA_GLB_CH5_IRQEN

/* PCM control registers */
#define PCM_CTRL		0x0000 /* 0xff860c00 */
#define PCM_CHANNEL_CTRL	0x04
#define PCM_INT_PENDING		0x08
#define PCM_INT_MASK		0x0c
#define PCM_CLK_CTRL0		0x10
#define PCM_CLK_CTRL1		0x14
#define PCM_CLK_CTRL2		0x18
#define PCM_FCW_READBACK	0x1c
#define PCM_TS_ALLOC0		0x40
#define PCM_TS_ALLOC1		0x44
#define PCM_TS_ALLOC2		0x48
#define PCM_TS_ALLOC3		0x4c

/* DMA CTRL registers */
#define DMA_CTRL		0xC00
#define DMA_CTRL_RST		0xC34
#define DMA_CTRL_GLB_IRQ	0xC40
#define DMA_CTRL_GLB_IRQMASK	0xC44

/* DMA channel control registers */
#define DMA_CH0_CFG		0x0E00
#define DMA_CH0_IRQ		0x0E04
#define DMA_CH0_IRQ_MASK	0x0E08
#define DMA_CH0_IRQ_BURST	0x0E0c

#define DMA_CH1_CFG		0x0E10
#define DMA_CH1_IRQ		0x0E14
#define DMA_CH1_IRQ_MASK	0x0E18
#define DMA_CH1_IRQ_BURST	0x0E1c

#define DMA_CH2_CFG		0x0E20
#define DMA_CH2_IRQ		0x0E24
#define DMA_CH2_IRQ_MASK	0x0E28
#define DMA_CH2_IRQ_BURST	0x0E2c

#define DMA_CH3_CFG		0x0E30
#define DMA_CH3_IRQ		0x0E34
#define DMA_CH3_IRQ_MASK	0x0E38
#define DMA_CH3_IRQ_BURST	0x0E3c

#define DMA_CH4_CFG		0x0E40
#define DMA_CH4_IRQ		0x0E44
#define DMA_CH4_IRQ_MASK	0x0E48
#define DMA_CH4_IRQ_BURST	0x0E4c

#define DMA_CH5_CFG		0x0E50
#define DMA_CH5_IRQ		0x0E54
#define DMA_CH5_IRQ_MASK	0x0E58
#define DMA_CH5_IRQ_BURST	0x0E5c

/* DMA channel state ram */
#define DMA_CH0_STATE_RAM1	0x1000
#define DMA_CH0_STATE_RAM2	0x1004
#define DMA_CH0_STATE_RAM3	0x1008
#define DMA_CH0_STATE_RAM4	0x100c

#define DMA_CH1_STATE_RAM1	0x1010
#define DMA_CH1_STATE_RAM2	0x1014
#define DMA_CH1_STATE_RAM3	0x1018
#define DMA_CH1_STATE_RAM4	0x101c

#define DMA_CH2_STATE_RAM1	0x1020
#define DMA_CH2_STATE_RAM2	0x1024
#define DMA_CH2_STATE_RAM3	0x1028
#define DMA_CH2_STATE_RAM4	0x102c

#define DMA_CH3_STATE_RAM1	0x1030
#define DMA_CH3_STATE_RAM2	0x1034
#define DMA_CH3_STATE_RAM3	0x1038
#define DMA_CH3_STATE_RAM4	0x103c

#define DMA_CH4_STATE_RAM1	0x1040
#define DMA_CH4_STATE_RAM2	0x1044
#define DMA_CH4_STATE_RAM3	0x1048
#define DMA_CH4_STATE_RAM4	0x104c

#define DMA_CH5_STATE_RAM1	0x1050
#define DMA_CH5_STATE_RAM2	0x1054
#define DMA_CH5_STATE_RAM3	0x1058
#define DMA_CH5_STATE_RAM4	0x105c

/****** PCM CONTROL bit ******/
#define PCM_ENABLE		(1 << 31)
#define PCM_SLAVE_MODE		(1 << 30)
#define PCM_CLK_INV		(1 << 29)
#define PCM_FS_INV		(1 << 28)
#define PCM_FS_LONG		(1 << 26)
#define PCM_FS_TRIG		(1 << 25)
#define PCM_DATA_OFF		(1 << 24)
#define PCM_SAMPLE_SZ_MASK	(3 << 22)
#define PCM_SAMPLE_SZ_8		(0 << 22)
#define PCM_SAMPLE_SZ_16	(1 << 22)
#define PCM_SAMPLE_SZ_32	(2 << 22)
#define PCM_LSB_FIRST		(1 << 18)
#define PCM_LOOPBACK		(1 << 17)
#define PCM_CLK_DIV_MASK	(7<<14)
#define PCM_CLK_DIV_2		(0<<14)
#define PCM_CLK_DIV_4		(1<<14)
#define PCM_CLK_DIV_8		(2<<14)
#define PCM_CLK_DIV_16		(3<<14)
#define PCM_CLK_DIV_32		(4<<14)
#define PCM_CLK_DIV_64		(5<<14)
#define PCM_CLK_DIV_128		(6<<14)
#define PCM_FRAME_SIZE_MASK	0xFF
#define PCM_FRAME_SIZE_2	2

/* PCM interrupt mask reg */
#define PCM_RX_OVERFLOW_IRQ_MASK	(1 << 1)
#define PCM_TX_UNDERFLOW_IRQ_MASK	(1 << 0)

/* PCM MISC and scale reg */
#define PCM_FCW_SCALE_VALUE		0x40000000

/* PCM mux ctrl soft inti load reg */
#define PCM_NCO_SOFT_INIT		( 1 << 7 )
#define PCM_NCO_LOAD_MISC		( 1 << 6 )
#define PCM_NCO_MUX_CTRL_SHIFT		4
#define PCM_NCO_MUX_CTRL_MASK		0x03
#define PCM_NCO_LOAD_DPLL		2
#define PCM_NCO_LOAD_MISC_FCW		3
#define PCM_NCO_SHIFT			0

/* to do PCM time slot reg */
#define PCM_TS_MASK(x) ( 1<<(x) )
#define PCM_TS_EN PCM_TS_MASK

/* DMA control */
#define DMA_EN_MASK		(1<<0)
#define DMA_EN			DMA_EN_MASK

/* DMA channel control registers bit configuration */
#define DMA_CH_EN		(1<<0)
#define DMA_INTMASK_BDONE	(1<<0)
#define DMA_INTMASK_PDONE	(1<<1)
#define DMA_INTMASK_NOTVLD	(1<<2)
#define DMA_CH_MAX_BURST_MASK	(0x3F)

#define DMA_GLB_CH0_IRQMASK	(1<<0)
#define DMA_GLB_CH1_IRQMASK	(1<<1)
#define DMA_GLB_CH2_IRQMASK	(1<<2)
#define DMA_GLB_CH3_IRQMASK	(1<<3)
#define DMA_GLB_CH4_IRQMASK	(1<<4)
#define DMA_GLB_CH5_IRQMASK	(1<<5)
#define DMA_GLB_CH0_IRQEN	DMA_GLB_CH0_IRQMASK
#define DMA_GLB_CH1_IRQEN	DMA_GLB_CH1_IRQMASK
#define DMA_GLB_CH2_IRQEN	DMA_GLB_CH2_IRQMASK
#define DMA_GLB_CH3_IRQEN	DMA_GLB_CH3_IRQMASK
#define DMA_GLB_CH4_IRQEN	DMA_GLB_CH4_IRQMASK
#define DMA_GLB_CH5_IRQEN	DMA_GLB_CH5_IRQMASK

#define DMA_DESC_LEN_SHIFT	16
#define DMA_DESC_LEN_MASK	(0xffff<<DMA_DESC_LEN_SHIFT)
#define DMA_DESC_STATUS_MASK	0x0000ffff
#define DMA_DESC_STATUS_SHIFT   0

#define DMA_STATUS_OWN	(1<<15)
#define DMA_STATUS_EOP	(1<<14)
#define DMA_STATUS_SOP	(1<<13)
#define DMA_STATUS_WRAP	(1<<12)
#define DMA_STATUS_PERIPH_MASK	(0x0fff)

#define PCM_REG_MAX			PCM_CTRL + 0x1c5f

#endif