/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019
 * Author(s): Giulio Benetti <giulio.benetti@benettiengineering.com>
 */

#ifndef DT_BINDINGS_IMXRT_SDRAM_H
#define DT_BINDINGS_IMXRT_SDRAM_H

#define MEM_SIZE_4K		0x00
#define MEM_SIZE_8K		0x01
#define MEM_SIZE_16K		0x02
#define MEM_SIZE_32K		0x03
#define MEM_SIZE_64K		0x04
#define MEM_SIZE_128K		0x05
#define MEM_SIZE_256K		0x06
#define MEM_SIZE_512K		0x07
#define MEM_SIZE_1M		0x08
#define MEM_SIZE_2M		0x09
#define MEM_SIZE_4M		0x0A
#define MEM_SIZE_8M		0x0B
#define MEM_SIZE_16M		0x0C
#define MEM_SIZE_32M		0x0D
#define MEM_SIZE_64M		0x0E
#define MEM_SIZE_128M		0x0F
#define MEM_SIZE_256M		0x10
#define MEM_SIZE_512M		0x11
#define MEM_SIZE_1G		0x12
#define MEM_SIZE_2G		0x13
#define MEM_SIZE_4G		0x14

#define MUX_A8_SDRAM_A8		0x0
#define MUX_A8_NAND_CE		0x1
#define MUX_A8_NOR_CE		0x2
#define MUX_A8_PSRAM_CE		0x3
#define MUX_A8_DBI_CSX		0x4

#define MUX_CSX0_NOR_PSRAM_A24	0x0
#define MUX_CSX0_SDRAM_CS1	0x1
#define MUX_CSX0_SDRAM_CS2	0x2
#define MUX_CSX0_SDRAM_CS3	0x3
#define MUX_CSX0_NAND_CE	0x4
#define MUX_CSX0_NOR_CE		0x5
#define MUX_CSX0_PSRAM_CE	0x6
#define MUX_CSX0_DBI_CSX	0x7

#define MUX_CSX1_NOR_PSRAM_A25	0x0
#define MUX_CSX1_SDRAM_CS1	0x1
#define MUX_CSX1_SDRAM_CS2	0x2
#define MUX_CSX1_SDRAM_CS3	0x3
#define MUX_CSX1_NAND_CE	0x4
#define MUX_CSX1_NOR_CE		0x5
#define MUX_CSX1_PSRAM_CE	0x6
#define MUX_CSX1_DBI_CSX	0x7

#define MUX_CSX2_NOR_PSRAM_A26	0x0
#define MUX_CSX2_SDRAM_CS1	0x1
#define MUX_CSX2_SDRAM_CS2	0x2
#define MUX_CSX2_SDRAM_CS3	0x3
#define MUX_CSX2_NAND_CE	0x4
#define MUX_CSX2_NOR_CE		0x5
#define MUX_CSX2_PSRAM_CE	0x6
#define MUX_CSX2_DBI_CSX	0x7

#define MUX_CSX3_NOR_PSRAM_A27	0x0
#define MUX_CSX3_SDRAM_CS1	0x1
#define MUX_CSX3_SDRAM_CS2	0x2
#define MUX_CSX3_SDRAM_CS3	0x3
#define MUX_CSX3_NAND_CE	0x4
#define MUX_CSX3_NOR_CE		0x5
#define MUX_CSX3_PSRAM_CE	0x6
#define MUX_CSX3_DBI_CSX	0x7

#define MUX_RDY_NAND_RDY_WAIT	0x0
#define MUX_RDY_SDRAM_CS1	0x1
#define MUX_RDY_SDRAM_CS2	0x2
#define MUX_RDY_SDRAM_CS3	0x3
#define MUX_RDY_NOR_CE		0x4
#define MUX_RDY_PSRAM_CE	0x5
#define MUX_RDY_DBI_CSX		0x6
#define MUX_RDY_NOR_PSRAM_A27	0x7

#define MEM_WIDTH_8BITS		0x0
#define MEM_WIDTH_16BITS	0x1
#define MEM_WIDTH_32BITS	0x2

#define BL_1			0x0
#define BL_2			0x1
#define BL_4			0x2
#define BL_8			0x3

#define COL_12BITS		0x0
#define COL_11BITS		0x1
#define COL_10BITS		0x2
#define COL_9BITS		0x3

#define CL_1			0x0
#define CL_2			0x2
#define CL_3			0x3

#endif
