#ifndef _SPMI_SPMI_H
#define _SPMI_SPMI_H

/**
 * struct dm_spmi_ops - SPMI device I/O interface
 *
 * Should be implemented by UCLASS_SPMI device drivers. The standard
 * device operations provides the I/O interface for it's childs.
 *
 * @read:      read register 'reg' of slave 'usid' and peripheral 'pid'
 * @write:     write register 'reg' of slave 'usid' and peripheral 'pid'
 *
 * Each register is 8-bit, both read and write can return negative values
 * on error.
 */
struct dm_spmi_ops {
	int (*read)(struct udevice *dev, int usid, int pid, int reg);
	int (*write)(struct udevice *dev, int usid, int pid, int reg,
		     uint8_t value);
};

/**
 * spmi_reg_read() - read a register from specific slave/peripheral
 *
 * @dev:	SPMI bus to read
 * @usid	SlaveID
 * @pid		Peripheral ID
 * @reg:	Register to read
 * Return: value read on success or negative value of errno.
 */
int spmi_reg_read(struct udevice *dev, int usid, int pid, int reg);

/**
 * spmi_reg_write() - write a register of specific slave/peripheral
 *
 * @dev:	SPMI bus to write
 * @usid	SlaveID
 * @pid		Peripheral ID
 * @reg:	Register to write
 * @value:	Value to write
 * Return: 0 on success or negative value of errno.
 */
int spmi_reg_write(struct udevice *dev, int usid, int pid, int reg,
		   uint8_t value);

#endif
