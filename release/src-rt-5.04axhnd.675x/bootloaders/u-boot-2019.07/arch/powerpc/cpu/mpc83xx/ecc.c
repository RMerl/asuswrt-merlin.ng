// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2007-2011 Freescale Semiconductor, Inc.
 *
 * Dave Liu <daveliu@freescale.com>
 * based on the contribution of Marian Balakowicz <m8@semihalf.com>
 */

#include <common.h>
#include <mpc83xx.h>
#include <command.h>

#if defined(CONFIG_DDR_ECC) && defined(CONFIG_DDR_ECC_CMD)
void ecc_print_status(void)
{
	immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
#ifdef CONFIG_SYS_FSL_DDR2
	struct ccsr_ddr __iomem *ddr = &immap->ddr;
#else
	ddr83xx_t *ddr = &immap->ddr;
#endif

	printf("\nECC mode: %s\n\n",
	       (ddr->sdram_cfg & SDRAM_CFG_ECC_EN) ? "ON" : "OFF");

	/* Interrupts */
	printf("Memory Error Interrupt Enable:\n");
	printf("  Multiple-Bit Error Interrupt Enable: %d\n",
	       (ddr->err_int_en & ECC_ERR_INT_EN_MBEE) ? 1 : 0);
	printf("  Single-Bit Error Interrupt Enable: %d\n",
	       (ddr->err_int_en & ECC_ERR_INT_EN_SBEE) ? 1 : 0);
	printf("  Memory Select Error Interrupt Enable: %d\n\n",
	       (ddr->err_int_en & ECC_ERR_INT_EN_MSEE) ? 1 : 0);

	/* Error disable */
	printf("Memory Error Disable:\n");
	printf("  Multiple-Bit Error Disable: %d\n",
	       (ddr->err_disable & ECC_ERROR_DISABLE_MBED) ? 1 : 0);
	printf("  Single-Bit Error Disable: %d\n",
	       (ddr->err_disable & ECC_ERROR_DISABLE_SBED) ? 1 : 0);
	printf("  Memory Select Error Disable: %d\n\n",
	       (ddr->err_disable & ECC_ERROR_DISABLE_MSED) ? 1 : 0);

	/* Error injection */
	printf("Memory Data Path Error Injection Mask High/Low: %08x %08x\n",
	       ddr->data_err_inject_hi, ddr->data_err_inject_lo);

	printf("Memory Data Path Error Injection Mask ECC:\n");
	printf("  ECC Mirror Byte: %d\n",
	       (ddr->ecc_err_inject & ECC_ERR_INJECT_EMB) ? 1 : 0);
	printf("  ECC Injection Enable: %d\n",
	       (ddr->ecc_err_inject & ECC_ERR_INJECT_EIEN) ? 1 : 0);
	printf("  ECC Error Injection Mask: 0x%02x\n\n",
	       ddr->ecc_err_inject & ECC_ERR_INJECT_EEIM);

	/* SBE counter/threshold */
	printf("Memory Single-Bit Error Management (0..255):\n");
	printf("  Single-Bit Error Threshold: %d\n",
	       (ddr->err_sbe & ECC_ERROR_MAN_SBET) >> ECC_ERROR_MAN_SBET_SHIFT);
	printf("  Single-Bit Error Counter: %d\n\n",
	       (ddr->err_sbe & ECC_ERROR_MAN_SBEC) >> ECC_ERROR_MAN_SBEC_SHIFT);

	/* Error detect */
	printf("Memory Error Detect:\n");
	printf("  Multiple Memory Errors: %d\n",
	       (ddr->err_detect & ECC_ERROR_DETECT_MME) ? 1 : 0);
	printf("  Multiple-Bit Error: %d\n",
	       (ddr->err_detect & ECC_ERROR_DETECT_MBE) ? 1 : 0);
	printf("  Single-Bit Error: %d\n",
	       (ddr->err_detect & ECC_ERROR_DETECT_SBE) ? 1 : 0);
	printf("  Memory Select Error: %d\n\n",
	       (ddr->err_detect & ECC_ERROR_DETECT_MSE) ? 1 : 0);

	/* Capture data */
	printf("Memory Error Address Capture: 0x%08x\n", ddr->capture_address);
	printf("Memory Data Path Read Capture High/Low: %08x %08x\n",
	       ddr->capture_data_hi, ddr->capture_data_lo);
	printf("Memory Data Path Read Capture ECC: 0x%02x\n\n",
	       ddr->capture_ecc & CAPTURE_ECC_ECE);

	printf("Memory Error Attributes Capture:\n");
	printf(" Data Beat Number: %d\n",
	       (ddr->capture_attributes & ECC_CAPT_ATTR_BNUM) >>
	       ECC_CAPT_ATTR_BNUM_SHIFT);
	printf("  Transaction Size: %d\n",
	       (ddr->capture_attributes & ECC_CAPT_ATTR_TSIZ) >>
	       ECC_CAPT_ATTR_TSIZ_SHIFT);
	printf("  Transaction Source: %d\n",
	       (ddr->capture_attributes & ECC_CAPT_ATTR_TSRC) >>
	       ECC_CAPT_ATTR_TSRC_SHIFT);
	printf("  Transaction Type: %d\n",
	       (ddr->capture_attributes & ECC_CAPT_ATTR_TTYP) >>
	       ECC_CAPT_ATTR_TTYP_SHIFT);
	printf("  Error Information Valid: %d\n\n",
	       ddr->capture_attributes & ECC_CAPT_ATTR_VLD);
}

int do_ecc(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
#ifdef CONFIG_SYS_FSL_DDR2
	struct ccsr_ddr __iomem *ddr = &immap->ddr;
#else
	ddr83xx_t *ddr = &immap->ddr;
#endif
	volatile u32 val;
	u64 *addr;
	u32 count;
	register u64 *i;
	u32 ret[2];
	u32 pattern[2];
	u32 writeback[2];

	/* The pattern is written into memory to generate error */
	pattern[0] = 0xfedcba98UL;
	pattern[1] = 0x76543210UL;

	/* After injecting error, re-initialize the memory with the value */
	writeback[0] = 0x01234567UL;
	writeback[1] = 0x89abcdefUL;

	if (argc > 4)
		return cmd_usage(cmdtp);

	if (argc == 2) {
		if (strcmp(argv[1], "status") == 0) {
			ecc_print_status();
			return 0;
		} else if (strcmp(argv[1], "captureclear") == 0) {
			ddr->capture_address = 0;
			ddr->capture_data_hi = 0;
			ddr->capture_data_lo = 0;
			ddr->capture_ecc = 0;
			ddr->capture_attributes = 0;
			return 0;
		}
	}
	if (argc == 3) {
		if (strcmp(argv[1], "sbecnt") == 0) {
			val = simple_strtoul(argv[2], NULL, 10);
			if (val > 255) {
				printf("Incorrect Counter value, "
				       "should be 0..255\n");
				return 1;
			}

			val = (val << ECC_ERROR_MAN_SBEC_SHIFT);
			val |= (ddr->err_sbe & ECC_ERROR_MAN_SBET);

			ddr->err_sbe = val;
			return 0;
		} else if (strcmp(argv[1], "sbethr") == 0) {
			val = simple_strtoul(argv[2], NULL, 10);
			if (val > 255) {
				printf("Incorrect Counter value, "
				       "should be 0..255\n");
				return 1;
			}

			val = (val << ECC_ERROR_MAN_SBET_SHIFT);
			val |= (ddr->err_sbe & ECC_ERROR_MAN_SBEC);

			ddr->err_sbe = val;
			return 0;
		} else if (strcmp(argv[1], "errdisable") == 0) {
			val = ddr->err_disable;

			if (strcmp(argv[2], "+sbe") == 0) {
				val |= ECC_ERROR_DISABLE_SBED;
			} else if (strcmp(argv[2], "+mbe") == 0) {
				val |= ECC_ERROR_DISABLE_MBED;
			} else if (strcmp(argv[2], "+mse") == 0) {
				val |= ECC_ERROR_DISABLE_MSED;
			} else if (strcmp(argv[2], "+all") == 0) {
				val |= (ECC_ERROR_DISABLE_SBED |
					ECC_ERROR_DISABLE_MBED |
					ECC_ERROR_DISABLE_MSED);
			} else if (strcmp(argv[2], "-sbe") == 0) {
				val &= ~ECC_ERROR_DISABLE_SBED;
			} else if (strcmp(argv[2], "-mbe") == 0) {
				val &= ~ECC_ERROR_DISABLE_MBED;
			} else if (strcmp(argv[2], "-mse") == 0) {
				val &= ~ECC_ERROR_DISABLE_MSED;
			} else if (strcmp(argv[2], "-all") == 0) {
				val &= ~(ECC_ERROR_DISABLE_SBED |
					 ECC_ERROR_DISABLE_MBED |
					 ECC_ERROR_DISABLE_MSED);
			} else {
				printf("Incorrect err_disable field\n");
				return 1;
			}

			ddr->err_disable = val;
			sync();
			isync();
			return 0;
		} else if (strcmp(argv[1], "errdetectclr") == 0) {
			val = ddr->err_detect;

			if (strcmp(argv[2], "mme") == 0) {
				val |= ECC_ERROR_DETECT_MME;
			} else if (strcmp(argv[2], "sbe") == 0) {
				val |= ECC_ERROR_DETECT_SBE;
			} else if (strcmp(argv[2], "mbe") == 0) {
				val |= ECC_ERROR_DETECT_MBE;
			} else if (strcmp(argv[2], "mse") == 0) {
				val |= ECC_ERROR_DETECT_MSE;
			} else if (strcmp(argv[2], "all") == 0) {
				val |= (ECC_ERROR_DETECT_MME |
					ECC_ERROR_DETECT_MBE |
					ECC_ERROR_DETECT_SBE |
					ECC_ERROR_DETECT_MSE);
			} else {
				printf("Incorrect err_detect field\n");
				return 1;
			}

			ddr->err_detect = val;
			return 0;
		} else if (strcmp(argv[1], "injectdatahi") == 0) {
			val = simple_strtoul(argv[2], NULL, 16);

			ddr->data_err_inject_hi = val;
			return 0;
		} else if (strcmp(argv[1], "injectdatalo") == 0) {
			val = simple_strtoul(argv[2], NULL, 16);

			ddr->data_err_inject_lo = val;
			return 0;
		} else if (strcmp(argv[1], "injectecc") == 0) {
			val = simple_strtoul(argv[2], NULL, 16);
			if (val > 0xff) {
				printf("Incorrect ECC inject mask, "
				       "should be 0x00..0xff\n");
				return 1;
			}
			val |= (ddr->ecc_err_inject & ~ECC_ERR_INJECT_EEIM);

			ddr->ecc_err_inject = val;
			return 0;
		} else if (strcmp(argv[1], "inject") == 0) {
			val = ddr->ecc_err_inject;

			if (strcmp(argv[2], "en") == 0)
				val |= ECC_ERR_INJECT_EIEN;
			else if (strcmp(argv[2], "dis") == 0)
				val &= ~ECC_ERR_INJECT_EIEN;
			else
				printf("Incorrect command\n");

			ddr->ecc_err_inject = val;
			sync();
			isync();
			return 0;
		} else if (strcmp(argv[1], "mirror") == 0) {
			val = ddr->ecc_err_inject;

			if (strcmp(argv[2], "en") == 0)
				val |= ECC_ERR_INJECT_EMB;
			else if (strcmp(argv[2], "dis") == 0)
				val &= ~ECC_ERR_INJECT_EMB;
			else
				printf("Incorrect command\n");

			ddr->ecc_err_inject = val;
			return 0;
		}
	}
	if (argc == 4) {
		if (strcmp(argv[1], "testdw") == 0) {
			addr = (u64 *) simple_strtoul(argv[2], NULL, 16);
			count = simple_strtoul(argv[3], NULL, 16);

			if ((u32) addr % 8) {
				printf("Address not aligned on "
				       "double word boundary\n");
				return 1;
			}
			disable_interrupts();

			for (i = addr; i < addr + count; i++) {

				/* enable injects */
				ddr->ecc_err_inject |= ECC_ERR_INJECT_EIEN;
				sync();
				isync();

				/* write memory location injecting errors */
				ppcDWstore((u32 *) i, pattern);
				sync();

				/* disable injects */
				ddr->ecc_err_inject &= ~ECC_ERR_INJECT_EIEN;
				sync();
				isync();

				/* read data, this generates ECC error */
				ppcDWload((u32 *) i, ret);
				sync();

				/* re-initialize memory, double word write the location again,
				 * generates new ECC code this time */
				ppcDWstore((u32 *) i, writeback);
				sync();
			}
			enable_interrupts();
			return 0;
		}
		if (strcmp(argv[1], "testword") == 0) {
			addr = (u64 *) simple_strtoul(argv[2], NULL, 16);
			count = simple_strtoul(argv[3], NULL, 16);

			if ((u32) addr % 8) {
				printf("Address not aligned on "
				       "double word boundary\n");
				return 1;
			}
			disable_interrupts();

			for (i = addr; i < addr + count; i++) {

				/* enable injects */
				ddr->ecc_err_inject |= ECC_ERR_INJECT_EIEN;
				sync();
				isync();

				/* write memory location injecting errors */
				*(u32 *) i = 0xfedcba98UL;
				sync();

				/* sub double word write,
				 * bus will read-modify-write,
				 * generates ECC error */
				*((u32 *) i + 1) = 0x76543210UL;
				sync();

				/* disable injects */
				ddr->ecc_err_inject &= ~ECC_ERR_INJECT_EIEN;
				sync();
				isync();

				/* re-initialize memory,
				 * double word write the location again,
				 * generates new ECC code this time */
				ppcDWstore((u32 *) i, writeback);
				sync();
			}
			enable_interrupts();
			return 0;
		}
	}
	return cmd_usage(cmdtp);
}

U_BOOT_CMD(ecc, 4, 0, do_ecc,
	   "support for DDR ECC features",
	   "status              - print out status info\n"
	   "ecc captureclear        - clear capture regs data\n"
	   "ecc sbecnt <val>        - set Single-Bit Error counter\n"
	   "ecc sbethr <val>        - set Single-Bit Threshold\n"
	   "ecc errdisable <flag>   - clear/set disable Memory Error Disable, flag:\n"
	   "  [-|+]sbe - Single-Bit Error\n"
	   "  [-|+]mbe - Multiple-Bit Error\n"
	   "  [-|+]mse - Memory Select Error\n"
	   "  [-|+]all - all errors\n"
	   "ecc errdetectclr <flag> - clear Memory Error Detect, flag:\n"
	   "  mme - Multiple Memory Errors\n"
	   "  sbe - Single-Bit Error\n"
	   "  mbe - Multiple-Bit Error\n"
	   "  mse - Memory Select Error\n"
	   "  all - all errors\n"
	   "ecc injectdatahi <hi>  - set Memory Data Path Error Injection Mask High\n"
	   "ecc injectdatalo <lo>  - set Memory Data Path Error Injection Mask Low\n"
	   "ecc injectecc <ecc>    - set ECC Error Injection Mask\n"
	   "ecc inject <en|dis>    - enable/disable error injection\n"
	   "ecc mirror <en|dis>    - enable/disable mirror byte\n"
	   "ecc testdw <addr> <cnt>  - test mem region with double word access:\n"
	   "  - enables injects\n"
	   "  - writes pattern injecting errors with double word access\n"
	   "  - disables injects\n"
	   "  - reads pattern back with double word access, generates error\n"
	   "  - re-inits memory\n"
	   "ecc testword <addr> <cnt>  - test mem region with word access:\n"
	   "  - enables injects\n"
	   "  - writes pattern injecting errors with word access\n"
	   "  - writes pattern with word access, generates error\n"
	   "  - disables injects\n" "  - re-inits memory");
#endif
