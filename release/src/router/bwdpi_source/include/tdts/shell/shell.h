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

#ifndef TDTS_SHELL_SHELL_H_
#define TDTS_SHELL_SHELL_H_

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

#include "tdts/tdts_core.h"

////////////////////////////////////////////////////////////////////////////////
#if TMCFG_USER_SPACE
/*
 * User space
 */

int tdts_shell_rule_parsing_trf_load(void *trf_ptr, unsigned trf_size);
void tdts_shell_rule_parsing_trf_unload(void);
int tdts_shell_rule_parsing_trf_sig_ver_copy(void *buf, uint32_t buf_len);
int tdts_shell_rule_parsing_trf_sig_num_get(unsigned *sig_num_ptr);
int tdts_shell_rule_parsing_get_ptn_data_len(unsigned *len_ptr);
int tdts_shell_rule_parsing_trf_ano_sec_tbl_copy(void *buf, uint32_t buf_len);
void tdts_shell_rule_parsing_trf_shared_info_data_free(void);
int tdts_shell_system_setting_state_set(unsigned state);
int tdts_shell_system_setting_state_get(unsigned *state_ptr);
int tdts_shell_rule_devid_data_len_get(unsigned *len_ptr);
int tdts_shell_devid_data_copy(void *buf, uint32_t buf_len);
int tdts_shell_appid_get_cat_name(unsigned char cat_id,unsigned char *name_ptr,unsigned name_len_max);
int tdts_shell_appid_get_nr_cat_name(uint8_t *buf, uint32_t buf_len);
int tdts_shell_appid_get_all_cat_name(uint8_t *buf, uint32_t buf_len);
int tdts_shell_appid_get_beh_name(unsigned char beh_id,unsigned char *name_ptr,unsigned name_len_max);
int tdts_shell_appid_get_nr_beh_name(uint8_t *buf, uint32_t buf_len);
int tdts_shell_appid_get_all_beh_name(uint8_t *buf, uint32_t buf_len);
int tdts_shell_appid_get_app_name(unsigned char cat_id,unsigned short app_id,unsigned char *name_ptr,unsigned name_len_max);
int tdts_shell_appid_get_nr_app_name(uint8_t *buf, uint32_t buf_len);
int tdts_shell_appid_get_all_app_name(uint8_t *buf, uint32_t buf_len, uint32_t *buf_used_len);
int tdts_shell_appid_combination_check(unsigned char cat_id, unsigned short app_id, unsigned char beh_id,unsigned char *is_existed_ptr);
int tdts_shell_appid_get_nr_id(uint8_t *buf, uint32_t buf_len);
int tdts_shell_appid_get_all_id(uint8_t *buf, uint32_t buf_len, uint32_t *buf_used_len);
int tdts_shell_get_matched_rule_buf_len(uint32_t *buf_len);
int tdts_shell_get_matched_rule_list(char *buf, uint32_t buf_len);
int tdts_shell_get_matched_rule_info(void *buf, uint32_t buf_len);
int tdts_shell_get_engine_status(void *buf, uint32_t buf_len);

int tdts_shell_system_setting_tcp_conn_max_set(unsigned int conn_max);
int tdts_shell_system_setting_tcp_conn_max_get(unsigned int *conn_max_ptr);
int tdts_shell_system_setting_udp_conn_max_set(unsigned int conn_max);
int tdts_shell_system_setting_udp_conn_max_get(unsigned int *conn_max_ptr);

int tdts_shell_dpi_l2_eth(tdts_pkt_parameter_t *pkt_parameter_ptr);
int tdts_shell_dpi_l3_pkt(uint32_t ip_proto, void *pkt_ptr, uint32_t pkt_len, tdts_pkt_parameter_t *param);
int tdts_shell_set_syscalls(void);
int tdts_shell_init(void);
void tdts_shell_cleanup(void);

#if TMCFG_E_CORE_METADATA_EXTRACT
int tdts_shell_dpi_register_mt(meta_extract cb, int type);
int tdts_shell_dpi_unregister_mt(meta_extract cb, int type);
#endif

# if TMCFG_E_CORE_PORT_SCAN_DETECTION
int tdts_shell_port_scan_log_cb_set(port_scan_log_func cb);
void *tdts_shell_port_scan_context_alloc(void);
int tdts_shell_port_scan_context_dealloc(void *ctx);
# endif

# if TMCFG_E_CORE_IP_SWEEP_DETECTION
void *tdts_shell_ip_sweep_context_alloc(void);
int tdts_shell_ip_sweep_context_dealloc(void *ctx);
# endif


int tdts_shell_flood_mem_init(void *void_ptr, unsigned mem_size, unsigned record_max);
int tdts_shell_flood_record(void *void_ptr, unsigned record_max, flood_spec_t *spec, pkt_info_t *pkt);
void tdts_shell_flood_record_output_and_init(void *void_ptr, unsigned record_max, uint32_t signature_id,
											uint16_t flood_type, flood_log_cb log_cb);

extern int tdts_shell_suspect_list_init(void *void_ptr, unsigned mem_size, unsigned entry_num);

extern int tdts_shell_insert_suspect_list_entry(void *void_ptr, unsigned entry_num,
												uint8_t ip_ver, uint8_t *dip, uint16_t dport,
												long (*get_curr_time)(void));

extern int tdts_shell_suspect_list_query(void *void_ptr, unsigned entry_num, pkt_info_t *pkt,
									unsigned timeout, long (*get_curr_time)(void));
////////////////////////////////////////////////////////////////////////////////
#elif TMCFG_KERN_SPACE
/*
 * Kernel space
 */

#include "tdts/shell/shell_ioctl.h"
#include "tdts/shell/shell_ioctl_dbg.h"
#include "tdts/shell/shell_ioctl_sig.h"
#include "tdts/shell/shell_ioctl_stat.h"

#ifdef __KERNEL__
#include <linux/skbuff.h>
int tdts_shell_system_setting_tcp_conn_max_set(unsigned int conn_max);
int tdts_shell_system_setting_tcp_conn_max_get(unsigned int *conn_max_ptr);
int tdts_shell_system_setting_tcp_conn_timeout_set(unsigned int timeout);
int tdts_shell_system_setting_tcp_conn_timeout_get(unsigned int *timeout_ptr);
int tdts_shell_system_setting_udp_conn_max_set(unsigned int conn_max);
int tdts_shell_system_setting_udp_conn_max_get(unsigned int *conn_max_ptr);

int tdts_shell_dpi_l2_eth(tdts_pkt_parameter_t *pkt_parameter_ptr);
int tdts_shell_dpi_l3_skb(struct sk_buff *skb, tdts_pkt_parameter_t *param);

#if TMCFG_E_CORE_METADATA_EXTRACT
int tdts_shell_dpi_register_mt(meta_extract cb, int type);
int tdts_shell_dpi_unregister_mt(meta_extract cb, int type);
#endif
# if TMCFG_E_CORE_PORT_SCAN_DETECTION
int tdts_shell_port_scan_log_cb_set(port_scan_log_func cb);
void *tdts_shell_port_scan_context_alloc(void);
int tdts_shell_port_scan_context_dealloc(void *ctx);
# endif

# if TMCFG_E_CORE_IP_SWEEP_DETECTION
void *tdts_shell_ip_sweep_context_alloc(void);
int tdts_shell_ip_sweep_context_dealloc(void *ctx);
# endif

int tdts_shell_flood_mem_init(void *void_ptr, unsigned mem_size, unsigned record_max);
int tdts_shell_suspect_list_init(void *void_ptr, unsigned mem_size, unsigned entry_num);
int tdts_shell_flood_record(void *void_ptr, unsigned record_max, flood_spec_t *spec, pkt_info_t *pkt);
void tdts_shell_flood_record_output_and_init(void *void_ptr, unsigned record_max, uint32_t signature_id,
	uint16_t flood_type, flood_log_cb log_cb);

#endif

////////////////////////////////////////////////////////////////////////////////
#else
#error "Unknown running space."
#endif

typedef struct
{
	tdts_core_sig_ver_t sig_ver;
	tdts_core_eng_ver_t eng_ver;
} __attribute__((packed)) tdts_shell_eng_status_t;

#endif /* TDTS_SHELL_SHELL_H_ */
