#include "first.h"

#include "base.h"
#include "array.h"
#include "buffer.h"
#include "log.h"
#include "etag.h"
#include "http_chunk.h"
#include "response.h"
#include "stat_cache.h"

#include <string.h>
#include <errno.h>

#include <time.h>

/*
 * This was 'borrowed' from tcpdump.
 *
 *
 * This is fun.
 *
 * In older BSD systems, socket addresses were fixed-length, and
 * "sizeof (struct sockaddr)" gave the size of the structure.
 * All addresses fit within a "struct sockaddr".
 *
 * In newer BSD systems, the socket address is variable-length, and
 * there's an "sa_len" field giving the length of the structure;
 * this allows socket addresses to be longer than 2 bytes of family
 * and 14 bytes of data.
 *
 * Some commercial UNIXes use the old BSD scheme, some use the RFC 2553
 * variant of the old BSD scheme (with "struct sockaddr_storage" rather
 * than "struct sockaddr"), and some use the new BSD scheme.
 *
 * Some versions of GNU libc use neither scheme, but has an "SA_LEN()"
 * macro that determines the size based on the address family.  Other
 * versions don't have "SA_LEN()" (as it was in drafts of RFC 2553
 * but not in the final version).  On the latter systems, we explicitly
 * check the AF_ type to determine the length; we assume that on
 * all those systems we have "struct sockaddr_storage".
 */

#ifdef HAVE_IPV6
# ifndef SA_LEN
#  ifdef HAVE_SOCKADDR_SA_LEN
#   define SA_LEN(addr)   ((addr)->sa_len)
#  else /* HAVE_SOCKADDR_SA_LEN */
#   ifdef HAVE_STRUCT_SOCKADDR_STORAGE
static size_t get_sa_len(const struct sockaddr *addr) {
	switch (addr->sa_family) {

#    ifdef AF_INET
	case AF_INET:
		return (sizeof (struct sockaddr_in));
#    endif

#    ifdef AF_INET6
	case AF_INET6:
		return (sizeof (struct sockaddr_in6));
#    endif

	default:
		return (sizeof (struct sockaddr));

	}
}
#    define SA_LEN(addr)   (get_sa_len(addr))
#   else /* HAVE_SOCKADDR_STORAGE */
#    define SA_LEN(addr)   (sizeof (struct sockaddr))
#   endif /* HAVE_SOCKADDR_STORAGE */
#  endif /* HAVE_SOCKADDR_SA_LEN */
# endif /* SA_LEN */
#endif




int response_header_insert(server *srv, connection *con, const char *key, size_t keylen, const char *value, size_t vallen) {
	data_string *ds;

	UNUSED(srv);

	if (NULL == (ds = (data_string *)array_get_unused_element(con->response.headers, TYPE_STRING))) {
		ds = data_response_init();
	}
	buffer_copy_string_len(ds->key, key, keylen);
	buffer_copy_string_len(ds->value, value, vallen);

	array_insert_unique(con->response.headers, (data_unset *)ds);

	return 0;
}

int response_header_overwrite(server *srv, connection *con, const char *key, size_t keylen, const char *value, size_t vallen) {
	data_string *ds;

	UNUSED(srv);

	/* if there already is a key by this name overwrite the value */
	if (NULL != (ds = (data_string *)array_get_element(con->response.headers, key))) {
		buffer_copy_string(ds->value, value);

		return 0;
	}

	return response_header_insert(srv, con, key, keylen, value, vallen);
}

int response_header_append(server *srv, connection *con, const char *key, size_t keylen, const char *value, size_t vallen) {
	data_string *ds;

	UNUSED(srv);

	/* if there already is a key by this name append the value */
	if (NULL != (ds = (data_string *)array_get_element(con->response.headers, key))) {
		buffer_append_string_len(ds->value, CONST_STR_LEN(", "));
		buffer_append_string_len(ds->value, value, vallen);
		return 0;
	}

	return response_header_insert(srv, con, key, keylen, value, vallen);
}

int http_response_redirect_to_directory(server *srv, connection *con) {
	buffer *o;

	o = buffer_init();

	buffer_copy_buffer(o, con->uri.scheme);
	buffer_append_string_len(o, CONST_STR_LEN("://"));
	if (!buffer_is_empty(con->uri.authority)) {
		buffer_append_string_buffer(o, con->uri.authority);
	} else {
		/* get the name of the currently connected socket */
		struct hostent *he;
#ifdef HAVE_IPV6
		char hbuf[256];
#endif
		sock_addr our_addr;
		socklen_t our_addr_len;

		our_addr_len = sizeof(our_addr);

		if (-1 == getsockname(con->fd, (struct sockaddr *)&our_addr, &our_addr_len)
		    || our_addr_len > (socklen_t)sizeof(our_addr)) {
			con->http_status = 500;

			log_error_write(srv, __FILE__, __LINE__, "ss",
					"can't get sockname", strerror(errno));

			buffer_free(o);
			return 0;
		}


		/* Lookup name: secondly try to get hostname for bind address */
		switch(our_addr.plain.sa_family) {
#ifdef HAVE_IPV6
		case AF_INET6:
			if (0 != getnameinfo((const struct sockaddr *)(&our_addr.ipv6),
					     SA_LEN((const struct sockaddr *)&our_addr.ipv6),
					     hbuf, sizeof(hbuf), NULL, 0, 0)) {

				char dst[INET6_ADDRSTRLEN];

				log_error_write(srv, __FILE__, __LINE__,
						"SSS", "NOTICE: getnameinfo failed: ",
						strerror(errno), ", using ip-address instead");

				buffer_append_string_len(o, CONST_STR_LEN("["));
				buffer_append_string(o,
						     inet_ntop(AF_INET6, (char *)&our_addr.ipv6.sin6_addr,
							       dst, sizeof(dst)));
				buffer_append_string_len(o, CONST_STR_LEN("]"));
			} else {
				buffer_append_string(o, hbuf);
			}
			break;
#endif
		case AF_INET:
			if (NULL == (he = gethostbyaddr((char *)&our_addr.ipv4.sin_addr, sizeof(struct in_addr), AF_INET))) {
				log_error_write(srv, __FILE__, __LINE__,
						"SdS", "NOTICE: gethostbyaddr failed: ",
						h_errno, ", using ip-address instead");

				buffer_append_string(o, inet_ntoa(our_addr.ipv4.sin_addr));
			} else {
				buffer_append_string(o, he->h_name);
			}
			break;
		default:
			log_error_write(srv, __FILE__, __LINE__,
					"S", "ERROR: unsupported address-type");

			buffer_free(o);
			return -1;
		}

		{
			unsigned short default_port = 80;
			if (buffer_is_equal_caseless_string(con->uri.scheme, CONST_STR_LEN("https"))) {
				default_port = 443;
			}
			if (default_port != srv->srvconf.port) {
				buffer_append_string_len(o, CONST_STR_LEN(":"));
				buffer_append_int(o, srv->srvconf.port);
			}
		}
	}
	buffer_append_string_encoded(o, CONST_BUF_LEN(con->uri.path), ENCODING_REL_URI);
	buffer_append_string_len(o, CONST_STR_LEN("/"));
	if (!buffer_string_is_empty(con->uri.query)) {
		buffer_append_string_len(o, CONST_STR_LEN("?"));
		buffer_append_string_buffer(o, con->uri.query);
	}

	response_header_insert(srv, con, CONST_STR_LEN("Location"), CONST_BUF_LEN(o));

	con->http_status = 301;
	con->file_finished = 1;

	buffer_free(o);

	return 0;
}

buffer * strftime_cache_get(server *srv, time_t last_mod) {
	struct tm *tm;
	size_t i;

	for (i = 0; i < FILE_CACHE_MAX; i++) {
		/* found cache-entry */
		if (srv->mtime_cache[i].mtime == last_mod) return srv->mtime_cache[i].str;

		/* found empty slot */
		if (srv->mtime_cache[i].mtime == 0) break;
	}

	if (i == FILE_CACHE_MAX) {
		i = 0;
	}

	srv->mtime_cache[i].mtime = last_mod;
	buffer_string_prepare_copy(srv->mtime_cache[i].str, 1023);
	tm = gmtime(&(srv->mtime_cache[i].mtime));
	buffer_append_strftime(srv->mtime_cache[i].str, "%a, %d %b %Y %H:%M:%S GMT", tm);

	return srv->mtime_cache[i].str;
}


int http_response_handle_cachable(server *srv, connection *con, buffer *mtime) {
	int head_or_get =
		(  HTTP_METHOD_GET  == con->request.http_method
		|| HTTP_METHOD_HEAD == con->request.http_method);
	UNUSED(srv);

	/*
	 * 14.26 If-None-Match
	 *    [...]
	 *    If none of the entity tags match, then the server MAY perform the
	 *    requested method as if the If-None-Match header field did not exist,
	 *    but MUST also ignore any If-Modified-Since header field(s) in the
	 *    request. That is, if no entity tags match, then the server MUST NOT
	 *    return a 304 (Not Modified) response.
	 */

	if (con->request.http_if_none_match) {
		/* use strong etag checking for now: weak comparison must not be used
		 * for ranged requests
		 */
		if (etag_is_equal(con->physical.etag, con->request.http_if_none_match, 0)) {
			if (head_or_get) {
				con->http_status = 304;
				return HANDLER_FINISHED;
			} else {
				con->http_status = 412;
				con->mode = DIRECT;
				return HANDLER_FINISHED;
			}
		}
	} else if (con->request.http_if_modified_since && head_or_get) {
		/* last-modified handling */
		size_t used_len;
		char *semicolon;

		if (NULL == (semicolon = strchr(con->request.http_if_modified_since, ';'))) {
			used_len = strlen(con->request.http_if_modified_since);
		} else {
			used_len = semicolon - con->request.http_if_modified_since;
		}

		if (0 == strncmp(con->request.http_if_modified_since, mtime->ptr, used_len)) {
			if ('\0' == mtime->ptr[used_len]) con->http_status = 304;
			return HANDLER_FINISHED;
		} else {
			char buf[sizeof("Sat, 23 Jul 2005 21:20:01 GMT")];
			time_t t_header, t_file;
			struct tm tm;

			/* convert to timestamp */
			if (used_len >= sizeof(buf)) return HANDLER_GO_ON;

			strncpy(buf, con->request.http_if_modified_since, used_len);
			buf[used_len] = '\0';

			if (NULL == strptime(buf, "%a, %d %b %Y %H:%M:%S GMT", &tm)) {
				/**
				 * parsing failed, let's get out of here 
				 */
				return HANDLER_GO_ON;
			}
			tm.tm_isdst = 0;
			t_header = mktime(&tm);

			strptime(mtime->ptr, "%a, %d %b %Y %H:%M:%S GMT", &tm);
			tm.tm_isdst = 0;
			t_file = mktime(&tm);

			if (t_file > t_header) return HANDLER_GO_ON;

			con->http_status = 304;
			return HANDLER_FINISHED;
		}
	}

	return HANDLER_GO_ON;
}


static int http_response_parse_range(server *srv, connection *con, buffer *path, stat_cache_entry *sce) {
	int multipart = 0;
	int error;
	off_t start, end;
	const char *s, *minus;
	char *boundary = "fkj49sn38dcn3";
	data_string *ds;
	buffer *content_type = NULL;

	start = 0;
	end = sce->st.st_size - 1;

	con->response.content_length = 0;

	if (NULL != (ds = (data_string *)array_get_element(con->response.headers, "Content-Type"))) {
		content_type = ds->value;
	}

	for (s = con->request.http_range, error = 0;
	     !error && *s && NULL != (minus = strchr(s, '-')); ) {
		char *err;
		off_t la, le;

		if (s == minus) {
			/* -<stop> */

			le = strtoll(s, &err, 10);

			if (le == 0) {
				/* RFC 2616 - 14.35.1 */

				con->http_status = 416;
				error = 1;
			} else if (*err == '\0') {
				/* end */
				s = err;

				end = sce->st.st_size - 1;
				start = sce->st.st_size + le;
			} else if (*err == ',') {
				multipart = 1;
				s = err + 1;

				end = sce->st.st_size - 1;
				start = sce->st.st_size + le;
			} else {
				error = 1;
			}

		} else if (*(minus+1) == '\0' || *(minus+1) == ',') {
			/* <start>- */

			la = strtoll(s, &err, 10);

			if (err == minus) {
				/* ok */

				if (*(err + 1) == '\0') {
					s = err + 1;

					end = sce->st.st_size - 1;
					start = la;

				} else if (*(err + 1) == ',') {
					multipart = 1;
					s = err + 2;

					end = sce->st.st_size - 1;
					start = la;
				} else {
					error = 1;
				}
			} else {
				/* error */
				error = 1;
			}
		} else {
			/* <start>-<stop> */

			la = strtoll(s, &err, 10);

			if (err == minus) {
				le = strtoll(minus+1, &err, 10);

				/* RFC 2616 - 14.35.1 */
				if (la > le) {
					error = 1;
				}

				if (*err == '\0') {
					/* ok, end*/
					s = err;

					end = le;
					start = la;
				} else if (*err == ',') {
					multipart = 1;
					s = err + 1;

					end = le;
					start = la;
				} else {
					/* error */

					error = 1;
				}
			} else {
				/* error */

				error = 1;
			}
		}

		if (!error) {
			if (start < 0) start = 0;

			/* RFC 2616 - 14.35.1 */
			if (end > sce->st.st_size - 1) end = sce->st.st_size - 1;

			if (start > sce->st.st_size - 1) {
				error = 1;

				con->http_status = 416;
			}
		}

		if (!error) {
			if (multipart) {
				/* write boundary-header */
				buffer *b = buffer_init();

				buffer_copy_string_len(b, CONST_STR_LEN("\r\n--"));
				buffer_append_string(b, boundary);

				/* write Content-Range */
				buffer_append_string_len(b, CONST_STR_LEN("\r\nContent-Range: bytes "));
				buffer_append_int(b, start);
				buffer_append_string_len(b, CONST_STR_LEN("-"));
				buffer_append_int(b, end);
				buffer_append_string_len(b, CONST_STR_LEN("/"));
				buffer_append_int(b, sce->st.st_size);

				buffer_append_string_len(b, CONST_STR_LEN("\r\nContent-Type: "));
				buffer_append_string_buffer(b, content_type);

				/* write END-OF-HEADER */
				buffer_append_string_len(b, CONST_STR_LEN("\r\n\r\n"));

				con->response.content_length += buffer_string_length(b);
				chunkqueue_append_buffer(con->write_queue, b);
				buffer_free(b);
			}

			chunkqueue_append_file(con->write_queue, path, start, end - start + 1);
			con->response.content_length += end - start + 1;
		}
	}

	/* something went wrong */
	if (error) return -1;

	if (multipart) {
		/* add boundary end */
		buffer *b = buffer_init();

		buffer_copy_string_len(b, "\r\n--", 4);
		buffer_append_string(b, boundary);
		buffer_append_string_len(b, "--\r\n", 4);

		con->response.content_length += buffer_string_length(b);
		chunkqueue_append_buffer(con->write_queue, b);
		buffer_free(b);

		/* set header-fields */

		buffer_copy_string_len(srv->tmp_buf, CONST_STR_LEN("multipart/byteranges; boundary="));
		buffer_append_string(srv->tmp_buf, boundary);

		/* overwrite content-type */
		response_header_overwrite(srv, con, CONST_STR_LEN("Content-Type"), CONST_BUF_LEN(srv->tmp_buf));
	} else {
		/* add Content-Range-header */

		buffer_copy_string_len(srv->tmp_buf, CONST_STR_LEN("bytes "));
		buffer_append_int(srv->tmp_buf, start);
		buffer_append_string_len(srv->tmp_buf, CONST_STR_LEN("-"));
		buffer_append_int(srv->tmp_buf, end);
		buffer_append_string_len(srv->tmp_buf, CONST_STR_LEN("/"));
		buffer_append_int(srv->tmp_buf, sce->st.st_size);

		response_header_insert(srv, con, CONST_STR_LEN("Content-Range"), CONST_BUF_LEN(srv->tmp_buf));
	}

	/* ok, the file is set-up */
	return 0;
}


void http_response_send_file (server *srv, connection *con, buffer *path) {
	stat_cache_entry *sce = NULL;
	buffer *mtime = NULL;
	data_string *ds;
	int allow_caching = (0 == con->http_status || 200 == con->http_status);

	if (HANDLER_ERROR == stat_cache_get_entry(srv, con, path, &sce)) {
		con->http_status = (errno == ENOENT) ? 404 : 403;

		log_error_write(srv, __FILE__, __LINE__, "sbsb",
				"not a regular file:", con->uri.path,
				"->", path);

		return;
	}

	/* we only handline regular files */
#ifdef HAVE_LSTAT
	if ((sce->is_symlink == 1) && !con->conf.follow_symlink) {
		con->http_status = 403;

		if (con->conf.log_request_handling) {
			log_error_write(srv, __FILE__, __LINE__,  "s",  "-- access denied due symlink restriction");
			log_error_write(srv, __FILE__, __LINE__,  "sb", "Path         :", path);
		}

		return;
	}
#endif
	if (!S_ISREG(sce->st.st_mode)) {
		con->http_status = 403;

		if (con->conf.log_file_not_found) {
			log_error_write(srv, __FILE__, __LINE__, "sbsb",
					"not a regular file:", con->uri.path,
					"->", sce->name);
		}

		return;
	}

	/* mod_compress might set several data directly, don't overwrite them */

	/* set response content-type, if not set already */

	if (NULL == array_get_element(con->response.headers, "Content-Type")) {
		if (buffer_string_is_empty(sce->content_type)) {
			/* we are setting application/octet-stream, but also announce that
			 * this header field might change in the seconds few requests
			 *
			 * This should fix the aggressive caching of FF and the script download
			 * seen by the first installations
			 */
			response_header_overwrite(srv, con, CONST_STR_LEN("Content-Type"), CONST_STR_LEN("application/octet-stream"));

			allow_caching = 0;
		} else {
			response_header_overwrite(srv, con, CONST_STR_LEN("Content-Type"), CONST_BUF_LEN(sce->content_type));
		}
	}

	if (con->conf.range_requests) {
		response_header_overwrite(srv, con, CONST_STR_LEN("Accept-Ranges"), CONST_STR_LEN("bytes"));
	}

	if (allow_caching) {
		if (con->etag_flags != 0 && !buffer_string_is_empty(sce->etag)) {
			if (NULL == array_get_element(con->response.headers, "ETag")) {
				/* generate e-tag */
				etag_mutate(con->physical.etag, sce->etag);

				response_header_overwrite(srv, con, CONST_STR_LEN("ETag"), CONST_BUF_LEN(con->physical.etag));
			}
		}

		/* prepare header */
		if (NULL == (ds = (data_string *)array_get_element(con->response.headers, "Last-Modified"))) {
			mtime = strftime_cache_get(srv, sce->st.st_mtime);
			response_header_overwrite(srv, con, CONST_STR_LEN("Last-Modified"), CONST_BUF_LEN(mtime));
		} else {
			mtime = ds->value;
		}

		if (HANDLER_FINISHED == http_response_handle_cachable(srv, con, mtime)) {
			return;
		}
	}

	if (con->request.http_range && con->conf.range_requests
	    && (200 == con->http_status || 0 == con->http_status)
	    && NULL == array_get_element(con->response.headers, "Content-Encoding")) {
		int do_range_request = 1;
		/* check if we have a conditional GET */

		if (NULL != (ds = (data_string *)array_get_element(con->request.headers, "If-Range"))) {
			/* if the value is the same as our ETag, we do a Range-request,
			 * otherwise a full 200 */

			if (ds->value->ptr[0] == '"') {
				/**
				 * client wants a ETag
				 */
				if (!con->physical.etag) {
					do_range_request = 0;
				} else if (!buffer_is_equal(ds->value, con->physical.etag)) {
					do_range_request = 0;
				}
			} else if (!mtime) {
				/**
				 * we don't have a Last-Modified and can match the If-Range:
				 *
				 * sending all
				 */
				do_range_request = 0;
			} else if (!buffer_is_equal(ds->value, mtime)) {
				do_range_request = 0;
			}
		}

		if (do_range_request) {
			/* content prepared, I'm done */
			con->file_finished = 1;

			if (0 == http_response_parse_range(srv, con, path, sce)) {
				con->http_status = 206;
			}
			return;
		}
	}

	/* if we are still here, prepare body */

	/* we add it here for all requests
	 * the HEAD request will drop it afterwards again
	 */
	if (0 == sce->st.st_size || 0 == http_chunk_append_file(srv, con, path)) {
		con->http_status = 200;
		con->file_finished = 1;
	} else {
		con->http_status = 403;
	}
}

void http_response_xsendfile (server *srv, connection *con, buffer *path, const array *xdocroot) {
	const int status = con->http_status;
	int valid = 1;

	con->file_started = 1;

	/* reset Content-Length, if set by backend
	 * Content-Length might later be set to size of X-Sendfile static file,
	 * determined by open(), fstat() to reduces race conditions if the file
	 * is modified between stat() (stat_cache_get_entry()) and open(). */
	if (con->parsed_response & HTTP_CONTENT_LENGTH) {
		data_string *ds = (data_string *) array_get_element(con->response.headers, "Content-Length");
		if (ds) buffer_reset(ds->value);
		con->parsed_response &= ~HTTP_CONTENT_LENGTH;
		con->response.content_length = -1;
	}

	buffer_urldecode_path(path);
	buffer_path_simplify(path, path);
	if (con->conf.force_lowercase_filenames) {
		buffer_to_lower(path);
	}

	/* check that path is under xdocroot(s)
	 * - xdocroot should have trailing slash appended at config time
	 * - con->conf.force_lowercase_filenames is not a server-wide setting,
	 *   and so can not be definitively applied to xdocroot at config time*/
	if (xdocroot->used) {
		size_t i, xlen = buffer_string_length(path);
		for (i = 0; i < xdocroot->used; ++i) {
			data_string *ds = (data_string *)xdocroot->data[i];
			size_t dlen = buffer_string_length(ds->value);
			if (dlen <= xlen
			    && (!con->conf.force_lowercase_filenames
				? 0 == memcmp(path->ptr, ds->value->ptr, dlen)
				: 0 == strncasecmp(path->ptr, ds->value->ptr, dlen))) {
				break;
			}
		}
		if (i == xdocroot->used) {
			log_error_write(srv, __FILE__, __LINE__, "SBs",
					"X-Sendfile (", path,
					") not under configured x-sendfile-docroot(s)");
			con->http_status = 403;
			valid = 0;
		}
	}

	if (valid) http_response_send_file(srv, con, path);

	if (con->http_status >= 400 && status < 300) {
		con->mode = DIRECT;
	} else if (0 != status && 200 != status) {
		con->http_status = status;
	}
}

void http_response_backend_error (server *srv, connection *con) {
	UNUSED(srv);
	if (con->file_started) {
		/*(response might have been already started, kill the connection)*/
		/*(mode == DIRECT to avoid later call to http_response_backend_done())*/
		con->mode = DIRECT;  /*(avoid sending final chunked block)*/
		con->keep_alive = 0; /*(no keep-alive; final chunked block not sent)*/
		con->file_finished = 1;
	} /*(else error status set later by http_response_backend_done())*/
}

void http_response_backend_done (server *srv, connection *con) {
	/* (not CON_STATE_ERROR and not CON_STATE_RESPONSE_END,
	 *  i.e. not called from handle_connection_close or connection_reset
	 *  hooks, except maybe from errdoc handler, which later resets state)*/
	switch (con->state) {
	case CON_STATE_HANDLE_REQUEST:
	case CON_STATE_READ_POST:
		if (!con->file_started) {
			/* Send an error if we haven't sent any data yet */
			con->http_status = 500;
			con->mode = DIRECT;
			break;
		} /* else fall through */
	case CON_STATE_WRITE:
		if (!con->file_finished) {
			http_chunk_close(srv, con);
			con->file_finished = 1;
		}
	default:
		break;
	}
}


int http_cgi_headers (server *srv, connection *con, http_cgi_opts *opts, http_cgi_header_append_cb cb, void *vdata) {

    /* CGI-SPEC 6.1.2, FastCGI spec 6.3 and SCGI spec */

    int rc = 0;
    unsigned short port;
    server_socket *srv_sock = con->srv_socket;
    const char *s;
    size_t n;
    char buf[LI_ITOSTRING_LENGTH];
  #ifdef HAVE_IPV6
    char b2[INET6_ADDRSTRLEN + 1];
  #endif
    sock_addr *addr;
    sock_addr addrbuf;

    /* (CONTENT_LENGTH must be first for SCGI) */
    if (!opts->authorizer) {
        li_itostrn(buf, sizeof(buf), con->request.content_length);
        rc |= cb(vdata, CONST_STR_LEN("CONTENT_LENGTH"), buf, strlen(buf));
    }

    if (!buffer_string_is_empty(con->uri.query)) {
        rc |= cb(vdata, CONST_STR_LEN("QUERY_STRING"),
                        CONST_BUF_LEN(con->uri.query));
    } else {
        rc |= cb(vdata, CONST_STR_LEN("QUERY_STRING"),
                        CONST_STR_LEN(""));
    }
    if (!buffer_string_is_empty(opts->strip_request_uri)) {
        /**
         * /app1/index/list
         *
         * stripping /app1 or /app1/ should lead to
         *
         * /index/list
         *
         */
        size_t len = buffer_string_length(opts->strip_request_uri);
        if ('/' == opts->strip_request_uri->ptr[len-1]) {
            --len;
        }

        if (buffer_string_length(con->request.orig_uri) >= len
            && 0 == memcmp(con->request.orig_uri->ptr,
                           opts->strip_request_uri->ptr, len)
            && con->request.orig_uri->ptr[len] == '/') {
            rc |= cb(vdata, CONST_STR_LEN("REQUEST_URI"),
                            con->request.orig_uri->ptr+len,
                            buffer_string_length(con->request.orig_uri) - len);
        } else {
            rc |= cb(vdata, CONST_STR_LEN("REQUEST_URI"),
                            CONST_BUF_LEN(con->request.orig_uri));
        }
    } else {
        rc |= cb(vdata, CONST_STR_LEN("REQUEST_URI"),
                        CONST_BUF_LEN(con->request.orig_uri));
    }
    if (!buffer_is_equal(con->request.uri, con->request.orig_uri)) {
        rc |= cb(vdata, CONST_STR_LEN("REDIRECT_URI"),
                        CONST_BUF_LEN(con->request.uri));
    }
    /* set REDIRECT_STATUS for php compiled with --force-redirect
     * (if REDIRECT_STATUS has not already been set by error handler) */
    if (0 == con->error_handler_saved_status) {
        rc |= cb(vdata, CONST_STR_LEN("REDIRECT_STATUS"),
                        CONST_STR_LEN("200"));
    }

    /*
     * SCRIPT_NAME, PATH_INFO and PATH_TRANSLATED according to
     * http://cgi-spec.golux.com/draft-coar-cgi-v11-03-clean.html
     * (6.1.14, 6.1.6, 6.1.7)
     */
    if (!opts->authorizer) {
        rc |= cb(vdata, CONST_STR_LEN("SCRIPT_NAME"),
                        CONST_BUF_LEN(con->uri.path));
        if (!buffer_string_is_empty(con->request.pathinfo)) {
            rc |= cb(vdata, CONST_STR_LEN("PATH_INFO"),
                            CONST_BUF_LEN(con->request.pathinfo));
            /* PATH_TRANSLATED is only defined if PATH_INFO is set */
            if (!buffer_string_is_empty(opts->docroot)) {
                buffer_copy_buffer(srv->tmp_buf, opts->docroot);
            } else {
                buffer_copy_buffer(srv->tmp_buf, con->physical.basedir);
            }
            buffer_append_string_buffer(srv->tmp_buf, con->request.pathinfo);
            rc |= cb(vdata, CONST_STR_LEN("PATH_TRANSLATED"),
                            CONST_BUF_LEN(srv->tmp_buf));
        }
    }

   /*
    * SCRIPT_FILENAME and DOCUMENT_ROOT for php
    * The PHP manual http://www.php.net/manual/en/reserved.variables.php
    * treatment of PATH_TRANSLATED is different from the one of CGI specs.
    * (see php.ini cgi.fix_pathinfo = 1 config parameter)
    */

    if (!buffer_string_is_empty(opts->docroot)) {
        /* alternate docroot, e.g. for remote FastCGI or SCGI server */
        buffer_copy_buffer(srv->tmp_buf, opts->docroot);
        buffer_append_string_buffer(srv->tmp_buf, con->uri.path);
        rc |= cb(vdata, CONST_STR_LEN("SCRIPT_FILENAME"),
                        CONST_BUF_LEN(srv->tmp_buf));
        rc |= cb(vdata, CONST_STR_LEN("DOCUMENT_ROOT"),
                        CONST_BUF_LEN(opts->docroot));
    } else {
        if (opts->break_scriptfilename_for_php) {
            /* php.ini config cgi.fix_pathinfo = 1 need a broken SCRIPT_FILENAME
             * to find out what PATH_INFO is itself
             *
             * see src/sapi/cgi_main.c, init_request_info()
             */
            buffer_copy_buffer(srv->tmp_buf, con->physical.path);
            buffer_append_string_buffer(srv->tmp_buf, con->request.pathinfo);
            rc |= cb(vdata, CONST_STR_LEN("SCRIPT_FILENAME"),
                            CONST_BUF_LEN(srv->tmp_buf));
        } else {
            rc |= cb(vdata, CONST_STR_LEN("SCRIPT_FILENAME"),
                            CONST_BUF_LEN(con->physical.path));
        }
        rc |= cb(vdata, CONST_STR_LEN("DOCUMENT_ROOT"),
                        CONST_BUF_LEN(con->physical.basedir));
    }

    s = get_http_method_name(con->request.http_method);
    force_assert(s);
    rc |= cb(vdata, CONST_STR_LEN("REQUEST_METHOD"), s, strlen(s));

    s = get_http_version_name(con->request.http_version);
    force_assert(s);
    rc |= cb(vdata, CONST_STR_LEN("SERVER_PROTOCOL"), s, strlen(s));

    rc |= cb(vdata, CONST_STR_LEN("SERVER_SOFTWARE"),
                    CONST_BUF_LEN(con->conf.server_tag));

    rc |= cb(vdata, CONST_STR_LEN("GATEWAY_INTERFACE"),
                    CONST_STR_LEN("CGI/1.1"));

    if (buffer_is_equal_caseless_string(con->uri.scheme,
                                        CONST_STR_LEN("https"))) {
        rc |= cb(vdata, CONST_STR_LEN("HTTPS"), CONST_STR_LEN("on"));
    }

    addr = &srv_sock->addr;
  #ifdef HAVE_IPV6
    port = addr->plain.sa_family == AF_INET6
         ? addr->ipv6.sin6_port
         : addr->ipv4.sin_port;
  #else
    port = addr->ipv4.sin_port;
  #endif
    li_utostrn(buf, sizeof(buf), ntohs(port));
    rc |= cb(vdata, CONST_STR_LEN("SERVER_PORT"), buf, strlen(buf));

    switch (addr->plain.sa_family) {
  #ifdef HAVE_IPV6
    case AF_INET6:
        if (0 ==memcmp(&addr->ipv6.sin6_addr,&in6addr_any,sizeof(in6addr_any))){
            socklen_t addrlen = sizeof(addrbuf);
            if (0 == getsockname(con->fd,(struct sockaddr *)&addrbuf,&addrlen)){
                addr = &addrbuf;
            } else {
                s = "";
                break;
            }
        }
        s = inet_ntop(AF_INET6, (const void *) &(addr->ipv6.sin6_addr),
                      b2, sizeof(b2)-1);
        break;
  #endif
    case AF_INET:
        if (srv_sock->addr.ipv4.sin_addr.s_addr == INADDR_ANY) {
            socklen_t addrlen = sizeof(addrbuf);
            if (0 == getsockname(con->fd,(struct sockaddr *)&addrbuf,&addrlen)){
                addr = &addrbuf;
            } else {
                s = "";
                break;
            }
        }
      #ifdef HAVE_IPV6
        s = inet_ntop(AF_INET, (const void *) &(addr->ipv4.sin_addr),
                      b2, sizeof(b2)-1);
      #else
        s = inet_ntoa(addr->ipv4.sin_addr);
      #endif
        break;
    default:
        s = "";
        break;
    }
    force_assert(s);
    rc |= cb(vdata, CONST_STR_LEN("SERVER_ADDR"), s, strlen(s));

    if (!buffer_string_is_empty(con->server_name)) {
        size_t len = buffer_string_length(con->server_name);

        if (con->server_name->ptr[0] == '[') {
            const char *colon = strstr(con->server_name->ptr, "]:");
            if (colon) len = (colon + 1) - con->server_name->ptr;
        } else {
            const char *colon = strchr(con->server_name->ptr, ':');
            if (colon) len = colon - con->server_name->ptr;
        }

        rc |= cb(vdata, CONST_STR_LEN("SERVER_NAME"),
                        con->server_name->ptr, len);
    } else {
        /* set to be same as SERVER_ADDR (above) */
        rc |= cb(vdata, CONST_STR_LEN("SERVER_NAME"), s, strlen(s));
    }

    rc |= cb(vdata, CONST_STR_LEN("REMOTE_ADDR"),
                    CONST_BUF_LEN(con->dst_addr_buf));

  #ifdef HAVE_IPV6
    port = con->dst_addr.plain.sa_family == AF_INET6
         ? con->dst_addr.ipv6.sin6_port
         : con->dst_addr.ipv4.sin_port;
  #else
    port = con->dst_addr.ipv4.sin_port;
  #endif
    li_utostrn(buf, sizeof(buf), ntohs(port));
    rc |= cb(vdata, CONST_STR_LEN("REMOTE_PORT"), buf, strlen(buf));

    for (n = 0; n < con->request.headers->used; n++) {
        data_string *ds = (data_string *)con->request.headers->data[n];
        if (!buffer_is_empty(ds->value) && !buffer_is_empty(ds->key)) {
            /* Security: Do not emit HTTP_PROXY in environment.
             * Some executables use HTTP_PROXY to configure
             * outgoing proxy.  See also https://httpoxy.org/ */
            if (buffer_is_equal_caseless_string(ds->key,
                                                CONST_STR_LEN("Proxy"))) {
                continue;
            }
            buffer_copy_string_encoded_cgi_varnames(srv->tmp_buf,
                                                    CONST_BUF_LEN(ds->key), 1);
            rc |= cb(vdata, CONST_BUF_LEN(srv->tmp_buf),
                            CONST_BUF_LEN(ds->value));
        }
    }

  #ifdef USE_OPENSSL
    if (con->ssl) http_cgi_ssl_env(srv, con);
  #endif

    for (n = 0; n < con->environment->used; n++) {
        data_string *ds = (data_string *)con->environment->data[n];
        if (!buffer_is_empty(ds->value) && !buffer_is_empty(ds->key)) {
            buffer_copy_string_encoded_cgi_varnames(srv->tmp_buf,
                                                    CONST_BUF_LEN(ds->key), 0);
            rc |= cb(vdata, CONST_BUF_LEN(srv->tmp_buf),
                            CONST_BUF_LEN(ds->value));
        }
    }

    return rc;
}


#ifdef USE_OPENSSL
void http_cgi_ssl_env(server *srv, connection *con) {
    const char *s;
    const SSL_CIPHER *cipher;
    UNUSED(srv);

    if (!con->ssl) return;

    s = SSL_get_version(con->ssl);
    array_set_key_value(con->environment,
                        CONST_STR_LEN("SSL_PROTOCOL"),
                        s, strlen(s));

    if ((cipher = SSL_get_current_cipher(con->ssl))) {
        int usekeysize, algkeysize;
        char buf[LI_ITOSTRING_LENGTH];
        s = SSL_CIPHER_get_name(cipher);
        array_set_key_value(con->environment,
                            CONST_STR_LEN("SSL_CIPHER"),
                            s, strlen(s));
        usekeysize = SSL_CIPHER_get_bits(cipher, &algkeysize);
        li_itostrn(buf, sizeof(buf), usekeysize);
        array_set_key_value(con->environment,
                            CONST_STR_LEN("SSL_CIPHER_USEKEYSIZE"),
                            buf, strlen(buf));
        li_itostrn(buf, sizeof(buf), algkeysize);
        array_set_key_value(con->environment,
                            CONST_STR_LEN("SSL_CIPHER_ALGKEYSIZE"),
                            buf, strlen(buf));
    }
}
#endif
