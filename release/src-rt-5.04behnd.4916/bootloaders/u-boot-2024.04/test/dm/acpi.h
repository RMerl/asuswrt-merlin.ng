/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common functions for ACPI tests
 *
 * Copyright 2020 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __TEST_DM_ACPI_H
#define __TEST_DM_ACPI_H

#define ACPI_TEST_DEV_NAME	"ABCD"
#define ACPI_TEST_CHILD_NAME	"EFGH"

/**
 * acpi_test_alloc_context_size() - Allocate an ACPI context of a given size
 *
 * @ctxp: Returns allocated context
 * @size: Size to allocate in bytes
 * Return: 0 if OK, -ENOMEM if out of memory
 */
int acpi_test_alloc_context_size(struct acpi_ctx **ctxp, int size);

/**
 * acpi_test_get_length() - decode a three-byte length field
 *
 * @ptr: Length encoded as per ACPI
 * Return: decoded length, or -EINVAL on error
 */
int acpi_test_get_length(u8 *ptr);

#endif /*__TEST_DM_ACPI_H */
