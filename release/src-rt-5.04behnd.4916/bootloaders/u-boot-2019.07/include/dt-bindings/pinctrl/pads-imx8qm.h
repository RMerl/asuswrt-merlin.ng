/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#ifndef SC_PADS_H
#define SC_PADS_H

#define SC_P_SIM0_CLK                            0	/* DMA.SIM0.CLK, LSIO.GPIO0.IO00 */
#define SC_P_SIM0_RST                            1	/* DMA.SIM0.RST, LSIO.GPIO0.IO01 */
#define SC_P_SIM0_IO                             2	/* DMA.SIM0.IO, LSIO.GPIO0.IO02 */
#define SC_P_SIM0_PD                             3	/* DMA.SIM0.PD, DMA.I2C3.SCL, LSIO.GPIO0.IO03 */
#define SC_P_SIM0_POWER_EN                       4	/* DMA.SIM0.POWER_EN, DMA.I2C3.SDA, LSIO.GPIO0.IO04 */
#define SC_P_SIM0_GPIO0_00                       5	/* DMA.SIM0.POWER_EN, LSIO.GPIO0.IO05 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_SIM           6	/*  */
#define SC_P_M40_I2C0_SCL                        7	/* M40.I2C0.SCL, M40.UART0.RX, M40.GPIO0.IO02, LSIO.GPIO0.IO06 */
#define SC_P_M40_I2C0_SDA                        8	/* M40.I2C0.SDA, M40.UART0.TX, M40.GPIO0.IO03, LSIO.GPIO0.IO07 */
#define SC_P_M40_GPIO0_00                        9	/* M40.GPIO0.IO00, M40.TPM0.CH0, DMA.UART4.RX, LSIO.GPIO0.IO08 */
#define SC_P_M40_GPIO0_01                        10	/* M40.GPIO0.IO01, M40.TPM0.CH1, DMA.UART4.TX, LSIO.GPIO0.IO09 */
#define SC_P_M41_I2C0_SCL                        11	/* M41.I2C0.SCL, M41.UART0.RX, M41.GPIO0.IO02, LSIO.GPIO0.IO10 */
#define SC_P_M41_I2C0_SDA                        12	/* M41.I2C0.SDA, M41.UART0.TX, M41.GPIO0.IO03, LSIO.GPIO0.IO11 */
#define SC_P_M41_GPIO0_00                        13	/* M41.GPIO0.IO00, M41.TPM0.CH0, DMA.UART3.RX, LSIO.GPIO0.IO12 */
#define SC_P_M41_GPIO0_01                        14	/* M41.GPIO0.IO01, M41.TPM0.CH1, DMA.UART3.TX, LSIO.GPIO0.IO13 */
#define SC_P_GPT0_CLK                            15	/* LSIO.GPT0.CLK, DMA.I2C1.SCL, LSIO.KPP0.COL4, LSIO.GPIO0.IO14 */
#define SC_P_GPT0_CAPTURE                        16	/* LSIO.GPT0.CAPTURE, DMA.I2C1.SDA, LSIO.KPP0.COL5, LSIO.GPIO0.IO15 */
#define SC_P_GPT0_COMPARE                        17	/* LSIO.GPT0.COMPARE, LSIO.PWM3.OUT, LSIO.KPP0.COL6, LSIO.GPIO0.IO16 */
#define SC_P_GPT1_CLK                            18	/* LSIO.GPT1.CLK, DMA.I2C2.SCL, LSIO.KPP0.COL7, LSIO.GPIO0.IO17 */
#define SC_P_GPT1_CAPTURE                        19	/* LSIO.GPT1.CAPTURE, DMA.I2C2.SDA, LSIO.KPP0.ROW4, LSIO.GPIO0.IO18 */
#define SC_P_GPT1_COMPARE                        20	/* LSIO.GPT1.COMPARE, LSIO.PWM2.OUT, LSIO.KPP0.ROW5, LSIO.GPIO0.IO19 */
#define SC_P_UART0_RX                            21	/* DMA.UART0.RX, SCU.UART0.RX, LSIO.GPIO0.IO20 */
#define SC_P_UART0_TX                            22	/* DMA.UART0.TX, SCU.UART0.TX, LSIO.GPIO0.IO21 */
#define SC_P_UART0_RTS_B                         23	/* DMA.UART0.RTS_B, LSIO.PWM0.OUT, DMA.UART2.RX, LSIO.GPIO0.IO22 */
#define SC_P_UART0_CTS_B                         24	/* DMA.UART0.CTS_B, LSIO.PWM1.OUT, DMA.UART2.TX, LSIO.GPIO0.IO23 */
#define SC_P_UART1_TX                            25	/* DMA.UART1.TX, DMA.SPI3.SCK, LSIO.GPIO0.IO24 */
#define SC_P_UART1_RX                            26	/* DMA.UART1.RX, DMA.SPI3.SDO, LSIO.GPIO0.IO25 */
#define SC_P_UART1_RTS_B                         27	/* DMA.UART1.RTS_B, DMA.SPI3.SDI, DMA.UART1.CTS_B, LSIO.GPIO0.IO26 */
#define SC_P_UART1_CTS_B                         28	/* DMA.UART1.CTS_B, DMA.SPI3.CS0, DMA.UART1.RTS_B, LSIO.GPIO0.IO27 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_GPIOLH        29	/*  */
#define SC_P_SCU_PMIC_MEMC_ON                    30	/* SCU.GPIO0.IOXX_PMIC_MEMC_ON */
#define SC_P_SCU_WDOG_OUT                        31	/* SCU.WDOG0.WDOG_OUT */
#define SC_P_PMIC_I2C_SDA                        32	/* SCU.PMIC_I2C.SDA */
#define SC_P_PMIC_I2C_SCL                        33	/* SCU.PMIC_I2C.SCL */
#define SC_P_PMIC_EARLY_WARNING                  34	/* SCU.PMIC_EARLY_WARNING */
#define SC_P_PMIC_INT_B                          35	/* SCU.DSC.PMIC_INT_B */
#define SC_P_SCU_GPIO0_00                        36	/* SCU.GPIO0.IO00, SCU.UART0.RX, LSIO.GPIO0.IO28 */
#define SC_P_SCU_GPIO0_01                        37	/* SCU.GPIO0.IO01, SCU.UART0.TX, LSIO.GPIO0.IO29 */
#define SC_P_SCU_GPIO0_02                        38	/* SCU.GPIO0.IO02, SCU.GPIO0.IOXX_PMIC_GPU0_ON, LSIO.GPIO0.IO30 */
#define SC_P_SCU_GPIO0_03                        39	/* SCU.GPIO0.IO03, SCU.GPIO0.IOXX_PMIC_GPU1_ON, LSIO.GPIO0.IO31 */
#define SC_P_SCU_GPIO0_04                        40	/* SCU.GPIO0.IO04, SCU.GPIO0.IOXX_PMIC_A72_ON, LSIO.GPIO1.IO00 */
#define SC_P_SCU_GPIO0_05                        41	/* SCU.GPIO0.IO05, SCU.GPIO0.IOXX_PMIC_A53_ON, LSIO.GPIO1.IO01 */
#define SC_P_SCU_GPIO0_06                        42	/* SCU.GPIO0.IO06, SCU.TPM0.CH0, LSIO.GPIO1.IO02 */
#define SC_P_SCU_GPIO0_07                        43	/* SCU.GPIO0.IO07, SCU.TPM0.CH1, SCU.DSC.RTC_CLOCK_OUTPUT_32K, LSIO.GPIO1.IO03 */
#define SC_P_SCU_BOOT_MODE0                      44	/* SCU.DSC.BOOT_MODE0 */
#define SC_P_SCU_BOOT_MODE1                      45	/* SCU.DSC.BOOT_MODE1 */
#define SC_P_SCU_BOOT_MODE2                      46	/* SCU.DSC.BOOT_MODE2 */
#define SC_P_SCU_BOOT_MODE3                      47	/* SCU.DSC.BOOT_MODE3 */
#define SC_P_SCU_BOOT_MODE4                      48	/* SCU.DSC.BOOT_MODE4, SCU.PMIC_I2C.SCL */
#define SC_P_SCU_BOOT_MODE5                      49	/* SCU.DSC.BOOT_MODE5, SCU.PMIC_I2C.SDA */
#define SC_P_LVDS0_GPIO00                        50	/* LVDS0.GPIO0.IO00, LVDS0.PWM0.OUT, LSIO.GPIO1.IO04 */
#define SC_P_LVDS0_GPIO01                        51	/* LVDS0.GPIO0.IO01, LSIO.GPIO1.IO05 */
#define SC_P_LVDS0_I2C0_SCL                      52	/* LVDS0.I2C0.SCL, LVDS0.GPIO0.IO02, LSIO.GPIO1.IO06 */
#define SC_P_LVDS0_I2C0_SDA                      53	/* LVDS0.I2C0.SDA, LVDS0.GPIO0.IO03, LSIO.GPIO1.IO07 */
#define SC_P_LVDS0_I2C1_SCL                      54	/* LVDS0.I2C1.SCL, DMA.UART2.TX, LSIO.GPIO1.IO08 */
#define SC_P_LVDS0_I2C1_SDA                      55	/* LVDS0.I2C1.SDA, DMA.UART2.RX, LSIO.GPIO1.IO09 */
#define SC_P_LVDS1_GPIO00                        56	/* LVDS1.GPIO0.IO00, LVDS1.PWM0.OUT, LSIO.GPIO1.IO10 */
#define SC_P_LVDS1_GPIO01                        57	/* LVDS1.GPIO0.IO01, LSIO.GPIO1.IO11 */
#define SC_P_LVDS1_I2C0_SCL                      58	/* LVDS1.I2C0.SCL, LVDS1.GPIO0.IO02, LSIO.GPIO1.IO12 */
#define SC_P_LVDS1_I2C0_SDA                      59	/* LVDS1.I2C0.SDA, LVDS1.GPIO0.IO03, LSIO.GPIO1.IO13 */
#define SC_P_LVDS1_I2C1_SCL                      60	/* LVDS1.I2C1.SCL, DMA.UART3.TX, LSIO.GPIO1.IO14 */
#define SC_P_LVDS1_I2C1_SDA                      61	/* LVDS1.I2C1.SDA, DMA.UART3.RX, LSIO.GPIO1.IO15 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_LVDSGPIO      62	/*  */
#define SC_P_MIPI_DSI0_I2C0_SCL                  63	/* MIPI_DSI0.I2C0.SCL, LSIO.GPIO1.IO16 */
#define SC_P_MIPI_DSI0_I2C0_SDA                  64	/* MIPI_DSI0.I2C0.SDA, LSIO.GPIO1.IO17 */
#define SC_P_MIPI_DSI0_GPIO0_00                  65	/* MIPI_DSI0.GPIO0.IO00, MIPI_DSI0.PWM0.OUT, LSIO.GPIO1.IO18 */
#define SC_P_MIPI_DSI0_GPIO0_01                  66	/* MIPI_DSI0.GPIO0.IO01, LSIO.GPIO1.IO19 */
#define SC_P_MIPI_DSI1_I2C0_SCL                  67	/* MIPI_DSI1.I2C0.SCL, LSIO.GPIO1.IO20 */
#define SC_P_MIPI_DSI1_I2C0_SDA                  68	/* MIPI_DSI1.I2C0.SDA, LSIO.GPIO1.IO21 */
#define SC_P_MIPI_DSI1_GPIO0_00                  69	/* MIPI_DSI1.GPIO0.IO00, MIPI_DSI1.PWM0.OUT, LSIO.GPIO1.IO22 */
#define SC_P_MIPI_DSI1_GPIO0_01                  70	/* MIPI_DSI1.GPIO0.IO01, LSIO.GPIO1.IO23 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_MIPIDSIGPIO   71	/*  */
#define SC_P_MIPI_CSI0_MCLK_OUT                  72	/* MIPI_CSI0.ACM.MCLK_OUT, LSIO.GPIO1.IO24 */
#define SC_P_MIPI_CSI0_I2C0_SCL                  73	/* MIPI_CSI0.I2C0.SCL, LSIO.GPIO1.IO25 */
#define SC_P_MIPI_CSI0_I2C0_SDA                  74	/* MIPI_CSI0.I2C0.SDA, LSIO.GPIO1.IO26 */
#define SC_P_MIPI_CSI0_GPIO0_00                  75	/* MIPI_CSI0.GPIO0.IO00, DMA.I2C0.SCL, MIPI_CSI1.I2C0.SCL, LSIO.GPIO1.IO27 */
#define SC_P_MIPI_CSI0_GPIO0_01                  76	/* MIPI_CSI0.GPIO0.IO01, DMA.I2C0.SDA, MIPI_CSI1.I2C0.SDA, LSIO.GPIO1.IO28 */
#define SC_P_MIPI_CSI1_MCLK_OUT                  77	/* MIPI_CSI1.ACM.MCLK_OUT, LSIO.GPIO1.IO29 */
#define SC_P_MIPI_CSI1_GPIO0_00                  78	/* MIPI_CSI1.GPIO0.IO00, DMA.UART4.RX, LSIO.GPIO1.IO30 */
#define SC_P_MIPI_CSI1_GPIO0_01                  79	/* MIPI_CSI1.GPIO0.IO01, DMA.UART4.TX, LSIO.GPIO1.IO31 */
#define SC_P_MIPI_CSI1_I2C0_SCL                  80	/* MIPI_CSI1.I2C0.SCL, LSIO.GPIO2.IO00 */
#define SC_P_MIPI_CSI1_I2C0_SDA                  81	/* MIPI_CSI1.I2C0.SDA, LSIO.GPIO2.IO01 */
#define SC_P_HDMI_TX0_TS_SCL                     82	/* HDMI_TX0.I2C0.SCL, DMA.I2C0.SCL, LSIO.GPIO2.IO02 */
#define SC_P_HDMI_TX0_TS_SDA                     83	/* HDMI_TX0.I2C0.SDA, DMA.I2C0.SDA, LSIO.GPIO2.IO03 */
#define SC_P_COMP_CTL_GPIO_3V3_HDMIGPIO          84	/*  */
#define SC_P_ESAI1_FSR                           85	/* AUD.ESAI1.FSR, LSIO.GPIO2.IO04 */
#define SC_P_ESAI1_FST                           86	/* AUD.ESAI1.FST, AUD.SPDIF0.EXT_CLK, LSIO.GPIO2.IO05 */
#define SC_P_ESAI1_SCKR                          87	/* AUD.ESAI1.SCKR, LSIO.GPIO2.IO06 */
#define SC_P_ESAI1_SCKT                          88	/* AUD.ESAI1.SCKT, AUD.SAI2.RXC, AUD.SPDIF0.EXT_CLK, LSIO.GPIO2.IO07 */
#define SC_P_ESAI1_TX0                           89	/* AUD.ESAI1.TX0, AUD.SAI2.RXD, AUD.SPDIF0.RX, LSIO.GPIO2.IO08 */
#define SC_P_ESAI1_TX1                           90	/* AUD.ESAI1.TX1, AUD.SAI2.RXFS, AUD.SPDIF0.TX, LSIO.GPIO2.IO09 */
#define SC_P_ESAI1_TX2_RX3                       91	/* AUD.ESAI1.TX2_RX3, AUD.SPDIF0.RX, LSIO.GPIO2.IO10 */
#define SC_P_ESAI1_TX3_RX2                       92	/* AUD.ESAI1.TX3_RX2, AUD.SPDIF0.TX, LSIO.GPIO2.IO11 */
#define SC_P_ESAI1_TX4_RX1                       93	/* AUD.ESAI1.TX4_RX1, LSIO.GPIO2.IO12 */
#define SC_P_ESAI1_TX5_RX0                       94	/* AUD.ESAI1.TX5_RX0, LSIO.GPIO2.IO13 */
#define SC_P_SPDIF0_RX                           95	/* AUD.SPDIF0.RX, AUD.MQS.R, AUD.ACM.MCLK_IN1, LSIO.GPIO2.IO14 */
#define SC_P_SPDIF0_TX                           96	/* AUD.SPDIF0.TX, AUD.MQS.L, AUD.ACM.MCLK_OUT1, LSIO.GPIO2.IO15 */
#define SC_P_SPDIF0_EXT_CLK                      97	/* AUD.SPDIF0.EXT_CLK, DMA.DMA0.REQ_IN0, LSIO.GPIO2.IO16 */
#define SC_P_SPI3_SCK                            98	/* DMA.SPI3.SCK, LSIO.GPIO2.IO17 */
#define SC_P_SPI3_SDO                            99	/* DMA.SPI3.SDO, DMA.FTM.CH0, LSIO.GPIO2.IO18 */
#define SC_P_SPI3_SDI                            100	/* DMA.SPI3.SDI, DMA.FTM.CH1, LSIO.GPIO2.IO19 */
#define SC_P_SPI3_CS0                            101	/* DMA.SPI3.CS0, DMA.FTM.CH2, LSIO.GPIO2.IO20 */
#define SC_P_SPI3_CS1                            102	/* DMA.SPI3.CS1, LSIO.GPIO2.IO21 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_GPIORHB       103	/*  */
#define SC_P_ESAI0_FSR                           104	/* AUD.ESAI0.FSR, LSIO.GPIO2.IO22 */
#define SC_P_ESAI0_FST                           105	/* AUD.ESAI0.FST, LSIO.GPIO2.IO23 */
#define SC_P_ESAI0_SCKR                          106	/* AUD.ESAI0.SCKR, LSIO.GPIO2.IO24 */
#define SC_P_ESAI0_SCKT                          107	/* AUD.ESAI0.SCKT, LSIO.GPIO2.IO25 */
#define SC_P_ESAI0_TX0                           108	/* AUD.ESAI0.TX0, LSIO.GPIO2.IO26 */
#define SC_P_ESAI0_TX1                           109	/* AUD.ESAI0.TX1, LSIO.GPIO2.IO27 */
#define SC_P_ESAI0_TX2_RX3                       110	/* AUD.ESAI0.TX2_RX3, LSIO.GPIO2.IO28 */
#define SC_P_ESAI0_TX3_RX2                       111	/* AUD.ESAI0.TX3_RX2, LSIO.GPIO2.IO29 */
#define SC_P_ESAI0_TX4_RX1                       112	/* AUD.ESAI0.TX4_RX1, LSIO.GPIO2.IO30 */
#define SC_P_ESAI0_TX5_RX0                       113	/* AUD.ESAI0.TX5_RX0, LSIO.GPIO2.IO31 */
#define SC_P_MCLK_IN0                            114	/* AUD.ACM.MCLK_IN0, AUD.ESAI0.RX_HF_CLK, AUD.ESAI1.RX_HF_CLK, LSIO.GPIO3.IO00 */
#define SC_P_MCLK_OUT0                           115	/* AUD.ACM.MCLK_OUT0, AUD.ESAI0.TX_HF_CLK, AUD.ESAI1.TX_HF_CLK, LSIO.GPIO3.IO01 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_GPIORHC       116	/*  */
#define SC_P_SPI0_SCK                            117	/* DMA.SPI0.SCK, AUD.SAI0.RXC, LSIO.GPIO3.IO02 */
#define SC_P_SPI0_SDO                            118	/* DMA.SPI0.SDO, AUD.SAI0.TXD, LSIO.GPIO3.IO03 */
#define SC_P_SPI0_SDI                            119	/* DMA.SPI0.SDI, AUD.SAI0.RXD, LSIO.GPIO3.IO04 */
#define SC_P_SPI0_CS0                            120	/* DMA.SPI0.CS0, AUD.SAI0.RXFS, LSIO.GPIO3.IO05 */
#define SC_P_SPI0_CS1                            121	/* DMA.SPI0.CS1, AUD.SAI0.TXC, LSIO.GPIO3.IO06 */
#define SC_P_SPI2_SCK                            122	/* DMA.SPI2.SCK, LSIO.GPIO3.IO07 */
#define SC_P_SPI2_SDO                            123	/* DMA.SPI2.SDO, LSIO.GPIO3.IO08 */
#define SC_P_SPI2_SDI                            124	/* DMA.SPI2.SDI, LSIO.GPIO3.IO09 */
#define SC_P_SPI2_CS0                            125	/* DMA.SPI2.CS0, LSIO.GPIO3.IO10 */
#define SC_P_SPI2_CS1                            126	/* DMA.SPI2.CS1, AUD.SAI0.TXFS, LSIO.GPIO3.IO11 */
#define SC_P_SAI1_RXC                            127	/* AUD.SAI1.RXC, AUD.SAI0.TXD, LSIO.GPIO3.IO12 */
#define SC_P_SAI1_RXD                            128	/* AUD.SAI1.RXD, AUD.SAI0.TXFS, LSIO.GPIO3.IO13 */
#define SC_P_SAI1_RXFS                           129	/* AUD.SAI1.RXFS, AUD.SAI0.RXD, LSIO.GPIO3.IO14 */
#define SC_P_SAI1_TXC                            130	/* AUD.SAI1.TXC, AUD.SAI0.TXC, LSIO.GPIO3.IO15 */
#define SC_P_SAI1_TXD                            131	/* AUD.SAI1.TXD, AUD.SAI1.RXC, LSIO.GPIO3.IO16 */
#define SC_P_SAI1_TXFS                           132	/* AUD.SAI1.TXFS, AUD.SAI1.RXFS, LSIO.GPIO3.IO17 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_GPIORHT       133	/*  */
#define SC_P_ADC_IN7                             134	/* DMA.ADC1.IN3, DMA.SPI1.CS1, LSIO.KPP0.ROW3, LSIO.GPIO3.IO25 */
#define SC_P_ADC_IN6                             135	/* DMA.ADC1.IN2, DMA.SPI1.CS0, LSIO.KPP0.ROW2, LSIO.GPIO3.IO24 */
#define SC_P_ADC_IN5                             136	/* DMA.ADC1.IN1, DMA.SPI1.SDI, LSIO.KPP0.ROW1, LSIO.GPIO3.IO23 */
#define SC_P_ADC_IN4                             137	/* DMA.ADC1.IN0, DMA.SPI1.SDO, LSIO.KPP0.ROW0, LSIO.GPIO3.IO22 */
#define SC_P_ADC_IN3                             138	/* DMA.ADC0.IN3, DMA.SPI1.SCK, LSIO.KPP0.COL3, LSIO.GPIO3.IO21 */
#define SC_P_ADC_IN2                             139	/* DMA.ADC0.IN2, LSIO.KPP0.COL2, LSIO.GPIO3.IO20 */
#define SC_P_ADC_IN1                             140	/* DMA.ADC0.IN1, LSIO.KPP0.COL1, LSIO.GPIO3.IO19 */
#define SC_P_ADC_IN0                             141	/* DMA.ADC0.IN0, LSIO.KPP0.COL0, LSIO.GPIO3.IO18 */
#define SC_P_MLB_SIG                             142	/* CONN.MLB.SIG, AUD.SAI3.RXC, LSIO.GPIO3.IO26 */
#define SC_P_MLB_CLK                             143	/* CONN.MLB.CLK, AUD.SAI3.RXFS, LSIO.GPIO3.IO27 */
#define SC_P_MLB_DATA                            144	/* CONN.MLB.DATA, AUD.SAI3.RXD, LSIO.GPIO3.IO28 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_GPIOLHT       145	/*  */
#define SC_P_FLEXCAN0_RX                         146	/* DMA.FLEXCAN0.RX, LSIO.GPIO3.IO29 */
#define SC_P_FLEXCAN0_TX                         147	/* DMA.FLEXCAN0.TX, LSIO.GPIO3.IO30 */
#define SC_P_FLEXCAN1_RX                         148	/* DMA.FLEXCAN1.RX, LSIO.GPIO3.IO31 */
#define SC_P_FLEXCAN1_TX                         149	/* DMA.FLEXCAN1.TX, LSIO.GPIO4.IO00 */
#define SC_P_FLEXCAN2_RX                         150	/* DMA.FLEXCAN2.RX, LSIO.GPIO4.IO01 */
#define SC_P_FLEXCAN2_TX                         151	/* DMA.FLEXCAN2.TX, LSIO.GPIO4.IO02 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_GPIOTHR       152	/*  */
#define SC_P_USB_SS3_TC0                         153	/* DMA.I2C1.SCL, CONN.USB_OTG1.PWR, LSIO.GPIO4.IO03 */
#define SC_P_USB_SS3_TC1                         154	/* DMA.I2C1.SCL, CONN.USB_OTG2.PWR, LSIO.GPIO4.IO04 */
#define SC_P_USB_SS3_TC2                         155	/* DMA.I2C1.SDA, CONN.USB_OTG1.OC, LSIO.GPIO4.IO05 */
#define SC_P_USB_SS3_TC3                         156	/* DMA.I2C1.SDA, CONN.USB_OTG2.OC, LSIO.GPIO4.IO06 */
#define SC_P_COMP_CTL_GPIO_3V3_USB3IO            157	/*  */
#define SC_P_USDHC1_RESET_B                      158	/* CONN.USDHC1.RESET_B, LSIO.GPIO4.IO07 */
#define SC_P_USDHC1_VSELECT                      159	/* CONN.USDHC1.VSELECT, LSIO.GPIO4.IO08 */
#define SC_P_USDHC2_RESET_B                      160	/* CONN.USDHC2.RESET_B, LSIO.GPIO4.IO09 */
#define SC_P_USDHC2_VSELECT                      161	/* CONN.USDHC2.VSELECT, LSIO.GPIO4.IO10 */
#define SC_P_USDHC2_WP                           162	/* CONN.USDHC2.WP, LSIO.GPIO4.IO11 */
#define SC_P_USDHC2_CD_B                         163	/* CONN.USDHC2.CD_B, LSIO.GPIO4.IO12 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_VSELSEP       164	/*  */
#define SC_P_ENET0_MDIO                          165	/* CONN.ENET0.MDIO, DMA.I2C4.SDA, LSIO.GPIO4.IO13 */
#define SC_P_ENET0_MDC                           166	/* CONN.ENET0.MDC, DMA.I2C4.SCL, LSIO.GPIO4.IO14 */
#define SC_P_ENET0_REFCLK_125M_25M               167	/* CONN.ENET0.REFCLK_125M_25M, CONN.ENET0.PPS, LSIO.GPIO4.IO15 */
#define SC_P_ENET1_REFCLK_125M_25M               168	/* CONN.ENET1.REFCLK_125M_25M, CONN.ENET1.PPS, LSIO.GPIO4.IO16 */
#define SC_P_ENET1_MDIO                          169	/* CONN.ENET1.MDIO, DMA.I2C4.SDA, LSIO.GPIO4.IO17 */
#define SC_P_ENET1_MDC                           170	/* CONN.ENET1.MDC, DMA.I2C4.SCL, LSIO.GPIO4.IO18 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_GPIOCT        171	/*  */
#define SC_P_QSPI1A_SS0_B                        172	/* LSIO.QSPI1A.SS0_B, LSIO.GPIO4.IO19 */
#define SC_P_QSPI1A_SS1_B                        173	/* LSIO.QSPI1A.SS1_B, LSIO.QSPI1A.SCLK2, LSIO.GPIO4.IO20 */
#define SC_P_QSPI1A_SCLK                         174	/* LSIO.QSPI1A.SCLK, LSIO.GPIO4.IO21 */
#define SC_P_QSPI1A_DQS                          175	/* LSIO.QSPI1A.DQS, LSIO.GPIO4.IO22 */
#define SC_P_QSPI1A_DATA3                        176	/* LSIO.QSPI1A.DATA3, DMA.I2C1.SDA, CONN.USB_OTG1.OC, LSIO.GPIO4.IO23 */
#define SC_P_QSPI1A_DATA2                        177	/* LSIO.QSPI1A.DATA2, DMA.I2C1.SCL, CONN.USB_OTG2.PWR, LSIO.GPIO4.IO24 */
#define SC_P_QSPI1A_DATA1                        178	/* LSIO.QSPI1A.DATA1, DMA.I2C1.SDA, CONN.USB_OTG2.OC, LSIO.GPIO4.IO25 */
#define SC_P_QSPI1A_DATA0                        179	/* LSIO.QSPI1A.DATA0, LSIO.GPIO4.IO26 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_QSPI1         180	/*  */
#define SC_P_QSPI0A_DATA0                        181	/* LSIO.QSPI0A.DATA0 */
#define SC_P_QSPI0A_DATA1                        182	/* LSIO.QSPI0A.DATA1 */
#define SC_P_QSPI0A_DATA2                        183	/* LSIO.QSPI0A.DATA2 */
#define SC_P_QSPI0A_DATA3                        184	/* LSIO.QSPI0A.DATA3 */
#define SC_P_QSPI0A_DQS                          185	/* LSIO.QSPI0A.DQS */
#define SC_P_QSPI0A_SS0_B                        186	/* LSIO.QSPI0A.SS0_B */
#define SC_P_QSPI0A_SS1_B                        187	/* LSIO.QSPI0A.SS1_B, LSIO.QSPI0A.SCLK2 */
#define SC_P_QSPI0A_SCLK                         188	/* LSIO.QSPI0A.SCLK */
#define SC_P_QSPI0B_SCLK                         189	/* LSIO.QSPI0B.SCLK */
#define SC_P_QSPI0B_DATA0                        190	/* LSIO.QSPI0B.DATA0 */
#define SC_P_QSPI0B_DATA1                        191	/* LSIO.QSPI0B.DATA1 */
#define SC_P_QSPI0B_DATA2                        192	/* LSIO.QSPI0B.DATA2 */
#define SC_P_QSPI0B_DATA3                        193	/* LSIO.QSPI0B.DATA3 */
#define SC_P_QSPI0B_DQS                          194	/* LSIO.QSPI0B.DQS */
#define SC_P_QSPI0B_SS0_B                        195	/* LSIO.QSPI0B.SS0_B */
#define SC_P_QSPI0B_SS1_B                        196	/* LSIO.QSPI0B.SS1_B, LSIO.QSPI0B.SCLK2 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_QSPI0         197	/*  */
#define SC_P_PCIE_CTRL0_CLKREQ_B                 198	/* HSIO.PCIE0.CLKREQ_B, LSIO.GPIO4.IO27 */
#define SC_P_PCIE_CTRL0_WAKE_B                   199	/* HSIO.PCIE0.WAKE_B, LSIO.GPIO4.IO28 */
#define SC_P_PCIE_CTRL0_PERST_B                  200	/* HSIO.PCIE0.PERST_B, LSIO.GPIO4.IO29 */
#define SC_P_PCIE_CTRL1_CLKREQ_B                 201	/* HSIO.PCIE1.CLKREQ_B, DMA.I2C1.SDA, CONN.USB_OTG2.OC, LSIO.GPIO4.IO30 */
#define SC_P_PCIE_CTRL1_WAKE_B                   202	/* HSIO.PCIE1.WAKE_B, DMA.I2C1.SCL, CONN.USB_OTG2.PWR, LSIO.GPIO4.IO31 */
#define SC_P_PCIE_CTRL1_PERST_B                  203	/* HSIO.PCIE1.PERST_B, DMA.I2C1.SCL, CONN.USB_OTG1.PWR, LSIO.GPIO5.IO00 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_PCIESEP       204	/*  */
#define SC_P_USB_HSIC0_DATA                      205	/* CONN.USB_HSIC0.DATA, DMA.I2C1.SDA, LSIO.GPIO5.IO01 */
#define SC_P_USB_HSIC0_STROBE                    206	/* CONN.USB_HSIC0.STROBE, DMA.I2C1.SCL, LSIO.GPIO5.IO02 */
#define SC_P_CALIBRATION_0_HSIC                  207	/*  */
#define SC_P_CALIBRATION_1_HSIC                  208	/*  */
#define SC_P_EMMC0_CLK                           209	/* CONN.EMMC0.CLK, CONN.NAND.READY_B */
#define SC_P_EMMC0_CMD                           210	/* CONN.EMMC0.CMD, CONN.NAND.DQS, AUD.MQS.R, LSIO.GPIO5.IO03 */
#define SC_P_EMMC0_DATA0                         211	/* CONN.EMMC0.DATA0, CONN.NAND.DATA00, LSIO.GPIO5.IO04 */
#define SC_P_EMMC0_DATA1                         212	/* CONN.EMMC0.DATA1, CONN.NAND.DATA01, LSIO.GPIO5.IO05 */
#define SC_P_EMMC0_DATA2                         213	/* CONN.EMMC0.DATA2, CONN.NAND.DATA02, LSIO.GPIO5.IO06 */
#define SC_P_EMMC0_DATA3                         214	/* CONN.EMMC0.DATA3, CONN.NAND.DATA03, LSIO.GPIO5.IO07 */
#define SC_P_EMMC0_DATA4                         215	/* CONN.EMMC0.DATA4, CONN.NAND.DATA04, LSIO.GPIO5.IO08 */
#define SC_P_EMMC0_DATA5                         216	/* CONN.EMMC0.DATA5, CONN.NAND.DATA05, LSIO.GPIO5.IO09 */
#define SC_P_EMMC0_DATA6                         217	/* CONN.EMMC0.DATA6, CONN.NAND.DATA06, LSIO.GPIO5.IO10 */
#define SC_P_EMMC0_DATA7                         218	/* CONN.EMMC0.DATA7, CONN.NAND.DATA07, LSIO.GPIO5.IO11 */
#define SC_P_EMMC0_STROBE                        219	/* CONN.EMMC0.STROBE, CONN.NAND.CLE, LSIO.GPIO5.IO12 */
#define SC_P_EMMC0_RESET_B                       220	/* CONN.EMMC0.RESET_B, CONN.NAND.WP_B, CONN.USDHC1.VSELECT, LSIO.GPIO5.IO13 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_SD1FIX        221	/*  */
#define SC_P_USDHC1_CLK                          222	/* CONN.USDHC1.CLK, AUD.MQS.R */
#define SC_P_USDHC1_CMD                          223	/* CONN.USDHC1.CMD, AUD.MQS.L, LSIO.GPIO5.IO14 */
#define SC_P_USDHC1_DATA0                        224	/* CONN.USDHC1.DATA0, CONN.NAND.RE_N, LSIO.GPIO5.IO15 */
#define SC_P_USDHC1_DATA1                        225	/* CONN.USDHC1.DATA1, CONN.NAND.RE_P, LSIO.GPIO5.IO16 */
#define SC_P_CTL_NAND_RE_P_N                     226	/*  */
#define SC_P_USDHC1_DATA2                        227	/* CONN.USDHC1.DATA2, CONN.NAND.DQS_N, LSIO.GPIO5.IO17 */
#define SC_P_USDHC1_DATA3                        228	/* CONN.USDHC1.DATA3, CONN.NAND.DQS_P, LSIO.GPIO5.IO18 */
#define SC_P_CTL_NAND_DQS_P_N                    229	/*  */
#define SC_P_USDHC1_DATA4                        230	/* CONN.USDHC1.DATA4, CONN.NAND.CE0_B, AUD.MQS.R, LSIO.GPIO5.IO19 */
#define SC_P_USDHC1_DATA5                        231	/* CONN.USDHC1.DATA5, CONN.NAND.RE_B, AUD.MQS.L, LSIO.GPIO5.IO20 */
#define SC_P_USDHC1_DATA6                        232	/* CONN.USDHC1.DATA6, CONN.NAND.WE_B, CONN.USDHC1.WP, LSIO.GPIO5.IO21 */
#define SC_P_USDHC1_DATA7                        233	/* CONN.USDHC1.DATA7, CONN.NAND.ALE, CONN.USDHC1.CD_B, LSIO.GPIO5.IO22 */
#define SC_P_USDHC1_STROBE                       234	/* CONN.USDHC1.STROBE, CONN.NAND.CE1_B, CONN.USDHC1.RESET_B, LSIO.GPIO5.IO23 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_VSEL2         235	/*  */
#define SC_P_USDHC2_CLK                          236	/* CONN.USDHC2.CLK, AUD.MQS.R, LSIO.GPIO5.IO24 */
#define SC_P_USDHC2_CMD                          237	/* CONN.USDHC2.CMD, AUD.MQS.L, LSIO.GPIO5.IO25 */
#define SC_P_USDHC2_DATA0                        238	/* CONN.USDHC2.DATA0, DMA.UART4.RX, LSIO.GPIO5.IO26 */
#define SC_P_USDHC2_DATA1                        239	/* CONN.USDHC2.DATA1, DMA.UART4.TX, LSIO.GPIO5.IO27 */
#define SC_P_USDHC2_DATA2                        240	/* CONN.USDHC2.DATA2, DMA.UART4.CTS_B, LSIO.GPIO5.IO28 */
#define SC_P_USDHC2_DATA3                        241	/* CONN.USDHC2.DATA3, DMA.UART4.RTS_B, LSIO.GPIO5.IO29 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_VSEL3         242	/*  */
#define SC_P_ENET0_RGMII_TXC                     243	/* CONN.ENET0.RGMII_TXC, CONN.ENET0.RCLK50M_OUT, CONN.ENET0.RCLK50M_IN, LSIO.GPIO5.IO30 */
#define SC_P_ENET0_RGMII_TX_CTL                  244	/* CONN.ENET0.RGMII_TX_CTL, LSIO.GPIO5.IO31 */
#define SC_P_ENET0_RGMII_TXD0                    245	/* CONN.ENET0.RGMII_TXD0, LSIO.GPIO6.IO00 */
#define SC_P_ENET0_RGMII_TXD1                    246	/* CONN.ENET0.RGMII_TXD1, LSIO.GPIO6.IO01 */
#define SC_P_ENET0_RGMII_TXD2                    247	/* CONN.ENET0.RGMII_TXD2, DMA.UART3.TX, VPU.TSI_S1.VID, LSIO.GPIO6.IO02 */
#define SC_P_ENET0_RGMII_TXD3                    248	/* CONN.ENET0.RGMII_TXD3, DMA.UART3.RTS_B, VPU.TSI_S1.SYNC, LSIO.GPIO6.IO03 */
#define SC_P_ENET0_RGMII_RXC                     249	/* CONN.ENET0.RGMII_RXC, DMA.UART3.CTS_B, VPU.TSI_S1.DATA, LSIO.GPIO6.IO04 */
#define SC_P_ENET0_RGMII_RX_CTL                  250	/* CONN.ENET0.RGMII_RX_CTL, VPU.TSI_S0.VID, LSIO.GPIO6.IO05 */
#define SC_P_ENET0_RGMII_RXD0                    251	/* CONN.ENET0.RGMII_RXD0, VPU.TSI_S0.SYNC, LSIO.GPIO6.IO06 */
#define SC_P_ENET0_RGMII_RXD1                    252	/* CONN.ENET0.RGMII_RXD1, VPU.TSI_S0.DATA, LSIO.GPIO6.IO07 */
#define SC_P_ENET0_RGMII_RXD2                    253	/* CONN.ENET0.RGMII_RXD2, CONN.ENET0.RMII_RX_ER, VPU.TSI_S0.CLK, LSIO.GPIO6.IO08 */
#define SC_P_ENET0_RGMII_RXD3                    254	/* CONN.ENET0.RGMII_RXD3, DMA.UART3.RX, VPU.TSI_S1.CLK, LSIO.GPIO6.IO09 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_ENET_ENETB    255	/*  */
#define SC_P_ENET1_RGMII_TXC                     256	/* CONN.ENET1.RGMII_TXC, CONN.ENET1.RCLK50M_OUT, CONN.ENET1.RCLK50M_IN, LSIO.GPIO6.IO10 */
#define SC_P_ENET1_RGMII_TX_CTL                  257	/* CONN.ENET1.RGMII_TX_CTL, LSIO.GPIO6.IO11 */
#define SC_P_ENET1_RGMII_TXD0                    258	/* CONN.ENET1.RGMII_TXD0, LSIO.GPIO6.IO12 */
#define SC_P_ENET1_RGMII_TXD1                    259	/* CONN.ENET1.RGMII_TXD1, LSIO.GPIO6.IO13 */
#define SC_P_ENET1_RGMII_TXD2                    260	/* CONN.ENET1.RGMII_TXD2, DMA.UART3.TX, VPU.TSI_S1.VID, LSIO.GPIO6.IO14 */
#define SC_P_ENET1_RGMII_TXD3                    261	/* CONN.ENET1.RGMII_TXD3, DMA.UART3.RTS_B, VPU.TSI_S1.SYNC, LSIO.GPIO6.IO15 */
#define SC_P_ENET1_RGMII_RXC                     262	/* CONN.ENET1.RGMII_RXC, DMA.UART3.CTS_B, VPU.TSI_S1.DATA, LSIO.GPIO6.IO16 */
#define SC_P_ENET1_RGMII_RX_CTL                  263	/* CONN.ENET1.RGMII_RX_CTL, VPU.TSI_S0.VID, LSIO.GPIO6.IO17 */
#define SC_P_ENET1_RGMII_RXD0                    264	/* CONN.ENET1.RGMII_RXD0, VPU.TSI_S0.SYNC, LSIO.GPIO6.IO18 */
#define SC_P_ENET1_RGMII_RXD1                    265	/* CONN.ENET1.RGMII_RXD1, VPU.TSI_S0.DATA, LSIO.GPIO6.IO19 */
#define SC_P_ENET1_RGMII_RXD2                    266	/* CONN.ENET1.RGMII_RXD2, CONN.ENET1.RMII_RX_ER, VPU.TSI_S0.CLK, LSIO.GPIO6.IO20 */
#define SC_P_ENET1_RGMII_RXD3                    267	/* CONN.ENET1.RGMII_RXD3, DMA.UART3.RX, VPU.TSI_S1.CLK, LSIO.GPIO6.IO21 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_ENET_ENETA    268	/*  */
/*@}*/

/*!
 * @name Pad Mux Definitions
 * format: name padid padmux
 */
/*@{*/
#define SC_P_SIM0_CLK_DMA_SIM0_CLK                              SC_P_SIM0_CLK                      0
#define SC_P_SIM0_CLK_LSIO_GPIO0_IO00                           SC_P_SIM0_CLK                      3
#define SC_P_SIM0_RST_DMA_SIM0_RST                              SC_P_SIM0_RST                      0
#define SC_P_SIM0_RST_LSIO_GPIO0_IO01                           SC_P_SIM0_RST                      3
#define SC_P_SIM0_IO_DMA_SIM0_IO                                SC_P_SIM0_IO                       0
#define SC_P_SIM0_IO_LSIO_GPIO0_IO02                            SC_P_SIM0_IO                       3
#define SC_P_SIM0_PD_DMA_SIM0_PD                                SC_P_SIM0_PD                       0
#define SC_P_SIM0_PD_DMA_I2C3_SCL                               SC_P_SIM0_PD                       1
#define SC_P_SIM0_PD_LSIO_GPIO0_IO03                            SC_P_SIM0_PD                       3
#define SC_P_SIM0_POWER_EN_DMA_SIM0_POWER_EN                    SC_P_SIM0_POWER_EN                 0
#define SC_P_SIM0_POWER_EN_DMA_I2C3_SDA                         SC_P_SIM0_POWER_EN                 1
#define SC_P_SIM0_POWER_EN_LSIO_GPIO0_IO04                      SC_P_SIM0_POWER_EN                 3
#define SC_P_SIM0_GPIO0_00_DMA_SIM0_POWER_EN                    SC_P_SIM0_GPIO0_00                 0
#define SC_P_SIM0_GPIO0_00_LSIO_GPIO0_IO05                      SC_P_SIM0_GPIO0_00                 3
#define SC_P_M40_I2C0_SCL_M40_I2C0_SCL                          SC_P_M40_I2C0_SCL                  0
#define SC_P_M40_I2C0_SCL_M40_UART0_RX                          SC_P_M40_I2C0_SCL                  1
#define SC_P_M40_I2C0_SCL_M40_GPIO0_IO02                        SC_P_M40_I2C0_SCL                  2
#define SC_P_M40_I2C0_SCL_LSIO_GPIO0_IO06                       SC_P_M40_I2C0_SCL                  3
#define SC_P_M40_I2C0_SDA_M40_I2C0_SDA                          SC_P_M40_I2C0_SDA                  0
#define SC_P_M40_I2C0_SDA_M40_UART0_TX                          SC_P_M40_I2C0_SDA                  1
#define SC_P_M40_I2C0_SDA_M40_GPIO0_IO03                        SC_P_M40_I2C0_SDA                  2
#define SC_P_M40_I2C0_SDA_LSIO_GPIO0_IO07                       SC_P_M40_I2C0_SDA                  3
#define SC_P_M40_GPIO0_00_M40_GPIO0_IO00                        SC_P_M40_GPIO0_00                  0
#define SC_P_M40_GPIO0_00_M40_TPM0_CH0                          SC_P_M40_GPIO0_00                  1
#define SC_P_M40_GPIO0_00_DMA_UART4_RX                          SC_P_M40_GPIO0_00                  2
#define SC_P_M40_GPIO0_00_LSIO_GPIO0_IO08                       SC_P_M40_GPIO0_00                  3
#define SC_P_M40_GPIO0_01_M40_GPIO0_IO01                        SC_P_M40_GPIO0_01                  0
#define SC_P_M40_GPIO0_01_M40_TPM0_CH1                          SC_P_M40_GPIO0_01                  1
#define SC_P_M40_GPIO0_01_DMA_UART4_TX                          SC_P_M40_GPIO0_01                  2
#define SC_P_M40_GPIO0_01_LSIO_GPIO0_IO09                       SC_P_M40_GPIO0_01                  3
#define SC_P_M41_I2C0_SCL_M41_I2C0_SCL                          SC_P_M41_I2C0_SCL                  0
#define SC_P_M41_I2C0_SCL_M41_UART0_RX                          SC_P_M41_I2C0_SCL                  1
#define SC_P_M41_I2C0_SCL_M41_GPIO0_IO02                        SC_P_M41_I2C0_SCL                  2
#define SC_P_M41_I2C0_SCL_LSIO_GPIO0_IO10                       SC_P_M41_I2C0_SCL                  3
#define SC_P_M41_I2C0_SDA_M41_I2C0_SDA                          SC_P_M41_I2C0_SDA                  0
#define SC_P_M41_I2C0_SDA_M41_UART0_TX                          SC_P_M41_I2C0_SDA                  1
#define SC_P_M41_I2C0_SDA_M41_GPIO0_IO03                        SC_P_M41_I2C0_SDA                  2
#define SC_P_M41_I2C0_SDA_LSIO_GPIO0_IO11                       SC_P_M41_I2C0_SDA                  3
#define SC_P_M41_GPIO0_00_M41_GPIO0_IO00                        SC_P_M41_GPIO0_00                  0
#define SC_P_M41_GPIO0_00_M41_TPM0_CH0                          SC_P_M41_GPIO0_00                  1
#define SC_P_M41_GPIO0_00_DMA_UART3_RX                          SC_P_M41_GPIO0_00                  2
#define SC_P_M41_GPIO0_00_LSIO_GPIO0_IO12                       SC_P_M41_GPIO0_00                  3
#define SC_P_M41_GPIO0_01_M41_GPIO0_IO01                        SC_P_M41_GPIO0_01                  0
#define SC_P_M41_GPIO0_01_M41_TPM0_CH1                          SC_P_M41_GPIO0_01                  1
#define SC_P_M41_GPIO0_01_DMA_UART3_TX                          SC_P_M41_GPIO0_01                  2
#define SC_P_M41_GPIO0_01_LSIO_GPIO0_IO13                       SC_P_M41_GPIO0_01                  3
#define SC_P_GPT0_CLK_LSIO_GPT0_CLK                             SC_P_GPT0_CLK                      0
#define SC_P_GPT0_CLK_DMA_I2C1_SCL                              SC_P_GPT0_CLK                      1
#define SC_P_GPT0_CLK_LSIO_KPP0_COL4                            SC_P_GPT0_CLK                      2
#define SC_P_GPT0_CLK_LSIO_GPIO0_IO14                           SC_P_GPT0_CLK                      3
#define SC_P_GPT0_CAPTURE_LSIO_GPT0_CAPTURE                     SC_P_GPT0_CAPTURE                  0
#define SC_P_GPT0_CAPTURE_DMA_I2C1_SDA                          SC_P_GPT0_CAPTURE                  1
#define SC_P_GPT0_CAPTURE_LSIO_KPP0_COL5                        SC_P_GPT0_CAPTURE                  2
#define SC_P_GPT0_CAPTURE_LSIO_GPIO0_IO15                       SC_P_GPT0_CAPTURE                  3
#define SC_P_GPT0_COMPARE_LSIO_GPT0_COMPARE                     SC_P_GPT0_COMPARE                  0
#define SC_P_GPT0_COMPARE_LSIO_PWM3_OUT                         SC_P_GPT0_COMPARE                  1
#define SC_P_GPT0_COMPARE_LSIO_KPP0_COL6                        SC_P_GPT0_COMPARE                  2
#define SC_P_GPT0_COMPARE_LSIO_GPIO0_IO16                       SC_P_GPT0_COMPARE                  3
#define SC_P_GPT1_CLK_LSIO_GPT1_CLK                             SC_P_GPT1_CLK                      0
#define SC_P_GPT1_CLK_DMA_I2C2_SCL                              SC_P_GPT1_CLK                      1
#define SC_P_GPT1_CLK_LSIO_KPP0_COL7                            SC_P_GPT1_CLK                      2
#define SC_P_GPT1_CLK_LSIO_GPIO0_IO17                           SC_P_GPT1_CLK                      3
#define SC_P_GPT1_CAPTURE_LSIO_GPT1_CAPTURE                     SC_P_GPT1_CAPTURE                  0
#define SC_P_GPT1_CAPTURE_DMA_I2C2_SDA                          SC_P_GPT1_CAPTURE                  1
#define SC_P_GPT1_CAPTURE_LSIO_KPP0_ROW4                        SC_P_GPT1_CAPTURE                  2
#define SC_P_GPT1_CAPTURE_LSIO_GPIO0_IO18                       SC_P_GPT1_CAPTURE                  3
#define SC_P_GPT1_COMPARE_LSIO_GPT1_COMPARE                     SC_P_GPT1_COMPARE                  0
#define SC_P_GPT1_COMPARE_LSIO_PWM2_OUT                         SC_P_GPT1_COMPARE                  1
#define SC_P_GPT1_COMPARE_LSIO_KPP0_ROW5                        SC_P_GPT1_COMPARE                  2
#define SC_P_GPT1_COMPARE_LSIO_GPIO0_IO19                       SC_P_GPT1_COMPARE                  3
#define SC_P_UART0_RX_DMA_UART0_RX                              SC_P_UART0_RX                      0
#define SC_P_UART0_RX_SCU_UART0_RX                              SC_P_UART0_RX                      1
#define SC_P_UART0_RX_LSIO_GPIO0_IO20                           SC_P_UART0_RX                      3
#define SC_P_UART0_TX_DMA_UART0_TX                              SC_P_UART0_TX                      0
#define SC_P_UART0_TX_SCU_UART0_TX                              SC_P_UART0_TX                      1
#define SC_P_UART0_TX_LSIO_GPIO0_IO21                           SC_P_UART0_TX                      3
#define SC_P_UART0_RTS_B_DMA_UART0_RTS_B                        SC_P_UART0_RTS_B                   0
#define SC_P_UART0_RTS_B_LSIO_PWM0_OUT                          SC_P_UART0_RTS_B                   1
#define SC_P_UART0_RTS_B_DMA_UART2_RX                           SC_P_UART0_RTS_B                   2
#define SC_P_UART0_RTS_B_LSIO_GPIO0_IO22                        SC_P_UART0_RTS_B                   3
#define SC_P_UART0_CTS_B_DMA_UART0_CTS_B                        SC_P_UART0_CTS_B                   0
#define SC_P_UART0_CTS_B_LSIO_PWM1_OUT                          SC_P_UART0_CTS_B                   1
#define SC_P_UART0_CTS_B_DMA_UART2_TX                           SC_P_UART0_CTS_B                   2
#define SC_P_UART0_CTS_B_LSIO_GPIO0_IO23                        SC_P_UART0_CTS_B                   3
#define SC_P_UART1_TX_DMA_UART1_TX                              SC_P_UART1_TX                      0
#define SC_P_UART1_TX_DMA_SPI3_SCK                              SC_P_UART1_TX                      1
#define SC_P_UART1_TX_LSIO_GPIO0_IO24                           SC_P_UART1_TX                      3
#define SC_P_UART1_RX_DMA_UART1_RX                              SC_P_UART1_RX                      0
#define SC_P_UART1_RX_DMA_SPI3_SDO                              SC_P_UART1_RX                      1
#define SC_P_UART1_RX_LSIO_GPIO0_IO25                           SC_P_UART1_RX                      3
#define SC_P_UART1_RTS_B_DMA_UART1_RTS_B                        SC_P_UART1_RTS_B                   0
#define SC_P_UART1_RTS_B_DMA_SPI3_SDI                           SC_P_UART1_RTS_B                   1
#define SC_P_UART1_RTS_B_DMA_UART1_CTS_B                        SC_P_UART1_RTS_B                   2
#define SC_P_UART1_RTS_B_LSIO_GPIO0_IO26                        SC_P_UART1_RTS_B                   3
#define SC_P_UART1_CTS_B_DMA_UART1_CTS_B                        SC_P_UART1_CTS_B                   0
#define SC_P_UART1_CTS_B_DMA_SPI3_CS0                           SC_P_UART1_CTS_B                   1
#define SC_P_UART1_CTS_B_DMA_UART1_RTS_B                        SC_P_UART1_CTS_B                   2
#define SC_P_UART1_CTS_B_LSIO_GPIO0_IO27                        SC_P_UART1_CTS_B                   3
#define SC_P_SCU_PMIC_MEMC_ON_SCU_GPIO0_IOXX_PMIC_MEMC_ON       SC_P_SCU_PMIC_MEMC_ON              0
#define SC_P_SCU_WDOG_OUT_SCU_WDOG0_WDOG_OUT                    SC_P_SCU_WDOG_OUT                  0
#define SC_P_PMIC_I2C_SDA_SCU_PMIC_I2C_SDA                      SC_P_PMIC_I2C_SDA                  0
#define SC_P_PMIC_I2C_SCL_SCU_PMIC_I2C_SCL                      SC_P_PMIC_I2C_SCL                  0
#define SC_P_PMIC_EARLY_WARNING_SCU_PMIC_EARLY_WARNING          SC_P_PMIC_EARLY_WARNING            0
#define SC_P_PMIC_INT_B_SCU_DSC_PMIC_INT_B                      SC_P_PMIC_INT_B                    0
#define SC_P_SCU_GPIO0_00_SCU_GPIO0_IO00                        SC_P_SCU_GPIO0_00                  0
#define SC_P_SCU_GPIO0_00_SCU_UART0_RX                          SC_P_SCU_GPIO0_00                  1
#define SC_P_SCU_GPIO0_00_LSIO_GPIO0_IO28                       SC_P_SCU_GPIO0_00                  3
#define SC_P_SCU_GPIO0_01_SCU_GPIO0_IO01                        SC_P_SCU_GPIO0_01                  0
#define SC_P_SCU_GPIO0_01_SCU_UART0_TX                          SC_P_SCU_GPIO0_01                  1
#define SC_P_SCU_GPIO0_01_LSIO_GPIO0_IO29                       SC_P_SCU_GPIO0_01                  3
#define SC_P_SCU_GPIO0_02_SCU_GPIO0_IO02                        SC_P_SCU_GPIO0_02                  0
#define SC_P_SCU_GPIO0_02_SCU_GPIO0_IOXX_PMIC_GPU0_ON           SC_P_SCU_GPIO0_02                  1
#define SC_P_SCU_GPIO0_02_LSIO_GPIO0_IO30                       SC_P_SCU_GPIO0_02                  3
#define SC_P_SCU_GPIO0_03_SCU_GPIO0_IO03                        SC_P_SCU_GPIO0_03                  0
#define SC_P_SCU_GPIO0_03_SCU_GPIO0_IOXX_PMIC_GPU1_ON           SC_P_SCU_GPIO0_03                  1
#define SC_P_SCU_GPIO0_03_LSIO_GPIO0_IO31                       SC_P_SCU_GPIO0_03                  3
#define SC_P_SCU_GPIO0_04_SCU_GPIO0_IO04                        SC_P_SCU_GPIO0_04                  0
#define SC_P_SCU_GPIO0_04_SCU_GPIO0_IOXX_PMIC_A72_ON            SC_P_SCU_GPIO0_04                  1
#define SC_P_SCU_GPIO0_04_LSIO_GPIO1_IO00                       SC_P_SCU_GPIO0_04                  3
#define SC_P_SCU_GPIO0_05_SCU_GPIO0_IO05                        SC_P_SCU_GPIO0_05                  0
#define SC_P_SCU_GPIO0_05_SCU_GPIO0_IOXX_PMIC_A53_ON            SC_P_SCU_GPIO0_05                  1
#define SC_P_SCU_GPIO0_05_LSIO_GPIO1_IO01                       SC_P_SCU_GPIO0_05                  3
#define SC_P_SCU_GPIO0_06_SCU_GPIO0_IO06                        SC_P_SCU_GPIO0_06                  0
#define SC_P_SCU_GPIO0_06_SCU_TPM0_CH0                          SC_P_SCU_GPIO0_06                  1
#define SC_P_SCU_GPIO0_06_LSIO_GPIO1_IO02                       SC_P_SCU_GPIO0_06                  3
#define SC_P_SCU_GPIO0_07_SCU_GPIO0_IO07                        SC_P_SCU_GPIO0_07                  0
#define SC_P_SCU_GPIO0_07_SCU_TPM0_CH1                          SC_P_SCU_GPIO0_07                  1
#define SC_P_SCU_GPIO0_07_SCU_DSC_RTC_CLOCK_OUTPUT_32K          SC_P_SCU_GPIO0_07                  2
#define SC_P_SCU_GPIO0_07_LSIO_GPIO1_IO03                       SC_P_SCU_GPIO0_07                  3
#define SC_P_SCU_BOOT_MODE0_SCU_DSC_BOOT_MODE0                  SC_P_SCU_BOOT_MODE0                0
#define SC_P_SCU_BOOT_MODE1_SCU_DSC_BOOT_MODE1                  SC_P_SCU_BOOT_MODE1                0
#define SC_P_SCU_BOOT_MODE2_SCU_DSC_BOOT_MODE2                  SC_P_SCU_BOOT_MODE2                0
#define SC_P_SCU_BOOT_MODE3_SCU_DSC_BOOT_MODE3                  SC_P_SCU_BOOT_MODE3                0
#define SC_P_SCU_BOOT_MODE4_SCU_DSC_BOOT_MODE4                  SC_P_SCU_BOOT_MODE4                0
#define SC_P_SCU_BOOT_MODE4_SCU_PMIC_I2C_SCL                    SC_P_SCU_BOOT_MODE4                1
#define SC_P_SCU_BOOT_MODE5_SCU_DSC_BOOT_MODE5                  SC_P_SCU_BOOT_MODE5                0
#define SC_P_SCU_BOOT_MODE5_SCU_PMIC_I2C_SDA                    SC_P_SCU_BOOT_MODE5                1
#define SC_P_LVDS0_GPIO00_LVDS0_GPIO0_IO00                      SC_P_LVDS0_GPIO00                  0
#define SC_P_LVDS0_GPIO00_LVDS0_PWM0_OUT                        SC_P_LVDS0_GPIO00                  1
#define SC_P_LVDS0_GPIO00_LSIO_GPIO1_IO04                       SC_P_LVDS0_GPIO00                  3
#define SC_P_LVDS0_GPIO01_LVDS0_GPIO0_IO01                      SC_P_LVDS0_GPIO01                  0
#define SC_P_LVDS0_GPIO01_LSIO_GPIO1_IO05                       SC_P_LVDS0_GPIO01                  3
#define SC_P_LVDS0_I2C0_SCL_LVDS0_I2C0_SCL                      SC_P_LVDS0_I2C0_SCL                0
#define SC_P_LVDS0_I2C0_SCL_LVDS0_GPIO0_IO02                    SC_P_LVDS0_I2C0_SCL                1
#define SC_P_LVDS0_I2C0_SCL_LSIO_GPIO1_IO06                     SC_P_LVDS0_I2C0_SCL                3
#define SC_P_LVDS0_I2C0_SDA_LVDS0_I2C0_SDA                      SC_P_LVDS0_I2C0_SDA                0
#define SC_P_LVDS0_I2C0_SDA_LVDS0_GPIO0_IO03                    SC_P_LVDS0_I2C0_SDA                1
#define SC_P_LVDS0_I2C0_SDA_LSIO_GPIO1_IO07                     SC_P_LVDS0_I2C0_SDA                3
#define SC_P_LVDS0_I2C1_SCL_LVDS0_I2C1_SCL                      SC_P_LVDS0_I2C1_SCL                0
#define SC_P_LVDS0_I2C1_SCL_DMA_UART2_TX                        SC_P_LVDS0_I2C1_SCL                1
#define SC_P_LVDS0_I2C1_SCL_LSIO_GPIO1_IO08                     SC_P_LVDS0_I2C1_SCL                3
#define SC_P_LVDS0_I2C1_SDA_LVDS0_I2C1_SDA                      SC_P_LVDS0_I2C1_SDA                0
#define SC_P_LVDS0_I2C1_SDA_DMA_UART2_RX                        SC_P_LVDS0_I2C1_SDA                1
#define SC_P_LVDS0_I2C1_SDA_LSIO_GPIO1_IO09                     SC_P_LVDS0_I2C1_SDA                3
#define SC_P_LVDS1_GPIO00_LVDS1_GPIO0_IO00                      SC_P_LVDS1_GPIO00                  0
#define SC_P_LVDS1_GPIO00_LVDS1_PWM0_OUT                        SC_P_LVDS1_GPIO00                  1
#define SC_P_LVDS1_GPIO00_LSIO_GPIO1_IO10                       SC_P_LVDS1_GPIO00                  3
#define SC_P_LVDS1_GPIO01_LVDS1_GPIO0_IO01                      SC_P_LVDS1_GPIO01                  0
#define SC_P_LVDS1_GPIO01_LSIO_GPIO1_IO11                       SC_P_LVDS1_GPIO01                  3
#define SC_P_LVDS1_I2C0_SCL_LVDS1_I2C0_SCL                      SC_P_LVDS1_I2C0_SCL                0
#define SC_P_LVDS1_I2C0_SCL_LVDS1_GPIO0_IO02                    SC_P_LVDS1_I2C0_SCL                1
#define SC_P_LVDS1_I2C0_SCL_LSIO_GPIO1_IO12                     SC_P_LVDS1_I2C0_SCL                3
#define SC_P_LVDS1_I2C0_SDA_LVDS1_I2C0_SDA                      SC_P_LVDS1_I2C0_SDA                0
#define SC_P_LVDS1_I2C0_SDA_LVDS1_GPIO0_IO03                    SC_P_LVDS1_I2C0_SDA                1
#define SC_P_LVDS1_I2C0_SDA_LSIO_GPIO1_IO13                     SC_P_LVDS1_I2C0_SDA                3
#define SC_P_LVDS1_I2C1_SCL_LVDS1_I2C1_SCL                      SC_P_LVDS1_I2C1_SCL                0
#define SC_P_LVDS1_I2C1_SCL_DMA_UART3_TX                        SC_P_LVDS1_I2C1_SCL                1
#define SC_P_LVDS1_I2C1_SCL_LSIO_GPIO1_IO14                     SC_P_LVDS1_I2C1_SCL                3
#define SC_P_LVDS1_I2C1_SDA_LVDS1_I2C1_SDA                      SC_P_LVDS1_I2C1_SDA                0
#define SC_P_LVDS1_I2C1_SDA_DMA_UART3_RX                        SC_P_LVDS1_I2C1_SDA                1
#define SC_P_LVDS1_I2C1_SDA_LSIO_GPIO1_IO15                     SC_P_LVDS1_I2C1_SDA                3
#define SC_P_MIPI_DSI0_I2C0_SCL_MIPI_DSI0_I2C0_SCL              SC_P_MIPI_DSI0_I2C0_SCL            0
#define SC_P_MIPI_DSI0_I2C0_SCL_LSIO_GPIO1_IO16                 SC_P_MIPI_DSI0_I2C0_SCL            3
#define SC_P_MIPI_DSI0_I2C0_SDA_MIPI_DSI0_I2C0_SDA              SC_P_MIPI_DSI0_I2C0_SDA            0
#define SC_P_MIPI_DSI0_I2C0_SDA_LSIO_GPIO1_IO17                 SC_P_MIPI_DSI0_I2C0_SDA            3
#define SC_P_MIPI_DSI0_GPIO0_00_MIPI_DSI0_GPIO0_IO00            SC_P_MIPI_DSI0_GPIO0_00            0
#define SC_P_MIPI_DSI0_GPIO0_00_MIPI_DSI0_PWM0_OUT              SC_P_MIPI_DSI0_GPIO0_00            1
#define SC_P_MIPI_DSI0_GPIO0_00_LSIO_GPIO1_IO18                 SC_P_MIPI_DSI0_GPIO0_00            3
#define SC_P_MIPI_DSI0_GPIO0_01_MIPI_DSI0_GPIO0_IO01            SC_P_MIPI_DSI0_GPIO0_01            0
#define SC_P_MIPI_DSI0_GPIO0_01_LSIO_GPIO1_IO19                 SC_P_MIPI_DSI0_GPIO0_01            3
#define SC_P_MIPI_DSI1_I2C0_SCL_MIPI_DSI1_I2C0_SCL              SC_P_MIPI_DSI1_I2C0_SCL            0
#define SC_P_MIPI_DSI1_I2C0_SCL_LSIO_GPIO1_IO20                 SC_P_MIPI_DSI1_I2C0_SCL            3
#define SC_P_MIPI_DSI1_I2C0_SDA_MIPI_DSI1_I2C0_SDA              SC_P_MIPI_DSI1_I2C0_SDA            0
#define SC_P_MIPI_DSI1_I2C0_SDA_LSIO_GPIO1_IO21                 SC_P_MIPI_DSI1_I2C0_SDA            3
#define SC_P_MIPI_DSI1_GPIO0_00_MIPI_DSI1_GPIO0_IO00            SC_P_MIPI_DSI1_GPIO0_00            0
#define SC_P_MIPI_DSI1_GPIO0_00_MIPI_DSI1_PWM0_OUT              SC_P_MIPI_DSI1_GPIO0_00            1
#define SC_P_MIPI_DSI1_GPIO0_00_LSIO_GPIO1_IO22                 SC_P_MIPI_DSI1_GPIO0_00            3
#define SC_P_MIPI_DSI1_GPIO0_01_MIPI_DSI1_GPIO0_IO01            SC_P_MIPI_DSI1_GPIO0_01            0
#define SC_P_MIPI_DSI1_GPIO0_01_LSIO_GPIO1_IO23                 SC_P_MIPI_DSI1_GPIO0_01            3
#define SC_P_MIPI_CSI0_MCLK_OUT_MIPI_CSI0_ACM_MCLK_OUT          SC_P_MIPI_CSI0_MCLK_OUT            0
#define SC_P_MIPI_CSI0_MCLK_OUT_LSIO_GPIO1_IO24                 SC_P_MIPI_CSI0_MCLK_OUT            3
#define SC_P_MIPI_CSI0_I2C0_SCL_MIPI_CSI0_I2C0_SCL              SC_P_MIPI_CSI0_I2C0_SCL            0
#define SC_P_MIPI_CSI0_I2C0_SCL_LSIO_GPIO1_IO25                 SC_P_MIPI_CSI0_I2C0_SCL            3
#define SC_P_MIPI_CSI0_I2C0_SDA_MIPI_CSI0_I2C0_SDA              SC_P_MIPI_CSI0_I2C0_SDA            0
#define SC_P_MIPI_CSI0_I2C0_SDA_LSIO_GPIO1_IO26                 SC_P_MIPI_CSI0_I2C0_SDA            3
#define SC_P_MIPI_CSI0_GPIO0_00_MIPI_CSI0_GPIO0_IO00            SC_P_MIPI_CSI0_GPIO0_00            0
#define SC_P_MIPI_CSI0_GPIO0_00_DMA_I2C0_SCL                    SC_P_MIPI_CSI0_GPIO0_00            1
#define SC_P_MIPI_CSI0_GPIO0_00_MIPI_CSI1_I2C0_SCL              SC_P_MIPI_CSI0_GPIO0_00            2
#define SC_P_MIPI_CSI0_GPIO0_00_LSIO_GPIO1_IO27                 SC_P_MIPI_CSI0_GPIO0_00            3
#define SC_P_MIPI_CSI0_GPIO0_01_MIPI_CSI0_GPIO0_IO01            SC_P_MIPI_CSI0_GPIO0_01            0
#define SC_P_MIPI_CSI0_GPIO0_01_DMA_I2C0_SDA                    SC_P_MIPI_CSI0_GPIO0_01            1
#define SC_P_MIPI_CSI0_GPIO0_01_MIPI_CSI1_I2C0_SDA              SC_P_MIPI_CSI0_GPIO0_01            2
#define SC_P_MIPI_CSI0_GPIO0_01_LSIO_GPIO1_IO28                 SC_P_MIPI_CSI0_GPIO0_01            3
#define SC_P_MIPI_CSI1_MCLK_OUT_MIPI_CSI1_ACM_MCLK_OUT          SC_P_MIPI_CSI1_MCLK_OUT            0
#define SC_P_MIPI_CSI1_MCLK_OUT_LSIO_GPIO1_IO29                 SC_P_MIPI_CSI1_MCLK_OUT            3
#define SC_P_MIPI_CSI1_GPIO0_00_MIPI_CSI1_GPIO0_IO00            SC_P_MIPI_CSI1_GPIO0_00            0
#define SC_P_MIPI_CSI1_GPIO0_00_DMA_UART4_RX                    SC_P_MIPI_CSI1_GPIO0_00            1
#define SC_P_MIPI_CSI1_GPIO0_00_LSIO_GPIO1_IO30                 SC_P_MIPI_CSI1_GPIO0_00            3
#define SC_P_MIPI_CSI1_GPIO0_01_MIPI_CSI1_GPIO0_IO01            SC_P_MIPI_CSI1_GPIO0_01            0
#define SC_P_MIPI_CSI1_GPIO0_01_DMA_UART4_TX                    SC_P_MIPI_CSI1_GPIO0_01            1
#define SC_P_MIPI_CSI1_GPIO0_01_LSIO_GPIO1_IO31                 SC_P_MIPI_CSI1_GPIO0_01            3
#define SC_P_MIPI_CSI1_I2C0_SCL_MIPI_CSI1_I2C0_SCL              SC_P_MIPI_CSI1_I2C0_SCL            0
#define SC_P_MIPI_CSI1_I2C0_SCL_LSIO_GPIO2_IO00                 SC_P_MIPI_CSI1_I2C0_SCL            3
#define SC_P_MIPI_CSI1_I2C0_SDA_MIPI_CSI1_I2C0_SDA              SC_P_MIPI_CSI1_I2C0_SDA            0
#define SC_P_MIPI_CSI1_I2C0_SDA_LSIO_GPIO2_IO01                 SC_P_MIPI_CSI1_I2C0_SDA            3
#define SC_P_HDMI_TX0_TS_SCL_HDMI_TX0_I2C0_SCL                  SC_P_HDMI_TX0_TS_SCL               0
#define SC_P_HDMI_TX0_TS_SCL_DMA_I2C0_SCL                       SC_P_HDMI_TX0_TS_SCL               1
#define SC_P_HDMI_TX0_TS_SCL_LSIO_GPIO2_IO02                    SC_P_HDMI_TX0_TS_SCL               3
#define SC_P_HDMI_TX0_TS_SDA_HDMI_TX0_I2C0_SDA                  SC_P_HDMI_TX0_TS_SDA               0
#define SC_P_HDMI_TX0_TS_SDA_DMA_I2C0_SDA                       SC_P_HDMI_TX0_TS_SDA               1
#define SC_P_HDMI_TX0_TS_SDA_LSIO_GPIO2_IO03                    SC_P_HDMI_TX0_TS_SDA               3
#define SC_P_ESAI1_FSR_AUD_ESAI1_FSR                            SC_P_ESAI1_FSR                     0
#define SC_P_ESAI1_FSR_LSIO_GPIO2_IO04                          SC_P_ESAI1_FSR                     3
#define SC_P_ESAI1_FST_AUD_ESAI1_FST                            SC_P_ESAI1_FST                     0
#define SC_P_ESAI1_FST_AUD_SPDIF0_EXT_CLK                       SC_P_ESAI1_FST                     1
#define SC_P_ESAI1_FST_LSIO_GPIO2_IO05                          SC_P_ESAI1_FST                     3
#define SC_P_ESAI1_SCKR_AUD_ESAI1_SCKR                          SC_P_ESAI1_SCKR                    0
#define SC_P_ESAI1_SCKR_LSIO_GPIO2_IO06                         SC_P_ESAI1_SCKR                    3
#define SC_P_ESAI1_SCKT_AUD_ESAI1_SCKT                          SC_P_ESAI1_SCKT                    0
#define SC_P_ESAI1_SCKT_AUD_SAI2_RXC                            SC_P_ESAI1_SCKT                    1
#define SC_P_ESAI1_SCKT_AUD_SPDIF0_EXT_CLK                      SC_P_ESAI1_SCKT                    2
#define SC_P_ESAI1_SCKT_LSIO_GPIO2_IO07                         SC_P_ESAI1_SCKT                    3
#define SC_P_ESAI1_TX0_AUD_ESAI1_TX0                            SC_P_ESAI1_TX0                     0
#define SC_P_ESAI1_TX0_AUD_SAI2_RXD                             SC_P_ESAI1_TX0                     1
#define SC_P_ESAI1_TX0_AUD_SPDIF0_RX                            SC_P_ESAI1_TX0                     2
#define SC_P_ESAI1_TX0_LSIO_GPIO2_IO08                          SC_P_ESAI1_TX0                     3
#define SC_P_ESAI1_TX1_AUD_ESAI1_TX1                            SC_P_ESAI1_TX1                     0
#define SC_P_ESAI1_TX1_AUD_SAI2_RXFS                            SC_P_ESAI1_TX1                     1
#define SC_P_ESAI1_TX1_AUD_SPDIF0_TX                            SC_P_ESAI1_TX1                     2
#define SC_P_ESAI1_TX1_LSIO_GPIO2_IO09                          SC_P_ESAI1_TX1                     3
#define SC_P_ESAI1_TX2_RX3_AUD_ESAI1_TX2_RX3                    SC_P_ESAI1_TX2_RX3                 0
#define SC_P_ESAI1_TX2_RX3_AUD_SPDIF0_RX                        SC_P_ESAI1_TX2_RX3                 1
#define SC_P_ESAI1_TX2_RX3_LSIO_GPIO2_IO10                      SC_P_ESAI1_TX2_RX3                 3
#define SC_P_ESAI1_TX3_RX2_AUD_ESAI1_TX3_RX2                    SC_P_ESAI1_TX3_RX2                 0
#define SC_P_ESAI1_TX3_RX2_AUD_SPDIF0_TX                        SC_P_ESAI1_TX3_RX2                 1
#define SC_P_ESAI1_TX3_RX2_LSIO_GPIO2_IO11                      SC_P_ESAI1_TX3_RX2                 3
#define SC_P_ESAI1_TX4_RX1_AUD_ESAI1_TX4_RX1                    SC_P_ESAI1_TX4_RX1                 0
#define SC_P_ESAI1_TX4_RX1_LSIO_GPIO2_IO12                      SC_P_ESAI1_TX4_RX1                 3
#define SC_P_ESAI1_TX5_RX0_AUD_ESAI1_TX5_RX0                    SC_P_ESAI1_TX5_RX0                 0
#define SC_P_ESAI1_TX5_RX0_LSIO_GPIO2_IO13                      SC_P_ESAI1_TX5_RX0                 3
#define SC_P_SPDIF0_RX_AUD_SPDIF0_RX                            SC_P_SPDIF0_RX                     0
#define SC_P_SPDIF0_RX_AUD_MQS_R                                SC_P_SPDIF0_RX                     1
#define SC_P_SPDIF0_RX_AUD_ACM_MCLK_IN1                         SC_P_SPDIF0_RX                     2
#define SC_P_SPDIF0_RX_LSIO_GPIO2_IO14                          SC_P_SPDIF0_RX                     3
#define SC_P_SPDIF0_TX_AUD_SPDIF0_TX                            SC_P_SPDIF0_TX                     0
#define SC_P_SPDIF0_TX_AUD_MQS_L                                SC_P_SPDIF0_TX                     1
#define SC_P_SPDIF0_TX_AUD_ACM_MCLK_OUT1                        SC_P_SPDIF0_TX                     2
#define SC_P_SPDIF0_TX_LSIO_GPIO2_IO15                          SC_P_SPDIF0_TX                     3
#define SC_P_SPDIF0_EXT_CLK_AUD_SPDIF0_EXT_CLK                  SC_P_SPDIF0_EXT_CLK                0
#define SC_P_SPDIF0_EXT_CLK_DMA_DMA0_REQ_IN0                    SC_P_SPDIF0_EXT_CLK                1
#define SC_P_SPDIF0_EXT_CLK_LSIO_GPIO2_IO16                     SC_P_SPDIF0_EXT_CLK                3
#define SC_P_SPI3_SCK_DMA_SPI3_SCK                              SC_P_SPI3_SCK                      0
#define SC_P_SPI3_SCK_LSIO_GPIO2_IO17                           SC_P_SPI3_SCK                      3
#define SC_P_SPI3_SDO_DMA_SPI3_SDO                              SC_P_SPI3_SDO                      0
#define SC_P_SPI3_SDO_DMA_FTM_CH0                               SC_P_SPI3_SDO                      1
#define SC_P_SPI3_SDO_LSIO_GPIO2_IO18                           SC_P_SPI3_SDO                      3
#define SC_P_SPI3_SDI_DMA_SPI3_SDI                              SC_P_SPI3_SDI                      0
#define SC_P_SPI3_SDI_DMA_FTM_CH1                               SC_P_SPI3_SDI                      1
#define SC_P_SPI3_SDI_LSIO_GPIO2_IO19                           SC_P_SPI3_SDI                      3
#define SC_P_SPI3_CS0_DMA_SPI3_CS0                              SC_P_SPI3_CS0                      0
#define SC_P_SPI3_CS0_DMA_FTM_CH2                               SC_P_SPI3_CS0                      1
#define SC_P_SPI3_CS0_LSIO_GPIO2_IO20                           SC_P_SPI3_CS0                      3
#define SC_P_SPI3_CS1_DMA_SPI3_CS1                              SC_P_SPI3_CS1                      0
#define SC_P_SPI3_CS1_LSIO_GPIO2_IO21                           SC_P_SPI3_CS1                      3
#define SC_P_ESAI0_FSR_AUD_ESAI0_FSR                            SC_P_ESAI0_FSR                     0
#define SC_P_ESAI0_FSR_LSIO_GPIO2_IO22                          SC_P_ESAI0_FSR                     3
#define SC_P_ESAI0_FST_AUD_ESAI0_FST                            SC_P_ESAI0_FST                     0
#define SC_P_ESAI0_FST_LSIO_GPIO2_IO23                          SC_P_ESAI0_FST                     3
#define SC_P_ESAI0_SCKR_AUD_ESAI0_SCKR                          SC_P_ESAI0_SCKR                    0
#define SC_P_ESAI0_SCKR_LSIO_GPIO2_IO24                         SC_P_ESAI0_SCKR                    3
#define SC_P_ESAI0_SCKT_AUD_ESAI0_SCKT                          SC_P_ESAI0_SCKT                    0
#define SC_P_ESAI0_SCKT_LSIO_GPIO2_IO25                         SC_P_ESAI0_SCKT                    3
#define SC_P_ESAI0_TX0_AUD_ESAI0_TX0                            SC_P_ESAI0_TX0                     0
#define SC_P_ESAI0_TX0_LSIO_GPIO2_IO26                          SC_P_ESAI0_TX0                     3
#define SC_P_ESAI0_TX1_AUD_ESAI0_TX1                            SC_P_ESAI0_TX1                     0
#define SC_P_ESAI0_TX1_LSIO_GPIO2_IO27                          SC_P_ESAI0_TX1                     3
#define SC_P_ESAI0_TX2_RX3_AUD_ESAI0_TX2_RX3                    SC_P_ESAI0_TX2_RX3                 0
#define SC_P_ESAI0_TX2_RX3_LSIO_GPIO2_IO28                      SC_P_ESAI0_TX2_RX3                 3
#define SC_P_ESAI0_TX3_RX2_AUD_ESAI0_TX3_RX2                    SC_P_ESAI0_TX3_RX2                 0
#define SC_P_ESAI0_TX3_RX2_LSIO_GPIO2_IO29                      SC_P_ESAI0_TX3_RX2                 3
#define SC_P_ESAI0_TX4_RX1_AUD_ESAI0_TX4_RX1                    SC_P_ESAI0_TX4_RX1                 0
#define SC_P_ESAI0_TX4_RX1_LSIO_GPIO2_IO30                      SC_P_ESAI0_TX4_RX1                 3
#define SC_P_ESAI0_TX5_RX0_AUD_ESAI0_TX5_RX0                    SC_P_ESAI0_TX5_RX0                 0
#define SC_P_ESAI0_TX5_RX0_LSIO_GPIO2_IO31                      SC_P_ESAI0_TX5_RX0                 3
#define SC_P_MCLK_IN0_AUD_ACM_MCLK_IN0                          SC_P_MCLK_IN0                      0
#define SC_P_MCLK_IN0_AUD_ESAI0_RX_HF_CLK                       SC_P_MCLK_IN0                      1
#define SC_P_MCLK_IN0_AUD_ESAI1_RX_HF_CLK                       SC_P_MCLK_IN0                      2
#define SC_P_MCLK_IN0_LSIO_GPIO3_IO00                           SC_P_MCLK_IN0                      3
#define SC_P_MCLK_OUT0_AUD_ACM_MCLK_OUT0                        SC_P_MCLK_OUT0                     0
#define SC_P_MCLK_OUT0_AUD_ESAI0_TX_HF_CLK                      SC_P_MCLK_OUT0                     1
#define SC_P_MCLK_OUT0_AUD_ESAI1_TX_HF_CLK                      SC_P_MCLK_OUT0                     2
#define SC_P_MCLK_OUT0_LSIO_GPIO3_IO01                          SC_P_MCLK_OUT0                     3
#define SC_P_SPI0_SCK_DMA_SPI0_SCK                              SC_P_SPI0_SCK                      0
#define SC_P_SPI0_SCK_AUD_SAI0_RXC                              SC_P_SPI0_SCK                      1
#define SC_P_SPI0_SCK_LSIO_GPIO3_IO02                           SC_P_SPI0_SCK                      3
#define SC_P_SPI0_SDO_DMA_SPI0_SDO                              SC_P_SPI0_SDO                      0
#define SC_P_SPI0_SDO_AUD_SAI0_TXD                              SC_P_SPI0_SDO                      1
#define SC_P_SPI0_SDO_LSIO_GPIO3_IO03                           SC_P_SPI0_SDO                      3
#define SC_P_SPI0_SDI_DMA_SPI0_SDI                              SC_P_SPI0_SDI                      0
#define SC_P_SPI0_SDI_AUD_SAI0_RXD                              SC_P_SPI0_SDI                      1
#define SC_P_SPI0_SDI_LSIO_GPIO3_IO04                           SC_P_SPI0_SDI                      3
#define SC_P_SPI0_CS0_DMA_SPI0_CS0                              SC_P_SPI0_CS0                      0
#define SC_P_SPI0_CS0_AUD_SAI0_RXFS                             SC_P_SPI0_CS0                      1
#define SC_P_SPI0_CS0_LSIO_GPIO3_IO05                           SC_P_SPI0_CS0                      3
#define SC_P_SPI0_CS1_DMA_SPI0_CS1                              SC_P_SPI0_CS1                      0
#define SC_P_SPI0_CS1_AUD_SAI0_TXC                              SC_P_SPI0_CS1                      1
#define SC_P_SPI0_CS1_LSIO_GPIO3_IO06                           SC_P_SPI0_CS1                      3
#define SC_P_SPI2_SCK_DMA_SPI2_SCK                              SC_P_SPI2_SCK                      0
#define SC_P_SPI2_SCK_LSIO_GPIO3_IO07                           SC_P_SPI2_SCK                      3
#define SC_P_SPI2_SDO_DMA_SPI2_SDO                              SC_P_SPI2_SDO                      0
#define SC_P_SPI2_SDO_LSIO_GPIO3_IO08                           SC_P_SPI2_SDO                      3
#define SC_P_SPI2_SDI_DMA_SPI2_SDI                              SC_P_SPI2_SDI                      0
#define SC_P_SPI2_SDI_LSIO_GPIO3_IO09                           SC_P_SPI2_SDI                      3
#define SC_P_SPI2_CS0_DMA_SPI2_CS0                              SC_P_SPI2_CS0                      0
#define SC_P_SPI2_CS0_LSIO_GPIO3_IO10                           SC_P_SPI2_CS0                      3
#define SC_P_SPI2_CS1_DMA_SPI2_CS1                              SC_P_SPI2_CS1                      0
#define SC_P_SPI2_CS1_AUD_SAI0_TXFS                             SC_P_SPI2_CS1                      1
#define SC_P_SPI2_CS1_LSIO_GPIO3_IO11                           SC_P_SPI2_CS1                      3
#define SC_P_SAI1_RXC_AUD_SAI1_RXC                              SC_P_SAI1_RXC                      0
#define SC_P_SAI1_RXC_AUD_SAI0_TXD                              SC_P_SAI1_RXC                      1
#define SC_P_SAI1_RXC_LSIO_GPIO3_IO12                           SC_P_SAI1_RXC                      3
#define SC_P_SAI1_RXD_AUD_SAI1_RXD                              SC_P_SAI1_RXD                      0
#define SC_P_SAI1_RXD_AUD_SAI0_TXFS                             SC_P_SAI1_RXD                      1
#define SC_P_SAI1_RXD_LSIO_GPIO3_IO13                           SC_P_SAI1_RXD                      3
#define SC_P_SAI1_RXFS_AUD_SAI1_RXFS                            SC_P_SAI1_RXFS                     0
#define SC_P_SAI1_RXFS_AUD_SAI0_RXD                             SC_P_SAI1_RXFS                     1
#define SC_P_SAI1_RXFS_LSIO_GPIO3_IO14                          SC_P_SAI1_RXFS                     3
#define SC_P_SAI1_TXC_AUD_SAI1_TXC                              SC_P_SAI1_TXC                      0
#define SC_P_SAI1_TXC_AUD_SAI0_TXC                              SC_P_SAI1_TXC                      1
#define SC_P_SAI1_TXC_LSIO_GPIO3_IO15                           SC_P_SAI1_TXC                      3
#define SC_P_SAI1_TXD_AUD_SAI1_TXD                              SC_P_SAI1_TXD                      0
#define SC_P_SAI1_TXD_AUD_SAI1_RXC                              SC_P_SAI1_TXD                      1
#define SC_P_SAI1_TXD_LSIO_GPIO3_IO16                           SC_P_SAI1_TXD                      3
#define SC_P_SAI1_TXFS_AUD_SAI1_TXFS                            SC_P_SAI1_TXFS                     0
#define SC_P_SAI1_TXFS_AUD_SAI1_RXFS                            SC_P_SAI1_TXFS                     1
#define SC_P_SAI1_TXFS_LSIO_GPIO3_IO17                          SC_P_SAI1_TXFS                     3
#define SC_P_ADC_IN7_DMA_ADC1_IN3                               SC_P_ADC_IN7                       0
#define SC_P_ADC_IN7_DMA_SPI1_CS1                               SC_P_ADC_IN7                       1
#define SC_P_ADC_IN7_LSIO_KPP0_ROW3                             SC_P_ADC_IN7                       2
#define SC_P_ADC_IN7_LSIO_GPIO3_IO25                            SC_P_ADC_IN7                       3
#define SC_P_ADC_IN6_DMA_ADC1_IN2                               SC_P_ADC_IN6                       0
#define SC_P_ADC_IN6_DMA_SPI1_CS0                               SC_P_ADC_IN6                       1
#define SC_P_ADC_IN6_LSIO_KPP0_ROW2                             SC_P_ADC_IN6                       2
#define SC_P_ADC_IN6_LSIO_GPIO3_IO24                            SC_P_ADC_IN6                       3
#define SC_P_ADC_IN5_DMA_ADC1_IN1                               SC_P_ADC_IN5                       0
#define SC_P_ADC_IN5_DMA_SPI1_SDI                               SC_P_ADC_IN5                       1
#define SC_P_ADC_IN5_LSIO_KPP0_ROW1                             SC_P_ADC_IN5                       2
#define SC_P_ADC_IN5_LSIO_GPIO3_IO23                            SC_P_ADC_IN5                       3
#define SC_P_ADC_IN4_DMA_ADC1_IN0                               SC_P_ADC_IN4                       0
#define SC_P_ADC_IN4_DMA_SPI1_SDO                               SC_P_ADC_IN4                       1
#define SC_P_ADC_IN4_LSIO_KPP0_ROW0                             SC_P_ADC_IN4                       2
#define SC_P_ADC_IN4_LSIO_GPIO3_IO22                            SC_P_ADC_IN4                       3
#define SC_P_ADC_IN3_DMA_ADC0_IN3                               SC_P_ADC_IN3                       0
#define SC_P_ADC_IN3_DMA_SPI1_SCK                               SC_P_ADC_IN3                       1
#define SC_P_ADC_IN3_LSIO_KPP0_COL3                             SC_P_ADC_IN3                       2
#define SC_P_ADC_IN3_LSIO_GPIO3_IO21                            SC_P_ADC_IN3                       3
#define SC_P_ADC_IN2_DMA_ADC0_IN2                               SC_P_ADC_IN2                       0
#define SC_P_ADC_IN2_LSIO_KPP0_COL2                             SC_P_ADC_IN2                       2
#define SC_P_ADC_IN2_LSIO_GPIO3_IO20                            SC_P_ADC_IN2                       3
#define SC_P_ADC_IN1_DMA_ADC0_IN1                               SC_P_ADC_IN1                       0
#define SC_P_ADC_IN1_LSIO_KPP0_COL1                             SC_P_ADC_IN1                       2
#define SC_P_ADC_IN1_LSIO_GPIO3_IO19                            SC_P_ADC_IN1                       3
#define SC_P_ADC_IN0_DMA_ADC0_IN0                               SC_P_ADC_IN0                       0
#define SC_P_ADC_IN0_LSIO_KPP0_COL0                             SC_P_ADC_IN0                       2
#define SC_P_ADC_IN0_LSIO_GPIO3_IO18                            SC_P_ADC_IN0                       3
#define SC_P_MLB_SIG_CONN_MLB_SIG                               SC_P_MLB_SIG                       0
#define SC_P_MLB_SIG_AUD_SAI3_RXC                               SC_P_MLB_SIG                       1
#define SC_P_MLB_SIG_LSIO_GPIO3_IO26                            SC_P_MLB_SIG                       3
#define SC_P_MLB_CLK_CONN_MLB_CLK                               SC_P_MLB_CLK                       0
#define SC_P_MLB_CLK_AUD_SAI3_RXFS                              SC_P_MLB_CLK                       1
#define SC_P_MLB_CLK_LSIO_GPIO3_IO27                            SC_P_MLB_CLK                       3
#define SC_P_MLB_DATA_CONN_MLB_DATA                             SC_P_MLB_DATA                      0
#define SC_P_MLB_DATA_AUD_SAI3_RXD                              SC_P_MLB_DATA                      1
#define SC_P_MLB_DATA_LSIO_GPIO3_IO28                           SC_P_MLB_DATA                      3
#define SC_P_FLEXCAN0_RX_DMA_FLEXCAN0_RX                        SC_P_FLEXCAN0_RX                   0
#define SC_P_FLEXCAN0_RX_LSIO_GPIO3_IO29                        SC_P_FLEXCAN0_RX                   3
#define SC_P_FLEXCAN0_TX_DMA_FLEXCAN0_TX                        SC_P_FLEXCAN0_TX                   0
#define SC_P_FLEXCAN0_TX_LSIO_GPIO3_IO30                        SC_P_FLEXCAN0_TX                   3
#define SC_P_FLEXCAN1_RX_DMA_FLEXCAN1_RX                        SC_P_FLEXCAN1_RX                   0
#define SC_P_FLEXCAN1_RX_LSIO_GPIO3_IO31                        SC_P_FLEXCAN1_RX                   3
#define SC_P_FLEXCAN1_TX_DMA_FLEXCAN1_TX                        SC_P_FLEXCAN1_TX                   0
#define SC_P_FLEXCAN1_TX_LSIO_GPIO4_IO00                        SC_P_FLEXCAN1_TX                   3
#define SC_P_FLEXCAN2_RX_DMA_FLEXCAN2_RX                        SC_P_FLEXCAN2_RX                   0
#define SC_P_FLEXCAN2_RX_LSIO_GPIO4_IO01                        SC_P_FLEXCAN2_RX                   3
#define SC_P_FLEXCAN2_TX_DMA_FLEXCAN2_TX                        SC_P_FLEXCAN2_TX                   0
#define SC_P_FLEXCAN2_TX_LSIO_GPIO4_IO02                        SC_P_FLEXCAN2_TX                   3
#define SC_P_USB_SS3_TC0_DMA_I2C1_SCL                           SC_P_USB_SS3_TC0                   0
#define SC_P_USB_SS3_TC0_CONN_USB_OTG1_PWR                      SC_P_USB_SS3_TC0                   1
#define SC_P_USB_SS3_TC0_LSIO_GPIO4_IO03                        SC_P_USB_SS3_TC0                   3
#define SC_P_USB_SS3_TC1_DMA_I2C1_SCL                           SC_P_USB_SS3_TC1                   0
#define SC_P_USB_SS3_TC1_CONN_USB_OTG2_PWR                      SC_P_USB_SS3_TC1                   1
#define SC_P_USB_SS3_TC1_LSIO_GPIO4_IO04                        SC_P_USB_SS3_TC1                   3
#define SC_P_USB_SS3_TC2_DMA_I2C1_SDA                           SC_P_USB_SS3_TC2                   0
#define SC_P_USB_SS3_TC2_CONN_USB_OTG1_OC                       SC_P_USB_SS3_TC2                   1
#define SC_P_USB_SS3_TC2_LSIO_GPIO4_IO05                        SC_P_USB_SS3_TC2                   3
#define SC_P_USB_SS3_TC3_DMA_I2C1_SDA                           SC_P_USB_SS3_TC3                   0
#define SC_P_USB_SS3_TC3_CONN_USB_OTG2_OC                       SC_P_USB_SS3_TC3                   1
#define SC_P_USB_SS3_TC3_LSIO_GPIO4_IO06                        SC_P_USB_SS3_TC3                   3
#define SC_P_USDHC1_RESET_B_CONN_USDHC1_RESET_B                 SC_P_USDHC1_RESET_B                0
#define SC_P_USDHC1_RESET_B_LSIO_GPIO4_IO07                     SC_P_USDHC1_RESET_B                3
#define SC_P_USDHC1_VSELECT_CONN_USDHC1_VSELECT                 SC_P_USDHC1_VSELECT                0
#define SC_P_USDHC1_VSELECT_LSIO_GPIO4_IO08                     SC_P_USDHC1_VSELECT                3
#define SC_P_USDHC2_RESET_B_CONN_USDHC2_RESET_B                 SC_P_USDHC2_RESET_B                0
#define SC_P_USDHC2_RESET_B_LSIO_GPIO4_IO09                     SC_P_USDHC2_RESET_B                3
#define SC_P_USDHC2_VSELECT_CONN_USDHC2_VSELECT                 SC_P_USDHC2_VSELECT                0
#define SC_P_USDHC2_VSELECT_LSIO_GPIO4_IO10                     SC_P_USDHC2_VSELECT                3
#define SC_P_USDHC2_WP_CONN_USDHC2_WP                           SC_P_USDHC2_WP                     0
#define SC_P_USDHC2_WP_LSIO_GPIO4_IO11                          SC_P_USDHC2_WP                     3
#define SC_P_USDHC2_CD_B_CONN_USDHC2_CD_B                       SC_P_USDHC2_CD_B                   0
#define SC_P_USDHC2_CD_B_LSIO_GPIO4_IO12                        SC_P_USDHC2_CD_B                   3
#define SC_P_ENET0_MDIO_CONN_ENET0_MDIO                         SC_P_ENET0_MDIO                    0
#define SC_P_ENET0_MDIO_DMA_I2C4_SDA                            SC_P_ENET0_MDIO                    1
#define SC_P_ENET0_MDIO_LSIO_GPIO4_IO13                         SC_P_ENET0_MDIO                    3
#define SC_P_ENET0_MDC_CONN_ENET0_MDC                           SC_P_ENET0_MDC                     0
#define SC_P_ENET0_MDC_DMA_I2C4_SCL                             SC_P_ENET0_MDC                     1
#define SC_P_ENET0_MDC_LSIO_GPIO4_IO14                          SC_P_ENET0_MDC                     3
#define SC_P_ENET0_REFCLK_125M_25M_CONN_ENET0_REFCLK_125M_25M   SC_P_ENET0_REFCLK_125M_25M         0
#define SC_P_ENET0_REFCLK_125M_25M_CONN_ENET0_PPS               SC_P_ENET0_REFCLK_125M_25M         1
#define SC_P_ENET0_REFCLK_125M_25M_LSIO_GPIO4_IO15              SC_P_ENET0_REFCLK_125M_25M         3
#define SC_P_ENET1_REFCLK_125M_25M_CONN_ENET1_REFCLK_125M_25M   SC_P_ENET1_REFCLK_125M_25M         0
#define SC_P_ENET1_REFCLK_125M_25M_CONN_ENET1_PPS               SC_P_ENET1_REFCLK_125M_25M         1
#define SC_P_ENET1_REFCLK_125M_25M_LSIO_GPIO4_IO16              SC_P_ENET1_REFCLK_125M_25M         3
#define SC_P_ENET1_MDIO_CONN_ENET1_MDIO                         SC_P_ENET1_MDIO                    0
#define SC_P_ENET1_MDIO_DMA_I2C4_SDA                            SC_P_ENET1_MDIO                    1
#define SC_P_ENET1_MDIO_LSIO_GPIO4_IO17                         SC_P_ENET1_MDIO                    3
#define SC_P_ENET1_MDC_CONN_ENET1_MDC                           SC_P_ENET1_MDC                     0
#define SC_P_ENET1_MDC_DMA_I2C4_SCL                             SC_P_ENET1_MDC                     1
#define SC_P_ENET1_MDC_LSIO_GPIO4_IO18                          SC_P_ENET1_MDC                     3
#define SC_P_QSPI1A_SS0_B_LSIO_QSPI1A_SS0_B                     SC_P_QSPI1A_SS0_B                  0
#define SC_P_QSPI1A_SS0_B_LSIO_GPIO4_IO19                       SC_P_QSPI1A_SS0_B                  3
#define SC_P_QSPI1A_SS1_B_LSIO_QSPI1A_SS1_B                     SC_P_QSPI1A_SS1_B                  0
#define SC_P_QSPI1A_SS1_B_LSIO_QSPI1A_SCLK2                     SC_P_QSPI1A_SS1_B                  1
#define SC_P_QSPI1A_SS1_B_LSIO_GPIO4_IO20                       SC_P_QSPI1A_SS1_B                  3
#define SC_P_QSPI1A_SCLK_LSIO_QSPI1A_SCLK                       SC_P_QSPI1A_SCLK                   0
#define SC_P_QSPI1A_SCLK_LSIO_GPIO4_IO21                        SC_P_QSPI1A_SCLK                   3
#define SC_P_QSPI1A_DQS_LSIO_QSPI1A_DQS                         SC_P_QSPI1A_DQS                    0
#define SC_P_QSPI1A_DQS_LSIO_GPIO4_IO22                         SC_P_QSPI1A_DQS                    3
#define SC_P_QSPI1A_DATA3_LSIO_QSPI1A_DATA3                     SC_P_QSPI1A_DATA3                  0
#define SC_P_QSPI1A_DATA3_DMA_I2C1_SDA                          SC_P_QSPI1A_DATA3                  1
#define SC_P_QSPI1A_DATA3_CONN_USB_OTG1_OC                      SC_P_QSPI1A_DATA3                  2
#define SC_P_QSPI1A_DATA3_LSIO_GPIO4_IO23                       SC_P_QSPI1A_DATA3                  3
#define SC_P_QSPI1A_DATA2_LSIO_QSPI1A_DATA2                     SC_P_QSPI1A_DATA2                  0
#define SC_P_QSPI1A_DATA2_DMA_I2C1_SCL                          SC_P_QSPI1A_DATA2                  1
#define SC_P_QSPI1A_DATA2_CONN_USB_OTG2_PWR                     SC_P_QSPI1A_DATA2                  2
#define SC_P_QSPI1A_DATA2_LSIO_GPIO4_IO24                       SC_P_QSPI1A_DATA2                  3
#define SC_P_QSPI1A_DATA1_LSIO_QSPI1A_DATA1                     SC_P_QSPI1A_DATA1                  0
#define SC_P_QSPI1A_DATA1_DMA_I2C1_SDA                          SC_P_QSPI1A_DATA1                  1
#define SC_P_QSPI1A_DATA1_CONN_USB_OTG2_OC                      SC_P_QSPI1A_DATA1                  2
#define SC_P_QSPI1A_DATA1_LSIO_GPIO4_IO25                       SC_P_QSPI1A_DATA1                  3
#define SC_P_QSPI1A_DATA0_LSIO_QSPI1A_DATA0                     SC_P_QSPI1A_DATA0                  0
#define SC_P_QSPI1A_DATA0_LSIO_GPIO4_IO26                       SC_P_QSPI1A_DATA0                  3
#define SC_P_QSPI0A_DATA0_LSIO_QSPI0A_DATA0                     SC_P_QSPI0A_DATA0                  0
#define SC_P_QSPI0A_DATA1_LSIO_QSPI0A_DATA1                     SC_P_QSPI0A_DATA1                  0
#define SC_P_QSPI0A_DATA2_LSIO_QSPI0A_DATA2                     SC_P_QSPI0A_DATA2                  0
#define SC_P_QSPI0A_DATA3_LSIO_QSPI0A_DATA3                     SC_P_QSPI0A_DATA3                  0
#define SC_P_QSPI0A_DQS_LSIO_QSPI0A_DQS                         SC_P_QSPI0A_DQS                    0
#define SC_P_QSPI0A_SS0_B_LSIO_QSPI0A_SS0_B                     SC_P_QSPI0A_SS0_B                  0
#define SC_P_QSPI0A_SS1_B_LSIO_QSPI0A_SS1_B                     SC_P_QSPI0A_SS1_B                  0
#define SC_P_QSPI0A_SS1_B_LSIO_QSPI0A_SCLK2                     SC_P_QSPI0A_SS1_B                  1
#define SC_P_QSPI0A_SCLK_LSIO_QSPI0A_SCLK                       SC_P_QSPI0A_SCLK                   0
#define SC_P_QSPI0B_SCLK_LSIO_QSPI0B_SCLK                       SC_P_QSPI0B_SCLK                   0
#define SC_P_QSPI0B_DATA0_LSIO_QSPI0B_DATA0                     SC_P_QSPI0B_DATA0                  0
#define SC_P_QSPI0B_DATA1_LSIO_QSPI0B_DATA1                     SC_P_QSPI0B_DATA1                  0
#define SC_P_QSPI0B_DATA2_LSIO_QSPI0B_DATA2                     SC_P_QSPI0B_DATA2                  0
#define SC_P_QSPI0B_DATA3_LSIO_QSPI0B_DATA3                     SC_P_QSPI0B_DATA3                  0
#define SC_P_QSPI0B_DQS_LSIO_QSPI0B_DQS                         SC_P_QSPI0B_DQS                    0
#define SC_P_QSPI0B_SS0_B_LSIO_QSPI0B_SS0_B                     SC_P_QSPI0B_SS0_B                  0
#define SC_P_QSPI0B_SS1_B_LSIO_QSPI0B_SS1_B                     SC_P_QSPI0B_SS1_B                  0
#define SC_P_QSPI0B_SS1_B_LSIO_QSPI0B_SCLK2                     SC_P_QSPI0B_SS1_B                  1
#define SC_P_PCIE_CTRL0_CLKREQ_B_HSIO_PCIE0_CLKREQ_B            SC_P_PCIE_CTRL0_CLKREQ_B           0
#define SC_P_PCIE_CTRL0_CLKREQ_B_LSIO_GPIO4_IO27                SC_P_PCIE_CTRL0_CLKREQ_B           3
#define SC_P_PCIE_CTRL0_WAKE_B_HSIO_PCIE0_WAKE_B                SC_P_PCIE_CTRL0_WAKE_B             0
#define SC_P_PCIE_CTRL0_WAKE_B_LSIO_GPIO4_IO28                  SC_P_PCIE_CTRL0_WAKE_B             3
#define SC_P_PCIE_CTRL0_PERST_B_HSIO_PCIE0_PERST_B              SC_P_PCIE_CTRL0_PERST_B            0
#define SC_P_PCIE_CTRL0_PERST_B_LSIO_GPIO4_IO29                 SC_P_PCIE_CTRL0_PERST_B            3
#define SC_P_PCIE_CTRL1_CLKREQ_B_HSIO_PCIE1_CLKREQ_B            SC_P_PCIE_CTRL1_CLKREQ_B           0
#define SC_P_PCIE_CTRL1_CLKREQ_B_DMA_I2C1_SDA                   SC_P_PCIE_CTRL1_CLKREQ_B           1
#define SC_P_PCIE_CTRL1_CLKREQ_B_CONN_USB_OTG2_OC               SC_P_PCIE_CTRL1_CLKREQ_B           2
#define SC_P_PCIE_CTRL1_CLKREQ_B_LSIO_GPIO4_IO30                SC_P_PCIE_CTRL1_CLKREQ_B           3
#define SC_P_PCIE_CTRL1_WAKE_B_HSIO_PCIE1_WAKE_B                SC_P_PCIE_CTRL1_WAKE_B             0
#define SC_P_PCIE_CTRL1_WAKE_B_DMA_I2C1_SCL                     SC_P_PCIE_CTRL1_WAKE_B             1
#define SC_P_PCIE_CTRL1_WAKE_B_CONN_USB_OTG2_PWR                SC_P_PCIE_CTRL1_WAKE_B             2
#define SC_P_PCIE_CTRL1_WAKE_B_LSIO_GPIO4_IO31                  SC_P_PCIE_CTRL1_WAKE_B             3
#define SC_P_PCIE_CTRL1_PERST_B_HSIO_PCIE1_PERST_B              SC_P_PCIE_CTRL1_PERST_B            0
#define SC_P_PCIE_CTRL1_PERST_B_DMA_I2C1_SCL                    SC_P_PCIE_CTRL1_PERST_B            1
#define SC_P_PCIE_CTRL1_PERST_B_CONN_USB_OTG1_PWR               SC_P_PCIE_CTRL1_PERST_B            2
#define SC_P_PCIE_CTRL1_PERST_B_LSIO_GPIO5_IO00                 SC_P_PCIE_CTRL1_PERST_B            3
#define SC_P_USB_HSIC0_DATA_CONN_USB_HSIC0_DATA                 SC_P_USB_HSIC0_DATA                0
#define SC_P_USB_HSIC0_DATA_DMA_I2C1_SDA                        SC_P_USB_HSIC0_DATA                1
#define SC_P_USB_HSIC0_DATA_LSIO_GPIO5_IO01                     SC_P_USB_HSIC0_DATA                3
#define SC_P_USB_HSIC0_STROBE_CONN_USB_HSIC0_STROBE             SC_P_USB_HSIC0_STROBE              0
#define SC_P_USB_HSIC0_STROBE_DMA_I2C1_SCL                      SC_P_USB_HSIC0_STROBE              1
#define SC_P_USB_HSIC0_STROBE_LSIO_GPIO5_IO02                   SC_P_USB_HSIC0_STROBE              3
#define SC_P_EMMC0_CLK_CONN_EMMC0_CLK                           SC_P_EMMC0_CLK                     0
#define SC_P_EMMC0_CLK_CONN_NAND_READY_B                        SC_P_EMMC0_CLK                     1
#define SC_P_EMMC0_CMD_CONN_EMMC0_CMD                           SC_P_EMMC0_CMD                     0
#define SC_P_EMMC0_CMD_CONN_NAND_DQS                            SC_P_EMMC0_CMD                     1
#define SC_P_EMMC0_CMD_AUD_MQS_R                                SC_P_EMMC0_CMD                     2
#define SC_P_EMMC0_CMD_LSIO_GPIO5_IO03                          SC_P_EMMC0_CMD                     3
#define SC_P_EMMC0_DATA0_CONN_EMMC0_DATA0                       SC_P_EMMC0_DATA0                   0
#define SC_P_EMMC0_DATA0_CONN_NAND_DATA00                       SC_P_EMMC0_DATA0                   1
#define SC_P_EMMC0_DATA0_LSIO_GPIO5_IO04                        SC_P_EMMC0_DATA0                   3
#define SC_P_EMMC0_DATA1_CONN_EMMC0_DATA1                       SC_P_EMMC0_DATA1                   0
#define SC_P_EMMC0_DATA1_CONN_NAND_DATA01                       SC_P_EMMC0_DATA1                   1
#define SC_P_EMMC0_DATA1_LSIO_GPIO5_IO05                        SC_P_EMMC0_DATA1                   3
#define SC_P_EMMC0_DATA2_CONN_EMMC0_DATA2                       SC_P_EMMC0_DATA2                   0
#define SC_P_EMMC0_DATA2_CONN_NAND_DATA02                       SC_P_EMMC0_DATA2                   1
#define SC_P_EMMC0_DATA2_LSIO_GPIO5_IO06                        SC_P_EMMC0_DATA2                   3
#define SC_P_EMMC0_DATA3_CONN_EMMC0_DATA3                       SC_P_EMMC0_DATA3                   0
#define SC_P_EMMC0_DATA3_CONN_NAND_DATA03                       SC_P_EMMC0_DATA3                   1
#define SC_P_EMMC0_DATA3_LSIO_GPIO5_IO07                        SC_P_EMMC0_DATA3                   3
#define SC_P_EMMC0_DATA4_CONN_EMMC0_DATA4                       SC_P_EMMC0_DATA4                   0
#define SC_P_EMMC0_DATA4_CONN_NAND_DATA04                       SC_P_EMMC0_DATA4                   1
#define SC_P_EMMC0_DATA4_LSIO_GPIO5_IO08                        SC_P_EMMC0_DATA4                   3
#define SC_P_EMMC0_DATA5_CONN_EMMC0_DATA5                       SC_P_EMMC0_DATA5                   0
#define SC_P_EMMC0_DATA5_CONN_NAND_DATA05                       SC_P_EMMC0_DATA5                   1
#define SC_P_EMMC0_DATA5_LSIO_GPIO5_IO09                        SC_P_EMMC0_DATA5                   3
#define SC_P_EMMC0_DATA6_CONN_EMMC0_DATA6                       SC_P_EMMC0_DATA6                   0
#define SC_P_EMMC0_DATA6_CONN_NAND_DATA06                       SC_P_EMMC0_DATA6                   1
#define SC_P_EMMC0_DATA6_LSIO_GPIO5_IO10                        SC_P_EMMC0_DATA6                   3
#define SC_P_EMMC0_DATA7_CONN_EMMC0_DATA7                       SC_P_EMMC0_DATA7                   0
#define SC_P_EMMC0_DATA7_CONN_NAND_DATA07                       SC_P_EMMC0_DATA7                   1
#define SC_P_EMMC0_DATA7_LSIO_GPIO5_IO11                        SC_P_EMMC0_DATA7                   3
#define SC_P_EMMC0_STROBE_CONN_EMMC0_STROBE                     SC_P_EMMC0_STROBE                  0
#define SC_P_EMMC0_STROBE_CONN_NAND_CLE                         SC_P_EMMC0_STROBE                  1
#define SC_P_EMMC0_STROBE_LSIO_GPIO5_IO12                       SC_P_EMMC0_STROBE                  3
#define SC_P_EMMC0_RESET_B_CONN_EMMC0_RESET_B                   SC_P_EMMC0_RESET_B                 0
#define SC_P_EMMC0_RESET_B_CONN_NAND_WP_B                       SC_P_EMMC0_RESET_B                 1
#define SC_P_EMMC0_RESET_B_CONN_USDHC1_VSELECT                  SC_P_EMMC0_RESET_B                 2
#define SC_P_EMMC0_RESET_B_LSIO_GPIO5_IO13                      SC_P_EMMC0_RESET_B                 3
#define SC_P_USDHC1_CLK_CONN_USDHC1_CLK                         SC_P_USDHC1_CLK                    0
#define SC_P_USDHC1_CLK_AUD_MQS_R                               SC_P_USDHC1_CLK                    1
#define SC_P_USDHC1_CMD_CONN_USDHC1_CMD                         SC_P_USDHC1_CMD                    0
#define SC_P_USDHC1_CMD_AUD_MQS_L                               SC_P_USDHC1_CMD                    1
#define SC_P_USDHC1_CMD_LSIO_GPIO5_IO14                         SC_P_USDHC1_CMD                    3
#define SC_P_USDHC1_DATA0_CONN_USDHC1_DATA0                     SC_P_USDHC1_DATA0                  0
#define SC_P_USDHC1_DATA0_CONN_NAND_RE_N                        SC_P_USDHC1_DATA0                  1
#define SC_P_USDHC1_DATA0_LSIO_GPIO5_IO15                       SC_P_USDHC1_DATA0                  3
#define SC_P_USDHC1_DATA1_CONN_USDHC1_DATA1                     SC_P_USDHC1_DATA1                  0
#define SC_P_USDHC1_DATA1_CONN_NAND_RE_P                        SC_P_USDHC1_DATA1                  1
#define SC_P_USDHC1_DATA1_LSIO_GPIO5_IO16                       SC_P_USDHC1_DATA1                  3
#define SC_P_USDHC1_DATA2_CONN_USDHC1_DATA2                     SC_P_USDHC1_DATA2                  0
#define SC_P_USDHC1_DATA2_CONN_NAND_DQS_N                       SC_P_USDHC1_DATA2                  1
#define SC_P_USDHC1_DATA2_LSIO_GPIO5_IO17                       SC_P_USDHC1_DATA2                  3
#define SC_P_USDHC1_DATA3_CONN_USDHC1_DATA3                     SC_P_USDHC1_DATA3                  0
#define SC_P_USDHC1_DATA3_CONN_NAND_DQS_P                       SC_P_USDHC1_DATA3                  1
#define SC_P_USDHC1_DATA3_LSIO_GPIO5_IO18                       SC_P_USDHC1_DATA3                  3
#define SC_P_USDHC1_DATA4_CONN_USDHC1_DATA4                     SC_P_USDHC1_DATA4                  0
#define SC_P_USDHC1_DATA4_CONN_NAND_CE0_B                       SC_P_USDHC1_DATA4                  1
#define SC_P_USDHC1_DATA4_AUD_MQS_R                             SC_P_USDHC1_DATA4                  2
#define SC_P_USDHC1_DATA4_LSIO_GPIO5_IO19                       SC_P_USDHC1_DATA4                  3
#define SC_P_USDHC1_DATA5_CONN_USDHC1_DATA5                     SC_P_USDHC1_DATA5                  0
#define SC_P_USDHC1_DATA5_CONN_NAND_RE_B                        SC_P_USDHC1_DATA5                  1
#define SC_P_USDHC1_DATA5_AUD_MQS_L                             SC_P_USDHC1_DATA5                  2
#define SC_P_USDHC1_DATA5_LSIO_GPIO5_IO20                       SC_P_USDHC1_DATA5                  3
#define SC_P_USDHC1_DATA6_CONN_USDHC1_DATA6                     SC_P_USDHC1_DATA6                  0
#define SC_P_USDHC1_DATA6_CONN_NAND_WE_B                        SC_P_USDHC1_DATA6                  1
#define SC_P_USDHC1_DATA6_CONN_USDHC1_WP                        SC_P_USDHC1_DATA6                  2
#define SC_P_USDHC1_DATA6_LSIO_GPIO5_IO21                       SC_P_USDHC1_DATA6                  3
#define SC_P_USDHC1_DATA7_CONN_USDHC1_DATA7                     SC_P_USDHC1_DATA7                  0
#define SC_P_USDHC1_DATA7_CONN_NAND_ALE                         SC_P_USDHC1_DATA7                  1
#define SC_P_USDHC1_DATA7_CONN_USDHC1_CD_B                      SC_P_USDHC1_DATA7                  2
#define SC_P_USDHC1_DATA7_LSIO_GPIO5_IO22                       SC_P_USDHC1_DATA7                  3
#define SC_P_USDHC1_STROBE_CONN_USDHC1_STROBE                   SC_P_USDHC1_STROBE                 0
#define SC_P_USDHC1_STROBE_CONN_NAND_CE1_B                      SC_P_USDHC1_STROBE                 1
#define SC_P_USDHC1_STROBE_CONN_USDHC1_RESET_B                  SC_P_USDHC1_STROBE                 2
#define SC_P_USDHC1_STROBE_LSIO_GPIO5_IO23                      SC_P_USDHC1_STROBE                 3
#define SC_P_USDHC2_CLK_CONN_USDHC2_CLK                         SC_P_USDHC2_CLK                    0
#define SC_P_USDHC2_CLK_AUD_MQS_R                               SC_P_USDHC2_CLK                    1
#define SC_P_USDHC2_CLK_LSIO_GPIO5_IO24                         SC_P_USDHC2_CLK                    3
#define SC_P_USDHC2_CMD_CONN_USDHC2_CMD                         SC_P_USDHC2_CMD                    0
#define SC_P_USDHC2_CMD_AUD_MQS_L                               SC_P_USDHC2_CMD                    1
#define SC_P_USDHC2_CMD_LSIO_GPIO5_IO25                         SC_P_USDHC2_CMD                    3
#define SC_P_USDHC2_DATA0_CONN_USDHC2_DATA0                     SC_P_USDHC2_DATA0                  0
#define SC_P_USDHC2_DATA0_DMA_UART4_RX                          SC_P_USDHC2_DATA0                  1
#define SC_P_USDHC2_DATA0_LSIO_GPIO5_IO26                       SC_P_USDHC2_DATA0                  3
#define SC_P_USDHC2_DATA1_CONN_USDHC2_DATA1                     SC_P_USDHC2_DATA1                  0
#define SC_P_USDHC2_DATA1_DMA_UART4_TX                          SC_P_USDHC2_DATA1                  1
#define SC_P_USDHC2_DATA1_LSIO_GPIO5_IO27                       SC_P_USDHC2_DATA1                  3
#define SC_P_USDHC2_DATA2_CONN_USDHC2_DATA2                     SC_P_USDHC2_DATA2                  0
#define SC_P_USDHC2_DATA2_DMA_UART4_CTS_B                       SC_P_USDHC2_DATA2                  1
#define SC_P_USDHC2_DATA2_LSIO_GPIO5_IO28                       SC_P_USDHC2_DATA2                  3
#define SC_P_USDHC2_DATA3_CONN_USDHC2_DATA3                     SC_P_USDHC2_DATA3                  0
#define SC_P_USDHC2_DATA3_DMA_UART4_RTS_B                       SC_P_USDHC2_DATA3                  1
#define SC_P_USDHC2_DATA3_LSIO_GPIO5_IO29                       SC_P_USDHC2_DATA3                  3
#define SC_P_ENET0_RGMII_TXC_CONN_ENET0_RGMII_TXC               SC_P_ENET0_RGMII_TXC               0
#define SC_P_ENET0_RGMII_TXC_CONN_ENET0_RCLK50M_OUT             SC_P_ENET0_RGMII_TXC               1
#define SC_P_ENET0_RGMII_TXC_CONN_ENET0_RCLK50M_IN              SC_P_ENET0_RGMII_TXC               2
#define SC_P_ENET0_RGMII_TXC_LSIO_GPIO5_IO30                    SC_P_ENET0_RGMII_TXC               3
#define SC_P_ENET0_RGMII_TX_CTL_CONN_ENET0_RGMII_TX_CTL         SC_P_ENET0_RGMII_TX_CTL            0
#define SC_P_ENET0_RGMII_TX_CTL_LSIO_GPIO5_IO31                 SC_P_ENET0_RGMII_TX_CTL            3
#define SC_P_ENET0_RGMII_TXD0_CONN_ENET0_RGMII_TXD0             SC_P_ENET0_RGMII_TXD0              0
#define SC_P_ENET0_RGMII_TXD0_LSIO_GPIO6_IO00                   SC_P_ENET0_RGMII_TXD0              3
#define SC_P_ENET0_RGMII_TXD1_CONN_ENET0_RGMII_TXD1             SC_P_ENET0_RGMII_TXD1              0
#define SC_P_ENET0_RGMII_TXD1_LSIO_GPIO6_IO01                   SC_P_ENET0_RGMII_TXD1              3
#define SC_P_ENET0_RGMII_TXD2_CONN_ENET0_RGMII_TXD2             SC_P_ENET0_RGMII_TXD2              0
#define SC_P_ENET0_RGMII_TXD2_DMA_UART3_TX                      SC_P_ENET0_RGMII_TXD2              1
#define SC_P_ENET0_RGMII_TXD2_VPU_TSI_S1_VID                    SC_P_ENET0_RGMII_TXD2              2
#define SC_P_ENET0_RGMII_TXD2_LSIO_GPIO6_IO02                   SC_P_ENET0_RGMII_TXD2              3
#define SC_P_ENET0_RGMII_TXD3_CONN_ENET0_RGMII_TXD3             SC_P_ENET0_RGMII_TXD3              0
#define SC_P_ENET0_RGMII_TXD3_DMA_UART3_RTS_B                   SC_P_ENET0_RGMII_TXD3              1
#define SC_P_ENET0_RGMII_TXD3_VPU_TSI_S1_SYNC                   SC_P_ENET0_RGMII_TXD3              2
#define SC_P_ENET0_RGMII_TXD3_LSIO_GPIO6_IO03                   SC_P_ENET0_RGMII_TXD3              3
#define SC_P_ENET0_RGMII_RXC_CONN_ENET0_RGMII_RXC               SC_P_ENET0_RGMII_RXC               0
#define SC_P_ENET0_RGMII_RXC_DMA_UART3_CTS_B                    SC_P_ENET0_RGMII_RXC               1
#define SC_P_ENET0_RGMII_RXC_VPU_TSI_S1_DATA                    SC_P_ENET0_RGMII_RXC               2
#define SC_P_ENET0_RGMII_RXC_LSIO_GPIO6_IO04                    SC_P_ENET0_RGMII_RXC               3
#define SC_P_ENET0_RGMII_RX_CTL_CONN_ENET0_RGMII_RX_CTL         SC_P_ENET0_RGMII_RX_CTL            0
#define SC_P_ENET0_RGMII_RX_CTL_VPU_TSI_S0_VID                  SC_P_ENET0_RGMII_RX_CTL            2
#define SC_P_ENET0_RGMII_RX_CTL_LSIO_GPIO6_IO05                 SC_P_ENET0_RGMII_RX_CTL            3
#define SC_P_ENET0_RGMII_RXD0_CONN_ENET0_RGMII_RXD0             SC_P_ENET0_RGMII_RXD0              0
#define SC_P_ENET0_RGMII_RXD0_VPU_TSI_S0_SYNC                   SC_P_ENET0_RGMII_RXD0              2
#define SC_P_ENET0_RGMII_RXD0_LSIO_GPIO6_IO06                   SC_P_ENET0_RGMII_RXD0              3
#define SC_P_ENET0_RGMII_RXD1_CONN_ENET0_RGMII_RXD1             SC_P_ENET0_RGMII_RXD1              0
#define SC_P_ENET0_RGMII_RXD1_VPU_TSI_S0_DATA                   SC_P_ENET0_RGMII_RXD1              2
#define SC_P_ENET0_RGMII_RXD1_LSIO_GPIO6_IO07                   SC_P_ENET0_RGMII_RXD1              3
#define SC_P_ENET0_RGMII_RXD2_CONN_ENET0_RGMII_RXD2             SC_P_ENET0_RGMII_RXD2              0
#define SC_P_ENET0_RGMII_RXD2_CONN_ENET0_RMII_RX_ER             SC_P_ENET0_RGMII_RXD2              1
#define SC_P_ENET0_RGMII_RXD2_VPU_TSI_S0_CLK                    SC_P_ENET0_RGMII_RXD2              2
#define SC_P_ENET0_RGMII_RXD2_LSIO_GPIO6_IO08                   SC_P_ENET0_RGMII_RXD2              3
#define SC_P_ENET0_RGMII_RXD3_CONN_ENET0_RGMII_RXD3             SC_P_ENET0_RGMII_RXD3              0
#define SC_P_ENET0_RGMII_RXD3_DMA_UART3_RX                      SC_P_ENET0_RGMII_RXD3              1
#define SC_P_ENET0_RGMII_RXD3_VPU_TSI_S1_CLK                    SC_P_ENET0_RGMII_RXD3              2
#define SC_P_ENET0_RGMII_RXD3_LSIO_GPIO6_IO09                   SC_P_ENET0_RGMII_RXD3              3
#define SC_P_ENET1_RGMII_TXC_CONN_ENET1_RGMII_TXC               SC_P_ENET1_RGMII_TXC               0
#define SC_P_ENET1_RGMII_TXC_CONN_ENET1_RCLK50M_OUT             SC_P_ENET1_RGMII_TXC               1
#define SC_P_ENET1_RGMII_TXC_CONN_ENET1_RCLK50M_IN              SC_P_ENET1_RGMII_TXC               2
#define SC_P_ENET1_RGMII_TXC_LSIO_GPIO6_IO10                    SC_P_ENET1_RGMII_TXC               3
#define SC_P_ENET1_RGMII_TX_CTL_CONN_ENET1_RGMII_TX_CTL         SC_P_ENET1_RGMII_TX_CTL            0
#define SC_P_ENET1_RGMII_TX_CTL_LSIO_GPIO6_IO11                 SC_P_ENET1_RGMII_TX_CTL            3
#define SC_P_ENET1_RGMII_TXD0_CONN_ENET1_RGMII_TXD0             SC_P_ENET1_RGMII_TXD0              0
#define SC_P_ENET1_RGMII_TXD0_LSIO_GPIO6_IO12                   SC_P_ENET1_RGMII_TXD0              3
#define SC_P_ENET1_RGMII_TXD1_CONN_ENET1_RGMII_TXD1             SC_P_ENET1_RGMII_TXD1              0
#define SC_P_ENET1_RGMII_TXD1_LSIO_GPIO6_IO13                   SC_P_ENET1_RGMII_TXD1              3
#define SC_P_ENET1_RGMII_TXD2_CONN_ENET1_RGMII_TXD2             SC_P_ENET1_RGMII_TXD2              0
#define SC_P_ENET1_RGMII_TXD2_DMA_UART3_TX                      SC_P_ENET1_RGMII_TXD2              1
#define SC_P_ENET1_RGMII_TXD2_VPU_TSI_S1_VID                    SC_P_ENET1_RGMII_TXD2              2
#define SC_P_ENET1_RGMII_TXD2_LSIO_GPIO6_IO14                   SC_P_ENET1_RGMII_TXD2              3
#define SC_P_ENET1_RGMII_TXD3_CONN_ENET1_RGMII_TXD3             SC_P_ENET1_RGMII_TXD3              0
#define SC_P_ENET1_RGMII_TXD3_DMA_UART3_RTS_B                   SC_P_ENET1_RGMII_TXD3              1
#define SC_P_ENET1_RGMII_TXD3_VPU_TSI_S1_SYNC                   SC_P_ENET1_RGMII_TXD3              2
#define SC_P_ENET1_RGMII_TXD3_LSIO_GPIO6_IO15                   SC_P_ENET1_RGMII_TXD3              3
#define SC_P_ENET1_RGMII_RXC_CONN_ENET1_RGMII_RXC               SC_P_ENET1_RGMII_RXC               0
#define SC_P_ENET1_RGMII_RXC_DMA_UART3_CTS_B                    SC_P_ENET1_RGMII_RXC               1
#define SC_P_ENET1_RGMII_RXC_VPU_TSI_S1_DATA                    SC_P_ENET1_RGMII_RXC               2
#define SC_P_ENET1_RGMII_RXC_LSIO_GPIO6_IO16                    SC_P_ENET1_RGMII_RXC               3
#define SC_P_ENET1_RGMII_RX_CTL_CONN_ENET1_RGMII_RX_CTL         SC_P_ENET1_RGMII_RX_CTL            0
#define SC_P_ENET1_RGMII_RX_CTL_VPU_TSI_S0_VID                  SC_P_ENET1_RGMII_RX_CTL            2
#define SC_P_ENET1_RGMII_RX_CTL_LSIO_GPIO6_IO17                 SC_P_ENET1_RGMII_RX_CTL            3
#define SC_P_ENET1_RGMII_RXD0_CONN_ENET1_RGMII_RXD0             SC_P_ENET1_RGMII_RXD0              0
#define SC_P_ENET1_RGMII_RXD0_VPU_TSI_S0_SYNC                   SC_P_ENET1_RGMII_RXD0              2
#define SC_P_ENET1_RGMII_RXD0_LSIO_GPIO6_IO18                   SC_P_ENET1_RGMII_RXD0              3
#define SC_P_ENET1_RGMII_RXD1_CONN_ENET1_RGMII_RXD1             SC_P_ENET1_RGMII_RXD1              0
#define SC_P_ENET1_RGMII_RXD1_VPU_TSI_S0_DATA                   SC_P_ENET1_RGMII_RXD1              2
#define SC_P_ENET1_RGMII_RXD1_LSIO_GPIO6_IO19                   SC_P_ENET1_RGMII_RXD1              3
#define SC_P_ENET1_RGMII_RXD2_CONN_ENET1_RGMII_RXD2             SC_P_ENET1_RGMII_RXD2              0
#define SC_P_ENET1_RGMII_RXD2_CONN_ENET1_RMII_RX_ER             SC_P_ENET1_RGMII_RXD2              1
#define SC_P_ENET1_RGMII_RXD2_VPU_TSI_S0_CLK                    SC_P_ENET1_RGMII_RXD2              2
#define SC_P_ENET1_RGMII_RXD2_LSIO_GPIO6_IO20                   SC_P_ENET1_RGMII_RXD2              3
#define SC_P_ENET1_RGMII_RXD3_CONN_ENET1_RGMII_RXD3             SC_P_ENET1_RGMII_RXD3              0
#define SC_P_ENET1_RGMII_RXD3_DMA_UART3_RX                      SC_P_ENET1_RGMII_RXD3              1
#define SC_P_ENET1_RGMII_RXD3_VPU_TSI_S1_CLK                    SC_P_ENET1_RGMII_RXD3              2
#define SC_P_ENET1_RGMII_RXD3_LSIO_GPIO6_IO21                   SC_P_ENET1_RGMII_RXD3              3
#define SC_P_COMP_CTL_GPIO_1V8_3V3_ENET_ENETB_PAD               SC_P_COMP_CTL_GPIO_1V8_3V3_ENET_ENETB              0
#define SC_P_COMP_CTL_GPIO_1V8_3V3_ENET_ENETA_PAD               SC_P_COMP_CTL_GPIO_1V8_3V3_ENET_ENETA              0

#endif				/* SC_PADS_H */
