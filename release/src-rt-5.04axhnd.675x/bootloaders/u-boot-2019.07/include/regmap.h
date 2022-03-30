/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __REGMAP_H
#define __REGMAP_H

/**
 * DOC: Overview
 *
 * Regmaps are an abstraction mechanism that allows device drivers to access
 * register maps irrespective of the underlying bus architecture. This entails
 * that for devices that support multiple busses (e.g. I2C and SPI for a GPIO
 * expander chip) only one driver has to be written. This driver will
 * instantiate a regmap with a backend depending on the bus the device is
 * attached to, and use the regmap API to access the register map through that
 * bus transparently.
 *
 * Read and write functions are supplied, which can read/write data of
 * arbitrary length from/to the regmap.
 *
 * The endianness of regmap accesses is selectable for each map through device
 * tree settings via the boolean "little-endian", "big-endian", and
 * "native-endian" properties.
 *
 * Furthermore, the register map described by a regmap can be split into
 * multiple disjoint areas called ranges. In this way, register maps with
 * "holes", i.e. areas of addressable memory that are not part of the register
 * map, can be accessed in a concise manner.
 *
 * Currently, only a bare "mem" backend for regmaps is supported, which
 * accesses the register map as regular IO-mapped memory.
 */

/**
 * enum regmap_size_t - Access sizes for regmap reads and writes
 *
 * @REGMAP_SIZE_8: 8-bit read/write access size
 * @REGMAP_SIZE_16: 16-bit read/write access size
 * @REGMAP_SIZE_32: 32-bit read/write access size
 * @REGMAP_SIZE_64: 64-bit read/write access size
 */
enum regmap_size_t {
	REGMAP_SIZE_8 = 1,
	REGMAP_SIZE_16 = 2,
	REGMAP_SIZE_32 = 4,
	REGMAP_SIZE_64 = 8,
};

/**
 * enum regmap_endianness_t - Endianness for regmap reads and writes
 *
 * @REGMAP_NATIVE_ENDIAN: Native endian read/write accesses
 * @REGMAP_LITTLE_ENDIAN: Little endian read/write accesses
 * @REGMAP_BIG_ENDIAN: Big endian read/write accesses
 */
enum regmap_endianness_t {
	REGMAP_NATIVE_ENDIAN,
	REGMAP_LITTLE_ENDIAN,
	REGMAP_BIG_ENDIAN,
};

/**
 * struct regmap_range - a register map range
 *
 * @start:	Start address
 * @size:	Size in bytes
 */
struct regmap_range {
	ulong start;
	ulong size;
};

/**
 * struct regmap - a way of accessing hardware/bus registers
 *
 * @range_count:	Number of ranges available within the map
 * @ranges:		Array of ranges
 */
struct regmap {
	enum regmap_endianness_t endianness;
	int range_count;
	struct regmap_range ranges[0];
};

/*
 * Interface to provide access to registers either through a direct memory
 * bus or through a peripheral bus like I2C, SPI.
 */

/**
 * regmap_write() - Write a 32-bit value to a regmap
 *
 * @map:	Regmap to write to
 * @offset:	Offset in the regmap to write to
 * @val:	Data to write to the regmap at the specified offset
 *
 * Note that this function will only write values of 32 bit width to the
 * regmap; if the size of data to be read is different, the regmap_raw_write
 * function can be used.
 *
 * Return: 0 if OK, -ve on error
 */
int regmap_write(struct regmap *map, uint offset, uint val);

/**
 * regmap_read() - Read a 32-bit value from a regmap
 *
 * @map:	Regmap to read from
 * @offset:	Offset in the regmap to read from
 * @valp:	Pointer to the buffer to receive the data read from the regmap
 *		at the specified offset
 *
 * Note that this function will only read values of 32 bit width from the
 * regmap; if the size of data to be read is different, the regmap_raw_read
 * function can be used.
 *
 * Return: 0 if OK, -ve on error
 */
int regmap_read(struct regmap *map, uint offset, uint *valp);

/**
 * regmap_raw_write() - Write a value of specified length to a regmap
 *
 * @map:	Regmap to write to
 * @offset:	Offset in the regmap to write to
 * @val:	Value to write to the regmap at the specified offset
 * @val_len:	Length of the data to be written to the regmap
 *
 * Note that this function will, as opposed to regmap_write, write data of
 * arbitrary length to the regmap, and not just 32-bit values, and is thus a
 * generalized version of regmap_write.
 *
 * Return: 0 if OK, -ve on error
 */
int regmap_raw_write(struct regmap *map, uint offset, const void *val,
		     size_t val_len);

/**
 * regmap_raw_read() - Read a value of specified length from a regmap
 *
 * @map:	Regmap to read from
 * @offset:	Offset in the regmap to read from
 * @valp:	Pointer to the buffer to receive the data read from the regmap
 *		at the specified offset
 * @val_len:	Length of the data to be read from the regmap
 *
 * Note that this function will, as opposed to regmap_read, read data of
 * arbitrary length from the regmap, and not just 32-bit values, and is thus a
 * generalized version of regmap_read.
 *
 * Return: 0 if OK, -ve on error
 */
int regmap_raw_read(struct regmap *map, uint offset, void *valp,
		    size_t val_len);

/**
 * regmap_raw_write_range() - Write a value of specified length to a range of a
 *			      regmap
 *
 * @map:	Regmap to write to
 * @range_num:	Number of the range in the regmap to write to
 * @offset:	Offset in the regmap to write to
 * @val:	Value to write to the regmap at the specified offset
 * @val_len:	Length of the data to be written to the regmap
 *
 * Return: 0 if OK, -ve on error
 */
int regmap_raw_write_range(struct regmap *map, uint range_num, uint offset,
			   const void *val, size_t val_len);

/**
 * regmap_raw_read_range() - Read a value of specified length from a range of a
 *			     regmap
 *
 * @map:	Regmap to read from
 * @range_num:	Number of the range in the regmap to write to
 * @offset:	Offset in the regmap to read from
 * @valp:	Pointer to the buffer to receive the data read from the regmap
 *		at the specified offset
 * @val_len:	Length of the data to be read from the regmap
 *
 * Return: 0 if OK, -ve on error
 */
int regmap_raw_read_range(struct regmap *map, uint range_num, uint offset,
			  void *valp, size_t val_len);

/**
 * regmap_range_set() - Set a value in a regmap range described by a struct
 * @map:    Regmap in which a value should be set
 * @range:  Range of the regmap in which a value should be set
 * @type:   Structure type that describes the memory layout of the regmap range
 * @member: Member of the describing structure that should be set in the regmap
 *          range
 * @val:    Value which should be written to the regmap range
 */
#define regmap_range_set(map, range, type, member, val) \
	do { \
		typeof(((type *)0)->member) __tmp = val; \
		regmap_raw_write_range(map, range, offsetof(type, member), \
				       &__tmp, sizeof(((type *)0)->member)); \
	} while (0)

/**
 * regmap_set() - Set a value in a regmap described by a struct
 * @map:    Regmap in which a value should be set
 * @type:   Structure type that describes the memory layout of the regmap
 * @member: Member of the describing structure that should be set in the regmap
 * @val:    Value which should be written to the regmap
 */
#define regmap_set(map, type, member, val) \
	regmap_range_set(map, 0, type, member, val)

/**
 * regmap_range_get() - Get a value from a regmap range described by a struct
 * @map:    Regmap from which a value should be read
 * @range:  Range of the regmap from which a value should be read
 * @type:   Structure type that describes the memory layout of the regmap
 *          range
 * @member: Member of the describing structure that should be read in the
 *          regmap range
 * @valp:   Variable that receives the value read from the regmap range
 */
#define regmap_range_get(map, range, type, member, valp) \
	regmap_raw_read_range(map, range, offsetof(type, member), \
			      (void *)valp, sizeof(((type *)0)->member))

/**
 * regmap_get() - Get a value from a regmap described by a struct
 * @map:    Regmap from which a value should be read
 * @type:   Structure type that describes the memory layout of the regmap
 *          range
 * @member: Member of the describing structure that should be read in the
 *          regmap
 * @valp:   Variable that receives the value read from the regmap
 */
#define regmap_get(map, type, member, valp) \
	regmap_range_get(map, 0, type, member, valp)

/**
 * regmap_read_poll_timeout - Poll until a condition is met or a timeout occurs
 *
 * @map:	Regmap to read from
 * @addr:	Offset to poll
 * @val:	Unsigned integer variable to read the value into
 * @cond:	Break condition (usually involving @val)
 * @sleep_us:	Maximum time to sleep between reads in us (0 tight-loops).
 * @timeout_ms:	Timeout in ms, 0 means never timeout
 * @test_add_time: Used for sandbox testing - amount of time to add after
 *		starting the loop (0 if not testing)
 *
 * Returns 0 on success and -ETIMEDOUT upon a timeout or the regmap_read
 * error return value in case of a error read. In the two former cases,
 * the last read value at @addr is stored in @val. Must not be called
 * from atomic context if sleep_us or timeout_us are used.
 *
 * This is modelled after the regmap_read_poll_timeout macros in linux but
 * with millisecond timeout.
 *
 * The _test version is for sandbox testing only. Do not use this in normal
 * code as it advances the timer.
 */
#define regmap_read_poll_timeout_test(map, addr, val, cond, sleep_us, \
				      timeout_ms, test_add_time) \
({ \
	unsigned long __start = get_timer(0); \
	int __ret; \
	for (;;) { \
		__ret = regmap_read((map), (addr), &(val)); \
		if (__ret) \
			break; \
		if (cond) \
			break; \
		if (IS_ENABLED(CONFIG_SANDBOX) && test_add_time) \
			timer_test_add_offset(test_add_time); \
		if ((timeout_ms) && get_timer(__start) > (timeout_ms)) { \
			__ret = regmap_read((map), (addr), &(val)); \
			break; \
		} \
		if ((sleep_us)) \
			udelay((sleep_us)); \
	} \
	__ret ?: ((cond) ? 0 : -ETIMEDOUT); \
})

#define regmap_read_poll_timeout(map, addr, val, cond, sleep_us, timeout_ms) \
	regmap_read_poll_timeout_test(map, addr, val, cond, sleep_us, \
				      timeout_ms, 0) \

/**
 * regmap_update_bits() - Perform a read/modify/write using a mask
 *
 * @map:	The map returned by regmap_init_mem*()
 * @offset:	Offset of the memory
 * @mask:	Mask to apply to the read value
 * @val:	Value to apply to the value to write
 * Return: 0 if OK, -ve on error
 */
int regmap_update_bits(struct regmap *map, uint offset, uint mask, uint val);

/**
 * regmap_init_mem() - Set up a new register map that uses memory access
 *
 * @node:	Device node that uses this map
 * @mapp:	Returns allocated map
 * Return: 0 if OK, -ve on error
 *
 * Use regmap_uninit() to free it.
 */
int regmap_init_mem(ofnode node, struct regmap **mapp);

/**
 * regmap_init_mem_platdata() - Set up a new memory register map for
 *				of-platdata
 *
 * @dev:	Device that uses this map
 * @reg:	List of address, size pairs
 * @count:	Number of pairs (e.g. 1 if the regmap has a single entry)
 * @mapp:	Returns allocated map
 * Return: 0 if OK, -ve on error
 *
 * This creates a new regmap with a list of regions passed in, rather than
 * using the device tree. It only supports 32-bit machines.
 *
 * Use regmap_uninit() to free it.
 *
 */
int regmap_init_mem_platdata(struct udevice *dev, fdt_val_t *reg, int count,
			     struct regmap **mapp);

/**
 * regmap_get_range() - Obtain the base memory address of a regmap range
 *
 * @map:	Regmap to query
 * @range_num:	Range to look up
 * Return: Pointer to the range in question if OK, NULL on error
 */
void *regmap_get_range(struct regmap *map, unsigned int range_num);

/**
 * regmap_uninit() - free a previously inited regmap
 *
 * @map:	Regmap to free
 * Return: 0 if OK, -ve on error
 */
int regmap_uninit(struct regmap *map);

#endif
