/*
 * Copyright (C) 2010 Hans de Goede <j.w.r.degoede@gmail.com>
 *
 * This file is part of libmms a free mms protocol library
 *
 * libmms is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * libmss is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

/* This file contains code which is shared between the mms and mmsh protocol
   handling code. */

#ifndef __MMS_COMMON_H
#define __MMS_COMMON_H

#define BUF_SIZE               102400   /* max packet size */
#define ASF_HEADER_SIZE     (8192 * 2)  /* max header size */

#define io_read(io, args...) ((io) ? (io)->read(io->read_data , ## args) : mms_default_io.read(NULL , ## args))
#define io_write(io, args...) ((io) ? (io)->write(io->write_data , ## args) : mms_default_io.write(NULL , ## args))
#define io_select(io, args...) ((io) ? (io)->select(io->select_data , ## args) : mms_default_io.select(NULL , ## args))
#define io_connect(io, args...) ((io) ? (io)->connect(io->connect_data , ## args) : mms_default_io.connect(NULL , ## args))

#ifdef _WIN32
#define set_errno(_err)     WSASetLastError(_err)
#define get_errno()         WSAGetLastError()
#ifndef EINPROGRESS
#define EINPROGRESS         WSAEINPROGRESS
#endif
#ifndef AI_ADDRCONFIG
#define AI_ADDRCONFIG 0
#endif
#ifndef AI_NUMERICSERV
#define AI_NUMERICSERV 0
#endif
#else
#define set_errno(_err)     errno = (_err)
#define get_errno()         errno
#define closesocket(_s)     close(_s)
#ifndef AI_ADDRCONFIG
//#define AI_ADDRCONFIG	0x0020
#define AI_ADDRCONFIG 0
#endif
#ifndef AI_NUMERICSERV
//#define AI_NUMERICSERV	0x0400
#define AI_NUMERICSERV 0
#endif
#endif

typedef struct mms_stream_s mms_stream_t;
struct mms_stream_s {
  int           stream_id;
  int           stream_type;
  int           bitrate;
  int           bitrate_pos;
};

extern mms_io_t mms_default_io;

int mms_internal_winsock_load(void);

#endif
