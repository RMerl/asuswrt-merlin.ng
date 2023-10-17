c: Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: tcp-fastopen
Added: 7.49.0
Help: Use TCP Fast Open
Category: connection
Example: --tcp-fastopen $URL
See-also: false-start
Multi: boolean
---

Enable use of TCP Fast Open (RFC 7413). TCP Fast Open is a TCP extension that
allows data to get sent earlier over the connection (before the final
handshake ACK) if the client and server have been connected previously.
