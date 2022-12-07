c: Copyright (C) 1998 - 2022, Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: tlsv1.2
Help: Use TLSv1.2 or greater
Protocols: TLS
Added: 7.34.0
Category: tls
Example: --tlsv1.2 $URL
See-also: tlsv1.3 tls-max
---
Forces curl to use TLS version 1.2 or later when connecting to a remote TLS server.

In old versions of curl this option was documented to allow _only_ TLS 1.2.
That behavior was inconsistent depending on the TLS library. Use --tls-max if
you want to set a maximum TLS version.
