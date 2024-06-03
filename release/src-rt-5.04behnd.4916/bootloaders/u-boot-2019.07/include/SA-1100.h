/*
 *	FILE		SA-1100.h
 *
 *	Version		1.2
 *	Author		Copyright (c) Marc A. Viredaz, 1998
 *			DEC Western Research Laboratory, Palo Alto, CA
 *	Date		January 1998 (April 1997)
 *	System		StrongARM SA-1100
 *	Language	C or ARM Assembly
 *	Purpose		Definition of constants related to the StrongARM
 *			SA-1100 microprocessor (Advanced RISC Machine (ARM)
 *			architecture version 4). This file is based on the
 *			StrongARM SA-1100 data sheet version 2.2.
 *
 *			Language-specific definitions are selected by the
 *			macro "LANGUAGE", which should be defined as either
 *			"C" (default) or "Assembly".
 */


#ifndef LANGUAGE
# ifdef __ASSEMBLY__
#  define LANGUAGE Assembly
# else
#  define LANGUAGE C
# endif
#endif

#ifndef io_p2v
#define io_p2v(PhAdd)	(PhAdd)
#endif

#include <asm/arch-sa1100/bitfield.h>

#define C		0
#define Assembly	1


#if LANGUAGE == C
typedef unsigned short	Word16 ;
typedef unsigned int	Word32 ;
typedef Word32		Word ;
typedef Word		Quad [4] ;
typedef void		*Address ;
typedef void		(*ExcpHndlr) (void) ;
#endif /* LANGUAGE == C */


/*
 * Memory
 */

#define MemBnkSp	0x08000000	/* Memory Bank Space [byte]	   */

#define StMemBnkSp	MemBnkSp	/* Static Memory Bank Space [byte] */
#define StMemBnk0Sp	StMemBnkSp	/* Static Memory Bank 0 Space	   */
					/* [byte]			   */
#define StMemBnk1Sp	StMemBnkSp	/* Static Memory Bank 1 Space	   */
					/* [byte]			   */
#define StMemBnk2Sp	StMemBnkSp	/* Static Memory Bank 2 Space	   */
					/* [byte]			   */
#define StMemBnk3Sp	StMemBnkSp	/* Static Memory Bank 3 Space	   */
					/* [byte]			   */

#define DRAMBnkSp	MemBnkSp	/* DRAM Bank Space [byte]	   */
#define DRAMBnk0Sp	DRAMBnkSp	/* DRAM Bank 0 Space [byte]	   */
#define DRAMBnk1Sp	DRAMBnkSp	/* DRAM Bank 1 Space [byte]	   */
#define DRAMBnk2Sp	DRAMBnkSp	/* DRAM Bank 2 Space [byte]	   */
#define DRAMBnk3Sp	DRAMBnkSp	/* DRAM Bank 3 Space [byte]	   */

#define ZeroMemSp	MemBnkSp	/* Zero Memory bank Space [byte]   */

#define _StMemBnk(Nb)			/* Static Memory Bank [0..3]	   */ \
			(0x00000000 + (Nb)*StMemBnkSp)
#define _StMemBnk0	_StMemBnk (0)	/* Static Memory Bank 0		   */
#define _StMemBnk1	_StMemBnk (1)	/* Static Memory Bank 1		   */
#define _StMemBnk2	_StMemBnk (2)	/* Static Memory Bank 2		   */
#define _StMemBnk3	_StMemBnk (3)	/* Static Memory Bank 3		   */

#if LANGUAGE == C
typedef Quad		StMemBnkType [StMemBnkSp/sizeof (Quad)] ;
#define StMemBnk			/* Static Memory Bank [0..3]	   */ \
			((StMemBnkType *) io_p2v (_StMemBnk (0)))
#define StMemBnk0	(StMemBnk [0])	/* Static Memory Bank 0		   */
#define StMemBnk1	(StMemBnk [1])	/* Static Memory Bank 1		   */
#define StMemBnk2	(StMemBnk [2])	/* Static Memory Bank 2		   */
#define StMemBnk3	(StMemBnk [3])	/* Static Memory Bank 3		   */
#endif /* LANGUAGE == C */

#define _DRAMBnk(Nb)			/* DRAM Bank [0..3]		   */ \
			(0xC0000000 + (Nb)*DRAMBnkSp)
#define _DRAMBnk0	_DRAMBnk (0)	/* DRAM Bank 0			   */
#define _DRAMBnk1	_DRAMBnk (1)	/* DRAM Bank 1			   */
#define _DRAMBnk2	_DRAMBnk (2)	/* DRAM Bank 2			   */
#define _DRAMBnk3	_DRAMBnk (3)	/* DRAM Bank 3			   */

#if LANGUAGE == C
typedef Quad		DRAMBnkType [DRAMBnkSp/sizeof (Quad)] ;
#define DRAMBnk				/* DRAM Bank [0..3]		   */ \
			((DRAMBnkType *) io_p2v (_DRAMBnk (0)))
#define DRAMBnk0	(DRAMBnk [0])	/* DRAM Bank 0			   */
#define DRAMBnk1	(DRAMBnk [1])	/* DRAM Bank 1			   */
#define DRAMBnk2	(DRAMBnk [2])	/* DRAM Bank 2			   */
#define DRAMBnk3	(DRAMBnk [3])	/* DRAM Bank 3			   */
#endif /* LANGUAGE == C */

#define _ZeroMem	0xE0000000	/* Zero Memory bank		   */

#if LANGUAGE == C
typedef Quad		ZeroMemType [ZeroMemSp/sizeof (Quad)] ;
#define ZeroMem				/* Zero Memory bank		   */ \
			(*((ZeroMemType *) io_p2v (_ZeroMem)))
#endif /* LANGUAGE == C */


/*
 * Personal Computer Memory Card International Association (PCMCIA) sockets
 */

#define PCMCIAPrtSp	0x04000000	/* PCMCIA Partition Space [byte]   */
#define PCMCIASp	(4*PCMCIAPrtSp)	/* PCMCIA Space [byte]		   */
#define PCMCIAIOSp	PCMCIAPrtSp	/* PCMCIA I/O Space [byte]	   */
#define PCMCIAAttrSp	PCMCIAPrtSp	/* PCMCIA Attribute Space [byte]   */
#define PCMCIAMemSp	PCMCIAPrtSp	/* PCMCIA Memory Space [byte]	   */

#define PCMCIA0Sp	PCMCIASp	/* PCMCIA 0 Space [byte]	   */
#define PCMCIA0IOSp	PCMCIAIOSp	/* PCMCIA 0 I/O Space [byte]	   */
#define PCMCIA0AttrSp	PCMCIAAttrSp	/* PCMCIA 0 Attribute Space [byte] */
#define PCMCIA0MemSp	PCMCIAMemSp	/* PCMCIA 0 Memory Space [byte]    */

#define PCMCIA1Sp	PCMCIASp	/* PCMCIA 1 Space [byte]	   */
#define PCMCIA1IOSp	PCMCIAIOSp	/* PCMCIA 1 I/O Space [byte]	   */
#define PCMCIA1AttrSp	PCMCIAAttrSp	/* PCMCIA 1 Attribute Space [byte] */
#define PCMCIA1MemSp	PCMCIAMemSp	/* PCMCIA 1 Memory Space [byte]    */

#define _PCMCIA(Nb)			/* PCMCIA [0..1]		   */ \
			(0x20000000 + (Nb)*PCMCIASp)
#define _PCMCIAIO(Nb)	_PCMCIA (Nb)	/* PCMCIA I/O [0..1]		   */
#define _PCMCIAAttr(Nb)			/* PCMCIA Attribute [0..1]	   */ \
			(_PCMCIA (Nb) + 2*PCMCIAPrtSp)
#define _PCMCIAMem(Nb)			/* PCMCIA Memory [0..1]		   */ \
			(_PCMCIA (Nb) + 3*PCMCIAPrtSp)

#define _PCMCIA0	_PCMCIA (0)	/* PCMCIA 0			   */
#define _PCMCIA0IO	_PCMCIAIO (0)	/* PCMCIA 0 I/O			   */
#define _PCMCIA0Attr	_PCMCIAAttr (0)	/* PCMCIA 0 Attribute		   */
#define _PCMCIA0Mem	_PCMCIAMem (0)	/* PCMCIA 0 Memory		   */

#define _PCMCIA1	_PCMCIA (1)	/* PCMCIA 1			   */
#define _PCMCIA1IO	_PCMCIAIO (1)	/* PCMCIA 1 I/O			   */
#define _PCMCIA1Attr	_PCMCIAAttr (1)	/* PCMCIA 1 Attribute		   */
#define _PCMCIA1Mem	_PCMCIAMem (1)	/* PCMCIA 1 Memory		   */

#if LANGUAGE == C

typedef Quad		PCMCIAPrtType [PCMCIAPrtSp/sizeof (Quad)] ;
typedef PCMCIAPrtType	PCMCIAType [PCMCIASp/PCMCIAPrtSp] ;

#define PCMCIA0				/* PCMCIA 0			   */ \
			(*((PCMCIAType *) io_p2v (_PCMCIA0)))
#define PCMCIA0IO			/* PCMCIA 0 I/O			   */ \
			(*((PCMCIAPrtType *) io_p2v (_PCMCIA0IO)))
#define PCMCIA0Attr			/* PCMCIA 0 Attribute		   */ \
			(*((PCMCIAPrtType *) io_p2v (_PCMCIA0Attr)))
#define PCMCIA0Mem			/* PCMCIA 0 Memory		   */ \
			(*((PCMCIAPrtType *) io_p2v (_PCMCIA0Mem)))

#define PCMCIA1				/* PCMCIA 1			   */ \
			(*((PCMCIAType *) io_p2v (_PCMCIA1)))
#define PCMCIA1IO			/* PCMCIA 1 I/O			   */ \
			(*((PCMCIAPrtType *) io_p2v (_PCMCIA1IO)))
#define PCMCIA1Attr			/* PCMCIA 1 Attribute		   */ \
			(*((PCMCIAPrtType *) io_p2v (_PCMCIA1Attr)))
#define PCMCIA1Mem			/* PCMCIA 1 Memory		   */ \
			(*((PCMCIAPrtType *) io_p2v (_PCMCIA1Mem)))

#endif /* LANGUAGE == C */


/*
 * Universal Serial Bus (USB) Device Controller (UDC) control registers
 *
 * Registers
 *    Ser0UDCCR		Serial port 0 Universal Serial Bus (USB) Device
 *			Controller (UDC) Control Register (read/write).
 *    Ser0UDCAR		Serial port 0 Universal Serial Bus (USB) Device
 *			Controller (UDC) Address Register (read/write).
 *    Ser0UDCOMP	Serial port 0 Universal Serial Bus (USB) Device
 *			Controller (UDC) Output Maximum Packet size register
 *			(read/write).
 *    Ser0UDCIMP	Serial port 0 Universal Serial Bus (USB) Device
 *			Controller (UDC) Input Maximum Packet size register
 *			(read/write).
 *    Ser0UDCCS0	Serial port 0 Universal Serial Bus (USB) Device
 *			Controller (UDC) Control/Status register end-point 0
 *			(read/write).
 *    Ser0UDCCS1	Serial port 0 Universal Serial Bus (USB) Device
 *			Controller (UDC) Control/Status register end-point 1
 *			(output, read/write).
 *    Ser0UDCCS2	Serial port 0 Universal Serial Bus (USB) Device
 *			Controller (UDC) Control/Status register end-point 2
 *			(input, read/write).
 *    Ser0UDCD0		Serial port 0 Universal Serial Bus (USB) Device
 *			Controller (UDC) Data register end-point 0
 *			(read/write).
 *    Ser0UDCWC		Serial port 0 Universal Serial Bus (USB) Device
 *			Controller (UDC) Write Count register end-point 0
 *			(read).
 *    Ser0UDCDR		Serial port 0 Universal Serial Bus (USB) Device
 *			Controller (UDC) Data Register (read/write).
 *    Ser0UDCSR		Serial port 0 Universal Serial Bus (USB) Device
 *			Controller (UDC) Status Register (read/write).
 */

#define _Ser0UDCCR	0x80000000	/* Ser. port 0 UDC Control Reg.    */
#define _Ser0UDCAR	0x80000004	/* Ser. port 0 UDC Address Reg.    */
#define _Ser0UDCOMP	0x80000008	/* Ser. port 0 UDC Output Maximum  */
					/* Packet size reg.		   */
#define _Ser0UDCIMP	0x8000000C	/* Ser. port 0 UDC Input Maximum   */
					/* Packet size reg.		   */
#define _Ser0UDCCS0	0x80000010	/* Ser. port 0 UDC Control/Status  */
					/* reg. end-point 0		   */
#define _Ser0UDCCS1	0x80000014	/* Ser. port 0 UDC Control/Status  */
					/* reg. end-point 1 (output)	   */
#define _Ser0UDCCS2	0x80000018	/* Ser. port 0 UDC Control/Status  */
					/* reg. end-point 2 (input)	   */
#define _Ser0UDCD0	0x8000001C	/* Ser. port 0 UDC Data reg.	   */
					/* end-point 0			   */
#define _Ser0UDCWC	0x80000020	/* Ser. port 0 UDC Write Count	   */
					/* reg. end-point 0		   */
#define _Ser0UDCDR	0x80000028	/* Ser. port 0 UDC Data Reg.	   */
#define _Ser0UDCSR	0x80000030	/* Ser. port 0 UDC Status Reg.	   */

#if LANGUAGE == C
#define Ser0UDCCR			/* Ser. port 0 UDC Control Reg.    */ \
			(*((volatile Word *) io_p2v (_Ser0UDCCR)))
#define Ser0UDCAR			/* Ser. port 0 UDC Address Reg.    */ \
			(*((volatile Word *) io_p2v (_Ser0UDCAR)))
#define Ser0UDCOMP			/* Ser. port 0 UDC Output Maximum  */ \
					/* Packet size reg.		   */ \
			(*((volatile Word *) io_p2v (_Ser0UDCOMP)))
#define Ser0UDCIMP			/* Ser. port 0 UDC Input Maximum   */ \
					/* Packet size reg.		   */ \
			(*((volatile Word *) io_p2v (_Ser0UDCIMP)))
#define Ser0UDCCS0			/* Ser. port 0 UDC Control/Status  */ \
					/* reg. end-point 0		   */ \
			(*((volatile Word *) io_p2v (_Ser0UDCCS0)))
#define Ser0UDCCS1			/* Ser. port 0 UDC Control/Status  */ \
					/* reg. end-point 1 (output)	   */ \
			(*((volatile Word *) io_p2v (_Ser0UDCCS1)))
#define Ser0UDCCS2			/* Ser. port 0 UDC Control/Status  */ \
					/* reg. end-point 2 (input)	   */ \
			(*((volatile Word *) io_p2v (_Ser0UDCCS2)))
#define Ser0UDCD0			/* Ser. port 0 UDC Data reg.	   */ \
					/* end-point 0			   */ \
			(*((volatile Word *) io_p2v (_Ser0UDCD0)))
#define Ser0UDCWC			/* Ser. port 0 UDC Write Count	   */ \
					/* reg. end-point 0		   */ \
			(*((volatile Word *) io_p2v (_Ser0UDCWC)))
#define Ser0UDCDR			/* Ser. port 0 UDC Data Reg.	   */ \
			(*((volatile Word *) io_p2v (_Ser0UDCDR)))
#define Ser0UDCSR			/* Ser. port 0 UDC Status Reg.	   */ \
			(*((volatile Word *) io_p2v (_Ser0UDCSR)))
#endif /* LANGUAGE == C */

#define UDCCR_UDD	0x00000001	/* UDC Disable			   */
#define UDCCR_UDA	0x00000002	/* UDC Active (read)		   */
#define UDCCR_RESIM	0x00000004	/* Resume Interrupt Mask, per errata */
#define UDCCR_EIM	0x00000008	/* End-point 0 Interrupt Mask	   */
					/* (disable)			   */
#define UDCCR_RIM	0x00000010	/* Receive Interrupt Mask	   */
					/* (disable)			   */
#define UDCCR_TIM	0x00000020	/* Transmit Interrupt Mask	   */
					/* (disable)			   */
#define UDCCR_SRM	0x00000040	/* Suspend/Resume interrupt Mask   */
					/* (disable)			   */
#define UDCCR_SUSIM	UDCCR_SRM	/* Per errata, SRM just masks suspend */
#define UDCCR_REM	0x00000080	/* REset interrupt Mask (disable)  */

#define UDCAR_ADD	Fld (7, 0)	/* function ADDress		   */

#define UDCOMP_OUTMAXP	Fld (8, 0)	/* OUTput MAXimum Packet size - 1  */
					/* [byte]			   */
#define UDCOMP_OutMaxPkt(Size)		/* Output Maximum Packet size	   */ \
					/* [1..256 byte]		   */ \
			(((Size) - 1) << FShft (UDCOMP_OUTMAXP))

#define UDCIMP_INMAXP	Fld (8, 0)	/* INput MAXimum Packet size - 1   */
					/* [byte]			   */
#define UDCIMP_InMaxPkt(Size)		/* Input Maximum Packet size	   */ \
					/* [1..256 byte]		   */ \
			(((Size) - 1) << FShft (UDCIMP_INMAXP))

#define UDCCS0_OPR	0x00000001	/* Output Packet Ready (read)	   */
#define UDCCS0_IPR	0x00000002	/* Input Packet Ready		   */
#define UDCCS0_SST	0x00000004	/* Sent STall			   */
#define UDCCS0_FST	0x00000008	/* Force STall			   */
#define UDCCS0_DE	0x00000010	/* Data End			   */
#define UDCCS0_SE	0x00000020	/* Setup End (read)		   */
#define UDCCS0_SO	0x00000040	/* Serviced Output packet ready    */
					/* (write)			   */
#define UDCCS0_SSE	0x00000080	/* Serviced Setup End (write)	   */

#define UDCCS1_RFS	0x00000001	/* Receive FIFO 12-bytes or more   */
					/* Service request (read)	   */
#define UDCCS1_RPC	0x00000002	/* Receive Packet Complete	   */
#define UDCCS1_RPE	0x00000004	/* Receive Packet Error (read)	   */
#define UDCCS1_SST	0x00000008	/* Sent STall			   */
#define UDCCS1_FST	0x00000010	/* Force STall			   */
#define UDCCS1_RNE	0x00000020	/* Receive FIFO Not Empty (read)   */

#define UDCCS2_TFS	0x00000001	/* Transmit FIFO 8-bytes or less   */
					/* Service request (read)	   */
#define UDCCS2_TPC	0x00000002	/* Transmit Packet Complete	   */
#define UDCCS2_TPE	0x00000004	/* Transmit Packet Error (read)    */
#define UDCCS2_TUR	0x00000008	/* Transmit FIFO Under-Run	   */
#define UDCCS2_SST	0x00000010	/* Sent STall			   */
#define UDCCS2_FST	0x00000020	/* Force STall			   */

#define UDCD0_DATA	Fld (8, 0)	/* receive/transmit DATA FIFOs	   */

#define UDCWC_WC	Fld (4, 0)	/* Write Count			   */

#define UDCDR_DATA	Fld (8, 0)	/* receive/transmit DATA FIFOs	   */

#define UDCSR_EIR	0x00000001	/* End-point 0 Interrupt Request   */
#define UDCSR_RIR	0x00000002	/* Receive Interrupt Request	   */
#define UDCSR_TIR	0x00000004	/* Transmit Interrupt Request	   */
#define UDCSR_SUSIR	0x00000008	/* SUSpend Interrupt Request	   */
#define UDCSR_RESIR	0x00000010	/* RESume Interrupt Request	   */
#define UDCSR_RSTIR	0x00000020	/* ReSeT Interrupt Request	   */


/*
 * Universal Asynchronous Receiver/Transmitter (UART) control registers
 *
 * Registers
 *    Ser1UTCR0		Serial port 1 Universal Asynchronous
 *			Receiver/Transmitter (UART) Control Register 0
 *			(read/write).
 *    Ser1UTCR1		Serial port 1 Universal Asynchronous
 *			Receiver/Transmitter (UART) Control Register 1
 *			(read/write).
 *    Ser1UTCR2		Serial port 1 Universal Asynchronous
 *			Receiver/Transmitter (UART) Control Register 2
 *			(read/write).
 *    Ser1UTCR3		Serial port 1 Universal Asynchronous
 *			Receiver/Transmitter (UART) Control Register 3
 *			(read/write).
 *    Ser1UTDR		Serial port 1 Universal Asynchronous
 *			Receiver/Transmitter (UART) Data Register
 *			(read/write).
 *    Ser1UTSR0		Serial port 1 Universal Asynchronous
 *			Receiver/Transmitter (UART) Status Register 0
 *			(read/write).
 *    Ser1UTSR1		Serial port 1 Universal Asynchronous
 *			Receiver/Transmitter (UART) Status Register 1 (read).
 *
 *    Ser2UTCR0		Serial port 2 Universal Asynchronous
 *			Receiver/Transmitter (UART) Control Register 0
 *			(read/write).
 *    Ser2UTCR1		Serial port 2 Universal Asynchronous
 *			Receiver/Transmitter (UART) Control Register 1
 *			(read/write).
 *    Ser2UTCR2		Serial port 2 Universal Asynchronous
 *			Receiver/Transmitter (UART) Control Register 2
 *			(read/write).
 *    Ser2UTCR3		Serial port 2 Universal Asynchronous
 *			Receiver/Transmitter (UART) Control Register 3
 *			(read/write).
 *    Ser2UTCR4		Serial port 2 Universal Asynchronous
 *			Receiver/Transmitter (UART) Control Register 4
 *			(read/write).
 *    Ser2UTDR		Serial port 2 Universal Asynchronous
 *			Receiver/Transmitter (UART) Data Register
 *			(read/write).
 *    Ser2UTSR0		Serial port 2 Universal Asynchronous
 *			Receiver/Transmitter (UART) Status Register 0
 *			(read/write).
 *    Ser2UTSR1		Serial port 2 Universal Asynchronous
 *			Receiver/Transmitter (UART) Status Register 1 (read).
 *
 *    Ser3UTCR0		Serial port 3 Universal Asynchronous
 *			Receiver/Transmitter (UART) Control Register 0
 *			(read/write).
 *    Ser3UTCR1		Serial port 3 Universal Asynchronous
 *			Receiver/Transmitter (UART) Control Register 1
 *			(read/write).
 *    Ser3UTCR2		Serial port 3 Universal Asynchronous
 *			Receiver/Transmitter (UART) Control Register 2
 *			(read/write).
 *    Ser3UTCR3		Serial port 3 Universal Asynchronous
 *			Receiver/Transmitter (UART) Control Register 3
 *			(read/write).
 *    Ser3UTDR		Serial port 3 Universal Asynchronous
 *			Receiver/Transmitter (UART) Data Register
 *			(read/write).
 *    Ser3UTSR0		Serial port 3 Universal Asynchronous
 *			Receiver/Transmitter (UART) Status Register 0
 *			(read/write).
 *    Ser3UTSR1		Serial port 3 Universal Asynchronous
 *			Receiver/Transmitter (UART) Status Register 1 (read).
 *
 * Clocks
 *    fxtl, Txtl	Frequency, period of the system crystal (3.6864 MHz
 *			or 3.5795 MHz).
 *    fua, Tua		Frequency, period of the UART communication.
 */

#define _UTCR0(Nb)			/* UART Control Reg. 0 [1..3]	   */ \
			(0x80010000 + ((Nb) - 1)*0x00020000)
#define _UTCR1(Nb)			/* UART Control Reg. 1 [1..3]	   */ \
			(0x80010004 + ((Nb) - 1)*0x00020000)
#define _UTCR2(Nb)			/* UART Control Reg. 2 [1..3]	   */ \
			(0x80010008 + ((Nb) - 1)*0x00020000)
#define _UTCR3(Nb)			/* UART Control Reg. 3 [1..3]	   */ \
			(0x8001000C + ((Nb) - 1)*0x00020000)
#define _UTCR4(Nb)			/* UART Control Reg. 4 [2]	   */ \
			(0x80010010 + ((Nb) - 1)*0x00020000)
#define _UTDR(Nb)			/* UART Data Reg. [1..3]	   */ \
			(0x80010014 + ((Nb) - 1)*0x00020000)
#define _UTSR0(Nb)			/* UART Status Reg. 0 [1..3]	   */ \
			(0x8001001C + ((Nb) - 1)*0x00020000)
#define _UTSR1(Nb)			/* UART Status Reg. 1 [1..3]	   */ \
			(0x80010020 + ((Nb) - 1)*0x00020000)

#define _Ser1UTCR0	_UTCR0 (1)	/* Ser. port 1 UART Control Reg. 0 */
#define _Ser1UTCR1	_UTCR1 (1)	/* Ser. port 1 UART Control Reg. 1 */
#define _Ser1UTCR2	_UTCR2 (1)	/* Ser. port 1 UART Control Reg. 2 */
#define _Ser1UTCR3	_UTCR3 (1)	/* Ser. port 1 UART Control Reg. 3 */
#define _Ser1UTDR	_UTDR (1)	/* Ser. port 1 UART Data Reg.	   */
#define _Ser1UTSR0	_UTSR0 (1)	/* Ser. port 1 UART Status Reg. 0  */
#define _Ser1UTSR1	_UTSR1 (1)	/* Ser. port 1 UART Status Reg. 1  */

#define _Ser2UTCR0	_UTCR0 (2)	/* Ser. port 2 UART Control Reg. 0 */
#define _Ser2UTCR1	_UTCR1 (2)	/* Ser. port 2 UART Control Reg. 1 */
#define _Ser2UTCR2	_UTCR2 (2)	/* Ser. port 2 UART Control Reg. 2 */
#define _Ser2UTCR3	_UTCR3 (2)	/* Ser. port 2 UART Control Reg. 3 */
#define _Ser2UTCR4	_UTCR4 (2)	/* Ser. port 2 UART Control Reg. 4 */
#define _Ser2UTDR	_UTDR (2)	/* Ser. port 2 UART Data Reg.	   */
#define _Ser2UTSR0	_UTSR0 (2)	/* Ser. port 2 UART Status Reg. 0  */
#define _Ser2UTSR1	_UTSR1 (2)	/* Ser. port 2 UART Status Reg. 1  */

#define _Ser3UTCR0	_UTCR0 (3)	/* Ser. port 3 UART Control Reg. 0 */
#define _Ser3UTCR1	_UTCR1 (3)	/* Ser. port 3 UART Control Reg. 1 */
#define _Ser3UTCR2	_UTCR2 (3)	/* Ser. port 3 UART Control Reg. 2 */
#define _Ser3UTCR3	_UTCR3 (3)	/* Ser. port 3 UART Control Reg. 3 */
#define _Ser3UTDR	_UTDR (3)	/* Ser. port 3 UART Data Reg.	   */
#define _Ser3UTSR0	_UTSR0 (3)	/* Ser. port 3 UART Status Reg. 0  */
#define _Ser3UTSR1	_UTSR1 (3)	/* Ser. port 3 UART Status Reg. 1  */

#if LANGUAGE == C

#define Ser1UTCR0			/* Ser. port 1 UART Control Reg. 0 */ \
			(*((volatile Word *) io_p2v (_Ser1UTCR0)))
#define Ser1UTCR1			/* Ser. port 1 UART Control Reg. 1 */ \
			(*((volatile Word *) io_p2v (_Ser1UTCR1)))
#define Ser1UTCR2			/* Ser. port 1 UART Control Reg. 2 */ \
			(*((volatile Word *) io_p2v (_Ser1UTCR2)))
#define Ser1UTCR3			/* Ser. port 1 UART Control Reg. 3 */ \
			(*((volatile Word *) io_p2v (_Ser1UTCR3)))
#define Ser1UTDR			/* Ser. port 1 UART Data Reg.	   */ \
			(*((volatile Word *) io_p2v (_Ser1UTDR)))
#define Ser1UTSR0			/* Ser. port 1 UART Status Reg. 0  */ \
			(*((volatile Word *) io_p2v (_Ser1UTSR0)))
#define Ser1UTSR1			/* Ser. port 1 UART Status Reg. 1  */ \
			(*((volatile Word *) io_p2v (_Ser1UTSR1)))

#define Ser2UTCR0			/* Ser. port 2 UART Control Reg. 0 */ \
			(*((volatile Word *) io_p2v (_Ser2UTCR0)))
#define Ser2UTCR1			/* Ser. port 2 UART Control Reg. 1 */ \
			(*((volatile Word *) io_p2v (_Ser2UTCR1)))
#define Ser2UTCR2			/* Ser. port 2 UART Control Reg. 2 */ \
			(*((volatile Word *) io_p2v (_Ser2UTCR2)))
#define Ser2UTCR3			/* Ser. port 2 UART Control Reg. 3 */ \
			(*((volatile Word *) io_p2v (_Ser2UTCR3)))
#define Ser2UTCR4			/* Ser. port 2 UART Control Reg. 4 */ \
			(*((volatile Word *) io_p2v (_Ser2UTCR4)))
#define Ser2UTDR			/* Ser. port 2 UART Data Reg.	   */ \
			(*((volatile Word *) io_p2v (_Ser2UTDR)))
#define Ser2UTSR0			/* Ser. port 2 UART Status Reg. 0  */ \
			(*((volatile Word *) io_p2v (_Ser2UTSR0)))
#define Ser2UTSR1			/* Ser. port 2 UART Status Reg. 1  */ \
			(*((volatile Word *) io_p2v (_Ser2UTSR1)))

#define Ser3UTCR0			/* Ser. port 3 UART Control Reg. 0 */ \
			(*((volatile Word *) io_p2v (_Ser3UTCR0)))
#define Ser3UTCR1			/* Ser. port 3 UART Control Reg. 1 */ \
			(*((volatile Word *) io_p2v (_Ser3UTCR1)))
#define Ser3UTCR2			/* Ser. port 3 UART Control Reg. 2 */ \
			(*((volatile Word *) io_p2v (_Ser3UTCR2)))
#define Ser3UTCR3			/* Ser. port 3 UART Control Reg. 3 */ \
			(*((volatile Word *) io_p2v (_Ser3UTCR3)))
#define Ser3UTDR			/* Ser. port 3 UART Data Reg.	   */ \
			(*((volatile Word *) io_p2v (_Ser3UTDR)))
#define Ser3UTSR0			/* Ser. port 3 UART Status Reg. 0  */ \
			(*((volatile Word *) io_p2v (_Ser3UTSR0)))
#define Ser3UTSR1			/* Ser. port 3 UART Status Reg. 1  */ \
			(*((volatile Word *) io_p2v (_Ser3UTSR1)))

#elif LANGUAGE == Assembly
#define Ser1UTCR0	( io_p2v (_Ser1UTCR0))
#define Ser1UTCR1	( io_p2v (_Ser1UTCR1))
#define Ser1UTCR2	( io_p2v (_Ser1UTCR2))
#define Ser1UTCR3	( io_p2v (_Ser1UTCR3))
#define Ser1UTDR	( io_p2v (_Ser1UTDR))
#define Ser1UTSR0	( io_p2v (_Ser1UTSR0))
#define Ser1UTSR1	( io_p2v (_Ser1UTSR1))

#define Ser2UTCR0	( io_p2v (_Ser2UTCR0))
#define Ser2UTCR1	( io_p2v (_Ser2UTCR1))
#define Ser2UTCR2	( io_p2v (_Ser2UTCR2))
#define Ser2UTCR3	( io_p2v (_Ser2UTCR3))
#define Ser2UTCR4	( io_p2v (_Ser2UTCR4))
#define Ser2UTDR	( io_p2v (_Ser2UTDR))
#define Ser2UTSR0	( io_p2v (_Ser2UTSR0))
#define Ser2UTSR1	( io_p2v (_Ser2UTSR1))

#define Ser3UTCR0	( io_p2v (_Ser3UTCR0))
#define Ser3UTCR1	( io_p2v (_Ser3UTCR1))
#define Ser3UTCR2	( io_p2v (_Ser3UTCR2))
#define Ser3UTCR3	( io_p2v (_Ser3UTCR3))
#define Ser3UTDR	( io_p2v (_Ser3UTDR))
#define Ser3UTSR0	( io_p2v (_Ser3UTSR0))
#define Ser3UTSR1	( io_p2v (_Ser3UTSR1))

#endif /* LANGUAGE == C */

#define UTCR0_PE	0x00000001	/* Parity Enable		   */
#define UTCR0_OES	0x00000002	/* Odd/Even parity Select	   */
#define UTCR0_OddPar	(UTCR0_OES*0)	/*  Odd Parity			   */
#define UTCR0_EvenPar	(UTCR0_OES*1)	/*  Even Parity			   */
#define UTCR0_SBS	0x00000004	/* Stop Bit Select		   */
#define UTCR0_1StpBit	(UTCR0_SBS*0)	/*  1 Stop Bit per frame	   */
#define UTCR0_2StpBit	(UTCR0_SBS*1)	/*  2 Stop Bits per frame	   */
#define UTCR0_DSS	0x00000008	/* Data Size Select		   */
#define UTCR0_7BitData	(UTCR0_DSS*0)	/*  7-Bit Data			   */
#define UTCR0_8BitData	(UTCR0_DSS*1)	/*  8-Bit Data			   */
#define UTCR0_SCE	0x00000010	/* Sample Clock Enable		   */
					/* (ser. port 1: GPIO [18],	   */
					/* ser. port 3: GPIO [20])	   */
#define UTCR0_RCE	0x00000020	/* Receive Clock Edge select	   */
#define UTCR0_RcRsEdg	(UTCR0_RCE*0)	/*  Receive clock Rising-Edge	   */
#define UTCR0_RcFlEdg	(UTCR0_RCE*1)	/*  Receive clock Falling-Edge	   */
#define UTCR0_TCE	0x00000040	/* Transmit Clock Edge select	   */
#define UTCR0_TrRsEdg	(UTCR0_TCE*0)	/*  Transmit clock Rising-Edge	   */
#define UTCR0_TrFlEdg	(UTCR0_TCE*1)	/*  Transmit clock Falling-Edge    */
#define UTCR0_Ser2IrDA			/* Ser. port 2 IrDA settings	   */ \
			(UTCR0_1StpBit + UTCR0_8BitData)

#define UTCR1_BRD	Fld (4, 0)	/* Baud Rate Divisor/16 - 1 [11:8] */
#define UTCR2_BRD	Fld (8, 0)	/* Baud Rate Divisor/16 - 1  [7:0] */
					/* fua = fxtl/(16*(BRD[11:0] + 1)) */
					/* Tua = 16*(BRD [11:0] + 1)*Txtl  */
#define UTCR1_BdRtDiv(Div)		/*  Baud Rate Divisor [16..65536]  */ \
			(((Div) - 16)/16 >> FSize (UTCR2_BRD) << \
			 FShft (UTCR1_BRD))
#define UTCR2_BdRtDiv(Div)		/*  Baud Rate Divisor [16..65536]  */ \
			(((Div) - 16)/16 & FAlnMsk (UTCR2_BRD) << \
			 FShft (UTCR2_BRD))
					/*  fua = fxtl/(16*Floor (Div/16)) */
					/*  Tua = 16*Floor (Div/16)*Txtl   */
#define UTCR1_CeilBdRtDiv(Div)		/*  Ceil. of BdRtDiv [16..65536]   */ \
			(((Div) - 1)/16 >> FSize (UTCR2_BRD) << \
			 FShft (UTCR1_BRD))
#define UTCR2_CeilBdRtDiv(Div)		/*  Ceil. of BdRtDiv [16..65536]   */ \
			(((Div) - 1)/16 & FAlnMsk (UTCR2_BRD) << \
			 FShft (UTCR2_BRD))
					/*  fua = fxtl/(16*Ceil (Div/16))  */
					/*  Tua = 16*Ceil (Div/16)*Txtl    */

#define UTCR3_RXE	0x00000001	/* Receive Enable		   */
#define UTCR3_TXE	0x00000002	/* Transmit Enable		   */
#define UTCR3_BRK	0x00000004	/* BReaK mode			   */
#define UTCR3_RIE	0x00000008	/* Receive FIFO 1/3-to-2/3-full or */
					/* more Interrupt Enable	   */
#define UTCR3_TIE	0x00000010	/* Transmit FIFO 1/2-full or less  */
					/* Interrupt Enable		   */
#define UTCR3_LBM	0x00000020	/* Look-Back Mode		   */
#define UTCR3_Ser2IrDA			/* Ser. port 2 IrDA settings (RIE, */ \
					/* TIE, LBM can be set or cleared) */ \
			(UTCR3_RXE + UTCR3_TXE)

#define UTCR4_HSE	0x00000001	/* Hewlett-Packard Serial InfraRed */
					/* (HP-SIR) modulation Enable	   */
#define UTCR4_NRZ	(UTCR4_HSE*0)	/*  Non-Return to Zero modulation  */
#define UTCR4_HPSIR	(UTCR4_HSE*1)	/*  HP-SIR modulation		   */
#define UTCR4_LPM	0x00000002	/* Low-Power Mode		   */
#define UTCR4_Z3_16Bit	(UTCR4_LPM*0)	/*  Zero pulse = 3/16 Bit time	   */
#define UTCR4_Z1_6us	(UTCR4_LPM*1)	/*  Zero pulse = 1.6 us		   */

#define UTDR_DATA	Fld (8, 0)	/* receive/transmit DATA FIFOs	   */
#if 0					/* Hidden receive FIFO bits	   */
#define UTDR_PRE	0x00000100	/*  receive PaRity Error (read)    */
#define UTDR_FRE	0x00000200	/*  receive FRaming Error (read)   */
#define UTDR_ROR	0x00000400	/*  Receive FIFO Over-Run (read)   */
#endif /* 0 */

#define UTSR0_TFS	0x00000001	/* Transmit FIFO 1/2-full or less  */
					/* Service request (read)	   */
#define UTSR0_RFS	0x00000002	/* Receive FIFO 1/3-to-2/3-full or */
					/* more Service request (read)	   */
#define UTSR0_RID	0x00000004	/* Receiver IDle		   */
#define UTSR0_RBB	0x00000008	/* Receive Beginning of Break	   */
#define UTSR0_REB	0x00000010	/* Receive End of Break		   */
#define UTSR0_EIF	0x00000020	/* Error In FIFO (read)		   */

#define UTSR1_TBY	0x00000001	/* Transmitter BusY (read)	   */
#define UTSR1_RNE	0x00000002	/* Receive FIFO Not Empty (read)   */
#define UTSR1_TNF	0x00000004	/* Transmit FIFO Not Full (read)   */
#define UTSR1_PRE	0x00000008	/* receive PaRity Error (read)	   */
#define UTSR1_FRE	0x00000010	/* receive FRaming Error (read)    */
#define UTSR1_ROR	0x00000020	/* Receive FIFO Over-Run (read)    */


/*
 * Synchronous Data Link Controller (SDLC) control registers
 *
 * Registers
 *    Ser1SDCR0		Serial port 1 Synchronous Data Link Controller (SDLC)
 *			Control Register 0 (read/write).
 *    Ser1SDCR1		Serial port 1 Synchronous Data Link Controller (SDLC)
 *			Control Register 1 (read/write).
 *    Ser1SDCR2		Serial port 1 Synchronous Data Link Controller (SDLC)
 *			Control Register 2 (read/write).
 *    Ser1SDCR3		Serial port 1 Synchronous Data Link Controller (SDLC)
 *			Control Register 3 (read/write).
 *    Ser1SDCR4		Serial port 1 Synchronous Data Link Controller (SDLC)
 *			Control Register 4 (read/write).
 *    Ser1SDDR		Serial port 1 Synchronous Data Link Controller (SDLC)
 *			Data Register (read/write).
 *    Ser1SDSR0		Serial port 1 Synchronous Data Link Controller (SDLC)
 *			Status Register 0 (read/write).
 *    Ser1SDSR1		Serial port 1 Synchronous Data Link Controller (SDLC)
 *			Status Register 1 (read/write).
 *
 * Clocks
 *    fxtl, Txtl	Frequency, period of the system crystal (3.6864 MHz
 *			or 3.5795 MHz).
 *    fsd, Tsd		Frequency, period of the SDLC communication.
 */

#define _Ser1SDCR0	0x80020060	/* Ser. port 1 SDLC Control Reg. 0 */
#define _Ser1SDCR1	0x80020064	/* Ser. port 1 SDLC Control Reg. 1 */
#define _Ser1SDCR2	0x80020068	/* Ser. port 1 SDLC Control Reg. 2 */
#define _Ser1SDCR3	0x8002006C	/* Ser. port 1 SDLC Control Reg. 3 */
#define _Ser1SDCR4	0x80020070	/* Ser. port 1 SDLC Control Reg. 4 */
#define _Ser1SDDR	0x80020078	/* Ser. port 1 SDLC Data Reg.	   */
#define _Ser1SDSR0	0x80020080	/* Ser. port 1 SDLC Status Reg. 0  */
#define _Ser1SDSR1	0x80020084	/* Ser. port 1 SDLC Status Reg. 1  */

#if LANGUAGE == C
#define Ser1SDCR0			/* Ser. port 1 SDLC Control Reg. 0 */ \
			(*((volatile Word *) io_p2v (_Ser1SDCR0)))
#define Ser1SDCR1			/* Ser. port 1 SDLC Control Reg. 1 */ \
			(*((volatile Word *) io_p2v (_Ser1SDCR1)))
#define Ser1SDCR2			/* Ser. port 1 SDLC Control Reg. 2 */ \
			(*((volatile Word *) io_p2v (_Ser1SDCR2)))
#define Ser1SDCR3			/* Ser. port 1 SDLC Control Reg. 3 */ \
			(*((volatile Word *) io_p2v (_Ser1SDCR3)))
#define Ser1SDCR4			/* Ser. port 1 SDLC Control Reg. 4 */ \
			(*((volatile Word *) io_p2v (_Ser1SDCR4)))
#define Ser1SDDR			/* Ser. port 1 SDLC Data Reg.	   */ \
			(*((volatile Word *) io_p2v (_Ser1SDDR)))
#define Ser1SDSR0			/* Ser. port 1 SDLC Status Reg. 0  */ \
			(*((volatile Word *) io_p2v (_Ser1SDSR0)))
#define Ser1SDSR1			/* Ser. port 1 SDLC Status Reg. 1  */ \
			(*((volatile Word *) io_p2v (_Ser1SDSR1)))
#endif /* LANGUAGE == C */

#define SDCR0_SUS	0x00000001	/* SDLC/UART Select		   */
#define SDCR0_SDLC	(SDCR0_SUS*0)	/*  SDLC mode (TXD1 & RXD1)	   */
#define SDCR0_UART	(SDCR0_SUS*1)	/*  UART mode (TXD1 & RXD1)	   */
#define SDCR0_SDF	0x00000002	/* Single/Double start Flag select */
#define SDCR0_SglFlg	(SDCR0_SDF*0)	/*  Single start Flag		   */
#define SDCR0_DblFlg	(SDCR0_SDF*1)	/*  Double start Flag		   */
#define SDCR0_LBM	0x00000004	/* Look-Back Mode		   */
#define SDCR0_BMS	0x00000008	/* Bit Modulation Select	   */
#define SDCR0_FM0	(SDCR0_BMS*0)	/*  Freq. Modulation zero (0)	   */
#define SDCR0_NRZ	(SDCR0_BMS*1)	/*  Non-Return to Zero modulation  */
#define SDCR0_SCE	0x00000010	/* Sample Clock Enable (GPIO [16]) */
#define SDCR0_SCD	0x00000020	/* Sample Clock Direction select   */
					/* (GPIO [16])			   */
#define SDCR0_SClkIn	(SDCR0_SCD*0)	/*  Sample Clock Input		   */
#define SDCR0_SClkOut	(SDCR0_SCD*1)	/*  Sample Clock Output		   */
#define SDCR0_RCE	0x00000040	/* Receive Clock Edge select	   */
#define SDCR0_RcRsEdg	(SDCR0_RCE*0)	/*  Receive clock Rising-Edge	   */
#define SDCR0_RcFlEdg	(SDCR0_RCE*1)	/*  Receive clock Falling-Edge	   */
#define SDCR0_TCE	0x00000080	/* Transmit Clock Edge select	   */
#define SDCR0_TrRsEdg	(SDCR0_TCE*0)	/*  Transmit clock Rising-Edge	   */
#define SDCR0_TrFlEdg	(SDCR0_TCE*1)	/*  Transmit clock Falling-Edge    */

#define SDCR1_AAF	0x00000001	/* Abort After Frame enable	   */
					/* (GPIO [17])			   */
#define SDCR1_TXE	0x00000002	/* Transmit Enable		   */
#define SDCR1_RXE	0x00000004	/* Receive Enable		   */
#define SDCR1_RIE	0x00000008	/* Receive FIFO 1/3-to-2/3-full or */
					/* more Interrupt Enable	   */
#define SDCR1_TIE	0x00000010	/* Transmit FIFO 1/2-full or less  */
					/* Interrupt Enable		   */
#define SDCR1_AME	0x00000020	/* Address Match Enable		   */
#define SDCR1_TUS	0x00000040	/* Transmit FIFO Under-run Select  */
#define SDCR1_EFrmURn	(SDCR1_TUS*0)	/*  End Frame on Under-Run	   */
#define SDCR1_AbortURn	(SDCR1_TUS*1)	/*  Abort on Under-Run		   */
#define SDCR1_RAE	0x00000080	/* Receive Abort interrupt Enable  */

#define SDCR2_AMV	Fld (8, 0)	/* Address Match Value		   */

#define SDCR3_BRD	Fld (4, 0)	/* Baud Rate Divisor/16 - 1 [11:8] */
#define SDCR4_BRD	Fld (8, 0)	/* Baud Rate Divisor/16 - 1  [7:0] */
					/* fsd = fxtl/(16*(BRD[11:0] + 1)) */
					/* Tsd = 16*(BRD[11:0] + 1)*Txtl   */
#define SDCR3_BdRtDiv(Div)		/*  Baud Rate Divisor [16..65536]  */ \
			(((Div) - 16)/16 >> FSize (SDCR4_BRD) << \
			 FShft (SDCR3_BRD))
#define SDCR4_BdRtDiv(Div)		/*  Baud Rate Divisor [16..65536]  */ \
			(((Div) - 16)/16 & FAlnMsk (SDCR4_BRD) << \
			 FShft (SDCR4_BRD))
					/*  fsd = fxtl/(16*Floor (Div/16)) */
					/*  Tsd = 16*Floor (Div/16)*Txtl   */
#define SDCR3_CeilBdRtDiv(Div)		/*  Ceil. of BdRtDiv [16..65536]   */ \
			(((Div) - 1)/16 >> FSize (SDCR4_BRD) << \
			 FShft (SDCR3_BRD))
#define SDCR4_CeilBdRtDiv(Div)		/*  Ceil. of BdRtDiv [16..65536]   */ \
			(((Div) - 1)/16 & FAlnMsk (SDCR4_BRD) << \
			 FShft (SDCR4_BRD))
					/*  fsd = fxtl/(16*Ceil (Div/16))  */
					/*  Tsd = 16*Ceil (Div/16)*Txtl    */

#define SDDR_DATA	Fld (8, 0)	/* receive/transmit DATA FIFOs	   */
#if 0					/* Hidden receive FIFO bits	   */
#define SDDR_EOF	0x00000100	/*  receive End-Of-Frame (read)    */
#define SDDR_CRE	0x00000200	/*  receive CRC Error (read)	   */
#define SDDR_ROR	0x00000400	/*  Receive FIFO Over-Run (read)   */
#endif /* 0 */

#define SDSR0_EIF	0x00000001	/* Error In FIFO (read)		   */
#define SDSR0_TUR	0x00000002	/* Transmit FIFO Under-Run	   */
#define SDSR0_RAB	0x00000004	/* Receive ABort		   */
#define SDSR0_TFS	0x00000008	/* Transmit FIFO 1/2-full or less  */
					/* Service request (read)	   */
#define SDSR0_RFS	0x00000010	/* Receive FIFO 1/3-to-2/3-full or */
					/* more Service request (read)	   */

#define SDSR1_RSY	0x00000001	/* Receiver SYnchronized (read)    */
#define SDSR1_TBY	0x00000002	/* Transmitter BusY (read)	   */
#define SDSR1_RNE	0x00000004	/* Receive FIFO Not Empty (read)   */
#define SDSR1_TNF	0x00000008	/* Transmit FIFO Not Full (read)   */
#define SDSR1_RTD	0x00000010	/* Receive Transition Detected	   */
#define SDSR1_EOF	0x00000020	/* receive End-Of-Frame (read)	   */
#define SDSR1_CRE	0x00000040	/* receive CRC Error (read)	   */
#define SDSR1_ROR	0x00000080	/* Receive FIFO Over-Run (read)    */


/*
 * High-Speed Serial to Parallel controller (HSSP) control registers
 *
 * Registers
 *    Ser2HSCR0		Serial port 2 High-Speed Serial to Parallel
 *			controller (HSSP) Control Register 0 (read/write).
 *    Ser2HSCR1		Serial port 2 High-Speed Serial to Parallel
 *			controller (HSSP) Control Register 1 (read/write).
 *    Ser2HSDR		Serial port 2 High-Speed Serial to Parallel
 *			controller (HSSP) Data Register (read/write).
 *    Ser2HSSR0		Serial port 2 High-Speed Serial to Parallel
 *			controller (HSSP) Status Register 0 (read/write).
 *    Ser2HSSR1		Serial port 2 High-Speed Serial to Parallel
 *			controller (HSSP) Status Register 1 (read).
 *    Ser2HSCR2		Serial port 2 High-Speed Serial to Parallel
 *			controller (HSSP) Control Register 2 (read/write).
 *			[The HSCR2 register is only implemented in
 *			versions 2.0 (rev. = 8) and higher of the StrongARM
 *			SA-1100.]
 */

#define _Ser2HSCR0	0x80040060	/* Ser. port 2 HSSP Control Reg. 0 */
#define _Ser2HSCR1	0x80040064	/* Ser. port 2 HSSP Control Reg. 1 */
#define _Ser2HSDR	0x8004006C	/* Ser. port 2 HSSP Data Reg.	   */
#define _Ser2HSSR0	0x80040074	/* Ser. port 2 HSSP Status Reg. 0  */
#define _Ser2HSSR1	0x80040078	/* Ser. port 2 HSSP Status Reg. 1  */
#define _Ser2HSCR2	0x90060028	/* Ser. port 2 HSSP Control Reg. 2 */

#if LANGUAGE == C
#define Ser2HSCR0			/* Ser. port 2 HSSP Control Reg. 0 */ \
			(*((volatile Word *) io_p2v (_Ser2HSCR0)))
#define Ser2HSCR1			/* Ser. port 2 HSSP Control Reg. 1 */ \
			(*((volatile Word *) io_p2v (_Ser2HSCR1)))
#define Ser2HSDR			/* Ser. port 2 HSSP Data Reg.	   */ \
			(*((volatile Word *) io_p2v (_Ser2HSDR)))
#define Ser2HSSR0			/* Ser. port 2 HSSP Status Reg. 0  */ \
			(*((volatile Word *) io_p2v (_Ser2HSSR0)))
#define Ser2HSSR1			/* Ser. port 2 HSSP Status Reg. 1  */ \
			(*((volatile Word *) io_p2v (_Ser2HSSR1)))
#define Ser2HSCR2			/* Ser. port 2 HSSP Control Reg. 2 */ \
			(*((volatile Word *) io_p2v (_Ser2HSCR2)))
#endif /* LANGUAGE == C */

#define HSCR0_ITR	0x00000001	/* IrDA Transmission Rate	   */
#define HSCR0_UART	(HSCR0_ITR*0)	/*  UART mode (115.2 kb/s if IrDA) */
#define HSCR0_HSSP	(HSCR0_ITR*1)	/*  HSSP mode (4 Mb/s)		   */
#define HSCR0_LBM	0x00000002	/* Look-Back Mode		   */
#define HSCR0_TUS	0x00000004	/* Transmit FIFO Under-run Select  */
#define HSCR0_EFrmURn	(HSCR0_TUS*0)	/*  End Frame on Under-Run	   */
#define HSCR0_AbortURn	(HSCR0_TUS*1)	/*  Abort on Under-Run		   */
#define HSCR0_TXE	0x00000008	/* Transmit Enable		   */
#define HSCR0_RXE	0x00000010	/* Receive Enable		   */
#define HSCR0_RIE	0x00000020	/* Receive FIFO 2/5-to-3/5-full or */
					/* more Interrupt Enable	   */
#define HSCR0_TIE	0x00000040	/* Transmit FIFO 1/2-full or less  */
					/* Interrupt Enable		   */
#define HSCR0_AME	0x00000080	/* Address Match Enable		   */

#define HSCR1_AMV	Fld (8, 0)	/* Address Match Value		   */

#define HSDR_DATA	Fld (8, 0)	/* receive/transmit DATA FIFOs	   */
#if 0					/* Hidden receive FIFO bits	   */
#define HSDR_EOF	0x00000100	/*  receive End-Of-Frame (read)    */
#define HSDR_CRE	0x00000200	/*  receive CRC Error (read)	   */
#define HSDR_ROR	0x00000400	/*  Receive FIFO Over-Run (read)   */
#endif /* 0 */

#define HSSR0_EIF	0x00000001	/* Error In FIFO (read)		   */
#define HSSR0_TUR	0x00000002	/* Transmit FIFO Under-Run	   */
#define HSSR0_RAB	0x00000004	/* Receive ABort		   */
#define HSSR0_TFS	0x00000008	/* Transmit FIFO 1/2-full or less  */
					/* Service request (read)	   */
#define HSSR0_RFS	0x00000010	/* Receive FIFO 2/5-to-3/5-full or */
					/* more Service request (read)	   */
#define HSSR0_FRE	0x00000020	/* receive FRaming Error	   */

#define HSSR1_RSY	0x00000001	/* Receiver SYnchronized (read)    */
#define HSSR1_TBY	0x00000002	/* Transmitter BusY (read)	   */
#define HSSR1_RNE	0x00000004	/* Receive FIFO Not Empty (read)   */
#define HSSR1_TNF	0x00000008	/* Transmit FIFO Not Full (read)   */
#define HSSR1_EOF	0x00000010	/* receive End-Of-Frame (read)	   */
#define HSSR1_CRE	0x00000020	/* receive CRC Error (read)	   */
#define HSSR1_ROR	0x00000040	/* Receive FIFO Over-Run (read)    */

#define HSCR2_TXP	0x00040000	/* Transmit data Polarity (TXD_2)  */
#define HSCR2_TrDataL	(HSCR2_TXP*0)	/*  Transmit Data active Low	   */
					/*  (inverted)			   */
#define HSCR2_TrDataH	(HSCR2_TXP*1)	/*  Transmit Data active High	   */
					/*  (non-inverted)		   */
#define HSCR2_RXP	0x00080000	/* Receive data Polarity (RXD_2)   */
#define HSCR2_RcDataL	(HSCR2_RXP*0)	/*  Receive Data active Low	   */
					/*  (inverted)			   */
#define HSCR2_RcDataH	(HSCR2_RXP*1)	/*  Receive Data active High	   */
					/*  (non-inverted)		   */


/*
 * Multi-media Communications Port (MCP) control registers
 *
 * Registers
 *    Ser4MCCR0		Serial port 4 Multi-media Communications Port (MCP)
 *			Control Register 0 (read/write).
 *    Ser4MCDR0		Serial port 4 Multi-media Communications Port (MCP)
 *			Data Register 0 (audio, read/write).
 *    Ser4MCDR1		Serial port 4 Multi-media Communications Port (MCP)
 *			Data Register 1 (telecom, read/write).
 *    Ser4MCDR2		Serial port 4 Multi-media Communications Port (MCP)
 *			Data Register 2 (CODEC registers, read/write).
 *    Ser4MCSR		Serial port 4 Multi-media Communications Port (MCP)
 *			Status Register (read/write).
 *    Ser4MCCR1		Serial port 4 Multi-media Communications Port (MCP)
 *			Control Register 1 (read/write).
 *			[The MCCR1 register is only implemented in
 *			versions 2.0 (rev. = 8) and higher of the StrongARM
 *			SA-1100.]
 *
 * Clocks
 *    fmc, Tmc		Frequency, period of the MCP communication (10 MHz,
 *			12 MHz, or GPIO [21]).
 *    faud, Taud	Frequency, period of the audio sampling.
 *    ftcm, Ttcm	Frequency, period of the telecom sampling.
 */

#define _Ser4MCCR0	0x80060000	/* Ser. port 4 MCP Control Reg. 0  */
#define _Ser4MCDR0	0x80060008	/* Ser. port 4 MCP Data Reg. 0	   */
					/* (audio)			   */
#define _Ser4MCDR1	0x8006000C	/* Ser. port 4 MCP Data Reg. 1	   */
					/* (telecom)			   */
#define _Ser4MCDR2	0x80060010	/* Ser. port 4 MCP Data Reg. 2	   */
					/* (CODEC reg.)			   */
#define _Ser4MCSR	0x80060018	/* Ser. port 4 MCP Status Reg.	   */
#define _Ser4MCCR1	0x90060030	/* Ser. port 4 MCP Control Reg. 1  */

#if LANGUAGE == C
#define Ser4MCCR0			/* Ser. port 4 MCP Control Reg. 0  */ \
			(*((volatile Word *) io_p2v (_Ser4MCCR0)))
#define Ser4MCDR0			/* Ser. port 4 MCP Data Reg. 0	   */ \
					/* (audio)			   */ \
			(*((volatile Word *) io_p2v (_Ser4MCDR0)))
#define Ser4MCDR1			/* Ser. port 4 MCP Data Reg. 1	   */ \
					/* (telecom)			   */ \
			(*((volatile Word *) io_p2v (_Ser4MCDR1)))
#define Ser4MCDR2			/* Ser. port 4 MCP Data Reg. 2	   */ \
					/* (CODEC reg.)			   */ \
			(*((volatile Word *) io_p2v (_Ser4MCDR2)))
#define Ser4MCSR			/* Ser. port 4 MCP Status Reg.	   */ \
			(*((volatile Word *) io_p2v (_Ser4MCSR)))
#define Ser4MCCR1			/* Ser. port 4 MCP Control Reg. 1  */ \
			(*((volatile Word *) io_p2v (_Ser4MCCR1)))
#endif /* LANGUAGE == C */

#define MCCR0_ASD	Fld (7, 0)	/* Audio Sampling rate Divisor/32  */
					/* [6..127]			   */
					/* faud = fmc/(32*ASD)		   */
					/* Taud = 32*ASD*Tmc		   */
#define MCCR0_AudSmpDiv(Div)		/*  Audio Sampling rate Divisor    */ \
					/*  [192..4064]			   */ \
			((Div)/32 << FShft (MCCR0_ASD))
					/*  faud = fmc/(32*Floor (Div/32)) */
					/*  Taud = 32*Floor (Div/32)*Tmc   */
#define MCCR0_CeilAudSmpDiv(Div)	/*  Ceil. of AudSmpDiv [192..4064] */ \
			(((Div) + 31)/32 << FShft (MCCR0_ASD))
					/*  faud = fmc/(32*Ceil (Div/32))  */
					/*  Taud = 32*Ceil (Div/32)*Tmc    */
#define MCCR0_TSD	Fld (7, 8)	/* Telecom Sampling rate	   */
					/* Divisor/32 [16..127]		   */
					/* ftcm = fmc/(32*TSD)		   */
					/* Ttcm = 32*TSD*Tmc		   */
#define MCCR0_TcmSmpDiv(Div)		/*  Telecom Sampling rate Divisor  */ \
					/*  [512..4064]			   */ \
			((Div)/32 << FShft (MCCR0_TSD))
					/*  ftcm = fmc/(32*Floor (Div/32)) */
					/*  Ttcm = 32*Floor (Div/32)*Tmc   */
#define MCCR0_CeilTcmSmpDiv(Div)	/*  Ceil. of TcmSmpDiv [512..4064] */ \
			(((Div) + 31)/32 << FShft (MCCR0_TSD))
					/*  ftcm = fmc/(32*Ceil (Div/32))  */
					/*  Ttcm = 32*Ceil (Div/32)*Tmc    */
#define MCCR0_MCE	0x00010000	/* MCP Enable			   */
#define MCCR0_ECS	0x00020000	/* External Clock Select	   */
#define MCCR0_IntClk	(MCCR0_ECS*0)	/*  Internal Clock (10 or 12 MHz)  */
#define MCCR0_ExtClk	(MCCR0_ECS*1)	/*  External Clock (GPIO [21])	   */
#define MCCR0_ADM	0x00040000	/* A/D (audio/telecom) data	   */
					/* sampling/storing Mode	   */
#define MCCR0_VldBit	(MCCR0_ADM*0)	/*  Valid Bit storing mode	   */
#define MCCR0_SmpCnt	(MCCR0_ADM*1)	/*  Sampling Counter storing mode  */
#define MCCR0_TTE	0x00080000	/* Telecom Transmit FIFO 1/2-full  */
					/* or less interrupt Enable	   */
#define MCCR0_TRE	0x00100000	/* Telecom Receive FIFO 1/2-full   */
					/* or more interrupt Enable	   */
#define MCCR0_ATE	0x00200000	/* Audio Transmit FIFO 1/2-full    */
					/* or less interrupt Enable	   */
#define MCCR0_ARE	0x00400000	/* Audio Receive FIFO 1/2-full or  */
					/* more interrupt Enable	   */
#define MCCR0_LBM	0x00800000	/* Look-Back Mode		   */
#define MCCR0_ECP	Fld (2, 24)	/* External Clock Prescaler - 1    */
#define MCCR0_ExtClkDiv(Div)		/*  External Clock Divisor [1..4]  */ \
			(((Div) - 1) << FShft (MCCR0_ECP))

#define MCDR0_DATA	Fld (12, 4)	/* receive/transmit audio DATA	   */
					/* FIFOs			   */

#define MCDR1_DATA	Fld (14, 2)	/* receive/transmit telecom DATA   */
					/* FIFOs			   */

					/* receive/transmit CODEC reg.	   */
					/* FIFOs:			   */
#define MCDR2_DATA	Fld (16, 0)	/*  reg. DATA			   */
#define MCDR2_RW	0x00010000	/*  reg. Read/Write (transmit)	   */
#define MCDR2_Rd	(MCDR2_RW*0)	/*   reg. Read			   */
#define MCDR2_Wr	(MCDR2_RW*1)	/*   reg. Write			   */
#define MCDR2_ADD	Fld (4, 17)	/*  reg. ADDress		   */

#define MCSR_ATS	0x00000001	/* Audio Transmit FIFO 1/2-full    */
					/* or less Service request (read)  */
#define MCSR_ARS	0x00000002	/* Audio Receive FIFO 1/2-full or  */
					/* more Service request (read)	   */
#define MCSR_TTS	0x00000004	/* Telecom Transmit FIFO 1/2-full  */
					/* or less Service request (read)  */
#define MCSR_TRS	0x00000008	/* Telecom Receive FIFO 1/2-full   */
					/* or more Service request (read)  */
#define MCSR_ATU	0x00000010	/* Audio Transmit FIFO Under-run   */
#define MCSR_ARO	0x00000020	/* Audio Receive FIFO Over-run	   */
#define MCSR_TTU	0x00000040	/* Telecom Transmit FIFO Under-run */
#define MCSR_TRO	0x00000080	/* Telecom Receive FIFO Over-run   */
#define MCSR_ANF	0x00000100	/* Audio transmit FIFO Not Full    */
					/* (read)			   */
#define MCSR_ANE	0x00000200	/* Audio receive FIFO Not Empty    */
					/* (read)			   */
#define MCSR_TNF	0x00000400	/* Telecom transmit FIFO Not Full  */
					/* (read)			   */
#define MCSR_TNE	0x00000800	/* Telecom receive FIFO Not Empty  */
					/* (read)			   */
#define MCSR_CWC	0x00001000	/* CODEC register Write Completed  */
					/* (read)			   */
#define MCSR_CRC	0x00002000	/* CODEC register Read Completed   */
					/* (read)			   */
#define MCSR_ACE	0x00004000	/* Audio CODEC Enabled (read)	   */
#define MCSR_TCE	0x00008000	/* Telecom CODEC Enabled (read)    */

#define MCCR1_CFS	0x00100000	/* Clock Freq. Select		   */
#define MCCR1_F12MHz	(MCCR1_CFS*0)	/*  Freq. (fmc) = ~ 12 MHz	   */
					/*  (11.981 MHz)		   */
#define MCCR1_F10MHz	(MCCR1_CFS*1)	/*  Freq. (fmc) = ~ 10 MHz	   */
					/*  (9.585 MHz)			   */


/*
 * Synchronous Serial Port (SSP) control registers
 *
 * Registers
 *    Ser4SSCR0		Serial port 4 Synchronous Serial Port (SSP) Control
 *			Register 0 (read/write).
 *    Ser4SSCR1		Serial port 4 Synchronous Serial Port (SSP) Control
 *			Register 1 (read/write).
 *			[Bits SPO and SP are only implemented in versions 2.0
 *			(rev. = 8) and higher of the StrongARM SA-1100.]
 *    Ser4SSDR		Serial port 4 Synchronous Serial Port (SSP) Data
 *			Register (read/write).
 *    Ser4SSSR		Serial port 4 Synchronous Serial Port (SSP) Status
 *			Register (read/write).
 *
 * Clocks
 *    fxtl, Txtl	Frequency, period of the system crystal (3.6864 MHz
 *			or 3.5795 MHz).
 *    fss, Tss		Frequency, period of the SSP communication.
 */

#define _Ser4SSCR0	0x80070060	/* Ser. port 4 SSP Control Reg. 0  */
#define _Ser4SSCR1	0x80070064	/* Ser. port 4 SSP Control Reg. 1  */
#define _Ser4SSDR	0x8007006C	/* Ser. port 4 SSP Data Reg.	   */
#define _Ser4SSSR	0x80070074	/* Ser. port 4 SSP Status Reg.	   */

#if LANGUAGE == C
#define Ser4SSCR0			/* Ser. port 4 SSP Control Reg. 0  */ \
			(*((volatile Word *) io_p2v (_Ser4SSCR0)))
#define Ser4SSCR1			/* Ser. port 4 SSP Control Reg. 1  */ \
			(*((volatile Word *) io_p2v (_Ser4SSCR1)))
#define Ser4SSDR			/* Ser. port 4 SSP Data Reg.	   */ \
			(*((volatile Word *) io_p2v (_Ser4SSDR)))
#define Ser4SSSR			/* Ser. port 4 SSP Status Reg.	   */ \
			(*((volatile Word *) io_p2v (_Ser4SSSR)))
#endif /* LANGUAGE == C */

#define SSCR0_DSS	Fld (4, 0)	/* Data Size - 1 Select [3..15]    */
#define SSCR0_DataSize(Size)		/*  Data Size Select [4..16]	   */ \
			(((Size) - 1) << FShft (SSCR0_DSS))
#define SSCR0_FRF	Fld (2, 4)	/* FRame Format			   */
#define SSCR0_Motorola			/*  Motorola Serial Peripheral	   */ \
					/*  Interface (SPI) format	   */ \
			(0 << FShft (SSCR0_FRF))
#define SSCR0_TI			/*  Texas Instruments Synchronous  */ \
					/*  Serial format		   */ \
			(1 << FShft (SSCR0_FRF))
#define SSCR0_National			/*  National Microwire format	   */ \
			(2 << FShft (SSCR0_FRF))
#define SSCR0_SSE	0x00000080	/* SSP Enable			   */
#define SSCR0_SCR	Fld (8, 8)	/* Serial Clock Rate divisor/2 - 1 */
					/* fss = fxtl/(2*(SCR + 1))	   */
					/* Tss = 2*(SCR + 1)*Txtl	   */
#define SSCR0_SerClkDiv(Div)		/*  Serial Clock Divisor [2..512]  */ \
			(((Div) - 2)/2 << FShft (SSCR0_SCR))
					/*  fss = fxtl/(2*Floor (Div/2))   */
					/*  Tss = 2*Floor (Div/2)*Txtl	   */
#define SSCR0_CeilSerClkDiv(Div)	/*  Ceil. of SerClkDiv [2..512]    */ \
			(((Div) - 1)/2 << FShft (SSCR0_SCR))
					/*  fss = fxtl/(2*Ceil (Div/2))    */
					/*  Tss = 2*Ceil (Div/2)*Txtl	   */

#define SSCR1_RIE	0x00000001	/* Receive FIFO 1/2-full or more   */
					/* Interrupt Enable		   */
#define SSCR1_TIE	0x00000002	/* Transmit FIFO 1/2-full or less  */
					/* Interrupt Enable		   */
#define SSCR1_LBM	0x00000004	/* Look-Back Mode		   */
#define SSCR1_SPO	0x00000008	/* Sample clock (SCLK) POlarity    */
#define SSCR1_SClkIactL	(SSCR1_SPO*0)	/*  Sample Clock Inactive Low	   */
#define SSCR1_SClkIactH	(SSCR1_SPO*1)	/*  Sample Clock Inactive High	   */
#define SSCR1_SP	0x00000010	/* Sample clock (SCLK) Phase	   */
#define SSCR1_SClk1P	(SSCR1_SP*0)	/*  Sample Clock active 1 Period   */
					/*  after frame (SFRM, 1st edge)   */
#define SSCR1_SClk1_2P	(SSCR1_SP*1)	/*  Sample Clock active 1/2 Period */
					/*  after frame (SFRM, 1st edge)   */
#define SSCR1_ECS	0x00000020	/* External Clock Select	   */
#define SSCR1_IntClk	(SSCR1_ECS*0)	/*  Internal Clock		   */
#define SSCR1_ExtClk	(SSCR1_ECS*1)	/*  External Clock (GPIO [19])	   */

#define SSDR_DATA	Fld (16, 0)	/* receive/transmit DATA FIFOs	   */

#define SSSR_TNF	0x00000002	/* Transmit FIFO Not Full (read)   */
#define SSSR_RNE	0x00000004	/* Receive FIFO Not Empty (read)   */
#define SSSR_BSY	0x00000008	/* SSP BuSY (read)		   */
#define SSSR_TFS	0x00000010	/* Transmit FIFO 1/2-full or less  */
					/* Service request (read)	   */
#define SSSR_RFS	0x00000020	/* Receive FIFO 1/2-full or more   */
					/* Service request (read)	   */
#define SSSR_ROR	0x00000040	/* Receive FIFO Over-Run	   */


/*
 * Operating System (OS) timer control registers
 *
 * Registers
 *    OSMR0		Operating System (OS) timer Match Register 0
 *			(read/write).
 *    OSMR1		Operating System (OS) timer Match Register 1
 *			(read/write).
 *    OSMR2		Operating System (OS) timer Match Register 2
 *			(read/write).
 *    OSMR3		Operating System (OS) timer Match Register 3
 *			(read/write).
 *    OSCR		Operating System (OS) timer Counter Register
 *			(read/write).
 *    OSSR		Operating System (OS) timer Status Register
 *			(read/write).
 *    OWER		Operating System (OS) timer Watch-dog Enable Register
 *			(read/write).
 *    OIER		Operating System (OS) timer Interrupt Enable Register
 *			(read/write).
 */

#define _OSMR(Nb)			/* OS timer Match Reg. [0..3]	   */ \
			(0x90000000 + (Nb)*4)
#define _OSMR0		_OSMR (0)	/* OS timer Match Reg. 0	   */
#define _OSMR1		_OSMR (1)	/* OS timer Match Reg. 1	   */
#define _OSMR2		_OSMR (2)	/* OS timer Match Reg. 2	   */
#define _OSMR3		_OSMR (3)	/* OS timer Match Reg. 3	   */
#define _OSCR		0x90000010	/* OS timer Counter Reg.	   */
#define _OSSR		0x90000014	/* OS timer Status Reg.		   */
#define _OWER		0x90000018	/* OS timer Watch-dog Enable Reg.  */
#define _OIER		0x9000001C	/* OS timer Interrupt Enable Reg.  */

#if LANGUAGE == C
#define OSMR				/* OS timer Match Reg. [0..3]	   */ \
			((volatile Word *) io_p2v (_OSMR (0)))
#define OSMR0		(OSMR [0])	/* OS timer Match Reg. 0	   */
#define OSMR1		(OSMR [1])	/* OS timer Match Reg. 1	   */
#define OSMR2		(OSMR [2])	/* OS timer Match Reg. 2	   */
#define OSMR3		(OSMR [3])	/* OS timer Match Reg. 3	   */
#define OSCR				/* OS timer Counter Reg.	   */ \
			(*((volatile Word *) io_p2v (_OSCR)))
#define OSSR				/* OS timer Status Reg.		   */ \
			(*((volatile Word *) io_p2v (_OSSR)))
#define OWER				/* OS timer Watch-dog Enable Reg.  */ \
			(*((volatile Word *) io_p2v (_OWER)))
#define OIER				/* OS timer Interrupt Enable Reg.  */ \
			(*((volatile Word *) io_p2v (_OIER)))
#endif /* LANGUAGE == C */

#define OSSR_M(Nb)			/* Match detected [0..3]	   */ \
			(0x00000001 << (Nb))
#define OSSR_M0		OSSR_M (0)	/* Match detected 0		   */
#define OSSR_M1		OSSR_M (1)	/* Match detected 1		   */
#define OSSR_M2		OSSR_M (2)	/* Match detected 2		   */
#define OSSR_M3		OSSR_M (3)	/* Match detected 3		   */

#define OWER_WME	0x00000001	/* Watch-dog Match Enable	   */
					/* (set only)			   */

#define OIER_E(Nb)			/* match interrupt Enable [0..3]   */ \
			(0x00000001 << (Nb))
#define OIER_E0		OIER_E (0)	/* match interrupt Enable 0	   */
#define OIER_E1		OIER_E (1)	/* match interrupt Enable 1	   */
#define OIER_E2		OIER_E (2)	/* match interrupt Enable 2	   */
#define OIER_E3		OIER_E (3)	/* match interrupt Enable 3	   */


/*
 * Real-Time Clock (RTC) control registers
 *
 * Registers
 *    RTAR		Real-Time Clock (RTC) Alarm Register (read/write).
 *    RCNR		Real-Time Clock (RTC) CouNt Register (read/write).
 *    RTTR		Real-Time Clock (RTC) Trim Register (read/write).
 *    RTSR		Real-Time Clock (RTC) Status Register (read/write).
 *
 * Clocks
 *    frtx, Trtx	Frequency, period of the real-time clock crystal
 *			(32.768 kHz nominal).
 *    frtc, Trtc	Frequency, period of the real-time clock counter
 *			(1 Hz nominal).
 */

#define _RTAR		0x90010000	/* RTC Alarm Reg.		   */
#define _RCNR		0x90010004	/* RTC CouNt Reg.		   */
#define _RTTR		0x90010008	/* RTC Trim Reg.		   */
#define _RTSR		0x90010010	/* RTC Status Reg.		   */

#if LANGUAGE == C
#define RTAR				/* RTC Alarm Reg.		   */ \
			(*((volatile Word *) io_p2v (_RTAR)))
#define RCNR				/* RTC CouNt Reg.		   */ \
			(*((volatile Word *) io_p2v (_RCNR)))
#define RTTR				/* RTC Trim Reg.		   */ \
			(*((volatile Word *) io_p2v (_RTTR)))
#define RTSR				/* RTC Status Reg.		   */ \
			(*((volatile Word *) io_p2v (_RTSR)))
#endif /* LANGUAGE == C */

#define RTTR_C		Fld (16, 0)	/* clock divider Count - 1	   */
#define RTTR_D		Fld (10, 16)	/* trim Delete count		   */
					/* frtc = (1023*(C + 1) - D)*frtx/ */
					/*	  (1023*(C + 1)^2)	   */
					/* Trtc = (1023*(C + 1)^2)*Trtx/   */
					/*	  (1023*(C + 1) - D)	   */

#define RTSR_AL		0x00000001	/* ALarm detected		   */
#define RTSR_HZ		0x00000002	/* 1 Hz clock detected		   */
#define RTSR_ALE	0x00000004	/* ALarm interrupt Enable	   */
#define RTSR_HZE	0x00000008	/* 1 Hz clock interrupt Enable	   */


/*
 * Power Manager (PM) control registers
 *
 * Registers
 *    PMCR		Power Manager (PM) Control Register (read/write).
 *    PSSR		Power Manager (PM) Sleep Status Register (read/write).
 *    PSPR		Power Manager (PM) Scratch-Pad Register (read/write).
 *    PWER		Power Manager (PM) Wake-up Enable Register
 *			(read/write).
 *    PCFR		Power Manager (PM) general ConFiguration Register
 *			(read/write).
 *    PPCR		Power Manager (PM) Phase-Locked Loop (PLL)
 *			Configuration Register (read/write).
 *    PGSR		Power Manager (PM) General-Purpose Input/Output (GPIO)
 *			Sleep state Register (read/write, see GPIO pins).
 *    POSR		Power Manager (PM) Oscillator Status Register (read).
 *
 * Clocks
 *    fxtl, Txtl	Frequency, period of the system crystal (3.6864 MHz
 *			or 3.5795 MHz).
 *    fcpu, Tcpu	Frequency, period of the CPU core clock (CCLK).
 */

#define _PMCR		0x90020000	/* PM Control Reg.		   */
#define _PSSR		0x90020004	/* PM Sleep Status Reg.		   */
#define _PSPR		0x90020008	/* PM Scratch-Pad Reg.		   */
#define _PWER		0x9002000C	/* PM Wake-up Enable Reg.	   */
#define _PCFR		0x90020010	/* PM general ConFiguration Reg.   */
#define _PPCR		0x90020014	/* PM PLL Configuration Reg.	   */
#define _PGSR		0x90020018	/* PM GPIO Sleep state Reg.	   */
#define _POSR		0x9002001C	/* PM Oscillator Status Reg.	   */

#if LANGUAGE == C
#define PMCR				/* PM Control Reg.		   */ \
			(*((volatile Word *) io_p2v (_PMCR)))
#define PSSR				/* PM Sleep Status Reg.		   */ \
			(*((volatile Word *) io_p2v (_PSSR)))
#define PSPR				/* PM Scratch-Pad Reg.		   */ \
			(*((volatile Word *) io_p2v (_PSPR)))
#define PWER				/* PM Wake-up Enable Reg.	   */ \
			(*((volatile Word *) io_p2v (_PWER)))
#define PCFR				/* PM general ConFiguration Reg.   */ \
			(*((volatile Word *) io_p2v (_PCFR)))
#define PPCR				/* PM PLL Configuration Reg.	   */ \
			(*((volatile Word *) io_p2v (_PPCR)))
#define PGSR				/* PM GPIO Sleep state Reg.	   */ \
			(*((volatile Word *) io_p2v (_PGSR)))
#define POSR				/* PM Oscillator Status Reg.	   */ \
			(*((volatile Word *) io_p2v (_POSR)))

#elif LANGUAGE == Assembly
#define PMCR		(io_p2v (_PMCR))
#define PSSR		(io_p2v (_PSSR))
#define PSPR		(io_p2v (_PSPR))
#define PWER		(io_p2v (_PWER))
#define PCFR		(io_p2v (_PCFR))
#define PPCR		(io_p2v (_PPCR))
#define PGSR		(io_p2v (_PGSR))
#define POSR		(io_p2v (_POSR))

#endif /* LANGUAGE == C */

#define PMCR_SF		0x00000001	/* Sleep Force (set only)	   */

#define PSSR_SS		0x00000001	/* Software Sleep		   */
#define PSSR_BFS	0x00000002	/* Battery Fault Status		   */
					/* (BATT_FAULT)			   */
#define PSSR_VFS	0x00000004	/* Vdd Fault Status (VDD_FAULT)    */
#define PSSR_DH		0x00000008	/* DRAM control Hold		   */
#define PSSR_PH		0x00000010	/* Peripheral control Hold	   */

#define PWER_GPIO(Nb)	GPIO_GPIO (Nb)	/* GPIO [0..27] wake-up enable	   */
#define PWER_GPIO0	PWER_GPIO (0)	/* GPIO  [0] wake-up enable	   */
#define PWER_GPIO1	PWER_GPIO (1)	/* GPIO  [1] wake-up enable	   */
#define PWER_GPIO2	PWER_GPIO (2)	/* GPIO  [2] wake-up enable	   */
#define PWER_GPIO3	PWER_GPIO (3)	/* GPIO  [3] wake-up enable	   */
#define PWER_GPIO4	PWER_GPIO (4)	/* GPIO  [4] wake-up enable	   */
#define PWER_GPIO5	PWER_GPIO (5)	/* GPIO  [5] wake-up enable	   */
#define PWER_GPIO6	PWER_GPIO (6)	/* GPIO  [6] wake-up enable	   */
#define PWER_GPIO7	PWER_GPIO (7)	/* GPIO  [7] wake-up enable	   */
#define PWER_GPIO8	PWER_GPIO (8)	/* GPIO  [8] wake-up enable	   */
#define PWER_GPIO9	PWER_GPIO (9)	/* GPIO  [9] wake-up enable	   */
#define PWER_GPIO10	PWER_GPIO (10)	/* GPIO [10] wake-up enable	   */
#define PWER_GPIO11	PWER_GPIO (11)	/* GPIO [11] wake-up enable	   */
#define PWER_GPIO12	PWER_GPIO (12)	/* GPIO [12] wake-up enable	   */
#define PWER_GPIO13	PWER_GPIO (13)	/* GPIO [13] wake-up enable	   */
#define PWER_GPIO14	PWER_GPIO (14)	/* GPIO [14] wake-up enable	   */
#define PWER_GPIO15	PWER_GPIO (15)	/* GPIO [15] wake-up enable	   */
#define PWER_GPIO16	PWER_GPIO (16)	/* GPIO [16] wake-up enable	   */
#define PWER_GPIO17	PWER_GPIO (17)	/* GPIO [17] wake-up enable	   */
#define PWER_GPIO18	PWER_GPIO (18)	/* GPIO [18] wake-up enable	   */
#define PWER_GPIO19	PWER_GPIO (19)	/* GPIO [19] wake-up enable	   */
#define PWER_GPIO20	PWER_GPIO (20)	/* GPIO [20] wake-up enable	   */
#define PWER_GPIO21	PWER_GPIO (21)	/* GPIO [21] wake-up enable	   */
#define PWER_GPIO22	PWER_GPIO (22)	/* GPIO [22] wake-up enable	   */
#define PWER_GPIO23	PWER_GPIO (23)	/* GPIO [23] wake-up enable	   */
#define PWER_GPIO24	PWER_GPIO (24)	/* GPIO [24] wake-up enable	   */
#define PWER_GPIO25	PWER_GPIO (25)	/* GPIO [25] wake-up enable	   */
#define PWER_GPIO26	PWER_GPIO (26)	/* GPIO [26] wake-up enable	   */
#define PWER_GPIO27	PWER_GPIO (27)	/* GPIO [27] wake-up enable	   */
#define PWER_RTC	0x80000000	/* RTC alarm wake-up enable	   */

#define PCFR_OPDE	0x00000001	/* Oscillator Power-Down Enable    */
#define PCFR_ClkRun	(PCFR_OPDE*0)	/*  Clock Running in sleep mode    */
#define PCFR_ClkStp	(PCFR_OPDE*1)	/*  Clock Stopped in sleep mode    */
#define PCFR_FP		0x00000002	/* Float PCMCIA pins		   */
#define PCFR_PCMCIANeg	(PCFR_FP*0)	/*  PCMCIA pins Negated (1)	   */
#define PCFR_PCMCIAFlt	(PCFR_FP*1)	/*  PCMCIA pins Floating	   */
#define PCFR_FS		0x00000004	/* Float Static memory pins	   */
#define PCFR_StMemNeg	(PCFR_FS*0)	/*  Static Memory pins Negated (1) */
#define PCFR_StMemFlt	(PCFR_FS*1)	/*  Static Memory pins Floating    */
#define PCFR_FO		0x00000008	/* Force RTC oscillator		   */
					/* (32.768 kHz) enable On	   */

#define PPCR_CCF	Fld (5, 0)	/* CPU core Clock (CCLK) Freq.	   */
#define PPCR_Fx16			/*  Freq. x 16 (fcpu = 16*fxtl)    */ \
			(0x00 << FShft (PPCR_CCF))
#define PPCR_Fx20			/*  Freq. x 20 (fcpu = 20*fxtl)    */ \
			(0x01 << FShft (PPCR_CCF))
#define PPCR_Fx24			/*  Freq. x 24 (fcpu = 24*fxtl)    */ \
			(0x02 << FShft (PPCR_CCF))
#define PPCR_Fx28			/*  Freq. x 28 (fcpu = 28*fxtl)    */ \
			(0x03 << FShft (PPCR_CCF))
#define PPCR_Fx32			/*  Freq. x 32 (fcpu = 32*fxtl)    */ \
			(0x04 << FShft (PPCR_CCF))
#define PPCR_Fx36			/*  Freq. x 36 (fcpu = 36*fxtl)    */ \
			(0x05 << FShft (PPCR_CCF))
#define PPCR_Fx40			/*  Freq. x 40 (fcpu = 40*fxtl)    */ \
			(0x06 << FShft (PPCR_CCF))
#define PPCR_Fx44			/*  Freq. x 44 (fcpu = 44*fxtl)    */ \
			(0x07 << FShft (PPCR_CCF))
#define PPCR_Fx48			/*  Freq. x 48 (fcpu = 48*fxtl)    */ \
			(0x08 << FShft (PPCR_CCF))
#define PPCR_Fx52			/*  Freq. x 52 (fcpu = 52*fxtl)    */ \
			(0x09 << FShft (PPCR_CCF))
#define PPCR_Fx56			/*  Freq. x 56 (fcpu = 56*fxtl)    */ \
			(0x0A << FShft (PPCR_CCF))
#define PPCR_Fx60			/*  Freq. x 60 (fcpu = 60*fxtl)    */ \
			(0x0B << FShft (PPCR_CCF))
#define PPCR_Fx64			/*  Freq. x 64 (fcpu = 64*fxtl)    */ \
			(0x0C << FShft (PPCR_CCF))
#define PPCR_Fx68			/*  Freq. x 68 (fcpu = 68*fxtl)    */ \
			(0x0D << FShft (PPCR_CCF))
#define PPCR_Fx72			/*  Freq. x 72 (fcpu = 72*fxtl)    */ \
			(0x0E << FShft (PPCR_CCF))
#define PPCR_Fx76			/*  Freq. x 76 (fcpu = 76*fxtl)    */ \
			(0x0F << FShft (PPCR_CCF))
					/*  3.6864 MHz crystal (fxtl):	   */
#define PPCR_F59_0MHz	PPCR_Fx16	/*   Freq. (fcpu) =  59.0 MHz	   */
#define PPCR_F73_7MHz	PPCR_Fx20	/*   Freq. (fcpu) =  73.7 MHz	   */
#define PPCR_F88_5MHz	PPCR_Fx24	/*   Freq. (fcpu) =  88.5 MHz	   */
#define PPCR_F103_2MHz	PPCR_Fx28	/*   Freq. (fcpu) = 103.2 MHz	   */
#define PPCR_F118_0MHz	PPCR_Fx32	/*   Freq. (fcpu) = 118.0 MHz	   */
#define PPCR_F132_7MHz	PPCR_Fx36	/*   Freq. (fcpu) = 132.7 MHz	   */
#define PPCR_F147_5MHz	PPCR_Fx40	/*   Freq. (fcpu) = 147.5 MHz	   */
#define PPCR_F162_2MHz	PPCR_Fx44	/*   Freq. (fcpu) = 162.2 MHz	   */
#define PPCR_F176_9MHz	PPCR_Fx48	/*   Freq. (fcpu) = 176.9 MHz	   */
#define PPCR_F191_7MHz	PPCR_Fx52	/*   Freq. (fcpu) = 191.7 MHz	   */
#define PPCR_F206_4MHz	PPCR_Fx56	/*   Freq. (fcpu) = 206.4 MHz	   */
#define PPCR_F221_2MHz	PPCR_Fx60	/*   Freq. (fcpu) = 221.2 MHz	   */
#define PPCR_F239_6MHz	PPCR_Fx64	/*   Freq. (fcpu) = 239.6 MHz	   */
#define PPCR_F250_7MHz	PPCR_Fx68	/*   Freq. (fcpu) = 250.7 MHz	   */
#define PPCR_F265_4MHz	PPCR_Fx72	/*   Freq. (fcpu) = 265.4 MHz	   */
#define PPCR_F280_2MHz	PPCR_Fx76	/*   Freq. (fcpu) = 280.2 MHz	   */
					/*  3.5795 MHz crystal (fxtl):	   */
#define PPCR_F57_3MHz	PPCR_Fx16	/*   Freq. (fcpu) =  57.3 MHz	   */
#define PPCR_F71_6MHz	PPCR_Fx20	/*   Freq. (fcpu) =  71.6 MHz	   */
#define PPCR_F85_9MHz	PPCR_Fx24	/*   Freq. (fcpu) =  85.9 MHz	   */
#define PPCR_F100_2MHz	PPCR_Fx28	/*   Freq. (fcpu) = 100.2 MHz	   */
#define PPCR_F114_5MHz	PPCR_Fx32	/*   Freq. (fcpu) = 114.5 MHz	   */
#define PPCR_F128_9MHz	PPCR_Fx36	/*   Freq. (fcpu) = 128.9 MHz	   */
#define PPCR_F143_2MHz	PPCR_Fx40	/*   Freq. (fcpu) = 143.2 MHz	   */
#define PPCR_F157_5MHz	PPCR_Fx44	/*   Freq. (fcpu) = 157.5 MHz	   */
#define PPCR_F171_8MHz	PPCR_Fx48	/*   Freq. (fcpu) = 171.8 MHz	   */
#define PPCR_F186_1MHz	PPCR_Fx52	/*   Freq. (fcpu) = 186.1 MHz	   */
#define PPCR_F200_5MHz	PPCR_Fx56	/*   Freq. (fcpu) = 200.5 MHz	   */
#define PPCR_F214_8MHz	PPCR_Fx60	/*   Freq. (fcpu) = 214.8 MHz	   */
#define PPCR_F229_1MHz	PPCR_Fx64	/*   Freq. (fcpu) = 229.1 MHz	   */
#define PPCR_F243_4MHz	PPCR_Fx68	/*   Freq. (fcpu) = 243.4 MHz	   */
#define PPCR_F257_7MHz	PPCR_Fx72	/*   Freq. (fcpu) = 257.7 MHz	   */
#define PPCR_F272_0MHz	PPCR_Fx76	/*   Freq. (fcpu) = 272.0 MHz	   */

#define POSR_OOK	0x00000001	/* RTC Oscillator (32.768 kHz) OK  */


/*
 * Reset Controller (RC) control registers
 *
 * Registers
 *    RSRR		Reset Controller (RC) Software Reset Register
 *			(read/write).
 *    RCSR		Reset Controller (RC) Status Register (read/write).
 */

#define _RSRR		0x90030000	/* RC Software Reset Reg.	   */
#define _RCSR		0x90030004	/* RC Status Reg.		   */

#if LANGUAGE == C
#define RSRR				/* RC Software Reset Reg.	   */ \
			(*((volatile Word *) io_p2v (_RSRR)))
#define RCSR				/* RC Status Reg.		   */ \
			(*((volatile Word *) io_p2v (_RCSR)))
#endif /* LANGUAGE == C */

#define RSRR_SWR	0x00000001	/* SoftWare Reset (set only)	   */

#define RCSR_HWR	0x00000001	/* HardWare Reset		   */
#define RCSR_SWR	0x00000002	/* SoftWare Reset		   */
#define RCSR_WDR	0x00000004	/* Watch-Dog Reset		   */
#define RCSR_SMR	0x00000008	/* Sleep-Mode Reset		   */


/*
 * Test unit control registers
 *
 * Registers
 *    TUCR		Test Unit Control Register (read/write).
 */

#define _TUCR		0x90030008	/* Test Unit Control Reg.	   */

#if LANGUAGE == C
#define TUCR				/* Test Unit Control Reg.	   */ \
			(*((volatile Word *) io_p2v (_TUCR)))
#endif /* LANGUAGE == C */

#define TUCR_TIC	0x00000040	/* TIC mode			   */
#define TUCR_TTST	0x00000080	/* Trim TeST mode		   */
#define TUCR_RCRC	0x00000100	/* Richard's Cyclic Redundancy	   */
					/* Check			   */
#define TUCR_PMD	0x00000200	/* Power Management Disable	   */
#define TUCR_MR		0x00000400	/* Memory Request mode		   */
#define TUCR_NoMB	(TUCR_MR*0)	/*  No Memory Bus request & grant  */
#define TUCR_MBGPIO	(TUCR_MR*1)	/*  Memory Bus request (MBREQ) &   */
					/*  grant (MBGNT) on GPIO [22:21]  */
#define TUCR_CTB	Fld (3, 20)	/* Clock Test Bits		   */
#define TUCR_FDC	0x00800000	/* RTC Force Delete Count	   */
#define TUCR_FMC	0x01000000	/* Force Michelle's Control mode   */
#define TUCR_TMC	0x02000000	/* RTC Trimmer Multiplexer Control */
#define TUCR_DPS	0x04000000	/* Disallow Pad Sleep		   */
#define TUCR_TSEL	Fld (3, 29)	/* clock Test SELect on GPIO [27]  */
#define TUCR_32_768kHz			/*  32.768 kHz osc. on GPIO [27]   */ \
			(0 << FShft (TUCR_TSEL))
#define TUCR_3_6864MHz			/*  3.6864 MHz osc. on GPIO [27]   */ \
			(1 << FShft (TUCR_TSEL))
#define TUCR_VDD			/*  VDD ring osc./16 on GPIO [27]  */ \
			(2 << FShft (TUCR_TSEL))
#define TUCR_96MHzPLL			/*  96 MHz PLL/4 on GPIO [27]	   */ \
			(3 << FShft (TUCR_TSEL))
#define TUCR_Clock			/*  internal (fcpu/2) & 32.768 kHz */ \
					/*  Clocks on GPIO [26:27]	   */ \
			(4 << FShft (TUCR_TSEL))
#define TUCR_3_6864MHzA			/*  3.6864 MHz osc. on GPIO [27]   */ \
					/*  (Alternative)		   */ \
			(5 << FShft (TUCR_TSEL))
#define TUCR_MainPLL			/*  Main PLL/16 on GPIO [27]	   */ \
			(6 << FShft (TUCR_TSEL))
#define TUCR_VDDL			/*  VDDL ring osc./4 on GPIO [27]  */ \
			(7 << FShft (TUCR_TSEL))


/*
 * General-Purpose Input/Output (GPIO) control registers
 *
 * Registers
 *    GPLR		General-Purpose Input/Output (GPIO) Pin Level
 *			Register (read).
 *    GPDR		General-Purpose Input/Output (GPIO) Pin Direction
 *			Register (read/write).
 *    GPSR		General-Purpose Input/Output (GPIO) Pin output Set
 *			Register (write).
 *    GPCR		General-Purpose Input/Output (GPIO) Pin output Clear
 *			Register (write).
 *    GRER		General-Purpose Input/Output (GPIO) Rising-Edge
 *			detect Register (read/write).
 *    GFER		General-Purpose Input/Output (GPIO) Falling-Edge
 *			detect Register (read/write).
 *    GEDR		General-Purpose Input/Output (GPIO) Edge Detect
 *			status Register (read/write).
 *    GAFR		General-Purpose Input/Output (GPIO) Alternate
 *			Function Register (read/write).
 *
 * Clock
 *    fcpu, Tcpu	Frequency, period of the CPU core clock (CCLK).
 */

#define _GPLR		0x90040000	/* GPIO Pin Level Reg.		   */
#define _GPDR		0x90040004	/* GPIO Pin Direction Reg.	   */
#define _GPSR		0x90040008	/* GPIO Pin output Set Reg.	   */
#define _GPCR		0x9004000C	/* GPIO Pin output Clear Reg.	   */
#define _GRER		0x90040010	/* GPIO Rising-Edge detect Reg.    */
#define _GFER		0x90040014	/* GPIO Falling-Edge detect Reg.   */
#define _GEDR		0x90040018	/* GPIO Edge Detect status Reg.    */
#define _GAFR		0x9004001C	/* GPIO Alternate Function Reg.    */

#if LANGUAGE == C
#define GPLR				/* GPIO Pin Level Reg.		   */ \
			(*((volatile Word *) io_p2v (_GPLR)))
#define GPDR				/* GPIO Pin Direction Reg.	   */ \
			(*((volatile Word *) io_p2v (_GPDR)))
#define GPSR				/* GPIO Pin output Set Reg.	   */ \
			(*((volatile Word *) io_p2v (_GPSR)))
#define GPCR				/* GPIO Pin output Clear Reg.	   */ \
			(*((volatile Word *) io_p2v (_GPCR)))
#define GRER				/* GPIO Rising-Edge detect Reg.    */ \
			(*((volatile Word *) io_p2v (_GRER)))
#define GFER				/* GPIO Falling-Edge detect Reg.   */ \
			(*((volatile Word *) io_p2v (_GFER)))
#define GEDR				/* GPIO Edge Detect status Reg.    */ \
			(*((volatile Word *) io_p2v (_GEDR)))
#define GAFR				/* GPIO Alternate Function Reg.    */ \
			(*((volatile Word *) io_p2v (_GAFR)))
#elif LANGUAGE == Assembly

#define GPLR  (io_p2v (_GPLR))
#define GPDR  (io_p2v (_GPDR))
#define GPSR  (io_p2v (_GPSR))
#define GPCR  (io_p2v (_GPCR))
#define GRER  (io_p2v (_GRER))
#define GFER  (io_p2v (_GFER))
#define GEDR  (io_p2v (_GEDR))
#define GAFR  (io_p2v (_GAFR))

#endif /* LANGUAGE == C */

#define GPIO_MIN	(0)
#define GPIO_MAX	(27)

#define GPIO_GPIO(Nb)			/* GPIO [0..27]			   */ \
			(0x00000001 << (Nb))
#define GPIO_GPIO0	GPIO_GPIO (0)	/* GPIO  [0]			   */
#define GPIO_GPIO1	GPIO_GPIO (1)	/* GPIO  [1]			   */
#define GPIO_GPIO2	GPIO_GPIO (2)	/* GPIO  [2]			   */
#define GPIO_GPIO3	GPIO_GPIO (3)	/* GPIO  [3]			   */
#define GPIO_GPIO4	GPIO_GPIO (4)	/* GPIO  [4]			   */
#define GPIO_GPIO5	GPIO_GPIO (5)	/* GPIO  [5]			   */
#define GPIO_GPIO6	GPIO_GPIO (6)	/* GPIO  [6]			   */
#define GPIO_GPIO7	GPIO_GPIO (7)	/* GPIO  [7]			   */
#define GPIO_GPIO8	GPIO_GPIO (8)	/* GPIO  [8]			   */
#define GPIO_GPIO9	GPIO_GPIO (9)	/* GPIO  [9]			   */
#define GPIO_GPIO10	GPIO_GPIO (10)	/* GPIO [10]			   */
#define GPIO_GPIO11	GPIO_GPIO (11)	/* GPIO [11]			   */
#define GPIO_GPIO12	GPIO_GPIO (12)	/* GPIO [12]			   */
#define GPIO_GPIO13	GPIO_GPIO (13)	/* GPIO [13]			   */
#define GPIO_GPIO14	GPIO_GPIO (14)	/* GPIO [14]			   */
#define GPIO_GPIO15	GPIO_GPIO (15)	/* GPIO [15]			   */
#define GPIO_GPIO16	GPIO_GPIO (16)	/* GPIO [16]			   */
#define GPIO_GPIO17	GPIO_GPIO (17)	/* GPIO [17]			   */
#define GPIO_GPIO18	GPIO_GPIO (18)	/* GPIO [18]			   */
#define GPIO_GPIO19	GPIO_GPIO (19)	/* GPIO [19]			   */
#define GPIO_GPIO20	GPIO_GPIO (20)	/* GPIO [20]			   */
#define GPIO_GPIO21	GPIO_GPIO (21)	/* GPIO [21]			   */
#define GPIO_GPIO22	GPIO_GPIO (22)	/* GPIO [22]			   */
#define GPIO_GPIO23	GPIO_GPIO (23)	/* GPIO [23]			   */
#define GPIO_GPIO24	GPIO_GPIO (24)	/* GPIO [24]			   */
#define GPIO_GPIO25	GPIO_GPIO (25)	/* GPIO [25]			   */
#define GPIO_GPIO26	GPIO_GPIO (26)	/* GPIO [26]			   */
#define GPIO_GPIO27	GPIO_GPIO (27)	/* GPIO [27]			   */

#define GPIO_LDD(Nb)			/* LCD Data [8..15] (O)		   */ \
			GPIO_GPIO ((Nb) - 6)
#define GPIO_LDD8	GPIO_LDD (8)	/* LCD Data  [8] (O)		   */
#define GPIO_LDD9	GPIO_LDD (9)	/* LCD Data  [9] (O)		   */
#define GPIO_LDD10	GPIO_LDD (10)	/* LCD Data [10] (O)		   */
#define GPIO_LDD11	GPIO_LDD (11)	/* LCD Data [11] (O)		   */
#define GPIO_LDD12	GPIO_LDD (12)	/* LCD Data [12] (O)		   */
#define GPIO_LDD13	GPIO_LDD (13)	/* LCD Data [13] (O)		   */
#define GPIO_LDD14	GPIO_LDD (14)	/* LCD Data [14] (O)		   */
#define GPIO_LDD15	GPIO_LDD (15)	/* LCD Data [15] (O)		   */
					/* ser. port 4:			   */
#define GPIO_SSP_TXD	GPIO_GPIO (10)	/*  SSP Transmit Data (O)	   */
#define GPIO_SSP_RXD	GPIO_GPIO (11)	/*  SSP Receive Data (I)	   */
#define GPIO_SSP_SCLK	GPIO_GPIO (12)	/*  SSP Sample CLocK (O)	   */
#define GPIO_SSP_SFRM	GPIO_GPIO (13)	/*  SSP Sample FRaMe (O)	   */
					/* ser. port 1:			   */
#define GPIO_UART_TXD	GPIO_GPIO (14)	/*  UART Transmit Data (O)	   */
#define GPIO_UART_RXD	GPIO_GPIO (15)	/*  UART Receive Data (I)	   */
#define GPIO_SDLC_SCLK	GPIO_GPIO (16)	/*  SDLC Sample CLocK (I/O)	   */
#define GPIO_SDLC_AAF	GPIO_GPIO (17)	/*  SDLC Abort After Frame (O)	   */
#define GPIO_UART_SCLK1	GPIO_GPIO (18)	/*  UART Sample CLocK 1 (I)	   */
					/* ser. port 4:			   */
#define GPIO_SSP_CLK	GPIO_GPIO (19)	/*  SSP external CLocK (I)	   */
					/* ser. port 3:			   */
#define GPIO_UART_SCLK3	GPIO_GPIO (20)	/*  UART Sample CLocK 3 (I)	   */
					/* ser. port 4:			   */
#define GPIO_MCP_CLK	GPIO_GPIO (21)	/*  MCP CLocK (I)		   */
					/* test controller:		   */
#define GPIO_TIC_ACK	GPIO_GPIO (21)	/*  TIC ACKnowledge (O)		   */
#define GPIO_MBGNT	GPIO_GPIO (21)	/*  Memory Bus GraNT (O)	   */
#define GPIO_TREQA	GPIO_GPIO (22)	/*  TIC REQuest A (I)		   */
#define GPIO_MBREQ	GPIO_GPIO (22)	/*  Memory Bus REQuest (I)	   */
#define GPIO_TREQB	GPIO_GPIO (23)	/*  TIC REQuest B (I)		   */
#define GPIO_1Hz	GPIO_GPIO (25)	/* 1 Hz clock (O)		   */
#define GPIO_RCLK	GPIO_GPIO (26)	/* internal (R) CLocK (O, fcpu/2)  */
#define GPIO_32_768kHz	GPIO_GPIO (27)	/* 32.768 kHz clock (O, RTC)	   */

#define GPDR_In		0		/* Input			   */
#define GPDR_Out	1		/* Output			   */


/*
 * Interrupt Controller (IC) control registers
 *
 * Registers
 *    ICIP		Interrupt Controller (IC) Interrupt ReQuest (IRQ)
 *			Pending register (read).
 *    ICMR		Interrupt Controller (IC) Mask Register (read/write).
 *    ICLR		Interrupt Controller (IC) Level Register (read/write).
 *    ICCR		Interrupt Controller (IC) Control Register
 *			(read/write).
 *			[The ICCR register is only implemented in versions 2.0
 *			(rev. = 8) and higher of the StrongARM SA-1100.]
 *    ICFP		Interrupt Controller (IC) Fast Interrupt reQuest
 *			(FIQ) Pending register (read).
 *    ICPR		Interrupt Controller (IC) Pending Register (read).
 *			[The ICPR register is active low (inverted) in
 *			versions 1.0 (rev. = 1) and 1.1 (rev. = 2) of the
 *			StrongARM SA-1100, it is active high (non-inverted) in
 *			versions 2.0 (rev. = 8) and higher.]
 */

#define _ICIP		0x90050000	/* IC IRQ Pending reg.		   */
#define _ICMR		0x90050004	/* IC Mask Reg.			   */
#define _ICLR		0x90050008	/* IC Level Reg.		   */
#define _ICCR		0x9005000C	/* IC Control Reg.		   */
#define _ICFP		0x90050010	/* IC FIQ Pending reg.		   */
#define _ICPR		0x90050020	/* IC Pending Reg.		   */

#if LANGUAGE == C
#define ICIP				/* IC IRQ Pending reg.		   */ \
			(*((volatile Word *) io_p2v (_ICIP)))
#define ICMR				/* IC Mask Reg.			   */ \
			(*((volatile Word *) io_p2v (_ICMR)))
#define ICLR				/* IC Level Reg.		   */ \
			(*((volatile Word *) io_p2v (_ICLR)))
#define ICCR				/* IC Control Reg.		   */ \
			(*((volatile Word *) io_p2v (_ICCR)))
#define ICFP				/* IC FIQ Pending reg.		   */ \
			(*((volatile Word *) io_p2v (_ICFP)))
#define ICPR				/* IC Pending Reg.		   */ \
			(*((volatile Word *) io_p2v (_ICPR)))
#endif /* LANGUAGE == C */

#define IC_GPIO(Nb)			/* GPIO [0..10]			   */ \
			(0x00000001 << (Nb))
#define IC_GPIO0	IC_GPIO (0)	/* GPIO  [0]			   */
#define IC_GPIO1	IC_GPIO (1)	/* GPIO  [1]			   */
#define IC_GPIO2	IC_GPIO (2)	/* GPIO  [2]			   */
#define IC_GPIO3	IC_GPIO (3)	/* GPIO  [3]			   */
#define IC_GPIO4	IC_GPIO (4)	/* GPIO  [4]			   */
#define IC_GPIO5	IC_GPIO (5)	/* GPIO  [5]			   */
#define IC_GPIO6	IC_GPIO (6)	/* GPIO  [6]			   */
#define IC_GPIO7	IC_GPIO (7)	/* GPIO  [7]			   */
#define IC_GPIO8	IC_GPIO (8)	/* GPIO  [8]			   */
#define IC_GPIO9	IC_GPIO (9)	/* GPIO  [9]			   */
#define IC_GPIO10	IC_GPIO (10)	/* GPIO [10]			   */
#define IC_GPIO11_27	0x00000800	/* GPIO [11:27] (ORed)		   */
#define IC_LCD		0x00001000	/* LCD controller		   */
#define IC_Ser0UDC	0x00002000	/* Ser. port 0 UDC		   */
#define IC_Ser1SDLC	0x00004000	/* Ser. port 1 SDLC		   */
#define IC_Ser1UART	0x00008000	/* Ser. port 1 UART		   */
#define IC_Ser2ICP	0x00010000	/* Ser. port 2 ICP		   */
#define IC_Ser3UART	0x00020000	/* Ser. port 3 UART		   */
#define IC_Ser4MCP	0x00040000	/* Ser. port 4 MCP		   */
#define IC_Ser4SSP	0x00080000	/* Ser. port 4 SSP		   */
#define IC_DMA(Nb)			/* DMA controller channel [0..5]   */ \
			(0x00100000 << (Nb))
#define IC_DMA0		IC_DMA (0)	/* DMA controller channel 0	   */
#define IC_DMA1		IC_DMA (1)	/* DMA controller channel 1	   */
#define IC_DMA2		IC_DMA (2)	/* DMA controller channel 2	   */
#define IC_DMA3		IC_DMA (3)	/* DMA controller channel 3	   */
#define IC_DMA4		IC_DMA (4)	/* DMA controller channel 4	   */
#define IC_DMA5		IC_DMA (5)	/* DMA controller channel 5	   */
#define IC_OST(Nb)			/* OS Timer match [0..3]	   */ \
			(0x04000000 << (Nb))
#define IC_OST0		IC_OST (0)	/* OS Timer match 0		   */
#define IC_OST1		IC_OST (1)	/* OS Timer match 1		   */
#define IC_OST2		IC_OST (2)	/* OS Timer match 2		   */
#define IC_OST3		IC_OST (3)	/* OS Timer match 3		   */
#define IC_RTC1Hz	0x40000000	/* RTC 1 Hz clock		   */
#define IC_RTCAlrm	0x80000000	/* RTC Alarm			   */

#define ICLR_IRQ	0		/* Interrupt ReQuest		   */
#define ICLR_FIQ	1		/* Fast Interrupt reQuest	   */

#define ICCR_DIM	0x00000001	/* Disable Idle-mode interrupt	   */
					/* Mask				   */
#define ICCR_IdleAllInt	(ICCR_DIM*0)	/*  Idle-mode All Interrupt enable */
					/*  (ICMR ignored)		   */
#define ICCR_IdleMskInt	(ICCR_DIM*1)	/*  Idle-mode non-Masked Interrupt */
					/*  enable (ICMR used)		   */


/*
 * Peripheral Pin Controller (PPC) control registers
 *
 * Registers
 *    PPDR		Peripheral Pin Controller (PPC) Pin Direction
 *			Register (read/write).
 *    PPSR		Peripheral Pin Controller (PPC) Pin State Register
 *			(read/write).
 *    PPAR		Peripheral Pin Controller (PPC) Pin Assignment
 *			Register (read/write).
 *    PSDR		Peripheral Pin Controller (PPC) Sleep-mode pin
 *			Direction Register (read/write).
 *    PPFR		Peripheral Pin Controller (PPC) Pin Flag Register
 *			(read).
 */

#define _PPDR		0x90060000	/* PPC Pin Direction Reg.	   */
#define _PPSR		0x90060004	/* PPC Pin State Reg.		   */
#define _PPAR		0x90060008	/* PPC Pin Assignment Reg.	   */
#define _PSDR		0x9006000C	/* PPC Sleep-mode pin Direction    */
					/* Reg.				   */
#define _PPFR		0x90060010	/* PPC Pin Flag Reg.		   */

#if LANGUAGE == C
#define PPDR				/* PPC Pin Direction Reg.	   */ \
			(*((volatile Word *) io_p2v (_PPDR)))
#define PPSR				/* PPC Pin State Reg.		   */ \
			(*((volatile Word *) io_p2v (_PPSR)))
#define PPAR				/* PPC Pin Assignment Reg.	   */ \
			(*((volatile Word *) io_p2v (_PPAR)))
#define PSDR				/* PPC Sleep-mode pin Direction    */ \
					/* Reg.				   */ \
			(*((volatile Word *) io_p2v (_PSDR)))
#define PPFR				/* PPC Pin Flag Reg.		   */ \
			(*((volatile Word *) io_p2v (_PPFR)))
#endif /* LANGUAGE == C */

#define PPC_LDD(Nb)			/* LCD Data [0..7]		   */ \
			(0x00000001 << (Nb))
#define PPC_LDD0	PPC_LDD (0)	/* LCD Data [0]			   */
#define PPC_LDD1	PPC_LDD (1)	/* LCD Data [1]			   */
#define PPC_LDD2	PPC_LDD (2)	/* LCD Data [2]			   */
#define PPC_LDD3	PPC_LDD (3)	/* LCD Data [3]			   */
#define PPC_LDD4	PPC_LDD (4)	/* LCD Data [4]			   */
#define PPC_LDD5	PPC_LDD (5)	/* LCD Data [5]			   */
#define PPC_LDD6	PPC_LDD (6)	/* LCD Data [6]			   */
#define PPC_LDD7	PPC_LDD (7)	/* LCD Data [7]			   */
#define PPC_L_PCLK	0x00000100	/* LCD Pixel CLocK		   */
#define PPC_L_LCLK	0x00000200	/* LCD Line CLocK		   */
#define PPC_L_FCLK	0x00000400	/* LCD Frame CLocK		   */
#define PPC_L_BIAS	0x00000800	/* LCD AC BIAS			   */
					/* ser. port 1:			   */
#define PPC_TXD1	0x00001000	/*  SDLC/UART Transmit Data 1	   */
#define PPC_RXD1	0x00002000	/*  SDLC/UART Receive Data 1	   */
					/* ser. port 2:			   */
#define PPC_TXD2	0x00004000	/*  IPC Transmit Data 2		   */
#define PPC_RXD2	0x00008000	/*  IPC Receive Data 2		   */
					/* ser. port 3:			   */
#define PPC_TXD3	0x00010000	/*  UART Transmit Data 3	   */
#define PPC_RXD3	0x00020000	/*  UART Receive Data 3		   */
					/* ser. port 4:			   */
#define PPC_TXD4	0x00040000	/*  MCP/SSP Transmit Data 4	   */
#define PPC_RXD4	0x00080000	/*  MCP/SSP Receive Data 4	   */
#define PPC_SCLK	0x00100000	/*  MCP/SSP Sample CLocK	   */
#define PPC_SFRM	0x00200000	/*  MCP/SSP Sample FRaMe	   */

#define PPDR_In		0		/* Input			   */
#define PPDR_Out	1		/* Output			   */

					/* ser. port 1:			   */
#define PPAR_UPR	0x00001000	/*  UART Pin Reassignment	   */
#define PPAR_UARTTR	(PPAR_UPR*0)	/*   UART on TXD_1 & RXD_1	   */
#define PPAR_UARTGPIO	(PPAR_UPR*1)	/*   UART on GPIO [14:15]	   */
					/* ser. port 4:			   */
#define PPAR_SPR	0x00040000	/*  SSP Pin Reassignment	   */
#define PPAR_SSPTRSS	(PPAR_SPR*0)	/*   SSP on TXD_C, RXD_C, SCLK_C,  */
					/*   & SFRM_C			   */
#define PPAR_SSPGPIO	(PPAR_SPR*1)	/*   SSP on GPIO [10:13]	   */

#define PSDR_OutL	0		/* Output Low in sleep mode	   */
#define PSDR_Flt	1		/* Floating (input) in sleep mode  */

#define PPFR_LCD	0x00000001	/* LCD controller		   */
#define PPFR_SP1TX	0x00001000	/* Ser. Port 1 SDLC/UART Transmit  */
#define PPFR_SP1RX	0x00002000	/* Ser. Port 1 SDLC/UART Receive   */
#define PPFR_SP2TX	0x00004000	/* Ser. Port 2 ICP Transmit	   */
#define PPFR_SP2RX	0x00008000	/* Ser. Port 2 ICP Receive	   */
#define PPFR_SP3TX	0x00010000	/* Ser. Port 3 UART Transmit	   */
#define PPFR_SP3RX	0x00020000	/* Ser. Port 3 UART Receive	   */
#define PPFR_SP4	0x00040000	/* Ser. Port 4 MCP/SSP		   */
#define PPFR_PerEn	0		/* Peripheral Enabled		   */
#define PPFR_PPCEn	1		/* PPC Enabled			   */


/*
 * Dynamic Random-Access Memory (DRAM) control registers
 *
 * Registers
 *    MDCNFG		Memory system: Dynamic Random-Access Memory (DRAM)
 *			CoNFiGuration register (read/write).
 *    MDCAS0		Memory system: Dynamic Random-Access Memory (DRAM)
 *			Column Address Strobe (CAS) shift register 0
 *			(read/write).
 *    MDCAS1		Memory system: Dynamic Random-Access Memory (DRAM)
 *			Column Address Strobe (CAS) shift register 1
 *			(read/write).
 *    MDCAS2		Memory system: Dynamic Random-Access Memory (DRAM)
 *			Column Address Strobe (CAS) shift register 2
 *			(read/write).
 *
 * Clocks
 *    fcpu, Tcpu	Frequency, period of the CPU core clock (CCLK).
 *    fmem, Tmem	Frequency, period of the memory clock (fmem = fcpu/2).
 *    fcas, Tcas	Frequency, period of the DRAM CAS shift registers.
 */

					/* Memory system:		   */
#define _MDCNFG		0xA0000000	/*  DRAM CoNFiGuration reg.	   */
#define _MDCAS(Nb)			/*  DRAM CAS shift reg. [0..3]	   */ \
			(0xA0000004 + (Nb)*4)
#define _MDCAS0		_MDCAS (0)	/*  DRAM CAS shift reg. 0	   */
#define _MDCAS1		_MDCAS (1)	/*  DRAM CAS shift reg. 1	   */
#define _MDCAS2		_MDCAS (2)	/*  DRAM CAS shift reg. 2	   */

#if LANGUAGE == C
					/* Memory system:		   */
#define MDCNFG				/*  DRAM CoNFiGuration reg.	   */ \
			(*((volatile Word *) io_p2v (_MDCNFG)))
#define MDCAS				/*  DRAM CAS shift reg. [0..3]	   */ \
			((volatile Word *) io_p2v (_MDCAS (0)))
#define MDCAS0		(MDCAS [0])	/*  DRAM CAS shift reg. 0	   */
#define MDCAS1		(MDCAS [1])	/*  DRAM CAS shift reg. 1	   */
#define MDCAS2		(MDCAS [2])	/*  DRAM CAS shift reg. 2	   */

#elif LANGUAGE == Assembly

#define MDCNFG		(io_p2v(_MDCNFG))

#endif /* LANGUAGE == C */

/* SA1100 MDCNFG values */
#define MDCNFG_DE(Nb)			/* DRAM Enable bank [0..3]	   */ \
			(0x00000001 << (Nb))
#define MDCNFG_DE0	MDCNFG_DE (0)	/* DRAM Enable bank 0		   */
#define MDCNFG_DE1	MDCNFG_DE (1)	/* DRAM Enable bank 1		   */
#define MDCNFG_DE2	MDCNFG_DE (2)	/* DRAM Enable bank 2		   */
#define MDCNFG_DE3	MDCNFG_DE (3)	/* DRAM Enable bank 3		   */
#define MDCNFG_DRAC	Fld (2, 4)	/* DRAM Row Address Count - 9	   */
#define MDCNFG_RowAdd(Add)		/*  Row Address count [9..12]	   */ \
			(((Add) - 9) << FShft (MDCNFG_DRAC))
#define MDCNFG_CDB2	0x00000040	/* shift reg. Clock Divide By 2    */
					/* (fcas = fcpu/2)		   */
#define MDCNFG_TRP	Fld (4, 7)	/* Time RAS Pre-charge - 1 [Tmem]  */
#define MDCNFG_PrChrg(Tcpu)		/*  Pre-Charge time [2..32 Tcpu]   */ \
			(((Tcpu) - 2)/2 << FShft (MDCNFG_TRP))
#define MDCNFG_CeilPrChrg(Tcpu)		/*  Ceil. of PrChrg [2..32 Tcpu]   */ \
			(((Tcpu) - 1)/2 << FShft (MDCNFG_TRP))
#define MDCNFG_TRASR	Fld (4, 11)	/* Time RAS Refresh - 1 [Tmem]	   */
#define MDCNFG_Ref(Tcpu)		/*  Refresh time [2..32 Tcpu]	   */ \
			(((Tcpu) - 2)/2 << FShft (MDCNFG_TRASR))
#define MDCNFG_CeilRef(Tcpu)		/*  Ceil. of Ref [2..32 Tcpu]	   */ \
			(((Tcpu) - 1)/2 << FShft (MDCNFG_TRASR))
#define MDCNFG_TDL	Fld (2, 15)	/* Time Data Latch [Tcpu]	   */
#define MDCNFG_DataLtch(Tcpu)		/*  Data Latch delay [0..3 Tcpu]   */ \
			((Tcpu) << FShft (MDCNFG_TDL))
#define MDCNFG_DRI	Fld (15, 17)	/* min. DRAM Refresh Interval/4    */
					/* [Tmem]			   */
#define MDCNFG_RefInt(Tcpu)		/*  min. Refresh Interval	   */ \
					/*  [0..262136 Tcpu]		   */ \
			((Tcpu)/8 << FShft (MDCNFG_DRI))

/* SA1110 MDCNFG values */
#define MDCNFG_SA1110_DE0	0x00000001	/* DRAM Enable bank 0	     */
#define MDCNFG_SA1110_DE1	0x00000002	/* DRAM Enable bank 1	     */
#define MDCNFG_SA1110_DTIM0	0x00000004	/* DRAM timing type 0/1      */
#define MDCNFG_SA1110_DWID0	0x00000008	/* DRAM bus width 0/1	     */
#define MDCNFG_SA1110_DRAC0	Fld(3, 4)	/* DRAM row addr bit count   */
						/* bank 0/1		     */
#define MDCNFG_SA1110_CDB20	0x00000080	/* Mem Clock divide by 2 0/1 */
#define MDCNFG_SA1110_TRP0	Fld(3, 8)	/* RAS precharge 0/1	     */
#define MDCNFG_SA1110_TDL0	Fld(2, 12)	/* Data input latch after CAS*/
						/* deassertion 0/1	     */
#define MDCNFG_SA1110_TWR0	Fld(2, 14)	/* SDRAM write recovery 0/1  */
#define MDCNFG_SA1110_DE2	0x00010000	/* DRAM Enable bank 0	     */
#define MDCNFG_SA1110_DE3	0x00020000	/* DRAM Enable bank 1	     */
#define MDCNFG_SA1110_DTIM2	0x00040000	/* DRAM timing type 0/1      */
#define MDCNFG_SA1110_DWID2	0x00080000	/* DRAM bus width 0/1	     */
#define MDCNFG_SA1110_DRAC2	Fld(3, 20)	/* DRAM row addr bit count   */
						/* bank 0/1		     */
#define MDCNFG_SA1110_CDB22	0x00800000	/* Mem Clock divide by 2 0/1 */
#define MDCNFG_SA1110_TRP2	Fld(3, 24)	/* RAS precharge 0/1	     */
#define MDCNFG_SA1110_TDL2	Fld(2, 28)	/* Data input latch after CAS*/
						/* deassertion 0/1	     */
#define MDCNFG_SA1110_TWR2	Fld(2, 30)	/* SDRAM write recovery 0/1  */


/*
 * Static memory control registers
 *
 * Registers
 *    MSC0		Memory system: Static memory Control register 0
 *			(read/write).
 *    MSC1		Memory system: Static memory Control register 1
 *			(read/write).
 *
 * Clocks
 *    fcpu, Tcpu	Frequency, period of the CPU core clock (CCLK).
 *    fmem, Tmem	Frequency, period of the memory clock (fmem = fcpu/2).
 */

					/* Memory system:		   */
#define _MSC(Nb)			/*  Static memory Control reg.	   */ \
					/*  [0..1]			   */ \
			(0xA0000010 + (Nb)*4)
#define _MSC0		_MSC (0)	/*  Static memory Control reg. 0   */
#define _MSC1		_MSC (1)	/*  Static memory Control reg. 1   */
#define _MSC2		0xA000002C	/*  Static memory Control reg. 2, not contiguous   */

#if LANGUAGE == C
					/* Memory system:		   */
#define MSC				/*  Static memory Control reg.	   */ \
					/*  [0..1]			   */ \
			((volatile Word *) io_p2v (_MSC (0)))
#define MSC0		(MSC [0])	/*  Static memory Control reg. 0   */
#define MSC1		(MSC [1])	/*  Static memory Control reg. 1   */
#define MSC2		(*(volatile Word *) io_p2v (_MSC2))	/*  Static memory Control reg. 2   */

#elif LANGUAGE == Assembly

#define MSC0		io_p2v(0xa0000010)
#define MSC1		io_p2v(0xa0000014)
#define MSC2		io_p2v(0xa000002c)

#endif /* LANGUAGE == C */

#define MSC_Bnk(Nb)			/* static memory Bank [0..3]	   */ \
			Fld (16, ((Nb) Modulo 2)*16)
#define MSC0_Bnk0	MSC_Bnk (0)	/* static memory Bank 0		   */
#define MSC0_Bnk1	MSC_Bnk (1)	/* static memory Bank 1		   */
#define MSC1_Bnk2	MSC_Bnk (2)	/* static memory Bank 2		   */
#define MSC1_Bnk3	MSC_Bnk (3)	/* static memory Bank 3		   */

#define MSC_RT		Fld (2, 0)	/* ROM/static memory Type	   */
#define MSC_NonBrst			/*  Non-Burst static memory	   */ \
			(0 << FShft (MSC_RT))
#define MSC_SRAM			/*  32-bit byte-writable SRAM	   */ \
			(1 << FShft (MSC_RT))
#define MSC_Brst4			/*  Burst-of-4 static memory	   */ \
			(2 << FShft (MSC_RT))
#define MSC_Brst8			/*  Burst-of-8 static memory	   */ \
			(3 << FShft (MSC_RT))
#define MSC_RBW		0x0004		/* ROM/static memory Bus Width	   */
#define MSC_32BitStMem	(MSC_RBW*0)	/*  32-Bit Static Memory	   */
#define MSC_16BitStMem	(MSC_RBW*1)	/*  16-Bit Static Memory	   */
#define MSC_RDF		Fld (5, 3)	/* ROM/static memory read Delay    */
					/* First access - 1(.5) [Tmem]	   */
#define MSC_1stRdAcc(Tcpu)		/*  1st Read Access time (burst    */ \
					/*  static memory) [3..65 Tcpu]    */ \
			((((Tcpu) - 3)/2) << FShft (MSC_RDF))
#define MSC_Ceil1stRdAcc(Tcpu)		/*  Ceil. of 1stRdAcc [3..65 Tcpu] */ \
			((((Tcpu) - 2)/2) << FShft (MSC_RDF))
#define MSC_RdAcc(Tcpu)			/*  Read Access time (non-burst    */ \
					/*  static memory) [2..64 Tcpu]    */ \
			((((Tcpu) - 2)/2) << FShft (MSC_RDF))
#define MSC_CeilRdAcc(Tcpu)		/*  Ceil. of RdAcc [2..64 Tcpu]    */ \
			((((Tcpu) - 1)/2) << FShft (MSC_RDF))
#define MSC_RDN		Fld (5, 8)	/* ROM/static memory read Delay    */
					/* Next access - 1 [Tmem]	   */
#define MSC_NxtRdAcc(Tcpu)		/*  Next Read Access time (burst   */ \
					/*  static memory) [2..64 Tcpu]    */ \
			((((Tcpu) - 2)/2) << FShft (MSC_RDN))
#define MSC_CeilNxtRdAcc(Tcpu)		/*  Ceil. of NxtRdAcc [2..64 Tcpu] */ \
			((((Tcpu) - 1)/2) << FShft (MSC_RDN))
#define MSC_WrAcc(Tcpu)			/*  Write Access time (non-burst   */ \
					/*  static memory) [2..64 Tcpu]    */ \
			((((Tcpu) - 2)/2) << FShft (MSC_RDN))
#define MSC_CeilWrAcc(Tcpu)		/*  Ceil. of WrAcc [2..64 Tcpu]    */ \
			((((Tcpu) - 1)/2) << FShft (MSC_RDN))
#define MSC_RRR		Fld (3, 13)	/* ROM/static memory RecoveRy	   */
					/* time/2 [Tmem]		   */
#define MSC_Rec(Tcpu)			/*  Recovery time [0..28 Tcpu]	   */ \
			(((Tcpu)/4) << FShft (MSC_RRR))
#define MSC_CeilRec(Tcpu)		/*  Ceil. of Rec [0..28 Tcpu]	   */ \
			((((Tcpu) + 3)/4) << FShft (MSC_RRR))


/*
 * Personal Computer Memory Card International Association (PCMCIA) control
 * register
 *
 * Register
 *    MECR		Memory system: Expansion memory bus (PCMCIA)
 *			Configuration Register (read/write).
 *
 * Clocks
 *    fcpu, Tcpu	Frequency, period of the CPU core clock (CCLK).
 *    fmem, Tmem	Frequency, period of the memory clock (fmem = fcpu/2).
 *    fbclk, Tbclk	Frequency, period of the PCMCIA clock (BCLK).
 */

					/* Memory system:		   */
#define _MECR		0xA0000018	/*  Expansion memory bus (PCMCIA)  */
					/*  Configuration Reg.		   */

#if LANGUAGE == C
					/* Memory system:		   */
#define MECR				/*  Expansion memory bus (PCMCIA)  */ \
					/*  Configuration Reg.		   */ \
			(*((volatile Word *) io_p2v (_MECR)))
#endif /* LANGUAGE == C */

#define MECR_PCMCIA(Nb)			/* PCMCIA [0..1]		   */ \
			Fld (15, (Nb)*16)
#define MECR_PCMCIA0	MECR_PCMCIA (0)	/* PCMCIA 0			   */
#define MECR_PCMCIA1	MECR_PCMCIA (1)	/* PCMCIA 1			   */

#define MECR_BSIO	Fld (5, 0)	/* BCLK Select I/O - 1 [Tmem]	   */
#define MECR_IOClk(Tcpu)		/*  I/O Clock [2..64 Tcpu]	   */ \
			((((Tcpu) - 2)/2) << FShft (MECR_BSIO))
#define MECR_CeilIOClk(Tcpu)		/*  Ceil. of IOClk [2..64 Tcpu]    */ \
			((((Tcpu) - 1)/2) << FShft (MECR_BSIO))
#define MECR_BSA	Fld (5, 5)	/* BCLK Select Attribute - 1	   */
					/* [Tmem]			   */
#define MECR_AttrClk(Tcpu)		/*  Attribute Clock [2..64 Tcpu]   */ \
			((((Tcpu) - 2)/2) << FShft (MECR_BSA))
#define MECR_CeilAttrClk(Tcpu)		/*  Ceil. of AttrClk [2..64 Tcpu]  */ \
			((((Tcpu) - 1)/2) << FShft (MECR_BSA))
#define MECR_BSM	Fld (5, 10)	/* BCLK Select Memory - 1 [Tmem]   */
#define MECR_MemClk(Tcpu)		/*  Memory Clock [2..64 Tcpu]	   */ \
			((((Tcpu) - 2)/2) << FShft (MECR_BSM))
#define MECR_CeilMemClk(Tcpu)		/*  Ceil. of MemClk [2..64 Tcpu]   */ \
			((((Tcpu) - 1)/2) << FShft (MECR_BSM))

/*
 * On SA1110 only
 */

#define _MDREFR		0xA000001C

#if LANGUAGE == C
					/* Memory system:		   */
#define MDREFR \
			(*((volatile Word *) io_p2v (_MDREFR)))

#elif LANGUAGE == Assembly

#define MDREFR		(io_p2v(_MDREFR))

#endif /* LANGUAGE == C */

#define MDREFR_TRASR		Fld (4, 0)
#define MDREFR_DRI		Fld (12, 4)
#define MDREFR_E0PIN		(1 << 16)
#define MDREFR_K0RUN		(1 << 17)
#define MDREFR_K0DB2		(1 << 18)
#define MDREFR_E1PIN		(1 << 20)
#define MDREFR_K1RUN		(1 << 21)
#define MDREFR_K1DB2		(1 << 22)
#define MDREFR_K2RUN		(1 << 25)
#define MDREFR_K2DB2		(1 << 26)
#define MDREFR_EAPD		(1 << 28)
#define MDREFR_KAPD		(1 << 29)
#define MDREFR_SLFRSH		(1 << 31)


/*
 * Direct Memory Access (DMA) control registers
 *
 * Registers
 *    DDAR0		Direct Memory Access (DMA) Device Address Register
 *			channel 0 (read/write).
 *    DCSR0		Direct Memory Access (DMA) Control and Status
 *			Register channel 0 (read/write).
 *    DBSA0		Direct Memory Access (DMA) Buffer Start address
 *			register A channel 0 (read/write).
 *    DBTA0		Direct Memory Access (DMA) Buffer Transfer count
 *			register A channel 0 (read/write).
 *    DBSB0		Direct Memory Access (DMA) Buffer Start address
 *			register B channel 0 (read/write).
 *    DBTB0		Direct Memory Access (DMA) Buffer Transfer count
 *			register B channel 0 (read/write).
 *
 *    DDAR1		Direct Memory Access (DMA) Device Address Register
 *			channel 1 (read/write).
 *    DCSR1		Direct Memory Access (DMA) Control and Status
 *			Register channel 1 (read/write).
 *    DBSA1		Direct Memory Access (DMA) Buffer Start address
 *			register A channel 1 (read/write).
 *    DBTA1		Direct Memory Access (DMA) Buffer Transfer count
 *			register A channel 1 (read/write).
 *    DBSB1		Direct Memory Access (DMA) Buffer Start address
 *			register B channel 1 (read/write).
 *    DBTB1		Direct Memory Access (DMA) Buffer Transfer count
 *			register B channel 1 (read/write).
 *
 *    DDAR2		Direct Memory Access (DMA) Device Address Register
 *			channel 2 (read/write).
 *    DCSR2		Direct Memory Access (DMA) Control and Status
 *			Register channel 2 (read/write).
 *    DBSA2		Direct Memory Access (DMA) Buffer Start address
 *			register A channel 2 (read/write).
 *    DBTA2		Direct Memory Access (DMA) Buffer Transfer count
 *			register A channel 2 (read/write).
 *    DBSB2		Direct Memory Access (DMA) Buffer Start address
 *			register B channel 2 (read/write).
 *    DBTB2		Direct Memory Access (DMA) Buffer Transfer count
 *			register B channel 2 (read/write).
 *
 *    DDAR3		Direct Memory Access (DMA) Device Address Register
 *			channel 3 (read/write).
 *    DCSR3		Direct Memory Access (DMA) Control and Status
 *			Register channel 3 (read/write).
 *    DBSA3		Direct Memory Access (DMA) Buffer Start address
 *			register A channel 3 (read/write).
 *    DBTA3		Direct Memory Access (DMA) Buffer Transfer count
 *			register A channel 3 (read/write).
 *    DBSB3		Direct Memory Access (DMA) Buffer Start address
 *			register B channel 3 (read/write).
 *    DBTB3		Direct Memory Access (DMA) Buffer Transfer count
 *			register B channel 3 (read/write).
 *
 *    DDAR4		Direct Memory Access (DMA) Device Address Register
 *			channel 4 (read/write).
 *    DCSR4		Direct Memory Access (DMA) Control and Status
 *			Register channel 4 (read/write).
 *    DBSA4		Direct Memory Access (DMA) Buffer Start address
 *			register A channel 4 (read/write).
 *    DBTA4		Direct Memory Access (DMA) Buffer Transfer count
 *			register A channel 4 (read/write).
 *    DBSB4		Direct Memory Access (DMA) Buffer Start address
 *			register B channel 4 (read/write).
 *    DBTB4		Direct Memory Access (DMA) Buffer Transfer count
 *			register B channel 4 (read/write).
 *
 *    DDAR5		Direct Memory Access (DMA) Device Address Register
 *			channel 5 (read/write).
 *    DCSR5		Direct Memory Access (DMA) Control and Status
 *			Register channel 5 (read/write).
 *    DBSA5		Direct Memory Access (DMA) Buffer Start address
 *			register A channel 5 (read/write).
 *    DBTA5		Direct Memory Access (DMA) Buffer Transfer count
 *			register A channel 5 (read/write).
 *    DBSB5		Direct Memory Access (DMA) Buffer Start address
 *			register B channel 5 (read/write).
 *    DBTB5		Direct Memory Access (DMA) Buffer Transfer count
 *			register B channel 5 (read/write).
 */

#define DMASp		0x00000020	/* DMA control reg. Space [byte]   */

#define _DDAR(Nb)			/* DMA Device Address Reg.	   */ \
					/* channel [0..5]		   */ \
			(0xB0000000 + (Nb)*DMASp)
#define _SetDCSR(Nb)			/* Set DMA Control & Status Reg.   */ \
					/* channel [0..5] (write)	   */ \
			(0xB0000004 + (Nb)*DMASp)
#define _ClrDCSR(Nb)			/* Clear DMA Control & Status Reg. */ \
					/* channel [0..5] (write)	   */ \
			(0xB0000008 + (Nb)*DMASp)
#define _RdDCSR(Nb)			/* Read DMA Control & Status Reg.  */ \
					/* channel [0..5] (read)	   */ \
			(0xB000000C + (Nb)*DMASp)
#define _DBSA(Nb)			/* DMA Buffer Start address reg. A */ \
					/* channel [0..5]		   */ \
			(0xB0000010 + (Nb)*DMASp)
#define _DBTA(Nb)			/* DMA Buffer Transfer count	   */ \
					/* reg. A channel [0..5]	   */ \
			(0xB0000014 + (Nb)*DMASp)
#define _DBSB(Nb)			/* DMA Buffer Start address reg. B */ \
					/* channel [0..5]		   */ \
			(0xB0000018 + (Nb)*DMASp)
#define _DBTB(Nb)			/* DMA Buffer Transfer count	   */ \
					/* reg. B channel [0..5]	   */ \
			(0xB000001C + (Nb)*DMASp)

#define _DDAR0		_DDAR (0)	/* DMA Device Address Reg.	   */
					/* channel 0			   */
#define _SetDCSR0	_SetDCSR (0)	/* Set DMA Control & Status Reg.   */
					/* channel 0 (write)		   */
#define _ClrDCSR0	_ClrDCSR (0)	/* Clear DMA Control & Status Reg. */
					/* channel 0 (write)		   */
#define _RdDCSR0	_RdDCSR (0)	/* Read DMA Control & Status Reg.  */
					/* channel 0 (read)		   */
#define _DBSA0		_DBSA (0)	/* DMA Buffer Start address reg. A */
					/* channel 0			   */
#define _DBTA0		_DBTA (0)	/* DMA Buffer Transfer count	   */
					/* reg. A channel 0		   */
#define _DBSB0		_DBSB (0)	/* DMA Buffer Start address reg. B */
					/* channel 0			   */
#define _DBTB0		_DBTB (0)	/* DMA Buffer Transfer count	   */
					/* reg. B channel 0		   */

#define _DDAR1		_DDAR (1)	/* DMA Device Address Reg.	   */
					/* channel 1			   */
#define _SetDCSR1	_SetDCSR (1)	/* Set DMA Control & Status Reg.   */
					/* channel 1 (write)		   */
#define _ClrDCSR1	_ClrDCSR (1)	/* Clear DMA Control & Status Reg. */
					/* channel 1 (write)		   */
#define _RdDCSR1	_RdDCSR (1)	/* Read DMA Control & Status Reg.  */
					/* channel 1 (read)		   */
#define _DBSA1		_DBSA (1)	/* DMA Buffer Start address reg. A */
					/* channel 1			   */
#define _DBTA1		_DBTA (1)	/* DMA Buffer Transfer count	   */
					/* reg. A channel 1		   */
#define _DBSB1		_DBSB (1)	/* DMA Buffer Start address reg. B */
					/* channel 1			   */
#define _DBTB1		_DBTB (1)	/* DMA Buffer Transfer count	   */
					/* reg. B channel 1		   */

#define _DDAR2		_DDAR (2)	/* DMA Device Address Reg.	   */
					/* channel 2			   */
#define _SetDCSR2	_SetDCSR (2)	/* Set DMA Control & Status Reg.   */
					/* channel 2 (write)		   */
#define _ClrDCSR2	_ClrDCSR (2)	/* Clear DMA Control & Status Reg. */
					/* channel 2 (write)		   */
#define _RdDCSR2	_RdDCSR (2)	/* Read DMA Control & Status Reg.  */
					/* channel 2 (read)		   */
#define _DBSA2		_DBSA (2)	/* DMA Buffer Start address reg. A */
					/* channel 2			   */
#define _DBTA2		_DBTA (2)	/* DMA Buffer Transfer count	   */
					/* reg. A channel 2		   */
#define _DBSB2		_DBSB (2)	/* DMA Buffer Start address reg. B */
					/* channel 2			   */
#define _DBTB2		_DBTB (2)	/* DMA Buffer Transfer count	   */
					/* reg. B channel 2		   */

#define _DDAR3		_DDAR (3)	/* DMA Device Address Reg.	   */
					/* channel 3			   */
#define _SetDCSR3	_SetDCSR (3)	/* Set DMA Control & Status Reg.   */
					/* channel 3 (write)		   */
#define _ClrDCSR3	_ClrDCSR (3)	/* Clear DMA Control & Status Reg. */
					/* channel 3 (write)		   */
#define _RdDCSR3	_RdDCSR (3)	/* Read DMA Control & Status Reg.  */
					/* channel 3 (read)		   */
#define _DBSA3		_DBSA (3)	/* DMA Buffer Start address reg. A */
					/* channel 3			   */
#define _DBTA3		_DBTA (3)	/* DMA Buffer Transfer count	   */
					/* reg. A channel 3		   */
#define _DBSB3		_DBSB (3)	/* DMA Buffer Start address reg. B */
					/* channel 3			   */
#define _DBTB3		_DBTB (3)	/* DMA Buffer Transfer count	   */
					/* reg. B channel 3		   */

#define _DDAR4		_DDAR (4)	/* DMA Device Address Reg.	   */
					/* channel 4			   */
#define _SetDCSR4	_SetDCSR (4)	/* Set DMA Control & Status Reg.   */
					/* channel 4 (write)		   */
#define _ClrDCSR4	_ClrDCSR (4)	/* Clear DMA Control & Status Reg. */
					/* channel 4 (write)		   */
#define _RdDCSR4	_RdDCSR (4)	/* Read DMA Control & Status Reg.  */
					/* channel 4 (read)		   */
#define _DBSA4		_DBSA (4)	/* DMA Buffer Start address reg. A */
					/* channel 4			   */
#define _DBTA4		_DBTA (4)	/* DMA Buffer Transfer count	   */
					/* reg. A channel 4		   */
#define _DBSB4		_DBSB (4)	/* DMA Buffer Start address reg. B */
					/* channel 4			   */
#define _DBTB4		_DBTB (4)	/* DMA Buffer Transfer count	   */
					/* reg. B channel 4		   */

#define _DDAR5		_DDAR (5)	/* DMA Device Address Reg.	   */
					/* channel 5			   */
#define _SetDCSR5	_SetDCSR (5)	/* Set DMA Control & Status Reg.   */
					/* channel 5 (write)		   */
#define _ClrDCSR5	_ClrDCSR (5)	/* Clear DMA Control & Status Reg. */
					/* channel 5 (write)		   */
#define _RdDCSR5	_RdDCSR (5)	/* Read DMA Control & Status Reg.  */
					/* channel 5 (read)		   */
#define _DBSA5		_DBSA (5)	/* DMA Buffer Start address reg. A */
					/* channel 5			   */
#define _DBTA5		_DBTA (5)	/* DMA Buffer Transfer count	   */
					/* reg. A channel 5		   */
#define _DBSB5		_DBSB (5)	/* DMA Buffer Start address reg. B */
					/* channel 5			   */
#define _DBTB5		_DBTB (5)	/* DMA Buffer Transfer count	   */
					/* reg. B channel 5		   */

#if LANGUAGE == C

#define DDAR0				/* DMA Device Address Reg.	   */ \
					/* channel 0			   */ \
			(*((volatile Word *) io_p2v (_DDAR0)))
#define SetDCSR0			/* Set DMA Control & Status Reg.   */ \
					/* channel 0 (write)		   */ \
			(*((volatile Word *) io_p2v (_SetDCSR0)))
#define ClrDCSR0			/* Clear DMA Control & Status Reg. */ \
					/* channel 0 (write)		   */ \
			(*((volatile Word *) io_p2v (_ClrDCSR0)))
#define RdDCSR0				/* Read DMA Control & Status Reg.  */ \
					/* channel 0 (read)		   */ \
			(*((volatile Word *) io_p2v (_RdDCSR0)))
#define DBSA0				/* DMA Buffer Start address reg. A */ \
					/* channel 0			   */ \
			(*((volatile Address *) io_p2v (_DBSA0)))
#define DBTA0				/* DMA Buffer Transfer count	   */ \
					/* reg. A channel 0		   */ \
			(*((volatile Word *) io_p2v (_DBTA0)))
#define DBSB0				/* DMA Buffer Start address reg. B */ \
					/* channel 0			   */ \
			(*((volatile Address *) io_p2v (_DBSB0)))
#define DBTB0				/* DMA Buffer Transfer count	   */ \
					/* reg. B channel 0		   */ \
			(*((volatile Word *) io_p2v (_DBTB0)))

#define DDAR1				/* DMA Device Address Reg.	   */ \
					/* channel 1			   */ \
			(*((volatile Word *) io_p2v (_DDAR1)))
#define SetDCSR1			/* Set DMA Control & Status Reg.   */ \
					/* channel 1 (write)		   */ \
			(*((volatile Word *) io_p2v (_SetDCSR1)))
#define ClrDCSR1			/* Clear DMA Control & Status Reg. */ \
					/* channel 1 (write)		   */ \
			(*((volatile Word *) io_p2v (_ClrDCSR1)))
#define RdDCSR1				/* Read DMA Control & Status Reg.  */ \
					/* channel 1 (read)		   */ \
			(*((volatile Word *) io_p2v (_RdDCSR1)))
#define DBSA1				/* DMA Buffer Start address reg. A */ \
					/* channel 1			   */ \
			(*((volatile Address *) io_p2v (_DBSA1)))
#define DBTA1				/* DMA Buffer Transfer count	   */ \
					/* reg. A channel 1		   */ \
			(*((volatile Word *) io_p2v (_DBTA1)))
#define DBSB1				/* DMA Buffer Start address reg. B */ \
					/* channel 1			   */ \
			(*((volatile Address *) io_p2v (_DBSB1)))
#define DBTB1				/* DMA Buffer Transfer count	   */ \
					/* reg. B channel 1		   */ \
			(*((volatile Word *) io_p2v (_DBTB1)))

#define DDAR2				/* DMA Device Address Reg.	   */ \
					/* channel 2			   */ \
			(*((volatile Word *) io_p2v (_DDAR2)))
#define SetDCSR2			/* Set DMA Control & Status Reg.   */ \
					/* channel 2 (write)		   */ \
			(*((volatile Word *) io_p2v (_SetDCSR2)))
#define ClrDCSR2			/* Clear DMA Control & Status Reg. */ \
					/* channel 2 (write)		   */ \
			(*((volatile Word *) io_p2v (_ClrDCSR2)))
#define RdDCSR2				/* Read DMA Control & Status Reg.  */ \
					/* channel 2 (read)		   */ \
			(*((volatile Word *) io_p2v (_RdDCSR2)))
#define DBSA2				/* DMA Buffer Start address reg. A */ \
					/* channel 2			   */ \
			(*((volatile Address *) io_p2v (_DBSA2)))
#define DBTA2				/* DMA Buffer Transfer count	   */ \
					/* reg. A channel 2		   */ \
			(*((volatile Word *) io_p2v (_DBTA2)))
#define DBSB2				/* DMA Buffer Start address reg. B */ \
					/* channel 2			   */ \
			(*((volatile Address *) io_p2v (_DBSB2)))
#define DBTB2				/* DMA Buffer Transfer count	   */ \
					/* reg. B channel 2		   */ \
			(*((volatile Word *) io_p2v (_DBTB2)))

#define DDAR3				/* DMA Device Address Reg.	   */ \
					/* channel 3			   */ \
			(*((volatile Word *) io_p2v (_DDAR3)))
#define SetDCSR3			/* Set DMA Control & Status Reg.   */ \
					/* channel 3 (write)		   */ \
			(*((volatile Word *) io_p2v (_SetDCSR3)))
#define ClrDCSR3			/* Clear DMA Control & Status Reg. */ \
					/* channel 3 (write)		   */ \
			(*((volatile Word *) io_p2v (_ClrDCSR3)))
#define RdDCSR3				/* Read DMA Control & Status Reg.  */ \
					/* channel 3 (read)		   */ \
			(*((volatile Word *) io_p2v (_RdDCSR3)))
#define DBSA3				/* DMA Buffer Start address reg. A */ \
					/* channel 3			   */ \
			(*((volatile Address *) io_p2v (_DBSA3)))
#define DBTA3				/* DMA Buffer Transfer count	   */ \
					/* reg. A channel 3		   */ \
			(*((volatile Word *) io_p2v (_DBTA3)))
#define DBSB3				/* DMA Buffer Start address reg. B */ \
					/* channel 3			   */ \
			(*((volatile Address *) io_p2v (_DBSB3)))
#define DBTB3				/* DMA Buffer Transfer count	   */ \
					/* reg. B channel 3		   */ \
			(*((volatile Word *) io_p2v (_DBTB3)))

#define DDAR4				/* DMA Device Address Reg.	   */ \
					/* channel 4			   */ \
			(*((volatile Word *) io_p2v (_DDAR4)))
#define SetDCSR4			/* Set DMA Control & Status Reg.   */ \
					/* channel 4 (write)		   */ \
			(*((volatile Word *) io_p2v (_SetDCSR4)))
#define ClrDCSR4			/* Clear DMA Control & Status Reg. */ \
					/* channel 4 (write)		   */ \
			(*((volatile Word *) io_p2v (_ClrDCSR4)))
#define RdDCSR4				/* Read DMA Control & Status Reg.  */ \
					/* channel 4 (read)		   */ \
			(*((volatile Word *) io_p2v (_RdDCSR4)))
#define DBSA4				/* DMA Buffer Start address reg. A */ \
					/* channel 4			   */ \
			(*((volatile Address *) io_p2v (_DBSA4)))
#define DBTA4				/* DMA Buffer Transfer count	   */ \
					/* reg. A channel 4		   */ \
			(*((volatile Word *) io_p2v (_DBTA4)))
#define DBSB4				/* DMA Buffer Start address reg. B */ \
					/* channel 4			   */ \
			(*((volatile Address *) io_p2v (_DBSB4)))
#define DBTB4				/* DMA Buffer Transfer count	   */ \
					/* reg. B channel 4		   */ \
			(*((volatile Word *) io_p2v (_DBTB4)))

#define DDAR5				/* DMA Device Address Reg.	   */ \
					/* channel 5			   */ \
			(*((volatile Word *) io_p2v (_DDAR5)))
#define SetDCSR5			/* Set DMA Control & Status Reg.   */ \
					/* channel 5 (write)		   */ \
			(*((volatile Word *) io_p2v (_SetDCSR5)))
#define ClrDCSR5			/* Clear DMA Control & Status Reg. */ \
					/* channel 5 (write)		   */ \
			(*((volatile Word *) io_p2v (_ClrDCSR5)))
#define RdDCSR5				/* Read DMA Control & Status Reg.  */ \
					/* channel 5 (read)		   */ \
			(*((volatile Word *) io_p2v (_RdDCSR5)))
#define DBSA5				/* DMA Buffer Start address reg. A */ \
					/* channel 5			   */ \
			(*((volatile Address *) io_p2v (_DBSA5)))
#define DBTA5				/* DMA Buffer Transfer count	   */ \
					/* reg. A channel 5		   */ \
			(*((volatile Word *) io_p2v (_DBTA5)))
#define DBSB5				/* DMA Buffer Start address reg. B */ \
					/* channel 5			   */ \
			(*((volatile Address *) io_p2v (_DBSB5)))
#define DBTB5				/* DMA Buffer Transfer count	   */ \
					/* reg. B channel 5		   */ \
			(*((volatile Word *) io_p2v (_DBTB5)))

#endif /* LANGUAGE == C */

#define DDAR_RW		0x00000001	/* device data Read/Write	   */
#define DDAR_DevWr	(DDAR_RW*0)	/*  Device data Write		   */
					/*  (memory -> device)		   */
#define DDAR_DevRd	(DDAR_RW*1)	/*  Device data Read		   */
					/*  (device -> memory)		   */
#define DDAR_E		0x00000002	/* big/little Endian device	   */
#define DDAR_LtlEnd	(DDAR_E*0)	/*  Little Endian device	   */
#define DDAR_BigEnd	(DDAR_E*1)	/*  Big Endian device		   */
#define DDAR_BS		0x00000004	/* device Burst Size		   */
#define DDAR_Brst4	(DDAR_BS*0)	/*  Burst-of-4 device		   */
#define DDAR_Brst8	(DDAR_BS*1)	/*  Burst-of-8 device		   */
#define DDAR_DW		0x00000008	/* device Data Width		   */
#define DDAR_8BitDev	(DDAR_DW*0)	/*  8-Bit Device		   */
#define DDAR_16BitDev	(DDAR_DW*1)	/*  16-Bit Device		   */
#define DDAR_DS		Fld (4, 4)	/* Device Select		   */
#define DDAR_Ser0UDCTr			/*  Ser. port 0 UDC Transmit	   */ \
			(0x0 << FShft (DDAR_DS))
#define DDAR_Ser0UDCRc			/*  Ser. port 0 UDC Receive	   */ \
			(0x1 << FShft (DDAR_DS))
#define DDAR_Ser1SDLCTr			/*  Ser. port 1 SDLC Transmit	   */ \
			(0x2 << FShft (DDAR_DS))
#define DDAR_Ser1SDLCRc			/*  Ser. port 1 SDLC Receive	   */ \
			(0x3 << FShft (DDAR_DS))
#define DDAR_Ser1UARTTr			/*  Ser. port 1 UART Transmit	   */ \
			(0x4 << FShft (DDAR_DS))
#define DDAR_Ser1UARTRc			/*  Ser. port 1 UART Receive	   */ \
			(0x5 << FShft (DDAR_DS))
#define DDAR_Ser2ICPTr			/*  Ser. port 2 ICP Transmit	   */ \
			(0x6 << FShft (DDAR_DS))
#define DDAR_Ser2ICPRc			/*  Ser. port 2 ICP Receive	   */ \
			(0x7 << FShft (DDAR_DS))
#define DDAR_Ser3UARTTr			/*  Ser. port 3 UART Transmit	   */ \
			(0x8 << FShft (DDAR_DS))
#define DDAR_Ser3UARTRc			/*  Ser. port 3 UART Receive	   */ \
			(0x9 << FShft (DDAR_DS))
#define DDAR_Ser4MCP0Tr			/*  Ser. port 4 MCP 0 Transmit	   */ \
					/*  (audio)			   */ \
			(0xA << FShft (DDAR_DS))
#define DDAR_Ser4MCP0Rc			/*  Ser. port 4 MCP 0 Receive	   */ \
					/*  (audio)			   */ \
			(0xB << FShft (DDAR_DS))
#define DDAR_Ser4MCP1Tr			/*  Ser. port 4 MCP 1 Transmit	   */ \
					/*  (telecom)			   */ \
			(0xC << FShft (DDAR_DS))
#define DDAR_Ser4MCP1Rc			/*  Ser. port 4 MCP 1 Receive	   */ \
					/*  (telecom)			   */ \
			(0xD << FShft (DDAR_DS))
#define DDAR_Ser4SSPTr			/*  Ser. port 4 SSP Transmit	   */ \
			(0xE << FShft (DDAR_DS))
#define DDAR_Ser4SSPRc			/*  Ser. port 4 SSP Receive	   */ \
			(0xF << FShft (DDAR_DS))
#define DDAR_DA		Fld (24, 8)	/* Device Address		   */
#define DDAR_DevAdd(Add)		/*  Device Address		   */ \
			(((Add) & 0xF0000000) | \
			 (((Add) & 0X003FFFFC) << (FShft (DDAR_DA) - 2)))
#define DDAR_Ser0UDCWr			/* Ser. port 0 UDC Write	   */ \
			(DDAR_DevWr + DDAR_Brst8 + DDAR_8BitDev + \
			 DDAR_Ser0UDCTr + DDAR_DevAdd (_Ser0UDCDR))
#define DDAR_Ser0UDCRd			/* Ser. port 0 UDC Read		   */ \
			(DDAR_DevRd + DDAR_Brst8 + DDAR_8BitDev + \
			 DDAR_Ser0UDCRc + DDAR_DevAdd (_Ser0UDCDR))
#define DDAR_Ser1UARTWr			/* Ser. port 1 UART Write	   */ \
			(DDAR_DevWr + DDAR_Brst4 + DDAR_8BitDev + \
			 DDAR_Ser1UARTTr + DDAR_DevAdd (_Ser1UTDR))
#define DDAR_Ser1UARTRd			/* Ser. port 1 UART Read	   */ \
			(DDAR_DevRd + DDAR_Brst4 + DDAR_8BitDev + \
			 DDAR_Ser1UARTRc + DDAR_DevAdd (_Ser1UTDR))
#define DDAR_Ser1SDLCWr			/* Ser. port 1 SDLC Write	   */ \
			(DDAR_DevWr + DDAR_Brst4 + DDAR_8BitDev + \
			 DDAR_Ser1SDLCTr + DDAR_DevAdd (_Ser1SDDR))
#define DDAR_Ser1SDLCRd			/* Ser. port 1 SDLC Read	   */ \
			(DDAR_DevRd + DDAR_Brst4 + DDAR_8BitDev + \
			 DDAR_Ser1SDLCRc + DDAR_DevAdd (_Ser1SDDR))
#define DDAR_Ser2UARTWr			/* Ser. port 2 UART Write	   */ \
			(DDAR_DevWr + DDAR_Brst4 + DDAR_8BitDev + \
			 DDAR_Ser2ICPTr + DDAR_DevAdd (_Ser2UTDR))
#define DDAR_Ser2UARTRd			/* Ser. port 2 UART Read	   */ \
			(DDAR_DevRd + DDAR_Brst4 + DDAR_8BitDev + \
			 DDAR_Ser2ICPRc + DDAR_DevAdd (_Ser2UTDR))
#define DDAR_Ser2HSSPWr			/* Ser. port 2 HSSP Write	   */ \
			(DDAR_DevWr + DDAR_Brst8 + DDAR_8BitDev + \
			 DDAR_Ser2ICPTr + DDAR_DevAdd (_Ser2HSDR))
#define DDAR_Ser2HSSPRd			/* Ser. port 2 HSSP Read	   */ \
			(DDAR_DevRd + DDAR_Brst8 + DDAR_8BitDev + \
			 DDAR_Ser2ICPRc + DDAR_DevAdd (_Ser2HSDR))
#define DDAR_Ser3UARTWr			/* Ser. port 3 UART Write	   */ \
			(DDAR_DevWr + DDAR_Brst4 + DDAR_8BitDev + \
			 DDAR_Ser3UARTTr + DDAR_DevAdd (_Ser3UTDR))
#define DDAR_Ser3UARTRd			/* Ser. port 3 UART Read	   */ \
			(DDAR_DevRd + DDAR_Brst4 + DDAR_8BitDev + \
			 DDAR_Ser3UARTRc + DDAR_DevAdd (_Ser3UTDR))
#define DDAR_Ser4MCP0Wr			/* Ser. port 4 MCP 0 Write (audio) */ \
			(DDAR_DevWr + DDAR_Brst4 + DDAR_16BitDev + \
			 DDAR_Ser4MCP0Tr + DDAR_DevAdd (_Ser4MCDR0))
#define DDAR_Ser4MCP0Rd			/* Ser. port 4 MCP 0 Read (audio)  */ \
			(DDAR_DevRd + DDAR_Brst4 + DDAR_16BitDev + \
			 DDAR_Ser4MCP0Rc + DDAR_DevAdd (_Ser4MCDR0))
#define DDAR_Ser4MCP1Wr			/* Ser. port 4 MCP 1 Write	   */ \
					/* (telecom)			   */ \
			(DDAR_DevWr + DDAR_Brst4 + DDAR_16BitDev + \
			 DDAR_Ser4MCP1Tr + DDAR_DevAdd (_Ser4MCDR1))
#define DDAR_Ser4MCP1Rd			/* Ser. port 4 MCP 1 Read	   */ \
					/* (telecom)			   */ \
			(DDAR_DevRd + DDAR_Brst4 + DDAR_16BitDev + \
			 DDAR_Ser4MCP1Rc + DDAR_DevAdd (_Ser4MCDR1))
#define DDAR_Ser4SSPWr			/* Ser. port 4 SSP Write (16 bits) */ \
			(DDAR_DevWr + DDAR_Brst4 + DDAR_16BitDev + \
			 DDAR_Ser4SSPTr + DDAR_DevAdd (_Ser4SSDR))
#define DDAR_Ser4SSPRd			/* Ser. port 4 SSP Read (16 bits)  */ \
			(DDAR_DevRd + DDAR_Brst4 + DDAR_16BitDev + \
			 DDAR_Ser4SSPRc + DDAR_DevAdd (_Ser4SSDR))

#define DCSR_RUN	0x00000001	/* DMA RUNing			   */
#define DCSR_IE		0x00000002	/* DMA Interrupt Enable		   */
#define DCSR_ERROR	0x00000004	/* DMA ERROR			   */
#define DCSR_DONEA	0x00000008	/* DONE DMA transfer buffer A	   */
#define DCSR_STRTA	0x00000010	/* STaRTed DMA transfer buffer A   */
#define DCSR_DONEB	0x00000020	/* DONE DMA transfer buffer B	   */
#define DCSR_STRTB	0x00000040	/* STaRTed DMA transfer buffer B   */
#define DCSR_BIU	0x00000080	/* DMA Buffer In Use		   */
#define DCSR_BufA	(DCSR_BIU*0)	/*  DMA Buffer A in use		   */
#define DCSR_BufB	(DCSR_BIU*1)	/*  DMA Buffer B in use		   */

#define DBT_TC		Fld (13, 0)	/* Transfer Count		   */
#define DBTA_TCA	DBT_TC		/* Transfer Count buffer A	   */
#define DBTB_TCB	DBT_TC		/* Transfer Count buffer B	   */


/*
 * Liquid Crystal Display (LCD) control registers
 *
 * Registers
 *    LCCR0		Liquid Crystal Display (LCD) Control Register 0
 *			(read/write).
 *			[Bits LDM, BAM, and ERM are only implemented in
 *			versions 2.0 (rev. = 8) and higher of the StrongARM
 *			SA-1100.]
 *    LCSR		Liquid Crystal Display (LCD) Status Register
 *			(read/write).
 *			[Bit LDD can be only read in versions 1.0 (rev. = 1)
 *			and 1.1 (rev. = 2) of the StrongARM SA-1100, it can be
 *			read and written (cleared) in versions 2.0 (rev. = 8)
 *			and higher.]
 *    DBAR1		Liquid Crystal Display (LCD) Direct Memory Access
 *			(DMA) Base Address Register channel 1 (read/write).
 *    DCAR1		Liquid Crystal Display (LCD) Direct Memory Access
 *			(DMA) Current Address Register channel 1 (read).
 *    DBAR2		Liquid Crystal Display (LCD) Direct Memory Access
 *			(DMA) Base Address Register channel 2 (read/write).
 *    DCAR2		Liquid Crystal Display (LCD) Direct Memory Access
 *			(DMA) Current Address Register channel 2 (read).
 *    LCCR1		Liquid Crystal Display (LCD) Control Register 1
 *			(read/write).
 *			[The LCCR1 register can be only written in
 *			versions 1.0 (rev. = 1) and 1.1 (rev. = 2) of the
 *			StrongARM SA-1100, it can be written and read in
 *			versions 2.0 (rev. = 8) and higher.]
 *    LCCR2		Liquid Crystal Display (LCD) Control Register 2
 *			(read/write).
 *			[The LCCR1 register can be only written in
 *			versions 1.0 (rev. = 1) and 1.1 (rev. = 2) of the
 *			StrongARM SA-1100, it can be written and read in
 *			versions 2.0 (rev. = 8) and higher.]
 *    LCCR3		Liquid Crystal Display (LCD) Control Register 3
 *			(read/write).
 *			[The LCCR1 register can be only written in
 *			versions 1.0 (rev. = 1) and 1.1 (rev. = 2) of the
 *			StrongARM SA-1100, it can be written and read in
 *			versions 2.0 (rev. = 8) and higher. Bit PCP is only
 *			implemented in versions 2.0 (rev. = 8) and higher of
 *			the StrongARM SA-1100.]
 *
 * Clocks
 *    fcpu, Tcpu	Frequency, period of the CPU core clock (CCLK).
 *    fmem, Tmem	Frequency, period of the memory clock (fmem = fcpu/2).
 *    fpix, Tpix	Frequency, period of the pixel clock.
 *    fln, Tln		Frequency, period of the line clock.
 *    fac, Tac		Frequency, period of the AC bias clock.
 */

#define LCD_PEntrySp	2		/* LCD Palette Entry Space [byte]  */
#define LCD_4BitPSp			/* LCD 4-Bit pixel Palette Space   */ \
					/* [byte]			   */ \
			(16*LCD_PEntrySp)
#define LCD_8BitPSp			/* LCD 8-Bit pixel Palette Space   */ \
					/* [byte]			   */ \
			(256*LCD_PEntrySp)
#define LCD_12_16BitPSp			/* LCD 12/16-Bit pixel		   */ \
					/* dummy-Palette Space [byte]	   */ \
			(16*LCD_PEntrySp)

#define LCD_PGrey	Fld (4, 0)	/* LCD Palette entry Grey value    */
#define LCD_PBlue	Fld (4, 0)	/* LCD Palette entry Blue value    */
#define LCD_PGreen	Fld (4, 4)	/* LCD Palette entry Green value   */
#define LCD_PRed	Fld (4, 8)	/* LCD Palette entry Red value	   */
#define LCD_PBS		Fld (2, 12)	/* LCD Pixel Bit Size		   */
#define LCD_4Bit			/*  LCD 4-Bit pixel mode	   */ \
			(0 << FShft (LCD_PBS))
#define LCD_8Bit			/*  LCD 8-Bit pixel mode	   */ \
			(1 << FShft (LCD_PBS))
#define LCD_12_16Bit			/*  LCD 12/16-Bit pixel mode	   */ \
			(2 << FShft (LCD_PBS))

#define LCD_Int0_0	0x0		/* LCD Intensity =   0.0% =  0	   */
#define LCD_Int11_1	0x1		/* LCD Intensity =  11.1% =  1/9   */
#define LCD_Int20_0	0x2		/* LCD Intensity =  20.0% =  1/5   */
#define LCD_Int26_7	0x3		/* LCD Intensity =  26.7% =  4/15  */
#define LCD_Int33_3	0x4		/* LCD Intensity =  33.3% =  3/9   */
#define LCD_Int40_0	0x5		/* LCD Intensity =  40.0% =  2/5   */
#define LCD_Int44_4	0x6		/* LCD Intensity =  44.4% =  4/9   */
#define LCD_Int50_0	0x7		/* LCD Intensity =  50.0% =  1/2   */
#define LCD_Int55_6	0x8		/* LCD Intensity =  55.6% =  5/9   */
#define LCD_Int60_0	0x9		/* LCD Intensity =  60.0% =  3/5   */
#define LCD_Int66_7	0xA		/* LCD Intensity =  66.7% =  6/9   */
#define LCD_Int73_3	0xB		/* LCD Intensity =  73.3% = 11/15  */
#define LCD_Int80_0	0xC		/* LCD Intensity =  80.0% =  4/5   */
#define LCD_Int88_9	0xD		/* LCD Intensity =  88.9% =  8/9   */
#define LCD_Int100_0	0xE		/* LCD Intensity = 100.0% =  1	   */
#define LCD_Int100_0A	0xF		/* LCD Intensity = 100.0% =  1	   */
					/* (Alternative)		   */

#define _LCCR0		0xB0100000	/* LCD Control Reg. 0		   */
#define _LCSR		0xB0100004	/* LCD Status Reg.		   */
#define _DBAR1		0xB0100010	/* LCD DMA Base Address Reg.	   */
					/* channel 1			   */
#define _DCAR1		0xB0100014	/* LCD DMA Current Address Reg.    */
					/* channel 1			   */
#define _DBAR2		0xB0100018	/* LCD DMA Base Address Reg.	   */
					/* channel 2			   */
#define _DCAR2		0xB010001C	/* LCD DMA Current Address Reg.    */
					/* channel 2			   */
#define _LCCR1		0xB0100020	/* LCD Control Reg. 1		   */
#define _LCCR2		0xB0100024	/* LCD Control Reg. 2		   */
#define _LCCR3		0xB0100028	/* LCD Control Reg. 3		   */

#if LANGUAGE == C
#define LCCR0				/* LCD Control Reg. 0		   */ \
			(*((volatile Word *) io_p2v (_LCCR0)))
#define LCSR				/* LCD Status Reg.		   */ \
			(*((volatile Word *) io_p2v (_LCSR)))
#define DBAR1				/* LCD DMA Base Address Reg.	   */ \
					/* channel 1			   */ \
			(*((volatile Address *) io_p2v (_DBAR1)))
#define DCAR1				/* LCD DMA Current Address Reg.    */ \
					/* channel 1			   */ \
			(*((volatile Address *) io_p2v (_DCAR1)))
#define DBAR2				/* LCD DMA Base Address Reg.	   */ \
					/* channel 2			   */ \
			(*((volatile Address *) io_p2v (_DBAR2)))
#define DCAR2				/* LCD DMA Current Address Reg.    */ \
					/* channel 2			   */ \
			(*((volatile Address *) io_p2v (_DCAR2)))
#define LCCR1				/* LCD Control Reg. 1		   */ \
			(*((volatile Word *) io_p2v (_LCCR1)))
#define LCCR2				/* LCD Control Reg. 2		   */ \
			(*((volatile Word *) io_p2v (_LCCR2)))
#define LCCR3				/* LCD Control Reg. 3		   */ \
			(*((volatile Word *) io_p2v (_LCCR3)))
#endif /* LANGUAGE == C */

#define LCCR0_LEN	0x00000001	/* LCD ENable			   */
#define LCCR0_CMS	0x00000002	/* Color/Monochrome display Select */
#define LCCR0_Color	(LCCR0_CMS*0)	/*  Color display		   */
#define LCCR0_Mono	(LCCR0_CMS*1)	/*  Monochrome display		   */
#define LCCR0_SDS	0x00000004	/* Single/Dual panel display	   */
					/* Select			   */
#define LCCR0_Sngl	(LCCR0_SDS*0)	/*  Single panel display	   */
#define LCCR0_Dual	(LCCR0_SDS*1)	/*  Dual panel display		   */
#define LCCR0_LDM	0x00000008	/* LCD Disable done (LDD)	   */
					/* interrupt Mask (disable)	   */
#define LCCR0_BAM	0x00000010	/* Base Address update (BAU)	   */
					/* interrupt Mask (disable)	   */
#define LCCR0_ERM	0x00000020	/* LCD ERror (BER, IOL, IUL, IOU,  */
					/* IUU, OOL, OUL, OOU, and OUU)    */
					/* interrupt Mask (disable)	   */
#define LCCR0_PAS	0x00000080	/* Passive/Active display Select   */
#define LCCR0_Pas	(LCCR0_PAS*0)	/*  Passive display (STN)	   */
#define LCCR0_Act	(LCCR0_PAS*1)	/*  Active display (TFT)	   */
#define LCCR0_BLE	0x00000100	/* Big/Little Endian select	   */
#define LCCR0_LtlEnd	(LCCR0_BLE*0)	/*  Little Endian frame buffer	   */
#define LCCR0_BigEnd	(LCCR0_BLE*1)	/*  Big Endian frame buffer	   */
#define LCCR0_DPD	0x00000200	/* Double Pixel Data (monochrome   */
					/* display mode)		   */
#define LCCR0_4PixMono	(LCCR0_DPD*0)	/*  4-Pixel/clock Monochrome	   */
					/*  display			   */
#define LCCR0_8PixMono	(LCCR0_DPD*1)	/*  8-Pixel/clock Monochrome	   */
					/*  display			   */
#define LCCR0_PDD	Fld (8, 12)	/* Palette DMA request Delay	   */
					/* [Tmem]			   */
#define LCCR0_DMADel(Tcpu)		/*  palette DMA request Delay	   */ \
					/*  [0..510 Tcpu]		   */ \
			((Tcpu)/2 << FShft (LCCR0_PDD))

#define LCSR_LDD	0x00000001	/* LCD Disable Done		   */
#define LCSR_BAU	0x00000002	/* Base Address Update (read)	   */
#define LCSR_BER	0x00000004	/* Bus ERror			   */
#define LCSR_ABC	0x00000008	/* AC Bias clock Count		   */
#define LCSR_IOL	0x00000010	/* Input FIFO Over-run Lower	   */
					/* panel			   */
#define LCSR_IUL	0x00000020	/* Input FIFO Under-run Lower	   */
					/* panel			   */
#define LCSR_IOU	0x00000040	/* Input FIFO Over-run Upper	   */
					/* panel			   */
#define LCSR_IUU	0x00000080	/* Input FIFO Under-run Upper	   */
					/* panel			   */
#define LCSR_OOL	0x00000100	/* Output FIFO Over-run Lower	   */
					/* panel			   */
#define LCSR_OUL	0x00000200	/* Output FIFO Under-run Lower	   */
					/* panel			   */
#define LCSR_OOU	0x00000400	/* Output FIFO Over-run Upper	   */
					/* panel			   */
#define LCSR_OUU	0x00000800	/* Output FIFO Under-run Upper	   */
					/* panel			   */

#define LCCR1_PPL	Fld (6, 4)	/* Pixels Per Line/16 - 1	   */
#define LCCR1_DisWdth(Pixel)		/*  Display Width [16..1024 pix.]  */ \
			(((Pixel) - 16)/16 << FShft (LCCR1_PPL))
#define LCCR1_HSW	Fld (6, 10)	/* Horizontal Synchronization	   */
					/* pulse Width - 2 [Tpix] (L_LCLK) */
#define LCCR1_HorSnchWdth(Tpix)		/*  Horizontal Synchronization	   */ \
					/*  pulse Width [2..65 Tpix]	   */ \
			(((Tpix) - 2) << FShft (LCCR1_HSW))
#define LCCR1_ELW	Fld (8, 16)	/* End-of-Line pixel clock Wait    */
					/* count - 1 [Tpix]		   */
#define LCCR1_EndLnDel(Tpix)		/*  End-of-Line Delay		   */ \
					/*  [1..256 Tpix]		   */ \
			(((Tpix) - 1) << FShft (LCCR1_ELW))
#define LCCR1_BLW	Fld (8, 24)	/* Beginning-of-Line pixel clock   */
					/* Wait count - 1 [Tpix]	   */
#define LCCR1_BegLnDel(Tpix)		/*  Beginning-of-Line Delay	   */ \
					/*  [1..256 Tpix]		   */ \
			(((Tpix) - 1) << FShft (LCCR1_BLW))

#define LCCR2_LPP	Fld (10, 0)	/* Line Per Panel - 1		   */
#define LCCR2_DisHght(Line)		/*  Display Height [1..1024 lines] */ \
			(((Line) - 1) << FShft (LCCR2_LPP))
#define LCCR2_VSW	Fld (6, 10)	/* Vertical Synchronization pulse  */
					/* Width - 1 [Tln] (L_FCLK)	   */
#define LCCR2_VrtSnchWdth(Tln)		/*  Vertical Synchronization pulse */ \
					/*  Width [1..64 Tln]		   */ \
			(((Tln) - 1) << FShft (LCCR2_VSW))
#define LCCR2_EFW	Fld (8, 16)	/* End-of-Frame line clock Wait    */
					/* count [Tln]			   */
#define LCCR2_EndFrmDel(Tln)		/*  End-of-Frame Delay		   */ \
					/*  [0..255 Tln]		   */ \
			((Tln) << FShft (LCCR2_EFW))
#define LCCR2_BFW	Fld (8, 24)	/* Beginning-of-Frame line clock   */
					/* Wait count [Tln]		   */
#define LCCR2_BegFrmDel(Tln)		/*  Beginning-of-Frame Delay	   */ \
					/*  [0..255 Tln]		   */ \
			((Tln) << FShft (LCCR2_BFW))

#define LCCR3_PCD	Fld (8, 0)	/* Pixel Clock Divisor/2 - 2	   */
					/* [1..255] (L_PCLK)		   */
					/* fpix = fcpu/(2*(PCD + 2))	   */
					/* Tpix = 2*(PCD + 2)*Tcpu	   */
#define LCCR3_PixClkDiv(Div)		/*  Pixel Clock Divisor [6..514]   */ \
			(((Div) - 4)/2 << FShft (LCCR3_PCD))
					/*  fpix = fcpu/(2*Floor (Div/2))  */
					/*  Tpix = 2*Floor (Div/2)*Tcpu    */
#define LCCR3_CeilPixClkDiv(Div)	/*  Ceil. of PixClkDiv [6..514]    */ \
			(((Div) - 3)/2 << FShft (LCCR3_PCD))
					/*  fpix = fcpu/(2*Ceil (Div/2))   */
					/*  Tpix = 2*Ceil (Div/2)*Tcpu	   */
#define LCCR3_ACB	Fld (8, 8)	/* AC Bias clock half period - 1   */
					/* [Tln] (L_BIAS)		   */
#define LCCR3_ACBsDiv(Div)		/*  AC Bias clock Divisor [2..512] */ \
			(((Div) - 2)/2 << FShft (LCCR3_ACB))
					/*  fac = fln/(2*Floor (Div/2))    */
					/*  Tac = 2*Floor (Div/2)*Tln	   */
#define LCCR3_CeilACBsDiv(Div)		/*  Ceil. of ACBsDiv [2..512]	   */ \
			(((Div) - 1)/2 << FShft (LCCR3_ACB))
					/*  fac = fln/(2*Ceil (Div/2))	   */
					/*  Tac = 2*Ceil (Div/2)*Tln	   */
#define LCCR3_API	Fld (4, 16)	/* AC bias Pin transitions per	   */
					/* Interrupt			   */
#define LCCR3_ACBsCntOff		/*  AC Bias clock transition Count */ \
					/*  Off				   */ \
			(0 << FShft (LCCR3_API))
#define LCCR3_ACBsCnt(Trans)		/*  AC Bias clock transition Count */ \
					/*  [1..15]			   */ \
			((Trans) << FShft (LCCR3_API))
#define LCCR3_VSP	0x00100000	/* Vertical Synchronization pulse  */
					/* Polarity (L_FCLK)		   */
#define LCCR3_VrtSnchH	(LCCR3_VSP*0)	/*  Vertical Synchronization pulse */
					/*  active High			   */
#define LCCR3_VrtSnchL	(LCCR3_VSP*1)	/*  Vertical Synchronization pulse */
					/*  active Low			   */
#define LCCR3_HSP	0x00200000	/* Horizontal Synchronization	   */
					/* pulse Polarity (L_LCLK)	   */
#define LCCR3_HorSnchH	(LCCR3_HSP*0)	/*  Horizontal Synchronization	   */
					/*  pulse active High		   */
#define LCCR3_HorSnchL	(LCCR3_HSP*1)	/*  Horizontal Synchronization	   */
					/*  pulse active Low		   */
#define LCCR3_PCP	0x00400000	/* Pixel Clock Polarity (L_PCLK)   */
#define LCCR3_PixRsEdg	(LCCR3_PCP*0)	/*  Pixel clock Rising-Edge	   */
#define LCCR3_PixFlEdg	(LCCR3_PCP*1)	/*  Pixel clock Falling-Edge	   */
#define LCCR3_OEP	0x00800000	/* Output Enable Polarity (L_BIAS, */
					/* active display mode)		   */
#define LCCR3_OutEnH	(LCCR3_OEP*0)	/*  Output Enable active High	   */
#define LCCR3_OutEnL	(LCCR3_OEP*1)	/*  Output Enable active Low	   */


#undef C
#undef Assembly
