// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Google LLC
 */

#include <common.h>
#include <acpi/acpigen.h>
#include <acpi/acpi_table.h>
#include <asm/acpigen.h>

void acpigen_write_empty_pct(struct acpi_ctx *ctx)
{
	/*
	 * Name (_PCT, Package (0x02)
	 * {
	 *	ResourceTemplate ()
	 *	{
	 *		Register (FFixedHW,
	 *			0x00,               // Bit Width
	 *			0x00,               // Bit Offset
	 *			0x0000000000000000, // Address
	 *			,)
	 *	},
	 *
	 *	ResourceTemplate ()
	 *	{
	 *		Register (FFixedHW,
	 *			0x00,               // Bit Width
	 *			0x00,               // Bit Offset
	 *			0x0000000000000000, // Address
	 *			,)
	 *	}
	 * })
	 */
	static char stream[] = {
		/* 00000030    "0._PCT.," */
		0x08, 0x5f, 0x50, 0x43, 0x54, 0x12, 0x2c,
		/* 00000038    "........" */
		0x02, 0x11, 0x14, 0x0a, 0x11, 0x82, 0x0c, 0x00,
		/* 00000040    "........" */
		0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		/* 00000048    "....y..." */
		0x00, 0x00, 0x00, 0x00, 0x79, 0x00, 0x11, 0x14,
		/* 00000050    "........" */
		0x0a, 0x11, 0x82, 0x0c, 0x00, 0x7f, 0x00, 0x00,
		/* 00000058    "........" */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x79, 0x00
	};
	acpigen_emit_stream(ctx, stream, ARRAY_SIZE(stream));
}

void acpigen_write_empty_ptc(struct acpi_ctx *ctx)
{
	/*
	 * Name (_PTC, Package (0x02)
	 * {
	 *	ResourceTemplate ()
	 *	{
	 *		Register (FFixedHW,
	 *			0x00,               // Bit Width
	 *			0x00,               // Bit Offset
	 *			0x0000000000000000, // Address
	 *			,)
	 *	},
	 *
	 *	ResourceTemplate ()
	 *	{
	 *		Register (FFixedHW,
	 *			0x00,               // Bit Width
	 *			0x00,               // Bit Offset
	 *			0x0000000000000000, // Address
	 *			,)
	 *	}
	 * })
	 */
	struct acpi_gen_regaddr addr = {
		.space_id    = ACPI_ADDRESS_SPACE_FIXED,
		.bit_width   = 0,
		.bit_offset  = 0,
		.access_size = 0,
		.addrl       = 0,
		.addrh       = 0,
	};

	acpigen_write_name(ctx, "_PTC");
	acpigen_write_package(ctx, 2);

	/* ControlRegister */
	acpigen_write_register_resource(ctx, &addr);

	/* StatusRegister */
	acpigen_write_register_resource(ctx, &addr);

	acpigen_pop_len(ctx);
}
