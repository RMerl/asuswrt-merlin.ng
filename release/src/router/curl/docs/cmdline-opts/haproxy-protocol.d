c: Copyright (C) 1998 - 2022, Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: haproxy-protocol
Help: Send HAProxy PROXY protocol v1 header
Protocols: HTTP
Added: 7.60.0
Category: http proxy
Example: --haproxy-protocol $URL
See-also: proxy
---
Send a HAProxy PROXY protocol v1 header at the beginning of the
connection. This is used by some load balancers and reverse proxies to
indicate the client's true IP address and port.

This option is primarily useful when sending test requests to a service that
expects this header.
