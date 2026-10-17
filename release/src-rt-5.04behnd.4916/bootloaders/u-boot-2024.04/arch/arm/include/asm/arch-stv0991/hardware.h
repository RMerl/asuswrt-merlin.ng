/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#ifndef _ASM_ARCH_HARDWARE_H
#define _ASM_ARCH_HARDWARE_H

/* STV0991 */
#define SRAM0_BASE_ADDR                          0x00000000UL
#define SRAM1_BASE_ADDR                          0x00068000UL
#define SRAM2_BASE_ADDR                          0x000D0000UL
#define SRAM3_BASE_ADDR                          0x00138000UL
#define CFS_SRAM0_BASE_ADDR                      0x00198000UL
#define CFS_SRAM1_BASE_ADDR                      0x001B8000UL
#define FAST_SRAM_BASE_ADDR                      0x001D8000UL
#define FLASH_BASE_ADDR                          0x40000000UL
#define PL310_BASE_ADDR                          0x70000000UL
#define HSAXIM_BASE_ADDR                         0x70100000UL
#define IMGSS_BASE_ADDR                          0x70200000UL
#define ADC_BASE_ADDR                            0x80000000UL
#define GPIOA_BASE_ADDR                          0x80001000UL
#define GPIOB_BASE_ADDR                          0x80002000UL
#define GPIOC_BASE_ADDR                          0x80003000UL
#define HDM_BASE_ADDR                            0x80004000UL
#define THSENS_BASE_ADDR                         0x80200000UL
#define GPTIMER2_BASE_ADDR                       0x80201000UL
#define GPTIMER1_BASE_ADDR                       0x80202000UL
#define QSPI_BASE_ADDR                           0x80203000UL
#define CGU_BASE_ADDR                            0x80204000UL
#define CREG_BASE_ADDR                           0x80205000UL
#define PEC_BASE_ADDR                            0x80206000UL
#define WDRU_BASE_ADDR                           0x80207000UL
#define BSEC_BASE_ADDR                           0x80208000UL
#define DAP_ROM_BASE_ADDR                        0x80210000UL
#define SOC_CTI_BASE_ADDR                        0x80211000UL
#define TPIU_BASE_ADDR                           0x80212000UL
#define TMC_ETF_BASE_ADDR                        0x80213000UL
#define R4_ETM_BASE_ADDR                         0x80214000UL
#define R4_CTI_BASE_ADDR                         0x80215000UL
#define R4_DBG_BASE_ADDR                         0x80216000UL
#define GMAC_BASE_ADDR                           0x80300000UL
#define RNSS_BASE_ADDR                           0x80302000UL
#define CRYP_BASE_ADDR                           0x80303000UL
#define HASH_BASE_ADDR                           0x80304000UL
#define GPDMA_BASE_ADDR                          0x80305000UL
#define ISA_BASE_ADDR                            0x8032A000UL
#define HCI_BASE_ADDR                            0x80400000UL
#define I2C1_BASE_ADDR                           0x80401000UL
#define I2C2_BASE_ADDR                           0x80402000UL
#define SAI_BASE_ADDR                            0x80403000UL
#define USI_BASE_ADDR                            0x80404000UL
#define SPI1_BASE_ADDR                           0x80405000UL
#define UART_BASE_ADDR                           0x80406000UL
#define SPI2_BASE_ADDR                           0x80500000UL
#define CAN_BASE_ADDR                            0x80501000UL
#define USART1_BASE_ADDR                         0x80502000UL
#define USART2_BASE_ADDR                         0x80503000UL
#define USART3_BASE_ADDR                         0x80504000UL
#define USART4_BASE_ADDR                         0x80505000UL
#define USART5_BASE_ADDR                         0x80506000UL
#define USART6_BASE_ADDR                         0x80507000UL
#define SDI2_BASE_ADDR                           0x80600000UL
#define SDI1_BASE_ADDR                           0x80601000UL
#define VICA_BASE_ADDR                           0x81000000UL
#define VICB_BASE_ADDR                           0x81001000UL
#define STM_CHANNELS_BASE_ADDR                   0x81100000UL
#define STM_BASE_ADDR                            0x81110000UL
#define SROM_BASE_ADDR                           0xFFFF0000UL

#endif /* _ASM_ARCH_HARDWARE_H */
