c: Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: ca-native
Help: Use CA certificates from the native OS
Protocols: TLS
Category: tls
See-also: cacert capath insecure
Example: --ca-native $URL
Added: 8.2.0
Multi: boolean
---
Tells curl to use the CA store from the native operating system to verify the
peer. By default, curl otherwise uses a CA store provided in a single file or
directory, but when using this option it interfaces the operating system's
own vault.

This option only works for curl on Windows when built to use OpenSSL. When
curl on Windows is built to use Schannel, this feature is implied and curl
then only uses the native CA store.

curl built with wolfSSL also supports this option (added in 8.3.0).
