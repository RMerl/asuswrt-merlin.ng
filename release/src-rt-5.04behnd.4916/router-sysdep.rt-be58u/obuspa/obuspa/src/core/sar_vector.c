/*
 *
 * Copyright (C) 2023, Broadband Forum
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
 * \file sar_vector.c
 *
 * Implements a vector used by the Segmentation and Reassembly (SAR)
 * mechanism during the End-to-End Session Context.
 *
 */
#include "sar_vector.h"

#include <string.h>

/*********************************************************************//**
**
** IsValidPayload
**
** Validate the SAR payload to insert is valid according to the content
** of the SAR vector.
**
** To be valid, the new payload
** - is the first to be append in the SAR vector, or
** - must have the same session_id and the following sequence_id.
**
** \param   sarv - pointer to SAR vector structure to initialise
** \param   new_payload - pointer to the payload to validate
**
** \return  None
**
**************************************************************************/
bool IsValidPayload(sar_vector_t *sarv, sar_payload_t *new_payload)
{
    sar_payload_t* last_payload;

    // If no entries, it is valid
    if (sarv->num_entries == 0) { return true; }

    USP_ASSERT(sarv->vector);
    last_payload = &sarv->vector[sarv->num_entries-1];

    // Exit if different session_id (it is invalid)
    if (last_payload->sess_id != new_payload->sess_id)
    {
        return false;
    }

    // Exit if not the next sequence_id (it is invalid)
    if (new_payload->seq_id != last_payload->seq_id+1)
    {
        return false;
    }

    return true;
}

/*********************************************************************//**
**
** SAR_VECTOR_Init
**
** Initialises a SAR payload vector structure
**
** \param   sarv - pointer to SAR vector structure to initialise
**
** \return  None
**
**************************************************************************/
void SAR_VECTOR_Init(sar_vector_t *sarv)
{
    sarv->vector = NULL;
    sarv->num_entries = 0;
    sarv->sum_length = 0;
}

/*********************************************************************//**
**
** SAR_VECTOR_Append
**
** Appends the pointed data at the end the vector. If the E2E segment is
** not valid according to the SAR vector (e.g., out of order or duplicated),
** the segment is not append and the function returns false;
**
** NOTE: This function performs a deep copy of the data.
**
** \param   sarv - pointer to vector structure to add the data into
** \param   sess_id - Session ID associated to the data
** \param   seq_id - Sequence ID associated to the data
** \param   data - pointer to data to add to the vector
** \param   len - length of the data to add to the vector
**
** \return  true if the segment is append to the SAR vector
**
**************************************************************************/
bool SAR_VECTOR_Append(sar_vector_t *sarv, uint64_t sess_id, uint64_t seq_id, uint8_t *data, int len)
{
    bool result = false;
    sar_payload_t new_payload;
    new_payload.sess_id = sess_id;
    new_payload.seq_id = seq_id;
    new_payload.len = len;

    if (IsValidPayload(sarv, &new_payload))
    {
        int last_index = sarv->num_entries;
        sarv->num_entries++;
        sarv->vector = USP_REALLOC(sarv->vector, sarv->num_entries*sizeof(sar_payload_t));

        sarv->vector[last_index] = new_payload;
        sarv->vector[last_index].data = USP_MALLOC(len);
        memcpy(sarv->vector[last_index].data, data, len);
        sarv->sum_length += len;
        result = true;
    }

    return result;
}

/*********************************************************************//**
**
** SAR_VECTOR_Destroy
**
** Deallocates all memory associated with the vector
**
** \param   sarv - pointer to structure to destroy all dynamically allocated memory it contains
**
** \return  None
**
**************************************************************************/
void SAR_VECTOR_Destroy(sar_vector_t *sarv)
{
    int i;

    // Exit if vector is already empty
    if (sarv->vector == NULL)
    {
        goto exit;
    }

    // Free all dynamically allocated memory associated with each payload
    for (i=0; i < sarv->num_entries; i++)
    {
        sar_payload_t *payload = &sarv->vector[i];
        USP_SAFE_FREE(payload->data);
    }

    // Free the vector itself
    USP_FREE(sarv->vector);

exit:
    // Ensure structure is re-initialised
    SAR_VECTOR_Init(sarv);
}

/*********************************************************************//**
**
** SAR_VECTOR_Get
**
** Get a pointer to the allocated payload in the vector
**
** \param   sarv - pointer to vector structure to get the payload from
** \param   index - 0-based index of the payload in the vector
**
** \return  pointer to the payload structure stored in the vector
**
**************************************************************************/
sar_payload_t *SAR_VECTOR_Get(sar_vector_t *sarv, unsigned index)
{
    if (index < sarv->num_entries)
    {
        return &sarv->vector[index];
    }

    return NULL;
}

/*********************************************************************//**
**
** SAR_VECTOR_Serialize
**
** Get a pointer to a serialized payload made from all allocated payloads.
** The ownership of the returned buffer belongs to the caller.
**
** \param   sarv - pointer to vector structure to get the payload from
** \param   len -  returns the length of the serialized data
**
** \return  returns pointer to serialized data (the ownership of
**          the buffer is passed to the caller) or NULL if SAR vector is empty.
**
**************************************************************************/
uint8_t* SAR_VECTOR_Serialize(sar_vector_t *sarv, int *len)
{
    uint8_t *data = NULL;
    *len = 0;
    unsigned u;

    if (sarv->sum_length <= 0) return NULL;

    // Concatenates all payloads in the destination buffer
    data = USP_MALLOC(sarv->sum_length);
    for (u = 0; u < sarv->num_entries; u++)
    {
        sar_payload_t *sp = &sarv->vector[u];
        memcpy(&data[*len], sp->data, sp->len);

        // Add the length of appended bytes.
        *len += sp->len;
    }

    // If the length of the serialized data is not exactly the sum_length,
    // something went wrong:
    // - inconsistent sum_length?
    // - wrong num_entries?
    USP_ASSERT(*len == sarv->sum_length);

    return data;
}
