 /*
 * Copyright 2017, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#ifndef _auth_common_h_
#define _auth_common_h_

/* header */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

/* DEBUG DEFINE */
#define SW_HW_AUTH_DEBUG             "/tmp/SW_HW_AUTH_DEBUG"
#define AUTH_DBG(fmt,args...) \
	if(f_exists(SW_HW_AUTH_DEBUG) > 0) { \
		printf("[SW_HW_AUTH][%s:(%d)]"fmt, __FUNCTION__, __LINE__, ##args); \
	}

/* app.c */
extern char *get_auth_code(char *initial_value, char *out_buf, int out_buf_size);

/* hw_auth.c */
extern char *DoHardwareComponent(char *index);
extern char *DoHardwareCheck(char *app_key);
extern char *hw_auth_check(char *app_id, char *app_auth_code, time_t timestamp, char *out_buf, int out_buf_size);

#endif /* _auth_common_h_ */
