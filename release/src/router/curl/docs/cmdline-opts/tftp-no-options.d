c: Copyright (C) 1998 - 2022, Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: tftp-no-options
Help: Do not send any TFTP options
Protocols: TFTP
Added: 7.48.0
Category: tftp
Example: --tftp-no-options tftp://192.168.0.1/
See-also: tftp-blksize
---
Tells curl not to send TFTP options requests.

This option improves interop with some legacy servers that do not acknowledge
or properly implement TFTP options. When this option is used --tftp-blksize is
ignored.
