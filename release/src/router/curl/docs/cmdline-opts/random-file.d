c: Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: random-file
Arg: <file>
Help: File for reading random data from
Category: misc
Example: --random-file rubbish $URL
Added: 7.7
See-also: egd-file
Multi: single
---
Deprecated option. This option is ignored (added in 7.84.0). Prior to that it
only had an effect on curl if built to use old versions of OpenSSL.

Specify the path name to file containing random data. The data may be used to
seed the random engine for SSL connections.
