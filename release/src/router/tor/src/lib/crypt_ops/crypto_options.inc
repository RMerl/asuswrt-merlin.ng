
/**
 * @file crypto_options.inc
 * @brief Declare configuration options for the crypto_ops module.
 **/

/** Holds configuration about our cryptography options. */
BEGIN_CONF_STRUCT(crypto_options_t)

/** Should we enable extra OpenSSL hardware acceleration (where available)?  */
CONF_VAR(HardwareAccel, BOOL, CFLG_IMMUTABLE, "0")

/** Optional OpenSSL hardware-acceleration engine name */
CONF_VAR(AccelName, STRING, CFLG_IMMUTABLE, NULL)

/** Optional OpenSSL hardware-acceleration engine search directory. */
CONF_VAR(AccelDir, FILENAME, CFLG_IMMUTABLE, NULL)

END_CONF_STRUCT(crypto_options_t)
