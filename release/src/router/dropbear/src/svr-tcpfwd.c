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

#if DROPBEAR_SVR_REMOTETCPFWD

static const struct ChanType svr_chan_tcpremote = {
    "forwarded-tcpip",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static int matchtcp(const void* typedata1, const void* typedata2) {

    const struct FwdListener *info1 = (struct FwdListener*)typedata1;
    const struct FwdListener *info2 = (struct FwdListener*)typedata2;

    return (info1->listenport == info2->listenport)
            && (strcmp(info1->request_listenaddr, info2->request_listenaddr) == 0);
}

int svr_cancelremotetcp(void) {

    int ret = DROPBEAR_FAILURE;
    char * request_addr = NULL;
    unsigned int addrlen;
    unsigned int port;
    struct Listener * listener = NULL;
    struct FwdListener tcpinfo;

    TRACE(("enter cancelremotetcp"))

    request_addr = buf_getstring(ses.payload, &addrlen);
    if (addrlen > MAX_HOST_LEN) {
        TRACE(("addr len too long: %d", addrlen))
        goto out;
    }

    port = buf_getint(ses.payload);

    memset(&tcpinfo, 0x0, sizeof(tcpinfo));
    tcpinfo.request_listenaddr = request_addr;
    tcpinfo.listenport = port;
    listener = get_listener(LISTENER_TYPE_TCPFORWARDED, &tcpinfo, matchtcp);
    if (listener) {
        remove_listener(listener);
        ret = DROPBEAR_SUCCESS;
    }

out:
    m_free(request_addr);
    TRACE(("leave cancelremotetcp"))
    return ret;
}

/* Will send its own reply if needed. */
int svr_remotetcpreq(int wantreply) {

    int ret = DROPBEAR_FAILURE;
    char * request_addr = NULL;
    unsigned int addrlen;
    struct FwdListener *tcpinfo = NULL;
    unsigned int port;
    struct Listener *listener = NULL;

    TRACE(("enter remotetcpreq"))

    request_addr = buf_getstring(ses.payload, &addrlen);
    if (addrlen > MAX_HOST_LEN) {
        TRACE(("addr len too long: %d", addrlen))
        goto out;
    }

    port = buf_getint(ses.payload);

    if (port != 0) {
        if (port < 1 || port > 65535) {
            TRACE(("invalid port: %d", port))
            goto out;
        }

        if (!ses.allowprivport && port < IPPORT_RESERVED) {
            TRACE(("can't assign port < 1024 for non-root"))
            goto out;
        }
    }

    if (!svr_pubkey_allows_remote_tcpfwd(request_addr, port)) {
        TRACE(("remote tcp forwarding listen address not permitted"));
        goto out;
    }

    tcpinfo = (struct FwdListener*)m_malloc(sizeof(struct FwdListener));
    tcpinfo->sendaddr = NULL;
    tcpinfo->sendport = 0;
    tcpinfo->listenport = port;
    tcpinfo->chantype = &svr_chan_tcpremote;
    tcpinfo->fwd_type = forwarded;
    tcpinfo->interface = svr_opts.interface;

    tcpinfo->request_listenaddr = request_addr;
    if (!opts.listen_fwd_all || (strcmp(request_addr, "localhost") == 0) ) {
        /* NULL means "localhost only" */
        tcpinfo->listenaddr = NULL;
    }
    else
    {
        tcpinfo->listenaddr = m_strdup(request_addr);
    }

    ret = listen_tcpfwd(tcpinfo, &listener);
    if (DROPBEAR_SUCCESS == ret) {
        tcpinfo->listenport = get_sock_port(listener->socks[0]);
    }

out:
    /* Send a reply if needed */
    if (wantreply) {
        CHECKCLEARTOWRITE();
        if (ret == DROPBEAR_SUCCESS) {
            buf_putbyte(ses.writepayload, SSH_MSG_REQUEST_SUCCESS);
            if (port == 0) {
                /* port is only included if allocated */
                buf_putint(ses.writepayload, tcpinfo->listenport);
            }
        } else {
            buf_putbyte(ses.writepayload, SSH_MSG_REQUEST_FAILURE);
        }
        encrypt_packet();
    }

    if (ret == DROPBEAR_FAILURE) {
        /* we only free it if a listener wasn't created, since the listener
         * has to remember it if it's to be cancelled */
        m_free(request_addr);
        m_free(tcpinfo);
    }

    TRACE(("leave remotetcpreq"))

    return ret;
}


#endif /* DROPBEAR_SVR_REMOTETCPFWD */


#if DROPBEAR_SVR_LOCALTCPFWD

/* Called upon creating a new direct tcp channel (ie we connect out to an
 * address */
static int newtcpdirect(struct Channel * channel) {

    char* desthost = NULL;
    unsigned int destport;
    char* orighost = NULL;
    unsigned int origport;
    char portstring[NI_MAXSERV];
    unsigned int len;
    int err = SSH_OPEN_ADMINISTRATIVELY_PROHIBITED;

    TRACE(("newtcpdirect channel %d", channel->index))

    if (svr_opts.nolocaltcp || !svr_pubkey_allows_tcpfwd()) {
        TRACE(("leave newtcpdirect: local tcp forwarding disabled"))
        goto out;
    }

    desthost = buf_getstring(ses.payload, &len);
    if (len > MAX_HOST_LEN) {
        TRACE(("leave newtcpdirect: desthost too long"))
        goto out;
    }

    destport = buf_getint(ses.payload);
    
    orighost = buf_getstring(ses.payload, &len);
    if (len > MAX_HOST_LEN) {
        TRACE(("leave newtcpdirect: orighost too long"))
        goto out;
    }

    origport = buf_getint(ses.payload);

    /* best be sure */
    if (origport > 65535 || destport > 65535) {
        TRACE(("leave newtcpdirect: port > 65535"))
        goto out;
    }

    if (!svr_pubkey_allows_local_tcpfwd(desthost, destport)) {
        TRACE(("leave newtcpdirect: local tcp forwarding not permitted to requested destination"));
        goto out;
    }

    snprintf(portstring, sizeof(portstring), "%u", destport);
    channel->conn_pending = connect_remote(desthost, portstring, channel_connect_done,
        channel, NULL, NULL, DROPBEAR_PRIO_NORMAL);

    err = SSH_OPEN_IN_PROGRESS;

out:
    m_free(desthost);
    m_free(orighost);
    TRACE(("leave newtcpdirect: err %d", err))
    return err;
}

const struct ChanType svr_chan_tcpdirect = {
    "direct-tcpip",
    newtcpdirect, /* init */
    NULL, /* checkclose */
    NULL, /* reqhandler */
    NULL, /* closehandler */
    NULL /* cleanup */
};

#endif /* DROPBEAR_SVR_LOCALTCPFWD */

