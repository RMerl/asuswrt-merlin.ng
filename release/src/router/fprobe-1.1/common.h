/*
	Copyright (C) Slava Astashonok <sla@0n.ru>

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.

	$Id: common.h,v 1.2.2.2 2004/02/02 08:06:24 sla Exp $
*/

#ifndef _COMMON_H_
#define _COMMON_H_

#define DBG my_log(LOG_DEBUG, "DBG: %s:%d", __FILE__, __LINE__)

#include <config.h>

/* Capture*/
#define DEBUG_C 1
/* Unpending */
#define DEBUG_U 2
/* Scan */
#define DEBUG_S 4
/* Emit */
#define DEBUG_E 8
/* Memory */
#define DEBUG_M 16
/* Fill */
#define DEBUG_F 32
/* Info */
#define DEBUG_I 64

#endif
