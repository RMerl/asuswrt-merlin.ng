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

#ifndef _SHELL_QOS_H_
#define _SHELL_QOS_H_

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

#include "udb/tdts_udb_core.h"

#ifndef TMCFG_E_CORE_METADATA_EXTRACT
#define TMCFG_E_CORE_METADATA_EXTRACT 0
#endif

#if 0
enum
{
	QOS_DBG_LEVEL_CRI = 0,
	QOS_DBG_LEVEL_ERR,
	QOS_DBG_LEVEL_DBG,
	QOS_DBG_LEVEL_INF,
	QOS_DBG_LEVEL_ALL
};
#endif

#if TMCFG_USER_SPACE

#elif TMCFG_KERN_SPACE
#if TMCFG_E_CORE_METADATA_EXTRACT
int udb_shell_update_qos_data(tdts_pkt_parameter_t *param, tdts_meta_paid_info_t *paid_info, tdts_meta_bndwth_info_t *bndwth_info, tdts_udb_param_t *fw_param);
#endif

int udb_shell_qos_init(void);
void udb_shell_qos_exit(void);
int udb_shell_register_qos_ops(int (*cb)(char *));
void udb_shell_unregister_qos_ops(void);
#else
#error "Unknown running space."
#endif

#endif /* _SHELL_QOS_H_ */
