/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2016-2024  CommScope, Inc
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
 * \file os_utils.h
 *
 * Header file for API to wrapper functions around POSIX Operating System functions, such as creating threads
 *
 */
#ifndef OS_UTILS_H
#define OS_UTILS_H

#include <pthread.h>

//-------------------------------------------------------------------------
// Defines for 'print_warning' argument for OS_UTILS_IsDataModelThread()
#define PRINT_WARNING true
#define DONT_PRINT_WARNING false

//-------------------------------------------------------------------------
// API functions
int OS_UTILS_CreateThread(const char* name, void *(* start_routine)(void *), void *args);
void OS_UTILS_SetDataModelThread(void);
bool OS_UTILS_IsDataModelThread(const char *caller, bool print_warning);
int OS_UTILS_InitMutex(pthread_mutex_t *mutex);
void OS_UTILS_LockMutex(pthread_mutex_t *mutex);
void OS_UTILS_UnlockMutex(pthread_mutex_t *mutex);
int OS_UTILS_CreateDirFromFilename(char *filename);
time_t OS_UTILS_TimeNow(void);

#endif

