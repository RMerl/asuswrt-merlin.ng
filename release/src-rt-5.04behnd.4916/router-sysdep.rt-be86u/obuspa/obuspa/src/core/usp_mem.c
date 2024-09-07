/*
 *
 * Copyright (C) 2019-2023, Broadband Forum
 * Copyright (C) 2016-2023  CommScope, Inc
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file usp_mem.c
 *
 * Wrapper functions for allocating and deallocating memory on USP Agent
 * The allocation functions terminate (logging an error message), if out of memory
 * It is the intention that all allocation/deallocation in USP Agent goes through functions in this file
 * This will allow us to debug memory leaks etc more easily
 */

#include <stdlib.h>
#include <string.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <protobuf-c/protobuf-c.h>
#include <pthread.h>
#include <unistd.h>
#include <dlfcn.h>

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#include "common_defs.h"
#include "cli.h"
#include "os_utils.h"
#include "sync_timer.h"

//------------------------------------------------------------------------------------
// Mutex used to protect access to the USP memory info sub-system.
static pthread_mutex_t mem_access_mutex;

//------------------------------------------------------------------------------------
// This function named in this string is called before and after memory collection is started
static char *sync_timer_add_str = "SYNC_TIMER_Add";

//------------------------------------------------------------------------------------
// Debug structure: used to track memory allocations and tracking down memory leaks
typedef struct
{
    void *ptr;
    const char *func;
    int line;
    int size;
    unsigned flags;
    char *callers[16];
} minfo_t;

// Defines for flags variable of minfo[]
#define MI_MODIFIED 0x00000001

//------------------------------------------------------------------------------------
// Variables associated with collecting memory info for debugging purposes
bool collect_memory_info = false;
bool print_leak_report = false;
unsigned baseline_memory_usage = 0;           // 0 indicates that the memory usage could not be obtained

static minfo_t *minfo = NULL;
//------------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void *Protobuf_Alloc(void *allocator_data, size_t size);
void Protobuf_Free(void *allocator_data, void *pointer);
minfo_t *FindFreeMemInfo(void);
minfo_t *FindMemInfoByPtr(void *ptr);
void PrintMemInfoEntry(minfo_t *mi, char *str, int index);
void GetCallers(char **callers, int num_callers);
unsigned GetMemUsage(void);

//------------------------------------------------------------------------------------
// Structure defining functions used to allocate and free memory associated with protocol buffers
// These functions terminate if an error occurs
static ProtobufCAllocator protobuf_allocator =
{
    Protobuf_Alloc,
    Protobuf_Free,
    NULL   // Opaque pointer passed to above 2 functions. Currently unused by those functions.
};

// Pointer to protobuf allocator which is externally visible
void *pbuf_allocator = (void *)&protobuf_allocator;

/*********************************************************************//**
**
** Protobuf_Alloc
**
** Allocates memory used when unpacking a protocol buffer message
** This function will terminate USP Agent, if out of memory
**
** \param   allocator_data - (UNUSED) opaque pointer passed into this function (defined in protobuf_allocator)
** \param   size - number of bytes to allocate
**
** \return  pointer to dynamically alloacted buffer
**
**************************************************************************/
void *Protobuf_Alloc(void *allocator_data, size_t size)
{
    void *ptr;

    ptr = USP_MALLOC(size);

    return ptr;
}

/*********************************************************************//**
**
** Protobuf_Free
**
** Wrapper function around free() to use it with Protocol buffers
**
** \param   allocator_data - (UNUSED) opaque pointer passed into this function (defined in protobuf_allocator)
** \param   pointer - pointer to dynamically allocated buffer to free
**
** \return  None
**
**************************************************************************/
void Protobuf_Free(void *allocator_data, void *pointer)
{
    USP_FREE(pointer);
}

/*********************************************************************//**
**
** USP_MEM_Init
**
** Initialise the wrapper around memory allocation/deallocation
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_MEM_Init(void)
{
    int err;

    // Initialise global varaiables
    minfo = NULL;
    collect_memory_info = false;

    // Exit if unable to create mutex protecting access to this subsystem
    err = OS_UTILS_InitMutex(&mem_access_mutex);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_MEM_Destroy
**
** Frees all memory used by this component
**
** \param   None
**
** \return  None
**
**************************************************************************/
void USP_MEM_Destroy(void)
{
    // Free the memory used by the leak detector
    if (minfo != NULL)
    {
        free(minfo);
    }
}

/*********************************************************************//**
**
** USP_MEM_Malloc
**
** Wrapper around malloc() that terminates with error message if it failed
**
** \param   func - name of caller
** \param   line - line number of caller
** \param   size - number of bytes to allocate
**
** \return  None
**
**************************************************************************/
void *USP_MEM_Malloc(const char *func, int line, int size)
{
    void *ptr;
    minfo_t *mi;

    // Terminate if out of memory
    ptr = malloc(size);
    if (ptr == NULL)
    {
        USP_ERR_Terminate("%s (%d): malloc(%d bytes) failed", func, line, size);
    }

    // Collect memory info, if enabled
    if (collect_memory_info)
    {
        OS_UTILS_LockMutex(&mem_access_mutex);
        // Uncomment the definition in the line below to get debug logging of memory API functions
        #define tr_mem(...) //USP_LOG_Info(__VA_ARGS__)
        tr_mem("%s(%d): malloc(%d) = %p", func, line, size, ptr);
        mi = FindFreeMemInfo();
        if (mi != NULL)
        {
            mi->ptr = ptr;
            mi->func = func;
            mi->line = line;
            mi->size = size;
            mi->flags |= MI_MODIFIED;
            GetCallers(mi->callers, NUM_ELEM(mi->callers));
        }
        else
        {
            USP_ERR_Terminate("Need to increase MAX_MINFO_ENTRIES");
        }
        OS_UTILS_UnlockMutex(&mem_access_mutex);
    }

    return ptr;
}

/*********************************************************************//**
**
** USP_MEM_Free
**
** Wrapper around free()
**
** \param   func - name of caller
** \param   line - line number of caller
** \param   ptr - pointer to dynamically allocated memory buffer to free
**
** \return  None
**
**************************************************************************/
void USP_MEM_Free(const char *func, int line, void *ptr)
{
    minfo_t *mi;

    // Collect memory info, if enabled
    if (collect_memory_info)
    {
        OS_UTILS_LockMutex(&mem_access_mutex);
        tr_mem("%s(%d): free(%p)", func, line, ptr);
        mi = FindMemInfoByPtr(ptr);
        if (mi != NULL)
        {
            memset(mi, 0, sizeof(minfo_t));
        }
        else
        {
            USP_ERR_Terminate("Trying to free memory that was not allocated");
        }
        OS_UTILS_UnlockMutex(&mem_access_mutex);
    }

    // Free the memory
    free(ptr);
}

/*********************************************************************//**
**
** USP_MEM_Realloc
**
** Wrapper around realloc() that terminates with error message if it failed
**
** \param   func - name of caller
** \param   line - line number of caller
** \param   ptr - pointer to current buffer than needs reallocating
** \param   size - number of bytes to reallocate
**
** \return  None
**
**************************************************************************/
void *USP_MEM_Realloc(const char *func, int line, void *ptr, int size)
{
    minfo_t *mi = NULL;;
    void *new_ptr;

    // Terminate if unable to find memory info (if enabled)
    if (collect_memory_info)
    {
        OS_UTILS_LockMutex(&mem_access_mutex);
        tr_mem("%s(%d): realloc(%p, %d) = %p", func, line, ptr, size, new_ptr);

        mi = FindMemInfoByPtr(ptr);
        if (mi == NULL)
        {
            USP_ERR_Terminate("Trying to reallocate memory that was not allocated");
        }
    }

    // Terminate if out of memory
    new_ptr = realloc(ptr, size);
    if (new_ptr == NULL)
    {
        USP_ERR_Terminate("%s (%d): realloc(%d bytes) failed", func, line, size);
    }

    // Update memory info, if enabled
    if ((collect_memory_info) && (mi != NULL))
    {
        mi->ptr = new_ptr;
        mi->func = func;
        mi->line = line;
        mi->size = size;
        mi->flags |= MI_MODIFIED;
        GetCallers(mi->callers, NUM_ELEM(mi->callers));
        OS_UTILS_UnlockMutex(&mem_access_mutex);
    }

    return new_ptr;
}

/*********************************************************************//**
**
** USP_MEM_Strdup
**
** Wrapper around strdup() that terminates with error message if it failed
** NOTE: This function treats a NULL input string, as a NULL output
**
** \param   func - name of caller
** \param   line - line number of caller
** \param   ptr - pointer to buffer containing string to copy
**
** \return  None
**
**************************************************************************/
void *USP_MEM_Strdup(const char *func, int line, void *ptr)
{
    void *new_ptr;
    int size;
    minfo_t *mi;

    // Exit if nothing to copy
    if (ptr == NULL)
    {
        return NULL;
    }

    // Terminate if out of memory
    new_ptr = strdup(ptr);
    if (new_ptr == NULL)
    {
        USP_ERR_Terminate("%s (%d): strdup(%d bytes) failed", func, line, (int)strlen(ptr)+1);
    }


    // Collect memory info, if enabled
    if (collect_memory_info)
    {
        OS_UTILS_LockMutex(&mem_access_mutex);
        size = strlen(ptr) + 1;
        tr_mem("%s(%d): strdup(%d) = %p", func, line, size, new_ptr);
        mi = FindFreeMemInfo();
        if (mi != NULL)
        {
            mi->ptr = new_ptr;
            mi->func = func;
            mi->line = line;
            mi->size = size;
            mi->flags |= MI_MODIFIED;
            GetCallers(mi->callers, NUM_ELEM(mi->callers));
        }
        else
        {
            USP_ERR_Terminate("Need to increase MAX_MINFO_ENTRIES");
        }
        OS_UTILS_UnlockMutex(&mem_access_mutex);
    }

    return new_ptr;
}

/*********************************************************************//**
**
** USP_MEM_StartCollection
**
** Starts collection of memory info, for the purpose of debugging memory leaks
**
** \param   None
**
** \return  None
**
**************************************************************************/
void USP_MEM_StartCollection(void)
{
    minfo_t *mi;

    OS_UTILS_LockMutex(&mem_access_mutex);

    #define MAX_MINFO_ENTRIES 10000     // NOTE: Performing a get on 'Device.' can use a lot of entries
    #define MINFO_SIZE (MAX_MINFO_ENTRIES*sizeof(minfo_t))
    minfo = malloc(MINFO_SIZE);
    USP_ASSERT(minfo != NULL);
    memset(minfo, 0, MINFO_SIZE);

    // From now on, all current allocations will be logged in the minfo array
    collect_memory_info = true;
    print_leak_report = true;

    // Store initial static memory usage after data model has been registered
    baseline_memory_usage = GetMemUsage();
    if (baseline_memory_usage != 0)
    {
        USP_LOG_Info("Baseline Memory usage: %u", (unsigned) baseline_memory_usage);
    }

    // The sync timer vector is reallocated by BulkDataCollection after collection has been started,
    // so needs to be in the meminfo array (otherwise we assert that a realloc has occured before an alloc)
    mi = FindFreeMemInfo();
    USP_ASSERT(mi != NULL);
    mi->ptr = SYNC_TIMER_PRIV_GetVector(&mi->size);
    mi->func = sync_timer_add_str;

    OS_UTILS_UnlockMutex(&mem_access_mutex);
}

/*********************************************************************//**
**
** USP_MEM_StopCollection
**
** Stops collection of memory info.
** This also allows us to free memory that was allocated before mem info collection was started
**
** \param   None
**
** \return  None
**
**************************************************************************/
void USP_MEM_StopCollection(void)
{
    collect_memory_info = false;
}

/*********************************************************************//**
**
** USP_MEM_PrintSummary
**
** Prints out total memory used by the application
**
** \param   None
**
** \return  None
**
**************************************************************************/
void USP_MEM_PrintSummary(void)
{
    unsigned mem_use;

    // Log memory usage (if available)
    mem_use = GetMemUsage();
    if (mem_use != 0)
    {
        USP_LOG_Info("Memory in use: %u", mem_use);
    }
    else
    {
        USP_LOG_Warning("WARNING: Unable to log memory in use. mallinfo/mallinfo2() not present");
    }
}

/*********************************************************************//**
**
** USP_MEM_Print
**
** Print out memory allocations since last time this was called
**
** \param   None
**
** \return  None
**
**************************************************************************/
void USP_MEM_Print(void)
{
    int i;
    minfo_t *mi;
    int count = 0;
    static unsigned last_memory_usage = 0;
    unsigned cur_memory_usage;

    // Exit if not collecting memory info
    if (collect_memory_info==false)
    {
        return;
    }

    // Exit if no change in memory usage since last time called (if memory usage available)
    cur_memory_usage = GetMemUsage();
    if (cur_memory_usage != 0)
    {
        if (cur_memory_usage == last_memory_usage)
        {
            USP_LOG_Info("No change in memory usage.\nMemory in use: %u (%s line %d)", cur_memory_usage, __FUNCTION__, __LINE__);
            return;
        }

        USP_LOG_Info("Memory usage changed to: %u (%s line %d)", cur_memory_usage, __FUNCTION__, __LINE__);
        last_memory_usage = cur_memory_usage;
    }

    // Iterate over the memory info array, printing out all entries which have changed since last time this function was called
    OS_UTILS_LockMutex(&mem_access_mutex);
    for (i=0; i<MAX_MINFO_ENTRIES; i++)
    {
        mi = &minfo[i];
        if (mi->ptr != NULL)
        {
            count++;
            if (mi->flags & MI_MODIFIED)
            {
                // Print out entries which have changed since the last time this function was called
                PrintMemInfoEntry(mi, "NEW", i);
                mi->flags &= ~MI_MODIFIED;
            }
        }
    }

    USP_LOG_Info("%d memory allocations", count);
    OS_UTILS_UnlockMutex(&mem_access_mutex);
}

/*********************************************************************//**
**
** USP_MEM_PrintLeakReport
**
** Called when gracefully shutting down USP Agent in the automated tests
** to print out all USP Agent core memory leaks detected
**
** \param   None
**
** \return  Count of memory leaks
**
**************************************************************************/
int USP_MEM_PrintLeakReport(void)
{
    int count = 0;

    // Exit if not collecting memory info
    if (print_leak_report==false)
    {
        return count;
    }

    // Print a memory leak report
    USP_LOG_Info("START: Memory leak report\n");
    count = USP_MEM_PrintAll();
    if (count > 0)
    {
        USP_LOG_Info("DETECTED_MEMORY_LEAK in this automated test\n");
    }
    USP_LOG_Info("STOP: Memory leak report\n");

    return count;
}

/*********************************************************************//**
**
** USP_MEM_PrintAll
**
** Print out all memory allocations
**
** \param   None
**
** \return  Count of number of memory blocks still allocated
**
**************************************************************************/
int USP_MEM_PrintAll(void)
{
    int i;
    minfo_t *mi;
    int count = 0;
    unsigned mem_use;

    // Log memory usage (if available)
    mem_use = GetMemUsage();
    if (mem_use != 0)
    {
        USP_LOG_Info("Memory in use: %u (%s line %d)", mem_use, __FUNCTION__, __LINE__);
        USP_LOG_Info("Baseline Memory usage: %u", baseline_memory_usage);
    }

    // Exit if not collecting memory info
    if (print_leak_report==false)
    {
        return 0;
    }

    // Iterate over the memory info array, printing out all entries
    // NOTE: The sync timer vector is deallocated after the leak report has been printed, so to avoid it erroneously reporting as a memory leak, we ignore it here
    OS_UTILS_LockMutex(&mem_access_mutex);
    for (i=0; i<MAX_MINFO_ENTRIES; i++)
    {
        mi = &minfo[i];
        if ((mi->ptr != NULL) && (strcmp(mi->func, sync_timer_add_str) != 0))
        {
            count++;

            // Print out all entries
            PrintMemInfoEntry(mi, "", i);
            mi->flags &= ~MI_MODIFIED;
        }
    }

    USP_LOG_Info("%d memory allocations", count);
    OS_UTILS_UnlockMutex(&mem_access_mutex);

    return count;
}


/*********************************************************************//**
**
** PrintMemInfoEntry
**
** Prints an individual meminfo entry
**
** \param   mi - pointer to memory info entry to print out
** \param   str - string to prefix the debug printout
** \param   index - index of the entry in the minfo[] array
**
** \return  None
**
**************************************************************************/
void PrintMemInfoEntry(minfo_t *mi, char *str, int index)
{
    int i;
    char *func;

    USP_LOG_Info("%s [%d] %p size=%d %s (line number:%d)", str, index, mi->ptr, mi->size, mi->func, mi->line);
    for (i=0; i<NUM_ELEM(mi->callers); i++)
    {
        func = mi->callers[i];
        if (func != NULL)
        {
            USP_LOG_Info("   %s", func);
        }
    }
}

/*********************************************************************//**
**
** FindFreeMemInfo
**
** Finds a free entry in the minfo array
**
** \param   None
**
** \return  Pointer to entry in the array
**
**************************************************************************/
minfo_t *FindFreeMemInfo(void)
{
    int i;
    minfo_t *mi;

    for (i=0; i<MAX_MINFO_ENTRIES; i++)
    {
        mi = &minfo[i];
        if (mi->ptr == NULL)
        {
            return mi;
        }
    }

    // If the code gets here, then the minfo array was full
    return NULL;
}

/*********************************************************************//**
**
** FindMemInfoByPtr
**
** Finds the meminfo entry matching the specified pointer
** NOTE: This function may be called with ptr==NULL, in which case it finds the first free entry (which is the correct behaviour)
**
** \param   ptr - pointer specifying the meminfo array entry to match
**
** \return  Pointer to entry in the array
**
**************************************************************************/
minfo_t *FindMemInfoByPtr(void *ptr)
{
    int i;
    minfo_t *mi;

    for (i=0; i<MAX_MINFO_ENTRIES; i++)
    {
        mi = &minfo[i];
        if (mi->ptr == ptr)
        {
            return mi;
        }
    }

    // If the code gets here, then the ptr could not be found
    return NULL;
}

/*********************************************************************//**
**
** GetCallers
**
** Gets an array of pointers to the names of the functions in the callstack
**
** \param   callers - pointer to array in which to return pointers to the names of the functions in the callstack
** \param   num_callers - number of entries in the array
**
** \return  None
**
**************************************************************************/
void GetCallers(char **callers, int num_callers)
{
    int i;
    int stack_end = 0;

#ifdef HAVE_EXECINFO_H
    #define MAX_CALLSTACK  30
    void *callstack[MAX_CALLSTACK];
    void **stack_start;
    int stack_size;
    int symbols_found;
    const char *func_name;
    Dl_info info;

    // Get program counter return addresses of all functions in the callstack
    stack_size = backtrace(callstack, NUM_ELEM(callstack));

    // Adjust callstack and stack_size, so that it starts at the grandparent caller of this function
    #define SKIP_UP_CALLSTACK 2
    stack_start = &callstack[SKIP_UP_CALLSTACK];
    stack_size -= SKIP_UP_CALLSTACK;
    stack_end = MIN(stack_size-2, num_callers);  // Minus 2 because we want to stop at main() - not callstack prior to main()

    // Iterate over the functions in the callstack, determining the names of the functions
    for (i=0; i<stack_end; i++)
    {
        symbols_found = dladdr(stack_start[i], &info);
        if (symbols_found)
        {
            func_name = (info.dli_sname != NULL) ? info.dli_sname : "Unknown";
            callers[i] = (char *)func_name;
        }
        else
        {
            callers[i] = "dladdr failed";
        }
    }
#endif

    // Fill in end of callers array, in the case that the callstack is shorter than num_callers
    for (i=stack_end; i<num_callers; i++)
    {
        callers[i] = NULL;
    }
}

/*********************************************************************//**
**
** GetMemUsage
**
** Returns the amount of memory in use, or 0 if unable to obtain this
**
** \param   None
**
** \return  Numer of bytes of memory in use by this executable, or 0 if unable to obtain this information
**
**************************************************************************/
unsigned GetMemUsage(void)
{
    unsigned mem_use = 0;

#if defined(HAVE_MALLINFO) || defined(HAVE_MALLINFO2)
#ifdef HAVE_MALLINFO2
    mem_use = (unsigned) mallinfo2().uordblks;
#else
    mem_use = (unsigned) mallinfo().uordblks;
#endif
#endif


    return mem_use;
}
