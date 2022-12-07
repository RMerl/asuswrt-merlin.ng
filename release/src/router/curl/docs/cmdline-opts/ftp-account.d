c: Copyright (C) 1998 - 2022, Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: ftp-account
Arg: <data>
Help: Account data string
Protocols: FTP
Added: 7.13.0
Category: ftp auth
Example: --ftp-account "mr.robot" ftp://example.com/
See-also: user
---
When an FTP server asks for "account data" after user name and password has
been provided, this data is sent off using the ACCT command.

If this option is used several times, the last one will be used.
