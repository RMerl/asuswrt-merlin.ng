# OpenSSL 1.1 compatibility boundary

The shim loads the firmware’s OpenSSL 3.5 `libcrypto.so.3` and `libssl.so.3`
and forwards only the APIs declared in `libcrypto.map` and `libssl.map`. It
also preserves the 1.1 SONAMEs and symbol versions expected by the blobs. The
ARM32 `SSL_CTX_set_options` ABI difference is handled explicitly in
`ssl_compat.c`.

Normal firmware code links directly to OpenSSL 3.5. The shim is linked only by
the compatibility-sensitive components required by vendor prebuilt libraries;
it is installed alongside the native 3.5 libraries so vendor blobs resolve it.
