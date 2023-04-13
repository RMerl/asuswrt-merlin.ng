c: Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: proxy-ssl-auto-client-cert
Help: Use auto client certificate for proxy (Schannel)
Added: 7.77.0
Category: proxy tls
Example: --proxy-ssl-auto-client-cert -x https://proxy $URL
See-also: ssl-auto-client-cert proxy
Multi: boolean
---
Same as --ssl-auto-client-cert but used in HTTPS proxy context.
