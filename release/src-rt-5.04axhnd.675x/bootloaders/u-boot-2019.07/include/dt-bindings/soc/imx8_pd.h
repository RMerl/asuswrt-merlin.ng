/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#ifndef __DT_BINDINGS_IMX8_PD_H
#define __DT_BINDINGS_IMX8_PD_H

/*!
 * These defines are used to indicate a resource. Resources include peripherals
 * and bus masters (but not memory regions). Note items from list should
 * never be changed or removed (only added to at the end of the list).
 */
#define PD_DC_0                     dc0_power_domain
#define PD_DC_0_PLL_0               dc0_pll0
#define PD_DC_0_PLL_1               dc0_pll1
#define PD_LVDS0                    lvds0_power_domain
#define PD_LVDS0_I2C0               lvds0_i2c0
#define PD_LVDS0_I2C1               lvds0_i2c1
#define PD_LVDS0_PWM                lvds0_pwm
#define PD_LVDS0_PWM                lvds0_pwm
#define PD_LVDS0_GPIO               lvds0_gpio
#define PD_DC_1                     dc1_power_domain
#define PD_DC_1_PLL_0               dc1_pll0
#define PD_DC_1_PLL_1               dc1_pll1
#define PD_LVDS1                    lvds1_power_domain
#define PD_LVDS1_I2C0               lvds1_i2c0
#define PD_LVDS1_I2C1               lvds1_i2c1
#define PD_LVDS1_PWM                lvds1_pwm
#define PD_LVDS1_GPIO               lvds1_gpio

#define PD_DMA                      dma_power_domain
#define PD_DMA_SPI_0                dma_spi0
#define PD_DMA_SPI_1                dma_spi1
#define PD_DMA_SPI_2                dma_spi2
#define PD_DMA_SPI_3                dma_spi3
#define PD_DMA_UART0                dma_lpuart0
#define PD_DMA_UART1                dma_lpuart1
#define PD_DMA_UART2                dma_lpuart2
#define PD_DMA_UART3                dma_lpuart3
#define PD_DMA_UART4                dma_lpuart4
#define PD_DMA_EMVSIM_0             dma_emvsim0
#define PD_DMA_EMVSIM_1             dma_emvsim1
#define PD_DMA_I2C_0                dma_lpi2c0
#define PD_DMA_I2C_1                dma_lpi2c1
#define PD_DMA_I2C_2                dma_lpi2c2
#define PD_DMA_I2C_3                dma_lpi2c3
#define PD_DMA_I2C_4                dma_lpi2c4
#define PD_DMA_ADC_0                dma_adc0
#define PD_DMA_ADC_1                dma_adc1
#define PD_DMA_FTM_0                dma_ftm0
#define PD_DMA_FTM_1                dma_ftm1
#define PD_DMA_CAN_0                dma_flexcan0
#define PD_DMA_CAN_1                dma_flexcan1
#define PD_DMA_CAN_2                dma_flexcan2
#define PD_DMA_PWM_0                dma_pwm0
#define PD_DMA_LCD_0                dma_lcd0

#define PD_HSIO                     hsio_power_domain
#define PD_HSIO_PCIE_A              hsio_pcie0
#define PD_HSIO_PCIE_B              hsio_pcie1
#define PD_HSIO_SATA_0              hsio_sata0
#define PD_HSIO_GPIO                hsio_gpio

#define PD_LCD_0                    lcd0_power_domain
#define PD_LCD_0_I2C_0              lcd0_i2c0
#define PD_LCD_0_I2C_1              lcd0_i2c1
#define PD_LCD_PWM_0                lcd0_pwm0

#define PD_LSIO                     lsio_power_domain
#define PD_LSIO_GPIO_0              lsio_gpio0
#define PD_LSIO_GPIO_1              lsio_gpio1
#define PD_LSIO_GPIO_2              lsio_gpio2
#define PD_LSIO_GPIO_3              lsio_gpio3
#define PD_LSIO_GPIO_4              lsio_gpio4
#define PD_LSIO_GPIO_5              lsio_gpio5
#define PD_LSIO_GPIO_6              lsio_gpio6
#define PD_LSIO_GPIO_7              lsio_gpio7
#define PD_LSIO_GPT_0               lsio_gpt0
#define PD_LSIO_GPT_1               lsio_gpt1
#define PD_LSIO_GPT_2               lsio_gpt2
#define PD_LSIO_GPT_3               lsio_gpt3
#define PD_LSIO_GPT_4               lsio_gpt4
#define PD_LSIO_KPP                 lsio_kpp
#define PD_LSIO_FSPI_0              lsio_fspi0
#define PD_LSIO_FSPI_1              lsio_fspi1
#define PD_LSIO_PWM_0               lsio_pwm0
#define PD_LSIO_PWM_1               lsio_pwm1
#define PD_LSIO_PWM_2               lsio_pwm2
#define PD_LSIO_PWM_3               lsio_pwm3
#define PD_LSIO_PWM_4               lsio_pwm4
#define PD_LSIO_PWM_5               lsio_pwm5
#define PD_LSIO_PWM_6               lsio_pwm6
#define PD_LSIO_PWM_7               lsio_pwm7

#define PD_CONN                     connectivity_power_domain
#define PD_CONN_SDHC_0              conn_sdhc0
#define PD_CONN_SDHC_1              conn_sdhc1
#define PD_CONN_SDHC_2              conn_sdhc2
#define PD_CONN_ENET_0              conn_enet0
#define PD_CONN_ENET_1              conn_enet1
#define PD_CONN_MLB_0               conn_mlb0
#define PD_CONN_DMA_4_CH0           conn_dma4_ch0
#define PD_CONN_DMA_4_CH1           conn_dma4_ch1
#define PD_CONN_DMA_4_CH2           conn_dma4_ch2
#define PD_CONN_DMA_4_CH3           conn_dma4_ch3
#define PD_CONN_DMA_4_CH4           conn_dma4_ch4
#define PD_CONN_USB_0               conn_usb0
#define PD_CONN_USB_1               conn_usb1
#define PD_CONN_USB_0_PHY           conn_usb0_phy
#define PD_CONN_USB_2               conn_usb2
#define PD_CONN_USB_2_PHY           conn_usb2_phy
#define PD_CONN_NAND                conn_nand

#define PD_AUDIO                    audio_power_domain
#define PD_AUD_SAI_0                audio_sai0
#define PD_AUD_SAI_1                audio_sai1
#define PD_AUD_SAI_2                audio_sai2
#define PD_AUD_ASRC_0               audio_asrc0
#define PD_AUD_ASRC_1               audio_asrc1
#define PD_AUD_ESAI_0               audio_esai0
#define PD_AUD_ESAI_1               audio_esai1
#define PD_AUD_SPDIF_0              audio_spdif0
#define PD_AUD_SPDIF_1              audio_spdif1
#define PD_AUD_SAI_3                audio_sai3
#define PD_AUD_SAI_4                audio_sai4
#define PD_AUD_SAI_5                audio_sai5
#define PD_AUD_SAI_6                audio_sai6
#define PD_AUD_SAI_7                audio_sai7
#define PD_AUD_GPT_5                audio_gpt5
#define PD_AUD_GPT_6                audio_gpt6
#define PD_AUD_GPT_7                audio_gpt7
#define PD_AUD_GPT_8                audio_gpt8
#define PD_AUD_GPT_9                audio_gpt9
#define PD_AUD_GPT_10               audio_gpt10
#define PD_AUD_AMIX                 audio_amix
#define PD_AUD_MQS_0                audio_mqs0
#define PD_AUD_HIFI                 audio_hifi
#define PD_AUD_OCRAM                audio_ocram
#define PD_AUD_MCLK_OUT_0           audio_mclkout0
#define PD_AUD_MCLK_OUT_1           audio_mclkout1
#define PD_AUD_AUDIO_PLL_0          audio_audiopll0
#define PD_AUD_AUDIO_PLL_1          audio_audiopll1
#define PD_AUD_AUDIO_CLK_0          audio_audioclk0
#define PD_AUD_AUDIO_CLK_1          audio_audioclk1

#define PD_IMAGING                  imaging_power_domain
#define PD_IMAGING_JPEG_DEC         imaging_jpeg_dec
#define PD_IMAGING_JPEG_ENC         imaging_jpeg_enc
#define PD_IMAGING_PDMA0            PD_IMAGING
#define PD_IMAGING_PDMA1            imaging_pdma1
#define PD_IMAGING_PDMA2            imaging_pdma2
#define PD_IMAGING_PDMA3            imaging_pdma3
#define PD_IMAGING_PDMA4            imaging_pdma4
#define PD_IMAGING_PDMA5            imaging_pdma5
#define PD_IMAGING_PDMA6            imaging_pdma6
#define PD_IMAGING_PDMA7            imaging_pdma7

#define PD_MIPI_0_DSI               mipi0_dsi_power_domain
#define PD_MIPI_0_DSI_I2C0          mipi0_dsi_i2c0
#define PD_MIPI_0_DSI_I2C1          mipi0_dsi_i2c1
#define PD_MIPI_0_DSI_PWM0          mipi0_dsi_pwm0
#define PD_MIPI_1_DSI               mipi1_dsi_power_domain
#define PD_MIPI_1_DSI_I2C0          mipi1_dsi_i2c0
#define PD_MIPI_1_DSI_I2C1          mipi1_dsi_i2c1
#define PD_MIPI_1_DSI_PWM0          mipi1_dsi_pwm0

#define PD_MIPI_CSI0                mipi_csi0_power_domain
#define PD_MIPI_CSI0_PWM            mipi_csi0_pwm
#define PD_MIPI_CSI0_I2C            mipi_csi0_i2c
#define PD_MIPI_CSI1                mipi_csi1_power_domain
#define PD_MIPI_CSI1_PWM_0          mipi_csi1_pwm
#define PD_MIPI_CSI1_I2C_0          mipi_csi1_i2c

#define PD_HDMI                     hdmi_power_domain
#define PD_HDMI_I2C_0               hdmi_i2c
#define PD_HDMI_PWM_0               hdmi_pwm
#define PD_HDMI_GPIO_0              hdmi_gpio

#define PD_HDMI_RX                  hdmi_rx_power_domain
#define PD_HDMI_RX_I2C              hdmi_rx_i2c
#define PD_HDMI_RX_PWM              hdmi_rx_pwm

#define PD_CM40                     cm40_power_domain
#define PD_CM40_I2C                 cm40_i2c
#define PD_CM40_INTMUX              cm40_intmux

#endif /* __DT_BINDINGS_IMX8_PD_H */
