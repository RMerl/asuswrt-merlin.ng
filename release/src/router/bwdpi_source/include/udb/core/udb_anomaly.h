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

#ifndef __UDB_ANOMALY_H__
#define __UDB_ANOMALY_H__

typedef struct
{
	int (*dpi_flood_record)(void *void_ptr, unsigned record_max, flood_spec_t *spec, pkt_info_t *pkt);
	void (*dpi_flood_record_output_and_init)(void *void_ptr, unsigned record_max, uint32_t signature_id, unsigned short flood_type, flood_log_cb log_cb);
	int (*dpi_flood_mem_init)(void *void_ptr, unsigned mem_size, unsigned record_max);
	void *(*dpi_port_scan_context_alloc)(void);
	int (*dpi_port_scan_context_dealloc)(void *ctx);
} udb_anomaly_ops_t;

extern int ioctl_ano_op_get(char *buf, uint32_t tbl_len, uint32_t *tbl_used_len);
extern int ioctl_ano_op_get_v2(char *buf, uint32_t tbl_len, uint32_t *tbl_used_len);
extern int ioctl_ano_op_clear(void);
extern int udb_core_anomaly_init(udb_anomaly_ops_t *ops);
extern int udb_core_anomaly_exit(void);
extern int udb_core_anomaly_setup(tdts_pkt_parameter_t *fw_param);

#endif // __UDB_ANOMALY_H__
