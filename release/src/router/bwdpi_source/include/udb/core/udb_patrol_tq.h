/*
 * Copyright 2014 Trend Micro Incorporated
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software without 
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 */

#ifndef __UDB_PATROL_TQ_H__
#define __UDB_PATROL_TQ_H__


extern int tdts_core_ioctl_app_op_time_get(char *tbl, uint32_t tbl_len, uint32_t *tbl_used_len);
extern int tdts_core_ioctl_op_tq_construct(char *buf, uint32_t tbl_len);
extern int tdts_core_ioctl_app_op_tq_get(char *tbl, uint32_t tbl_len, uint32_t *tbl_used_len);
extern int tdts_core_ioctl_app_op_tq_log_get(char *tbl, uint32_t tbl_len, uint32_t *tbl_used_len);
extern int tdts_core_calc_patrol_tq(uint8_t uid, uint8_t cat_id, uint16_t app_id, uint32_t bytes, uint8_t is_download, uint8_t new_session);
extern int tdts_core_ioctl_op_tq_reset(void);
#endif // __UDB_PATROL_TQ_H__
