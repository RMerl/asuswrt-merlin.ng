/*
   <:copyright-BRCM:2013:DUAL/GPL:standard
   
      Copyright (c) 2013 Broadcom 
      All Rights Reserved
   
   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:
   
      As a special exception, the copyright holders of this software give
      you permission to link this software with independent modules, and
      to copy and distribute the resulting executable under terms of your
      choice, provided that you also meet, for each linked independent
      module, the terms and conditions of the license of that module.
      An independent module is a module which is not derived from this
      software.  The special exception does not apply to any modifications
      of the software.
   
   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.
   
   :> 
*/ 
    /* *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  ARM Cortex A9 CPU definitions   File: arm.h
    *  Some definition based on ARM bare-metal example
    *  
    *  Author:  
    *  
    *********************************************************************  */

#ifndef	_ARM_H_
#define _ARM_H_


/* ARM defines */

#ifdef	__ASSEMBLER__

/*
 * LEAF - declare leaf routine
 */
#define LEAF(function)				\
		.section .text.function, "ax";	\
		.global	function;		\
		.func	function;		\
function:

#define THUMBLEAF(function)			\
		.section .text.function, "ax";	\
		.global	function;		\
		.func	function;		\
		.thumb;				\
		.thumb_func;			\
function:

/*
 * END - mark end of function
 */
#define END(function)				\
		.ltorg;				\
		.endfunc;			\
		.size	function, . - function

#define	DW(var, val)			\
	.global	var;			\
	.type	var, %object;		\
	.size	var, 4;			\
	.align	2;			\
var:	.word	val


#define _ULCAST_

#else

/*
 * The following macros are especially useful for __asm__
 * inline assembler.
 */
#ifndef __STR
#define __STR(x) #x
#endif
#ifndef STR
#define STR(x) __STR(x)
#endif

#define _ULCAST_ (unsigned long)

#endif	/* __ASSEMBLER__ */

/* Fields in cpsr */
#define	PS_USR		0x00000010		/* Mode: User */
#define	PS_FIQ		0x00000011		/* Mode: FIQ */
#define	PS_IRQ		0x00000012		/* Mode: IRQ */
#define	PS_SVC		0x00000013		/* Mode: Supervisor */
#define	PS_MON		0x00000016		/* Mode: Monitor */
#define	PS_ABT		0x00000017		/* Mode: Abort */
#define	PS_UND		0x0000001b		/* Mode: Undefined */
#define	PS_SYS		0x0000001f		/* Mode: System */
#define	PS_MM		0x0000001f		/* Mode bits mask */
#define	PS_T		0x00000020		/* Thumb mode */
#define	PS_F		0x00000040		/* FIQ disable */
#define	PS_I		0x00000080		/* IRQ disable */
#define	PS_A		0x00000100		/* Imprecise abort */
#define	PS_E		0x00000200		/* Endianess */
#define	PS_IT72		0x0000fc00		/* IT[7:2] */
#define	PS_GE		0x000f0000		/* IT[7:2] */
#define	PS_J		0x01000000		/* Java state */
#define	PS_IT10		0x06000000		/* IT[1:0] */
#define	PS_Q		0x08000000		/* Sticky overflow */
#define	PS_V		0x10000000		/* Overflow cc */
#define	PS_C		0x20000000		/* Carry cc */
#define	PS_Z		0x40000000		/* Zero cc */
#define	PS_N		0x80000000		/* Negative cc */

/* Trap types */
#define	TR_RST		0			/* Reset trap */
#define	TR_UND		1			/* Indefined instruction trap */
#define	TR_SWI		2			/* Software intrrupt */
#define	TR_IAB		3			/* Instruction fetch abort */
#define	TR_DAB		4			/* Data access abort */
#define	TR_BAD		5			/* Bad trap: Not used by ARM */
#define	TR_IRQ		6			/* Interrupt */
#define	TR_FIQ		7			/* Fast interrupt */

/*
 * CR1 bits (CP#15 CR1)
 */
#define CR_M	(1 << 0)	/* MMU enable				*/
#define CR_A	(1 << 1)	/* Alignment abort enable		*/
#define CR_C	(1 << 2)	/* Dcache enable			*/
#define CR_W	(1 << 3)	/* Write buffer enable			*/
#define CR_P	(1 << 4)	/* 32-bit exception handler		*/
#define CR_D	(1 << 5)	/* 32-bit data address range		*/
#define CR_L	(1 << 6)	/* Implementation defined		*/
#define CR_B	(1 << 7)	/* Big endian				*/
#define CR_S	(1 << 8)	/* System MMU protection		*/
#define CR_R	(1 << 9)	/* ROM MMU protection			*/
#define CR_F	(1 << 10)	/* Implementation defined		*/
#define CR_Z	(1 << 11)	/* Program Flow Prediction		*/
#define CR_I	(1 << 12)	/* Icache enable			*/
#define CR_V	(1 << 13)	/* Vectors relocated to 0xffff0000	*/
#define CR_RR	(1 << 14)	/* Round Robin cache replacement	*/
#define CR_L4	(1 << 15)	/* LDR pc can set T bit			*/
#define CR_DT	(1 << 16)
#define CR_IT	(1 << 18)
#define CR_ST	(1 << 19)
#define CR_FI	(1 << 21)	/* Fast interrupt (lower latency mode)	*/
#define CR_U	(1 << 22)	/* Unaligned access operation		*/
#define CR_XP	(1 << 23)	/* Extended page tables			*/
#define CR_VE	(1 << 24)	/* Vectored interrupts			*/
#define CR_EE	(1 << 25)	/* Exception (Big) Endian		*/
#define CR_TRE	(1 << 28)	/* TEX remap enable			*/
#define CR_AFE	(1 << 29)	/* Access flag enable			*/
#define CR_TE	(1 << 30)	/* Thumb exception enable		*/

#define isb() __asm__ __volatile__ ("isb")
#define nop() __asm__ __volatile__("mov\tr0,r0\t@ nop\n\t")

/* address mappping macros */
/* For ARM CFE, MMU setup the virtual address to be same as physical address.
   There are two sets of virtual address for DRAM memory that maps to the same
   DRAM phy. One is cached and the other is uncached where cached address is same
   as physical address and uncached address is cached address plus a fix offset.  
   This is to mimic the K0 K1 segment behavior in the MIPS to keep the cfe code 
   compatiable with ARM environment. 

   For current 63138 platform, DDR address start from physical address 0 and has up 
   1GB memory support.  So we define cached DDR virtual address same as physical 
   adddress and uncached DDR virtual address equals cached address plus 1GB. When 
   new ARM chip comes out with different memory size support and different hardware
   address space, we can customize this in part_map.h */

#define cache_to_uncache(va) ((va) | 0x40000000)
#define uncache_to_cache(va) ((va) & ~0x40000000)

/* There is no k0 and K1 conception in arm. Definition them here for code compatiblity only*/
#define PHYS_TO_K0(pa)	((pa))
#define PHYS_TO_K1(pa)	cache_to_uncache((pa))
#define K0_TO_PHYS(va)	((va))
#define K1_TO_PHYS(va)	uncache_to_cache((va))
#define K0_TO_K1(va)	cache_to_uncache((va))
#define K1_TO_K0(va)	uncache_to_cache((va))

#define VA_TO_PHYS_SIZE(va, ptr_size) \
  K0_TO_PHYS(va); \
  *ptr_size = mem_topofmem - va

/* Pieces of a CPU Id */
#define CID_IMPL	0xff000000		/* Implementor: 0x41 for ARM Ltd. */
#define CID_VARIANT	0x00f00000
#define CID_ARCH	0x000f0000
#define CID_PART	0x0000fff0
#define CID_REV		0x0000000f
#define CID_MASK	(CID_IMPL | CID_ARCH | CID_PART)

#endif	/* _ARM_H_ */
