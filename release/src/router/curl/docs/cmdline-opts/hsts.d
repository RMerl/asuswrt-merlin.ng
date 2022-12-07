c: Copyright (C) 1998 - 2022, Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: hsts
Arg: <file name>
Protocols: HTTPS
Help: Enable HSTS with this cache file
Added: 7.74.0
Category: http
Example: --hsts cache.txt $URL
See-also: proto
---
This option enables HSTS for the transfer. If the file name points to an
existing HSTS cache file, that will be used. After a completed transfer, the
cache will be saved to the file name again if it has been modified.

Specify a "" file name (zero length) to avoid loading/saving and make curl
just handle HSTS in memory.

If this option is used several times, curl will load contents from all the
files but the last one will be used for saving.
