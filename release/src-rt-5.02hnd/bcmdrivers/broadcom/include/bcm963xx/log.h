/***************************************************************************
*    Copyright 2000  Broadcom Corporation
*    All Rights Reserved
*    No portions of this material may be reproduced in any form without the
*    written permission of:
*             Broadcom Corporation
*             16251 Laguna Canyon Road
*             Irvine, California  92618
*    All information contained in this document is Broadcom Corporation
*    company private, proprietary, and trade secret.
*
****************************************************************************
*
*    Filename: log.h
*    Creation Date: 4 July 2000 (v0.00)
*    VSS Info:
*        $Revision: 23 $
*        $Date: 9/14/01 4:54p $
*
****************************************************************************
*    Description:
*
*     This header file contains the needed macros and function prototypes
*     for logging on the terminal.
*
****************************************************************************/

#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_DBG

#if (! (defined(__KERNEL__) || defined (LINUX)) )

extern void Log( char const *format,...);
extern void LogMsg( char *string );
extern void LogDirectSerial( char const *format,...);

#define PANIC(m)  Log(m)
#define LOG(fmt)	Log fmt
#define LOGERROR(fmt)	Log fmt
#define LOG1(fmt)	Log fmt              /* Level 1 logging  */
#define LOG2(fmt)	                     /* Level 2 logging  */
#define LOGMSG(buf)	LogMsg buf

/***********************************
** Error logging                   *
***********************************/
#elif defined(__KERNEL__)

#define LOG(fmt) printk fmt   ;  printk("\n");
#define LOG_RAW(fmt)    printk fmt  ;  printk("\n");
#define PANIC(fmt)      printk("!!! PANIC !!! \n");   printk fmt  ;  printk("\n");
#define LOGERROR(fmt)	printk("!!! ERROR !!! \n");   printk fmt  ;  printk("\n");

#elif defined(LINUX)

#include <stdio.h>

#ifdef LOG_DBG

#include <time.h>

#define LOG(fmt)                    \
{                                   \
   struct tm *tm_ptr;               \
   time_t curtime;                  \
   time( &curtime );                \
   tm_ptr = gmtime( &curtime );     \
   printf("%02d:%02d:%02d ",        \
            tm_ptr->tm_hour,        \
            tm_ptr->tm_min,         \
            tm_ptr->tm_sec);        \
   printf fmt;                      \
   printf("\n");                    \
}

#define LOGBYTES(name, pbytes, size)                  \
{                                                     \
   int i;                                             \
   printf( "%s (size=%d):\n", name, size );           \
   for ( i = 0; i < size; i++)                        \
   {                                                  \
      printf( "%02X", pbytes[i] );                    \
      printf( "%s", ((i+1)%4) ? " " : "   " );        \
      if ( ((i+1)%16)==0 ) printf( "\n" );            \
   }                                                  \
   printf( "\n\n" );                                  \
}


#else
#define LOG(fmt) printf fmt   ;  printf("\n");
#endif /* LOG_DBG */

#define PANIC(fmt)      printk("!!! PANIC !!! \n");   printk fmt  ;  printk("\n");
#define LOGERROR(fmt)                     \
{                                         \
   printf("ERROR !!! File %s (line %u): ", __FILE__, __LINE__);\
   printf fmt;                            \
   printf("\n");                          \
}

#else
#error Unknown OS
#endif

#ifdef __cplusplus
    }
#endif

#endif /* LOG_H */



