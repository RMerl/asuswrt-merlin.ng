/* ASUSWRT-Merlin custom configuration */

/* Override SSH 2.0 ident */
#define LOCAL_IDENT "SSH-2.0-dropbear"

/* Set INETD_MODE if you want to be able to run Dropbear with inetd (or
 * similar), where it will use stdin/stdout for connections, and each process
 * lasts for a single connection. Dropbear should be invoked with the -i flag
 * for inetd, and can only accept IPv4 connections. */
#define INETD_MODE 0

/* Specify the number of clients we will allow to be connected but
 * not yet authenticated. After this limit, connections are rejected */
/* The first setting is per-IP, to avoid denial of service */
#define MAX_UNAUTH_PER_IP 3

/* And then a global limit to avoid chewing memory if connections 
 * come from many IPs */
#define MAX_UNAUTH_CLIENTS 9

/* Default maximum number of failed authentication tries (server option) */
/* -T server option overrides */
#define MAX_AUTH_TRIES 3

/* The command to invoke for xauth when using X11 forwarding.
 * "-q" for quiet */
#define XAUTH_COMMAND "/opt/bin/xauth -q"

/* If you want to enable running an sftp server (such as the one included with
 * OpenSSH), set the path below and set DROPBEAR_SFTPSERVER. 
 * The sftp-server program is not provided by Dropbear itself */
#define DROPBEAR_SFTPSERVER 1
#define SFTPSERVER_PATH "/opt/libexec/sftp-server"

/* The default path. This will often get replaced by the shell */
#define DEFAULT_PATH "/bin:/sbin:/usr/bin:/usr/sbin:/opt/bin:/opt/sbin:/opt/usr/bin:/opt/usr/sbin"

/* Linux will attempt TCP fast open, falling back if not supported by the kernel.
 * Currently server is enabled but client is disabled by default until there
 * is further compatibility testing */
#define DROPBEAR_SERVER_TCP_FAST_OPEN 0
#define DROPBEAR_CLIENT_TCP_FAST_OPEN 0
