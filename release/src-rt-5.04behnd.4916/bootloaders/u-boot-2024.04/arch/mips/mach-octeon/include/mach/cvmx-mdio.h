/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Interface to the SMI/MDIO hardware, including support for both IEEE 802.3
 * clause 22 and clause 45 operations.
 */

#ifndef __CVMX_MIO_H__
#define __CVMX_MIO_H__

/**
 * PHY register 0 from the 802.3 spec
 */
#define CVMX_MDIO_PHY_REG_CONTROL 0

typedef union {
	u16 u16;
	struct {
		u16 reset : 1;
		u16 loopback : 1;
		u16 speed_lsb : 1;
		u16 autoneg_enable : 1;
		u16 power_down : 1;
		u16 isolate : 1;
		u16 restart_autoneg : 1;
		u16 duplex : 1;
		u16 collision_test : 1;
		u16 speed_msb : 1;
		u16 unidirectional_enable : 1;
		u16 reserved_0_4 : 5;
	} s;
} cvmx_mdio_phy_reg_control_t;

/**
 * PHY register 1 from the 802.3 spec
 */
#define CVMX_MDIO_PHY_REG_STATUS 1
typedef union {
	u16 u16;
	struct {
		u16 capable_100base_t4 : 1;
		u16 capable_100base_x_full : 1;
		u16 capable_100base_x_half : 1;
		u16 capable_10_full : 1;
		u16 capable_10_half : 1;
		u16 capable_100base_t2_full : 1;
		u16 capable_100base_t2_half : 1;
		u16 capable_extended_status : 1;
		u16 capable_unidirectional : 1;
		u16 capable_mf_preamble_suppression : 1;
		u16 autoneg_complete : 1;
		u16 remote_fault : 1;
		u16 capable_autoneg : 1;
		u16 link_status : 1;
		u16 jabber_detect : 1;
		u16 capable_extended_registers : 1;

	} s;
} cvmx_mdio_phy_reg_status_t;

/**
 * PHY register 2 from the 802.3 spec
 */
#define CVMX_MDIO_PHY_REG_ID1 2
typedef union {
	u16 u16;
	struct {
		u16 oui_bits_3_18;
	} s;
} cvmx_mdio_phy_reg_id1_t;

/**
 * PHY register 3 from the 802.3 spec
 */
#define CVMX_MDIO_PHY_REG_ID2 3
typedef union {
	u16 u16;
	struct {
		u16 oui_bits_19_24 : 6;
		u16 model : 6;
		u16 revision : 4;
	} s;
} cvmx_mdio_phy_reg_id2_t;

/**
 * PHY register 4 from the 802.3 spec
 */
#define CVMX_MDIO_PHY_REG_AUTONEG_ADVER 4
typedef union {
	u16 u16;
	struct {
		u16 next_page : 1;
		u16 reserved_14 : 1;
		u16 remote_fault : 1;
		u16 reserved_12 : 1;
		u16 asymmetric_pause : 1;
		u16 pause : 1;
		u16 advert_100base_t4 : 1;
		u16 advert_100base_tx_full : 1;
		u16 advert_100base_tx_half : 1;
		u16 advert_10base_tx_full : 1;
		u16 advert_10base_tx_half : 1;
		u16 selector : 5;
	} s;
} cvmx_mdio_phy_reg_autoneg_adver_t;

/**
 * PHY register 5 from the 802.3 spec
 */
#define CVMX_MDIO_PHY_REG_LINK_PARTNER_ABILITY 5
typedef union {
	u16 u16;
	struct {
		u16 next_page : 1;
		u16 ack : 1;
		u16 remote_fault : 1;
		u16 reserved_12 : 1;
		u16 asymmetric_pause : 1;
		u16 pause : 1;
		u16 advert_100base_t4 : 1;
		u16 advert_100base_tx_full : 1;
		u16 advert_100base_tx_half : 1;
		u16 advert_10base_tx_full : 1;
		u16 advert_10base_tx_half : 1;
		u16 selector : 5;
	} s;
} cvmx_mdio_phy_reg_link_partner_ability_t;

/**
 * PHY register 6 from the 802.3 spec
 */
#define CVMX_MDIO_PHY_REG_AUTONEG_EXPANSION 6
typedef union {
	u16 u16;
	struct {
		u16 reserved_5_15 : 11;
		u16 parallel_detection_fault : 1;
		u16 link_partner_next_page_capable : 1;
		u16 local_next_page_capable : 1;
		u16 page_received : 1;
		u16 link_partner_autoneg_capable : 1;

	} s;
} cvmx_mdio_phy_reg_autoneg_expansion_t;

/**
 * PHY register 9 from the 802.3 spec
 */
#define CVMX_MDIO_PHY_REG_CONTROL_1000 9
typedef union {
	u16 u16;
	struct {
		u16 test_mode : 3;
		u16 manual_master_slave : 1;
		u16 master : 1;
		u16 port_type : 1;
		u16 advert_1000base_t_full : 1;
		u16 advert_1000base_t_half : 1;
		u16 reserved_0_7 : 8;
	} s;
} cvmx_mdio_phy_reg_control_1000_t;

/**
 * PHY register 10 from the 802.3 spec
 */
#define CVMX_MDIO_PHY_REG_STATUS_1000 10
typedef union {
	u16 u16;
	struct {
		u16 master_slave_fault : 1;
		u16 is_master : 1;
		u16 local_receiver_ok : 1;
		u16 remote_receiver_ok : 1;
		u16 remote_capable_1000base_t_full : 1;
		u16 remote_capable_1000base_t_half : 1;
		u16 reserved_8_9 : 2;
		u16 idle_error_count : 8;
	} s;
} cvmx_mdio_phy_reg_status_1000_t;

/**
 * PHY register 15 from the 802.3 spec
 */
#define CVMX_MDIO_PHY_REG_EXTENDED_STATUS 15
typedef union {
	u16 u16;
	struct {
		u16 capable_1000base_x_full : 1;
		u16 capable_1000base_x_half : 1;
		u16 capable_1000base_t_full : 1;
		u16 capable_1000base_t_half : 1;
		u16 reserved_0_11 : 12;
	} s;
} cvmx_mdio_phy_reg_extended_status_t;

/**
 * PHY register 13 from the 802.3 spec
 */
#define CVMX_MDIO_PHY_REG_MMD_CONTROL 13
typedef union {
	u16 u16;
	struct {
		u16 function : 2;
		u16 reserved_5_13 : 9;
		u16 devad : 5;
	} s;
} cvmx_mdio_phy_reg_mmd_control_t;

/**
 * PHY register 14 from the 802.3 spec
 */
#define CVMX_MDIO_PHY_REG_MMD_ADDRESS_DATA 14
typedef union {
	u16 u16;
	struct {
		u16 address_data : 16;
	} s;
} cvmx_mdio_phy_reg_mmd_address_data_t;

/* Operating request encodings. */
#define MDIO_CLAUSE_22_WRITE 0
#define MDIO_CLAUSE_22_READ  1

#define MDIO_CLAUSE_45_ADDRESS	0
#define MDIO_CLAUSE_45_WRITE	1
#define MDIO_CLAUSE_45_READ_INC 2
#define MDIO_CLAUSE_45_READ	3

/* MMD identifiers, mostly for accessing devices within XENPAK modules. */
#define CVMX_MMD_DEVICE_PMA_PMD	 1
#define CVMX_MMD_DEVICE_WIS	 2
#define CVMX_MMD_DEVICE_PCS	 3
#define CVMX_MMD_DEVICE_PHY_XS	 4
#define CVMX_MMD_DEVICE_DTS_XS	 5
#define CVMX_MMD_DEVICE_TC	 6
#define CVMX_MMD_DEVICE_CL22_EXT 29
#define CVMX_MMD_DEVICE_VENDOR_1 30
#define CVMX_MMD_DEVICE_VENDOR_2 31

#define CVMX_MDIO_TIMEOUT 100000 /* 100 millisec */

static inline int cvmx_mdio_bus_id_to_node(int bus_id)
{
	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		return (bus_id >> 2) & CVMX_NODE_MASK;
	else
		return 0;
}

static inline int cvmx_mdio_bus_id_to_bus(int bus_id)
{
	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		return bus_id & 3;
	else
		return bus_id;
}

/* Helper function to put MDIO interface into clause 45 mode */
static inline void __cvmx_mdio_set_clause45_mode(int bus_id)
{
	cvmx_smix_clk_t smi_clk;
	int node = cvmx_mdio_bus_id_to_node(bus_id);
	int bus = cvmx_mdio_bus_id_to_bus(bus_id);

	/* Put bus into clause 45 mode */
	smi_clk.u64 = csr_rd_node(node, CVMX_SMIX_CLK(bus));
	smi_clk.s.mode = 1;
	smi_clk.s.preamble = 1;
	csr_wr_node(node, CVMX_SMIX_CLK(bus), smi_clk.u64);
}

/* Helper function to put MDIO interface into clause 22 mode */
static inline void __cvmx_mdio_set_clause22_mode(int bus_id)
{
	cvmx_smix_clk_t smi_clk;
	int node = cvmx_mdio_bus_id_to_node(bus_id);
	int bus = cvmx_mdio_bus_id_to_bus(bus_id);

	/* Put bus into clause 22 mode */
	smi_clk.u64 = csr_rd_node(node, CVMX_SMIX_CLK(bus));
	smi_clk.s.mode = 0;
	csr_wr_node(node, CVMX_SMIX_CLK(bus), smi_clk.u64);
}

/**
 * @INTERNAL
 * Function to read SMIX_RD_DAT and check for timeouts. This
 * code sequence is done fairly often, so put in one spot.
 *
 * @param bus_id SMI/MDIO bus to read
 *
 * @return Value of SMIX_RD_DAT. pending will be set on
 *         a timeout.
 */
static inline cvmx_smix_rd_dat_t __cvmx_mdio_read_rd_dat(int bus_id)
{
	cvmx_smix_rd_dat_t smi_rd;
	int node = cvmx_mdio_bus_id_to_node(bus_id);
	int bus = cvmx_mdio_bus_id_to_bus(bus_id);
	u64 done;

	done = get_timer(0);

	do {
		mdelay(1);
		smi_rd.u64 = csr_rd_node(node, CVMX_SMIX_RD_DAT(bus));
		if (get_timer(done) > (CVMX_MDIO_TIMEOUT / 1000))
			break;
	} while (smi_rd.s.pending);

	return smi_rd;
}

/**
 * Perform an MII read. This function is used to read PHY
 * registers controlling auto negotiation.
 *
 * @param bus_id   MDIO bus number. Zero on most chips, but some chips (ex CN56XX)
 *                 support multiple busses.
 * @param phy_id   The MII phy id
 * @param location Register location to read
 *
 * @return Result from the read or -1 on failure
 */
static inline int cvmx_mdio_read(int bus_id, int phy_id, int location)
{
	int node = cvmx_mdio_bus_id_to_node(bus_id);
	int bus = cvmx_mdio_bus_id_to_bus(bus_id);
	cvmx_smix_cmd_t smi_cmd;
	cvmx_smix_rd_dat_t smi_rd;

	if (octeon_has_feature(OCTEON_FEATURE_MDIO_CLAUSE_45))
		__cvmx_mdio_set_clause22_mode(bus_id);

	smi_cmd.u64 = 0;
	smi_cmd.s.phy_op = MDIO_CLAUSE_22_READ;
	smi_cmd.s.phy_adr = phy_id;
	smi_cmd.s.reg_adr = location;
	csr_wr_node(node, CVMX_SMIX_CMD(bus), smi_cmd.u64);

	smi_rd = __cvmx_mdio_read_rd_dat(bus_id);
	if (smi_rd.s.val)
		return smi_rd.s.dat;
	else
		return -1;
}

/**
 * Perform an MII write. This function is used to write PHY
 * registers controlling auto negotiation.
 *
 * @param bus_id   MDIO bus number. Zero on most chips, but some chips (ex CN56XX)
 *                 support multiple busses.
 * @param phy_id   The MII phy id
 * @param location Register location to write
 * @param val      Value to write
 *
 * @return -1 on error
 *         0 on success
 */
static inline int cvmx_mdio_write(int bus_id, int phy_id, int location, int val)
{
	int node = cvmx_mdio_bus_id_to_node(bus_id);
	int bus = cvmx_mdio_bus_id_to_bus(bus_id);
	cvmx_smix_cmd_t smi_cmd;
	cvmx_smix_wr_dat_t smi_wr;

	if (octeon_has_feature(OCTEON_FEATURE_MDIO_CLAUSE_45))
		__cvmx_mdio_set_clause22_mode(bus_id);

	smi_wr.u64 = 0;
	smi_wr.s.dat = val;
	csr_wr_node(node, CVMX_SMIX_WR_DAT(bus), smi_wr.u64);

	smi_cmd.u64 = 0;
	smi_cmd.s.phy_op = MDIO_CLAUSE_22_WRITE;
	smi_cmd.s.phy_adr = phy_id;
	smi_cmd.s.reg_adr = location;
	csr_wr_node(node, CVMX_SMIX_CMD(bus), smi_cmd.u64);

	if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_SMIX_WR_DAT(bus),
				       cvmx_smix_wr_dat_t, pending, ==, 0,
				       CVMX_MDIO_TIMEOUT))
		return -1;

	return 0;
}

/**
 * Perform an IEEE 802.3 clause 45 MII read. This function is used to read PHY
 * registers controlling auto negotiation.
 *
 * @param bus_id   MDIO bus number. Zero on most chips, but some chips (ex CN56XX)
 *                 support multiple busses.
 * @param phy_id   The MII phy id
 * @param device   MDIO Manageable Device (MMD) id
 * @param location Register location to read
 *
 * @return Result from the read or -1 on failure
 */

static inline int cvmx_mdio_45_read(int bus_id, int phy_id, int device,
				    int location)
{
	cvmx_smix_cmd_t smi_cmd;
	cvmx_smix_rd_dat_t smi_rd;
	cvmx_smix_wr_dat_t smi_wr;
	int node = cvmx_mdio_bus_id_to_node(bus_id);
	int bus = cvmx_mdio_bus_id_to_bus(bus_id);

	if (!octeon_has_feature(OCTEON_FEATURE_MDIO_CLAUSE_45))
		return -1;

	__cvmx_mdio_set_clause45_mode(bus_id);

	smi_wr.u64 = 0;
	smi_wr.s.dat = location;
	csr_wr_node(node, CVMX_SMIX_WR_DAT(bus), smi_wr.u64);

	smi_cmd.u64 = 0;
	smi_cmd.s.phy_op = MDIO_CLAUSE_45_ADDRESS;
	smi_cmd.s.phy_adr = phy_id;
	smi_cmd.s.reg_adr = device;
	csr_wr_node(node, CVMX_SMIX_CMD(bus), smi_cmd.u64);

	if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_SMIX_WR_DAT(bus),
				       cvmx_smix_wr_dat_t, pending, ==, 0,
				       CVMX_MDIO_TIMEOUT)) {
		debug("cvmx_mdio_45_read: bus_id %d phy_id %2d device %2d register %2d   TIME OUT(address)\n",
		      bus_id, phy_id, device, location);
		return -1;
	}

	smi_cmd.u64 = 0;
	smi_cmd.s.phy_op = MDIO_CLAUSE_45_READ;
	smi_cmd.s.phy_adr = phy_id;
	smi_cmd.s.reg_adr = device;
	csr_wr_node(node, CVMX_SMIX_CMD(bus), smi_cmd.u64);

	smi_rd = __cvmx_mdio_read_rd_dat(bus_id);
	if (smi_rd.s.pending) {
		debug("cvmx_mdio_45_read: bus_id %d phy_id %2d device %2d register %2d   TIME OUT(data)\n",
		      bus_id, phy_id, device, location);
		return -1;
	}

	if (smi_rd.s.val)
		return smi_rd.s.dat;

	debug("cvmx_mdio_45_read: bus_id %d phy_id %2d device %2d register %2d   INVALID READ\n",
	      bus_id, phy_id, device, location);
	return -1;
}

/**
 * Perform an IEEE 802.3 clause 45 MII write. This function is used to write PHY
 * registers controlling auto negotiation.
 *
 * @param bus_id   MDIO bus number. Zero on most chips, but some chips (ex CN56XX)
 *                 support multiple busses.
 * @param phy_id   The MII phy id
 * @param device   MDIO Manageable Device (MMD) id
 * @param location Register location to write
 * @param val      Value to write
 *
 * @return -1 on error
 *         0 on success
 */
static inline int cvmx_mdio_45_write(int bus_id, int phy_id, int device,
				     int location, int val)
{
	cvmx_smix_cmd_t smi_cmd;
	cvmx_smix_wr_dat_t smi_wr;
	int node = cvmx_mdio_bus_id_to_node(bus_id);
	int bus = cvmx_mdio_bus_id_to_bus(bus_id);

	if (!octeon_has_feature(OCTEON_FEATURE_MDIO_CLAUSE_45))
		return -1;

	__cvmx_mdio_set_clause45_mode(bus_id);

	smi_wr.u64 = 0;
	smi_wr.s.dat = location;
	csr_wr_node(node, CVMX_SMIX_WR_DAT(bus), smi_wr.u64);

	smi_cmd.u64 = 0;
	smi_cmd.s.phy_op = MDIO_CLAUSE_45_ADDRESS;
	smi_cmd.s.phy_adr = phy_id;
	smi_cmd.s.reg_adr = device;
	csr_wr_node(node, CVMX_SMIX_CMD(bus), smi_cmd.u64);

	if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_SMIX_WR_DAT(bus),
				       cvmx_smix_wr_dat_t, pending, ==, 0,
				       CVMX_MDIO_TIMEOUT))
		return -1;

	smi_wr.u64 = 0;
	smi_wr.s.dat = val;
	csr_wr_node(node, CVMX_SMIX_WR_DAT(bus), smi_wr.u64);

	smi_cmd.u64 = 0;
	smi_cmd.s.phy_op = MDIO_CLAUSE_45_WRITE;
	smi_cmd.s.phy_adr = phy_id;
	smi_cmd.s.reg_adr = device;
	csr_wr_node(node, CVMX_SMIX_CMD(bus), smi_cmd.u64);

	if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_SMIX_WR_DAT(bus),
				       cvmx_smix_wr_dat_t, pending, ==, 0,
				       CVMX_MDIO_TIMEOUT))
		return -1;

	return 0;
}

#endif
