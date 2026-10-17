// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Dirk Eibach, Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

#include <common.h>
#include <command.h>
#include <console.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#include <gdsys_fpga.h>

#ifndef CONFIG_GDSYS_LEGACY_DRIVERS
#include <dm.h>
#include <misc.h>
#include <regmap.h>
#include <sysinfo.h>

#include "../../../drivers/misc/gdsys_soc.h"
#include "../../../drivers/misc/gdsys_ioep.h"
#include "../../../drivers/misc/ihs_fpga.h"

const int HEADER_WORDS = sizeof(struct io_generic_packet) / 2;
#endif /* !CONFIG_GDSYS_LEGACY_DRIVERS */

enum status_print_type {
	STATUS_LOUD = 0,
	STATUS_SILENT = 1,
};

#ifdef CONFIG_GDSYS_LEGACY_DRIVERS
enum {
	STATE_TX_PACKET_BUILDING = BIT(0),
	STATE_TX_TRANSMITTING = BIT(1),
	STATE_TX_BUFFER_FULL = BIT(2),
	STATE_TX_ERR = BIT(3),
	STATE_RECEIVE_TIMEOUT = BIT(4),
	STATE_PROC_RX_STORE_TIMEOUT = BIT(5),
	STATE_PROC_RX_RECEIVE_TIMEOUT = BIT(6),
	STATE_RX_DIST_ERR = BIT(7),
	STATE_RX_LENGTH_ERR = BIT(8),
	STATE_RX_FRAME_CTR_ERR = BIT(9),
	STATE_RX_FCS_ERR = BIT(10),
	STATE_RX_PACKET_DROPPED = BIT(11),
	STATE_RX_DATA_LAST = BIT(12),
	STATE_RX_DATA_FIRST = BIT(13),
	STATE_RX_DATA_AVAILABLE = BIT(15),
};

enum {
	IRQ_CPU_TRANSMITBUFFER_FREE_STATUS = BIT(5),
	IRQ_CPU_PACKET_TRANSMITTED_EVENT = BIT(6),
	IRQ_NEW_CPU_PACKET_RECEIVED_EVENT = BIT(7),
	IRQ_CPU_RECEIVE_DATA_AVAILABLE_STATUS = BIT(8),
};

enum {
	CTRL_PROC_RECEIVE_ENABLE = BIT(12),
	CTRL_FLUSH_TRANSMIT_BUFFER = BIT(15),
};

struct io_generic_packet {
	u16 target_address;
	u16 source_address;
	u8 packet_type;
	u8 bc;
	u16 packet_length;
} __attribute__((__packed__));
#endif /* CONFIG_GDSYS_LEGACY_DRIVERS */

unsigned long long rx_ctr;
unsigned long long tx_ctr;
unsigned long long err_ctr;
#ifndef CONFIG_GDSYS_LEGACY_DRIVERS
struct udevice *dev;
#endif /* !CONFIG_GDSYS_LEGACY_DRIVERS */

#ifdef CONFIG_GDSYS_LEGACY_DRIVERS
static void io_check_status(uint fpga, u16 status, enum status_print_type type)
{
	u16 mask = STATE_RX_DIST_ERR | STATE_RX_LENGTH_ERR |
		   STATE_RX_FRAME_CTR_ERR | STATE_RX_FCS_ERR |
		   STATE_RX_PACKET_DROPPED | STATE_TX_ERR;

	if (!(status & mask)) {
		FPGA_SET_REG(fpga, ep.rx_tx_status, status);
		return;
	}

	err_ctr++;
	FPGA_SET_REG(fpga, ep.rx_tx_status, status);

	if (type == STATUS_SILENT)
		return;

	if (status & STATE_RX_PACKET_DROPPED)
		printf("RX_PACKET_DROPPED, status %04x\n", status);

	if (status & STATE_RX_DIST_ERR)
		printf("RX_DIST_ERR\n");
	if (status & STATE_RX_LENGTH_ERR)
		printf("RX_LENGTH_ERR\n");
	if (status & STATE_RX_FRAME_CTR_ERR)
		printf("RX_FRAME_CTR_ERR\n");
	if (status & STATE_RX_FCS_ERR)
		printf("RX_FCS_ERR\n");

	if (status & STATE_TX_ERR)
		printf("TX_ERR\n");
}
#else
static void io_check_status(struct udevice *dev, enum status_print_type type)
{
	u16 status = 0;
	int ret;

	ret = misc_call(dev, 0, NULL, 0, &status, 0);
	if (!ret)
		return;

	err_ctr++;

	if (type != STATUS_LOUD)
		return;

	if (status & STATE_RX_PACKET_DROPPED)
		printf("RX_PACKET_DROPPED, status %04x\n", status);

	if (status & STATE_RX_DIST_ERR)
		printf("RX_DIST_ERR\n");
	if (status & STATE_RX_LENGTH_ERR)
		printf("RX_LENGTH_ERR\n");
	if (status & STATE_RX_FRAME_CTR_ERR)
		printf("RX_FRAME_CTR_ERR\n");
	if (status & STATE_RX_FCS_ERR)
		printf("RX_FCS_ERR\n");

	if (status & STATE_TX_ERR)
		printf("TX_ERR\n");
}
#endif /* CONFIG_GDSYS_LEGACY_DRIVERS */

#ifdef CONFIG_GDSYS_LEGACY_DRIVERS
static void io_send(uint fpga, uint size)
{
	uint k;
	struct io_generic_packet packet = {
		.source_address = 1,
		.packet_type = 1,
		.packet_length = size,
	};
	u16 *p = (u16 *)&packet;

	for (k = 0; k < sizeof(packet) / 2; ++k)
		FPGA_SET_REG(fpga, ep.transmit_data, *p++);

	for (k = 0; k < (size + 1) / 2; ++k)
		FPGA_SET_REG(fpga, ep.transmit_data, k);

	FPGA_SET_REG(fpga, ep.rx_tx_control,
		     CTRL_PROC_RECEIVE_ENABLE | CTRL_FLUSH_TRANSMIT_BUFFER);

	tx_ctr++;
}
#else
static void io_send(struct udevice *dev, uint size)
{
	uint k;
	u16 buffer[HEADER_WORDS + 128];
	struct io_generic_packet header = {
		.source_address = 1,
		.packet_type = 1,
		.packet_length = size,
	};
	const uint words = (size + 1) / 2;

	memcpy(buffer, &header, 2 * HEADER_WORDS);
	for (k = 0; k < words; ++k)
		buffer[k + HEADER_WORDS] = (2 * k + 1) + ((2 * k) << 8);

	misc_write(dev, 0, buffer, HEADER_WORDS + words);

	tx_ctr++;
}
#endif /* CONFIG_GDSYS_LEGACY_DRIVERS */

#ifdef CONFIG_GDSYS_LEGACY_DRIVERS
static void io_receive(uint fpga)
{
	u16 rx_tx_status;

	FPGA_GET_REG(fpga, ep.rx_tx_status, &rx_tx_status);

	while (rx_tx_status & STATE_RX_DATA_AVAILABLE) {
		u16 rx;

		if (rx_tx_status & STATE_RX_DATA_LAST)
			rx_ctr++;

		FPGA_GET_REG(fpga, ep.receive_data, &rx);

		FPGA_GET_REG(fpga, ep.rx_tx_status, &rx_tx_status);
	}
}
#else
static void io_receive(struct udevice *dev)
{
	u16 buffer[HEADER_WORDS + 128];

	if (!misc_read(dev, 0, buffer, 0))
		rx_ctr++;
}
#endif /* CONFIG_GDSYS_LEGACY_DRIVERS */

#ifdef CONFIG_GDSYS_LEGACY_DRIVERS
static void io_reflect(uint fpga)
{
	u16 buffer[128];

	uint k = 0;
	uint n;
	u16 rx_tx_status;

	FPGA_GET_REG(fpga, ep.rx_tx_status, &rx_tx_status);

	while (rx_tx_status & STATE_RX_DATA_AVAILABLE) {
		FPGA_GET_REG(fpga, ep.receive_data, &buffer[k++]);
		if (rx_tx_status & STATE_RX_DATA_LAST)
			break;

		FPGA_GET_REG(fpga, ep.rx_tx_status, &rx_tx_status);
	}

	if (!k)
		return;

	for (n = 0; n < k; ++n)
		FPGA_SET_REG(fpga, ep.transmit_data, buffer[n]);

	FPGA_SET_REG(fpga, ep.rx_tx_control,
		     CTRL_PROC_RECEIVE_ENABLE | CTRL_FLUSH_TRANSMIT_BUFFER);

	tx_ctr++;
}
#else
static void io_reflect(struct udevice *dev)
{
	u16 buffer[HEADER_WORDS + 128];
	struct io_generic_packet *header;

	if (misc_read(dev, 0, buffer, 0))
		return;

	header = (struct io_generic_packet *)&buffer;

	misc_write(dev, 0, buffer, HEADER_WORDS + header->packet_length);
}
#endif /* CONFIG_GDSYS_LEGACY_DRIVERS */

#ifdef CONFIG_GDSYS_LEGACY_DRIVERS
/*
 * FPGA io-endpoint reflector
 *
 * Syntax:
 *	ioreflect {fpga} {reportrate}
 */
int do_ioreflect(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	uint fpga;
	uint rate = 0;
	unsigned long long last_seen = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	fpga = dectoul(argv[1], NULL);

	/*
	 * If another parameter, it is the report rate in packets.
	 */
	if (argc > 2)
		rate = dectoul(argv[2], NULL);

	/* Enable receive path */
	FPGA_SET_REG(fpga, ep.rx_tx_control, CTRL_PROC_RECEIVE_ENABLE);

	/* Set device address to dummy 1*/
	FPGA_SET_REG(fpga, ep.device_address, 1);

	rx_ctr = 0; tx_ctr = 0; err_ctr = 0;

	while (1) {
		u16 top_int;
		u16 rx_tx_status;

		FPGA_GET_REG(fpga, top_interrupt, &top_int);
		FPGA_GET_REG(fpga, ep.rx_tx_status, &rx_tx_status);

		io_check_status(fpga, rx_tx_status, STATUS_SILENT);
		if ((top_int & IRQ_CPU_RECEIVE_DATA_AVAILABLE_STATUS) &&
		    (top_int & IRQ_CPU_TRANSMITBUFFER_FREE_STATUS))
			io_reflect(fpga);

		if (rate) {
			if (!(tx_ctr % rate) && (tx_ctr != last_seen))
				printf("refl %llu, err %llu\n", tx_ctr,
				       err_ctr);
			last_seen = tx_ctr;
		}

		if (ctrlc())
			break;
	}

	return 0;
}
#else
/*
 * FPGA io-endpoint reflector
 *
 * Syntax:
 *	ioreflect {reportrate}
 */
int do_ioreflect(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct udevice *fpga;
	struct regmap *map;
	uint rate = 0;
	unsigned long long last_seen = 0;

	if (!dev) {
		printf("No device selected\n");
		return 1;
	}

	gdsys_soc_get_fpga(dev, &fpga);
	regmap_init_mem(dev_ofnode(dev), &map);

	/* Enable receive path */
	misc_set_enabled(dev, true);

	rx_ctr = 0; tx_ctr = 0; err_ctr = 0;

	while (1) {
		uint top_int;

		ihs_fpga_get(map, top_interrupt, &top_int);
		io_check_status(dev, STATUS_SILENT);
		if ((top_int & IRQ_CPU_RECEIVE_DATA_AVAILABLE_STATUS) &&
		    (top_int & IRQ_CPU_TRANSMITBUFFER_FREE_STATUS))
			io_reflect(dev);

		if (rate) {
			if (!(tx_ctr % rate) && (tx_ctr != last_seen))
				printf("refl %llu, err %llu\n", tx_ctr,
				       err_ctr);
			last_seen = tx_ctr;
		}

		if (ctrlc())
			break;
	}

	return 0;
}
#endif /* CONFIG_GDSYS_LEGACY_DRIVERS */

#define DISP_LINE_LEN	16

#ifdef CONFIG_GDSYS_LEGACY_DRIVERS
/*
 * FPGA io-endpoint looptest
 *
 * Syntax:
 *	ioloop {fpga} {size} {rate}
 */
int do_ioloop(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	uint fpga;
	uint size;
	uint rate = 0;

	if (argc < 3)
		return CMD_RET_USAGE;

	/*
	 * FPGA is specified since argc > 2
	 */
	fpga = dectoul(argv[1], NULL);

	/*
	 * packet size is specified since argc > 2
	 */
	size = dectoul(argv[2], NULL);

	/*
	 * If another parameter, it is the test rate in packets per second.
	 */
	if (argc > 3)
		rate = dectoul(argv[3], NULL);

	/* enable receive path */
	FPGA_SET_REG(fpga, ep.rx_tx_control, CTRL_PROC_RECEIVE_ENABLE);

	/* set device address to dummy 1*/
	FPGA_SET_REG(fpga, ep.device_address, 1);

	rx_ctr = 0; tx_ctr = 0; err_ctr = 0;

	while (1) {
		u16 top_int;
		u16 rx_tx_status;

		FPGA_GET_REG(fpga, top_interrupt, &top_int);
		FPGA_GET_REG(fpga, ep.rx_tx_status, &rx_tx_status);

		io_check_status(fpga, rx_tx_status, STATUS_LOUD);
		if (top_int & IRQ_CPU_TRANSMITBUFFER_FREE_STATUS)
			io_send(fpga, size);
		if (top_int & IRQ_CPU_RECEIVE_DATA_AVAILABLE_STATUS)
			io_receive(fpga);

		if (rate) {
			if (ctrlc())
				break;
			udelay(1000000 / rate);
			if (!(tx_ctr % rate))
				printf("d %llu, tx %llu, rx %llu, err %llu\n",
				       tx_ctr - rx_ctr, tx_ctr, rx_ctr,
				       err_ctr);
		}
	}

	return 0;
}
#else
/*
 * FPGA io-endpoint looptest
 *
 * Syntax:
 *	ioloop {size} {rate}
 */
int do_ioloop(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	uint size;
	uint rate = 0;
	struct udevice *fpga;
	struct regmap *map;

	if (!dev) {
		printf("No device selected\n");
		return 1;
	}

	gdsys_soc_get_fpga(dev, &fpga);
	regmap_init_mem(dev_ofnode(dev), &map);

	if (argc < 2)
		return CMD_RET_USAGE;

	/*
	 * packet size is specified since argc > 1
	 */
	size = dectoul(argv[2], NULL);

	/*
	 * If another parameter, it is the test rate in packets per second.
	 */
	if (argc > 2)
		rate = dectoul(argv[3], NULL);

	/* Enable receive path */
	misc_set_enabled(dev, true);

	rx_ctr = 0; tx_ctr = 0; err_ctr = 0;

	while (1) {
		uint top_int;

		if (ctrlc())
			break;

		ihs_fpga_get(map, top_interrupt, &top_int);

		io_check_status(dev, STATUS_LOUD);
		if (top_int & IRQ_CPU_TRANSMITBUFFER_FREE_STATUS)
			io_send(dev, size);
		if (top_int & IRQ_CPU_RECEIVE_DATA_AVAILABLE_STATUS)
			io_receive(dev);

		if (rate) {
			udelay(1000000 / rate);
			if (!(tx_ctr % rate))
				printf("d %llu, tx %llu, rx %llu, err %llu\n",
				       tx_ctr - rx_ctr, tx_ctr, rx_ctr,
				       err_ctr);
		}
	}
	return 0;
}
#endif /* CONFIG_GDSYS_LEGACY_DRIVERS */

#ifndef CONFIG_GDSYS_LEGACY_DRIVERS
int do_iodev(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct udevice *ioep = NULL;
	struct udevice *sysinfo;
	char name[8];
	int ret;

	if (sysinfo_get(&sysinfo))
		return CMD_RET_FAILURE;

	if (argc > 1) {
		int i = dectoul(argv[1], NULL);

		snprintf(name, sizeof(name), "ioep%d", i);

		ret = uclass_get_device_by_phandle(UCLASS_MISC, sysinfo, name,
						   &ioep);

		if (ret || !ioep) {
			printf("Invalid IOEP %d\n", i);
			return CMD_RET_FAILURE;
		}

		dev = ioep;
	} else {
		int i = 0;

		while (1) {
			snprintf(name, sizeof(name), "ioep%d", i);

			ret = uclass_get_device_by_phandle(UCLASS_MISC, sysinfo,
							   name, &ioep);

			if (ret || !ioep)
				break;

			printf("IOEP %d:\t%s\n", i++, ioep->name);
		}

		if (dev)
			printf("\nSelected IOEP: %s\n", dev->name);
		else
			puts("\nNo IOEP selected.\n");
	}

	return 0;
}
#endif /* !CONFIG_GDSYS_LEGACY_DRIVERS */

#ifdef CONFIG_GDSYS_LEGACY_DRIVERS
U_BOOT_CMD(
	ioloop,	4,	0,	do_ioloop,
	"fpga io-endpoint looptest",
	"fpga packetsize [packets/sec]"
);

U_BOOT_CMD(
	ioreflect, 3,	0,	do_ioreflect,
	"fpga io-endpoint reflector",
	"fpga reportrate"
);
#else
U_BOOT_CMD(
	ioloop,	3,	0,	do_ioloop,
	"fpga io-endpoint looptest",
	"packetsize [packets/sec]"
);

U_BOOT_CMD(
	ioreflect, 2,	0,	do_ioreflect,
	"fpga io-endpoint reflector",
	"reportrate"
);

U_BOOT_CMD(
	iodev, 2,	0,	do_iodev,
	"fpga io-endpoint listing/selection",
	"[ioep device to select]"
);
#endif /* CONFIG_GDSYS_LEGACY_DRIVERS */
