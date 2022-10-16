/*
 * Sample showing how to use libssh2 to request agent forwarding
 * on the remote host. The command executed will run with agent forwarded
 * so you should be able to do things like clone out protected git
 * repos and such.
 *
 * The example uses agent authentication to ensure an agent to forward
 * is running.
 *
 * Run it like this:
 *
 * $ ./ssh2_agent_forwarding 127.0.0.1 user "uptime"
 *
 */

#include "libssh2_config.h"
#include <libssh2.h>

#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif
# ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

static int waitsocket(int socket_fd, LIBSSH2_SESSION *session)
{
    struct timeval timeout;
    int rc;
    fd_set fd;
    fd_set *writefd = NULL;
    fd_set *readfd = NULL;
    int dir;

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    FD_ZERO(&fd);

    FD_SET(socket_fd, &fd);

    /* now make sure we wait in the correct direction */
    dir = libssh2_session_block_directions(session);

    if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
        readfd = &fd;

    if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
        writefd = &fd;

    rc = select(socket_fd + 1, readfd, writefd, NULL, &timeout);

    return rc;
}

int main(int argc, char *argv[])
{
    const char *hostname = "127.0.0.1";
    const char *commandline = "uptime";
    const char *username    = NULL;
    unsigned long hostaddr;
    int sock;
    struct sockaddr_in sin;
    LIBSSH2_SESSION *session;
    LIBSSH2_CHANNEL *channel;
    LIBSSH2_AGENT *agent = NULL;
    struct libssh2_agent_publickey *identity, *prev_identity = NULL;
    int rc;
    int exitcode;
    char *exitsignal = (char *)"none";
    int bytecount = 0;

#ifdef WIN32
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 0), &wsadata);
#endif
    if(argc < 2) {
        fprintf(stderr, "At least IP and username arguments are required.\n");
        return 1;
    }
    /* must be ip address only */
    hostname = argv[1];
    username = argv[2];

    if(argc > 3) {
        commandline = argv[3];
    }

    rc = libssh2_init(0);
    if(rc != 0) {
        fprintf(stderr, "libssh2 initialization failed (%d)\n", rc);
        return 1;
    }

    hostaddr = inet_addr(hostname);

    /* Ultra basic "connect to port 22 on localhost"
     * Your code is responsible for creating the socket establishing the
     * connection
     */
    sock = socket(AF_INET, SOCK_STREAM, 0);

    sin.sin_family = AF_INET;
    sin.sin_port = htons(22);
    sin.sin_addr.s_addr = hostaddr;
    if(connect(sock, (struct sockaddr*)(&sin),
                sizeof(struct sockaddr_in)) != 0) {
        fprintf(stderr, "failed to connect!\n");
        return -1;
    }

    /* Create a session instance */
    session = libssh2_session_init();
    if(!session)
        return -1;

    if(libssh2_session_handshake(session, sock) != 0) {
        fprintf(stderr, "Failure establishing SSH session: %d\n", rc);
        return -1;
    }

    /* Connect to the ssh-agent */
    agent = libssh2_agent_init(session);
    if(!agent) {
        fprintf(stderr, "Failure initializing ssh-agent support\n");
        rc = 1;
        goto shutdown;
    }
    if(libssh2_agent_connect(agent)) {
        fprintf(stderr, "Failure connecting to ssh-agent\n");
        rc = 1;
        goto shutdown;
    }
    if(libssh2_agent_list_identities(agent)) {
        fprintf(stderr, "Failure requesting identities to ssh-agent\n");
        rc = 1;
        goto shutdown;
    }
    while(1) {
        rc = libssh2_agent_get_identity(agent, &identity, prev_identity);
        if(rc == 1)
            break;
        if(rc < 0) {
            fprintf(stderr,
                    "Failure obtaining identity from ssh-agent support\n");
            rc = 1;
            goto shutdown;
        }
        if(libssh2_agent_userauth(agent, username, identity)) {
            fprintf(stderr, "\tAuthentication with username %s and "
                   "public key %s failed!\n",
                   username, identity->comment);
        }
        else {
            fprintf(stderr, "\tAuthentication with username %s and "
                   "public key %s succeeded!\n",
                   username, identity->comment);
            break;
        }
        prev_identity = identity;
    }
    if(rc) {
        fprintf(stderr, "Couldn't continue authentication\n");
        goto shutdown;
    }

#if 0
    libssh2_trace(session, ~0);
#endif

    /* Set session to non-blocking */
    libssh2_session_set_blocking(session, 0);

    /* Exec non-blocking on the remove host */
    while((channel = libssh2_channel_open_session(session)) == NULL &&
          libssh2_session_last_error(session, NULL, NULL, 0) ==
          LIBSSH2_ERROR_EAGAIN) {
        waitsocket(sock, session);
    }
    if(channel == NULL) {
        fprintf(stderr, "Error\n");
        exit(1);
    }
    while((rc = libssh2_channel_request_auth_agent(channel)) ==
          LIBSSH2_ERROR_EAGAIN) {
        waitsocket(sock, session);
    }
    if(rc != 0) {
        fprintf(stderr, "Error, couldn't request auth agent, error code %d.\n",
                rc);
        exit(1);
    }
    else {
        fprintf(stdout, "\tAgent forwarding request succeeded!\n");
    }
    while((rc = libssh2_channel_exec(channel, commandline)) ==
          LIBSSH2_ERROR_EAGAIN) {
        waitsocket(sock, session);
    }
    if(rc != 0) {
        fprintf(stderr, "Error\n");
        exit(1);
    }
    for(;;) {
        /* loop until we block */
        int rc;
        do {
            char buffer[0x4000];
            rc = libssh2_channel_read(channel, buffer, sizeof(buffer) );
            if(rc > 0) {
                int i;
                bytecount += rc;
                fprintf(stderr, "We read:\n");
                for(i = 0; i < rc; ++i)
                    fputc(buffer[i], stderr);
                fprintf(stderr, "\n");
            }
            else {
                if(rc != LIBSSH2_ERROR_EAGAIN)
                    /* no need to output this for the EAGAIN case */
                    fprintf(stderr, "libssh2_channel_read returned %d\n", rc);
            }
        }
        while(rc > 0);

        /* this is due to blocking that would occur otherwise so we loop on
           this condition */
        if(rc == LIBSSH2_ERROR_EAGAIN) {
            waitsocket(sock, session);
        }
        else
            break;
    }
    exitcode = 127;
    while((rc = libssh2_channel_close(channel)) == LIBSSH2_ERROR_EAGAIN) {
        waitsocket(sock, session);
    }
    if(rc == 0) {
        exitcode = libssh2_channel_get_exit_status(channel);
        libssh2_channel_get_exit_signal(channel, &exitsignal,
                                        NULL, NULL, NULL, NULL, NULL);
    }

    if(exitsignal) {
        printf("\nGot signal: %s\n", exitsignal);
    }
    else {
        printf("\nEXIT: %d bytecount: %d\n", exitcode, bytecount);
    }

    libssh2_channel_free(channel);
    channel = NULL;

shutdown:

    libssh2_session_disconnect(session,
                               "Normal Shutdown, Thank you for playing");
    libssh2_session_free(session);

#ifdef WIN32
    closesocket(sock);
#else
    close(sock);
#endif
    fprintf(stderr, "all done\n");

    libssh2_exit();

    return 0;
}
