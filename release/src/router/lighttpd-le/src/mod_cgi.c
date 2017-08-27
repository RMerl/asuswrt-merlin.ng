#include "first.h"

#include "server.h"
#include "stat_cache.h"
#include "keyvalue.h"
#include "log.h"
#include "connections.h"
#include "joblist.h"
#include "response.h"
#include "http_chunk.h"

#include "plugin.h"

#include <sys/types.h>
#include "sys-mmap.h"

#ifdef __WIN32
# include <winsock2.h>
#else
# include <sys/socket.h>
# include <sys/wait.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#endif

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fdevent.h>
#include <signal.h>
#include <ctype.h>
#include <assert.h>

#include <stdio.h>
#include <fcntl.h>

static int pipe_cloexec(int pipefd[2]) {
  #ifdef HAVE_PIPE2
    if (0 == pipe2(pipefd, O_CLOEXEC)) return 0;
  #endif
    return 0 == pipe(pipefd)
       #ifdef FD_CLOEXEC
        && 0 == fcntl(pipefd[0], F_SETFD, FD_CLOEXEC)
        && 0 == fcntl(pipefd[1], F_SETFD, FD_CLOEXEC)
       #endif
      ?  0
      : -1;
}

enum {EOL_UNSET, EOL_N, EOL_RN};

typedef struct {
	char **ptr;

	size_t size;
	size_t used;
} char_array;

typedef struct {
	pid_t *ptr;
	size_t used;
	size_t size;
} buffer_pid_t;

typedef struct {
	array *cgi;
	unsigned short execute_x_only;
	unsigned short xsendfile_allow;
	array *xsendfile_docroot;
} plugin_config;

typedef struct {
	PLUGIN_DATA;
	buffer_pid_t cgi_pid;

	buffer *parse_response;

	plugin_config **config_storage;

	plugin_config conf;
} plugin_data;

typedef struct {
	pid_t pid;
	int fd;
	int fdtocgi;
	int fde_ndx; /* index into the fd-event buffer */
	int fde_ndx_tocgi; /* index into the fd-event buffer */

	connection *remote_conn;  /* dumb pointer */
	plugin_data *plugin_data; /* dumb pointer */

	buffer *response;
	buffer *response_header;
	buffer *cgi_handler;      /* dumb pointer */
	plugin_config conf;
} handler_ctx;

static handler_ctx * cgi_handler_ctx_init(void) {
	handler_ctx *hctx = calloc(1, sizeof(*hctx));

	force_assert(hctx);

	hctx->response = buffer_init();
	hctx->response_header = buffer_init();
	hctx->fd = -1;
	hctx->fdtocgi = -1;

	return hctx;
}

static void cgi_handler_ctx_free(handler_ctx *hctx) {
	buffer_free(hctx->response);
	buffer_free(hctx->response_header);

	free(hctx);
}

enum {FDEVENT_HANDLED_UNSET, FDEVENT_HANDLED_FINISHED, FDEVENT_HANDLED_NOT_FINISHED, FDEVENT_HANDLED_COMEBACK, FDEVENT_HANDLED_ERROR};

INIT_FUNC(mod_cgi_init) {
	plugin_data *p;

	p = calloc(1, sizeof(*p));

	force_assert(p);

	p->parse_response = buffer_init();

	return p;
}


FREE_FUNC(mod_cgi_free) {
	plugin_data *p = p_d;
	buffer_pid_t *r = &(p->cgi_pid);

	UNUSED(srv);

	if (p->config_storage) {
		size_t i;
		for (i = 0; i < srv->config_context->used; i++) {
			plugin_config *s = p->config_storage[i];

			if (NULL == s) continue;

			array_free(s->cgi);
			array_free(s->xsendfile_docroot);

			free(s);
		}
		free(p->config_storage);
	}


	if (r->ptr) free(r->ptr);

	buffer_free(p->parse_response);

	free(p);

	return HANDLER_GO_ON;
}

SETDEFAULTS_FUNC(mod_fastcgi_set_defaults) {
	plugin_data *p = p_d;
	size_t i = 0;

	config_values_t cv[] = {
		{ "cgi.assign",                  NULL, T_CONFIG_ARRAY, T_CONFIG_SCOPE_CONNECTION },       /* 0 */
		{ "cgi.execute-x-only",          NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION },     /* 1 */
		{ "cgi.x-sendfile",              NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION },     /* 2 */
		{ "cgi.x-sendfile-docroot",      NULL, T_CONFIG_ARRAY,   T_CONFIG_SCOPE_CONNECTION },     /* 3 */
		{ NULL,                          NULL, T_CONFIG_UNSET, T_CONFIG_SCOPE_UNSET}
	};

	if (!p) return HANDLER_ERROR;

	p->config_storage = calloc(1, srv->config_context->used * sizeof(plugin_config *));
	force_assert(p->config_storage);

	for (i = 0; i < srv->config_context->used; i++) {
		data_config const* config = (data_config const*)srv->config_context->data[i];
		plugin_config *s;

		s = calloc(1, sizeof(plugin_config));
		force_assert(s);

		s->cgi    = array_init();
		s->execute_x_only = 0;
		s->xsendfile_allow= 0;
		s->xsendfile_docroot = array_init();

		cv[0].destination = s->cgi;
		cv[1].destination = &(s->execute_x_only);
		cv[2].destination = &(s->xsendfile_allow);
		cv[3].destination = s->xsendfile_docroot;

		p->config_storage[i] = s;

		if (0 != config_insert_values_global(srv, config->value, cv, i == 0 ? T_CONFIG_SCOPE_SERVER : T_CONFIG_SCOPE_CONNECTION)) {
			return HANDLER_ERROR;
		}

		if (s->xsendfile_docroot->used) {
			size_t j;
			for (j = 0; j < s->xsendfile_docroot->used; ++j) {
				data_string *ds = (data_string *)s->xsendfile_docroot->data[j];
				if (ds->type != TYPE_STRING) {
					log_error_write(srv, __FILE__, __LINE__, "s",
						"unexpected type for key cgi.x-sendfile-docroot; expected: cgi.x-sendfile-docroot = ( \"/allowed/path\", ... )");
					return HANDLER_ERROR;
				}
				if (ds->value->ptr[0] != '/') {
					log_error_write(srv, __FILE__, __LINE__, "SBs",
						"cgi.x-sendfile-docroot paths must begin with '/'; invalid: \"", ds->value, "\"");
					return HANDLER_ERROR;
				}
				buffer_path_simplify(ds->value, ds->value);
				buffer_append_slash(ds->value);
			}
		}
	}

	return HANDLER_GO_ON;
}


static int cgi_pid_add(server *srv, plugin_data *p, pid_t pid) {
	int m = -1;
	size_t i;
	buffer_pid_t *r = &(p->cgi_pid);

	UNUSED(srv);

	for (i = 0; i < r->used; i++) {
		if (r->ptr[i] > m) m = r->ptr[i];
	}

	if (r->size == 0) {
		r->size = 16;
		r->ptr = malloc(sizeof(*r->ptr) * r->size);
		force_assert(r->ptr);
	} else if (r->used == r->size) {
		r->size += 16;
		r->ptr = realloc(r->ptr, sizeof(*r->ptr) * r->size);
		force_assert(r->ptr);
	}

	r->ptr[r->used++] = pid;

	return m;
}

static int cgi_pid_del(server *srv, plugin_data *p, pid_t pid) {
	size_t i;
	buffer_pid_t *r = &(p->cgi_pid);

	UNUSED(srv);

	for (i = 0; i < r->used; i++) {
		if (r->ptr[i] == pid) break;
	}

	if (i != r->used) {
		/* found */

		if (i != r->used - 1) {
			r->ptr[i] = r->ptr[r->used - 1];
		}
		r->used--;
	}

	return 0;
}

static int cgi_response_parse(server *srv, connection *con, plugin_data *p, buffer *in) {
	char *ns;
	const char *s;
	int line = 0;

	UNUSED(srv);

	buffer_copy_buffer(p->parse_response, in);

	for (s = p->parse_response->ptr;
	     NULL != (ns = strchr(s, '\n'));
	     s = ns + 1, line++) {
		const char *key, *value;
		int key_len;
		data_string *ds;

		/* strip the \n */
		ns[0] = '\0';

		if (ns > s && ns[-1] == '\r') ns[-1] = '\0';

		if (line == 0 &&
		    0 == strncmp(s, "HTTP/1.", 7)) {
			/* non-parsed header ... we parse them anyway */

			if ((s[7] == '1' ||
			     s[7] == '0') &&
			    s[8] == ' ') {
				int status;
				/* after the space should be a status code for us */

				status = strtol(s+9, NULL, 10);

				if (status >= 100 &&
				    status < 1000) {
					/* we expected 3 digits and didn't got them */
					con->parsed_response |= HTTP_STATUS;
					con->http_status = status;
				}
			}
		} else {
			/* parse the headers */
			key = s;
			if (NULL == (value = strchr(s, ':'))) {
				/* we expect: "<key>: <value>\r\n" */
				continue;
			}

			key_len = value - key;
			value += 1;

			/* skip LWS */
			while (*value == ' ' || *value == '\t') value++;

			if (NULL == (ds = (data_string *)array_get_unused_element(con->response.headers, TYPE_STRING))) {
				ds = data_response_init();
			}
			buffer_copy_string_len(ds->key, key, key_len);
			buffer_copy_string(ds->value, value);

			array_insert_unique(con->response.headers, (data_unset *)ds);

			switch(key_len) {
			case 4:
				if (0 == strncasecmp(key, "Date", key_len)) {
					con->parsed_response |= HTTP_DATE;
				}
				break;
			case 6:
				if (0 == strncasecmp(key, "Status", key_len)) {
					int status = strtol(value, NULL, 10);
					if (status >= 100 && status < 1000) {
						con->http_status = status;
						con->parsed_response |= HTTP_STATUS;
					} else {
						con->http_status = 502;
					}
				}
				break;
			case 8:
				if (0 == strncasecmp(key, "Location", key_len)) {
					con->parsed_response |= HTTP_LOCATION;
				}
				break;
			case 10:
				if (0 == strncasecmp(key, "Connection", key_len)) {
					con->response.keep_alive = (0 == strcasecmp(value, "Keep-Alive")) ? 1 : 0;
					con->parsed_response |= HTTP_CONNECTION;
				}
				break;
			case 14:
				if (0 == strncasecmp(key, "Content-Length", key_len)) {
					con->response.content_length = strtoul(value, NULL, 10);
					con->parsed_response |= HTTP_CONTENT_LENGTH;
				}
				break;
			default:
				break;
			}
		}
	}

	/* CGI/1.1 rev 03 - 7.2.1.2 */
	if ((con->parsed_response & HTTP_LOCATION) &&
	    !(con->parsed_response & HTTP_STATUS)) {
		con->http_status = 302;
	}

	return 0;
}


static int cgi_demux_response(server *srv, handler_ctx *hctx) {
	plugin_data *p    = hctx->plugin_data;
	connection  *con  = hctx->remote_conn;

	while(1) {
		int n;
		int toread;

#if defined(__WIN32)
		buffer_string_prepare_copy(hctx->response, 4 * 1024);
#else
		if (ioctl(hctx->fd, FIONREAD, &toread) || toread <= 4*1024) {
			buffer_string_prepare_copy(hctx->response, 4 * 1024);
		} else {
			if (toread > MAX_READ_LIMIT) toread = MAX_READ_LIMIT;
			buffer_string_prepare_copy(hctx->response, toread);
		}
#endif

		if (-1 == (n = read(hctx->fd, hctx->response->ptr, hctx->response->size - 1))) {
			if (errno == EAGAIN || errno == EINTR) {
				/* would block, wait for signal */
				fdevent_event_add(srv->ev, &(hctx->fde_ndx), hctx->fd, FDEVENT_IN);
				return FDEVENT_HANDLED_NOT_FINISHED;
			}
			/* error */
			log_error_write(srv, __FILE__, __LINE__, "sdd", strerror(errno), con->fd, hctx->fd);
			return FDEVENT_HANDLED_ERROR;
		}

		if (n == 0) {
			/* read finished */
			return FDEVENT_HANDLED_FINISHED;
		}

		buffer_commit(hctx->response, n);

		/* split header from body */

		if (con->file_started == 0) {
			int is_header = 0;
			int is_header_end = 0;
			size_t last_eol = 0;
			size_t i, header_len;

			buffer_append_string_buffer(hctx->response_header, hctx->response);

			/**
			 * we have to handle a few cases:
			 *
			 * nph:
			 * 
			 *   HTTP/1.0 200 Ok\n
			 *   Header: Value\n
			 *   \n
			 *
			 * CGI:
			 *   Header: Value\n
			 *   Status: 200\n
			 *   \n
			 *
			 * and different mixes of \n and \r\n combinations
			 * 
			 * Some users also forget about CGI and just send a response and hope 
			 * we handle it. No headers, no header-content seperator
			 * 
			 */
			
			/* nph (non-parsed headers) */
			if (0 == strncmp(hctx->response_header->ptr, "HTTP/1.", 7)) is_header = 1;

			header_len = buffer_string_length(hctx->response_header);
			for (i = 0; !is_header_end && i < header_len; i++) {
				char c = hctx->response_header->ptr[i];

				switch (c) {
				case ':':
					/* we found a colon
					 *
					 * looks like we have a normal header 
					 */
					is_header = 1;
					break;
				case '\n':
					/* EOL */
					if (is_header == 0) {
						/* we got a EOL but we don't seem to got a HTTP header */

						is_header_end = 1;

						break;
					}

					/**
					 * check if we saw a \n(\r)?\n sequence 
					 */
					if (last_eol > 0 && 
					    ((i - last_eol == 1) || 
					     (i - last_eol == 2 && hctx->response_header->ptr[i - 1] == '\r'))) {
						is_header_end = 1;
						break;
					}

					last_eol = i;

					break;
				}
			}

			if (is_header_end) {
				if (!is_header) {
					/* no header, but a body */
					if (0 != http_chunk_append_buffer(srv, con, hctx->response_header)) {
						return FDEVENT_HANDLED_ERROR;
					}
				} else {
					const char *bstart;
					size_t blen;

					/* the body starts after the EOL */
					bstart = hctx->response_header->ptr + i;
					blen = header_len - i;

					/**
					 * i still points to the char after the terminating EOL EOL
					 *
					 * put it on the last \n again
					 */
					i--;

					/* string the last \r?\n */
					if (i > 0 && (hctx->response_header->ptr[i - 1] == '\r')) {
						i--;
					}

					buffer_string_set_length(hctx->response_header, i);

					/* parse the response header */
					cgi_response_parse(srv, con, p, hctx->response_header);

					if (con->http_status >= 300 && con->http_status < 400) {
						/*(con->parsed_response & HTTP_LOCATION)*/
						size_t ulen = buffer_string_length(con->uri.path);
						data_string *ds;
						if (NULL != (ds = (data_string *) array_get_element(con->response.headers, "Location"))
						    && ds->value->ptr[0] == '/'
						    && (0 != strncmp(ds->value->ptr, con->uri.path->ptr, ulen)
							|| (ds->value->ptr[ulen] != '\0' && ds->value->ptr[ulen] != '/' && ds->value->ptr[ulen] != '?'))
						    && NULL == array_get_element(con->response.headers, "Set-Cookie")) {
							if (++con->loops_per_request > 5) {
								log_error_write(srv, __FILE__, __LINE__, "sb", "too many internal loops while processing request:", con->request.orig_uri);
								con->http_status = 500; /* Internal Server Error */
								con->mode = DIRECT;
								return FDEVENT_HANDLED_FINISHED;
							}

							buffer_copy_buffer(con->request.uri, ds->value);

							if (con->request.content_length) {
								if (con->request.content_length != con->request_content_queue->bytes_in) {
									con->keep_alive = 0;
								}
								con->request.content_length = 0;
								chunkqueue_reset(con->request_content_queue);
							}

							if (con->http_status != 307 && con->http_status != 308) {
								/* Note: request body (if any) sent to initial dynamic handler
								 * and is not available to the internal redirect */
								con->request.http_method = HTTP_METHOD_GET;
							}

							connection_response_reset(srv, con); /*(includes con->http_status = 0)*/

							con->mode = DIRECT;
							return FDEVENT_HANDLED_COMEBACK;
						}
					}

					if (hctx->conf.xsendfile_allow) {
						data_string *ds;
						if (NULL != (ds = (data_string *) array_get_element(con->response.headers, "X-Sendfile"))) {
							http_response_xsendfile(srv, con, ds->value, hctx->conf.xsendfile_docroot);
							return FDEVENT_HANDLED_FINISHED;
						}
					}

					if (blen > 0) {
						if (0 != http_chunk_append_mem(srv, con, bstart, blen)) {
							return FDEVENT_HANDLED_ERROR;
						}
					}
				}

				con->file_started = 1;
			} else {
				/*(reuse MAX_HTTP_REQUEST_HEADER as max size for response headers from backends)*/
				if (header_len > MAX_HTTP_REQUEST_HEADER) {
					log_error_write(srv, __FILE__, __LINE__, "sb", "response headers too large for", con->uri.path);
					con->http_status = 502; /* Bad Gateway */
					con->mode = DIRECT;
					return FDEVENT_HANDLED_FINISHED;
				}
			}
		} else {
			if (0 != http_chunk_append_buffer(srv, con, hctx->response)) {
				return FDEVENT_HANDLED_ERROR;
			}
			if ((con->conf.stream_response_body & FDEVENT_STREAM_RESPONSE_BUFMIN)
			    && chunkqueue_length(con->write_queue) > 65536 - 4096) {
				if (!con->is_writable) {
					/*(defer removal of FDEVENT_IN interest since
					 * connection_state_machine() might be able to send data
					 * immediately, unless !con->is_writable, where
					 * connection_state_machine() might not loop back to call
					 * mod_cgi_handle_subrequest())*/
					fdevent_event_clr(srv->ev, &(hctx->fde_ndx), hctx->fd, FDEVENT_IN);
				}
				break;
			}
		}

#if 0
		log_error_write(srv, __FILE__, __LINE__, "ddss", con->fd, hctx->fd, connection_get_state(con->state), b->ptr);
#endif
	}

	return FDEVENT_HANDLED_NOT_FINISHED;
}

static void cgi_connection_close_fdtocgi(server *srv, handler_ctx *hctx) {
	/*(closes only hctx->fdtocgi)*/
	fdevent_event_del(srv->ev, &(hctx->fde_ndx_tocgi), hctx->fdtocgi);
	fdevent_unregister(srv->ev, hctx->fdtocgi);
	fdevent_sched_close(srv->ev, hctx->fdtocgi, 0);
	hctx->fdtocgi = -1;
}

static void cgi_connection_close(server *srv, handler_ctx *hctx) {
	int status;
	pid_t pid;
	plugin_data *p = hctx->plugin_data;
	connection *con = hctx->remote_conn;

#ifndef __WIN32

	/* the connection to the browser went away, but we still have a connection
	 * to the CGI script
	 *
	 * close cgi-connection
	 */

	if (hctx->fd != -1) {
		/* close connection to the cgi-script */
		fdevent_event_del(srv->ev, &(hctx->fde_ndx), hctx->fd);
		fdevent_unregister(srv->ev, hctx->fd);
		fdevent_sched_close(srv->ev, hctx->fd, 0);
	}

	if (hctx->fdtocgi != -1) {
		cgi_connection_close_fdtocgi(srv, hctx); /*(closes only hctx->fdtocgi)*/
	}

	pid = hctx->pid;

	con->plugin_ctx[p->id] = NULL;

	cgi_handler_ctx_free(hctx);

	/* if waitpid hasn't been called by response.c yet, do it here */
	if (pid) {
		/* check if the CGI-script is already gone */
		switch(waitpid(pid, &status, WNOHANG)) {
		case 0:
			/* not finished yet */
#if 0
			log_error_write(srv, __FILE__, __LINE__, "sd", "(debug) child isn't done yet, pid:", pid);
#endif
			break;
		case -1:
			/* */
			if (errno == EINTR) break;

			/*
			 * errno == ECHILD happens if _subrequest catches the process-status before
			 * we have read the response of the cgi process
			 *
			 * -> catch status
			 * -> WAIT_FOR_EVENT
			 * -> read response
			 * -> we get here with waitpid == ECHILD
			 *
			 */
			if (errno != ECHILD) {
				log_error_write(srv, __FILE__, __LINE__, "ss", "waitpid failed: ", strerror(errno));
			}
			/* anyway: don't wait for it anymore */
			pid = 0;
			break;
		default:
			if (WIFEXITED(status)) {
#if 0
				log_error_write(srv, __FILE__, __LINE__, "sd", "(debug) cgi exited fine, pid:", pid);
#endif
			} else {
				log_error_write(srv, __FILE__, __LINE__, "sd", "cgi died, pid:", pid);
			}

			pid = 0;
			break;
		}

		if (pid) {
			kill(pid, SIGTERM);

			/* cgi-script is still alive, queue the PID for removal */
			cgi_pid_add(srv, p, pid);
		}
	}
#endif

	/* finish response (if not already con->file_started, con->file_finished) */
	if (con->mode == p->id) {
		http_response_backend_done(srv, con);
	}
}

static handler_t cgi_connection_close_callback(server *srv, connection *con, void *p_d) {
	plugin_data *p = p_d;
	handler_ctx *hctx = con->plugin_ctx[p->id];
	if (hctx) cgi_connection_close(srv, hctx);

	return HANDLER_GO_ON;
}


static int cgi_write_request(server *srv, handler_ctx *hctx, int fd);


static handler_t cgi_handle_fdevent_send (server *srv, void *ctx, int revents) {
	handler_ctx *hctx = ctx;
	connection  *con  = hctx->remote_conn;

	/*(joblist only actually necessary here in mod_cgi fdevent send if returning HANDLER_ERROR)*/
	joblist_append(srv, con);

	if (revents & FDEVENT_OUT) {
		if (0 != cgi_write_request(srv, hctx, hctx->fdtocgi)) {
			cgi_connection_close(srv, hctx);
			return HANDLER_ERROR;
		}
		/* more request body to be sent to CGI */
	}

	if (revents & FDEVENT_HUP) {
		/* skip sending remaining data to CGI */
		if (con->request.content_length) {
			chunkqueue *cq = con->request_content_queue;
			chunkqueue_mark_written(cq, chunkqueue_length(cq));
			if (cq->bytes_in != (off_t)con->request.content_length) {
				con->keep_alive = 0;
			}
		}

		cgi_connection_close_fdtocgi(srv, hctx); /*(closes only hctx->fdtocgi)*/
	} else if (revents & FDEVENT_ERR) {
		/* kill all connections to the cgi process */
#if 1
		log_error_write(srv, __FILE__, __LINE__, "s", "cgi-FDEVENT_ERR");
#endif
		cgi_connection_close(srv, hctx);
		return HANDLER_ERROR;
	}

	return HANDLER_FINISHED;
}


static int cgi_recv_response(server *srv, handler_ctx *hctx) {
		switch (cgi_demux_response(srv, hctx)) {
		case FDEVENT_HANDLED_NOT_FINISHED:
			break;
		case FDEVENT_HANDLED_FINISHED:
			/* we are done */

#if 0
			log_error_write(srv, __FILE__, __LINE__, "ddss", con->fd, hctx->fd, connection_get_state(con->state), "finished");
#endif
			cgi_connection_close(srv, hctx);

			/* if we get a IN|HUP and have read everything don't exec the close twice */
			return HANDLER_FINISHED;
		case FDEVENT_HANDLED_COMEBACK:
			cgi_connection_close(srv, hctx);
			return HANDLER_COMEBACK;
		case FDEVENT_HANDLED_ERROR:
			log_error_write(srv, __FILE__, __LINE__, "s", "demuxer failed: ");

			cgi_connection_close(srv, hctx);
			return HANDLER_FINISHED;
		}

		return HANDLER_GO_ON;
}


static handler_t cgi_handle_fdevent(server *srv, void *ctx, int revents) {
	handler_ctx *hctx = ctx;
	connection  *con  = hctx->remote_conn;

	joblist_append(srv, con);

	if (revents & FDEVENT_IN) {
		handler_t rc = cgi_recv_response(srv, hctx);/*(might invalidate hctx)*/
		if (rc != HANDLER_GO_ON) return rc;         /*(unless HANDLER_GO_ON)*/
	}

	/* perhaps this issue is already handled */
	if (revents & FDEVENT_HUP) {
		if (con->file_started) {
			/* drain any remaining data from kernel pipe buffers
			 * even if (con->conf.stream_response_body
			 *          & FDEVENT_STREAM_RESPONSE_BUFMIN)
			 * since event loop will spin on fd FDEVENT_HUP event
			 * until unregistered. */
			handler_t rc;
			do {
				rc = cgi_recv_response(srv,hctx);/*(might invalidate hctx)*/
			} while (rc == HANDLER_GO_ON);           /*(unless HANDLER_GO_ON)*/
			return rc; /* HANDLER_FINISHED or HANDLER_COMEBACK or HANDLER_ERROR */
		} else if (!buffer_string_is_empty(hctx->response_header)) {
			/* unfinished header package which is a body in reality */
			con->file_started = 1;
			if (0 != http_chunk_append_buffer(srv, con, hctx->response_header)) {
				cgi_connection_close(srv, hctx);
				return HANDLER_ERROR;
			}
		} else {
# if 0
			log_error_write(srv, __FILE__, __LINE__, "sddd", "got HUP from cgi", con->fd, hctx->fd, revents);
# endif
		}
		cgi_connection_close(srv, hctx);
	} else if (revents & FDEVENT_ERR) {
		/* kill all connections to the cgi process */
		cgi_connection_close(srv, hctx);
#if 1
		log_error_write(srv, __FILE__, __LINE__, "s", "cgi-FDEVENT_ERR");
#endif
		return HANDLER_ERROR;
	}

	return HANDLER_FINISHED;
}


static int cgi_env_add(void *venv, const char *key, size_t key_len, const char *val, size_t val_len) {
	char_array *env = venv;
	char *dst;

	if (!key || !val) return -1;

	dst = malloc(key_len + val_len + 2);
	force_assert(dst);
	memcpy(dst, key, key_len);
	dst[key_len] = '=';
	memcpy(dst + key_len + 1, val, val_len);
	dst[key_len + 1 + val_len] = '\0';

	if (env->size == 0) {
		env->size = 16;
		env->ptr = malloc(env->size * sizeof(*env->ptr));
		force_assert(env->ptr);
	} else if (env->size == env->used) {
		env->size += 16;
		env->ptr = realloc(env->ptr, env->size * sizeof(*env->ptr));
		force_assert(env->ptr);
	}

	env->ptr[env->used++] = dst;

	return 0;
}

/*(improved from network_write_mmap.c)*/
static off_t mmap_align_offset(off_t start) {
    static off_t pagemask = 0;
    if (0 == pagemask) {
        long pagesize = sysconf(_SC_PAGESIZE);
        if (-1 == pagesize) pagesize = 4096;
        pagemask = ~((off_t)pagesize - 1); /* pagesize always power-of-2 */
    }
    return (start & pagemask);
}

/* returns: 0: continue, -1: fatal error, -2: connection reset */
/* similar to network_write_file_chunk_mmap, but doesn't use send on windows (because we're on pipes),
 * also mmaps and sends complete chunk instead of only small parts - the files
 * are supposed to be temp files with reasonable chunk sizes.
 *
 * Also always use mmap; the files are "trusted", as we created them.
 */
static ssize_t cgi_write_file_chunk_mmap(server *srv, connection *con, int fd, chunkqueue *cq) {
	chunk* const c = cq->first;
	off_t offset, toSend, file_end;
	ssize_t r;
	size_t mmap_offset, mmap_avail;
	char *data;

	force_assert(NULL != c);
	force_assert(FILE_CHUNK == c->type);
	force_assert(c->offset >= 0 && c->offset <= c->file.length);

	offset = c->file.start + c->offset;
	toSend = c->file.length - c->offset;
	file_end = c->file.start + c->file.length; /* offset to file end in this chunk */

	if (0 == toSend) {
		chunkqueue_remove_finished_chunks(cq);
		return 0;
	}

	/*(simplified from network_write_no_mmap.c:network_open_file_chunk())*/
	UNUSED(con);
	if (-1 == c->file.fd) {
		if (-1 == (c->file.fd = fdevent_open_cloexec(c->file.name->ptr, O_RDONLY, 0))) {
			log_error_write(srv, __FILE__, __LINE__, "ssb", "open failed:", strerror(errno), c->file.name);
			return -1;
		}
	}

	/* (re)mmap the buffer if range is not covered completely */
	if (MAP_FAILED == c->file.mmap.start
		|| offset < c->file.mmap.offset
		|| file_end > (off_t)(c->file.mmap.offset + c->file.mmap.length)) {

		if (MAP_FAILED != c->file.mmap.start) {
			munmap(c->file.mmap.start, c->file.mmap.length);
			c->file.mmap.start = MAP_FAILED;
		}

		c->file.mmap.offset = mmap_align_offset(offset);
		c->file.mmap.length = file_end - c->file.mmap.offset;

		if (MAP_FAILED == (c->file.mmap.start = mmap(NULL, c->file.mmap.length, PROT_READ, MAP_PRIVATE, c->file.fd, c->file.mmap.offset))) {
			if (toSend > 65536) toSend = 65536;
			data = malloc(toSend);
			force_assert(data);
			if (-1 == lseek(c->file.fd, offset, SEEK_SET)
			    || 0 >= (toSend = read(c->file.fd, data, toSend))) {
				if (-1 == toSend) {
					log_error_write(srv, __FILE__, __LINE__, "ssbdo", "lseek/read failed:",
						strerror(errno), c->file.name, c->file.fd, offset);
				} else { /*(0 == toSend)*/
					log_error_write(srv, __FILE__, __LINE__, "sbdo", "unexpected EOF (input truncated?):",
						c->file.name, c->file.fd, offset);
				}
				free(data);
				return -1;
			}
		}
	}

	if (MAP_FAILED != c->file.mmap.start) {
		force_assert(offset >= c->file.mmap.offset);
		mmap_offset = offset - c->file.mmap.offset;
		force_assert(c->file.mmap.length > mmap_offset);
		mmap_avail = c->file.mmap.length - mmap_offset;
		force_assert(toSend <= (off_t) mmap_avail);

		data = c->file.mmap.start + mmap_offset;
	}

	r = write(fd, data, toSend);

	if (MAP_FAILED == c->file.mmap.start) free(data);

	if (r < 0) {
		switch (errno) {
		case EAGAIN:
		case EINTR:
			return 0;
		case EPIPE:
		case ECONNRESET:
			return -2;
		default:
			log_error_write(srv, __FILE__, __LINE__, "ssd",
				"write failed:", strerror(errno), fd);
			return -1;
		}
	}

	if (r >= 0) {
		chunkqueue_mark_written(cq, r);
	}

	return r;
}

static int cgi_write_request(server *srv, handler_ctx *hctx, int fd) {
	connection *con = hctx->remote_conn;
	chunkqueue *cq = con->request_content_queue;
	chunk *c;

	/* old comment: windows doesn't support select() on pipes - wouldn't be easy to fix for all platforms.
	 * solution: if this is still a problem on windows, then substitute
	 * socketpair() for pipe() and closesocket() for close() on windows.
	 */

	for (c = cq->first; c; c = cq->first) {
		ssize_t r = -1;

		switch(c->type) {
		case FILE_CHUNK:
			r = cgi_write_file_chunk_mmap(srv, con, fd, cq);
			break;

		case MEM_CHUNK:
			if ((r = write(fd, c->mem->ptr + c->offset, buffer_string_length(c->mem) - c->offset)) < 0) {
				switch(errno) {
				case EAGAIN:
				case EINTR:
					/* ignore and try again */
					r = 0;
					break;
				case EPIPE:
				case ECONNRESET:
					/* connection closed */
					r = -2;
					break;
				default:
					/* fatal error */
					log_error_write(srv, __FILE__, __LINE__, "ss", "write failed due to: ", strerror(errno));
					r = -1;
					break;
				}
			} else if (r > 0) {
				chunkqueue_mark_written(cq, r);
			}
			break;
		}

		if (0 == r) break; /*(might block)*/

		switch (r) {
		case -1:
			/* fatal error */
			return -1;
		case -2:
			/* connection reset */
			log_error_write(srv, __FILE__, __LINE__, "s", "failed to send post data to cgi, connection closed by CGI");
			/* skip all remaining data */
			chunkqueue_mark_written(cq, chunkqueue_length(cq));
			break;
		default:
			break;
		}
	}

	if (cq->bytes_out == (off_t)con->request.content_length) {
		/* sent all request body input */
		/* close connection to the cgi-script */
		if (-1 == hctx->fdtocgi) { /*(received request body sent in initial send to pipe buffer)*/
			--srv->cur_fds;
			if (close(fd)) {
				log_error_write(srv, __FILE__, __LINE__, "sds", "cgi stdin close failed ", fd, strerror(errno));
			}
		} else {
			cgi_connection_close_fdtocgi(srv, hctx); /*(closes only hctx->fdtocgi)*/
		}
	} else {
		off_t cqlen = cq->bytes_in - cq->bytes_out;
		if (cq->bytes_in != con->request.content_length && cqlen < 65536 - 16384) {
			/*(con->conf.stream_request_body & FDEVENT_STREAM_REQUEST)*/
			if (!(con->conf.stream_request_body & FDEVENT_STREAM_REQUEST_POLLIN)) {
				con->conf.stream_request_body |= FDEVENT_STREAM_REQUEST_POLLIN;
				con->is_readable = 1; /* trigger optimistic read from client */
			}
		}
		if (-1 == hctx->fdtocgi) { /*(not registered yet)*/
			hctx->fdtocgi = fd;
			hctx->fde_ndx_tocgi = -1;
			fdevent_register(srv->ev, hctx->fdtocgi, cgi_handle_fdevent_send, hctx);
		}
		if (0 == cqlen) { /*(chunkqueue_is_empty(cq))*/
			if ((fdevent_event_get_interest(srv->ev, hctx->fdtocgi) & FDEVENT_OUT)) {
				fdevent_event_set(srv->ev, &(hctx->fde_ndx_tocgi), hctx->fdtocgi, 0);
			}
		} else {
			/* more request body remains to be sent to CGI so register for fdevents */
			fdevent_event_set(srv->ev, &(hctx->fde_ndx_tocgi), hctx->fdtocgi, FDEVENT_OUT);
		}
	}

	return 0;
}

static int cgi_create_env(server *srv, connection *con, plugin_data *p, handler_ctx *hctx, buffer *cgi_handler) {
	pid_t pid;

	int to_cgi_fds[2];
	int from_cgi_fds[2];
	struct stat st;
	UNUSED(p);

#ifndef __WIN32

	if (!buffer_string_is_empty(cgi_handler)) {
		/* stat the exec file */
		if (-1 == (stat(cgi_handler->ptr, &st))) {
			log_error_write(srv, __FILE__, __LINE__, "sbss",
					"stat for cgi-handler", cgi_handler,
					"failed:", strerror(errno));
			return -1;
		}
	}

	if (pipe_cloexec(to_cgi_fds)) {
		log_error_write(srv, __FILE__, __LINE__, "ss", "pipe failed:", strerror(errno));
		return -1;
	}

	if (pipe_cloexec(from_cgi_fds)) {
		close(to_cgi_fds[0]);
		close(to_cgi_fds[1]);
		log_error_write(srv, __FILE__, __LINE__, "ss", "pipe failed:", strerror(errno));
		return -1;
	}

	/* fork, execve */
	switch (pid = fork()) {
	case 0: {
		/* child */
		char **args;
		int argc;
		int i = 0;
		char_array env;
		char *c;
		const char *s;
		http_cgi_opts opts = { 0, 0, NULL, NULL };

		/* move stdout to from_cgi_fd[1] */
		dup2(from_cgi_fds[1], STDOUT_FILENO);
	      #ifndef FD_CLOEXEC
		close(from_cgi_fds[1]);
		/* not needed */
		close(from_cgi_fds[0]);
	      #endif

		/* move the stdin to to_cgi_fd[0] */
		dup2(to_cgi_fds[0], STDIN_FILENO);
	      #ifndef FD_CLOEXEC
		close(to_cgi_fds[0]);
		/* not needed */
		close(to_cgi_fds[1]);
	      #endif

		/* create environment */
		env.ptr = NULL;
		env.size = 0;
		env.used = 0;

		http_cgi_headers(srv, con, &opts, cgi_env_add, &env);

		/* for valgrind */
		if (NULL != (s = getenv("LD_PRELOAD"))) {
			cgi_env_add(&env, CONST_STR_LEN("LD_PRELOAD"), s, strlen(s));
		}

		if (NULL != (s = getenv("LD_LIBRARY_PATH"))) {
			cgi_env_add(&env, CONST_STR_LEN("LD_LIBRARY_PATH"), s, strlen(s));
		}
#ifdef __CYGWIN__
		/* CYGWIN needs SYSTEMROOT */
		if (NULL != (s = getenv("SYSTEMROOT"))) {
			cgi_env_add(&env, CONST_STR_LEN("SYSTEMROOT"), s, strlen(s));
		}
#endif

		if (env.size == env.used) {
			env.size += 16;
			env.ptr = realloc(env.ptr, env.size * sizeof(*env.ptr));
		}

		env.ptr[env.used] = NULL;

		/* set up args */
		argc = 3;
		args = malloc(sizeof(*args) * argc);
		force_assert(args);
		i = 0;

		if (!buffer_string_is_empty(cgi_handler)) {
			args[i++] = cgi_handler->ptr;
		}
		args[i++] = con->physical.path->ptr;
		args[i  ] = NULL;

		/* search for the last / */
		if (NULL != (c = strrchr(con->physical.path->ptr, '/'))) {
			/* handle special case of file in root directory */
			const char* physdir = (c == con->physical.path->ptr) ? "/" : con->physical.path->ptr;

			/* temporarily shorten con->physical.path to directory without terminating '/' */
			*c = '\0';
			/* change to the physical directory */
			if (-1 == chdir(physdir)) {
				log_error_write(srv, __FILE__, __LINE__, "ssb", "chdir failed:", strerror(errno), con->physical.path);
			}
			*c = '/';
		}

		/* we don't need the client socket */
		for (i = 3; i < 256; i++) {
			if (i != srv->errorlog_fd) close(i);
		}

		/* exec the cgi */
		execve(args[0], args, env.ptr);

		/* most log files may have been closed/redirected by this point,
		 * though stderr might still point to lighttpd.breakage.log */
		perror(args[0]);
		_exit(1);
	}
	case -1:
		/* error */
		log_error_write(srv, __FILE__, __LINE__, "ss", "fork failed:", strerror(errno));
		close(from_cgi_fds[0]);
		close(from_cgi_fds[1]);
		close(to_cgi_fds[0]);
		close(to_cgi_fds[1]);
		return -1;
	default: {
		/* parent process */

		close(from_cgi_fds[1]);
		close(to_cgi_fds[0]);

		/* register PID and wait for them asynchronously */

		hctx->pid = pid;
		hctx->fd = from_cgi_fds[0];
		hctx->fde_ndx = -1;

		++srv->cur_fds;

		if (0 == con->request.content_length) {
			close(to_cgi_fds[1]);
		} else {
			/* there is content to send */
			if (-1 == fdevent_fcntl_set_nb(srv->ev, to_cgi_fds[1])) {
				log_error_write(srv, __FILE__, __LINE__, "ss", "fcntl failed: ", strerror(errno));
				close(to_cgi_fds[1]);
				cgi_connection_close(srv, hctx);
				return -1;
			}

			if (0 != cgi_write_request(srv, hctx, to_cgi_fds[1])) {
				close(to_cgi_fds[1]);
				cgi_connection_close(srv, hctx);
				return -1;
			}

			++srv->cur_fds;
		}

		fdevent_register(srv->ev, hctx->fd, cgi_handle_fdevent, hctx);
		if (-1 == fdevent_fcntl_set_nb(srv->ev, hctx->fd)) {
			log_error_write(srv, __FILE__, __LINE__, "ss", "fcntl failed: ", strerror(errno));
			cgi_connection_close(srv, hctx);
			return -1;
		}
		fdevent_event_set(srv->ev, &(hctx->fde_ndx), hctx->fd, FDEVENT_IN);

		break;
	}
	}

	return 0;
#else
	return -1;
#endif
}

static buffer * cgi_get_handler(array *a, buffer *fn) {
	size_t k, s_len = buffer_string_length(fn);
	for (k = 0; k < a->used; ++k) {
		data_string *ds = (data_string *)a->data[k];
		size_t ct_len = buffer_string_length(ds->key);

		if (buffer_is_empty(ds->key)) continue;
		if (s_len < ct_len) continue;

		if (0 == strncmp(fn->ptr + s_len - ct_len, ds->key->ptr, ct_len)) {
			return ds->value;
		}
	}

	return NULL;
}

#define PATCH(x) \
	p->conf.x = s->x;
static int mod_cgi_patch_connection(server *srv, connection *con, plugin_data *p) {
	size_t i, j;
	plugin_config *s = p->config_storage[0];

	PATCH(cgi);
	PATCH(execute_x_only);
	PATCH(xsendfile_allow);
	PATCH(xsendfile_docroot);

	/* skip the first, the global context */
	for (i = 1; i < srv->config_context->used; i++) {
		data_config *dc = (data_config *)srv->config_context->data[i];
		s = p->config_storage[i];

		/* condition didn't match */
		if (!config_check_cond(srv, con, dc)) continue;

		/* merge config */
		for (j = 0; j < dc->value->used; j++) {
			data_unset *du = dc->value->data[j];

			if (buffer_is_equal_string(du->key, CONST_STR_LEN("cgi.assign"))) {
				PATCH(cgi);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("cgi.execute-x-only"))) {
				PATCH(execute_x_only);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("cgi.x-sendfile"))) {
				PATCH(xsendfile_allow);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("cgi.x-sendfile-docroot"))) {
				PATCH(xsendfile_docroot);
			}
		}
	}

	return 0;
}
#undef PATCH

URIHANDLER_FUNC(cgi_is_handled) {
	plugin_data *p = p_d;
	buffer *fn = con->physical.path;
	stat_cache_entry *sce = NULL;
	struct stat stbuf;
	struct stat *st;
	buffer *cgi_handler;

	if (con->mode != DIRECT) return HANDLER_GO_ON;

	if (buffer_is_empty(fn)) return HANDLER_GO_ON;

	mod_cgi_patch_connection(srv, con, p);

	if (HANDLER_ERROR != stat_cache_get_entry(srv, con, con->physical.path, &sce)) {
		st = &sce->st;
	} else {
		/* CGI might be executable even if it is not readable
		 * (stat_cache_get_entry() currently checks file is readable)*/
		if (0 != stat(con->physical.path->ptr, &stbuf)) return HANDLER_GO_ON;
		st = &stbuf;
	}

	if (!S_ISREG(st->st_mode)) return HANDLER_GO_ON;
	if (p->conf.execute_x_only == 1 && (st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) == 0) return HANDLER_GO_ON;

	if (NULL != (cgi_handler = cgi_get_handler(p->conf.cgi, fn))) {
		handler_ctx *hctx = cgi_handler_ctx_init();
		hctx->remote_conn = con;
		hctx->plugin_data = p;
		hctx->cgi_handler = cgi_handler;
		memcpy(&hctx->conf, &p->conf, sizeof(plugin_config));
		con->plugin_ctx[p->id] = hctx;
		con->mode = p->id;
	}

	return HANDLER_GO_ON;
}

TRIGGER_FUNC(cgi_trigger) {
	plugin_data *p = p_d;
	size_t ndx;
	/* the trigger handle only cares about lonely PID which we have to wait for */
#ifndef __WIN32

	for (ndx = 0; ndx < p->cgi_pid.used; ndx++) {
		int status;

		switch(waitpid(p->cgi_pid.ptr[ndx], &status, WNOHANG)) {
		case 0:
			/* not finished yet */
#if 0
			log_error_write(srv, __FILE__, __LINE__, "sd", "(debug) child isn't done yet, pid:", p->cgi_pid.ptr[ndx]);
#endif
			break;
		case -1:
			if (errno == ECHILD) {
				/* someone else called waitpid... remove the pid to stop looping the error each time */
				log_error_write(srv, __FILE__, __LINE__, "s", "cgi child vanished, probably someone else called waitpid");

				cgi_pid_del(srv, p, p->cgi_pid.ptr[ndx]);
				ndx--;
				continue;
			}

			log_error_write(srv, __FILE__, __LINE__, "ss", "waitpid failed: ", strerror(errno));

			return HANDLER_ERROR;
		default:

			if (WIFEXITED(status)) {
#if 0
				log_error_write(srv, __FILE__, __LINE__, "sd", "(debug) cgi exited fine, pid:", p->cgi_pid.ptr[ndx]);
#endif
			} else if (WIFSIGNALED(status)) {
				/* FIXME: what if we killed the CGI script with a kill(..., SIGTERM) ?
				 */
				if (WTERMSIG(status) != SIGTERM) {
					log_error_write(srv, __FILE__, __LINE__, "sd", "cleaning up CGI: process died with signal", WTERMSIG(status));
				}
			} else {
				log_error_write(srv, __FILE__, __LINE__, "s", "cleaning up CGI: ended unexpectedly");
			}

			cgi_pid_del(srv, p, p->cgi_pid.ptr[ndx]);
			/* del modified the buffer structure
			 * and copies the last entry to the current one
			 * -> recheck the current index
			 */
			ndx--;
		}
	}
#endif
	return HANDLER_GO_ON;
}

/*
 * - HANDLER_GO_ON : not our job
 * - HANDLER_FINISHED: got response
 * - HANDLER_WAIT_FOR_EVENT: waiting for response
 */
SUBREQUEST_FUNC(mod_cgi_handle_subrequest) {
	plugin_data *p = p_d;
	handler_ctx *hctx = con->plugin_ctx[p->id];
	chunkqueue *cq = con->request_content_queue;

	if (con->mode != p->id) return HANDLER_GO_ON;
	if (NULL == hctx) return HANDLER_GO_ON;

	if ((con->conf.stream_response_body & FDEVENT_STREAM_RESPONSE_BUFMIN)
	    && con->file_started) {
		if (chunkqueue_length(con->write_queue) > 65536 - 4096) {
			fdevent_event_clr(srv->ev, &(hctx->fde_ndx), hctx->fd, FDEVENT_IN);
		} else if (!(fdevent_event_get_interest(srv->ev, hctx->fd) & FDEVENT_IN)) {
			/* optimistic read from backend, which might re-enable FDEVENT_IN */
			handler_t rc = cgi_recv_response(srv, hctx); /*(might invalidate hctx)*/
			if (rc != HANDLER_GO_ON) return rc;          /*(unless HANDLER_GO_ON)*/
		}
	}

	if (cq->bytes_in != (off_t)con->request.content_length) {
		/*(64k - 4k to attempt to avoid temporary files
		 * in conjunction with FDEVENT_STREAM_REQUEST_BUFMIN)*/
		if (cq->bytes_in - cq->bytes_out > 65536 - 4096
		    && (con->conf.stream_request_body & FDEVENT_STREAM_REQUEST_BUFMIN)){
			con->conf.stream_request_body &= ~FDEVENT_STREAM_REQUEST_POLLIN;
			if (-1 != hctx->fd) return HANDLER_WAIT_FOR_EVENT;
		} else {
			handler_t r = connection_handle_read_post_state(srv, con);
			if (!chunkqueue_is_empty(cq)) {
				if (fdevent_event_get_interest(srv->ev, hctx->fdtocgi) & FDEVENT_OUT) {
					return (r == HANDLER_GO_ON) ? HANDLER_WAIT_FOR_EVENT : r;
				}
			}
			if (r != HANDLER_GO_ON) return r;

			/* CGI environment requires that Content-Length be set.
			 * Send 411 Length Required if Content-Length missing.
			 * (occurs here if client sends Transfer-Encoding: chunked
			 *  and module is flagged to stream request body to backend) */
			if (-1 == con->request.content_length) {
				return connection_handle_read_post_error(srv, con, 411);
			}
		}
	}

	if (-1 == hctx->fd) {
		if (cgi_create_env(srv, con, p, hctx, hctx->cgi_handler)) {
			con->http_status = 500;
			con->mode = DIRECT;

			return HANDLER_FINISHED;
		}
#if 0
	log_error_write(srv, __FILE__, __LINE__, "sdd", "subrequest, pid =", hctx, hctx->pid);
#endif
	} else if (!chunkqueue_is_empty(con->request_content_queue)) {
		if (0 != cgi_write_request(srv, hctx, hctx->fdtocgi)) {
			cgi_connection_close(srv, hctx);
			return HANDLER_ERROR;
		}
	}

	/* if not done, wait for CGI to close stdout, so we read EOF on pipe */
	return HANDLER_WAIT_FOR_EVENT;
}


int mod_cgi_plugin_init(plugin *p);
int mod_cgi_plugin_init(plugin *p) {
	p->version     = LIGHTTPD_VERSION_ID;
	p->name        = buffer_init_string("cgi");

	p->connection_reset = cgi_connection_close_callback;
	p->handle_subrequest_start = cgi_is_handled;
	p->handle_subrequest = mod_cgi_handle_subrequest;
	p->handle_trigger = cgi_trigger;
	p->init           = mod_cgi_init;
	p->cleanup        = mod_cgi_free;
	p->set_defaults   = mod_fastcgi_set_defaults;

	p->data        = NULL;

	return 0;
}
