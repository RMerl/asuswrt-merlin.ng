#include "sbmips.h"
#include "bsp_config.h"

#include "bcm_cpu.h"
#include "bcm_map.h"

#include "bcm_hwdefs.h"
#include "boardparms.h"
#include "mipsmacros.h"
#include "bcm63xx_rom.h"

#include "segtable.h"

#ifndef INC_BTRM_BOOT
#define INC_BTRM_BOOT         0
#endif

#if defined(CFE_ABORT_KEY)
#define PRINT_STR(s)                    \
            b 2f ;                      \
            nop ;                       \
1:          .string s ;      \
            .align   2 ;                \
2:          LOADREL(a0,1b) ;            \
            bal board_puts;          
#endif

#if defined(_BCM96848_)

	.text

#define R_BOARD_GET_REVID	_TBLIDX(0)

boardinit_table:
        _LONG_  otp_get_revid	# [ 0 ] R_BOARD_GET_REVID

#endif
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
    *       a0 - Flash base address (address of MIPS reset)
    *
    *  Return value:
    *       nothing
    ********************************************************************* */

LEAF(board_earlyinit)

        .set    noreorder

#ifdef CP0_CMT_TPID
        /* 6318(BMIPS 3300) is single threaded */
        mfc0    t1, C0_BCM_CONFIG, 3
        li      t2, CP0_CMT_TPID
        and     t1, t2
        bnez    t1, 3f                  # if we are running on thread 1, skip init
        nop
#endif
        /**-------------------------------------------------------------**/
        /** platform specific code                                      **/
        /**-------------------------------------------------------------**/

#if defined(_BCM960333_)
        /** ---- Set MIPS clock divider to 1. It defaults to 2. -------------------------- */
        mfc0    t1, C0_BCM_CONFIG, 4  
        li      t2, 0xFE7FFFFF // Clear bits [24:23]
        and     t1, t2
        sync
        mtc0    t1, C0_BCM_CONFIG, 4        
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        mfc0    t1, C0_BCM_CONFIG, 4
        li      t2, 0xFE3FFFFF // Clear bits [24:22]. Set up in sync mode. Only then RAC works(mips bug)!
        and     t1, t2
        sync
        mtc0    t1, C0_BCM_CONFIG, 4
        nop
        nop
        nop
        nop
        nop
        mfc0    t1, C0_BCM_CONFIG, 0
        li      t2, 0xFF7FFFFF // Clear bit 23.
        and     t1, t2
        sync
        mtc0    t1, C0_BCM_CONFIG, 0
        
#endif




#if defined (_BCM963268_)
        /* Configure VREG to use external 20 MHz xtal  */
        li      t2, MISC_BASE

        /* First set NOVL AND OVERCUR_SEL_1 to correct values */
        lw      t1, MISC_VREG_CONTROL0(t2)
        and     t1, ~((MISC_VREG_CONTROL0_NOVL_1_MASK << MISC_VREG_CONTROL0_NOVL_1_SHIFT) | (MISC_VREG_CONTROL0_NOVL_2_MASK << MISC_VREG_CONTROL0_NOVL_2_SHIFT) | (MISC_VREG_CONTROL0_OVERCUR_SEL_1_MASK << MISC_VREG_CONTROL0_OVERCUR_SEL_1_SHIFT))
        or      t1, (0x4 << MISC_VREG_CONTROL0_NOVL_1_SHIFT) | (0x2 << MISC_VREG_CONTROL0_NOVL_2_SHIFT) | (0x3 << MISC_VREG_CONTROL0_OVERCUR_SEL_1_SHIFT)
        sw      t1, MISC_VREG_CONTROL0(t2)

        /* Then set VCM to correct values */
        lw      t1, MISC_VREG_CONTROL1(t2)
        and     t1, ~((MISC_VREG_CONTROL1_VCM1_ADJ_MASK << MISC_VREG_CONTROL1_VCM1_ADJ_SHIFT) | (MISC_VREG_CONTROL1_VCM2_ADJ_MASK << MISC_VREG_CONTROL1_VCM2_ADJ_SHIFT))
        or      t1, (0x7d << MISC_VREG_CONTROL1_VCM1_ADJ_SHIFT) | (0x7d << MISC_VREG_CONTROL1_VCM2_ADJ_SHIFT)
        sw      t1, MISC_VREG_CONTROL1(t2)

        /* Then set external xtal */
        lw      t1, MISC_VREG_CONTROL2(t2)
        or      t1, MISC_VREG_CONTROL2_SWITCHCLOCKEN
        sw      t1, MISC_VREG_CONTROL2(t2)
        
        /* Finally latch-in changes */
        lw      t1, MISC_VREG_CONTROL0(t2)
        or      t1, MISC_VREG_CONTROL0_REG_RESET_B
        sw      t1, MISC_VREG_CONTROL0(t2)

        lw      t1, MISC_VDSL_CONTROL(t2)
        and     t1, ~(MISC_VDSL_CONTROL_VDSL_MIPS_RESET | MISC_VDSL_CONTROL_VDSL_MIPS_POR_RESET)
        sw      t1, MISC_VDSL_CONTROL(t2)
#endif

#if defined(JTAG_DELAY)

#if (INC_BTRM_BOOT==1)
        /* If secure boot, enable jtag interface. Auth has passed and the  */
        /* mips is running 2nd level bootloader. It is up to the customer  */
        /* to enable jtag after auth pass, and since we are compiling in   */
        /* jtag delay, one has to assume we want to connect to jtag        */
        li      t8, OTP_BASE
        lw      t9, OTP_OTP_SECURE_BOOT_CFG(t8)
        or      t9, OTP_OTP_SECURE_BOOT_CFG_UNLOCK_JTAG
        sw      t9, OTP_OTP_SECURE_BOOT_CFG(t8)
#endif

        /* wait for a while to allow catch by jtag debugger */
        /* doing it without overriding the C0_COUNT register */
        /* loop code relies on C0_COUNT being close to 0, far from its max */
        mfc0    t8, C0_COUNT
        li      t7, (200000000*3)
        add     t7, t8
catchloop:
        blt     t8, t7, catchloop
        mfc0    t8, C0_COUNT
#endif

        /**----- Enable I Cache -----------------------------------------**/
        mfc0    t1, C0_BCM_CONFIG
        or      t1, (CP0_BCM_CFG_ICSHEN | CP0_BCM_CFG_DCSHEN)
        mtc0    t1, C0_BCM_CONFIG       # Enable I Cache

#if defined (MIPS_LMB_CR)
        /* BMIPS 3300(6318) does not have these registers */

        // In the begining MIPS core registers are mapped to 0xbfax_xxxx
        li      t1, 0x1FA0000C          # Set up CBR to 1FAx_xxxx
        mtc0    t1, C0_BCM_CONFIG, 6

        li      t1, MIPS_BASE_BOOT
        lw      t2, MIPS_LMB_CR(t1)
        or      t2, 0xC0000000          # enable ffxx_xxxx space
        sw      t2, MIPS_LMB_CR(t1)
        li      t2, 0xFFF80001          # SBR FFF8_xxxx and enable
        sw      t2, MIPS_SBR(t1)
#endif

#if defined (_BCM960333_)
        li      t0, 0xbfa00014  #<---- core register offset 0x14 for mips3300.
        lw      t1, 0x0(t0)
        li      t2, 0xC0000000
        or      t1, t1, t2
        sw      t1, 0x0(t0)
#endif   

        // Now map MIPS core registers to 0xFF4x_xxxx space
        li      t1, 0xFF40000C          # CBR FF4x_xxxx (and reserved bits 0xc).
        mtc0    t1, C0_BCM_CONFIG, 6
        ehb
     
#if defined(_BCM96848_)
        # Enable LMB and clr half sp LMB clock
        li      t1, MIPS_BASE
        add     t1, MIPS_LMB_CR
        lw      t2, 0x0(t1)
        or      t2, LMB_EN
        and     t2, ~LMB_SPEED_MASK
        sw      t2, 0x0(t1)               

        /* Only in A0 , Change LDO voltage */
        move    t7, ra
        CALLINIT_KSEG1(boardinit_table,R_BOARD_GET_REVID)
        nop
        bne     v0, 0, _skip_ldo
        nop

        # Config LDO voltage
        li      t1, TOP_CNTRL_1V_LDO_CTRL
        li      t2, 0x81ba
        sw      t2, 0x0(t1)
        li      t1, TOP_CNTRL_1V_LDO_CTRL_EN
        li      t2, 0x1
        sw      t2, 0x0(t1)

_skip_ldo:
        nop
        move    ra, t7

#endif


        /**----- Initialize Serial --------------------------------------**/
#if defined(IKOS_FAST_UART)
	/* set to the maximum possible rate for simualtion: 50M/32=1562500 */
	li      t3, 0x0
#else	
        li      t3, ((FPERIPH / 115200) / 16)
        /*
        #  Baudword = (FPeriph)/Baud/32-1.  We have to perform rounding
        #  and subtraction.  Above we divided by 16 (instead of 32).  If
        #  bit0 is set, we round up.  However, we then subtract 1, so final
        #  result should be t3/2.  If bit0 is 0, then we truncate and subtract
        #  1, t3=t3/2-1.
        */
        andi    t0, t3, 0x1
        bne     t0,zero,1f              # do shift only (in delay slot)
                                        # and jump to apply
        srl     t3,1                    # do divide by 2
        addiu   t3, -1                  # subtract 1
1:
#endif
        // t3 contains the UART BAUDWORD
        li      t0, UART_BASE
        sw      t3, UART0BAUD(t0)       # Store BaudRate
        li      t1, BITS8SYM|ONESTOP
        sb      t1, UART0CONFIG(t0)     # 8 Bits/1 Stop
        li      t1, TXEN|RXEN|BRGEN
        sb      t1, UART0CONTROL(t0)    # Enable, No Parity
        move    t1, zero
        sh      t1, UART0INTMASK(t0)

#if defined (_BCM960333_)
        /* Configure GPIO_MUX to enable GPIOs for UART and SPI.
         * Set bits 8:4 in GPIO_FUNC_MODE_CTRL to map GPIOs 8:4
         * according to GPIO_MUX and use the default MUX setup:
         * GPIO4: UART_Rx  GPIO5: UART_Tx  GPIO6: SPIS_MOSI
         * GPIO7: SPIS_MISO  GPIO8: SPIS_SS*/
        li      t0, 0x1f0                 # Bits 8:4
        li      t1, GPIO_FUNC_MODE_CTRL
        sw      t0, 0(t1)
#endif
        .set    reorder
3:
#if defined(_BCM960333_)
/* Reset the whole chip if we came from STBY */
#define BSTI_CTRL_READ_AON_WAKEUP_CODE  ((AON_WAKEUP_READ_REG << BSTI_SER_CTRL_ADDR_SHIFT) & BSTI_SER_CTRL_ADDR_MASK) | \
                                        ((BSTI_READ_OP << BSTI_SER_CTRL_CMD_SHIFT) & BSTI_SER_CTRL_CMD_MASK) | \
                                        ((BSTI_START_OP << BSTI_SER_CTRL_START_SHIFT) & BSTI_SER_CTRL_START_MASK)

#define BSTI_CTRL_CLEAR_AON_WAKEUP_CODE  (0x0 & BSTI_SER_CTRL_WR_DATA_MASK) | \
                                         ((AON_WAKEUP_READ_REG << BSTI_SER_CTRL_ADDR_SHIFT) & BSTI_SER_CTRL_ADDR_MASK) | \
                                         ((BSTI_WRITE_OP << BSTI_SER_CTRL_CMD_SHIFT) & BSTI_SER_CTRL_CMD_MASK) | \
                                         ((BSTI_START_OP << BSTI_SER_CTRL_START_SHIFT) & BSTI_SER_CTRL_START_MASK)
        /* Read AON wake up code */
        li      t0, BSTI_BASE
        li      t1, BSTI_CTRL_READ_AON_WAKEUP_CODE
        sw      t1, BSTI_SER_CTRL(t0)
        nop
bsti_read_wait:
        li      t0, BSTI_BASE
        lw      t1, BSTI_SER_CTRL(t0)
        li      t0, BSTI_SER_CTRL_START_MASK
        and     t0, t1, t0
        srl     t1, t0, BSTI_SER_CTRL_START_SHIFT
        li      t0, 1
        beq     t1, t0, bsti_read_wait
        nop

        /* Check if we come from STBY */
        li      t0, BSTI_BASE
        lw      t0, BSTI_SER_STATUS(t0)
        andi    t1, t0, (BSTI_SER_STATUS_RD_DATA_MASK & AON_STBY_WOKEN_STAT_MASK)
        beqz    t1, no_stby
        nop

        /* From STBY: Clear AON wake code */
        li      t0, BSTI_BASE
        li      t1, BSTI_CTRL_CLEAR_AON_WAKEUP_CODE
        sw      t1, BSTI_SER_CTRL(t0)
        nop
bsti_write_wait:
        li      t0, BSTI_BASE
        lw      t1, BSTI_SER_CTRL(t0)
        li      t0, BSTI_SER_CTRL_START_MASK
        and     t0, t1, t0
        srl     t1, t0, BSTI_SER_CTRL_START_SHIFT
        li      t0, 1
        beq     t1, t0, bsti_write_wait
        nop
        /* SW reset and wait */
        li      t0, TIMER_EN_SW_PLL
        li      t1, 1
        sw      t1, 0(t0)
loop:
        b       loop
        nop
no_stby:
#endif

        j       ra
        nop
END(board_earlyinit)

/*  *********************************************************************
    *  BOARD_DRAMINFO
    *
    *  Return the address of the DRAM information table
    *
    *  Input parameters:
    *       nothing
    *
    *  Return value:
    *       v0 - DRAM info table, return 0 to use default table
    ********************************************************************* */
LEAF(board_draminfo)
        j       ra
END(board_draminfo)

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
    *       a0 - LED value (8 bits per character, 4 characters)
    *
    *  Return value:
    *       nothing
    *
    *  Registers used:
    *       t7,t8,t9
    ********************************************************************* */
LEAF(board_setleds)
#if 1
        li      t7, UART_BASE
        li      t8, TXFIFOEMT

1:      lh      t9, UART0INTSTAT(t7)
        and     t9, t8
        bne     t9, t8, 1b

        srl     t8, a0, 24
        sb      t8, UART0DATA(t7)
        srl     t8, a0, 16
        sb      t8, UART0DATA(t7)
        srl     t8, a0, 8
        sb      t8, UART0DATA(t7)
        sb      a0, UART0DATA(t7)
        li      a0, '\r'
        sb      a0, UART0DATA(t7)
        li      a0, '\n'
        sb      a0, UART0DATA(t7)
#endif
        j       ra
END(board_setleds)

LEAF(board_puts)
1:  li      t0, UART_BASE /* wait for room in uart */
    li      t1, TXFIFOEMT
    lh      t2, UART0INTSTAT(t0)
    and     t2, t1
    bne     t2, t1, 1b
    nop
    lbu	    t1, 0(a0)
    beqz    t1, 2f /* null ... done */
    nop 
    sb      t1, UART0DATA(t0)
    addi    a0, 1
    b       1b /* go to next char */
    nop

2:
    li      v0, 0	
    j       ra
END(board_puts)

#if defined(_BCM96848_)
LEAF(board_putc)
1:  li      t0, UART_BASE /* wait for room in uart */
    li      t1, TXFIFOEMT
    lh      t2, UART0INTSTAT(t0)
    and     t2, t1
    bne     t2, t1, 1b
    nop
 
    sb      a0, UART0DATA(t0)
   
    j       ra
END(board_putc)
#endif

	
#if defined(JTAG_DELAY)
/*  *********************************************************************
    *  board_setleds1(x)
    *
    *  This prints 4 characters on the UART0. No line return will be printed.
    *
    *  Input parameters:
    *       a0 - 32bit char value (8 bits per character, 4 characters)
    *
    *  Return value:
    *       nothing
    *
    *  Registers used:
    *       t7,t8,t9
    ********************************************************************* */

LEAF(board_setleds1)
        li      t7, UART_BASE
        li      t8, TXFIFOEMT

1:      lh      t9, UART0INTSTAT(t7)
        and     t9, t8
        bne     t9, t8, 1b

        srl     t8, a0, 24
        sb      t8, UART0DATA(t7)
        srl     t8, a0, 16
        sb      t8, UART0DATA(t7)
        srl     t8, a0, 8
        sb      t8, UART0DATA(t7)
        sb      a0, UART0DATA(t7)        

        j       ra
END(board_setleds1)

/*  *********************************************************************
    *  board_print_register(x)
    *
    *  Prints a register value using UART0.
    *
    *  Input parameters:
    *       a0 - Register value(32bit) to be printed in hexadecimal.
    *       a1 - If 0xA5A5 no newline will be printed at the end of the 32 bit hexadecimal char.
    *
    *  Return value:
    *       nothing
    *
    *  Registers used:
    *      s1,t5,t6,t7,t8,t9
    ********************************************************************* */
LEAF(board_print_register)

        
        move    t9, a0
        move    s1, ra   /*  ra will be overwritten by below funciton calls and LOADREL */
   
        LOADREL(t7, ascii_num_table) 
        
        srl     t8, t9, 16 /* Take 16 HI Bits */
        
        andi    t6, t8, 0x000F /* Remove first nibble */
        add     t6, t7         /* Locate ascii code for nibble */ 
        lb      a0, 0x00(t6)   /* Load ascii code into a0 */   
        
        andi    t6, t8, 0x00F0 /* Second nibble */
        srl     t6, 4          /* Move removed nibble to LSB */
        add     t6, t7             
        lb      t5, 0x00(t6)   /* Load ascii code into t5 */
        sll     t5, 8          /* Move ascii code into position */
        or      a0, t5         /* Accumulate ascii codes in position */
           
        andi    t6, t8, 0x0F00 /* Third nibble */
        srl     t6, 8          
        add     t6, t7             
        lb      t5, 0x00(t6)   
        sll     t5, 16          
        or      a0, t5             
        
        andi    t6, t8, 0xF000 /* Fourth nibble */
        srl     t6, 12          
        add     t6, t7             
        lb      t5, 0x00(t6)   
        sll     t5, 24          
        or      a0, t5  

        move    t5, t9      /* t7, t9 will be corrupted by board_setleds1 */
        
        bal     board_setleds1
        nop

        LOADREL(t7, ascii_num_table)         

        move    t8, t5        /* Take 16 LO Bits */
        
        andi    t6, t8, 0x000F /* Remove first nibble */
        add     t6, t7         /* Locate ascii code for nibble */ 
        lb      a0, 0x00(t6)   /* Load ascii code into a0 */   
        
        andi    t6, t8, 0x00F0 /* Second nibble */
        srl     t6, 4          /* Move removed nibble to LSB */
        add     t6, t7             
        lb      t5, 0x00(t6)   /* Load ascii code into t5 */
        sll     t5, 8          /* Move ascii code into position */
        or      a0, t5         /* Accumulate ascii codes in position  */
           
        andi    t6, t8, 0x0F00 /* Third nibble */
        srl     t6, 8          
        add     t6, t7             
        lb      t5, 0x00(t6)   
        sll     t5, 16          
        or      a0, t5             
        
        andi    t6, t8, 0xF000 /* Fourth nibble */
        srl     t6, 12          
        add     t6, t7             
        lb      t5, 0x00(t6)   
        sll     t5, 24          
        or      a0, t5  

        li      t5, 0xA5A5
        beq     a1, t5, 1f
        bal     board_setleds 
        b       2f
        
        
1:      bal     board_setleds1

2:      move    ra, s1
        j       ra
        
END(board_print_register)
#endif

#if defined(CFE_ABORT_KEY)
/*  *********************************************************************
*  BOARD_GETC(x)
*
*  Get one character from uart0
*
*  Input parameters:
*  		None
*
*  Return value:
*  		v0 - Input chracter value (8 bit).
*
*  Registers used:
*  		t7, t8, t9
********************************************************************* */
LEAF(board_getc)
1:  li      t7, UART_BASE /* check if rx fifo is not empty */
    li      t8, RXFIFONE
    lh      t9, UART0INTSTAT(t7)
    and     t9, t8
    bne     t9, t8, 2f
    nop
    lb      v0, UART0DATA(t7)
2:  j       ra
END(board_getc)
#endif

/*  *********************************************************************
    *  BCMCORE_TP1_SWITCH()
    *
    *  Check if the thread switch is required. If we are already
    *  running on thread 1 this function will do nothing and just return
    *  If we are running on thread 0 this function will take thread 1
    *  out of reset and put thread 0 to sleep waiting for singnal from
    *  thread 1.
    *
    *  Input parameters:
    *       nothing
    *
    *  Return value:
    *       nothing
    ********************************************************************* */

#ifdef CP0_CMT_TPID

LEAF(bcmcore_tp1_switch)


        mfc0    t1, C0_BCM_CONFIG, 3
        li      t2, CP0_CMT_TPID
        and     t1, t2
        bnez    t1, tp1                 # Already running on thread 1

# Start TP1
# Set boot address for TP1
        li      t1, MIPS_BASE
        li      t2, 0xB8000000 | ENABLE_ALT_BV
        sw      t2, MIPS_TP1_ALT_BV(t1)

# Set a flag so we can wait for TP1 to catch up
        mfc0    t1, C0_BCM_CONFIG
        move    t2, t1                  # Save C0_BCM_CONFIG register
        and     t1, ~CP0_BCM_CFG_TLBPD  # Reset TLBPD bit
        mtc0    t1, C0_BCM_CONFIG      

# Take TP1 out of reset
        mfc0    t1, C0_BCM_CONFIG, 2
        or      t1, CP0_CMT_RSTSE
        mtc0    t1, C0_BCM_CONFIG, 2

        /* wait until second thread catches up with the first */
waittp1:
        mfc0    t0, C0_BCM_CONFIG
        and     t0, CP0_BCM_CFG_TLBPD
        beqz    t0, waittp1             # branch while TLBPD is reset
        nop
        mtc0    t2, C0_BCM_CONFIG       # Restore C0_BCM_CONFIG register

        li      t0, THREAD_NUM_ADDRESS
        FIXUP(t0)
        lw      t0, 0(t0)
        li      t1, 1
        bne     t0, t1, return          # Linux will run on TP0, continue running bootloader

# Voice will run on TP0. Set it up and put it to sleep

        # enable interrupts and enable SW IRQ 0
        li      t0, M_SR_IE | M_SR_IBIT1
        mtc0    t0, C0_SR

        # Set up to use alternate exception vector 0x80000200
        li      t0, M_CAUSE_IV
        mtc0    t0, C0_CAUSE

        mfc0    t1, C0_BCM_CONFIG, 1
        # set all ints except IRQ1 to TP1 and cross over SW IRQ 0
        or      t1, (CP0_CMT_XIR_4 | CP0_CMT_XIR_3 | CP0_CMT_XIR_2 | CP0_CMT_XIR_0 | CP0_CMT_SIR_0 | CP0_CMT_NMIR_TP1)
        mtc0    t1, C0_BCM_CONFIG, 1

        mfc0    t1, C0_BCM_CONFIG, 2
        # Set debug on TP1 and set TLB exception serialization to ignore SCNT value in CP0 reg22 sel 4
        and     t1, ~CP0_CMT_TPS_MASK;
        or      t1, (CP0_CMT_DSU_TP1 | (1 << CP0_CMT_TPS_SHFT))
        mtc0    t1, C0_BCM_CONFIG, 2

2:
        b       wait_for_wake

tp1:
# Running on TP1....
# First signal to TP0 that TP1 is up
        mfc0    t1, C0_BCM_CONFIG
        or      t1, CP0_BCM_CFG_TLBPD   # Set TLBPD bit
        mtc0    t1, C0_BCM_CONFIG

        li      t0, THREAD_NUM_ADDRESS
        FIXUP(t0)
        lw      t0, 0(t0)
        li      t1, 1
        beq     t0, t1, return          # Linux will run on TP1, continue running bootloader

# Voice will run on TP1. Set it up and put it to sleep

        # enable interrupts and enable SW IRQ 0
        li      t0, M_SR_IE | M_SR_IBIT1
        mtc0    t0, C0_SR

        # Set up to use alternate exception vector 0x80000200
        li      t0, M_CAUSE_IV
        mtc0    t0, C0_CAUSE

        mfc0    t1, C0_BCM_CONFIG, 1
        # set IRQ1 to TP1 and cross over SW IRQ 0
        or      t1, (CP0_CMT_XIR_1 | CP0_CMT_SIR_0 | CP0_CMT_NMIR_TP0)
        mtc0    t1, C0_BCM_CONFIG, 1

        mfc0    t1, C0_BCM_CONFIG, 2
        # Set debug on TP0 and set TLB exception serialization to ignore SCNT value in CP0 reg22 sel 4
        and     t1, ~CP0_CMT_TPS_MASK;
        or      t1, (1 << CP0_CMT_TPS_SHFT)
        mtc0    t1, C0_BCM_CONFIG, 2

        b       2b

return:
        j       ra

END(bcmcore_tp1_switch)
#endif


LEAF(board_init)

#if defined(_BCM96848_)
    .set    noreorder

    /* set fast async mode */
    mfc0    t1, C0_BCM_CONFIG, 5
    and     t1, ~(0x3 << 28)
    mtc0    t1, C0_BCM_CONFIG, 5

    .set    reorder
#endif

        j       ra

END(board_init)

#if defined(_BCM96848_)
LEAF(post_ddr_board_init)

    .set    noreorder

    /* Enable RAC (for I-cache only) */
    li      t0, MIPS_BASE
    li      t2, 0xFFF << RAC_UPB_SHFT  /* Enable prefetch for RAM address range up to 256MB */
    sw      t2, MIPS_RAC_ARR(t0)

    lw      t2, MIPS_RAC_CR0(t0)
    or      t2, (RAC_C_INV | RAC_I | RAC_PF_I)
    sw      t2, MIPS_RAC_CR0(t0)

    lw      t2, MIPS_RAC_CR1(t0)
    or      t2, (RAC_C_INV | RAC_I | RAC_PF_I)
    sw      t2, MIPS_RAC_CR1(t0)

    /* Enable branch prediction */
    mfc0    t1, C0_BCM_CONFIG
    and     t1, ~CP0_BCM_CFG_BTHD   /* Enable branch history table */
    or      t1, CP0_BCM_CFG_NBK     /* Data cache non blocking */
    or      t1, CP0_BCM_CFG_CLF     /* Critical line first */
    mtc0    t1, C0_BCM_CONFIG

    .set    reorder

    j		ra

END(post_ddr_board_init)
#endif



# align this code to cache line. NAND flash is not memory mapped after system boots
# so when we are signaling to the second TP to wake we need
# jal instruction to be in cache
        .align  4
LEAF(wait_for_wake)
        sync
        wait                            # wait for interrupt
        jal     t8                      # jump to entry point
END(wait_for_wake)

#if defined(CFE_ABORT_KEY)
LEAF(set_romparm)
       li   t1, 0x1
       sll  t1, t1, 29
       or ROMOPTION, t1

       j	ra
END(set_romparm)
#endif

