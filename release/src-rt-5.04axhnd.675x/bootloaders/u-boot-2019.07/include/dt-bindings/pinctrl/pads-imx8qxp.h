/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#ifndef _SC_PADS_H
#define _SC_PADS_H

#define SC_P_PCIE_CTRL0_PERST_B                  0	/* HSIO.PCIE0.PERST_B, LSIO.GPIO4.IO00 */
#define SC_P_PCIE_CTRL0_CLKREQ_B                 1	/* HSIO.PCIE0.CLKREQ_B, LSIO.GPIO4.IO01 */
#define SC_P_PCIE_CTRL0_WAKE_B                   2	/* HSIO.PCIE0.WAKE_B, LSIO.GPIO4.IO02 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_PCIESEP       3	/*  */
#define SC_P_USB_SS3_TC0                         4	/* ADMA.I2C1.SCL, CONN.USB_OTG1.PWR, CONN.USB_OTG2.PWR, LSIO.GPIO4.IO03 */
#define SC_P_USB_SS3_TC1                         5	/* ADMA.I2C1.SCL, CONN.USB_OTG2.PWR, LSIO.GPIO4.IO04 */
#define SC_P_USB_SS3_TC2                         6	/* ADMA.I2C1.SDA, CONN.USB_OTG1.OC, CONN.USB_OTG2.OC, LSIO.GPIO4.IO05 */
#define SC_P_USB_SS3_TC3                         7	/* ADMA.I2C1.SDA, CONN.USB_OTG2.OC, LSIO.GPIO4.IO06 */
#define SC_P_COMP_CTL_GPIO_3V3_USB3IO            8	/*  */
#define SC_P_EMMC0_CLK                           9	/* CONN.EMMC0.CLK, CONN.NAND.READY_B, LSIO.GPIO4.IO07 */
#define SC_P_EMMC0_CMD                           10	/* CONN.EMMC0.CMD, CONN.NAND.DQS, LSIO.GPIO4.IO08 */
#define SC_P_EMMC0_DATA0                         11	/* CONN.EMMC0.DATA0, CONN.NAND.DATA00, LSIO.GPIO4.IO09 */
#define SC_P_EMMC0_DATA1                         12	/* CONN.EMMC0.DATA1, CONN.NAND.DATA01, LSIO.GPIO4.IO10 */
#define SC_P_EMMC0_DATA2                         13	/* CONN.EMMC0.DATA2, CONN.NAND.DATA02, LSIO.GPIO4.IO11 */
#define SC_P_EMMC0_DATA3                         14	/* CONN.EMMC0.DATA3, CONN.NAND.DATA03, LSIO.GPIO4.IO12 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_SD1FIX0       15	/*  */
#define SC_P_EMMC0_DATA4                         16	/* CONN.EMMC0.DATA4, CONN.NAND.DATA04, CONN.EMMC0.WP, LSIO.GPIO4.IO13 */
#define SC_P_EMMC0_DATA5                         17	/* CONN.EMMC0.DATA5, CONN.NAND.DATA05, CONN.EMMC0.VSELECT, LSIO.GPIO4.IO14 */
#define SC_P_EMMC0_DATA6                         18	/* CONN.EMMC0.DATA6, CONN.NAND.DATA06, CONN.MLB.CLK, LSIO.GPIO4.IO15 */
#define SC_P_EMMC0_DATA7                         19	/* CONN.EMMC0.DATA7, CONN.NAND.DATA07, CONN.MLB.SIG, LSIO.GPIO4.IO16 */
#define SC_P_EMMC0_STROBE                        20	/* CONN.EMMC0.STROBE, CONN.NAND.CLE, CONN.MLB.DATA, LSIO.GPIO4.IO17 */
#define SC_P_EMMC0_RESET_B                       21	/* CONN.EMMC0.RESET_B, CONN.NAND.WP_B, LSIO.GPIO4.IO18 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_SD1FIX1       22	/*  */
#define SC_P_USDHC1_RESET_B                      23	/* CONN.USDHC1.RESET_B, CONN.NAND.RE_N, ADMA.SPI2.SCK, LSIO.GPIO4.IO19 */
#define SC_P_USDHC1_VSELECT                      24	/* CONN.USDHC1.VSELECT, CONN.NAND.RE_P, ADMA.SPI2.SDO, CONN.NAND.RE_B, LSIO.GPIO4.IO20 */
#define SC_P_CTL_NAND_RE_P_N                     25	/*  */
#define SC_P_USDHC1_WP                           26	/* CONN.USDHC1.WP, CONN.NAND.DQS_N, ADMA.SPI2.SDI, LSIO.GPIO4.IO21 */
#define SC_P_USDHC1_CD_B                         27	/* CONN.USDHC1.CD_B, CONN.NAND.DQS_P, ADMA.SPI2.CS0, CONN.NAND.DQS, LSIO.GPIO4.IO22 */
#define SC_P_CTL_NAND_DQS_P_N                    28	/*  */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_VSELSEP       29	/*  */
#define SC_P_USDHC1_CLK                          30	/* CONN.USDHC1.CLK, ADMA.UART3.RX, LSIO.GPIO4.IO23 */
#define SC_P_USDHC1_CMD                          31	/* CONN.USDHC1.CMD, CONN.NAND.CE0_B, ADMA.MQS.R, LSIO.GPIO4.IO24 */
#define SC_P_USDHC1_DATA0                        32	/* CONN.USDHC1.DATA0, CONN.NAND.CE1_B, ADMA.MQS.L, LSIO.GPIO4.IO25 */
#define SC_P_USDHC1_DATA1                        33	/* CONN.USDHC1.DATA1, CONN.NAND.RE_B, ADMA.UART3.TX, LSIO.GPIO4.IO26 */
#define SC_P_USDHC1_DATA2                        34	/* CONN.USDHC1.DATA2, CONN.NAND.WE_B, ADMA.UART3.CTS_B, LSIO.GPIO4.IO27 */
#define SC_P_USDHC1_DATA3                        35	/* CONN.USDHC1.DATA3, CONN.NAND.ALE, ADMA.UART3.RTS_B, LSIO.GPIO4.IO28 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_VSEL3         36	/*  */
#define SC_P_ENET0_RGMII_TXC                     37	/* CONN.ENET0.RGMII_TXC, CONN.ENET0.RCLK50M_OUT, CONN.ENET0.RCLK50M_IN, CONN.NAND.CE1_B, LSIO.GPIO4.IO29 */
#define SC_P_ENET0_RGMII_TX_CTL                  38	/* CONN.ENET0.RGMII_TX_CTL, CONN.USDHC1.RESET_B, LSIO.GPIO4.IO30 */
#define SC_P_ENET0_RGMII_TXD0                    39	/* CONN.ENET0.RGMII_TXD0, CONN.USDHC1.VSELECT, LSIO.GPIO4.IO31 */
#define SC_P_ENET0_RGMII_TXD1                    40	/* CONN.ENET0.RGMII_TXD1, CONN.USDHC1.WP, LSIO.GPIO5.IO00 */
#define SC_P_ENET0_RGMII_TXD2                    41	/* CONN.ENET0.RGMII_TXD2, CONN.MLB.CLK, CONN.NAND.CE0_B, CONN.USDHC1.CD_B, LSIO.GPIO5.IO01 */
#define SC_P_ENET0_RGMII_TXD3                    42	/* CONN.ENET0.RGMII_TXD3, CONN.MLB.SIG, CONN.NAND.RE_B, LSIO.GPIO5.IO02 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_ENET_ENETB0   43	/*  */
#define SC_P_ENET0_RGMII_RXC                     44	/* CONN.ENET0.RGMII_RXC, CONN.MLB.DATA, CONN.NAND.WE_B, CONN.USDHC1.CLK, LSIO.GPIO5.IO03 */
#define SC_P_ENET0_RGMII_RX_CTL                  45	/* CONN.ENET0.RGMII_RX_CTL, CONN.USDHC1.CMD, LSIO.GPIO5.IO04 */
#define SC_P_ENET0_RGMII_RXD0                    46	/* CONN.ENET0.RGMII_RXD0, CONN.USDHC1.DATA0, LSIO.GPIO5.IO05 */
#define SC_P_ENET0_RGMII_RXD1                    47	/* CONN.ENET0.RGMII_RXD1, CONN.USDHC1.DATA1, LSIO.GPIO5.IO06 */
#define SC_P_ENET0_RGMII_RXD2                    48	/* CONN.ENET0.RGMII_RXD2, CONN.ENET0.RMII_RX_ER, CONN.USDHC1.DATA2, LSIO.GPIO5.IO07 */
#define SC_P_ENET0_RGMII_RXD3                    49	/* CONN.ENET0.RGMII_RXD3, CONN.NAND.ALE, CONN.USDHC1.DATA3, LSIO.GPIO5.IO08 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_ENET_ENETB1   50	/*  */
#define SC_P_ENET0_REFCLK_125M_25M               51	/* CONN.ENET0.REFCLK_125M_25M, CONN.ENET0.PPS, CONN.ENET1.PPS, LSIO.GPIO5.IO09 */
#define SC_P_ENET0_MDIO                          52	/* CONN.ENET0.MDIO, ADMA.I2C3.SDA, CONN.ENET1.MDIO, LSIO.GPIO5.IO10 */
#define SC_P_ENET0_MDC                           53	/* CONN.ENET0.MDC, ADMA.I2C3.SCL, CONN.ENET1.MDC, LSIO.GPIO5.IO11 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_GPIOCT        54	/*  */
#define SC_P_ESAI0_FSR                           55	/* ADMA.ESAI0.FSR, CONN.ENET1.RCLK50M_OUT, ADMA.LCDIF.D00, CONN.ENET1.RGMII_TXC, CONN.ENET1.RCLK50M_IN */
#define SC_P_ESAI0_FST                           56	/* ADMA.ESAI0.FST, CONN.MLB.CLK, ADMA.LCDIF.D01, CONN.ENET1.RGMII_TXD2, LSIO.GPIO0.IO01 */
#define SC_P_ESAI0_SCKR                          57	/* ADMA.ESAI0.SCKR, ADMA.LCDIF.D02, CONN.ENET1.RGMII_TX_CTL, LSIO.GPIO0.IO02 */
#define SC_P_ESAI0_SCKT                          58	/* ADMA.ESAI0.SCKT, CONN.MLB.SIG, ADMA.LCDIF.D03, CONN.ENET1.RGMII_TXD3, LSIO.GPIO0.IO03 */
#define SC_P_ESAI0_TX0                           59	/* ADMA.ESAI0.TX0, CONN.MLB.DATA, ADMA.LCDIF.D04, CONN.ENET1.RGMII_RXC, LSIO.GPIO0.IO04 */
#define SC_P_ESAI0_TX1                           60	/* ADMA.ESAI0.TX1, ADMA.LCDIF.D05, CONN.ENET1.RGMII_RXD3, LSIO.GPIO0.IO05 */
#define SC_P_ESAI0_TX2_RX3                       61	/* ADMA.ESAI0.TX2_RX3, CONN.ENET1.RMII_RX_ER, ADMA.LCDIF.D06, CONN.ENET1.RGMII_RXD2, LSIO.GPIO0.IO06 */
#define SC_P_ESAI0_TX3_RX2                       62	/* ADMA.ESAI0.TX3_RX2, ADMA.LCDIF.D07, CONN.ENET1.RGMII_RXD1, LSIO.GPIO0.IO07 */
#define SC_P_ESAI0_TX4_RX1                       63	/* ADMA.ESAI0.TX4_RX1, ADMA.LCDIF.D08, CONN.ENET1.RGMII_TXD0, LSIO.GPIO0.IO08 */
#define SC_P_ESAI0_TX5_RX0                       64	/* ADMA.ESAI0.TX5_RX0, ADMA.LCDIF.D09, CONN.ENET1.RGMII_TXD1, LSIO.GPIO0.IO09 */
#define SC_P_SPDIF0_RX                           65	/* ADMA.SPDIF0.RX, ADMA.MQS.R, ADMA.LCDIF.D10, CONN.ENET1.RGMII_RXD0, LSIO.GPIO0.IO10 */
#define SC_P_SPDIF0_TX                           66	/* ADMA.SPDIF0.TX, ADMA.MQS.L, ADMA.LCDIF.D11, CONN.ENET1.RGMII_RX_CTL, LSIO.GPIO0.IO11 */
#define SC_P_SPDIF0_EXT_CLK                      67	/* ADMA.SPDIF0.EXT_CLK, ADMA.LCDIF.D12, CONN.ENET1.REFCLK_125M_25M, LSIO.GPIO0.IO12 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_GPIORHB       68	/*  */
#define SC_P_SPI3_SCK                            69	/* ADMA.SPI3.SCK, ADMA.LCDIF.D13, LSIO.GPIO0.IO13 */
#define SC_P_SPI3_SDO                            70	/* ADMA.SPI3.SDO, ADMA.LCDIF.D14, LSIO.GPIO0.IO14 */
#define SC_P_SPI3_SDI                            71	/* ADMA.SPI3.SDI, ADMA.LCDIF.D15, LSIO.GPIO0.IO15 */
#define SC_P_SPI3_CS0                            72	/* ADMA.SPI3.CS0, ADMA.ACM.MCLK_OUT1, ADMA.LCDIF.HSYNC, LSIO.GPIO0.IO16 */
#define SC_P_SPI3_CS1                            73	/* ADMA.SPI3.CS1, ADMA.I2C3.SCL, ADMA.LCDIF.RESET, ADMA.SPI2.CS0, ADMA.LCDIF.D16 */
#define SC_P_MCLK_IN1                            74	/* ADMA.ACM.MCLK_IN1, ADMA.I2C3.SDA, ADMA.LCDIF.EN, ADMA.SPI2.SCK, ADMA.LCDIF.D17 */
#define SC_P_MCLK_IN0                            75	/* ADMA.ACM.MCLK_IN0, ADMA.ESAI0.RX_HF_CLK, ADMA.LCDIF.VSYNC, ADMA.SPI2.SDI, LSIO.GPIO0.IO19 */
#define SC_P_MCLK_OUT0                           76	/* ADMA.ACM.MCLK_OUT0, ADMA.ESAI0.TX_HF_CLK, ADMA.LCDIF.CLK, ADMA.SPI2.SDO, LSIO.GPIO0.IO20 */
#define SC_P_UART1_TX                            77	/* ADMA.UART1.TX, LSIO.PWM0.OUT, LSIO.GPT0.CAPTURE, LSIO.GPIO0.IO21 */
#define SC_P_UART1_RX                            78	/* ADMA.UART1.RX, LSIO.PWM1.OUT, LSIO.GPT0.COMPARE, LSIO.GPT1.CLK, LSIO.GPIO0.IO22 */
#define SC_P_UART1_RTS_B                         79	/* ADMA.UART1.RTS_B, LSIO.PWM2.OUT, ADMA.LCDIF.D16, LSIO.GPT1.CAPTURE, LSIO.GPT0.CLK */
#define SC_P_UART1_CTS_B                         80	/* ADMA.UART1.CTS_B, LSIO.PWM3.OUT, ADMA.LCDIF.D17, LSIO.GPT1.COMPARE, LSIO.GPIO0.IO24 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_GPIORHK       81	/*  */
#define SC_P_SAI0_TXD                            82	/* ADMA.SAI0.TXD, ADMA.SAI1.RXC, ADMA.SPI1.SDO, ADMA.LCDIF.D18, LSIO.GPIO0.IO25 */
#define SC_P_SAI0_TXC                            83	/* ADMA.SAI0.TXC, ADMA.SAI1.TXD, ADMA.SPI1.SDI, ADMA.LCDIF.D19, LSIO.GPIO0.IO26 */
#define SC_P_SAI0_RXD                            84	/* ADMA.SAI0.RXD, ADMA.SAI1.RXFS, ADMA.SPI1.CS0, ADMA.LCDIF.D20, LSIO.GPIO0.IO27 */
#define SC_P_SAI0_TXFS                           85	/* ADMA.SAI0.TXFS, ADMA.SPI2.CS1, ADMA.SPI1.SCK, LSIO.GPIO0.IO28 */
#define SC_P_SAI1_RXD                            86	/* ADMA.SAI1.RXD, ADMA.SAI0.RXFS, ADMA.SPI1.CS1, ADMA.LCDIF.D21, LSIO.GPIO0.IO29 */
#define SC_P_SAI1_RXC                            87	/* ADMA.SAI1.RXC, ADMA.SAI1.TXC, ADMA.LCDIF.D22, LSIO.GPIO0.IO30 */
#define SC_P_SAI1_RXFS                           88	/* ADMA.SAI1.RXFS, ADMA.SAI1.TXFS, ADMA.LCDIF.D23, LSIO.GPIO0.IO31 */
#define SC_P_SPI2_CS0                            89	/* ADMA.SPI2.CS0, LSIO.GPIO1.IO00 */
#define SC_P_SPI2_SDO                            90	/* ADMA.SPI2.SDO, LSIO.GPIO1.IO01 */
#define SC_P_SPI2_SDI                            91	/* ADMA.SPI2.SDI, LSIO.GPIO1.IO02 */
#define SC_P_SPI2_SCK                            92	/* ADMA.SPI2.SCK, LSIO.GPIO1.IO03 */
#define SC_P_SPI0_SCK                            93	/* ADMA.SPI0.SCK, ADMA.SAI0.TXC, M40.I2C0.SCL, M40.GPIO0.IO00, LSIO.GPIO1.IO04 */
#define SC_P_SPI0_SDI                            94	/* ADMA.SPI0.SDI, ADMA.SAI0.TXD, M40.TPM0.CH0, M40.GPIO0.IO02, LSIO.GPIO1.IO05 */
#define SC_P_SPI0_SDO                            95	/* ADMA.SPI0.SDO, ADMA.SAI0.TXFS, M40.I2C0.SDA, M40.GPIO0.IO01, LSIO.GPIO1.IO06 */
#define SC_P_SPI0_CS1                            96	/* ADMA.SPI0.CS1, ADMA.SAI0.RXC, ADMA.SAI1.TXD, ADMA.LCD_PWM0.OUT, LSIO.GPIO1.IO07 */
#define SC_P_SPI0_CS0                            97	/* ADMA.SPI0.CS0, ADMA.SAI0.RXD, M40.TPM0.CH1, M40.GPIO0.IO03, LSIO.GPIO1.IO08 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_GPIORHT       98	/*  */
#define SC_P_ADC_IN1                             99	/* ADMA.ADC.IN1, M40.I2C0.SDA, M40.GPIO0.IO01, LSIO.GPIO1.IO09 */
#define SC_P_ADC_IN0                             100	/* ADMA.ADC.IN0, M40.I2C0.SCL, M40.GPIO0.IO00, LSIO.GPIO1.IO10 */
#define SC_P_ADC_IN3                             101	/* ADMA.ADC.IN3, M40.UART0.TX, M40.GPIO0.IO03, ADMA.ACM.MCLK_OUT0, LSIO.GPIO1.IO11 */
#define SC_P_ADC_IN2                             102	/* ADMA.ADC.IN2, M40.UART0.RX, M40.GPIO0.IO02, ADMA.ACM.MCLK_IN0, LSIO.GPIO1.IO12 */
#define SC_P_ADC_IN5                             103	/* ADMA.ADC.IN5, M40.TPM0.CH1, M40.GPIO0.IO05, LSIO.GPIO1.IO13 */
#define SC_P_ADC_IN4                             104	/* ADMA.ADC.IN4, M40.TPM0.CH0, M40.GPIO0.IO04, LSIO.GPIO1.IO14 */
#define SC_P_FLEXCAN0_RX                         105	/* ADMA.FLEXCAN0.RX, ADMA.SAI2.RXC, ADMA.UART0.RTS_B, ADMA.SAI1.TXC, LSIO.GPIO1.IO15 */
#define SC_P_FLEXCAN0_TX                         106	/* ADMA.FLEXCAN0.TX, ADMA.SAI2.RXD, ADMA.UART0.CTS_B, ADMA.SAI1.TXFS, LSIO.GPIO1.IO16 */
#define SC_P_FLEXCAN1_RX                         107	/* ADMA.FLEXCAN1.RX, ADMA.SAI2.RXFS, ADMA.FTM.CH2, ADMA.SAI1.TXD, LSIO.GPIO1.IO17 */
#define SC_P_FLEXCAN1_TX                         108	/* ADMA.FLEXCAN1.TX, ADMA.SAI3.RXC, ADMA.DMA0.REQ_IN0, ADMA.SAI1.RXD, LSIO.GPIO1.IO18 */
#define SC_P_FLEXCAN2_RX                         109	/* ADMA.FLEXCAN2.RX, ADMA.SAI3.RXD, ADMA.UART3.RX, ADMA.SAI1.RXFS, LSIO.GPIO1.IO19 */
#define SC_P_FLEXCAN2_TX                         110	/* ADMA.FLEXCAN2.TX, ADMA.SAI3.RXFS, ADMA.UART3.TX, ADMA.SAI1.RXC, LSIO.GPIO1.IO20 */
#define SC_P_UART0_RX                            111	/* ADMA.UART0.RX, ADMA.MQS.R, ADMA.FLEXCAN0.RX, SCU.UART0.RX, LSIO.GPIO1.IO21 */
#define SC_P_UART0_TX                            112	/* ADMA.UART0.TX, ADMA.MQS.L, ADMA.FLEXCAN0.TX, SCU.UART0.TX, LSIO.GPIO1.IO22 */
#define SC_P_UART2_TX                            113	/* ADMA.UART2.TX, ADMA.FTM.CH1, ADMA.FLEXCAN1.TX, LSIO.GPIO1.IO23 */
#define SC_P_UART2_RX                            114	/* ADMA.UART2.RX, ADMA.FTM.CH0, ADMA.FLEXCAN1.RX, LSIO.GPIO1.IO24 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_GPIOLH        115	/*  */
#define SC_P_MIPI_DSI0_I2C0_SCL                  116	/* MIPI_DSI0.I2C0.SCL, MIPI_DSI1.GPIO0.IO02, LSIO.GPIO1.IO25 */
#define SC_P_MIPI_DSI0_I2C0_SDA                  117	/* MIPI_DSI0.I2C0.SDA, MIPI_DSI1.GPIO0.IO03, LSIO.GPIO1.IO26 */
#define SC_P_MIPI_DSI0_GPIO0_00                  118	/* MIPI_DSI0.GPIO0.IO00, ADMA.I2C1.SCL, MIPI_DSI0.PWM0.OUT, LSIO.GPIO1.IO27 */
#define SC_P_MIPI_DSI0_GPIO0_01                  119	/* MIPI_DSI0.GPIO0.IO01, ADMA.I2C1.SDA, LSIO.GPIO1.IO28 */
#define SC_P_MIPI_DSI1_I2C0_SCL                  120	/* MIPI_DSI1.I2C0.SCL, MIPI_DSI0.GPIO0.IO02, LSIO.GPIO1.IO29 */
#define SC_P_MIPI_DSI1_I2C0_SDA                  121	/* MIPI_DSI1.I2C0.SDA, MIPI_DSI0.GPIO0.IO03, LSIO.GPIO1.IO30 */
#define SC_P_MIPI_DSI1_GPIO0_00                  122	/* MIPI_DSI1.GPIO0.IO00, ADMA.I2C2.SCL, MIPI_DSI1.PWM0.OUT, LSIO.GPIO1.IO31 */
#define SC_P_MIPI_DSI1_GPIO0_01                  123	/* MIPI_DSI1.GPIO0.IO01, ADMA.I2C2.SDA, LSIO.GPIO2.IO00 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_MIPIDSIGPIO   124	/*  */
#define SC_P_JTAG_TRST_B                         125	/* SCU.JTAG.TRST_B, SCU.WDOG0.WDOG_OUT */
#define SC_P_PMIC_I2C_SCL                        126	/* SCU.PMIC_I2C.SCL, SCU.GPIO0.IOXX_PMIC_A35_ON, LSIO.GPIO2.IO01 */
#define SC_P_PMIC_I2C_SDA                        127	/* SCU.PMIC_I2C.SDA, SCU.GPIO0.IOXX_PMIC_GPU_ON, LSIO.GPIO2.IO02 */
#define SC_P_PMIC_INT_B                          128	/* SCU.DSC.PMIC_INT_B */
#define SC_P_SCU_GPIO0_00                        129	/* SCU.GPIO0.IO00, SCU.UART0.RX, M40.UART0.RX, ADMA.UART3.RX, LSIO.GPIO2.IO03 */
#define SC_P_SCU_GPIO0_01                        130	/* SCU.GPIO0.IO01, SCU.UART0.TX, M40.UART0.TX, ADMA.UART3.TX, SCU.WDOG0.WDOG_OUT */
#define SC_P_SCU_PMIC_STANDBY                    131	/* SCU.DSC.PMIC_STANDBY */
#define SC_P_SCU_BOOT_MODE0                      132	/* SCU.DSC.BOOT_MODE0 */
#define SC_P_SCU_BOOT_MODE1                      133	/* SCU.DSC.BOOT_MODE1 */
#define SC_P_SCU_BOOT_MODE2                      134	/* SCU.DSC.BOOT_MODE2, SCU.PMIC_I2C.SDA */
#define SC_P_SCU_BOOT_MODE3                      135	/* SCU.DSC.BOOT_MODE3, SCU.PMIC_I2C.SCL, SCU.DSC.RTC_CLOCK_OUTPUT_32K */
#define SC_P_CSI_D00                             136	/* CI_PI.D02, ADMA.SAI0.RXC */
#define SC_P_CSI_D01                             137	/* CI_PI.D03, ADMA.SAI0.RXD */
#define SC_P_CSI_D02                             138	/* CI_PI.D04, ADMA.SAI0.RXFS */
#define SC_P_CSI_D03                             139	/* CI_PI.D05, ADMA.SAI2.RXC */
#define SC_P_CSI_D04                             140	/* CI_PI.D06, ADMA.SAI2.RXD */
#define SC_P_CSI_D05                             141	/* CI_PI.D07, ADMA.SAI2.RXFS */
#define SC_P_CSI_D06                             142	/* CI_PI.D08, ADMA.SAI3.RXC */
#define SC_P_CSI_D07                             143	/* CI_PI.D09, ADMA.SAI3.RXD */
#define SC_P_CSI_HSYNC                           144	/* CI_PI.HSYNC, CI_PI.D00, ADMA.SAI3.RXFS */
#define SC_P_CSI_VSYNC                           145	/* CI_PI.VSYNC, CI_PI.D01 */
#define SC_P_CSI_PCLK                            146	/* CI_PI.PCLK, MIPI_CSI0.I2C0.SCL, ADMA.SPI1.SCK, LSIO.GPIO3.IO00 */
#define SC_P_CSI_MCLK                            147	/* CI_PI.MCLK, MIPI_CSI0.I2C0.SDA, ADMA.SPI1.SDO, LSIO.GPIO3.IO01 */
#define SC_P_CSI_EN                              148	/* CI_PI.EN, CI_PI.I2C.SCL, ADMA.I2C3.SCL, ADMA.SPI1.SDI, LSIO.GPIO3.IO02 */
#define SC_P_CSI_RESET                           149	/* CI_PI.RESET, CI_PI.I2C.SDA, ADMA.I2C3.SDA, ADMA.SPI1.CS0, LSIO.GPIO3.IO03 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_GPIORHD       150	/*  */
#define SC_P_MIPI_CSI0_MCLK_OUT                  151	/* MIPI_CSI0.ACM.MCLK_OUT, LSIO.GPIO3.IO04 */
#define SC_P_MIPI_CSI0_I2C0_SCL                  152	/* MIPI_CSI0.I2C0.SCL, MIPI_CSI0.GPIO0.IO02, LSIO.GPIO3.IO05 */
#define SC_P_MIPI_CSI0_I2C0_SDA                  153	/* MIPI_CSI0.I2C0.SDA, MIPI_CSI0.GPIO0.IO03, LSIO.GPIO3.IO06 */
#define SC_P_MIPI_CSI0_GPIO0_01                  154	/* MIPI_CSI0.GPIO0.IO01, ADMA.I2C0.SDA, LSIO.GPIO3.IO07 */
#define SC_P_MIPI_CSI0_GPIO0_00                  155	/* MIPI_CSI0.GPIO0.IO00, ADMA.I2C0.SCL, LSIO.GPIO3.IO08 */
#define SC_P_QSPI0A_DATA0                        156	/* LSIO.QSPI0A.DATA0, LSIO.GPIO3.IO09 */
#define SC_P_QSPI0A_DATA1                        157	/* LSIO.QSPI0A.DATA1, LSIO.GPIO3.IO10 */
#define SC_P_QSPI0A_DATA2                        158	/* LSIO.QSPI0A.DATA2, LSIO.GPIO3.IO11 */
#define SC_P_QSPI0A_DATA3                        159	/* LSIO.QSPI0A.DATA3, LSIO.GPIO3.IO12 */
#define SC_P_QSPI0A_DQS                          160	/* LSIO.QSPI0A.DQS, LSIO.GPIO3.IO13 */
#define SC_P_QSPI0A_SS0_B                        161	/* LSIO.QSPI0A.SS0_B, LSIO.GPIO3.IO14 */
#define SC_P_QSPI0A_SS1_B                        162	/* LSIO.QSPI0A.SS1_B, LSIO.GPIO3.IO15 */
#define SC_P_QSPI0A_SCLK                         163	/* LSIO.QSPI0A.SCLK, LSIO.GPIO3.IO16 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_QSPI0A        164	/*  */
#define SC_P_QSPI0B_SCLK                         165	/* LSIO.QSPI0B.SCLK, LSIO.QSPI1A.SCLK, LSIO.KPP0.COL0, LSIO.GPIO3.IO17 */
#define SC_P_QSPI0B_DATA0                        166	/* LSIO.QSPI0B.DATA0, LSIO.QSPI1A.DATA0, LSIO.KPP0.COL1, LSIO.GPIO3.IO18 */
#define SC_P_QSPI0B_DATA1                        167	/* LSIO.QSPI0B.DATA1, LSIO.QSPI1A.DATA1, LSIO.KPP0.COL2, LSIO.GPIO3.IO19 */
#define SC_P_QSPI0B_DATA2                        168	/* LSIO.QSPI0B.DATA2, LSIO.QSPI1A.DATA2, LSIO.KPP0.COL3, LSIO.GPIO3.IO20 */
#define SC_P_QSPI0B_DATA3                        169	/* LSIO.QSPI0B.DATA3, LSIO.QSPI1A.DATA3, LSIO.KPP0.ROW0, LSIO.GPIO3.IO21 */
#define SC_P_QSPI0B_DQS                          170	/* LSIO.QSPI0B.DQS, LSIO.QSPI1A.DQS, LSIO.KPP0.ROW1, LSIO.GPIO3.IO22 */
#define SC_P_QSPI0B_SS0_B                        171	/* LSIO.QSPI0B.SS0_B, LSIO.QSPI1A.SS0_B, LSIO.KPP0.ROW2, LSIO.GPIO3.IO23 */
#define SC_P_QSPI0B_SS1_B                        172	/* LSIO.QSPI0B.SS1_B, LSIO.QSPI1A.SS1_B, LSIO.KPP0.ROW3, LSIO.GPIO3.IO24 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_QSPI0B        173	/*  */
/*@}*/

/*!
 * @name Pad Mux Definitions
 * format: name padid padmux
 */
/*@{*/
#define SC_P_PCIE_CTRL0_PERST_B_HSIO_PCIE0_PERST_B              SC_P_PCIE_CTRL0_PERST_B            0
#define SC_P_PCIE_CTRL0_PERST_B_LSIO_GPIO4_IO00                 SC_P_PCIE_CTRL0_PERST_B            4
#define SC_P_PCIE_CTRL0_CLKREQ_B_HSIO_PCIE0_CLKREQ_B            SC_P_PCIE_CTRL0_CLKREQ_B           0
#define SC_P_PCIE_CTRL0_CLKREQ_B_LSIO_GPIO4_IO01                SC_P_PCIE_CTRL0_CLKREQ_B           4
#define SC_P_PCIE_CTRL0_WAKE_B_HSIO_PCIE0_WAKE_B                SC_P_PCIE_CTRL0_WAKE_B             0
#define SC_P_PCIE_CTRL0_WAKE_B_LSIO_GPIO4_IO02                  SC_P_PCIE_CTRL0_WAKE_B             4
#define SC_P_USB_SS3_TC0_ADMA_I2C1_SCL                          SC_P_USB_SS3_TC0                   0
#define SC_P_USB_SS3_TC0_CONN_USB_OTG1_PWR                      SC_P_USB_SS3_TC0                   1
#define SC_P_USB_SS3_TC0_CONN_USB_OTG2_PWR                      SC_P_USB_SS3_TC0                   2
#define SC_P_USB_SS3_TC0_LSIO_GPIO4_IO03                        SC_P_USB_SS3_TC0                   4
#define SC_P_USB_SS3_TC1_ADMA_I2C1_SCL                          SC_P_USB_SS3_TC1                   0
#define SC_P_USB_SS3_TC1_CONN_USB_OTG2_PWR                      SC_P_USB_SS3_TC1                   1
#define SC_P_USB_SS3_TC1_LSIO_GPIO4_IO04                        SC_P_USB_SS3_TC1                   4
#define SC_P_USB_SS3_TC2_ADMA_I2C1_SDA                          SC_P_USB_SS3_TC2                   0
#define SC_P_USB_SS3_TC2_CONN_USB_OTG1_OC                       SC_P_USB_SS3_TC2                   1
#define SC_P_USB_SS3_TC2_CONN_USB_OTG2_OC                       SC_P_USB_SS3_TC2                   2
#define SC_P_USB_SS3_TC2_LSIO_GPIO4_IO05                        SC_P_USB_SS3_TC2                   4
#define SC_P_USB_SS3_TC3_ADMA_I2C1_SDA                          SC_P_USB_SS3_TC3                   0
#define SC_P_USB_SS3_TC3_CONN_USB_OTG2_OC                       SC_P_USB_SS3_TC3                   1
#define SC_P_USB_SS3_TC3_LSIO_GPIO4_IO06                        SC_P_USB_SS3_TC3                   4
#define SC_P_EMMC0_CLK_CONN_EMMC0_CLK                           SC_P_EMMC0_CLK                     0
#define SC_P_EMMC0_CLK_CONN_NAND_READY_B                        SC_P_EMMC0_CLK                     1
#define SC_P_EMMC0_CLK_LSIO_GPIO4_IO07                          SC_P_EMMC0_CLK                     4
#define SC_P_EMMC0_CMD_CONN_EMMC0_CMD                           SC_P_EMMC0_CMD                     0
#define SC_P_EMMC0_CMD_CONN_NAND_DQS                            SC_P_EMMC0_CMD                     1
#define SC_P_EMMC0_CMD_LSIO_GPIO4_IO08                          SC_P_EMMC0_CMD                     4
#define SC_P_EMMC0_DATA0_CONN_EMMC0_DATA0                       SC_P_EMMC0_DATA0                   0
#define SC_P_EMMC0_DATA0_CONN_NAND_DATA00                       SC_P_EMMC0_DATA0                   1
#define SC_P_EMMC0_DATA0_LSIO_GPIO4_IO09                        SC_P_EMMC0_DATA0                   4
#define SC_P_EMMC0_DATA1_CONN_EMMC0_DATA1                       SC_P_EMMC0_DATA1                   0
#define SC_P_EMMC0_DATA1_CONN_NAND_DATA01                       SC_P_EMMC0_DATA1                   1
#define SC_P_EMMC0_DATA1_LSIO_GPIO4_IO10                        SC_P_EMMC0_DATA1                   4
#define SC_P_EMMC0_DATA2_CONN_EMMC0_DATA2                       SC_P_EMMC0_DATA2                   0
#define SC_P_EMMC0_DATA2_CONN_NAND_DATA02                       SC_P_EMMC0_DATA2                   1
#define SC_P_EMMC0_DATA2_LSIO_GPIO4_IO11                        SC_P_EMMC0_DATA2                   4
#define SC_P_EMMC0_DATA3_CONN_EMMC0_DATA3                       SC_P_EMMC0_DATA3                   0
#define SC_P_EMMC0_DATA3_CONN_NAND_DATA03                       SC_P_EMMC0_DATA3                   1
#define SC_P_EMMC0_DATA3_LSIO_GPIO4_IO12                        SC_P_EMMC0_DATA3                   4
#define SC_P_EMMC0_DATA4_CONN_EMMC0_DATA4                       SC_P_EMMC0_DATA4                   0
#define SC_P_EMMC0_DATA4_CONN_NAND_DATA04                       SC_P_EMMC0_DATA4                   1
#define SC_P_EMMC0_DATA4_CONN_EMMC0_WP                          SC_P_EMMC0_DATA4                   3
#define SC_P_EMMC0_DATA4_LSIO_GPIO4_IO13                        SC_P_EMMC0_DATA4                   4
#define SC_P_EMMC0_DATA5_CONN_EMMC0_DATA5                       SC_P_EMMC0_DATA5                   0
#define SC_P_EMMC0_DATA5_CONN_NAND_DATA05                       SC_P_EMMC0_DATA5                   1
#define SC_P_EMMC0_DATA5_CONN_EMMC0_VSELECT                     SC_P_EMMC0_DATA5                   3
#define SC_P_EMMC0_DATA5_LSIO_GPIO4_IO14                        SC_P_EMMC0_DATA5                   4
#define SC_P_EMMC0_DATA6_CONN_EMMC0_DATA6                       SC_P_EMMC0_DATA6                   0
#define SC_P_EMMC0_DATA6_CONN_NAND_DATA06                       SC_P_EMMC0_DATA6                   1
#define SC_P_EMMC0_DATA6_CONN_MLB_CLK                           SC_P_EMMC0_DATA6                   3
#define SC_P_EMMC0_DATA6_LSIO_GPIO4_IO15                        SC_P_EMMC0_DATA6                   4
#define SC_P_EMMC0_DATA7_CONN_EMMC0_DATA7                       SC_P_EMMC0_DATA7                   0
#define SC_P_EMMC0_DATA7_CONN_NAND_DATA07                       SC_P_EMMC0_DATA7                   1
#define SC_P_EMMC0_DATA7_CONN_MLB_SIG                           SC_P_EMMC0_DATA7                   3
#define SC_P_EMMC0_DATA7_LSIO_GPIO4_IO16                        SC_P_EMMC0_DATA7                   4
#define SC_P_EMMC0_STROBE_CONN_EMMC0_STROBE                     SC_P_EMMC0_STROBE                  0
#define SC_P_EMMC0_STROBE_CONN_NAND_CLE                         SC_P_EMMC0_STROBE                  1
#define SC_P_EMMC0_STROBE_CONN_MLB_DATA                         SC_P_EMMC0_STROBE                  3
#define SC_P_EMMC0_STROBE_LSIO_GPIO4_IO17                       SC_P_EMMC0_STROBE                  4
#define SC_P_EMMC0_RESET_B_CONN_EMMC0_RESET_B                   SC_P_EMMC0_RESET_B                 0
#define SC_P_EMMC0_RESET_B_CONN_NAND_WP_B                       SC_P_EMMC0_RESET_B                 1
#define SC_P_EMMC0_RESET_B_LSIO_GPIO4_IO18                      SC_P_EMMC0_RESET_B                 4
#define SC_P_USDHC1_RESET_B_CONN_USDHC1_RESET_B                 SC_P_USDHC1_RESET_B                0
#define SC_P_USDHC1_RESET_B_CONN_NAND_RE_N                      SC_P_USDHC1_RESET_B                1
#define SC_P_USDHC1_RESET_B_ADMA_SPI2_SCK                       SC_P_USDHC1_RESET_B                2
#define SC_P_USDHC1_RESET_B_LSIO_GPIO4_IO19                     SC_P_USDHC1_RESET_B                4
#define SC_P_USDHC1_VSELECT_CONN_USDHC1_VSELECT                 SC_P_USDHC1_VSELECT                0
#define SC_P_USDHC1_VSELECT_CONN_NAND_RE_P                      SC_P_USDHC1_VSELECT                1
#define SC_P_USDHC1_VSELECT_ADMA_SPI2_SDO                       SC_P_USDHC1_VSELECT                2
#define SC_P_USDHC1_VSELECT_CONN_NAND_RE_B                      SC_P_USDHC1_VSELECT                3
#define SC_P_USDHC1_VSELECT_LSIO_GPIO4_IO20                     SC_P_USDHC1_VSELECT                4
#define SC_P_USDHC1_WP_CONN_USDHC1_WP                           SC_P_USDHC1_WP                     0
#define SC_P_USDHC1_WP_CONN_NAND_DQS_N                          SC_P_USDHC1_WP                     1
#define SC_P_USDHC1_WP_ADMA_SPI2_SDI                            SC_P_USDHC1_WP                     2
#define SC_P_USDHC1_WP_LSIO_GPIO4_IO21                          SC_P_USDHC1_WP                     4
#define SC_P_USDHC1_CD_B_CONN_USDHC1_CD_B                       SC_P_USDHC1_CD_B                   0
#define SC_P_USDHC1_CD_B_CONN_NAND_DQS_P                        SC_P_USDHC1_CD_B                   1
#define SC_P_USDHC1_CD_B_ADMA_SPI2_CS0                          SC_P_USDHC1_CD_B                   2
#define SC_P_USDHC1_CD_B_CONN_NAND_DQS                          SC_P_USDHC1_CD_B                   3
#define SC_P_USDHC1_CD_B_LSIO_GPIO4_IO22                        SC_P_USDHC1_CD_B                   4
#define SC_P_USDHC1_CLK_CONN_USDHC1_CLK                         SC_P_USDHC1_CLK                    0
#define SC_P_USDHC1_CLK_ADMA_UART3_RX                           SC_P_USDHC1_CLK                    2
#define SC_P_USDHC1_CLK_LSIO_GPIO4_IO23                         SC_P_USDHC1_CLK                    4
#define SC_P_USDHC1_CMD_CONN_USDHC1_CMD                         SC_P_USDHC1_CMD                    0
#define SC_P_USDHC1_CMD_CONN_NAND_CE0_B                         SC_P_USDHC1_CMD                    1
#define SC_P_USDHC1_CMD_ADMA_MQS_R                              SC_P_USDHC1_CMD                    2
#define SC_P_USDHC1_CMD_LSIO_GPIO4_IO24                         SC_P_USDHC1_CMD                    4
#define SC_P_USDHC1_DATA0_CONN_USDHC1_DATA0                     SC_P_USDHC1_DATA0                  0
#define SC_P_USDHC1_DATA0_CONN_NAND_CE1_B                       SC_P_USDHC1_DATA0                  1
#define SC_P_USDHC1_DATA0_ADMA_MQS_L                            SC_P_USDHC1_DATA0                  2
#define SC_P_USDHC1_DATA0_LSIO_GPIO4_IO25                       SC_P_USDHC1_DATA0                  4
#define SC_P_USDHC1_DATA1_CONN_USDHC1_DATA1                     SC_P_USDHC1_DATA1                  0
#define SC_P_USDHC1_DATA1_CONN_NAND_RE_B                        SC_P_USDHC1_DATA1                  1
#define SC_P_USDHC1_DATA1_ADMA_UART3_TX                         SC_P_USDHC1_DATA1                  2
#define SC_P_USDHC1_DATA1_LSIO_GPIO4_IO26                       SC_P_USDHC1_DATA1                  4
#define SC_P_USDHC1_DATA2_CONN_USDHC1_DATA2                     SC_P_USDHC1_DATA2                  0
#define SC_P_USDHC1_DATA2_CONN_NAND_WE_B                        SC_P_USDHC1_DATA2                  1
#define SC_P_USDHC1_DATA2_ADMA_UART3_CTS_B                      SC_P_USDHC1_DATA2                  2
#define SC_P_USDHC1_DATA2_LSIO_GPIO4_IO27                       SC_P_USDHC1_DATA2                  4
#define SC_P_USDHC1_DATA3_CONN_USDHC1_DATA3                     SC_P_USDHC1_DATA3                  0
#define SC_P_USDHC1_DATA3_CONN_NAND_ALE                         SC_P_USDHC1_DATA3                  1
#define SC_P_USDHC1_DATA3_ADMA_UART3_RTS_B                      SC_P_USDHC1_DATA3                  2
#define SC_P_USDHC1_DATA3_LSIO_GPIO4_IO28                       SC_P_USDHC1_DATA3                  4
#define SC_P_ENET0_RGMII_TXC_CONN_ENET0_RGMII_TXC               SC_P_ENET0_RGMII_TXC               0
#define SC_P_ENET0_RGMII_TXC_CONN_ENET0_RCLK50M_OUT             SC_P_ENET0_RGMII_TXC               1
#define SC_P_ENET0_RGMII_TXC_CONN_ENET0_RCLK50M_IN              SC_P_ENET0_RGMII_TXC               2
#define SC_P_ENET0_RGMII_TXC_CONN_NAND_CE1_B                    SC_P_ENET0_RGMII_TXC               3
#define SC_P_ENET0_RGMII_TXC_LSIO_GPIO4_IO29                    SC_P_ENET0_RGMII_TXC               4
#define SC_P_ENET0_RGMII_TX_CTL_CONN_ENET0_RGMII_TX_CTL         SC_P_ENET0_RGMII_TX_CTL            0
#define SC_P_ENET0_RGMII_TX_CTL_CONN_USDHC1_RESET_B             SC_P_ENET0_RGMII_TX_CTL            3
#define SC_P_ENET0_RGMII_TX_CTL_LSIO_GPIO4_IO30                 SC_P_ENET0_RGMII_TX_CTL            4
#define SC_P_ENET0_RGMII_TXD0_CONN_ENET0_RGMII_TXD0             SC_P_ENET0_RGMII_TXD0              0
#define SC_P_ENET0_RGMII_TXD0_CONN_USDHC1_VSELECT               SC_P_ENET0_RGMII_TXD0              3
#define SC_P_ENET0_RGMII_TXD0_LSIO_GPIO4_IO31                   SC_P_ENET0_RGMII_TXD0              4
#define SC_P_ENET0_RGMII_TXD1_CONN_ENET0_RGMII_TXD1             SC_P_ENET0_RGMII_TXD1              0
#define SC_P_ENET0_RGMII_TXD1_CONN_USDHC1_WP                    SC_P_ENET0_RGMII_TXD1              3
#define SC_P_ENET0_RGMII_TXD1_LSIO_GPIO5_IO00                   SC_P_ENET0_RGMII_TXD1              4
#define SC_P_ENET0_RGMII_TXD2_CONN_ENET0_RGMII_TXD2             SC_P_ENET0_RGMII_TXD2              0
#define SC_P_ENET0_RGMII_TXD2_CONN_MLB_CLK                      SC_P_ENET0_RGMII_TXD2              1
#define SC_P_ENET0_RGMII_TXD2_CONN_NAND_CE0_B                   SC_P_ENET0_RGMII_TXD2              2
#define SC_P_ENET0_RGMII_TXD2_CONN_USDHC1_CD_B                  SC_P_ENET0_RGMII_TXD2              3
#define SC_P_ENET0_RGMII_TXD2_LSIO_GPIO5_IO01                   SC_P_ENET0_RGMII_TXD2              4
#define SC_P_ENET0_RGMII_TXD3_CONN_ENET0_RGMII_TXD3             SC_P_ENET0_RGMII_TXD3              0
#define SC_P_ENET0_RGMII_TXD3_CONN_MLB_SIG                      SC_P_ENET0_RGMII_TXD3              1
#define SC_P_ENET0_RGMII_TXD3_CONN_NAND_RE_B                    SC_P_ENET0_RGMII_TXD3              2
#define SC_P_ENET0_RGMII_TXD3_LSIO_GPIO5_IO02                   SC_P_ENET0_RGMII_TXD3              4
#define SC_P_COMP_CTL_GPIO_1V8_3V3_ENET_ENETB0_PAD              SC_P_COMP_CTL_GPIO_1V8_3V3_ENET_ENETB0	0
#define SC_P_ENET0_RGMII_RXC_CONN_ENET0_RGMII_RXC               SC_P_ENET0_RGMII_RXC               0
#define SC_P_ENET0_RGMII_RXC_CONN_MLB_DATA                      SC_P_ENET0_RGMII_RXC               1
#define SC_P_ENET0_RGMII_RXC_CONN_NAND_WE_B                     SC_P_ENET0_RGMII_RXC               2
#define SC_P_ENET0_RGMII_RXC_CONN_USDHC1_CLK                    SC_P_ENET0_RGMII_RXC               3
#define SC_P_ENET0_RGMII_RXC_LSIO_GPIO5_IO03                    SC_P_ENET0_RGMII_RXC               4
#define SC_P_ENET0_RGMII_RX_CTL_CONN_ENET0_RGMII_RX_CTL         SC_P_ENET0_RGMII_RX_CTL            0
#define SC_P_ENET0_RGMII_RX_CTL_CONN_USDHC1_CMD                 SC_P_ENET0_RGMII_RX_CTL            3
#define SC_P_ENET0_RGMII_RX_CTL_LSIO_GPIO5_IO04                 SC_P_ENET0_RGMII_RX_CTL            4
#define SC_P_ENET0_RGMII_RXD0_CONN_ENET0_RGMII_RXD0             SC_P_ENET0_RGMII_RXD0              0
#define SC_P_ENET0_RGMII_RXD0_CONN_USDHC1_DATA0                 SC_P_ENET0_RGMII_RXD0              3
#define SC_P_ENET0_RGMII_RXD0_LSIO_GPIO5_IO05                   SC_P_ENET0_RGMII_RXD0              4
#define SC_P_ENET0_RGMII_RXD1_CONN_ENET0_RGMII_RXD1             SC_P_ENET0_RGMII_RXD1              0
#define SC_P_ENET0_RGMII_RXD1_CONN_USDHC1_DATA1                 SC_P_ENET0_RGMII_RXD1              3
#define SC_P_ENET0_RGMII_RXD1_LSIO_GPIO5_IO06                   SC_P_ENET0_RGMII_RXD1              4
#define SC_P_ENET0_RGMII_RXD2_CONN_ENET0_RGMII_RXD2             SC_P_ENET0_RGMII_RXD2              0
#define SC_P_ENET0_RGMII_RXD2_CONN_ENET0_RMII_RX_ER             SC_P_ENET0_RGMII_RXD2              1
#define SC_P_ENET0_RGMII_RXD2_CONN_USDHC1_DATA2                 SC_P_ENET0_RGMII_RXD2              3
#define SC_P_ENET0_RGMII_RXD2_LSIO_GPIO5_IO07                   SC_P_ENET0_RGMII_RXD2              4
#define SC_P_ENET0_RGMII_RXD3_CONN_ENET0_RGMII_RXD3             SC_P_ENET0_RGMII_RXD3              0
#define SC_P_ENET0_RGMII_RXD3_CONN_NAND_ALE                     SC_P_ENET0_RGMII_RXD3              2
#define SC_P_ENET0_RGMII_RXD3_CONN_USDHC1_DATA3                 SC_P_ENET0_RGMII_RXD3              3
#define SC_P_ENET0_RGMII_RXD3_LSIO_GPIO5_IO08                   SC_P_ENET0_RGMII_RXD3              4
#define SC_P_COMP_CTL_GPIO_1V8_3V3_ENET_ENETB1_PAD              SC_P_COMP_CTL_GPIO_1V8_3V3_ENET_ENETB1	0
#define SC_P_ENET0_REFCLK_125M_25M_CONN_ENET0_REFCLK_125M_25M   SC_P_ENET0_REFCLK_125M_25M         0
#define SC_P_ENET0_REFCLK_125M_25M_CONN_ENET0_PPS               SC_P_ENET0_REFCLK_125M_25M         1
#define SC_P_ENET0_REFCLK_125M_25M_CONN_ENET1_PPS               SC_P_ENET0_REFCLK_125M_25M         2
#define SC_P_ENET0_REFCLK_125M_25M_LSIO_GPIO5_IO09              SC_P_ENET0_REFCLK_125M_25M         4
#define SC_P_ENET0_MDIO_CONN_ENET0_MDIO                         SC_P_ENET0_MDIO                    0
#define SC_P_ENET0_MDIO_ADMA_I2C3_SDA                           SC_P_ENET0_MDIO                    1
#define SC_P_ENET0_MDIO_CONN_ENET1_MDIO                         SC_P_ENET0_MDIO                    2
#define SC_P_ENET0_MDIO_LSIO_GPIO5_IO10                         SC_P_ENET0_MDIO                    4
#define SC_P_ENET0_MDC_CONN_ENET0_MDC                           SC_P_ENET0_MDC                     0
#define SC_P_ENET0_MDC_ADMA_I2C3_SCL                            SC_P_ENET0_MDC                     1
#define SC_P_ENET0_MDC_CONN_ENET1_MDC                           SC_P_ENET0_MDC                     2
#define SC_P_ENET0_MDC_LSIO_GPIO5_IO11                          SC_P_ENET0_MDC                     4
#define SC_P_ESAI0_FSR_ADMA_ESAI0_FSR                           SC_P_ESAI0_FSR                     0
#define SC_P_ESAI0_FSR_CONN_ENET1_RCLK50M_OUT                   SC_P_ESAI0_FSR                     1
#define SC_P_ESAI0_FSR_ADMA_LCDIF_D00                           SC_P_ESAI0_FSR                     2
#define SC_P_ESAI0_FSR_CONN_ENET1_RGMII_TXC                     SC_P_ESAI0_FSR                     3
#define SC_P_ESAI0_FSR_CONN_ENET1_RCLK50M_IN                    SC_P_ESAI0_FSR                     4
#define SC_P_ESAI0_FST_ADMA_ESAI0_FST                           SC_P_ESAI0_FST                     0
#define SC_P_ESAI0_FST_CONN_MLB_CLK                             SC_P_ESAI0_FST                     1
#define SC_P_ESAI0_FST_ADMA_LCDIF_D01                           SC_P_ESAI0_FST                     2
#define SC_P_ESAI0_FST_CONN_ENET1_RGMII_TXD2                    SC_P_ESAI0_FST                     3
#define SC_P_ESAI0_FST_LSIO_GPIO0_IO01                          SC_P_ESAI0_FST                     4
#define SC_P_ESAI0_SCKR_ADMA_ESAI0_SCKR                         SC_P_ESAI0_SCKR                    0
#define SC_P_ESAI0_SCKR_ADMA_LCDIF_D02                          SC_P_ESAI0_SCKR                    2
#define SC_P_ESAI0_SCKR_CONN_ENET1_RGMII_TX_CTL                 SC_P_ESAI0_SCKR                    3
#define SC_P_ESAI0_SCKR_LSIO_GPIO0_IO02                         SC_P_ESAI0_SCKR                    4
#define SC_P_ESAI0_SCKT_ADMA_ESAI0_SCKT                         SC_P_ESAI0_SCKT                    0
#define SC_P_ESAI0_SCKT_CONN_MLB_SIG                            SC_P_ESAI0_SCKT                    1
#define SC_P_ESAI0_SCKT_ADMA_LCDIF_D03                          SC_P_ESAI0_SCKT                    2
#define SC_P_ESAI0_SCKT_CONN_ENET1_RGMII_TXD3                   SC_P_ESAI0_SCKT                    3
#define SC_P_ESAI0_SCKT_LSIO_GPIO0_IO03                         SC_P_ESAI0_SCKT                    4
#define SC_P_ESAI0_TX0_ADMA_ESAI0_TX0                           SC_P_ESAI0_TX0                     0
#define SC_P_ESAI0_TX0_CONN_MLB_DATA                            SC_P_ESAI0_TX0                     1
#define SC_P_ESAI0_TX0_ADMA_LCDIF_D04                           SC_P_ESAI0_TX0                     2
#define SC_P_ESAI0_TX0_CONN_ENET1_RGMII_RXC                     SC_P_ESAI0_TX0                     3
#define SC_P_ESAI0_TX0_LSIO_GPIO0_IO04                          SC_P_ESAI0_TX0                     4
#define SC_P_ESAI0_TX1_ADMA_ESAI0_TX1                           SC_P_ESAI0_TX1                     0
#define SC_P_ESAI0_TX1_ADMA_LCDIF_D05                           SC_P_ESAI0_TX1                     2
#define SC_P_ESAI0_TX1_CONN_ENET1_RGMII_RXD3                    SC_P_ESAI0_TX1                     3
#define SC_P_ESAI0_TX1_LSIO_GPIO0_IO05                          SC_P_ESAI0_TX1                     4
#define SC_P_ESAI0_TX2_RX3_ADMA_ESAI0_TX2_RX3                   SC_P_ESAI0_TX2_RX3                 0
#define SC_P_ESAI0_TX2_RX3_CONN_ENET1_RMII_RX_ER                SC_P_ESAI0_TX2_RX3                 1
#define SC_P_ESAI0_TX2_RX3_ADMA_LCDIF_D06                       SC_P_ESAI0_TX2_RX3                 2
#define SC_P_ESAI0_TX2_RX3_CONN_ENET1_RGMII_RXD2                SC_P_ESAI0_TX2_RX3                 3
#define SC_P_ESAI0_TX2_RX3_LSIO_GPIO0_IO06                      SC_P_ESAI0_TX2_RX3                 4
#define SC_P_ESAI0_TX3_RX2_ADMA_ESAI0_TX3_RX2                   SC_P_ESAI0_TX3_RX2                 0
#define SC_P_ESAI0_TX3_RX2_ADMA_LCDIF_D07                       SC_P_ESAI0_TX3_RX2                 2
#define SC_P_ESAI0_TX3_RX2_CONN_ENET1_RGMII_RXD1                SC_P_ESAI0_TX3_RX2                 3
#define SC_P_ESAI0_TX3_RX2_LSIO_GPIO0_IO07                      SC_P_ESAI0_TX3_RX2                 4
#define SC_P_ESAI0_TX4_RX1_ADMA_ESAI0_TX4_RX1                   SC_P_ESAI0_TX4_RX1                 0
#define SC_P_ESAI0_TX4_RX1_ADMA_LCDIF_D08                       SC_P_ESAI0_TX4_RX1                 2
#define SC_P_ESAI0_TX4_RX1_CONN_ENET1_RGMII_TXD0                SC_P_ESAI0_TX4_RX1                 3
#define SC_P_ESAI0_TX4_RX1_LSIO_GPIO0_IO08                      SC_P_ESAI0_TX4_RX1                 4
#define SC_P_ESAI0_TX5_RX0_ADMA_ESAI0_TX5_RX0                   SC_P_ESAI0_TX5_RX0                 0
#define SC_P_ESAI0_TX5_RX0_ADMA_LCDIF_D09                       SC_P_ESAI0_TX5_RX0                 2
#define SC_P_ESAI0_TX5_RX0_CONN_ENET1_RGMII_TXD1                SC_P_ESAI0_TX5_RX0                 3
#define SC_P_ESAI0_TX5_RX0_LSIO_GPIO0_IO09                      SC_P_ESAI0_TX5_RX0                 4
#define SC_P_SPDIF0_RX_ADMA_SPDIF0_RX                           SC_P_SPDIF0_RX                     0
#define SC_P_SPDIF0_RX_ADMA_MQS_R                               SC_P_SPDIF0_RX                     1
#define SC_P_SPDIF0_RX_ADMA_LCDIF_D10                           SC_P_SPDIF0_RX                     2
#define SC_P_SPDIF0_RX_CONN_ENET1_RGMII_RXD0                    SC_P_SPDIF0_RX                     3
#define SC_P_SPDIF0_RX_LSIO_GPIO0_IO10                          SC_P_SPDIF0_RX                     4
#define SC_P_SPDIF0_TX_ADMA_SPDIF0_TX                           SC_P_SPDIF0_TX                     0
#define SC_P_SPDIF0_TX_ADMA_MQS_L                               SC_P_SPDIF0_TX                     1
#define SC_P_SPDIF0_TX_ADMA_LCDIF_D11                           SC_P_SPDIF0_TX                     2
#define SC_P_SPDIF0_TX_CONN_ENET1_RGMII_RX_CTL                  SC_P_SPDIF0_TX                     3
#define SC_P_SPDIF0_TX_LSIO_GPIO0_IO11                          SC_P_SPDIF0_TX                     4
#define SC_P_SPDIF0_EXT_CLK_ADMA_SPDIF0_EXT_CLK                 SC_P_SPDIF0_EXT_CLK                0
#define SC_P_SPDIF0_EXT_CLK_ADMA_LCDIF_D12                      SC_P_SPDIF0_EXT_CLK                2
#define SC_P_SPDIF0_EXT_CLK_CONN_ENET1_REFCLK_125M_25M          SC_P_SPDIF0_EXT_CLK                3
#define SC_P_SPDIF0_EXT_CLK_LSIO_GPIO0_IO12                     SC_P_SPDIF0_EXT_CLK                4
#define SC_P_COMP_CTL_GPIO_1V8_3V3_GPIORHB_PAD			SC_P_COMP_CTL_GPIO_1V8_3V3_GPIORHB 0
#define SC_P_SPI3_SCK_ADMA_SPI3_SCK                             SC_P_SPI3_SCK                      0
#define SC_P_SPI3_SCK_ADMA_LCDIF_D13                            SC_P_SPI3_SCK                      2
#define SC_P_SPI3_SCK_LSIO_GPIO0_IO13                           SC_P_SPI3_SCK                      4
#define SC_P_SPI3_SDO_ADMA_SPI3_SDO                             SC_P_SPI3_SDO                      0
#define SC_P_SPI3_SDO_ADMA_LCDIF_D14                            SC_P_SPI3_SDO                      2
#define SC_P_SPI3_SDO_LSIO_GPIO0_IO14                           SC_P_SPI3_SDO                      4
#define SC_P_SPI3_SDI_ADMA_SPI3_SDI                             SC_P_SPI3_SDI                      0
#define SC_P_SPI3_SDI_ADMA_LCDIF_D15                            SC_P_SPI3_SDI                      2
#define SC_P_SPI3_SDI_LSIO_GPIO0_IO15                           SC_P_SPI3_SDI                      4
#define SC_P_SPI3_CS0_ADMA_SPI3_CS0                             SC_P_SPI3_CS0                      0
#define SC_P_SPI3_CS0_ADMA_ACM_MCLK_OUT1                        SC_P_SPI3_CS0                      1
#define SC_P_SPI3_CS0_ADMA_LCDIF_HSYNC                          SC_P_SPI3_CS0                      2
#define SC_P_SPI3_CS0_LSIO_GPIO0_IO16                           SC_P_SPI3_CS0                      4
#define SC_P_SPI3_CS1_ADMA_SPI3_CS1                             SC_P_SPI3_CS1                      0
#define SC_P_SPI3_CS1_ADMA_I2C3_SCL                             SC_P_SPI3_CS1                      1
#define SC_P_SPI3_CS1_ADMA_LCDIF_RESET                          SC_P_SPI3_CS1                      2
#define SC_P_SPI3_CS1_ADMA_SPI2_CS0                             SC_P_SPI3_CS1                      3
#define SC_P_SPI3_CS1_ADMA_LCDIF_D16                            SC_P_SPI3_CS1                      4
#define SC_P_MCLK_IN1_ADMA_ACM_MCLK_IN1                         SC_P_MCLK_IN1                      0
#define SC_P_MCLK_IN1_ADMA_I2C3_SDA                             SC_P_MCLK_IN1                      1
#define SC_P_MCLK_IN1_ADMA_LCDIF_EN                             SC_P_MCLK_IN1                      2
#define SC_P_MCLK_IN1_ADMA_SPI2_SCK                             SC_P_MCLK_IN1                      3
#define SC_P_MCLK_IN1_ADMA_LCDIF_D17                            SC_P_MCLK_IN1                      4
#define SC_P_MCLK_IN0_ADMA_ACM_MCLK_IN0                         SC_P_MCLK_IN0                      0
#define SC_P_MCLK_IN0_ADMA_ESAI0_RX_HF_CLK                      SC_P_MCLK_IN0                      1
#define SC_P_MCLK_IN0_ADMA_LCDIF_VSYNC                          SC_P_MCLK_IN0                      2
#define SC_P_MCLK_IN0_ADMA_SPI2_SDI                             SC_P_MCLK_IN0                      3
#define SC_P_MCLK_IN0_LSIO_GPIO0_IO19                           SC_P_MCLK_IN0                      4
#define SC_P_MCLK_OUT0_ADMA_ACM_MCLK_OUT0                       SC_P_MCLK_OUT0                     0
#define SC_P_MCLK_OUT0_ADMA_ESAI0_TX_HF_CLK                     SC_P_MCLK_OUT0                     1
#define SC_P_MCLK_OUT0_ADMA_LCDIF_CLK                           SC_P_MCLK_OUT0                     2
#define SC_P_MCLK_OUT0_ADMA_SPI2_SDO                            SC_P_MCLK_OUT0                     3
#define SC_P_MCLK_OUT0_LSIO_GPIO0_IO20                          SC_P_MCLK_OUT0                     4
#define SC_P_UART1_TX_ADMA_UART1_TX                             SC_P_UART1_TX                      0
#define SC_P_UART1_TX_LSIO_PWM0_OUT                             SC_P_UART1_TX                      1
#define SC_P_UART1_TX_LSIO_GPT0_CAPTURE                         SC_P_UART1_TX                      2
#define SC_P_UART1_TX_LSIO_GPIO0_IO21                           SC_P_UART1_TX                      4
#define SC_P_UART1_RX_ADMA_UART1_RX                             SC_P_UART1_RX                      0
#define SC_P_UART1_RX_LSIO_PWM1_OUT                             SC_P_UART1_RX                      1
#define SC_P_UART1_RX_LSIO_GPT0_COMPARE                         SC_P_UART1_RX                      2
#define SC_P_UART1_RX_LSIO_GPT1_CLK                             SC_P_UART1_RX                      3
#define SC_P_UART1_RX_LSIO_GPIO0_IO22                           SC_P_UART1_RX                      4
#define SC_P_UART1_RTS_B_ADMA_UART1_RTS_B                       SC_P_UART1_RTS_B                   0
#define SC_P_UART1_RTS_B_LSIO_PWM2_OUT                          SC_P_UART1_RTS_B                   1
#define SC_P_UART1_RTS_B_ADMA_LCDIF_D16                         SC_P_UART1_RTS_B                   2
#define SC_P_UART1_RTS_B_LSIO_GPT1_CAPTURE                      SC_P_UART1_RTS_B                   3
#define SC_P_UART1_RTS_B_LSIO_GPT0_CLK                          SC_P_UART1_RTS_B                   4
#define SC_P_UART1_CTS_B_ADMA_UART1_CTS_B                       SC_P_UART1_CTS_B                   0
#define SC_P_UART1_CTS_B_LSIO_PWM3_OUT                          SC_P_UART1_CTS_B                   1
#define SC_P_UART1_CTS_B_ADMA_LCDIF_D17                         SC_P_UART1_CTS_B                   2
#define SC_P_UART1_CTS_B_LSIO_GPT1_COMPARE                      SC_P_UART1_CTS_B                   3
#define SC_P_UART1_CTS_B_LSIO_GPIO0_IO24                        SC_P_UART1_CTS_B                   4
#define SC_P_SAI0_TXD_ADMA_SAI0_TXD                             SC_P_SAI0_TXD                      0
#define SC_P_SAI0_TXD_ADMA_SAI1_RXC                             SC_P_SAI0_TXD                      1
#define SC_P_SAI0_TXD_ADMA_SPI1_SDO                             SC_P_SAI0_TXD                      2
#define SC_P_SAI0_TXD_ADMA_LCDIF_D18                            SC_P_SAI0_TXD                      3
#define SC_P_SAI0_TXD_LSIO_GPIO0_IO25                           SC_P_SAI0_TXD                      4
#define SC_P_SAI0_TXC_ADMA_SAI0_TXC                             SC_P_SAI0_TXC                      0
#define SC_P_SAI0_TXC_ADMA_SAI1_TXD                             SC_P_SAI0_TXC                      1
#define SC_P_SAI0_TXC_ADMA_SPI1_SDI                             SC_P_SAI0_TXC                      2
#define SC_P_SAI0_TXC_ADMA_LCDIF_D19                            SC_P_SAI0_TXC                      3
#define SC_P_SAI0_TXC_LSIO_GPIO0_IO26                           SC_P_SAI0_TXC                      4
#define SC_P_SAI0_RXD_ADMA_SAI0_RXD                             SC_P_SAI0_RXD                      0
#define SC_P_SAI0_RXD_ADMA_SAI1_RXFS                            SC_P_SAI0_RXD                      1
#define SC_P_SAI0_RXD_ADMA_SPI1_CS0                             SC_P_SAI0_RXD                      2
#define SC_P_SAI0_RXD_ADMA_LCDIF_D20                            SC_P_SAI0_RXD                      3
#define SC_P_SAI0_RXD_LSIO_GPIO0_IO27                           SC_P_SAI0_RXD                      4
#define SC_P_SAI0_TXFS_ADMA_SAI0_TXFS                           SC_P_SAI0_TXFS                     0
#define SC_P_SAI0_TXFS_ADMA_SPI2_CS1                            SC_P_SAI0_TXFS                     1
#define SC_P_SAI0_TXFS_ADMA_SPI1_SCK                            SC_P_SAI0_TXFS                     2
#define SC_P_SAI0_TXFS_LSIO_GPIO0_IO28                          SC_P_SAI0_TXFS                     4
#define SC_P_SAI1_RXD_ADMA_SAI1_RXD                             SC_P_SAI1_RXD                      0
#define SC_P_SAI1_RXD_ADMA_SAI0_RXFS                            SC_P_SAI1_RXD                      1
#define SC_P_SAI1_RXD_ADMA_SPI1_CS1                             SC_P_SAI1_RXD                      2
#define SC_P_SAI1_RXD_ADMA_LCDIF_D21                            SC_P_SAI1_RXD                      3
#define SC_P_SAI1_RXD_LSIO_GPIO0_IO29                           SC_P_SAI1_RXD                      4
#define SC_P_SAI1_RXC_ADMA_SAI1_RXC                             SC_P_SAI1_RXC                      0
#define SC_P_SAI1_RXC_ADMA_SAI1_TXC                             SC_P_SAI1_RXC                      1
#define SC_P_SAI1_RXC_ADMA_LCDIF_D22                            SC_P_SAI1_RXC                      3
#define SC_P_SAI1_RXC_LSIO_GPIO0_IO30                           SC_P_SAI1_RXC                      4
#define SC_P_SAI1_RXFS_ADMA_SAI1_RXFS                           SC_P_SAI1_RXFS                     0
#define SC_P_SAI1_RXFS_ADMA_SAI1_TXFS                           SC_P_SAI1_RXFS                     1
#define SC_P_SAI1_RXFS_ADMA_LCDIF_D23                           SC_P_SAI1_RXFS                     3
#define SC_P_SAI1_RXFS_LSIO_GPIO0_IO31                          SC_P_SAI1_RXFS                     4
#define SC_P_SPI2_CS0_ADMA_SPI2_CS0                             SC_P_SPI2_CS0                      0
#define SC_P_SPI2_CS0_LSIO_GPIO1_IO00                           SC_P_SPI2_CS0                      4
#define SC_P_SPI2_SDO_ADMA_SPI2_SDO                             SC_P_SPI2_SDO                      0
#define SC_P_SPI2_SDO_LSIO_GPIO1_IO01                           SC_P_SPI2_SDO                      4
#define SC_P_SPI2_SDI_ADMA_SPI2_SDI                             SC_P_SPI2_SDI                      0
#define SC_P_SPI2_SDI_LSIO_GPIO1_IO02                           SC_P_SPI2_SDI                      4
#define SC_P_SPI2_SCK_ADMA_SPI2_SCK                             SC_P_SPI2_SCK                      0
#define SC_P_SPI2_SCK_LSIO_GPIO1_IO03                           SC_P_SPI2_SCK                      4
#define SC_P_SPI0_SCK_ADMA_SPI0_SCK                             SC_P_SPI0_SCK                      0
#define SC_P_SPI0_SCK_ADMA_SAI0_TXC                             SC_P_SPI0_SCK                      1
#define SC_P_SPI0_SCK_M40_I2C0_SCL                              SC_P_SPI0_SCK                      2
#define SC_P_SPI0_SCK_M40_GPIO0_IO00                            SC_P_SPI0_SCK                      3
#define SC_P_SPI0_SCK_LSIO_GPIO1_IO04                           SC_P_SPI0_SCK                      4
#define SC_P_SPI0_SDI_ADMA_SPI0_SDI                             SC_P_SPI0_SDI                      0
#define SC_P_SPI0_SDI_ADMA_SAI0_TXD                             SC_P_SPI0_SDI                      1
#define SC_P_SPI0_SDI_M40_TPM0_CH0                              SC_P_SPI0_SDI                      2
#define SC_P_SPI0_SDI_M40_GPIO0_IO02                            SC_P_SPI0_SDI                      3
#define SC_P_SPI0_SDI_LSIO_GPIO1_IO05                           SC_P_SPI0_SDI                      4
#define SC_P_SPI0_SDO_ADMA_SPI0_SDO                             SC_P_SPI0_SDO                      0
#define SC_P_SPI0_SDO_ADMA_SAI0_TXFS                            SC_P_SPI0_SDO                      1
#define SC_P_SPI0_SDO_M40_I2C0_SDA                              SC_P_SPI0_SDO                      2
#define SC_P_SPI0_SDO_M40_GPIO0_IO01                            SC_P_SPI0_SDO                      3
#define SC_P_SPI0_SDO_LSIO_GPIO1_IO06                           SC_P_SPI0_SDO                      4
#define SC_P_SPI0_CS1_ADMA_SPI0_CS1                             SC_P_SPI0_CS1                      0
#define SC_P_SPI0_CS1_ADMA_SAI0_RXC                             SC_P_SPI0_CS1                      1
#define SC_P_SPI0_CS1_ADMA_SAI1_TXD                             SC_P_SPI0_CS1                      2
#define SC_P_SPI0_CS1_ADMA_LCD_PWM0_OUT                         SC_P_SPI0_CS1                      3
#define SC_P_SPI0_CS1_LSIO_GPIO1_IO07                           SC_P_SPI0_CS1                      4
#define SC_P_SPI0_CS0_ADMA_SPI0_CS0                             SC_P_SPI0_CS0                      0
#define SC_P_SPI0_CS0_ADMA_SAI0_RXD                             SC_P_SPI0_CS0                      1
#define SC_P_SPI0_CS0_M40_TPM0_CH1                              SC_P_SPI0_CS0                      2
#define SC_P_SPI0_CS0_M40_GPIO0_IO03                            SC_P_SPI0_CS0                      3
#define SC_P_SPI0_CS0_LSIO_GPIO1_IO08                           SC_P_SPI0_CS0                      4
#define SC_P_ADC_IN1_ADMA_ADC_IN1                               SC_P_ADC_IN1                       0
#define SC_P_ADC_IN1_M40_I2C0_SDA                               SC_P_ADC_IN1                       1
#define SC_P_ADC_IN1_M40_GPIO0_IO01                             SC_P_ADC_IN1                       2
#define SC_P_ADC_IN1_LSIO_GPIO1_IO09                            SC_P_ADC_IN1                       4
#define SC_P_ADC_IN0_ADMA_ADC_IN0                               SC_P_ADC_IN0                       0
#define SC_P_ADC_IN0_M40_I2C0_SCL                               SC_P_ADC_IN0                       1
#define SC_P_ADC_IN0_M40_GPIO0_IO00                             SC_P_ADC_IN0                       2
#define SC_P_ADC_IN0_LSIO_GPIO1_IO10                            SC_P_ADC_IN0                       4
#define SC_P_ADC_IN3_ADMA_ADC_IN3                               SC_P_ADC_IN3                       0
#define SC_P_ADC_IN3_M40_UART0_TX                               SC_P_ADC_IN3                       1
#define SC_P_ADC_IN3_M40_GPIO0_IO03                             SC_P_ADC_IN3                       2
#define SC_P_ADC_IN3_ADMA_ACM_MCLK_OUT0                         SC_P_ADC_IN3                       3
#define SC_P_ADC_IN3_LSIO_GPIO1_IO11                            SC_P_ADC_IN3                       4
#define SC_P_ADC_IN2_ADMA_ADC_IN2                               SC_P_ADC_IN2                       0
#define SC_P_ADC_IN2_M40_UART0_RX                               SC_P_ADC_IN2                       1
#define SC_P_ADC_IN2_M40_GPIO0_IO02                             SC_P_ADC_IN2                       2
#define SC_P_ADC_IN2_ADMA_ACM_MCLK_IN0                          SC_P_ADC_IN2                       3
#define SC_P_ADC_IN2_LSIO_GPIO1_IO12                            SC_P_ADC_IN2                       4
#define SC_P_ADC_IN5_ADMA_ADC_IN5                               SC_P_ADC_IN5                       0
#define SC_P_ADC_IN5_M40_TPM0_CH1                               SC_P_ADC_IN5                       1
#define SC_P_ADC_IN5_M40_GPIO0_IO05                             SC_P_ADC_IN5                       2
#define SC_P_ADC_IN5_LSIO_GPIO1_IO13                            SC_P_ADC_IN5                       4
#define SC_P_ADC_IN4_ADMA_ADC_IN4                               SC_P_ADC_IN4                       0
#define SC_P_ADC_IN4_M40_TPM0_CH0                               SC_P_ADC_IN4                       1
#define SC_P_ADC_IN4_M40_GPIO0_IO04                             SC_P_ADC_IN4                       2
#define SC_P_ADC_IN4_LSIO_GPIO1_IO14                            SC_P_ADC_IN4                       4
#define SC_P_FLEXCAN0_RX_ADMA_FLEXCAN0_RX                       SC_P_FLEXCAN0_RX                   0
#define SC_P_FLEXCAN0_RX_ADMA_SAI2_RXC                          SC_P_FLEXCAN0_RX                   1
#define SC_P_FLEXCAN0_RX_ADMA_UART0_RTS_B                       SC_P_FLEXCAN0_RX                   2
#define SC_P_FLEXCAN0_RX_ADMA_SAI1_TXC                          SC_P_FLEXCAN0_RX                   3
#define SC_P_FLEXCAN0_RX_LSIO_GPIO1_IO15                        SC_P_FLEXCAN0_RX                   4
#define SC_P_FLEXCAN0_TX_ADMA_FLEXCAN0_TX                       SC_P_FLEXCAN0_TX                   0
#define SC_P_FLEXCAN0_TX_ADMA_SAI2_RXD                          SC_P_FLEXCAN0_TX                   1
#define SC_P_FLEXCAN0_TX_ADMA_UART0_CTS_B                       SC_P_FLEXCAN0_TX                   2
#define SC_P_FLEXCAN0_TX_ADMA_SAI1_TXFS                         SC_P_FLEXCAN0_TX                   3
#define SC_P_FLEXCAN0_TX_LSIO_GPIO1_IO16                        SC_P_FLEXCAN0_TX                   4
#define SC_P_FLEXCAN1_RX_ADMA_FLEXCAN1_RX                       SC_P_FLEXCAN1_RX                   0
#define SC_P_FLEXCAN1_RX_ADMA_SAI2_RXFS                         SC_P_FLEXCAN1_RX                   1
#define SC_P_FLEXCAN1_RX_ADMA_FTM_CH2                           SC_P_FLEXCAN1_RX                   2
#define SC_P_FLEXCAN1_RX_ADMA_SAI1_TXD                          SC_P_FLEXCAN1_RX                   3
#define SC_P_FLEXCAN1_RX_LSIO_GPIO1_IO17                        SC_P_FLEXCAN1_RX                   4
#define SC_P_FLEXCAN1_TX_ADMA_FLEXCAN1_TX                       SC_P_FLEXCAN1_TX                   0
#define SC_P_FLEXCAN1_TX_ADMA_SAI3_RXC                          SC_P_FLEXCAN1_TX                   1
#define SC_P_FLEXCAN1_TX_ADMA_DMA0_REQ_IN0                      SC_P_FLEXCAN1_TX                   2
#define SC_P_FLEXCAN1_TX_ADMA_SAI1_RXD                          SC_P_FLEXCAN1_TX                   3
#define SC_P_FLEXCAN1_TX_LSIO_GPIO1_IO18                        SC_P_FLEXCAN1_TX                   4
#define SC_P_FLEXCAN2_RX_ADMA_FLEXCAN2_RX                       SC_P_FLEXCAN2_RX                   0
#define SC_P_FLEXCAN2_RX_ADMA_SAI3_RXD                          SC_P_FLEXCAN2_RX                   1
#define SC_P_FLEXCAN2_RX_ADMA_UART3_RX                          SC_P_FLEXCAN2_RX                   2
#define SC_P_FLEXCAN2_RX_ADMA_SAI1_RXFS                         SC_P_FLEXCAN2_RX                   3
#define SC_P_FLEXCAN2_RX_LSIO_GPIO1_IO19                        SC_P_FLEXCAN2_RX                   4
#define SC_P_FLEXCAN2_TX_ADMA_FLEXCAN2_TX                       SC_P_FLEXCAN2_TX                   0
#define SC_P_FLEXCAN2_TX_ADMA_SAI3_RXFS                         SC_P_FLEXCAN2_TX                   1
#define SC_P_FLEXCAN2_TX_ADMA_UART3_TX                          SC_P_FLEXCAN2_TX                   2
#define SC_P_FLEXCAN2_TX_ADMA_SAI1_RXC                          SC_P_FLEXCAN2_TX                   3
#define SC_P_FLEXCAN2_TX_LSIO_GPIO1_IO20                        SC_P_FLEXCAN2_TX                   4
#define SC_P_UART0_RX_ADMA_UART0_RX                             SC_P_UART0_RX                      0
#define SC_P_UART0_RX_ADMA_MQS_R                                SC_P_UART0_RX                      1
#define SC_P_UART0_RX_ADMA_FLEXCAN0_RX                          SC_P_UART0_RX                      2
#define SC_P_UART0_RX_SCU_UART0_RX                              SC_P_UART0_RX                      3
#define SC_P_UART0_RX_LSIO_GPIO1_IO21                           SC_P_UART0_RX                      4
#define SC_P_UART0_TX_ADMA_UART0_TX                             SC_P_UART0_TX                      0
#define SC_P_UART0_TX_ADMA_MQS_L                                SC_P_UART0_TX                      1
#define SC_P_UART0_TX_ADMA_FLEXCAN0_TX                          SC_P_UART0_TX                      2
#define SC_P_UART0_TX_SCU_UART0_TX                              SC_P_UART0_TX                      3
#define SC_P_UART0_TX_LSIO_GPIO1_IO22                           SC_P_UART0_TX                      4
#define SC_P_UART2_TX_ADMA_UART2_TX                             SC_P_UART2_TX                      0
#define SC_P_UART2_TX_ADMA_FTM_CH1                              SC_P_UART2_TX                      1
#define SC_P_UART2_TX_ADMA_FLEXCAN1_TX                          SC_P_UART2_TX                      2
#define SC_P_UART2_TX_LSIO_GPIO1_IO23                           SC_P_UART2_TX                      4
#define SC_P_UART2_RX_ADMA_UART2_RX                             SC_P_UART2_RX                      0
#define SC_P_UART2_RX_ADMA_FTM_CH0                              SC_P_UART2_RX                      1
#define SC_P_UART2_RX_ADMA_FLEXCAN1_RX                          SC_P_UART2_RX                      2
#define SC_P_UART2_RX_LSIO_GPIO1_IO24                           SC_P_UART2_RX                      4
#define SC_P_MIPI_DSI0_I2C0_SCL_MIPI_DSI0_I2C0_SCL              SC_P_MIPI_DSI0_I2C0_SCL            0
#define SC_P_MIPI_DSI0_I2C0_SCL_MIPI_DSI1_GPIO0_IO02            SC_P_MIPI_DSI0_I2C0_SCL            1
#define SC_P_MIPI_DSI0_I2C0_SCL_LSIO_GPIO1_IO25                 SC_P_MIPI_DSI0_I2C0_SCL            4
#define SC_P_MIPI_DSI0_I2C0_SDA_MIPI_DSI0_I2C0_SDA              SC_P_MIPI_DSI0_I2C0_SDA            0
#define SC_P_MIPI_DSI0_I2C0_SDA_MIPI_DSI1_GPIO0_IO03            SC_P_MIPI_DSI0_I2C0_SDA            1
#define SC_P_MIPI_DSI0_I2C0_SDA_LSIO_GPIO1_IO26                 SC_P_MIPI_DSI0_I2C0_SDA            4
#define SC_P_MIPI_DSI0_GPIO0_00_MIPI_DSI0_GPIO0_IO00            SC_P_MIPI_DSI0_GPIO0_00            0
#define SC_P_MIPI_DSI0_GPIO0_00_ADMA_I2C1_SCL                   SC_P_MIPI_DSI0_GPIO0_00            1
#define SC_P_MIPI_DSI0_GPIO0_00_MIPI_DSI0_PWM0_OUT              SC_P_MIPI_DSI0_GPIO0_00            2
#define SC_P_MIPI_DSI0_GPIO0_00_LSIO_GPIO1_IO27                 SC_P_MIPI_DSI0_GPIO0_00            4
#define SC_P_MIPI_DSI0_GPIO0_01_MIPI_DSI0_GPIO0_IO01            SC_P_MIPI_DSI0_GPIO0_01            0
#define SC_P_MIPI_DSI0_GPIO0_01_ADMA_I2C1_SDA                   SC_P_MIPI_DSI0_GPIO0_01            1
#define SC_P_MIPI_DSI0_GPIO0_01_LSIO_GPIO1_IO28                 SC_P_MIPI_DSI0_GPIO0_01            4
#define SC_P_MIPI_DSI1_I2C0_SCL_MIPI_DSI1_I2C0_SCL              SC_P_MIPI_DSI1_I2C0_SCL            0
#define SC_P_MIPI_DSI1_I2C0_SCL_MIPI_DSI0_GPIO0_IO02            SC_P_MIPI_DSI1_I2C0_SCL            1
#define SC_P_MIPI_DSI1_I2C0_SCL_LSIO_GPIO1_IO29                 SC_P_MIPI_DSI1_I2C0_SCL            4
#define SC_P_MIPI_DSI1_I2C0_SDA_MIPI_DSI1_I2C0_SDA              SC_P_MIPI_DSI1_I2C0_SDA            0
#define SC_P_MIPI_DSI1_I2C0_SDA_MIPI_DSI0_GPIO0_IO03            SC_P_MIPI_DSI1_I2C0_SDA            1
#define SC_P_MIPI_DSI1_I2C0_SDA_LSIO_GPIO1_IO30                 SC_P_MIPI_DSI1_I2C0_SDA            4
#define SC_P_MIPI_DSI1_GPIO0_00_MIPI_DSI1_GPIO0_IO00            SC_P_MIPI_DSI1_GPIO0_00            0
#define SC_P_MIPI_DSI1_GPIO0_00_ADMA_I2C2_SCL                   SC_P_MIPI_DSI1_GPIO0_00            1
#define SC_P_MIPI_DSI1_GPIO0_00_MIPI_DSI1_PWM0_OUT              SC_P_MIPI_DSI1_GPIO0_00            2
#define SC_P_MIPI_DSI1_GPIO0_00_LSIO_GPIO1_IO31                 SC_P_MIPI_DSI1_GPIO0_00            4
#define SC_P_MIPI_DSI1_GPIO0_01_MIPI_DSI1_GPIO0_IO01            SC_P_MIPI_DSI1_GPIO0_01            0
#define SC_P_MIPI_DSI1_GPIO0_01_ADMA_I2C2_SDA                   SC_P_MIPI_DSI1_GPIO0_01            1
#define SC_P_MIPI_DSI1_GPIO0_01_LSIO_GPIO2_IO00                 SC_P_MIPI_DSI1_GPIO0_01            4
#define SC_P_JTAG_TRST_B_SCU_JTAG_TRST_B                        SC_P_JTAG_TRST_B                   0
#define SC_P_JTAG_TRST_B_SCU_WDOG0_WDOG_OUT                     SC_P_JTAG_TRST_B                   1
#define SC_P_PMIC_I2C_SCL_SCU_PMIC_I2C_SCL                      SC_P_PMIC_I2C_SCL                  0
#define SC_P_PMIC_I2C_SCL_SCU_GPIO0_IOXX_PMIC_A35_ON            SC_P_PMIC_I2C_SCL                  1
#define SC_P_PMIC_I2C_SCL_LSIO_GPIO2_IO01                       SC_P_PMIC_I2C_SCL                  4
#define SC_P_PMIC_I2C_SDA_SCU_PMIC_I2C_SDA                      SC_P_PMIC_I2C_SDA                  0
#define SC_P_PMIC_I2C_SDA_SCU_GPIO0_IOXX_PMIC_GPU_ON            SC_P_PMIC_I2C_SDA                  1
#define SC_P_PMIC_I2C_SDA_LSIO_GPIO2_IO02                       SC_P_PMIC_I2C_SDA                  4
#define SC_P_PMIC_INT_B_SCU_DSC_PMIC_INT_B                      SC_P_PMIC_INT_B                    0
#define SC_P_SCU_GPIO0_00_SCU_GPIO0_IO00                        SC_P_SCU_GPIO0_00                  0
#define SC_P_SCU_GPIO0_00_SCU_UART0_RX                          SC_P_SCU_GPIO0_00                  1
#define SC_P_SCU_GPIO0_00_M40_UART0_RX                          SC_P_SCU_GPIO0_00                  2
#define SC_P_SCU_GPIO0_00_ADMA_UART3_RX                         SC_P_SCU_GPIO0_00                  3
#define SC_P_SCU_GPIO0_00_LSIO_GPIO2_IO03                       SC_P_SCU_GPIO0_00                  4
#define SC_P_SCU_GPIO0_01_SCU_GPIO0_IO01                        SC_P_SCU_GPIO0_01                  0
#define SC_P_SCU_GPIO0_01_SCU_UART0_TX                          SC_P_SCU_GPIO0_01                  1
#define SC_P_SCU_GPIO0_01_M40_UART0_TX                          SC_P_SCU_GPIO0_01                  2
#define SC_P_SCU_GPIO0_01_ADMA_UART3_TX                         SC_P_SCU_GPIO0_01                  3
#define SC_P_SCU_GPIO0_01_SCU_WDOG0_WDOG_OUT                    SC_P_SCU_GPIO0_01                  4
#define SC_P_SCU_PMIC_STANDBY_SCU_DSC_PMIC_STANDBY              SC_P_SCU_PMIC_STANDBY              0
#define SC_P_SCU_BOOT_MODE0_SCU_DSC_BOOT_MODE0                  SC_P_SCU_BOOT_MODE0                0
#define SC_P_SCU_BOOT_MODE1_SCU_DSC_BOOT_MODE1                  SC_P_SCU_BOOT_MODE1                0
#define SC_P_SCU_BOOT_MODE2_SCU_DSC_BOOT_MODE2                  SC_P_SCU_BOOT_MODE2                0
#define SC_P_SCU_BOOT_MODE2_SCU_PMIC_I2C_SDA                    SC_P_SCU_BOOT_MODE2                1
#define SC_P_SCU_BOOT_MODE3_SCU_DSC_BOOT_MODE3                  SC_P_SCU_BOOT_MODE3                0
#define SC_P_SCU_BOOT_MODE3_SCU_PMIC_I2C_SCL                    SC_P_SCU_BOOT_MODE3                1
#define SC_P_SCU_BOOT_MODE3_SCU_DSC_RTC_CLOCK_OUTPUT_32K        SC_P_SCU_BOOT_MODE3                3
#define SC_P_CSI_D00_CI_PI_D02                                  SC_P_CSI_D00                       0
#define SC_P_CSI_D00_ADMA_SAI0_RXC                              SC_P_CSI_D00                       2
#define SC_P_CSI_D01_CI_PI_D03                                  SC_P_CSI_D01                       0
#define SC_P_CSI_D01_ADMA_SAI0_RXD                              SC_P_CSI_D01                       2
#define SC_P_CSI_D02_CI_PI_D04                                  SC_P_CSI_D02                       0
#define SC_P_CSI_D02_ADMA_SAI0_RXFS                             SC_P_CSI_D02                       2
#define SC_P_CSI_D03_CI_PI_D05                                  SC_P_CSI_D03                       0
#define SC_P_CSI_D03_ADMA_SAI2_RXC                              SC_P_CSI_D03                       2
#define SC_P_CSI_D04_CI_PI_D06                                  SC_P_CSI_D04                       0
#define SC_P_CSI_D04_ADMA_SAI2_RXD                              SC_P_CSI_D04                       2
#define SC_P_CSI_D05_CI_PI_D07                                  SC_P_CSI_D05                       0
#define SC_P_CSI_D05_ADMA_SAI2_RXFS                             SC_P_CSI_D05                       2
#define SC_P_CSI_D06_CI_PI_D08                                  SC_P_CSI_D06                       0
#define SC_P_CSI_D06_ADMA_SAI3_RXC                              SC_P_CSI_D06                       2
#define SC_P_CSI_D07_CI_PI_D09                                  SC_P_CSI_D07                       0
#define SC_P_CSI_D07_ADMA_SAI3_RXD                              SC_P_CSI_D07                       2
#define SC_P_CSI_HSYNC_CI_PI_HSYNC                              SC_P_CSI_HSYNC                     0
#define SC_P_CSI_HSYNC_CI_PI_D00                                SC_P_CSI_HSYNC                     1
#define SC_P_CSI_HSYNC_ADMA_SAI3_RXFS                           SC_P_CSI_HSYNC                     2
#define SC_P_CSI_VSYNC_CI_PI_VSYNC                              SC_P_CSI_VSYNC                     0
#define SC_P_CSI_VSYNC_CI_PI_D01                                SC_P_CSI_VSYNC                     1
#define SC_P_CSI_PCLK_CI_PI_PCLK                                SC_P_CSI_PCLK                      0
#define SC_P_CSI_PCLK_MIPI_CSI0_I2C0_SCL                        SC_P_CSI_PCLK                      1
#define SC_P_CSI_PCLK_ADMA_SPI1_SCK                             SC_P_CSI_PCLK                      3
#define SC_P_CSI_PCLK_LSIO_GPIO3_IO00                           SC_P_CSI_PCLK                      4
#define SC_P_CSI_MCLK_CI_PI_MCLK                                SC_P_CSI_MCLK                      0
#define SC_P_CSI_MCLK_MIPI_CSI0_I2C0_SDA                        SC_P_CSI_MCLK                      1
#define SC_P_CSI_MCLK_ADMA_SPI1_SDO                             SC_P_CSI_MCLK                      3
#define SC_P_CSI_MCLK_LSIO_GPIO3_IO01                           SC_P_CSI_MCLK                      4
#define SC_P_CSI_EN_CI_PI_EN                                    SC_P_CSI_EN                        0
#define SC_P_CSI_EN_CI_PI_I2C_SCL                               SC_P_CSI_EN                        1
#define SC_P_CSI_EN_ADMA_I2C3_SCL                               SC_P_CSI_EN                        2
#define SC_P_CSI_EN_ADMA_SPI1_SDI                               SC_P_CSI_EN                        3
#define SC_P_CSI_EN_LSIO_GPIO3_IO02                             SC_P_CSI_EN                        4
#define SC_P_CSI_RESET_CI_PI_RESET                              SC_P_CSI_RESET                     0
#define SC_P_CSI_RESET_CI_PI_I2C_SDA                            SC_P_CSI_RESET                     1
#define SC_P_CSI_RESET_ADMA_I2C3_SDA                            SC_P_CSI_RESET                     2
#define SC_P_CSI_RESET_ADMA_SPI1_CS0                            SC_P_CSI_RESET                     3
#define SC_P_CSI_RESET_LSIO_GPIO3_IO03                          SC_P_CSI_RESET                     4
#define SC_P_MIPI_CSI0_MCLK_OUT_MIPI_CSI0_ACM_MCLK_OUT          SC_P_MIPI_CSI0_MCLK_OUT            0
#define SC_P_MIPI_CSI0_MCLK_OUT_LSIO_GPIO3_IO04                 SC_P_MIPI_CSI0_MCLK_OUT            4
#define SC_P_MIPI_CSI0_I2C0_SCL_MIPI_CSI0_I2C0_SCL              SC_P_MIPI_CSI0_I2C0_SCL            0
#define SC_P_MIPI_CSI0_I2C0_SCL_MIPI_CSI0_GPIO0_IO02            SC_P_MIPI_CSI0_I2C0_SCL            1
#define SC_P_MIPI_CSI0_I2C0_SCL_LSIO_GPIO3_IO05                 SC_P_MIPI_CSI0_I2C0_SCL            4
#define SC_P_MIPI_CSI0_I2C0_SDA_MIPI_CSI0_I2C0_SDA              SC_P_MIPI_CSI0_I2C0_SDA            0
#define SC_P_MIPI_CSI0_I2C0_SDA_MIPI_CSI0_GPIO0_IO03            SC_P_MIPI_CSI0_I2C0_SDA            1
#define SC_P_MIPI_CSI0_I2C0_SDA_LSIO_GPIO3_IO06                 SC_P_MIPI_CSI0_I2C0_SDA            4
#define SC_P_MIPI_CSI0_GPIO0_01_MIPI_CSI0_GPIO0_IO01            SC_P_MIPI_CSI0_GPIO0_01            0
#define SC_P_MIPI_CSI0_GPIO0_01_ADMA_I2C0_SDA                   SC_P_MIPI_CSI0_GPIO0_01            1
#define SC_P_MIPI_CSI0_GPIO0_01_LSIO_GPIO3_IO07                 SC_P_MIPI_CSI0_GPIO0_01            4
#define SC_P_MIPI_CSI0_GPIO0_00_MIPI_CSI0_GPIO0_IO00            SC_P_MIPI_CSI0_GPIO0_00            0
#define SC_P_MIPI_CSI0_GPIO0_00_ADMA_I2C0_SCL                   SC_P_MIPI_CSI0_GPIO0_00            1
#define SC_P_MIPI_CSI0_GPIO0_00_LSIO_GPIO3_IO08                 SC_P_MIPI_CSI0_GPIO0_00            4
#define SC_P_QSPI0A_DATA0_LSIO_QSPI0A_DATA0                     SC_P_QSPI0A_DATA0                  0
#define SC_P_QSPI0A_DATA0_LSIO_GPIO3_IO09                       SC_P_QSPI0A_DATA0                  4
#define SC_P_QSPI0A_DATA1_LSIO_QSPI0A_DATA1                     SC_P_QSPI0A_DATA1                  0
#define SC_P_QSPI0A_DATA1_LSIO_GPIO3_IO10                       SC_P_QSPI0A_DATA1                  4
#define SC_P_QSPI0A_DATA2_LSIO_QSPI0A_DATA2                     SC_P_QSPI0A_DATA2                  0
#define SC_P_QSPI0A_DATA2_LSIO_GPIO3_IO11                       SC_P_QSPI0A_DATA2                  4
#define SC_P_QSPI0A_DATA3_LSIO_QSPI0A_DATA3                     SC_P_QSPI0A_DATA3                  0
#define SC_P_QSPI0A_DATA3_LSIO_GPIO3_IO12                       SC_P_QSPI0A_DATA3                  4
#define SC_P_QSPI0A_DQS_LSIO_QSPI0A_DQS                         SC_P_QSPI0A_DQS                    0
#define SC_P_QSPI0A_DQS_LSIO_GPIO3_IO13                         SC_P_QSPI0A_DQS                    4
#define SC_P_QSPI0A_SS0_B_LSIO_QSPI0A_SS0_B                     SC_P_QSPI0A_SS0_B                  0
#define SC_P_QSPI0A_SS0_B_LSIO_GPIO3_IO14                       SC_P_QSPI0A_SS0_B                  4
#define SC_P_QSPI0A_SS1_B_LSIO_QSPI0A_SS1_B                     SC_P_QSPI0A_SS1_B                  0
#define SC_P_QSPI0A_SS1_B_LSIO_GPIO3_IO15                       SC_P_QSPI0A_SS1_B                  4
#define SC_P_QSPI0A_SCLK_LSIO_QSPI0A_SCLK                       SC_P_QSPI0A_SCLK                   0
#define SC_P_QSPI0A_SCLK_LSIO_GPIO3_IO16                        SC_P_QSPI0A_SCLK                   4
#define SC_P_QSPI0B_SCLK_LSIO_QSPI0B_SCLK                       SC_P_QSPI0B_SCLK                   0
#define SC_P_QSPI0B_SCLK_LSIO_QSPI1A_SCLK                       SC_P_QSPI0B_SCLK                   1
#define SC_P_QSPI0B_SCLK_LSIO_KPP0_COL0                         SC_P_QSPI0B_SCLK                   2
#define SC_P_QSPI0B_SCLK_LSIO_GPIO3_IO17                        SC_P_QSPI0B_SCLK                   4
#define SC_P_QSPI0B_DATA0_LSIO_QSPI0B_DATA0                     SC_P_QSPI0B_DATA0                  0
#define SC_P_QSPI0B_DATA0_LSIO_QSPI1A_DATA0                     SC_P_QSPI0B_DATA0                  1
#define SC_P_QSPI0B_DATA0_LSIO_KPP0_COL1                        SC_P_QSPI0B_DATA0                  2
#define SC_P_QSPI0B_DATA0_LSIO_GPIO3_IO18                       SC_P_QSPI0B_DATA0                  4
#define SC_P_QSPI0B_DATA1_LSIO_QSPI0B_DATA1                     SC_P_QSPI0B_DATA1                  0
#define SC_P_QSPI0B_DATA1_LSIO_QSPI1A_DATA1                     SC_P_QSPI0B_DATA1                  1
#define SC_P_QSPI0B_DATA1_LSIO_KPP0_COL2                        SC_P_QSPI0B_DATA1                  2
#define SC_P_QSPI0B_DATA1_LSIO_GPIO3_IO19                       SC_P_QSPI0B_DATA1                  4
#define SC_P_QSPI0B_DATA2_LSIO_QSPI0B_DATA2                     SC_P_QSPI0B_DATA2                  0
#define SC_P_QSPI0B_DATA2_LSIO_QSPI1A_DATA2                     SC_P_QSPI0B_DATA2                  1
#define SC_P_QSPI0B_DATA2_LSIO_KPP0_COL3                        SC_P_QSPI0B_DATA2                  2
#define SC_P_QSPI0B_DATA2_LSIO_GPIO3_IO20                       SC_P_QSPI0B_DATA2                  4
#define SC_P_QSPI0B_DATA3_LSIO_QSPI0B_DATA3                     SC_P_QSPI0B_DATA3                  0
#define SC_P_QSPI0B_DATA3_LSIO_QSPI1A_DATA3                     SC_P_QSPI0B_DATA3                  1
#define SC_P_QSPI0B_DATA3_LSIO_KPP0_ROW0                        SC_P_QSPI0B_DATA3                  2
#define SC_P_QSPI0B_DATA3_LSIO_GPIO3_IO21                       SC_P_QSPI0B_DATA3                  4
#define SC_P_QSPI0B_DQS_LSIO_QSPI0B_DQS                         SC_P_QSPI0B_DQS                    0
#define SC_P_QSPI0B_DQS_LSIO_QSPI1A_DQS                         SC_P_QSPI0B_DQS                    1
#define SC_P_QSPI0B_DQS_LSIO_KPP0_ROW1                          SC_P_QSPI0B_DQS                    2
#define SC_P_QSPI0B_DQS_LSIO_GPIO3_IO22                         SC_P_QSPI0B_DQS                    4
#define SC_P_QSPI0B_SS0_B_LSIO_QSPI0B_SS0_B                     SC_P_QSPI0B_SS0_B                  0
#define SC_P_QSPI0B_SS0_B_LSIO_QSPI1A_SS0_B                     SC_P_QSPI0B_SS0_B                  1
#define SC_P_QSPI0B_SS0_B_LSIO_KPP0_ROW2                        SC_P_QSPI0B_SS0_B                  2
#define SC_P_QSPI0B_SS0_B_LSIO_GPIO3_IO23                       SC_P_QSPI0B_SS0_B                  4
#define SC_P_QSPI0B_SS1_B_LSIO_QSPI0B_SS1_B                     SC_P_QSPI0B_SS1_B                  0
#define SC_P_QSPI0B_SS1_B_LSIO_QSPI1A_SS1_B                     SC_P_QSPI0B_SS1_B                  1
#define SC_P_QSPI0B_SS1_B_LSIO_KPP0_ROW3                        SC_P_QSPI0B_SS1_B                  2
#define SC_P_QSPI0B_SS1_B_LSIO_GPIO3_IO24                       SC_P_QSPI0B_SS1_B                  4

#endif				/* _SC_PADS_H */
