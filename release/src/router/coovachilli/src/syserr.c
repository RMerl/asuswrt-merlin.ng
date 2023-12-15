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

#include "chilli.h"

void sys_err(int pri, char *fn, int ln, int en, const char *fmt, ...) {
  if (pri==LOG_DEBUG && !_options.debug) return;
  {
    bstring bt = bfromcstralloc(128,"");
    int sz;
    
    bvformata(sz, bt, fmt, fmt);
    if (sz == BSTR_OK) {
      if (_options.foreground && _options.debug) {
	fprintf(stderr, "%s: %d: %d (%s) %s\n", fn, ln, en, en ? strerror(en) : "Debug", bt->data);
      } else {
	if (en)
	  syslog(pri, "%s: %d: %d (%s) %s", fn, ln, en, strerror(en), bt->data);
	else
	  syslog(pri, "%s: %d: %s", fn, ln, bt->data);
      }
    }
    bdestroy(bt);
  }
}

void sys_errpack(int pri, char *fn, int ln, int en, struct sockaddr_in *peer,
		 void *pack, unsigned len, char *fmt, ...) {
  bstring bt = bfromcstr("");
  bstring bt2 = bfromcstr("");
  int sz;
  int n;
  
  bvformata(sz, bt, fmt, fmt);
  if (sz == BSTR_OK) {

    bassignformat(bt2, ". Packet from %s:%u, length: %d, content:",
		  inet_ntoa(peer->sin_addr),
		  ntohs(peer->sin_port),
		  len);
    
    bconcat(bt, bt2);
    
    for(n=0; n < len; n++) {
      bassignformat(bt, " %02hhx", ((unsigned char*)pack)[n]);
      bconcat(bt, bt2);
    }
  
    if (_options.foreground && _options.debug) {
      fprintf(stderr, "%s: %d: %d (%s) %s", fn, ln, en, strerror(en), bt->data);
    } else {
      if (en)
	syslog(pri, "%s: %d: %d (%s) %s", fn, ln, en, strerror(en), bt->data);
      else
	syslog(pri, "%s: %d: %s", fn, ln, bt->data);
    }
  }

  bdestroy(bt);
  bdestroy(bt2);
}
