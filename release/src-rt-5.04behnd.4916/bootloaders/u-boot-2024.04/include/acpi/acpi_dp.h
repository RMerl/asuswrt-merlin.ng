/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Device properties, a temporary data structure for adding to ACPI code
 *
 * Copyright 2019 Google LLC
 * Mostly taken from coreboot file acpi_device.h
 */

#ifndef __ACPI_DP_H
#define __ACPI_DP_H

struct acpi_ctx;

#include <acpi/acpi_device.h>

/*
 * Writing Device Properties objects via _DSD
 *
 * This is described in ACPI 6.3 section 6.2.5
 *
 * This provides a structure to handle nested device-specific data which ends
 * up in a _DSD table.
 *
 * https://www.kernel.org/doc/html/latest/firmware-guide/acpi/DSD-properties-rules.html
 * https://uefi.org/sites/default/files/resources/_DSD-device-properties-UUID.pdf
 * https://uefi.org/sites/default/files/resources/_DSD-hierarchical-data-extension-UUID-v1.1.pdf
 *
 * The Device Property Hierarchy can be multiple levels deep with multiple
 * children possible in each level.  In order to support this flexibility
 * the device property hierarchy must be built up before being written out.
 *
 * For example:
 *
 * Child table with string and integer:
 * struct acpi_dp *child = acpi_dp_new_table("CHLD");
 * acpi_dp_add_string(child, "childstring", "CHILD");
 * acpi_dp_add_integer(child, "childint", 100);
 *
 * _DSD table with integer and gpio and child pointer:
 * struct acpi_dp *dsd = acpi_dp_new_table("_DSD");
 * acpi_dp_add_integer(dsd, "number1", 1);
 * acpi_dp_add_gpio(dsd, "gpio", "\_SB.PCI0.GPIO", 0, 0, 1);
 * acpi_dp_add_child(dsd, "child", child);
 *
 * Write entries into SSDT and clean up resources:
 * acpi_dp_write(dsd);
 *
 * Name(_DSD, Package() {
 *   ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301")
 *   Package() {
 *     Package() { "gpio", Package() { \_SB.PCI0.GPIO, 0, 0, 0 } }
 *     Package() { "number1", 1 }
 *   }
 *   ToUUID("dbb8e3e6-5886-4ba6-8795-1319f52a966b")
 *   Package() {
 *     Package() { "child", CHLD }
 *   }
 * }
 * Name(CHLD, Package() {
 *   ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301")
 *   Package() {
 *     Package() { "childstring", "CHILD" }
 *     Package() { "childint", 100 }
 *   }
 * }
 */

#define ACPI_DP_UUID		"daffd814-6eba-4d8c-8a91-bc9bbf4aa301"
#define ACPI_DP_CHILD_UUID	"dbb8e3e6-5886-4ba6-8795-1319f52a966b"

/**
 * enum acpi_dp_type - types of device property objects
 *
 * These refer to the types defined by struct acpi_dp below
 *
 * @ACPI_DP_TYPE_UNKNOWN: Unknown / do not use
 * @ACPI_DP_TYPE_INTEGER: Integer value (u64) in @integer
 * @ACPI_DP_TYPE_STRING: String value in @string
 * @ACPI_DP_TYPE_REFERENCE: Reference to another object, with value in @string
 * @ACPI_DP_TYPE_TABLE: Type for a top-level table which may have children
 * @ACPI_DP_TYPE_ARRAY: Array of items with first item in @array and following
 *	items linked from that item's @next
 * @ACPI_DP_TYPE_CHILD: Child object, with siblings in that child's @next
 */
enum acpi_dp_type {
	ACPI_DP_TYPE_UNKNOWN,
	ACPI_DP_TYPE_INTEGER,
	ACPI_DP_TYPE_STRING,
	ACPI_DP_TYPE_REFERENCE,
	ACPI_DP_TYPE_TABLE,
	ACPI_DP_TYPE_ARRAY,
	ACPI_DP_TYPE_CHILD,
};

/**
 * struct acpi_dp - ACPI device properties
 *
 * @type: Table type
 * @name: Name of object, typically _DSD but could be CHLD for a child object.
 *	This can be NULL if there is no name
 * @next: Next object in list (next array element or next sibling)
 * @child: Pointer to first child, if @type == ACPI_DP_TYPE_CHILD, else NULL
 * @array: First array element, if @type == ACPI_DP_TYPE_ARRAY, else NULL
 * @integer: Integer value of the property, if @type == ACPI_DP_TYPE_INTEGER
 * @string: String value of the property, if @type == ACPI_DP_TYPE_STRING;
 *	child name if @type == ACPI_DP_TYPE_CHILD;
 *	reference name if @type == ACPI_DP_TYPE_REFERENCE;
 */
struct acpi_dp {
	enum acpi_dp_type type;
	const char *name;
	struct acpi_dp *next;
	union {
		struct acpi_dp *child;
		struct acpi_dp *array;
	};
	union {
		u64 integer;
		const char *string;
	};
};

/**
 * acpi_dp_new_table() - Start a new Device Property table
 *
 * @ref: ACPI reference (e.g. "_DSD")
 * Return: pointer to table, or NULL if out of memory
 */
struct acpi_dp *acpi_dp_new_table(const char *ref);

/**
 * acpi_dp_add_integer() - Add integer Device Property
 *
 * A new node is added to the end of the property list of @dp
 *
 * @dp: Table to add this property to
 * @name: Name of property, or NULL for none
 * @value: Integer value
 * Return: pointer to new node, or NULL if out of memory
 */
struct acpi_dp *acpi_dp_add_integer(struct acpi_dp *dp, const char *name,
				    u64 value);

/**
 * acpi_dp_add_string() - Add string Device Property
 *
 * A new node is added to the end of the property list of @dp
 *
 * @dp: Table to add this property to
 * @name: Name of property, or NULL for none
 * @string: String value
 * Return: pointer to new node, or NULL if out of memory
 */
struct acpi_dp *acpi_dp_add_string(struct acpi_dp *dp, const char *name,
				   const char *string);

/**
 * acpi_dp_add_reference() - Add reference Device Property
 *
 * A new node is added to the end of the property list of @dp
 *
 * @dp: Table to add this property to
 * @name: Name of property, or NULL for none
 * @reference: Reference value
 * Return: pointer to new node, or NULL if out of memory
 */
struct acpi_dp *acpi_dp_add_reference(struct acpi_dp *dp, const char *name,
				      const char *reference);

/**
 * acpi_dp_add_array() - Add array Device Property
 *
 * A new node is added to the end of the property list of @dp, with the array
 * attached to that.
 *
 * @dp: Table to add this property to
 * @name: Name of property, or NULL for none
 * Return: pointer to new node, or NULL if out of memory
 */
struct acpi_dp *acpi_dp_add_array(struct acpi_dp *dp, struct acpi_dp *array);

/**
 * acpi_dp_add_integer_array() - Add an array of integers
 *
 * A new node is added to the end of the property list of @dp, with the array
 * attached to that. Each element of the array becomes a new node.
 *
 * @dp: Table to add this property to
 * @name: Name of property, or NULL for none
 * Return: pointer to new array node, or NULL if out of memory
 */
struct acpi_dp *acpi_dp_add_integer_array(struct acpi_dp *dp, const char *name,
					  u64 *array, int len);

/**
 * acpi_dp_add_child() - Add a child table of Device Properties
 *
 * A new node is added as a child of @dp
 *
 * @dp: Table to add this child to
 * @name: Name of child, or NULL for none
 * @child: Child node to add
 * Return: pointer to new child node, or NULL if out of memory
 */
struct acpi_dp *acpi_dp_add_child(struct acpi_dp *dp, const char *name,
				  struct acpi_dp *child);

/**
 * acpi_dp_add_gpio() - Add a GPIO to a list of Device Properties
 *
 * A new node is added to the end of the property list of @dp, with the
 * GPIO properties added to the the new node
 *
 * @dp: Table to add this property to
 * @name: Name of property
 * @ref: Reference to device with a _CRS containing GpioIO or GpioInt
 * @index: Index of the GPIO resource in _CRS starting from zero
 * @pin: Pin in the GPIO resource, typically zero
 * @polarity: GPIO polarity. Note that ACPI_IRQ_ACTIVE_BOTH is not supported
 * Return: pointer to new node, or NULL if out of memory
 */
struct acpi_dp *acpi_dp_add_gpio(struct acpi_dp *dp, const char *name,
				 const char *ref, int index, int pin,
				 enum acpi_gpio_polarity polarity);

/**
 * acpi_dp_write() - Write Device Property hierarchy and clean up resources
 *
 * This writes the table using acpigen and then frees it
 *
 * @ctx: ACPI context
 * @table: Table to write
 * Return: 0 if OK, -ve on error
 */
int acpi_dp_write(struct acpi_ctx *ctx, struct acpi_dp *table);

/**
 * acpi_dp_ofnode_copy_int() - Copy a property from device tree to DP
 *
 * This copies an integer property from the device tree to the ACPI DP table.
 *
 * @node: Node to copy from
 * @dp: DP to copy to
 * @prop: Property name to copy
 * Return: 0 if OK, -ve on error
 */
int acpi_dp_ofnode_copy_int(ofnode node, struct acpi_dp *dp, const char *prop);

/**
 * acpi_dp_ofnode_copy_str() - Copy a property from device tree to DP
 *
 * This copies a string property from the device tree to the ACPI DP table.
 *
 * @node: Node to copy from
 * @dp: DP to copy to
 * @prop: Property name to copy
 * Return: 0 if OK, -ve on error
 */
int acpi_dp_ofnode_copy_str(ofnode node, struct acpi_dp *dp, const char *prop);

/**
 * acpi_dp_dev_copy_int() - Copy a property from device tree to DP
 *
 * This copies an integer property from the device tree to the ACPI DP table.
 *
 * @dev: Device to copy from
 * @dp: DP to copy to
 * @prop: Property name to copy
 * Return: 0 if OK, -ve on error
 */
int acpi_dp_dev_copy_int(const struct udevice *dev, struct acpi_dp *dp,
			 const char *prop);

/**
 * acpi_dp_dev_copy_str() - Copy a property from device tree to DP
 *
 * This copies a string property from the device tree to the ACPI DP table.
 *
 * @dev: Device to copy from
 * @dp: DP to copy to
 * @prop: Property name to copy
 * Return: 0 if OK, -ve on error
 */
int acpi_dp_dev_copy_str(const struct udevice *dev, struct acpi_dp *dp,
			 const char *prop);

#endif
