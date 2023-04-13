c: Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: ntlm-wb
Help: Use HTTP NTLM authentication with winbind
Protocols: HTTP
See-also: ntlm proxy-ntlm
Category: auth http
Example: --ntlm-wb -u user:password $URL
Added: 7.22.0
Multi: mutex
---
Enables NTLM much in the style --ntlm does, but hand over the authentication
to the separate binary ntlmauth application that is executed when needed.
