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
#define I2S_REG_MAX                   I2S_DESC_OFF_LEN

#endif
