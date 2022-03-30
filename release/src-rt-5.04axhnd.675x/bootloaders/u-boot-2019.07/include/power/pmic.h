/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Copyright (C) 2014-2015 Samsung Electronics
 *  Przemyslaw Marczak <p.marczak@samsung.com>
 *
 *  Copyright (C) 2011-2012 Samsung Electronics
 *  Lukasz Majewski <l.majewski@samsung.com>
 */

#ifndef __CORE_PMIC_H_
#define __CORE_PMIC_H_

#include <dm/ofnode.h>
#include <i2c.h>
#include <linux/list.h>
#include <power/power_chrg.h>

enum { PMIC_I2C, PMIC_SPI, PMIC_NONE};

#ifdef CONFIG_POWER
enum { I2C_PMIC, I2C_NUM, };
enum { PMIC_READ, PMIC_WRITE, };
enum { PMIC_SENSOR_BYTE_ORDER_LITTLE, PMIC_SENSOR_BYTE_ORDER_BIG, };

enum {
	PMIC_CHARGER_DISABLE,
	PMIC_CHARGER_ENABLE,
};

struct p_i2c {
	unsigned char addr;
	unsigned char *buf;
	unsigned char tx_num;
};

struct p_spi {
	unsigned int cs;
	unsigned int mode;
	unsigned int bitlen;
	unsigned int clk;
	unsigned int flags;
	u32 (*prepare_tx)(u32 reg, u32 *val, u32 write);
};

struct pmic;
struct power_fg {
	int (*fg_battery_check) (struct pmic *p, struct pmic *bat);
	int (*fg_battery_update) (struct pmic *p, struct pmic *bat);
};

struct power_chrg {
	int (*chrg_type) (struct pmic *p);
	int (*chrg_bat_present) (struct pmic *p);
	int (*chrg_state) (struct pmic *p, int state, int current);
};

struct power_battery {
	struct battery *bat;
	int (*battery_init) (struct pmic *bat, struct pmic *p1,
			     struct pmic *p2, struct pmic *p3);
	int (*battery_charge) (struct pmic *bat);
	/* Keep info about power devices involved with battery operation */
	struct pmic *chrg, *fg, *muic;
};

struct pmic {
	const char *name;
	unsigned char bus;
	unsigned char interface;
	unsigned char sensor_byte_order;
	unsigned int number_of_regs;
	union hw {
		struct p_i2c i2c;
		struct p_spi spi;
	} hw;

	void (*low_power_mode) (void);
	struct power_battery *pbat;
	struct power_chrg *chrg;
	struct power_fg *fg;

	struct pmic *parent;
	struct list_head list;
};
#endif /* CONFIG_POWER */

#ifdef CONFIG_DM_PMIC
/**
 * U-Boot PMIC Framework
 * =====================
 *
 * UCLASS_PMIC - This is designed to provide an I/O interface for PMIC devices.
 *
 * For the multi-function PMIC devices, this can be used as parent I/O device
 * for each IC's interface. Then, each child uses its parent for read/write.
 *
 * The driver model tree could look like this:
 *
 *_ root device
 * |_ BUS 0 device (e.g. I2C0)                 - UCLASS_I2C/SPI/...
 * | |_ PMIC device (READ/WRITE ops)           - UCLASS_PMIC
 * |   |_ REGULATOR device (ldo/buck/... ops)  - UCLASS_REGULATOR
 * |   |_ CHARGER device (charger ops)         - UCLASS_CHARGER (in the future)
 * |   |_ MUIC device (microUSB connector ops) - UCLASS_MUIC    (in the future)
 * |   |_ ...
 * |
 * |_ BUS 1 device (e.g. I2C1)                 - UCLASS_I2C/SPI/...
 *   |_ PMIC device (READ/WRITE ops)           - UCLASS_PMIC
 *     |_ RTC device (rtc ops)                 - UCLASS_RTC     (in the future)
 *
 * We can find two PMIC cases in boards design:
 * - single I/O interface
 * - multiple I/O interfaces
 * We bind a single PMIC device for each interface, to provide an I/O for
 * its child devices. And each child usually implements a different function,
 * controlled by the same interface.
 *
 * The binding should be done automatically. If device tree nodes/subnodes are
 * proper defined, then:
 *
 * |_ the ROOT driver will bind the device for I2C/SPI node:
 *   |_ the I2C/SPI driver should bind a device for pmic node:
 *     |_ the PMIC driver should bind devices for its childs:
 *       |_ regulator (child)
 *       |_ charger   (child)
 *       |_ other     (child)
 *
 * The same for other device nodes, for multi-interface PMIC.
 *
 * Note:
 * Each PMIC interface driver should use a different compatible string.
 *
 * If a PMIC child device driver needs access the PMIC-specific registers,
 * it need know only the register address and the access can be done through
 * the parent pmic driver. Like in the example:
 *
 *_ root driver
 * |_ dev: bus I2C0                                         - UCLASS_I2C
 * | |_ dev: my_pmic (read/write)              (is parent)  - UCLASS_PMIC
 * |   |_ dev: my_regulator (set value/etc..)  (is child)   - UCLASS_REGULATOR
 *
 * To ensure such device relationship, the pmic device driver should also bind
 * all its child devices, like in the example below. It can be done by calling
 * the 'pmic_bind_children()' - please refer to the function description, which
 * can be found in this header file. This function, should be called inside the
 * driver's bind() method.
 *
 * For the example driver, please refer the MAX77686 driver:
 * - 'drivers/power/pmic/max77686.c'
 */

/**
 * struct dm_pmic_ops - PMIC device I/O interface
 *
 * Should be implemented by UCLASS_PMIC device drivers. The standard
 * device operations provides the I/O interface for it's childs.
 *
 * @reg_count: device's register count
 * @read:      read 'len' bytes at "reg" and store it into the 'buffer'
 * @write:     write 'len' bytes from the 'buffer' to the register at 'reg' address
 */
struct dm_pmic_ops {
	int (*reg_count)(struct udevice *dev);
	int (*read)(struct udevice *dev, uint reg, uint8_t *buffer, int len);
	int (*write)(struct udevice *dev, uint reg, const uint8_t *buffer,
		     int len);
};

/**
 * enum pmic_op_type - used for various pmic devices operation calls,
 * for reduce a number of lines with the same code for read/write or get/set.
 *
 * @PMIC_OP_GET - get operation
 * @PMIC_OP_SET - set operation
*/
enum pmic_op_type {
	PMIC_OP_GET,
	PMIC_OP_SET,
};

/**
 * struct pmic_child_info - basic device's child info for bind child nodes with
 * the driver by the node name prefix and driver name. This is a helper struct
 * for function: pmic_bind_children().
 *
 * @prefix - child node name prefix (or its name if is unique or single)
 * @driver - driver name for the sub-node with prefix
 */
struct pmic_child_info {
	const char *prefix;
	const char *driver;
};

/* drivers/power/pmic-uclass.c */

/**
 * pmic_bind_children() - bind drivers for given parent pmic, using child info
 * found in 'child_info' array.
 *
 * @pmic       - pmic device - the parent of found child's
 * @child_info - N-childs info array
 * @return a positive number of childs, or 0 if no child found (error)
 *
 * Note: For N-childs the child_info array should have N+1 entries and the last
 * entry prefix should be NULL - the same as for drivers compatible.
 *
 * For example, a single prefix info (N=1):
 * static const struct pmic_child_info bind_info[] = {
 *     { .prefix = "ldo", .driver = "ldo_driver" },
 *     { },
 * };
 *
 * This function is useful for regulator sub-nodes:
 * my_regulator@0xa {
 *     reg = <0xa>;
 *     (pmic - bind automatically by compatible)
 *     compatible = "my_pmic";
 *     ...
 *     (pmic's childs - bind by pmic_bind_children())
 *     (nodes prefix: "ldo", driver: "my_regulator_ldo")
 *     ldo1 { ... };
 *     ldo2 { ... };
 *
 *     (nodes prefix: "buck", driver: "my_regulator_buck")
 *     buck1 { ... };
 *     buck2 { ... };
 * };
 */
int pmic_bind_children(struct udevice *pmic, ofnode parent,
		       const struct pmic_child_info *child_info);

/**
 * pmic_get: get the pmic device using its name
 *
 * @name - device name
 * @devp - returned pointer to the pmic device
 * @return 0 on success or negative value of errno.
 *
 * The returned devp device can be used with pmic_read/write calls
 */
int pmic_get(const char *name, struct udevice **devp);

/**
 * pmic_reg_count: get the pmic register count
 *
 * The required pmic device can be obtained by 'pmic_get()'
 *
 * @dev - pointer to the UCLASS_PMIC device
 * @return register count value on success or negative value of errno.
 */
int pmic_reg_count(struct udevice *dev);

/**
 * pmic_read/write: read/write to the UCLASS_PMIC device
 *
 * The required pmic device can be obtained by 'pmic_get()'
 *
 * @pmic   - pointer to the UCLASS_PMIC device
 * @reg    - device register offset
 * @buffer - pointer to read/write buffer
 * @len    - byte count for read/write
 * @return 0 on success or negative value of errno.
 */
int pmic_read(struct udevice *dev, uint reg, uint8_t *buffer, int len);
int pmic_write(struct udevice *dev, uint reg, const uint8_t *buffer, int len);

/**
 * pmic_reg_read() - read a PMIC register value
 *
 * @dev:	PMIC device to read
 * @reg:	Register to read
 * @return value read on success or negative value of errno.
 */
int pmic_reg_read(struct udevice *dev, uint reg);

/**
 * pmic_reg_write() - write a PMIC register value
 *
 * @dev:	PMIC device to write
 * @reg:	Register to write
 * @value:	Value to write
 * @return 0 on success or negative value of errno.
 */
int pmic_reg_write(struct udevice *dev, uint reg, uint value);

/**
 * pmic_clrsetbits() - clear and set bits in a PMIC register
 *
 * This reads a register, optionally clears some bits, optionally sets some
 * bits, then writes the register.
 *
 * @dev:	PMIC device to update
 * @reg:	Register to update
 * @clr:	Bit mask to clear (set those bits that you want cleared)
 * @set:	Bit mask to set (set those bits that you want set)
 * @return 0 on success or negative value of errno.
 */
int pmic_clrsetbits(struct udevice *dev, uint reg, uint clr, uint set);

/*
 * This structure holds the private data for PMIC uclass
 * For now we store information about the number of bytes
 * being sent at once to the device.
 */
struct uc_pmic_priv {
	uint trans_len;
};

#endif /* CONFIG_DM_PMIC */

#ifdef CONFIG_POWER
int pmic_init(unsigned char bus);
int power_init_board(void);
int pmic_dialog_init(unsigned char bus);
int check_reg(struct pmic *p, u32 reg);
struct pmic *pmic_alloc(void);
struct pmic *pmic_get(const char *s);
int pmic_probe(struct pmic *p);
int pmic_reg_read(struct pmic *p, u32 reg, u32 *val);
int pmic_reg_write(struct pmic *p, u32 reg, u32 val);
int pmic_set_output(struct pmic *p, u32 reg, int ldo, int on);
#endif

#define pmic_i2c_addr (p->hw.i2c.addr)
#define pmic_i2c_tx_num (p->hw.i2c.tx_num)

#define pmic_spi_bitlen (p->hw.spi.bitlen)
#define pmic_spi_flags (p->hw.spi.flags)

#endif /* __CORE_PMIC_H_ */
