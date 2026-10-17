/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2017
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#ifndef __SYSINFO_H__
#define __SYSINFO_H__

#include <linux/errno.h>

struct udevice;

/*
 * This uclass encapsulates hardware methods to gather information about a
 * sysinfo or a specific device such as hard-wired GPIOs on GPIO expanders,
 * read-only data in flash ICs, or similar.
 *
 * The interface offers functions to read the usual standard data types (bool,
 * int, string) from the device, each of which is identified by a static
 * numeric ID (which will usually be defined as a enum in a header file).
 *
 * If for example the sysinfo had a read-only serial number flash IC, we could
 * call
 *
 * ret = sysinfo_detect(dev);
 * if (ret) {
 *	debug("sysinfo device not found.");
 *	return ret;
 * }
 *
 * ret = sysinfo_get_int(dev, ID_SERIAL_NUMBER, &serial);
 * if (ret) {
 *	debug("Error when reading serial number from device.");
 *	return ret;
 * }
 *
 * to read the serial number.
 */

/** enum sysinfo_id - Standard IDs defined by U-Boot */
enum sysinfo_id {
	SYSINFO_ID_NONE,

	/* For SMBIOS tables */
	SYSINFO_ID_SMBIOS_SYSTEM_VERSION,
	SYSINFO_ID_SMBIOS_BASEBOARD_VERSION,

	/* For show_board_info() */
	SYSINFO_ID_BOARD_MODEL,
	SYSINFO_ID_BOARD_MANUFACTURER,
	SYSINFO_ID_PRIOR_STAGE_VERSION,
	SYSINFO_ID_PRIOR_STAGE_DATE,

	/* First value available for downstream/board used */
	SYSINFO_ID_USER = 0x1000,
};

struct sysinfo_ops {
	/**
	 * detect() - Run the hardware info detection procedure for this
	 *	      device.
	 * @dev:      The device containing the information
	 *
	 * This operation might take a long time (e.g. read from EEPROM,
	 * check the presence of a device on a bus etc.), hence this is not
	 * done in the probe() method, but later during operation in this
	 * dedicated method. This method will be called before any other
	 * methods.
	 *
	 * Return: 0 if OK, -ve on error.
	 */
	int (*detect)(struct udevice *dev);

	/**
	 * get_bool() - Read a specific bool data value that describes the
	 *		hardware setup.
	 * @dev:	The sysinfo instance to gather the data.
	 * @id:		A unique identifier for the bool value to be read.
	 * @val:	Pointer to a buffer that receives the value read.
	 *
	 * Return: 0 if OK, -ve on error.
	 */
	int (*get_bool)(struct udevice *dev, int id, bool *val);

	/**
	 * get_int() - Read a specific int data value that describes the
	 *	       hardware setup.
	 * @dev:       The sysinfo instance to gather the data.
	 * @id:        A unique identifier for the int value to be read.
	 * @val:       Pointer to a buffer that receives the value read.
	 *
	 * Return: 0 if OK, -ve on error.
	 */
	int (*get_int)(struct udevice *dev, int id, int *val);

	/**
	 * get_str() - Read a specific string data value that describes the
	 *	       hardware setup.
	 * @dev:	The sysinfo instance to gather the data.
	 * @id:		A unique identifier for the string value to be read.
	 * @size:	The size of the buffer to receive the string data.
	 * @val:	Pointer to a buffer that receives the value read.
	 *
	 * Return: 0 if OK, -ve on error.
	 */
	int (*get_str)(struct udevice *dev, int id, size_t size, char *val);

	/**
	 * get_fit_loadable - Get the name of an image to load from FIT
	 * This function can be used to provide the image names based on runtime
	 * detection. A classic use-case would when DTBOs are used to describe
	 * additional daughter cards.
	 *
	 * @dev:	The sysinfo instance to gather the data.
	 * @index:	Index of the image. Starts at 0 and gets incremented
	 *		after each call to this function.
	 * @type:	The type of image. For example, "fdt" for DTBs
	 * @strp:	A pointer to string. Untouched if the function fails
	 *
	 * Return: 0 if OK, -ENOENT if no loadable is available else -ve on
	 * error.
	 */
	int (*get_fit_loadable)(struct udevice *dev, int index,
				const char *type, const char **strp);
};

#define sysinfo_get_ops(dev)	((struct sysinfo_ops *)(dev)->driver->ops)

#if CONFIG_IS_ENABLED(SYSINFO)
/**
 * sysinfo_detect() - Run the hardware info detection procedure for this device.
 *
 * @dev:	The device containing the information
 *
 * This function must be called before any other accessor function for this
 * device.
 *
 * Return: 0 if OK, -ve on error.
 */
int sysinfo_detect(struct udevice *dev);

/**
 * sysinfo_get_bool() - Read a specific bool data value that describes the
 *		      hardware setup.
 * @dev:	The sysinfo instance to gather the data.
 * @id:		A unique identifier for the bool value to be read.
 * @val:	Pointer to a buffer that receives the value read.
 *
 * Return: 0 if OK, -EPERM if called before sysinfo_detect(), else -ve on
 * error.
 */
int sysinfo_get_bool(struct udevice *dev, int id, bool *val);

/**
 * sysinfo_get_int() - Read a specific int data value that describes the
 *		     hardware setup.
 * @dev:	The sysinfo instance to gather the data.
 * @id:		A unique identifier for the int value to be read.
 * @val:	Pointer to a buffer that receives the value read.
 *
 * Return: 0 if OK, -EPERM if called before sysinfo_detect(), else -ve on
 * error.
 */
int sysinfo_get_int(struct udevice *dev, int id, int *val);

/**
 * sysinfo_get_str() - Read a specific string data value that describes the
 *		     hardware setup.
 * @dev:	The sysinfo instance to gather the data.
 * @id:		A unique identifier for the string value to be read.
 * @size:	The size of the buffer to receive the string data.
 * @val:	Pointer to a buffer that receives the value read.
 *
 * Return: 0 if OK, -EPERM if called before sysinfo_detect(), else -ve on
 * error.
 */
int sysinfo_get_str(struct udevice *dev, int id, size_t size, char *val);

/**
 * sysinfo_get() - Return the sysinfo device for the sysinfo in question.
 * @devp: Pointer to structure to receive the sysinfo device.
 *
 * Since there can only be at most one sysinfo instance, the API can supply a
 * function that returns the unique device. This is especially useful for use
 * in sysinfo files.
 *
 * Return: 0 if OK, -EPERM if called before sysinfo_detect(), else -ve on
 * error.
 */
int sysinfo_get(struct udevice **devp);

/**
 * sysinfo_get_fit_loadable - Get the name of an image to load from FIT
 * This function can be used to provide the image names based on runtime
 * detection. A classic use-case would when DTBOs are used to describe
 * additional daughter cards.
 *
 * @dev:	The sysinfo instance to gather the data.
 * @index:	Index of the image. Starts at 0 and gets incremented
 *		after each call to this function.
 * @type:	The type of image. For example, "fdt" for DTBs
 * @strp:	A pointer to string. Untouched if the function fails
 *
 *
 * Return: 0 if OK, -EPERM if called before sysinfo_detect(), -ENOENT if no
 * loadable is available else -ve on error.
 */
int sysinfo_get_fit_loadable(struct udevice *dev, int index, const char *type,
			     const char **strp);

#else

static inline int sysinfo_detect(struct udevice *dev)
{
	return -ENOSYS;
}

static inline int sysinfo_get_bool(struct udevice *dev, int id, bool *val)
{
	return -ENOSYS;
}

static inline int sysinfo_get_int(struct udevice *dev, int id, int *val)
{
	return -ENOSYS;
}

static inline int sysinfo_get_str(struct udevice *dev, int id, size_t size,
				  char *val)
{
	return -ENOSYS;
}

static inline int sysinfo_get(struct udevice **devp)
{
	return -ENOSYS;
}

static inline int sysinfo_get_fit_loadable(struct udevice *dev, int index,
					   const char *type, const char **strp)
{
	return -ENOSYS;
}

#endif
#endif
