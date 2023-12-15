/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2003, 2004, 2005 Mondru AB.
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef _SYSERR_H
#define _SYSERR_H

#define SYSERR_MSGSIZE 256

void sys_err(int pri, char *filename, int line, int en, const char *fmt, ...);
void sys_errpack(int pri, char *fn, int ln, int en, struct sockaddr_in *peer,
		 void *pack, unsigned len, char *fmt, ...);

#define log(p,fmt,args...)      sys_err(p,           __FILE__,__LINE__,0,fmt,## args)
#define log_dbg(fmt,args...)    if (_options.debug) {\
                                sys_err(LOG_DEBUG,   __FILE__,__LINE__,0,fmt,## args); }
#define log_warn(e,fmt,args...) sys_err(LOG_WARNING, __FILE__,__LINE__,e,fmt,## args)
#define log_info(fmt,args...)   sys_err(LOG_NOTICE,  __FILE__,__LINE__,0,fmt,## args)
#define log_err(e,fmt,args...)  sys_err(LOG_ERR,     __FILE__,__LINE__,e,fmt,## args)

#endif	/* !_SYSERR_H */
