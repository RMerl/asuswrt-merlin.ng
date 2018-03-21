#define INETD_MODE 0

#define DROPBEAR_X11FWD 0

#define DROPBEAR_SHA1_96_HMAC 0

#define MAX_UNAUTH_PER_IP 3
#define MAX_UNAUTH_CLIENTS 9
#define MAX_AUTH_TRIES 3

#define DROPBEAR_SFTPSERVER 1
#define SFTPSERVER_PATH "/opt/libexec/sftp-server"

#define DEFAULT_PATH "/bin:/sbin:/usr/bin:/usr/sbin:/opt/bin:/opt/sbin:/opt/usr/bin:/opt/usr/sbin"

#define DO_MOTD 1

/* Overrides for sysoptions.h */
#ifdef DROPBEAR_SERVER_TCP_FAST_OPEN
#undef DROPBEAR_SERVER_TCP_FAST_OPEN
#endif
