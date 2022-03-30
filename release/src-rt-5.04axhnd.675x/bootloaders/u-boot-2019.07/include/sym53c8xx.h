/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * Most of these definitions are derived from
 * linux/drivers/scsi/sym53c8xx_defs.h
 */

#ifndef _SYM53C8XX_DEFS_H
#define _SYM53C8XX_DEFS_H


#define SCNTL0		0x00    /* full arb., ena parity, par->ATN  */

#define SCNTL1		0x01    /* no reset                         */
  #define   ISCON   0x10  /* connected to scsi						*/
  #define   CRST    0x08  /* force reset                      */
  #define   IARB    0x02  /* immediate arbitration            */

#define SCNTL2		0x02    /* no disconnect expected           */
	#define   SDU     0x80  /* cmd: disconnect will raise error */
	#define   CHM     0x40  /* sta: chained mode                */
	#define   WSS     0x08  /* sta: wide scsi send           [W]*/
	#define   WSR     0x01  /* sta: wide scsi received       [W]*/

#define SCNTL3		0x03    /* cnf system clock dependent       */
	#define   EWS     0x08  /* cmd: enable wide scsi         [W]*/
	#define   ULTRA   0x80  /* cmd: ULTRA enable                */
				/* bits 0-2, 7 rsvd for C1010       */

#define SCID			0x04		/* cnf host adapter scsi address    */
	#define   RRE     0x40  /* r/w:e enable response to resel.  */
	#define   SRE     0x20  /* r/w:e enable response to select  */

#define SXFER			0x05		/* ### Sync speed and count         */
				/* bits 6-7 rsvd for C1010          */

#define SDID			0x06	/* ### Destination-ID               */

#define GPREG			0x07	/* ??? IO-Pins                      */

#define SFBR			0x08	/* ### First byte in phase          */

#define SOCL			0x09
	#define   CREQ	  0x80	/* r/w: SCSI-REQ                    */
	#define   CACK	  0x40	/* r/w: SCSI-ACK                    */
	#define   CBSY	  0x20	/* r/w: SCSI-BSY                    */
	#define   CSEL	  0x10	/* r/w: SCSI-SEL                    */
	#define   CATN	  0x08	/* r/w: SCSI-ATN                    */
	#define   CMSG	  0x04	/* r/w: SCSI-MSG                    */
	#define   CC_D	  0x02	/* r/w: SCSI-C_D                    */
	#define   CI_O	  0x01	/* r/w: SCSI-I_O                    */

#define SSID			0x0a

#define SBCL			0x0b

#define DSTAT			0x0c
  #define   DFE     0x80  /* sta: dma fifo empty              */
  #define   MDPE    0x40  /* int: master data parity error    */
  #define   BF      0x20  /* int: script: bus fault           */
  #define   ABRT    0x10  /* int: script: command aborted     */
  #define   SSI     0x08  /* int: script: single step         */
  #define   SIR     0x04  /* int: script: interrupt instruct. */
  #define   IID     0x01  /* int: script: illegal instruct.   */

#define SSTAT0		0x0d
  #define   ILF     0x80  /* sta: data in SIDL register lsb   */
  #define   ORF     0x40  /* sta: data in SODR register lsb   */
  #define   OLF     0x20  /* sta: data in SODL register lsb   */
  #define   AIP     0x10  /* sta: arbitration in progress     */
  #define   LOA     0x08  /* sta: arbitration lost            */
  #define   WOA     0x04  /* sta: arbitration won             */
  #define   IRST    0x02  /* sta: scsi reset signal           */
  #define   SDP     0x01  /* sta: scsi parity signal          */

#define SSTAT1		0x0e
	#define   FF3210  0xf0	/* sta: bytes in the scsi fifo      */

#define SSTAT2		0x0f
  #define   ILF1    0x80  /* sta: data in SIDL register msb[W]*/
  #define   ORF1    0x40  /* sta: data in SODR register msb[W]*/
  #define   OLF1    0x20  /* sta: data in SODL register msb[W]*/
  #define   DM      0x04  /* sta: DIFFSENS mismatch (895/6 only) */
  #define   LDSC    0x02  /* sta: disconnect & reconnect      */

#define DSA				0x10		/* --> Base page                    */
#define DSA1			0x11
#define DSA2			0x12
#define DSA3			0x13

#define ISTAT			0x14	/* --> Main Command and status      */
  #define   CABRT   0x80  /* cmd: abort current operation     */
  #define   SRST    0x40  /* mod: reset chip                  */
  #define   SIGP    0x20  /* r/w: message from host to ncr    */
  #define   SEM     0x10  /* r/w: message between host + ncr  */
  #define   CON     0x08  /* sta: connected to scsi           */
  #define   INTF    0x04  /* sta: int on the fly (reset by wr)*/
  #define   SIP     0x02  /* sta: scsi-interrupt              */
  #define   DIP     0x01  /* sta: host/script interrupt       */


#define CTEST0		0x18
#define CTEST1		0x19
#define CTEST2		0x1a
	#define   CSIGP   0x40
				/* bits 0-2,7 rsvd for C1010        */

#define CTEST3		0x1b
	#define   FLF     0x08  /* cmd: flush dma fifo              */
	#define   CLF		0x04	/* cmd: clear dma fifo		    */
	#define   FM      0x02  /* mod: fetch pin mode              */
	#define   WRIE    0x01  /* mod: write and invalidate enable */
				/* bits 4-7 rsvd for C1010          */

#define DFIFO			0x20
#define CTEST4		0x21
	#define   BDIS    0x80  /* mod: burst disable               */
	#define   MPEE    0x08  /* mod: master parity error enable  */

#define CTEST5		0x22
	#define   DFS     0x20  /* mod: dma fifo size               */
				/* bits 0-1, 3-7 rsvd for C1010          */
#define CTEST6		0x23

#define DBC				0x24	/* ### Byte count and command       */
#define DNAD			0x28	/* ### Next command register        */
#define DSP				0x2c	/* --> Script Pointer               */
#define DSPS			0x30	/* --> Script pointer save/opcode#2 */

#define SCRATCHA	0x34  /* Temporary register a            */
#define SCRATCHA1	0x35
#define SCRATCHA2	0x36
#define SCRATCHA3	0x37

#define DMODE			0x38
	#define   BL_2    0x80  /* mod: burst length shift value +2 */
	#define   BL_1    0x40  /* mod: burst length shift value +1 */
	#define   ERL     0x08  /* mod: enable read line            */
	#define   ERMP    0x04  /* mod: enable read multiple        */
	#define   BOF     0x02  /* mod: burst op code fetch         */
	#define   MAN     0x01  /* mod: manual start				         */

#define DIEN		0x39
#define SBR			0x3a

#define DCNTL		0x3b			/* --> Script execution control     */
	#define   CLSE    0x80  /* mod: cache line size enable      */
	#define   PFF     0x40  /* cmd: pre-fetch flush             */
	#define   PFEN    0x20  /* mod: pre-fetch enable            */
	#define   SSM     0x10  /* mod: single step mode            */
	#define   IRQM    0x08  /* mod: irq mode (1 = totem pole !) */
	#define   STD     0x04  /* cmd: start dma mode              */
	#define   IRQD    0x02  /* mod: irq disable                 */
	#define	  NOCOM   0x01	/* cmd: protect sfbr while reselect */
				/* bits 0-1 rsvd for C1010          */

#define ADDER			0x3c

#define SIEN			0x40	/* -->: interrupt enable            */
#define SIST			0x42	/* <--: interrupt status            */
  #define   SBMC    0x1000/* sta: SCSI Bus Mode Change (895/6 only) */
  #define   STO     0x0400/* sta: timeout (select)            */
  #define   GEN     0x0200/* sta: timeout (general)           */
  #define   HTH     0x0100/* sta: timeout (handshake)         */
  #define   MA      0x80  /* sta: phase mismatch              */
  #define   CMP     0x40  /* sta: arbitration complete        */
  #define   SEL     0x20  /* sta: selected by another device  */
  #define   RSL     0x10  /* sta: reselected by another device*/
  #define   SGE     0x08  /* sta: gross error (over/underflow)*/
  #define   UDC     0x04  /* sta: unexpected disconnect       */
  #define   RST     0x02  /* sta: scsi bus reset detected     */
  #define   PAR     0x01  /* sta: scsi parity error           */

#define SLPAR				0x44
#define SWIDE				0x45
#define MACNTL			0x46
#define GPCNTL			0x47
#define STIME0			0x48    /* cmd: timeout for select&handshake*/
#define STIME1			0x49    /* cmd: timeout user defined        */
#define RESPID			0x4a    /* sta: Reselect-IDs                */

#define STEST0			0x4c

#define STEST1			0x4d
	#define   SCLK    0x80	/* Use the PCI clock as SCSI clock	*/
	#define   DBLEN   0x08	/* clock doubler running		*/
	#define   DBLSEL  0x04	/* clock doubler selected		*/


#define STEST2			0x4e
	#define   ROF     0x40	/* reset scsi offset (after gross error!) */
	#define   EXT     0x02  /* extended filtering                     */

#define STEST3			0x4f
	#define   TE     0x80	/* c: tolerAnt enable */
	#define   HSC    0x20	/* c: Halt SCSI Clock */
	#define   CSF    0x02	/* c: clear scsi fifo */

#define SIDL			0x50	/* Lowlevel: latched from scsi data */
#define STEST4		0x52
	#define SMODE	0xc0	/* SCSI bus mode      (895/6 only) */
	#define SMODE_HVD 0x40	/* High Voltage Differential       */
	#define SMODE_SE  0x80	/* Single Ended                    */
	#define SMODE_LVD 0xc0	/* Low Voltage Differential        */
	#define LCKFRQ 0x20	/* Frequency Lock (895/6 only)     */
				/* bits 0-5 rsvd for C1010          */

#define SODL			0x54	/* Lowlevel: data out to scsi data  */

#define SBDL			0x58	/* Lowlevel: data from scsi data    */


/*-----------------------------------------------------------
**
**	Utility macros for the script.
**
**-----------------------------------------------------------
*/

#define REG(r) (r)

/*-----------------------------------------------------------
**
**	SCSI phases
**
**	DT phases illegal for ncr driver.
**
**-----------------------------------------------------------
*/

#define	SCR_DATA_OUT	0x00000000
#define	SCR_DATA_IN	0x01000000
#define	SCR_COMMAND	0x02000000
#define	SCR_STATUS	0x03000000
#define SCR_DT_DATA_OUT	0x04000000
#define SCR_DT_DATA_IN	0x05000000
#define SCR_MSG_OUT	0x06000000
#define SCR_MSG_IN      0x07000000

#define SCR_ILG_OUT	0x04000000
#define SCR_ILG_IN	0x05000000

/*-----------------------------------------------------------
**
**	Data transfer via SCSI.
**
**-----------------------------------------------------------
**
**	MOVE_ABS (LEN)
**	<<start address>>
**
**	MOVE_IND (LEN)
**	<<dnad_offset>>
**
**	MOVE_TBL
**	<<dnad_offset>>
**
**-----------------------------------------------------------
*/

#define OPC_MOVE          0x08000000

#define SCR_MOVE_ABS(l) ((0x00000000 | OPC_MOVE) | (l))
#define SCR_MOVE_IND(l) ((0x20000000 | OPC_MOVE) | (l))
#define SCR_MOVE_TBL     (0x10000000 | OPC_MOVE)

#define SCR_CHMOV_ABS(l) ((0x00000000) | (l))
#define SCR_CHMOV_IND(l) ((0x20000000) | (l))
#define SCR_CHMOV_TBL     (0x10000000)


/*-----------------------------------------------------------
**
**	Selection
**
**-----------------------------------------------------------
**
**	SEL_ABS | SCR_ID (0..15)    [ | REL_JMP]
**	<<alternate_address>>
**
**	SEL_TBL | << dnad_offset>>  [ | REL_JMP]
**	<<alternate_address>>
**
**-----------------------------------------------------------
*/

#define	SCR_SEL_ABS	0x40000000
#define	SCR_SEL_ABS_ATN	0x41000000
#define	SCR_SEL_TBL	0x42000000
#define	SCR_SEL_TBL_ATN	0x43000000


#define SCR_JMP_REL     0x04000000
#define SCR_ID(id)	(((unsigned long)(id)) << 16)

/*-----------------------------------------------------------
**
**	Waiting for Disconnect or Reselect
**
**-----------------------------------------------------------
**
**	WAIT_DISC
**	dummy: <<alternate_address>>
**
**	WAIT_RESEL
**	<<alternate_address>>
**
**-----------------------------------------------------------
*/

#define	SCR_WAIT_DISC	0x48000000
#define SCR_WAIT_RESEL  0x50000000

/*-----------------------------------------------------------
**
**	Bit Set / Reset
**
**-----------------------------------------------------------
**
**	SET (flags {|.. })
**
**	CLR (flags {|.. })
**
**-----------------------------------------------------------
*/

#define SCR_SET(f)     (0x58000000 | (f))
#define SCR_CLR(f)     (0x60000000 | (f))

#define	SCR_CARRY	0x00000400
#define	SCR_TRG		0x00000200
#define	SCR_ACK		0x00000040
#define	SCR_ATN		0x00000008


/*-----------------------------------------------------------
**
**	Memory to memory move
**
**-----------------------------------------------------------
**
**	COPY (bytecount)
**	<< source_address >>
**	<< destination_address >>
**
**	SCR_COPY   sets the NO FLUSH option by default.
**	SCR_COPY_F does not set this option.
**
**	For chips which do not support this option,
**	ncr_copy_and_bind() will remove this bit.
**-----------------------------------------------------------
*/

#define SCR_NO_FLUSH 0x01000000

#define SCR_COPY(n) (0xc0000000 | SCR_NO_FLUSH | (n))
#define SCR_COPY_F(n) (0xc0000000 | (n))

/*-----------------------------------------------------------
**
**	Register move and binary operations
**
**-----------------------------------------------------------
**
**	SFBR_REG (reg, op, data)        reg  = SFBR op data
**	<< 0 >>
**
**	REG_SFBR (reg, op, data)        SFBR = reg op data
**	<< 0 >>
**
**	REG_REG  (reg, op, data)        reg  = reg op data
**	<< 0 >>
**
**-----------------------------------------------------------
**	On 810A, 860, 825A, 875, 895 and 896 chips the content
**	of SFBR register can be used as data (SCR_SFBR_DATA).
**	The 896 has additionnal IO registers starting at
**	offset 0x80. Bit 7 of register offset is stored in
**	bit 7 of the SCRIPTS instruction first DWORD.
**-----------------------------------------------------------
*/

#define SCR_REG_OFS(ofs) ((((ofs) & 0x7f) << 16ul)) /* + ((ofs) & 0x80)) */

#define SCR_SFBR_REG(reg,op,data) \
	(0x68000000 | (SCR_REG_OFS(REG(reg))) | (op) | (((data)&0xff)<<8ul))

#define SCR_REG_SFBR(reg,op,data) \
	(0x70000000 | (SCR_REG_OFS(REG(reg))) | (op) | (((data)&0xff)<<8ul))

#define SCR_REG_REG(reg,op,data) \
	(0x78000000 | (SCR_REG_OFS(REG(reg))) | (op) | (((data)&0xff)<<8ul))


#define      SCR_LOAD   0x00000000
#define      SCR_SHL    0x01000000
#define      SCR_OR     0x02000000
#define      SCR_XOR    0x03000000
#define      SCR_AND    0x04000000
#define      SCR_SHR    0x05000000
#define      SCR_ADD    0x06000000
#define      SCR_ADDC   0x07000000

#define      SCR_SFBR_DATA   (0x00800000>>8ul)	/* Use SFBR as data */

/*-----------------------------------------------------------
**
**	FROM_REG (reg)		  SFBR = reg
**	<< 0 >>
**
**	TO_REG	 (reg)		  reg  = SFBR
**	<< 0 >>
**
**	LOAD_REG (reg, data)	  reg  = <data>
**	<< 0 >>
**
**	LOAD_SFBR(data)		  SFBR = <data>
**	<< 0 >>
**
**-----------------------------------------------------------
*/

#define	SCR_FROM_REG(reg) \
	SCR_REG_SFBR(reg,SCR_OR,0)

#define	SCR_TO_REG(reg) \
	SCR_SFBR_REG(reg,SCR_OR,0)

#define	SCR_LOAD_REG(reg,data) \
	SCR_REG_REG(reg,SCR_LOAD,data)

#define SCR_LOAD_SFBR(data) \
	(SCR_REG_SFBR (gpreg, SCR_LOAD, data))

/*-----------------------------------------------------------
**
**	LOAD  from memory   to register.
**	STORE from register to memory.
**
**	Only supported by 810A, 860, 825A, 875, 895 and 896.
**
**-----------------------------------------------------------
**
**	LOAD_ABS (LEN)
**	<<start address>>
**
**	LOAD_REL (LEN)        (DSA relative)
**	<<dsa_offset>>
**
**-----------------------------------------------------------
*/

#define SCR_REG_OFS2(ofs) (((ofs) & 0xff) << 16ul)
#define SCR_NO_FLUSH2	0x02000000
#define SCR_DSA_REL2	0x10000000

#define SCR_LOAD_R(reg, how, n) \
	(0xe1000000 | how | (SCR_REG_OFS2(REG(reg))) | (n))

#define SCR_STORE_R(reg, how, n) \
	(0xe0000000 | how | (SCR_REG_OFS2(REG(reg))) | (n))

#define SCR_LOAD_ABS(reg, n)	SCR_LOAD_R(reg, SCR_NO_FLUSH2, n)
#define SCR_LOAD_REL(reg, n)	SCR_LOAD_R(reg, SCR_NO_FLUSH2|SCR_DSA_REL2, n)
#define SCR_LOAD_ABS_F(reg, n)	SCR_LOAD_R(reg, 0, n)
#define SCR_LOAD_REL_F(reg, n)	SCR_LOAD_R(reg, SCR_DSA_REL2, n)

#define SCR_STORE_ABS(reg, n)	SCR_STORE_R(reg, SCR_NO_FLUSH2, n)
#define SCR_STORE_REL(reg, n)	SCR_STORE_R(reg, SCR_NO_FLUSH2|SCR_DSA_REL2,n)
#define SCR_STORE_ABS_F(reg, n)	SCR_STORE_R(reg, 0, n)
#define SCR_STORE_REL_F(reg, n)	SCR_STORE_R(reg, SCR_DSA_REL2, n)


/*-----------------------------------------------------------
**
**	Waiting for Disconnect or Reselect
**
**-----------------------------------------------------------
**
**	JUMP            [ | IFTRUE/IFFALSE ( ... ) ]
**	<<address>>
**
**	JUMPR           [ | IFTRUE/IFFALSE ( ... ) ]
**	<<distance>>
**
**	CALL            [ | IFTRUE/IFFALSE ( ... ) ]
**	<<address>>
**
**	CALLR           [ | IFTRUE/IFFALSE ( ... ) ]
**	<<distance>>
**
**	RETURN          [ | IFTRUE/IFFALSE ( ... ) ]
**	<<dummy>>
**
**	INT             [ | IFTRUE/IFFALSE ( ... ) ]
**	<<ident>>
**
**	INT_FLY         [ | IFTRUE/IFFALSE ( ... ) ]
**	<<ident>>
**
**	Conditions:
**	     WHEN (phase)
**	     IF   (phase)
**	     CARRYSET
**	     DATA (data, mask)
**
**-----------------------------------------------------------
*/

#define SCR_NO_OP       0x80000000
#define SCR_JUMP        0x80080000
#define SCR_JUMP64      0x80480000
#define SCR_JUMPR       0x80880000
#define SCR_CALL        0x88080000
#define SCR_CALLR       0x88880000
#define SCR_RETURN      0x90080000
#define SCR_INT         0x98080000
#define SCR_INT_FLY     0x98180000

#define IFFALSE(arg)   (0x00080000 | (arg))
#define IFTRUE(arg)    (0x00000000 | (arg))

#define WHEN(phase)    (0x00030000 | (phase))
#define IF(phase)      (0x00020000 | (phase))

#define DATA(D)        (0x00040000 | ((D) & 0xff))
#define MASK(D,M)      (0x00040000 | (((M ^ 0xff) & 0xff) << 8ul)|((D) & 0xff))

#define CARRYSET       (0x00200000)


#define SIR_COMPLETE					 0x10000000
/* script errors */
#define SIR_SEL_ATN_NO_MSG_OUT 0x00000001
#define SIR_CMD_OUT_ILL_PH     0x00000002
#define SIR_STATUS_ILL_PH			 0x00000003
#define SIR_MSG_RECEIVED			 0x00000004
#define SIR_DATA_IN_ERR        0x00000005
#define SIR_DATA_OUT_ERR			 0x00000006
#define SIR_SCRIPT_ERROR			 0x00000007
#define SIR_MSG_OUT_NO_CMD		 0x00000008
#define SIR_MSG_OVER7					 0x00000009
/* Fly interrupt */
#define INT_ON_FY							 0x00000080

/* Hardware errors  are defined in scsi.h */

#define SCSI_IDENTIFY					0xC0

#endif
