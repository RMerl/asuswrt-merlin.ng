/*
 * Copyright (C) 2002-2003 the xine project
 *
 * This file is part of xine, a free video player.
 *
 * xine is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
 * $Id: mmsh.c,v 1.16 2007/12/11 20:50:43 jwrdegoede Exp $
 *
 * MMS over HTTP protocol
 *   written by Thibaut Mattern
 *   based on mms.c and specs from avifile
 *   (http://avifile.sourceforge.net/asf-1.0.htm)
 *
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
#include <stdlib.h>
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
#include <assert.h>

#define lprintf(...) \
    if (getenv("LIBMMS_DEBUG")) \
        fprintf(stderr, "mmsh: " __VA_ARGS__)

/* cheat a bit and call ourselves mms.c to keep the code in mmsio.h clean */
#define __MMS_C__
#define TYPE mmsh_t

#include "bswap.h"
#include "mmsh.h"
#include "asfheader.h"
#include "uri.h"
#include "mms-common.h"

/* #define USERAGENT "User-Agent: NSPlayer/7.1.0.3055\r\n" */
#define USERAGENT "User-Agent: NSPlayer/4.1.0.3856\r\n"
#define CLIENTGUID "Pragma: xClientGUID={c77e7400-738a-11d2-9add-0020af0a3278}\r\n"

#define DEFAULT_PORT               80
#define MMSH_UNKNOWN                0
#define MMSH_SEEKABLE               1
#define MMSH_LIVE                   2

#define CHUNK_HEADER_LENGTH         4
#define EXT_HEADER_LENGTH           8
#define CHUNK_TYPE_RESET       0x4324
#define CHUNK_TYPE_DATA        0x4424
#define CHUNK_TYPE_END         0x4524
#define CHUNK_TYPE_ASF_HEADER  0x4824

#define SCRATCH_SIZE             1024

#define MMSH_SUCCESS 0
#define MMSH_ERROR 1
#define EOS 2
#define GOT_HEADER_N_DATA 3

static const char* mmsh_FirstRequest =
    "GET %s HTTP/1.0\r\n"
    "Accept: */*\r\n"
    USERAGENT
    "Host: %s:%d\r\n"
    "Pragma: no-cache,rate=1.000000,stream-time=0,stream-offset=0:0,request-context=%u,max-duration=0\r\n"
    CLIENTGUID
    "Connection: Close\r\n\r\n";

static const char* mmsh_SeekableRequest =
    "GET %s HTTP/1.0\r\n"
    "Accept: */*\r\n"
    USERAGENT
    "Host: %s:%d\r\n"
    "Pragma: no-cache,rate=1.000000,stream-time=%u,stream-offset=%u:%u,request-context=%u,max-duration=%u\r\n"
    CLIENTGUID
    "Pragma: xPlayStrm=1\r\n"
    "Pragma: stream-switch-count=%d\r\n"
    "Pragma: stream-switch-entry=%s\r\n" /*  ffff:1:0 ffff:2:0 */
    "Connection: Close\r\n\r\n";

static const char* mmsh_LiveRequest =
    "GET %s HTTP/1.0\r\n"
    "Accept: */*\r\n"
    USERAGENT
    "Host: %s:%d\r\n"
    "Pragma: no-cache,rate=1.000000,request-context=%u\r\n"
    "Pragma: xPlayStrm=1\r\n"
    CLIENTGUID
    "Pragma: stream-switch-count=%d\r\n"
    "Pragma: stream-switch-entry=%s\r\n"
    "Connection: Close\r\n\r\n";

#if 0
/* Unused requests */
static const char* mmsh_PostRequest =
    "POST %s HTTP/1.0\r\n"
    "Accept: */*\r\n"
    USERAGENT
    "Host: %s\r\n"
    "Pragma: client-id=%u\r\n"
/*    "Pragma: log-line=no-cache,rate=1.000000,stream-time=%u,stream-offset=%u:%u,request-context=2,max-duration=%u\r\n" */
    "Pragma: Content-Length: 0\r\n"
    CLIENTGUID
    "\r\n";

static const char* mmsh_RangeRequest =
    "GET %s HTTP/1.0\r\n"
    "Accept: */*\r\n"
    USERAGENT
    "Host: %s:%d\r\n"
    "Range: bytes=%Lu-\r\n"
    CLIENTGUID
    "Connection: Close\r\n\r\n";
#endif

/* 
 * mmsh specific types 
 */

struct mmsh_s {
  int           s;

  /* url parsing */
  char         *url;
  char         *proxy_url;
  char         *proto;
  char         *connect_host;
  int           connect_port;
  char         *http_host;
  int           http_port;
  int           http_request_number;
  char         *proxy_user;
  char         *proxy_password;
  char         *host_user;
  char         *host_password;
  char         *uri;

  char          str[SCRATCH_SIZE]; /* scratch buffer to built strings */

  int           stream_type;  /* seekable or broadcast */
  
  /* receive buffer */
  
  /* chunk */
  uint16_t      chunk_type;
  uint16_t      chunk_length;
  uint32_t      chunk_seq_number;
  uint8_t       buf[BUF_SIZE];

  int           buf_size;
  int           buf_read;

  uint8_t       asf_header[ASF_HEADER_SIZE];
  uint32_t      asf_header_len;
  uint32_t      asf_header_read;
  int           num_stream_ids;
  mms_stream_t  streams[ASF_MAX_NUM_STREAMS];
  uint32_t      asf_packet_len;
  int64_t       file_len;
  uint64_t      file_time;
  uint64_t      time_len; /* playback time in 100 nanosecs (10^-7) */
  uint64_t      preroll;
  uint64_t      asf_num_packets;
  char          guid[37];

  int           has_audio;
  int           has_video;
  int           seekable;

  off_t         current_pos;
  int           bandwidth;
};

const static char *const mms_proto_s[] = { "mms", "mmsh", NULL };

#include "mms-common-funcs.h"

static int send_command(mms_io_t *io, mmsh_t *this, char *cmd)
{
  int length;

  lprintf("send_command:\n%s\n", cmd);

  length = strlen(cmd);
  if (io_write(io, this->s, cmd, length) != length) {
    lprintf("send error.\n");
    return 0;
  }
  return 1;
}

static int get_answer (mms_io_t *io, mmsh_t *this) {
 
  int done, len, linenum;
  char *features;

  done = 0; len = 0; linenum = 0;
  this->stream_type = MMSH_UNKNOWN;

  while (!done) {

    if (io_read(io, this->s, &(this->buf[len]), 1) != 1) {
      lprintf("end of stream\n");
      return 0;
    }

    if (this->buf[len] == '\012') {

      this->buf[len] = '\0';
      len--;
      
      if ((len >= 0) && (this->buf[len] == '\015')) {
        this->buf[len] = '\0';
        len--;
      }

      linenum++;
      
      lprintf("answer: >%s<\n", this->buf);

      if (linenum == 1) {
        int httpver, httpsub, httpcode;
        char httpstatus[51];

        if (sscanf(this->buf, "HTTP/%d.%d %d %50[^\015\012]", &httpver, &httpsub,
            &httpcode, httpstatus) != 4) {
          lprintf("bad response format\n");
          return 0;
        }

        if (httpcode >= 300 && httpcode < 400) {
          lprintf("3xx redirection not implemented: >%d %s<\n", httpcode, httpstatus);
          return 0;
        }

        if (httpcode < 200 || httpcode >= 300) {
          lprintf("http status not 2xx: >%d %s<\n", httpcode, httpstatus);
          return 0;
        }
      } else {

        if (!strncasecmp(this->buf, "Location: ", 10)) {
          lprintf("Location redirection not implemented.\n");
          return 0;
        }
        
        if (!strncasecmp(this->buf, "Pragma:", 7)) {
          features = strstr(this->buf + 7, "features=");
          if (features) {
            if (strstr(features, "seekable")) {
              lprintf("seekable stream\n");
              this->stream_type = MMSH_SEEKABLE;
              this->seekable = 1;
            } else {
              if (strstr(features, "broadcast")) {
                lprintf("live stream\n");
                this->stream_type = MMSH_LIVE;
                this->seekable = 0;
              }
            }
          }
        }
      }
      
      if (len == -1) {
        done = 1;
      } else {
        len = 0;
      }
    } else {
      len ++;
    }
  }
  if (this->stream_type == MMSH_UNKNOWN) {
    lprintf("unknown stream type\n");
    this->stream_type = MMSH_SEEKABLE; /* FIXME ? */
    this->seekable = 1;
  }
  return 1;
}

static int get_chunk_header (mms_io_t *io, mmsh_t *this) {
  uint8_t chunk_header[CHUNK_HEADER_LENGTH];
  uint8_t ext_header[EXT_HEADER_LENGTH];
  int read_len;
  int ext_header_len;

  /* read chunk header */
  read_len = io_read(io, this->s, chunk_header, CHUNK_HEADER_LENGTH);
  if (read_len != CHUNK_HEADER_LENGTH) {
    if (read_len == 0)
      return EOS;
    lprintf("chunk header read failed, %d != %d\n", read_len, CHUNK_HEADER_LENGTH);
    return MMSH_ERROR;
  }
  this->chunk_type       = LE_16 (&chunk_header[0]);
  this->chunk_length     = LE_16 (&chunk_header[2]);

  switch (this->chunk_type) {
    case CHUNK_TYPE_DATA:
      ext_header_len = 8;
      break;
    case CHUNK_TYPE_END:
      ext_header_len = 4;
      break;
    case CHUNK_TYPE_ASF_HEADER:
      ext_header_len = 8;
      break;
    case CHUNK_TYPE_RESET:
      ext_header_len = 4;
      break;
    default:
      ext_header_len = 0;
  }
  /* read extended header */
  if (ext_header_len > 0) {
    read_len = io_read (io, this->s, ext_header, ext_header_len);
    if (read_len != ext_header_len) {
      lprintf("extended header read failed. %d != %d\n", read_len, ext_header_len);
      return MMSH_ERROR;
    }
  }
  
  if (this->chunk_type == CHUNK_TYPE_DATA || this->chunk_type == CHUNK_TYPE_END)
    this->chunk_seq_number = LE_32 (&ext_header[0]);

  /* display debug infos */
#ifdef DEBUG
  switch (this->chunk_type) {
    case CHUNK_TYPE_DATA:
      lprintf ("chunk type:       CHUNK_TYPE_DATA\n");
      lprintf ("chunk length:     %d\n", this->chunk_length);
      lprintf ("chunk seq:        %d\n", this->chunk_seq_number);
      lprintf ("unknown:          %d\n", ext_header[4]);
      lprintf ("mmsh seq:         %d\n", ext_header[5]);
      lprintf ("len2:             %d\n", LE_16(&ext_header[6]));
      break;
    case CHUNK_TYPE_END:
      lprintf ("chunk type:       CHUNK_TYPE_END\n");
      lprintf ("continue: %d\n", this->chunk_seq_number);
      break;
    case CHUNK_TYPE_ASF_HEADER:
      lprintf ("chunk type:       CHUNK_TYPE_ASF_HEADER\n");
      lprintf ("chunk length:     %d\n", this->chunk_length);
      lprintf ("unknown:          %2X %2X %2X %2X %2X %2X\n",
               ext_header[0], ext_header[1], ext_header[2], ext_header[3],
               ext_header[4], ext_header[5]);
      lprintf ("len2:             %d\n", LE_16(&ext_header[6]));
      break;
    case CHUNK_TYPE_RESET:
      lprintf ("chunk type:       CHUNK_TYPE_RESET\n");
      lprintf ("chunk seq:        %d\n", this->chunk_seq_number);
      lprintf ("unknown:          %2X %2X %2X %2X\n",
               ext_header[0], ext_header[1], ext_header[2], ext_header[3]);
      break;
    default:
      lprintf ("unknown chunk:    %4X\n", this->chunk_type);
  }
#endif

  this->chunk_length -= ext_header_len;
  return MMSH_SUCCESS;
}

static int get_header (mms_io_t *io, mmsh_t *this) {
  int ret, len = 0;

  this->asf_header_len = 0;
  this->asf_header_read = 0;
  this->buf_size = 0;

  /* read chunk */
  while (1) {
    if ((ret = get_chunk_header(io, this)) == MMSH_SUCCESS) {
      if (this->chunk_type == CHUNK_TYPE_ASF_HEADER) {
        if ((this->asf_header_len + this->chunk_length) > ASF_HEADER_SIZE) {
          lprintf("the asf header exceed %d bytes\n", ASF_HEADER_SIZE);
          return MMSH_ERROR;
        } else {
          len = io_read(io, this->s, this->asf_header + this->asf_header_len,
                        this->chunk_length);
          if (len > 0)
            this->asf_header_len += len;
          if (len != this->chunk_length) {
            lprintf("asf header chunk read failed, %d != %d\n", len,
                     this->chunk_length);
            return MMSH_ERROR;
          }
        }
      } else {
        break;
      }
    } else {
      if (this->asf_header_len == 0 || ret != EOS)
        lprintf("get_header failed to get chunk header\n");
      return ret;
    }
  }

  if (this->chunk_type == CHUNK_TYPE_DATA) {
    /* read the first data chunk */
    len = io_read (io, this->s, this->buf, this->chunk_length);

    if (len != this->chunk_length) {
      lprintf("asf data chunk read failed, %d != %d\n", len,
               this->chunk_length);
      return MMSH_ERROR;
    } else {
      /* check and 0 pad the first data chunk */
      if (this->chunk_length > this->asf_packet_len) {
        lprintf("chunk_length(%d) > asf_packet_len(%d)\n",
                this->chunk_length, this->asf_packet_len);
        return MMSH_ERROR;
      }

      /* explicit padding with 0 */
      if (this->chunk_length < this->asf_packet_len)
        memset(this->buf + this->chunk_length, 0,
               this->asf_packet_len - this->chunk_length);

      this->buf_size = this->asf_packet_len;

      return MMSH_SUCCESS;
    }
  } else {
    /* unexpected packet type */
    lprintf("unexpected chunk_type(0x%04x)\n", this->chunk_type);
    return MMSH_ERROR;
  }
}

static int mmsh_connect_int (mms_io_t *io, mmsh_t *this, off_t seek, uint32_t time_seek) {
  int    i;
  int    video_stream = -1;
  int    audio_stream = -1;
  char   stream_selection[10 * ASF_MAX_NUM_STREAMS]; /* 10 chars per stream */
  int    offset;
  
  /* Close exisiting connection (if any) and connect */
  if (this->s != -1)
    closesocket(this->s);

  if (mms_tcp_connect(io, this)) {
    return 0;
  }

  /*
   * let the negotiations begin...
   */
  this->num_stream_ids = 0;

  /* first request */
  lprintf("first http request\n");
  
  snprintf (this->str, SCRATCH_SIZE, mmsh_FirstRequest, this->uri,
            this->http_host, this->http_port, this->http_request_number++);

  if (!send_command (io, this, this->str))
    goto fail;

  if (!get_answer (io, this))
    goto fail;

  /* Don't check for != MMSH_SUCCESS as EOS is normal here too */    
  if (get_header(io, this) == MMSH_ERROR)
    goto fail;

  interp_asf_header(this);
  if (!this->asf_packet_len || !this->num_stream_ids)
    goto fail;
  
  closesocket(this->s);

  mms_get_best_stream_ids(this, &audio_stream, &video_stream);

  /* second request */
  lprintf("second http request\n");

  if (mms_tcp_connect(io, this)) {
    return 0;
  }

  /* stream selection string */
  /* The same selection is done with mmst */
  /* 0 means selected */
  /* 2 means disabled */
  offset = 0;
  for (i = 0; i < this->num_stream_ids; i++) {
    int size;
    if ((this->streams[i].stream_id == audio_stream) ||
        (this->streams[i].stream_id == video_stream)) {
      lprintf("selecting stream %d\n", this->streams[i].stream_id);
      size = snprintf(stream_selection + offset, sizeof(stream_selection) - offset,
                      "ffff:%d:0 ", this->streams[i].stream_id);
    } else {
      lprintf("disabling stream %d\n", this->streams[i].stream_id);
      size = snprintf(stream_selection + offset, sizeof(stream_selection) - offset,
                      "ffff:%d:2 ", this->streams[i].stream_id);
    }
    if (size < 0) goto fail;
    offset += size;
  }

  switch (this->stream_type) {
    case MMSH_SEEKABLE:
      snprintf (this->str, SCRATCH_SIZE, mmsh_SeekableRequest, this->uri,
                this->http_host, this->http_port, time_seek,
                (unsigned int)(seek >> 32),
                (unsigned int)seek, this->http_request_number++, 0,
                this->num_stream_ids, stream_selection);
      break;
    case MMSH_LIVE:
      snprintf (this->str, SCRATCH_SIZE, mmsh_LiveRequest, this->uri,
                this->http_host, this->http_port, this->http_request_number++,
                this->num_stream_ids, stream_selection);
      break;
  }
  
  if (!send_command (io, this, this->str))
    goto fail;
  
  if (!get_answer (io, this))
    goto fail;

  if (get_header(io, this) != MMSH_SUCCESS)
    goto fail;

  interp_asf_header(this);
  if (!this->asf_packet_len || !this->num_stream_ids)
    goto fail;

  mms_disable_disabled_streams_in_asf_header(this, audio_stream, video_stream);

  return 1;
fail:
  closesocket(this->s);
  this->s = -1;
  return 0;
}

mmsh_t *mmsh_connect (mms_io_t *io, void *data, const char *url, int bandwidth) {
  mmsh_t *this;
  GURI  *uri = NULL;
  GURI  *proxy_uri = NULL;
  char  *proxy_env;
  if (!url)
    return NULL;

#ifdef _WIN32
  if (!mms_internal_winsock_load())
    return NULL;
#endif

  /*
   * initializatoin is essential here.  the fail: label depends
   * on the various char * in our this structure to be
   * NULL if they haven't been assigned yet.
   */
  this = calloc(1, sizeof(mmsh_t));

  this->url             = strdup(url);
  if ((proxy_env = getenv("http_proxy")) != NULL)
    this->proxy_url = strdup(proxy_env);
  this->s               = -1;
  this->bandwidth       = bandwidth;
  this->http_request_number = 1;

  if (this->proxy_url) {
    proxy_uri = gnet_uri_new(this->proxy_url);
    if (!proxy_uri) {
      lprintf("invalid proxy url\n");
      goto fail;
    }
    if (! proxy_uri->port ) {
      proxy_uri->port = 3128; //default squid port
    }
  }
  uri = gnet_uri_new(this->url);
  if (!uri) {
    lprintf("invalid url\n");
    goto fail;
  }
  if (! uri->port ) {
    //checked in tcp_connect, but it's better to initialize it here
    uri->port = DEFAULT_PORT;
  }
  if (this->proxy_url) {
    this->proto = (uri->scheme) ? strdup(uri->scheme) : NULL;
    this->connect_host = (proxy_uri->hostname) ? strdup(proxy_uri->hostname) : NULL;
    this->connect_port = proxy_uri->port;
    this->http_host = (uri->scheme) ? strdup(uri->hostname) : NULL;
    this->http_port = uri->port;
    this->proxy_user = (proxy_uri->user) ? strdup(proxy_uri->user) : NULL;
    this->proxy_password = (proxy_uri->passwd) ? strdup(proxy_uri->passwd) : NULL;
    this->host_user = (uri->user) ? strdup(uri->user) : NULL;
    this->host_password = (uri->passwd) ? strdup(uri->passwd) : NULL;
    gnet_uri_set_scheme(uri,"http");
    this->uri = gnet_mms_helper(uri, 1);
  } else {
    this->proto = (uri->scheme) ? strdup(uri->scheme) : NULL;
    this->connect_host = (uri->hostname) ? strdup(uri->hostname) : NULL;
    this->connect_port = uri->port;
    this->http_host = (uri->hostname) ? strdup(uri->hostname) : NULL;
    this->http_port = uri->port;
    this->proxy_user = NULL;
    this->proxy_password = NULL;
    this->host_user =(uri->user) ?  strdup(uri->user) : NULL;
    this->host_password = (uri->passwd) ? strdup(uri->passwd) : NULL;
    this->uri = gnet_mms_helper(uri, 1);
  }

  if(!this->uri)
        goto fail;

  if (proxy_uri) {
    gnet_uri_delete(proxy_uri);
    proxy_uri = NULL;
  }
  if (uri) {
    gnet_uri_delete(uri);
    uri = NULL;
  }
  if (!mms_valid_proto(this->proto)) {
    lprintf("unsupported protocol\n");
    goto fail;
  }

  if (!mmsh_connect_int(io, this, 0, 0))
    goto fail;

  return this;

fail:
  lprintf("connect failed\n");
  if (proxy_uri)
    gnet_uri_delete(proxy_uri);
  if (uri)
    gnet_uri_delete(uri);

  mmsh_close (this);

  return NULL;
}

static int get_media_packet (mms_io_t *io, mmsh_t *this) {
  int ret, len = 0;

  if ((ret = get_chunk_header(io, this)) == MMSH_SUCCESS) {
    switch (this->chunk_type) {
      case CHUNK_TYPE_END:
        /* this->chunk_seq_number:
         *     0: stop
         *     1: a new stream follows
         */
        if (this->chunk_seq_number == 0)
          return EOS;

        this->http_request_number = 1;
        if (!mmsh_connect_int (io, this, 0, 0))
          return MMSH_ERROR;

        /* What todo with: current_pos ??
           Also our chunk_seq_numbers will probably restart from 0!
           If this happens with a seekable stream (does it ever?)
           and we get a seek request after this were fscked! */
        this->seekable = 0;

        /* mmsh_connect_int reads the first data packet */
        /* this->buf_size is set by mmsh_connect_int */
        return GOT_HEADER_N_DATA;

      case CHUNK_TYPE_DATA:
        /* nothing to do */
        break;

      case CHUNK_TYPE_RESET:
        /* next chunk is an ASF header */

        if (this->chunk_length != 0) {
          /* that's strange, don't know what to do */
          lprintf("non 0 sized reset chunk");
          return MMSH_ERROR;
        }
        if ((ret = get_header (io, this)) != MMSH_SUCCESS) {
          lprintf("failed to get header after reset chunk\n");
          return ret;
        }
        interp_asf_header(this);

        /* What todo with: current_pos ??
           Also our chunk_seq_numbers might restart from 0!
           If this happens with a seekable stream (does it ever?) 
           and we get a seek request after this were fscked! */
        this->seekable = 0;

        /* get_header reads the first data packet */
        /* this->buf_size is set by get_header */
        return GOT_HEADER_N_DATA;

      default:
        lprintf("unexpected chunk_type(0x%04x)\n", this->chunk_type);
        return MMSH_ERROR;
    }

    len = io_read (io, this->s, this->buf, this->chunk_length);
      
    if (len == this->chunk_length) {
      /* explicit padding with 0 */
      if (this->chunk_length > this->asf_packet_len) {
        lprintf("chunk_length(%d) > asf_packet_len(%d)\n",
                this->chunk_length, this->asf_packet_len);
        return MMSH_ERROR;
      }

      memset(this->buf + this->chunk_length, 0,
             this->asf_packet_len - this->chunk_length);
      this->buf_size = this->asf_packet_len;

      return MMSH_SUCCESS;
    } else {
      lprintf("media packet read error, %d != %d\n", len,
              this->chunk_length);
      return MMSH_ERROR;
    }
  } else if (ret == EOS) {
    return EOS;
  } else {
    lprintf("get_media_packet failed to get chunk header\n");
    return ret;
  }
}

int mmsh_peek_header (mmsh_t *this, char *data, int maxsize) {
  int len;

  len = (this->asf_header_len < maxsize) ? this->asf_header_len : maxsize;

  memcpy(data, this->asf_header, len);
  return len;
}

int mmsh_read (mms_io_t *io, mmsh_t *this, char *data, int len) {
  int total;

  total = 0;

  /* Check if the stream didn't get closed because of previous errors */
  if (this->s == -1)
    return total;

  while (total < len) {

    if (this->asf_header_read < this->asf_header_len) {
      int n, bytes_left ;

      bytes_left = this->asf_header_len - this->asf_header_read ;

      if ((len-total) < bytes_left)
        n = len-total;
      else
        n = bytes_left;

      memcpy (&data[total], &this->asf_header[this->asf_header_read], n);

      this->asf_header_read += n;
      total += n;
      this->current_pos += n;
    } else {

      int n, bytes_left ;

      bytes_left = this->buf_size - this->buf_read;

      if (bytes_left == 0) {
        int ret;

        this->buf_size=this ->buf_read = 0;
        ret = get_media_packet (io, this);

        switch (ret) {
        case MMSH_SUCCESS:
          break;
        case MMSH_ERROR:
          lprintf("get_media_packet failed\n");
          return total;
        case EOS:
          return total;
        case GOT_HEADER_N_DATA:
          continue;
        }
        bytes_left = this->buf_size;
      }

      if ((len-total) < bytes_left)
        n = len-total;
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

off_t mmsh_seek (mms_io_t *io, mmsh_t *this, off_t offset, int origin) {
  off_t dest;
  off_t dest_packet_seq;
  uint32_t orig_asf_header_len = this->asf_header_len;
  uint32_t orig_asf_packet_len = this->asf_packet_len;
  
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
      dest = mmsh_get_length (this) + offset;
    default:
      return this->current_pos;
  }

  dest_packet_seq = dest - this->asf_header_len;
  dest_packet_seq = dest_packet_seq >= 0 ?
    dest_packet_seq / this->asf_packet_len : -1;

  if (dest_packet_seq < 0) {
    if (this->chunk_seq_number > 0) {
      lprintf("seek within header, already read beyond first packet, resetting connection\n");
      if (!mmsh_connect_int(io, this, 0, 0)) {
        /* Oops no more connection let our caller know things are fscked up */
        return this->current_pos = -1;
      }
      /* Some what simple / naive check to check for changed headers
         if the header was changed we are once more fscked up */
      if (this->asf_header_len != orig_asf_header_len ||
          this->asf_packet_len != orig_asf_packet_len) {
        lprintf("AIIEEE asf header or packet length changed on re-open for seek\n");
        /* Its a different stream, so its useless! */
        closesocket(this->s);
        this->s = -1;
        return this->current_pos = -1;
      }
    } else
      lprintf("seek within header, resetting buf_read\n");

    // reset buf_read
    this->buf_read = 0;
    this->asf_header_read = dest;
    return this->current_pos = dest;
  }

  // dest_packet_seq >= 0
  if (this->asf_num_packets && dest == this->asf_header_len +
      this->asf_num_packets * this->asf_packet_len) {
    // Requesting the packet beyond the last packet, can cause the server to
    // not return any packet or any eos command.  This can cause
    // mms_packet_seek() to hang.
    // This is to allow seeking at end of stream, and avoid hanging.
    --dest_packet_seq;
    lprintf("seek to eos!\n");
  }

  if (dest_packet_seq != this->chunk_seq_number) {

    if (this->asf_num_packets && dest_packet_seq >= this->asf_num_packets) {
      // Do not seek beyond the last packet.
      return this->current_pos;
    }
    
    lprintf("seek to %d, packet: %d\n", (int)dest, (int)dest_packet_seq);
    if (!mmsh_connect_int(io, this, (dest_packet_seq+1) * this->asf_packet_len, 0)) {
      /* Oops no more connection let our caller know things are fscked up */
      return this->current_pos = -1;
    }
    /* Some what simple / naive check to check for changed headers
       if the header was changed we are once more fscked up */
    if (this->asf_header_len != orig_asf_header_len ||
        this->asf_packet_len != orig_asf_packet_len) {
      lprintf("AIIEEE asf header or packet length changed on re-open for seek\n");
      /* Its a different stream, so its useless! */
      closesocket(this->s);
      this->s = -1;
      return this->current_pos = -1;
    }
  }
  else
    lprintf("seek within current packet, dest: %d, current pos: %d\n",
       (int)dest, (int)this->current_pos);

  /* make sure asf_header is seen as fully read by mmsh_read() this is needed
     in case our caller tries to seek over part of the header, or when we've
     done an actual packet seek as get_header() resets asf_header_read then. */
  this->asf_header_read = this->asf_header_len;

  /* check we got what we want */
  if (dest_packet_seq == this->chunk_seq_number) {
    this->buf_read = dest -
      (this->asf_header_len + dest_packet_seq * this->asf_packet_len);
    this->current_pos = dest;
  } else {
    lprintf("Seek failed, wanted packet: %d, got packet: %d\n",
      (int)dest_packet_seq, (int)this->chunk_seq_number);
    this->buf_read = 0;
    this->current_pos = this->asf_header_len + this->chunk_seq_number *
      this->asf_packet_len;
  }

  lprintf("current_pos after seek to %d: %d (buf_read %d)\n",
    (int)dest, (int)this->current_pos, (int)this->buf_read);

  return this->current_pos;
}

int mmsh_time_seek (mms_io_t *io, mmsh_t *this, double time_sec) {
  uint32_t orig_asf_header_len = this->asf_header_len;
  uint32_t orig_asf_packet_len = this->asf_packet_len;

  if (!this->seekable)
    return 0;

  lprintf("time seek to %f secs\n", time_sec);
  if (!mmsh_connect_int(io, this, 0, time_sec * 1000 + this->preroll)) {
    /* Oops no more connection let our caller know things are fscked up */
    this->current_pos = -1;
    return 0;
  }
  /* Some what simple / naive check to check for changed headers
     if the header was changed we are once more fscked up */
  if (this->asf_header_len != orig_asf_header_len ||
      this->asf_packet_len != orig_asf_packet_len) {
    lprintf("AIIEEE asf header or packet length changed on re-open for seek\n");
    /* Its a different stream, so its useless! */
    closesocket(this->s);
    this->s = -1;
    this->current_pos = -1;
    return 0;
  }

  this->asf_header_read = this->asf_header_len;
  this->buf_read = 0;
  this->current_pos = this->asf_header_len + this->chunk_seq_number *
    this->asf_packet_len;
  
  lprintf("mmsh, current_pos after time_seek:%d\n", (int)this->current_pos);

  return 1;
}

void mmsh_close (mmsh_t *this) {
  if (this == NULL)
    return;
  if (this->s != -1)
    closesocket(this->s);
  if (this->url)
    free(this->url);
  if (this->proxy_url)
    free(this->proxy_url);
  if (this->proto)
    free(this->proto);
  if (this->connect_host)
    free(this->connect_host);
  if (this->http_host)
    free(this->http_host);
  if (this->proxy_user)
    free(this->proxy_user);
  if (this->proxy_password)
    free(this->proxy_password);
  if (this->host_user)
    free(this->host_user);
  if (this->host_password)
    free(this->host_password);
  if (this->uri)
    free(this->uri);

  free (this);
}


uint32_t mmsh_get_length (mmsh_t *this) {
  /* we could / should return this->file_len here, but usually this->file_len
     is longer then the calculation below, as usually an asf file contains an
     asf index object after the data stream. However since we do not have a
     (known) way to get to this index object through mms, we return a
     calculated size of what we can get to when we know. */
  if (this->asf_num_packets)
    return this->asf_header_len + this->asf_num_packets * this->asf_packet_len;
  else
    return this->file_len;
}

double mmsh_get_time_length (mmsh_t *this) {
  return (double)(this->time_len) / 1e7;
}

uint64_t mmsh_get_raw_time_length (mmsh_t *this) {
  return this->time_len;
}

uint64_t mmsh_get_file_time (mmsh_t *this) {
  return this->file_time;
}

off_t mmsh_get_current_pos (mmsh_t *this) {
  return this->current_pos;
}

uint32_t mmsh_get_asf_header_len (mmsh_t *this) {
  return this->asf_header_len;
}

uint32_t mmsh_get_asf_packet_len (mmsh_t *this) {
  return this->asf_packet_len;
}

int mmsh_get_seekable (mmsh_t *this) {
  return this->seekable;
}
