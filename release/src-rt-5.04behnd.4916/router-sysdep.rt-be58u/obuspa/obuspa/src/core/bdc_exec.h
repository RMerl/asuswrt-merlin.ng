/*
 *
 * Copyright (C) 2019-2021, Broadband Forum
 * Copyright (C) 2016-2021  CommScope, Inc
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
 * \file bdc_exec.h
 *
 * Header file for Main execution loop of bulk data collection thread
 *
 */
#ifndef BDC_EXEC_H
#define BDC_EXEC_H

//------------------------------------------------------------------------------
// API functions
int BDC_EXEC_Init(void);
int BDC_EXEC_PostReportToSend(int profile_id, char *full_url, char *query_string, char *username, char *password, unsigned char *report, int report_len, unsigned flags);
void *BDC_EXEC_Main(void *args);
void BDC_EXEC_ScheduleExit(void);

//------------------------------------------------------------------------------
// Definitions of bits in flags parameter of BDC_EXEC_PostReportToSend()
#define BDC_FLAG_PUT            0x00000001  // If set, HTTP PUT should be used instead of HTTP POST when sending the report to the BDC server
#define BDC_FLAG_GZIP           0x00000002  // If set, the reports contants are Gzipped
#define BDC_FLAG_DATE_HEADER    0x00000004  // If set, the date header should be included in the HTTP post.


#endif
