// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2006-2007 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2006 Freescale Semiconductor, Inc.
 * (C) Copyright 2003 Motorola Inc.
 * Xianghua Xiao (X.Xiao@motorola.com)
 */

#ifndef CONFIG_MPC83XX_SDRAM

#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <i2c.h>
#include <spd.h>
#include <asm/mmu.h>
#include <spd_sdram.h>

DECLARE_GLOBAL_DATA_PTR;

void board_add_ram_info(int use_default)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile ddr83xx_t *ddr = &immap->ddr;
	char buf[32];

	printf(" (DDR%d", ((ddr->sdram_cfg & SDRAM_CFG_SDRAM_TYPE_MASK)
			   >> SDRAM_CFG_SDRAM_TYPE_SHIFT) - 1);

#if defined(CONFIG_ARCH_MPC8308) || defined(CONFIG_ARCH_MPC831X)
	if ((ddr->sdram_cfg & SDRAM_CFG_DBW_MASK) == SDRAM_CFG_DBW_16)
		puts(", 16-bit");
	else if ((ddr->sdram_cfg & SDRAM_CFG_DBW_MASK) == SDRAM_CFG_DBW_32)
		puts(", 32-bit");
	else
		puts(", unknown width");
#else
	if (ddr->sdram_cfg & SDRAM_CFG_32_BE)
		puts(", 32-bit");
	else
		puts(", 64-bit");
#endif

	if (ddr->sdram_cfg & SDRAM_CFG_ECC_EN)
		puts(", ECC on");
	else
		puts(", ECC off");

	printf(", %s MHz)", strmhz(buf, gd->mem_clk));

#if defined(CONFIG_SYS_LB_SDRAM) && defined(CONFIG_SYS_LBC_SDRAM_SIZE)
	puts("\nSDRAM: ");
	print_size (CONFIG_SYS_LBC_SDRAM_SIZE * 1024 * 1024, " (local bus)");
#endif
}

#ifdef CONFIG_SPD_EEPROM
#ifndef	CONFIG_SYS_READ_SPD
#define CONFIG_SYS_READ_SPD	i2c_read
#endif
#ifndef SPD_EEPROM_OFFSET
#define SPD_EEPROM_OFFSET	0
#endif
#ifndef SPD_EEPROM_ADDR_LEN
#define SPD_EEPROM_ADDR_LEN     1
#endif

/*
 * Convert picoseconds into clock cycles (rounding up if needed).
 */
int
picos_to_clk(int picos)
{
	unsigned int mem_bus_clk;
	int clks;

	mem_bus_clk = gd->mem_clk >> 1;
	clks = picos / (1000000000 / (mem_bus_clk / 1000));
	if (picos % (1000000000 / (mem_bus_clk / 1000)) != 0)
		clks++;

	return clks;
}

unsigned int banksize(unsigned char row_dens)
{
	return ((row_dens >> 2) | ((row_dens & 3) << 6)) << 24;
}

int read_spd(uint addr)
{
	return ((int) addr);
}

#undef SPD_DEBUG
#ifdef SPD_DEBUG
static void spd_debug(spd_eeprom_t *spd)
{
	printf ("\nDIMM type:       %-18.18s\n", spd->mpart);
	printf ("SPD size:        %d\n", spd->info_size);
	printf ("EEPROM size:     %d\n", 1 << spd->chip_size);
	printf ("Memory type:     %d\n", spd->mem_type);
	printf ("Row addr:        %d\n", spd->nrow_addr);
	printf ("Column addr:     %d\n", spd->ncol_addr);
	printf ("# of rows:       %d\n", spd->nrows);
	printf ("Row density:     %d\n", spd->row_dens);
	printf ("# of banks:      %d\n", spd->nbanks);
	printf ("Data width:      %d\n",
			256 * spd->dataw_msb + spd->dataw_lsb);
	printf ("Chip width:      %d\n", spd->primw);
	printf ("Refresh rate:    %02X\n", spd->refresh);
	printf ("CAS latencies:   %02X\n", spd->cas_lat);
	printf ("Write latencies: %02X\n", spd->write_lat);
	printf ("tRP:             %d\n", spd->trp);
	printf ("tRCD:            %d\n", spd->trcd);
	printf ("\n");
}
#endif /* SPD_DEBUG */

long int spd_sdram()
{
	volatile immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	volatile ddr83xx_t *ddr = &immap->ddr;
	volatile law83xx_t *ecm = &immap->sysconf.ddrlaw[0];
	spd_eeprom_t spd;
	unsigned int n_ranks;
	unsigned int odt_rd_cfg, odt_wr_cfg;
	unsigned char twr_clk, twtr_clk;
	unsigned int sdram_type;
	unsigned int memsize;
	unsigned int law_size;
	unsigned char caslat, caslat_ctrl;
	unsigned int trfc, trfc_clk, trfc_low;
	unsigned int trcd_clk, trtp_clk;
	unsigned char cke_min_clk;
	unsigned char add_lat, wr_lat;
	unsigned char wr_data_delay;
	unsigned char four_act;
	unsigned char cpo;
	unsigned char burstlen;
	unsigned char odt_cfg, mode_odt_enable;
	unsigned int max_bus_clk;
	unsigned int max_data_rate, effective_data_rate;
	unsigned int ddrc_clk;
	unsigned int refresh_clk;
	unsigned int sdram_cfg;
	unsigned int ddrc_ecc_enable;
	unsigned int pvr = get_pvr();

	/*
	 * First disable the memory controller (could be enabled
	 * by the debugger)
	 */
	clrsetbits_be32(&ddr->sdram_cfg, SDRAM_CFG_MEM_EN, 0);
	sync();
	isync();

	/* Read SPD parameters with I2C */
	CONFIG_SYS_READ_SPD(SPD_EEPROM_ADDRESS, SPD_EEPROM_OFFSET,
		SPD_EEPROM_ADDR_LEN, (uchar *) &spd, sizeof(spd));
#ifdef SPD_DEBUG
	spd_debug(&spd);
#endif
	/* Check the memory type */
	if (spd.mem_type != SPD_MEMTYPE_DDR && spd.mem_type != SPD_MEMTYPE_DDR2) {
		debug("DDR: Module mem type is %02X\n", spd.mem_type);
		return 0;
	}

	/* Check the number of physical bank */
	if (spd.mem_type == SPD_MEMTYPE_DDR) {
		n_ranks = spd.nrows;
	} else {
		n_ranks = (spd.nrows & 0x7) + 1;
	}

	if (n_ranks > 2) {
		printf("DDR: The number of physical bank is %02X\n", n_ranks);
		return 0;
	}

	/* Check if the number of row of the module is in the range of DDRC */
	if (spd.nrow_addr < 12 || spd.nrow_addr > 15) {
		printf("DDR: Row number is out of range of DDRC, row=%02X\n",
							 spd.nrow_addr);
		return 0;
	}

	/* Check if the number of col of the module is in the range of DDRC */
	if (spd.ncol_addr < 8 || spd.ncol_addr > 11) {
		printf("DDR: Col number is out of range of DDRC, col=%02X\n",
							 spd.ncol_addr);
		return 0;
	}

#ifdef CONFIG_SYS_DDRCDR_VALUE
	/*
	 * Adjust DDR II IO voltage biasing.  It just makes it work.
	 */
	if(spd.mem_type == SPD_MEMTYPE_DDR2) {
		immap->sysconf.ddrcdr = CONFIG_SYS_DDRCDR_VALUE;
	}
	udelay(50000);
#endif

	/*
	 * ODT configuration recommendation from DDR Controller Chapter.
	 */
	odt_rd_cfg = 0;			/* Never assert ODT */
	odt_wr_cfg = 0;			/* Never assert ODT */
	if (spd.mem_type == SPD_MEMTYPE_DDR2) {
		odt_wr_cfg = 1;		/* Assert ODT on writes to CSn */
	}

	/* Setup DDR chip select register */
#ifdef CONFIG_SYS_83XX_DDR_USES_CS0
	ddr->csbnds[0].csbnds = (banksize(spd.row_dens) >> 24) - 1;
	ddr->cs_config[0] = ( 1 << 31
			    | (odt_rd_cfg << 20)
			    | (odt_wr_cfg << 16)
			    | ((spd.nbanks == 8 ? 1 : 0) << 14)
			    | ((spd.nrow_addr - 12) << 8)
			    | (spd.ncol_addr - 8) );
	debug("\n");
	debug("cs0_bnds = 0x%08x\n",ddr->csbnds[0].csbnds);
	debug("cs0_config = 0x%08x\n",ddr->cs_config[0]);

	if (n_ranks == 2) {
		ddr->csbnds[1].csbnds = ( (banksize(spd.row_dens) >> 8)
				  | ((banksize(spd.row_dens) >> 23) - 1) );
		ddr->cs_config[1] = ( 1<<31
				    | (odt_rd_cfg << 20)
				    | (odt_wr_cfg << 16)
				    | ((spd.nbanks == 8 ? 1 : 0) << 14)
				    | ((spd.nrow_addr - 12) << 8)
				    | (spd.ncol_addr - 8) );
		debug("cs1_bnds = 0x%08x\n",ddr->csbnds[1].csbnds);
		debug("cs1_config = 0x%08x\n",ddr->cs_config[1]);
	}

#else
	ddr->csbnds[2].csbnds = (banksize(spd.row_dens) >> 24) - 1;
	ddr->cs_config[2] = ( 1 << 31
			    | (odt_rd_cfg << 20)
			    | (odt_wr_cfg << 16)
			    | ((spd.nbanks == 8 ? 1 : 0) << 14)
			    | ((spd.nrow_addr - 12) << 8)
			    | (spd.ncol_addr - 8) );
	debug("\n");
	debug("cs2_bnds = 0x%08x\n",ddr->csbnds[2].csbnds);
	debug("cs2_config = 0x%08x\n",ddr->cs_config[2]);

	if (n_ranks == 2) {
		ddr->csbnds[3].csbnds = ( (banksize(spd.row_dens) >> 8)
				  | ((banksize(spd.row_dens) >> 23) - 1) );
		ddr->cs_config[3] = ( 1<<31
				    | (odt_rd_cfg << 20)
				    | (odt_wr_cfg << 16)
				    | ((spd.nbanks == 8 ? 1 : 0) << 14)
				    | ((spd.nrow_addr - 12) << 8)
				    | (spd.ncol_addr - 8) );
		debug("cs3_bnds = 0x%08x\n",ddr->csbnds[3].csbnds);
		debug("cs3_config = 0x%08x\n",ddr->cs_config[3]);
	}
#endif

	/*
	 * Figure out memory size in Megabytes.
	 */
	memsize = n_ranks * banksize(spd.row_dens) / 0x100000;

	/*
	 * First supported LAW size is 16M, at LAWAR_SIZE_16M == 23.
	 */
	law_size = 19 + __ilog2(memsize);

	/*
	 * Set up LAWBAR for all of DDR.
	 */
	ecm->bar = CONFIG_SYS_SDRAM_BASE & 0xfffff000;
	ecm->ar  = (LAWAR_EN | LAWAR_TRGT_IF_DDR | (LAWAR_SIZE & law_size));
	debug("DDR:bar=0x%08x\n", ecm->bar);
	debug("DDR:ar=0x%08x\n", ecm->ar);

	/*
	 * Find the largest CAS by locating the highest 1 bit
	 * in the spd.cas_lat field.  Translate it to a DDR
	 * controller field value:
	 *
	 *	CAS Lat	DDR I	DDR II	Ctrl
	 *	Clocks	SPD Bit	SPD Bit	Value
	 *	-------	-------	-------	-----
	 *	1.0	0		0001
	 *	1.5	1		0010
	 *	2.0	2	2	0011
	 *	2.5	3		0100
	 *	3.0	4	3	0101
	 *	3.5	5		0110
	 *	4.0	6	4	0111
	 *	4.5			1000
	 *	5.0		5	1001
	 */
	caslat = __ilog2(spd.cas_lat);
	if ((spd.mem_type == SPD_MEMTYPE_DDR)
	    && (caslat > 6)) {
		printf("DDR I: Invalid SPD CAS Latency: 0x%x.\n", spd.cas_lat);
		return 0;
	} else if (spd.mem_type == SPD_MEMTYPE_DDR2
		   && (caslat < 2 || caslat > 5)) {
		printf("DDR II: Invalid SPD CAS Latency: 0x%x.\n",
		       spd.cas_lat);
		return 0;
	}
	debug("DDR: caslat SPD bit is %d\n", caslat);

	max_bus_clk = 1000 *10 / (((spd.clk_cycle & 0xF0) >> 4) * 10
			+ (spd.clk_cycle & 0x0f));
	max_data_rate = max_bus_clk * 2;

	debug("DDR:Module maximum data rate is: %d MHz\n", max_data_rate);

	ddrc_clk = gd->mem_clk / 1000000;
	effective_data_rate = 0;

	if (max_data_rate >= 460) { /* it is DDR2-800, 667, 533 */
		if (spd.cas_lat & 0x08)
			caslat = 3;
		else
			caslat = 4;
		if (ddrc_clk <= 460 && ddrc_clk > 350)
			effective_data_rate = 400;
		else if (ddrc_clk <=350 && ddrc_clk > 280)
			effective_data_rate = 333;
		else if (ddrc_clk <= 280 && ddrc_clk > 230)
			effective_data_rate = 266;
		else
			effective_data_rate = 200;
	} else if (max_data_rate >= 390 && max_data_rate < 460) { /* it is DDR 400 */
		if (ddrc_clk <= 460 && ddrc_clk > 350) {
			/* DDR controller clk at 350~460 */
			effective_data_rate = 400; /* 5ns */
			caslat = caslat;
		} else if (ddrc_clk <= 350 && ddrc_clk > 280) {
			/* DDR controller clk at 280~350 */
			effective_data_rate = 333; /* 6ns */
			if (spd.clk_cycle2 == 0x60)
				caslat = caslat - 1;
			else
				caslat = caslat;
		} else if (ddrc_clk <= 280 && ddrc_clk > 230) {
			/* DDR controller clk at 230~280 */
			effective_data_rate = 266; /* 7.5ns */
			if (spd.clk_cycle3 == 0x75)
				caslat = caslat - 2;
			else if (spd.clk_cycle2 == 0x75)
				caslat = caslat - 1;
			else
				caslat = caslat;
		} else if (ddrc_clk <= 230 && ddrc_clk > 90) {
			/* DDR controller clk at 90~230 */
			effective_data_rate = 200; /* 10ns */
			if (spd.clk_cycle3 == 0xa0)
				caslat = caslat - 2;
			else if (spd.clk_cycle2 == 0xa0)
				caslat = caslat - 1;
			else
				caslat = caslat;
		}
	} else if (max_data_rate >= 323) { /* it is DDR 333 */
		if (ddrc_clk <= 350 && ddrc_clk > 280) {
			/* DDR controller clk at 280~350 */
			effective_data_rate = 333; /* 6ns */
			caslat = caslat;
		} else if (ddrc_clk <= 280 && ddrc_clk > 230) {
			/* DDR controller clk at 230~280 */
			effective_data_rate = 266; /* 7.5ns */
			if (spd.clk_cycle2 == 0x75)
				caslat = caslat - 1;
			else
				caslat = caslat;
		} else if (ddrc_clk <= 230 && ddrc_clk > 90) {
			/* DDR controller clk at 90~230 */
			effective_data_rate = 200; /* 10ns */
			if (spd.clk_cycle3 == 0xa0)
				caslat = caslat - 2;
			else if (spd.clk_cycle2 == 0xa0)
				caslat = caslat - 1;
			else
				caslat = caslat;
		}
	} else if (max_data_rate >= 256) { /* it is DDR 266 */
		if (ddrc_clk <= 350 && ddrc_clk > 280) {
			/* DDR controller clk at 280~350 */
			printf("DDR: DDR controller freq is more than "
				"max data rate of the module\n");
			return 0;
		} else if (ddrc_clk <= 280 && ddrc_clk > 230) {
			/* DDR controller clk at 230~280 */
			effective_data_rate = 266; /* 7.5ns */
			caslat = caslat;
		} else if (ddrc_clk <= 230 && ddrc_clk > 90) {
			/* DDR controller clk at 90~230 */
			effective_data_rate = 200; /* 10ns */
			if (spd.clk_cycle2 == 0xa0)
				caslat = caslat - 1;
		}
	} else if (max_data_rate >= 190) { /* it is DDR 200 */
		if (ddrc_clk <= 350 && ddrc_clk > 230) {
			/* DDR controller clk at 230~350 */
			printf("DDR: DDR controller freq is more than "
				"max data rate of the module\n");
			return 0;
		} else if (ddrc_clk <= 230 && ddrc_clk > 90) {
			/* DDR controller clk at 90~230 */
			effective_data_rate = 200; /* 10ns */
			caslat = caslat;
		}
	}

	debug("DDR:Effective data rate is: %dMHz\n", effective_data_rate);
	debug("DDR:The MSB 1 of CAS Latency is: %d\n", caslat);

	/*
	 * Errata DDR6 work around: input enable 2 cycles earlier.
	 * including MPC834X Rev1.0/1.1 and MPC8360 Rev1.1/1.2.
	 */
	if(PVR_MAJ(pvr) <= 1 && spd.mem_type == SPD_MEMTYPE_DDR){
		if (caslat == 2)
			ddr->debug_reg = 0x201c0000; /* CL=2 */
		else if (caslat == 3)
			ddr->debug_reg = 0x202c0000; /* CL=2.5 */
		else if (caslat == 4)
			ddr->debug_reg = 0x202c0000; /* CL=3.0 */

		sync();

		debug("Errata DDR6 (debug_reg=0x%08x)\n", ddr->debug_reg);
	}

	/*
	 * Convert caslat clocks to DDR controller value.
	 * Force caslat_ctrl to be DDR Controller field-sized.
	 */
	if (spd.mem_type == SPD_MEMTYPE_DDR) {
		caslat_ctrl = (caslat + 1) & 0x07;
	} else {
		caslat_ctrl =  (2 * caslat - 1) & 0x0f;
	}

	debug("DDR: effective data rate is %d MHz\n", effective_data_rate);
	debug("DDR: caslat SPD bit is %d, controller field is 0x%x\n",
	      caslat, caslat_ctrl);

	/*
	 * Timing Config 0.
	 * Avoid writing for DDR I.
	 */
	if (spd.mem_type == SPD_MEMTYPE_DDR2) {
		unsigned char taxpd_clk = 8;		/* By the book. */
		unsigned char tmrd_clk = 2;		/* By the book. */
		unsigned char act_pd_exit = 2;		/* Empirical? */
		unsigned char pre_pd_exit = 6;		/* Empirical? */

		ddr->timing_cfg_0 = (0
			| ((act_pd_exit & 0x7) << 20)	/* ACT_PD_EXIT */
			| ((pre_pd_exit & 0x7) << 16)	/* PRE_PD_EXIT */
			| ((taxpd_clk & 0xf) << 8)	/* ODT_PD_EXIT */
			| ((tmrd_clk & 0xf) << 0)	/* MRS_CYC */
			);
		debug("DDR: timing_cfg_0 = 0x%08x\n", ddr->timing_cfg_0);
	}

	/*
	 * For DDR I, WRREC(Twr) and WRTORD(Twtr) are not in SPD,
	 * use conservative value.
	 * For DDR II, they are bytes 36 and 37, in quarter nanos.
	 */

	if (spd.mem_type == SPD_MEMTYPE_DDR) {
		twr_clk = 3;	/* Clocks */
		twtr_clk = 1;	/* Clocks */
	} else {
		twr_clk = picos_to_clk(spd.twr * 250);
		twtr_clk = picos_to_clk(spd.twtr * 250);
		if (twtr_clk < 2)
			twtr_clk = 2;
	}

	/*
	 * Calculate Trfc, in picos.
	 * DDR I:  Byte 42 straight up in ns.
	 * DDR II: Byte 40 and 42 swizzled some, in ns.
	 */
	if (spd.mem_type == SPD_MEMTYPE_DDR) {
		trfc = spd.trfc * 1000;		/* up to ps */
	} else {
		unsigned int byte40_table_ps[8] = {
			0,
			250,
			330,
			500,
			660,
			750,
			0,
			0
		};

		trfc = (((spd.trctrfc_ext & 0x1) * 256) + spd.trfc) * 1000
			+ byte40_table_ps[(spd.trctrfc_ext >> 1) & 0x7];
	}
	trfc_clk = picos_to_clk(trfc);

	/*
	 * Trcd, Byte 29, from quarter nanos to ps and clocks.
	 */
	trcd_clk = picos_to_clk(spd.trcd * 250) & 0x7;

	/*
	 * Convert trfc_clk to DDR controller fields.  DDR I should
	 * fit in the REFREC field (16-19) of TIMING_CFG_1, but the
	 * 83xx controller has an extended REFREC field of three bits.
	 * The controller automatically adds 8 clocks to this value,
	 * so preadjust it down 8 first before splitting it up.
	 */
	trfc_low = (trfc_clk - 8) & 0xf;

	ddr->timing_cfg_1 =
	    (((picos_to_clk(spd.trp * 250) & 0x07) << 28 ) |	/* PRETOACT */
	     ((picos_to_clk(spd.tras * 1000) & 0x0f ) << 24 ) | /* ACTTOPRE */
	     (trcd_clk << 20 ) |				/* ACTTORW */
	     (caslat_ctrl << 16 ) |				/* CASLAT */
	     (trfc_low << 12 ) |				/* REFEC */
	     ((twr_clk & 0x07) << 8) |				/* WRRREC */
	     ((picos_to_clk(spd.trrd * 250) & 0x07) << 4) |	/* ACTTOACT */
	     ((twtr_clk & 0x07) << 0)				/* WRTORD */
	    );

	/*
	 * Additive Latency
	 * For DDR I, 0.
	 * For DDR II, with ODT enabled, use "a value" less than ACTTORW,
	 * which comes from Trcd, and also note that:
	 *	add_lat + caslat must be >= 4
	 */
	add_lat = 0;
	if (spd.mem_type == SPD_MEMTYPE_DDR2
	    && (odt_wr_cfg || odt_rd_cfg)
	    && (caslat < 4)) {
		add_lat = 4 - caslat;
		if ((add_lat + caslat) < 4) {
			add_lat = 0;
		}
	}

	/*
	 * Write Data Delay
	 * Historically 0x2 == 4/8 clock delay.
	 * Empirically, 0x3 == 6/8 clock delay is suggested for DDR I 266.
	 */
	wr_data_delay = 2;
#ifdef CONFIG_SYS_DDR_WRITE_DATA_DELAY
	wr_data_delay = CONFIG_SYS_DDR_WRITE_DATA_DELAY;
#endif

	/*
	 * Write Latency
	 * Read to Precharge
	 * Minimum CKE Pulse Width.
	 * Four Activate Window
	 */
	if (spd.mem_type == SPD_MEMTYPE_DDR) {
		/*
		 * This is a lie.  It should really be 1, but if it is
		 * set to 1, bits overlap into the old controller's
		 * otherwise unused ACSM field.  If we leave it 0, then
		 * the HW will magically treat it as 1 for DDR 1.  Oh Yea.
		 */
		wr_lat = 0;

		trtp_clk = 2;		/* By the book. */
		cke_min_clk = 1;	/* By the book. */
		four_act = 1;		/* By the book. */

	} else {
		wr_lat = caslat - 1;

		/* Convert SPD value from quarter nanos to picos. */
		trtp_clk = picos_to_clk(spd.trtp * 250);
		if (trtp_clk < 2)
			trtp_clk = 2;
		trtp_clk += add_lat;

		cke_min_clk = 3;	/* By the book. */
		four_act = picos_to_clk(37500);	/* By the book. 1k pages? */
	}

	/*
	 * Empirically set ~MCAS-to-preamble override for DDR 2.
	 * Your mileage will vary.
	 */
	cpo = 0;
	if (spd.mem_type == SPD_MEMTYPE_DDR2) {
#ifdef CONFIG_SYS_DDR_CPO
		cpo = CONFIG_SYS_DDR_CPO;
#else
		if (effective_data_rate == 266) {
			cpo = 0x4;		/* READ_LAT + 1/2 */
		} else if (effective_data_rate == 333) {
			cpo = 0x6;		/* READ_LAT + 1 */
		} else if (effective_data_rate == 400) {
			cpo = 0x7;		/* READ_LAT + 5/4 */
		} else {
			/* Automatic calibration */
			cpo = 0x1f;
		}
#endif
	}

	ddr->timing_cfg_2 = (0
		| ((add_lat & 0x7) << 28)		/* ADD_LAT */
		| ((cpo & 0x1f) << 23)			/* CPO */
		| ((wr_lat & 0x7) << 19)		/* WR_LAT */
		| ((trtp_clk & 0x7) << 13)		/* RD_TO_PRE */
		| ((wr_data_delay & 0x7) << 10)		/* WR_DATA_DELAY */
		| ((cke_min_clk & 0x7) << 6)		/* CKE_PLS */
		| ((four_act & 0x1f) << 0)		/* FOUR_ACT */
		);

	debug("DDR:timing_cfg_1=0x%08x\n", ddr->timing_cfg_1);
	debug("DDR:timing_cfg_2=0x%08x\n", ddr->timing_cfg_2);

	/* Check DIMM data bus width */
	if (spd.dataw_lsb < 64) {
		if (spd.mem_type == SPD_MEMTYPE_DDR)
			burstlen = 0x03; /* 32 bit data bus, burst len is 8 */
		else
			burstlen = 0x02; /* 32 bit data bus, burst len is 4 */
		debug("\n   DDR DIMM: data bus width is 32 bit");
	} else {
		burstlen = 0x02; /* Others act as 64 bit bus, burst len is 4 */
		debug("\n   DDR DIMM: data bus width is 64 bit");
	}

	/* Is this an ECC DDR chip? */
	if (spd.config == 0x02)
		debug(" with ECC\n");
	else
		debug(" without ECC\n");

	/* Burst length is always 4 for 64 bit data bus, 8 for 32 bit data bus,
	   Burst type is sequential
	 */
	if (spd.mem_type == SPD_MEMTYPE_DDR) {
		switch (caslat) {
		case 1:
			ddr->sdram_mode = 0x50 | burstlen; /* CL=1.5 */
			break;
		case 2:
			ddr->sdram_mode = 0x20 | burstlen; /* CL=2.0 */
			break;
		case 3:
			ddr->sdram_mode = 0x60 | burstlen; /* CL=2.5 */
			break;
		case 4:
			ddr->sdram_mode = 0x30 | burstlen; /* CL=3.0 */
			break;
		default:
			printf("DDR:only CL 1.5, 2.0, 2.5, 3.0 is supported\n");
			return 0;
		}
	} else {
		mode_odt_enable = 0x0;                  /* Default disabled */
		if (odt_wr_cfg || odt_rd_cfg) {
			/*
			 * Bits 6 and 2 in Extended MRS(1)
			 * Bit 2 == 0x04 == 75 Ohm, with 2 DIMM modules.
			 * Bit 6 == 0x40 == 150 Ohm, with 1 DIMM module.
			 */
			mode_odt_enable = 0x40;         /* 150 Ohm */
		}

		ddr->sdram_mode =
			(0
			 | (1 << (16 + 10))             /* DQS Differential disable */
#ifdef CONFIG_SYS_DDR_MODE_WEAK
			 | (1 << (16 + 1))		/* weak driver (~60%) */
#endif
			 | (add_lat << (16 + 3))        /* Additive Latency in EMRS1 */
			 | (mode_odt_enable << 16)      /* ODT Enable in EMRS1 */
			 | ((twr_clk - 1) << 9)         /* Write Recovery Autopre */
			 | (caslat << 4)                /* caslat */
			 | (burstlen << 0)              /* Burst length */
			);
	}
	debug("DDR:sdram_mode=0x%08x\n", ddr->sdram_mode);

	/*
	 * Clear EMRS2 and EMRS3.
	 */
	ddr->sdram_mode2 = 0;
	debug("DDR: sdram_mode2 = 0x%08x\n", ddr->sdram_mode2);

	switch (spd.refresh) {
		case 0x00:
		case 0x80:
			refresh_clk = picos_to_clk(15625000);
			break;
		case 0x01:
		case 0x81:
			refresh_clk = picos_to_clk(3900000);
			break;
		case 0x02:
		case 0x82:
			refresh_clk = picos_to_clk(7800000);
			break;
		case 0x03:
		case 0x83:
			refresh_clk = picos_to_clk(31300000);
			break;
		case 0x04:
		case 0x84:
			refresh_clk = picos_to_clk(62500000);
			break;
		case 0x05:
		case 0x85:
			refresh_clk = picos_to_clk(125000000);
			break;
		default:
			refresh_clk = 0x512;
			break;
	}

	/*
	 * Set BSTOPRE to 0x100 for page mode
	 * If auto-charge is used, set BSTOPRE = 0
	 */
	ddr->sdram_interval = ((refresh_clk & 0x3fff) << 16) | 0x100;
	debug("DDR:sdram_interval=0x%08x\n", ddr->sdram_interval);

	/*
	 * SDRAM Cfg 2
	 */
	odt_cfg = 0;
#ifndef CONFIG_NEVER_ASSERT_ODT_TO_CPU
	if (odt_rd_cfg | odt_wr_cfg) {
		odt_cfg = 0x2;		/* ODT to IOs during reads */
	}
#endif
	if (spd.mem_type == SPD_MEMTYPE_DDR2) {
		ddr->sdram_cfg2 = (0
			    | (0 << 26)	/* True DQS */
			    | (odt_cfg << 21)	/* ODT only read */
			    | (1 << 12)	/* 1 refresh at a time */
			    );

		debug("DDR: sdram_cfg2  = 0x%08x\n", ddr->sdram_cfg2);
	}

#ifdef CONFIG_SYS_DDR_SDRAM_CLK_CNTL	/* Optional platform specific value */
	ddr->sdram_clk_cntl = CONFIG_SYS_DDR_SDRAM_CLK_CNTL;
#endif
	debug("DDR:sdram_clk_cntl=0x%08x\n", ddr->sdram_clk_cntl);

	sync();
	isync();

	udelay(600);

	/*
	 * Figure out the settings for the sdram_cfg register. Build up
	 * the value in 'sdram_cfg' before writing since the write into
	 * the register will actually enable the memory controller, and all
	 * settings must be done before enabling.
	 *
	 * sdram_cfg[0]   = 1 (ddr sdram logic enable)
	 * sdram_cfg[1]   = 1 (self-refresh-enable)
	 * sdram_cfg[5:7] = (SDRAM type = DDR SDRAM)
	 *			010 DDR 1 SDRAM
	 *			011 DDR 2 SDRAM
	 * sdram_cfg[12] = 0 (32_BE =0 , 64 bit bus mode)
	 * sdram_cfg[13] = 0 (8_BE =0, 4-beat bursts)
	 */
	if (spd.mem_type == SPD_MEMTYPE_DDR)
		sdram_type = SDRAM_CFG_SDRAM_TYPE_DDR1;
	else
		sdram_type = SDRAM_CFG_SDRAM_TYPE_DDR2;

	sdram_cfg = (0
		     | SDRAM_CFG_MEM_EN		/* DDR enable */
		     | SDRAM_CFG_SREN		/* Self refresh */
		     | sdram_type		/* SDRAM type */
		     );

	/* sdram_cfg[3] = RD_EN - registered DIMM enable */
	if (spd.mod_attr & 0x02)
		sdram_cfg |= SDRAM_CFG_RD_EN;

	/* The DIMM is 32bit width */
	if (spd.dataw_lsb < 64) {
		if (spd.mem_type == SPD_MEMTYPE_DDR)
			sdram_cfg |= SDRAM_CFG_32_BE | SDRAM_CFG_8_BE;
		if (spd.mem_type == SPD_MEMTYPE_DDR2)
			sdram_cfg |= SDRAM_CFG_32_BE;
	}

	ddrc_ecc_enable = 0;

#if defined(CONFIG_DDR_ECC)
	/* Enable ECC with sdram_cfg[2] */
	if (spd.config == 0x02) {
		sdram_cfg |= 0x20000000;
		ddrc_ecc_enable = 1;
		/* disable error detection */
		ddr->err_disable = ~ECC_ERROR_ENABLE;
		/* set single bit error threshold to maximum value,
		 * reset counter to zero */
		ddr->err_sbe = (255 << ECC_ERROR_MAN_SBET_SHIFT) |
				(0 << ECC_ERROR_MAN_SBEC_SHIFT);
	}

	debug("DDR:err_disable=0x%08x\n", ddr->err_disable);
	debug("DDR:err_sbe=0x%08x\n", ddr->err_sbe);
#endif
	debug("   DDRC ECC mode: %s\n", ddrc_ecc_enable ? "ON":"OFF");

#if defined(CONFIG_DDR_2T_TIMING)
	/*
	 * Enable 2T timing by setting sdram_cfg[16].
	 */
	sdram_cfg |= SDRAM_CFG_2T_EN;
#endif
	/* Enable controller, and GO! */
	ddr->sdram_cfg = sdram_cfg;
	sync();
	isync();
	udelay(500);

	debug("DDR:sdram_cfg=0x%08x\n", ddr->sdram_cfg);
	return memsize; /*in MBytes*/
}
#endif /* CONFIG_SPD_EEPROM */

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
static inline u32 mftbu(void)
{
	u32 rval;

	asm volatile("mftbu %0" : "=r" (rval));
	return rval;
}

static inline u32 mftb(void)
{
	u32 rval;

	asm volatile("mftb %0" : "=r" (rval));
	return rval;
}

/*
 * Use timebase counter, get_timer() is not available
 * at this point of initialization yet.
 */
static __inline__ unsigned long get_tbms (void)
{
	unsigned long tbl;
	unsigned long tbu1, tbu2;
	unsigned long ms;
	unsigned long long tmp;

	ulong tbclk = get_tbclk();

	/* get the timebase ticks */
	do {
		tbu1 = mftbu();
		tbl = mftb();
		tbu2 = mftbu();
	} while (tbu1 != tbu2);

	/* convert ticks to ms */
	tmp = (unsigned long long)(tbu1);
	tmp = (tmp << 32);
	tmp += (unsigned long long)(tbl);
	ms = tmp/(tbclk/1000);

	return ms;
}

/*
 * Initialize all of memory for ECC, then enable errors.
 */
void ddr_enable_ecc(unsigned int dram_size)
{
	volatile immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	volatile ddr83xx_t *ddr= &immap->ddr;
	unsigned long t_start, t_end;
	register u64 *p;
	register uint size;
	unsigned int pattern[2];

	icache_enable();
	t_start = get_tbms();
	pattern[0] = 0xdeadbeef;
	pattern[1] = 0xdeadbeef;

#if defined(CONFIG_DDR_ECC_INIT_VIA_DMA)
	dma_meminit(pattern[0], dram_size);
#else
	debug("ddr init: CPU FP write method\n");
	size = dram_size;
	for (p = 0; p < (u64*)(size); p++) {
		ppcDWstore((u32*)p, pattern);
	}
	sync();
#endif

	t_end = get_tbms();
	icache_disable();

	debug("\nREADY!!\n");
	debug("ddr init duration: %ld ms\n", t_end - t_start);

	/* Clear All ECC Errors */
	if ((ddr->err_detect & ECC_ERROR_DETECT_MME) == ECC_ERROR_DETECT_MME)
		ddr->err_detect |= ECC_ERROR_DETECT_MME;
	if ((ddr->err_detect & ECC_ERROR_DETECT_MBE) == ECC_ERROR_DETECT_MBE)
		ddr->err_detect |= ECC_ERROR_DETECT_MBE;
	if ((ddr->err_detect & ECC_ERROR_DETECT_SBE) == ECC_ERROR_DETECT_SBE)
		ddr->err_detect |= ECC_ERROR_DETECT_SBE;
	if ((ddr->err_detect & ECC_ERROR_DETECT_MSE) == ECC_ERROR_DETECT_MSE)
		ddr->err_detect |= ECC_ERROR_DETECT_MSE;

	/* Disable ECC-Interrupts */
	ddr->err_int_en &= ECC_ERR_INT_DISABLE;

	/* Enable errors for ECC */
	ddr->err_disable &= ECC_ERROR_ENABLE;

	sync();
	isync();
}
#endif	/* CONFIG_DDR_ECC */

#endif /* !CONFIG_MPC83XX_SDRAM */
