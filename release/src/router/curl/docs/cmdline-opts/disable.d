c: Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: disable
Short: q
Help: Disable .curlrc
Category: curl
Example: -q $URL
Added: 5.0
See-also: config
Multi: boolean
---
If used as the **first** parameter on the command line, the *curlrc* config
file is not read or used. See the --config for details on the default config
file search path.

Prior to 7.50.0 curl supported the short option name *q* but not the long
option name *disable*.
