/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _QUARK_MSG_PORT_H_
#define _QUARK_MSG_PORT_H_

/*
 * In the Quark SoC, some chipset commands are accomplished by utilizing
 * the internal message network within the host bridge (D0:F0). Accesses
 * to this network are accomplished by populating the message control
 * register (MCR), Message Control Register eXtension (MCRX) and the
 * message data register (MDR).
 */
#define MSG_CTRL_REG		0xd0	/* Message Control Register */
#define MSG_DATA_REG		0xd4	/* Message Data Register */
#define MSG_CTRL_EXT_REG	0xd8	/* Message Control Register EXT */

/* Normal Read/Write OpCodes */
#define MSG_OP_READ		0x10
#define MSG_OP_WRITE		0x11

/* Alternative Read/Write OpCodes */
#define MSG_OP_ALT_READ		0x06
#define MSG_OP_ALT_WRITE	0x07

/* IO Read/Write OpCodes */
#define MSG_OP_IO_READ		0x02
#define MSG_OP_IO_WRITE		0x03

/* All byte enables */
#define MSG_BYTE_ENABLE		0xf0

#ifndef __ASSEMBLY__

/**
 * msg_port_setup - set up the message port control register
 *
 * @op:     message bus access opcode
 * @port:   port number on the message bus
 * @reg:    register number within a port
 */
void msg_port_setup(int op, int port, int reg);

/**
 * msg_port_read - read a message port register using normal opcode
 *
 * @port:   port number on the message bus
 * @reg:    register number within a port
 *
 * @return: message port register value
 */
u32 msg_port_read(u8 port, u32 reg);

/**
 * msg_port_write - write a message port register using normal opcode
 *
 * @port:   port number on the message bus
 * @reg:    register number within a port
 * @value:  register value to write
 */
void msg_port_write(u8 port, u32 reg, u32 value);

/**
 * msg_port_alt_read - read a message port register using alternative opcode
 *
 * @port:   port number on the message bus
 * @reg:    register number within a port
 *
 * @return: message port register value
 */
u32 msg_port_alt_read(u8 port, u32 reg);

/**
 * msg_port_alt_write - write a message port register using alternative opcode
 *
 * @port:   port number on the message bus
 * @reg:    register number within a port
 * @value:  register value to write
 */
void msg_port_alt_write(u8 port, u32 reg, u32 value);

/**
 * msg_port_io_read - read a message port register using I/O opcode
 *
 * @port:   port number on the message bus
 * @reg:    register number within a port
 *
 * @return: message port register value
 */
u32 msg_port_io_read(u8 port, u32 reg);

/**
 * msg_port_io_write - write a message port register using I/O opcode
 *
 * @port:   port number on the message bus
 * @reg:    register number within a port
 * @value:  register value to write
 */
void msg_port_io_write(u8 port, u32 reg, u32 value);

/* clrbits, setbits, clrsetbits macros for message port access */

#define msg_port_normal_read	msg_port_read
#define msg_port_normal_write	msg_port_write

#define msg_port_generic_clrsetbits(type, port, reg, clr, set)		\
	msg_port_##type##_write(port, reg,				\
				(msg_port_##type##_read(port, reg)	\
				& ~(clr)) | (set))

#define msg_port_clrbits(port, reg, clr)		\
	msg_port_generic_clrsetbits(normal, port, reg, clr, 0)
#define msg_port_setbits(port, reg, set)		\
	msg_port_generic_clrsetbits(normal, port, reg, 0, set)
#define msg_port_clrsetbits(port, reg, clr, set)	\
	msg_port_generic_clrsetbits(normal, port, reg, clr, set)

#define msg_port_alt_clrbits(port, reg, clr)		\
	msg_port_generic_clrsetbits(alt, port, reg, clr, 0)
#define msg_port_alt_setbits(port, reg, set)		\
	msg_port_generic_clrsetbits(alt, port, reg, 0, set)
#define msg_port_alt_clrsetbits(port, reg, clr, set)	\
	msg_port_generic_clrsetbits(alt, port, reg, clr, set)

#define msg_port_io_clrbits(port, reg, clr)		\
	msg_port_generic_clrsetbits(io, port, reg, clr, 0)
#define msg_port_io_setbits(port, reg, set)		\
	msg_port_generic_clrsetbits(io, port, reg, 0, set)
#define msg_port_io_clrsetbits(port, reg, clr, set)	\
	msg_port_generic_clrsetbits(io, port, reg, clr, set)

#endif /* __ASSEMBLY__ */

#endif /* _QUARK_MSG_PORT_H_ */
