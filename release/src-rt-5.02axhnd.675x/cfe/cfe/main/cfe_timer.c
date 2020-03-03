/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  Timer routines				File: cfe_timer.c
    *  
    *  This module contains routines to keep track of the system time,.
    *  Since we don't have any interrupts in the firmware, even the
    *  timer is polled.  The timer must be called often enough
    *  to prevent missing the overflow of the CP0 COUNT
    *  register, approximately 2 billion cycles (half the count)
    * 
    *  Be sure to use the POLL() macro each time you enter a loop
    *  where you are waiting for some I/O event to occur or
    *  are waiting for time to elapse.
    *
    *  It is *not* a time-of-year clock.  The timer is only used
    *  for timing I/O events.
    *
    *  Internally, time is maintained in units of "CLOCKSPERTICK",
    *  which should be about tenths of seconds.
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


#include "lib_types.h"
#include "lib_printf.h"

#include "cfe_timer.h"

#include "cfe.h"

#include "bsp_config.h"
#include "cpu_config.h"

#ifndef CFG_CPU_SPEED
#define CFG_CPU_SPEED	500000		/* CPU speed in Hz */
#endif

#ifndef CPUCFG_CYCLESPERCPUTICK
#define CPUCFG_CYCLESPERCPUTICK 1	/* CPU clock ticks per CP0 COUNT */
#endif

/*  *********************************************************************
    *  Externs
    ********************************************************************* */

/*  *********************************************************************
    *  Data
    ********************************************************************* */

volatile int64_t cfe_ticks;		/* current system time */

int cfe_cpu_speed = CFG_CPU_SPEED;	/* CPU speed in clocks/second */

static unsigned int cfe_clocks_per_usec;
static unsigned int cfe_clocks_per_tick;

static unsigned long cfe_oldcount;		/* For keeping track of ticks */
static unsigned long cfe_remticks;
static int cfe_timer_initflg = 0;

/*
 * C0_COUNT clocks per microsecond and per tick.  Some CPUs tick CP0
 * every 'n' cycles, that's what CPUCFG_CYCLESPERCPUTICK is for.  */
#define CFE_CLOCKSPERUSEC (cfe_cpu_speed/1000000/(CPUCFG_CYCLESPERCPUTICK))
#define CFE_CLOCKSPERTICK (cfe_cpu_speed/(CFE_HZ)/(CPUCFG_CYCLESPERCPUTICK)) 


/*  *********************************************************************
    *  cfe_timer_task()
    *  
    *  This routine is called as part of normal device polling to 
    *  update the system time.   We read the CP0 COUNT register,
    *  add the delta into our current time, convert to ticks,
    *  and keep track of the COUNT register overflow
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */


static void cfe_timer_task(void *arg)
{
    unsigned long count;	
    unsigned long deltaticks;
    unsigned long clockspertick;

    clockspertick = CFE_CLOCKSPERTICK;

    count = _getticks();

    deltaticks    = (count - cfe_oldcount) / clockspertick;
    cfe_remticks += (count - cfe_oldcount) % clockspertick;

    cfe_ticks += deltaticks + (cfe_remticks / clockspertick);
    cfe_remticks %= clockspertick;


    cfe_oldcount = count;
}


/*  *********************************************************************
    *  cfe_timer_init()
    *  
    *  Initialize the timer module.
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

void cfe_timer_init(void)
{
    cfe_clocks_per_tick = CFE_CLOCKSPERTICK;
    cfe_clocks_per_usec = CFE_CLOCKSPERUSEC;
    if (cfe_clocks_per_usec == 0)
	cfe_clocks_per_usec = 1;    /* for the simulator */

    cfe_oldcount = _getticks();		/* get current COUNT register */
    cfe_ticks = 0;

    if (!cfe_timer_initflg) {
	cfe_bg_add(cfe_timer_task,NULL); /* add task for background polling */
	cfe_timer_initflg = 1;
	}
}


/*  *********************************************************************
    *  cfe_sleep(ticks)
    *  
    *  Sleep for 'ticks' ticks.  Background tasks are processed while
    *  we wait.
    *  
    *  Input parameters: 
    *  	   ticks - number of ticks to sleep (note: *not* clocks!)
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

void cfe_sleep(int ticks)
{
    int64_t timer;

    TIMER_SET(timer,ticks);
    while (!TIMER_EXPIRED(timer)) {	
	POLL();
	}
}



/*  *********************************************************************
    *  cfe_usleep(usec)
    *  
    *  Sleep for approximately the specified number of microseconds.
    *  
    *  Input parameters: 
    *  	   usec - number of microseconds to wait
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

void cfe_usleep(int usec)
{
    unsigned long newcount;
    unsigned long now;

    /* XXX fix the wrap problem */

    now = _getticks();
    newcount = now + ((unsigned long)usec)*((unsigned long)cfe_clocks_per_usec);

    if (newcount < now)  	/* wait for wraparound */
        while (_getticks() > now)
	    ;
    

    while (_getticks() < newcount)
	;
}

