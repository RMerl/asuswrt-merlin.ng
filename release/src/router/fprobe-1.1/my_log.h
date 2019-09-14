/*
	Copyright (C) Slava Astashonok <sla@0n.ru>

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.

	$Id: my_log.h,v 1.1.1.1.2.2 2004/02/02 08:06:24 sla Exp $
*/

#ifndef _MY_LOG_H_
#define _MY_LOG_H_

#define MY_LOG_SYSLOG 0x01
#define MY_LOG_STDOUT 0x02
#define MY_LOG_SYSLOG_FACILITY LOG_DAEMON
#define MY_LOG_MAX_INDEX 32

#include <syslog.h>

void my_log_open(char *, unsigned, unsigned);
void my_log_close(void);
void my_log(unsigned, const char *, ...);

#endif
