/* Copyright (c) 2019-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define PROTO_HAPROXY_PRIVATE
#include "lib/malloc/malloc.h"
#include "lib/net/address.h"
#include "lib/string/printf.h"
#include "core/proto/proto_haproxy.h"

/** Return a newly allocated PROXY header null-terminated string. Returns NULL
 * if addr_port->addr is incompatible with the proxy protocol.
 */
char *
haproxy_format_proxy_header_line(const tor_addr_port_t *addr_port)
{
  tor_assert(addr_port);

  sa_family_t family = tor_addr_family(&addr_port->addr);
  const char *family_string = NULL;
  const char *src_addr_string = NULL;

  switch (family) {
    case AF_INET:
      family_string = "TCP4";
      src_addr_string = "0.0.0.0";
      break;
    case AF_INET6:
      family_string = "TCP6";
      src_addr_string = "::";
      break;
    default:
      /* Unknown family. */
      return NULL;
  }

  char *buf;
  char addrbuf[TOR_ADDR_BUF_LEN];

  tor_addr_to_str(addrbuf, &addr_port->addr, sizeof(addrbuf), 0);

  tor_asprintf(&buf, "PROXY %s %s %s 0 %d\r\n", family_string, src_addr_string,
                                                addrbuf, addr_port->port);

  return buf;
}
