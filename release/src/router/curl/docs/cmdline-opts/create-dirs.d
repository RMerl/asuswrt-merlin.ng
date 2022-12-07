c: Copyright (C) 1998 - 2022, Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: create-dirs
Help: Create necessary local directory hierarchy
Category: curl
Example: --create-dirs --output local/dir/file $URL
Added: 7.10.3
See-also: ftp-create-dirs output-dir
---
When used in conjunction with the --output option, curl will create the
necessary local directory hierarchy as needed. This option creates the
directories mentioned with the --output option, nothing else. If the --output
file name uses no directory, or if the directories it mentions already exist,
no directories will be created.

Created dirs are made with mode 0750 on unix style file systems.

To create remote directories when using FTP or SFTP, try --ftp-create-dirs.
