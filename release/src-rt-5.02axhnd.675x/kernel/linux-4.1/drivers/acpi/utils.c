/*
 *  acpi_utils.c - ACPI Utility Functions ($Revision: 10 $)
 *
 *  Copyright (C) 2001, 2002 Andy Grover <andrew.grover@intel.com>
 *  Copyright (C) 2001, 2002 Paul Diefenbaugh <paul.s.diefenbaugh@intel.com>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or (at
 *  your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/hardirq.h>
#include <linux/acpi.h>
#include <linux/dynamic_debug.h>

#include "internal.h"

#define _COMPONENT		ACPI_BUS_COMPONENT
ACPI_MODULE_NAME("utils");

/* --------------------------------------------------------------------------
                            Object Evaluation Helpers
   -------------------------------------------------------------------------- */
static void
acpi_util_eval_error(acpi_handle h, acpi_string p, acpi_status s)
{
#ifdef ACPI_DEBUG_OUTPUT
	char prefix[80] = {'\0'};
	struct acpi_buffer buffer = {sizeof(prefix), prefix};
	acpi_get_name(h, ACPI_FULL_PATHNAME, &buffer);
	ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Evaluate [%s.%s]: %s\n",
		(char *) prefix, p, acpi_format_exception(s)));
#else
	return;
#endif
}

acpi_status
acpi_extract_package(union acpi_object *package,
		     struct acpi_buffer *format, struct acpi_buffer *buffer)
{
	u32 size_required = 0;
	u32 tail_offset = 0;
	char *format_string = NULL;
	u32 format_count = 0;
	u32 i = 0;
	u8 *head = NULL;
	u8 *tail = NULL;


	if (!package || (package->type != ACPI_TYPE_PACKAGE)
	    || (package->package.count < 1)) {
		printk(KERN_WARNING PREFIX "Invalid package argument\n");
		return AE_BAD_PARAMETER;
	}

	if (!format || !format->pointer || (format->length < 1)) {
		printk(KERN_WARNING PREFIX "Invalid format argument\n");
		return AE_BAD_PARAMETER;
	}

	if (!buffer) {
		printk(KERN_WARNING PREFIX "Invalid buffer argument\n");
		return AE_BAD_PARAMETER;
	}

	format_count = (format->length / sizeof(char)) - 1;
	if (format_count > package->package.count) {
		printk(KERN_WARNING PREFIX "Format specifies more objects [%d]"
			      " than exist in package [%d].\n",
			      format_count, package->package.count);
		return AE_BAD_DATA;
	}

	format_string = format->pointer;

	/*
	 * Calculate size_required.
	 */
	for (i = 0; i < format_count; i++) {

		union acpi_object *element = &(package->package.elements[i]);

		switch (element->type) {

		case ACPI_TYPE_INTEGER:
			switch (format_string[i]) {
			case 'N':
				size_required += sizeof(u64);
				tail_offset += sizeof(u64);
				break;
			case 'S':
				size_required +=
				    sizeof(char *) + sizeof(u64) +
				    sizeof(char);
				tail_offset += sizeof(char *);
				break;
			default:
				printk(KERN_WARNING PREFIX "Invalid package element"
					      " [%d]: got number, expecting"
					      " [%c]\n",
					      i, format_string[i]);
				return AE_BAD_DATA;
				break;
			}
			break;

		case ACPI_TYPE_STRING:
		case ACPI_TYPE_BUFFER:
			switch (format_string[i]) {
			case 'S':
				size_required +=
				    sizeof(char *) +
				    (element->string.length * sizeof(char)) +
				    sizeof(char);
				tail_offset += sizeof(char *);
				break;
			case 'B':
				size_required +=
				    sizeof(u8 *) + element->buffer.length;
				tail_offset += sizeof(u8 *);
				break;
			default:
				printk(KERN_WARNING PREFIX "Invalid package element"
					      " [%d] got string/buffer,"
					      " expecting [%c]\n",
					      i, format_string[i]);
				return AE_BAD_DATA;
				break;
			}
			break;
		case ACPI_TYPE_LOCAL_REFERENCE:
			switch (format_string[i]) {
			case 'R':
				size_required += sizeof(void *);
				tail_offset += sizeof(void *);
				break;
			default:
				printk(KERN_WARNING PREFIX "Invalid package element"
					      " [%d] got reference,"
					      " expecting [%c]\n",
					      i, format_string[i]);
				return AE_BAD_DATA;
				break;
			}
			break;

		case ACPI_TYPE_PACKAGE:
		default:
			ACPI_DEBUG_PRINT((ACPI_DB_INFO,
					  "Found unsupported element at index=%d\n",
					  i));
			/* TBD: handle nested packages... */
			return AE_SUPPORT;
			break;
		}
	}

	/*
	 * Validate output buffer.
	 */
	if (buffer->length == ACPI_ALLOCATE_BUFFER) {
		buffer->pointer = ACPI_ALLOCATE_ZEROED(size_required);
		if (!buffer->pointer)
			return AE_NO_MEMORY;
		buffer->length = size_required;
	} else {
		if (buffer->length < size_required) {
			buffer->length = size_required;
			return AE_BUFFER_OVERFLOW;
		} else if (buffer->length != size_required ||
			   !buffer->pointer) {
			return AE_BAD_PARAMETER;
		}
	}

	head = buffer->pointer;
	tail = buffer->pointer + tail_offset;

	/*
	 * Extract package data.
	 */
	for (i = 0; i < format_count; i++) {

		u8 **pointer = NULL;
		union acpi_object *element = &(package->package.elements[i]);

		if (!element) {
			return AE_BAD_DATA;
		}

		switch (element->type) {

		case ACPI_TYPE_INTEGER:
			switch (format_string[i]) {
			case 'N':
				*((u64 *) head) =
				    element->integer.value;
				head += sizeof(u64);
				break;
			case 'S':
				pointer = (u8 **) head;
				*pointer = tail;
				*((u64 *) tail) =
				    element->integer.value;
				head += sizeof(u64 *);
				tail += sizeof(u64);
				/* NULL terminate string */
				*tail = (char)0;
				tail += sizeof(char);
				break;
			default:
				/* Should never get here */
				break;
			}
			break;

		case ACPI_TYPE_STRING:
		case ACPI_TYPE_BUFFER:
			switch (format_string[i]) {
			case 'S':
				pointer = (u8 **) head;
				*pointer = tail;
				memcpy(tail, element->string.pointer,
				       element->string.length);
				head += sizeof(char *);
				tail += element->string.length * sizeof(char);
				/* NULL terminate string */
				*tail = (char)0;
				tail += sizeof(char);
				break;
			case 'B':
				pointer = (u8 **) head;
				*pointer = tail;
				memcpy(tail, element->buffer.pointer,
				       element->buffer.length);
				head += sizeof(u8 *);
				tail += element->buffer.length;
				break;
			default:
				/* Should never get here */
				break;
			}
			break;
		case ACPI_TYPE_LOCAL_REFERENCE:
			switch (format_string[i]) {
			case 'R':
				*(void **)head =
				    (void *)element->reference.handle;
				head += sizeof(void *);
				break;
			default:
				/* Should never get here */
				break;
			}
			break;
		case ACPI_TYPE_PACKAGE:
			/* TBD: handle nested packages... */
		default:
			/* Should never get here */
			break;
		}
	}

	return AE_OK;
}

EXPORT_SYMBOL(acpi_extract_package);

acpi_status
acpi_evaluate_integer(acpi_handle handle,
		      acpi_string pathname,
		      struct acpi_object_list *arguments, unsigned long long *data)
{
	acpi_status status = AE_OK;
	union acpi_object element;
	struct acpi_buffer buffer = { 0, NULL };

	if (!data)
		return AE_BAD_PARAMETER;

	buffer.length = sizeof(union acpi_object);
	buffer.pointer = &element;
	status = acpi_evaluate_object(handle, pathname, arguments, &buffer);
	if (ACPI_FAILURE(status)) {
		acpi_util_eval_error(handle, pathname, status);
		return status;
	}

	if (element.type != ACPI_TYPE_INTEGER) {
		acpi_util_eval_error(handle, pathname, AE_BAD_DATA);
		return AE_BAD_DATA;
	}

	*data = element.integer.value;

	ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Return value [%llu]\n", *data));

	return AE_OK;
}

EXPORT_SYMBOL(acpi_evaluate_integer);

acpi_status
acpi_evaluate_reference(acpi_handle handle,
			acpi_string pathname,
			struct acpi_object_list *arguments,
			struct acpi_handle_list *list)
{
	acpi_status status = AE_OK;
	union acpi_object *package = NULL;
	union acpi_object *element = NULL;
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };
	u32 i = 0;


	if (!list) {
		return AE_BAD_PARAMETER;
	}

	/* Evaluate object. */

	status = acpi_evaluate_object(handle, pathname, arguments, &buffer);
	if (ACPI_FAILURE(status))
		goto end;

	package = buffer.pointer;

	if ((buffer.length == 0) || !package) {
		status = AE_BAD_DATA;
		acpi_util_eval_error(handle, pathname, status);
		goto end;
	}
	if (package->type != ACPI_TYPE_PACKAGE) {
		status = AE_BAD_DATA;
		acpi_util_eval_error(handle, pathname, status);
		goto end;
	}
	if (!package->package.count) {
		status = AE_BAD_DATA;
		acpi_util_eval_error(handle, pathname, status);
		goto end;
	}

	if (package->package.count > ACPI_MAX_HANDLES) {
		return AE_NO_MEMORY;
	}
	list->count = package->package.count;

	/* Extract package data. */

	for (i = 0; i < list->count; i++) {

		element = &(package->package.elements[i]);

		if (element->type != ACPI_TYPE_LOCAL_REFERENCE) {
			status = AE_BAD_DATA;
			acpi_util_eval_error(handle, pathname, status);
			break;
		}

		if (!element->reference.handle) {
			status = AE_NULL_ENTRY;
			acpi_util_eval_error(handle, pathname, status);
			break;
		}
		/* Get the  acpi_handle. */

		list->handles[i] = element->reference.handle;
		ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Found reference [%p]\n",
				  list->handles[i]));
	}

      end:
	if (ACPI_FAILURE(status)) {
		list->count = 0;
		//kfree(list->handles);
	}

	kfree(buffer.pointer);

	return status;
}

EXPORT_SYMBOL(acpi_evaluate_reference);

acpi_status
acpi_get_physical_device_location(acpi_handle handle, struct acpi_pld_info **pld)
{
	acpi_status status;
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };
	union acpi_object *output;

	status = acpi_evaluate_object(handle, "_PLD", NULL, &buffer);

	if (ACPI_FAILURE(status))
		return status;

	output = buffer.pointer;

	if (!output || output->type != ACPI_TYPE_PACKAGE
	    || !output->package.count
	    || output->package.elements[0].type != ACPI_TYPE_BUFFER
	    || output->package.elements[0].buffer.length < ACPI_PLD_REV1_BUFFER_SIZE) {
		status = AE_TYPE;
		goto out;
	}

	status = acpi_decode_pld_buffer(
			output->package.elements[0].buffer.pointer,
			output->package.elements[0].buffer.length,
			pld);

out:
	kfree(buffer.pointer);
	return status;
}
EXPORT_SYMBOL(acpi_get_physical_device_location);

/**
 * acpi_evaluate_ost: Evaluate _OST for hotplug operations
 * @handle: ACPI device handle
 * @source_event: source event code
 * @status_code: status code
 * @status_buf: optional detailed information (NULL if none)
 *
 * Evaluate _OST for hotplug operations. All ACPI hotplug handlers
 * must call this function when evaluating _OST for hotplug operations.
 * When the platform does not support _OST, this function has no effect.
 */
acpi_status
acpi_evaluate_ost(acpi_handle handle, u32 source_event, u32 status_code,
		  struct acpi_buffer *status_buf)
{
	union acpi_object params[3] = {
		{.type = ACPI_TYPE_INTEGER,},
		{.type = ACPI_TYPE_INTEGER,},
		{.type = ACPI_TYPE_BUFFER,}
	};
	struct acpi_object_list arg_list = {3, params};

	params[0].integer.value = source_event;
	params[1].integer.value = status_code;
	if (status_buf != NULL) {
		params[2].buffer.pointer = status_buf->pointer;
		params[2].buffer.length = status_buf->length;
	} else {
		params[2].buffer.pointer = NULL;
		params[2].buffer.length = 0;
	}

	return acpi_evaluate_object(handle, "_OST", &arg_list, NULL);
}
EXPORT_SYMBOL(acpi_evaluate_ost);

/**
 * acpi_handle_path: Return the object path of handle
 *
 * Caller must free the returned buffer
 */
static char *acpi_handle_path(acpi_handle handle)
{
	struct acpi_buffer buffer = {
		.length = ACPI_ALLOCATE_BUFFER,
		.pointer = NULL
	};

	if (in_interrupt() ||
	    acpi_get_name(handle, ACPI_FULL_PATHNAME, &buffer) != AE_OK)
		return NULL;
	return buffer.pointer;
}

/**
 * acpi_handle_printk: Print message with ACPI prefix and object path
 *
 * This function is called through acpi_handle_<level> macros and prints
 * a message with ACPI prefix and object path.  This function acquires
 * the global namespace mutex to obtain an object path.  In interrupt
 * context, it shows the object path as <n/a>.
 */
void
acpi_handle_printk(const char *level, acpi_handle handle, const char *fmt, ...)
{
	struct va_format vaf;
	va_list args;
	const char *path;

	va_start(args, fmt);
	vaf.fmt = fmt;
	vaf.va = &args;

	path = acpi_handle_path(handle);
	printk("%sACPI: %s: %pV", level, path ? path : "<n/a>" , &vaf);

	va_end(args);
	kfree(path);
}
EXPORT_SYMBOL(acpi_handle_printk);

#if defined(CONFIG_DYNAMIC_DEBUG)
/**
 * __acpi_handle_debug: pr_debug with ACPI prefix and object path
 *
 * This function is called through acpi_handle_debug macro and debug
 * prints a message with ACPI prefix and object path. This function
 * acquires the global namespace mutex to obtain an object path.  In
 * interrupt context, it shows the object path as <n/a>.
 */
void
__acpi_handle_debug(struct _ddebug *descriptor, acpi_handle handle,
		    const char *fmt, ...)
{
	struct va_format vaf;
	va_list args;
	const char *path;

	va_start(args, fmt);
	vaf.fmt = fmt;
	vaf.va = &args;

	path = acpi_handle_path(handle);
	__dynamic_pr_debug(descriptor, "ACPI: %s: %pV", path ? path : "<n/a>", &vaf);

	va_end(args);
	kfree(path);
}
EXPORT_SYMBOL(__acpi_handle_debug);
#endif

/**
 * acpi_has_method: Check whether @handle has a method named @name
 * @handle: ACPI device handle
 * @name: name of object or method
 *
 * Check whether @handle has a method named @name.
 */
bool acpi_has_method(acpi_handle handle, char *name)
{
	acpi_handle tmp;

	return ACPI_SUCCESS(acpi_get_handle(handle, name, &tmp));
}
EXPORT_SYMBOL(acpi_has_method);

acpi_status acpi_execute_simple_method(acpi_handle handle, char *method,
				       u64 arg)
{
	union acpi_object obj = { .type = ACPI_TYPE_INTEGER };
	struct acpi_object_list arg_list = { .count = 1, .pointer = &obj, };

	obj.integer.value = arg;

	return acpi_evaluate_object(handle, method, &arg_list, NULL);
}
EXPORT_SYMBOL(acpi_execute_simple_method);

/**
 * acpi_evaluate_ej0: Evaluate _EJ0 method for hotplug operations
 * @handle: ACPI device handle
 *
 * Evaluate device's _EJ0 method for hotplug operations.
 */
acpi_status acpi_evaluate_ej0(acpi_handle handle)
{
	acpi_status status;

	status = acpi_execute_simple_method(handle, "_EJ0", 1);
	if (status == AE_NOT_FOUND)
		acpi_handle_warn(handle, "No _EJ0 support for device\n");
	else if (ACPI_FAILURE(status))
		acpi_handle_warn(handle, "Eject failed (0x%x)\n", status);

	return status;
}

/**
 * acpi_evaluate_lck: Evaluate _LCK method to lock/unlock device
 * @handle: ACPI device handle
 * @lock: lock device if non-zero, otherwise unlock device
 *
 * Evaluate device's _LCK method if present to lock/unlock device
 */
acpi_status acpi_evaluate_lck(acpi_handle handle, int lock)
{
	acpi_status status;

	status = acpi_execute_simple_method(handle, "_LCK", !!lock);
	if (ACPI_FAILURE(status) && status != AE_NOT_FOUND) {
		if (lock)
			acpi_handle_warn(handle,
				"Locking device failed (0x%x)\n", status);
		else
			acpi_handle_warn(handle,
				"Unlocking device failed (0x%x)\n", status);
	}

	return status;
}

/**
 * acpi_evaluate_dsm - evaluate device's _DSM method
 * @handle: ACPI device handle
 * @uuid: UUID of requested functions, should be 16 bytes
 * @rev: revision number of requested function
 * @func: requested function number
 * @argv4: the function specific parameter
 *
 * Evaluate device's _DSM method with specified UUID, revision id and
 * function number. Caller needs to free the returned object.
 *
 * Though ACPI defines the fourth parameter for _DSM should be a package,
 * some old BIOSes do expect a buffer or an integer etc.
 */
union acpi_object *
acpi_evaluate_dsm(acpi_handle handle, const u8 *uuid, int rev, int func,
		  union acpi_object *argv4)
{
	acpi_status ret;
	struct acpi_buffer buf = {ACPI_ALLOCATE_BUFFER, NULL};
	union acpi_object params[4];
	struct acpi_object_list input = {
		.count = 4,
		.pointer = params,
	};

	params[0].type = ACPI_TYPE_BUFFER;
	params[0].buffer.length = 16;
	params[0].buffer.pointer = (char *)uuid;
	params[1].type = ACPI_TYPE_INTEGER;
	params[1].integer.value = rev;
	params[2].type = ACPI_TYPE_INTEGER;
	params[2].integer.value = func;
	if (argv4) {
		params[3] = *argv4;
	} else {
		params[3].type = ACPI_TYPE_PACKAGE;
		params[3].package.count = 0;
		params[3].package.elements = NULL;
	}

	ret = acpi_evaluate_object(handle, "_DSM", &input, &buf);
	if (ACPI_SUCCESS(ret))
		return (union acpi_object *)buf.pointer;

	if (ret != AE_NOT_FOUND)
		acpi_handle_warn(handle,
				"failed to evaluate _DSM (0x%x)\n", ret);

	return NULL;
}
EXPORT_SYMBOL(acpi_evaluate_dsm);

/**
 * acpi_check_dsm - check if _DSM method supports requested functions.
 * @handle: ACPI device handle
 * @uuid: UUID of requested functions, should be 16 bytes at least
 * @rev: revision number of requested functions
 * @funcs: bitmap of requested functions
 *
 * Evaluate device's _DSM method to check whether it supports requested
 * functions. Currently only support 64 functions at maximum, should be
 * enough for now.
 */
bool acpi_check_dsm(acpi_handle handle, const u8 *uuid, int rev, u64 funcs)
{
	int i;
	u64 mask = 0;
	union acpi_object *obj;

	if (funcs == 0)
		return false;

	obj = acpi_evaluate_dsm(handle, uuid, rev, 0, NULL);
	if (!obj)
		return false;

	/* For compatibility, old BIOSes may return an integer */
	if (obj->type == ACPI_TYPE_INTEGER)
		mask = obj->integer.value;
	else if (obj->type == ACPI_TYPE_BUFFER)
		for (i = 0; i < obj->buffer.length && i < 8; i++)
			mask |= (((u8)obj->buffer.pointer[i]) << (i * 8));
	ACPI_FREE(obj);

	/*
	 * Bit 0 indicates whether there's support for any functions other than
	 * function 0 for the specified UUID and revision.
	 */
	if ((mask & 0x1) && (mask & funcs) == funcs)
		return true;

	return false;
}
EXPORT_SYMBOL(acpi_check_dsm);
