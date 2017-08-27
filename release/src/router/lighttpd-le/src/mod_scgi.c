#include "first.h"

#include "buffer.h"
#include "server.h"
#include "keyvalue.h"
#include "log.h"

#include "http_chunk.h"
#include "fdevent.h"
#include "connections.h"
#include "response.h"
#include "joblist.h"

#include "plugin.h"

#include "inet_ntop_cache.h"

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <signal.h>

#include <stdio.h>

#include "sys-socket.h"
#include "sys-endian.h"

#ifdef HAVE_SYS_UIO_H
# include <sys/uio.h>
#endif

#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

enum {EOL_UNSET, EOL_N, EOL_RN};

/*
 *
 * TODO:
 *
 * - add timeout for a connect to a non-scgi process
 *   (use state_timestamp + state)
 *
 */

typedef struct scgi_proc {
	size_t id; /* id will be between 1 and max_procs */
	buffer *socket; /* config.socket + "-" + id */
	unsigned port;  /* config.port + pno */

	pid_t pid;   /* PID of the spawned process (0 if not spawned locally) */


	size_t load; /* number of requests waiting on this process */

	time_t last_used; /* see idle_timeout */
	size_t requests;  /* see max_requests */
	struct scgi_proc *prev, *next; /* see first */

	time_t disable_ts; /* replace by host->something */

	int is_local;

	enum { PROC_STATE_UNSET,            /* init-phase */
			PROC_STATE_RUNNING, /* alive */
			PROC_STATE_DIED_WAIT_FOR_PID,
			PROC_STATE_KILLED,  /* was killed as we don't have the load anymore */
			PROC_STATE_DIED,    /* marked as dead, should be restarted */
			PROC_STATE_DISABLED /* proc disabled as it resulted in an error */
	} state;
} scgi_proc;

typedef struct {
	/* list of processes handling this extension
	 * sorted by lowest load
	 *
	 * whenever a job is done move it up in the list
	 * until it is sorted, move it down as soon as the
	 * job is started
	 */
	scgi_proc *first;
	scgi_proc *unused_procs;

	/*
	 * spawn at least min_procs, at max_procs.
	 *
	 * as soon as the load of the first entry
	 * is max_load_per_proc we spawn a new one
	 * and add it to the first entry and give it
	 * the load
	 *
	 */

	unsigned short min_procs;
	unsigned short max_procs;
	size_t num_procs;    /* how many procs are started */
	size_t active_procs; /* how many of them are really running */

	unsigned short max_load_per_proc;

	/*
	 * kick the process from the list if it was not
	 * used for idle_timeout until min_procs is
	 * reached. this helps to get the processlist
	 * small again we had a small peak load.
	 *
	 */

	unsigned short idle_timeout;

	/*
	 * time after a disabled remote connection is tried to be re-enabled
	 *
	 *
	 */

	unsigned short disable_time;

	/*
	 * same scgi processes get a little bit larger
	 * than wanted. max_requests_per_proc kills a
	 * process after a number of handled requests.
	 *
	 */
	size_t max_requests_per_proc;


	/* config */

	/*
	 * host:port
	 *
	 * if host is one of the local IP adresses the
	 * whole connection is local
	 *
	 * if tcp/ip should be used host AND port have
	 * to be specified
	 *
	 */
	buffer *host;
	unsigned short port;
	sa_family_t family;

	/*
	 * Unix Domain Socket
	 *
	 * instead of TCP/IP we can use Unix Domain Sockets
	 * - more secure (you have fileperms to play with)
	 * - more control (on locally)
	 * - more speed (no extra overhead)
	 */
	buffer *unixsocket;

	/* if socket is local we can start the scgi
	 * process ourself
	 *
	 * bin-path is the path to the binary
	 *
	 * check min_procs and max_procs for the number
	 * of process to start-up
	 */
	buffer *bin_path;

	/* bin-path is set bin-environment is taken to
	 * create the environement before starting the
	 * FastCGI process
	 *
	 */
	array *bin_env;

	array *bin_env_copy;

	/*
	 * docroot-translation between URL->phys and the
	 * remote host
	 *
	 * reasons:
	 * - different dir-layout if remote
	 * - chroot if local
	 *
	 */
	buffer *docroot;

	/*
	 * check_local tell you if the phys file is stat()ed
	 * or not. FastCGI doesn't care if the service is
	 * remote. If the web-server side doesn't contain
	 * the scgi-files we should not stat() for them
	 * and say '404 not found'.
	 */
	unsigned short check_local;

	/*
	 * append PATH_INFO to SCRIPT_FILENAME
	 *
	 * php needs this if cgi.fix_pathinfo is provied
	 *
	 */

	/*
	 * workaround for program when prefix="/"
	 *
	 * rule to build PATH_INFO is hardcoded for when check_local is disabled
	 * enable this option to use the workaround
	 *
	 */

	unsigned short fix_root_path_name;

	/*
	 * If the backend includes X-Sendfile in the response
	 * we use the value as filename and ignore the content.
	 *
	 */
	unsigned short xsendfile_allow;
	array *xsendfile_docroot;

	ssize_t load; /* replace by host->load */

	size_t max_id; /* corresponds most of the time to
	num_procs.

	only if a process is killed max_id waits for the process itself
	to die and decrements its afterwards */

	int listen_backlog;
	int refcount;
} scgi_extension_host;

/*
 * one extension can have multiple hosts assigned
 * one host can spawn additional processes on the same
 *   socket (if we control it)
 *
 * ext -> host -> procs
 *    1:n     1:n
 *
 * if the scgi process is remote that whole goes down
 * to
 *
 * ext -> host -> procs
 *    1:n     1:1
 *
 * in case of PHP and FCGI_CHILDREN we have again a procs
 * but we don't control it directly.
 *
 */

typedef struct {
	buffer *key; /* like .php */

	int note_is_sent;
	scgi_extension_host **hosts;

	size_t used;
	size_t size;
} scgi_extension;

typedef struct {
	scgi_extension **exts;

	size_t used;
	size_t size;
} scgi_exts;

enum { LI_PROTOCOL_SCGI, LI_PROTOCOL_UWSGI };

typedef struct {
	scgi_exts *exts;

	int proto;
	int debug;
} plugin_config;

typedef struct {
	char **ptr;

	size_t size;
	size_t used;
} char_array;

/* generic plugin data, shared between all connections */
typedef struct {
	PLUGIN_DATA;

	buffer *scgi_env;

	buffer *parse_response;

	plugin_config **config_storage;

	plugin_config conf; /* this is only used as long as no handler_ctx is setup */
} plugin_data;

/* connection specific data */
typedef enum { FCGI_STATE_INIT, FCGI_STATE_CONNECT, FCGI_STATE_PREPARE_WRITE,
		FCGI_STATE_WRITE, FCGI_STATE_READ
} scgi_connection_state_t;

typedef struct {
	buffer  *response;

	scgi_proc *proc;
	scgi_extension_host *host;

	scgi_connection_state_t state;
	time_t   state_timestamp;

	chunkqueue *wb;
	off_t     wb_reqlen;

	buffer   *response_header;

	int       fd;        /* fd to the scgi process */
	int       fde_ndx;   /* index into the fd-event buffer */

	pid_t     pid;
	int       got_proc;
	int       reconnects; /* number of reconnect attempts */

	plugin_config conf;

	connection *remote_conn;  /* dumb pointer */
	plugin_data *plugin_data; /* dumb pointer */
	scgi_extension *ext;
} handler_ctx;


/* ok, we need a prototype */
static handler_t scgi_handle_fdevent(server *srv, void *ctx, int revents);

int scgi_proclist_sort_down(server *srv, scgi_extension_host *host, scgi_proc *proc);

static void reset_signals(void) {
#ifdef SIGTTOU
	signal(SIGTTOU, SIG_DFL);
#endif
#ifdef SIGTTIN
	signal(SIGTTIN, SIG_DFL);
#endif
#ifdef SIGTSTP
	signal(SIGTSTP, SIG_DFL);
#endif
	signal(SIGHUP, SIG_DFL);
	signal(SIGPIPE, SIG_DFL);
	signal(SIGUSR1, SIG_DFL);
}

static handler_ctx * handler_ctx_init(void) {
	handler_ctx * hctx;

	hctx = calloc(1, sizeof(*hctx));
	force_assert(hctx);

	hctx->fde_ndx = -1;

	hctx->response = buffer_init();
	hctx->response_header = buffer_init();

	hctx->state = FCGI_STATE_INIT;
	hctx->proc = NULL;

	hctx->fd = -1;

	hctx->reconnects = 0;

	hctx->wb = chunkqueue_init();
	hctx->wb_reqlen = 0;

	return hctx;
}

static void handler_ctx_free(handler_ctx *hctx) {
	buffer_free(hctx->response);
	buffer_free(hctx->response_header);

	chunkqueue_free(hctx->wb);

	free(hctx);
}

static scgi_proc *scgi_process_init(void) {
	scgi_proc *f;

	f = calloc(1, sizeof(*f));
	force_assert(f);
	f->socket = buffer_init();

	f->prev = NULL;
	f->next = NULL;

	return f;
}

static void scgi_process_free(scgi_proc *f) {
	if (!f) return;

	scgi_process_free(f->next);

	buffer_free(f->socket);

	free(f);
}

static scgi_extension_host *scgi_host_init(void) {
	scgi_extension_host *f;

	f = calloc(1, sizeof(*f));

	f->host = buffer_init();
	f->unixsocket = buffer_init();
	f->docroot = buffer_init();
	f->bin_path = buffer_init();
	f->bin_env = array_init();
	f->bin_env_copy = array_init();
	f->xsendfile_docroot = array_init();

	return f;
}

static void scgi_host_free(scgi_extension_host *h) {
	if (!h) return;
	if (h->refcount) {
		--h->refcount;
		return;
	}

	buffer_free(h->host);
	buffer_free(h->unixsocket);
	buffer_free(h->docroot);
	buffer_free(h->bin_path);
	array_free(h->bin_env);
	array_free(h->bin_env_copy);
	array_free(h->xsendfile_docroot);

	scgi_process_free(h->first);
	scgi_process_free(h->unused_procs);

	free(h);

}

static scgi_exts *scgi_extensions_init(void) {
	scgi_exts *f;

	f = calloc(1, sizeof(*f));
	force_assert(f);

	return f;
}

static void scgi_extensions_free(scgi_exts *f) {
	size_t i;

	if (!f) return;

	for (i = 0; i < f->used; i++) {
		scgi_extension *fe;
		size_t j;

		fe = f->exts[i];

		for (j = 0; j < fe->used; j++) {
			scgi_extension_host *h;

			h = fe->hosts[j];

			scgi_host_free(h);
		}

		buffer_free(fe->key);
		free(fe->hosts);

		free(fe);
	}

	free(f->exts);

	free(f);
}

static int scgi_extension_insert(scgi_exts *ext, buffer *key, scgi_extension_host *fh) {
	scgi_extension *fe;
	size_t i;

	/* there is something */

	for (i = 0; i < ext->used; i++) {
		if (buffer_is_equal(key, ext->exts[i]->key)) {
			break;
		}
	}

	if (i == ext->used) {
		/* filextension is new */
		fe = calloc(1, sizeof(*fe));
		force_assert(fe);
		fe->key = buffer_init();
		buffer_copy_buffer(fe->key, key);

		/* */

		if (ext->size == 0) {
			ext->size = 8;
			ext->exts = malloc(ext->size * sizeof(*(ext->exts)));
			force_assert(ext->exts);
		} else if (ext->used == ext->size) {
			ext->size += 8;
			ext->exts = realloc(ext->exts, ext->size * sizeof(*(ext->exts)));
			force_assert(ext->exts);
		}
		ext->exts[ext->used++] = fe;
	} else {
		fe = ext->exts[i];
	}

	if (fe->size == 0) {
		fe->size = 4;
		fe->hosts = malloc(fe->size * sizeof(*(fe->hosts)));
		force_assert(fe->hosts);
	} else if (fe->size == fe->used) {
		fe->size += 4;
		fe->hosts = realloc(fe->hosts, fe->size * sizeof(*(fe->hosts)));
		force_assert(fe->hosts);
	}

	fe->hosts[fe->used++] = fh;

	return 0;

}

INIT_FUNC(mod_scgi_init) {
	plugin_data *p;

	p = calloc(1, sizeof(*p));
	force_assert(p);

	p->scgi_env = buffer_init();

	p->parse_response = buffer_init();

	return p;
}


FREE_FUNC(mod_scgi_free) {
	plugin_data *p = p_d;

	UNUSED(srv);

	buffer_free(p->scgi_env);
	buffer_free(p->parse_response);

	if (p->config_storage) {
		size_t i, j, n;
		for (i = 0; i < srv->config_context->used; i++) {
			plugin_config *s = p->config_storage[i];
			scgi_exts *exts;

			if (NULL == s) continue;

			exts = s->exts;

			for (j = 0; j < exts->used; j++) {
				scgi_extension *ex;

				ex = exts->exts[j];

				for (n = 0; n < ex->used; n++) {
					scgi_proc *proc;
					scgi_extension_host *host;

					host = ex->hosts[n];

					for (proc = host->first; proc; proc = proc->next) {
						if (proc->pid != 0) kill(proc->pid, SIGTERM);

						if (proc->is_local &&
						    !buffer_string_is_empty(proc->socket)) {
							unlink(proc->socket->ptr);
						}
					}

					for (proc = host->unused_procs; proc; proc = proc->next) {
						if (proc->pid != 0) kill(proc->pid, SIGTERM);

						if (proc->is_local &&
						    !buffer_string_is_empty(proc->socket)) {
							unlink(proc->socket->ptr);
						}
					}
				}
			}

			scgi_extensions_free(s->exts);

			free(s);
		}
		free(p->config_storage);
	}

	free(p);

	return HANDLER_GO_ON;
}

static int env_add(char_array *env, const char *key, size_t key_len, const char *val, size_t val_len) {
	char *dst;
	size_t i;

	if (!key || !val) return -1;

	dst = malloc(key_len + val_len + 3);
	force_assert(dst);
	memcpy(dst, key, key_len);
	dst[key_len] = '=';
	/* add the \0 from the value */
	memcpy(dst + key_len + 1, val, val_len + 1);

	for (i = 0; i < env->used; i++) {
		if (0 == strncmp(dst, env->ptr[i], key_len + 1)) {
			/* don't care about free as we are in a forked child which is going to exec(...) */
			/* free(env->ptr[i]); */
			env->ptr[i] = dst;
			return 0;
		}
	}

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

#if !defined(HAVE_FORK)
static int scgi_spawn_connection(server *srv,
                                 plugin_data *p,
                                 scgi_extension_host *host,
                                 scgi_proc *proc) {
	UNUSED(srv);
	UNUSED(p);
	UNUSED(host);
	UNUSED(proc);
	return -1;
}

#else /* -> defined(HAVE_FORK) */

static int scgi_spawn_connection(server *srv,
                                 plugin_data *p,
                                 scgi_extension_host *host,
                                 scgi_proc *proc) {
	int scgi_fd;
	int status;
	struct timeval tv = { 0, 100 * 1000 };
#ifdef HAVE_SYS_UN_H
	struct sockaddr_un scgi_addr_un;
#endif
#if defined(HAVE_IPV6) && defined(HAVE_INET_PTON)
	struct sockaddr_in6 scgi_addr_in6;
#endif
	struct sockaddr_in scgi_addr_in;
	struct sockaddr *scgi_addr;

	socklen_t servlen;

	if (p->conf.debug) {
		log_error_write(srv, __FILE__, __LINE__, "sdb",
				"new proc, socket:", proc->port, proc->socket);
	}


	if (!buffer_string_is_empty(proc->socket)) {
#ifdef HAVE_SYS_UN_H
		memset(&scgi_addr_un, 0, sizeof(scgi_addr_un));
		scgi_addr_un.sun_family = AF_UNIX;
		if (buffer_string_length(proc->socket) + 1 > sizeof(scgi_addr_un.sun_path)) {
			log_error_write(srv, __FILE__, __LINE__, "sB",
					"ERROR: Unix Domain socket filename too long:",
					proc->socket);
			return -1;
		}
		memcpy(scgi_addr_un.sun_path, proc->socket->ptr, buffer_string_length(proc->socket) + 1);

#ifdef SUN_LEN
		servlen = SUN_LEN(&scgi_addr_un);
#else
		/* stevens says: */
		servlen = buffer_string_length(proc->socket) + 1 + sizeof(scgi_addr_un.sun_family);
#endif
		scgi_addr = (struct sockaddr *) &scgi_addr_un;
#else
		log_error_write(srv, __FILE__, __LINE__, "s",
				"ERROR: Unix Domain sockets are not supported.");
		return -1;
#endif
#if defined(HAVE_IPV6) && defined(HAVE_INET_PTON)
	} else if (host->family == AF_INET6 && !buffer_string_is_empty(host->host)) {
		memset(&scgi_addr_in6, 0, sizeof(scgi_addr_in6));
		scgi_addr_in6.sin6_family = AF_INET6;
		inet_pton(AF_INET6, host->host->ptr, (char *) &scgi_addr_in6.sin6_addr);
		scgi_addr_in6.sin6_port = htons(proc->port);
		servlen = sizeof(scgi_addr_in6);
		scgi_addr = (struct sockaddr *) &scgi_addr_in6;
#endif
	} else {
		memset(&scgi_addr_in, 0, sizeof(scgi_addr_in));
		scgi_addr_in.sin_family = AF_INET;

		if (buffer_string_is_empty(host->host)) {
			scgi_addr_in.sin_addr.s_addr = htonl(INADDR_ANY);
		} else {
			struct hostent *he;

			/* set a usefull default */
			scgi_addr_in.sin_addr.s_addr = htonl(INADDR_ANY);


			if (NULL == (he = gethostbyname(host->host->ptr))) {
				log_error_write(srv, __FILE__, __LINE__,
						"sdb", "gethostbyname failed: ",
						h_errno, host->host);
				return -1;
			}

			if (he->h_addrtype != AF_INET) {
				log_error_write(srv, __FILE__, __LINE__, "sd", "addr-type != AF_INET: ", he->h_addrtype);
				return -1;
			}

			if (he->h_length != sizeof(struct in_addr)) {
				log_error_write(srv, __FILE__, __LINE__, "sd", "addr-length != sizeof(in_addr): ", he->h_length);
				return -1;
			}

			memcpy(&(scgi_addr_in.sin_addr.s_addr), he->h_addr_list[0], he->h_length);

		}
		scgi_addr_in.sin_port = htons(proc->port);
		servlen = sizeof(scgi_addr_in);

		scgi_addr = (struct sockaddr *) &scgi_addr_in;
	}

	if (-1 == (scgi_fd = fdevent_socket_cloexec(scgi_addr->sa_family, SOCK_STREAM, 0))) {
		log_error_write(srv, __FILE__, __LINE__, "ss",
				"failed:", strerror(errno));
		return -1;
	}

	if (-1 == connect(scgi_fd, scgi_addr, servlen)) {
		/* server is not up, spawn in  */
		pid_t child;
		int val;

		if (!buffer_string_is_empty(proc->socket)) {
			unlink(proc->socket->ptr);
		}

		close(scgi_fd);

		/* reopen socket */
		if (-1 == (scgi_fd = fdevent_socket_cloexec(scgi_addr->sa_family, SOCK_STREAM, 0))) {
			log_error_write(srv, __FILE__, __LINE__, "ss",
				"socket failed:", strerror(errno));
			return -1;
		}

		val = 1;
		if (setsockopt(scgi_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
			log_error_write(srv, __FILE__, __LINE__, "ss",
					"socketsockopt failed:", strerror(errno));
			close(scgi_fd);
			return -1;
		}

		/* create socket */
		if (-1 == bind(scgi_fd, scgi_addr, servlen)) {
			log_error_write(srv, __FILE__, __LINE__, "sbds",
				"bind failed for:",
				proc->socket,
				proc->port,
				strerror(errno));
			close(scgi_fd);
			return -1;
		}

		if (-1 == listen(scgi_fd, host->listen_backlog)) {
			log_error_write(srv, __FILE__, __LINE__, "ss",
				"listen failed:", strerror(errno));
			close(scgi_fd);
			return -1;
		}

		switch ((child = fork())) {
		case 0: {
			buffer *b;
			size_t i = 0;
			int fd = 0;
			char_array env;


			/* create environment */
			env.ptr = NULL;
			env.size = 0;
			env.used = 0;

			if (scgi_fd != 0) {
				dup2(scgi_fd, 0);
				close(scgi_fd);
			}
		      #ifdef SOCK_CLOEXEC
			else
				(void)fcntl(scgi_fd, F_SETFD, 0); /* clear cloexec */
		      #endif

			/* we don't need the client socket */
			for (fd = 3; fd < 256; fd++) {
				close(fd);
			}

			/* build clean environment */
			if (host->bin_env_copy->used) {
				for (i = 0; i < host->bin_env_copy->used; i++) {
					data_string *ds = (data_string *)host->bin_env_copy->data[i];
					char *ge;

					if (NULL != (ge = getenv(ds->value->ptr))) {
						env_add(&env, CONST_BUF_LEN(ds->value), ge, strlen(ge));
					}
				}
			} else {
				char ** const e = environ;
				for (i = 0; e[i]; ++i) {
					char *eq;

					if (NULL != (eq = strchr(e[i], '='))) {
						env_add(&env, e[i], eq - e[i], eq+1, strlen(eq+1));
					}
				}
			}

			/* create environment */
			for (i = 0; i < host->bin_env->used; i++) {
				data_string *ds = (data_string *)host->bin_env->data[i];

				env_add(&env, CONST_BUF_LEN(ds->key), CONST_BUF_LEN(ds->value));
			}

			for (i = 0; i < env.used; i++) {
				/* search for PHP_FCGI_CHILDREN */
				if (0 == strncmp(env.ptr[i], "PHP_FCGI_CHILDREN=", sizeof("PHP_FCGI_CHILDREN=") - 1)) break;
			}

			/* not found, add a default */
			if (i == env.used) {
				env_add(&env, CONST_STR_LEN("PHP_FCGI_CHILDREN"), CONST_STR_LEN("1"));
			}

			env.ptr[env.used] = NULL;

			b = buffer_init();
			buffer_copy_string_len(b, CONST_STR_LEN("exec "));
			buffer_append_string_buffer(b, host->bin_path);

			reset_signals();

			/* exec the cgi */
			execle("/bin/sh", "sh", "-c", b->ptr, (char *)NULL, env.ptr);

			log_error_write(srv, __FILE__, __LINE__, "sbs",
					"execl failed for:", host->bin_path, strerror(errno));

			_exit(errno);

			break;
		}
		case -1:
			/* error */
			close(scgi_fd);
			break;
		default:
			/* father */
			close(scgi_fd);

			/* wait */
			select(0, NULL, NULL, NULL, &tv);

			switch (waitpid(child, &status, WNOHANG)) {
			case 0:
				/* child still running after timeout, good */
				break;
			case -1:
				/* no PID found ? should never happen */
				log_error_write(srv, __FILE__, __LINE__, "ss",
						"pid not found:", strerror(errno));
				return -1;
			default:
				/* the child should not terminate at all */
				if (WIFEXITED(status)) {
					log_error_write(srv, __FILE__, __LINE__, "sd",
							"child exited (is this a SCGI binary ?):",
							WEXITSTATUS(status));
				} else if (WIFSIGNALED(status)) {
					log_error_write(srv, __FILE__, __LINE__, "sd",
							"child signaled:",
							WTERMSIG(status));
				} else {
					log_error_write(srv, __FILE__, __LINE__, "sd",
							"child died somehow:",
							status);
				}
				return -1;
			}

			/* register process */
			proc->pid = child;
			proc->last_used = srv->cur_ts;
			proc->is_local = 1;

			break;
		}
	} else {
		close(scgi_fd);

		proc->is_local = 0;
		proc->pid = 0;

		if (p->conf.debug) {
			log_error_write(srv, __FILE__, __LINE__, "sb",
					"(debug) socket is already used, won't spawn:",
					proc->socket);
		}
	}

	proc->state = PROC_STATE_RUNNING;
	host->active_procs++;

	return 0;
}

#endif /* HAVE_FORK */

static scgi_extension_host * unixsocket_is_dup(plugin_data *p, size_t used, buffer *unixsocket) {
	size_t i, j, n;
	for (i = 0; i < used; ++i) {
		scgi_exts *exts = p->config_storage[i]->exts;
		for (j = 0; j < exts->used; ++j) {
			scgi_extension *ex = exts->exts[j];
			for (n = 0; n < ex->used; ++n) {
				scgi_extension_host *host = ex->hosts[n];
				if (!buffer_string_is_empty(host->unixsocket)
				    && buffer_is_equal(host->unixsocket, unixsocket)
				    && !buffer_string_is_empty(host->bin_path))
					return host;
			}
		}
	}

	return NULL;
}

SETDEFAULTS_FUNC(mod_scgi_set_defaults) {
	plugin_data *p = p_d;
	data_unset *du;
	size_t i = 0;
	scgi_extension_host *df = NULL;

	config_values_t cv[] = {
		{ "scgi.server",              NULL, T_CONFIG_LOCAL, T_CONFIG_SCOPE_CONNECTION },       /* 0 */
		{ "scgi.debug",               NULL, T_CONFIG_SHORT, T_CONFIG_SCOPE_CONNECTION },       /* 1 */
		{ "scgi.protocol",            NULL, T_CONFIG_LOCAL, T_CONFIG_SCOPE_CONNECTION },       /* 2 */
		{ NULL,                          NULL, T_CONFIG_UNSET, T_CONFIG_SCOPE_UNSET }
	};

	p->config_storage = calloc(1, srv->config_context->used * sizeof(plugin_config *));
	force_assert(p->config_storage);

	for (i = 0; i < srv->config_context->used; i++) {
		data_config const* config = (data_config const*)srv->config_context->data[i];
		plugin_config *s;

		s = malloc(sizeof(plugin_config));
		force_assert(s);
		s->exts          = scgi_extensions_init();
		s->debug         = 0;
		s->proto         = LI_PROTOCOL_SCGI;

		cv[0].destination = s->exts;
		cv[1].destination = &(s->debug);
		cv[2].destination = NULL; /* T_CONFIG_LOCAL */

		p->config_storage[i] = s;

		if (0 != config_insert_values_global(srv, config->value, cv, i == 0 ? T_CONFIG_SCOPE_SERVER : T_CONFIG_SCOPE_CONNECTION)) {
			goto error;
		}

		/*
		 * <key> = ( ... )
		 */

		if (NULL != (du = array_get_element(config->value, "scgi.protocol"))) {
			data_string *ds = (data_string *)du;
			if (du->type == TYPE_STRING
			    && buffer_is_equal_string(ds->value, CONST_STR_LEN("scgi"))) {
				s->proto = LI_PROTOCOL_SCGI;
			} else if (du->type == TYPE_STRING
			           && buffer_is_equal_string(ds->value, CONST_STR_LEN("uwsgi"))) {
				s->proto = LI_PROTOCOL_UWSGI;
			} else {
				log_error_write(srv, __FILE__, __LINE__, "sss",
						"unexpected type for key: ", "scgi.protocol", "expected \"scgi\" or \"uwsgi\"");

				goto error;
			}
		}

		if (NULL != (du = array_get_element(config->value, "scgi.server"))) {
			size_t j;
			data_array *da = (data_array *)du;

			if (du->type != TYPE_ARRAY) {
				log_error_write(srv, __FILE__, __LINE__, "sss",
						"unexpected type for key: ", "scgi.server", "expected ( \"ext\" => ( \"backend-label\" => ( \"key\" => \"value\" )))");

				goto error;
			}


			/*
			 * scgi.server = ( "<ext>" => ( ... ),
			 *                    "<ext>" => ( ... ) )
			 */

			for (j = 0; j < da->value->used; j++) {
				size_t n;
				data_array *da_ext = (data_array *)da->value->data[j];

				if (da->value->data[j]->type != TYPE_ARRAY) {
					log_error_write(srv, __FILE__, __LINE__, "sssbs",
							"unexpected type for key: ", "scgi.server",
							"[", da->value->data[j]->key, "](string); expected ( \"ext\" => ( \"backend-label\" => ( \"key\" => \"value\" )))");

					goto error;
				}

				/*
				 * da_ext->key == name of the extension
				 */

				/*
				 * scgi.server = ( "<ext>" =>
				 *                     ( "<host>" => ( ... ),
				 *                       "<host>" => ( ... )
				 *                     ),
				 *                    "<ext>" => ... )
				 */

				for (n = 0; n < da_ext->value->used; n++) {
					data_array *da_host = (data_array *)da_ext->value->data[n];

					config_values_t fcv[] = {
						{ "host",              NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION },       /* 0 */
						{ "docroot",           NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION },       /* 1 */
						{ "socket",            NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION },       /* 2 */
						{ "bin-path",          NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION },       /* 3 */

						{ "check-local",       NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION },      /* 4 */
						{ "port",              NULL, T_CONFIG_SHORT, T_CONFIG_SCOPE_CONNECTION },        /* 5 */
						{ "min-procs-not-working",         NULL, T_CONFIG_SHORT, T_CONFIG_SCOPE_CONNECTION },        /* 7 this is broken for now */
						{ "max-procs",         NULL, T_CONFIG_SHORT, T_CONFIG_SCOPE_CONNECTION },        /* 7 */
						{ "max-load-per-proc", NULL, T_CONFIG_SHORT, T_CONFIG_SCOPE_CONNECTION },        /* 8 */
						{ "idle-timeout",      NULL, T_CONFIG_SHORT, T_CONFIG_SCOPE_CONNECTION },        /* 9 */
						{ "disable-time",      NULL, T_CONFIG_SHORT, T_CONFIG_SCOPE_CONNECTION },        /* 10 */

						{ "bin-environment",   NULL, T_CONFIG_ARRAY, T_CONFIG_SCOPE_CONNECTION },        /* 11 */
						{ "bin-copy-environment", NULL, T_CONFIG_ARRAY, T_CONFIG_SCOPE_CONNECTION },     /* 12 */
						{ "fix-root-scriptname",  NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION },   /* 13 */
						{ "listen-backlog",    NULL, T_CONFIG_INT,   T_CONFIG_SCOPE_CONNECTION },        /* 14 */
						{ "x-sendfile",        NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION },      /* 15 */
						{ "x-sendfile-docroot",NULL, T_CONFIG_ARRAY,   T_CONFIG_SCOPE_CONNECTION },      /* 16 */


						{ NULL,                NULL, T_CONFIG_UNSET, T_CONFIG_SCOPE_UNSET }
					};

					if (da_host->type != TYPE_ARRAY) {
						log_error_write(srv, __FILE__, __LINE__, "ssSBS",
								"unexpected type for key:",
								"scgi.server",
								"[", da_host->key, "](string); expected ( \"ext\" => ( \"backend-label\" => ( \"key\" => \"value\" )))");

						goto error;
					}

					df = scgi_host_init();

					df->check_local  = 1;
					df->min_procs    = 4;
					df->max_procs    = 4;
					df->max_load_per_proc = 1;
					df->idle_timeout = 60;
					df->disable_time = 60;
					df->fix_root_path_name = 0;
					df->listen_backlog = 1024;
					df->xsendfile_allow = 0;
					df->refcount = 0;

					fcv[0].destination = df->host;
					fcv[1].destination = df->docroot;
					fcv[2].destination = df->unixsocket;
					fcv[3].destination = df->bin_path;

					fcv[4].destination = &(df->check_local);
					fcv[5].destination = &(df->port);
					fcv[6].destination = &(df->min_procs);
					fcv[7].destination = &(df->max_procs);
					fcv[8].destination = &(df->max_load_per_proc);
					fcv[9].destination = &(df->idle_timeout);
					fcv[10].destination = &(df->disable_time);

					fcv[11].destination = df->bin_env;
					fcv[12].destination = df->bin_env_copy;
					fcv[13].destination = &(df->fix_root_path_name);
					fcv[14].destination = &(df->listen_backlog);
					fcv[15].destination = &(df->xsendfile_allow);
					fcv[16].destination = df->xsendfile_docroot;


					if (0 != config_insert_values_internal(srv, da_host->value, fcv, T_CONFIG_SCOPE_CONNECTION)) {
						goto error;
					}

					if ((!buffer_string_is_empty(df->host) || df->port) &&
					    !buffer_string_is_empty(df->unixsocket)) {
						log_error_write(srv, __FILE__, __LINE__, "s",
								"either host+port or socket");

						goto error;
					}

					if (!buffer_string_is_empty(df->unixsocket)) {
						/* unix domain socket */
						struct sockaddr_un un;

						if (buffer_string_length(df->unixsocket) + 1 > sizeof(un.sun_path) - 2) {
							log_error_write(srv, __FILE__, __LINE__, "s",
									"path of the unixdomain socket is too large");
							goto error;
						}

						if (!buffer_string_is_empty(df->bin_path)) {
							scgi_extension_host *duplicate = unixsocket_is_dup(p, i+1, df->unixsocket);
							if (NULL != duplicate) {
								if (!buffer_is_equal(df->bin_path, duplicate->bin_path)) {
									log_error_write(srv, __FILE__, __LINE__, "sb",
										"duplicate unixsocket path:",
										df->unixsocket);
									goto error;
								}
								scgi_host_free(df);
								df = duplicate;
								++df->refcount;
							}
						}

						df->family = AF_UNIX;
					} else {
						/* tcp/ip */

						if (buffer_string_is_empty(df->host) &&
						    buffer_string_is_empty(df->bin_path)) {
							log_error_write(srv, __FILE__, __LINE__, "sbbbs",
									"missing key (string):",
									da->key,
									da_ext->key,
									da_host->key,
									"host");

							goto error;
						} else if (df->port == 0) {
							log_error_write(srv, __FILE__, __LINE__, "sbbbs",
									"missing key (short):",
									da->key,
									da_ext->key,
									da_host->key,
									"port");
							goto error;
						}

						df->family = (!buffer_string_is_empty(df->host) && NULL != strchr(df->host->ptr, ':')) ? AF_INET6 : AF_INET;
					}

					if (df->refcount) {
						/* already init'd; skip spawning */
					} else if (!buffer_string_is_empty(df->bin_path)) {
						/* a local socket + self spawning */
						size_t pno;

						struct stat st;
						size_t nchars = strcspn(df->bin_path->ptr, " \t");
						char c = df->bin_path->ptr[nchars];
						df->bin_path->ptr[nchars] = '\0';
						if (0 == nchars || 0 != stat(df->bin_path->ptr, &st) || !S_ISREG(st.st_mode) || !(st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))) {
							df->bin_path->ptr[nchars] = c;
							log_error_write(srv, __FILE__, __LINE__, "SSs",
									"invalid \"bin-path\" => \"", df->bin_path->ptr,
									"\" (check that file exists, is regular file, and is executable by lighttpd)");
						}
						df->bin_path->ptr[nchars] = c;

						/* HACK:  just to make sure the adaptive spawing is disabled */
						df->min_procs = df->max_procs;

						if (df->min_procs > df->max_procs) df->max_procs = df->min_procs;
						if (df->max_load_per_proc < 1) df->max_load_per_proc = 0;

						if (s->debug) {
							log_error_write(srv, __FILE__, __LINE__, "ssbsdsbsdsd",
									"--- scgi spawning local",
									"\n\tproc:", df->bin_path,
									"\n\tport:", df->port,
									"\n\tsocket", df->unixsocket,
									"\n\tmin-procs:", df->min_procs,
									"\n\tmax-procs:", df->max_procs);
						}

						for (pno = 0; pno < df->min_procs; pno++) {
							scgi_proc *proc;

							proc = scgi_process_init();
							proc->id = df->num_procs++;
							df->max_id++;

							if (buffer_string_is_empty(df->unixsocket)) {
								proc->port = df->port + pno;
							} else {
								buffer_copy_buffer(proc->socket, df->unixsocket);
								buffer_append_string_len(proc->socket, CONST_STR_LEN("-"));
								buffer_append_int(proc->socket, pno);
							}

							if (s->debug) {
								log_error_write(srv, __FILE__, __LINE__, "ssdsbsdsd",
										"--- scgi spawning",
										"\n\tport:", df->port,
										"\n\tsocket", df->unixsocket,
										"\n\tcurrent:", pno, "/", df->min_procs);
							}

							if (!srv->srvconf.preflight_check
							    && scgi_spawn_connection(srv, p, df, proc)) {
								log_error_write(srv, __FILE__, __LINE__, "s",
										"[ERROR]: spawning fcgi failed.");
								scgi_process_free(proc);
								goto error;
							}

							proc->next = df->first;
							if (df->first) 	df->first->prev = proc;

							df->first = proc;
						}
					} else {
						scgi_proc *fp;

						fp = scgi_process_init();
						fp->id = df->num_procs++;
						df->max_id++;
						df->active_procs++;
						fp->state = PROC_STATE_RUNNING;

						if (buffer_string_is_empty(df->unixsocket)) {
							fp->port = df->port;
						} else {
							buffer_copy_buffer(fp->socket, df->unixsocket);
						}

						df->first = fp;

						df->min_procs = 1;
						df->max_procs = 1;
					}

					if (df->xsendfile_docroot->used) {
						size_t k;
						for (k = 0; k < df->xsendfile_docroot->used; ++k) {
							data_string *ds = (data_string *)df->xsendfile_docroot->data[k];
							if (ds->type != TYPE_STRING) {
								log_error_write(srv, __FILE__, __LINE__, "s",
									"unexpected type for x-sendfile-docroot; expected: \"x-sendfile-docroot\" => ( \"/allowed/path\", ... )");
								goto error;
							}
							if (ds->value->ptr[0] != '/') {
								log_error_write(srv, __FILE__, __LINE__, "SBs",
									"x-sendfile-docroot paths must begin with '/'; invalid: \"", ds->value, "\"");
								goto error;
							}
							buffer_path_simplify(ds->value, ds->value);
							buffer_append_slash(ds->value);
						}
					}

					/* if extension already exists, take it */
					scgi_extension_insert(s->exts, da_ext->key, df);
					df = NULL;
				}
			}
		}
	}

	return HANDLER_GO_ON;

error:
	if (NULL != df) scgi_host_free(df);
	return HANDLER_ERROR;
}

static int scgi_set_state(server *srv, handler_ctx *hctx, scgi_connection_state_t state) {
	hctx->state = state;
	hctx->state_timestamp = srv->cur_ts;

	return 0;
}


static void scgi_backend_close(server *srv, handler_ctx *hctx) {
	if (hctx->fd != -1) {
		fdevent_event_del(srv->ev, &(hctx->fde_ndx), hctx->fd);
		fdevent_unregister(srv->ev, hctx->fd);
		fdevent_sched_close(srv->ev, hctx->fd, 1);
		hctx->fd = -1;
		hctx->fde_ndx = -1;
	}

	if (hctx->host) {
		if (hctx->proc) {
			/* after the connect the process gets a load */
			if (hctx->got_proc) hctx->proc->load--;
			scgi_proclist_sort_down(srv, hctx->host, hctx->proc);

			if (hctx->conf.debug) {
				log_error_write(srv, __FILE__, __LINE__, "sddb",
						"release proc:",
						hctx->fd,
						hctx->proc->pid, hctx->proc->socket);
			}
		}

		hctx->host->load--;
		hctx->host = NULL;
	}
}

static scgi_extension_host * scgi_extension_host_get(server *srv, connection *con, plugin_data *p, scgi_extension *extension) {
	int used = -1;
	scgi_extension_host *host = NULL;
	UNUSED(p);

	/* get best server */
	for (size_t k = 0; k < extension->used; ++k) {
		scgi_extension_host *h = extension->hosts[k];

		/* we should have at least one proc that can do something */
		if (h->active_procs == 0) {
			continue;
		}

		if (used == -1 || h->load < used) {
			used = h->load;

			host = h;
		}
	}

	if (!host) {
		/* sorry, we don't have a server alive for this ext */
		con->http_status = 503; /* Service Unavailable */
		con->mode = DIRECT;

		/* only send the 'no handler' once */
		if (!extension->note_is_sent) {
			extension->note_is_sent = 1;

			log_error_write(srv, __FILE__, __LINE__, "sbsbs",
					"all handlers for ", con->uri.path,
					"on", extension->key,
					"are down.");
		}
	}

	return host;
}

static void scgi_connection_close(server *srv, handler_ctx *hctx) {
	plugin_data *p;
	connection  *con;

	p    = hctx->plugin_data;
	con  = hctx->remote_conn;

	scgi_backend_close(srv, hctx);
	handler_ctx_free(hctx);
	con->plugin_ctx[p->id] = NULL;

	/* finish response (if not already con->file_started, con->file_finished) */
	if (con->mode == p->id) {
		http_response_backend_done(srv, con);
	}
}

static handler_t scgi_reconnect(server *srv, handler_ctx *hctx) {
	scgi_backend_close(srv, hctx);

	hctx->host = scgi_extension_host_get(srv, hctx->remote_conn, hctx->plugin_data, hctx->ext);
	if (NULL == hctx->host) return HANDLER_FINISHED;

	hctx->host->load++;
	scgi_set_state(srv, hctx, FCGI_STATE_INIT);
	return HANDLER_COMEBACK;
}


static handler_t scgi_connection_reset(server *srv, connection *con, void *p_d) {
	plugin_data *p = p_d;
	handler_ctx *hctx = con->plugin_ctx[p->id];
	if (hctx) scgi_connection_close(srv, hctx);

	return HANDLER_GO_ON;
}


static int scgi_env_add_scgi(void *venv, const char *key, size_t key_len, const char *val, size_t val_len) {
	buffer *env = venv;
	size_t len;

	if (!key || !val) return -1;

	len = key_len + val_len + 2;

	buffer_string_prepare_append(env, len);

	buffer_append_string_len(env, key, key_len);
	buffer_append_string_len(env, "", 1);
	buffer_append_string_len(env, val, val_len);
	buffer_append_string_len(env, "", 1);

	return 0;
}


#ifdef __LITTLE_ENDIAN__
#define uwsgi_htole16(x) (x)
#else /* __BIG_ENDIAN__ */
#define uwsgi_htole16(x) ((uint16_t) (((x) & 0xff) << 8 | ((x) & 0xff00) >> 8))
#endif


static int scgi_env_add_uwsgi(void *venv, const char *key, size_t key_len, const char *val, size_t val_len) {
	buffer *env = venv;
	size_t len;
	uint16_t uwlen;

	if (!key || !val) return -1;
	if (key_len > USHRT_MAX || val_len > USHRT_MAX) return -1;

	len = 2 + key_len + 2 + val_len;

	buffer_string_prepare_append(env, len);

	uwlen = uwsgi_htole16((uint16_t)key_len);
	buffer_append_string_len(env, (char *)&uwlen, 2);
	buffer_append_string_len(env, key, key_len);
	uwlen = uwsgi_htole16((uint16_t)val_len);
	buffer_append_string_len(env, (char *)&uwlen, 2);
	buffer_append_string_len(env, val, val_len);

	return 0;
}


/**
 *
 * returns
 *   -1 error
 *    0 connected
 *    1 not connected yet
 */

static int scgi_establish_connection(server *srv, handler_ctx *hctx) {
	struct sockaddr *scgi_addr;
	struct sockaddr_in scgi_addr_in;
#if defined(HAVE_IPV6) && defined(HAVE_INET_PTON)
	struct sockaddr_in6 scgi_addr_in6;
#endif
#ifdef HAVE_SYS_UN_H
	struct sockaddr_un scgi_addr_un;
#endif
	socklen_t servlen;

	scgi_extension_host *host = hctx->host;
	scgi_proc *proc   = hctx->proc;
	int scgi_fd       = hctx->fd;

	if (!buffer_string_is_empty(proc->socket)) {
#ifdef HAVE_SYS_UN_H
		/* use the unix domain socket */
		memset(&scgi_addr_un, 0, sizeof(scgi_addr_un));
		scgi_addr_un.sun_family = AF_UNIX;
		if (buffer_string_length(proc->socket) + 1 > sizeof(scgi_addr_un.sun_path)) {
			log_error_write(srv, __FILE__, __LINE__, "sB",
					"ERROR: Unix Domain socket filename too long:",
					proc->socket);
			return -1;
		}
		memcpy(scgi_addr_un.sun_path, proc->socket->ptr, buffer_string_length(proc->socket) + 1);

#ifdef SUN_LEN
		servlen = SUN_LEN(&scgi_addr_un);
#else
		/* stevens says: */
		servlen = buffer_string_length(proc->socket) + 1 + sizeof(scgi_addr_un.sun_family);
#endif
		scgi_addr = (struct sockaddr *) &scgi_addr_un;
#else
		return -1;
#endif
#if defined(HAVE_IPV6) && defined(HAVE_INET_PTON)
	} else if (host->family == AF_INET6 && !buffer_string_is_empty(host->host)) {
		memset(&scgi_addr_in6, 0, sizeof(scgi_addr_in6));
		scgi_addr_in6.sin6_family = AF_INET6;
		inet_pton(AF_INET6, host->host->ptr, (char *) &scgi_addr_in6.sin6_addr);
		scgi_addr_in6.sin6_port = htons(proc->port);
		servlen = sizeof(scgi_addr_in6);
		scgi_addr = (struct sockaddr *) &scgi_addr_in6;
#endif
	} else {
		memset(&scgi_addr_in, 0, sizeof(scgi_addr_in));
		scgi_addr_in.sin_family = AF_INET;
		if (0 == inet_aton(host->host->ptr, &(scgi_addr_in.sin_addr))) {
			log_error_write(srv, __FILE__, __LINE__, "sbs",
					"converting IP-adress failed for", host->host,
					"\nBe sure to specify an IP address here");

			return -1;
		}
		scgi_addr_in.sin_port = htons(proc->port);
		servlen = sizeof(scgi_addr_in);

		scgi_addr = (struct sockaddr *) &scgi_addr_in;
	}

	if (-1 == connect(scgi_fd, scgi_addr, servlen)) {
		if (errno == EINPROGRESS ||
		    errno == EALREADY ||
		    errno == EINTR) {
			if (hctx->conf.debug) {
				log_error_write(srv, __FILE__, __LINE__, "sd",
						"connect delayed, will continue later:", scgi_fd);
			}

			return 1;
		} else {
			log_error_write(srv, __FILE__, __LINE__, "sdsddb",
					"connect failed:", scgi_fd,
					strerror(errno), errno,
					proc->port, proc->socket);

			if (errno == EAGAIN) {
				/* this is Linux only */

				log_error_write(srv, __FILE__, __LINE__, "s",
						"If this happend on Linux: You have been run out of local ports. "
						"Check the manual, section Performance how to handle this.");
			}

			return -1;
		}
	}
	if (hctx->conf.debug > 1) {
		log_error_write(srv, __FILE__, __LINE__, "sd",
				"connect succeeded: ", scgi_fd);
	}



	return 0;
}


static int scgi_create_env(server *srv, handler_ctx *hctx) {
	buffer *b;

	plugin_data *p    = hctx->plugin_data;
	scgi_extension_host *host= hctx->host;

	connection *con   = hctx->remote_conn;

	http_cgi_opts opts = { 0, 0, host->docroot, NULL };

	http_cgi_header_append_cb scgi_env_add = hctx->conf.proto == LI_PROTOCOL_SCGI
	  ? scgi_env_add_scgi
	  : scgi_env_add_uwsgi;

	buffer_string_prepare_copy(p->scgi_env, 1023);

	if (0 != http_cgi_headers(srv, con, &opts, scgi_env_add, p->scgi_env)) {
		con->http_status = 400;
		return -1;
	}

	if (hctx->conf.proto == LI_PROTOCOL_SCGI) {
		scgi_env_add(p->scgi_env, CONST_STR_LEN("SCGI"), CONST_STR_LEN("1"));
		b = buffer_init();
		buffer_append_int(b, buffer_string_length(p->scgi_env));
		buffer_append_string_len(b, CONST_STR_LEN(":"));
		buffer_append_string_buffer(b, p->scgi_env);
		buffer_append_string_len(b, CONST_STR_LEN(","));
	} else { /* LI_PROTOCOL_UWSGI */
		/* http://uwsgi-docs.readthedocs.io/en/latest/Protocol.html */
		size_t len = buffer_string_length(p->scgi_env);
		uint32_t uwsgi_header;
		if (len > USHRT_MAX) {
			con->http_status = 431; /* Request Header Fields Too Large */
			con->mode = DIRECT;
			return -1; /* trigger return of HANDLER_FINISHED */
		}
		b = buffer_init();
		buffer_string_prepare_copy(b, 4 + len);
		uwsgi_header = ((uint32_t)uwsgi_htole16((uint16_t)len)) << 8;
		memcpy(b->ptr, (char *)&uwsgi_header, 4);
		buffer_commit(b, 4);
		buffer_append_string_buffer(b, p->scgi_env);
	}

	hctx->wb_reqlen = buffer_string_length(b);
	chunkqueue_append_buffer(hctx->wb, b);
	buffer_free(b);

	if (con->request.content_length) {
		chunkqueue_append_chunkqueue(hctx->wb, con->request_content_queue);
		hctx->wb_reqlen += con->request.content_length;/* (eventual) total request size */
	}

	return 0;
}

static int scgi_response_parse(server *srv, connection *con, plugin_data *p, buffer *in, int eol) {
	char *ns;
	const char *s;
	int line = 0;

	UNUSED(srv);

	buffer_copy_buffer(p->parse_response, in);

	for (s = p->parse_response->ptr;
	     NULL != (ns = (eol == EOL_RN ? strstr(s, "\r\n") : strchr(s, '\n')));
	     s = ns + (eol == EOL_RN ? 2 : 1), line++) {
		const char *key, *value;
		int key_len;
		data_string *ds;

		ns[0] = '\0';

		if (line == 0 &&
		    0 == strncmp(s, "HTTP/1.", 7)) {
			/* non-parsed header ... we parse them anyway */

			if ((s[7] == '1' ||
			     s[7] == '0') &&
			    s[8] == ' ') {
				int status;
				/* after the space should be a status code for us */

				status = strtol(s+9, NULL, 10);

				if (status >= 100 && status < 1000) {
					/* we expected 3 digits got them */
					con->parsed_response |= HTTP_STATUS;
					con->http_status = status;
				}
			}
		} else {

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


static int scgi_demux_response(server *srv, handler_ctx *hctx) {
	plugin_data *p    = hctx->plugin_data;
	connection  *con  = hctx->remote_conn;

	while(1) {
		int n;

		buffer_string_prepare_copy(hctx->response, 1023);
		if (-1 == (n = read(hctx->fd, hctx->response->ptr, hctx->response->size - 1))) {
			if (errno == EAGAIN || errno == EINTR) {
				/* would block, wait for signal */
				fdevent_event_add(srv->ev, &(hctx->fde_ndx), hctx->fd, FDEVENT_IN);
				return 0;
			}
			/* error */
			log_error_write(srv, __FILE__, __LINE__, "sdd", strerror(errno), con->fd, hctx->fd);
			return -1;
		}

		if (n == 0) {
			/* read finished */
			return 1;
		}

		buffer_commit(hctx->response, n);

		/* split header from body */

		if (con->file_started == 0) {
			char *c;
			int in_header = 0;
			int header_end = 0;
			int cp, eol = EOL_UNSET;
			size_t used = 0;
			size_t hlen = 0;

			buffer_append_string_buffer(hctx->response_header, hctx->response);

			/* nph (non-parsed headers) */
			if (0 == strncmp(hctx->response_header->ptr, "HTTP/1.", 7)) in_header = 1;

			/* search for the \r\n\r\n or \n\n in the string */
			for (c = hctx->response_header->ptr, cp = 0, used = buffer_string_length(hctx->response_header); used; c++, cp++, used--) {
				if (*c == ':') in_header = 1;
				else if (*c == '\n') {
					if (in_header == 0) {
						/* got a response without a response header */

						c = NULL;
						header_end = 1;
						break;
					}

					if (eol == EOL_UNSET) eol = EOL_N;

					if (*(c+1) == '\n') {
						header_end = 1;
						hlen = cp + 2;
						break;
					}

				} else if (used > 1 && *c == '\r' && *(c+1) == '\n') {
					if (in_header == 0) {
						/* got a response without a response header */

						c = NULL;
						header_end = 1;
						break;
					}

					if (eol == EOL_UNSET) eol = EOL_RN;

					if (used > 3 &&
					    *(c+2) == '\r' &&
					    *(c+3) == '\n') {
						header_end = 1;
						hlen = cp + 4;
						break;
					}

					/* skip the \n */
					c++;
					cp++;
					used--;
				}
			}

			if (header_end) {
				if (c == NULL) {
					/* no header, but a body */
					if (0 != http_chunk_append_buffer(srv, con, hctx->response_header)) {
						/* error writing to tempfile;
						 * truncate response or send 500 if nothing sent yet */
						return 1;
					}
				} else {
					size_t blen = buffer_string_length(hctx->response_header) - hlen;

					/* a small hack: terminate after at the second \r */
					buffer_string_set_length(hctx->response_header, hlen - 1);

					/* parse the response header */
					scgi_response_parse(srv, con, p, hctx->response_header, eol);

					if (hctx->host->xsendfile_allow) {
						data_string *ds;
						if (NULL != (ds = (data_string *) array_get_element(con->response.headers, "X-Sendfile"))) {
							http_response_xsendfile(srv, con, ds->value, hctx->host->xsendfile_docroot);
							return 1;
						}
					}

					if (blen > 0) {
						if (0 != http_chunk_append_mem(srv, con, hctx->response_header->ptr + hlen, blen)) {
							/* error writing to tempfile;
							 * truncate response or send 500 if nothing sent yet */
							return 1;
						}
					}
				}

				con->file_started = 1;
			} else {
				/*(reuse MAX_HTTP_REQUEST_HEADER as max size for response headers from backends)*/
				if (buffer_string_length(hctx->response_header) > MAX_HTTP_REQUEST_HEADER) {
					log_error_write(srv, __FILE__, __LINE__, "sb", "response headers too large for", con->uri.path);
					con->http_status = 502; /* Bad Gateway */
					con->mode = DIRECT;
					return 1;
				}
			}
		} else {
			if (0 != http_chunk_append_buffer(srv, con, hctx->response)) {
				/* error writing to tempfile;
				 * truncate response or send 500 if nothing sent yet */
				return 1;
			}
			if ((con->conf.stream_response_body & FDEVENT_STREAM_RESPONSE_BUFMIN)
			    && chunkqueue_length(con->write_queue) > 65536 - 4096) {
				if (!con->is_writable) {
					/*(defer removal of FDEVENT_IN interest since
					 * connection_state_machine() might be able to send data
					 * immediately, unless !con->is_writable, where
					 * connection_state_machine() might not loop back to call
					 * mod_scgi_handle_subrequest())*/
					fdevent_event_clr(srv->ev, &(hctx->fde_ndx), hctx->fd, FDEVENT_IN);
				}
				break;
			}
		}

#if 0
		log_error_write(srv, __FILE__, __LINE__, "ddss", con->fd, hctx->fd, connection_get_state(con->state), b->ptr);
#endif
	}

	return 0;
}


static int scgi_proclist_sort_up(server *srv, scgi_extension_host *host, scgi_proc *proc) {
	scgi_proc *p;

	UNUSED(srv);

	/* we have been the smallest of the current list
	 * and we want to insert the node sorted as soon
	 * possible
	 *
	 * 1 0 0 0 1 1 1
	 * |      ^
	 * |      |
	 * +------+
	 *
	 */

	/* nothing to sort, only one element */
	if (host->first == proc && proc->next == NULL) return 0;

	for (p = proc; p->next && p->next->load < proc->load; p = p->next);

	/* no need to move something
	 *
	 * 1 2 2 2 3 3 3
	 * ^
	 * |
	 * +
	 *
	 */
	if (p == proc) return 0;

	if (host->first == proc) {
		/* we have been the first elememt */

		host->first = proc->next;
		host->first->prev = NULL;
	}

	/* disconnect proc */

	if (proc->prev) proc->prev->next = proc->next;
	if (proc->next) proc->next->prev = proc->prev;

	/* proc should be right of p */

	proc->next = p->next;
	proc->prev = p;
	if (p->next) p->next->prev = proc;
	p->next = proc;
#if 0
	for(p = host->first; p; p = p->next) {
		log_error_write(srv, __FILE__, __LINE__, "dd",
				p->pid, p->load);
	}
#else
	UNUSED(srv);
#endif

	return 0;
}

int scgi_proclist_sort_down(server *srv, scgi_extension_host *host, scgi_proc *proc) {
	scgi_proc *p;

	UNUSED(srv);

	/* we have been the smallest of the current list
	 * and we want to insert the node sorted as soon
	 * possible
	 *
	 *  0 0 0 0 1 0 1
	 * ^          |
	 * |          |
	 * +----------+
	 *
	 *
	 * the basic is idea is:
	 * - the last active scgi process should be still
	 *   in ram and is not swapped out yet
	 * - processes that are not reused will be killed
	 *   after some time by the trigger-handler
	 * - remember it as:
	 *   everything > 0 is hot
	 *   all unused procs are colder the more right they are
	 *   ice-cold processes are propably unused since more
	 *   than 'unused-timeout', are swaped out and won't be
	 *   reused in the next seconds anyway.
	 *
	 */

	/* nothing to sort, only one element */
	if (host->first == proc && proc->next == NULL) return 0;

	for (p = host->first; p != proc && p->load < proc->load; p = p->next);


	/* no need to move something
	 *
	 * 1 2 2 2 3 3 3
	 * ^
	 * |
	 * +
	 *
	 */
	if (p == proc) return 0;

	/* we have to move left. If we are already the first element
	 * we are done */
	if (host->first == proc) return 0;

	/* release proc */
	if (proc->prev) proc->prev->next = proc->next;
	if (proc->next) proc->next->prev = proc->prev;

	/* proc should be left of p */
	proc->next = p;
	proc->prev = p->prev;
	if (p->prev) p->prev->next = proc;
	p->prev = proc;

	if (proc->prev == NULL) host->first = proc;
#if 0
	for(p = host->first; p; p = p->next) {
		log_error_write(srv, __FILE__, __LINE__, "dd",
				p->pid, p->load);
	}
#else
	UNUSED(srv);
#endif

	return 0;
}

static int scgi_restart_dead_procs(server *srv, plugin_data *p, scgi_extension_host *host) {
	scgi_proc *proc;

	for (proc = host->first; proc; proc = proc->next) {
		if (p->conf.debug) {
			log_error_write(srv, __FILE__, __LINE__,  "sbdbdddd",
					"proc:",
					host->host, proc->port,
					proc->socket,
					proc->state,
					proc->is_local,
					proc->load,
					proc->pid);
		}

		if (0 == proc->is_local) {
			/*
			 * external servers might get disabled
			 *
			 * enable the server again, perhaps it is back again
			 */

			if ((proc->state == PROC_STATE_DISABLED) &&
			    (srv->cur_ts - proc->disable_ts > host->disable_time)) {
				proc->state = PROC_STATE_RUNNING;
				host->active_procs++;

				log_error_write(srv, __FILE__, __LINE__,  "sbdb",
						"fcgi-server re-enabled:",
						host->host, host->port,
						host->unixsocket);
			}
		} else {
			/* the child should not terminate at all */
			int status;

			if (proc->state == PROC_STATE_DIED_WAIT_FOR_PID) {
				switch(waitpid(proc->pid, &status, WNOHANG)) {
				case 0:
					/* child is still alive */
					break;
				case -1:
					break;
				default:
					if (WIFEXITED(status)) {
#if 0
						log_error_write(srv, __FILE__, __LINE__, "sdsd",
								"child exited, pid:", proc->pid,
								"status:", WEXITSTATUS(status));
#endif
					} else if (WIFSIGNALED(status)) {
						log_error_write(srv, __FILE__, __LINE__, "sd",
								"child signaled:",
								WTERMSIG(status));
					} else {
						log_error_write(srv, __FILE__, __LINE__, "sd",
								"child died somehow:",
								status);
					}

					proc->state = PROC_STATE_DIED;
					break;
				}
			}

			/*
			 * local servers might died, but we restart them
			 *
			 */
			if (proc->state == PROC_STATE_DIED &&
			    proc->load == 0) {
				/* restart the child */

				if (p->conf.debug) {
					log_error_write(srv, __FILE__, __LINE__, "ssdsbsdsd",
							"--- scgi spawning",
							"\n\tport:", host->port,
							"\n\tsocket", host->unixsocket,
							"\n\tcurrent:", 1, "/", host->min_procs);
				}

				if (scgi_spawn_connection(srv, p, host, proc)) {
					log_error_write(srv, __FILE__, __LINE__, "s",
							"ERROR: spawning fcgi failed.");
					return HANDLER_ERROR;
				}

				scgi_proclist_sort_down(srv, host, proc);
			}
		}
	}

	return 0;
}


static handler_t scgi_write_request(server *srv, handler_ctx *hctx) {
	scgi_extension_host *host= hctx->host;
	connection *con   = hctx->remote_conn;

	int ret;

	switch(hctx->state) {
	case FCGI_STATE_INIT:
		if (-1 == (hctx->fd = fdevent_socket_nb_cloexec(host->family, SOCK_STREAM, 0))) {
			if (errno == EMFILE ||
			    errno == EINTR) {
				log_error_write(srv, __FILE__, __LINE__, "sd",
						"wait for fd at connection:", con->fd);

				return HANDLER_WAIT_FOR_FD;
			}

			log_error_write(srv, __FILE__, __LINE__, "ssdd",
					"socket failed:", strerror(errno), srv->cur_fds, srv->max_fds);
			return HANDLER_ERROR;
		}
		hctx->fde_ndx = -1;

		srv->cur_fds++;

		fdevent_register(srv->ev, hctx->fd, scgi_handle_fdevent, hctx);

		if (-1 == fdevent_fcntl_set(srv->ev, hctx->fd)) {
			log_error_write(srv, __FILE__, __LINE__, "ss",
					"fcntl failed: ", strerror(errno));
			return HANDLER_ERROR;
		}

		/* fall through */
	case FCGI_STATE_CONNECT:
		if (hctx->state == FCGI_STATE_INIT) {
			for (hctx->proc = hctx->host->first;
			     hctx->proc && hctx->proc->state != PROC_STATE_RUNNING;
			     hctx->proc = hctx->proc->next);

			/* all childs are dead */
			if (hctx->proc == NULL) {
				hctx->fde_ndx = -1;

				return HANDLER_ERROR;
			}

			if (hctx->proc->is_local) {
				hctx->pid = hctx->proc->pid;
			}

			switch (scgi_establish_connection(srv, hctx)) {
			case 1:
				scgi_set_state(srv, hctx, FCGI_STATE_CONNECT);

				/* connection is in progress, wait for an event and call getsockopt() below */

				fdevent_event_set(srv->ev, &(hctx->fde_ndx), hctx->fd, FDEVENT_OUT);

				return HANDLER_WAIT_FOR_EVENT;
			case -1:
				/* if ECONNREFUSED; choose another connection */
				hctx->fde_ndx = -1;

				return HANDLER_ERROR;
			default:
				/* everything is ok, go on */
				break;
			}


		} else {
			int socket_error;
			socklen_t socket_error_len = sizeof(socket_error);

			/* try to finish the connect() */
			if (0 != getsockopt(hctx->fd, SOL_SOCKET, SO_ERROR, &socket_error, &socket_error_len)) {
				log_error_write(srv, __FILE__, __LINE__, "ss",
						"getsockopt failed:", strerror(errno));

				return HANDLER_ERROR;
			}
			if (socket_error != 0) {
				if (!hctx->proc->is_local || hctx->conf.debug) {
					/* local procs get restarted */

					log_error_write(srv, __FILE__, __LINE__, "ss",
							"establishing connection failed:", strerror(socket_error),
							"port:", hctx->proc->port);
				}

				return HANDLER_ERROR;
			}
		}

		/* ok, we have the connection */

		hctx->proc->load++;
		hctx->proc->last_used = srv->cur_ts;
		hctx->got_proc = 1;

		if (hctx->conf.debug) {
			log_error_write(srv, __FILE__, __LINE__, "sddbdd",
					"got proc:",
					hctx->fd,
					hctx->proc->pid,
					hctx->proc->socket,
					hctx->proc->port,
					hctx->proc->load);
		}

		/* move the proc-list entry down the list */
		scgi_proclist_sort_up(srv, hctx->host, hctx->proc);

		scgi_set_state(srv, hctx, FCGI_STATE_PREPARE_WRITE);
		/* fall through */
	case FCGI_STATE_PREPARE_WRITE:
		if (0 != scgi_create_env(srv, hctx)) {
			return HANDLER_FINISHED;
		}

		fdevent_event_add(srv->ev, &(hctx->fde_ndx), hctx->fd, FDEVENT_IN);
		scgi_set_state(srv, hctx, FCGI_STATE_WRITE);

		/* fall through */
	case FCGI_STATE_WRITE:
		ret = srv->network_backend_write(srv, con, hctx->fd, hctx->wb, MAX_WRITE_LIMIT);

		chunkqueue_remove_finished_chunks(hctx->wb);

		if (ret < 0) {
			if (errno == ENOTCONN || ret == -2) {
				/* the connection got dropped after accept()
				 *
				 * this is most of the time a PHP which dies
				 * after PHP_FCGI_MAX_REQUESTS
				 *
				 */
				if (hctx->wb->bytes_out == 0 &&
				    hctx->reconnects++ < 5) {
					usleep(10000); /* take away the load of the webserver
							* to let the php a chance to restart
							*/

					return scgi_reconnect(srv, hctx);
				}

				/* not reconnected ... why
				 *
				 * far@#lighttpd report this for FreeBSD
				 *
				 */

				log_error_write(srv, __FILE__, __LINE__, "ssosd",
						"connection was dropped after accept(). reconnect() denied:",
						"write-offset:", hctx->wb->bytes_out,
						"reconnect attempts:", hctx->reconnects);

				return HANDLER_ERROR;
			} else {
				/* -1 == ret => error on our side */
				log_error_write(srv, __FILE__, __LINE__, "ssd",
					"write failed:", strerror(errno), errno);

				return HANDLER_ERROR;
			}
		}

		if (hctx->wb->bytes_out == hctx->wb_reqlen) {
			fdevent_event_clr(srv->ev, &(hctx->fde_ndx), hctx->fd, FDEVENT_OUT);
			scgi_set_state(srv, hctx, FCGI_STATE_READ);
		} else {
			off_t wblen = hctx->wb->bytes_in - hctx->wb->bytes_out;
			if (hctx->wb->bytes_in < hctx->wb_reqlen && wblen < 65536 - 16384) {
				/*(con->conf.stream_request_body & FDEVENT_STREAM_REQUEST)*/
				if (!(con->conf.stream_request_body & FDEVENT_STREAM_REQUEST_POLLIN)) {
					con->conf.stream_request_body |= FDEVENT_STREAM_REQUEST_POLLIN;
					con->is_readable = 1; /* trigger optimistic read from client */
				}
			}
			if (0 == wblen) {
				fdevent_event_clr(srv->ev, &(hctx->fde_ndx), hctx->fd, FDEVENT_OUT);
			} else {
				fdevent_event_add(srv->ev, &(hctx->fde_ndx), hctx->fd, FDEVENT_OUT);
			}
		}

		return HANDLER_WAIT_FOR_EVENT;
	case FCGI_STATE_READ:
		/* waiting for a response */
		return HANDLER_WAIT_FOR_EVENT;
	default:
		log_error_write(srv, __FILE__, __LINE__, "s", "(debug) unknown state");
		return HANDLER_ERROR;
	}
}

static handler_t scgi_send_request(server *srv, handler_ctx *hctx) {
	/* ok, create the request */
	handler_t rc = scgi_write_request(srv, hctx);
	if (HANDLER_ERROR != rc) {
		return rc;
	} else {
		scgi_proc *proc = hctx->proc;
		scgi_extension_host *host = hctx->host;
		plugin_data *p  = hctx->plugin_data;
		connection *con = hctx->remote_conn;

		if (proc &&
		    0 == proc->is_local &&
		    proc->state != PROC_STATE_DISABLED) {
			/* only disable remote servers as we don't manage them*/

			log_error_write(srv, __FILE__, __LINE__,  "sbdb", "fcgi-server disabled:",
					host->host,
					proc->port,
					proc->socket);

			/* disable this server */
			proc->disable_ts = srv->cur_ts;
			proc->state = PROC_STATE_DISABLED;
			host->active_procs--;
		}

		if (hctx->state == FCGI_STATE_INIT ||
		    hctx->state == FCGI_STATE_CONNECT) {
			/* connect() or getsockopt() failed,
			 * restart the request-handling
			 */
			if (proc && proc->is_local) {

				if (hctx->conf.debug) {
					log_error_write(srv, __FILE__, __LINE__,  "sbdb", "connect() to scgi failed, restarting the request-handling:",
							host->host,
							proc->port,
							proc->socket);
				}

				/*
				 * several hctx might reference the same proc
				 *
				 * Only one of them should mark the proc as dead all the other
				 * ones should just take a new one.
				 *
				 * If a new proc was started with the old struct this might lead
				 * the mark a perfect proc as dead otherwise
				 *
				 */
				if (proc->state == PROC_STATE_RUNNING &&
				    hctx->pid == proc->pid) {
					proc->state = PROC_STATE_DIED_WAIT_FOR_PID;
				}
			}
			scgi_restart_dead_procs(srv, p, host);

			return scgi_reconnect(srv, hctx);
		} else {
			scgi_connection_close(srv, hctx);
			con->http_status = 503;

			return HANDLER_FINISHED;
		}
	}
}


static handler_t scgi_recv_response(server *srv, handler_ctx *hctx);


SUBREQUEST_FUNC(mod_scgi_handle_subrequest) {
	plugin_data *p = p_d;

	handler_ctx *hctx = con->plugin_ctx[p->id];

	if (NULL == hctx) return HANDLER_GO_ON;

	/* not my job */
	if (con->mode != p->id) return HANDLER_GO_ON;

	if ((con->conf.stream_response_body & FDEVENT_STREAM_RESPONSE_BUFMIN)
	    && con->file_started) {
		if (chunkqueue_length(con->write_queue) > 65536 - 4096) {
			fdevent_event_clr(srv->ev, &(hctx->fde_ndx), hctx->fd, FDEVENT_IN);
		} else if (!(fdevent_event_get_interest(srv->ev, hctx->fd) & FDEVENT_IN)) {
			/* optimistic read from backend, which might re-enable FDEVENT_IN */
			handler_t rc = scgi_recv_response(srv, hctx); /*(might invalidate hctx)*/
			if (rc != HANDLER_GO_ON) return rc;           /*(unless HANDLER_GO_ON)*/
		}
	}

	if (0 == hctx->wb->bytes_in
	    ? con->state == CON_STATE_READ_POST
	    : hctx->wb->bytes_in < hctx->wb_reqlen) {
		/*(64k - 4k to attempt to avoid temporary files
		 * in conjunction with FDEVENT_STREAM_REQUEST_BUFMIN)*/
		if (hctx->wb->bytes_in - hctx->wb->bytes_out > 65536 - 4096
		    && (con->conf.stream_request_body & FDEVENT_STREAM_REQUEST_BUFMIN)){
			con->conf.stream_request_body &= ~FDEVENT_STREAM_REQUEST_POLLIN;
			if (0 != hctx->wb->bytes_in) return HANDLER_WAIT_FOR_EVENT;
		} else {
			handler_t r = connection_handle_read_post_state(srv, con);
			chunkqueue *req_cq = con->request_content_queue;
			if (0 != hctx->wb->bytes_in && !chunkqueue_is_empty(req_cq)) {
				chunkqueue_append_chunkqueue(hctx->wb, req_cq);
				if (fdevent_event_get_interest(srv->ev, hctx->fd) & FDEVENT_OUT) {
					return (r == HANDLER_GO_ON) ? HANDLER_WAIT_FOR_EVENT : r;
				}
			}
			if (r != HANDLER_GO_ON) return r;

			/* SCGI requires that Content-Length be set.
			 * Send 411 Length Required if Content-Length missing.
			 * (occurs here if client sends Transfer-Encoding: chunked
			 *  and module is flagged to stream request body to backend) */
			if (-1 == con->request.content_length) {
				return connection_handle_read_post_error(srv, con, 411);
			}
		}
	}

	return ((0 == hctx->wb->bytes_in || !chunkqueue_is_empty(hctx->wb))
		&& hctx->state != FCGI_STATE_CONNECT)
	  ? scgi_send_request(srv, hctx)
	  : HANDLER_WAIT_FOR_EVENT;
}


static handler_t scgi_recv_response(server *srv, handler_ctx *hctx) {

		switch (scgi_demux_response(srv, hctx)) {
		case 0:
			break;
		case 1:
			/* we are done */
			scgi_connection_close(srv, hctx);

			return HANDLER_FINISHED;
		case -1: {
			connection  *con  = hctx->remote_conn;
			plugin_data *p    = hctx->plugin_data;

			scgi_proc *proc   = hctx->proc;
			scgi_extension_host *host= hctx->host;

			if (proc->pid && proc->state != PROC_STATE_DIED) {
				int status;

				/* only fetch the zombie if it is not already done */

				switch(waitpid(proc->pid, &status, WNOHANG)) {
				case 0:
					/* child is still alive */
					break;
				case -1:
					break;
				default:
					/* the child should not terminate at all */
					if (WIFEXITED(status)) {
						log_error_write(srv, __FILE__, __LINE__, "sdsd",
								"child exited, pid:", proc->pid,
								"status:", WEXITSTATUS(status));
					} else if (WIFSIGNALED(status)) {
						log_error_write(srv, __FILE__, __LINE__, "sd",
								"child signaled:",
								WTERMSIG(status));
					} else {
						log_error_write(srv, __FILE__, __LINE__, "sd",
								"child died somehow:",
								status);
					}

					if (hctx->conf.debug) {
						log_error_write(srv, __FILE__, __LINE__, "ssdsbsdsd",
								"--- scgi spawning",
								"\n\tport:", host->port,
								"\n\tsocket", host->unixsocket,
								"\n\tcurrent:", 1, "/", host->min_procs);
					}

					if (scgi_spawn_connection(srv, p, host, proc)) {
						/* child died */
						proc->state = PROC_STATE_DIED;
					} else {
						scgi_proclist_sort_down(srv, host, proc);
					}

					break;
				}
			}

			if (con->file_started == 0) {
				/* nothing has been send out yet, try to use another child */

				if (hctx->wb->bytes_out == 0 &&
				    hctx->reconnects++ < 5) {

					log_error_write(srv, __FILE__, __LINE__, "ssdsd",
						"response not sent, request not sent, reconnection.",
						"connection-fd:", con->fd,
						"fcgi-fd:", hctx->fd);

					return scgi_reconnect(srv, hctx);
				}

				log_error_write(srv, __FILE__, __LINE__, "sosdsd",
						"response not sent, request sent:", hctx->wb->bytes_out,
						"connection-fd:", con->fd,
						"fcgi-fd:", hctx->fd);
			} else {
				log_error_write(srv, __FILE__, __LINE__, "ssdsd",
						"response already sent out, termination connection",
						"connection-fd:", con->fd,
						"fcgi-fd:", hctx->fd);
			}

			http_response_backend_error(srv, con);
			scgi_connection_close(srv, hctx);
			return HANDLER_FINISHED;
		}
		}

		return HANDLER_GO_ON;
}


static handler_t scgi_handle_fdevent(server *srv, void *ctx, int revents) {
	handler_ctx *hctx = ctx;
	connection  *con  = hctx->remote_conn;

	joblist_append(srv, con);

	if (revents & FDEVENT_IN) {
		handler_t rc = scgi_recv_response(srv, hctx);/*(might invalidate hctx)*/
		if (rc != HANDLER_GO_ON) return rc;          /*(unless HANDLER_GO_ON)*/
	}

	if (revents & FDEVENT_OUT) {
		return scgi_send_request(srv, hctx); /*(might invalidate hctx)*/
	}

	/* perhaps this issue is already handled */
	if (revents & FDEVENT_HUP) {
		if (hctx->state == FCGI_STATE_CONNECT) {
			/* getoptsock will catch this one (right ?)
			 *
			 * if we are in connect we might get a EINPROGRESS
			 * in the first call and a FDEVENT_HUP in the
			 * second round
			 *
			 * FIXME: as it is a bit ugly.
			 *
			 */
			scgi_send_request(srv, hctx);
		} else if (con->file_started) {
			/* drain any remaining data from kernel pipe buffers
			 * even if (con->conf.stream_response_body
			 *          & FDEVENT_STREAM_RESPONSE_BUFMIN)
			 * since event loop will spin on fd FDEVENT_HUP event
			 * until unregistered. */
			handler_t rc;
			do {
				rc = scgi_recv_response(srv,hctx);/*(might invalidate hctx)*/
			} while (rc == HANDLER_GO_ON);            /*(unless HANDLER_GO_ON)*/
			return rc; /* HANDLER_FINISHED or HANDLER_ERROR */
		} else {
			scgi_extension_host *host= hctx->host;
			log_error_write(srv, __FILE__, __LINE__, "sbSBSDSd",
					"error: unexpected close of scgi connection for",
					con->uri.path,
					"(no scgi process on host: ",
					host->host,
					", port: ",
					host->port,
					" ?)",
					hctx->state);

			scgi_connection_close(srv, hctx);
		}
	} else if (revents & FDEVENT_ERR) {
		log_error_write(srv, __FILE__, __LINE__, "s",
				"fcgi: got a FDEVENT_ERR. Don't know why.");

		http_response_backend_error(srv, con);
		scgi_connection_close(srv, hctx);
	}

	return HANDLER_FINISHED;
}
#define PATCH(x) \
	p->conf.x = s->x;
static int scgi_patch_connection(server *srv, connection *con, plugin_data *p) {
	size_t i, j;
	plugin_config *s = p->config_storage[0];

	PATCH(exts);
	PATCH(proto);
	PATCH(debug);

	/* skip the first, the global context */
	for (i = 1; i < srv->config_context->used; i++) {
		data_config *dc = (data_config *)srv->config_context->data[i];
		s = p->config_storage[i];

		/* condition didn't match */
		if (!config_check_cond(srv, con, dc)) continue;

		/* merge config */
		for (j = 0; j < dc->value->used; j++) {
			data_unset *du = dc->value->data[j];

			if (buffer_is_equal_string(du->key, CONST_STR_LEN("scgi.server"))) {
				PATCH(exts);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("scgi.protocol"))) {
				PATCH(proto);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("scgi.debug"))) {
				PATCH(debug);
			}
		}
	}

	return 0;
}
#undef PATCH


static handler_t scgi_check_extension(server *srv, connection *con, void *p_d, int uri_path_handler) {
	plugin_data *p = p_d;
	size_t s_len, uri_path_len;
	size_t k;
	buffer *fn;
	scgi_extension *extension = NULL;
	scgi_extension_host *host = NULL;

	if (con->mode != DIRECT) return HANDLER_GO_ON;

	/* Possibly, we processed already this request */
	if (con->file_started == 1) return HANDLER_GO_ON;

	fn = uri_path_handler ? con->uri.path : con->physical.path;

	if (buffer_string_is_empty(fn)) return HANDLER_GO_ON;

	s_len = buffer_string_length(fn);
	uri_path_len = buffer_string_length(con->uri.path);

	scgi_patch_connection(srv, con, p);

	/* check if extension matches */
	for (k = 0; k < p->conf.exts->used; k++) {
		size_t ct_len;
		scgi_extension *ext = p->conf.exts->exts[k];

		if (buffer_is_empty(ext->key)) continue;

		ct_len = buffer_string_length(ext->key);

		/* check _url_ in the form "/scgi_pattern" */
		if (ext->key->ptr[0] == '/') {
			if (ct_len <= uri_path_len
			    && 0 == strncmp(con->uri.path->ptr, ext->key->ptr, ct_len)) {
				extension = ext;
				break;
			}
		} else if (ct_len <= s_len
			   && 0 == strncmp(fn->ptr + s_len - ct_len, ext->key->ptr, ct_len)) {
			/* check extension in the form ".fcg" */
			extension = ext;
			break;
		}
	}

	/* extension doesn't match */
	if (NULL == extension) {
		return HANDLER_GO_ON;
	}

	/* get best server */
	host = scgi_extension_host_get(srv, con, p, extension);
	if (NULL == host) {
		return HANDLER_FINISHED;
	}

	/* a note about no handler is not sent yet */
	extension->note_is_sent = 0;

	/*
	 * if check-local is disabled, use the uri.path handler
	 *
	 */

	/* init handler-context */
	if (uri_path_handler) {
		if (host->check_local == 0) {
			handler_ctx *hctx;
			char *pathinfo;

			hctx = handler_ctx_init();

			hctx->remote_conn      = con;
			hctx->plugin_data      = p;
			hctx->host             = host;
			hctx->proc	       = NULL;
			hctx->ext              = extension;

			/*hctx->conf.exts        = p->conf.exts;*/
			hctx->conf.proto       = p->conf.proto;
			hctx->conf.debug       = p->conf.debug;

			con->plugin_ctx[p->id] = hctx;

			host->load++;

			con->mode = p->id;

			if (con->conf.log_request_handling) {
				log_error_write(srv, __FILE__, __LINE__, "s",
				"handling it in mod_scgi");
			}

			/* the prefix is the SCRIPT_NAME,
			 * everything from start to the next slash
			 * this is important for check-local = "disable"
			 *
			 * if prefix = /admin.fcgi
			 *
			 * /admin.fcgi/foo/bar
			 *
			 * SCRIPT_NAME = /admin.fcgi
			 * PATH_INFO   = /foo/bar
			 *
			 * if prefix = /fcgi-bin/
			 *
			 * /fcgi-bin/foo/bar
			 *
			 * SCRIPT_NAME = /fcgi-bin/foo
			 * PATH_INFO   = /bar
			 *
			 */

			/* the rewrite is only done for /prefix/? matches */
			if (host->fix_root_path_name && extension->key->ptr[0] == '/' && extension->key->ptr[1] == '\0') {
				buffer_copy_string(con->request.pathinfo, con->uri.path->ptr);
				buffer_string_set_length(con->uri.path, 0);
			} else if (extension->key->ptr[0] == '/' &&
			    buffer_string_length(con->uri.path) > buffer_string_length(extension->key) &&
			    NULL != (pathinfo = strchr(con->uri.path->ptr + buffer_string_length(extension->key), '/'))) {
				/* rewrite uri.path and pathinfo */

				buffer_copy_string(con->request.pathinfo, pathinfo);
				buffer_string_set_length(con->uri.path, buffer_string_length(con->uri.path) - buffer_string_length(con->request.pathinfo));
			}
		}
	} else {
		handler_ctx *hctx;
		hctx = handler_ctx_init();

		hctx->remote_conn      = con;
		hctx->plugin_data      = p;
		hctx->host             = host;
		hctx->proc             = NULL;
		hctx->ext              = extension;

		/*hctx->conf.exts        = p->conf.exts;*/
		hctx->conf.proto       = p->conf.proto;
		hctx->conf.debug       = p->conf.debug;

		con->plugin_ctx[p->id] = hctx;

		host->load++;

		con->mode = p->id;

		if (con->conf.log_request_handling) {
			log_error_write(srv, __FILE__, __LINE__, "s", "handling it in mod_scgi");
		}
	}

	return HANDLER_GO_ON;
}

/* uri-path handler */
static handler_t scgi_check_extension_1(server *srv, connection *con, void *p_d) {
	return scgi_check_extension(srv, con, p_d, 1);
}

/* start request handler */
static handler_t scgi_check_extension_2(server *srv, connection *con, void *p_d) {
	return scgi_check_extension(srv, con, p_d, 0);
}


TRIGGER_FUNC(mod_scgi_handle_trigger) {
	plugin_data *p = p_d;
	size_t i, j, n;


	/* perhaps we should kill a connect attempt after 10-15 seconds
	 *
	 * currently we wait for the TCP timeout which is on Linux 180 seconds
	 *
	 *
	 *
	 */

	/* check all childs if they are still up */

	for (i = 0; i < srv->config_context->used; i++) {
		plugin_config *conf;
		scgi_exts *exts;

		conf = p->config_storage[i];

		exts = conf->exts;

		for (j = 0; j < exts->used; j++) {
			scgi_extension *ex;

			ex = exts->exts[j];

			for (n = 0; n < ex->used; n++) {

				scgi_proc *proc;
				unsigned long sum_load = 0;
				scgi_extension_host *host;

				host = ex->hosts[n];

				for (proc = host->first; proc; proc = proc->next) {
					int status;

					if (proc->pid == 0) continue;

					switch (waitpid(proc->pid, &status, WNOHANG)) {
					case 0:
						/* child still running after timeout, good */
						break;
					case -1:
						if (errno != EINTR) {
							/* no PID found ? should never happen */
							log_error_write(srv, __FILE__, __LINE__, "sddss",
									"pid ", proc->pid, proc->state,
									"not found:", strerror(errno));
						}
						break;
					default:
						/* the child should not terminate at all */
						if (WIFEXITED(status)) {
							if (proc->state != PROC_STATE_KILLED) {
								log_error_write(srv, __FILE__, __LINE__, "sdb",
										"child exited:",
										WEXITSTATUS(status), proc->socket);
							}
						} else if (WIFSIGNALED(status)) {
							if (WTERMSIG(status) != SIGTERM) {
								log_error_write(srv, __FILE__, __LINE__, "sd",
										"child signaled:",
										WTERMSIG(status));
							}
						} else {
							log_error_write(srv, __FILE__, __LINE__, "sd",
									"child died somehow:",
									status);
						}
						proc->pid = 0;
						if (proc->state == PROC_STATE_RUNNING) host->active_procs--;
						proc->state = PROC_STATE_DIED;
						host->max_id--;
					}
				}

				scgi_restart_dead_procs(srv, p, host);

				for (proc = host->first; proc; proc = proc->next) {
					sum_load += proc->load;
				}

				if (host->num_procs &&
				    host->num_procs < host->max_procs &&
				    (sum_load / host->num_procs) > host->max_load_per_proc) {
					/* overload, spawn new child */
					scgi_proc *fp = NULL;

					if (p->conf.debug) {
						log_error_write(srv, __FILE__, __LINE__, "s",
								"overload detected, spawning a new child");
					}

					for (fp = host->unused_procs; fp && fp->pid != 0; fp = fp->next);

					if (fp) {
						if (fp == host->unused_procs) host->unused_procs = fp->next;

						if (fp->next) fp->next->prev = NULL;

						host->max_id++;
					} else {
						fp = scgi_process_init();
						fp->id = host->max_id++;
					}

					host->num_procs++;

					if (buffer_string_is_empty(host->unixsocket)) {
						fp->port = host->port + fp->id;
					} else {
						buffer_copy_buffer(fp->socket, host->unixsocket);
						buffer_append_string_len(fp->socket, CONST_STR_LEN("-"));
						buffer_append_int(fp->socket, fp->id);
					}

					if (scgi_spawn_connection(srv, p, host, fp)) {
						log_error_write(srv, __FILE__, __LINE__, "s",
								"ERROR: spawning fcgi failed.");
						scgi_process_free(fp);
						return HANDLER_ERROR;
					}

					fp->prev = NULL;
					fp->next = host->first;
					if (host->first) {
						host->first->prev = fp;
					}
					host->first = fp;
				}

				for (proc = host->first; proc; proc = proc->next) {
					if (proc->load != 0) break;
					if (host->num_procs <= host->min_procs) break;
					if (proc->pid == 0) continue;

					if (srv->cur_ts - proc->last_used > host->idle_timeout) {
						/* a proc is idling for a long time now,
						 * terminated it */

						if (p->conf.debug) {
							log_error_write(srv, __FILE__, __LINE__, "ssbsd",
									"idle-timeout reached, terminating child:",
									"socket:", proc->socket,
									"pid", proc->pid);
						}


						if (proc->next) proc->next->prev = proc->prev;
						if (proc->prev) proc->prev->next = proc->next;

						if (proc->prev == NULL) host->first = proc->next;

						proc->prev = NULL;
						proc->next = host->unused_procs;

						if (host->unused_procs) host->unused_procs->prev = proc;
						host->unused_procs = proc;

						kill(proc->pid, SIGTERM);

						proc->state = PROC_STATE_KILLED;

						log_error_write(srv, __FILE__, __LINE__, "ssbsd",
									"killed:",
									"socket:", proc->socket,
									"pid", proc->pid);

						host->num_procs--;

						/* proc is now in unused, let the next second handle the next process */
						break;
					}
				}

				for (proc = host->unused_procs; proc; proc = proc->next) {
					int status;

					if (proc->pid == 0) continue;

					switch (waitpid(proc->pid, &status, WNOHANG)) {
					case 0:
						/* child still running after timeout, good */
						break;
					case -1:
						if (errno != EINTR) {
							/* no PID found ? should never happen */
							log_error_write(srv, __FILE__, __LINE__, "sddss",
									"pid ", proc->pid, proc->state,
									"not found:", strerror(errno));

#if 0
							if (errno == ECHILD) {
								/* someone else has cleaned up for us */
								proc->pid = 0;
								proc->state = PROC_STATE_UNSET;
							}
#endif
						}
						break;
					default:
						/* the child should not terminate at all */
						if (WIFEXITED(status)) {
							if (proc->state != PROC_STATE_KILLED) {
								log_error_write(srv, __FILE__, __LINE__, "sdb",
										"child exited:",
										WEXITSTATUS(status), proc->socket);
							}
						} else if (WIFSIGNALED(status)) {
							if (WTERMSIG(status) != SIGTERM) {
								log_error_write(srv, __FILE__, __LINE__, "sd",
										"child signaled:",
										WTERMSIG(status));
							}
						} else {
							log_error_write(srv, __FILE__, __LINE__, "sd",
									"child died somehow:",
									status);
						}
						proc->pid = 0;
						proc->state = PROC_STATE_UNSET;
						host->max_id--;
					}
				}
			}
		}
	}

	return HANDLER_GO_ON;
}


int mod_scgi_plugin_init(plugin *p);
int mod_scgi_plugin_init(plugin *p) {
	p->version     = LIGHTTPD_VERSION_ID;
	p->name         = buffer_init_string("scgi");

	p->init         = mod_scgi_init;
	p->cleanup      = mod_scgi_free;
	p->set_defaults = mod_scgi_set_defaults;
	p->connection_reset        = scgi_connection_reset;
	p->handle_connection_close = scgi_connection_reset;
	p->handle_uri_clean        = scgi_check_extension_1;
	p->handle_subrequest_start = scgi_check_extension_2;
	p->handle_subrequest       = mod_scgi_handle_subrequest;
	p->handle_trigger          = mod_scgi_handle_trigger;

	p->data         = NULL;

	return 0;
}
