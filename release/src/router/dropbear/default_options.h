#ifndef DROPBEAR_DEFAULT_OPTIONS_H_
#define DROPBEAR_DEFAULT_OPTIONS_H_
/*
                     > > > Read This < < <

default_options.h.in (this file) documents compile-time options, and provides 
default values.

Local customisation should be added to localoptions.h which is
used if it exists. Options defined there will override any options in this
file (#ifndef guards added by ifndef_wrapper.sh).

Options can also be defined with -DDROPBEAR_XXX Makefile CFLAGS

IMPORTANT: Many options will require "make clean" after changes */

#ifndef DROPBEAR_DEFPORT
#define DROPBEAR_DEFPORT "22"
#endif

/* Listen on all interfaces */
#ifndef DROPBEAR_DEFADDRESS
#define DROPBEAR_DEFADDRESS ""
#endif

/* Default hostkey paths - these can be specified on the command line */
#ifndef DSS_PRIV_FILENAME
#define DSS_PRIV_FILENAME "/etc/dropbear/dropbear_dss_host_key"
#endif
#ifndef RSA_PRIV_FILENAME
#define RSA_PRIV_FILENAME "/etc/dropbear/dropbear_rsa_host_key"
#endif
#ifndef ECDSA_PRIV_FILENAME
#define ECDSA_PRIV_FILENAME "/etc/dropbear/dropbear_ecdsa_host_key"
#endif

/* Set NON_INETD_MODE if you require daemon functionality (ie Dropbear listens
 * on chosen ports and keeps accepting connections. This is the default.
 *
 * Set INETD_MODE if you want to be able to run Dropbear with inetd (or
 * similar), where it will use stdin/stdout for connections, and each process
 * lasts for a single connection. Dropbear should be invoked with the -i flag
 * for inetd, and can only accept IPv4 connections.
 *
 * Both of these flags can be defined at once, don't compile without at least
 * one of them. */
#ifndef NON_INETD_MODE
#define NON_INETD_MODE 1
#endif
#ifndef INETD_MODE
#define INETD_MODE 1
#endif

/* Setting this disables the fast exptmod bignum code. It saves ~5kB, but is
 * perhaps 20% slower for pubkey operations (it is probably worth experimenting
 * if you want to use this) */
/*#define NO_FAST_EXPTMOD*/

/* Set this if you want to use the DROPBEAR_SMALL_CODE option. This can save
several kB in binary size however will make the symmetrical ciphers and hashes
slower, perhaps by 50%. Recommended for small systems that aren't doing
much traffic. */
#ifndef DROPBEAR_SMALL_CODE
#define DROPBEAR_SMALL_CODE 1
#endif

/* Enable X11 Forwarding - server only */
#ifndef DROPBEAR_X11FWD
#define DROPBEAR_X11FWD 1
#endif

/* Enable TCP Fowarding */
/* 'Local' is "-L" style (client listening port forwarded via server)
 * 'Remote' is "-R" style (server listening port forwarded via client) */

#ifndef DROPBEAR_CLI_LOCALTCPFWD
#define DROPBEAR_CLI_LOCALTCPFWD 1
#endif
#ifndef DROPBEAR_CLI_REMOTETCPFWD
#define DROPBEAR_CLI_REMOTETCPFWD 1
#endif

#ifndef DROPBEAR_SVR_LOCALTCPFWD
#define DROPBEAR_SVR_LOCALTCPFWD 1
#endif
#ifndef DROPBEAR_SVR_REMOTETCPFWD
#define DROPBEAR_SVR_REMOTETCPFWD 1
#endif

/* Enable Authentication Agent Forwarding */
#ifndef DROPBEAR_SVR_AGENTFWD
#define DROPBEAR_SVR_AGENTFWD 1
#endif
#ifndef DROPBEAR_CLI_AGENTFWD
#define DROPBEAR_CLI_AGENTFWD 1
#endif


/* Note: Both DROPBEAR_CLI_PROXYCMD and DROPBEAR_CLI_NETCAT must be set to
 * allow multihop dbclient connections */

/* Allow using -J <proxycommand> to run the connection through a 
   pipe to a program, rather the normal TCP connection */
#ifndef DROPBEAR_CLI_PROXYCMD
#define DROPBEAR_CLI_PROXYCMD 1
#endif

/* Enable "Netcat mode" option. This will forward standard input/output
 * to a remote TCP-forwarded connection */
#ifndef DROPBEAR_CLI_NETCAT
#define DROPBEAR_CLI_NETCAT 1
#endif

/* Whether to support "-c" and "-m" flags to choose ciphers/MACs at runtime */
#ifndef ENABLE_USER_ALGO_LIST
#define ENABLE_USER_ALGO_LIST 1
#endif

/* Encryption - at least one required.
 * Protocol RFC requires 3DES and recommends AES128 for interoperability.
 * Including multiple keysize variants the same cipher 
 * (eg AES256 as well as AES128) will result in a minimal size increase.*/
#ifndef DROPBEAR_AES128
#define DROPBEAR_AES128 1
#endif
#ifndef DROPBEAR_3DES
#define DROPBEAR_3DES 1
#endif
#ifndef DROPBEAR_AES256
#define DROPBEAR_AES256 1
#endif
/* Compiling in Blowfish will add ~6kB to runtime heap memory usage */
/*#define DROPBEAR_BLOWFISH*/
#ifndef DROPBEAR_TWOFISH256
#define DROPBEAR_TWOFISH256 1
#endif
#ifndef DROPBEAR_TWOFISH128
#define DROPBEAR_TWOFISH128 1
#endif

/* Enable CBC mode for ciphers. This has security issues though
 * is the most compatible with older SSH implementations */
#ifndef DROPBEAR_ENABLE_CBC_MODE
#define DROPBEAR_ENABLE_CBC_MODE 0
#endif

/* Enable "Counter Mode" for ciphers. This is more secure than normal
 * CBC mode against certain attacks. It is recommended for security
 * and forwards compatibility */
#ifndef DROPBEAR_ENABLE_CTR_MODE
#define DROPBEAR_ENABLE_CTR_MODE 1
#endif

/* Twofish counter mode is disabled by default because it 
has not been tested for interoperability with other SSH implementations.
If you test it please contact the Dropbear author */
#ifndef DROPBEAR_TWOFISH_CTR
#define DROPBEAR_TWOFISH_CTR 0
#endif

/* Message integrity. sha2-256 is recommended as a default, 
   sha1 for compatibility */
#ifndef DROPBEAR_SHA1_HMAC
#define DROPBEAR_SHA1_HMAC 1
#endif
#ifndef DROPBEAR_SHA1_96_HMAC
#define DROPBEAR_SHA1_96_HMAC 0
#endif
#ifndef DROPBEAR_SHA2_256_HMAC
#define DROPBEAR_SHA2_256_HMAC 1
#endif
/* Default is to include it is sha512 is being compiled in for ECDSA */
#ifndef DROPBEAR_SHA2_512_HMAC
#define DROPBEAR_SHA2_512_HMAC (DROPBEAR_ECDSA)
#endif

/* XXX needed for fingerprints */
#ifndef DROPBEAR_MD5_HMAC
#define DROPBEAR_MD5_HMAC 0
#endif

/* Hostkey/public key algorithms - at least one required, these are used
 * for hostkey as well as for verifying signatures with pubkey auth.
 * Removing either of these won't save very much space.
 * RSA is recommended
 * DSS may be necessary to connect to some systems though
   is not recommended for new keys */
#ifndef DROPBEAR_RSA
#define DROPBEAR_RSA 1
#endif
#ifndef DROPBEAR_DSS
#define DROPBEAR_DSS 1
#endif
/* ECDSA is significantly faster than RSA or DSS. Compiling in ECC
 * code (either ECDSA or ECDH) increases binary size - around 30kB
 * on x86-64 */
#ifndef DROPBEAR_ECDSA
#define DROPBEAR_ECDSA 1
#endif

/* Add runtime flag "-R" to generate hostkeys as-needed when the first 
   connection using that key type occurs.
   This avoids the need to otherwise run "dropbearkey" and avoids some problems
   with badly seeded /dev/urandom when systems first boot. */
#ifndef DROPBEAR_DELAY_HOSTKEY
#define DROPBEAR_DELAY_HOSTKEY 1
#endif

/* Enable Curve25519 for key exchange. This is another elliptic
 * curve method with good security properties. Increases binary size
 * by ~8kB on x86-64 */
#ifndef DROPBEAR_CURVE25519
#define DROPBEAR_CURVE25519 1
#endif

/* Enable elliptic curve Diffie Hellman key exchange, see note about
 * ECDSA above */
#ifndef DROPBEAR_ECDH
#define DROPBEAR_ECDH 1
#endif

/* Key exchange algorithm.
 * group14_sha1 - 2048 bit, sha1
 * group14_sha256 - 2048 bit, sha2-256
 * group16 - 4096 bit, sha2-512
 * group1 - 1024 bit, sha1
 *
 * group14 is supported by most implementations.
 * group16 provides a greater strength level but is slower and increases binary size
 * group1 is too small for security though is necessary if you need 
     compatibility with some implementations such as Dropbear versions < 0.53
 */ 
#ifndef DROPBEAR_DH_GROUP1
#define DROPBEAR_DH_GROUP1 1
#endif
#ifndef DROPBEAR_DH_GROUP14_SHA1
#define DROPBEAR_DH_GROUP14_SHA1 1
#endif
#ifndef DROPBEAR_DH_GROUP14_SHA256
#define DROPBEAR_DH_GROUP14_SHA256 1
#endif
#ifndef DROPBEAR_DH_GROUP16
#define DROPBEAR_DH_GROUP16 0
#endif

/* Control the memory/performance/compression tradeoff for zlib.
 * Set windowBits=8 for least memory usage, see your system's
 * zlib.h for full details.
 * Default settings (windowBits=15) will use 256kB for compression
 * windowBits=8 will use 129kB for compression.
 * Both modes will use ~35kB for decompression (using windowBits=15 for
 * interoperability) */
#ifndef DROPBEAR_ZLIB_WINDOW_BITS
#define DROPBEAR_ZLIB_WINDOW_BITS 15 
#endif

/* Whether to do reverse DNS lookups. */
#ifndef DO_HOST_LOOKUP
#define DO_HOST_LOOKUP 0
#endif

/* Whether to print the message of the day (MOTD). */
#ifndef DO_MOTD
#define DO_MOTD 0
#endif

/* The MOTD file path */
#ifndef MOTD_FILENAME
#define MOTD_FILENAME "/etc/motd"
#endif

/* Authentication Types - at least one required.
   RFC Draft requires pubkey auth, and recommends password */

/* Note: PAM auth is quite simple and only works for PAM modules which just do
 * a simple "Login: " "Password: " (you can edit the strings in svr-authpam.c).
 * It's useful for systems like OS X where standard password crypts don't work
 * but there's an interface via a PAM module. It won't work for more complex
 * PAM challenge/response.
 * You can't enable both PASSWORD and PAM. */

/* This requires crypt() */
#ifdef HAVE_CRYPT
#ifndef DROPBEAR_SVR_PASSWORD_AUTH
#define DROPBEAR_SVR_PASSWORD_AUTH 1
#endif
#else
#ifndef DROPBEAR_SVR_PASSWORD_AUTH
#define DROPBEAR_SVR_PASSWORD_AUTH 0
#endif
#endif
/* PAM requires ./configure --enable-pam */
#ifndef DROPBEAR_SVR_PAM_AUTH
#define DROPBEAR_SVR_PAM_AUTH 0
#endif
#ifndef DROPBEAR_SVR_PUBKEY_AUTH
#define DROPBEAR_SVR_PUBKEY_AUTH 1
#endif

/* Whether to take public key options in 
 * authorized_keys file into account */
#ifndef DROPBEAR_SVR_PUBKEY_OPTIONS
#define DROPBEAR_SVR_PUBKEY_OPTIONS 1
#endif

/* This requires getpass. */
#ifdef HAVE_GETPASS
#ifndef DROPBEAR_CLI_PASSWORD_AUTH
#define DROPBEAR_CLI_PASSWORD_AUTH 1
#endif
#ifndef DROPBEAR_CLI_INTERACT_AUTH
#define DROPBEAR_CLI_INTERACT_AUTH 1
#endif
#endif
#ifndef DROPBEAR_CLI_PUBKEY_AUTH
#define DROPBEAR_CLI_PUBKEY_AUTH 1
#endif

/* A default argument for dbclient -i <privatekey>. 
Homedir is prepended unless path begins with / */
#ifndef DROPBEAR_DEFAULT_CLI_AUTHKEY
#define DROPBEAR_DEFAULT_CLI_AUTHKEY ".ssh/id_dropbear"
#endif

/* This variable can be used to set a password for client
 * authentication on the commandline. Beware of platforms
 * that don't protect environment variables of processes etc. Also
 * note that it will be provided for all "hidden" client-interactive
 * style prompts - if you want something more sophisticated, use 
 * SSH_ASKPASS instead. Comment out this var to remove this functionality.*/
#ifndef DROPBEAR_PASSWORD_ENV
#define DROPBEAR_PASSWORD_ENV "DROPBEAR_PASSWORD"
#endif

/* Define this (as well as DROPBEAR_CLI_PASSWORD_AUTH) to allow the use of
 * a helper program for the ssh client. The helper program should be
 * specified in the SSH_ASKPASS environment variable, and dbclient
 * should be run with DISPLAY set and no tty. The program should
 * return the password on standard output */
#ifndef DROPBEAR_CLI_ASKPASS_HELPER
#define DROPBEAR_CLI_ASKPASS_HELPER 0
#endif

/* Save a network roundtrip by sendng a real auth request immediately after
 * sending a query for the available methods.  It is at the expense of < 100
 * bytes of extra network traffic. This is not yet enabled by default since it
 * could cause problems with non-compliant servers */
#ifndef DROPBEAR_CLI_IMMEDIATE_AUTH
#define DROPBEAR_CLI_IMMEDIATE_AUTH 0
#endif

/* Source for randomness. This must be able to provide hundreds of bytes per SSH
 * connection without blocking. In addition /dev/random is used for seeding
 * rsa/dss key generation */
#ifndef DROPBEAR_URANDOM_DEV
#define DROPBEAR_URANDOM_DEV "/dev/urandom"
#endif

/* Set this to use PRNGD or EGD instead of /dev/urandom or /dev/random */
/*#define DROPBEAR_PRNGD_SOCKET "/var/run/dropbear-rng"*/


/* Specify the number of clients we will allow to be connected but
 * not yet authenticated. After this limit, connections are rejected */
/* The first setting is per-IP, to avoid denial of service */
#ifndef MAX_UNAUTH_PER_IP
#define MAX_UNAUTH_PER_IP 5
#endif

/* And then a global limit to avoid chewing memory if connections 
 * come from many IPs */
#ifndef MAX_UNAUTH_CLIENTS
#define MAX_UNAUTH_CLIENTS 30
#endif

/* Maximum number of failed authentication tries (server option) */
#ifndef MAX_AUTH_TRIES
#define MAX_AUTH_TRIES 3
#endif

/* The default file to store the daemon's process ID, for shutdown
   scripts etc. This can be overridden with the -P flag */
#ifndef DROPBEAR_PIDFILE
#define DROPBEAR_PIDFILE "/var/run/dropbear.pid"
#endif

/* The command to invoke for xauth when using X11 forwarding.
 * "-q" for quiet */
#ifndef XAUTH_COMMAND
#define XAUTH_COMMAND "/usr/bin/xauth -q"
#endif

/* if you want to enable running an sftp server (such as the one included with
 * OpenSSH), set the path below. If the path isn't defined, sftp will not
 * be enabled */
#ifndef SFTPSERVER_PATH
#define SFTPSERVER_PATH "/usr/libexec/sftp-server"
#endif

/* This is used by the scp binary when used as a client binary. If you're
 * not using the Dropbear client, you'll need to change it */
#ifndef DROPBEAR_PATH_SSH_PROGRAM
#define DROPBEAR_PATH_SSH_PROGRAM "/usr/bin/dbclient"
#endif

/* Whether to log commands executed by a client. This only logs the 
 * (single) command sent to the server, not what a user did in a 
 * shell/sftp session etc. */
#ifndef LOG_COMMANDS
#define LOG_COMMANDS 0
#endif

/* Window size limits. These tend to be a trade-off between memory
   usage and network performance: */
/* Size of the network receive window. This amount of memory is allocated
   as a per-channel receive buffer. Increasing this value can make a
   significant difference to network performance. 24kB was empirically
   chosen for a 100mbit ethernet network. The value can be altered at
   runtime with the -W argument. */
#ifndef DEFAULT_RECV_WINDOW
#define DEFAULT_RECV_WINDOW 24576
#endif
/* Maximum size of a received SSH data packet - this _MUST_ be >= 32768
   in order to interoperate with other implementations */
#ifndef RECV_MAX_PAYLOAD_LEN
#define RECV_MAX_PAYLOAD_LEN 32768
#endif
/* Maximum size of a transmitted data packet - this can be any value,
   though increasing it may not make a significant difference. */
#ifndef TRANS_MAX_PAYLOAD_LEN
#define TRANS_MAX_PAYLOAD_LEN 16384
#endif

/* Ensure that data is transmitted every KEEPALIVE seconds. This can
be overridden at runtime with -K. 0 disables keepalives */
#ifndef DEFAULT_KEEPALIVE
#define DEFAULT_KEEPALIVE 0
#endif

/* If this many KEEPALIVES are sent with no packets received from the
other side, exit. Not run-time configurable - if you have a need
for runtime configuration please mail the Dropbear list */
#ifndef DEFAULT_KEEPALIVE_LIMIT
#define DEFAULT_KEEPALIVE_LIMIT 3
#endif

/* Ensure that data is received within IDLE_TIMEOUT seconds. This can
be overridden at runtime with -I. 0 disables idle timeouts */
#ifndef DEFAULT_IDLE_TIMEOUT
#define DEFAULT_IDLE_TIMEOUT 0
#endif

/* The default path. This will often get replaced by the shell */
#ifndef DEFAULT_PATH
#define DEFAULT_PATH "/usr/bin:/bin"
#endif

#endif /* DROPBEAR_DEFAULT_OPTIONS_H_ */
