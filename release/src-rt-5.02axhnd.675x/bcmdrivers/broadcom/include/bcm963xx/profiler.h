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
*    Filename: profiler.h
*
****************************************************************************
*    Description:
*
*      This file contains the API definition for usage of the profiler tool
*
****************************************************************************/
#ifndef PROFILER__H__INCLUDED
#define PROFILER__H__INCLUDED

#include "profdrv.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PROFILER_MAX_MONITORED_PROFILE          32
#define PROFILER_CPU_UTILIZATION_MIN            0.000001
#define PROFILER_MISC_STRING_LENGTH             256
#define PROFILER_CPU_TICK_FACTOR				2.0
#define PROFILER_S2MS_FACTOR					1000.0

#define PROFILER_FLAG_RESOURCE_FREE             0x00
#define PROFILER_FLAG_RESOURCE_ALLOCATED        0x01
#define PROFILER_FLAG_RESOURCE_ERROR            0x02
#define PROFILER_FLAG_RESOURCE_COLLECT_PENDING  0x04

/*
    This structure defines the data collected by the profiler.
*/
typedef struct
{
	unsigned char source;					/* User or Kernel */
    unsigned char flag;                     /* Generic flag */
    char name[PROFILER_NAME_MAX_LENGTH];    /* Name of the function monitored */
    unsigned int now_cycle;                 /* The current cycle count saved */
    unsigned int min_cycle;                 /* The minimum number of cycles calculated for this function */
    unsigned int max_cycle;                 /* The maximum number of cycles calculated for this function */
    unsigned int avg_cycle;                 /* The average numnber of cycles calculated for this function */
    unsigned int count;                     /* The number of time this function has been profiled */

} PROFILER_COLLECTED_DATA; 


/*
    This structure defines the recorded sequence data collected by the profiler.
*/
typedef struct PROFILER_RECSEQ_DATA
{
   unsigned int   id;
   unsigned long  startTime;
   unsigned long  endTime;
} PROFILER_RECSEQ_DATA;


void kernel_profiler_register( char *pName, unsigned char src );
void kernel_profiler_deregister( char *pName, unsigned char src );
void kernel_profiler_start( char *pName, unsigned char src );
void kernel_profiler_stop( char *pName, unsigned char src );
void kernel_profiler_recseq_start( unsigned int id );
void kernel_profiler_recseq_stop( unsigned int id );

void kernel_profiler_reinit_collected( void );
void kernel_profiler_dump( void );
void kernel_profiler_recseq_dump( void );
void kernel_profiler_start_collect( void );
void kernel_profiler_stop_collect( void );


void profiler_init( void );
void profiler_get_status( PROFILER_STATUS *pStatus );
PROFILER_COLLECTED_DATA *profiler_get_data_dump( void );
PROFILER_RECSEQ_DATA* profiler_get_recseq_data_dump( void );
void profiler_get_cpu_util( PROFILER_CPU_UTILIZATION *pData );
void profiler_set_cpu_util( PROFILER_CPU_UTILIZATION *pData );
unsigned int profiler_get_recseq_data_index( void );

/* 
    This is the generic API that should be used by clients to access the profiler
*/
#define PROFILER_REGISTER( name ) ( kernel_profiler_register( (name), PROFILER_SOURCE_KERNEL ) )
#define PROFILER_DEREGISTER( name ) ( kernel_profiler_deregister( (name), PROFILER_SOURCE_KERNEL ) )
#define PROFILER_START( name ) ( kernel_profiler_start( (name), PROFILER_SOURCE_KERNEL ) )
#define PROFILER_STOP( name ) ( kernel_profiler_stop( (name), PROFILER_SOURCE_KERNEL ) )
#define PROFILER_RECSEQ_START( source ) ( kernel_profiler_recseq_start( (source) ) )
#define PROFILER_RECSEQ_STOP( source ) ( kernel_profiler_recseq_stop( (source)) )

#define PROFILER_REINIT_COLLECTED() ( kernel_profiler_reinit_collected() )
#define PROFILER_DUMP() ( kernel_profiler_dump() )
#define PROFILER_RECSEQ_DUMP() ( kernel_profiler_recseq_dump() )
#define PROFILER_START_COLLECT() ( kernel_profiler_start_collect() )
#define PROFILER_STOP_COLLECT() ( kernel_profiler_stop_collect() )


#ifdef __cplusplus
}
#endif

#endif /* PROFILER__H__INCLUDED */
