/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#ifndef __SOC_ROCKCHIP_RK3399_GRF_H__
#define __SOC_ROCKCHIP_RK3399_GRF_H__

struct rk3399_grf_regs {
	u32 reserved[0x800];
	u32 usb3_perf_con0;
	u32 usb3_perf_con1;
	u32 usb3_perf_con2;
	u32 usb3_perf_rd_max_latency_num;
	u32 usb3_perf_rd_latency_samp_num;
	u32 usb3_perf_rd_latency_acc_num;
	u32 usb3_perf_rd_axi_total_byte;
	u32 usb3_perf_wr_axi_total_byte;
	u32 usb3_perf_working_cnt;
	u32 reserved1[0x103];
	u32 usb3otg0_con0;
	u32 usb3otg0_con1;
	u32 reserved2[2];
	u32 usb3otg1_con0;
	u32 usb3otg1_con1;
	u32 reserved3[2];
	u32 usb3otg0_status_lat0;
	u32 usb3otg0_status_lat1;
	u32 usb3otg0_status_cb;
	u32 reserved4;
	u32 usb3otg1_status_lat0;
	u32 usb3otg1_status_lat1;
	u32 usb3ogt1_status_cb;
	u32 reserved5[0x6e5];
	u32 pcie_perf_con0;
	u32 pcie_perf_con1;
	u32 pcie_perf_con2;
	u32 pcie_perf_rd_max_latency_num;
	u32 pcie_perf_rd_latency_samp_num;
	u32 pcie_perf_rd_laterncy_acc_num;
	u32 pcie_perf_rd_axi_total_byte;
	u32 pcie_perf_wr_axi_total_byte;
	u32 pcie_perf_working_cnt;
	u32 reserved6[0x37];
	u32 usb20_host0_con0;
	u32 usb20_host0_con1;
	u32 reserved7[2];
	u32 usb20_host1_con0;
	u32 usb20_host1_con1;
	u32 reserved8[2];
	u32 hsic_con0;
	u32 hsic_con1;
	u32 reserved9[6];
	u32 grf_usbhost0_status;
	u32 grf_usbhost1_Status;
	u32 grf_hsic_status;
	u32 reserved10[0xc9];
	u32 hsicphy_con0;
	u32 reserved11[3];
	u32 usbphy0_ctrl[26];
	u32 reserved12[6];
	u32 usbphy1[26];
	u32 reserved13[0x72f];
	u32 soc_con9;
	u32 reserved14[0x0a];
	u32 soc_con20;
	u32 soc_con21;
	u32 soc_con22;
	u32 soc_con23;
	u32 soc_con24;
	u32 soc_con25;
	u32 soc_con26;
	u32 reserved15[0xf65];
	u32 cpu_con[4];
	u32 reserved16[0x1c];
	u32 cpu_status[6];
	u32 reserved17[0x1a];
	u32 a53_perf_con[4];
	u32 a53_perf_rd_mon_st;
	u32 a53_perf_rd_mon_end;
	u32 a53_perf_wr_mon_st;
	u32 a53_perf_wr_mon_end;
	u32 a53_perf_rd_max_latency_num;
	u32 a53_perf_rd_latency_samp_num;
	u32 a53_perf_rd_laterncy_acc_num;
	u32 a53_perf_rd_axi_total_byte;
	u32 a53_perf_wr_axi_total_byte;
	u32 a53_perf_working_cnt;
	u32 a53_perf_int_status;
	u32 reserved18[0x31];
	u32 a72_perf_con[4];
	u32 a72_perf_rd_mon_st;
	u32 a72_perf_rd_mon_end;
	u32 a72_perf_wr_mon_st;
	u32 a72_perf_wr_mon_end;
	u32 a72_perf_rd_max_latency_num;
	u32 a72_perf_rd_latency_samp_num;
	u32 a72_perf_rd_laterncy_acc_num;
	u32 a72_perf_rd_axi_total_byte;
	u32 a72_perf_wr_axi_total_byte;
	u32 a72_perf_working_cnt;
	u32 a72_perf_int_status;
	u32 reserved19[0x7f6];
	u32 soc_con5;
	u32 soc_con6;
	u32 reserved20[0x779];
	u32 gpio2a_iomux;
	union {
		u32 iomux_spi2;
		u32 gpio2b_iomux;
	};
	union {
		u32 gpio2c_iomux;
		u32 iomux_spi5;
	};
	u32 gpio2d_iomux;
	union {
		u32 gpio3a_iomux;
		u32 iomux_spi0;
	};
	u32 gpio3b_iomux;
	u32 gpio3c_iomux;
	union {
		u32 iomux_i2s0;
		u32 gpio3d_iomux;
	};
	union {
		u32 iomux_i2sclk;
		u32 gpio4a_iomux;
	};
	union {
		u32 iomux_sdmmc;
		u32 iomux_uart2a;
		u32 gpio4b_iomux;
	};
	union {
		u32 iomux_pwm_0;
		u32 iomux_pwm_1;
		u32 iomux_uart2b;
		u32 iomux_uart2c;
		u32 iomux_edp_hotplug;
		u32 gpio4c_iomux;
	};
	u32 gpio4d_iomux;
	u32 reserved21[4];
	u32 gpio2_p[4];
	u32 gpio3_p[4];
	u32 gpio4_p[4];
	u32 reserved22[4];
	u32 gpio2_sr[3][4];
	u32 reserved23[4];
	u32 gpio2_smt[3][4];
	u32 reserved24[(0xe100 - 0xe0ec)/4 - 1];
	u32 gpio2_e[4];
	u32 gpio3_e[7];
	u32 gpio4_e[5];
	u32 reserved24a[(0xe200 - 0xe13c)/4 - 1];
	u32 soc_con0;
	u32 soc_con1;
	u32 soc_con2;
	u32 soc_con3;
	u32 soc_con4;
	u32 soc_con5_pcie;
	u32 reserved25;
	u32 soc_con7;
	u32 soc_con8;
	u32 soc_con9_pcie;
	u32 reserved26[0x1e];
	u32 soc_status[6];
	u32 reserved27[0x32];
	u32 ddrc0_con0;
	u32 ddrc0_con1;
	u32 ddrc1_con0;
	u32 ddrc1_con1;
	u32 reserved28[0xac];
	u32 io_vsel;
	u32 saradc_testbit;
	u32 tsadc_testbit_l;
	u32 tsadc_testbit_h;
	u32 reserved29[0x6c];
	u32 chip_id_addr;
	u32 reserved30[0x1f];
	u32 fast_boot_addr;
	u32 reserved31[0x1df];
	u32 emmccore_con[12];
	u32 reserved32[4];
	u32 emmccore_status[4];
	u32 reserved33[0x1cc];
	u32 emmcphy_con[7];
	u32 reserved34;
	u32 emmcphy_status;
};
check_member(rk3399_grf_regs, emmcphy_status, 0xf7a0);

struct rk3399_pmugrf_regs {
	union {
		u32 iomux_pwm_3a;
		u32 gpio0a_iomux;
	};
	u32 gpio0b_iomux;
	u32 reserved0[2];
	union {
		u32 spi1_rxd;
		u32 tsadc_int;
		u32 gpio1a_iomux;
	};
	union {
		u32 spi1_csclktx;
		u32 iomux_pwm_3b;
		u32 iomux_i2c0_sda;
		u32 gpio1b_iomux;
	};
	union {
		u32 iomux_pwm_2;
		u32 iomux_i2c0_scl;
		u32 gpio1c_iomux;
	};
	u32 gpio1d_iomux;
	u32 reserved1[8];
	u32 gpio0_p[2];
	u32 reserved2[2];
	u32 gpio1_p[4];
	u32 reserved3[8];
	u32 gpio0a_e;
	u32 reserved4;
	u32 gpio0b_e;
	u32 reserved5[5];
	u32 gpio1a_e;
	u32 reserved6;
	u32 gpio1b_e;
	u32 reserved7;
	u32 gpio1c_e;
	u32 reserved8;
	u32 gpio1d_e;
	u32 reserved9[0x11];
	u32 gpio0l_sr;
	u32 reserved10;
	u32 gpio1l_sr;
	u32 gpio1h_sr;
	u32 reserved11[4];
	u32 gpio0a_smt;
	u32 gpio0b_smt;
	u32 reserved12[2];
	u32 gpio1a_smt;
	u32 gpio1b_smt;
	u32 gpio1c_smt;
	u32 gpio1d_smt;
	u32 reserved13[8];
	u32 gpio0l_he;
	u32 reserved14;
	u32 gpio1l_he;
	u32 gpio1h_he;
	u32 reserved15[4];
	u32 soc_con0;
	u32 reserved16[9];
	u32 soc_con10;
	u32 soc_con11;
	u32 reserved17[0x24];
	u32 pmupvtm_con0;
	u32 pmupvtm_con1;
	u32 pmupvtm_status0;
	u32 pmupvtm_status1;
	u32 grf_osc_e;
	u32 reserved18[0x2b];
	u32 os_reg0;
	u32 os_reg1;
	u32 os_reg2;
	u32 os_reg3;
};
check_member(rk3399_pmugrf_regs, os_reg3, 0x30c);

struct rk3399_pmusgrf_regs {
	u32 ddr_rgn_con[35];
	u32 reserved[0x1fe5];
	u32 soc_con8;
	u32 soc_con9;
	u32 soc_con10;
	u32 soc_con11;
	u32 soc_con12;
	u32 soc_con13;
	u32 soc_con14;
	u32 soc_con15;
	u32 reserved1[3];
	u32 soc_con19;
	u32 soc_con20;
	u32 soc_con21;
	u32 soc_con22;
	u32 reserved2[0x29];
	u32 perilp_con[9];
	u32 reserved4[7];
	u32 perilp_status;
	u32 reserved5[0xfaf];
	u32 soc_con0;
	u32 soc_con1;
	u32 reserved6[0x3e];
	u32 pmu_con[9];
	u32 reserved7[0x17];
	u32 fast_boot_addr;
	u32 reserved8[0x1f];
	u32 efuse_prg_mask;
	u32 efuse_read_mask;
	u32 reserved9[0x0e];
	u32 pmu_slv_con0;
	u32 pmu_slv_con1;
	u32 reserved10[0x771];
	u32 soc_con3;
	u32 soc_con4;
	u32 soc_con5;
	u32 soc_con6;
	u32 soc_con7;
	u32 reserved11[8];
	u32 soc_con16;
	u32 soc_con17;
	u32 soc_con18;
	u32 reserved12[0xdd];
	u32 slv_secure_con0;
	u32 slv_secure_con1;
	u32 reserved13;
	u32 slv_secure_con2;
	u32 slv_secure_con3;
	u32 slv_secure_con4;
};
check_member(rk3399_pmusgrf_regs, slv_secure_con4, 0xe3d4);

enum {
	/* GRF_GPIO2A_IOMUX */
	GRF_GPIO2A0_SEL_SHIFT   = 0,
	GRF_GPIO2A0_SEL_MASK    = 3 << GRF_GPIO2A0_SEL_SHIFT,
	GRF_I2C2_SDA            = 2,
	GRF_GPIO2A1_SEL_SHIFT   = 2,
	GRF_GPIO2A1_SEL_MASK    = 3 << GRF_GPIO2A1_SEL_SHIFT,
	GRF_I2C2_SCL            = 2,
	GRF_GPIO2A7_SEL_SHIFT   = 14,
	GRF_GPIO2A7_SEL_MASK    = 3 << GRF_GPIO2A7_SEL_SHIFT,
	GRF_I2C7_SDA            = 2,

	/* GRF_GPIO2B_IOMUX */
	GRF_GPIO2B0_SEL_SHIFT   = 0,
	GRF_GPIO2B0_SEL_MASK    = 3 << GRF_GPIO2B0_SEL_SHIFT,
	GRF_I2C7_SCL            = 2,
	GRF_GPIO2B1_SEL_SHIFT	= 2,
	GRF_GPIO2B1_SEL_MASK	= 3 << GRF_GPIO2B1_SEL_SHIFT,
	GRF_SPI2TPM_RXD		= 1,
	GRF_I2C6_SDA            = 2,
	GRF_GPIO2B2_SEL_SHIFT	= 4,
	GRF_GPIO2B2_SEL_MASK	= 3 << GRF_GPIO2B2_SEL_SHIFT,
	GRF_SPI2TPM_TXD		= 1,
	GRF_I2C6_SCL            = 2,
	GRF_GPIO2B3_SEL_SHIFT	= 6,
	GRF_GPIO2B3_SEL_MASK	= 3 << GRF_GPIO2B3_SEL_SHIFT,
	GRF_SPI2TPM_CLK		= 1,
	GRF_GPIO2B4_SEL_SHIFT	= 8,
	GRF_GPIO2B4_SEL_MASK	= 3 << GRF_GPIO2B4_SEL_SHIFT,
	GRF_SPI2TPM_CSN0	= 1,

	/* GRF_GPIO2C_IOMUX */
	GRF_GPIO2C0_SEL_SHIFT   = 0,
	GRF_GPIO2C0_SEL_MASK    = 3 << GRF_GPIO2C0_SEL_SHIFT,
	GRF_UART0BT_SIN         = 1,
	GRF_GPIO2C1_SEL_SHIFT   = 2,
	GRF_GPIO2C1_SEL_MASK    = 3 << GRF_GPIO2C1_SEL_SHIFT,
	GRF_UART0BT_SOUT        = 1,
	GRF_GPIO2C4_SEL_SHIFT   = 8,
	GRF_GPIO2C4_SEL_MASK    = 3 << GRF_GPIO2C4_SEL_SHIFT,
	GRF_SPI5EXPPLUS_RXD     = 2,
	GRF_GPIO2C5_SEL_SHIFT   = 10,
	GRF_GPIO2C5_SEL_MASK    = 3 << GRF_GPIO2C5_SEL_SHIFT,
	GRF_SPI5EXPPLUS_TXD     = 2,
	GRF_GPIO2C6_SEL_SHIFT   = 12,
	GRF_GPIO2C6_SEL_MASK    = 3 << GRF_GPIO2C6_SEL_SHIFT,
	GRF_SPI5EXPPLUS_CLK     = 2,
	GRF_GPIO2C7_SEL_SHIFT   = 14,
	GRF_GPIO2C7_SEL_MASK    = 3 << GRF_GPIO2C7_SEL_SHIFT,
	GRF_SPI5EXPPLUS_CSN0    = 2,

	/* GRF_GPIO3A_IOMUX */
	GRF_GPIO3A0_SEL_SHIFT   = 0,
	GRF_GPIO3A0_SEL_MASK    = 3 << GRF_GPIO3A0_SEL_SHIFT,
	GRF_MAC_TXD2            = 1,
	GRF_GPIO3A1_SEL_SHIFT   = 2,
	GRF_GPIO3A1_SEL_MASK    = 3 << GRF_GPIO3A1_SEL_SHIFT,
	GRF_MAC_TXD3            = 1,
	GRF_GPIO3A2_SEL_SHIFT   = 4,
	GRF_GPIO3A2_SEL_MASK    = 3 << GRF_GPIO3A2_SEL_SHIFT,
	GRF_MAC_RXD2            = 1,
	GRF_GPIO3A3_SEL_SHIFT   = 6,
	GRF_GPIO3A3_SEL_MASK    = 3 << GRF_GPIO3A3_SEL_SHIFT,
	GRF_MAC_RXD3            = 1,
	GRF_GPIO3A4_SEL_SHIFT	= 8,
	GRF_GPIO3A4_SEL_MASK	= 3 << GRF_GPIO3A4_SEL_SHIFT,
	GRF_MAC_TXD0            = 1,
	GRF_SPI0NORCODEC_RXD	= 2,
	GRF_GPIO3A5_SEL_SHIFT	= 10,
	GRF_GPIO3A5_SEL_MASK	= 3 << GRF_GPIO3A5_SEL_SHIFT,
	GRF_MAC_TXD1            = 1,
	GRF_SPI0NORCODEC_TXD	= 2,
	GRF_GPIO3A6_SEL_SHIFT	= 12,
	GRF_GPIO3A6_SEL_MASK	= 3 << GRF_GPIO3A6_SEL_SHIFT,
	GRF_MAC_RXD0            = 1,
	GRF_SPI0NORCODEC_CLK	= 2,
	GRF_GPIO3A7_SEL_SHIFT	= 14,
	GRF_GPIO3A7_SEL_MASK	= 3 << GRF_GPIO3A7_SEL_SHIFT,
	GRF_MAC_RXD1            = 1,
	GRF_SPI0NORCODEC_CSN0	= 2,

	/* GRF_GPIO3B_IOMUX */
	GRF_GPIO3B0_SEL_SHIFT	= 0,
	GRF_GPIO3B0_SEL_MASK	= 3 << GRF_GPIO3B0_SEL_SHIFT,
	GRF_MAC_MDC             = 1,
	GRF_SPI0NORCODEC_CSN1	= 2,
	GRF_GPIO3B1_SEL_SHIFT	= 2,
	GRF_GPIO3B1_SEL_MASK	= 3 << GRF_GPIO3B1_SEL_SHIFT,
	GRF_MAC_RXDV            = 1,
	GRF_GPIO3B3_SEL_SHIFT	= 6,
	GRF_GPIO3B3_SEL_MASK	= 3 << GRF_GPIO3B3_SEL_SHIFT,
	GRF_MAC_CLK             = 1,
	GRF_GPIO3B4_SEL_SHIFT	= 8,
	GRF_GPIO3B4_SEL_MASK	= 3 << GRF_GPIO3B4_SEL_SHIFT,
	GRF_MAC_TXEN            = 1,
	GRF_GPIO3B5_SEL_SHIFT	= 10,
	GRF_GPIO3B5_SEL_MASK	= 3 << GRF_GPIO3B5_SEL_SHIFT,
	GRF_MAC_MDIO            = 1,
	GRF_GPIO3B6_SEL_SHIFT   = 12,
	GRF_GPIO3B6_SEL_MASK    = 3 << GRF_GPIO3B6_SEL_SHIFT,
	GRF_MAC_RXCLK           = 1,
	GRF_UART3_SIN           = 2,
	GRF_GPIO3B7_SEL_SHIFT   = 14,
	GRF_GPIO3B7_SEL_MASK    = 3 << GRF_GPIO3B7_SEL_SHIFT,
	GRF_UART3_SOUT          = 2,

	/* GRF_GPIO3C_IOMUX */
	GRF_GPIO3C1_SEL_SHIFT	= 2,
	GRF_GPIO3C1_SEL_MASK	= 3 << GRF_GPIO3C1_SEL_SHIFT,
	GRF_MAC_TXCLK           = 1,

	/* GRF_GPIO4A_IOMUX */
	GRF_GPIO4A1_SEL_SHIFT   = 2,
	GRF_GPIO4A1_SEL_MASK    = 3 << GRF_GPIO4A1_SEL_SHIFT,
	GRF_I2C1_SDA            = 1,
	GRF_GPIO4A2_SEL_SHIFT   = 4,
	GRF_GPIO4A2_SEL_MASK    = 3 << GRF_GPIO4A2_SEL_SHIFT,
	GRF_I2C1_SCL            = 1,

	/* GRF_GPIO4B_IOMUX */
	GRF_GPIO4B0_SEL_SHIFT	= 0,
	GRF_GPIO4B0_SEL_MASK	= 3 << GRF_GPIO4B0_SEL_SHIFT,
	GRF_SDMMC_DATA0		= 1,
	GRF_UART2DBGA_SIN	= 2,
	GRF_GPIO4B1_SEL_SHIFT	= 2,
	GRF_GPIO4B1_SEL_MASK	= 3 << GRF_GPIO4B1_SEL_SHIFT,
	GRF_SDMMC_DATA1		= 1,
	GRF_UART2DBGA_SOUT	= 2,
	GRF_GPIO4B2_SEL_SHIFT	= 4,
	GRF_GPIO4B2_SEL_MASK	= 3 << GRF_GPIO4B2_SEL_SHIFT,
	GRF_SDMMC_DATA2		= 1,
	GRF_GPIO4B3_SEL_SHIFT	= 6,
	GRF_GPIO4B3_SEL_MASK	= 3 << GRF_GPIO4B3_SEL_SHIFT,
	GRF_SDMMC_DATA3		= 1,
	GRF_GPIO4B4_SEL_SHIFT	= 8,
	GRF_GPIO4B4_SEL_MASK    = 3 << GRF_GPIO4B4_SEL_SHIFT,
	GRF_SDMMC_CLKOUT        = 1,
	GRF_GPIO4B5_SEL_SHIFT   = 10,
	GRF_GPIO4B5_SEL_MASK    = 3 << GRF_GPIO4B5_SEL_SHIFT,
	GRF_SDMMC_CMD           = 1,

	/*  GRF_GPIO4C_IOMUX */
	GRF_GPIO4C0_SEL_SHIFT   = 0,
	GRF_GPIO4C0_SEL_MASK    = 3 << GRF_GPIO4C0_SEL_SHIFT,
	GRF_UART2DGBB_SIN       = 2,
	GRF_HDMII2C_SCL         = 3,
	GRF_GPIO4C1_SEL_SHIFT   = 2,
	GRF_GPIO4C1_SEL_MASK    = 3 << GRF_GPIO4C1_SEL_SHIFT,
	GRF_UART2DGBB_SOUT      = 2,
	GRF_HDMII2C_SDA         = 3,
	GRF_GPIO4C2_SEL_SHIFT   = 4,
	GRF_GPIO4C2_SEL_MASK    = 3 << GRF_GPIO4C2_SEL_SHIFT,
	GRF_PWM_0               = 1,
	GRF_GPIO4C3_SEL_SHIFT   = 6,
	GRF_GPIO4C3_SEL_MASK    = 3 << GRF_GPIO4C3_SEL_SHIFT,
	GRF_UART2DGBC_SIN       = 1,
	GRF_GPIO4C4_SEL_SHIFT   = 8,
	GRF_GPIO4C4_SEL_MASK    = 3 << GRF_GPIO4C4_SEL_SHIFT,
	GRF_UART2DBGC_SOUT      = 1,
	GRF_GPIO4C6_SEL_SHIFT   = 12,
	GRF_GPIO4C6_SEL_MASK    = 3 << GRF_GPIO4C6_SEL_SHIFT,
	GRF_PWM_1               = 1,

	/* GRF_GPIO3A_E01 */
	GRF_GPIO3A0_E_SHIFT = 0,
	GRF_GPIO3A0_E_MASK = 7 << GRF_GPIO3A0_E_SHIFT,
	GRF_GPIO3A1_E_SHIFT = 3,
	GRF_GPIO3A1_E_MASK = 7 << GRF_GPIO3A1_E_SHIFT,
	GRF_GPIO3A2_E_SHIFT = 6,
	GRF_GPIO3A2_E_MASK = 7 << GRF_GPIO3A2_E_SHIFT,
	GRF_GPIO3A3_E_SHIFT = 9,
	GRF_GPIO3A3_E_MASK = 7 << GRF_GPIO3A3_E_SHIFT,
	GRF_GPIO3A4_E_SHIFT = 12,
	GRF_GPIO3A4_E_MASK = 7 << GRF_GPIO3A4_E_SHIFT,
	GRF_GPIO3A5_E0_SHIFT = 15,
	GRF_GPIO3A5_E0_MASK = 1 << GRF_GPIO3A5_E0_SHIFT,

	/*  GRF_GPIO3A_E2 */
	GRF_GPIO3A5_E12_SHIFT = 0,
	GRF_GPIO3A5_E12_MASK = 3 << GRF_GPIO3A5_E12_SHIFT,
	GRF_GPIO3A6_E_SHIFT = 2,
	GRF_GPIO3A6_E_MASK = 7 << GRF_GPIO3A6_E_SHIFT,
	GRF_GPIO3A7_E_SHIFT = 5,
	GRF_GPIO3A7_E_MASK = 7 << GRF_GPIO3A7_E_SHIFT,

	/* GRF_GPIO3B_E01 */
	GRF_GPIO3B0_E_SHIFT = 0,
	GRF_GPIO3B0_E_MASK = 7 << GRF_GPIO3B0_E_SHIFT,
	GRF_GPIO3B1_E_SHIFT = 3,
	GRF_GPIO3B1_E_MASK = 7 << GRF_GPIO3B1_E_SHIFT,
	GRF_GPIO3B2_E_SHIFT = 6,
	GRF_GPIO3B2_E_MASK = 7 << GRF_GPIO3B2_E_SHIFT,
	GRF_GPIO3B3_E_SHIFT = 9,
	GRF_GPIO3B3_E_MASK = 7 << GRF_GPIO3B3_E_SHIFT,
	GRF_GPIO3B4_E_SHIFT = 12,
	GRF_GPIO3B4_E_MASK = 7 << GRF_GPIO3B4_E_SHIFT,
	GRF_GPIO3B5_E0_SHIFT = 15,
	GRF_GPIO3B5_E0_MASK = 1 << GRF_GPIO3B5_E0_SHIFT,

	/*  GRF_GPIO3A_E2 */
	GRF_GPIO3B5_E12_SHIFT = 0,
	GRF_GPIO3B5_E12_MASK = 3 << GRF_GPIO3B5_E12_SHIFT,
	GRF_GPIO3B6_E_SHIFT = 2,
	GRF_GPIO3B6_E_MASK = 7 << GRF_GPIO3B6_E_SHIFT,
	GRF_GPIO3B7_E_SHIFT = 5,
	GRF_GPIO3B7_E_MASK = 7 << GRF_GPIO3B7_E_SHIFT,

	/* GRF_GPIO3C_E01 */
	GRF_GPIO3C0_E_SHIFT = 0,
	GRF_GPIO3C0_E_MASK = 7 << GRF_GPIO3C0_E_SHIFT,
	GRF_GPIO3C1_E_SHIFT = 3,
	GRF_GPIO3C1_E_MASK = 7 << GRF_GPIO3C1_E_SHIFT,
	GRF_GPIO3C2_E_SHIFT = 6,
	GRF_GPIO3C2_E_MASK = 7 << GRF_GPIO3C2_E_SHIFT,
	GRF_GPIO3C3_E_SHIFT = 9,
	GRF_GPIO3C3_E_MASK = 7 << GRF_GPIO3C3_E_SHIFT,
	GRF_GPIO3C4_E_SHIFT = 12,
	GRF_GPIO3C4_E_MASK = 7 << GRF_GPIO3C4_E_SHIFT,
	GRF_GPIO3C5_E0_SHIFT = 15,
	GRF_GPIO3C5_E0_MASK = 1 << GRF_GPIO3C5_E0_SHIFT,

	/*  GRF_GPIO3C_E2 */
	GRF_GPIO3C5_E12_SHIFT = 0,
	GRF_GPIO3C5_E12_MASK = 3 << GRF_GPIO3C5_E12_SHIFT,
	GRF_GPIO3C6_E_SHIFT = 2,
	GRF_GPIO3C6_E_MASK = 7 << GRF_GPIO3C6_E_SHIFT,
	GRF_GPIO3C7_E_SHIFT = 5,
	GRF_GPIO3C7_E_MASK = 7 << GRF_GPIO3C7_E_SHIFT,

	/* GRF_SOC_CON7 */
	GRF_UART_DBG_SEL_SHIFT  = 10,
	GRF_UART_DBG_SEL_MASK   = 3 << GRF_UART_DBG_SEL_SHIFT,
	GRF_UART_DBG_SEL_C      = 2,

	/* GRF_SOC_CON20 */
	GRF_DSI0_VOP_SEL_SHIFT  = 0,
	GRF_DSI0_VOP_SEL_MASK   = 1 << GRF_DSI0_VOP_SEL_SHIFT,
	GRF_DSI0_VOP_SEL_B      = 0,
	GRF_DSI0_VOP_SEL_L      = 1,
	GRF_RK3399_HDMI_VOP_SEL_MASK = 1 << 6,
	GRF_RK3399_HDMI_VOP_SEL_B = 0 << 6,
	GRF_RK3399_HDMI_VOP_SEL_L = 1 << 6,

	/* GRF_SOC_CON22 */
	GRF_DPHY_TX0_RXMODE_SHIFT = 0,
	GRF_DPHY_TX0_RXMODE_MASK  = 0xf << GRF_DPHY_TX0_RXMODE_SHIFT,
	GRF_DPHY_TX0_RXMODE_EN    = 0xb,
	GRF_DPHY_TX0_RXMODE_DIS   = 0,

	GRF_DPHY_TX0_TXSTOPMODE_SHIFT = 4,
	GRF_DPHY_TX0_TXSTOPMODE_MASK  = 0xf0 << GRF_DPHY_TX0_TXSTOPMODE_SHIFT,
	GRF_DPHY_TX0_TXSTOPMODE_EN    = 0xc,
	GRF_DPHY_TX0_TXSTOPMODE_DIS   = 0,

	GRF_DPHY_TX0_TURNREQUEST_SHIFT = 12,
	GRF_DPHY_TX0_TURNREQUEST_MASK  =
		0xf000 << GRF_DPHY_TX0_TURNREQUEST_SHIFT,
	GRF_DPHY_TX0_TURNREQUEST_EN    = 0x1,
	GRF_DPHY_TX0_TURNREQUEST_DIS   = 0,

	/*  PMUGRF_GPIO0A_IOMUX */
	PMUGRF_GPIO0A6_SEL_SHIFT        = 12,
	PMUGRF_GPIO0A6_SEL_MASK = 3 << PMUGRF_GPIO0A6_SEL_SHIFT,
	PMUGRF_PWM_3A           = 1,

	/*  PMUGRF_GPIO1A_IOMUX */
	PMUGRF_GPIO1A7_SEL_SHIFT        = 14,
	PMUGRF_GPIO1A7_SEL_MASK = 3 << PMUGRF_GPIO1A7_SEL_SHIFT,
	PMUGRF_SPI1EC_RXD       = 2,

	/*  PMUGRF_GPIO1B_IOMUX */
	PMUGRF_GPIO1B0_SEL_SHIFT        = 0,
	PMUGRF_GPIO1B0_SEL_MASK = 3 << PMUGRF_GPIO1B0_SEL_SHIFT,
	PMUGRF_SPI1EC_TXD       = 2,
	PMUGRF_GPIO1B1_SEL_SHIFT        = 2,
	PMUGRF_GPIO1B1_SEL_MASK = 3 << PMUGRF_GPIO1B1_SEL_SHIFT,
	PMUGRF_SPI1EC_CLK       = 2,
	PMUGRF_GPIO1B2_SEL_SHIFT        = 4,
	PMUGRF_GPIO1B2_SEL_MASK = 3 << PMUGRF_GPIO1B2_SEL_SHIFT,
	PMUGRF_SPI1EC_CSN0      = 2,
	PMUGRF_GPIO1B3_SEL_SHIFT	= 6,
	PMUGRF_GPIO1B3_SEL_MASK	= 3 << PMUGRF_GPIO1B3_SEL_SHIFT,
	PMUGRF_I2C4_SDA         = 1,
	PMUGRF_GPIO1B4_SEL_SHIFT        = 8,
	PMUGRF_GPIO1B4_SEL_MASK	= 3 << PMUGRF_GPIO1B4_SEL_SHIFT,
	PMUGRF_I2C4_SCL	        = 1,
	PMUGRF_GPIO1B6_SEL_SHIFT        = 12,
	PMUGRF_GPIO1B6_SEL_MASK = 3 << PMUGRF_GPIO1B6_SEL_SHIFT,
	PMUGRF_PWM_3B           = 1,
	PMUGRF_GPIO1B7_SEL_SHIFT        = 14,
	PMUGRF_GPIO1B7_SEL_MASK = 3 << PMUGRF_GPIO1B7_SEL_SHIFT,
	PMUGRF_I2C0PMU_SDA      = 2,

	/*  PMUGRF_GPIO1C_IOMUX */
	PMUGRF_GPIO1C0_SEL_SHIFT        = 0,
	PMUGRF_GPIO1C0_SEL_MASK = 3 << PMUGRF_GPIO1C0_SEL_SHIFT,
	PMUGRF_I2C0PMU_SCL      = 2,
	PMUGRF_GPIO1C3_SEL_SHIFT        = 6,
	PMUGRF_GPIO1C3_SEL_MASK = 3 << PMUGRF_GPIO1C3_SEL_SHIFT,
	PMUGRF_PWM_2            = 1,
	PMUGRF_GPIO1C4_SEL_SHIFT = 8,
	PMUGRF_GPIO1C4_SEL_MASK = 3 << PMUGRF_GPIO1C4_SEL_SHIFT,
	PMUGRF_I2C8PMU_SDA = 1,
	PMUGRF_GPIO1C5_SEL_SHIFT = 10,
	PMUGRF_GPIO1C5_SEL_MASK = 3 << PMUGRF_GPIO1C5_SEL_SHIFT,
	PMUGRF_I2C8PMU_SCL = 1,
};

/* GRF_SOC_CON5 */
enum {
	RK3399_GMAC_PHY_INTF_SEL_SHIFT = 9,
	RK3399_GMAC_PHY_INTF_SEL_MASK  = (7 << RK3399_GMAC_PHY_INTF_SEL_SHIFT),
	RK3399_GMAC_PHY_INTF_SEL_RGMII = (1 << RK3399_GMAC_PHY_INTF_SEL_SHIFT),
	RK3399_GMAC_PHY_INTF_SEL_RMII  = (4 << RK3399_GMAC_PHY_INTF_SEL_SHIFT),

	RK3399_GMAC_CLK_SEL_SHIFT = 4,
	RK3399_GMAC_CLK_SEL_MASK  = (3 << RK3399_GMAC_CLK_SEL_SHIFT),
	RK3399_GMAC_CLK_SEL_125M  = (0 << RK3399_GMAC_CLK_SEL_SHIFT),
	RK3399_GMAC_CLK_SEL_25M	  = (3 << RK3399_GMAC_CLK_SEL_SHIFT),
	RK3399_GMAC_CLK_SEL_2_5M  = (2 << RK3399_GMAC_CLK_SEL_SHIFT),
};

/* GRF_SOC_CON6 */
enum {
	RK3399_RXCLK_DLY_ENA_GMAC_SHIFT = 15,
	RK3399_RXCLK_DLY_ENA_GMAC_MASK =
		(1 << RK3399_RXCLK_DLY_ENA_GMAC_SHIFT),
	RK3399_RXCLK_DLY_ENA_GMAC_DISABLE = 0,
	RK3399_RXCLK_DLY_ENA_GMAC_ENABLE =
		(1 << RK3399_RXCLK_DLY_ENA_GMAC_SHIFT),

	RK3399_TXCLK_DLY_ENA_GMAC_SHIFT	= 7,
	RK3399_TXCLK_DLY_ENA_GMAC_MASK =
		(1 << RK3399_TXCLK_DLY_ENA_GMAC_SHIFT),
	RK3399_TXCLK_DLY_ENA_GMAC_DISABLE = 0,
	RK3399_TXCLK_DLY_ENA_GMAC_ENABLE =
		(1 << RK3399_TXCLK_DLY_ENA_GMAC_SHIFT),

	RK3399_CLK_RX_DL_CFG_GMAC_SHIFT = 8,
	RK3399_CLK_RX_DL_CFG_GMAC_MASK =
		(0x7f << RK3399_CLK_RX_DL_CFG_GMAC_SHIFT),

	RK3399_CLK_TX_DL_CFG_GMAC_SHIFT = 0,
	RK3399_CLK_TX_DL_CFG_GMAC_MASK =
		(0x7f << RK3399_CLK_TX_DL_CFG_GMAC_SHIFT),
};

#endif	/* __SOC_ROCKCHIP_RK3399_GRF_H__ */
