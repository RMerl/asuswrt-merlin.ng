#include "sbmips.h"

/*  *********************************************************************
    *  BOARD_EARLYINIT()
    *  
    *  Initialize board registers.  This is the earliest 
    *  time the BSP gets control.  This routine cannot assume that
    *  memory is operational, and therefore all code in this routine
    *  must run from registers only.  The $ra register must not
    *  be modified, as it contains the return address.
    *
    *  This routine will be called from uncached space, before
    *  the caches are initialized.  If you want to make
    *  subroutine calls from here, you must use the CALLKSEG1 macro.
    *
    *  Among other things, this is where the GPIO registers get 
    *  programmed to make on-board LEDs function, or other startup
    *  that has to be done before anything will work.
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */
LEAF(board_earlyinit)
#if defined (_BCM968500_)
/* FIXME this is a bit ugly at the moment, but it works */
		li	sp, 0xa3000000 // safe value for the moment
		li	t1, 0xa3fffffc // last word of 64M
		li	t2, 0xa7fffffc // last word of 128M
		li	t3, 0xaffffffc // last word of 256M
		li	t4, 0xfeed6464 
		li	t5, 0x12801280 
		li	t6, 0x02560256
		sw	t4, 0(t1)
		sw	t5, 0(t2)
		sw	t6, 0(t3)

		// now see which survived
		lw	t6, 0(t1)
		li	a0, 0x04000000 // 64M
		bne	t6,t4,1f
		nop

		lw	t6, 0(t2)
		li	a0, 0x08000000 // 128M
		bne	t6,t5,1f
		nop

		li	a0, 0x10000000 // 256M

1:		la	v0, cfe_sdramsize
		sw	a0, 0(v0)
#endif
		j	ra
END(board_earlyinit)

/*  *********************************************************************
    *  BOARD_DRAMINFO
    *  
    *  Return the address of the DRAM information table
    *  
    *  Input parameters: 
    *  	   nothing
    *  	    
    *  Return value:
    *  	   v0 - DRAM info table, return 0 to use default table
    ********************************************************************* */
LEAF(board_draminfo)
		j	ra
END(board_draminfo)

/*  *********************************************************************
    *  BOARD_DRAMINIT
    *  
    *  This routine should activate memory.
    *  
    *  Input parameters: 
    *  	   a0 - points to configuration table returned by board_draminfo
    *           or 0 to use an automatic table
    *  	   
    *  Return value:
    *  	   v0 - total memory installed
    *  	   
    *  Registers used:
    *  	   can use all registers.
    ********************************************************************* */
LEAF(board_draminit)
		j	ra
END(board_draminit)

/*  *********************************************************************
    *  BOARD_SETLEDS(x)
    *  
    *  Set LEDs for boot-time progress indication.  Not used if
    *  the board does not have progress LEDs.  This routine
    *  must not call any other routines, since it may be invoked
    *  either from KSEG0 or KSEG1 and it may be invoked 
    *  whether or not the icache is operational.
    *  
    *  Input parameters: 
    *  	   a0 - LED value (8 bits per character, 4 characters)
    *  	   
    *  Return value:
    *  	   nothing
    *  
    *  Registers used:
    *  	   t0,t1,t2,t3
    ********************************************************************* */
LEAF(board_setleds)
		j	ra
END(board_setleds)

LEAF(bcmcore_tp1_switch)
		j	ra
END(bcmcore_tp1_switch)
