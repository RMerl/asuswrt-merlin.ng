c: Copyright (C) 1998 - 2022, Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: ignore-content-length
Help: Ignore the size of the remote resource
Protocols: FTP HTTP
Category: http ftp
Example: --ignore-content-length $URL
Added: 7.14.1
See-also: ftp-skip-pasv-ip
---
For HTTP, Ignore the Content-Length header. This is particularly useful for
servers running Apache 1.x, which will report incorrect Content-Length for
files larger than 2 gigabytes.

For FTP (since 7.46.0), skip the RETR command to figure out the size before
downloading a file.

This option does not work for HTTP if libcurl was built to use hyper.
