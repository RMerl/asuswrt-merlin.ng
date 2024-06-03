#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <post.h>

struct serial_device {
	/* enough bytes to match alignment of following func pointer */
	char	name[16];

	int	(*start)(void);
	int	(*stop)(void);
	void	(*setbrg)(void);
	int	(*getc)(void);
	int	(*tstc)(void);
	void	(*putc)(const char c);
	void	(*puts)(const char *s);
#if CONFIG_POST & CONFIG_SYS_POST_UART
	void	(*loop)(int);
#endif
	struct serial_device	*next;
};

void default_serial_puts(const char *s);

extern struct serial_device serial_smc_device;
extern struct serial_device serial_scc_device;
extern struct serial_device *default_serial_console(void);

#if	defined(CONFIG_MPC83xx) || defined(CONFIG_MPC85xx) || \
	defined(CONFIG_MPC86xx) || \
	defined(CONFIG_TEGRA) || defined(CONFIG_SYS_COREBOOT) || \
	defined(CONFIG_MICROBLAZE)
extern struct serial_device serial0_device;
extern struct serial_device serial1_device;
#endif

extern struct serial_device eserial1_device;
extern struct serial_device eserial2_device;
extern struct serial_device eserial3_device;
extern struct serial_device eserial4_device;
extern struct serial_device eserial5_device;
extern struct serial_device eserial6_device;

extern void serial_register(struct serial_device *);
extern void serial_initialize(void);
extern void serial_stdio_init(void);
extern int serial_assign(const char *name);
extern void serial_reinit_all(void);

/* For usbtty */
#ifdef CONFIG_USB_TTY

extern int usbtty_getc(void);
extern void usbtty_putc(const char c);
extern void usbtty_puts(const char *str);
extern int usbtty_tstc(void);

#else

/* stubs */
#define usbtty_getc() 0
#define usbtty_putc(a)
#define usbtty_puts(a)
#define usbtty_tstc() 0

#endif /* CONFIG_USB_TTY */

struct udevice;

enum serial_par {
	SERIAL_PAR_NONE,
	SERIAL_PAR_ODD,
	SERIAL_PAR_EVEN
};

#define SERIAL_PAR_SHIFT	0
#define SERIAL_PAR_MASK		(0x03 << SERIAL_PAR_SHIFT)
#define SERIAL_SET_PARITY(parity) \
	((parity << SERIAL_PAR_SHIFT) & SERIAL_PAR_MASK)
#define SERIAL_GET_PARITY(config) \
	((config & SERIAL_PAR_MASK) >> SERIAL_PAR_SHIFT)

enum serial_bits {
	SERIAL_5_BITS,
	SERIAL_6_BITS,
	SERIAL_7_BITS,
	SERIAL_8_BITS
};

#define SERIAL_BITS_SHIFT	2
#define SERIAL_BITS_MASK	(0x3 << SERIAL_BITS_SHIFT)
#define SERIAL_SET_BITS(bits) \
	((bits << SERIAL_BITS_SHIFT) & SERIAL_BITS_MASK)
#define SERIAL_GET_BITS(config) \
	((config & SERIAL_BITS_MASK) >> SERIAL_BITS_SHIFT)

enum serial_stop {
	SERIAL_HALF_STOP,	/* 0.5 stop bit */
	SERIAL_ONE_STOP,	/*   1 stop bit */
	SERIAL_ONE_HALF_STOP,	/* 1.5 stop bit */
	SERIAL_TWO_STOP		/*   2 stop bit */
};

#define SERIAL_STOP_SHIFT	4
#define SERIAL_STOP_MASK	(0x3 << SERIAL_STOP_SHIFT)
#define SERIAL_SET_STOP(stop) \
	((stop << SERIAL_STOP_SHIFT) & SERIAL_STOP_MASK)
#define SERIAL_GET_STOP(config) \
	((config & SERIAL_STOP_MASK) >> SERIAL_STOP_SHIFT)

#define SERIAL_CONFIG(par, bits, stop) \
		     (par << SERIAL_PAR_SHIFT | \
		      bits << SERIAL_BITS_SHIFT | \
		      stop << SERIAL_STOP_SHIFT)

#define SERIAL_DEFAULT_CONFIG \
			(SERIAL_PAR_NONE << SERIAL_PAR_SHIFT | \
			 SERIAL_8_BITS << SERIAL_BITS_SHIFT | \
			 SERIAL_ONE_STOP << SERIAL_STOP_SHIFT)

enum serial_chip_type {
	SERIAL_CHIP_UNKNOWN = -1,
	SERIAL_CHIP_16550_COMPATIBLE,
};

enum adr_space_type {
	SERIAL_ADDRESS_SPACE_MEMORY = 0,
	SERIAL_ADDRESS_SPACE_IO,
};

/**
 * struct serial_device_info - structure to hold serial device info
 *
 * @type:	type of the UART chip
 * @addr_space:	address space to access the registers
 * @addr:	physical address of the registers
 * @reg_width:	size (in bytes) of the IO accesses to the registers
 * @reg_offset:	offset to apply to the @addr from the start of the registers
 * @reg_shift:	quantity to shift the register offsets by
 * @baudrate:	baud rate
 */
struct serial_device_info {
	enum serial_chip_type type;
	enum adr_space_type addr_space;
	ulong addr;
	u8 reg_width;
	u8 reg_offset;
	u8 reg_shift;
	unsigned int baudrate;
};

#define SERIAL_DEFAULT_ADDRESS	0xBADACCE5

/**
 * struct struct dm_serial_ops - Driver model serial operations
 *
 * The uclass interface is implemented by all serial devices which use
 * driver model.
 */
struct dm_serial_ops {
	/**
	 * setbrg() - Set up the baud rate generator
	 *
	 * Adjust baud rate divisors to set up a new baud rate for this
	 * device. Not all devices will support all rates. If the rate
	 * cannot be supported, the driver is free to select the nearest
	 * available rate. or return -EINVAL if this is not possible.
	 *
	 * @dev: Device pointer
	 * @baudrate: New baud rate to use
	 * @return 0 if OK, -ve on error
	 */
	int (*setbrg)(struct udevice *dev, int baudrate);
	/**
	 * getc() - Read a character and return it
	 *
	 * If no character is available, this should return -EAGAIN without
	 * waiting.
	 *
	 * @dev: Device pointer
	 * @return character (0..255), -ve on error
	 */
	int (*getc)(struct udevice *dev);
	/**
	 * putc() - Write a character
	 *
	 * @dev: Device pointer
	 * @ch: character to write
	 * @return 0 if OK, -ve on error
	 */
	int (*putc)(struct udevice *dev, const char ch);
	/**
	 * pending() - Check if input/output characters are waiting
	 *
	 * This can be used to return an indication of the number of waiting
	 * characters if the driver knows this (e.g. by looking at the FIFO
	 * level). It is acceptable to return 1 if an indeterminant number
	 * of characters is waiting.
	 *
	 * This method is optional.
	 *
	 * @dev: Device pointer
	 * @input: true to check input characters, false for output
	 * @return number of waiting characters, 0 for none, -ve on error
	 */
	int (*pending)(struct udevice *dev, bool input);
	/**
	 * clear() - Clear the serial FIFOs/holding registers
	 *
	 * This method is optional.
	 *
	 * This quickly clears any input/output characters from the UART.
	 * If this is not possible, but characters still exist, then it
	 * is acceptable to return -EAGAIN (try again) or -EINVAL (not
	 * supported).
	 *
	 * @dev: Device pointer
	 * @return 0 if OK, -ve on error
	 */
	int (*clear)(struct udevice *dev);
#if CONFIG_POST & CONFIG_SYS_POST_UART
	/**
	 * loop() - Control serial device loopback mode
	 *
	 * @dev: Device pointer
	 * @on: 1 to turn loopback on, 0 to turn if off
	 */
	int (*loop)(struct udevice *dev, int on);
#endif

	/**
	 * getconfig() - Get the uart configuration
	 * (parity, 5/6/7/8 bits word length, stop bits)
	 *
	 * Get a current config for this device.
	 *
	 * @dev: Device pointer
	 * @serial_config: Returns config information (see SERIAL_... above)
	 * @return 0 if OK, -ve on error
	 */
	int (*getconfig)(struct udevice *dev, uint *serial_config);
	/**
	 * setconfig() - Set up the uart configuration
	 * (parity, 5/6/7/8 bits word length, stop bits)
	 *
	 * Set up a new config for this device.
	 *
	 * @dev: Device pointer
	 * @serial_config: number of bits, parity and number of stopbits to use
	 * @return 0 if OK, -ve on error
	 */
	int (*setconfig)(struct udevice *dev, uint serial_config);
	/**
	 * getinfo() - Get serial device information
	 *
	 * @dev: Device pointer
	 * @info: struct serial_device_info to fill
	 * @return 0 if OK, -ve on error
	 */
	int (*getinfo)(struct udevice *dev, struct serial_device_info *info);
};

/**
 * struct serial_dev_priv - information about a device used by the uclass
 *
 * @sdev:	stdio device attached to this uart
 *
 * @buf:	Pointer to the RX buffer
 * @rd_ptr:	Read pointer in the RX buffer
 * @wr_ptr:	Write pointer in the RX buffer
 */
struct serial_dev_priv {
	struct stdio_dev *sdev;

	char *buf;
	int rd_ptr;
	int wr_ptr;
};

/* Access the serial operations for a device */
#define serial_get_ops(dev)	((struct dm_serial_ops *)(dev)->driver->ops)

/**
 * serial_getconfig() - Get the uart configuration
 * (parity, 5/6/7/8 bits word length, stop bits)
 *
 * Get a current config for this device.
 *
 * @dev: Device pointer
 * @serial_config: Returns config information (see SERIAL_... above)
 * @return 0 if OK, -ve on error
 */
int serial_getconfig(struct udevice *dev, uint *config);

/**
 * serial_setconfig() - Set up the uart configuration
 * (parity, 5/6/7/8 bits word length, stop bits)
 *
 * Set up a new config for this device.
 *
 * @dev: Device pointer
 * @serial_config: number of bits, parity and number of stopbits to use
 * @return 0 if OK, -ve on error
 */
int serial_setconfig(struct udevice *dev, uint config);

/**
 * serial_getinfo() - Get serial device information
 *
 * @dev: Device pointer
 * @info: struct serial_device_info to fill
 * @return 0 if OK, -ve on error
 */
int serial_getinfo(struct udevice *dev, struct serial_device_info *info);

void atmel_serial_initialize(void);
void mcf_serial_initialize(void);
void mpc85xx_serial_initialize(void);
void mxc_serial_initialize(void);
void ns16550_serial_initialize(void);
void pl01x_serial_initialize(void);
void pxa_serial_initialize(void);
void sh_serial_initialize(void);

#endif
