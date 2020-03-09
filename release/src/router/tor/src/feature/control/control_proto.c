/* Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file control_proto.c
 * \brief Formatting functions for controller data.
 */

#include "core/or/or.h"

#include "core/mainloop/connection.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitlist.h"
#include "core/or/connection_edge.h"
#include "feature/control/control_proto.h"
#include "feature/nodelist/nodelist.h"

#include "core/or/cpath_build_state_st.h"
#include "core/or/entry_connection_st.h"
#include "core/or/or_connection_st.h"
#include "core/or/origin_circuit_st.h"
#include "core/or/socks_request_st.h"
#include "feature/control/control_connection_st.h"

/** Append a NUL-terminated string <b>s</b> to the end of
 * <b>conn</b>-\>outbuf.
 */
void
connection_write_str_to_buf(const char *s, control_connection_t *conn)
{
  size_t len = strlen(s);
  connection_buf_add(s, len, TO_CONN(conn));
}

/** Acts like sprintf, but writes its formatted string to the end of
 * <b>conn</b>-\>outbuf. */
void
connection_printf_to_buf(control_connection_t *conn, const char *format, ...)
{
  va_list ap;
  char *buf = NULL;
  int len;

  va_start(ap,format);
  len = tor_vasprintf(&buf, format, ap);
  va_end(ap);

  if (len < 0) {
    log_err(LD_BUG, "Unable to format string for controller.");
    tor_assert(0);
  }

  connection_buf_add(buf, (size_t)len, TO_CONN(conn));

  tor_free(buf);
}

/** Given a <b>len</b>-character string in <b>data</b>, made of lines
 * terminated by CRLF, allocate a new string in *<b>out</b>, and copy the
 * contents of <b>data</b> into *<b>out</b>, adding a period before any period
 * that appears at the start of a line, and adding a period-CRLF line at
 * the end. Replace all LF characters sequences with CRLF.  Return the number
 * of bytes in *<b>out</b>.
 *
 * This corresponds to CmdData in control-spec.txt.
 */
size_t
write_escaped_data(const char *data, size_t len, char **out)
{
  tor_assert(len < SIZE_MAX - 9);
  size_t sz_out = len+8+1;
  char *outp;
  const char *start = data, *end;
  size_t i;
  int start_of_line;
  for (i=0; i < len; ++i) {
    if (data[i] == '\n') {
      sz_out += 2; /* Maybe add a CR; maybe add a dot. */
      if (sz_out >= SIZE_T_CEILING) {
        log_warn(LD_BUG, "Input to write_escaped_data was too long");
        *out = tor_strdup(".\r\n");
        return 3;
      }
    }
  }
  *out = outp = tor_malloc(sz_out);
  end = data+len;
  start_of_line = 1;
  while (data < end) {
    if (*data == '\n') {
      if (data > start && data[-1] != '\r')
        *outp++ = '\r';
      start_of_line = 1;
    } else if (*data == '.') {
      if (start_of_line) {
        start_of_line = 0;
        *outp++ = '.';
      }
    } else {
      start_of_line = 0;
    }
    *outp++ = *data++;
  }
  if (outp < *out+2 || fast_memcmp(outp-2, "\r\n", 2)) {
    *outp++ = '\r';
    *outp++ = '\n';
  }
  *outp++ = '.';
  *outp++ = '\r';
  *outp++ = '\n';
  *outp = '\0'; /* NUL-terminate just in case. */
  tor_assert(outp >= *out);
  tor_assert((size_t)(outp - *out) <= sz_out);
  return outp - *out;
}

/** Given a <b>len</b>-character string in <b>data</b>, made of lines
 * terminated by CRLF, allocate a new string in *<b>out</b>, and copy
 * the contents of <b>data</b> into *<b>out</b>, removing any period
 * that appears at the start of a line, and replacing all CRLF sequences
 * with LF.   Return the number of
 * bytes in *<b>out</b>.
 *
 * This corresponds to CmdData in control-spec.txt.
 */
size_t
read_escaped_data(const char *data, size_t len, char **out)
{
  char *outp;
  const char *next;
  const char *end;

  *out = outp = tor_malloc(len+1);

  end = data+len;

  while (data < end) {
    /* we're at the start of a line. */
    if (*data == '.')
      ++data;
    next = memchr(data, '\n', end-data);
    if (next) {
      size_t n_to_copy = next-data;
      /* Don't copy a CR that precedes this LF. */
      if (n_to_copy && *(next-1) == '\r')
        --n_to_copy;
      memcpy(outp, data, n_to_copy);
      outp += n_to_copy;
      data = next+1; /* This will point at the start of the next line,
                      * or the end of the string, or a period. */
    } else {
      memcpy(outp, data, end-data);
      outp += (end-data);
      *outp = '\0';
      return outp - *out;
    }
    *outp++ = '\n';
  }

  *outp = '\0';
  return outp - *out;
}

/** Send a "DONE" message down the control connection <b>conn</b>. */
void
send_control_done(control_connection_t *conn)
{
  control_write_endreply(conn, 250, "OK");
}

/** Write a reply to the control channel.
 *
 * @param conn control connection
 * @param code numeric result code
 * @param c separator character, usually ' ', '-', or '+'
 * @param s string reply content
 */
MOCK_IMPL(void,
control_write_reply, (control_connection_t *conn, int code, int c,
                      const char *s))
{
  connection_printf_to_buf(conn, "%03d%c%s\r\n", code, c, s);
}

/** Write a formatted reply to the control channel.
 *
 * @param conn control connection
 * @param code numeric result code
 * @param c separator character, usually ' ', '-', or '+'
 * @param fmt format string
 * @param ap va_list from caller
 */
void
control_vprintf_reply(control_connection_t *conn, int code, int c,
                      const char *fmt, va_list ap)
{
  char *buf = NULL;
  int len;

  len = tor_vasprintf(&buf, fmt, ap);
  if (len < 0) {
    log_err(LD_BUG, "Unable to format string for controller.");
    tor_assert(0);
  }
  control_write_reply(conn, code, c, buf);
  tor_free(buf);
}

/** Write an EndReplyLine */
void
control_write_endreply(control_connection_t *conn, int code, const char *s)
{
  control_write_reply(conn, code, ' ', s);
}

/** Write a formatted EndReplyLine */
void
control_printf_endreply(control_connection_t *conn, int code,
                        const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  control_vprintf_reply(conn, code, ' ', fmt, ap);
  va_end(ap);
}

/** Write a MidReplyLine */
void
control_write_midreply(control_connection_t *conn, int code, const char *s)
{
  control_write_reply(conn, code, '-', s);
}

/** Write a formatted MidReplyLine */
void
control_printf_midreply(control_connection_t *conn, int code, const char *fmt,
                        ...)
{
  va_list ap;

  va_start(ap, fmt);
  control_vprintf_reply(conn, code, '-', fmt, ap);
  va_end(ap);
}

/** Write a DataReplyLine */
void
control_write_datareply(control_connection_t *conn, int code, const char *s)
{
  control_write_reply(conn, code, '+', s);
}

/** Write a formatted DataReplyLine */
void
control_printf_datareply(control_connection_t *conn, int code, const char *fmt,
                         ...)
{
  va_list ap;

  va_start(ap, fmt);
  control_vprintf_reply(conn, code, '+', fmt, ap);
  va_end(ap);
}

/** Write a CmdData */
void
control_write_data(control_connection_t *conn, const char *data)
{
  char *esc = NULL;
  size_t esc_len;

  esc_len = write_escaped_data(data, strlen(data), &esc);
  connection_buf_add(esc, esc_len, TO_CONN(conn));
  tor_free(esc);
}
