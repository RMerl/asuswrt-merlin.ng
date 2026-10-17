// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <asm/arch/device.h>
#include <asm/arch/msg_port.h>
#include <asm/arch/quark.h>

void msg_port_setup(int op, int port, int reg)
{
	qrk_pci_write_config_dword(QUARK_HOST_BRIDGE, MSG_CTRL_REG,
				   (((op) << 24) | ((port) << 16) |
				   (((reg) << 8) & 0xff00) | MSG_BYTE_ENABLE));
}

u32 msg_port_read(u8 port, u32 reg)
{
	u32 value;

	qrk_pci_write_config_dword(QUARK_HOST_BRIDGE, MSG_CTRL_EXT_REG,
				   reg & 0xffffff00);
	msg_port_setup(MSG_OP_READ, port, reg);
	qrk_pci_read_config_dword(QUARK_HOST_BRIDGE, MSG_DATA_REG, &value);

	return value;
}

void msg_port_write(u8 port, u32 reg, u32 value)
{
	qrk_pci_write_config_dword(QUARK_HOST_BRIDGE, MSG_DATA_REG, value);
	qrk_pci_write_config_dword(QUARK_HOST_BRIDGE, MSG_CTRL_EXT_REG,
				   reg & 0xffffff00);
	msg_port_setup(MSG_OP_WRITE, port, reg);
}

u32 msg_port_alt_read(u8 port, u32 reg)
{
	u32 value;

	qrk_pci_write_config_dword(QUARK_HOST_BRIDGE, MSG_CTRL_EXT_REG,
				   reg & 0xffffff00);
	msg_port_setup(MSG_OP_ALT_READ, port, reg);
	qrk_pci_read_config_dword(QUARK_HOST_BRIDGE, MSG_DATA_REG, &value);

	return value;
}

void msg_port_alt_write(u8 port, u32 reg, u32 value)
{
	qrk_pci_write_config_dword(QUARK_HOST_BRIDGE, MSG_DATA_REG, value);
	qrk_pci_write_config_dword(QUARK_HOST_BRIDGE, MSG_CTRL_EXT_REG,
				   reg & 0xffffff00);
	msg_port_setup(MSG_OP_ALT_WRITE, port, reg);
}

u32 msg_port_io_read(u8 port, u32 reg)
{
	u32 value;

	qrk_pci_write_config_dword(QUARK_HOST_BRIDGE, MSG_CTRL_EXT_REG,
				   reg & 0xffffff00);
	msg_port_setup(MSG_OP_IO_READ, port, reg);
	qrk_pci_read_config_dword(QUARK_HOST_BRIDGE, MSG_DATA_REG, &value);

	return value;
}

void msg_port_io_write(u8 port, u32 reg, u32 value)
{
	qrk_pci_write_config_dword(QUARK_HOST_BRIDGE, MSG_DATA_REG, value);
	qrk_pci_write_config_dword(QUARK_HOST_BRIDGE, MSG_CTRL_EXT_REG,
				   reg & 0xffffff00);
	msg_port_setup(MSG_OP_IO_WRITE, port, reg);
}
