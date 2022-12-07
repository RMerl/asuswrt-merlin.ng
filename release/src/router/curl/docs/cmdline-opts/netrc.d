c: Copyright (C) 1998 - 2022, Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: netrc
Short: n
Help: Must read .netrc for user name and password
Category: curl
Example: --netrc $URL
Added: 4.6
See-also: netrc-file config user
---
Makes curl scan the *.netrc* (*_netrc* on Windows) file in the user's home
directory for login name and password. This is typically used for FTP on
Unix. If used with HTTP, curl will enable user authentication. See
*netrc(5)* and *ftp(1)* for details on the file format. Curl will not
complain if that file does not have the right permissions (it should be
neither world- nor group-readable). The environment variable "HOME" is used
to find the home directory.

A quick and simple example of how to setup a *.netrc* to allow curl to FTP to
the machine host.domain.com with user name \&'myself' and password \&'secret'
could look similar to:

 machine host.domain.com
 login myself
 password secret"
