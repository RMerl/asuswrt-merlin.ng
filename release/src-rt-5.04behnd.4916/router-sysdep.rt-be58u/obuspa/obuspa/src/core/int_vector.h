/*
 *
 * Copyright (C) 2019-2020, Broadband Forum
 * Copyright (C) 2016-2020  CommScope, Inc
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
 * \file int_vector.h
 *
 * Implements a vector of integers
 * Unlike the other vector data structures, this has been re-implemented using a maximum fixed size array,
 * as this vector is only used to collect the instance numbers of an object.
 * Using a fixed size array minimizes the number of memory allocations, speeding up the code and minimising the risk of memory leaks
 *
 */

#ifndef INT_VECTOR_H
#define INT_VECTOR_H

#include "usp_api.h"

//-----------------------------------------------------------------------------------------
// Int Vector API
void INT_VECTOR_Init(int_vector_t *iv);
void INT_VECTOR_Add(int_vector_t *iv, int number);
int  INT_VECTOR_Find(int_vector_t *iv, int number);
void INT_VECTOR_Destroy(int_vector_t *iv);

#endif
