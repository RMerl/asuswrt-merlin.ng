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
 * \file e2e_defs.h
 *
 * Header file containing commonly used definitions for End-to-End exchanges
 *
 */
#ifndef E2E_DEFS_H
#define E2E_DEFS_H

#include "usp-msg.pb-c.h"
#include "sar_vector.h"

#define E2E_FIRST_VALID_SESS_ID 2  // According to USP spec, it must be greater than 1.
#define E2E_FIRST_SEQ_ID 1  // A sequence_id starts at 1

typedef enum
{
    kE2EMode_Require,
    kE2EMode_Allow,
    kE2EMode_Forbid,
    kE2EMode_Max  // This must always be the last entry in this enumeration. It is used as array size.
                  // Update the kE2E_EVENT_MAP in device_controller.c for new enums
} e2e_session_mode_t;

typedef enum
{
    kE2EStatus_Up,
    kE2EStatus_Negotiating,
    kE2EStatus_Down,
    kE2EStatus_Max  // This must always be the last entry in this enumeration. It is used as array size.
                    // Update the kE2E_EVENT_MAP in device_controller.c for new enums
} e2e_session_status_t;

typedef enum
{
    kE2EEvent_None,
    kE2EEvent_Establishment,
    kE2EEvent_Termination,
    kE2EEvent_Restart,
} e2e_event_t;

//------------------------------------------------------------------------------
// Structure representing entries in the Device.LocalAgent.Controller.{i}.E2ESession object
typedef struct
{
    e2e_session_mode_t mode;                // E2ESession.SessionMode currently applied
    e2e_session_mode_t mode_buffered;       // SessionMode value to apply on next E2E_CONTEXT_E2eSessionEvent
    unsigned max_record_size;               // E2ESession.MaxUSPRecordSize
    e2e_session_status_t status;            // E2ESession.Status

    // State variables maintained by the E2E Session Context
    uint64_t current_session_id;

    // State variables required during segmentation process
    uint64_t last_sent_sequence_id;

    // State variables required during reassembly process
    uint64_t last_recv_sequence_id;
    sar_vector_t received_payloads;
} e2e_session_t;

#endif
