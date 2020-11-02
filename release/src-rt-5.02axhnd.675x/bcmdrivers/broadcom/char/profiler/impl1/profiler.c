/***************************************************************************
*    Copyright 2004  Broadcom Corporation
*    All Rights Reserved
*    No portions of this material may be reproduced in any form without the
*    written permission of:
*             Broadcom Corporation
*             16215 Alton Parkway
*             P.O. Box 57013
*             Irvine, California 92619-7013
*    All information contained in this document is Broadcom Corporation
*    company private, proprietary, and trade secret.
*
****************************************************************************
*
*    Filename: profiler_tool.c
*
****************************************************************************
*    Description:
*
*      Implementation of the profiler tool utility.
*
****************************************************************************/

#include <linux/module.h>
#include <linux/string.h>
#include <linux/param.h>
#include <asm/cpu.h>
#include <bcm_map_part.h>
#include <board.h>
#include <bcm_intr.h>

#include "profdrv.h"
#include "profiler.h"

extern unsigned long volatile jiffies;

/*
    Uncomment the following define for profiler debug information
*/
// #define PROFILER_DEBUG

static PROFILER_COLLECTED_DATA profiler_data_pool[PROFILER_MAX_MONITORED_PROFILE];
static PROFILER_RECSEQ_DATA profiler_recseq_data_pool[PROFILER_MAX_RECSEQ];
static PROFILER_CPU_UTILIZATION profiler_cpu_util;
static unsigned profiler_start_operation = 0;
static unsigned profiler_start_collect_jiffies_count = 0;
static unsigned profiler_stop_collect_jiffies_count = 0;
static unsigned profiler_cpu_clock = 1;
static unsigned profiler_auto_register;
static unsigned int profiler_recseq_data_index = 0;

/***************************************************************************
// Function Name: profiler_get_cycles
// Description  : Gets the cycle count from register 9 in CP0 (CP0_COUNT).
// Parameters   : None.
// Returns      : The number of elapsed CPU cycles.
****************************************************************************/
static inline unsigned int profiler_get_cycles( void )
{
    return profdrv_read_32bit_cp0_register( $9 );
}

/***************************************************************************
// Function Name: profiler_get_cpu_speed
// Description  : Gets the current CPU speed.
// Parameters   : None.
// Returns      : The CPU speed.
// Note         : This is extracted from 
//                  linux\arch\mips\brcm-boards\generic\prom.c
****************************************************************************/
static inline unsigned profiler_get_cpu_speed( void )
{
    unsigned cpu_speed;
#error The variable cpu_speed is not set.
    return cpu_speed;       
}

/***************************************************************************
// Function Name: profiler_init
// Description  : Initializes the profiler tool data.
// Parameters   : None.
// Returns      : None.
****************************************************************************/
void profiler_init( void )
{
    memset( profiler_data_pool, 0, sizeof( profiler_data_pool ));
    memset( &profiler_cpu_util, 0, sizeof( profiler_cpu_util ));
    profiler_start_operation = 0;
	/*
		Set the auto-register flag to 1 by default for now.  This means that
		each function to be monitored will be automatically registered on its
		first call.  User does not need to register the function per say.
	*/
	profiler_auto_register = 1;
}

/***************************************************************************
// Function Name: profiler_get_data_dump
// Description  : Gets the pointer to the start of the data information.
// Parameters   : None.
// Returns      : The pointer to the start of the data.
****************************************************************************/
PROFILER_COLLECTED_DATA *profiler_get_data_dump( void )
{
	return &profiler_data_pool[0];
}

/***************************************************************************
// Function Name: profiler_get_recseq_data_dump
// Description  : Gets the pointer to the start of the recorded sequence information.
// Parameters   : None.
// Returns      : The pointer to the start of the data.
****************************************************************************/
PROFILER_RECSEQ_DATA* profiler_get_recseq_data_dump( void )
{
	return &profiler_recseq_data_pool[0];
}

/***************************************************************************
// Function Name: profiler_get_cpu_util
// Description  : Gets the CPU utilization saved information.
// Parameters   : pData - pointer to store the data to.
// Returns      : None.
****************************************************************************/
void profiler_get_cpu_util( PROFILER_CPU_UTILIZATION *pData )
{
	memcpy( pData, &profiler_cpu_util, sizeof(PROFILER_CPU_UTILIZATION)); 
}

/***************************************************************************
// Function Name: profiler_set_cpu_util
// Description  : Sets the CPU utilization collected information.
// Parameters   : pData - pointer to the data to save.
// Returns      : None.
****************************************************************************/
void profiler_set_cpu_util( PROFILER_CPU_UTILIZATION *pData )
{
	if( pData->valid_data == PROFILER_CPU_UTIL_VALID_START )
	{
		profiler_cpu_util.tick_uptime_start = pData->tick_uptime_start;
		profiler_cpu_util.tick_idle_start = pData->tick_idle_start;
		profiler_cpu_util.tick_user_start = pData->tick_user_start;
		profiler_cpu_util.tick_kernel_start = pData->tick_kernel_start;
	}
	else if( pData->valid_data == PROFILER_CPU_UTIL_VALID_STOP )
	{
		profiler_cpu_util.tick_uptime_stop = pData->tick_uptime_stop;
		profiler_cpu_util.tick_idle_stop = pData->tick_idle_stop;
		profiler_cpu_util.tick_user_stop = pData->tick_user_stop;
		profiler_cpu_util.tick_kernel_stop = pData->tick_kernel_stop;
	}

/*
	printk( "\n[Knl] CPU util - start - %d(idl) %d(usr) %d(knl) %d(up)\n",
		profiler_cpu_util.tick_idle_start, profiler_cpu_util.tick_user_start,
		profiler_cpu_util.tick_kernel_start, profiler_cpu_util.tick_uptime_start );
	printk( "\n[Knl] CPU util - stop - %d(idl) %d(usr) %d(knl) %d(up)\n",
		profiler_cpu_util.tick_idle_stop, profiler_cpu_util.tick_user_stop,
		profiler_cpu_util.tick_kernel_stop,	profiler_cpu_util.tick_uptime_stop );
*/
}

/***************************************************************************
// Function Name: profiler_get_status
// Description  : Gets the profiler status information.
// Parameters   : pStatus - pointer to the data to be kept in here.
// Returns      : None.
****************************************************************************/
void profiler_get_status( PROFILER_STATUS *pStatus )
{
	pStatus->status = profiler_start_operation;
	pStatus->cpu_jiffies_start = profiler_start_collect_jiffies_count;
	pStatus->cpu_jiffies_stop = profiler_stop_collect_jiffies_count;
	pStatus->cpu_clock = profiler_cpu_clock;
	pStatus->cpu_jiffies_factor = HZ;

/*
	printk( "\n[Knl] Status - %u(st) %u(sta) %u(sto) %u(clk) %u(fac)\n",
		pStatus->status, pStatus->cpu_jiffies_start, 
		pStatus->cpu_jiffies_stop, pStatus->cpu_clock,
		pStatus->cpu_jiffies_factor );
*/
}
 
/***************************************************************************
// Function Name: profiler_get_recseq_data_index
// Description  : Gets the index for the recorded sequence data table
// Parameters   : None
// Returns      : Index to the recorded sequence data table
****************************************************************************/
unsigned int profiler_get_recseq_data_index( void )
{
   return( profiler_recseq_data_index );
}
 
/***************************************************************************
// Function Name: kernel_profiler_register
// Description  : Registers a function to be profiled with the profiler.
// Parameters   : pName - The name of the function to register. Note that only the
//                first 32 characters are used and those should be distinct
//                from any existing one already registered.
//				  src - the source of this call (user or kernel)
// Returns      : None.
****************************************************************************/
void kernel_profiler_register( char *pName, unsigned char src )
{
    int index, free_index = -1;

#   ifdef PROFILER_DEBUG
    printk("[kernel_profiler_register]=> %s", pName );
#   endif /* PROFILER_DEBUG */

    for( index = 0 ; index < PROFILER_MAX_MONITORED_PROFILE ; index++ )
    {
        if( profiler_data_pool[index].flag != PROFILER_FLAG_RESOURCE_FREE )
        {
            if( strncmp( pName, profiler_data_pool[index].name, 
                         PROFILER_NAME_MAX_LENGTH ) == 0 )
            {
		        /*
					Do not display an error if we are in automatic registration mode as this
					check will be perfomed on each start call.
				*/
		        if( profiler_auto_register == 0 )
				{
		        	printk( "\n[profiler_register]=> %s - already registered!", pName );
				}
				return;
            }
        }
        else if( free_index == -1 )
        {
            free_index = index;
        }
    }

    if( free_index != -1 )
    {
        profiler_data_pool[free_index].source = src;
        profiler_data_pool[free_index].flag = PROFILER_FLAG_RESOURCE_ALLOCATED;
        strncpy( profiler_data_pool[free_index].name, pName, PROFILER_NAME_MAX_LENGTH );
        profiler_data_pool[free_index].min_cycle = 0;
        profiler_data_pool[free_index].max_cycle = 0;
        profiler_data_pool[free_index].avg_cycle = 0;
        profiler_data_pool[free_index].count = 0;
        return;
    }
    else
    {
        printk( "\n[profiler_register]=> %s - cannot register!", pName );
        return;
    } 
    return;
}

/***************************************************************************
// Function Name: kernel_profiler_deregister
// Description  : Deregisters a function to be profiled from the profiler.
//                The data collected for this function will be lost.
// Parameters   : pName - The name of the function to deregister. This should 
//				  match the name of the registered function.
//				  src - the source of this call (user or kernel)
// Returns      : None.
****************************************************************************/
void kernel_profiler_deregister( char *pName, unsigned char src )
{
    int index;

    for( index = 0 ; index < PROFILER_MAX_MONITORED_PROFILE ; index++ )
    {
        if( profiler_data_pool[index].flag != PROFILER_FLAG_RESOURCE_FREE )
        {
            if( strncmp( pName, profiler_data_pool[index].name, 
                         PROFILER_NAME_MAX_LENGTH ) == 0 )
            {
                profiler_data_pool[index].flag = PROFILER_FLAG_RESOURCE_FREE;
                return;       
            }
        }
    }
    return;
}

/***************************************************************************
// Function Name: kernel_profiler_start
// Description  : Entry point for the collection for this function.
// Parameters   : pName - The name of the function to profile. This should 
//				  match the name of the registered function.
//				  src - the source of this call (user or kernel)
// Returns      : None.
****************************************************************************/
void kernel_profiler_start( char *pName, unsigned char src )
{
    int index;
    unsigned int now_cycles = profiler_get_cycles();

#   ifdef PROFILER_DEBUG
    printk("[kernel_profiler_start]=> %s", pName );
#   endif /* PROFILER_DEBUG */

    /* 
    	Just to make people's life easier, for now, let's register on the 
       	first start call... 
    */
	if( profiler_auto_register == 1 )
	{
    	kernel_profiler_register( pName, src );
	}

    for( index = 0 ; index < PROFILER_MAX_MONITORED_PROFILE ; index++ )
    {
        if( profiler_data_pool[index].flag != PROFILER_FLAG_RESOURCE_FREE )
        {
            if( strncmp( pName, profiler_data_pool[index].name, 
                         PROFILER_NAME_MAX_LENGTH ) == 0 )
            {
                /*
                    Only collect data if the collection is enabled.  If not, mark the
                    control block so we know not to mess up the data collected.
                */
                if( profiler_start_operation > 0 )
                {
                    profiler_data_pool[index].flag = PROFILER_FLAG_RESOURCE_ALLOCATED;  
                    profiler_data_pool[index].now_cycle = now_cycles;
                }
                else
                {
                    profiler_data_pool[index].flag |= PROFILER_FLAG_RESOURCE_COLLECT_PENDING;
                }
                return;       
            }
        }
    }

	/*
		If we are here, we were not successful in profiling this function.  Warn the user.
	*/
    printk("[kernel_profiler_start]=> %s unregistered function...", pName );
    return;
}

/***************************************************************************
// Function Name: kernel_profiler_stop
// Description  : Exit point for the collection for this function.  Stats
//                for this function are calculated and accumulated at this
//                point.
// Parameters   : pName - The name of the function to profile. This should 
//				  match the name of the registered function.
//				  src - the source of this call (user or kernel)
// Returns      : None.
****************************************************************************/
void kernel_profiler_stop( char *pName, unsigned char src )
{
    int index;
    unsigned int now_cycles = profiler_get_cycles();

#   ifdef PROFILER_DEBUG
    printk("[kernel_profiler_stop]=> %s", pName );
#   endif /* PROFILER_DEBUG */

    /*
        Only collect data if the collection is enabled.
    */
    if( profiler_start_operation == 0 )
    {
        return;
    }

    for( index = 0 ; index < PROFILER_MAX_MONITORED_PROFILE ; index++ )
    {
        if( profiler_data_pool[index].flag != PROFILER_FLAG_RESOURCE_FREE )
        {
            if( strncmp( pName, profiler_data_pool[index].name, 
                         PROFILER_NAME_MAX_LENGTH ) == 0 )
            {
                if((profiler_data_pool[index].flag & PROFILER_FLAG_RESOURCE_COLLECT_PENDING) ==
                    PROFILER_FLAG_RESOURCE_COLLECT_PENDING )
                {
                    /* 
                        This means that the data collection has been asynchronously enabled, but 
                        we do not have valid data to evaluate this expression, wait for the next
                        start operation to clean up the flag and allow the collection to proceed.
                    */
                    return;
                }

                if( now_cycles >= profiler_data_pool[index].now_cycle )
                {
                    unsigned int diff_cycles = now_cycles - profiler_data_pool[index].now_cycle;
                    if( profiler_data_pool[index].min_cycle > diff_cycles ||
                        /* If this is the initial call into this monitoring then count is zero and min
                           needs to be set */
                        (profiler_data_pool[index].count == 0))
                    {
                        profiler_data_pool[index].min_cycle = diff_cycles;
                    }
                    if( profiler_data_pool[index].max_cycle < diff_cycles )
                    {
                        profiler_data_pool[index].max_cycle = diff_cycles;
                    }
                    profiler_data_pool[index].avg_cycle = 
                        (profiler_data_pool[index].avg_cycle * profiler_data_pool[index].count + diff_cycles) / 
                        ++profiler_data_pool[index].count;
#   ifdef PROFILER_DEBUG
                    printk( "\n[profiler_stop]=> Loging-stop %s (%d)...", pName, now_cycles );
#   endif /* PROFILER_DEBUG */
                }
                else
                {
                    profiler_data_pool[index].flag |= PROFILER_FLAG_RESOURCE_ERROR; 
                }
                return;       
            }
        }
    }
    return;
}

/***************************************************************************
// Function Name: kernel_profiler_reinit_collected
// Description  : Re-initializes the collected data.  Data collected prior
//                to this call are lost.
// Parameters   : None.
// Returns      : None.
****************************************************************************/
void kernel_profiler_reinit_collected()
{
    int index;

    for( index = 0 ; index < PROFILER_MAX_MONITORED_PROFILE ; index++ )
    {
        if( profiler_data_pool[index].flag != PROFILER_FLAG_RESOURCE_FREE )
        {
            /*
                Note: do not clear the now_cycle data member information so we
                can recover properly if this happens in between a start/stop call
                to this profiler function.
            */
            if((profiler_data_pool[index].flag & PROFILER_FLAG_RESOURCE_COLLECT_PENDING) ==
                PROFILER_FLAG_RESOURCE_COLLECT_PENDING )
            {
                profiler_data_pool[index].flag = 
                    (PROFILER_FLAG_RESOURCE_ALLOCATED | PROFILER_FLAG_RESOURCE_COLLECT_PENDING);
            }
            else
            {
                profiler_data_pool[index].flag = PROFILER_FLAG_RESOURCE_ALLOCATED;
            }
            profiler_data_pool[index].min_cycle = 0;
            profiler_data_pool[index].max_cycle = 0;
            profiler_data_pool[index].avg_cycle = 0;
            profiler_data_pool[index].count = 0;
        }
    }
}

/***************************************************************************
// Function Name: kernel_profiler_start_collect
// Description  : Starts the collection of data.
// Parameters   : None.
// Returns      : None.
****************************************************************************/
void kernel_profiler_start_collect( void )
{
   memset( profiler_recseq_data_pool, 0, sizeof(profiler_recseq_data_pool) );
   profiler_recseq_data_index = 0;

    if( profiler_start_operation == 0 )
    { 
        profiler_start_operation = 1;
	    /* 
	        Get the CPU clock information (needed when data is converted into
	        human friendly information). 
	    */
	    profiler_cpu_clock = profiler_get_cpu_speed();
        /*
            Re-init the collected data to ensure accuracy.
        */
        kernel_profiler_reinit_collected();
        profiler_start_collect_jiffies_count = jiffies;
//        printk( "\n[profiler_start_collect]" ); 
    }
}

/***************************************************************************
// Function Name: kernel_profiler_stop_collect
// Description  : Stops the collection of data.
// Parameters   : None.
// Returns      : None.
****************************************************************************/
void kernel_profiler_stop_collect( void )
{
    if( profiler_start_operation > 0 )
    {
        profiler_start_operation = 0;
        /*
            Get the system idle time.
        */
        profiler_stop_collect_jiffies_count = jiffies;
//        printk( "\n[profiler_stop_collect]" ); 
    }
}

/***************************************************************************
// Function Name: kernel_profiler_dump
// Description  : Do nothing.  Currently, this can only be called from the 
//				  user space (via profiler CLI)
// Parameters   : None.
// Returns      : None.
****************************************************************************/
void kernel_profiler_dump()
{
	return;
}

/***************************************************************************
// Function Name: kernel_profiler_recseq_dump
// Description  : Do nothing.  Currently, this can only be called from the 
//				  user space (via profiler CLI)
// Parameters   : None.
// Returns      : None.
****************************************************************************/
void kernel_profiler_recseq_dump()
{
	return;
}

/***************************************************************************
// Function Name: kernel_profiler_recseq_start
// Description  : Entry point for the recording of sequence of start/stop times
// Parameters   : id - id of the function for which the recording is taken
//                In case of interrupts, id can be the interrupt number.
//                In case of tasklets, id can be the tasklet function address
// Returns      : None.
****************************************************************************/
void kernel_profiler_recseq_start( unsigned int id )
{
   if ( (profiler_start_operation > 0) && (id != MIPS_TIMER_INT ) ) // skip profiling the timer interrupt
   {
      if ( profiler_recseq_data_index < (PROFILER_MAX_RECSEQ-10) )
      {
         profiler_recseq_data_pool[profiler_recseq_data_index].id = id;
         profiler_recseq_data_pool[profiler_recseq_data_index].startTime = profiler_get_cycles();
      }
   }
}


/***************************************************************************
// Function Name: kernel_profiler_recseq_start
// Description  : Entry point for the recording of sequence of start/stop times
// Parameters   : id - id of the function for which the recording is taken 
//                In case of interrupts, id can be the interrupt number.
//                In case of tasklets, id can be the tasklet function address.
//                The
// Returns      : None.
****************************************************************************/
void kernel_profiler_recseq_stop( unsigned int id )
{
   if ( (profiler_start_operation == 1) && (id != MIPS_TIMER_INT) ) // skip profiling the timer interrupt
   {
      if ( id != profiler_recseq_data_pool[profiler_recseq_data_index].id )
      {
         /* stop id doesn't correspond to start id.
         ** Invalidate the record. */
         profiler_recseq_data_pool[profiler_recseq_data_index].id          = 0xFFFF;
         profiler_recseq_data_pool[profiler_recseq_data_index].startTime   = 0xFFFF;
         profiler_recseq_data_pool[profiler_recseq_data_index].endTime     = 0xFFFF;
      }
      else if ( profiler_recseq_data_index < (PROFILER_MAX_RECSEQ-10) )
      {
         profiler_recseq_data_pool[profiler_recseq_data_index].endTime = profiler_get_cycles();
         profiler_recseq_data_index++;
      }
   }
}


/*
	Export those symbols for the other modules within the kernel
	to be able to use the API's to the profiler.
*/
EXPORT_SYMBOL(kernel_profiler_register);
EXPORT_SYMBOL(kernel_profiler_deregister);
EXPORT_SYMBOL(kernel_profiler_start);
EXPORT_SYMBOL(kernel_profiler_stop);
EXPORT_SYMBOL(kernel_profiler_recseq_start);
EXPORT_SYMBOL(kernel_profiler_recseq_stop);

