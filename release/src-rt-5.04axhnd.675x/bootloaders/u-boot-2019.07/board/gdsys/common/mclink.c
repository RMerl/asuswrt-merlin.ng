// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

#ifdef CONFIG_GDSYS_LEGACY_DRIVERS

#include <common.h>
#include <asm/io.h>
#include <errno.h>

#include <gdsys_fpga.h>

enum {
	MCINT_SLAVE_LINK_CHANGED_EV = 1 << 7,
	MCINT_TX_ERROR_EV = 1 << 9,
	MCINT_TX_BUFFER_FREE = 1 << 10,
	MCINT_TX_PACKET_TRANSMITTED_EV = 1 << 11,
	MCINT_RX_ERROR_EV = 1 << 13,
	MCINT_RX_CONTENT_AVAILABLE = 1 << 14,
	MCINT_RX_PACKET_RECEIVED_EV = 1 << 15,
};

int mclink_probe(void)
{
	unsigned int k;
	int slaves = 0;

	for (k = 0; k < CONFIG_SYS_MCLINK_MAX; ++k) {
		int timeout = 0;
		unsigned int ctr = 0;
		u16 mc_status;

		FPGA_GET_REG(k, mc_status, &mc_status);

		if (!(mc_status & (1 << 15)))
			break;

		FPGA_SET_REG(k, mc_control, 0x8000);

		FPGA_GET_REG(k, mc_status, &mc_status);
		while (!(mc_status & (1 << 14))) {
			udelay(100);
			if (ctr++ > 500) {
				timeout = 1;
				break;
			}
			FPGA_GET_REG(k, mc_status, &mc_status);
		}
		if (timeout)
			break;

		printf("waited %d us for mclink %d to come up\n", ctr * 100, k);

		slaves++;
	}

	return slaves;
}

int mclink_send(u8 slave, u16 addr, u16 data)
{
	unsigned int ctr = 0;
	u16 int_status;
	u16 rx_cmd_status;
	u16 rx_cmd;

	/* reset interrupt status */
	FPGA_GET_REG(0, mc_int, &int_status);
	FPGA_SET_REG(0, mc_int, int_status);

	/* send */
	FPGA_SET_REG(0, mc_tx_address, addr);
	FPGA_SET_REG(0, mc_tx_data, data);
	FPGA_SET_REG(0, mc_tx_cmd, (slave & 0x03) << 14);
	FPGA_SET_REG(0, mc_control, 0x8001);

	/* wait for reply */
	FPGA_GET_REG(0, mc_int, &int_status);
	while (!(int_status & MCINT_RX_PACKET_RECEIVED_EV)) {
		udelay(100);
		if (ctr++ > 3)
			return -ETIMEDOUT;
		FPGA_GET_REG(0, mc_int, &int_status);
	}

	FPGA_GET_REG(0, mc_rx_cmd_status, &rx_cmd_status);
	rx_cmd = (rx_cmd_status >> 12) & 0x03;
	if (rx_cmd != 0)
		printf("mclink_send: received cmd %d, expected %d\n", rx_cmd,
		       0);

	return 0;
}

int mclink_receive(u8 slave, u16 addr, u16 *data)
{
	u16 rx_cmd_status;
	u16 rx_cmd;
	u16 int_status;
	unsigned int ctr = 0;

	/* send read request */
	FPGA_SET_REG(0, mc_tx_address, addr);
	FPGA_SET_REG(0, mc_tx_cmd,
		     ((slave & 0x03) << 14) | (1 << 12) | (1 << 0));
	FPGA_SET_REG(0, mc_control, 0x8001);


	/* wait for reply */
	FPGA_GET_REG(0, mc_int, &int_status);
	while (!(int_status & MCINT_RX_CONTENT_AVAILABLE)) {
		udelay(100);
		if (ctr++ > 3)
			return -ETIMEDOUT;
		FPGA_GET_REG(0, mc_int, &int_status);
	}

	/* check reply */
	FPGA_GET_REG(0, mc_rx_cmd_status, &rx_cmd_status);
	if ((rx_cmd_status >> 14) != slave) {
		printf("mclink_receive: reply from slave %d, expected %d\n",
		       rx_cmd_status >> 14, slave);
		return -EINVAL;
	}

	rx_cmd = (rx_cmd_status >> 12) & 0x03;
	if (rx_cmd != 1) {
		printf("mclink_send: received cmd %d, expected %d\n",
		       rx_cmd, 1);
		return -EIO;
	}

	FPGA_GET_REG(0, mc_rx_data, data);

	return 0;
}

#endif /* CONFIG_GDSYS_LEGACY_DRIVERS */
