/*
 * EJTAG access interface - top level routines, will make calls
 * to either jamjtag.c or jtagm.c based on JTAG type (dongle type)
 *
 * Copyright 2004 Broadcom Corporation
 *
 * $Id: ejtag.c 467150 2014-04-02 17:30:43Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#ifdef BCMDBG
#define EJ_MSG(x) printf x
#else
#define EJ_MSG(x)
#endif
#else	/* BCMDRIVER */
#include <stdio.h>
#include <stdlib.h>
#define EJ_MSG(x) printf x
#endif	/* BCMDRIVER */
#include <hndsoc.h>
#include <sbchipc.h>
#include <bcmdevs.h>
#ifndef BCMDRIVER
#include "syspci.h"
#include "remotepci.h"
#include "jamjtag.h"
#endif	/* BCMDRIVER */
#include "jtagm.h"
#include "ejtag.h"

#define DMA_RETRIES	2500000

/* type of jtag jig */
int dongle = XILINX;

/* Instruction & data register sizes */

uint	dr_size = DEF_DATA_SIZE;
uint	ir_size = DEF_INST_SIZE;

/* Tracing of jtag signals and/or register accesses */
int	jtag_trace = 0;
/* global indicating endianness of target */
bool ejtag_bigend = FALSE;

/* global indicating mode of target access */
int ejtag_mode = EJTAG_CHIPC;


/* MIPS mode defines: */

/* Register addresses */
#define MIPS_ADDR	0x08
#define MIPS_DATA	0x09
#define MIPS_CTRL	0x0a

/* DMA related bits in the Control register */
#define DMA_SZ1		0x00000000
#define DMA_SZ2		0x00000080
#define DMA_SZ4		0x00000100
#define DMA_SZ3		0x00000180
#define DMA_READ	0x00000200
#define DMA_ERROR	0x00000400
#define DMA_START	0x00000800
#define DMA_BREAK	0x00001000
#define DMA_ACC		0x00020000

/* chipc mode defines: */

/* Register addresses */
#define CHIPC_ADDR	0x30
#define CHIPC_DATA	0x32
#define CHIPC_CTRL	0x34

#define	CHIPC_RO	1		/* Or in this to get the read-only address */

/* Control register bits */
#define CCC_BE0		0x00000001
#define CCC_BE1		0x00000002
#define CCC_BE2		0x00000004
#define CCC_BE3		0x00000008
#define	CCC_SZ1		(CCC_BE0)
#define	CCC_SZ2		(CCC_BE1 | CCC_BE0)
#define	CCC_SZ4		(CCC_BE3 | CCC_BE2 | CCC_BE1 | CCC_BE0)
#define CCC_READ	0x00000010
#define CCC_START	0x00000020
#define CCC_ERROR	0x00000040

/* Bits written into the control register need to be shifted */
#define	CCC_WR_SHIFT	25

/* ejtag initialization */
#ifdef BCMDRIVER
int
ejtag_init(uint16 devid, uint32 sbidh, void *regsva, bool diffend)
{
	int status = jtagm_init(devid, sbidh, regsva, 0);
	if (status)
		return status;

	/* config EJTAG */
	ejtag_bigend = diffend;
	dongle = HND_JTAGM;
	ejtag_mode = EJTAG_CHIPC;
	dr_size = DEF_DATA_SIZE;
	ir_size = 8;

	return 0;
}

void
ejtag_cleanup(void)
{
}
#else
void
initialize_jtag_hardware(bool remote)
{
	int		i;
	dev_info	*di;
	map_info	*map = NULL;
	bar_info	bi;
	sbconfig_t	*sb;
	uint32		tmp;


	/* Check to see if we have a jtag master */
	for (i = 0; i < free_dev; i++) {
		di = devs[i];

		/* If we are doing remote access don't check the id, there
		 * should be a single target anyway
		 */
		if (remote || (di->config.vendor == VENDOR_BROADCOM)) {
			uint32 sbidh;

			/* Get its bar0 */
			crack_bar(di, 0, &bi);
			if (bi.base == 0) {
				EJ_MSG(("Found jtagm with id 0x%04x, but its bar0 is disabled.",
				        di->config.device));
				continue;
			}

			/* Make sure the window points to chipc (always the first core) */
			if (remote) {
				tmp = SI_ENUM_BASE;
				if (rempci_write(REMPCI_CONFIG_BASE + PCI_BAR0_WIN, (uchar *)(&tmp),
				                 4, REMPCI_CFG) != 0) {
					EJ_MSG(("%s: Error writing to remote pci config space\n",
					        __FUNCTION__));
					continue;
				}
				if (rempci_read(bi.base + SBCONFIGOFF + SBIDHIGH, (uchar *)&sbidh,
				                4, REMPCI_MEM) != 0) {
					EJ_MSG(("%s: Error reading sbidh\n", __FUNCTION__));
					continue;
				}
			} else {
				write_pci_config(di->bus, di->slot, di->fun, PCI_BAR0_WIN,
				                 SI_ENUM_BASE);

				if (((map = map_dev (i, 0)) == NULL) ||
				    (map->vaddr == NULL)) {
					EJ_MSG(("Cannot map bar0 of jtagm id 0x%04x\n",
					        di->config.device));
					continue;
				}

				sb = (sbconfig_t *)((uint)map->vaddr + SBCONFIGOFF);
				sbidh = sb->sbidhigh;
			}

			/* Init jtagm access */
			if (jtagm_init(di->config.device, sbidh, remote ? (void *)bi.base :
			               map->vaddr, remote))
				continue;

			/* Config EJTAG */
			dongle = HND_JTAGM;
			ejtag_mode = EJTAG_CHIPC;
			dr_size = DEF_DATA_SIZE;
			ir_size = 8;

			return;
		}
	}

	/* Didn't find a jtag master */
	if (remote) {
		EJ_MSG(("Error : cannot mix remote with parallel port ejtag\n"));
		exit(1);
	}

	/* Use parallal port jtag */
	jam_jtag_init(0);
}

void
close_jtag_hardware()
{
	jam_jtag_cleanup();
}
#endif /* BCMDRIVER */

int
read_ejtag(ulong addr, ulong *read_data, uint size)
{
	ulong	zeros = 0x00000000;
	ulong	cmd, mask, shift;
	ulong	data, dataout;
	int	i;
	bool	error = FALSE;


	shift = addr & 3;
	switch (size) {
	case 1:
		cmd = (ejtag_mode == EJTAG_MIPS) ?
		        (DMA_ACC | DMA_READ | DMA_SZ1) :
		        (CCC_START | CCC_READ | (CCC_SZ1 << shift));
		if (ejtag_bigend)
			mask = 0xff000000 >> (shift * 8);
		else
			mask = 0x000000ff << (shift * 8);
		break;

	case 2:
		cmd = (ejtag_mode == EJTAG_MIPS) ?
		        (DMA_ACC | DMA_READ | DMA_SZ2) :
		        (CCC_START | CCC_READ | (CCC_SZ2 << shift));
		if (ejtag_bigend)
			mask = 0xffff0000 >> (shift * 8);
		else
			mask = 0x0000ffff << (shift * 8);
		if (addr & 1)
			error = TRUE;
		break;

	case 4:
		cmd = (ejtag_mode == EJTAG_MIPS) ?
		        (DMA_ACC | DMA_READ | DMA_SZ4) :
		        (CCC_START | CCC_READ | CCC_SZ4);
		mask = 0xffffffff;
		if (addr & 3)
			error = TRUE;
		break;

	default:
		/* Bad size */
		return -1;
	}

	if (error)
		return error;

	if (dongle == HND_JTAGM) {
		uint irsz = ir_size, drsz = dr_size;

		/* Do we need to reset? */

		if (ejtag_mode == EJTAG_MIPS) {
			/* set the dmaacc bit */
			dataout = jtagm_scmd(MIPS_CTRL, irsz, cmd, drsz);

			/* scan in the address */
			dataout = jtagm_scmd(MIPS_ADDR, irsz, addr, drsz);

			/* set the dma start bit, size bit and read/write bit */
			dataout = jtagm_scmd(MIPS_CTRL, irsz, cmd | DMA_START, drsz);

			/* wait for dma to complete */
			i = 0;
			do {
				dataout = jtagm_scmd(MIPS_CTRL, irsz, cmd, drsz);
			} while ((dataout & DMA_START) && (++i < DMA_RETRIES));

			if ((dataout & DMA_ERROR) || (i >= DMA_RETRIES)) {
				data = 0xffffffff;
			} else {
				/* read out the data */
				data = jtagm_scmd(MIPS_DATA, irsz, zeros, drsz);
			}

			/* clear the dmaacc bit */
			dataout = jtagm_scmd(MIPS_CTRL, irsz, zeros, drsz);
		} else {
			/* scan in the address */
			dataout = jtagm_scmd(CHIPC_ADDR, irsz, addr, drsz);

			/* set the dma start bit, size bit and read/write bit */
			dataout = jtagm_scmd(CHIPC_CTRL, irsz, (cmd << CCC_WR_SHIFT), drsz);

			/* wait for dma to complete */
			i = 0;
			do {
				dataout = jtagm_scmd((CHIPC_CTRL | CHIPC_RO), irsz, zeros, drsz);
			} while ((dataout & CCC_START) && (++i < DMA_RETRIES));

			if (i < DMA_RETRIES) {
				/* Read the control reg one more time to get error bit */
				dataout = jtagm_scmd((CHIPC_CTRL | CHIPC_RO), irsz, zeros, drsz);
				if (dataout & CCC_ERROR) {
					data = 0xffffffff;
				} else {
					/* read out the data */
					data = jtagm_scmd((CHIPC_DATA | CHIPC_RO), irsz, zeros,
					                  drsz);
				}
			} else {
				data = 0xffffffff;
			}
		}
	} else {
#ifdef BCMDRIVER
		return -1;
#else
		/* force to idle */
		jam_jtag_reset_idle();

		if (ejtag_mode == EJTAG_MIPS) {
			/* set the dmaacc bit */
			jam_jtag_irscan(MIPS_CTRL, &dataout);
			jam_jtag_drscan(cmd, &dataout);

			/* scan in the address */
			jam_jtag_irscan(MIPS_ADDR, &dataout);
			jam_jtag_drscan(addr, &dataout);

			/* set the dma start bit, size bit and read/write bit */
			jam_jtag_irscan(MIPS_CTRL, &dataout);
			jam_jtag_drscan(cmd | DMA_START, &dataout);

			/* wait for dma to complete */
			i = 0;
			do {
				jam_jtag_irscan(MIPS_CTRL, &dataout);
				jam_jtag_drscan(cmd, &dataout);
			} while ((dataout & DMA_START) && (++i < DMA_RETRIES));

			if ((dataout & DMA_ERROR) || (i >= DMA_RETRIES)) {
				data = 0xffffffff;
			} else {
				/* read out the data */
				jam_jtag_irscan(MIPS_DATA, &dataout);
				jam_jtag_drscan(zeros, &data);
			}

			/* clear the dmaacc bit */
			jam_jtag_irscan(MIPS_CTRL, &dataout);
			jam_jtag_drscan(zeros, &dataout);

		} else {
			/* scan in the address */
			jam_jtag_irscan(CHIPC_ADDR, &dataout);
			jam_jtag_drscan(addr, &dataout);

			/* set the dma start bit, size bit and read/write bit */
			jam_jtag_irscan(CHIPC_CTRL, &dataout);
			jam_jtag_drscan((cmd << CCC_WR_SHIFT), &dataout);

			/* wait for dma to complete */
			i = 0;
			do {
				jam_jtag_irscan(CHIPC_CTRL | CHIPC_RO, &dataout);
				jam_jtag_drscan(zeros, &dataout);
			} while ((dataout & CCC_START) && (++i < DMA_RETRIES));

			if (i < DMA_RETRIES) {
				/* Read the control reg one more time to get error bit */
				jam_jtag_irscan(CHIPC_CTRL | CHIPC_RO, &dataout);
				jam_jtag_drscan(zeros, &dataout);
				if (dataout & CCC_ERROR) {
					data = 0xffffffff;
				} else {
					/* read out the data */
					jam_jtag_irscan(CHIPC_DATA | CHIPC_RO, &dataout);
					jam_jtag_drscan(zeros, &data);
				}
			} else {
				data = 0xffffffff;
			}
		}
#endif /* BCMDRIVER */
	}

	/* retrieve the data from the correct bytes */
	data &= mask;
	while (!(mask & 0xff)) {
		mask = mask >> 8;
		data = data >> 8;
	}

	*read_data = data;

	return 0;
}

int
write_ejtag(ulong addr, ulong write_data, uint size)
{
	ulong zeros = 0x00000000;
	ulong cmd, shift;
	ulong dataout;
	int i, error = 0;

	/* move data in to correct byte location, depends on
	 * size, addr and endianess
	 */
	switch (size) {
	case 1:
		if (ejtag_bigend)
			shift = ~addr & 3;
		else
			shift = addr & 3;
		write_data <<= (shift * 8);
		if (ejtag_mode == EJTAG_MIPS)
			cmd = DMA_ACC | DMA_SZ1;
		else
			cmd = CCC_START | (CCC_SZ1 << shift);
		break;

	case 2:
		/* check alignment */
		if (addr & 1)
			error = -1;

		if (ejtag_bigend)
			shift = (addr+2) & 3;
		else
			shift = addr & 3;

		write_data <<= (shift * 8);

		if (ejtag_mode == EJTAG_MIPS)
			cmd = DMA_ACC | DMA_SZ2;
		else
			cmd = CCC_START | (CCC_SZ2 << shift);
		break;

	case 4:
		/* check alignment */
		if (addr & 3)
			error = -1;

		cmd = (ejtag_mode == EJTAG_MIPS) ?
		        (DMA_ACC | DMA_SZ4) :
		        (CCC_START | CCC_SZ4);
		break;

	default:
		/* Bad size */
		return -1;
	}

	if (error)
		return error;

	if (dongle == HND_JTAGM) {
		uint irsz = ir_size, drsz = dr_size;

		/* Do we need to reset? */

		if (ejtag_mode == EJTAG_MIPS) {
			/* set the dmaacc bit */
			dataout = jtagm_scmd(MIPS_CTRL, irsz, cmd, drsz);

			/* scan in the address */
			dataout = jtagm_scmd(MIPS_ADDR, irsz, addr, drsz);

			/* write the data */
			dataout = jtagm_scmd(MIPS_DATA, irsz, write_data, drsz);

			/* set the dma start bit, size bit and read/write bit */
			dataout = jtagm_scmd(MIPS_CTRL, irsz, cmd | DMA_START, drsz);

			/* wait for dma to complete */
			i = 0;
			do {
				dataout = jtagm_scmd(MIPS_CTRL, irsz, cmd, drsz);
			} while ((dataout & DMA_START) && (++i < DMA_RETRIES));

			if ((dataout & DMA_ERROR) || (i >= DMA_RETRIES))
				error = -1;

			/* clear the dmaacc bit */
			dataout = jtagm_scmd(MIPS_CTRL, irsz, zeros, drsz);
		} else {
			/* scan in the address */
			dataout = jtagm_scmd(CHIPC_ADDR, irsz, addr, drsz);

			/* write the data */
			dataout = jtagm_scmd(CHIPC_DATA, irsz, write_data, drsz);

			/* set the dma start bit, size bit and read/write bit */
			dataout = jtagm_scmd(CHIPC_CTRL, irsz, (cmd << CCC_WR_SHIFT), drsz);

			/* wait for dma to complete */
			i = 0;
			do {
				dataout = jtagm_scmd((CHIPC_CTRL | CHIPC_RO), irsz, zeros, drsz);
			} while ((dataout & CCC_START) && (++i < DMA_RETRIES));

			if (i >= DMA_RETRIES)
				error = -1;
		}
	} else {
#ifdef BCMDRIVER
		return -1;
#else
		/* force to idle */
		jam_jtag_reset_idle();

		if (ejtag_mode == EJTAG_MIPS) {
			/* set the dmaacc bit */
			jam_jtag_irscan(MIPS_CTRL, &dataout);
			jam_jtag_drscan(cmd, &dataout);

			/* scan in the address */
			jam_jtag_irscan(MIPS_ADDR, &dataout);
			jam_jtag_drscan(addr, &dataout);

			/* write the data */
			jam_jtag_irscan(MIPS_DATA, &dataout);
			jam_jtag_drscan(write_data, &dataout);

			/* set the dma start bit, size bit and read/write bit */
			jam_jtag_irscan(MIPS_CTRL, &dataout);
			jam_jtag_drscan(cmd | DMA_START, &dataout);

			/* wait for dma to complete */
			i = 0;
			do {
				jam_jtag_irscan(MIPS_CTRL, &dataout);
				jam_jtag_drscan(cmd, &dataout);
			} while ((dataout & DMA_START) && (++i < DMA_RETRIES));

			if ((dataout & DMA_ERROR) || (i >= DMA_RETRIES))
				error = -1;

			/* clear the dmaacc bit */
			jam_jtag_irscan(MIPS_CTRL, &dataout);
			jam_jtag_drscan(zeros, &dataout);
		} else {
			/* scan in the address */
			jam_jtag_irscan(CHIPC_ADDR, &dataout);
			jam_jtag_drscan(addr, &dataout);

			/* write the data */
			jam_jtag_irscan(CHIPC_DATA, &dataout);
			jam_jtag_drscan(write_data, &dataout);

			/* set the dma start bit, size bit and read/write bit */
			jam_jtag_irscan(CHIPC_CTRL, &dataout);
			jam_jtag_drscan((cmd << CCC_WR_SHIFT), &dataout);

			/* wait for dma to complete */
			i = 0;
			do {
				jam_jtag_irscan(CHIPC_CTRL | CHIPC_RO, &dataout);
				jam_jtag_drscan(zeros, &dataout);
			} while ((dataout & CCC_START) && (++i < DMA_RETRIES));

			if (i >= DMA_RETRIES)
				error = -1;
		}
#endif /* BCMDRIVER */
	}

	return error;
}

#ifndef BCMDRIVER
int
ejtag_readreg(ulong instr, ulong *read_data)
{
	ulong zeros = 0x00000000;
	uint irsz = ir_size, drsz = dr_size;

	if (instr >= (1 << irsz))
		return -1;

	if (dongle == HND_JTAGM) {
		*read_data = jtagm_scmd(instr, irsz, zeros, drsz);
	} else {
		/* force to idle */
		jam_jtag_reset_idle();

		/* scan in instruction (ignore the read_data returned) */
		jam_jtag_irscan(instr, read_data);
		/* scan in the data writing zeroes */
		jam_jtag_drscan(zeros, read_data);
	}

	return 0;
}

int
ejtag_writereg(ulong instr, ulong write_data)
{
	ulong dataout;
	uint irsz = ir_size, drsz = dr_size;

	if (instr >= (1 << irsz))
		return -1;

	if (dongle == HND_JTAGM) {
		dataout = jtagm_scmd(instr, irsz, write_data, drsz);
	} else {
		/* force to idle */
		jam_jtag_reset_idle();

		/* scan in instruction (ignore the read_data returned) */
		jam_jtag_irscan(instr, &dataout);
		/* scan out the data */
		jam_jtag_drscan(write_data, &dataout);
	}

	return 0;
}

void
ejtag_reset(void)
{
	if (dongle == HND_JTAGM) {
		jtagm_wreg(CC_JTAGCMD, (JCMD_START |
		                        ((jtagm_crev == 10) ? JCMD0_ACC_RESET : JCMD_ACC_RESET)));
		while ((jtagm_rreg(CC_JTAGCMD) & JCMD_BUSY) == JCMD_BUSY) {
			/* usleep(1) */;
		}
	} else {
		/* force to idle */
		jam_jtag_reset_idle();
	}
}
#endif /* BCMDRIVER */
