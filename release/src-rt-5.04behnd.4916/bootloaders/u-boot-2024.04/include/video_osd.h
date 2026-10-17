/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2017
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#ifndef _VIDEO_OSD_H_
#define _VIDEO_OSD_H_

struct video_osd_info {
	/* The width of the OSD display in columns */
	uint width;
	/* The height of the OSD display in rows */
	uint height;
	/* The major version of the OSD device */
	uint major_version;
	/* The minor version of the OSD device */
	uint minor_version;
};

/**
 * struct video_osd_ops - driver operations for OSD uclass
 *
 * The OSD uclass implements support for text-oriented on-screen displays,
 * which are taken to be devices that independently display a graphical
 * text-based overlay over the video output of an associated display.
 *
 * The functions defined by the uclass support writing text to the display in
 * either a generic form (by specifying a string, a driver-specific color value
 * for the text, and screen coordinates in rows and columns) or a
 * driver-specific form (by specifying "raw" driver-specific data to display at
 * a given coordinate).
 *
 * Functions to read device information and set the size of the virtual OSD
 * screen (in rows and columns) are also supported.
 *
 * Drivers should support these operations unless otherwise noted. These
 * operations are intended to be used by uclass code, not directly from
 * other code.
 */
struct video_osd_ops {
	/**
	 * get_info() - Get information about a OSD instance
	 *
	 * A OSD instance may keep some internal data about itself. This
	 * function can be used to access this data.
	 *
	 * @dev:	OSD instance to query.
	 * @info:	Pointer to a structure that takes the information read
	 *		from the OSD instance.
	 * @return 0 if OK, -ve on error.
	 */
	int (*get_info)(struct udevice *dev, struct video_osd_info *info);

	/**
	 * set_mem() - Write driver-specific text data to OSD screen
	 *
	 * The passed data are device-specific, and it's up to the driver how
	 * to interpret them. How the count parameter is interpreted is also
	 * driver-specific; most likely the given data will be written to the
	 * OSD count times back-to-back, which is e.g. convenient for filling
	 * areas of the OSD with a single character.
	 *
	 * For example a invocation of
	 *
	 * video_osd_set_mem(dev, 0, 0, "A", 1, 10);
	 *
	 * will write the device-specific text data "A" to the positions (0, 0)
	 * to (9, 0) on the OSD.
	 *
	 * Device-specific text data may, e.g. be a special encoding of glyphs
	 * to display and color values in binary format.
	 *
	 * @dev:	OSD instance to write to.
	 * @col:	Horizontal character coordinate to write to.
	 * @row		Vertical character coordinate to write to.
	 * @buf:	Array containing device-specific data to write to the
	 *		specified coordinate on the OSD screen.
	 * @buflen:	Length of the data in the passed buffer (in byte).
	 * @count:	Write count many repetitions of the given text data
	 * @return 0 if OK, -ve on error.
	 */
	int (*set_mem)(struct udevice *dev, uint col, uint row, u8 *buf,
		       size_t buflen, uint count);

	/**
	 * set_size() - Set the position and dimension of the OSD's
	 *              writeable window
	 *
	 * @dev:	OSD instance to write to.
	 * @col		The number of characters in the window's columns
	 * @row		The number of characters in the window's rows
	 * @return 0 if OK, -ve on error.
	 */
	int (*set_size)(struct udevice *dev, uint col, uint row);

	/**
	 * print() - Print a string in a given color to specified coordinates
	 *	     on the OSD
	 *
	 * @dev:	OSD instance to write to.
	 * @col		The x-coordinate of the position the string should be
	 *		written to
	 * @row		The y-coordinate of the position the string should be
	 *		written to
	 * @color:	The color in which the specified string should be
	 *		printed; the interpretation of the value is
	 *		driver-specific, and possible values should be defined
	 *		e.g. in a driver include file.
	 * @text:	The string data that should be printed on the OSD
	 * @return 0 if OK, -ve on error.
	 */
	int (*print)(struct udevice *dev, uint col, uint row, ulong color,
		     char *text);
};

#define video_osd_get_ops(dev)	((struct video_osd_ops *)(dev)->driver->ops)

/**
 * video_osd_get_info() - Get information about a OSD instance
 *
 * A OSD instance may keep some internal data about itself. This function can
 * be used to access this data.
 *
 * @dev:	OSD instance to query.
 * @info:	Pointer to a structure that takes the information read from the
 *		OSD instance.
 * Return: 0 if OK, -ve on error.
 */
int video_osd_get_info(struct udevice *dev, struct video_osd_info *info);

/**
 * video_osd_set_mem() - Write text data to OSD memory
 *
 * The passed data are device-specific, and it's up to the driver how to
 * interpret them. How the count parameter is interpreted is also
 * driver-specific; most likely the given data will be written to the OSD count
 * times back-to-back, which is e.g. convenient for filling areas of the OSD
 * with a single character.
 *
 * For example a invocation of
 *
 * video_osd_set_mem(dev, 0, 0, "A", 1, 10);
 *
 * will write the device-specific text data "A" to the positions (0, 0) to (9,
 * 0) on the OSD.
 *
 * Device-specific text data may, e.g. be a special encoding of glyphs to
 * display and color values in binary format.
 *
 * @dev:	OSD instance to write to.
 * @col:	Horizontal character coordinate to write to.
 * @row		Vertical character coordinate to write to.
 * @buf:	Array containing device-specific data to write to the specified
 *		coordinate on the OSD screen.
 * @buflen:	Length of the data in the passed buffer (in byte).
 * @count:	Write count many repetitions of the given text data
 * Return: 0 if OK, -ve on error.
 */
int video_osd_set_mem(struct udevice *dev, uint col, uint row, u8 *buf,
		      size_t buflen, uint count);

/**
 * video_osd_set_size() - Set the position and dimension of the OSD's
 *              writeable window
 *
 * @dev:	OSD instance to write to.
 * @col		The number of characters in the window's columns
 * @row		The number of characters in the window's rows
 * Return: 0 if OK, -ve on error.
 */
int video_osd_set_size(struct udevice *dev, uint col, uint row);

/**
 * video_osd_print() - Print a string in a given color to specified coordinates
 *		       on the OSD
 *
 * @dev:	OSD instance to write to.
 * @col		The x-coordinate of the position the string should be written
 *		to
 * @row		The y-coordinate of the position the string should be written
 *		to
 * @color:	The color in which the specified string should be printed; the
 *		interpretation of the value is driver-specific, and possible
 *		values should be defined e.g. in a driver include file.
 * @text:	The string data that should be printed on the OSD
 * Return: 0 if OK, -ve on error.
 */
int video_osd_print(struct udevice *dev, uint col, uint row, ulong color,
		    char *text);

#endif /* !_VIDEO_OSD_H_ */
