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
#ifndef __BCM63XX_I2S_H
#define __BCM63XX_I2S_H

#define I2S_ENABLE                     (1 << 31)    
#define I2S_MCLK_RATE_SHIFT            20 
#define I2S_OUT_R                      (1 << 19)    
#define I2S_OUT_L                      (1 << 18)    
#define I2S_CLKSEL_SHIFT               16 
#define I2S_CLK_100MHZ                 0
#define I2S_CLK_50MHZ                  1
#define I2S_CLK_25MHZ                  2
#define I2S_CLK_PLL                    3
#define I2S_MCLK_CLKSEL_CLR_MASK       0x00F30000
#define I2S_SCLK_POLARITY              (1 << 9)    
#define I2S_LRCK_POLARITY              (1 << 8)    
#define I2S_SCLKS_PER_1FS_DIV32_SHIFT  4 
#define I2S_DATA_JUSTIFICATION         (1 << 3)    
#define I2S_DATA_ALIGNMENT             (1 << 2)    
#define I2S_DATA_ENABLE                (1 << 1)    
#define I2S_CLOCK_ENABLE               (1 << 0)
#define I2S_DESC_OFF_LEVEL_SHIFT       12
#define I2S_DESC_IFF_LEVEL_SHIFT       8    
#define I2S_DESC_LEVEL_MASK            0x0F
#define I2S_DESC_OFF_OVERRUN_INTR      (1 << 3)
#define I2S_DESC_IFF_UNDERRUN_INTR     (1 << 2)
#define I2S_DESC_OFF_INTR              (1 << 1)
#define I2S_DESC_IFF_INTR              (1 << 0)
#define I2S_INTR_MASK                  0x0F
   
#define I2S_DESC_INTR_TYPE_SEL        (1 << 4)
#define I2S_DESC_OFF_OVERRUN_INTR_EN  (1 << 3)
#define I2S_DESC_IFF_UNDERRUN_INTR_EN (1 << 2)
#define I2S_DESC_OFF_INTR_EN          (1 << 1)
#define I2S_DESC_IFF_INTR_EN          (1 << 0)
   
#define I2S_DESC_IFF_INTR_THLD_MASK   0x07
   
#define I2S_DESC_EOP                  (1 << 31)                                 
#define I2S_DESC_FIFO_DEPTH           8
#define I2S_DMA_BUFF_MAX_LEN          0xFFFF
#define I2S_DESC_LEN_MASK             I2S_DMA_BUFF_MAX_LEN

#if defined(CONFIG_BCM963148) || defined(CONFIG_BCM963138)
#define ISR_TABLE_OFFSET              32
#define ISR_TABLE2_OFFSET             ISR_TABLE_OFFSET + 32
#define ISR_TABLE3_OFFSET             ISR_TABLE2_OFFSET + 32
#define INTERRUPT_ID_I2S              (ISR_TABLE3_OFFSET + 20)
#endif

#define I2S_INTR_SHIFT_REGMAP         0x00
#define I2S_INTR_MASK_REGMAP          ( 0x0F<<I2S_INTR_SHIFT_REGMAP ) 


#define I2S_64BITS_PERFRAME           ( 2<<I2S_SCLKS_PER_1FS_DIV32_SHIFT )
#define I2S_SCLKS_PER_1FS_DIV32_MASK  ( 0x0f<<I2S_SCLKS_PER_1FS_DIV32_SHIFT )

/* I2S REGS */
#if defined(CONFIG_BCM963148) || defined(CONFIG_BCM963138)   
#define I2S_REG_PLATFORM_OFFSET       0x0c
#else
#define I2S_REG_PLATFORM_OFFSET       0      
#endif

#define I2S_CFG                       (0x0000)
#define I2S_IRQ_CTL                   (0x0004 + I2S_REG_PLATFORM_OFFSET)
#define I2S_IRQ_EN                    (0x0008 + I2S_REG_PLATFORM_OFFSET)
#define I2S_IRQ_IFF_THLD              (0x000c + I2S_REG_PLATFORM_OFFSET)
#define I2S_IRQ_OFF_THLD              (0x0010 + I2S_REG_PLATFORM_OFFSET)
#define I2S_DESC_IFF_ADDR             (0x0014 + I2S_REG_PLATFORM_OFFSET)
#define I2S_DESC_IFF_LEN              (0x0018 + I2S_REG_PLATFORM_OFFSET)
#define I2S_DESC_OFF_ADDR             (0x001c + I2S_REG_PLATFORM_OFFSET)
#define I2S_DESC_OFF_LEN              (0x0020 + I2S_REG_PLATFORM_OFFSET)

#define I2S_TX_CFG                    (0x0000)
#define I2S_TX_IRQ_CTL                (0x0004 + I2S_REG_PLATFORM_OFFSET)
#define I2S_TX_IRQ_EN                 (0x0008 + I2S_REG_PLATFORM_OFFSET)
#define I2S_TX_IRQ_IFF_THLD           (0x000c + I2S_REG_PLATFORM_OFFSET)
#define I2S_TX_IRQ_OFF_THLD           (0x0010 + I2S_REG_PLATFORM_OFFSET)
#define I2S_TX_DESC_IFF_ADDR          (0x0014 + I2S_REG_PLATFORM_OFFSET)
#define I2S_TX_DESC_IFF_LEN           (0x0018 + I2S_REG_PLATFORM_OFFSET)
#define I2S_TX_DESC_OFF_ADDR          (0x001c + I2S_REG_PLATFORM_OFFSET)
#define I2S_TX_DESC_OFF_LEN           (0x0020 + I2S_REG_PLATFORM_OFFSET)
#define I2S_TX_CFG_2                  (0x0024 + I2S_REG_PLATFORM_OFFSET)  /*20a4*/
#define I2S_TX_SLAVE_MODE_SHIFT       13
#define I2S_TX_SLAVE_MODE_MASK        ( 1 << I2S_TX_SLAVE_MODE_SHIFT )
#define I2S_TX_SLAVE_MODE             ( 1 << I2S_TX_SLAVE_MODE_SHIFT )
#define I2S_TX_MASTER_MODE            ~I2S_TX_SLAVE_MODE
#define I2S_TX_BITS_PER_SAMPLE_SHIFT  10  /* the number of bits per sample. for transmitter LSB-justified data, MSB-justified data ignored*/
#define I2S_TX_BITS_PER_SAMPLE_32     0
#define I2S_TX_BITS_PER_SAMPLE_24     24
#define I2S_TX_BITS_PER_SAMPLE_20     20
#define I2S_TX_BITS_PER_SAMPLE_18     18
#define I2S_TX_BITS_PER_SAMPLE_16     16

#define I2S_MISC_CFG                  (0x003c + I2S_REG_PLATFORM_OFFSET)  /*20bc*/
#define I2S_MISC_CFG_MASK             0x07
#define I2S_ECHO_LPBK_RX_TX_ENABLE            1<<0
#define I2S_INT_LPBK_TX_RX_ENABLE             1<<1
#define I2S_INT_LPBK_TX_RX_ENABLE_MASK        1<<1
#define I2S_PAD_LVL_SCLK_LRCK_LOOP_DIS_ENABLE 1<<2
#define I2S_PAD_LVL_SCLK_LRCK_LOOP_DIS_SHIFT  2
#define I2S_PAD_LVL_SCLK_LRCK_LOOP_DIS_MASK   1<<2

#define I2S_RX_CFG                        (0x0040 + I2S_REG_PLATFORM_OFFSET) /* 20c0 */
#define I2S_RX_ENABLE_MASK                0x80000000
#define I2S_RX_ENABLE                     (1 << 31)        /* set to 1 to enable I2S transmitter */
#define I2S_RX_MCLK_RATE_SHIFT            20               /* offset of TX MCLK RATE, it occupied bit20-23 */
#define I2S_RX_MCLK_RATE_MASK             0x0f
#define I2S_RX_IN_R                       (1 << 19)        /* determine whether the righter channel is from lower 32bits or higher 32 bits */
#define I2S_RX_IN_R_MASK                  I2S_RX_IN_R
#define I2S_RX_IN_L                       (1 << 18)
#define I2S_RX_IN_L_MASK                  I2S_RX_IN_L

/*#define I2S_TX_LRCK_ENABLE              (1 << 15 )*/      /* so far it is always enabled for LR clk, default=1 */  
#define I2S_RX_BITS_PER_SAMPLE_SHIFT      10  /* the number of bits per sample. for transmitter LSB-justified data, MSB-justified data ignored*/
#define I2S_RX_BITS_PER_SAMPLE_MASK       ( 0x1f <<  I2S_RX_BITS_PER_SAMPLE_SHIFT )
#define I2S_RX_BITS_PER_SAMPLE_32         0 
#define I2S_RX_BITS_PER_SAMPLE_24         24
#define I2S_RX_BITS_PER_SAMPLE_20         20
#define I2S_RX_BITS_PER_SAMPLE_18         18
#define I2S_RX_BITS_PER_SAMPLE_16         16
#define I2S_RX_SCLK_POLARITY              (1 << 9)  /* set polarity of the clk in transmitter */  
#define I2S_RX_SCLK_POLARITY_MASK         (1 << 9)  
#define I2S_RX_LRCK_POLARITY              (1 << 8)  /* set the polarity of the L/R clkc in transmitter */
#define I2S_RX_LRCK_POLARITY_MASK         (1 << 8)  
#define I2S_RX_SCLKS_PER_1FS_DIV32_SHIFT  4   /* 1 for 32, 2 for 64 bits in a FS */
#define I2S_RX_SCLKS_PER_1FS_DIV32_MASK   (0x0f<<I2S_RX_SCLKS_PER_1FS_DIV32_SHIFT)
#define I2S_RX_SCLKS_PER_1FS_DIV64        2
#define I2S_RX_DATA_JUSTIFICATION         (1 << 3)
#define I2S_RX_DATA_JUSTIFICATION_MASK    (1 << 3)       
#define I2S_RX_DATA_ALIGNMENT             (1 << 2)
#define I2S_RX_DATA_ALIGNMENT_MASK        (1 << 2)      
#define I2S_RX_CLOCK_ENABLE               (1 << 0)
#define I2S_RX_CLOCK_ENABLE_MASK          (1 << 0)

#define I2S_RX_IRQ_CTL                    (0x0044 + I2S_REG_PLATFORM_OFFSET) /* 20c4 */
#define I2S_RX_DESC_OFF_LEVEL_SHIFT       12        /* descriptor output FIFO level shift*/
#define I2S_RX_DESC_IFF_LEVEL_SHIFT       8         /* descriptor input FIFO level shift */
#define I2S_RX_DESC_LEVEL_MASK            0x0F
#define I2S_RX_DESC_IFF_OVERRUN_INTR      (1 << 5)  /* descriptor input FIFO overrun interrupt. the FIFO is full and a write happened. write 1 to clear the interrupt*/
#define I2S_RX_DESC_OFF_UNDERRUN_INTR     (1 << 4)  /* descriptor output FIFO underrun interrupt. the FIFO is empty and a read happened. write 1 to clear the interrupt*/
#define I2S_RX_DESC_OFF_OVERRUN_INTR      (1 << 3)  /* descriptor output FIFO overrun interrupt. the FIFO is full and a write happened. write 1 to clear the interrupt*/
#define I2S_RX_DESC_IFF_UNDERRUN_INTR     (1 << 2)  /* descriptor input FIFO underrun interrupt. the FIFO is empty and a read happened. write 1 to clear the interrupt*/
#define I2S_RX_DESC_OFF_INTR              (1 << 1)  /* descriptor output FIFO interrupt, depending on threshold set by TX_I2S_DESC_INTR_TYPE_SEL*/
#define I2S_RX_DESC_IFF_INTR              (1 << 0)  /* same above */
#define I2S_RX_INTR_MASK                  0x3F

#define I2S_RX_IRQ_EN                     (0x0048 + I2S_REG_PLATFORM_OFFSET) /* 20c8 */
#define I2S_RX_DESC_IFF_OVERRUN_INTR_EN   (1 << 6)
#define I2S_RX_DESC_OFF_UNDERRUN_INTR_EN  (1 << 5)
#define I2S_RX_DESC_INTR_TYPE_SEL         (1 << 4) /* 0, IRQ happened when input FIFO <= threshold, or output FIFO >= threshold. 1.other...*/
#define I2S_RX_DESC_INTR_TYPE_SEL_MASK    (1 << 4)
#define I2S_RX_DESC_OFF_OVERRUN_INTR_EN   (1 << 3)
#define I2S_RX_DESC_IFF_UNDERRUN_INTR_EN  (1 << 2)
#define I2S_RX_DESC_OFF_INTR_EN           (1 << 1)
#define I2S_RX_DESC_OFF_INTR_EN_MASK      (1 << 1)
#define I2S_RX_DESC_IFF_INTR_EN           (1 << 0)

#define I2S_RX_IRQ_IFF_THLD               (0x004c + I2S_REG_PLATFORM_OFFSET) /* 20cc */
#define I2S_RX_DESC_IFF_INTR_THLD_MASK    0x07
#define I2S_RX_IRQ_OFF_THLD               (0x0050 + I2S_REG_PLATFORM_OFFSET) /* 20d0 */
#define I2S_RX_DESC_OFF_INTR_THLD_MASK    0x07

#define I2S_RX_DESC_IFF_ADDR              (0x0054 + I2S_REG_PLATFORM_OFFSET) /* 20d4 */
#define I2S_RX_DESC_IFF_LEN               (0x0058 + I2S_REG_PLATFORM_OFFSET) /* 20d8 */
#define I2S_RX_DESC_OFF_ADDR              (0x005c + I2S_REG_PLATFORM_OFFSET) /* 20dc */
#define I2S_RX_DESC_OFF_LEN               (0x0060 + I2S_REG_PLATFORM_OFFSET) /* 20e0 */
#define I2S_RX_DESC_EOP                   (1 << 31)                                 
#define I2S_RX_DESC_FIFO_DEPTH            8
#define I2S_RX_DMA_BUFF_MAX_LEN           0xFFFF
#define I2S_RX_DESC_LEN_MASK              I2S_TX_DMA_BUFF_MAX_LEN

#define I2S_RX_CFG_2                      (0x0064 + I2S_REG_PLATFORM_OFFSET) /* 20e4 */
#define I2S_RX_SLAVE_MODE                 ( 1 << 13 )
#define I2S_RX_SLAVE_MODE_MASK            ( 1 << 13 )

#define I2S_REG_MAX                       (0x007c+I2S_REG_PLATFORM_OFFSET)  /* 20fc */

#endif
