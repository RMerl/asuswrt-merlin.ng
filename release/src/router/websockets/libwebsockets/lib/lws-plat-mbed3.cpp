#include "private-libwebsockets.h"
#include "core-util/CriticalSectionLock.h"

extern "C" void *mbed3_create_tcp_stream_socket(void)
{
	lws_conn_listener *srv = new lws_conn_listener;

	//lwsl_notice("%s: %p\r\n", __func__, (void *)srv);

	return (void *)srv;
}

/* this is called by compatible_close() */
extern "C" void mbed3_delete_tcp_stream_socket(void *sock)
{
	lws_conn *conn = (lws_conn *)sock;

	conn->ts->close();

	lwsl_notice("%s: wsi %p: conn %p\r\n", __func__, (void *)conn->wsi, sock);
	delete conn;
}

void lws_conn::serialized_writeable(struct lws *_wsi)
{
	struct lws *wsi = (struct lws *)_wsi;
	struct lws_pollfd pollfd;
	lws_conn *conn = (lws_conn *)wsi->sock;

	conn->awaiting_on_writeable = 0;

	pollfd.fd = wsi->sock;
	pollfd.events = POLLOUT;
	pollfd.revents = POLLOUT;

	lwsl_debug("%s: wsi %p\r\n", __func__, (void *)wsi);

	lws_service_fd(lws_get_context(wsi), &pollfd);
}

extern "C" void mbed3_tcp_stream_bind(void *sock, int port, struct lws *wsi)
{
	lws_conn_listener *srv = (lws_conn_listener *)sock;

	lwsl_debug("%s\r\n", __func__);
	/* associate us with the listening wsi */
	((lws_conn *)srv)->set_wsi(wsi);

	mbed::util::FunctionPointer1<void, uint16_t> fp(srv, &lws_conn_listener::start);
	minar::Scheduler::postCallback(fp.bind(port));
}

extern "C" void mbed3_tcp_stream_accept(void *sock, struct lws *wsi)
{
	lws_conn *conn = (lws_conn *)sock;

	lwsl_debug("%s\r\n", __func__);
	conn->set_wsi(wsi);
}

extern "C" LWS_VISIBLE int
lws_plat_change_pollfd(struct lws_context *context,
		      struct lws *wsi, struct lws_pollfd *pfd)
{
	lws_conn *conn = (lws_conn *)wsi->sock;

	(void)context;
	if (pfd->events & POLLOUT) {
		conn->awaiting_on_writeable = 1;
		if (conn->writeable) {
 			mbed::util::FunctionPointer1<void, struct lws *> book(conn, &lws_conn::serialized_writeable);
			minar::Scheduler::postCallback(book.bind(wsi));
			lwsl_debug("%s: wsi %p (booked callback)\r\n", __func__, (void *)wsi);
		} else {

			lwsl_debug("%s: wsi %p (set awaiting_on_writeable)\r\n", __func__, (void *)wsi);
		}
	} else
		conn->awaiting_on_writeable = 0;

	return 0;
}

extern "C" LWS_VISIBLE int
lws_ssl_capable_read_no_ssl(struct lws *wsi, unsigned char *buf, int len)
{
	socket_error_t err;
	size_t _len = len;

	lwsl_debug("%s\r\n", __func__);

	err = ((lws_conn *)wsi->sock)->ts->recv((char *)buf, &_len);
	if (err == SOCKET_ERROR_NONE) {
		lwsl_info("%s: got %d bytes\n", __func__, _len);
		return _len;
	}
#if LWS_POSIX
	if (LWS_ERRNO == LWS_EAGAIN ||
	    LWS_ERRNO == LWS_EWOULDBLOCK ||
	    LWS_ERRNO == LWS_EINTR)
#else
	if (err == SOCKET_ERROR_WOULD_BLOCK)
#endif
		return LWS_SSL_CAPABLE_MORE_SERVICE;

	lwsl_warn("error on reading from skt: %d\n", err);
	return LWS_SSL_CAPABLE_ERROR;
}

extern "C" LWS_VISIBLE int
lws_ssl_capable_write_no_ssl(struct lws *wsi, unsigned char *buf, int len)
{
	socket_error_t err;
	lws_conn *conn = (lws_conn *)wsi->sock;

	lwsl_debug("%s: wsi %p: write %d (from %p)\n", __func__, (void *)wsi, len, (void *)buf);

	lwsl_debug("%s: wsi %p: clear writeable\n", __func__, (void *)wsi);
	conn->writeable = 0;

	err = conn->ts->send((char *)buf, len);
	if (err == SOCKET_ERROR_NONE)
		return len;

#if LWS_POSIX
	if (LWS_ERRNO == LWS_EAGAIN ||
	    LWS_ERRNO == LWS_EWOULDBLOCK ||
	    LWS_ERRNO == LWS_EINTR) {
		if (LWS_ERRNO == LWS_EWOULDBLOCK)
			lws_set_blocking_send(wsi);
#else
	if (err == SOCKET_ERROR_WOULD_BLOCK)
		return LWS_SSL_CAPABLE_MORE_SERVICE;
#endif

	lwsl_warn("%s: wsi %p: ERROR %d writing len %d to skt\n", __func__, (void *)wsi, err, len);
	return LWS_SSL_CAPABLE_ERROR;
}

/*
 * Set the listening socket to listen.
 */

void lws_conn_listener::start(const uint16_t port)
{
	socket_error_t err = srv.open(SOCKET_AF_INET4);

	if (srv.error_check(err))
		return;
	err = srv.bind("0.0.0.0", port);
	if (srv.error_check(err))
		return;
	err = srv.start_listening(TCPListener::IncomingHandler_t(this,
					&lws_conn_listener::onIncoming));
	srv.error_check(err);
}

void lws_conn::onRX(Socket *s)
{
	struct lws_pollfd pollfd;

	(void)s;

	pollfd.fd = this;
	pollfd.events = POLLIN;
	pollfd.revents = POLLIN;

	lwsl_debug("%s: lws %p\n", __func__, wsi);

	lws_service_fd(lws_get_context(wsi), &pollfd);
}

/*
 * this gets called from the OS when the TCPListener gets a connection that
 * needs accept()-ing.  LWS needs to run the associated flow.
 */

void lws_conn_listener::onIncoming(TCPListener *tl, void *impl)
{
	mbed::util::CriticalSectionLock lock;
	lws_conn *conn;

	if (!impl) {
		onError(tl, SOCKET_ERROR_NULL_PTR);
		return;
	}

	conn = new(lws_conn);
	if (!conn) {
		lwsl_err("OOM\n");
		return;
	}
	conn->ts = srv.accept(impl);
	if (!conn->ts)
		return;

	conn->ts->setNagle(0);

	/*
	 * we use the listen socket wsi to get started, but a new wsi is
	 * created.  mbed3_tcp_stream_accept() is also called from
	 * here to bind the conn and new wsi together
	 */
	lws_server_socket_service(lws_get_context(wsi),
				  wsi, (struct pollfd *)conn);

	conn->ts->setOnError(TCPStream::ErrorHandler_t(conn, &lws_conn::onError));
	conn->ts->setOnDisconnect(TCPStream::DisconnectHandler_t(conn,
			    &lws_conn::onDisconnect));
	conn->ts->setOnSent(Socket::SentHandler_t(conn, &lws_conn::onSent));
	conn->ts->setOnReadable(TCPStream::ReadableHandler_t(conn, &lws_conn::onRX));

	conn->onRX((Socket *)conn->ts);

	lwsl_debug("%s: exit\n", __func__);
}

extern "C" LWS_VISIBLE struct lws *
wsi_from_fd(const struct lws_context *context, lws_sockfd_type fd)
{
	lws_conn *conn = (lws_conn *)fd;
	(void)context;

	return conn->wsi;
}

extern "C" LWS_VISIBLE void
lws_plat_insert_socket_into_fds(struct lws_context *context,
						       struct lws *wsi)
{
	(void)wsi;
	lws_libev_io(wsi, LWS_EV_START | LWS_EV_READ);
	context->fds[context->fds_count++].revents = 0;
}

extern "C" LWS_VISIBLE void
lws_plat_delete_socket_from_fds(struct lws_context *context,
						struct lws *wsi, int m)
{
	(void)context;
	(void)wsi;
	(void)m;
}

void lws_conn_listener::onDisconnect(TCPStream *s)
{
	lwsl_info("%s\r\n", __func__);
	(void)s;
	//if (s)
	//delete this;
}

extern "C" LWS_VISIBLE int
lws_plat_service(struct lws_context *context, int timeout_ms)
{
	(void)context;
	(void)timeout_ms;

	return 0;
}

void lws_conn::onSent(Socket *s, uint16_t len)
{
	struct lws_pollfd pollfd;

	(void)s;
	(void)len;

	if (!awaiting_on_writeable) {
		lwsl_debug("%s: wsi %p (setting writable=1)\r\n",
			   __func__, (void *)wsi);
		writeable = 1;
		return;
	}

	writeable = 1;

	pollfd.fd = wsi->sock;
	pollfd.events = POLLOUT;
	pollfd.revents = POLLOUT;

	lwsl_debug("%s: wsi %p (servicing now)\r\n", __func__, (void *)wsi);

	lws_service_fd(lws_get_context(wsi), &pollfd);
}

void lws_conn_listener::onError(Socket *s, socket_error_t err)
{
	(void) s;
	lwsl_notice("Socket Error: %s (%d)\r\n", socket_strerror(err), err);
	if (ts)
		ts->close();
}

void lws_conn::onDisconnect(TCPStream *s)
{
	lwsl_notice("%s:\r\n", __func__);
	(void)s;
	lws_close_free_wsi(wsi, LWS_CLOSE_STATUS_NOSTATUS);
}


void lws_conn::onError(Socket *s, socket_error_t err)
{
	(void) s;
	lwsl_notice("Socket Error: %s (%d)\r\n", socket_strerror(err), err);
	s->close();
}
