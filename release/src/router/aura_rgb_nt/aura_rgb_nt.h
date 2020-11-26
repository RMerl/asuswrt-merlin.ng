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

#ifndef _aura_rgb_nt_h_
#define _aura_rgb_nt_h_

//extern void update_rgb_led(int sig);

/* DEBUG DEFINE */
#define AURA_RGB_NT_DEBUG             "/tmp/AURA_RGB_NT_DEBUG"
#define AURA_NT_DBG(fmt,args...) \
	if(f_exists(AURA_RGB_NT_DEBUG) > 0) { \
		dbg("[AURA_RGB_NT][%s:(%d)]"fmt, __FUNCTION__, __LINE__, ##args); \
	}
#endif /*  _aura_rgb_h_ */
