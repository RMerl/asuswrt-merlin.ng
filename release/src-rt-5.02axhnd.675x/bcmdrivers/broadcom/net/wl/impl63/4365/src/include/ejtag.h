/*
 * ejtag access interface
 *
 * Copyright 2004 Broadcom Corporation
 *
 * $Id: ejtag.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef INC_EJTAG_H
#define INC_EJTAG_H

/* type of jtag jig */
#define ALTERA	0
#define ALTERNATIVE	1
#define XILINX	2
#define HND_JTAGM	3
extern int dongle;

/* global indicating endianness of target */
extern bool ejtag_bigend;

/* global indicating mode of target access */
#define	EJTAG_MIPS	1
#define	EJTAG_CHIPC	2
extern int ejtag_mode;

/* Instruction & data register sizes */
#define DEF_DATA_SIZE	32		/* Default DR size */
#define DEF_INST_SIZE	8		/* Default IR size */
extern uint dr_size;
extern uint ir_size;

/* Tracing of jtag signals and/or register accesses */
#define	JTAG_TRACE_REGS		1
#define	JTAG_TRACE_SIGNALS	2
extern int jtag_trace;

/* Ejtag function prototypes */
#ifdef BCMDRIVER
extern int ejtag_init(uint16 devid, uint32 sbidh, void *regsva, bool bendian);
extern void ejtag_cleanup(void);
#else
extern void initialize_jtag_hardware(bool remote);
extern void close_jtag_hardware(void);

extern int ejtag_writereg(ulong instr, ulong write_data);
extern int ejtag_readreg(ulong instr, ulong *read_data);

extern void ejtag_reset(void);
#endif // endif

extern int write_ejtag(ulong addr, ulong write_data, uint size);
extern int read_ejtag(ulong addr, ulong *read_data, uint size);

#endif /* INC_EJTAG_H */
