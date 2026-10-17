// SPDX-License-Identifier: GPL-2.0
/*
 * Generation of tables for particular device types
 *
 * Copyright 2019 Google LLC
 * Mostly taken from coreboot file acpi_device.c
 */

#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <uuid.h>
#include <acpi/acpigen.h>
#include <acpi/acpi_dp.h>
#include <dm/acpi.h>

static void acpi_dp_write_array(struct acpi_ctx *ctx,
				const struct acpi_dp *array);

static void acpi_dp_write_value(struct acpi_ctx *ctx,
				const struct acpi_dp *prop)
{
	switch (prop->type) {
	case ACPI_DP_TYPE_INTEGER:
		acpigen_write_integer(ctx, prop->integer);
		break;
	case ACPI_DP_TYPE_STRING:
	case ACPI_DP_TYPE_CHILD:
		acpigen_write_string(ctx, prop->string);
		break;
	case ACPI_DP_TYPE_REFERENCE:
		acpigen_emit_namestring(ctx, prop->string);
		break;
	case ACPI_DP_TYPE_ARRAY:
		acpi_dp_write_array(ctx, prop->array);
		break;
	default:
		break;
	}
}

/* Package (2) { "prop->name", VALUE } */
static void acpi_dp_write_property(struct acpi_ctx *ctx,
				   const struct acpi_dp *prop)
{
	acpigen_write_package(ctx, 2);
	acpigen_write_string(ctx, prop->name);
	acpi_dp_write_value(ctx, prop);
	acpigen_pop_len(ctx);
}

/* Write array of Device Properties */
static void acpi_dp_write_array(struct acpi_ctx *ctx,
				const struct acpi_dp *array)
{
	const struct acpi_dp *dp;
	char *pkg_count;

	/* Package element count determined as it is populated */
	pkg_count = acpigen_write_package(ctx, 0);

	/*
	 * Only acpi_dp of type DP_TYPE_TABLE is allowed to be an array.
	 * DP_TYPE_TABLE does not have a value to be written. Thus, start
	 * the loop from next type in the array.
	 */
	for (dp = array->next; dp; dp = dp->next) {
		acpi_dp_write_value(ctx, dp);
		(*pkg_count)++;
	}

	acpigen_pop_len(ctx);
}

static void acpi_dp_free(struct acpi_dp *dp)
{
	assert(dp);
	while (dp) {
		struct acpi_dp *p = dp->next;

		switch (dp->type) {
		case ACPI_DP_TYPE_CHILD:
			acpi_dp_free(dp->child);
			break;
		case ACPI_DP_TYPE_ARRAY:
			acpi_dp_free(dp->array);
			break;
		default:
			break;
		}

		free(dp);
		dp = p;
	}
}

static int acpi_dp_write_internal(struct acpi_ctx *ctx, struct acpi_dp *table)
{
	struct acpi_dp *dp, *prop;
	char *dp_count, *prop_count = NULL;
	int child_count = 0;
	int ret;

	assert(table);
	if (table->type != ACPI_DP_TYPE_TABLE)
		return 0;

	/* Name (name) */
	acpigen_write_name(ctx, table->name);

	/* Device Property list starts with the next entry */
	prop = table->next;

	/* Package (DP), default to assuming no properties or children */
	dp_count = acpigen_write_package(ctx, 0);

	/* Print base properties */
	for (dp = prop; dp; dp = dp->next) {
		if (dp->type == ACPI_DP_TYPE_CHILD) {
			child_count++;
		} else {
			/*
			 * The UUID and package is only added when
			 * we come across the first property.  This
			 * is to avoid creating a zero-length package
			 * in situations where there are only children.
			 */
			if (!prop_count) {
				*dp_count += 2;
				/* ToUUID (ACPI_DP_UUID) */
				ret = acpigen_write_uuid(ctx, ACPI_DP_UUID);
				if (ret)
					return log_msg_ret("touuid", ret);
				/*
				 * Package (PROP), element count determined as
				 * it is populated
				 */
				prop_count = acpigen_write_package(ctx, 0);
			}
			(*prop_count)++;
			acpi_dp_write_property(ctx, dp);
		}
	}

	if (prop_count) {
		/* Package (PROP) length, if a package was written */
		acpigen_pop_len(ctx);
	}

	if (child_count) {
		/* Update DP package count to 2 or 4 */
		*dp_count += 2;
		/* ToUUID (ACPI_DP_CHILD_UUID) */
		ret = acpigen_write_uuid(ctx, ACPI_DP_CHILD_UUID);
		if (ret)
			return log_msg_ret("child uuid", ret);

		/* Print child pointer properties */
		acpigen_write_package(ctx, child_count);

		for (dp = prop; dp; dp = dp->next)
			if (dp->type == ACPI_DP_TYPE_CHILD)
				acpi_dp_write_property(ctx, dp);
		/* Package (CHILD) length */
		acpigen_pop_len(ctx);
	}

	/* Package (DP) length */
	acpigen_pop_len(ctx);

	/* Recursively parse children into separate tables */
	for (dp = prop; dp; dp = dp->next) {
		if (dp->type == ACPI_DP_TYPE_CHILD) {
			ret = acpi_dp_write_internal(ctx, dp->child);
			if (ret)
				return log_msg_ret("dp child", ret);
		}
	}

	return 0;
}

int acpi_dp_write(struct acpi_ctx *ctx, struct acpi_dp *table)
{
	int ret;

	ret = acpi_dp_write_internal(ctx, table);

	/* Clean up */
	acpi_dp_free(table);

	if (ret)
		return log_msg_ret("write", ret);

	return 0;
}

static struct acpi_dp *acpi_dp_new(struct acpi_dp *dp, enum acpi_dp_type type,
				   const char *name)
{
	struct acpi_dp *new;

	new = malloc(sizeof(struct acpi_dp));
	if (!new)
		return NULL;

	memset(new, '\0', sizeof(*new));
	new->type = type;
	new->name = name;

	if (dp) {
		/* Add to end of property list */
		while (dp->next)
			dp = dp->next;
		dp->next = new;
	}

	return new;
}

struct acpi_dp *acpi_dp_new_table(const char *name)
{
	return acpi_dp_new(NULL, ACPI_DP_TYPE_TABLE, name);
}

struct acpi_dp *acpi_dp_add_integer(struct acpi_dp *dp, const char *name,
				    u64 value)
{
	struct acpi_dp *new;

	assert(dp);
	new = acpi_dp_new(dp, ACPI_DP_TYPE_INTEGER, name);

	if (new)
		new->integer = value;

	return new;
}

struct acpi_dp *acpi_dp_add_string(struct acpi_dp *dp, const char *name,
				   const char *string)
{
	struct acpi_dp *new;

	assert(dp);
	new = acpi_dp_new(dp, ACPI_DP_TYPE_STRING, name);
	if (new)
		new->string = string;

	return new;
}

struct acpi_dp *acpi_dp_add_reference(struct acpi_dp *dp, const char *name,
				      const char *reference)
{
	struct acpi_dp *new;

	assert(dp);
	new = acpi_dp_new(dp, ACPI_DP_TYPE_REFERENCE, name);
	if (new)
		new->string = reference;

	return new;
}

struct acpi_dp *acpi_dp_add_child(struct acpi_dp *dp, const char *name,
				  struct acpi_dp *child)
{
	struct acpi_dp *new;

	assert(dp);
	if (child->type != ACPI_DP_TYPE_TABLE)
		return NULL;

	new = acpi_dp_new(dp, ACPI_DP_TYPE_CHILD, name);
	if (new) {
		new->child = child;
		new->string = child->name;
	}

	return new;
}

struct acpi_dp *acpi_dp_add_array(struct acpi_dp *dp, struct acpi_dp *array)
{
	struct acpi_dp *new;

	assert(dp);
	assert(array);
	if (array->type != ACPI_DP_TYPE_TABLE)
		return NULL;

	new = acpi_dp_new(dp, ACPI_DP_TYPE_ARRAY, array->name);
	if (new)
		new->array = array;

	return new;
}

struct acpi_dp *acpi_dp_add_integer_array(struct acpi_dp *dp, const char *name,
					  u64 *array, int len)
{
	struct acpi_dp *dp_array;
	int i;

	assert(dp);
	if (len <= 0)
		return NULL;

	dp_array = acpi_dp_new_table(name);
	if (!dp_array)
		return NULL;

	for (i = 0; i < len; i++)
		if (!acpi_dp_add_integer(dp_array, NULL, array[i]))
			break;

	if (!acpi_dp_add_array(dp, dp_array))
		return NULL;

	return dp_array;
}

struct acpi_dp *acpi_dp_add_gpio(struct acpi_dp *dp, const char *name,
				 const char *ref, int index, int pin,
				 enum acpi_gpio_polarity polarity)
{
	struct acpi_dp *gpio;

	assert(dp);
	gpio = acpi_dp_new_table(name);
	if (!gpio)
		return NULL;

	if (!acpi_dp_add_reference(gpio, NULL, ref) ||
	    !acpi_dp_add_integer(gpio, NULL, index) ||
	    !acpi_dp_add_integer(gpio, NULL, pin) ||
	    !acpi_dp_add_integer(gpio, NULL, polarity == ACPI_GPIO_ACTIVE_LOW))
		return NULL;

	if (!acpi_dp_add_array(dp, gpio))
		return NULL;

	return gpio;
}

int acpi_dp_ofnode_copy_int(ofnode node, struct acpi_dp *dp, const char *prop)
{
	int ret;
	u32 val = 0;

	ret = ofnode_read_u32(node, prop, &val);
	if (ret)
		return ret;
	if (!acpi_dp_add_integer(dp, prop, val))
		return log_ret(-ENOMEM);

	return 0;
}

int acpi_dp_ofnode_copy_str(ofnode node, struct acpi_dp *dp, const char *prop)
{
	const char *val;

	val = ofnode_read_string(node, prop);
	if (!val)
		return -EINVAL;
	if (!acpi_dp_add_string(dp, prop, val))
		return log_ret(-ENOMEM);

	return 0;
}

int acpi_dp_dev_copy_int(const struct udevice *dev, struct acpi_dp *dp,
			 const char *prop)
{
	int ret;
	u32 val = 0;

	ret = dev_read_u32(dev, prop, &val);
	if (ret)
		return ret;
	if (!acpi_dp_add_integer(dp, prop, val))
		return log_ret(-ENOMEM);

	return ret;
}

int acpi_dp_dev_copy_str(const struct udevice *dev, struct acpi_dp *dp,
			 const char *prop)
{
	const char *val;

	val = dev_read_string(dev, prop);
	if (!val)
		return -EINVAL;
	if (!acpi_dp_add_string(dp, prop, val))
		return log_ret(-ENOMEM);

	return 0;
}
