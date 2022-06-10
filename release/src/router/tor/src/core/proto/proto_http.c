/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file proto_http.c
 * @brief Parse a subset of the HTTP protocol.
 **/

#define PROTO_HTTP_PRIVATE
#include "core/or/or.h"
#include "lib/buf/buffers.h"
#include "core/proto/proto_http.h"

/** Return true if <b>cmd</b> looks like a HTTP (proxy) request. */
int
peek_buf_has_http_command(const buf_t *buf)
{
  if (buf_peek_startswith(buf, "CONNECT ") ||
      buf_peek_startswith(buf, "DELETE ") ||
      buf_peek_startswith(buf, "GET ") ||
      buf_peek_startswith(buf, "POST ") ||
      buf_peek_startswith(buf, "PUT " ))
    return 1;
  return 0;
}

/** There is a (possibly incomplete) http statement on <b>buf</b>, of the
 * form "\%s\\r\\n\\r\\n\%s", headers, body. (body may contain NULs.)
 * If a) the headers include a Content-Length field and all bytes in
 * the body are present, or b) there's no Content-Length field and
 * all headers are present, then:
 *
 *  - strdup headers into <b>*headers_out</b>, and NUL-terminate it.
 *  - memdup body into <b>*body_out</b>, and NUL-terminate it.
 *  - Then remove them from <b>buf</b>, and return 1.
 *
 *  - If headers or body is NULL, discard that part of the buf.
 *  - If a headers or body doesn't fit in the arg, return -1.
 *  (We ensure that the headers or body don't exceed max len,
 *   _even if_ we're planning to discard them.)
 *  - If force_complete is true, then succeed even if not all of the
 *    content has arrived.
 *
 * Else, change nothing and return 0.
 */
int
fetch_from_buf_http(buf_t *buf,
                    char **headers_out, size_t max_headerlen,
                    char **body_out, size_t *body_used, size_t max_bodylen,
                    int force_complete)
{
  const char *headers;
  size_t headerlen, bodylen, contentlen=0;
  int crlf_offset;
  int r;

  if (buf_datalen(buf) == 0)
    return 0;

  crlf_offset = buf_find_string_offset(buf, "\r\n\r\n", 4);
  if (crlf_offset > (int)max_headerlen ||
      (crlf_offset < 0 && buf_datalen(buf) > max_headerlen)) {
    log_debug(LD_HTTP,"headers too long.");
    return -1;
  } else if (crlf_offset < 0) {
    log_debug(LD_HTTP,"headers not all here yet.");
    return 0;
  }
  /* Okay, we have a full header.  Make sure it all appears in the first
   * chunk. */
  headerlen = crlf_offset + 4;
  size_t headers_in_chunk = 0;
  buf_pullup(buf, headerlen, &headers, &headers_in_chunk);

  bodylen = buf_datalen(buf) - headerlen;
  log_debug(LD_HTTP,"headerlen %d, bodylen %d.", (int)headerlen, (int)bodylen);

  if (max_headerlen <= headerlen) {
    log_warn(LD_HTTP,"headerlen %d larger than %d. Failing.",
             (int)headerlen, (int)max_headerlen-1);
    return -1;
  }
  if (max_bodylen <= bodylen) {
    log_warn(LD_HTTP,"bodylen %d larger than %d. Failing.",
             (int)bodylen, (int)max_bodylen-1);
    return -1;
  }

  r = buf_http_find_content_length(headers, headerlen, &contentlen);
  if (r == -1) {
    log_warn(LD_PROTOCOL, "Content-Length is bogus; maybe "
             "someone is trying to crash us.");
    return -1;
  } else if (r == 1) {
    /* if content-length is malformed, then our body length is 0. fine. */
    log_debug(LD_HTTP,"Got a contentlen of %d.",(int)contentlen);
    if (bodylen < contentlen) {
      if (!force_complete) {
        log_debug(LD_HTTP,"body not all here yet.");
        return 0; /* not all there yet */
      }
    }
    if (bodylen > contentlen) {
      bodylen = contentlen;
      log_debug(LD_HTTP,"bodylen reduced to %d.",(int)bodylen);
    }
  } else {
    tor_assert(r == 0);
    /* Leave bodylen alone */
  }

  /* all happy. copy into the appropriate places, and return 1 */
  if (headers_out) {
    *headers_out = tor_malloc(headerlen+1);
    buf_get_bytes(buf, *headers_out, headerlen);
    (*headers_out)[headerlen] = 0; /* NUL terminate it */
  }
  if (body_out) {
    tor_assert(body_used);
    *body_used = bodylen;
    *body_out = tor_malloc(bodylen+1);
    buf_get_bytes(buf, *body_out, bodylen);
    (*body_out)[bodylen] = 0; /* NUL terminate it */
  }
  return 1;
}

/**
 * Scan the HTTP headers in the <b>headerlen</b>-byte memory range at
 * <b>headers</b>, looking for a "Content-Length" header.  Try to set
 * *<b>result_out</b> to the numeric value of that header if possible.
 * Return -1 if the header was malformed, 0 if it was missing, and 1 if
 * it was present and well-formed.
 */
STATIC int
buf_http_find_content_length(const char *headers, size_t headerlen,
                             size_t *result_out)
{
  const char *p, *newline;
  char *len_str, *eos=NULL;
  size_t remaining, result;
  int ok;
  *result_out = 0; /* The caller shouldn't look at this unless the
                    * return value is 1, but let's prevent confusion */

#define CONTENT_LENGTH "\r\nContent-Length: "
  p = (char*) tor_memstr(headers, headerlen, CONTENT_LENGTH);
  if (p == NULL)
    return 0;

  tor_assert(p >= headers && p < headers+headerlen);
  remaining = (headers+headerlen)-p;
  p += strlen(CONTENT_LENGTH);
  remaining -= strlen(CONTENT_LENGTH);

  newline = memchr(p, '\n', remaining);
  if (newline == NULL)
    return -1;

  len_str = tor_memdup_nulterm(p, newline-p);
  /* We limit the size to INT_MAX because other parts of the buffer.c
   * code don't like buffers to be any bigger than that. */
  result = (size_t) tor_parse_uint64(len_str, 10, 0, INT_MAX, &ok, &eos);
  if (eos && !tor_strisspace(eos)) {
    ok = 0;
  } else {
    *result_out = result;
  }
  tor_free(len_str);

  return ok ? 1 : -1;
}
