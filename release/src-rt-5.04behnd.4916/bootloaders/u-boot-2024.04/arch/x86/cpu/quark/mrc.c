// SPDX-License-Identifier: Intel
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * Ported from Intel released Quark UEFI BIOS
 * QuarkSocPkg/QuarkNorthCluster/MemoryInit/Pei
 */

/*
 * This is the main Quark Memory Reference Code (MRC)
 *
 * These functions are generic and should work for any Quark-based board.
 *
 * MRC requires two data structures to be passed in which are initialized by
 * mrc_adjust_params().
 *
 * The basic flow is as follows:
 * 01) Check for supported DDR speed configuration
 * 02) Set up Memory Manager buffer as pass-through (POR)
 * 03) Set Channel Interleaving Mode and Channel Stride to the most aggressive
 *     setting possible
 * 04) Set up the Memory Controller logic
 * 05) Set up the DDR_PHY logic
 * 06) Initialise the DRAMs (JEDEC)
 * 07) Perform the Receive Enable Calibration algorithm
 * 08) Perform the Write Leveling algorithm
 * 09) Perform the Read Training algorithm (includes internal Vref)
 * 10) Perform the Write Training algorithm
 * 11) Set Channel Interleaving Mode and Channel Stride to the desired settings
 *
 * DRAM unit configuration based on Valleyview MRC.
 */

#include <common.h>
#include <asm/arch/mrc.h>
#include <asm/arch/msg_port.h>
#include "mrc_util.h"
#include "smc.h"

static const struct mem_init init[] = {
	{ 0x0101, BM_COLD | BM_FAST | BM_WARM | BM_S3, clear_self_refresh       },
	{ 0x0200, BM_COLD | BM_FAST | BM_WARM | BM_S3, prog_ddr_timing_control  },
	{ 0x0103, BM_COLD | BM_FAST                  , prog_decode_before_jedec },
	{ 0x0104, BM_COLD | BM_FAST                  , perform_ddr_reset        },
	{ 0x0300, BM_COLD | BM_FAST           | BM_S3, ddrphy_init              },
	{ 0x0400, BM_COLD | BM_FAST                  , perform_jedec_init       },
	{ 0x0105, BM_COLD | BM_FAST                  , set_ddr_init_complete    },
	{ 0x0106,           BM_FAST | BM_WARM | BM_S3, restore_timings          },
	{ 0x0106, BM_COLD                            , default_timings          },
	{ 0x0500, BM_COLD                            , rcvn_cal                 },
	{ 0x0600, BM_COLD                            , wr_level                 },
	{ 0x0120, BM_COLD                            , prog_page_ctrl           },
	{ 0x0700, BM_COLD                            , rd_train                 },
	{ 0x0800, BM_COLD                            , wr_train                 },
	{ 0x010b, BM_COLD                            , store_timings            },
	{ 0x010c, BM_COLD | BM_FAST | BM_WARM | BM_S3, enable_scrambling        },
	{ 0x010d, BM_COLD | BM_FAST | BM_WARM | BM_S3, prog_ddr_control         },
	{ 0x010e, BM_COLD | BM_FAST | BM_WARM | BM_S3, prog_dra_drb             },
	{ 0x010f,                     BM_WARM | BM_S3, perform_wake             },
	{ 0x0110, BM_COLD | BM_FAST | BM_WARM | BM_S3, change_refresh_period    },
	{ 0x0111, BM_COLD | BM_FAST | BM_WARM | BM_S3, set_auto_refresh         },
	{ 0x0112, BM_COLD | BM_FAST | BM_WARM | BM_S3, ecc_enable               },
	{ 0x0113, BM_COLD | BM_FAST                  , memory_test              },
	{ 0x0114, BM_COLD | BM_FAST | BM_WARM | BM_S3, lock_registers           }
};

/* Adjust configuration parameters before initialization sequence */
static void mrc_adjust_params(struct mrc_params *mrc_params)
{
	const struct dram_params *dram_params;
	uint8_t dram_width;
	uint32_t rank_enables;
	uint32_t channel_width;

	ENTERFN();

	/* initially expect success */
	mrc_params->status = MRC_SUCCESS;

	dram_width = mrc_params->dram_width;
	rank_enables = mrc_params->rank_enables;
	channel_width = mrc_params->channel_width;

	/*
	 * Setup board layout (must be reviewed as is selecting static timings)
	 * 0 == R0 (DDR3 x16), 1 == R1 (DDR3 x16),
	 * 2 == DV (DDR3 x8), 3 == SV (DDR3 x8).
	 */
	if (dram_width == X8)
		mrc_params->board_id = 2;	/* select x8 layout */
	else
		mrc_params->board_id = 0;	/* select x16 layout */

	/* initially no memory */
	mrc_params->mem_size = 0;

	/* begin of channel settings */
	dram_params = &mrc_params->params;

	/*
	 * Determine column bits:
	 *
	 * Column: 11 for 8Gbx8, else 10
	 */
	mrc_params->column_bits[0] =
		(dram_params[0].density == 4) &&
		(dram_width == X8) ? 11 : 10;

	/*
	 * Determine row bits:
	 *
	 * 512Mbx16=12 512Mbx8=13
	 * 1Gbx16=13   1Gbx8=14
	 * 2Gbx16=14   2Gbx8=15
	 * 4Gbx16=15   4Gbx8=16
	 * 8Gbx16=16   8Gbx8=16
	 */
	mrc_params->row_bits[0] = 12 + dram_params[0].density +
		(dram_params[0].density < 4) &&
		(dram_width == X8) ? 1 : 0;

	/*
	 * Determine per-channel memory size:
	 *
	 * (For 2 RANKs, multiply by 2)
	 * (For 16 bit data bus, divide by 2)
	 *
	 * DENSITY WIDTH MEM_AVAILABLE
	 * 512Mb   x16   0x008000000 ( 128MB)
	 * 512Mb   x8    0x010000000 ( 256MB)
	 * 1Gb     x16   0x010000000 ( 256MB)
	 * 1Gb     x8    0x020000000 ( 512MB)
	 * 2Gb     x16   0x020000000 ( 512MB)
	 * 2Gb     x8    0x040000000 (1024MB)
	 * 4Gb     x16   0x040000000 (1024MB)
	 * 4Gb     x8    0x080000000 (2048MB)
	 */
	mrc_params->channel_size[0] = 1 << dram_params[0].density;
	mrc_params->channel_size[0] *= (dram_width == X8) ? 2 : 1;
	mrc_params->channel_size[0] *= (rank_enables == 0x3) ? 2 : 1;
	mrc_params->channel_size[0] *= (channel_width == X16) ? 1 : 2;

	/* Determine memory size (convert number of 64MB/512Mb units) */
	mrc_params->mem_size += mrc_params->channel_size[0] << 26;

	LEAVEFN();
}

static void mrc_mem_init(struct mrc_params *mrc_params)
{
	int i;

	ENTERFN();

	/* MRC started */
	mrc_post_code(0x01, 0x00);

	if (mrc_params->boot_mode != BM_COLD) {
		if (mrc_params->ddr_speed != mrc_params->timings.ddr_speed) {
			/* full training required as frequency changed */
			mrc_params->boot_mode = BM_COLD;
		}
	}

	for (i = 0; i < ARRAY_SIZE(init); i++) {
		uint64_t my_tsc;

		if (mrc_params->boot_mode & init[i].boot_path) {
			uint8_t major = init[i].post_code >> 8 & 0xff;
			uint8_t minor = init[i].post_code >> 0 & 0xff;
			mrc_post_code(major, minor);

			my_tsc = rdtsc();
			init[i].init_fn(mrc_params);
			DPF(D_TIME, "Execution time %llx", rdtsc() - my_tsc);
		}
	}

	/* display the timings */
	print_timings(mrc_params);

	/* MRC complete */
	mrc_post_code(0x01, 0xff);

	LEAVEFN();
}

void mrc_init(struct mrc_params *mrc_params)
{
	ENTERFN();

	DPF(D_INFO, "MRC Version %04x\n", MRC_VERSION);

	/* Set up the data structures used by mrc_mem_init() */
	mrc_adjust_params(mrc_params);

	/* Initialize system memory */
	mrc_mem_init(mrc_params);

	LEAVEFN();
}
