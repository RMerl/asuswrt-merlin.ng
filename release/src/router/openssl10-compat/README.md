# OpenSSL 1.0 compatibility shim

This component is built for both HNS and Wtfast profiles using OpenSSL 3.5
(`RTCONFIG_WTFAST=y`, `RTCONFIG_OPENSSL35=y`). It supplies the historical
`libcrypto.so.1.0.0` and `libssl.so.1.0.0` SONAMEs required by bith the HNS and Wtfast
prebuilt payloads. It is designed the same way as the openssl11-compat shim.
