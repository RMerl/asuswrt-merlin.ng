/******************************************************************************

  Copyright (c) 2014 Lantiq Deutschland GmbH
  Copyright 2016, Intel Corporation.

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/
/**
   \file os_linux.c
   This file implements the OS functions for Linux.
   used. 
   
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "os_linux.h"
#include "os_types.h"
#include "fcntl.h"
#include "unistd.h"

/* ============================= */
/* Global function definition    */
/* ============================= */

int os_open(const char *pDevName)
{
   if (pDevName == OS_NULL)
   {  
      return OS_ERROR;
   }
   return open((const char*)pDevName, O_RDWR, 0644);
}


int os_close(const int devFd)
{
   if (devFd == -1)
   {  
      return OS_ERROR;
   }
   return close(devFd);
}
   
int os_ioctl(const int devFd, 
             const unsigned int devCmd, 
             OS_uintptr_t param)
{
   if (devFd == -1)
   {  
      return OS_ERROR;
   }
   return ioctl(devFd, devCmd, param);
}   


void *os_memalloc(OS_size_t memSize_byte)
{
   void *pMemBlock = OS_NULL;

   if(memSize_byte)
   {
      pMemBlock = (void*)malloc((size_t)memSize_byte);
   }
   return (pMemBlock);
}


void os_memfree(void *pMemBlock)
{
   if (pMemBlock)
   {
      free((void*)pMemBlock);
   }
}


int os_fileload (char const     *pName, 
                 unsigned char **ppDataBuf, 
                 OS_size_t    *pBufSize_byte)
{
   int ret;
   struct stat stats;
   unsigned char *pDataBuf;
   OS_size_t     size, retVal;
   FILE *fd = OS_NULL;

   if (pName         == OS_NULL || 
       ppDataBuf     == OS_NULL ||
       pBufSize_byte == OS_NULL )
   {
      return OS_ERROR;
   }
   
   if (stat((char *)pName, &stats))
   {
      return OS_ERROR;
   }
   
   size = stats.st_size;
   if (size == 0)
   {
      return OS_ERROR;
   }
  
   fd = fopen(pName, "rb");      
   if (fd == OS_NULL)
   {
      return OS_ERROR;
   }
   
   /* size + 1 - to add null termination */
   pDataBuf = (unsigned char*)os_memalloc(size + 1);
   if (pDataBuf == OS_NULL)
   {
      ret = fclose(fd);
      return OS_ERROR;
   }
   
   retVal = fread(pDataBuf, 1, size, fd);
   if (retVal != size)
   {
      os_memfree(pDataBuf);
      ret = fclose(fd);
      return OS_ERROR;
   }

   ret = fclose(fd);

   pDataBuf[size] = '\0';
   *pBufSize_byte = (unsigned int)size;
   *ppDataBuf     = pDataBuf;

   return OS_SUCCESS;
}


int os_system (char const *pCmd) 
{
   int ret;
   if (pCmd == OS_NULL)
   {
      return OS_ERROR;
   }
   
   ret = system (pCmd);
   return OS_SUCCESS;
}




int os_fprintf (FILE *stream, 
                            const char *format, ...) 
{

   va_list ap;         /* points to each unnamed arg in turn */
   int nRet = 0;

   if (stream == OS_NULL || format == OS_NULL)
   {
      return OS_ERROR;
   }

   va_start(ap, format);   /* set ap pointer to 1st unnamed arg */
   nRet = vfprintf(stream, format, ap);   
   va_end(ap);
   
   return nRet;
}



int target_os_snprintf (char *pStrBuf, 
                             int bufSize, 
                             const char *format, ...) 
{
   va_list arg;
   int nRet = 0;

   if (pStrBuf == OS_NULL || format == OS_NULL)
   {
      return OS_ERROR;
   }

   va_start(arg, format);
   nRet = vsnprintf(pStrBuf, bufSize, format, arg);
   va_end(arg);
   
   return nRet;
}


static void nano_sleep(struct timespec *pTVal)
{
   while (1)
   {
      /* Sleep for the time specified in pTVal. If interrupted by a
      signal, place the remaining time left to sleep back into pTVal. */
      int rval = nanosleep(pTVal, pTVal);
      if (rval == 0)
      {
         /* Completed the entire sleep time; all done. */
         return;
      }
      else 
      {
         if (errno == EINTR)
            /* Interrupted by a signal. Try again. */
            continue;
         else
            /* Some other error; bail out. */
            return;
      }
   }
}


void os_mssleep(OS_time_t sleepTime_ms)
{
   struct timespec tv;
   /* Construct the timespec from the number of whole seconds... */
   tv.tv_sec = sleepTime_ms/1000;
   /* ... and the remainder in nanoseconds. */
   tv.tv_nsec = (long) ((sleepTime_ms - (tv.tv_sec * 1000)) * 1000 * 1000);
   nano_sleep(&tv);

   return;
}
