/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Copyright (C) 2014-2015 Samsung Electronics
 *  Przemyslaw Marczak <p.marczak@samsung.com>
 */

#ifndef _INCLUDE_REGULATOR_H_
#define _INCLUDE_REGULATOR_H_

/**
 * U-Boot Voltage/Current Regulator
 * ================================
 *
 * The regulator API is based on a driver model, with the device tree support.
 * And this header describes the functions and data types for the uclass id:
 * 'UCLASS_REGULATOR' and the regulator driver API.
 *
 * The regulator uclass - is based on uclass platform data which is allocated,
 * automatically for each regulator device on bind and 'dev->uclass_platdata'
 * points to it. The data type is: 'struct dm_regulator_uclass_platdata'.
 * The uclass file: 'drivers/power/regulator/regulator-uclass.c'
 *
 * The regulator device - is based on driver's model 'struct udevice'.
 * The API can use regulator name in two meanings:
 * - devname  - the regulator device's name: 'dev->name'
 * - platname - the device's platdata's name. So in the code it looks like:
 *              'uc_pdata = dev->uclass_platdata'; 'name = uc_pdata->name'.
 *
 * The regulator device driver - provide an implementation of uclass operations
 * pointed by 'dev->driver->ops' as a struct of type 'struct dm_regulator_ops'.
 *
 * To proper bind the regulator device, the device tree node should provide
 * regulator constraints, like in the example below:
 *
 * ldo1 {
 *      regulator-name = "VDD_MMC_1.8V";     (must be unique for proper bind)
 *      regulator-min-microvolt = <1000000>; (optional)
 *      regulator-max-microvolt = <1000000>; (optional)
 *      regulator-min-microamp = <1000>;     (optional)
 *      regulator-max-microamp = <1000>;     (optional)
 *      regulator-always-on;                 (optional)
 *      regulator-boot-on;                   (optional)
 * };
 *
 * Note: For the proper operation, at least name constraint is needed, since
 * it can be used when calling regulator_get_by_platname(). And the mandatory
 * rule for this name is, that it must be globally unique for the single dts.
 * If regulator-name property is not provided, node name will be chosen.
 *
 * Regulator bind:
 * For each regulator device, the device_bind() should be called with passed
 * device tree offset. This is required for this uclass's '.post_bind' method,
 * which does the scan on the device node, for the 'regulator-name' constraint.
 * If the parent is not a PMIC device, and the child is not bind by function:
 * 'pmic_bind_childs()', then it's recommended to bind the device by call to
 * dm_scan_fdt_dev() - this is usually done automatically for bus devices,
 * as a post bind method.
 *
 * Regulator get:
 * Having the device's name constraint, we can call regulator_by_platname(),
 * to find the required regulator. Before return, the regulator is probed,
 * and the rest of its constraints are put into the device's uclass platform
 * data, by the uclass regulator '.pre_probe' method.
 *
 * For more info about PMIC bind, please refer to file: 'include/power/pmic.h'
 *
 * Note:
 * Please do not use the device_bind_by_name() function, since it pass '-1' as
 * device node offset - and the bind will fail on uclass .post_bind method,
 * because of missing 'regulator-name' constraint.
 *
 *
 * Fixed Voltage/Current Regulator
 * ===============================
 *
 * When fixed voltage regulator is needed, then enable the config:
 * - CONFIG_DM_REGULATOR_FIXED
 *
 * The driver file: 'drivers/power/regulator/fixed.c', provides basic support
 * for control the GPIO, and return the device tree constraint values.
 *
 * To bind the fixed voltage regulator device, we usually use a 'simple-bus'
 * node as a parent. And 'regulator-fixed' for the driver compatible. This is
 * the same as in the kernel. The example node of fixed regulator:
 *
 * simple-bus {
 *     compatible = "simple-bus";
 *     #address-cells = <1>;
 *     #size-cells = <0>;
 *
 *     blue_led {
 *         compatible = "regulator-fixed";
 *         regulator-name = "VDD_LED_3.3V";
 *         regulator-min-microvolt = <3300000>;
 *         regulator-max-microvolt = <3300000>;
 *         gpio = <&gpc1 0 GPIO_ACTIVE_LOW>;
 *     };
 * };
 *
 * The fixed regulator devices also provide regulator uclass platform data. And
 * devices bound from such node, can use the regulator drivers API.
*/

/* enum regulator_type - used for regulator_*() variant calls */
enum regulator_type {
	REGULATOR_TYPE_LDO = 0,
	REGULATOR_TYPE_BUCK,
	REGULATOR_TYPE_DVS,
	REGULATOR_TYPE_FIXED,
	REGULATOR_TYPE_GPIO,
	REGULATOR_TYPE_OTHER,
};

/**
 * struct dm_regulator_mode - this structure holds an information about
 * each regulator operation mode. Probably in most cases - an array.
 * This will be probably a driver-static data, since it is device-specific.
 *
 * @id             - a driver-specific mode id
 * @register_value - a driver-specific value for its mode id
 * @name           - the name of mode - used for regulator command
 * Note:
 * The field 'id', should be always a positive number, since the negative values
 * are reserved for the errno numbers when returns the mode id.
 */
struct dm_regulator_mode {
	int id; /* Set only as >= 0 (negative value is reserved for errno) */
	int register_value;
	const char *name;
};

enum regulator_flag {
	REGULATOR_FLAG_AUTOSET_UV	= 1 << 0,
	REGULATOR_FLAG_AUTOSET_UA	= 1 << 1,
};

/**
 * struct dm_regulator_uclass_platdata - pointed by dev->uclass_platdata, and
 * allocated on each regulator bind. This structure holds an information
 * about each regulator's constraints and supported operation modes.
 * There is no "step" voltage value - so driver should take care of this.
 *
 * @type       - one of 'enum regulator_type'
 * @mode       - pointer to the regulator mode (array if more than one)
 * @mode_count - number of '.mode' entries
 * @min_uV*    - minimum voltage (micro Volts)
 * @max_uV*    - maximum voltage (micro Volts)
 * @min_uA*    - minimum amperage (micro Amps)
 * @max_uA*    - maximum amperage (micro Amps)
 * @always_on* - bool type, true or false
 * @boot_on*   - bool type, true or false
 * TODO(sjg@chromium.org): Consider putting the above two into @flags
 * @ramp_delay - Time to settle down after voltage change (unit: uV/us)
 * @flags:     - flags value (see REGULATOR_FLAG_...)
 * @name**     - fdt regulator name - should be taken from the device tree
 * ctrl_reg:   - Control register offset used to enable/disable regulator
 * volt_reg:   - register offset for writing voltage vsel values
 *
 * Note:
 * *  - set automatically on device probe by the uclass's '.pre_probe' method.
 * ** - set automatically on device bind by the uclass's '.post_bind' method.
 * The constraints: type, mode, mode_count, can be set by device driver, e.g.
 * by the driver '.probe' method.
 */
struct dm_regulator_uclass_platdata {
	enum regulator_type type;
	struct dm_regulator_mode *mode;
	int mode_count;
	int min_uV;
	int max_uV;
	int min_uA;
	int max_uA;
	unsigned int ramp_delay;
	bool always_on;
	bool boot_on;
	const char *name;
	int flags;
	u8 ctrl_reg;
	u8 volt_reg;
};

/* Regulator device operations */
struct dm_regulator_ops {
	/**
	 * The regulator output value function calls operates on a micro Volts.
	 *
	 * get/set_value - get/set output value of the given output number
	 * @dev          - regulator device
	 * Sets:
	 * @uV           - set the output value [micro Volts]
	 * @return output value [uV] on success or negative errno if fail.
	 */
	int (*get_value)(struct udevice *dev);
	int (*set_value)(struct udevice *dev, int uV);

	/**
	 * The regulator output current function calls operates on a micro Amps.
	 *
	 * get/set_current - get/set output current of the given output number
	 * @dev            - regulator device
	 * Sets:
	 * @uA           - set the output current [micro Amps]
	 * @return output value [uA] on success or negative errno if fail.
	 */
	int (*get_current)(struct udevice *dev);
	int (*set_current)(struct udevice *dev, int uA);

	/**
	 * The most basic feature of the regulator output is its enable state.
	 *
	 * get/set_enable - get/set enable state of the given output number
	 * @dev           - regulator device
	 * Sets:
	 * @enable         - set true - enable or false - disable
	 * @return true/false for get or -errno if fail; 0 / -errno for set.
	 */
	int (*get_enable)(struct udevice *dev);
	int (*set_enable)(struct udevice *dev, bool enable);

	/**
	 * The 'get/set_mode()' function calls should operate on a driver-
	 * specific mode id definitions, which should be found in:
	 * field 'id' of struct dm_regulator_mode.
	 *
	 * get/set_mode - get/set operation mode of the given output number
	 * @dev         - regulator device
	 * Sets
	 * @mode_id     - set output mode id (struct dm_regulator_mode->id)
	 * @return id/0 for get/set on success or negative errno if fail.
	 * Note:
	 * The field 'id' of struct type 'dm_regulator_mode', should be always
	 * a positive number, since the negative is reserved for the error.
	 */
	int (*get_mode)(struct udevice *dev);
	int (*set_mode)(struct udevice *dev, int mode_id);
};

/**
 * regulator_mode: returns a pointer to the array of regulator mode info
 *
 * @dev        - pointer to the regulator device
 * @modep      - pointer to the returned mode info array
 * @return     - count of modep entries on success or negative errno if fail.
 */
int regulator_mode(struct udevice *dev, struct dm_regulator_mode **modep);

/**
 * regulator_get_value: get microvoltage voltage value of a given regulator
 *
 * @dev    - pointer to the regulator device
 * @return - positive output value [uV] on success or negative errno if fail.
 */
int regulator_get_value(struct udevice *dev);

/**
 * regulator_set_value: set the microvoltage value of a given regulator.
 *
 * @dev    - pointer to the regulator device
 * @uV     - the output value to set [micro Volts]
 * @return - 0 on success or -errno val if fails
 */
int regulator_set_value(struct udevice *dev, int uV);

/**
 * regulator_set_value_force: set the microvoltage value of a given regulator
 *			      without any min-,max condition check
 *
 * @dev    - pointer to the regulator device
 * @uV     - the output value to set [micro Volts]
 * @return - 0 on success or -errno val if fails
 */
int regulator_set_value_force(struct udevice *dev, int uV);

/**
 * regulator_get_current: get microampere value of a given regulator
 *
 * @dev    - pointer to the regulator device
 * @return - positive output current [uA] on success or negative errno if fail.
 */
int regulator_get_current(struct udevice *dev);

/**
 * regulator_set_current: set the microampere value of a given regulator.
 *
 * @dev    - pointer to the regulator device
 * @uA     - set the output current [micro Amps]
 * @return - 0 on success or -errno val if fails
 */
int regulator_set_current(struct udevice *dev, int uA);

/**
 * regulator_get_enable: get regulator device enable state.
 *
 * @dev    - pointer to the regulator device
 * @return - true/false of enable state or -errno val if fails
 */
int regulator_get_enable(struct udevice *dev);

/**
 * regulator_set_enable: set regulator enable state
 *
 * @dev    - pointer to the regulator device
 * @enable - set true or false
 * @return - 0 on success or -errno val if fails
 */
int regulator_set_enable(struct udevice *dev, bool enable);

/**
 * regulator_set_enable_if_allowed: set regulator enable state if allowed by
 *					regulator
 *
 * @dev    - pointer to the regulator device
 * @enable - set true or false
 * @return - 0 on success or if enabling is not supported
 *	     -errno val if fails.
 */
int regulator_set_enable_if_allowed(struct udevice *dev, bool enable);

/**
 * regulator_get_mode: get active operation mode id of a given regulator
 *
 * @dev    - pointer to the regulator device
 * @return - positive mode 'id' number on success or -errno val if fails
 * Note:
 * The device can provide an array of operating modes, which is type of struct
 * dm_regulator_mode. Each mode has it's own 'id', which should be unique inside
 * that array. By calling this function, the driver should return an active mode
 * id of the given regulator device.
 */
int regulator_get_mode(struct udevice *dev);

/**
 * regulator_set_mode: set the given regulator's, active mode id
 *
 * @dev     - pointer to the regulator device
 * @mode_id - mode id to set ('id' field of struct type dm_regulator_mode)
 * @return  - 0 on success or -errno value if fails
 * Note:
 * The device can provide an array of operating modes, which is type of struct
 * dm_regulator_mode. Each mode has it's own 'id', which should be unique inside
 * that array. By calling this function, the driver should set the active mode
 * of a given regulator to given by "mode_id" argument.
 */
int regulator_set_mode(struct udevice *dev, int mode_id);

/**
 * regulators_enable_boot_on() - enable regulators needed for boot
 *
 * This enables all regulators which are marked to be on at boot time. This
 * only works for regulators which don't have a range for voltage/current,
 * since in that case it is not possible to know which value to use.
 *
 * This effectively calls regulator_autoset() for every regulator.
 */
int regulators_enable_boot_on(bool verbose);

/**
 * regulator_autoset: setup the voltage/current on a regulator
 *
 * The setup depends on constraints found in device's uclass's platform data
 * (struct dm_regulator_uclass_platdata):
 *
 * - Enable - will set - if any of: 'always_on' or 'boot_on' is set to true,
 *   or if both are unset, then the function returns
 * - Voltage value - will set - if '.min_uV' and '.max_uV' values are equal
 * - Current limit - will set - if '.min_uA' and '.max_uA' values are equal
 *
 * The function returns on the first-encountered error.
 *
 * @platname - expected string for dm_regulator_uclass_platdata .name field
 * @devp     - returned pointer to the regulator device - if non-NULL passed
 * @return: 0 on success or negative value of errno.
 */
int regulator_autoset(struct udevice *dev);

/**
 * regulator_autoset_by_name: setup the regulator given by its uclass's
 * platform data name field. The setup depends on constraints found in device's
 * uclass's platform data (struct dm_regulator_uclass_platdata):
 * - Enable - will set - if any of: 'always_on' or 'boot_on' is set to true,
 *   or if both are unset, then the function returns
 * - Voltage value - will set - if '.min_uV' and '.max_uV' values are equal
 * - Current limit - will set - if '.min_uA' and '.max_uA' values are equal
 *
 * The function returns on first encountered error.
 *
 * @platname - expected string for dm_regulator_uclass_platdata .name field
 * @devp     - returned pointer to the regulator device - if non-NULL passed
 * @return: 0 on success or negative value of errno.
 *
 * The returned 'regulator' device can be used with:
 * - regulator_get/set_*
 */
int regulator_autoset_by_name(const char *platname, struct udevice **devp);

/**
 * regulator_list_autoset: setup the regulators given by list of their uclass's
 * platform data name field. The setup depends on constraints found in device's
 * uclass's platform data. The function loops with calls to:
 * regulator_autoset_by_name() for each name from the list.
 *
 * @list_platname - an array of expected strings for .name field of each
 *                  regulator's uclass platdata
 * @list_devp     - an array of returned pointers to the successfully setup
 *                  regulator devices if non-NULL passed
 * @verbose       - (true/false) print each regulator setup info, or be quiet
 * @return 0 on successfully setup of all list entries, otherwise first error.
 *
 * The returned 'regulator' devices can be used with:
 * - regulator_get/set_*
 *
 * Note: The list must ends with NULL entry, like in the "platname" list below:
 * char *my_regulators[] = {
 *     "VCC_3.3V",
 *     "VCC_1.8V",
 *     NULL,
 * };
 */
int regulator_list_autoset(const char *list_platname[],
			   struct udevice *list_devp[],
			   bool verbose);

/**
 * regulator_get_by_devname: returns the pointer to the pmic regulator device.
 * Search by name, found in regulator device's name.
 *
 * @devname - expected string for 'dev->name' of regulator device
 * @devp    - returned pointer to the regulator device
 * @return 0 on success or negative value of errno.
 *
 * The returned 'regulator' device is probed and can be used with:
 * - regulator_get/set_*
 */
int regulator_get_by_devname(const char *devname, struct udevice **devp);

/**
 * regulator_get_by_platname: returns the pointer to the pmic regulator device.
 * Search by name, found in regulator uclass platdata.
 *
 * @platname - expected string for uc_pdata->name of regulator uclass platdata
 * @devp     - returns pointer to the regulator device or NULL on error
 * @return 0 on success or negative value of errno.
 *
 * The returned 'regulator' device is probed and can be used with:
 * - regulator_get/set_*
 */
int regulator_get_by_platname(const char *platname, struct udevice **devp);

/**
 * device_get_supply_regulator: returns the pointer to the supply regulator.
 * Search by phandle, found in device's node.
 *
 * Note: Please pay attention to proper order of device bind sequence.
 * The regulator device searched by the phandle, must be binded before
 * this function call.
 *
 * @dev         - device with supply phandle
 * @supply_name - phandle name of regulator
 * @devp        - returned pointer to the supply device
 * @return 0 on success or negative value of errno.
 */
int device_get_supply_regulator(struct udevice *dev, const char *supply_name,
				struct udevice **devp);

#endif /* _INCLUDE_REGULATOR_H_ */
