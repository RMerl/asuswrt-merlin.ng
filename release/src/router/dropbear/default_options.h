#ifndef DROPBEAR_DEFAULT_OPTIONS_H_
#define DROPBEAR_DEFAULT_OPTIONS_H_
/*
                     > > > Read This < < <

default_options.h  documents compile-time options, and provides default values.

Local customisation should be added to localoptions.h which is
used if it exists. Options defined there will override any options in this
file.

Options can also be defined with -DDROPBEAR_XXX=[0,1] in Makefile CFLAGS

IMPORTANT: Some options will require "make clean" after changes */

#define DROPBEAR_DEFPORT "22"

/* Listen on all interfaces */
#define DROPBEAR_DEFADDRESS ""

/* Default hostkey paths - these can be specified on the command line */
#define DSS_PRIV_FILENAME "/etc/dropbear/dropbear_dss_host_key"
#define RSA_PRIV_FILENAME "/etc/dropbear/dropbear_rsa_host_key"
#define ECDSA_PRIV_FILENAME "/etc/dropbear/dropbear_ecdsa_host_key"

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
#define NON_INETD_MODE 1
#define INETD_MODE 1

/* Include verbose debug output, enabled with -v at runtime. 
 * This will add a reasonable amount to your executable size. */
#define DEBUG_TRACE 0

/* Set this if you want to use the DROPBEAR_SMALL_CODE option. This can save
 * several kB in binary size however will make the symmetrical ciphers and hashes
 * slower, perhaps by 50%. Recommended for small systems that aren't doing
 * much traffic. */
#define DROPBEAR_SMALL_CODE 1

/* Enable X11 Forwarding - server only */
#define DROPBEAR_X11FWD 1

/* Enable TCP Fowarding */
/* 'Local' is "-L" style (client listening port forwarded via server)
 * 'Remote' is "-R" style (server listening port forwarded via client) */
#define DROPBEAR_CLI_LOCALTCPFWD 1
#define DROPBEAR_CLI_REMOTETCPFWD 1

#define DROPBEAR_SVR_LOCALTCPFWD 1
#define DROPBEAR_SVR_REMOTETCPFWD 1

/* Enable Authentication Agent Forwarding */
#define DROPBEAR_SVR_AGENTFWD 1
#define DROPBEAR_CLI_AGENTFWD 1

/* Note: Both DROPBEAR_CLI_PROXYCMD and DROPBEAR_CLI_NETCAT must be set to
 * allow multihop dbclient connections */

/* Allow using -J <proxycommand> to run the connection through a 
   pipe to a program, rather the normal TCP connection */
#define DROPBEAR_CLI_PROXYCMD 1

/* Enable "Netcat mode" option. This will forward standard input/output
 * to a remote TCP-forwarded connection */
#define DROPBEAR_CLI_NETCAT 1

/* Whether to support "-c" and "-m" flags to choose ciphers/MACs at runtime */
#define DROPBEAR_USER_ALGO_LIST 1

/* Encryption - at least one required.
 * AES128 should be enabled, some very old implementations might only
 * support 3DES.
 * Including both AES keysize variants (128 and 256) will result in 
 * a minimal size increase */
#define DROPBEAR_AES128 1
#define DROPBEAR_3DES 1
#define DROPBEAR_AES256 1
#define DROPBEAR_TWOFISH256 0
#define DROPBEAR_TWOFISH128 0
/* Compiling in Blowfish will add ~6kB to runtime heap memory usage */
#define DROPBEAR_BLOWFISH 0

/* Enable CBC mode for ciphers. This has security issues though
 * is the most compatible with older SSH implementations */
#define DROPBEAR_ENABLE_CBC_MODE 1

/* Enable "Counter Mode" for ciphers. This is more secure than
 * CBC mode against certain attacks. It is recommended for security
 * and forwards compatibility */
#define DROPBEAR_ENABLE_CTR_MODE 1

/* Message integrity. sha2-256 is recommended as a default, 
   sha1 for compatibility */
#define DROPBEAR_SHA1_HMAC 1
#define DROPBEAR_SHA1_96_HMAC 1
#define DROPBEAR_SHA2_256_HMAC 1

/* Hostkey/public key algorithms - at least one required, these are used
 * for hostkey as well as for verifying signatures with pubkey auth.
 * Removing either of these won't save very much space.
 * RSA is recommended
 * DSS may be necessary to connect to some systems though
   is not recommended for new keys */
#define DROPBEAR_RSA 1
#define DROPBEAR_DSS 1
/* ECDSA is significantly faster than RSA or DSS. Compiling in ECC
 * code (either ECDSA or ECDH) increases binary size - around 30kB
 * on x86-64 */
#define DROPBEAR_ECDSA 1

/* RSA must be >=1024 */
#define DROPBEAR_DEFAULT_RSA_SIZE 2048
/* DSS is always 1024 */
/* ECDSA defaults to largest size configured, usually 521 */

/* Add runtime flag "-R" to generate hostkeys as-needed when the first 
   connection using that key type occurs.
   This avoids the need to otherwise run "dropbearkey" and avoids some problems
   with badly seeded /dev/urandom when systems first boot. */
#define DROPBEAR_DELAY_HOSTKEY 1


/* Key exchange algorithm.

 * group14_sha1 - 2048 bit, sha1
 * group14_sha256 - 2048 bit, sha2-256
 * group16 - 4096 bit, sha2-512
 * group1 - 1024 bit, sha1
 * curve25519 - elliptic curve DH
 * ecdh - NIST elliptic curve DH (256, 384, 521)
 *
 * group1 is too small for security though is necessary if you need 
     compatibility with some implementations such as Dropbear versions < 0.53
 * group14 is supported by most implementations.
 * group16 provides a greater strength level but is slower and increases binary size
 * curve25519 and ecdh algorithms are faster than non-elliptic curve methods
 * curve25519 increases binary size by ~8kB on x86-64
 * including either ECDH or ECDSA increases binary size by ~30kB on x86-64

 * Small systems should generally include either curve25519 or ecdh for performance.
 * curve25519 is less widely supported but is faster
 */ 
#define DROPBEAR_DH_GROUP14_SHA1 1
#define DROPBEAR_DH_GROUP14_SHA256 1
#define DROPBEAR_DH_GROUP16 0
#define DROPBEAR_CURVE25519 1
#define DROPBEAR_ECDH 1
#define DROPBEAR_DH_GROUP1 1

/* When group1 is enabled it will only be allowed by Dropbear client
not as a server, due to concerns over its strength. Set to 0 to allow
group1 in Dropbear server too */
#define DROPBEAR_DH_GROUP1_CLIENTONLY 1

/* Control the memory/performance/compression tradeoff for zlib.
 * Set windowBits=8 for least memory usage, see your system's
 * zlib.h for full details.
 * Default settings (windowBits=15) will use 256kB for compression
 * windowBits=8 will use 129kB for compression.
 * Both modes will use ~35kB for decompression (using windowBits=15 for
 * interoperability) */
#define DROPBEAR_ZLIB_WINDOW_BITS 15 

/* Whether to do reverse DNS lookups. */
#define DO_HOST_LOOKUP 0

/* Whether to print the message of the day (MOTD). */
#define DO_MOTD 0
#define MOTD_FILENAME "/etc/motd"

/* Authentication Types - at least one required.
   RFC Draft requires pubkey auth, and recommends password */
#define DROPBEAR_SVR_PASSWORD_AUTH 1

/* Note: PAM auth is quite simple and only works for PAM modules which just do
 * a simple "Login: " "Password: " (you can edit the strings in svr-authpam.c).
 * It's useful for systems like OS X where standard password crypts don't work
 * but there's an interface via a PAM module. It won't work for more complex
 * PAM challenge/response.
 * You can't enable both PASSWORD and PAM. */
#define DROPBEAR_SVR_PAM_AUTH 0

/* ~/.ssh/authorized_keys authentication */
#define DROPBEAR_SVR_PUBKEY_AUTH 1

/* Whether to take public key options in 
 * authorized_keys file into account */
#define DROPBEAR_SVR_PUBKEY_OPTIONS 1

/* Client authentication options */
#define DROPBEAR_CLI_PASSWORD_AUTH 1
#define DROPBEAR_CLI_PUBKEY_AUTH 1

/* A default argument for dbclient -i <privatekey>. 
Homedir is prepended unless path begins with / */
#define DROPBEAR_DEFAULT_CLI_AUTHKEY ".ssh/id_dropbear"

/* Allow specifying the password for dbclient via the DROPBEAR_PASSWORD
 * environment variable. */
#define DROPBEAR_USE_PASSWORD_ENV 1

/* Define this (as well as DROPBEAR_CLI_PASSWORD_AUTH) to allow the use of
 * a helper program for the ssh client. The helper program should be
 * specified in the SSH_ASKPASS environment variable, and dbclient
 * should be run with DISPLAY set and no tty. The program should
 * return the password on standard output */
#define DROPBEAR_CLI_ASKPASS_HELPER 0

/* Save a network roundtrip by sendng a real auth request immediately after
 * sending a query for the available methods. This is not yet enabled by default 
 since it could cause problems with non-compliant servers */ 
#define DROPBEAR_CLI_IMMEDIATE_AUTH 0

/* Set this to use PRNGD or EGD instead of /dev/urandom */
#define DROPBEAR_USE_PRNGD 0
#define DROPBEAR_PRNGD_SOCKET "/var/run/dropbear-rng"

/* Specify the number of clients we will allow to be connected but
 * not yet authenticated. After this limit, connections are rejected */
/* The first setting is per-IP, to avoid denial of service */
#define MAX_UNAUTH_PER_IP 5

/* And then a global limit to avoid chewing memory if connections 
 * come from many IPs */
#define MAX_UNAUTH_CLIENTS 30

/* Default maximum number of failed authentication tries (server option) */
/* -T server option overrides */
#define MAX_AUTH_TRIES 10

/* The default file to store the daemon's process ID, for shutdown
   scripts etc. This can be overridden with the -P flag */
#define DROPBEAR_PIDFILE "/var/run/dropbear.pid"

/* The command to invoke for xauth when using X11 forwarding.
 * "-q" for quiet */
#define XAUTH_COMMAND "/usr/bin/xauth -q"


/* if you want to enable running an sftp server (such as the one included with
 * OpenSSH), set the path below and set DROPBEAR_SFTPSERVER. 
 * The sftp-server program is not provided by Dropbear itself */
#define DROPBEAR_SFTPSERVER 1
#define SFTPSERVER_PATH "/usr/libexec/sftp-server"

/* This is used by the scp binary when used as a client binary. If you're
 * not using the Dropbear client, you'll need to change it */
#define DROPBEAR_PATH_SSH_PROGRAM "/usr/bin/dbclient"

/* Whether to log commands executed by a client. This only logs the 
 * (single) command sent to the server, not what a user did in a 
 * shell/sftp session etc. */
#define LOG_COMMANDS 0

/* Window size limits. These tend to be a trade-off between memory
   usage and network performance: */
/* Size of the network receive window. This amount of memory is allocated
   as a per-channel receive buffer. Increasing this value can make a
   significant difference to network performance. 24kB was empirically
   chosen for a 100mbit ethernet network. The value can be altered at
   runtime with the -W argument. */
#define DEFAULT_RECV_WINDOW 24576
/* Maximum size of a received SSH data packet - this _MUST_ be >= 32768
   in order to interoperate with other implementations */
#define RECV_MAX_PAYLOAD_LEN 32768
/* Maximum size of a transmitted data packet - this can be any value,
   though increasing it may not make a significant difference. */
#define TRANS_MAX_PAYLOAD_LEN 16384

/* Ensure that data is transmitted every KEEPALIVE seconds. This can
be overridden at runtime with -K. 0 disables keepalives */
#define DEFAULT_KEEPALIVE 0

/* If this many KEEPALIVES are sent with no packets received from the
other side, exit. Not run-time configurable - if you have a need
for runtime configuration please mail the Dropbear list */
#define DEFAULT_KEEPALIVE_LIMIT 3

/* Ensure that data is received within IDLE_TIMEOUT seconds. This can
be overridden at runtime with -I. 0 disables idle timeouts */
#define DEFAULT_IDLE_TIMEOUT 0

/* The default path. This will often get replaced by the shell */
#define DEFAULT_PATH "/usr/bin:/bin"

#endif /* DROPBEAR_DEFAULT_OPTIONS_H_ */
