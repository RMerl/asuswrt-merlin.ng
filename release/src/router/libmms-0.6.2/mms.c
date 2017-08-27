/*
 * Copyright (C) 2002-2004 the xine project
 * 
 * This file is part of LibMMS, an MMS protocol handling library.
 * 
 * xine is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the ree Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * xine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *
 * $Id: mms.c,v 1.31 2007/12/11 20:35:01 jwrdegoede Exp $
 *
 * MMS over TCP protocol
 *   based on work from major mms
 *   utility functions to handle communication with an mms server
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#include <ws2tcpip.h>
#if _WIN32_WINNT <= 0x500
#include <wspiapi.h>
#endif
#endif

#include <unistd.h>
#include <stdio.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#ifdef VV_USE_ICONV
#include <iconv.h>
#endif

/********** logging **********/
#define lprintf(...) \
    if (getenv("LIBMMS_DEBUG")) \
        fprintf(stderr, "mms: " __VA_ARGS__)

#define __MMS_C__
#define TYPE mms_t

#include "bswap.h"
#include "mms.h"
#include "asfheader.h"
#include "uri.h"
#include "mms-common.h"

/* 
 * mms specific types 
 */

#define DEFAULT_PORT 1755

#define CMD_HEADER_LEN   40
#define CMD_PREFIX_LEN    8
#define CMD_BODY_LEN   1024 * 16 /* FIXME: make this dynamic */

#define MMS_PACKET_ERR        0
#define MMS_PACKET_COMMAND    1
#define MMS_PACKET_ASF_HEADER 2
#define MMS_PACKET_ASF_PACKET 3

#define ASF_HEADER_PACKET_ID_TYPE 2
#define ASF_MEDIA_PACKET_ID_TYPE  4

typedef struct mms_buffer_s mms_buffer_t;
struct mms_buffer_s {
  uint8_t *buffer;
  int pos;
};

typedef struct mms_packet_header_s mms_packet_header_t;
struct mms_packet_header_s {
  uint32_t  packet_len;
  uint8_t   flags;
  uint8_t   packet_id_type;
  uint32_t  packet_seq;
};

struct mms_s {
  int           s;
  
  /* url parsing */
  GURI         *guri;
  char         *url;
  char         *proto;
  char         *connect_host;
  int           connect_port;
  char         *user;
  char         *password;
  char         *uri;

  /* command to send */
  char          scmd[CMD_HEADER_LEN + CMD_BODY_LEN];
  char         *scmd_body; /* pointer to &scmd[CMD_HEADER_LEN] */
  int           scmd_len; /* num bytes written in header */
  
  char          str[1024]; /* scratch buffer to built strings */
  
  /* receive buffer */
  uint8_t       buf[BUF_SIZE];
  int           buf_size;
  int           buf_read;
  off_t         buf_packet_seq_offset; /* packet sequence offset residing in
                                          buf */
  
  uint8_t       asf_header[ASF_HEADER_SIZE];
  uint32_t      asf_header_len;
  uint32_t      asf_header_read;
  int           seq_num;
  int           num_stream_ids;
  mms_stream_t  streams[ASF_MAX_NUM_STREAMS];
  uint8_t       packet_id_type;
  off_t         start_packet_seq; /* for live streams != 0, need to keep it around */
  int           need_discont; /* whether we need to set start_packet_seq */
  uint32_t      asf_packet_len;
  uint64_t      file_len;
  uint64_t      file_time;
  uint64_t      time_len; /* playback time in 100 nanosecs (10^-7) */
  uint64_t      preroll;
  uint64_t      asf_num_packets;
  char          guid[37];
  int           bandwidth;
  
  int           has_audio;
  int           has_video;
  int           live_flag;
  int           seekable;
  off_t         current_pos;
  int           eos;
};

const static char *const mms_proto_s[] = { "mms", "mmst", NULL };

#include "mms-common-funcs.h"

static int fallback_io_select(void *data, int socket, int state, int timeout_msec)
{
  fd_set set;
  struct timeval tv = { timeout_msec / 1000, (timeout_msec % 1000) * 1000};
  FD_ZERO(&set);
  FD_SET(socket, &set);
  return select(1, (state == MMS_IO_READ_READY) ? &set : NULL,
                   (state == MMS_IO_WRITE_READY) ? &set : NULL, NULL, &tv);
}

static off_t fallback_io_read(void *data, int socket, char *buf, off_t num)
{
  off_t len = 0, ret;
/*   lprintf("%d\n", fallback_io_select(data, socket, MMS_IO_READ_READY, 1000)); */
  set_errno(0);
  while (len < num)
  {
#ifdef WIN32
    ret = (off_t)recv(socket, buf + len, num - len, 0);
#else
    ret = (off_t)read(socket, buf + len, num - len);
#endif
    if(ret == 0)
      break; /* EOF */
    if(ret < 0) {
      lprintf("read error @ len = %lld: %s\n", (long long int) len,
              strerror(get_errno()));
      switch(get_errno())
      {
          case EAGAIN:
            continue;
          default:
            /* if already read something, return it, we will fail next time */
            return len ? len : ret; 
      }
    }
    len += ret;
  }
  return len;
}

static off_t fallback_io_write(void *data, int socket, char *buf, off_t num)
{
#ifdef WIN32
  return (off_t)send(socket, buf, num, 0);
#else
  return (off_t)write(socket, buf, num);
#endif
}

static int fallback_io_tcp_connect(void *data, const char *host, int port)
{
  struct addrinfo *r, *res, hints;
  char port_str[16];
  int i, s;

  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_ADDRCONFIG | AI_NUMERICSERV;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  sprintf(port_str, "%d", port);
  i = getaddrinfo(host, port_str, &hints, &res);
  if (i != 0) {
    lprintf("unable to resolve host: %s\n", host);
    return -1;
  }

  for (r = res; r != NULL; r = r->ai_next) {
    s = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
    if (s != -1) {
      if (connect(s, r->ai_addr, r->ai_addrlen) != -1) {
        freeaddrinfo(res);
        return s;
      }
      closesocket(s);
    }
  }

  freeaddrinfo(res);
  return -1;
}

#ifdef _WIN32
static int iWinsockNeed = WSASYSNOTREADY;

int mms_internal_winsock_load(void)
{
  if (iWinsockNeed) {
    WORD    wVersionReq = MAKEWORD(1, 1);
    WSADATA wsaData;

    iWinsockNeed = WSAStartup(wVersionReq, &wsaData);
    if (iWinsockNeed != 0) {
        /* Cannot load Winsock DLL. */
        return 0;
    }

    /* Check if the WinSock DLL supports 1.1+ */
    if (wsaData.wVersion != wVersionReq) {
        /* No good WinSock DLL. */
        WSACleanup();
        iWinsockNeed = WSAVERNOTSUPPORTED;
        return 0;
    }
  }
  return 1;
}
#endif


static mms_io_t fallback_io =
  {
    &fallback_io_select,
    NULL,
    &fallback_io_read,
    NULL,
    &fallback_io_write,
    NULL,
    &fallback_io_tcp_connect,
    NULL,
  };

mms_io_t mms_default_io = {
    &fallback_io_select,
    NULL,
    &fallback_io_read,
    NULL,
    &fallback_io_write,
    NULL,
    &fallback_io_tcp_connect,
    NULL,
  };
  
const mms_io_t* mms_get_default_io_impl()
{
  return &mms_default_io;
}

void mms_set_default_io_impl(const mms_io_t *io)
{
  if (io->select) {
    mms_default_io.select = io->select;
    mms_default_io.select_data = io->select_data;
  } else {
    mms_default_io.select = fallback_io.select;
    mms_default_io.select_data = fallback_io.select_data;
  }
  if (io->read) {
    mms_default_io.read = io->read;
    mms_default_io.read_data = io->read_data;
  } else {
    mms_default_io.read = fallback_io.read;
    mms_default_io.read_data = fallback_io.read_data;
  }
  if (io->write) {
    mms_default_io.write = io->write;
    mms_default_io.write_data = io->write_data;
  } else {
    mms_default_io.write = fallback_io.write;
    mms_default_io.write_data = fallback_io.write_data;
  }
  if (io->connect) {
    mms_default_io.connect = io->connect;
    mms_default_io.connect_data = io->connect_data;
  } else {
    mms_default_io.connect = fallback_io.connect;
    mms_default_io.connect_data = fallback_io.connect_data;
  }
}

static void mms_buffer_init (mms_buffer_t *mms_buffer, uint8_t *buffer) {
  mms_buffer->buffer = buffer;
  mms_buffer->pos = 0;
}

static void mms_buffer_put_8 (mms_buffer_t *mms_buffer, uint8_t value) {

  mms_buffer->buffer[mms_buffer->pos]     = value          & 0xff;

  mms_buffer->pos += 1;
}

#if 0
static void mms_buffer_put_16 (mms_buffer_t *mms_buffer, uint16_t value) {

  mms_buffer->buffer[mms_buffer->pos]     = value          & 0xff;
  mms_buffer->buffer[mms_buffer->pos + 1] = (value  >> 8)  & 0xff;

  mms_buffer->pos += 2;
}
#endif

static void mms_buffer_put_32 (mms_buffer_t *mms_buffer, uint32_t value) {

  mms_buffer->buffer[mms_buffer->pos]     = value          & 0xff;
  mms_buffer->buffer[mms_buffer->pos + 1] = (value  >> 8)  & 0xff;
  mms_buffer->buffer[mms_buffer->pos + 2] = (value  >> 16) & 0xff;
  mms_buffer->buffer[mms_buffer->pos + 3] = (value  >> 24) & 0xff;

  mms_buffer->pos += 4;
}

static void print_command(char *data, int len)
{
#ifdef DEBUG
  int i;
  int dir = LE_32 (data + 36) >> 16;
  int comm = LE_32 (data + 36) & 0xFFFF;

  lprintf ("----------------------------------------------\n");
  if (dir == 3) {
    lprintf ("send command 0x%02x, %d bytes\n", comm, len);
  } else {
    lprintf ("receive command 0x%02x, %d bytes\n", comm, len);
  }
  lprintf ("  start sequence %08x\n", LE_32 (data +  0));
  lprintf ("  command id     %08x\n", LE_32 (data +  4));
  lprintf ("  length         %8x \n", LE_32 (data +  8));
  lprintf ("  protocol       %08x\n", LE_32 (data + 12));
  lprintf ("  len8           %8x \n", LE_32 (data + 16));
  lprintf ("  sequence #     %08x\n", LE_32 (data + 20));
  lprintf ("  len8  (II)     %8x \n", LE_32 (data + 32));
  lprintf ("  dir | comm     %08x\n", LE_32 (data + 36));
  if (len >= 4)
    lprintf ("  prefix1        %08x\n", LE_32 (data + 40));
  if (len >= 8)
    lprintf ("  prefix2        %08x\n", LE_32 (data + 44));

  for (i = (CMD_HEADER_LEN + CMD_PREFIX_LEN); i < (CMD_HEADER_LEN + CMD_PREFIX_LEN + len); i += 1) {
    unsigned char c = data[i];
    
    if ((c >= 32) && (c < 128))
      lprintf ("%c", c);
    else
      lprintf (" %02x ", c);
    
  }
  if (len > CMD_HEADER_LEN)
    lprintf ("\n");
  lprintf ("----------------------------------------------\n");
#endif
}  



static int send_command (mms_io_t *io, mms_t *this, int command,
                         uint32_t prefix1, uint32_t prefix2,
                         int length) {
  int    len8;
  off_t  n;
  mms_buffer_t command_buffer;

  len8 = (length + 7) / 8;

  this->scmd_len = 0;

  mms_buffer_init(&command_buffer, this->scmd);
  mms_buffer_put_32 (&command_buffer, 0x00000001);   /* start sequence */
  mms_buffer_put_32 (&command_buffer, 0xB00BFACE);   /* #-)) */
  mms_buffer_put_32 (&command_buffer, len8 * 8 + 32);
  mms_buffer_put_32 (&command_buffer, 0x20534d4d);   /* protocol type "MMS " */
  mms_buffer_put_32 (&command_buffer, len8 + 4);
  mms_buffer_put_32 (&command_buffer, this->seq_num);
  this->seq_num++;
  mms_buffer_put_32 (&command_buffer, 0x0);          /* timestamp */
  mms_buffer_put_32 (&command_buffer, 0x0);
  mms_buffer_put_32 (&command_buffer, len8 + 2);
  mms_buffer_put_32 (&command_buffer, 0x00030000 | command); /* dir | command */
  /* end of the 40 byte command header */
  
  mms_buffer_put_32 (&command_buffer, prefix1);
  mms_buffer_put_32 (&command_buffer, prefix2);

  if (length & 7)
          memset(this->scmd + length + CMD_HEADER_LEN + CMD_PREFIX_LEN, 0, 8 - (length & 7));

  n = io_write(io,  this->s, this->scmd, len8 * 8 + CMD_HEADER_LEN + CMD_PREFIX_LEN);
  if (n != (len8 * 8 + CMD_HEADER_LEN + CMD_PREFIX_LEN)) {
    return 0;
  }

  print_command (this->scmd, length);

  return 1;
}

#ifdef VV_USE_ICONV
static int string_utf16(iconv_t url_conv, char *dest, char *src, int dest_len)
{
  char *ip = src, *op = dest;
  size_t ip_len = strlen(src);
  size_t op_len = dest_len - 2; /* reserve 2 bytes for 0 termination */

  if (iconv(url_conv, &ip, &ip_len, &op, &op_len) == (size_t)-1) {
    lprintf("Error converting uri to unicode: %s\n", strerror(get_errno()));
    return 0;
  }
  
  /* 0 terminate the string */
  *op++ = 0;
  *op++ = 0;

  return op - dest;
}
#endif

/*
 * return packet type
 */
static int get_packet_header (mms_io_t *io, mms_t *this, mms_packet_header_t *header) {
  size_t len;
  int packet_type;

  header->packet_len     = 0;
  header->packet_seq     = 0;
  header->flags          = 0;
  header->packet_id_type = 0;
  len = io_read(io,  this->s, this->buf, 8);
  this->buf_packet_seq_offset = -1;
  if (len != 8)
    goto error;

  if (LE_32(this->buf + 4) == 0xb00bface) {
    /* command packet */
    header->flags = this->buf[3];
    len = io_read(io,  this->s, this->buf + 8, 4);
    if (len != 4)
      goto error;
    
    header->packet_len = LE_32(this->buf + 8) + 4;
    if (header->packet_len > BUF_SIZE - 12) {
        lprintf("get_packet_header error cmd packet length > bufsize\n");
        header->packet_len = 0;
        return MMS_PACKET_ERR;
    }
    packet_type = MMS_PACKET_COMMAND;
  } else {
    header->packet_seq     = LE_32(this->buf);
    header->packet_id_type = this->buf[4];
    header->flags          = this->buf[5];
    header->packet_len     = (LE_16(this->buf + 6) - 8) & 0xffff;
    if (header->packet_id_type == ASF_HEADER_PACKET_ID_TYPE) {
      packet_type = MMS_PACKET_ASF_HEADER;
    } else {
      packet_type = MMS_PACKET_ASF_PACKET;
    }
  }
  
  return packet_type;
  
error:
  lprintf("error reading packet header\n");
  return MMS_PACKET_ERR;
}


static int get_packet_command (mms_io_t *io, mms_t *this, uint32_t packet_len) {


  int  command = 0;
  size_t len;
  
  len = io_read(io,  this->s, this->buf + 12, packet_len) ;
  //this->buf_packet_seq_offset = -1; // already set in get_packet_header
  if (len != packet_len) {
    lprintf("error reading command packet\n");
    return 0;
  }

  print_command (this->buf, len);
  
  /* check protocol type ("MMS ") */
  if (LE_32(this->buf + 12) != 0x20534D4D) {
    lprintf("unknown protocol type: %c%c%c%c (0x%08X)\n",
            this->buf[12], this->buf[13], this->buf[14], this->buf[15],
            LE_32(this->buf + 12));  
    return 0;
  }

  command = LE_32 (this->buf + 36) & 0xFFFF;
  lprintf("received command = %02x, len: %d\n", command, packet_len);
    
  return command;
}

static int get_answer (mms_io_t *io, mms_t *this) {
  int command = 0;
  mms_packet_header_t header;

  switch (get_packet_header (io, this, &header)) {
    case MMS_PACKET_ERR:
      break;
    case MMS_PACKET_COMMAND:
      command = get_packet_command (io, this, header.packet_len);
      if (command == 0)
        return 0;

      if (command == 0x1b) {
        if (!send_command (io, this, 0x1b, 0, 0, 0)) {
          lprintf("error sending ping reply\n");
          return 0;
        }
        /* FIXME: limit recursion */
        command = get_answer (io, this);
      }
      break;
    case MMS_PACKET_ASF_HEADER:
      lprintf("unexpected asf header packet\n");
      break;
    case MMS_PACKET_ASF_PACKET:
      lprintf("unexpected asf packet\n");
      break;
  }
  
  return command;
}


static int get_asf_header (mms_io_t *io, mms_t *this) {

  off_t len;
  int stop = 0;
  
  this->asf_header_read = 0;
  this->asf_header_len = 0;

  while (!stop) {
    mms_packet_header_t header;
    int command;

    switch (get_packet_header (io, this, &header)) {
      case MMS_PACKET_ERR:
        return 0;
      case MMS_PACKET_COMMAND:
        command = get_packet_command (io, this, header.packet_len);
        if (command == 0)
          return 0;

        if (command == 0x1b) {
          if (!send_command (io, this, 0x1b, 0, 0, 0)) {
            lprintf("error sending ping reply\n");
            return 0;
          }
          command = get_answer (io, this);
        } else {
          lprintf("unexpected command packet\n");
        }
        break;
      case MMS_PACKET_ASF_HEADER:
      case MMS_PACKET_ASF_PACKET:
        if (header.packet_len + this->asf_header_len > ASF_HEADER_SIZE) {
            lprintf("asf packet too large: %d\n", 
                    header.packet_len + this->asf_header_len);
            return 0;
        }
        len = io_read(io,  this->s,
                              this->asf_header + this->asf_header_len, header.packet_len);
        if (len != header.packet_len) {
           lprintf("error reading asf header\n");
           return 0;
        }
        this->asf_header_len += header.packet_len;
        lprintf("header flags: %d\n", header.flags);
        if ((header.flags == 0X08) || (header.flags == 0X0C))
          stop = 1;
        break;
    }
  }
  return 1;
}

static void mms_gen_guid(char guid[]) {
  static char digit[16] = "0123456789ABCDEF";
  int i = 0;

  srand(time(NULL));
  for (i = 0; i < 36; i++) {
    guid[i] = digit[(int) ((16.0*rand())/(RAND_MAX+1.0))];
  }
  guid[8] = '-'; guid[13] = '-'; guid[18] = '-'; guid[23] = '-';
  guid[36] = '\0';
}

const char *status_to_string(int status)
{
  switch (status) {
  case 0x80070003:
    return "Path not found";
  case 0x80070005:
    return "Access Denied";
  default:
    return "Unknown";
  }
}

/*
 * return 0 on error
 */
int static mms_choose_best_streams(mms_io_t *io, mms_t *this) {
  int     i;
  int     video_stream = -1;
  int     audio_stream = -1;
  int     res;

  /* command 0x33 */
  mms_get_best_stream_ids(this, &audio_stream, &video_stream);
  memset (this->scmd_body, 0, 40);
  /* For stream 0, the 0xffff + stream 0 id gets added to the buffer as
     prefix2, but we still need to add the enable / disable command */
  if ((this->streams[0].stream_id == audio_stream) ||
      (this->streams[0].stream_id == video_stream)) {
    lprintf("selecting stream %d\n", this->streams[0].stream_id);
    this->scmd_body[0] = 0x00;
    this->scmd_body[1] = 0x00;
  } else {
    lprintf("disabling stream %d\n", this->streams[0].stream_id);
    this->scmd_body[0] = 0x02;
    this->scmd_body[1] = 0x00;
  }

  for (i = 1; i < this->num_stream_ids; i++) {
    this->scmd_body [ (i - 1) * 6 + 2 ] = 0xFF;
    this->scmd_body [ (i - 1) * 6 + 3 ] = 0xFF;
    this->scmd_body [ (i - 1) * 6 + 4 ] = this->streams[i].stream_id ;
    this->scmd_body [ (i - 1) * 6 + 5 ] = this->streams[i].stream_id >> 8;
    if ((this->streams[i].stream_id == audio_stream) ||
        (this->streams[i].stream_id == video_stream)) {
      lprintf("selecting stream %d\n", this->streams[i].stream_id);
      this->scmd_body [ (i - 1) * 6 + 6 ] = 0x00;
      this->scmd_body [ (i - 1) * 6 + 7 ] = 0x00;
    } else {
      lprintf("disabling stream %d\n", this->streams[i].stream_id);
      this->scmd_body [ (i - 1) * 6 + 6 ] = 0x02;
      this->scmd_body [ (i - 1) * 6 + 7 ] = 0x00;
    }      
  }

  mms_disable_disabled_streams_in_asf_header(this, audio_stream, video_stream);

  lprintf("send command 0x33\n");
  if (!send_command (io, this, 0x33, this->num_stream_ids, 
                     0xFFFF | this->streams[0].stream_id << 16, 
                     (this->num_stream_ids - 1) * 6 + 2)) {
    lprintf("mms_choose_best_streams failed\n");
    return 0;
  }

  if ((res = get_answer (io, this)) != 0x21) {
    lprintf("unexpected response: %02x (0x21)\n", res);
    return 0;
  }

  res = LE_32(this->buf + 40);
  if (res != 0) {
    lprintf("error answer 0x21 status: %08x (%s)\n",
            res, status_to_string(res));
    return 0;
  }

  return 1;
}

/*
 * TODO: error messages
 *       network timing request
 */
/* FIXME: got somewhat broken during xine_stream_t->(void*) conversion */
mms_t *mms_connect (mms_io_t *io, void *data, const char *url, int bandwidth) {
#ifdef VV_USE_ICONV
  iconv_t url_conv = (iconv_t)-1;
#endif
  mms_t  *this;
  int     res;
  uint32_t openid;
  mms_buffer_t command_buffer;
  
  if (!url)
    return NULL;

#ifdef _WIN32
  if (!mms_internal_winsock_load())
    return NULL;
#endif

  /* FIXME: needs proper error-signalling work */
  this = (mms_t*)calloc(1, sizeof(mms_t));

  this->url             = strdup (url);
  this->s               = -1;
  this->scmd_body       = this->scmd + CMD_HEADER_LEN + CMD_PREFIX_LEN;
  this->need_discont    = 1;
  this->buf_packet_seq_offset = -1;
  this->bandwidth       = bandwidth;

  this->guri = gnet_uri_new(this->url);
  if(!this->guri) {
    lprintf("invalid url\n");
    goto fail;
  }

  /* MMS wants unescaped (so not percent coded) strings */
  gnet_uri_unescape(this->guri);

  this->proto = this->guri->scheme;
  this->user = this->guri->user;
  this->connect_host = this->guri->hostname;
  this->connect_port = this->guri->port;
  this->password = this->guri->passwd;
  this->uri = gnet_mms_helper(this->guri, 0);

  if(!this->uri)
        goto fail;

  if (!mms_valid_proto(this->proto)) {
    lprintf("unsupported protocol: %s\n", this->proto);
    goto fail;
  }
  
  if (mms_tcp_connect(io, this)) {
    goto fail;
  }
  
#ifdef VV_USE_ICONV
  url_conv = iconv_open("UTF-16LE", "UTF-8");
  if (url_conv == (iconv_t)-1) {
    lprintf("could not get iconv handle to convert url to unicode\n");
    goto fail;
  }
#endif

  /*
   * let the negotiations begin...
   */

  /* command 0x1 */
  lprintf("send command 0x01\n");
  mms_buffer_init(&command_buffer, this->scmd_body);
  mms_buffer_put_32 (&command_buffer, 0x0003001C);
  mms_gen_guid(this->guid);
  sprintf(this->str, "NSPlayer/7.0.0.1956; {%s}; Host: %s", this->guid,
          this->connect_host);
#ifdef VV_USE_ICONV
  res = string_utf16(url_conv, this->scmd_body + command_buffer.pos, this->str,
                     CMD_BODY_LEN - command_buffer.pos);
  if(!res)
    goto fail;
#else
  strncpy(this->scmd_body + command_buffer.pos, this->str, CMD_BODY_LEN - command_buffer.pos);
#endif

  if (!send_command(io, this, 1, 0, 0x0004000b, command_buffer.pos + res)) {
    lprintf("failed to send command 0x01\n");
    goto fail;
  }
  
  if ((res = get_answer (io, this)) != 0x01) {
    lprintf("unexpected response: %02x (0x01)\n", res);
    goto fail;
  }

  res = LE_32(this->buf + 40);
  if (res != 0) {
    lprintf("error answer 0x01 status: %08x (%s)\n",
            res, status_to_string(res));
    goto fail;
  }

  /* TODO: insert network timing request here */
  /* command 0x2 */
  lprintf("send command 0x02\n");
  mms_buffer_init(&command_buffer, this->scmd_body);
  mms_buffer_put_32 (&command_buffer, 0x00000000);
  mms_buffer_put_32 (&command_buffer, 0x00989680);
  mms_buffer_put_32 (&command_buffer, 0x00000002);
#ifdef VV_USE_ICONV
  res = string_utf16(url_conv, this->scmd_body + command_buffer.pos,
                     "\\\\192.168.0.129\\TCP\\1037",
                     CMD_BODY_LEN - command_buffer.pos);
  if(!res)
    goto fail;
#else
  strncpy(this->scmd_body + command_buffer.pos, "\\\\192.168.0.129\\TCP\\1037", CMD_BODY_LEN - command_buffer.pos);
#endif

  if (!send_command(io, this, 2, 0, 0xffffffff, command_buffer.pos + res)) {
    lprintf("failed to send command 0x02\n");
    goto fail;
  }

  switch (res = get_answer (io, this)) {
    case 0x02:
      /* protocol accepted */
      break;
    case 0x03:
      lprintf("protocol failed\n");
      goto fail;
    default:
      lprintf("unexpected response: %02x (0x02 or 0x03)\n", res);
      goto fail;
  }

  res = LE_32(this->buf + 40);
  if (res != 0) {
    lprintf("error answer 0x02 status: %08x (%s)\n",
            res, status_to_string(res));
    goto fail;
  }

  /* command 0x5 */
  {
    mms_buffer_t command_buffer;
    
    lprintf("send command 0x05\n");
    mms_buffer_init(&command_buffer, this->scmd_body);
    mms_buffer_put_32 (&command_buffer, 0x00000000); /* ?? */
    mms_buffer_put_32 (&command_buffer, 0x00000000); /* ?? */

#ifdef VV_USE_ICONV
    res = string_utf16(url_conv, this->scmd_body + command_buffer.pos,
                       this->uri, CMD_BODY_LEN - command_buffer.pos);
    if(!res)
      goto fail;
#else
    strncpy(this->scmd_body + command_buffer.pos, this->uri, CMD_BODY_LEN - command_buffer.pos);
#endif

    if (!send_command(io, this, 5, 1, 0, command_buffer.pos + res)) {
      lprintf("failed to send command 0x05\n");
      goto fail;
    }
  }
  
  switch (res = get_answer (io, this)) {
    case 0x06:
      {
        int xx, yy;
        /* no authentication required */
        openid = LE_32(this->buf + 48);
      
        /* Warning: sdp is not right here */
        xx = this->buf[62];
        yy = this->buf[63];
        this->live_flag = ((xx == 0) && ((yy & 0xf) == 2));
        this->seekable = !this->live_flag;
        lprintf("openid=%d, live: live_flag=%d, xx=%d, yy=%d\n", openid, this->live_flag, xx, yy);
      }
      break;
    case 0x1A:
      /* authentication request, not yet supported */
      lprintf("authentication request, not yet supported\n");
      goto fail;
      break;
    default:
      lprintf("unexpected response: %02x (0x06 or 0x1A)\n", res);
      goto fail;
  }

  res = LE_32(this->buf + 40);
  if (res != 0) {
    lprintf("error answer 0x06 status: %08x (%s)\n",
            res, status_to_string(res));
    goto fail;
  }

  /* command 0x15 */
  lprintf("send command 0x15\n");
  {
    mms_buffer_t command_buffer;
    mms_buffer_init(&command_buffer, this->scmd_body);
    mms_buffer_put_32 (&command_buffer, 0x00000000);                  /* ?? */
    mms_buffer_put_32 (&command_buffer, 0x00008000);                  /* ?? */
    mms_buffer_put_32 (&command_buffer, 0xFFFFFFFF);                  /* ?? */
    mms_buffer_put_32 (&command_buffer, 0x00000000);                  /* ?? */
    mms_buffer_put_32 (&command_buffer, 0x00000000);                  /* ?? */
    mms_buffer_put_32 (&command_buffer, 0x00000000);                  /* ?? */
    mms_buffer_put_32 (&command_buffer, 0x00000000);                  /* ?? */
    mms_buffer_put_32 (&command_buffer, 0x40AC2000);                  /* ?? */
    mms_buffer_put_32 (&command_buffer, ASF_HEADER_PACKET_ID_TYPE);   /* Header Packet ID type */
    mms_buffer_put_32 (&command_buffer, 0x00000000);                  /* ?? */
    if (!send_command (io, this, 0x15, openid, 0, command_buffer.pos)) {
      lprintf("failed to send command 0x15\n");
      goto fail;
    }
  }
  
  if ((res = get_answer (io, this)) != 0x11) {
    lprintf("unexpected response: %02x (0x11)\n", res);
    goto fail;
  }

  res = LE_32(this->buf + 40);
  if (res != 0) {
    lprintf("error answer 0x11 status: %08x (%s)\n",
            res, status_to_string(res));
    goto fail;
  }

  this->num_stream_ids = 0;

  if (!get_asf_header (io, this))
    goto fail;

  interp_asf_header (this);
  if (!this->asf_packet_len || !this->num_stream_ids)
    goto fail;

  if (!mms_choose_best_streams(io, this)) {
    lprintf("mms_choose_best_streams failed\n");
    goto fail;
  }

  /* command 0x07 */
  this->packet_id_type = ASF_MEDIA_PACKET_ID_TYPE;
  {
    mms_buffer_t command_buffer;
    mms_buffer_init(&command_buffer, this->scmd_body);
    mms_buffer_put_32 (&command_buffer, 0x00000000);                  /* 64 byte float timestamp */
    mms_buffer_put_32 (&command_buffer, 0x00000000);                  
    mms_buffer_put_32 (&command_buffer, 0xFFFFFFFF);                  /* ?? */
    mms_buffer_put_32 (&command_buffer, 0xFFFFFFFF);                  /* first packet sequence */
    mms_buffer_put_8  (&command_buffer, 0xFF);                        /* max stream time limit (3 bytes) */
    mms_buffer_put_8  (&command_buffer, 0xFF);
    mms_buffer_put_8  (&command_buffer, 0xFF);
    mms_buffer_put_8  (&command_buffer, 0x00);                        /* stream time limit flag */
    mms_buffer_put_32 (&command_buffer, this->packet_id_type);    /* asf media packet id type */
    if (!send_command (io, this, 0x07, 1, 0x0001FFFF, command_buffer.pos)) {
      lprintf("failed to send command 0x07\n");
      goto fail;
    }
  }

#ifdef VV_USE_ICONV
  iconv_close(url_conv);
#endif
  lprintf("connect: passed\n");
 
  return this;

fail:
  mms_close(this);

#ifdef VV_USE_ICONV
  if (url_conv != (iconv_t)-1)
    iconv_close(url_conv);
#endif

  return NULL;
}

static int get_media_packet (mms_io_t *io, mms_t *this) {
  mms_packet_header_t header;
  off_t len;
  
  switch (get_packet_header (io, this, &header)) {
    case MMS_PACKET_ERR:
      return 0;
    
    case MMS_PACKET_COMMAND:
      {
        int command;
        command = get_packet_command (io, this, header.packet_len);
      
        switch (command) {
          case 0:
            return 0;

          case 0x1e:
            {
              uint32_t error_code;

              /* Warning: sdp is incomplete. Do not stop if error_code==1 */
              error_code = LE_32(this->buf + CMD_HEADER_LEN);
              lprintf("End of the current stream. Continue=%d\n", error_code);

              if (error_code == 0) {
                this->eos = 1;
                return 0;
              }
              
            }
            break;
  
          case 0x20:
            {
              lprintf("new stream.\n");
              /* asf header */
              if (!get_asf_header (io, this)) {
                lprintf("failed to read new ASF header\n");
                return 0;
              }

              interp_asf_header (this);
              if (!this->asf_packet_len || !this->num_stream_ids)
                return 0;

              if (!mms_choose_best_streams(io, this))
                return 0;

              /* send command 0x07 */
              /* TODO: ugly */
              /* command 0x07 */
              {
                mms_buffer_t command_buffer;
                mms_buffer_init(&command_buffer, this->scmd_body);
                mms_buffer_put_32 (&command_buffer, 0x00000000);                  /* 64 byte float timestamp */
                mms_buffer_put_32 (&command_buffer, 0x00000000);                  
                mms_buffer_put_32 (&command_buffer, 0xFFFFFFFF);                  /* ?? */
                mms_buffer_put_32 (&command_buffer, 0xFFFFFFFF);                  /* first packet sequence */
                mms_buffer_put_8  (&command_buffer, 0xFF);                        /* max stream time limit (3 bytes) */
                mms_buffer_put_8  (&command_buffer, 0xFF);
                mms_buffer_put_8  (&command_buffer, 0xFF);
                mms_buffer_put_8  (&command_buffer, 0x00);                        /* stream time limit flag */
                mms_buffer_put_32 (&command_buffer, ASF_MEDIA_PACKET_ID_TYPE);    /* asf media packet id type */
                if (!send_command (io, this, 0x07, 1, 0x0001FFFF, command_buffer.pos)) {
                  lprintf("failed to send command 0x07\n");
                  return 0;
                }
              }
              this->current_pos = 0;
              
              /* I don't know if this ever happens with none live (and thus
                 seekable streams), but I do know that if it happens all bets
                 with regards to seeking are off */
              this->seekable = 0;
            }
            break;

          case 0x1b:
            {
              if (!send_command (io, this, 0x1b, 0, 0, 0)) {
                lprintf("error sending ping reply\n");
                return 0;
              }
            }
            break;
          
          case 0x05:
            break;
  
          default:
            lprintf("unexpected mms command %02x\n", command);
        }
        this->buf_size = 0;
      }
      break;

    case MMS_PACKET_ASF_HEADER:
      lprintf("unexpected asf header packet\n");
      this->buf_size = 0;
      break;

    case MMS_PACKET_ASF_PACKET:
      {
        /* media packet */

        /* FIXME: probably needs some more sophisticated logic, but
           until we do seeking, this should work */
        if(this->need_discont &&
           header.packet_id_type == ASF_MEDIA_PACKET_ID_TYPE)
        {
          this->need_discont = 0;
          this->start_packet_seq = header.packet_seq;
        }
        
        if (header.packet_len > this->asf_packet_len) {
          lprintf("invalid asf packet len: %d bytes\n", header.packet_len);
          return 0;
        }
    
        /* simulate a seek */
        this->current_pos = (off_t)this->asf_header_len +
          ((off_t)header.packet_seq - this->start_packet_seq) * (off_t)this->asf_packet_len;

        len = io_read(io,  this->s, this->buf, header.packet_len);
        if (len != header.packet_len) {
          lprintf("error reading asf packet\n");
          return 0;
        }

        /* explicit padding with 0 */
        memset(this->buf + header.packet_len, 0, this->asf_packet_len - header.packet_len);
        if (header.packet_id_type == this->packet_id_type) {
          this->buf_size = this->asf_packet_len;
          this->buf_packet_seq_offset =
            header.packet_seq - this->start_packet_seq;
        } else {
          this->buf_size = 0;
          // Don't set this packet sequence for reuse in seek(), since the
          // subsequence packet may be discontinued.
          //this->buf_packet_seq_offset = header.packet_seq;
          // already set to -1 in get_packet_header
          //this->buf_packet_seq_offset = -1;
        }
      }
      break;
  }

  return 1;
}


int mms_peek_header (mms_t *this, char *data, int maxsize) {

  int len;

  len = (this->asf_header_len < maxsize) ? this->asf_header_len : maxsize;

  memcpy(data, this->asf_header, len);
  return len;
}

int mms_read (mms_io_t *io, mms_t *this, char *data, int len) {
  int total;

  total = 0;
  while (total < len && !this->eos) {

    if (this->asf_header_read < this->asf_header_len) {
      int n, bytes_left;

      bytes_left = this->asf_header_len - this->asf_header_read ;

      if ((len - total) < bytes_left)
        n = len-total;
      else
        n = bytes_left;

      memcpy (&data[total], &this->asf_header[this->asf_header_read], n);

      this->asf_header_read += n;
      total += n;
      this->current_pos += n;
    } else {

      int n, bytes_left;

      bytes_left = this->buf_size - this->buf_read;
      if (bytes_left == 0) {
        this->buf_size = this->buf_read = 0;
        if (!get_media_packet (io, this)) {
          lprintf("get_media_packet failed\n");
          return total;
        }
        bytes_left = this->buf_size;
      }

      if ((len - total) < bytes_left)
        n = len - total;
      else
        n = bytes_left;

      memcpy (&data[total], &this->buf[this->buf_read], n);

      this->buf_read += n;
      total += n;
      this->current_pos += n;
    }
  }
  return total;
}

// To be inline function?
static int mms_request_data_packet (mms_io_t *io, mms_t *this,
  double time_sec, unsigned long first_packet, unsigned long time_msec_limit) {
  /* command 0x07 */
  {
    mms_buffer_t command_buffer;
    //mms_buffer_init(&command_buffer, this->scmd_body);
    //mms_buffer_put_32 (&command_buffer, 0x00000000);                  /* 64 byte float timestamp */
    //mms_buffer_put_32 (&command_buffer, 0x00000000);                  
    memcpy(this->scmd_body, &time_sec, 8);
    mms_buffer_init(&command_buffer, this->scmd_body+8);
    mms_buffer_put_32 (&command_buffer, 0xFFFFFFFF);                  /* ?? */
    //mms_buffer_put_32 (&command_buffer, 0xFFFFFFFF);                  /* first packet sequence */
    mms_buffer_put_32 (&command_buffer, first_packet);                  /* first packet sequence */
    //mms_buffer_put_8  (&command_buffer, 0xFF);                        /* max stream time limit (3 bytes) */
    //mms_buffer_put_8  (&command_buffer, 0xFF);
    //mms_buffer_put_8  (&command_buffer, 0xFF);
    //mms_buffer_put_8  (&command_buffer, 0x00);                        /* stream time limit flag */
    mms_buffer_put_32 (&command_buffer, time_msec_limit & 0x00FFFFFF);/* max stream time limit (3 bytes) */
    mms_buffer_put_32 (&command_buffer, this->packet_id_type);    /* asf media packet id type */
    if (!send_command (io, this, 0x07, 1, 0x0001FFFF, 8+command_buffer.pos)) {
      lprintf("failed to send command 0x07\n");
      return 0;
    }
  }
  /* TODO: adjust current_pos, considering asf_header_read */
  return 1;
}

int mms_request_time_seek (mms_io_t *io, mms_t *this, double time_sec) {
  if (++this->packet_id_type <= ASF_MEDIA_PACKET_ID_TYPE)
    this->packet_id_type = ASF_MEDIA_PACKET_ID_TYPE+1;
  //return mms_request_data_packet (io, this, time_sec, 0xFFFFFFFF, 0x00FFFFFF);
  // also adjust time by preroll
  return mms_request_data_packet (io, this,
                                  time_sec+(double)(this->preroll)/1000,
                                  0xFFFFFFFF, 0x00FFFFFF);
}

// set current_pos to the first byte of the requested packet by peeking at
// the first packet.
// To be inline function?
static int peek_and_set_pos (mms_io_t *io, mms_t *this) {
  uint8_t saved_buf[BUF_SIZE];
  int     saved_buf_size;
  off_t   saved_buf_packet_seq_offset;
  // save buf and buf_size that may be changed in get_media_packet()
  memcpy(saved_buf, this->buf, this->buf_size);
  saved_buf_size = this->buf_size;
  saved_buf_packet_seq_offset = this->buf_packet_seq_offset;
  //this->buf_size = this->buf_read = 0; // reset buf, only if success peeking
  this->buf_size = 0;
  while (!this->eos) {
    // get_media_packet() will set current_pos if data packet is read.
    if (!get_media_packet (io, this)) {
      lprintf("get_media_packet failed\n");
      // restore buf and buf_size that may be changed in get_media_packet()
      memcpy(this->buf, saved_buf, saved_buf_size);
      this->buf_size = saved_buf_size;
      this->buf_packet_seq_offset = saved_buf_packet_seq_offset;
      return 0;
    }
    if (this->buf_size > 0) break;
  }
  // flush header and reset buf_read, only if success peeking
  this->asf_header_read = this->asf_header_len;
  this->buf_read = 0;
  return 1;
  //return this->current_pos;
}

// send time seek request, and update current_pos corresponding to the next
// requested packet
// Note that, the current_pos will always does not less than asf_header_len
int mms_time_seek (mms_io_t *io, mms_t *this, double time_sec) {
  if (!this->seekable)
    return 0;

  if (!mms_request_time_seek (io, this, time_sec)) return 0;
  return peek_and_set_pos (io, this);
}

// http://sdp.ppona.com/zipfiles/MMSprotocol_pdf.zip said that, this
// packet_seq value make no difference in version 9 servers.
// But from my experiment with
// mms://202.142.200.130/tltk/56k/tltkD2006-08-08ID-7209.wmv and
// mms://202.142.200.130/tltk/56k/tltkD2006-09-01ID-7467.wmv (the url may valid
// in only 2-3 months) whose server is version 9, it does response and return
// the requested packet.
int mms_request_packet_seek (mms_io_t *io, mms_t *this,
                             unsigned long packet_seq) {
  if (++this->packet_id_type <= ASF_MEDIA_PACKET_ID_TYPE)
    this->packet_id_type = ASF_MEDIA_PACKET_ID_TYPE+1;
  return mms_request_data_packet (io, this, 0, packet_seq, 0x00FFFFFF);
}

// send packet seek request, and update current_pos corresponding to the next
// requested packet
// Note that, the current_pos will always does not less than asf_header_len
// Not export this function.  Let user use mms_seek() instead?
static int mms_packet_seek (mms_io_t *io, mms_t *this,
                            unsigned long packet_seq) {
  if (!mms_request_packet_seek (io, this, packet_seq)) return 0;
  return peek_and_set_pos (io, this);
}

/*
TODO: To use this table to calculate buf_packet_seq_offset rather than store
and retrieve it from this->buf_packet_seq_offset?
current_packet_seq == (current_pos - asf_header_len) / asf_packet_len
current_packet_seq == -1 if current_pos < asf_header_len
buf_packet_seq_offset indicating which packet sequence are residing in the buf.
Possible status after read(), "last" means last value or unchange.
current_packet_seq | buf_read       | buf_size  | buf_packet_seq_offset
-------------------+----------------+-----------+---------------
<= 0               | 0 (last)       | 0 (last)  | none
<= 0               | 0 (last)       | 0 (last)  | eos at #0
<= 0               | 0 (last)       | 0 (last)  | eos at > #0
<= 0               | 0 (last)       | > 0 (last)| #0
<= 0               | buf_size (last)| > 0 (last)| > #0
> 0                | 0              | 0         | eos at current_packet_seq
> 0                | 0(never happen)| > 0       | (never happen)
> 0                | buf_size       | > 0       | current_packet_seq-1
*/
// TODO: How to handle seek() in multi stream source?
// The stream that follows 0x20 ("new stream") command.
off_t mms_seek (mms_io_t *io, mms_t *this, off_t offset, int origin) {
  off_t dest;
  off_t dest_packet_seq;
  //off_t buf_packet_seq_offset;
  
  if (!this->seekable)
    return this->current_pos;
  
  switch (origin) {
    case SEEK_SET:
      dest = offset;
      break;
    case SEEK_CUR:
      dest = this->current_pos + offset;
      break;
    case SEEK_END:
      //if (this->asf_num_packets == 0) {
      //  //printf ("input_mms: unknown end position in seek!\n");
      //  return this->current_pos;
      //}
      dest = mms_get_length (this) + offset;
    default:
      lprintf("unknown origin in seek!\n");
      return this->current_pos;
  }

  dest_packet_seq = dest - this->asf_header_len;
  //if (dest_packet_seq > 0) dest_packet_seq /= this->asf_packet_len;
  dest_packet_seq = dest_packet_seq >= 0 ?
    dest_packet_seq / this->asf_packet_len : -1;
#if 0
  // buf_packet_seq_offset will identify which packet sequence are residing in
  // the buf.
#if 1 /* To show both of the alternate styles :D */
  buf_packet_seq_offset = this->current_pos - this->asf_header_len;
  //if (buf_packet_seq_offset > 0) buf_packet_seq_offset /= this->asf_packet_len;
  buf_packet_seq_offset = buf_packet_seq_offset >= 0 ?
    buf_packet_seq_offset / this->asf_packet_len : -1;
  // Note: buf_read == buf_size == 0 may means that it is eos,
  //       eos means that the packet has been peek'ed.
  if (this->buf_read >= this->buf_size && this->buf_size > 0 &&
      buf_packet_seq_offset >= 0 ||
      // assuming packet not peek'ed in the following condition
      /*this->buf_read >= this->buf_size && */this->buf_size == 0 &&
      buf_packet_seq_offset == 0)
    // The buf is all read but the packet has not been peek'ed.
    --buf_packet_seq_offset;
#else
  buf_packet_seq_offset = this->current_pos - this->asf_header_len - 1;
  //if (buf_packet_seq_offset > 0) buf_packet_seq_offset /= this->asf_packet_len;
  buf_packet_seq_offset = buf_packet_seq_offset >= 0 ?
    buf_packet_seq_offset / this->asf_packet_len : -1;
  // Note: buf_read == buf_size == 0 may means that it is eos,
  //       eos means that the packet has been peek'ed.
  if (this->buf_read == 0/* && buf_packet_seq_offset >= 0*/)
    // Since the packet has just been peek'ed.
    ++buf_packet_seq_offset;
#endif
#endif

  if (dest_packet_seq < 0) {
    if (this->buf_packet_seq_offset > 0) {
      if (!mms_request_packet_seek (io, this, 0xFFFFFFFF))
        return this->current_pos;
#if 1
      // clear buf
      this->buf_read = this->buf_size = 0;
      this->buf_packet_seq_offset = -1;
    } else {
#else
      // clear buf
      this->buf_read = this->buf_size;
      // Set this packet sequence not to be reused, since the subsequence
      // packet may be discontinued.
      this->buf_packet_seq_offset = -1;
    // don't reset buf_read if buf_packet_seq_offset < 0, since the previous
    // buf may not be cleared.
    } else if (this->buf_packet_seq_offset == 0) {
#endif
      // reset buf_read
      this->buf_read = 0;
    }
    this->asf_header_read = dest;
    return this->current_pos = dest;
  }
  // dest_packet_seq >= 0
  if (this->asf_num_packets && dest == this->asf_header_len +
      this->asf_num_packets*this->asf_packet_len) {
    // Requesting the packet beyond the last packet, can cause the server to
    // not return any packet or any eos command.  This can cause
    // mms_packet_seek() to hang.
    // This is to allow seeking at end of stream, and avoid hanging.
    --dest_packet_seq;
  }
  if (dest_packet_seq != this->buf_packet_seq_offset) {
    if (this->asf_num_packets && dest_packet_seq >= this->asf_num_packets) {
      // Do not seek beyond the last packet.
      return this->current_pos;
    }
    if (!mms_packet_seek (io, this, this->start_packet_seq + dest_packet_seq))
      return this->current_pos;
    // Check if current_pos is correct.
    // This can happen if the server ignore packet seek command.
    // If so, just return unchanged current_pos, rather than trying to
    // mms_read() to reach the destination pos.
    // It should let the caller to decide to choose the alternate method, such
    // as, mms_time_seek() and/or mms_read() until the destination pos is
    // reached.
    if (dest_packet_seq != this->buf_packet_seq_offset)
      return this->current_pos;
    // This has already been set in mms_packet_seek().
    //if (current_packet_seq < 0)
    //  this->asf_header_read = this->asf_header_len;
    //this->asf_header_read = this->asf_header_len;
  }
  // eos is reached ?
  //if (this->buf_size <= 0) return this->current_pos;
  //this->buf_read = (dest - this->asf_header_len) % this->asf_packet_len;
  this->buf_read = dest -
    (this->asf_header_len + dest_packet_seq*this->asf_packet_len);
  // will never happen ?
  //if (this->buf_size <= this->buf_read) return this->current_pos;
  return this->current_pos = dest;
}


void mms_close (mms_t *this) {

  if (this == NULL)
    return;
  if (this->s != -1)
    closesocket (this->s);
  if (this->url)
    free(this->url);
  if (this->guri)
    gnet_uri_delete(this->guri);
  if (this->uri)
    free(this->uri);

  free (this);
}

double mms_get_time_length (mms_t *this) {
  return (double)(this->time_len) / 1e7;
}

uint64_t mms_get_raw_time_length (mms_t *this) {
  return this->time_len;
}

uint64_t mms_get_file_time (mms_t *this) {
  return this->file_time;
}

uint32_t mms_get_length (mms_t *this) {
  /* we could / should return this->file_len here, but usually this->file_len
     is longer then the calculation below, as usually an asf file contains an
     asf index object after the data stream. However since we do not have a
     (known) way to get to this index object through mms, we return a
     calculated size of what we can get to when we know. */
  if (this->asf_num_packets)
    return this->asf_header_len + this->asf_num_packets*this->asf_packet_len;
  else
    return this->file_len;
}

off_t mms_get_current_pos (mms_t *this) {
  return this->current_pos;
}

uint32_t mms_get_asf_header_len (mms_t *this) {
  return this->asf_header_len;
}

uint64_t mms_get_asf_packet_len (mms_t *this) {
  return this->asf_packet_len;
}

int mms_get_seekable (mms_t *this) {
  return this->seekable;
}
