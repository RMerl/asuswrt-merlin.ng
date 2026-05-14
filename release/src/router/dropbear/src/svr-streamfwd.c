#include "includes.h"
#include "ssh.h"
#include "forward.h"
#include "dbutil.h"
#include "session.h"
#include "buffer.h"
#include "packet.h"
#include "listener.h"
#include "runopts.h"
#include "auth.h"
#include "netio.h"


#if DROPBEAR_SVR_REMOTESTREAMFWD
static const struct ChanType svr_chan_streamlocalremote = {
    "forwarded-streamlocal@openssh.com",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static int matchstreamlocal(const void* typedata1, const void* typedata2) {

    const struct FwdListener *info1 = (const struct FwdListener*)typedata1;
    const struct FwdListener *info2 = (const struct FwdListener*)typedata2;

    if (info1->socket_path == NULL || info2->socket_path == NULL) {
        return 0;
    }

    return (info1->chantype == info2->chantype)
            && (strcmp(info1->socket_path, info2->socket_path) == 0);
}

int svr_cancelremotestreamlocal() {

    int ret = DROPBEAR_FAILURE;
    char * socket_path = NULL;
    unsigned int pathlen;
    struct Listener * listener = NULL;
    struct FwdListener tcpinfo;

    TRACE(("enter cancelremotestreamlocal"))

    socket_path = buf_getstring(ses.payload, &pathlen);
    if (pathlen > MAX_HOST_LEN) {
        TRACE(("path len too long: %d", pathlen))
        goto out;
    }
    if (strlen(socket_path) != pathlen) {
        TRACE(("path has nul byte"));
        goto out;
    }

    tcpinfo.socket_path = socket_path;
    tcpinfo.chantype = &svr_chan_streamlocalremote;
    listener = get_listener(CHANNEL_ID_STREAMLOCALFORWARDED, &tcpinfo, matchstreamlocal);
    if (listener) {
        remove_listener( listener );
        ret = DROPBEAR_SUCCESS;
    }

out:
    m_free(socket_path);
    TRACE(("leave cancelremotestreamlocal"))
    return ret;
}

static void unlink_streamsocket(const char* path) {
    if (unlink(path) < 0 && errno != ENOENT) {
        /* Not fatal */
        DEBUG1(("Failed removing unix socket %s: %s",
            path, strerror(errno)));
    }
}

static void cleanup_streamlocal(const struct Listener *listener) {

    struct FwdListener *tcpinfo = (struct FwdListener*)(listener->typedata);

    if (tcpinfo && tcpinfo->socket_path) {
        unlink_streamsocket(tcpinfo->socket_path);
        m_free(tcpinfo->socket_path);
    }
    m_free(tcpinfo->request_listenaddr);
    m_free(tcpinfo);
}

static void streamlocal_acceptor(const struct Listener *listener, int sock) {

    int fd;
    struct FwdListener *tcpinfo = (struct FwdListener*)(listener->typedata);

    fd = accept(sock, NULL, NULL);
    if (fd < 0) {
        return;
    }

    if (send_msg_channel_open_init(fd, tcpinfo->chantype) == DROPBEAR_SUCCESS) {
        /* "forwarded-streamlocal@openssh.com" */
        /* socket path that was connected to */
        buf_putstring(ses.writepayload, tcpinfo->request_listenaddr,
                strlen(tcpinfo->request_listenaddr));
        /* reserved field */
        buf_putstring(ses.writepayload, "", 0);

        encrypt_packet();

    } else {
        /* XXX debug? */
        close(fd);
    }
}

int listen_streamlocal(struct FwdListener* tcpinfo, struct Listener **ret_listener) {

    int sock, rc, saved_errno;
    struct Listener *listener = NULL;
    struct sockaddr_un addr;
    mode_t old_umask;

    TRACE(("enter listen_streamlocal"))

    if (tcpinfo->socket_path == NULL) {
        TRACE(("leave listen_streamlocal: no socket path"))
        return DROPBEAR_FAILURE;
    }

    if (strlen(tcpinfo->socket_path) >= sizeof(addr.sun_path)) {
        dropbear_log(LOG_INFO, "Streamlocal forward failed: socket path too long");
        TRACE(("leave listen_streamlocal: path too long"))
        return DROPBEAR_FAILURE;
    }

#if DROPBEAR_FUZZ
    if (fuzz.fuzzing) {
        // fuzzing streamlocal is unimplemented
        return DROPBEAR_FAILURE;
    }
#endif

    sock = socket(PF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        dropbear_log(LOG_INFO, "Streamlocal forward failed: socket() failed");
        TRACE(("leave listen_streamlocal: socket() failed"))
        return DROPBEAR_FAILURE;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strlcpy(addr.sun_path, tcpinfo->socket_path, sizeof(addr.sun_path));

    /* Unlink existing socket if it exists */
    unlink_streamsocket(tcpinfo->socket_path);

    /* Set umask to allow proper permissions on the socket */
    old_umask = umask(0177);
    rc = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    saved_errno = errno;
    umask(old_umask);

    if (rc < 0) {
        dropbear_log(LOG_INFO, "Streamlocal forward failed: bind() failed: %s", strerror(saved_errno));
        m_close(sock);
        TRACE(("leave listen_streamlocal: bind() failed"))
        return DROPBEAR_FAILURE;
    }


    if (listen(sock, DROPBEAR_LISTEN_BACKLOG) < 0) {
        dropbear_log(LOG_INFO, "Streamlocal forward failed: listen() failed: %s", strerror(errno));
        unlink_streamsocket(tcpinfo->socket_path);
        m_close(sock);
        TRACE(("leave listen_streamlocal: listen() failed"))
        return DROPBEAR_FAILURE;
    }

    setnonblocking(sock);

    listener = new_listener(&sock, 1, LISTENER_TYPE_STREAMFORWARDED, tcpinfo,
            streamlocal_acceptor, cleanup_streamlocal);

    if (listener == NULL) {
        unlink_streamsocket(tcpinfo->socket_path);
        m_close(sock);
        TRACE(("leave listen_streamlocal: listener failed"))
        return DROPBEAR_FAILURE;
    }

    if (ret_listener) {
        *ret_listener = listener;
    }

    TRACE(("leave listen_streamlocal: success"))
    return DROPBEAR_SUCCESS;
}

int svr_remotestreamlocalreq() {

    int ret = DROPBEAR_FAILURE;
    char * request_path = NULL;
    unsigned int pathlen;
    struct FwdListener *tcpinfo = NULL;
    struct Listener *listener = NULL;

    TRACE(("enter remotestreamlocalreq"))

    if (svr_opts.forced_command || svr_pubkey_has_forced_command()) {
        /* Creating a unix socket in the right place could probably subvert
         * a forcedcommand, so don't allow that.
         * This could be relaxed if an authorized_keys "permitlisten"
         * equivalent were added for streamlocal */
        TRACE(("leave newstreamlocal: no unix forwarding for forced command"))
        goto out;
    }

    if (svr_opts.noremotefwd || !svr_pubkey_allows_tcpfwd()) {
        TRACE(("leave remotestreamlocalreq: remote forwarding disabled"))
        goto out;
    }

    request_path = buf_getstring(ses.payload, &pathlen);
    if (pathlen > MAX_HOST_LEN) {
        TRACE(("path len too long: %d", pathlen))
        goto out;
    }
    if (strlen(request_path) != pathlen) {
        TRACE(("path has nul byte"));
        goto out;
    }

    tcpinfo = (struct FwdListener*)m_malloc(sizeof(struct FwdListener));
    memset(tcpinfo, 0, sizeof(struct FwdListener));
    tcpinfo->sendaddr = NULL;
    tcpinfo->sendport = 0;
    tcpinfo->listenaddr = NULL;
    tcpinfo->listenport = 0;
    tcpinfo->chantype = &svr_chan_streamlocalremote;
    tcpinfo->fwd_type = forwarded;
    tcpinfo->interface = NULL;
    tcpinfo->socket_path = m_strdup(request_path);
    tcpinfo->request_listenaddr = request_path;

    ret = listen_streamlocal(tcpinfo, &listener);

out:
    if (ret == DROPBEAR_FAILURE) {
        /* we only free it if a listener wasn't created, since the listener
         * has to remember it if it's to be cancelled */
        m_free(request_path);
        m_free(tcpinfo->socket_path);
        m_free(tcpinfo);
    }

    TRACE(("leave remotestreamlocalreq"))
    return ret;
}
#endif /* DROPBEAR_SVR_REMOTESTREAMFWD */

#if DROPBEAR_SVR_LOCALSTREAMFWD

/* Called upon creating a new stream local channel (ie we connect out to an
 * address */
static int newstreamlocal(struct Channel * channel) {

    /*
    https://cvsweb.openbsd.org/cgi-bin/cvsweb/src/usr.bin/ssh/PROTOCOL#rev1.30

    byte        SSH_MSG_CHANNEL_OPEN
    string      "direct-streamlocal@openssh.com"
    uint32      sender channel
    uint32      initial window size
    uint32      maximum packet size
    string      socket path
    string      reserved
    uint32      reserved
    */

    char* destsocket = NULL;
    unsigned int len;
    int err = SSH_OPEN_ADMINISTRATIVELY_PROHIBITED;

    TRACE(("streamlocal channel %d", channel->index))

    if (svr_opts.forced_command || svr_pubkey_has_forced_command()) {
        TRACE(("leave newstreamlocal: no unix forwarding for forced command"))
        goto out;
    }

    if (svr_opts.nolocaltcp || !svr_pubkey_allows_tcpfwd()) {
        TRACE(("leave newstreamlocal: local unix forwarding disabled"))
        goto out;
    }

    destsocket = buf_getstring(ses.payload, &len);
    if (len > MAX_HOST_LEN) {
        TRACE(("leave streamlocal: destsocket too long"))
        goto out;
    }

    channel->conn_pending = connect_streamlocal(destsocket, channel_connect_done,
        channel, DROPBEAR_PRIO_NORMAL);

    err = SSH_OPEN_IN_PROGRESS;

out:
    m_free(destsocket);
    TRACE(("leave streamlocal: err %d", err))
    return err;
}

const struct ChanType svr_chan_streamlocal = {
    "direct-streamlocal@openssh.com",
    newstreamlocal, /* init */
    NULL, /* checkclose */
    NULL, /* reqhandler */
    NULL, /* closehandler */
    NULL /* cleanup */
};

#endif /* DROPBEAR_SVR_LOCALSTREAMFWD */
