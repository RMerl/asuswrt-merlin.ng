/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2017
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

/*
 * This uclass encapsulates hardware methods to gather information about a
 * board or a specific device such as hard-wired GPIOs on GPIO expanders,
 * read-only data in flash ICs, or similar.
 *
 * The interface offers functions to read the usual standard data types (bool,
 * int, string) from the device, each of which is identified by a static
 * numeric ID (which will usually be defined as a enum in a header file).
 *
 * If for example the board had a read-only serial number flash IC, we could
 * call
 *
 * ret = board_detect(dev);
 * if (ret) {
 *	debug("board device not found.");
 *	return ret;
 * }
 *
 * ret = board_get_int(dev, ID_SERIAL_NUMBER, &serial);
 * if (ret) {
 *	debug("Error when reading serial number from device.");
 *	return ret;
 * }
 *
 * to read the serial number.
 */

struct board_ops {
	/**
	 * detect() - Run the hardware info detection procedure for this
	 *	      device.
	 * @dev:      The device containing the information
	 *
	 * This operation might take a long time (e.g. read from EEPROM,
	 * check the presence of a device on a bus etc.), hence this is not
	 * done in the probe() method, but later during operation in this
	 * dedicated method.
	 *
	 * Return: 0 if OK, -ve on error.
	 */
	int (*detect)(struct udevice *dev);

	/**
	 * get_bool() - Read a specific bool data value that describes the
	 *		hardware setup.
	 * @dev:	The board instance to gather the data.
	 * @id:		A unique identifier for the bool value to be read.
	 * @val:	Pointer to a buffer that receives the value read.
	 *
	 * Return: 0 if OK, -ve on error.
	 */
	int (*get_bool)(struct udevice *dev, int id, bool *val);

	/**
	 * get_int() - Read a specific int data value that describes the
	 *	       hardware setup.
	 * @dev:       The board instance to gather the data.
	 * @id:        A unique identifier for the int value to be read.
	 * @val:       Pointer to a buffer that receives the value read.
	 *
	 * Return: 0 if OK, -ve on error.
	 */
	int (*get_int)(struct udevice *dev, int id, int *val);

	/**
	 * get_str() - Read a specific string data value that describes the
	 *	       hardware setup.
	 * @dev:	The board instance to gather the data.
	 * @id:		A unique identifier for the string value to be read.
	 * @size:	The size of the buffer to receive the string data.
	 * @val:	Pointer to a buffer that receives the value read.
	 *
	 * Return: 0 if OK, -ve on error.
	 */
	int (*get_str)(struct udevice *dev, int id, size_t size, char *val);
};

#define board_get_ops(dev)	((struct board_ops *)(dev)->driver->ops)

/**
 * board_detect() - Run the hardware info detection procedure for this device.
 *
 * @dev:	The device containing the information
 *
 * Return: 0 if OK, -ve on error.
 */
int board_detect(struct udevice *dev);

/**
 * board_get_bool() - Read a specific bool data value that describes the
 *		      hardware setup.
 * @dev:	The board instance to gather the data.
 * @id:		A unique identifier for the bool value to be read.
 * @val:	Pointer to a buffer that receives the value read.
 *
 * Return: 0 if OK, -ve on error.
 */
int board_get_bool(struct udevice *dev, int id, bool *val);

/**
 * board_get_int() - Read a specific int data value that describes the
 *		     hardware setup.
 * @dev:	The board instance to gather the data.
 * @id:		A unique identifier for the int value to be read.
 * @val:	Pointer to a buffer that receives the value read.
 *
 * Return: 0 if OK, -ve on error.
 */
int board_get_int(struct udevice *dev, int id, int *val);

/**
 * board_get_str() - Read a specific string data value that describes the
 *		     hardware setup.
 * @dev:	The board instance to gather the data.
 * @id:		A unique identifier for the string value to be read.
 * @size:	The size of the buffer to receive the string data.
 * @val:	Pointer to a buffer that receives the value read.
 *
 * Return: 0 if OK, -ve on error.
 */
int board_get_str(struct udevice *dev, int id, size_t size, char *val);

/**
 * board_get() - Return the board device for the board in question.
 * @devp: Pointer to structure to receive the board device.
 *
 * Since there can only be at most one board instance, the API can supply a
 * function that returns the unique device. This is especially useful for use
 * in board files.
 *
 * Return: 0 if OK, -ve on error.
 */
int board_get(struct udevice **devp);
