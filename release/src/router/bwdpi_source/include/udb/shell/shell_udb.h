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

#ifndef _SHELL_UDB_H_
#define _SHELL_UDB_H_

#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/skbuff.h>
#else
#include <stdint.h>
#endif

#include "udb/tdts_udb_core.h"

#if TMCFG_USER_SPACE

#elif TMCFG_KERN_SPACE
unsigned long shell_get_seconds(void);
void * shell_timer_list_alloc(void);
void shell_timer_list_free(void *timer_list_ptr);
void shell_timer_setup(void *timer_list_ptr, void (*function)(unsigned long),
		unsigned long data);
int shell_timer_mod(void *timer_list_ptr, unsigned long period);
int shell_timer_del(void *timer_list_ptr);
int shell_bitop_find_first_zero_bit(const void * p, unsigned long size);
void shell_bitop_set_bit(int bit, unsigned long *p);
void shell_bitop_clear_bit(int bit, unsigned long * p);
void shell_bitop_bitmap_zero(unsigned long *dst, int nbits);
int shell_bitop_test_bit(int nr, const unsigned long *addr);
int shell_find_next_bit(const unsigned long *p, int size, int offset);
unsigned long shell_find_next_zero_bit(const unsigned long *addr, unsigned long size, unsigned long offset);

//--------------------------------------------------------------------------------------
void udb_shell_init_pkt_parameter(tdts_pkt_parameter_t *param, uint16_t req_flag, tdts_hook_t hook);

tdts_res_t udb_shell_do_fastpath_action(tdts_udb_param_t *fw_param);
tdts_res_t udb_shell_do_ford_action(tdts_pkt_parameter_t *param, int dpi_ret, tdts_udb_param_t *fw_param);
tdts_res_t udb_shell_do_lin_action(tdts_pkt_parameter_t *param, int dpi_ret, tdts_udb_param_t *fw_param);
tdts_res_t udb_shell_do_lout_action(tdts_pkt_parameter_t *param, int dpi_ret, tdts_udb_param_t *fw_param);
tdts_act_t udb_shell_get_action(tdts_udb_param_t *fw_param, tdts_hook_t hook);
uint16_t udb_shell_get_req_flag(tdts_udb_param_t *fw_param, tdts_hook_t hook);

int udb_shell_is_skb_upload(struct sk_buff *skb, uint8_t *smac);
int udb_shell_udb_init(void);
void udb_shell_udb_exit(void);

int udb_shell_usr_msg_handler(uint8_t *msg, int size, int pid, int type);
int udb_shell_update_devid_un_http_ua(uint8_t *addr, uint8_t ip_ver, uint8_t *data);
int udb_shell_update_devid_un_bootp(uint8_t *addr, uint8_t ip_ver, uint8_t *data);
int udb_shell_update_dns_reply(uint8_t *addr, uint8_t ip_ver, uint8_t *data);
int udb_shell_update_upnp_data(uint8_t *data, uint32_t index, void *cb);

int udb_shell_anomaly_init(void);
void udb_shell_anomaly_exit(void);
int udb_shell_anomaly_setup(tdts_pkt_parameter_t *param);
tdts_res_t udb_shell_policy_match(tdts_udb_param_t *fw_param);
#else
#error "Unknown running space."
#endif

#endif /* UDB_SHELL_H_ */
