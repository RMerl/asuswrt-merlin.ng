c: Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: connect-timeout
Arg: <fractional seconds>
Help: Maximum time allowed for connection
See-also: max-time
Category: connection
Example: --connect-timeout 20 $URL
Example: --connect-timeout 3.14 $URL
Added: 7.7
Multi: single
---
Maximum time in seconds that you allow curl's connection to take.  This only
limits the connection phase, so if curl connects within the given period it
continues - if not it exits.

This option accepts decimal values (added in 7.32.0). The decimal value needs
to be provided using a dot (.) as decimal separator - not the local version
even if it might be using another separator.

The connection phase is considered complete when the DNS lookup and requested
TCP, TLS or QUIC handshakes are done.
