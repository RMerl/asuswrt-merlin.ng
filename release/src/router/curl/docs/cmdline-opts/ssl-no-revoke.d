c: Copyright (C) 1998 - 2022, Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: ssl-no-revoke
Help: Disable cert revocation checks (Schannel)
Added: 7.44.0
Category: tls
Example: --ssl-no-revoke $URL
See-also: crlfile
---
(Schannel) This option tells curl to disable certificate revocation checks.
WARNING: this option loosens the SSL security, and by using this flag you ask
for exactly that.
