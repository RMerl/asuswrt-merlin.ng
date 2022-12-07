c: Copyright (C) 1998 - 2022, Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: negotiate
Help: Use HTTP Negotiate (SPNEGO) authentication
Protocols: HTTP
See-also: basic ntlm anyauth proxy-negotiate
Category: auth http
Example: --negotiate -u : $URL
Added: 7.10.6
---
Enables Negotiate (SPNEGO) authentication.

This option requires a library built with GSS-API or SSPI support. Use
--version to see if your curl supports GSS-API/SSPI or SPNEGO.

When using this option, you must also provide a fake --user option to activate
the authentication code properly. Sending a '-u :' is enough as the user name
and password from the --user option are not actually used.

If this option is used several times, only the first one is used.
