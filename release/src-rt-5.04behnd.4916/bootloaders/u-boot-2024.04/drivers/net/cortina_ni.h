/* SPDX-License-Identifier: GPL-2.0+ */

/*
 * Copyright (C) 2020 Cortina Access Inc.
 * Author: Aaron Tseng <aaron.tseng@cortina-access.com>
 *
 * Ethernet MAC Driver for all supported CAxxxx SoCs
 */

#ifndef __CORTINA_NI_H
#define __CORTINA_NI_H

#include <asm/types.h>
#include <asm/io.h>
#include <config.h>

#define GE_MAC_INTF_GMII                0x0
#define GE_MAC_INTF_MII                 0x1
#define GE_MAC_INTF_RGMII_1000          0x2
#define GE_MAC_INTF_RGMII_100           0x3

/* Defines the base and top address in CPU XRA
 * for packets to cpu instance 0
 * 0x300 * 8-byte = 6K-byte
 */
#define RX_TOP_ADDR			0x02FF
#define RX_BASE_ADDR			0x0000

/* Defines the base and top address in CPU XRAM
 * for packets from cpu instance 0.
 * 0x100 * 8-byte = 2K-byte
 */
#define TX_TOP_ADDR			0x03FF
#define TX_BASE_ADDR			0x0300

struct port_map_s {
	u32 phy_addr;
	u32 port;
};

struct gphy_cal_s {
	u32 reg_off;
	u32 value;
};

#if !defined(__ASSEMBLER__) && !defined(__ASSEMBLY__)
struct cortina_ni_priv {
	u32 ni_xram_base;
	u32 rx_xram_base_adr;
	u32 rx_xram_end_adr;
	u16 rx_xram_start;
	u16 rx_xram_end;
	u32 tx_xram_base_adr;
	u32 tx_xram_end_adr;
	u16 tx_xram_start;
	u16 tx_xram_end;
	u32 valid_port_map;
	u32 valid_port_num;
	u32 init_rgmii;
	u32 gphy_num;
	struct port_map_s port_map[5];
	struct gphy_cal_s gphy_values[10];
	void __iomem *glb_base_addr;
	void __iomem *per_mdio_base_addr;
	void __iomem *ni_hv_base_addr;

	struct mii_dev *mdio_bus;
	struct phy_device *phydev;
	int phy_interface;
	int active_port;
};

struct NI_HEADER_X_T {
	u32 next_link		: 10; /* bits  9: 0 */
	u32 bytes_valid		:  4; /* bits 13:10 */
	u32 reserved		: 16; /* bits 29:14 */
	u32 hdr_a		:  1; /* bits 30:30 */
	u32 ownership		:  1; /* bits 31:31 */
};

struct NI_PACKET_STATUS {
	u32 packet_size       : 14; /* bits 13:0 */
	u32 byte_valid        :  4; /* bits 17:14 */
	u32 pfc               :  1; /* bits 18:18 */
	u32 valid             :  1; /* bits 19:19 */
	u32 drop              :  1; /* bits 20:20 */
	u32 runt              :  1; /* bits 21:21 */
	u32 oversize          :  1; /* bits 22:22 */
	u32 jumbo             :  1; /* bits 23:23 */
	u32 link_status       :  1; /* bits 24:24 */
	u32 jabber            :  1; /* bits 25:25 */
	u32 crc_error         :  1; /* bits 26:26 */
	u32 pause             :  1; /* bits 27:27 */
	u32 oam               :  1; /* bits 28:28 */
	u32 unknown_opcode    :  1; /* bits 29:29 */
	u32 multicast         :  1; /* bits 30:30 */
	u32 broadcast         :  1; /* bits 31:31 */
};

struct NI_MDIO_OPER_T {
	u32 reserved       : 2; /* bits  1:0 */
	u32 reg_off        : 5; /* bits  6:2 */
	u32 phy_addr       : 5; /* bits 11:7 */
	u32 reg_base       : 20; /* bits 31:12 */
};

#define __MDIO_WR_FLAG				(0)
#define __MDIO_RD_FLAG				(1)
#define __MDIO_ACCESS_TIMEOUT			(1000000)
#define CA_MDIO_ADDR_MIN			(1)
#define CA_MDIO_ADDR_MAX			(31)

#endif /* !__ASSEMBLER__ */

/* HW REG */
struct NI_HV_GLB_MAC_ADDR_CFG0_t {
	u32 mac_addr0            : 32; /* bits 31:0 */
};

struct NI_HV_GLB_MAC_ADDR_CFG1_t {
	u32 mac_addr1            :  8; /* bits 7:0 */
	u32 rsrvd1               : 24;
};

struct NI_HV_PT_PORT_STATIC_CFG_t {
	u32 int_cfg              :  4; /* bits 3:0 */
	u32 phy_mode             :  1; /* bits 4:4 */
	u32 rmii_clksrc          :  1; /* bits 5:5 */
	u32 inv_clk_in           :  1; /* bits 6:6 */
	u32 inv_clk_out          :  1; /* bits 7:7 */
	u32 inv_rxclk_out        :  1; /* bits 8:8 */
	u32 tx_use_gefifo        :  1; /* bits 9:9 */
	u32 smii_tx_stat         :  1; /* bits 10:10 */
	u32 crs_polarity         :  1; /* bits 11:11 */
	u32 lpbk_mode            :  2; /* bits 13:12 */
	u32 gmii_like_half_duplex_en :  1; /* bits 14:14 */
	u32 sup_tx_to_rx_lpbk_data :  1; /* bits 15:15 */
	u32 rsrvd1               :  8;
	u32 mac_addr6            :  8; /* bits 31:24 */
};

struct NI_HV_XRAM_CPUXRAM_CFG_t {
	u32 rx_0_cpu_pkt_dis     :  1; /* bits 0:0 */
	u32 rsrvd1               :  8;
	u32 tx_0_cpu_pkt_dis     :  1; /* bits 9:9 */
	u32 rsrvd2               :  1;
	u32 rx_x_drop_err_pkt    :  1; /* bits 11:11 */
	u32 xram_mgmt_dis_drop_ovsz_pkt :  1; /* bits 12:12 */
	u32 xram_mgmt_term_large_pkt :  1; /* bits 13:13 */
	u32 xram_mgmt_promisc_mode :  2; /* bits 15:14 */
	u32 xram_cntr_debug_mode :  1; /* bits 16:16 */
	u32 xram_cntr_op_code    :  2; /* bits 18:17 */
	u32 rsrvd3               :  2;
	u32 xram_rx_mgmtfifo_srst :  1; /* bits 21:21 */
	u32 xram_dma_fifo_srst   :  1; /* bits 22:22 */
	u32 rsrvd4               :  9;
};

struct NI_HV_PT_RXMAC_CFG_t {
	u32 rx_en                :  1; /* bits 0:0 */
	u32 rsrvd1               :  7;
	u32 rx_flow_disable      :  1; /* bits 8:8 */
	u32 rsrvd2               :  3;
	u32 rx_flow_to_tx_en     :  1; /* bits 12:12 */
	u32 rx_pfc_disable       :  1; /* bits 13:13 */
	u32 rsrvd3               : 15;
	u32 send_pg_data         :  1; /* bits 29:29 */
	u32 rsrvd4               :  2;
};

struct NI_HV_PT_TXMAC_CFG_t {
	u32 tx_en                :  1; /* bits 0:0 */
	u32 rsrvd1               :  7;
	u32 mac_crc_calc_en      :  1; /* bits 8:8 */
	u32 tx_ipg_sel           :  3; /* bits 11:9 */
	u32 tx_flow_disable      :  1; /* bits 12:12 */
	u32 tx_drain             :  1; /* bits 13:13 */
	u32 tx_pfc_disable       :  1; /* bits 14:14 */
	u32 tx_pau_sel           :  2; /* bits 16:15 */
	u32 rsrvd2               :  9;
	u32 tx_auto_xon          :  1; /* bits 26:26 */
	u32 rsrvd3               :  1;
	u32 pass_thru_hdr        :  1; /* bits 28:28 */
	u32 rsrvd4               :  3;
};

struct NI_HV_GLB_INTF_RST_CONFIG_t {
	u32 intf_rst_p0          :  1; /* bits 0:0 */
	u32 intf_rst_p1          :  1; /* bits 1:1 */
	u32 intf_rst_p2          :  1; /* bits 2:2 */
	u32 intf_rst_p3          :  1; /* bits 3:3 */
	u32 intf_rst_p4          :  1; /* bits 4:4 */
	u32 mac_rx_rst_p0        :  1; /* bits 5:5 */
	u32 mac_rx_rst_p1        :  1; /* bits 6:6 */
	u32 mac_rx_rst_p2        :  1; /* bits 7:7 */
	u32 mac_rx_rst_p3        :  1; /* bits 8:8 */
	u32 mac_rx_rst_p4        :  1; /* bits 9:9 */
	u32 mac_tx_rst_p0        :  1; /* bits 10:10 */
	u32 mac_tx_rst_p1        :  1; /* bits 11:11 */
	u32 mac_tx_rst_p2        :  1; /* bits 12:12 */
	u32 mac_tx_rst_p3        :  1; /* bits 13:13 */
	u32 mac_tx_rst_p4        :  1; /* bits 14:14 */
	u32 port_rst_p5          :  1; /* bits 15:15 */
	u32 pcs_rst_p6           :  1; /* bits 16:16 */
	u32 pcs_rst_p7           :  1; /* bits 17:17 */
	u32 mac_rst_p6           :  1; /* bits 18:18 */
	u32 mac_rst_p7           :  1; /* bits 19:19 */
	u32 rsrvd1               : 12;
};

struct NI_HV_GLB_STATIC_CFG_t {
	u32 port_to_cpu          :  4; /* bits 3:0 */
	u32 mgmt_pt_to_fe_also   :  1; /* bits 4:4 */
	u32 txcrc_chk_en         :  1; /* bits 5:5 */
	u32 p4_rgmii_tx_clk_phase :  2; /* bits 7:6 */
	u32 p4_rgmii_tx_data_order :  1; /* bits 8:8 */
	u32 rsrvd1               :  7;
	u32 rxmib_mode           :  1; /* bits 16:16 */
	u32 txmib_mode           :  1; /* bits 17:17 */
	u32 eth_sch_rdy_pkt      :  1; /* bits 18:18 */
	u32 rsrvd2               :  1;
	u32 rxaui_mode           :  2; /* bits 21:20 */
	u32 rxaui_sigdet         :  2; /* bits 23:22 */
	u32 cnt_op_mode          :  3; /* bits 26:24 */
	u32 rsrvd3               :  5;
};

struct GLOBAL_BLOCK_RESET_t {
	u32 reset_ni             :  1; /* bits 0:0 */
	u32 reset_l2fe           :  1; /* bits 1:1 */
	u32 reset_l2tm           :  1; /* bits 2:2 */
	u32 reset_l3fe           :  1; /* bits 3:3 */
	u32 reset_sdram          :  1; /* bits 4:4 */
	u32 reset_tqm            :  1; /* bits 5:5 */
	u32 reset_pcie0          :  1; /* bits 6:6 */
	u32 reset_pcie1          :  1; /* bits 7:7 */
	u32 reset_pcie2          :  1; /* bits 8:8 */
	u32 reset_sata           :  1; /* bits 9:9 */
	u32 reset_gic400         :  1; /* bits 10:10 */
	u32 rsrvd1               :  2;
	u32 reset_usb            :  1; /* bits 13:13 */
	u32 reset_flash          :  1; /* bits 14:14 */
	u32 reset_per            :  1; /* bits 15:15 */
	u32 reset_dma            :  1; /* bits 16:16 */
	u32 reset_rtc            :  1; /* bits 17:17 */
	u32 reset_pe0            :  1; /* bits 18:18 */
	u32 reset_pe1            :  1; /* bits 19:19 */
	u32 reset_rcpu0          :  1; /* bits 20:20 */
	u32 reset_rcpu1          :  1; /* bits 21:21 */
	u32 reset_sadb           :  1; /* bits 22:22 */
	u32 rsrvd2               :  1;
	u32 reset_rcrypto        :  1; /* bits 24:24 */
	u32 reset_ldma           :  1; /* bits 25:25 */
	u32 reset_fbm            :  1; /* bits 26:26 */
	u32 reset_eaxi           :  1; /* bits 27:27 */
	u32 reset_sd             :  1; /* bits 28:28 */
	u32 reset_otprom         :  1; /* bits 29:29 */
	u32 rsrvd3               :  2;
};

struct PER_MDIO_ADDR_t {
	u32 mdio_addr            :  5; /* bits 4:0 */
	u32 rsrvd1               :  3;
	u32 mdio_offset          :  5; /* bits 12:8 */
	u32 rsrvd2               :  2;
	u32 mdio_rd_wr           :  1; /* bits 15:15 */
	u32 mdio_st              :  1; /* bits 16:16 */
	u32 rsrvd3               :  1;
	u32 mdio_op              :  2; /* bits 19:18 */
	u32 rsrvd4               : 12;
};

struct PER_MDIO_CTRL_t {
	u32 mdiodone             :  1; /* bits 0:0 */
	u32 rsrvd1               :  6;
	u32 mdiostart            :  1; /* bits 7:7 */
	u32 rsrvd2               : 24;
};

struct PER_MDIO_RDDATA_t {
	u32 mdio_rddata          : 16; /* bits 15:0 */
	u32 rsrvd1               : 16;
};

/* XRAM */

struct NI_HV_XRAM_CPUXRAM_ADRCFG_RX_t {
	u32 rx_base_addr         : 10; /* bits 9:0 */
	u32 rsrvd1               :  6;
	u32 rx_top_addr          : 10; /* bits 25:16 */
	u32 rsrvd2               :  6;
};

struct NI_HV_XRAM_CPUXRAM_ADRCFG_TX_0_t {
	u32 tx_base_addr         : 10; /* bits 9:0 */
	u32 rsrvd1               :  6;
	u32 tx_top_addr          : 10; /* bits 25:16 */
	u32 rsrvd2               :  6;
};

struct NI_HV_XRAM_CPUXRAM_CPU_STA_RX_0_t {
	u32 pkt_wr_ptr           : 10; /* bits 9:0 */
	u32 rsrvd1               :  5;
	u32 int_colsc_thresh_reached :  1; /* bits 15:15 */
	u32 rsrvd2               : 16;
};

struct NI_HV_XRAM_CPUXRAM_CPU_CFG_RX_0_t {
	u32 pkt_rd_ptr           : 10; /* bits 9:0 */
	u32 rsrvd1               : 22;
};

struct NI_HV_XRAM_CPUXRAM_CPU_CFG_TX_0_t {
	u32 pkt_wr_ptr           : 10; /* bits 9:0 */
	u32 rsrvd1               : 22;
};

struct GLOBAL_GLOBAL_CONFIG_t {
	u32 rsrvd1               :  4;
	u32 wd_reset_subsys_enable :  1; /* bits 4:4 */
	u32 rsrvd2               :  1;
	u32 wd_reset_all_blocks  :  1; /* bits 6:6 */
	u32 wd_reset_remap       :  1; /* bits 7:7 */
	u32 wd_reset_ext_reset   :  1; /* bits 8:8 */
	u32 ext_reset            :  1; /* bits 9:9 */
	u32 cfg_pcie_0_clken     :  1; /* bits 10:10 */
	u32 cfg_sata_clken       :  1; /* bits 11:11 */
	u32 cfg_pcie_1_clken     :  1; /* bits 12:12 */
	u32 rsrvd3               :  1;
	u32 cfg_pcie_2_clken     :  1; /* bits 14:14 */
	u32 rsrvd4               :  2;
	u32 ext_eth_refclk       :  1; /* bits 17:17 */
	u32 refclk_sel           :  2; /* bits 19:18 */
	u32 rsrvd5               :  7;
	u32 l3fe_pd              :  1; /* bits 27:27 */
	u32 offload0_pd          :  1; /* bits 28:28 */
	u32 offload1_pd          :  1; /* bits 29:29 */
	u32 crypto_pd            :  1; /* bits 30:30 */
	u32 core_pd              :  1; /* bits 31:31 */
};

struct GLOBAL_IO_DRIVE_CONTROL_t {
	u32 gmac_dp              :  3; /* bits 2:0 */
	u32 gmac_dn              :  3; /* bits 5:3 */
	u32 gmac_mode            :  2; /* bits 7:6 */
	u32 gmac_ds              :  1; /* bits 8:8 */
	u32 flash_ds             :  1; /* bits 9:9 */
	u32 nu_ds                :  1; /* bits 10:10 */
	u32 ssp_ds               :  1; /* bits 11:11 */
	u32 spi_ds               :  1; /* bits 12:12 */
	u32 gpio_ds              :  1; /* bits 13:13 */
	u32 misc_ds              :  1; /* bits 14:14 */
	u32 eaxi_ds              :  1; /* bits 15:15 */
	u32 sd_ds                :  8; /* bits 23:16 */
	u32 rsrvd1               :  8;
};

struct NI_HV_GLB_INIT_DONE_t {
	u32 rsrvd1               :  1;
	u32 ni_init_done         :  1; /* bits 1:1 */
	u32 rsrvd2               : 30;
};

struct NI_HV_PT_PORT_GLB_CFG_t {
	u32 speed                :  1; /* bits 0:0 */
	u32 duplex               :  1; /* bits 1:1 */
	u32 link_status          :  1; /* bits 2:2 */
	u32 link_stat_mask       :  1; /* bits 3:3 */
	u32 rsrvd1               :  7;
	u32 power_dwn_rx         :  1; /* bits 11:11 */
	u32 power_dwn_tx         :  1; /* bits 12:12 */
	u32 tx_intf_lp_time      :  1; /* bits 13:13 */
	u32 rsrvd2               : 18;
};

#define NI_HV_GLB_INIT_DONE_OFFSET                      0x004
#define NI_HV_GLB_INTF_RST_CONFIG_OFFSET                0x008
#define NI_HV_GLB_STATIC_CFG_OFFSET                     0x00c

#define NI_HV_PT_PORT_STATIC_CFG_OFFSET                 NI_HV_PT_BASE
#define NI_HV_PT_PORT_GLB_CFG_OFFSET                    (0x4 + NI_HV_PT_BASE)
#define NI_HV_PT_RXMAC_CFG_OFFSET                       (0x8 + NI_HV_PT_BASE)
#define NI_HV_PT_TXMAC_CFG_OFFSET                       (0x14 + NI_HV_PT_BASE)

#define NI_HV_XRAM_CPUXRAM_ADRCFG_RX_OFFSET             NI_HV_XRAM_BASE
#define NI_HV_XRAM_CPUXRAM_ADRCFG_TX_0_OFFSET           (0x4 + NI_HV_XRAM_BASE)
#define NI_HV_XRAM_CPUXRAM_CFG_OFFSET                   (0x8 + NI_HV_XRAM_BASE)
#define NI_HV_XRAM_CPUXRAM_CPU_CFG_RX_0_OFFSET          (0xc + NI_HV_XRAM_BASE)
#define NI_HV_XRAM_CPUXRAM_CPU_STA_RX_0_OFFSET          (0x10 + NI_HV_XRAM_BASE)
#define NI_HV_XRAM_CPUXRAM_CPU_CFG_TX_0_OFFSET          (0x24 + NI_HV_XRAM_BASE)
#define NI_HV_XRAM_CPUXRAM_CPU_STAT_TX_0_OFFSET         (0x28 + NI_HV_XRAM_BASE)

#define PER_MDIO_CFG_OFFSET                             0x00
#define PER_MDIO_ADDR_OFFSET                            0x04
#define PER_MDIO_WRDATA_OFFSET                          0x08
#define PER_MDIO_RDDATA_OFFSET                          0x0C
#define PER_MDIO_CTRL_OFFSET                            0x10

#define APB0_NI_HV_PT_STRIDE				160

#endif /* __CORTINA_NI_H */
