/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2009 Sergey Kubushyn <ksi@koi8.net>
 * Copyright (C) 2009 - 2013 Heiko Schocher <hs@denx.de>
 * Changes for multibus/multiadapter I2C support.
 *
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 *
 * The original I2C interface was
 *   (C) 2000 by Paolo Scaffardi (arsenio@tin.it)
 *   AIRVENT SAM s.p.a - RIMINI(ITALY)
 * but has been changed substantially.
 */

#ifndef _I2C_H_
#define _I2C_H_

/*
 * For now there are essentially two parts to this file - driver model
 * here at the top, and the older code below (with CONFIG_SYS_I2C being
 * most recent). The plan is to migrate everything to driver model.
 * The driver model structures and API are separate as they are different
 * enough as to be incompatible for compilation purposes.
 */

enum dm_i2c_chip_flags {
	DM_I2C_CHIP_10BIT	= 1 << 0, /* Use 10-bit addressing */
	DM_I2C_CHIP_RD_ADDRESS	= 1 << 1, /* Send address for each read byte */
	DM_I2C_CHIP_WR_ADDRESS	= 1 << 2, /* Send address for each write byte */
};

struct udevice;
/**
 * struct dm_i2c_chip - information about an i2c chip
 *
 * An I2C chip is a device on the I2C bus. It sits at a particular address
 * and normally supports 7-bit or 10-bit addressing.
 *
 * To obtain this structure, use dev_get_parent_platdata(dev) where dev is
 * the chip to examine.
 *
 * @chip_addr:	Chip address on bus
 * @offset_len: Length of offset in bytes. A single byte offset can
 *		represent up to 256 bytes. A value larger than 1 may be
 *		needed for larger devices.
 * @flags:	Flags for this chip (dm_i2c_chip_flags)
 * @emul: Emulator for this chip address (only used for emulation)
 */
struct dm_i2c_chip {
	uint chip_addr;
	uint offset_len;
	uint flags;
#ifdef CONFIG_SANDBOX
	struct udevice *emul;
	bool test_mode;
#endif
};

/**
 * struct dm_i2c_bus- information about an i2c bus
 *
 * An I2C bus contains 0 or more chips on it, each at its own address. The
 * bus can operate at different speeds (measured in Hz, typically 100KHz
 * or 400KHz).
 *
 * To obtain this structure, use dev_get_uclass_priv(bus) where bus is the
 * I2C bus udevice.
 *
 * @speed_hz: Bus speed in hertz (typically 100000)
 * @max_transaction_bytes: Maximal size of single I2C transfer
 */
struct dm_i2c_bus {
	int speed_hz;
	int max_transaction_bytes;
};

/*
 * Not all of these flags are implemented in the U-Boot API
 */
enum dm_i2c_msg_flags {
	I2C_M_TEN		= 0x0010, /* ten-bit chip address */
	I2C_M_RD		= 0x0001, /* read data, from slave to master */
	I2C_M_STOP		= 0x8000, /* send stop after this message */
	I2C_M_NOSTART		= 0x4000, /* no start before this message */
	I2C_M_REV_DIR_ADDR	= 0x2000, /* invert polarity of R/W bit */
	I2C_M_IGNORE_NAK	= 0x1000, /* continue after NAK */
	I2C_M_NO_RD_ACK		= 0x0800, /* skip the Ack bit on reads */
	I2C_M_RECV_LEN		= 0x0400, /* length is first received byte */
};

/**
 * struct i2c_msg - an I2C message
 *
 * @addr:	Slave address
 * @flags:	Flags (see enum dm_i2c_msg_flags)
 * @len:	Length of buffer in bytes, may be 0 for a probe
 * @buf:	Buffer to send/receive, or NULL if no data
 */
struct i2c_msg {
	uint addr;
	uint flags;
	uint len;
	u8 *buf;
};

/**
 * struct i2c_msg_list - a list of I2C messages
 *
 * This is called i2c_rdwr_ioctl_data in Linux but the name does not seem
 * appropriate in U-Boot.
 *
 * @msg:	Pointer to i2c_msg array
 * @nmsgs:	Number of elements in the array
 */
struct i2c_msg_list {
	struct i2c_msg *msgs;
	uint nmsgs;
};

/**
 * dm_i2c_read() - read bytes from an I2C chip
 *
 * To obtain an I2C device (called a 'chip') given the I2C bus address you
 * can use i2c_get_chip(). To obtain a bus by bus number use
 * uclass_get_device_by_seq(UCLASS_I2C, <bus number>).
 *
 * To set the address length of a devce use i2c_set_addr_len(). It
 * defaults to 1.
 *
 * @dev:	Chip to read from
 * @offset:	Offset within chip to start reading
 * @buffer:	Place to put data
 * @len:	Number of bytes to read
 *
 * @return 0 on success, -ve on failure
 */
int dm_i2c_read(struct udevice *dev, uint offset, uint8_t *buffer, int len);

/**
 * dm_i2c_write() - write bytes to an I2C chip
 *
 * See notes for dm_i2c_read() above.
 *
 * @dev:	Chip to write to
 * @offset:	Offset within chip to start writing
 * @buffer:	Buffer containing data to write
 * @len:	Number of bytes to write
 *
 * @return 0 on success, -ve on failure
 */
int dm_i2c_write(struct udevice *dev, uint offset, const uint8_t *buffer,
		 int len);

/**
 * dm_i2c_probe() - probe a particular chip address
 *
 * This can be useful to check for the existence of a chip on the bus.
 * It is typically implemented by writing the chip address to the bus
 * and checking that the chip replies with an ACK.
 *
 * @bus:	Bus to probe
 * @chip_addr:	7-bit address to probe (10-bit and others are not supported)
 * @chip_flags:	Flags for the probe (see enum dm_i2c_chip_flags)
 * @devp:	Returns the device found, or NULL if none
 * @return 0 if a chip was found at that address, -ve if not
 */
int dm_i2c_probe(struct udevice *bus, uint chip_addr, uint chip_flags,
		 struct udevice **devp);

/**
 * dm_i2c_reg_read() - Read a value from an I2C register
 *
 * This reads a single value from the given address in an I2C chip
 *
 * @dev:	Device to use for transfer
 * @addr:	Address to read from
 * @return value read, or -ve on error
 */
int dm_i2c_reg_read(struct udevice *dev, uint offset);

/**
 * dm_i2c_reg_write() - Write a value to an I2C register
 *
 * This writes a single value to the given address in an I2C chip
 *
 * @dev:	Device to use for transfer
 * @addr:	Address to write to
 * @val:	Value to write (normally a byte)
 * @return 0 on success, -ve on error
 */
int dm_i2c_reg_write(struct udevice *dev, uint offset, unsigned int val);

/**
 * dm_i2c_xfer() - Transfer messages over I2C
 *
 * This transfers a raw message. It is best to use dm_i2c_reg_read/write()
 * instead.
 *
 * @dev:	Device to use for transfer
 * @msg:	List of messages to transfer
 * @nmsgs:	Number of messages to transfer
 * @return 0 on success, -ve on error
 */
int dm_i2c_xfer(struct udevice *dev, struct i2c_msg *msg, int nmsgs);

/**
 * dm_i2c_set_bus_speed() - set the speed of a bus
 *
 * @bus:	Bus to adjust
 * @speed:	Requested speed in Hz
 * @return 0 if OK, -EINVAL for invalid values
 */
int dm_i2c_set_bus_speed(struct udevice *bus, unsigned int speed);

/**
 * dm_i2c_get_bus_speed() - get the speed of a bus
 *
 * @bus:	Bus to check
 * @return speed of selected I2C bus in Hz, -ve on error
 */
int dm_i2c_get_bus_speed(struct udevice *bus);

/**
 * i2c_set_chip_flags() - set flags for a chip
 *
 * Typically addresses are 7 bits, but for 10-bit addresses you should set
 * flags to DM_I2C_CHIP_10BIT. All accesses will then use 10-bit addressing.
 *
 * @dev:	Chip to adjust
 * @flags:	New flags
 * @return 0 if OK, -EINVAL if value is unsupported, other -ve value on error
 */
int i2c_set_chip_flags(struct udevice *dev, uint flags);

/**
 * i2c_get_chip_flags() - get flags for a chip
 *
 * @dev:	Chip to check
 * @flagsp:	Place to put flags
 * @return 0 if OK, other -ve value on error
 */
int i2c_get_chip_flags(struct udevice *dev, uint *flagsp);

/**
 * i2c_set_offset_len() - set the offset length for a chip
 *
 * The offset used to access a chip may be up to 4 bytes long. Typically it
 * is only 1 byte, which is enough for chips with 256 bytes of memory or
 * registers. The default value is 1, but you can call this function to
 * change it.
 *
 * @offset_len:	New offset length value (typically 1 or 2)
 */
int i2c_set_chip_offset_len(struct udevice *dev, uint offset_len);

/**
 * i2c_get_offset_len() - get the offset length for a chip
 *
 * @return:	Current offset length value (typically 1 or 2)
 */
int i2c_get_chip_offset_len(struct udevice *dev);

/**
 * i2c_deblock() - recover a bus that is in an unknown state
 *
 * See the deblock() method in 'struct dm_i2c_ops' for full information
 *
 * @bus:	Bus to recover
 * @return 0 if OK, -ve on error
 */
int i2c_deblock(struct udevice *bus);

#ifdef CONFIG_DM_I2C_COMPAT
/**
 * i2c_probe() - Compatibility function for driver model
 *
 * Calls dm_i2c_probe() on the current bus
 */
int i2c_probe(uint8_t chip_addr);

/**
 * i2c_read() - Compatibility function for driver model
 *
 * Calls dm_i2c_read() with the device corresponding to @chip_addr, and offset
 * set to @addr. @alen must match the current setting for the device.
 */
int i2c_read(uint8_t chip_addr, unsigned int addr, int alen, uint8_t *buffer,
	     int len);

/**
 * i2c_write() - Compatibility function for driver model
 *
 * Calls dm_i2c_write() with the device corresponding to @chip_addr, and offset
 * set to @addr. @alen must match the current setting for the device.
 */
int i2c_write(uint8_t chip_addr, unsigned int addr, int alen, uint8_t *buffer,
	      int len);

/**
 * i2c_get_bus_num_fdt() - Compatibility function for driver model
 *
 * @return the bus number associated with the given device tree node
 */
int i2c_get_bus_num_fdt(int node);

/**
 * i2c_get_bus_num() - Compatibility function for driver model
 *
 * @return the 'current' bus number
 */
unsigned int i2c_get_bus_num(void);

/**
 * i2c_set_bus_num() - Compatibility function for driver model
 *
 * Sets the 'current' bus
 */
int i2c_set_bus_num(unsigned int bus);

static inline void I2C_SET_BUS(unsigned int bus)
{
	i2c_set_bus_num(bus);
}

static inline unsigned int I2C_GET_BUS(void)
{
	return i2c_get_bus_num();
}

/**
 * i2c_init() - Compatibility function for driver model
 *
 * This function does nothing.
 */
void i2c_init(int speed, int slaveaddr);

/**
 * board_i2c_init() - Compatibility function for driver model
 *
 * @param blob  Device tree blbo
 * @return the number of I2C bus
 */
void board_i2c_init(const void *blob);

/*
 * Compatibility functions for driver model.
 */
uint8_t i2c_reg_read(uint8_t addr, uint8_t reg);
void i2c_reg_write(uint8_t addr, uint8_t reg, uint8_t val);

#endif

/**
 * struct dm_i2c_ops - driver operations for I2C uclass
 *
 * Drivers should support these operations unless otherwise noted. These
 * operations are intended to be used by uclass code, not directly from
 * other code.
 */
struct dm_i2c_ops {
	/**
	 * xfer() - transfer a list of I2C messages
	 *
	 * @bus:	Bus to read from
	 * @msg:	List of messages to transfer
	 * @nmsgs:	Number of messages in the list
	 * @return 0 if OK, -EREMOTEIO if the slave did not ACK a byte,
	 *	-ECOMM if the speed cannot be supported, -EPROTO if the chip
	 *	flags cannot be supported, other -ve value on some other error
	 */
	int (*xfer)(struct udevice *bus, struct i2c_msg *msg, int nmsgs);

	/**
	 * probe_chip() - probe for the presense of a chip address
	 *
	 * This function is optional. If omitted, the uclass will send a zero
	 * length message instead.
	 *
	 * @bus:	Bus to probe
	 * @chip_addr:	Chip address to probe
	 * @chip_flags:	Probe flags (enum dm_i2c_chip_flags)
	 * @return 0 if chip was found, -EREMOTEIO if not, -ENOSYS to fall back
	 * to default probem other -ve value on error
	 */
	int (*probe_chip)(struct udevice *bus, uint chip_addr, uint chip_flags);

	/**
	 * set_bus_speed() - set the speed of a bus (optional)
	 *
	 * The bus speed value will be updated by the uclass if this function
	 * does not return an error. This method is optional - if it is not
	 * provided then the driver can read the speed from
	 * dev_get_uclass_priv(bus)->speed_hz
	 *
	 * @bus:	Bus to adjust
	 * @speed:	Requested speed in Hz
	 * @return 0 if OK, -EINVAL for invalid values
	 */
	int (*set_bus_speed)(struct udevice *bus, unsigned int speed);

	/**
	 * get_bus_speed() - get the speed of a bus (optional)
	 *
	 * Normally this can be provided by the uclass, but if you want your
	 * driver to check the bus speed by looking at the hardware, you can
	 * implement that here. This method is optional. This method would
	 * normally be expected to return dev_get_uclass_priv(bus)->speed_hz.
	 *
	 * @bus:	Bus to check
	 * @return speed of selected I2C bus in Hz, -ve on error
	 */
	int (*get_bus_speed)(struct udevice *bus);

	/**
	 * set_flags() - set the flags for a chip (optional)
	 *
	 * This is generally implemented by the uclass, but drivers can
	 * check the value to ensure that unsupported options are not used.
	 * This method is optional. If provided, this method will always be
	 * called when the flags change.
	 *
	 * @dev:	Chip to adjust
	 * @flags:	New flags value
	 * @return 0 if OK, -EINVAL if value is unsupported
	 */
	int (*set_flags)(struct udevice *dev, uint flags);

	/**
	 * deblock() - recover a bus that is in an unknown state
	 *
	 * I2C is a synchronous protocol and resets of the processor in the
	 * middle of an access can block the I2C Bus until a powerdown of
	 * the full unit is done. This is because slaves can be stuck
	 * waiting for addition bus transitions for a transaction that will
	 * never complete. Resetting the I2C master does not help. The only
	 * way is to force the bus through a series of transitions to make
	 * sure that all slaves are done with the transaction. This method
	 * performs this 'deblocking' if support by the driver.
	 *
	 * This method is optional.
	 */
	int (*deblock)(struct udevice *bus);
};

#define i2c_get_ops(dev)	((struct dm_i2c_ops *)(dev)->driver->ops)

/**
 * struct i2c_mux_ops - operations for an I2C mux
 *
 * The current mux state is expected to be stored in the mux itself since
 * it is the only thing that knows how to make things work. The mux can
 * record the current state and then avoid switching unless it is necessary.
 * So select() can be skipped if the mux is already in the correct state.
 * Also deselect() can be made a nop if required.
 */
struct i2c_mux_ops {
	/**
	 * select() - select one of of I2C buses attached to a mux
	 *
	 * This will be called when there is no bus currently selected by the
	 * mux. This method does not need to deselect the old bus since
	 * deselect() will be already have been called if necessary.
	 *
	 * @mux:	Mux device
	 * @bus:	I2C bus to select
	 * @channel:	Channel number correponding to the bus to select
	 * @return 0 if OK, -ve on error
	 */
	int (*select)(struct udevice *mux, struct udevice *bus, uint channel);

	/**
	 * deselect() - select one of of I2C buses attached to a mux
	 *
	 * This is used to deselect the currently selected I2C bus.
	 *
	 * @mux:	Mux device
	 * @bus:	I2C bus to deselect
	 * @channel:	Channel number correponding to the bus to deselect
	 * @return 0 if OK, -ve on error
	 */
	int (*deselect)(struct udevice *mux, struct udevice *bus, uint channel);
};

#define i2c_mux_get_ops(dev)	((struct i2c_mux_ops *)(dev)->driver->ops)

/**
 * i2c_get_chip() - get a device to use to access a chip on a bus
 *
 * This returns the device for the given chip address. The device can then
 * be used with calls to i2c_read(), i2c_write(), i2c_probe(), etc.
 *
 * @bus:	Bus to examine
 * @chip_addr:	Chip address for the new device
 * @offset_len:	Length of a register offset in bytes (normally 1)
 * @devp:	Returns pointer to new device if found or -ENODEV if not
 *		found
 */
int i2c_get_chip(struct udevice *bus, uint chip_addr, uint offset_len,
		 struct udevice **devp);

/**
 * i2c_get_chip_for_busnum() - get a device to use to access a chip on
 *			       a bus number
 *
 * This returns the device for the given chip address on a particular bus
 * number.
 *
 * @busnum:	Bus number to examine
 * @chip_addr:	Chip address for the new device
 * @offset_len:	Length of a register offset in bytes (normally 1)
 * @devp:	Returns pointer to new device if found or -ENODEV if not
 *		found
 */
int i2c_get_chip_for_busnum(int busnum, int chip_addr, uint offset_len,
			    struct udevice **devp);

/**
 * i2c_chip_ofdata_to_platdata() - Decode standard I2C platform data
 *
 * This decodes the chip address from a device tree node and puts it into
 * its dm_i2c_chip structure. This should be called in your driver's
 * ofdata_to_platdata() method.
 *
 * @blob:	Device tree blob
 * @node:	Node offset to read from
 * @spi:	Place to put the decoded information
 */
int i2c_chip_ofdata_to_platdata(struct udevice *dev, struct dm_i2c_chip *chip);

/**
 * i2c_dump_msgs() - Dump a list of I2C messages
 *
 * This may be useful for debugging.
 *
 * @msg:	Message list to dump
 * @nmsgs:	Number of messages
 */
void i2c_dump_msgs(struct i2c_msg *msg, int nmsgs);

/**
 * i2c_emul_find() - Find an emulator for an i2c sandbox device
 *
 * This looks at the device's 'emul' phandle
 *
 * @dev: Device to find an emulator for
 * @emulp: Returns the associated emulator, if found *
 * @return 0 if OK, -ENOENT or -ENODEV if not found
 */
int i2c_emul_find(struct udevice *dev, struct udevice **emulp);

/**
 * i2c_emul_get_device() - Find the device being emulated
 *
 * Given an emulator this returns the associated device
 *
 * @emul: Emulator for the device
 * @return device that @emul is emulating
 */
struct udevice *i2c_emul_get_device(struct udevice *emul);

#ifndef CONFIG_DM_I2C

/*
 * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 *
 * The implementation MUST NOT use static or global variables if the
 * I2C routines are used to read SDRAM configuration information
 * because this is done before the memories are initialized. Limited
 * use of stack-based variables are OK (the initial stack size is
 * limited).
 *
 * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 */

/*
 * Configuration items.
 */
#define I2C_RXTX_LEN	128	/* maximum tx/rx buffer length */

#if !defined(CONFIG_SYS_I2C_MAX_HOPS)
/* no muxes used bus = i2c adapters */
#define CONFIG_SYS_I2C_DIRECT_BUS	1
#define CONFIG_SYS_I2C_MAX_HOPS		0
#define CONFIG_SYS_NUM_I2C_BUSES	ll_entry_count(struct i2c_adapter, i2c)
#else
/* we use i2c muxes */
#undef CONFIG_SYS_I2C_DIRECT_BUS
#endif

/* define the I2C bus number for RTC and DTT if not already done */
#if !defined(CONFIG_SYS_RTC_BUS_NUM)
#define CONFIG_SYS_RTC_BUS_NUM		0
#endif
#if !defined(CONFIG_SYS_SPD_BUS_NUM)
#define CONFIG_SYS_SPD_BUS_NUM		0
#endif

struct i2c_adapter {
	void		(*init)(struct i2c_adapter *adap, int speed,
				int slaveaddr);
	int		(*probe)(struct i2c_adapter *adap, uint8_t chip);
	int		(*read)(struct i2c_adapter *adap, uint8_t chip,
				uint addr, int alen, uint8_t *buffer,
				int len);
	int		(*write)(struct i2c_adapter *adap, uint8_t chip,
				uint addr, int alen, uint8_t *buffer,
				int len);
	uint		(*set_bus_speed)(struct i2c_adapter *adap,
				uint speed);
	int		speed;
	int		waitdelay;
	int		slaveaddr;
	int		init_done;
	int		hwadapnr;
	char		*name;
};

#define U_BOOT_I2C_MKENT_COMPLETE(_init, _probe, _read, _write, \
		_set_speed, _speed, _slaveaddr, _hwadapnr, _name) \
	{ \
		.init		=	_init, \
		.probe		=	_probe, \
		.read		=	_read, \
		.write		=	_write, \
		.set_bus_speed	=	_set_speed, \
		.speed		=	_speed, \
		.slaveaddr	=	_slaveaddr, \
		.init_done	=	0, \
		.hwadapnr	=	_hwadapnr, \
		.name		=	#_name \
};

#define U_BOOT_I2C_ADAP_COMPLETE(_name, _init, _probe, _read, _write, \
			_set_speed, _speed, _slaveaddr, _hwadapnr) \
	ll_entry_declare(struct i2c_adapter, _name, i2c) = \
	U_BOOT_I2C_MKENT_COMPLETE(_init, _probe, _read, _write, \
		 _set_speed, _speed, _slaveaddr, _hwadapnr, _name);

struct i2c_adapter *i2c_get_adapter(int index);

#ifndef CONFIG_SYS_I2C_DIRECT_BUS
struct i2c_mux {
	int	id;
	char	name[16];
};

struct i2c_next_hop {
	struct i2c_mux		mux;
	uint8_t		chip;
	uint8_t		channel;
};

struct i2c_bus_hose {
	int	adapter;
	struct i2c_next_hop	next_hop[CONFIG_SYS_I2C_MAX_HOPS];
};
#define I2C_NULL_HOP	{{-1, ""}, 0, 0}
extern struct i2c_bus_hose	i2c_bus[];

#define I2C_ADAPTER(bus)	i2c_bus[bus].adapter
#else
#define I2C_ADAPTER(bus)	bus
#endif
#define	I2C_BUS			gd->cur_i2c_bus

#define	I2C_ADAP_NR(bus)	i2c_get_adapter(I2C_ADAPTER(bus))
#define	I2C_ADAP		I2C_ADAP_NR(gd->cur_i2c_bus)
#define I2C_ADAP_HWNR		(I2C_ADAP->hwadapnr)

#ifndef CONFIG_SYS_I2C_DIRECT_BUS
#define I2C_MUX_PCA9540_ID	1
#define I2C_MUX_PCA9540		{I2C_MUX_PCA9540_ID, "PCA9540B"}
#define I2C_MUX_PCA9542_ID	2
#define I2C_MUX_PCA9542		{I2C_MUX_PCA9542_ID, "PCA9542A"}
#define I2C_MUX_PCA9544_ID	3
#define I2C_MUX_PCA9544		{I2C_MUX_PCA9544_ID, "PCA9544A"}
#define I2C_MUX_PCA9547_ID	4
#define I2C_MUX_PCA9547		{I2C_MUX_PCA9547_ID, "PCA9547A"}
#define I2C_MUX_PCA9548_ID	5
#define I2C_MUX_PCA9548		{I2C_MUX_PCA9548_ID, "PCA9548"}
#endif

#ifndef I2C_SOFT_DECLARATIONS
# if (defined(CONFIG_AT91RM9200) || \
	defined(CONFIG_AT91SAM9260) ||  defined(CONFIG_AT91SAM9261) || \
	defined(CONFIG_AT91SAM9263))
#  define I2C_SOFT_DECLARATIONS	at91_pio_t *pio	= (at91_pio_t *) ATMEL_BASE_PIOA;
# else
#  define I2C_SOFT_DECLARATIONS
# endif
#endif

/*
 * Many boards/controllers/drivers don't support an I2C slave interface so
 * provide a default slave address for them for use in common code.  A real
 * value for CONFIG_SYS_I2C_SLAVE should be defined for any board which does
 * support a slave interface.
 */
#ifndef	CONFIG_SYS_I2C_SLAVE
#define	CONFIG_SYS_I2C_SLAVE	0xfe
#endif

/*
 * Initialization, must be called once on start up, may be called
 * repeatedly to change the speed and slave addresses.
 */
#ifdef CONFIG_SYS_I2C_EARLY_INIT
void i2c_early_init_f(void);
#endif
void i2c_init(int speed, int slaveaddr);
void i2c_init_board(void);

#ifdef CONFIG_SYS_I2C
/*
 * i2c_get_bus_num:
 *
 *  Returns index of currently active I2C bus.  Zero-based.
 */
unsigned int i2c_get_bus_num(void);

/*
 * i2c_set_bus_num:
 *
 *  Change the active I2C bus.  Subsequent read/write calls will
 *  go to this one.
 *
 *	bus - bus index, zero based
 *
 *	Returns: 0 on success, not 0 on failure
 *
 */
int i2c_set_bus_num(unsigned int bus);

/*
 * i2c_init_all():
 *
 * Initializes all I2C adapters in the system. All i2c_adap structures must
 * be initialized beforehead with function pointers and data, including
 * speed and slaveaddr. Returns 0 on success, non-0 on failure.
 */
void i2c_init_all(void);

/*
 * Probe the given I2C chip address.  Returns 0 if a chip responded,
 * not 0 on failure.
 */
int i2c_probe(uint8_t chip);

/*
 * Read/Write interface:
 *   chip:    I2C chip address, range 0..127
 *   addr:    Memory (register) address within the chip
 *   alen:    Number of bytes to use for addr (typically 1, 2 for larger
 *              memories, 0 for register type devices with only one
 *              register)
 *   buffer:  Where to read/write the data
 *   len:     How many bytes to read/write
 *
 *   Returns: 0 on success, not 0 on failure
 */
int i2c_read(uint8_t chip, unsigned int addr, int alen,
				uint8_t *buffer, int len);

int i2c_write(uint8_t chip, unsigned int addr, int alen,
				uint8_t *buffer, int len);

/*
 * Utility routines to read/write registers.
 */
uint8_t i2c_reg_read(uint8_t addr, uint8_t reg);

void i2c_reg_write(uint8_t addr, uint8_t reg, uint8_t val);

/*
 * i2c_set_bus_speed:
 *
 *  Change the speed of the active I2C bus
 *
 *	speed - bus speed in Hz
 *
 *	Returns: new bus speed
 *
 */
unsigned int i2c_set_bus_speed(unsigned int speed);

/*
 * i2c_get_bus_speed:
 *
 *  Returns speed of currently active I2C bus in Hz
 */

unsigned int i2c_get_bus_speed(void);

#else

/*
 * Probe the given I2C chip address.  Returns 0 if a chip responded,
 * not 0 on failure.
 */
int i2c_probe(uchar chip);

/*
 * Read/Write interface:
 *   chip:    I2C chip address, range 0..127
 *   addr:    Memory (register) address within the chip
 *   alen:    Number of bytes to use for addr (typically 1, 2 for larger
 *              memories, 0 for register type devices with only one
 *              register)
 *   buffer:  Where to read/write the data
 *   len:     How many bytes to read/write
 *
 *   Returns: 0 on success, not 0 on failure
 */
int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len);
int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len);

/*
 * Utility routines to read/write registers.
 */
static inline u8 i2c_reg_read(u8 addr, u8 reg)
{
	u8 buf;

#ifdef DEBUG
	printf("%s: addr=0x%02x, reg=0x%02x\n", __func__, addr, reg);
#endif

	i2c_read(addr, reg, 1, &buf, 1);

	return buf;
}

static inline void i2c_reg_write(u8 addr, u8 reg, u8 val)
{
#ifdef DEBUG
	printf("%s: addr=0x%02x, reg=0x%02x, val=0x%02x\n",
	       __func__, addr, reg, val);
#endif

	i2c_write(addr, reg, 1, &val, 1);
}

/*
 * Functions for setting the current I2C bus and its speed
 */

/*
 * i2c_set_bus_num:
 *
 *  Change the active I2C bus.  Subsequent read/write calls will
 *  go to this one.
 *
 *	bus - bus index, zero based
 *
 *	Returns: 0 on success, not 0 on failure
 *
 */
int i2c_set_bus_num(unsigned int bus);

/*
 * i2c_get_bus_num:
 *
 *  Returns index of currently active I2C bus.  Zero-based.
 */

unsigned int i2c_get_bus_num(void);

/*
 * i2c_set_bus_speed:
 *
 *  Change the speed of the active I2C bus
 *
 *	speed - bus speed in Hz
 *
 *	Returns: 0 on success, not 0 on failure
 *
 */
int i2c_set_bus_speed(unsigned int);

/*
 * i2c_get_bus_speed:
 *
 *  Returns speed of currently active I2C bus in Hz
 */

unsigned int i2c_get_bus_speed(void);
#endif /* CONFIG_SYS_I2C */

/*
 * only for backwardcompatibility, should go away if we switched
 * completely to new multibus support.
 */
#if defined(CONFIG_SYS_I2C) || defined(CONFIG_I2C_MULTI_BUS)
# if !defined(CONFIG_SYS_MAX_I2C_BUS)
#  define CONFIG_SYS_MAX_I2C_BUS		2
# endif
# define I2C_MULTI_BUS				1
#else
# define CONFIG_SYS_MAX_I2C_BUS		1
# define I2C_MULTI_BUS				0
#endif

/* NOTE: These two functions MUST be always_inline to avoid code growth! */
static inline unsigned int I2C_GET_BUS(void) __attribute__((always_inline));
static inline unsigned int I2C_GET_BUS(void)
{
	return I2C_MULTI_BUS ? i2c_get_bus_num() : 0;
}

static inline void I2C_SET_BUS(unsigned int bus) __attribute__((always_inline));
static inline void I2C_SET_BUS(unsigned int bus)
{
	if (I2C_MULTI_BUS)
		i2c_set_bus_num(bus);
}

/* Multi I2C definitions */
enum {
	I2C_0, I2C_1, I2C_2, I2C_3, I2C_4, I2C_5, I2C_6, I2C_7,
	I2C_8, I2C_9, I2C_10,
};

/**
 * Get FDT values for i2c bus.
 *
 * @param blob  Device tree blbo
 * @return the number of I2C bus
 */
void board_i2c_init(const void *blob);

/**
 * Find the I2C bus number by given a FDT I2C node.
 *
 * @param blob  Device tree blbo
 * @param node  FDT I2C node to find
 * @return the number of I2C bus (zero based), or -1 on error
 */
int i2c_get_bus_num_fdt(int node);

/**
 * Reset the I2C bus represented by the given a FDT I2C node.
 *
 * @param blob  Device tree blbo
 * @param node  FDT I2C node to find
 * @return 0 if port was reset, -1 if not found
 */
int i2c_reset_port_fdt(const void *blob, int node);

#endif /* !CONFIG_DM_I2C */

#endif	/* _I2C_H_ */
