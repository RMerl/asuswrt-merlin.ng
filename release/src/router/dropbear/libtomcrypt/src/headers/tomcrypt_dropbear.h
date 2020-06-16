/* compile options depend on Dropbear options.h */
#include "options.h"

/* Dropbear config */

#define LTC_NOTHING

/* Use small code where possible */
#if DROPBEAR_SMALL_CODE
#define LTC_SMALL_CODE
#endif

/* Fewer entries needed */
#define TAB_SIZE      5

#if DROPBEAR_AES
#define LTC_RIJNDAEL
#endif
/* _TABLES tells it to use tables during setup, _SMALL means to use the smaller scheduled key format
 * (saves 4KB of ram), _ALL_TABLES enables all tables during setup */
#if DROPBEAR_TWOFISH
#define LTC_TWOFISH
#define LTC_TWOFISH_SMALL
#endif

#if DROPBEAR_3DES
#define LTC_DES
#endif

#if DROPBEAR_ENABLE_CBC_MODE
#define LTC_CBC_MODE
#endif

#if DROPBEAR_ENABLE_CTR_MODE
#define LTC_CTR_MODE
#endif

#if DROPBEAR_ENABLE_GCM_MODE
#define LTC_GCM_MODE
#endif

#if DROPBEAR_CHACHA20POLY1305
#define LTC_CHACHA
#define LTC_POLY1305
#endif

#if DROPBEAR_SHA512
#define LTC_SHA512
#endif

#if DROPBEAR_SHA384
#define LTC_SHA384
#endif

#if DROPBEAR_SHA256
#define LTC_SHA256
#endif

#define LTC_SHA1

#if DROPBEAR_MD5
#define LTC_MD5
#endif

/* ECC */
#if DROPBEAR_ECC
#define LTC_MECC
#define LTM_DESC

/* use Shamir's trick for point mul (speeds up signature verification) */
#define LTC_ECC_SHAMIR

#if DROPBEAR_ECC_256
#define LTC_ECC256
#endif
#if DROPBEAR_ECC_384
#define LTC_ECC384
#endif
#if DROPBEAR_ECC_521
#define LTC_ECC521
#endif

#endif /* DROPBEAR_ECC */

#define LTC_HMAC
#define LTC_HASH_HELPERS

#define LTC_NO_TEST

#define LTC_BASE64

/* end Dropbear config */
