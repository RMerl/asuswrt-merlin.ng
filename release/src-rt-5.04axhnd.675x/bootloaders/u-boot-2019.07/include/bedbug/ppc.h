/* $Id$ */

#ifndef _PPC_H
#define _PPC_H

/*======================================================================
 *
 *  OPERANDS
 *
 *======================================================================*/

enum OP_FIELD {
  O_AA = 1, O_BD, O_BI, O_BO, O_crbD, O_crbA, O_crbB, O_CRM, O_d, O_frC, O_frD,
  O_frS, O_IMM, O_LI, O_LK, O_MB, O_ME, O_NB, O_OE, O_rA, O_rB, O_Rc, O_rD,
  O_rS, O_SH, O_SIMM, O_SR, O_TO, O_UIMM, O_crfD, O_crfS, O_L, O_spr, O_tbr,
  O_cr2 };

struct operand {
  enum OP_FIELD	field;		/* The operand identifier from the
				   enum above */

  char *	name;		/* Symbolic name of this operand */

  unsigned int	bits;		/* The number of bits used by this
				   operand */

  unsigned int	shift;		/* How far to the right the operand
				   should be shifted so that it is
				   aligned at the beginning of the
				   word */

  unsigned int	hint;		/* A bitwise-inclusive-OR of the
				   values shown below.  These are used
				   tell the disassembler how to print
				   this operand */
};

/* Values for operand hint */
#define OH_SILENT	0x01	/* dont print this operand */
#define OH_ADDR		0x02	/* this operand is an address */
#define OH_REG		0x04	/* this operand is a register */
#define OH_SPR		0x08	/* this operand is an SPR */
#define OH_TBR		0x10	/* this operand is a TBR */
#define OH_OFFSET	0x20	/* this operand is an offset */
#define OH_LITERAL      0x40    /* a literal string */


/*======================================================================
 *
 *  OPCODES
 *
 *======================================================================*/

/* From the MPCxxx instruction set documentation, all instructions are
 * 32 bits long and word aligned.  Bits 0-5 always specify the primary
 * opcode.  Many instructions also have an extended opcode.
 */

#define GET_OPCD(i) (((unsigned long)(i) >> 26) & 0x3f)
#define MAKE_OPCODE(i) ((((unsigned long)(i)) & 0x3f) << 26)

/* The MPC860 User's Manual, Appendix D.4 contains the definitions of the
 * instruction forms
 */


/*-------------------------------------------------
 *              I-Form Instructions:
 * bX
 *-------------------------------------------------
 * OPCD |           LI                       |AA|LK
 *-------------------------------------------------*/

#define I_OPCODE(i,aa,lk) (MAKE_OPCODE(i) | (((aa) & 0x1) << 1) | ((lk) & 0x1))
#define I_MASK I_OPCODE(0x3f,0x1,0x1)


/*-------------------------------------------------
 *              B-Form Instructions:
 * bcX
 *-------------------------------------------------
 * OPCD |    BO   |  BI  |   BD              |AA|LK
 *-------------------------------------------------*/

#define B_OPCODE(i,aa,lk) (MAKE_OPCODE(i) | (((aa) & 0x1) << 1) | ((lk) & 0x1))
#define B_MASK B_OPCODE(0x3f,0x1,0x1)


/*-------------------------------------------------
 *             SC-Form Instructions:
 * sc
 *-------------------------------------------------
 * OPCD | 00000 | 00000 | 00000000000000       |1|0
 *-------------------------------------------------*/

#define SC_OPCODE(i) (MAKE_OPCODE(i) | 0x2)
#define SC_MASK SC_OPCODE(0x3f)


/*-------------------------------------------------
 *             D-Form Instructions:
 * addi addic addic. addis andi. andis. cmpi cmpli
 * lbz lbzu lha lhau lhz lhzu lmw lwz lwzu mulli
 * ori oris stb stbu sth sthu stmw stw stwu subfic
 * twi xori xoris
 *-------------------------------------------------
 * OPCD |   D    |   A   |            d
 * OPCD |   D    |   A   |           SIMM
 * OPCD |   S    |   A   |            d
 * OPCD |   S    |   A   |           UIMM
 * OPCD |crfD|0|L|   A   |           SIMM
 * OPCD |crfD|0|L|   A   |           UIMM
 * OPCD |   TO   |   A   |           SIMM
 *-------------------------------------------------*/

#define D_OPCODE(i) MAKE_OPCODE(i)
#define D_MASK MAKE_OPCODE(0x3f)


/*-------------------------------------------------
 *            DS-Form Instructions:
 * (none supported by MPC860)
 *-------------------------------------------------
 * OPCD |   D    |   A   |          ds          |XO
 * OPCD |   S    |   A   |          ds          |XO
 *-------------------------------------------------*/

#define DS_OPCODE(i,xo) (MAKE_OPCODE(i) | ((xo) & 0x3))
#define DS_MASK DS_OPCODE(0x3f,0x1)


/*---------------------------------------------------
 *            X-Form Instructions:
 * andX andcX cmp cmpl cntlzwX dcbf dcbi dcbst dcbt
 * dcbtst dcbz eciwx ecowx eieio eqvX extsbX extshX
 * icbi lbzux lbxz lhaux lhax lhbrx lhzux lhxz lswi
 * lswx lwarx lwbrx lwzux lwxz mcrfs mcrxr mfcr
 * mfmsr mfsr mfsrin mtmsr mtsr mtsrin nandX norX
 * orX orcX slwX srawX srawiX srwX stbux stbx
 * sthbrx sthuxsthx stswi stswx stwbrx stwcx. stwux
 * stwx sync tlbie tlbld tlbli tlbsync tw xorX
 *---------------------------------------------------
 * OPCD |   D    |    A   |    B   |      XO      |0
 * OPCD |   D    |    A   |   NB   |      XO      |0
 * OPCD |   D    |  00000 |    B   |      XO      |0
 * OPCD |   D    |  00000 |  00000 |      XO      |0
 * OPCD |   D    |0|  SR  |  00000 |      XO      |0
 * OPCD |   S    |    A   |    B   |      XO      |Rc
 * OPCD |   S    |    A   |    B   |      XO      |1
 * OPCD |   S    |    A   |    B   |      XO      |0
 * OPCD |   S    |    A   |   NB   |      XO      |0
 * OPCD |   S    |    A   |  00000 |      XO      |Rc
 * OPCD |   S    |  00000 |    B   |      XO      |0
 * OPCD |   S    |  00000 |  00000 |      XO      |0
 * OPCD |   S    |0|  SR  |  00000 |      XO      |0
 * OPCD |   S    |    A   |   SH   |      XO      |Rc
 * OPCD |crfD|0|L|    A   |   SH   |      XO      |0
 * OPCD |crfD |00|    A   |    B   |      XO      |0
 * OPCD |crfD |00|crfS |00|  00000 |      XO      |0
 * OPCD |crfD |00|  00000 |  00000 |      XO      |0
 * OPCD |crfD |00|  00000 | IMM  |0|      XO      |Rc
 * OPCD |   TO   |    A   |    B   |      XO      |0
 * OPCD |   D    |  00000 |    B   |      XO      |Rc
 * OPCD |   D    |  00000 |  00000 |      XO      |Rc
 * OPCD |  crbD  |  00000 |  00000 |      XO      |Rc
 * OPCD |  00000 |    A   |    B   |      XO      |0
 * OPCD |  00000 |  00000 |    B   |      XO      |0
 * OPCD |  00000 |  00000 |  00000 |      XO      |0
 *---------------------------------------------------*/

#define X_OPCODE(i,xo,rc) (MAKE_OPCODE(i) | (((xo) & 0x3ff) << 1) | \
			   ((rc) & 0x1))
#define X_MASK X_OPCODE(0x3f,0x3ff,0x1)


/*---------------------------------------------------
 *            XL-Form Instructions:
 * bcctrX bclrX crand crandc creqv crnand crnor cror
 * croc crxorisync mcrf rfi
 *---------------------------------------------------
 * OPCD |   BO   |  BI    |  00000 |      XO      |LK
 * OPCD |  crbD  | crbA   |  crbB  |      XO      |0
 * OPCD |crfD |00|crfS |00|  00000 |      XO      |0
 * OPCD |  00000 |  00000 |  00000 |      XO      |0
 *---------------------------------------------------*/

#define XL_OPCODE(i,xo,lk) (MAKE_OPCODE(i) | (((xo) & 0x3ff) << 1) | \
			    ((lk) & 0x1))
#define XL_MASK XL_OPCODE(0x3f,0x3ff,0x1)


/*---------------------------------------------------
 *            XFX-Form Instructions:
 * mfspr mftb mtcrf mtspr
 *---------------------------------------------------
 * OPCD |   D    |      spr        |      XO       |0
 * OPCD |   D    |0|    CRM      |0|      XO       |0
 * OPCD |   S    |      spr        |      XO       |0
 * OPCD |   D    |      tbr        |      XO       |0
 *---------------------------------------------------*/

#define XFX_OPCODE(i,xo,rc) (MAKE_OPCODE(i) | (((xo) & 0x3ff) << 1) | \
			     ((rc) & 0x1))
#define XFX_MASK XFX_OPCODE(0x3f,0x3ff,0x1)


/*---------------------------------------------------
 *            XFL-Form Instructions:
 * (none supported by MPC860)
 *---------------------------------------------------
 * OPCD |0|      FM     |0|   B    |      XO       |0
 *---------------------------------------------------*/

#define XFL_OPCODE(i,xo,rc) (MAKE_OPCODE(i) | (((xo) & 0x3ff) << 1) | \
			     ((rc) & 0x1))
#define XFL_MASK XFL_OPCODE(0x3f,0x3ff,0x1)


/*---------------------------------------------------
 *            XS-Form Instructions:
 * (none supported by MPC860)
 *---------------------------------------------------
 * OPCD |    S   |   A    |   sh   |      XO   |sh|LK
 *---------------------------------------------------*/

#define XS_OPCODE(i,xo,rc) (MAKE_OPCODE(i) | (((xo) & 0x1ff) << 2) | \
			     ((rc) & 0x1))
#define XS_MASK XS_OPCODE(0x3f,0x1ff,0x1)


/*---------------------------------------------------
 *            XO-Form Instructions:
 * addX addcXaddeX addmeX addzeX divwX divwuX mulhwX
 * mulhwuX mullwX negX subfX subfcX subfeX subfmeX
 * subfzeX
 *---------------------------------------------------
 * OPCD |    D   |   A    |    B   |OE|     XO    |Rc
 * OPCD |    D   |   A    |    B   |0 |     XO    |Rc
 * OPCD |    D   |   A    |  00000 |OE|     XO    |Rc
 *---------------------------------------------------*/

#define XO_OPCODE(i,xo,oe,rc) (MAKE_OPCODE(i) | (((oe) & 0x1) << 10) | \
			       (((xo) & 0x1ff) << 1) | ((rc) & 0x1))
#define XO_MASK XO_OPCODE(0x3f,0x1ff,0x1,0x1)


/*---------------------------------------------------
 *            A-Form Instructions:
 * (none supported by MPC860)
 *---------------------------------------------------
 * OPCD |    D   |   A    |    B   |00000|  XO    |Rc
 * OPCD |    D   |   A    |    B   |  C  |  XO    |Rc
 * OPCD |    D   |   A    |  00000 |  C  |  XO    |Rc
 * OPCD |    D   |  00000 |    B   |00000|  XO    |Rc
 *---------------------------------------------------*/

#define A_OPCODE(i,xo,rc) (MAKE_OPCODE(i) | (((xo) & 0x1f) << 1) | \
			   ((rc) & 0x1))
#define A_MASK A_OPCODE(0x3f,0x1f,0x1)


/*---------------------------------------------------
 *            M-Form Instructions:
 * rlwimiX rlwinmX rlwnmX
 *---------------------------------------------------
 * OPCD |    S   |   A    |    SH   |  MB |  ME   |Rc
 * OPCD |    S   |   A    |     B   |  MB |  ME   |Rc
 *---------------------------------------------------*/

#define M_OPCODE(i,rc) (MAKE_OPCODE(i) | ((rc) & 0x1))
#define M_MASK M_OPCODE(0x3f,0x1)


/*---------------------------------------------------
 *            MD-Form Instructions:
 * (none supported by MPC860)
 *---------------------------------------------------
 * OPCD |    S   |   A    |    sh   |  mb | XO |sh|Rc
 * OPCD |    S   |   A    |    sh   |  me | XO |sh|Rc
 *---------------------------------------------------*/

#define MD_OPCODE(i,xo,rc) (MAKE_OPCODE(i) | (((xo) & 0x7) << 2) | \
			   ((rc) & 0x1))
#define MD_MASK MD_OPCODE(0x3f,0x7,0x1)


/*---------------------------------------------------
 *            MDS-Form Instructions:
 * (none supported by MPC860)
 *---------------------------------------------------
 * OPCD |    S   |   A    |    B    |  mb | XO    |Rc
 * OPCD |    S   |   A    |    B    |  me | XO    |Rc
 *---------------------------------------------------*/

#define MDS_OPCODE(i,xo,rc) (MAKE_OPCODE(i) | (((xo) & 0xf) << 1) | \
			   ((rc) & 0x1))
#define MDS_MASK MDS_OPCODE(0x3f,0xf,0x1)

#define INSTRUCTION( memaddr ) ntohl(*(unsigned long *)(memaddr))

#define MAX_OPERANDS  8

struct ppc_ctx;

struct opcode {
  unsigned long	opcode;		/* The complete opcode as produced by
				   one of the XXX_OPCODE macros above */

  unsigned long	mask;		/* The mask to use on an instruction
				   before comparing with the opcode
				   field to see if it matches */

  enum OP_FIELD	fields[MAX_OPERANDS];
				/* An array defining the operands for
				   this opcode.  The values of the
				   array are the operand identifiers */

  int (*hfunc)(struct ppc_ctx *);
				/* Address of a function to handle the given
				   mnemonic */

  char *	name;		/* The symbolic name of this opcode */

  unsigned int	hint;		/* A bitwise-inclusive-OR of the
				   values shown below.  These are used
				   tell the disassembler how to print
				   some operands for this opcode */
};

/* values for opcode hints */
#define H_RELATIVE	0x1	/* The address operand is relative */
#define H_IMM_HIGH	0x2	/* [U|S]IMM field shifted high */
#define H_RA0_IS_0	0x4	/* If rA = 0 then treat as literal 0 */

struct ppc_ctx {
  struct opcode *	op;
  unsigned long		instr;
  unsigned int		flags;
  int			datalen;
  char			data[ 256 ];
  char			radix_fmt[ 8 ];
  unsigned char *	virtual;
};


/*======================================================================
 *
 *  FUNCTIONS
 *
 *======================================================================*/

/* Values for flags as passed to various ppc routines */
#define F_RADOCTAL	0x1	/* output radix = unsigned octal */
#define F_RADUDECIMAL	0x2	/* output radix = unsigned decimal */
#define F_RADSDECIMAL	0x4	/* output radix = signed decimal */
#define F_RADHEX	0x8	/* output radix = unsigned hex */
#define F_SIMPLE	0x10	/* use simplified mnemonics */
#define F_SYMBOL	0x20	/* use symbol lookups for addresses */
#define F_INSTR		0x40	/* output the raw instruction */
#define F_LOCALMEM	0x80	/* retrieve opcodes from local memory
				   rather than from the HMI */
#define F_LINENO	0x100	/* show line number info if available */
#define F_VALIDONLY	0x200	/* cache: valid entries only */

/* Values for assembler error codes */
#define E_ASM_BAD_OPCODE	1
#define E_ASM_NUM_OPERANDS	2
#define E_ASM_BAD_REGISTER	3
#define E_ASM_BAD_SPR		4
#define E_ASM_BAD_TBR		5

extern int disppc __P((unsigned char *,unsigned char *,int,
		       int (*)(const char *), unsigned long));
extern int print_source_line __P((char *,char *,int,
				  int (*pfunc)(const char *)));
extern int find_next_address __P((unsigned char *,int,struct pt_regs *));
extern int handle_bc __P((struct ppc_ctx *));
extern unsigned long asmppc __P((unsigned long,char*,int*));
extern char *asm_error_str __P((int));

/*======================================================================
 *
 *  GLOBAL VARIABLES
 *
 *======================================================================*/

extern struct operand operands[];
extern const unsigned int n_operands;
extern struct opcode opcodes[];
extern const unsigned int n_opcodes;

#endif /* _PPC_H */


/*
 * Copyright (c) 2000 William L. Pitts and W. Gerald Hicks
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are freely
 * permitted provided that the above copyright notice and this
 * paragraph and the following disclaimer are duplicated in all
 * such forms.
 *
 * This software is provided "AS IS" and without any express or
 * implied warranties, including, without limitation, the implied
 * warranties of merchantability and fitness for a particular
 * purpose.
 */
