/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#ifndef _SYSRESET_MPC83XX_H_
#define _SYSRESET_MPC83XX_H_

/*
 * String array for all possible event types; indexed by the EVENT field of the
 * AEATR register.
 */
static const char * const event[] = {
	"Address Time Out",
	"Data Time Out",
	"Address Only Transfer Type",
	"External Control Word Transfer Type",
	"Reserved Transfer Type",
	"Transfer Error",
	"reserved",
	"reserved"
};

/*
 * String array for all possible master IDs, which reflects the source of the
 * transaction that caused the error; indexed by the MSTR_ID field of the AEATR
 * register.
 */
static const char * const master[] = {
	"e300 Core Data Transaction",
	"reserved",
	"e300 Core Instruction Fetch",
	"reserved",
	"TSEC1",
	"TSEC2",
	"USB MPH",
	"USB DR",
	"Encryption Core",
	"I2C Boot Sequencer",
	"JTAG",
	"reserved",
	"eSDHC",
	"PCI1",
	"PCI2",
	"DMA",
	"QUICC Engine 00",
	"QUICC Engine 01",
	"QUICC Engine 10",
	"QUICC Engine 11",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"SATA1",
	"SATA2",
	"SATA3",
	"SATA4",
	"reserved",
	"PCI Express 1",
	"PCI Express 2",
	"TDM-DMAC"
};

/*
 * String array for all possible transfer types; indexed by the TTYPE field of
 * the AEATR register.
 */
static const char * const transfer[] = {
	"Address-only, Clean Block",
	"Address-only, lwarx reservation set",
	"Single-beat or Burst write",
	"reserved",
	"Address-only, Flush Block",
	"reserved",
	"Burst write",
	"reserved",
	"Address-only, sync",
	"Address-only, tlbsync",
	"Single-beat or Burst read",
	"Single-beat or Burst read",
	"Address-only, Kill Block",
	"Address-only, icbi",
	"Burst read",
	"reserved",
	"Address-only, eieio",
	"reserved",
	"Single-beat write",
	"reserved",
	"ecowx - Illegal single-beat write",
	"reserved",
	"reserved",
	"reserved",
	"Address-only, TLB Invalidate",
	"reserved",
	"Single-beat or Burst read",
	"reserved",
	"eciwx - Illegal single-beat read",
	"reserved",
	"Burst read",
	"reserved"
};
#endif /* _SYSRESET_MPC83XX_H_ */
