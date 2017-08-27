/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  Segment Table definitions		File: segtable.h
    *
    *  The 'segment table' (bad name) is just a list of addresses
    *  of important stuff used during initialization.  We use these
    *  indirections to make life less complicated during code
    *  relocation.
    *  
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *  
    *********************************************************************  
    *
    *  Copyright 2000,2001,2002,2003
    *  Broadcom Corporation. All rights reserved.
    *  
    *  This software is furnished under license and may be used and 
    *  copied only in accordance with the following terms and 
    *  conditions.  Subject to these conditions, you may download, 
    *  copy, install, use, modify and distribute modified or unmodified 
    *  copies of this software in source and/or binary form.  No title 
    *  or ownership is transferred hereby.
    *  
    *  1) Any source code used, modified or distributed must reproduce 
    *     and retain this copyright notice and list of conditions 
    *     as they appear in the source file.
    *  
    *  2) No right is granted to use any trade name, trademark, or 
    *     logo of Broadcom Corporation.  The "Broadcom Corporation" 
    *     name may not be used to endorse or promote products derived 
    *     from this software without the prior written permission of 
    *     Broadcom Corporation.
    *  
    *  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
    *     IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
    *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
    *     PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
    *     SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
    *     PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
    *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
    *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    *     GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    *     BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
    *     OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
    *     TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
    *     THE POSSIBILITY OF SUCH DAMAGE.
    ********************************************************************* */


#if !defined(__ASSEMBLER__)
#define _TBLIDX(x)   (x)		/* C handles indexing for us */
typedef long segtable_t;		/* 32 for long32, 64 for long64 */
#endif

/*
 * Definitions for the segment_table
 */

#define R_SEG_ETEXT	     _TBLIDX(0)		/* end of text segment */
#define R_SEG_FDATA	     _TBLIDX(1)		/* Beginning of data segment */
#define R_SEG_EDATA	     _TBLIDX(2)		/* end of data segment */
#define R_SEG_END	     _TBLIDX(3)		/* End of BSS */
#define R_SEG_FTEXT          _TBLIDX(4)		/* Beginning of text segment */
#define R_SEG_FBSS           _TBLIDX(5)		/* Beginning of BSS */
#define R_SEG_GP	     _TBLIDX(6)		/* Global Pointer */
#define R_SEG_RELOCSTART     _TBLIDX(7)		/* Start of reloc table */
#define R_SEG_RELOCEND       _TBLIDX(8)		/* End of reloc table */
#define R_SEG_APIENTRY       _TBLIDX(9)		/* API Entry address */
#define R_SEG_CFERAM         _TBLIDX(10)	/* CFE RAM image address for cfe util*/
#define R_SEG_NAND_END	     _TBLIDX(11)	/* NAND loader end address for cfe util*/
/*
 * Definitions for the init_table
 */

#define R_INIT_EARLYINIT     _TBLIDX(0)		/* pointer to board_earlyinit */
#define R_INIT_SETLEDS       _TBLIDX(1)		/* pointer to board_setleds */
#define R_INIT_DRAMINFO      _TBLIDX(2)		/* pointer to board_draminfo */
#define R_INIT_CPUINIT       _TBLIDX(3)		/* pointer tp cpuinit */
#define R_INIT_ALTCPU_START1 _TBLIDX(4)		/* pointer to altcpu_start1 */
#define R_INIT_ALTCPU_START2 _TBLIDX(5)		/* pointer to altcpu_start2 */
#define R_INIT_ALTCPU_RESET  _TBLIDX(6)		/* pointer to altcpu_reset */
#define R_INIT_CPURESTART    _TBLIDX(7)		/* pointer to cpurestart */
#define R_INIT_DRAMINIT      _TBLIDX(8)		/* pointer to draminit */
#define R_INIT_CACHEOPS	     _TBLIDX(9)		/* pointer to cacheops */
#define R_INIT_TLBHANDLER    _TBLIDX(10)	/* pointer to TLB fault handler */
#define R_INIT_CMDSTART      _TBLIDX(11)	/* pointer to cfe_main */
#define R_INIT_CMDRESTART    _TBLIDX(12)	/* pointer to cfe_cmd_restart */
#define R_INIT_DOXREQ        _TBLIDX(13)	/* pointer to cfe_doxreq */
#define R_INIT_TP1_SWITCH    _TBLIDX(14)	/* pointer to tp1_switch */
#define R_INIT_SIZERAM       _TBLIDX(15)	/* pointer to board_sizeram */
#define R_INIT_BOARDINIT     _TBLIDX(16)	/* pointer to board_init */
#define R_INIT_PLL           _TBLIDX(17)        /* pointer to pll_init*/
#define R_INIT_PUTS          _TBLIDX(18)        /* pointer to board_puts */
#define R_INIT_GPIOLOOP      _TBLIDX(19)        /* pointer to gpio_loop */
#define R_INIT_NANDLDR       _TBLIDX(20)        /* pointer to nand_ldr for cfe util*/
#define R_INIT_PMC           _TBLIDX(21)        /* pointer to pmc_init */
#define R_INIT_GETC          _TBLIDX(22)        /* pointer to board_getc */
#define R_INIT_BTRM_ENABLED  _TBLIDX(23)        /* pointer to otp_is_btrm_enabled */
#define R_INIT_BOOT_SECURE   _TBLIDX(24)        /* pointer to otp_is_boot_secure */
#define R_INIT_WAIT_AVS      _TBLIDX(25)    /* pointer to wait_avs_done */
#define R_INIT_TP1_ENABLE    _TBLIDX(26)    /* pointer to otp_is_tp1_enabled */
#define R_INIT_POST_DDR      _TBLIDX(27)    /* pointer to post_ddr_board_init */

/*
 * Definitions for the diag_table
 */

#define R_DIAG_TEST1	    _TBLIDX(0)		/* after CPU and cache init, before DRAM init */
#define R_DIAG_TEST2	    _TBLIDX(1)		/* after DRAM init, before main */

/*
* Definitions for the abort_table
*/

#define R_ABORT_SETROMPARM      _TBLIDX(0)  /* pointer to set_romparm */
