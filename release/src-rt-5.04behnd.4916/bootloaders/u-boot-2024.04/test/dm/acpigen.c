// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for ACPI code generation
 *
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <irq.h>
#include <malloc.h>
#include <uuid.h>
#include <acpi/acpigen.h>
#include <acpi/acpi_device.h>
#include <acpi/acpi_table.h>
#include <asm/gpio.h>
#include <asm/unaligned.h>
#include <dm/acpi.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <test/ut.h>
#include "acpi.h"

/* Maximum size of the ACPI context needed for most tests */
#define ACPI_CONTEXT_SIZE	150

#define TEST_STRING	"frogmore"
#define TEST_STRING2	"ranch"
#define TEST_STREAM2	"\xfa\xde"

#define TEST_INT8	0x7d
#define TEST_INT16	0x2345
#define TEST_INT32	0x12345678
#define TEST_INT64	0x4567890123456

int acpi_test_alloc_context_size(struct acpi_ctx **ctxp, int size)
{
	struct acpi_ctx *ctx;

	*ctxp = NULL;
	ctx = malloc(sizeof(*ctx));
	if (!ctx)
		return -ENOMEM;
	ctx->base = malloc(size);
	if (!ctx->base) {
		free(ctx);
		return -ENOMEM;
	}
	ctx->ltop = 0;
	ctx->current = ctx->base;
	*ctxp = ctx;

	return 0;
}

int acpi_test_get_length(u8 *ptr)
{
	if (!(*ptr & 0x80))
		return -EINVAL;

	return (*ptr & 0xf) | ptr[1] << 4 | ptr[2] << 12;
}

static int alloc_context(struct acpi_ctx **ctxp)
{
	return acpi_test_alloc_context_size(ctxp, ACPI_CONTEXT_SIZE);
}

static void free_context(struct acpi_ctx **ctxp)
{
	free((*ctxp)->base);
	free(*ctxp);
	*ctxp = NULL;
}

/* Test emitting simple types and acpigen_get_current() */
static int dm_test_acpi_emit_simple(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);
	acpigen_emit_byte(ctx, 0x23);
	ut_asserteq(1, acpigen_get_current(ctx) - ptr);
	ut_asserteq(0x23, *(u8 *)ptr);

	acpigen_emit_word(ctx, 0x1234);
	ut_asserteq(3, acpigen_get_current(ctx) - ptr);
	ut_asserteq(0x1234, get_unaligned((u16 *)(ptr + 1)));

	acpigen_emit_dword(ctx, 0x87654321);
	ut_asserteq(7, acpigen_get_current(ctx) - ptr);
	ut_asserteq(0x87654321, get_unaligned((u32 *)(ptr + 3)));

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_emit_simple, 0);

/* Test emitting a stream */
static int dm_test_acpi_emit_stream(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);
	acpigen_emit_stream(ctx, TEST_STREAM2, 2);
	ut_asserteq(2, acpigen_get_current(ctx) - ptr);
	ut_asserteq((u8)TEST_STREAM2[0], ptr[0]);
	ut_asserteq((u8)TEST_STREAM2[1], ptr[1]);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_emit_stream, 0);

/* Test emitting a string */
static int dm_test_acpi_emit_string(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);
	acpigen_emit_string(ctx, TEST_STRING);
	ut_asserteq(sizeof(TEST_STRING), acpigen_get_current(ctx) - ptr);
	ut_asserteq_str(TEST_STRING, (char *)ptr);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_emit_string, 0);

/* Test emitting an interrupt descriptor */
static int dm_test_acpi_interrupt(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	struct udevice *dev;
	struct irq irq;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	ut_assertok(uclass_first_device_err(UCLASS_TEST_FDT, &dev));
	ut_assertok(irq_get_by_index(dev, 0, &irq));

	/* See a-test, property interrupts-extended in the device tree */
	ut_asserteq(3, acpi_device_write_interrupt_irq(ctx, &irq));
	ut_asserteq(9, acpigen_get_current(ctx) - ptr);
	ut_asserteq(ACPI_DESCRIPTOR_INTERRUPT, ptr[0]);
	ut_asserteq(6, get_unaligned((u16 *)(ptr + 1)));
	ut_asserteq(0x19, ptr[3]);
	ut_asserteq(1, ptr[4]);
	ut_asserteq(3, get_unaligned((u32 *)(ptr + 5)));

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_interrupt, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test emitting a GPIO descriptor */
static int dm_test_acpi_gpio(struct unit_test_state *uts)
{
	struct gpio_desc desc;
	struct acpi_ctx *ctx;
	struct udevice *dev;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));
	ut_asserteq_str("a-test", dev->name);
	ut_assertok(gpio_request_by_name(dev, "test-gpios", 1, &desc, 0));

	/* This should write GPIO pin 4 (see device tree test.dts ) */
	ut_asserteq(4, acpi_device_write_gpio_desc(ctx, &desc));
	ut_asserteq(35, acpigen_get_current(ctx) - ptr);
	ut_asserteq(ACPI_DESCRIPTOR_GPIO, ptr[0]);
	ut_asserteq(32, get_unaligned((u16 *)(ptr + 1)));
	ut_asserteq(ACPI_GPIO_REVISION_ID, ptr[3]);
	ut_asserteq(ACPI_GPIO_TYPE_IO, ptr[4]);
	ut_asserteq(1, get_unaligned((u16 *)(ptr + 5)));
	ut_asserteq(9, get_unaligned((u16 *)(ptr + 7)));
	ut_asserteq(ACPI_GPIO_PULL_UP, ptr[9]);
	ut_asserteq(1234, get_unaligned((u16 *)(ptr + 10)));
	ut_asserteq(0, get_unaligned((u16 *)(ptr + 12)));
	ut_asserteq(23, get_unaligned((u16 *)(ptr + 14)));
	ut_asserteq(0, ptr[16]);
	ut_asserteq(25, get_unaligned((u16 *)(ptr + 17)));
	ut_asserteq(35, get_unaligned((u16 *)(ptr + 19)));
	ut_asserteq(0, get_unaligned((u16 *)(ptr + 21)));

	/* pin0 */
	ut_asserteq(4, get_unaligned((u16 *)(ptr + 23)));

	ut_asserteq_str("\\_SB.PINC", (char *)ptr + 25);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_gpio, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test emitting a GPIO descriptor with an interrupt */
static int dm_test_acpi_gpio_irq(struct unit_test_state *uts)
{
	struct gpio_desc desc;
	struct acpi_ctx *ctx;
	struct udevice *dev;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));
	ut_asserteq_str("a-test", dev->name);
	ut_assertok(gpio_request_by_name(dev, "test2-gpios", 2, &desc, 0));

	/* This should write GPIO pin 6 (see device tree test.dts ) */
	ut_asserteq(6, acpi_device_write_gpio_desc(ctx, &desc));
	ut_asserteq(35, acpigen_get_current(ctx) - ptr);
	ut_asserteq(ACPI_DESCRIPTOR_GPIO, ptr[0]);
	ut_asserteq(32, get_unaligned((u16 *)(ptr + 1)));
	ut_asserteq(ACPI_GPIO_REVISION_ID, ptr[3]);
	ut_asserteq(ACPI_GPIO_TYPE_INTERRUPT, ptr[4]);
	ut_asserteq(1, get_unaligned((u16 *)(ptr + 5)));
	ut_asserteq(29, get_unaligned((u16 *)(ptr + 7)));
	ut_asserteq(ACPI_GPIO_PULL_DOWN, ptr[9]);
	ut_asserteq(0, get_unaligned((u16 *)(ptr + 10)));
	ut_asserteq(4321, get_unaligned((u16 *)(ptr + 12)));
	ut_asserteq(23, get_unaligned((u16 *)(ptr + 14)));
	ut_asserteq(0, ptr[16]);
	ut_asserteq(25, get_unaligned((u16 *)(ptr + 17)));
	ut_asserteq(35, get_unaligned((u16 *)(ptr + 19)));
	ut_asserteq(0, get_unaligned((u16 *)(ptr + 21)));

	/* pin0 */
	ut_asserteq(6, get_unaligned((u16 *)(ptr + 23)));

	ut_asserteq_str("\\_SB.PINC", (char *)ptr + 25);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_gpio_irq, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test emitting either a GPIO or interrupt descriptor */
static int dm_test_acpi_interrupt_or_gpio(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	struct udevice *dev;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	/* This should produce an interrupt, even though it also has a GPIO */
	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));
	ut_asserteq_str("a-test", dev->name);
	ut_asserteq(3, acpi_device_write_interrupt_or_gpio(ctx, dev,
							   "test2-gpios"));
	ut_asserteq(ACPI_DESCRIPTOR_INTERRUPT, ptr[0]);

	/* This has no interrupt so should produce a GPIO */
	ptr = ctx->current;
	ut_assertok(uclass_find_first_device(UCLASS_PANEL_BACKLIGHT, &dev));
	ut_asserteq(1, acpi_device_write_interrupt_or_gpio(ctx, dev,
							   "enable-gpios"));
	ut_asserteq(ACPI_DESCRIPTOR_GPIO, ptr[0]);

	/* This one has neither */
	ptr = acpigen_get_current(ctx);
	ut_assertok(uclass_get_device_by_seq(UCLASS_TEST_FDT, 3, &dev));
	ut_asserteq_str("b-test", dev->name);
	ut_asserteq(-ENOENT,
		    acpi_device_write_interrupt_or_gpio(ctx, dev,
							"enable-gpios"));

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_interrupt_or_gpio,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test emitting an I2C descriptor */
static int dm_test_acpi_i2c(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	struct udevice *dev;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	ut_assertok(uclass_get_device(UCLASS_RTC, 0, &dev));
	ut_asserteq(0x43, acpi_device_write_i2c_dev(ctx, dev));
	ut_asserteq(28, acpigen_get_current(ctx) - ptr);
	ut_asserteq(ACPI_DESCRIPTOR_SERIAL_BUS, ptr[0]);
	ut_asserteq(25, get_unaligned((u16 *)(ptr + 1)));
	ut_asserteq(ACPI_I2C_SERIAL_BUS_REVISION_ID, ptr[3]);
	ut_asserteq(0, ptr[4]);
	ut_asserteq(ACPI_SERIAL_BUS_TYPE_I2C, ptr[5]);
	ut_asserteq(0, get_unaligned((u16 *)(ptr + 7)));
	ut_asserteq(ACPI_I2C_TYPE_SPECIFIC_REVISION_ID, ptr[9]);
	ut_asserteq(6, get_unaligned((u16 *)(ptr + 10)));
	ut_asserteq(100000, get_unaligned((u32 *)(ptr + 12)));
	ut_asserteq(0x43, get_unaligned((u16 *)(ptr + 16)));
	ut_asserteq_str("\\_SB.I2C0", (char *)ptr + 18);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_i2c, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test emitting a SPI descriptor */
static int dm_test_acpi_spi(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	struct udevice *dev;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	ut_assertok(uclass_first_device_err(UCLASS_SPI_FLASH, &dev));
	ut_assertok(acpi_device_write_spi_dev(ctx, dev));
	ut_asserteq(31, acpigen_get_current(ctx) - ptr);
	ut_asserteq(ACPI_DESCRIPTOR_SERIAL_BUS, ptr[0]);
	ut_asserteq(28, get_unaligned((u16 *)(ptr + 1)));
	ut_asserteq(ACPI_SPI_SERIAL_BUS_REVISION_ID, ptr[3]);
	ut_asserteq(0, ptr[4]);
	ut_asserteq(ACPI_SERIAL_BUS_TYPE_SPI, ptr[5]);
	ut_asserteq(2, ptr[6]);
	ut_asserteq(0, get_unaligned((u16 *)(ptr + 7)));
	ut_asserteq(ACPI_SPI_TYPE_SPECIFIC_REVISION_ID, ptr[9]);
	ut_asserteq(9, get_unaligned((u16 *)(ptr + 10)));
	ut_asserteq(40000000, get_unaligned((u32 *)(ptr + 12)));
	ut_asserteq(8, ptr[16]);
	ut_asserteq(0, ptr[17]);
	ut_asserteq(0, ptr[18]);
	ut_asserteq(0, get_unaligned((u16 *)(ptr + 19)));
	ut_asserteq_str("\\_SB.SPI0", (char *)ptr + 21);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_spi, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test emitting a length */
static int dm_test_acpi_len(struct unit_test_state *uts)
{
	const int size = 0xc0000;
	struct acpi_ctx *ctx;
	u8 *ptr;
	int i;

	ut_assertok(acpi_test_alloc_context_size(&ctx, size));

	ptr = acpigen_get_current(ctx);

	/* Write a byte and a 3-byte length */
	acpigen_write_len_f(ctx);
	acpigen_emit_byte(ctx, 0x23);
	acpigen_pop_len(ctx);
	ut_asserteq(1 + 3, acpi_test_get_length(ptr));

	/* Write 200 bytes so we need two length bytes */
	ptr = ctx->current;
	acpigen_write_len_f(ctx);
	for (i = 0; i < 200; i++)
		acpigen_emit_byte(ctx, 0x23);
	acpigen_pop_len(ctx);
	ut_asserteq(200 + 3, acpi_test_get_length(ptr));

	/* Write 40KB so we need three length bytes */
	ptr = ctx->current;
	acpigen_write_len_f(ctx);
	for (i = 0; i < 40000; i++)
		acpigen_emit_byte(ctx, 0x23);
	acpigen_pop_len(ctx);
	ut_asserteq(40000 + 3, acpi_test_get_length(ptr));

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_len, 0);

/* Test writing a package */
static int dm_test_acpi_package(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	char *num_elements;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	num_elements = acpigen_write_package(ctx, 3);
	ut_asserteq_ptr(num_elements, ptr + 4);

	/* For ease of testing, just emit a byte, not valid package contents */
	acpigen_emit_byte(ctx, 0x23);
	acpigen_pop_len(ctx);
	ut_asserteq(PACKAGE_OP, ptr[0]);
	ut_asserteq(5, acpi_test_get_length(ptr + 1));
	ut_asserteq(3, ptr[4]);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_package, 0);

/* Test writing an integer */
static int dm_test_acpi_integer(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	acpigen_write_integer(ctx, 0);
	acpigen_write_integer(ctx, 1);
	acpigen_write_integer(ctx, TEST_INT8);
	acpigen_write_integer(ctx, TEST_INT16);
	acpigen_write_integer(ctx, TEST_INT32);
	acpigen_write_integer(ctx, TEST_INT64);

	ut_asserteq(6 + 1 + 2 + 4 + 8, acpigen_get_current(ctx) - ptr);

	ut_asserteq(ZERO_OP, ptr[0]);

	ut_asserteq(ONE_OP, ptr[1]);

	ut_asserteq(BYTE_PREFIX, ptr[2]);
	ut_asserteq(TEST_INT8, ptr[3]);

	ut_asserteq(WORD_PREFIX, ptr[4]);
	ut_asserteq(TEST_INT16, get_unaligned((u16 *)(ptr + 5)));

	ut_asserteq(DWORD_PREFIX, ptr[7]);
	ut_asserteq(TEST_INT32, get_unaligned((u32 *)(ptr + 8)));

	ut_asserteq(QWORD_PREFIX, ptr[12]);
	ut_asserteq_64(TEST_INT64, get_unaligned((u64 *)(ptr + 13)));

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_integer, 0);

/* Test writing a string */
static int dm_test_acpi_string(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	acpigen_write_string(ctx, TEST_STRING);
	acpigen_write_string(ctx, TEST_STRING2);

	ut_asserteq(2 + sizeof(TEST_STRING) + sizeof(TEST_STRING2),
		    acpigen_get_current(ctx) - ptr);
	ut_asserteq(STRING_PREFIX, ptr[0]);
	ut_asserteq_str(TEST_STRING, (char *)ptr + 1);
	ptr += 1 + sizeof(TEST_STRING);
	ut_asserteq(STRING_PREFIX, ptr[0]);
	ut_asserteq_str(TEST_STRING2, (char *)ptr + 1);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_string, 0);

/* Test writing a name */
static int dm_test_acpi_name(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	/*
	 * The names here are made up for testing the various cases. The
	 * grammar is in the ACPI spec 6.3 section 19.2.2
	 */
	acpigen_write_name(ctx, "\\_SB");
	acpigen_write_name(ctx, "\\_SB.I2C0");
	acpigen_write_name(ctx, "\\_SB.I2C0.TPM2");
	acpigen_write_name(ctx, "\\_SB.I2C0.TPM2.LONG");
	acpigen_write_name(ctx, "^^^^SPI0.FLAS");
	acpigen_write_name(ctx, "NN");
	acpigen_write_name(ctx, "^AB.CD.D.EFG");
	acpigen_write_name(ctx, "^^^^");
	acpigen_write_name(ctx, "\\");
	acpigen_write_name(ctx, "\\ABCD");

	ut_asserteq(107, acpigen_get_current(ctx) - ptr);
	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq_strn("\\_SB_", (char *)ptr + 1);
	ptr += 6;

	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq('\\', ptr[1]);
	ut_asserteq(DUAL_NAME_PREFIX, ptr[2]);
	ut_asserteq_strn("_SB_I2C0", (char *)ptr + 3);
	ptr += 11;

	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq('\\', ptr[1]);
	ut_asserteq(MULTI_NAME_PREFIX, ptr[2]);
	ut_asserteq(3, ptr[3]);
	ut_asserteq_strn("_SB_I2C0TPM2", (char *)ptr + 4);
	ptr += 16;

	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq('\\', ptr[1]);
	ut_asserteq(MULTI_NAME_PREFIX, ptr[2]);
	ut_asserteq(4, ptr[3]);
	ut_asserteq_strn("_SB_I2C0TPM2LONG", (char *)ptr + 4);
	ptr += 20;

	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq('^', ptr[1]);
	ut_asserteq('^', ptr[2]);
	ut_asserteq('^', ptr[3]);
	ut_asserteq('^', ptr[4]);
	ut_asserteq(DUAL_NAME_PREFIX, ptr[5]);
	ut_asserteq_strn("SPI0FLAS", (char *)ptr + 6);
	ptr += 14;

	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq_strn("NN__", (char *)ptr + 1);
	ptr += 5;

	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq('^', ptr[1]);
	ut_asserteq(MULTI_NAME_PREFIX, ptr[2]);
	ut_asserteq(4, ptr[3]);
	ut_asserteq_strn("AB__CD__D___EFG_", (char *)ptr + 4);
	ptr += 20;

	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq('^', ptr[1]);
	ut_asserteq('^', ptr[2]);
	ut_asserteq('^', ptr[3]);
	ut_asserteq('^', ptr[4]);
	ut_asserteq(ZERO_OP, ptr[5]);
	ptr += 6;

	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq('\\', ptr[1]);
	ut_asserteq(ZERO_OP, ptr[2]);
	ptr += 3;

	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq_strn("\\ABCD", (char *)ptr + 1);
	ptr += 5;

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_name, 0);

/* Test writing a UUID */
static int dm_test_acpi_uuid(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	ut_assertok(acpigen_write_uuid(ctx,
				       "dbb8e3e6-5886-4ba6-8795-1319f52a966b"));
	ut_asserteq(23, acpigen_get_current(ctx) - ptr);
	ut_asserteq(BUFFER_OP, ptr[0]);
	ut_asserteq(22, acpi_test_get_length(ptr + 1));
	ut_asserteq(0xdbb8e3e6, get_unaligned((u32 *)(ptr + 7)));
	ut_asserteq(0x5886, get_unaligned((u16 *)(ptr + 11)));
	ut_asserteq(0x4ba6, get_unaligned((u16 *)(ptr + 13)));
	ut_asserteq(0x9587, get_unaligned((u16 *)(ptr + 15)));
	ut_asserteq(0x2af51913, get_unaligned((u32 *)(ptr + 17)));
	ut_asserteq(0x6b96, get_unaligned((u16 *)(ptr + 21)));

	/* Try a bad UUID */
	ut_asserteq(-EINVAL,
		    acpigen_write_uuid(ctx,
				       "dbb8e3e6-5886-4ba6x8795-1319f52a966b"));

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_uuid, 0);

/* Test writing misc ACPI codes */
static int dm_test_acpi_misc(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	const int flags = 3;
	const int nargs = 4;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);
	acpigen_write_sleep(ctx, TEST_INT64);
	ut_asserteq_64(TEST_INT64, get_unaligned((u64 *)(ptr + 3)));
	ptr += 11;

	acpigen_write_store(ctx);
	ut_asserteq(STORE_OP, *ptr);
	ptr++;

	acpigen_write_debug_string(ctx, TEST_STRING);
	ut_asserteq_str(TEST_STRING, (char *)ptr + 2);
	ptr += 2 +  sizeof(TEST_STRING);
	ut_asserteq(EXT_OP_PREFIX, ptr[0]);
	ut_asserteq(DEBUG_OP, ptr[1]);
	ptr += 2;

	acpigen_write_sta(ctx, flags);
	ut_asserteq(METHOD_OP, ptr[0]);
	ut_asserteq(11, acpi_test_get_length(ptr + 1));
	ut_asserteq_strn("_STA", (char *)ptr + 4);
	ut_asserteq(0, ptr[8]);
	ut_asserteq(RETURN_OP, ptr[9]);
	ut_asserteq(BYTE_PREFIX, ptr[10]);
	ut_asserteq(flags, ptr[11]);
	ptr += 12;

	acpigen_write_sleep(ctx, TEST_INT16);
	ut_asserteq(SLEEP_OP, ptr[1]);
	ut_asserteq(TEST_INT16, get_unaligned((u16 *)(ptr + 3)));
	ptr += 5;

	acpigen_write_method_serialized(ctx, "FRED", nargs);
	ut_asserteq(METHOD_OP, ptr[0]);
	ut_asserteq_strn("FRED", (char *)ptr + 4);
	ut_asserteq(1 << 3 | nargs, ptr[8]);
	ut_asserteq(1, ctx->ltop);	/* method is unfinished */

	ptr += 9;
	acpigen_write_or(ctx, LOCAL0_OP, LOCAL1_OP, LOCAL2_OP);
	acpigen_write_and(ctx, LOCAL3_OP, LOCAL4_OP, LOCAL5_OP);
	acpigen_write_not(ctx, LOCAL6_OP, LOCAL7_OP);
	ut_asserteq(OR_OP, ptr[0]);
	ut_asserteq(LOCAL0_OP, ptr[1]);
	ut_asserteq(LOCAL1_OP, ptr[2]);
	ut_asserteq(LOCAL2_OP, ptr[3]);

	ptr += 4;
	ut_asserteq(AND_OP, ptr[0]);
	ut_asserteq(LOCAL3_OP, ptr[1]);
	ut_asserteq(LOCAL4_OP, ptr[2]);
	ut_asserteq(LOCAL5_OP, ptr[3]);

	ptr += 4;
	ut_asserteq(NOT_OP, ptr[0]);
	ut_asserteq(LOCAL6_OP, ptr[1]);
	ut_asserteq(LOCAL7_OP, ptr[2]);
	ptr += 3;
	ut_asserteq_ptr(ptr, ctx->current);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_misc, 0);

/* Test writing an ACPI power resource */
static int dm_test_acpi_power_res(struct unit_test_state *uts)
{
	const char *const states[] = { "_PR0", "_PR3" };
	const char *name = "PRIC";
	const int level = 3;
	const int order = 2;
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	/* PowerResource (PRIC, 0, 0) */
	acpigen_write_power_res(ctx, name, level, order, states,
				ARRAY_SIZE(states));
	ut_asserteq(0x28, acpigen_get_current(ctx) - ptr);
	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq_strn(states[0], (char *)ptr + 1);
	ut_asserteq(8, acpi_test_get_length(ptr + 6));
	ut_asserteq_strn(name, (char *)ptr + 0xa);

	ut_asserteq_strn(states[1], (char *)ptr + 0xf);
	ut_asserteq(8, acpi_test_get_length(ptr + 0x14));
	ut_asserteq_strn(name, (char *)ptr + 0x18);

	ut_asserteq(POWER_RES_OP, ptr[0x1d]);
	ut_asserteq_strn(name, (char *)ptr + 0x21);
	ut_asserteq(level, ptr[0x25]);
	ut_asserteq(order, get_unaligned((u16 *)(ptr + 0x26)));

	/* The length is not set - caller must use acpigen_pop_len() */
	ut_asserteq(1, ctx->ltop);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_power_res, 0);

/* Test writing ACPI code to toggle a GPIO */
static int dm_test_acpi_gpio_toggle(struct unit_test_state *uts)
{
	const uint addr = 0x80012;
	const int txbit = BIT(2);
	struct gpio_desc desc;
	struct acpi_gpio gpio;
	struct acpi_ctx *ctx;
	struct udevice *dev;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));
	ut_asserteq_str("a-test", dev->name);
	ut_assertok(gpio_request_by_name(dev, "test2-gpios", 2, &desc, 0));
	ut_assertok(gpio_get_acpi(&desc, &gpio));

	/* Spot-check the results - see sb_gpio_get_acpi() */
	ptr = acpigen_get_current(ctx);
	acpigen_set_enable_tx_gpio(ctx, txbit, "\\_SB.GPC0", "\\_SB.SPC0",
				   &gpio, true);
	acpigen_set_enable_tx_gpio(ctx, txbit, "\\_SB.GPC0", "\\_SB.SPC0",
				   &gpio, false);

	/* Since this GPIO is active low, we expect it to be cleared here */
	ut_asserteq(STORE_OP, *ptr);
	ut_asserteq_strn("_SB_GPC0", (char *)ptr + 3);
	ut_asserteq(addr + desc.offset, get_unaligned((u32 *)(ptr + 0xc)));
	ut_asserteq(LOCAL5_OP, ptr[0x10]);

	ut_asserteq(STORE_OP, ptr[0x11]);
	ut_asserteq(BYTE_PREFIX, ptr[0x12]);
	ut_asserteq(txbit, ptr[0x13]);
	ut_asserteq(LOCAL0_OP, ptr[0x14]);

	ut_asserteq(NOT_OP, ptr[0x15]);
	ut_asserteq(LOCAL0_OP, ptr[0x16]);
	ut_asserteq(LOCAL6_OP, ptr[0x17]);
	ut_asserteq(AND_OP, ptr[0x18]);
	ut_asserteq_strn("_SB_SPC0", (char *)ptr + 0x1e);
	ut_asserteq(addr + desc.offset, get_unaligned((u32 *)(ptr + 0x27)));
	ut_asserteq(LOCAL5_OP, ptr[0x2b]);

	/* Now the second one, which should be set */
	ut_asserteq_strn("_SB_GPC0", (char *)ptr + 0x2f);
	ut_asserteq(addr + desc.offset, get_unaligned((u32 *)(ptr + 0x38)));
	ut_asserteq(LOCAL5_OP, ptr[0x3c]);

	ut_asserteq(STORE_OP, ptr[0x3d]);

	ut_asserteq(OR_OP, ptr[0x41]);
	ut_asserteq(LOCAL0_OP, ptr[0x43]);
	ut_asserteq_strn("_SB_SPC0", (char *)ptr + 0x47);
	ut_asserteq(addr + desc.offset, get_unaligned((u32 *)(ptr + 0x50)));
	ut_asserteq(LOCAL5_OP, ptr[0x54]);
	ut_asserteq(0x55, acpigen_get_current(ctx) - ptr);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_gpio_toggle, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test writing ACPI code to output power-sequence info */
static int dm_test_acpi_power_seq(struct unit_test_state *uts)
{
	struct gpio_desc reset, enable, stop;
	const uint addr = 0xc00dc, addr_act_low = 0x80012;
	const int txbit = BIT(2);
	struct acpi_ctx *ctx;
	struct udevice *dev;
	u8 *ptr;

	ut_assertok(acpi_test_alloc_context_size(&ctx, 400));

	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));
	ut_asserteq_str("a-test", dev->name);
	ut_assertok(gpio_request_by_name(dev, "test2-gpios", 0, &reset, 0));
	ut_assertok(gpio_request_by_name(dev, "test2-gpios", 1, &enable, 0));
	ut_assertok(gpio_request_by_name(dev, "test2-gpios", 2, &stop, 0));
	ptr = acpigen_get_current(ctx);

	ut_assertok(acpi_device_add_power_res(ctx, txbit, "\\_SB.GPC0",
					      "\\_SB.SPC0", &reset, 2, 3,
					      &enable, 4, 5, &stop, 6, 7));
	ut_asserteq(0x186, acpigen_get_current(ctx) - ptr);
	ut_asserteq_strn("PRIC", (char *)ptr + 0x18);

	/* First the 'ON' sequence - spot check */
	ut_asserteq_strn("_ON_", (char *)ptr + 0x38);

	/* reset set */
	ut_asserteq(addr + reset.offset, get_unaligned((u32 *)(ptr + 0x49)));
	ut_asserteq(OR_OP, ptr[0x52]);

	/* enable set */
	ut_asserteq(addr + enable.offset, get_unaligned((u32 *)(ptr + 0x72)));
	ut_asserteq(OR_OP, ptr[0x7b]);

	/* reset clear */
	ut_asserteq(addr + reset.offset, get_unaligned((u32 *)(ptr + 0x9f)));
	ut_asserteq(NOT_OP, ptr[0xa8]);

	/* stop set (disable, active low) */
	ut_asserteq(addr_act_low + stop.offset,
		    get_unaligned((u32 *)(ptr + 0xcf)));
	ut_asserteq(OR_OP, ptr[0xd8]);

	/* Now the 'OFF' sequence */
	ut_asserteq_strn("_OFF", (char *)ptr + 0xf4);

	/* stop clear (enable, active low) */
	ut_asserteq(addr_act_low + stop.offset,
		    get_unaligned((u32 *)(ptr + 0x105)));
	ut_asserteq(NOT_OP, ptr[0x10e]);

	/* reset clear */
	ut_asserteq(addr + reset.offset, get_unaligned((u32 *)(ptr + 0x135)));
	ut_asserteq(OR_OP, ptr[0x13e]);

	/* enable clear */
	ut_asserteq(addr + enable.offset, get_unaligned((u32 *)(ptr + 0x162)));
	ut_asserteq(NOT_OP, ptr[0x16b]);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_power_seq, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test writing values */
static int dm_test_acpi_write_values(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));
	ptr = acpigen_get_current(ctx);

	acpigen_write_zero(ctx);
	acpigen_write_one(ctx);
	acpigen_write_byte(ctx, TEST_INT8);
	acpigen_write_word(ctx, TEST_INT16);
	acpigen_write_dword(ctx, TEST_INT32);
	acpigen_write_qword(ctx, TEST_INT64);

	ut_asserteq(ZERO_OP, *ptr++);

	ut_asserteq(ONE_OP, *ptr++);

	ut_asserteq(BYTE_PREFIX, *ptr++);
	ut_asserteq(TEST_INT8, *ptr++);

	ut_asserteq(WORD_PREFIX, *ptr++);
	ut_asserteq(TEST_INT16, get_unaligned((u16 *)ptr));
	ptr += 2;

	ut_asserteq(DWORD_PREFIX, *ptr++);
	ut_asserteq(TEST_INT32, get_unaligned((u32 *)ptr));
	ptr += 4;

	ut_asserteq(QWORD_PREFIX, *ptr++);
	ut_asserteq_64(TEST_INT64, get_unaligned((u64 *)ptr));
	ptr += 8;

	ut_asserteq_ptr(ptr, ctx->current);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_write_values, 0);

/* Test writing a scope */
static int dm_test_acpi_scope(struct unit_test_state *uts)
{
	char buf[ACPI_PATH_MAX];
	struct acpi_ctx *ctx;
	struct udevice *dev;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));
	ptr = acpigen_get_current(ctx);

	ut_assertok(uclass_first_device_err(UCLASS_TEST_ACPI, &dev));
	ut_assertok(acpi_device_path(dev, buf, sizeof(buf)));
	acpigen_write_scope(ctx, buf);
	acpigen_pop_len(ctx);

	ut_asserteq(SCOPE_OP, *ptr++);
	ut_asserteq(13, acpi_test_get_length(ptr));
	ptr += 3;
	ut_asserteq(ROOT_PREFIX, *ptr++);
	ut_asserteq(DUAL_NAME_PREFIX, *ptr++);
	ut_asserteq_strn("_SB_" ACPI_TEST_DEV_NAME, (char *)ptr);
	ptr += 8;
	ut_asserteq_ptr(ptr, ctx->current);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_scope, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test writing a resource template */
static int dm_test_acpi_resource_template(struct unit_test_state *uts)
{
	struct acpi_gen_regaddr addr;
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));
	ptr = acpigen_get_current(ctx);

	addr.space_id = ACPI_ADDRESS_SPACE_EC;
	addr.bit_width = 32;
	addr.bit_offset = 8;
	addr.access_size = ACPI_ACCESS_SIZE_DWORD_ACCESS;
	addr.addrl = TEST_INT64 & 0xffffffff;
	addr.addrh = TEST_INT64 >> 32;
	acpigen_write_register_resource(ctx, &addr);

	ut_asserteq(BUFFER_OP, *ptr++);
	ut_asserteq(0x17, acpi_test_get_length(ptr));
	ptr += 3;
	ut_asserteq(WORD_PREFIX, *ptr++);
	ut_asserteq(0x11, get_unaligned((u16 *)ptr));
	ptr += 2;
	ut_asserteq(ACPI_DESCRIPTOR_REGISTER, *ptr++);
	ut_asserteq(0xc, *ptr++);
	ut_asserteq(0, *ptr++);
	ut_asserteq(ACPI_ADDRESS_SPACE_EC, *ptr++);
	ut_asserteq(32, *ptr++);
	ut_asserteq(8, *ptr++);
	ut_asserteq(ACPI_ACCESS_SIZE_DWORD_ACCESS, *ptr++);
	ut_asserteq(TEST_INT64 & 0xffffffff, get_unaligned((u32 *)ptr));
	ptr += 4;
	ut_asserteq(TEST_INT64 >> 32, get_unaligned((u32 *)ptr));
	ptr += 4;
	ut_asserteq(ACPI_END_TAG, *ptr++);
	ut_asserteq(0x00, *ptr++);
	ut_asserteq_ptr(ptr, ctx->current);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_resource_template, 0);

/* Test writing a device */
static int dm_test_acpi_device(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));
	ptr = acpigen_get_current(ctx);

	acpigen_write_device(ctx, "\\_SB." ACPI_TEST_DEV_NAME);
	acpigen_pop_len(ctx);

	ut_asserteq(EXT_OP_PREFIX, *ptr++);
	ut_asserteq(DEVICE_OP, *ptr++);
	ut_asserteq(0xd, acpi_test_get_length(ptr));
	ptr += 3;
	ut_asserteq(ROOT_PREFIX, *ptr++);
	ut_asserteq(DUAL_NAME_PREFIX, *ptr++);
	ptr += 8;
	ut_asserteq_ptr(ptr, ctx->current);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_device, 0);

/* Test writing named values */
static int dm_test_acpi_write_name(struct unit_test_state *uts)
{
	const char *name = "\\_SB." ACPI_TEST_DEV_NAME;
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));
	ptr = acpigen_get_current(ctx);

	acpigen_write_name_zero(ctx, name);
	acpigen_write_name_one(ctx, name);
	acpigen_write_name_byte(ctx, name, TEST_INT8);
	acpigen_write_name_word(ctx, name, TEST_INT16);
	acpigen_write_name_dword(ctx, name, TEST_INT32);
	acpigen_write_name_qword(ctx, name, TEST_INT64);
	acpigen_write_name_integer(ctx, name, TEST_INT64 + 1);
	acpigen_write_name_string(ctx, name, "baldrick");
	acpigen_write_name_string(ctx, name, NULL);

	ut_asserteq(NAME_OP, *ptr++);
	ut_asserteq_strn("\\._SB_ABCD", (char *)ptr);
	ptr += 10;
	ut_asserteq(ZERO_OP, *ptr++);

	ut_asserteq(NAME_OP, *ptr++);
	ptr += 10;
	ut_asserteq(ONE_OP, *ptr++);

	ut_asserteq(NAME_OP, *ptr++);
	ptr += 10;
	ut_asserteq(BYTE_PREFIX, *ptr++);
	ut_asserteq(TEST_INT8, *ptr++);

	ut_asserteq(NAME_OP, *ptr++);
	ptr += 10;
	ut_asserteq(WORD_PREFIX, *ptr++);
	ut_asserteq(TEST_INT16, get_unaligned((u16 *)ptr));
	ptr += 2;

	ut_asserteq(NAME_OP, *ptr++);
	ptr += 10;
	ut_asserteq(DWORD_PREFIX, *ptr++);
	ut_asserteq(TEST_INT32, get_unaligned((u32 *)ptr));
	ptr += 4;

	ut_asserteq(NAME_OP, *ptr++);
	ptr += 10;
	ut_asserteq(QWORD_PREFIX, *ptr++);
	ut_asserteq_64(TEST_INT64, get_unaligned((u64 *)ptr));
	ptr += 8;

	ut_asserteq(NAME_OP, *ptr++);
	ptr += 10;
	ut_asserteq(QWORD_PREFIX, *ptr++);
	ut_asserteq_64(TEST_INT64 + 1, get_unaligned((u64 *)ptr));
	ptr += 8;

	ut_asserteq(NAME_OP, *ptr++);
	ptr += 10;
	ut_asserteq(STRING_PREFIX, *ptr++);
	ut_asserteq_str("baldrick", (char *)ptr);
	ptr += 9;

	ut_asserteq(NAME_OP, *ptr++);
	ptr += 10;
	ut_asserteq(STRING_PREFIX, *ptr++);
	ut_asserteq('\0', *ptr++);

	ut_asserteq_ptr(ptr, ctx->current);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_write_name, 0);

/* Test emitting a _PRW component */
static int dm_test_acpi_write_prw(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);
	acpigen_write_prw(ctx, 5, 3);
	ut_asserteq(NAME_OP, *ptr++);

	ut_asserteq_strn("_PRW", (char *)ptr);
	ptr += 4;
	ut_asserteq(PACKAGE_OP, *ptr++);
	ut_asserteq(8, acpi_test_get_length(ptr));
	ptr += 3;
	ut_asserteq(2, *ptr++);
	ut_asserteq(BYTE_PREFIX, *ptr++);
	ut_asserteq(5, *ptr++);
	ut_asserteq(BYTE_PREFIX, *ptr++);
	ut_asserteq(3, *ptr++);
	ut_asserteq_ptr(ptr, ctx->current);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_write_prw, 0);

/* Test emitting writing conditionals */
static int dm_test_acpi_write_cond(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);
	acpigen_write_if(ctx);
	acpigen_pop_len(ctx);
	ut_asserteq(IF_OP, *ptr++);
	ut_asserteq(3, acpi_test_get_length(ptr));
	ptr += 3;

	acpigen_write_else(ctx);
	acpigen_pop_len(ctx);
	ut_asserteq(ELSE_OP, *ptr++);
	ut_asserteq(3, acpi_test_get_length(ptr));
	ptr += 3;

	acpigen_write_if_lequal_op_int(ctx, LOCAL1_OP, 5);
	acpigen_pop_len(ctx);
	ut_asserteq(IF_OP, *ptr++);
	ut_asserteq(7, acpi_test_get_length(ptr));
	ptr += 3;
	ut_asserteq(LEQUAL_OP, *ptr++);
	ut_asserteq(LOCAL1_OP, *ptr++);
	ut_asserteq(BYTE_PREFIX, *ptr++);
	ut_asserteq(5, *ptr++);

	ut_asserteq_ptr(ptr, ctx->current);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_write_cond, 0);

/* Test emitting writing return values and ToBuffer/ToInteger */
static int dm_test_acpi_write_return(struct unit_test_state *uts)
{
	int len = sizeof(TEST_STRING);
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);
	acpigen_write_to_buffer(ctx, ARG0_OP, LOCAL0_OP);
	ut_asserteq(TO_BUFFER_OP, *ptr++);
	ut_asserteq(ARG0_OP, *ptr++);
	ut_asserteq(LOCAL0_OP, *ptr++);

	acpigen_write_to_integer(ctx, ARG0_OP, LOCAL0_OP);
	ut_asserteq(TO_INTEGER_OP, *ptr++);
	ut_asserteq(ARG0_OP, *ptr++);
	ut_asserteq(LOCAL0_OP, *ptr++);

	acpigen_write_return_byte_buffer(ctx, (u8 *)TEST_STRING, len);
	ut_asserteq(RETURN_OP, *ptr++);
	ut_asserteq(BUFFER_OP, *ptr++);
	ut_asserteq(5 + len, acpi_test_get_length(ptr));
	ptr += 3;
	ut_asserteq(BYTE_PREFIX, *ptr++);
	ut_asserteq(len, *ptr++);
	ut_asserteq_mem(TEST_STRING, ptr, len);
	ptr += len;

	acpigen_write_return_singleton_buffer(ctx, 123);
	len = 1;
	ut_asserteq(RETURN_OP, *ptr++);
	ut_asserteq(BUFFER_OP, *ptr++);
	ut_asserteq(4 + len, acpi_test_get_length(ptr));
	ptr += 3;
	ut_asserteq(ONE_OP, *ptr++);
	ut_asserteq(123, *ptr++);

	acpigen_write_return_byte(ctx, 43);
	ut_asserteq(RETURN_OP, *ptr++);
	ut_asserteq(BYTE_PREFIX, *ptr++);
	ut_asserteq(43, *ptr++);

	ut_asserteq_ptr(ptr, ctx->current);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_write_return, 0);

/* Test emitting a DSM for an I2C HID */
static int dm_test_acpi_write_i2c_dsm(struct unit_test_state *uts)
{
	char uuid_str[UUID_STR_LEN + 1];
	const int reg_offset = 0x20;
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);
	ut_assertok(acpi_device_write_dsm_i2c_hid(ctx, reg_offset));

	/* acpigen_write_dsm_start() */
	ut_asserteq(METHOD_OP, *ptr++);
	ut_asserteq(0x78, acpi_test_get_length(ptr));
	ptr += 3;
	ut_asserteq_strn("_DSM", (char *)ptr);
	ptr += 4;
	ut_asserteq(ACPI_METHOD_SERIALIZED_MASK | 4, *ptr++);

	ut_asserteq(TO_BUFFER_OP, *ptr++);
	ut_asserteq(ARG0_OP, *ptr++);
	ut_asserteq(LOCAL0_OP, *ptr++);

	/* acpigen_write_dsm_uuid_start() */
	ut_asserteq(IF_OP, *ptr++);
	ut_asserteq(0x65, acpi_test_get_length(ptr));
	ptr += 3;
	ut_asserteq(LEQUAL_OP, *ptr++);
	ut_asserteq(LOCAL0_OP, *ptr++);

	ut_asserteq(BUFFER_OP, *ptr++);
	ut_asserteq(UUID_BIN_LEN + 6, acpi_test_get_length(ptr));
	ptr += 3;
	ut_asserteq(WORD_PREFIX, *ptr++);
	ut_asserteq(UUID_BIN_LEN, get_unaligned((u16 *)ptr));
	ptr += 2;
	uuid_bin_to_str(ptr, uuid_str, UUID_STR_FORMAT_GUID);
	ut_asserteq_str(ACPI_DSM_I2C_HID_UUID, uuid_str);
	ptr += UUID_BIN_LEN;

	ut_asserteq(TO_INTEGER_OP, *ptr++);
	ut_asserteq(ARG2_OP, *ptr++);
	ut_asserteq(LOCAL1_OP, *ptr++);

	/* acpigen_write_dsm_uuid_start_cond() */
	ut_asserteq(IF_OP, *ptr++);
	ut_asserteq(0x34, acpi_test_get_length(ptr));
	ptr += 3;
	ut_asserteq(LEQUAL_OP, *ptr++);
	ut_asserteq(LOCAL1_OP, *ptr++);
	ut_asserteq(ZERO_OP, *ptr++);

	/*
	 * See code from acpi_device_write_dsm_i2c_hid(). We don't check every
	 * piece
	 */
	ut_asserteq(TO_INTEGER_OP, *ptr++);
	ut_asserteq(ARG1_OP, *ptr++);
	ut_asserteq(LOCAL2_OP, *ptr++);

	ut_asserteq(IF_OP, *ptr++);
	ut_asserteq(0xd, acpi_test_get_length(ptr));
	ptr += 3;
	ut_asserteq(LEQUAL_OP, *ptr++);
	ut_asserteq(LOCAL2_OP, *ptr++);
	ut_asserteq(ZERO_OP, *ptr++);	/* function 0 */

	ut_asserteq(RETURN_OP, *ptr++);
	ut_asserteq(BUFFER_OP, *ptr++);
	ptr += 5;

	ut_asserteq(ELSE_OP, *ptr++);
	ptr += 3;

	ut_asserteq(IF_OP, *ptr++);
	ut_asserteq(0xd, acpi_test_get_length(ptr));
	ptr += 3;
	ut_asserteq(LEQUAL_OP, *ptr++);
	ut_asserteq(LOCAL2_OP, *ptr++);
	ut_asserteq(ONE_OP, *ptr++);

	ut_asserteq(RETURN_OP, *ptr++);
	ut_asserteq(BUFFER_OP, *ptr++);
	ptr += 5;

	ut_asserteq(ELSE_OP, *ptr++);
	ptr += 3;

	ut_asserteq(RETURN_OP, *ptr++);
	ut_asserteq(BUFFER_OP, *ptr++);
	ptr += 5;

	/* acpigen_write_dsm_uuid_start_cond() */
	ut_asserteq(IF_OP, *ptr++);
	ut_asserteq(9, acpi_test_get_length(ptr));
	ptr += 3;
	ut_asserteq(LEQUAL_OP, *ptr++);
	ut_asserteq(LOCAL1_OP, *ptr++);
	ut_asserteq(ONE_OP, *ptr++);	/* function 1 */

	ut_asserteq(RETURN_OP, *ptr++);
	ut_asserteq(BYTE_PREFIX, *ptr++);
	ut_asserteq(reg_offset, *ptr++);

	/* acpigen_write_dsm_uuid_end() */
	ut_asserteq(RETURN_OP, *ptr++);
	ut_asserteq(BUFFER_OP, *ptr++);
	ptr += 5;

	/* acpigen_write_dsm_end */
	ut_asserteq(RETURN_OP, *ptr++);
	ut_asserteq(BUFFER_OP, *ptr++);
	ptr += 5;

	ut_asserteq_ptr(ptr, ctx->current);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_write_i2c_dsm, 0);

/* Test emitting a processor */
static int dm_test_acpi_write_processor(struct unit_test_state *uts)
{
	const int cpuindex = 6;
	const u32 pblock_addr = 0x12345600;
	const u32 pblock_len = 0x60;
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);
	acpigen_write_processor(ctx, cpuindex, pblock_addr, pblock_len);
	acpigen_pop_len(ctx);

	ut_asserteq(EXT_OP_PREFIX, *ptr++);
	ut_asserteq(PROCESSOR_OP, *ptr++);
	ut_asserteq(0x13, acpi_test_get_length(ptr));
	ptr += 3;
	ut_asserteq_strn("\\._PR_CP06", (char *)ptr);
	ptr += 10;
	ut_asserteq(cpuindex, *ptr++);
	ut_asserteq(pblock_addr, get_unaligned((u32 *)ptr));
	ptr += 4;
	ut_asserteq(pblock_len, *ptr++);

	ut_asserteq_ptr(ptr, ctx->current);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_write_processor, 0);

/* Test emitting a processor package */
static int dm_test_acpi_write_processor_package(struct unit_test_state *uts)
{
	const int core_count = 3;
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);
	acpigen_write_processor_package(ctx, "XCPU", 0, core_count);

	ut_asserteq(NAME_OP, *ptr++);
	ut_asserteq_strn("XCPU", (char *)ptr);
	ptr += 4;
	ut_asserteq(PACKAGE_OP, *ptr++);
	ptr += 3;  /* skip length */
	ut_asserteq(core_count, *ptr++);

	ut_asserteq_strn("\\._PR_CP00", (char *)ptr);
	ptr += 10;
	ut_asserteq_strn("\\._PR_CP01", (char *)ptr);
	ptr += 10;
	ut_asserteq_strn("\\._PR_CP02", (char *)ptr);
	ptr += 10;

	ut_asserteq_ptr(ptr, ctx->current);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_write_processor_package, 0);

/* Test emitting a processor notification package */
static int dm_test_acpi_write_processor_cnot(struct unit_test_state *uts)
{
	const int core_count = 3;
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);
	acpigen_write_processor_cnot(ctx, core_count);

	ut_asserteq(METHOD_OP, *ptr++);
	ptr += 3;  /* skip length */
	ut_asserteq_strn("\\._PR_CNOT", (char *)ptr);
	ptr += 10;
	ut_asserteq(1, *ptr++);

	ut_asserteq(NOTIFY_OP, *ptr++);
	ut_asserteq_strn("\\._PR_CP00", (char *)ptr);
	ptr += 10;
	ut_asserteq(ARG0_OP, *ptr++);
	ut_asserteq(NOTIFY_OP, *ptr++);
	ut_asserteq_strn("\\._PR_CP01", (char *)ptr);
	ptr += 10;
	ut_asserteq(ARG0_OP, *ptr++);
	ut_asserteq(NOTIFY_OP, *ptr++);
	ut_asserteq_strn("\\._PR_CP02", (char *)ptr);
	ptr += 10;
	ut_asserteq(ARG0_OP, *ptr++);

	ut_asserteq_ptr(ptr, ctx->current);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_write_processor_cnot, 0);

/* Test acpigen_write_tpc */
static int dm_test_acpi_write_tpc(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);
	acpigen_write_tpc(ctx, "\\TLVL");

	ut_asserteq(METHOD_OP, *ptr++);
	ptr += 3;  /* skip length */
	ut_asserteq_strn("_TPC", (char *)ptr);
	ptr += 4;
	ut_asserteq(0, *ptr++);
	ut_asserteq(RETURN_OP, *ptr++);
	ut_asserteq_strn("\\TLVL", (char *)ptr);
	ptr += 5;

	ut_asserteq_ptr(ptr, ctx->current);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_write_tpc, 0);

/* Test acpigen_write_pss_package(), etc. */
static int dm_test_acpi_write_pss_psd(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);
	acpigen_write_pss_package(ctx, 1, 2, 3, 4, 5, 6);
	ut_asserteq(PACKAGE_OP, *ptr++);
	ptr += 3;  /* skip length */
	ut_asserteq(6, *ptr++);

	ut_asserteq(DWORD_PREFIX, *ptr++);
	ut_asserteq(1, get_unaligned((u32 *)ptr));
	ptr += 5;

	ut_asserteq(2, get_unaligned((u32 *)ptr));
	ptr += 5;

	ut_asserteq(3, get_unaligned((u32 *)ptr));
	ptr += 5;

	ut_asserteq(4, get_unaligned((u32 *)ptr));
	ptr += 5;

	ut_asserteq(5, get_unaligned((u32 *)ptr));
	ptr += 5;

	ut_asserteq(6, get_unaligned((u32 *)ptr));
	ptr += 4;

	acpigen_write_psd_package(ctx, 6, 7, HW_ALL);
	ut_asserteq(NAME_OP, *ptr++);
	ut_asserteq_strn("_PSD", (char *)ptr);
	ptr += 4;
	ut_asserteq(PACKAGE_OP, *ptr++);
	ptr += 3;  /* skip length */
	ut_asserteq(1, *ptr++);
	ut_asserteq(PACKAGE_OP, *ptr++);
	ptr += 3;  /* skip length */
	ut_asserteq(5, *ptr++);

	ut_asserteq(BYTE_PREFIX, *ptr++);
	ut_asserteq(5, *ptr++);
	ut_asserteq(BYTE_PREFIX, *ptr++);
	ut_asserteq(0, *ptr++);

	ut_asserteq(DWORD_PREFIX, *ptr++);
	ut_asserteq(6, get_unaligned((u32 *)ptr));
	ptr += 5;

	ut_asserteq(HW_ALL, get_unaligned((u32 *)ptr));
	ptr += 5;

	ut_asserteq(7, get_unaligned((u32 *)ptr));
	ptr += 4;

	ut_asserteq_ptr(ptr, ctx->current);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_write_pss_psd, 0);

/* Test acpi_write_cst_package() */
static int dm_test_acpi_write_cst(struct unit_test_state *uts)
{
	static struct acpi_cstate cstate_map[] = {
		{
			/* C1 */
			.ctype = 1,		/* ACPI C1 */
			.latency = 1,
			.power = 1000,
			.resource = {
				.space_id = ACPI_ADDRESS_SPACE_FIXED,
			},
		}, {
			.ctype = 2,		/* ACPI C2 */
			.latency = 50,
			.power = 10,
			.resource = {
				.space_id = ACPI_ADDRESS_SPACE_IO,
				.bit_width = 8,
				.addrl = 0x415,
			},
		},
	};
	int nentries = ARRAY_SIZE(cstate_map);
	struct acpi_ctx *ctx;
	u8 *ptr;
	int i;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);
	acpigen_write_cst_package(ctx, cstate_map, nentries);

	ut_asserteq(NAME_OP, *ptr++);
	ut_asserteq_strn("_CST", (char *)ptr);
	ptr += 4;
	ut_asserteq(PACKAGE_OP, *ptr++);
	ptr += 3;  /* skip length */
	ut_asserteq(nentries + 1, *ptr++);
	ut_asserteq(DWORD_PREFIX, *ptr++);
	ut_asserteq(nentries, get_unaligned((u32 *)ptr));
	ptr += 4;

	for (i = 0; i < nentries; i++) {
		ut_asserteq(PACKAGE_OP, *ptr++);
		ptr += 3;  /* skip length */
		ut_asserteq(4, *ptr++);
		ut_asserteq(BUFFER_OP, *ptr++);
		ptr += 0x17;
		ut_asserteq(DWORD_PREFIX, *ptr++);
		ut_asserteq(cstate_map[i].ctype, get_unaligned((u32 *)ptr));
		ptr += 5;
		ut_asserteq(cstate_map[i].latency, get_unaligned((u32 *)ptr));
		ptr += 5;
		ut_asserteq(cstate_map[i].power, get_unaligned((u32 *)ptr));
		ptr += 4;
	}

	ut_asserteq_ptr(ptr, ctx->current);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_write_cst, 0);

/* Test acpi_write_cst_package() */
static int dm_test_acpi_write_csd(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);
	acpigen_write_csd_package(ctx, 12, 34, CSD_HW_ALL, 56);

	ut_asserteq(NAME_OP, *ptr++);
	ut_asserteq_strn("_CSD", (char *)ptr);
	ptr += 4;
	ut_asserteq(PACKAGE_OP, *ptr++);
	ptr += 3;  /* skip length */
	ut_asserteq(1, *ptr++);
	ut_asserteq(PACKAGE_OP, *ptr++);
	ptr += 3;  /* skip length */
	ut_asserteq(6, *ptr++);

	ut_asserteq(BYTE_PREFIX, *ptr++);
	ut_asserteq(6, *ptr++);
	ut_asserteq(BYTE_PREFIX, *ptr++);
	ut_asserteq(0, *ptr++);
	ut_asserteq(DWORD_PREFIX, *ptr++);
	ut_asserteq(12, get_unaligned((u32 *)ptr));
	ptr += 5;
	ut_asserteq(CSD_HW_ALL, get_unaligned((u32 *)ptr));
	ptr += 5;
	ut_asserteq(34, get_unaligned((u32 *)ptr));
	ptr += 5;
	ut_asserteq(56, get_unaligned((u32 *)ptr));
	ptr += 4;

	ut_asserteq_ptr(ptr, ctx->current);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_write_csd, 0);

/* Test acpigen_write_tss_package() */
static int dm_test_acpi_write_tss(struct unit_test_state *uts)
{
	static struct acpi_tstate tstate_list[] = {
		{ 1, 2, 3, 4, 5, },
		{ 6, 7, 8, 9, 10, },
	};
	int nentries = ARRAY_SIZE(tstate_list);
	struct acpi_ctx *ctx;
	u8 *ptr;
	int i;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);
	acpigen_write_tss_package(ctx, tstate_list, nentries);

	ut_asserteq(NAME_OP, *ptr++);
	ut_asserteq_strn("_TSS", (char *)ptr);
	ptr += 4;
	ut_asserteq(PACKAGE_OP, *ptr++);
	ptr += 3;  /* skip length */
	ut_asserteq(nentries, *ptr++);

	for (i = 0; i < nentries; i++) {
		ut_asserteq(PACKAGE_OP, *ptr++);
		ptr += 3;  /* skip length */
		ut_asserteq(5, *ptr++);
		ut_asserteq(DWORD_PREFIX, *ptr++);
		ut_asserteq(tstate_list[i].percent, get_unaligned((u32 *)ptr));
		ptr += 5;
		ut_asserteq(tstate_list[i].power, get_unaligned((u32 *)ptr));
		ptr += 5;
		ut_asserteq(tstate_list[i].latency, get_unaligned((u32 *)ptr));
		ptr += 5;
		ut_asserteq(tstate_list[i].control, get_unaligned((u32 *)ptr));
		ptr += 5;
		ut_asserteq(tstate_list[i].status, get_unaligned((u32 *)ptr));
		ptr += 4;
	}

	ut_asserteq_ptr(ptr, ctx->current);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_write_tss, 0);

/* Test acpigen_write_tsd_package() */
static int dm_test_acpi_write_tsd_package(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);
	acpigen_write_tsd_package(ctx, 12, 34, HW_ALL);

	ut_asserteq(NAME_OP, *ptr++);
	ut_asserteq_strn("_TSD", (char *)ptr);
	ptr += 4;
	ut_asserteq(PACKAGE_OP, *ptr++);
	ptr += 3;  /* skip length */
	ut_asserteq(1, *ptr++);
	ut_asserteq(PACKAGE_OP, *ptr++);
	ptr += 3;  /* skip length */
	ut_asserteq(5, *ptr++);

	ut_asserteq(BYTE_PREFIX, *ptr++);
	ut_asserteq(5, *ptr++);
	ut_asserteq(BYTE_PREFIX, *ptr++);
	ut_asserteq(0, *ptr++);
	ut_asserteq(DWORD_PREFIX, *ptr++);
	ut_asserteq(12, get_unaligned((u32 *)ptr));
	ptr += 5;
	ut_asserteq(CSD_HW_ALL, get_unaligned((u32 *)ptr));
	ptr += 5;
	ut_asserteq(34, get_unaligned((u32 *)ptr));
	ptr += 4;

	ut_asserteq_ptr(ptr, ctx->current);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_write_tsd_package, 0);
