/*******************************************************************
 * You shouldn't edit this file unless you know you need to. 
 * This file is only included from options.h
 *******************************************************************/

#ifndef DROPBEAR_VERSION
#define DROPBEAR_VERSION "2018.76"
#endif

#define LOCAL_IDENT "SSH-2.0-dropbear_" DROPBEAR_VERSION
#define PROGNAME "dropbear"

/* Spec recommends after one hour or 1 gigabyte of data. One hour
 * is a bit too verbose, so we try 8 hours */
#ifndef KEX_REKEY_TIMEOUT
#define KEX_REKEY_TIMEOUT (3600 * 8)
#endif
#ifndef KEX_REKEY_DATA
#define KEX_REKEY_DATA (1<<30) /* 2^30 == 1GB, this value must be < INT_MAX */
#endif
/* Close connections to clients which haven't authorised after AUTH_TIMEOUT */
#ifndef AUTH_TIMEOUT
#define AUTH_TIMEOUT 300 /* we choose 5 minutes */
#endif

#define DROPBEAR_SVR_PUBKEY_OPTIONS_BUILT ((DROPBEAR_SVR_PUBKEY_AUTH) && (DROPBEAR_SVR_PUBKEY_OPTIONS))

#if !(NON_INETD_MODE || INETD_MODE)
	#error "NON_INETD_MODE or INETD_MODE (or both) must be enabled."
#endif

/* A client should try and send an initial key exchange packet guessing
 * the algorithm that will match - saves a round trip connecting, has little
 * overhead if the guess was "wrong". */
#ifndef DROPBEAR_KEX_FIRST_FOLLOWS
#define DROPBEAR_KEX_FIRST_FOLLOWS 1
#endif
/* Use protocol extension to allow "first follows" to succeed more frequently.
 * This is currently Dropbear-specific but will gracefully fallback when connecting
 * to other implementations. */
#ifndef DROPBEAR_KEXGUESS2
#define DROPBEAR_KEXGUESS2 1
#endif

/* Minimum key sizes for DSS and RSA */
#ifndef MIN_DSS_KEYLEN
#define MIN_DSS_KEYLEN 1024
#endif
#ifndef MIN_RSA_KEYLEN
#define MIN_RSA_KEYLEN 1024
#endif

#define MAX_BANNER_SIZE 2000 /* this is 25*80 chars, any more is foolish */
#define MAX_BANNER_LINES 20 /* How many lines the client will display */

/* the number of NAME=VALUE pairs to malloc for environ, if we don't have
 * the clearenv() function */
#define ENV_SIZE 100

#define MAX_CMD_LEN 9000 /* max length of a command */
#define MAX_TERM_LEN 200 /* max length of TERM name */

#define MAX_HOST_LEN 254 /* max hostname len for tcp fwding */
#define MAX_IP_LEN 15 /* strlen("255.255.255.255") == 15 */

#define DROPBEAR_MAX_PORTS 10 /* max number of ports which can be specified,
								 ipv4 and ipv6 don't count twice */

/* Each port might have at least a v4 and a v6 address */
#define MAX_LISTEN_ADDR (DROPBEAR_MAX_PORTS*3)

#define _PATH_TTY "/dev/tty"

#define _PATH_CP "/bin/cp"

#define DROPBEAR_ESCAPE_CHAR '~'

/* success/failure defines */
#define DROPBEAR_SUCCESS 0
#define DROPBEAR_FAILURE -1
 
#define DROPBEAR_PASSWORD_ENV "DROPBEAR_PASSWORD"

#define DROPBEAR_NGROUP_MAX 1024

/* Required for pubkey auth */
#define DROPBEAR_SIGNKEY_VERIFY ((DROPBEAR_SVR_PUBKEY_AUTH) || (DROPBEAR_CLIENT))

#define SHA1_HASH_SIZE 20
#define MD5_HASH_SIZE 16
#define MAX_HASH_SIZE 64 /* sha512 */

#define MAX_KEY_LEN 32 /* 256 bits for aes256 etc */
#define MAX_IV_LEN 20 /* must be same as max blocksize,  */

#if DROPBEAR_SHA2_512_HMAC
#define MAX_MAC_LEN 64
#elif DROPBEAR_SHA2_256_HMAC
#define MAX_MAC_LEN 32
#else
#define MAX_MAC_LEN 20
#endif

/* sha2-512 is not necessary unless unforseen problems arise with sha2-256 */
#ifndef DROPBEAR_SHA2_512_HMAC
#define DROPBEAR_SHA2_512_HMAC 0
#endif

/* might be needed for compatibility with very old implementations */
#ifndef DROPBEAR_MD5_HMAC
#define DROPBEAR_MD5_HMAC 0
#endif

/* Twofish counter mode is disabled by default because it 
has not been tested for interoperability with other SSH implementations.
If you test it please contact the Dropbear author */
#ifndef DROPBEAR_TWOFISH_CTR
#define DROPBEAR_TWOFISH_CTR 0
#endif


#define DROPBEAR_ECC ((DROPBEAR_ECDH) || (DROPBEAR_ECDSA))

/* Debian doesn't define this in system headers */
#if !defined(LTM_DESC) && (DROPBEAR_ECC)
#define LTM_DESC 
#endif

#define DROPBEAR_ECC_256 (DROPBEAR_ECC)
#define DROPBEAR_ECC_384 (DROPBEAR_ECC)
#define DROPBEAR_ECC_521 (DROPBEAR_ECC)

#define DROPBEAR_LTC_PRNG (DROPBEAR_ECC)

/* RSA can be vulnerable to timing attacks which use the time required for
 * signing to guess the private key. Blinding avoids this attack, though makes
 * signing operations slightly slower. */
#define DROPBEAR_RSA_BLINDING 1

/* hashes which will be linked and registered */
#define DROPBEAR_SHA256 ((DROPBEAR_SHA2_256_HMAC) || (DROPBEAR_ECC_256)  \
 			|| (DROPBEAR_CURVE25519) || (DROPBEAR_DH_GROUP14_SHA256))
#define DROPBEAR_SHA384 (DROPBEAR_ECC_384)
/* LTC SHA384 depends on SHA512 */
#define DROPBEAR_SHA512 ((DROPBEAR_SHA2_512_HMAC) || (DROPBEAR_ECC_521) \
			|| (DROPBEAR_SHA384) || (DROPBEAR_DH_GROUP16))
#define DROPBEAR_MD5 (DROPBEAR_MD5_HMAC)

#define DROPBEAR_DH_GROUP14 ((DROPBEAR_DH_GROUP14_SHA256) || (DROPBEAR_DH_GROUP14_SHA1))

#define DROPBEAR_NORMAL_DH ((DROPBEAR_DH_GROUP1) || (DROPBEAR_DH_GROUP14) || (DROPBEAR_DH_GROUP16))

/* roughly 2x 521 bits */
#define MAX_ECC_SIZE 140

#define MAX_NAME_LEN 64 /* maximum length of a protocol name, isn't
						   explicitly specified for all protocols (just
						   for algos) but seems valid */

#define MAX_PROPOSED_ALGO 20

/* size/count limits */
/* From transport rfc */
#define MIN_PACKET_LEN 16

#define RECV_MAX_PACKET_LEN (MAX(35000, ((RECV_MAX_PAYLOAD_LEN)+100)))

/* for channel code */
#define TRANS_MAX_WINDOW 500000000 /* 500MB is sufficient, stopping overflow */
#define TRANS_MAX_WIN_INCR 500000000 /* overflow prevention */

#define RECV_WINDOWEXTEND (opts.recv_window / 3) /* We send a "window extend" every
								RECV_WINDOWEXTEND bytes */
#define MAX_RECV_WINDOW (1024*1024) /* 1 MB should be enough */

#define MAX_CHANNELS 1000 /* simple mem restriction, includes each tcp/x11
							connection, so can't be _too_ small */

#define MAX_STRING_LEN (MAX(MAX_CMD_LEN, 2400)) /* Sun SSH needs 2400 for algos,
                                                   MAX_CMD_LEN is usually longer */

/* For a 4096 bit DSS key, empirically determined */
#define MAX_PUBKEY_SIZE 1700
/* For a 4096 bit DSS key, empirically determined */
#define MAX_PRIVKEY_SIZE 1700

#define MAX_HOSTKEYS 3

/* The maximum size of the bignum portion of the kexhash buffer */
/* Sect. 8 of the transport rfc 4253, K_S + e + f + K */
#define KEXHASHBUF_MAX_INTS (1700 + 130 + 130 + 130)

#define DROPBEAR_MAX_SOCKS 2 /* IPv4, IPv6 are all we'll get for now. Revisit
								in a few years time.... */

#define DROPBEAR_MAX_CLI_PASS 1024

#define DROPBEAR_MAX_CLI_INTERACT_PROMPTS 80 /* The number of prompts we'll 
												accept for keyb-interactive
												auth */


#define DROPBEAR_AES ((DROPBEAR_AES256) || (DROPBEAR_AES128))

#define DROPBEAR_TWOFISH ((DROPBEAR_TWOFISH256) || (DROPBEAR_TWOFISH128))

#define DROPBEAR_CLI_ANYTCPFWD ((DROPBEAR_CLI_REMOTETCPFWD) || (DROPBEAR_CLI_LOCALTCPFWD))

#define DROPBEAR_TCP_ACCEPT ((DROPBEAR_CLI_LOCALTCPFWD) || (DROPBEAR_SVR_REMOTETCPFWD))

#define DROPBEAR_LISTENERS \
   ((DROPBEAR_CLI_REMOTETCPFWD) || (DROPBEAR_CLI_LOCALTCPFWD) || \
	(DROPBEAR_SVR_REMOTETCPFWD) || (DROPBEAR_SVR_LOCALTCPFWD) || \
	(DROPBEAR_SVR_AGENTFWD) || (DROPBEAR_X11FWD))

#define DROPBEAR_CLI_MULTIHOP ((DROPBEAR_CLI_NETCAT) && (DROPBEAR_CLI_PROXYCMD))

#define ENABLE_CONNECT_UNIX ((DROPBEAR_CLI_AGENTFWD) || (DROPBEAR_USE_PRNGD))

/* if we're using authorized_keys or known_hosts */ 
#define DROPBEAR_KEY_LINES ((DROPBEAR_CLIENT) || (DROPBEAR_SVR_PUBKEY_AUTH))

/* Changing this is inadvisable, it appears to have problems
 * with flushing compressed data */
#define DROPBEAR_ZLIB_MEM_LEVEL 8

#if (DROPBEAR_SVR_PASSWORD_AUTH) && (DROPBEAR_SVR_PAM_AUTH)
#error "You can't turn on PASSWORD and PAM auth both at once. Fix it in options.h"
#endif

/* PAM requires ./configure --enable-pam */
#if !defined(HAVE_LIBPAM) && DROPBEAR_SVR_PAM_AUTH
#error "DROPBEAR_SVR_PATM_AUTH requires PAM headers. Perhaps ./configure --enable-pam ?"
#endif

#if DROPBEAR_SVR_PASSWORD_AUTH && !HAVE_CRYPT
	#error "DROPBEAR_SVR_PASSWORD_AUTH requires `crypt()'."
#endif

#if !(DROPBEAR_SVR_PASSWORD_AUTH || DROPBEAR_SVR_PAM_AUTH || DROPBEAR_SVR_PUBKEY_AUTH)
	#error "At least one server authentication type must be enabled. DROPBEAR_SVR_PUBKEY_AUTH and DROPBEAR_SVR_PASSWORD_AUTH are recommended."
#endif


#if !(DROPBEAR_AES128 || DROPBEAR_3DES || DROPBEAR_AES256 || DROPBEAR_BLOWFISH \
      || DROPBEAR_TWOFISH256 || DROPBEAR_TWOFISH128)
	#error "At least one encryption algorithm must be enabled. AES128 is recommended."
#endif

#if !(DROPBEAR_RSA || DROPBEAR_DSS || DROPBEAR_ECDSA)
	#error "At least one hostkey or public-key algorithm must be enabled; RSA is recommended."
#endif

/* Source for randomness. This must be able to provide hundreds of bytes per SSH
 * connection without blocking. */
#ifndef DROPBEAR_URANDOM_DEV
#define DROPBEAR_URANDOM_DEV "/dev/urandom"
#endif

/* client keyboard interactive authentication is often used for password auth.
 rfc4256 */
#define DROPBEAR_CLI_INTERACT_AUTH (DROPBEAR_CLI_PASSWORD_AUTH)

/* We use dropbear_client and dropbear_server as shortcuts to avoid redundant
 * code, if we're just compiling as client or server */
#if (DROPBEAR_SERVER) && (DROPBEAR_CLIENT)

#define IS_DROPBEAR_SERVER (ses.isserver == 1)
#define IS_DROPBEAR_CLIENT (ses.isserver == 0)

#elif DROPBEAR_SERVER

#define IS_DROPBEAR_SERVER 1
#define IS_DROPBEAR_CLIENT 0

#elif DROPBEAR_CLIENT

#define IS_DROPBEAR_SERVER 0
#define IS_DROPBEAR_CLIENT 1

#else
/* Just building key utils? */
#define IS_DROPBEAR_SERVER 0
#define IS_DROPBEAR_CLIENT 0

#endif /* neither DROPBEAR_SERVER nor DROPBEAR_CLIENT */

#ifdef HAVE_FORK
#define DROPBEAR_VFORK 0
#else
#define DROPBEAR_VFORK 1
#endif

#ifndef DROPBEAR_LISTEN_BACKLOG
#if MAX_UNAUTH_CLIENTS > MAX_CHANNELS
#define DROPBEAR_LISTEN_BACKLOG MAX_UNAUTH_CLIENTS
#else
#define DROPBEAR_LISTEN_BACKLOG MAX_CHANNELS
#endif
#endif

/* free memory before exiting */
#define DROPBEAR_CLEANUP 1

/* Use this string since some implementations might special-case it */
#define DROPBEAR_KEEPALIVE_STRING "keepalive@openssh.com"

/* Linux will attempt TCP fast open, falling back if not supported by the kernel.
 * Currently server is enabled but client is disabled by default until there
 * is further compatibility testing */
#ifdef __linux__
#define DROPBEAR_SERVER_TCP_FAST_OPEN 1
#define DROPBEAR_CLIENT_TCP_FAST_OPEN 0
#else
#define DROPBEAR_SERVER_TCP_FAST_OPEN 0
#define DROPBEAR_CLIENT_TCP_FAST_OPEN 0
#endif

/* no include guard for this file */
