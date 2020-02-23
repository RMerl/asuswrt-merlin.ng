/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  Exception Handler			File: exchandler.c       
    *  
    *  This is the "C" part of the exception handler and the
    *  associated setup routines.  We call these routines from
    *  the assembly-language exception handler.
    *  
    *  Author: 
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


#include "lib_types.h"
#include "lib_string.h"
#include "lib_printf.h"
#include "lib_queue.h"
#include "lib_malloc.h"
#include "exception.h"
#include "cfe.h"
#include "cfe_error.h"
#include "cfe_iocb.h"
#include "exchandler.h"
#include "cpu_config.h"
#include "bsp_config.h"

/*  *********************************************************************
    *  Constants
    ********************************************************************* */

/*  *********************************************************************
    *  Globals 
    ********************************************************************* */

exc_handler_t exc_handler;

extern void softReset(unsigned int delay);


/*  *********************************************************************
    *  cfe_exception(code,info)
    *  
    *  Exception handler.  This routine is called when any CPU 
    *  exception that is handled by the assembly-language
    *  vectors is reached.  The usual thing to do here is just to
    *  reboot.
    *  
    *  Input parameters: 
    *         code - exception type
    *         info - exception stack frame
    *         
    *  Return value:
    *         usually reboots
    ********************************************************************* */
#ifdef CFG_ARMV8_AARCH64
void cfe_exception(trap_t *tr)
{
    uint64_t *stack = (uint64_t*)(uintptr_t)tr->sp;
    char *tr_type_str0[4] = {"Current EL SP0", "Current EL SPx", "Lower EL AArch64", "Lower EL AArch32"};
    char *tr_type_str1[4] = {"Synchronous", "IRQ", "FIQ", "SystemError"};

    char *type_str0 = tr_type_str0[tr->type>>2];
    char *type_str1 = tr_type_str1[tr->type&0x3];

    if(exc_handler.catch_exc == 1) {
        /*Deal with exception without restarting CFE.*/
        /*Clear relevant SR bits*/
        /*Reset flag*/
        exc_handler.catch_exc = 0;

        exc_longjmp_handler();
    }

    /* Note that UTF parses the first line, so the format should not be changed. */
    printf("\nException type [%s %s], Exception frame at %p:\n", type_str0, type_str1, (uintptr_t)tr);
    printf("\tesr[%08x], cpsr[%08x], spsr[%08x], elr[%016llx], far[%016llx]\n",
       tr->esr, tr->cpsr, tr->spsr, tr->elr, tr->far);

    printf("\t  x0[%016llx],  x1[%016llx],  x2[%016llx],  x3[%016llx]\n",
           tr->x0, tr->x1, tr->x2, tr->x3);
    printf("\t  x4[%016llx],  x5[%016llx],  x6[%016llx],  x7[%016llx]\n",
           tr->x4, tr->x5, tr->x6, tr->x7);
    printf("\t  x8[%016llx],  x9[%016llx], x10[%016llx], x11[%016llx]\n",
           tr->x8, tr->x9, tr->x10, tr->x11);
    printf("\t x12[%016llx], x13[%016llx], x14[%016llx], x15[%016llx]\n",
           tr->x12, tr->x13, tr->x14, tr->x15);

    printf("\t x16[%016llx], x17[%016llx], x18[%016llx], x19[%016llx]\n",
           tr->x16, tr->x17, tr->x18, tr->x19);
    printf("\t x20[%016llx], x21[%016llx], x22[%016llx], x23[%016llx]\n",
           tr->x20, tr->x21, tr->x22, tr->x23);
    printf("\t x24[%016llx], x25[%016llx], x26[%016llx], x27[%016llx]\n",
           tr->x24, tr->x25, tr->x26, tr->x27);
    printf("\t x28[%016llx], x29[%016llx], x30[%016llx],  sp[%016llx]\n",
           tr->x28, tr->x29, tr->x30, tr->sp);

    /*
     * stack content before trap occured
     */
    printf("\nStack dump:\n");
    printf("\t0x%016llx: %016llx %016llx %016llx %016llx\n",
        tr->sp, stack[0], stack[1], stack[2], stack[3]);
    printf("\t0x%016llx: %016llx %016llx %016llx %016llx\n",
        tr->sp+0x20, stack[4], stack[5], stack[6], stack[7]);
    printf("\n");
    softReset(60); //set watch dog timer to 60s to give some time for JTAG connection.
}
#else
void cfe_exception(trap_t *tr)
{
    /*
     * ARM7TDMI trap types:
     *    0=RST, 1=UND, 2=SWI, 3=IAB, 4=DAB, 5=BAD, 6=IRQ, 7=FIQ
     *
     * ARM CM3 trap types:
     *    1=RST, 2=NMI, 3=FAULT, 4=MM, 5=BUS, 6=USAGE, 11=SVC,
     *    12=DMON, 14=PENDSV, 15=SYSTICK, 16+=ISR
      *
     * ARM CA9 trap types:
     *    0=RST, 1=UND, 2=SWI, 3=IAB, 4=DAB, 5=BAD, 6=IRQ, 7=FIQ
     */

    uint32 *stack = (uint32*)(uintptr_t)tr->r13;
    char *tr_type_str[8] = {"RST", "UND", "SWI", "IAB", "DAB", "BAD", "IRQ", "FIQ"};
    char *type_str = "UKN";

    if(exc_handler.catch_exc == 1) {
        /*Deal with exception without restarting CFE.*/
        /*Clear relevant SR bits*/
        /*Reset flag*/
        exc_handler.catch_exc = 0;

        exc_longjmp_handler();
    }

    if (tr->type < 8)
        type_str = tr_type_str[tr->type];

    /* Note that UTF parses the first line, so the format should not be changed. */
    printf("\nException type [%s], Exception frame at %p:\n", type_str, tr);
    printf("\tpc[%08x], lr[%08x], sp[%08x], cpsr[%08x], spsr[%08x]\n",
           tr->pc, tr->r14, tr->r13, tr->cpsr, tr->spsr);
    printf("\tr0[%08x], r1[%08x], r2[%08x],  r3[%08x],  r4[%08x],  r5[%08x],  r6[%08x]\n",
           tr->r0, tr->r1, tr->r2, tr->r3, tr->r4, tr->r5, tr->r6);
    printf("\tr7[%08x], r8[%08x], r9[%08x], r10[%08x], r11[%08x], r12[%08x]\n",
           tr->r7, tr->r8, tr->r9, tr->r10, tr->r11, tr->r12);

    /*
     * stack content before trap occured
     */
    printf("\nStack dump:\n");

    printf("\t0x%08x: %08x %08x %08x %08x\n",
        tr->r13, stack[0], stack[1], stack[2], stack[3]);
    printf("\t0x%08x: %08x %08x %08x %08x\n\n",
        tr->r13+0x10, stack[4], stack[5], stack[6], stack[7]);

    xprintf("\n");
    softReset(60); //set watch dog timer to 60s to give some time for JTAG connection.
}
#endif
/*  *********************************************************************
    *  cfe_setup_exceptions()
    *  
    *  Set up the exception handlers.  
    *  
    *  Input parameters: 
    *         nothing
    *         
    *  Return value:
    *         nothing
    ********************************************************************* */
void cfe_setup_exceptions(void)
{
    exc_handler.catch_exc = 0;
    q_init( &(exc_handler.jmpbuf_stack));

    /* Set trap handler */
    _exc_set_trap(cfe_exception);
}


/*  *********************************************************************
    *  exc_initialize_block()
    *
    *  Set up the exception handler.  Allow exceptions to be caught. 
    *  Allocate memory for jmpbuf and store it away.
    *
    *  Returns NULL if error in memory allocation.
    *  
    *  Input parameters: 
    *         nothing
    *         
    *  Return value:
    *         jmpbuf_t structure, or NULL if no memory
    ********************************************************************* */
jmpbuf_t *exc_initialize_block(void)
{
    jmpbuf_t *jmpbuf_local;

    exc_handler.catch_exc = 1;

    /* Create the jmpbuf_t object */
    jmpbuf_local = (jmpbuf_t *) KMALLOC((sizeof(jmpbuf_t)),0);

    if (jmpbuf_local == NULL) {
        return NULL;
    }

    q_enqueue( &(exc_handler.jmpbuf_stack), &((*jmpbuf_local).stack));

    return jmpbuf_local;
}

/*  *********************************************************************
    *  exc_cleanup_block(dq_jmpbuf)
    *  
    *  Remove dq_jmpbuf from the exception handler stack and free
    *  the memory.
    *  
    *  Input parameters: 
    *         dq_jmpbuf - block to deallocate
    *         
    *  Return value:
    *         nothing
    ********************************************************************* */

void exc_cleanup_block(jmpbuf_t *dq_jmpbuf)
{
    int count;

    if (dq_jmpbuf == NULL) {
        return;
    }

    count = q_count( &(exc_handler.jmpbuf_stack));

    if( count > 0 ) {
        q_dequeue( &(*dq_jmpbuf).stack );
        KFREE(dq_jmpbuf);
    }
}

/*  *********************************************************************
    *  exc_cleanup_handler(dq_jmpbuf,chain_exc)
    *  
    *  Clean a block, then chain to the next exception if required.
    *  
    *  Input parameters: 
    *         dq_jmpbuf - current exception
    *         chain_exc - true if we should chain to the next handler
    *         
    *  Return value:
    *         nothing
    ********************************************************************* */

void exc_cleanup_handler(jmpbuf_t *dq_jmpbuf, int chain_exc)
{
    exc_cleanup_block(dq_jmpbuf);

    if( chain_exc == EXC_CHAIN_EXC ) {
        /*Go to next exception on stack */
        exc_longjmp_handler();
    }
}



/*  *********************************************************************
    *  exc_longjmp_handler()
    *  
    *  This routine long jumps to the exception handler on the top
    *  of the exception stack.
    *  
    *  Input parameters: 
    *         nothing
    *         
    *  Return value:
    *         nothing
    ********************************************************************* */
void exc_longjmp_handler(void)
{
    int count;   
    jmpbuf_t *jmpbuf_local;

    count = q_count( &(exc_handler.jmpbuf_stack));

    if( count > 0 ) {
        jmpbuf_local = (jmpbuf_t *) q_getlast(&(exc_handler.jmpbuf_stack));

        SETLEDS("CFE ");

        lib_longjmp( (*jmpbuf_local).jmpbuf, -1);
    }
}


/*  *********************************************************************
    *  mem_peek(d,addr,type)
    *  
    *  Read memory of the specified type at the specified address.
    *  Exceptions are caught in the case of a bad memory reference.
    *  
    *  Input parameters: 
    *         d - pointer to where data should be placed
    *         addr - address to read
    *         type - type of read to do (MEM_BYTE, etc.)
    *         
    *  Return value:
    *         0 if ok
    *         else error code
    ********************************************************************* */

int mem_peek(void *d, long addr, int type)
{

    jmpbuf_t *jb;

    jb = exc_initialize_block();
    if( jb == NULL ) {
        return CFE_ERR_NOMEM;
    }

    if (exc_try(jb) == 0) {
  
    switch (type) {
        case MEM_BYTE:
            *(uint8_t *)d = *((volatile uint8_t *) addr);
        break;
        case MEM_HALFWORD:
            *(uint16_t *)d = *((volatile uint16_t *) addr);
        break;
        case MEM_WORD:
            *(uint32_t *)d = *((volatile uint32_t *) addr);
        break;
        case MEM_QUADWORD:
            *(uint64_t *)d = *((volatile uint64_t *) addr);
        break;
        default:
            return CFE_ERR_INV_PARAM;
        }

        exc_cleanup_block(jb);
    }
    else {
        /*Exception handler*/

        exc_cleanup_handler(jb, EXC_NORMAL_RETURN);
        return CFE_ERR_GETMEM;
    }

    return 0;
}

/* *********************************************************************
   *  Write memory of type at address addr with value val. 
   *  Exceptions are caught, handled (error message) and function 
   *  returns with 0. 
   *
   *  1 success
   *  0 failure
   ********************************************************************* */

int mem_poke(long addr, uint64_t val, int type)
{

    jmpbuf_t *jb;

    jb = exc_initialize_block();
    if( jb == NULL ) {
        return CFE_ERR_NOMEM;
    }

    if (exc_try(jb) == 0) {
  
    switch (type) {
        case MEM_BYTE:
            *((volatile uint8_t *) addr) = (uint8_t) val;
        break;
        case MEM_HALFWORD:
            *((volatile uint16_t *) addr) = (uint16_t) val;
        break;
        case MEM_WORD:
            *((volatile uint32_t *) addr) = (uint32_t) val;
        break;
        case MEM_QUADWORD:
            *((volatile uint64_t *) addr) = (uint64_t) val;
        break;
        default:
            return CFE_ERR_INV_PARAM;
        }

        exc_cleanup_block(jb);
    }
    else {
        /*Exception handler*/
        exc_cleanup_handler(jb, EXC_NORMAL_RETURN);
        return CFE_ERR_SETMEM;
    }

    return 0;
}
