c: Copyright (C) 1998 - 2022, Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: dns-ipv4-addr
Arg: <address>
Help: IPv4 address to use for DNS requests
Protocols: DNS
See-also: dns-interface dns-ipv6-addr
Added: 7.33.0
Requires: c-ares
Category: dns
Example: --dns-ipv4-addr 10.1.2.3 $URL
---
Tell curl to bind to <ip-address> when making IPv4 DNS requests, so that
the DNS requests originate from this address. The argument should be a
single IPv4 address.

If this option is used several times, the last one will be used.
