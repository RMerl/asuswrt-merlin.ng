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
 * \file usp_mem.h
 *
 * Header file for functions which wrap memory allocation on USP Agent
 *
 */
#ifndef USP_MEM_H
#define USP_MEM_H

#include "compiler.h"

//------------------------------------------------------------------------------------
// Helper macros, so that the code does not have to provide (__FUNCTION__, __LINE__) to the underlying function
#define USP_MALLOC(x)               USP_MEM_Malloc(__FUNCTION__, __LINE__, x)
#define USP_FREE(x)                 USP_MEM_Free(__FUNCTION__, __LINE__, x)
#define USP_SAFE_FREE(x)            if (x != NULL) { USP_MEM_Free(__FUNCTION__, __LINE__, x); x = NULL; }
#define USP_REALLOC(x, y)           USP_MEM_Realloc(__FUNCTION__, __LINE__, x, y)
#define USP_STRDUP(x)               USP_MEM_Strdup(__FUNCTION__, __LINE__, x)

//------------------------------------------------------------------------------------
// Functions wrapping memory allocation
int USP_MEM_Init(void);
void USP_MEM_Destroy(void);
void *USP_MEM_Malloc(const char *func, int line, int size) ARGS_NONNULL MALLOC RETURNS_NONNULL;
void USP_MEM_Free(const char *func, int line, void *ptr) ARGS_NONNULL;
void *USP_MEM_Realloc(const char *func, int line, void *ptr, int size) ARGINDEX_NONNULL(1) MALLOC RETURNS_NONNULL;
void *USP_MEM_Strdup(const char *func, int line, void *ptr) ARGINDEX_NONNULL(1) MALLOC;
void USP_MEM_StartCollection(void);
void USP_MEM_StopCollection(void);
void USP_MEM_Print(void);
void USP_MEM_PrintSummary(void);
int USP_MEM_PrintLeakReport(void);
int USP_MEM_PrintAll(void);
void MAIN_Stop(void);

// Pointer to structure containing the protocol buffer allocator function
extern void *pbuf_allocator;

#endif
