/* Here are the TLS1.3 ciphers. Note that we don't have XCIPHER instances
 * here, since we don't want to ever fake them.
 *
 * This matches Firefox's list:
 *    https://searchfox.org/mozilla-central/source/security/nss/lib/ssl/ssl3con.c#100
 */
#ifdef TLS1_3_RFC_AES_128_GCM_SHA256
   CIPHER(0x1301, TLS1_3_RFC_AES_128_GCM_SHA256)
#endif
#ifdef TLS1_3_RFC_CHACHA20_POLY1305_SHA256
   CIPHER(0x1303, TLS1_3_RFC_CHACHA20_POLY1305_SHA256)
#endif
#ifdef TLS1_3_RFC_AES_256_GCM_SHA384
   CIPHER(0x1302, TLS1_3_RFC_AES_256_GCM_SHA384)
#endif
