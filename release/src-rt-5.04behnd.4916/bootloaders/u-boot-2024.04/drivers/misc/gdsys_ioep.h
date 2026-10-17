/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#ifndef __GDSYS_IOEP_H_
#define __GDSYS_IOEP_H_

/**
 * struct io_generic_packet - header structure for GDSYS IOEP packets
 * @target_address:     Target protocol address of the packet.
 * @source_address:     Source protocol address of the packet.
 * @packet_type:        Packet type.
 * @bc:                 Block counter (filled in by FPGA).
 * @packet_length:      Length of the packet's payload bytes.
 */
#include <linux/bitops.h>
struct io_generic_packet {
	u16 target_address;
	u16 source_address;
	u8 packet_type;
	u8 bc;
	u16 packet_length;
} __attribute__((__packed__));

/**
 * struct gdsys_ioep_regs - Registers of a IOEP device
 * @transmit_data:  Register that receives data to be sent
 * @tx_control:     TX control register
 * @receive_data:   Register filled with the received data
 * @rx_tx_status:   RX/TX status register
 * @device_address: Register for setting/reading the device's address
 * @target_address: Register for setting/reading the remote endpoint's address
 * @int_enable:     Interrupt/Interrupt enable register
 */
struct gdsys_ioep_regs {
	u16 transmit_data;
	u16 tx_control;
	u16 receive_data;
	u16 rx_tx_status;
	u16 device_address;
	u16 target_address;
	u16 int_enable;
};

/**
 * gdsys_ioep_set() - Convenience macro to write registers of a IOEP device
 * @map:    Register map to write the value in
 * @member: Name of the member in the gdsys_ioep_regs structure to write
 * @val:    Value to write to the register
 */
#define gdsys_ioep_set(map, member, val) \
	regmap_set(map, struct gdsys_ioep_regs, member, val)

/**
 * gdsys_ioep_get() - Convenience macro to read registers of a IOEP device
 * @map:    Register map to read the value from
 * @member: Name of the member in the gdsys_ioep_regs structure to read
 * @valp:   Pointer to buffer to read the register value into
 */
#define gdsys_ioep_get(map, member, valp) \
	regmap_get(map, struct gdsys_ioep_regs, member, valp)

/**
 * enum rx_tx_status_values - Enum to describe the fields of the rx_tx_status
 *			      register
 * @STATE_TX_PACKET_BUILDING:      The device is currently building a packet
 *				   (and accepting data for it)
 * @STATE_TX_TRANSMITTING:         A packet is currenly being transmitted
 * @STATE_TX_BUFFER_FULL:          The TX buffer is full
 * @STATE_TX_ERR:                  A TX error occurred
 * @STATE_RECEIVE_TIMEOUT:         A receive timeout occurred
 * @STATE_PROC_RX_STORE_TIMEOUT:   A RX store timeout for a processor packet
 *				   occurred
 * @STATE_PROC_RX_RECEIVE_TIMEOUT: A RX receive timeout for a processor packet
 *				   occurred
 * @STATE_RX_DIST_ERR:             A error occurred in the distribution block
 * @STATE_RX_LENGTH_ERR:           A length invalid error occurred
 * @STATE_RX_FRAME_CTR_ERR:        A frame count error occurred (two
 *				   non-increasing frame count numbers
 *				   encountered)
 * @STATE_RX_FCS_ERR:              A CRC error occurred
 * @STATE_RX_PACKET_DROPPED:       A RX packet has been dropped
 * @STATE_RX_DATA_LAST:            The data to be read is the final data of the
 *				   current packet
 * @STATE_RX_DATA_FIRST:           The data to be read is the first data of the
 *				   current packet
 * @STATE_RX_DATA_AVAILABLE:       RX data is available to be read
 */
enum rx_tx_status_values {
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

/**
 * enum tx_control_values - Enum to describe the fields of the tx_control
 *			    register
 * @CTRL_PROC_RECEIVE_ENABLE:   Enable packet reception for the processor
 * @CTRL_FLUSH_TRANSMIT_BUFFER: Flush the transmit buffer (and send packet data)
 */
enum tx_control_values {
	CTRL_PROC_RECEIVE_ENABLE = BIT(12),
	CTRL_FLUSH_TRANSMIT_BUFFER = BIT(15),
};

/**
 * enum int_enable_values - Enum to describe the fields of the int_enable
 *			    register
 * @IRQ_CPU_TRANSMITBUFFER_FREE_STATUS:    The transmit buffer is free (packet
 *					   data can be transmitted to the
 *					   device)
 * @IRQ_CPU_PACKET_TRANSMITTED_EVENT:      A packet has been transmitted
 * @IRQ_NEW_CPU_PACKET_RECEIVED_EVENT:     A new packet has been received
 * @IRQ_CPU_RECEIVE_DATA_AVAILABLE_STATUS: RX packet data are available to be
 *					   read
 */
enum int_enable_values {
	IRQ_CPU_TRANSMITBUFFER_FREE_STATUS = BIT(5),
	IRQ_CPU_PACKET_TRANSMITTED_EVENT = BIT(6),
	IRQ_NEW_CPU_PACKET_RECEIVED_EVENT = BIT(7),
	IRQ_CPU_RECEIVE_DATA_AVAILABLE_STATUS = BIT(8),
};

#endif /* __GDSYS_IOEP_H_ */
