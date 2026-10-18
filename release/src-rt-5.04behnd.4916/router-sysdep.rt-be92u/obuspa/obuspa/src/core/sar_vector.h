/*
 *
 * Copyright (C) 2022-2024, Broadband Forum
 * Copyright (C) 2022, Snom Technology GmbH
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
 * \file sar_vector.h
 *
 * Implements a vector used by the Segmentation and Reassembly (SAR)
 * mechanism during the End-to-End Session Context.
 *
 */

#ifndef SAR_VECTOR_H
#define SAR_VECTOR_H

#include <stdint.h>
#include <stdlib.h>

#include "common_defs.h"
#include "kv_vector.h"

//------------------------------------------------------------------------------
// Element of binary payload with metadata related to SAR.
// The contained data is an arbitrary sequence of bytes. It may contain embedded
// NULL characters and is not required to be NULL-terminated.
typedef struct
{
    uint64_t sess_id;  // session_id associated to this data
    uint64_t seq_id;  // sequence_id associated to this data
	int	len;  // Number of bytes in the data field.
	uint8_t	*data;  // Data bytes.
} sar_payload_t;

//------------------------------------------------------------------------------
// Vector of SAR payload
typedef struct
{
    int num_entries;
    sar_payload_t *vector;
    int sum_length;
} sar_vector_t;

//-----------------------------------------------------------------------------------------
// E2E Session Context SAR Vector API
void SAR_VECTOR_Init(sar_vector_t *sarv);
bool SAR_VECTOR_Append(sar_vector_t *sarv, uint64_t sess_id, uint64_t seq_id, uint8_t *data, int len);
void SAR_VECTOR_Destroy(sar_vector_t *sarv);
sar_payload_t *SAR_VECTOR_Get(sar_vector_t *sarv, unsigned index);
uint8_t* SAR_VECTOR_Serialize(sar_vector_t *sarv, int *len);

#endif
