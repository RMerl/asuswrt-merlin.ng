/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __REGMAP_H
#define __REGMAP_H

#include <linux/delay.h>

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

struct regmap_bus;

/**
 * struct regmap_config - Configure the behaviour of a regmap
 *
 * @width:		Width of the read/write operations. Defaults to
 *			REGMAP_SIZE_32 if set to 0.
 * @reg_offset_shift	Left shift the register offset by this value before
 *			performing read or write.
 * @r_start:		If specified, the regmap is created with one range
 *			which starts at this address, instead of finding the
 *			start from device tree.
 * @r_size:		Same as above for the range size
 */
struct regmap_config {
	enum regmap_size_t width;
	u32 reg_offset_shift;
	ulong r_start;
	ulong r_size;
};

/**
 * struct regmap - a way of accessing hardware/bus registers
 *
 * @width:		Width of the read/write operations. Defaults to
 *			REGMAP_SIZE_32 if set to 0.
 * @reg_offset_shift	Left shift the register offset by this value before
 *			performing read or write.
 * @range_count:	Number of ranges available within the map
 * @ranges:		Array of ranges
 */
struct regmap {
	enum regmap_endianness_t endianness;
	enum regmap_size_t width;
	u32 reg_offset_shift;
	int range_count;
	struct regmap_range ranges[0];
};

/*
 * Interface to provide access to registers either through a direct memory
 * bus or through a peripheral bus like I2C, SPI.
 */

/**
 * regmap_write() - Write a value to a regmap
 *
 * @map:	Regmap to write to
 * @offset:	Offset in the regmap to write to
 * @val:	Data to write to the regmap at the specified offset
 *
 * Return: 0 if OK, -ve on error
 */
int regmap_write(struct regmap *map, uint offset, uint val);

/**
 * regmap_read() - Read a value from a regmap
 *
 * @map:	Regmap to read from
 * @offset:	Offset in the regmap to read from
 * @valp:	Pointer to the buffer to receive the data read from the regmap
 *		at the specified offset
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
 * arbitrary length to the regmap, and not just the size configured in the
 * regmap (defaults to 32-bit) and is thus a generalized version of
 * regmap_write.
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
 * arbitrary length from the regmap, and not just the size configured in the
 * regmap (defaults to 32-bit) and is thus a generalized version of
 * regmap_read.
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
 * regmap_field_read_poll_timeout - Poll until a condition is met or a timeout
 *				    occurs
 *
 * @field:	Regmap field to read from
 * @val:	Unsigned integer variable to read the value into
 * @cond:	Break condition (usually involving @val)
 * @sleep_us:	Maximum time to sleep between reads in us (0 tight-loops).
 * @timeout_ms:	Timeout in ms, 0 means never timeout
 *
 * Returns 0 on success and -ETIMEDOUT upon a timeout or the regmap_field_read
 * error return value in case of a error read. In the two former cases,
 * the last read value at @addr is stored in @val.
 *
 * This is modelled after the regmap_read_poll_timeout macros in linux but
 * with millisecond timeout.
 */
#define regmap_field_read_poll_timeout(field, val, cond, sleep_us, timeout_ms) \
({ \
	unsigned long __start = get_timer(0); \
	int __ret; \
	for (;;) { \
		__ret = regmap_field_read((field), &(val)); \
		if (__ret) \
			break; \
		if (cond) \
			break; \
		if ((timeout_ms) && get_timer(__start) > (timeout_ms)) { \
			__ret = regmap_field_read((field), &(val)); \
			break; \
		} \
		if ((sleep_us)) \
			udelay((sleep_us)); \
	} \
	__ret ?: ((cond) ? 0 : -ETIMEDOUT); \
})

/**
 * regmap_update_bits() - Perform a read/modify/write using a mask
 *
 * @map:	The map returned by regmap_init_mem*()
 * @offset:	Offset of the memory
 * @mask:	Mask to apply to the read value
 * @val:	Value to OR with the read value after masking. Note that any
 *	bits set in @val which are not set in @mask are ignored
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
 * regmap_init_mem_plat() - Set up a new memory register map for
 *				of-platdata
 *
 * @dev:	Device that uses this map
 * @reg:	List of address, size pairs
 * @size:	Size of one reg array item
 * @count:	Number of pairs (e.g. 1 if the regmap has a single entry)
 * @mapp:	Returns allocated map
 * Return: 0 if OK, -ve on error
 *
 * This creates a new regmap with a list of regions passed in, rather than
 * using the device tree.
 *
 * Use regmap_uninit() to free it.
 *
 */
int regmap_init_mem_plat(struct udevice *dev, void *reg, int size, int count,
			 struct regmap **mapp);

int regmap_init_mem_index(ofnode node, struct regmap **mapp, int index);

/**
 * regmap_init_mem_range() - Set up a new memory region for ofnode with the
 *			     specified range.
 *
 * @node:	The ofnode for the map.
 * @r_start:	Start of the range.
 * @r_size:	Size of the range.
 * @mapp:	Returns allocated map.
 *
 * Return: 0 in success, -errno otherwise
 *
 * This creates a regmap with one range where instead of extracting the range
 * from 'node', it is created based on the parameters specified. This is
 * useful when a driver needs to calculate the base of the regmap at runtime,
 * and can't specify it in device tree.
 */
int regmap_init_mem_range(ofnode node, ulong r_start, ulong r_size,
			  struct regmap **mapp);

/**
 * devm_regmap_init() - Initialise register map (device managed)
 *
 * @dev: Device that will be interacted with
 * @bus: Bus-specific callbacks to use with device (IGNORED)
 * @bus_context: Data passed to bus-specific callbacks (IGNORED)
 * @config: Configuration for register map
 *
 * @Return a valid pointer to a struct regmap or a ERR_PTR() on error.
 * The structure is automatically freed when the device is unbound
 */
struct regmap *devm_regmap_init(struct udevice *dev,
				const struct regmap_bus *bus,
				void *bus_context,
				const struct regmap_config *config);
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

/**
 * struct reg_field - Description of an register field
 *
 * @reg: Offset of the register within the regmap bank
 * @lsb: lsb of the register field.
 * @msb: msb of the register field.
 */
struct reg_field {
	unsigned int reg;
	unsigned int lsb;
	unsigned int msb;
};

struct regmap_field;

/**
 * REG_FIELD() - A convenient way to initialize a 'struct reg_field'.
 *
 * @_reg: Offset of the register within the regmap bank
 * @_lsb: lsb of the register field.
 * @_msb: msb of the register field.
 *
 * Register fields are often described in terms of 3 things: the register it
 * belongs to, its LSB, and its MSB. This macro can be used by drivers to
 * clearly and easily initialize a 'struct regmap_field'.
 *
 * For example, say a device has a register at offset DEV_REG1 (0x100) and a
 * field of DEV_REG1 is on bits [7:3]. So a driver can initialize a regmap
 * field for this by doing:
 *     struct reg_field field = REG_FIELD(DEV_REG1, 3, 7);
 */
#define REG_FIELD(_reg, _lsb, _msb) {		\
				.reg = _reg,	\
				.lsb = _lsb,	\
				.msb = _msb,	\
				}

/**
 * devm_regmap_field_alloc() - Allocate and initialise a register field.
 *
 * @dev: Device that will be interacted with
 * @regmap: regmap bank in which this register field is located.
 * @reg_field: Register field with in the bank.
 *
 * The return value will be an ERR_PTR() on error or a valid pointer
 * to a struct regmap_field. The regmap_field will be automatically freed
 * by the device management code.
 */
struct regmap_field *devm_regmap_field_alloc(struct udevice *dev,
					     struct regmap *regmap,
					     struct reg_field reg_field);
/**
 * devm_regmap_field_free() - Free a register field allocated using
 *                            devm_regmap_field_alloc.
 *
 * @dev: Device that will be interacted with
 * @field: regmap field which should be freed.
 *
 * Free register field allocated using devm_regmap_field_alloc(). Usually
 * drivers need not call this function, as the memory allocated via devm
 * will be freed as per device-driver life-cyle.
 */
void devm_regmap_field_free(struct udevice *dev, struct regmap_field *field);

/**
 * regmap_field_write() - Write a value to a regmap field
 *
 * @field:	Regmap field to write to
 * @val:	Data to write to the regmap at the specified offset
 *
 * Return: 0 if OK, -ve on error
 */
int regmap_field_write(struct regmap_field *field, unsigned int val);

/**
 * regmap_field_read() - Read a 32-bit value from a regmap
 *
 * @field:	Regmap field to read from
 * @valp:	Pointer to the buffer to receive the data read from the regmap
 *		field
 *
 * Return: 0 if OK, -ve on error
 */
int regmap_field_read(struct regmap_field *field, unsigned int *val);

#endif
