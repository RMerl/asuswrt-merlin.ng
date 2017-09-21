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

#ifndef __UDB_QOS_H__
#define __UDB_QOS_H__

#ifdef __KERNEL__
extern int udb_core_update_qos_data(tdts_pkt_parameter_t *param, uint8_t *paid_info, uint8_t *bndwth_info, tdts_udb_param_t *fw_param);
extern int udb_core_qos_init(char *wan, char *lan);
extern void udb_core_qos_exit(void);
extern int ioctl_iqos_op_config(char *buf, uint32_t buf_len);
extern int ioctl_iqos_op_switch(int32_t flag);
extern int ioctl_iqos_op_get_config(char *buf, uint32_t buf_len, uint32_t *buf_used_len);
extern int ioctl_iqos_op_get_app_info(char *buf, uint32_t buf_len, uint32_t *buf_used_len);
extern int udb_core_register_qos_ops(int (*cb)(char *));
extern void udb_core_qos_unregister_qos_ops(void);
extern void udb_core_qos_set_dbg_level(int level);
extern void udb_core_qos_get_dbg_level(int *level);
#endif // __KERNEL__

#endif // __UDB_QOS_H__
