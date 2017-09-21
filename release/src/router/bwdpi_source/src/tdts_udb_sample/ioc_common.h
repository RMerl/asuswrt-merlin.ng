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

#ifndef __IOC_COMMON_H__
#define __IOC_COMMON_H__

#include "udb/ioctl/udb_ioctl_common.h"

#include "conf_app.h"

enum
{
	ACT_COMMON_GET_ALL_USR = 0,	/* get_all_user */
	ACT_COMMON_GET_USR_DETAIL,	/* get_user_detail */
	ACT_COMMON_GET_ALL_APP,		/* get_all_app */
	ACT_COMMON_GET_ALL_APP_CLEAR,	/* get_all_app_clear */
	ACT_COMMON_SET_APP_PATROL,	/* set_app_patrol */
	ACT_COMMON_GET_APP_PATROL,	/* get_app_patrol */
	ACT_COMMON_SET_WPR_CONF,	/* set_wpr_conf */
	ACT_COMMON_SET_WPR_ON,		/* set_wpr_on */
	ACT_COMMON_SET_WPR_OFF,		/* set_wpr_off */
	ACT_COMMON_SET_REDIRECT_URL,	/* set_redirect_url */
	ACT_COMMON_SET_META_DATA,	/* internal: set_meta_data */
	ACT_COMMON_DISABLE_FEEDBACK,	/* internal: disable_feedback */
	ACT_COMMON_ENABLE_FEEDBACK,	/* internal: enable_feedback */
	ACT_COMMON_SET_EULA_AGREED,	/* internal: set_eula_agreed */
	ACT_COMMON_SET_EULA_DISAGREED,	/* internal: set_eula_disagreed */
	ACT_COMMON_MAX
};

#define IOC_SHIFT_LEN_SAFE(len, siz, max) \
({ \
	uint8_t ok = 1; \
	if ((len) + (siz) > (max)) { \
		ok = 0; \
		DBG("len (%d) exceed max (%d)!\n", (len) + (siz), (max)); \
	} \
	else { \
		(len) += (siz); \
	} \
	ok; \
})

int run_ioctl(const char *path, int req, void *arg);
int get_fw_user_list(udb_ioctl_entry_t **output, uint32_t *used_len);

unsigned long get_build_date(void);
unsigned long get_build_number(void);

int parse_single_str_arg(int argc, char **argv, char opt, char *buf, int buf_len);

int common_options_init(struct cmd_option *cmd);

#endif /* __IOC_COMMON_H__ */

