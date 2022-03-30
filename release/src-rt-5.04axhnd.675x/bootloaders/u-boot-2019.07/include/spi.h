/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common SPI Interface: Controller-specific definitions
 *
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 */

#ifndef _SPI_H_
#define _SPI_H_

#include <common.h>

/* SPI mode flags */
#define SPI_CPHA	BIT(0)			/* clock phase */
#define SPI_CPOL	BIT(1)			/* clock polarity */
#define SPI_MODE_0	(0|0)			/* (original MicroWire) */
#define SPI_MODE_1	(0|SPI_CPHA)
#define SPI_MODE_2	(SPI_CPOL|0)
#define SPI_MODE_3	(SPI_CPOL|SPI_CPHA)
#define SPI_CS_HIGH	BIT(2)			/* CS active high */
#define SPI_LSB_FIRST	BIT(3)			/* per-word bits-on-wire */
#define SPI_3WIRE	BIT(4)			/* SI/SO signals shared */
#define SPI_LOOP	BIT(5)			/* loopback mode */
#define SPI_SLAVE	BIT(6)			/* slave mode */
#define SPI_PREAMBLE	BIT(7)			/* Skip preamble bytes */
#define SPI_TX_BYTE	BIT(8)			/* transmit with 1 wire byte */
#define SPI_TX_DUAL	BIT(9)			/* transmit with 2 wires */
#define SPI_TX_QUAD	BIT(10)			/* transmit with 4 wires */
#define SPI_RX_SLOW	BIT(11)			/* receive with 1 wire slow */
#define SPI_RX_DUAL	BIT(12)			/* receive with 2 wires */
#define SPI_RX_QUAD	BIT(13)			/* receive with 4 wires */

/* Header byte that marks the start of the message */
#define SPI_PREAMBLE_END_BYTE	0xec

#define SPI_DEFAULT_WORDLEN	8

#ifdef CONFIG_DM_SPI
/* TODO(sjg@chromium.org): Remove this and use max_hz from struct spi_slave */
struct dm_spi_bus {
	uint max_hz;
};

/**
 * struct dm_spi_platdata - platform data for all SPI slaves
 *
 * This describes a SPI slave, a child device of the SPI bus. To obtain this
 * struct from a spi_slave, use dev_get_parent_platdata(dev) or
 * dev_get_parent_platdata(slave->dev).
 *
 * This data is immuatable. Each time the device is probed, @max_hz and @mode
 * will be copied to struct spi_slave.
 *
 * @cs:		Chip select number (0..n-1)
 * @max_hz:	Maximum bus speed that this slave can tolerate
 * @mode:	SPI mode to use for this device (see SPI mode flags)
 */
struct dm_spi_slave_platdata {
	unsigned int cs;
	uint max_hz;
	uint mode;
};

#endif /* CONFIG_DM_SPI */

/**
 * struct spi_slave - Representation of a SPI slave
 *
 * For driver model this is the per-child data used by the SPI bus. It can
 * be accessed using dev_get_parent_priv() on the slave device. The SPI uclass
 * sets uip per_child_auto_alloc_size to sizeof(struct spi_slave), and the
 * driver should not override it. Two platform data fields (max_hz and mode)
 * are copied into this structure to provide an initial value. This allows
 * them to be changed, since we should never change platform data in drivers.
 *
 * If not using driver model, drivers are expected to extend this with
 * controller-specific data.
 *
 * @dev:		SPI slave device
 * @max_hz:		Maximum speed for this slave
 * @speed:		Current bus speed. This is 0 until the bus is first
 *			claimed.
 * @bus:		ID of the bus that the slave is attached to. For
 *			driver model this is the sequence number of the SPI
 *			bus (bus->seq) so does not need to be stored
 * @cs:			ID of the chip select connected to the slave.
 * @mode:		SPI mode to use for this slave (see SPI mode flags)
 * @wordlen:		Size of SPI word in number of bits
 * @max_read_size:	If non-zero, the maximum number of bytes which can
 *			be read at once.
 * @max_write_size:	If non-zero, the maximum number of bytes which can
 *			be written at once.
 * @memory_map:		Address of read-only SPI flash access.
 * @flags:		Indication of SPI flags.
 */
struct spi_slave {
#ifdef CONFIG_DM_SPI
	struct udevice *dev;	/* struct spi_slave is dev->parentdata */
	uint max_hz;
	uint speed;
#else
	unsigned int bus;
	unsigned int cs;
#endif
	uint mode;
	unsigned int wordlen;
	unsigned int max_read_size;
	unsigned int max_write_size;
	void *memory_map;

	u8 flags;
#define SPI_XFER_BEGIN		BIT(0)	/* Assert CS before transfer */
#define SPI_XFER_END		BIT(1)	/* Deassert CS after transfer */
#define SPI_XFER_ONCE		(SPI_XFER_BEGIN | SPI_XFER_END)
#define SPI_XFER_MMAP		BIT(2)	/* Memory Mapped start */
#define SPI_XFER_MMAP_END	BIT(3)	/* Memory Mapped End */
};

/**
 * spi_do_alloc_slave - Allocate a new SPI slave (internal)
 *
 * Allocate and zero all fields in the spi slave, and set the bus/chip
 * select. Use the helper macro spi_alloc_slave() to call this.
 *
 * @offset:	Offset of struct spi_slave within slave structure.
 * @size:	Size of slave structure.
 * @bus:	Bus ID of the slave chip.
 * @cs:		Chip select ID of the slave chip on the specified bus.
 */
void *spi_do_alloc_slave(int offset, int size, unsigned int bus,
			 unsigned int cs);

/**
 * spi_alloc_slave - Allocate a new SPI slave
 *
 * Allocate and zero all fields in the spi slave, and set the bus/chip
 * select.
 *
 * @_struct:	Name of structure to allocate (e.g. struct tegra_spi).
 *		This structure must contain a member 'struct spi_slave *slave'.
 * @bus:	Bus ID of the slave chip.
 * @cs:		Chip select ID of the slave chip on the specified bus.
 */
#define spi_alloc_slave(_struct, bus, cs) \
	spi_do_alloc_slave(offsetof(_struct, slave), \
			    sizeof(_struct), bus, cs)

/**
 * spi_alloc_slave_base - Allocate a new SPI slave with no private data
 *
 * Allocate and zero all fields in the spi slave, and set the bus/chip
 * select.
 *
 * @bus:	Bus ID of the slave chip.
 * @cs:		Chip select ID of the slave chip on the specified bus.
 */
#define spi_alloc_slave_base(bus, cs) \
	spi_do_alloc_slave(0, sizeof(struct spi_slave), bus, cs)

/**
 * Set up communications parameters for a SPI slave.
 *
 * This must be called once for each slave. Note that this function
 * usually doesn't touch any actual hardware, it only initializes the
 * contents of spi_slave so that the hardware can be easily
 * initialized later.
 *
 * @bus:	Bus ID of the slave chip.
 * @cs:		Chip select ID of the slave chip on the specified bus.
 * @max_hz:	Maximum SCK rate in Hz.
 * @mode:	Clock polarity, clock phase and other parameters.
 *
 * Returns: A spi_slave reference that can be used in subsequent SPI
 * calls, or NULL if one or more of the parameters are not supported.
 */
struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode);

/**
 * Free any memory associated with a SPI slave.
 *
 * @slave:	The SPI slave
 */
void spi_free_slave(struct spi_slave *slave);

/**
 * Claim the bus and prepare it for communication with a given slave.
 *
 * This must be called before doing any transfers with a SPI slave. It
 * will enable and initialize any SPI hardware as necessary, and make
 * sure that the SCK line is in the correct idle state. It is not
 * allowed to claim the same bus for several slaves without releasing
 * the bus in between.
 *
 * @slave:	The SPI slave
 *
 * Returns: 0 if the bus was claimed successfully, or a negative value
 * if it wasn't.
 */
int spi_claim_bus(struct spi_slave *slave);

/**
 * Release the SPI bus
 *
 * This must be called once for every call to spi_claim_bus() after
 * all transfers have finished. It may disable any SPI hardware as
 * appropriate.
 *
 * @slave:	The SPI slave
 */
void spi_release_bus(struct spi_slave *slave);

/**
 * Set the word length for SPI transactions
 *
 * Set the word length (number of bits per word) for SPI transactions.
 *
 * @slave:	The SPI slave
 * @wordlen:	The number of bits in a word
 *
 * Returns: 0 on success, -1 on failure.
 */
int spi_set_wordlen(struct spi_slave *slave, unsigned int wordlen);

/**
 * SPI transfer
 *
 * This writes "bitlen" bits out the SPI MOSI port and simultaneously clocks
 * "bitlen" bits in the SPI MISO port.  That's just the way SPI works.
 *
 * The source of the outgoing bits is the "dout" parameter and the
 * destination of the input bits is the "din" parameter.  Note that "dout"
 * and "din" can point to the same memory location, in which case the
 * input data overwrites the output data (since both are buffered by
 * temporary variables, this is OK).
 *
 * spi_xfer() interface:
 * @slave:	The SPI slave which will be sending/receiving the data.
 * @bitlen:	How many bits to write and read.
 * @dout:	Pointer to a string of bits to send out.  The bits are
 *		held in a byte array and are sent MSB first.
 * @din:	Pointer to a string of bits that will be filled in.
 * @flags:	A bitwise combination of SPI_XFER_* flags.
 *
 * Returns: 0 on success, not 0 on failure
 */
int  spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
		void *din, unsigned long flags);

/* Copy memory mapped data */
void spi_flash_copy_mmap(void *data, void *offset, size_t len);

/**
 * Determine if a SPI chipselect is valid.
 * This function is provided by the board if the low-level SPI driver
 * needs it to determine if a given chipselect is actually valid.
 *
 * Returns: 1 if bus:cs identifies a valid chip on this board, 0
 * otherwise.
 */
int spi_cs_is_valid(unsigned int bus, unsigned int cs);

#ifndef CONFIG_DM_SPI
/**
 * Activate a SPI chipselect.
 * This function is provided by the board code when using a driver
 * that can't control its chipselects automatically (e.g.
 * common/soft_spi.c). When called, it should activate the chip select
 * to the device identified by "slave".
 */
void spi_cs_activate(struct spi_slave *slave);

/**
 * Deactivate a SPI chipselect.
 * This function is provided by the board code when using a driver
 * that can't control its chipselects automatically (e.g.
 * common/soft_spi.c). When called, it should deactivate the chip
 * select to the device identified by "slave".
 */
void spi_cs_deactivate(struct spi_slave *slave);

/**
 * Set transfer speed.
 * This sets a new speed to be applied for next spi_xfer().
 * @slave:	The SPI slave
 * @hz:		The transfer speed
 */
void spi_set_speed(struct spi_slave *slave, uint hz);
#endif

/**
 * Write 8 bits, then read 8 bits.
 * @slave:	The SPI slave we're communicating with
 * @byte:	Byte to be written
 *
 * Returns: The value that was read, or a negative value on error.
 *
 * TODO: This function probably shouldn't be inlined.
 */
static inline int spi_w8r8(struct spi_slave *slave, unsigned char byte)
{
	unsigned char dout[2];
	unsigned char din[2];
	int ret;

	dout[0] = byte;
	dout[1] = 0;

	ret = spi_xfer(slave, 16, dout, din, SPI_XFER_BEGIN | SPI_XFER_END);
	return ret < 0 ? ret : din[1];
}

#ifdef CONFIG_DM_SPI

/**
 * struct spi_cs_info - Information about a bus chip select
 *
 * @dev:	Connected device, or NULL if none
 */
struct spi_cs_info {
	struct udevice *dev;
};

/**
 * struct struct dm_spi_ops - Driver model SPI operations
 *
 * The uclass interface is implemented by all SPI devices which use
 * driver model.
 */
struct dm_spi_ops {
	/**
	 * Claim the bus and prepare it for communication.
	 *
	 * The device provided is the slave device. It's parent controller
	 * will be used to provide the communication.
	 *
	 * This must be called before doing any transfers with a SPI slave. It
	 * will enable and initialize any SPI hardware as necessary, and make
	 * sure that the SCK line is in the correct idle state. It is not
	 * allowed to claim the same bus for several slaves without releasing
	 * the bus in between.
	 *
	 * @dev:	The SPI slave
	 *
	 * Returns: 0 if the bus was claimed successfully, or a negative value
	 * if it wasn't.
	 */
	int (*claim_bus)(struct udevice *dev);

	/**
	 * Release the SPI bus
	 *
	 * This must be called once for every call to spi_claim_bus() after
	 * all transfers have finished. It may disable any SPI hardware as
	 * appropriate.
	 *
	 * @dev:	The SPI slave
	 */
	int (*release_bus)(struct udevice *dev);

	/**
	 * Set the word length for SPI transactions
	 *
	 * Set the word length (number of bits per word) for SPI transactions.
	 *
	 * @bus:	The SPI slave
	 * @wordlen:	The number of bits in a word
	 *
	 * Returns: 0 on success, -ve on failure.
	 */
	int (*set_wordlen)(struct udevice *dev, unsigned int wordlen);

	/**
	 * SPI transfer
	 *
	 * This writes "bitlen" bits out the SPI MOSI port and simultaneously
	 * clocks "bitlen" bits in the SPI MISO port.  That's just the way SPI
	 * works.
	 *
	 * The source of the outgoing bits is the "dout" parameter and the
	 * destination of the input bits is the "din" parameter.  Note that
	 * "dout" and "din" can point to the same memory location, in which
	 * case the input data overwrites the output data (since both are
	 * buffered by temporary variables, this is OK).
	 *
	 * spi_xfer() interface:
	 * @dev:	The slave device to communicate with
	 * @bitlen:	How many bits to write and read.
	 * @dout:	Pointer to a string of bits to send out.  The bits are
	 *		held in a byte array and are sent MSB first.
	 * @din:	Pointer to a string of bits that will be filled in.
	 * @flags:	A bitwise combination of SPI_XFER_* flags.
	 *
	 * Returns: 0 on success, not -1 on failure
	 */
	int (*xfer)(struct udevice *dev, unsigned int bitlen, const void *dout,
		    void *din, unsigned long flags);

	/**
	 * Optimized handlers for SPI memory-like operations.
	 *
	 * Optimized/dedicated operations for interactions with SPI memory. This
	 * field is optional and should only be implemented if the controller
	 * has native support for memory like operations.
	 */
	const struct spi_controller_mem_ops *mem_ops;

	/**
	 * Set transfer speed.
	 * This sets a new speed to be applied for next spi_xfer().
	 * @bus:	The SPI bus
	 * @hz:		The transfer speed
	 * @return 0 if OK, -ve on error
	 */
	int (*set_speed)(struct udevice *bus, uint hz);

	/**
	 * Set the SPI mode/flags
	 *
	 * It is unclear if we want to set speed and mode together instead
	 * of separately.
	 *
	 * @bus:	The SPI bus
	 * @mode:	Requested SPI mode (SPI_... flags)
	 * @return 0 if OK, -ve on error
	 */
	int (*set_mode)(struct udevice *bus, uint mode);

	/**
	 * Get information on a chip select
	 *
	 * This is only called when the SPI uclass does not know about a
	 * chip select, i.e. it has no attached device. It gives the driver
	 * a chance to allow activity on that chip select even so.
	 *
	 * @bus:	The SPI bus
	 * @cs:		The chip select (0..n-1)
	 * @info:	Returns information about the chip select, if valid.
	 *		On entry info->dev is NULL
	 * @return 0 if OK (and @info is set up), -ENODEV if the chip select
	 *	   is invalid, other -ve value on error
	 */
	int (*cs_info)(struct udevice *bus, uint cs, struct spi_cs_info *info);
};

struct dm_spi_emul_ops {
	/**
	 * SPI transfer
	 *
	 * This writes "bitlen" bits out the SPI MOSI port and simultaneously
	 * clocks "bitlen" bits in the SPI MISO port.  That's just the way SPI
	 * works. Here the device is a slave.
	 *
	 * The source of the outgoing bits is the "dout" parameter and the
	 * destination of the input bits is the "din" parameter.  Note that
	 * "dout" and "din" can point to the same memory location, in which
	 * case the input data overwrites the output data (since both are
	 * buffered by temporary variables, this is OK).
	 *
	 * spi_xfer() interface:
	 * @slave:	The SPI slave which will be sending/receiving the data.
	 * @bitlen:	How many bits to write and read.
	 * @dout:	Pointer to a string of bits sent to the device. The
	 *		bits are held in a byte array and are sent MSB first.
	 * @din:	Pointer to a string of bits that will be sent back to
	 *		the master.
	 * @flags:	A bitwise combination of SPI_XFER_* flags.
	 *
	 * Returns: 0 on success, not -1 on failure
	 */
	int (*xfer)(struct udevice *slave, unsigned int bitlen,
		    const void *dout, void *din, unsigned long flags);
};

/**
 * spi_find_bus_and_cs() - Find bus and slave devices by number
 *
 * Given a bus number and chip select, this finds the corresponding bus
 * device and slave device. Neither device is activated by this function,
 * although they may have been activated previously.
 *
 * @busnum:	SPI bus number
 * @cs:		Chip select to look for
 * @busp:	Returns bus device
 * @devp:	Return slave device
 * @return 0 if found, -ENODEV on error
 */
int spi_find_bus_and_cs(int busnum, int cs, struct udevice **busp,
			struct udevice **devp);

/**
 * spi_get_bus_and_cs() - Find and activate bus and slave devices by number
 *
 * Given a bus number and chip select, this finds the corresponding bus
 * device and slave device.
 *
 * If no such slave exists, and drv_name is not NULL, then a new slave device
 * is automatically bound on this chip select with requested speed and mode.
 *
 * Ths new slave device is probed ready for use with the speed and mode
 * from platdata when available or the requested values.
 *
 * @busnum:	SPI bus number
 * @cs:		Chip select to look for
 * @speed:	SPI speed to use for this slave when not available in platdata
 * @mode:	SPI mode to use for this slave when not available in platdata
 * @drv_name:	Name of driver to attach to this chip select
 * @dev_name:	Name of the new device thus created
 * @busp:	Returns bus device
 * @devp:	Return slave device
 * @return 0 if found, -ve on error
 */
int spi_get_bus_and_cs(int busnum, int cs, int speed, int mode,
			const char *drv_name, const char *dev_name,
			struct udevice **busp, struct spi_slave **devp);

/**
 * spi_chip_select() - Get the chip select for a slave
 *
 * @return the chip select this slave is attached to
 */
int spi_chip_select(struct udevice *slave);

/**
 * spi_find_chip_select() - Find the slave attached to chip select
 *
 * @bus:	SPI bus to search
 * @cs:		Chip select to look for
 * @devp:	Returns the slave device if found
 * @return 0 if found, -ENODEV on error
 */
int spi_find_chip_select(struct udevice *bus, int cs, struct udevice **devp);

/**
 * spi_slave_ofdata_to_platdata() - decode standard SPI platform data
 *
 * This decodes the speed and mode for a slave from a device tree node
 *
 * @blob:	Device tree blob
 * @node:	Node offset to read from
 * @plat:	Place to put the decoded information
 */
int spi_slave_ofdata_to_platdata(struct udevice *dev,
				 struct dm_spi_slave_platdata *plat);

/**
 * spi_cs_info() - Check information on a chip select
 *
 * This checks a particular chip select on a bus to see if it has a device
 * attached, or is even valid.
 *
 * @bus:	The SPI bus
 * @cs:		The chip select (0..n-1)
 * @info:	Returns information about the chip select, if valid
 * @return 0 if OK (and @info is set up), -ENODEV if the chip select
 *	   is invalid, other -ve value on error
 */
int spi_cs_info(struct udevice *bus, uint cs, struct spi_cs_info *info);

struct sandbox_state;

/**
 * sandbox_spi_get_emul() - get an emulator for a SPI slave
 *
 * This provides a way to attach an emulated SPI device to a particular SPI
 * slave, so that xfer() operations on the slave will be handled by the
 * emulator. If a emulator already exists on that chip select it is returned.
 * Otherwise one is created.
 *
 * @state:	Sandbox state
 * @bus:	SPI bus requesting the emulator
 * @slave:	SPI slave device requesting the emulator
 * @emuip:	Returns pointer to emulator
 * @return 0 if OK, -ve on error
 */
int sandbox_spi_get_emul(struct sandbox_state *state,
			 struct udevice *bus, struct udevice *slave,
			 struct udevice **emulp);

/**
 * Claim the bus and prepare it for communication with a given slave.
 *
 * This must be called before doing any transfers with a SPI slave. It
 * will enable and initialize any SPI hardware as necessary, and make
 * sure that the SCK line is in the correct idle state. It is not
 * allowed to claim the same bus for several slaves without releasing
 * the bus in between.
 *
 * @dev:	The SPI slave device
 *
 * Returns: 0 if the bus was claimed successfully, or a negative value
 * if it wasn't.
 */
int dm_spi_claim_bus(struct udevice *dev);

/**
 * Release the SPI bus
 *
 * This must be called once for every call to dm_spi_claim_bus() after
 * all transfers have finished. It may disable any SPI hardware as
 * appropriate.
 *
 * @slave:	The SPI slave device
 */
void dm_spi_release_bus(struct udevice *dev);

/**
 * SPI transfer
 *
 * This writes "bitlen" bits out the SPI MOSI port and simultaneously clocks
 * "bitlen" bits in the SPI MISO port.  That's just the way SPI works.
 *
 * The source of the outgoing bits is the "dout" parameter and the
 * destination of the input bits is the "din" parameter.  Note that "dout"
 * and "din" can point to the same memory location, in which case the
 * input data overwrites the output data (since both are buffered by
 * temporary variables, this is OK).
 *
 * dm_spi_xfer() interface:
 * @dev:	The SPI slave device which will be sending/receiving the data.
 * @bitlen:	How many bits to write and read.
 * @dout:	Pointer to a string of bits to send out.  The bits are
 *		held in a byte array and are sent MSB first.
 * @din:	Pointer to a string of bits that will be filled in.
 * @flags:	A bitwise combination of SPI_XFER_* flags.
 *
 * Returns: 0 on success, not 0 on failure
 */
int dm_spi_xfer(struct udevice *dev, unsigned int bitlen,
		const void *dout, void *din, unsigned long flags);

/* Access the operations for a SPI device */
#define spi_get_ops(dev)	((struct dm_spi_ops *)(dev)->driver->ops)
#define spi_emul_get_ops(dev)	((struct dm_spi_emul_ops *)(dev)->driver->ops)
#endif /* CONFIG_DM_SPI */

#endif	/* _SPI_H_ */
