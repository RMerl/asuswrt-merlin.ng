/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */
#ifndef _CPU_ASM_H
#define _CPU_ASM_H

#define BIT_C				0x00000001

#define OP_BLR				0x4e800020
#define OP_EXTSB			0x7c000774
#define OP_EXTSH			0x7c000734
#define OP_NEG				0x7c0000d0
#define OP_CNTLZW			0x7c000034
#define OP_ADD				0x7c000214
#define OP_ADDC				0x7c000014
#define OP_ADDME			0x7c0001d4
#define OP_ADDZE			0x7c000194
#define OP_ADDE				0x7c000114
#define OP_ADDI				0x38000000
#define OP_SUBF				0x7c000050
#define OP_SUBFC			0x7c000010
#define OP_SUBFE			0x7c000110
#define OP_SUBFME			0x7c0001d0
#define OP_SUBFZE			0x7c000190
#define OP_MFCR				0x7c000026
#define OP_MTCR				0x7c0ff120
#define OP_MFXER			0x7c0102a6
#define OP_MTXER			0x7c0103a6
#define OP_MCRXR			0x7c000400
#define OP_MCRF				0x4c000000
#define OP_CRAND			0x4c000202
#define OP_CRANDC			0x4c000102
#define OP_CROR				0x4c000382
#define OP_CRORC			0x4c000342
#define OP_CRXOR			0x4c000182
#define OP_CRNAND			0x4c0001c2
#define OP_CRNOR			0x4c000042
#define OP_CREQV			0x4c000242
#define OP_CMPW				0x7c000000
#define OP_CMPLW			0x7c000040
#define OP_CMPWI			0x2c000000
#define OP_CMPLWI			0x28000000
#define OP_MULLW			0x7c0001d6
#define OP_MULHW			0x7c000096
#define OP_MULHWU			0x7c000016
#define OP_DIVW				0x7c0003d6
#define OP_DIVWU			0x7c000396
#define OP_OR				0x7c000378
#define OP_ORC				0x7c000338
#define OP_XOR				0x7c000278
#define OP_NAND				0x7c0003b8
#define OP_NOR				0x7c0000f8
#define OP_EQV				0x7c000238
#define OP_SLW				0x7c000030
#define OP_SRW				0x7c000430
#define OP_SRAW				0x7c000630
#define OP_ORI				0x60000000
#define OP_ORIS				0x64000000
#define OP_XORI				0x68000000
#define OP_XORIS			0x6c000000
#define OP_ANDI_			0x70000000
#define OP_ANDIS_			0x74000000
#define OP_SRAWI			0x7c000670
#define OP_RLWINM			0x54000000
#define OP_RLWNM			0x5c000000
#define OP_RLWIMI			0x50000000
#define OP_LWZ				0x80000000
#define OP_LHZ				0xa0000000
#define OP_LHA				0xa8000000
#define OP_LBZ				0x88000000
#define OP_LWZU				0x84000000
#define OP_LHZU				0xa4000000
#define OP_LHAU				0xac000000
#define OP_LBZU				0x8c000000
#define OP_LWZX				0x7c00002e
#define OP_LHZX				0x7c00022e
#define OP_LHAX				0x7c0002ae
#define OP_LBZX				0x7c0000ae
#define OP_LWZUX			0x7c00006e
#define OP_LHZUX			0x7c00026e
#define OP_LHAUX			0x7c0002ee
#define OP_LBZUX			0x7c0000ee
#define OP_STW				0x90000000
#define OP_STH				0xb0000000
#define OP_STB				0x98000000
#define OP_STWU				0x94000000
#define OP_STHU				0xb4000000
#define OP_STBU				0x9c000000
#define OP_STWX				0x7c00012e
#define OP_STHX				0x7c00032e
#define OP_STBX				0x7c0001ae
#define OP_STWUX			0x7c00016e
#define OP_STHUX			0x7c00036e
#define OP_STBUX			0x7c0001ee
#define OP_B				0x48000000
#define OP_BL				0x48000001
#define OP_BC				0x40000000
#define OP_BCL				0x40000001
#define OP_MTLR				0x7c0803a6
#define OP_MFLR				0x7c0802a6
#define OP_MTCTR			0x7c0903a6
#define OP_MFCTR			0x7c0902a6
#define OP_LMW				0xb8000000
#define OP_STMW				0xbc000000
#define OP_LSWI				0x7c0004aa
#define OP_LSWX				0x7c00042a
#define OP_STSWI			0x7c0005aa
#define OP_STSWX			0x7c00052a

#define ASM_0(opcode)			(opcode)
#define ASM_1(opcode, rd)		((opcode) +		\
					 ((rd) << 21))
#define ASM_1C(opcode, cr)		((opcode) +		\
					 ((cr) << 23))
#define ASM_11(opcode, rd, rs)		((opcode) +		\
					 ((rd) << 21) +		\
					 ((rs) << 16))
#define ASM_11C(opcode, cd, cs)		((opcode) +		\
					 ((cd) << 23) +		\
					 ((cs) << 18))
#define ASM_11X(opcode, rd, rs)		((opcode) +		\
					 ((rs) << 21) +		\
					 ((rd) << 16))
#define ASM_11I(opcode, rd, rs, simm)	((opcode) +		\
					 ((rd) << 21) +		\
					 ((rs) << 16) +		\
					 ((simm) & 0xffff))
#define ASM_11IF(opcode, rd, rs, simm)	((opcode) +		\
					 ((rd) << 21) +		\
					 ((rs) << 16) +		\
					 ((simm) << 11))
#define ASM_11S(opcode, rd, rs, sh)	((opcode) +		\
					 ((rs) << 21) +		\
					 ((rd) << 16) +		\
					 ((sh) << 11))
#define ASM_11IX(opcode, rd, rs, imm)	((opcode) +		\
					 ((rs) << 21) +		\
					 ((rd) << 16) +		\
					 ((imm) & 0xffff))
#define ASM_12(opcode, rd, rs1, rs2)	((opcode) +		\
					 ((rd) << 21) +		\
					 ((rs1) << 16) +	\
					 ((rs2) << 11))
#define ASM_12F(opcode, fd, fs1, fs2)	((opcode) +		\
					 ((fd) << 21) +		\
					 ((fs1) << 16) +	\
					 ((fs2) << 11))
#define ASM_12X(opcode, rd, rs1, rs2)	((opcode) +		\
					 ((rs1) << 21) +	\
					 ((rd) << 16) +		\
					 ((rs2) << 11))
#define ASM_2C(opcode, cr, rs1, rs2)	((opcode) +		\
					 ((cr) << 23) +		\
					 ((rs1) << 16) +	\
					 ((rs2) << 11))
#define ASM_1IC(opcode, cr, rs, imm)	((opcode) +		\
					 ((cr) << 23) +		\
					 ((rs) << 16) +		\
					 ((imm) & 0xffff))
#define ASM_122(opcode, rd, rs1, rs2, imm1, imm2)		\
					((opcode) +		\
					 ((rs1) << 21) +	\
					 ((rd) << 16) +		\
					 ((rs2) << 11) +	\
					 ((imm1) << 6) +	\
					 ((imm2) << 1))
#define ASM_113(opcode, rd, rs, imm1, imm2, imm3)		\
					((opcode) +		\
					 ((rs) << 21) +		\
					 ((rd) << 16) +		\
					 ((imm1) << 11) +	\
					 ((imm2) << 6) +	\
					 ((imm3) << 1))
#define ASM_1O(opcode, off)		((opcode) + (off))
#define ASM_3O(opcode, bo, bi, off)	((opcode) +		\
					 ((bo) << 21) +		\
					 ((bi) << 16) +		\
					 (off))

#define ASM_ADDI(rd, rs, simm)		ASM_11I(OP_ADDI, rd, rs, simm)
#define ASM_BLR				ASM_0(OP_BLR)
#define ASM_STW(rd, rs, simm)		ASM_11I(OP_STW, rd, rs, simm)
#define ASM_LWZ(rd, rs, simm)		ASM_11I(OP_LWZ, rd, rs, simm)
#define ASM_MFCR(rd)			ASM_1(OP_MFCR, rd)
#define ASM_MTCR(rd)			ASM_1(OP_MTCR, rd)
#define ASM_MFXER(rd)			ASM_1(OP_MFXER, rd)
#define ASM_MTXER(rd)			ASM_1(OP_MTXER, rd)
#define ASM_MFCTR(rd)			ASM_1(OP_MFCTR, rd)
#define ASM_MTCTR(rd)			ASM_1(OP_MTCTR, rd)
#define ASM_MCRXR(cr)			ASM_1C(OP_MCRXR, cr)
#define ASM_MCRF(cd, cs)		ASM_11C(OP_MCRF, cd, cs)
#define ASM_B(off)			ASM_1O(OP_B, off)
#define ASM_BL(off)			ASM_1O(OP_BL, off)
#define ASM_MFLR(rd)			ASM_1(OP_MFLR, rd)
#define ASM_MTLR(rd)			ASM_1(OP_MTLR, rd)
#define ASM_LI(rd, imm)			ASM_ADDI(rd, 0, imm)
#define ASM_LMW(rd, rs, simm)		ASM_11I(OP_LMW, rd, rs, simm)
#define ASM_STMW(rd, rs, simm)		ASM_11I(OP_STMW, rd, rs, simm)
#define ASM_LSWI(rd, rs, simm)		ASM_11IF(OP_LSWI, rd, rs, simm)
#define ASM_LSWX(rd, rs1, rs2)		ASM_12(OP_LSWX, rd, rs1, rs2)
#define ASM_STSWI(rd, rs, simm)		ASM_11IF(OP_STSWI, rd, rs, simm)
#define ASM_STSWX(rd, rs1, rs2)		ASM_12(OP_STSWX, rd, rs1, rs2)


#endif /* _CPU_ASM_H */
