#include "first.h"

#include "server.h"
#include "log.h"
#include "stream.h"
#include "plugin.h"

#include "configparser.h"
#include "configfile.h"
#include "proc_open.h"
#include "request.h"
#include "version.h"

#include <sys/stat.h>

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>
#include <glob.h>


#if defined(HAVE_MYSQL) || (defined(HAVE_LDAP_H) && defined(HAVE_LBER_H) && defined(HAVE_LIBLDAP) && defined(HAVE_LIBLBER))
static void config_warn_authn_module (server *srv, const char *module) {
	size_t len = strlen(module);
	for (size_t i = 0; i < srv->config_context->used; ++i) {
		const data_config *config = (data_config const*)srv->config_context->data[i];
		const data_unset *du = array_get_element(config->value, "auth.backend");
		if (NULL != du && du->type == TYPE_STRING) {
			data_string *ds = (data_string *)du;
			if (buffer_is_equal_string(ds->value, module, len)) {
				ds = data_string_init();
				buffer_copy_string_len(ds->value, CONST_STR_LEN("mod_authn_"));
				buffer_append_string(ds->value, module);
				array_insert_unique(srv->srvconf.modules, (data_unset *)ds);
				log_error_write(srv, __FILE__, __LINE__, "SSSsSSS", "Warning: please add \"mod_authn_", module, "\" to server.modules list in lighttpd.conf.  A future release of lighttpd 1.4.x will not automatically load mod_authn_", module, "and lighttpd will fail to start up since your lighttpd.conf uses auth.backend = \"", module, "\".");
				return;
			}
		}
	}
}
#endif

static int config_insert(server *srv) {
	size_t i;
	int ret = 0;
	buffer *stat_cache_string;

	config_values_t cv[] = {
		{ "server.bind",                       NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_SERVER     }, /* 0 */
		{ "server.errorlog",                   NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_SERVER     }, /* 1 */
		{ "server.errorfile-prefix",           NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_CONNECTION }, /* 2 */
		{ "server.chroot",                     NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_SERVER     }, /* 3 */
		{ "server.username",                   NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_SERVER     }, /* 4 */
		{ "server.groupname",                  NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_SERVER     }, /* 5 */
		{ "server.port",                       NULL, T_CONFIG_SHORT,   T_CONFIG_SCOPE_SERVER     }, /* 6 */
		{ "server.tag",                        NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_CONNECTION }, /* 7 */
		{ "server.use-ipv6",                   NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 8 */
		{ "server.modules",                    NULL, T_CONFIG_ARRAY,   T_CONFIG_SCOPE_SERVER     }, /* 9 */

		{ "server.event-handler",              NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_SERVER     }, /* 10 */
		{ "server.pid-file",                   NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_SERVER     }, /* 11 */
		{ "server.max-request-size",           NULL, T_CONFIG_INT,     T_CONFIG_SCOPE_CONNECTION }, /* 12 */
		{ "server.max-worker",                 NULL, T_CONFIG_SHORT,   T_CONFIG_SCOPE_SERVER     }, /* 13 */
		{ "server.document-root",              NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_CONNECTION }, /* 14 */
		{ "server.force-lowercase-filenames",  NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 15 */
		{ "debug.log-condition-handling",      NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 16 */
		{ "server.max-keep-alive-requests",    NULL, T_CONFIG_SHORT,   T_CONFIG_SCOPE_CONNECTION }, /* 17 */
		{ "server.name",                       NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_CONNECTION }, /* 18 */
		{ "server.max-keep-alive-idle",        NULL, T_CONFIG_SHORT,   T_CONFIG_SCOPE_CONNECTION }, /* 19 */

		{ "server.max-read-idle",              NULL, T_CONFIG_SHORT,   T_CONFIG_SCOPE_CONNECTION }, /* 20 */
		{ "server.max-write-idle",             NULL, T_CONFIG_SHORT,   T_CONFIG_SCOPE_CONNECTION }, /* 21 */
		{ "server.error-handler",              NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_CONNECTION }, /* 22 */
		{ "server.max-fds",                    NULL, T_CONFIG_SHORT,   T_CONFIG_SCOPE_SERVER     }, /* 23 */
#ifdef HAVE_LSTAT
		{ "server.follow-symlink",             NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 24 */
#else
		{ "server.follow-symlink",
			"Your system lacks lstat(). We can not differ symlinks from files."
			"Please remove server.follow-symlinks from your config.",
			T_CONFIG_UNSUPPORTED, T_CONFIG_SCOPE_UNSET },
#endif
		{ "server.kbytes-per-second",          NULL, T_CONFIG_SHORT,   T_CONFIG_SCOPE_CONNECTION }, /* 25 */
		{ "connection.kbytes-per-second",      NULL, T_CONFIG_SHORT,   T_CONFIG_SCOPE_CONNECTION }, /* 26 */
		{ "mimetype.use-xattr",                NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 27 */
		{ "mimetype.assign",                   NULL, T_CONFIG_ARRAY,   T_CONFIG_SCOPE_CONNECTION }, /* 28 */
		{ "ssl.pemfile",                       NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_CONNECTION }, /* 29 */

		{ "ssl.engine",                        NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 30 */
		{ "debug.log-file-not-found",          NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 31 */
		{ "debug.log-request-handling",        NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 32 */
		{ "debug.log-response-header",         NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 33 */
		{ "debug.log-request-header",          NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 34 */
		{ "debug.log-ssl-noise",               NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 35 */
		{ "server.protocol-http11",            NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 36 */
		{ "debug.log-request-header-on-error", NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_SERVER     }, /* 37 */
		{ "debug.log-state-handling",          NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_SERVER     }, /* 38 */
		{ "ssl.ca-file",                       NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_CONNECTION }, /* 39 */

		{ "server.errorlog-use-syslog",        NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_SERVER     }, /* 40 */
		{ "server.range-requests",             NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 41 */
		{ "server.stat-cache-engine",          NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_SERVER     }, /* 42 */
		{ "server.max-connections",            NULL, T_CONFIG_SHORT,   T_CONFIG_SCOPE_SERVER     }, /* 43 */
		{ "server.network-backend",            NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_SERVER     }, /* 44 */
		{ "server.upload-dirs",                NULL, T_CONFIG_ARRAY,   T_CONFIG_SCOPE_SERVER     }, /* 45 */
		{ "server.core-files",                 NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_SERVER     }, /* 46 */
		{ "ssl.cipher-list",                   NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_CONNECTION }, /* 47 */
		{ "ssl.use-sslv2",                     NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 48 */
		{ "etag.use-inode",                    NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 49 */

		{ "etag.use-mtime",                    NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 50 */
		{ "etag.use-size",                     NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 51 */
		{ "server.reject-expect-100-with-417", NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_SERVER     }, /* 52 */
		{ "debug.log-timeouts",                NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 53 */
		{ "server.defer-accept",               NULL, T_CONFIG_SHORT,   T_CONFIG_SCOPE_CONNECTION }, /* 54 */
		{ "server.breakagelog",                NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_SERVER     }, /* 55 */
		{ "ssl.verifyclient.activate",         NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 56 */
		{ "ssl.verifyclient.enforce",          NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 57 */
		{ "ssl.verifyclient.depth",            NULL, T_CONFIG_SHORT,   T_CONFIG_SCOPE_CONNECTION }, /* 58 */
		{ "ssl.verifyclient.username",         NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_CONNECTION }, /* 59 */

		{ "ssl.verifyclient.exportcert",       NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 60 */
		{ "server.set-v6only",                 NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 61 */
		{ "ssl.use-sslv3",                     NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 62 */
		{ "ssl.dh-file",                       NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_CONNECTION }, /* 63 */
		{ "ssl.ec-curve",                      NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_CONNECTION }, /* 64 */
		{ "ssl.disable-client-renegotiation",  NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 65 */
		{ "ssl.honor-cipher-order",            NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 66 */
		{ "ssl.empty-fragments",               NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 67 */
		{ "server.upload-temp-file-size",      NULL, T_CONFIG_INT,     T_CONFIG_SCOPE_SERVER     }, /* 68 */
		{ "mimetype.xattr-name",               NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_SERVER     }, /* 69 */
		{ "server.listen-backlog",             NULL, T_CONFIG_INT,     T_CONFIG_SCOPE_CONNECTION }, /* 70 */
		{ "server.error-handler-404",          NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_CONNECTION }, /* 71 */
		{ "server.http-parseopt-header-strict",NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_SERVER     }, /* 72 */
		{ "server.http-parseopt-host-strict",  NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_SERVER     }, /* 73 */
		{ "server.http-parseopt-host-normalize",NULL,T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_SERVER     }, /* 74 */
		{ "server.bsd-accept-filter",          NULL, T_CONFIG_STRING,  T_CONFIG_SCOPE_CONNECTION }, /* 75 */
		{ "server.stream-request-body",        NULL, T_CONFIG_SHORT,   T_CONFIG_SCOPE_CONNECTION }, /* 76 */
		{ "server.stream-response-body",       NULL, T_CONFIG_SHORT,   T_CONFIG_SCOPE_CONNECTION }, /* 77 */
		{ "server.max-request-field-size",     NULL, T_CONFIG_INT,     T_CONFIG_SCOPE_SERVER     }, /* 78 */
		{ "ssl.read-ahead",                    NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION }, /* 79 */

		{ NULL,                                NULL, T_CONFIG_UNSET,   T_CONFIG_SCOPE_UNSET      }
	};

	/* all T_CONFIG_SCOPE_SERVER options */
	cv[0].destination = srv->srvconf.bindhost;
	cv[1].destination = srv->srvconf.errorlog_file;
	cv[3].destination = srv->srvconf.changeroot;
	cv[4].destination = srv->srvconf.username;
	cv[5].destination = srv->srvconf.groupname;
	cv[6].destination = &(srv->srvconf.port);
	cv[9].destination = srv->srvconf.modules;

	cv[10].destination = srv->srvconf.event_handler;
	cv[11].destination = srv->srvconf.pid_file;
	cv[13].destination = &(srv->srvconf.max_worker);

	cv[23].destination = &(srv->srvconf.max_fds);

	cv[37].destination = &(srv->srvconf.log_request_header_on_error);
	cv[38].destination = &(srv->srvconf.log_state_handling);

	cv[40].destination = &(srv->srvconf.errorlog_use_syslog);
	stat_cache_string = buffer_init();
	cv[42].destination = stat_cache_string;
	cv[43].destination = &(srv->srvconf.max_conns);
	cv[44].destination = srv->srvconf.network_backend;
	cv[45].destination = srv->srvconf.upload_tempdirs;
	cv[46].destination = &(srv->srvconf.enable_cores);

	cv[52].destination = &(srv->srvconf.reject_expect_100_with_417);
	cv[55].destination = srv->srvconf.breakagelog_file;

	cv[68].destination = &(srv->srvconf.upload_temp_file_size);
	cv[69].destination = srv->srvconf.xattr_name;
	cv[72].destination = &(srv->srvconf.http_header_strict);
	cv[73].destination = &(srv->srvconf.http_host_strict);
	cv[74].destination = &(srv->srvconf.http_host_normalize);
	cv[78].destination = &(srv->srvconf.max_request_field_size);

	srv->config_storage = calloc(1, srv->config_context->used * sizeof(specific_config *));

	force_assert(srv->config_storage);
	force_assert(srv->config_context->used); /* static analysis hint for ccc
-analyzer */

	for (i = 0; i < srv->config_context->used; i++) {
		data_config const* config = (data_config const*)srv->config_context->data[i];
		specific_config *s;

		s = calloc(1, sizeof(specific_config));
		force_assert(s);
		s->document_root = buffer_init();
		s->mimetypes     = array_init();
		s->server_name   = buffer_init();
		s->ssl_pemfile   = buffer_init();
		s->ssl_ca_file   = buffer_init();
		s->error_handler = buffer_init();
		s->error_handler_404 = buffer_init();
		s->server_tag    = buffer_init_string(PACKAGE_DESC);
		s->ssl_cipher_list = buffer_init();
		s->ssl_dh_file   = buffer_init();
		s->ssl_ec_curve  = buffer_init();
		s->errorfile_prefix = buffer_init();
	      #if defined(__FreeBSD__) || defined(__NetBSD__) \
	       || defined(__OpenBSD__) || defined(__DragonFly__)
		s->bsd_accept_filter = (i == 0)
		  ? buffer_init()
		  : buffer_init_buffer(srv->config_storage[0]->bsd_accept_filter);
	      #endif
		s->max_keep_alive_requests = 16;
		s->max_keep_alive_idle = 5;
		s->max_read_idle = 60;
		s->max_write_idle = 360;
		s->max_request_size = 0;
		s->use_xattr     = 0;
		s->ssl_enabled   = 0;
		s->ssl_honor_cipher_order = 1;
		s->ssl_empty_fragments = 0;
		s->ssl_use_sslv2 = 0;
		s->ssl_use_sslv3 = 0;
		s->use_ipv6      = (i == 0) ? 0 : srv->config_storage[0]->use_ipv6;
		s->set_v6only    = (i == 0) ? 1 : srv->config_storage[0]->set_v6only;
		s->defer_accept  = (i == 0) ? 0 : srv->config_storage[0]->defer_accept;
#ifdef HAVE_LSTAT
		s->follow_symlink = 1;
#endif
		s->kbytes_per_second = 0;
		s->allow_http11  = 1;
		s->etag_use_inode = 1;
		s->etag_use_mtime = 1;
		s->etag_use_size  = 1;
		s->range_requests = 1;
		s->force_lowercase_filenames = (i == 0) ? 2 : 0; /* we wan't to detect later if user changed this for global section */
		s->global_kbytes_per_second = 0;
		s->global_bytes_per_second_cnt = 0;
		s->global_bytes_per_second_cnt_ptr = &s->global_bytes_per_second_cnt;
		s->ssl_verifyclient = 0;
		s->ssl_verifyclient_enforce = 1;
		s->ssl_verifyclient_username = buffer_init();
		s->ssl_verifyclient_depth = 9;
		s->ssl_verifyclient_export_cert = 0;
		s->ssl_disable_client_renegotiation = 1;
		s->ssl_read_ahead = (0 == i ? 1 : srv->config_storage[0]->ssl_read_ahead);
		s->listen_backlog = (0 == i ? 1024 : srv->config_storage[0]->listen_backlog);
		s->stream_request_body = 0;
		s->stream_response_body = 0;

		/* all T_CONFIG_SCOPE_CONNECTION options */
		cv[2].destination = s->errorfile_prefix;
		cv[7].destination = s->server_tag;
		cv[8].destination = &(s->use_ipv6);

		cv[12].destination = &(s->max_request_size);
		cv[14].destination = s->document_root;
		cv[15].destination = &(s->force_lowercase_filenames);
		cv[16].destination = &(s->log_condition_handling);
		cv[17].destination = &(s->max_keep_alive_requests);
		cv[18].destination = s->server_name;
		cv[19].destination = &(s->max_keep_alive_idle);

		cv[20].destination = &(s->max_read_idle);
		cv[21].destination = &(s->max_write_idle);
		cv[22].destination = s->error_handler;
#ifdef HAVE_LSTAT
		cv[24].destination = &(s->follow_symlink);
#endif
		cv[25].destination = &(s->global_kbytes_per_second);
		cv[26].destination = &(s->kbytes_per_second);
		cv[27].destination = &(s->use_xattr);
		cv[28].destination = s->mimetypes;
		cv[29].destination = s->ssl_pemfile;

		cv[30].destination = &(s->ssl_enabled);
		cv[31].destination = &(s->log_file_not_found);
		cv[32].destination = &(s->log_request_handling);
		cv[33].destination = &(s->log_response_header);
		cv[34].destination = &(s->log_request_header);
		cv[35].destination = &(s->log_ssl_noise);
		cv[36].destination = &(s->allow_http11);
		cv[39].destination = s->ssl_ca_file;

		cv[41].destination = &(s->range_requests);
		cv[47].destination = s->ssl_cipher_list;
		cv[48].destination = &(s->ssl_use_sslv2);
		cv[49].destination = &(s->etag_use_inode);

		cv[50].destination = &(s->etag_use_mtime);
		cv[51].destination = &(s->etag_use_size);
		cv[53].destination = &(s->log_timeouts);
		cv[54].destination = &(s->defer_accept);
		cv[56].destination = &(s->ssl_verifyclient);
		cv[57].destination = &(s->ssl_verifyclient_enforce);
		cv[58].destination = &(s->ssl_verifyclient_depth);
		cv[59].destination = s->ssl_verifyclient_username;

		cv[60].destination = &(s->ssl_verifyclient_export_cert);
		cv[61].destination = &(s->set_v6only);
		cv[62].destination = &(s->ssl_use_sslv3);
		cv[63].destination = s->ssl_dh_file;
		cv[64].destination = s->ssl_ec_curve;
		cv[65].destination = &(s->ssl_disable_client_renegotiation);
		cv[66].destination = &(s->ssl_honor_cipher_order);
		cv[67].destination = &(s->ssl_empty_fragments);
		cv[70].destination = &(s->listen_backlog);
		cv[71].destination = s->error_handler_404;
	      #if defined(__FreeBSD__) || defined(__NetBSD__) \
	       || defined(__OpenBSD__) || defined(__DragonFly__)
		cv[75].destination = s->bsd_accept_filter;
	      #endif
		cv[76].destination = &(s->stream_request_body);
		cv[77].destination = &(s->stream_response_body);
		cv[79].destination = &(s->ssl_read_ahead);

		srv->config_storage[i] = s;

		if (0 != (ret = config_insert_values_global(srv, config->value, cv, i == 0 ? T_CONFIG_SCOPE_SERVER : T_CONFIG_SCOPE_CONNECTION))) {
			break;
		}

		if (s->stream_request_body & FDEVENT_STREAM_REQUEST_BUFMIN) {
			s->stream_request_body |= FDEVENT_STREAM_REQUEST;
		}
		if (s->stream_response_body & FDEVENT_STREAM_RESPONSE_BUFMIN) {
			s->stream_response_body |= FDEVENT_STREAM_RESPONSE;
		}
	}

	{
		specific_config *s = srv->config_storage[0];
		s->http_parseopts= /*(global, but stored in con->conf.http_parseopts)*/
		   (srv->srvconf.http_header_strict  ?(HTTP_PARSEOPT_HEADER_STRICT) :0)
		  |(srv->srvconf.http_host_strict    ?(HTTP_PARSEOPT_HOST_STRICT
		                                      |HTTP_PARSEOPT_HOST_NORMALIZE):0)
		  |(srv->srvconf.http_host_normalize ?(HTTP_PARSEOPT_HOST_NORMALIZE):0);
	}

	if (buffer_string_is_empty(stat_cache_string)) {
		srv->srvconf.stat_cache_engine = STAT_CACHE_ENGINE_SIMPLE;
	} else if (buffer_is_equal_string(stat_cache_string, CONST_STR_LEN("simple"))) {
		srv->srvconf.stat_cache_engine = STAT_CACHE_ENGINE_SIMPLE;
#ifdef HAVE_FAM_H
	} else if (buffer_is_equal_string(stat_cache_string, CONST_STR_LEN("fam"))) {
		srv->srvconf.stat_cache_engine = STAT_CACHE_ENGINE_FAM;
#endif
	} else if (buffer_is_equal_string(stat_cache_string, CONST_STR_LEN("disable"))) {
		srv->srvconf.stat_cache_engine = STAT_CACHE_ENGINE_NONE;
	} else {
		log_error_write(srv, __FILE__, __LINE__, "sb",
				"server.stat-cache-engine can be one of \"disable\", \"simple\","
#ifdef HAVE_FAM_H
				" \"fam\","
#endif
				" but not:", stat_cache_string);
		ret = HANDLER_ERROR;
	}

	buffer_free(stat_cache_string);

	{
		data_string *ds;
		int prepend_mod_indexfile = 1;
		int append_mod_dirlisting = 1;
		int append_mod_staticfile = 1;
		int append_mod_authn_file = 1;
		int append_mod_authn_ldap = 1;
		int append_mod_authn_mysql = 1;
		int contains_mod_auth = 0;

		/* prepend default modules */
		for (i = 0; i < srv->srvconf.modules->used; i++) {
			ds = (data_string *)srv->srvconf.modules->data[i];

			if (buffer_is_equal_string(ds->value, CONST_STR_LEN("mod_indexfile"))) {
				prepend_mod_indexfile = 0;
			}

			if (buffer_is_equal_string(ds->value, CONST_STR_LEN("mod_staticfile"))) {
				append_mod_staticfile = 0;
			}

			if (buffer_is_equal_string(ds->value, CONST_STR_LEN("mod_dirlisting"))) {
				append_mod_dirlisting = 0;
			}

			if (buffer_is_equal_string(ds->value, CONST_STR_LEN("mod_authn_file"))) {
				append_mod_authn_file = 0;
			}

			if (buffer_is_equal_string(ds->value, CONST_STR_LEN("mod_authn_ldap"))) {
				append_mod_authn_ldap = 0;
			}

			if (buffer_is_equal_string(ds->value, CONST_STR_LEN("mod_authn_mysql"))) {
				append_mod_authn_mysql = 0;
			}

			if (buffer_is_equal_string(ds->value, CONST_STR_LEN("mod_auth"))) {
				contains_mod_auth = 1;
			}

			if (0 == prepend_mod_indexfile &&
			    0 == append_mod_dirlisting &&
			    0 == append_mod_staticfile &&
			    0 == append_mod_authn_file &&
			    0 == append_mod_authn_ldap &&
			    0 == append_mod_authn_mysql &&
			    1 == contains_mod_auth) {
				break;
			}
		}

		if (prepend_mod_indexfile) {
			/* mod_indexfile has to be loaded before mod_fastcgi and friends */
			array *modules = array_init();

			ds = data_string_init();
			buffer_copy_string_len(ds->value, CONST_STR_LEN("mod_indexfile"));
			array_insert_unique(modules, (data_unset *)ds);

			for (i = 0; i < srv->srvconf.modules->used; i++) {
				data_unset *du = srv->srvconf.modules->data[i];
				array_insert_unique(modules, du->copy(du));
			}

			array_free(srv->srvconf.modules);
			srv->srvconf.modules = modules;
		}

		/* append default modules */
		if (append_mod_dirlisting) {
			ds = data_string_init();
			buffer_copy_string_len(ds->value, CONST_STR_LEN("mod_dirlisting"));
			array_insert_unique(srv->srvconf.modules, (data_unset *)ds);
		}

		if (append_mod_staticfile) {
			ds = data_string_init();
			buffer_copy_string_len(ds->value, CONST_STR_LEN("mod_staticfile"));
			array_insert_unique(srv->srvconf.modules, (data_unset *)ds);
		}

		/* mod_auth.c,http_auth.c auth backends were split into separate modules
		 * Automatically load auth backend modules for compatibility with
		 * existing lighttpd 1.4.x configs */
		if (contains_mod_auth) {
			if (append_mod_authn_file) {
				ds = data_string_init();
				buffer_copy_string_len(ds->value, CONST_STR_LEN("mod_authn_file"));
				array_insert_unique(srv->srvconf.modules, (data_unset *)ds);
			}
			if (append_mod_authn_ldap) {
			      #if defined(HAVE_LDAP_H) && defined(HAVE_LBER_H) && defined(HAVE_LIBLDAP) && defined(HAVE_LIBLBER)
				config_warn_authn_module(srv, "ldap");
			      #endif
			}
			if (append_mod_authn_mysql) {
			      #if defined(HAVE_MYSQL)
				config_warn_authn_module(srv, "mysql");
			      #endif
			}
		}
	}

	return ret;

}


#define PATCH(x) con->conf.x = s->x
int config_setup_connection(server *srv, connection *con) {
	specific_config *s = srv->config_storage[0];

	PATCH(http_parseopts);

	PATCH(allow_http11);
	PATCH(mimetypes);
	PATCH(document_root);
	PATCH(high_precision_timestamps);
	PATCH(max_keep_alive_requests);
	PATCH(max_keep_alive_idle);
	PATCH(max_read_idle);
	PATCH(max_write_idle);
	PATCH(max_request_size);
	PATCH(use_xattr);
	PATCH(error_handler);
	PATCH(error_handler_404);
	PATCH(errorfile_prefix);
#ifdef HAVE_LSTAT
	PATCH(follow_symlink);
#endif
	PATCH(server_tag);
	PATCH(kbytes_per_second);
	PATCH(global_kbytes_per_second);
	PATCH(global_bytes_per_second_cnt);

	con->conf.global_bytes_per_second_cnt_ptr = &s->global_bytes_per_second_cnt;
	buffer_copy_buffer(con->server_name, s->server_name);

	PATCH(log_request_header);
	PATCH(log_response_header);
	PATCH(log_request_handling);
	PATCH(log_condition_handling);
	PATCH(log_file_not_found);
	PATCH(log_ssl_noise);
	PATCH(log_timeouts);

	PATCH(range_requests);
	PATCH(force_lowercase_filenames);
	/*PATCH(listen_backlog);*//*(not necessary; used only at startup)*/
	PATCH(stream_request_body);
	PATCH(stream_response_body);

	PATCH(ssl_enabled);

	PATCH(ssl_pemfile);
#ifdef USE_OPENSSL
	PATCH(ssl_pemfile_x509);
	PATCH(ssl_pemfile_pkey);
#endif
	PATCH(ssl_ca_file);
#ifdef USE_OPENSSL
	PATCH(ssl_ca_file_cert_names);
#endif
	PATCH(ssl_cipher_list);
	PATCH(ssl_dh_file);
	PATCH(ssl_ec_curve);
	PATCH(ssl_honor_cipher_order);
	PATCH(ssl_empty_fragments);
	PATCH(ssl_use_sslv2);
	PATCH(ssl_use_sslv3);
	PATCH(etag_use_inode);
	PATCH(etag_use_mtime);
	PATCH(etag_use_size);

	PATCH(ssl_verifyclient);
	PATCH(ssl_verifyclient_enforce);
	PATCH(ssl_verifyclient_depth);
	PATCH(ssl_verifyclient_username);
	PATCH(ssl_verifyclient_export_cert);
	PATCH(ssl_disable_client_renegotiation);
	PATCH(ssl_read_ahead);

	return 0;
}

int config_patch_connection(server *srv, connection *con) {
	size_t i, j;

	/* skip the first, the global context */
	for (i = 1; i < srv->config_context->used; i++) {
		data_config *dc = (data_config *)srv->config_context->data[i];
		specific_config *s = srv->config_storage[i];

		/* condition didn't match */
		if (!config_check_cond(srv, con, dc)) continue;

		/* merge config */
		for (j = 0; j < dc->value->used; j++) {
			data_unset *du = dc->value->data[j];

			if (buffer_is_equal_string(du->key, CONST_STR_LEN("server.document-root"))) {
				PATCH(document_root);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("server.range-requests"))) {
				PATCH(range_requests);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("server.error-handler"))) {
				PATCH(error_handler);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("server.error-handler-404"))) {
				PATCH(error_handler_404);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("server.errorfile-prefix"))) {
				PATCH(errorfile_prefix);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("mimetype.assign"))) {
				PATCH(mimetypes);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("server.max-keep-alive-requests"))) {
				PATCH(max_keep_alive_requests);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("server.max-keep-alive-idle"))) {
				PATCH(max_keep_alive_idle);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("server.max-write-idle"))) {
				PATCH(max_write_idle);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("server.max-read-idle"))) {
				PATCH(max_read_idle);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("server.max-request-size"))) {
				PATCH(max_request_size);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("mimetype.use-xattr"))) {
				PATCH(use_xattr);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("etag.use-inode"))) {
				PATCH(etag_use_inode);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("etag.use-mtime"))) {
				PATCH(etag_use_mtime);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("etag.use-size"))) {
				PATCH(etag_use_size);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssl.pemfile"))) {
				PATCH(ssl_pemfile);
#ifdef USE_OPENSSL
				PATCH(ssl_pemfile_x509);
				PATCH(ssl_pemfile_pkey);
#endif
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssl.ca-file"))) {
				PATCH(ssl_ca_file);
#ifdef USE_OPENSSL
				PATCH(ssl_ca_file_cert_names);
#endif
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssl.honor-cipher-order"))) {
				PATCH(ssl_honor_cipher_order);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssl.empty-fragments"))) {
				PATCH(ssl_empty_fragments);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssl.use-sslv2"))) {
				PATCH(ssl_use_sslv2);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssl.use-sslv3"))) {
				PATCH(ssl_use_sslv3);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssl.cipher-list"))) {
				PATCH(ssl_cipher_list);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssl.engine"))) {
				PATCH(ssl_enabled);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssl.dh-file"))) {
				PATCH(ssl_dh_file);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssl.ec-curve"))) {
				PATCH(ssl_ec_curve);
#ifdef HAVE_LSTAT
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("server.follow-symlink"))) {
				PATCH(follow_symlink);
#endif
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("server.name"))) {
				buffer_copy_buffer(con->server_name, s->server_name);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("server.tag"))) {
				PATCH(server_tag);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("server.stream-request-body"))) {
				PATCH(stream_request_body);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("server.stream-response-body"))) {
				PATCH(stream_response_body);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("connection.kbytes-per-second"))) {
				PATCH(kbytes_per_second);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("debug.log-request-handling"))) {
				PATCH(log_request_handling);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("debug.log-request-header"))) {
				PATCH(log_request_header);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("debug.log-response-header"))) {
				PATCH(log_response_header);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("debug.log-condition-handling"))) {
				PATCH(log_condition_handling);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("debug.log-file-not-found"))) {
				PATCH(log_file_not_found);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("debug.log-ssl-noise"))) {
				PATCH(log_ssl_noise);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("debug.log-timeouts"))) {
				PATCH(log_timeouts);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("server.protocol-http11"))) {
				PATCH(allow_http11);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("server.force-lowercase-filenames"))) {
				PATCH(force_lowercase_filenames);
		      #if 0 /*(not necessary; used only at startup)*/
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("server.listen-backlog"))) {
				PATCH(listen_backlog);
		      #endif
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("server.kbytes-per-second"))) {
				PATCH(global_kbytes_per_second);
				PATCH(global_bytes_per_second_cnt);
				con->conf.global_bytes_per_second_cnt_ptr = &s->global_bytes_per_second_cnt;
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssl.verifyclient.activate"))) {
				PATCH(ssl_verifyclient);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssl.verifyclient.enforce"))) {
				PATCH(ssl_verifyclient_enforce);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssl.verifyclient.depth"))) {
				PATCH(ssl_verifyclient_depth);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssl.verifyclient.username"))) {
				PATCH(ssl_verifyclient_username);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssl.verifyclient.exportcert"))) {
				PATCH(ssl_verifyclient_export_cert);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssl.disable-client-renegotiation"))) {
				PATCH(ssl_disable_client_renegotiation);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("ssl.read-ahead"))) {
				PATCH(ssl_read_ahead);
			}
		}
	}

		con->etag_flags = (con->conf.etag_use_mtime ? ETAG_USE_MTIME : 0) |
				  (con->conf.etag_use_inode ? ETAG_USE_INODE : 0) |
				  (con->conf.etag_use_size  ? ETAG_USE_SIZE  : 0);

	return 0;
}
#undef PATCH

typedef struct {
	int foo;
	int bar;

	const buffer *source;
	const char *input;
	size_t offset;
	size_t size;

	int line_pos;
	int line;

	int in_key;
	int in_brace;
	int in_cond;
} tokenizer_t;

#if 0
static int tokenizer_open(server *srv, tokenizer_t *t, buffer *basedir, const char *fn) {
	if (buffer_string_is_empty(basedir) ||
			(fn[0] == '/' || fn[0] == '\\') ||
			(fn[0] == '.' && (fn[1] == '/' || fn[1] == '\\'))) {
		t->file = buffer_init_string(fn);
	} else {
		t->file = buffer_init_buffer(basedir);
		buffer_append_string(t->file, fn);
	}

	if (0 != stream_open(&(t->s), t->file)) {
		log_error_write(srv, __FILE__, __LINE__, "sbss",
				"opening configfile ", t->file, "failed:", strerror(errno));
		buffer_free(t->file);
		return -1;
	}

	t->input = t->s.start;
	t->offset = 0;
	t->size = t->s.size;
	t->line = 1;
	t->line_pos = 1;

	t->in_key = 1;
	t->in_brace = 0;
	t->in_cond = 0;
	return 0;
}

static int tokenizer_close(server *srv, tokenizer_t *t) {
	UNUSED(srv);

	buffer_free(t->file);
	return stream_close(&(t->s));
}
#endif
static int config_skip_newline(tokenizer_t *t) {
	int skipped = 1;
	force_assert(t->input[t->offset] == '\r' || t->input[t->offset] == '\n');
	if (t->input[t->offset] == '\r' && t->input[t->offset + 1] == '\n') {
		skipped ++;
		t->offset ++;
	}
	t->offset ++;
	return skipped;
}

static int config_skip_comment(tokenizer_t *t) {
	int i;
	force_assert(t->input[t->offset] == '#');
	for (i = 1; t->input[t->offset + i] &&
	     (t->input[t->offset + i] != '\n' && t->input[t->offset + i] != '\r');
	     i++);
	t->offset += i;
	return i;
}

static int config_tokenizer(server *srv, tokenizer_t *t, int *token_id, buffer *token) {
	int tid = 0;
	size_t i;

	for (tid = 0; tid == 0 && t->offset < t->size && t->input[t->offset] ; ) {
		char c = t->input[t->offset];
		const char *start = NULL;

		switch (c) {
		case '=':
			if (t->in_brace) {
				if (t->input[t->offset + 1] == '>') {
					t->offset += 2;

					buffer_copy_string_len(token, CONST_STR_LEN("=>"));

					tid = TK_ARRAY_ASSIGN;
				} else {
					log_error_write(srv, __FILE__, __LINE__, "sbsdsds",
							"source:", t->source,
							"line:", t->line, "pos:", t->line_pos,
							"use => for assignments in arrays");
					return -1;
				}
			} else if (t->in_cond) {
				if (t->input[t->offset + 1] == '=') {
					t->offset += 2;

					buffer_copy_string_len(token, CONST_STR_LEN("=="));

					tid = TK_EQ;
				} else if (t->input[t->offset + 1] == '~') {
					t->offset += 2;

					buffer_copy_string_len(token, CONST_STR_LEN("=~"));

					tid = TK_MATCH;
				} else {
					log_error_write(srv, __FILE__, __LINE__, "sbsdsds",
							"source:", t->source,
							"line:", t->line, "pos:", t->line_pos,
							"only =~ and == are allowed in the condition");
					return -1;
				}
				t->in_key = 1;
				t->in_cond = 0;
			} else if (t->in_key) {
				tid = TK_ASSIGN;

				buffer_copy_string_len(token, t->input + t->offset, 1);

				t->offset++;
				t->line_pos++;
			} else {
				log_error_write(srv, __FILE__, __LINE__, "sbsdsds",
						"source:", t->source,
						"line:", t->line, "pos:", t->line_pos,
						"unexpected equal-sign: =");
				return -1;
			}

			break;
		case '!':
			if (t->in_cond) {
				if (t->input[t->offset + 1] == '=') {
					t->offset += 2;

					buffer_copy_string_len(token, CONST_STR_LEN("!="));

					tid = TK_NE;
				} else if (t->input[t->offset + 1] == '~') {
					t->offset += 2;

					buffer_copy_string_len(token, CONST_STR_LEN("!~"));

					tid = TK_NOMATCH;
				} else {
					log_error_write(srv, __FILE__, __LINE__, "sbsdsds",
							"source:", t->source,
							"line:", t->line, "pos:", t->line_pos,
							"only !~ and != are allowed in the condition");
					return -1;
				}
				t->in_key = 1;
				t->in_cond = 0;
			} else {
				log_error_write(srv, __FILE__, __LINE__, "sbsdsds",
						"source:", t->source,
						"line:", t->line, "pos:", t->line_pos,
						"unexpected exclamation-marks: !");
				return -1;
			}

			break;
		case '\t':
		case ' ':
			t->offset++;
			t->line_pos++;
			break;
		case '\n':
		case '\r':
			if (t->in_brace == 0) {
				int done = 0;
				while (!done && t->offset < t->size) {
					switch (t->input[t->offset]) {
					case '\r':
					case '\n':
						config_skip_newline(t);
						t->line_pos = 1;
						t->line++;
						break;

					case '#':
						t->line_pos += config_skip_comment(t);
						break;

					case '\t':
					case ' ':
						t->offset++;
						t->line_pos++;
						break;

					default:
						done = 1;
					}
				}
				t->in_key = 1;
				tid = TK_EOL;
				buffer_copy_string_len(token, CONST_STR_LEN("(EOL)"));
			} else {
				config_skip_newline(t);
				t->line_pos = 1;
				t->line++;
			}
			break;
		case ',':
			if (t->in_brace > 0) {
				tid = TK_COMMA;

				buffer_copy_string_len(token, CONST_STR_LEN("(COMMA)"));
			}

			t->offset++;
			t->line_pos++;
			break;
		case '"':
			/* search for the terminating " */
			start = t->input + t->offset + 1;
			buffer_copy_string_len(token, CONST_STR_LEN(""));

			for (i = 1; t->input[t->offset + i]; i++) {
				if (t->input[t->offset + i] == '\\' &&
				    t->input[t->offset + i + 1] == '"') {

					buffer_append_string_len(token, start, t->input + t->offset + i - start);

					start = t->input + t->offset + i + 1;

					/* skip the " */
					i++;
					continue;
				}


				if (t->input[t->offset + i] == '"') {
					tid = TK_STRING;

					buffer_append_string_len(token, start, t->input + t->offset + i - start);

					break;
				}
			}

			if (t->input[t->offset + i] == '\0') {
				/* ERROR */

				log_error_write(srv, __FILE__, __LINE__, "sbsdsds",
						"source:", t->source,
						"line:", t->line, "pos:", t->line_pos,
						"missing closing quote");

				return -1;
			}

			t->offset += i + 1;
			t->line_pos += i + 1;

			break;
		case '(':
			t->offset++;
			t->in_brace++;

			tid = TK_LPARAN;

			buffer_copy_string_len(token, CONST_STR_LEN("("));
			break;
		case ')':
			t->offset++;
			t->in_brace--;

			tid = TK_RPARAN;

			buffer_copy_string_len(token, CONST_STR_LEN(")"));
			break;
		case '$':
			t->offset++;

			tid = TK_DOLLAR;
			t->in_cond = 1;
			t->in_key = 0;

			buffer_copy_string_len(token, CONST_STR_LEN("$"));

			break;

		case '+':
			if (t->input[t->offset + 1] == '=') {
				t->offset += 2;
				buffer_copy_string_len(token, CONST_STR_LEN("+="));
				tid = TK_APPEND;
			} else {
				t->offset++;
				tid = TK_PLUS;
				buffer_copy_string_len(token, CONST_STR_LEN("+"));
			}
			break;

		case '{':
			t->offset++;

			tid = TK_LCURLY;

			buffer_copy_string_len(token, CONST_STR_LEN("{"));

			break;

		case '}':
			t->offset++;

			tid = TK_RCURLY;

			buffer_copy_string_len(token, CONST_STR_LEN("}"));

			break;

		case '[':
			t->offset++;

			tid = TK_LBRACKET;

			buffer_copy_string_len(token, CONST_STR_LEN("["));

			break;

		case ']':
			t->offset++;

			tid = TK_RBRACKET;

			buffer_copy_string_len(token, CONST_STR_LEN("]"));

			break;
		case '#':
			t->line_pos += config_skip_comment(t);

			break;
		default:
			if (t->in_cond) {
				for (i = 0; t->input[t->offset + i] &&
				     (isalpha((unsigned char)t->input[t->offset + i])
				      ); i++);

				if (i && t->input[t->offset + i]) {
					tid = TK_SRVVARNAME;
					buffer_copy_string_len(token, t->input + t->offset, i);

					t->offset += i;
					t->line_pos += i;
				} else {
					/* ERROR */
					log_error_write(srv, __FILE__, __LINE__, "sbsdsds",
							"source:", t->source,
							"line:", t->line, "pos:", t->line_pos,
							"invalid character in condition");
					return -1;
				}
			} else if (isdigit((unsigned char)c)) {
				/* take all digits */
				for (i = 0; t->input[t->offset + i] && isdigit((unsigned char)t->input[t->offset + i]);  i++);

				/* was there it least a digit ? */
				if (i) {
					tid = TK_INTEGER;

					buffer_copy_string_len(token, t->input + t->offset, i);

					t->offset += i;
					t->line_pos += i;
				}
			} else {
				/* the key might consist of [-.0-9a-z] */
				for (i = 0; t->input[t->offset + i] &&
				     (isalnum((unsigned char)t->input[t->offset + i]) ||
				      t->input[t->offset + i] == '.' ||
				      t->input[t->offset + i] == '_' || /* for env.* */
				      t->input[t->offset + i] == '-'
				      ); i++);

				if (i && t->input[t->offset + i]) {
					buffer_copy_string_len(token, t->input + t->offset, i);

					if (strcmp(token->ptr, "include") == 0) {
						tid = TK_INCLUDE;
					} else if (strcmp(token->ptr, "include_shell") == 0) {
						tid = TK_INCLUDE_SHELL;
					} else if (strcmp(token->ptr, "global") == 0) {
						tid = TK_GLOBAL;
					} else if (strcmp(token->ptr, "else") == 0) {
						tid = TK_ELSE;
					} else {
						tid = TK_LKEY;
					}

					t->offset += i;
					t->line_pos += i;
				} else {
					/* ERROR */
					log_error_write(srv, __FILE__, __LINE__, "sbsdsds",
							"source:", t->source,
							"line:", t->line, "pos:", t->line_pos,
							"invalid character in variable name");
					return -1;
				}
			}
			break;
		}
	}

	if (tid) {
		*token_id = tid;
#if 0
		log_error_write(srv, __FILE__, __LINE__, "sbsdsdbdd",
				"source:", t->source,
				"line:", t->line, "pos:", t->line_pos,
				token, token->used - 1, tid);
#endif

		return 1;
	} else if (t->offset < t->size) {
		fprintf(stderr, "%s.%d: %d, %s\n",
			__FILE__, __LINE__,
			tid, token->ptr);
	}
	return 0;
}

static int config_parse(server *srv, config_t *context, tokenizer_t *t) {
	void *pParser;
	int token_id;
	buffer *token, *lasttoken;
	int ret;

	pParser = configparserAlloc( malloc );
	force_assert(pParser);
	lasttoken = buffer_init();
	token = buffer_init();
	while((1 == (ret = config_tokenizer(srv, t, &token_id, token))) && context->ok) {
		buffer_copy_buffer(lasttoken, token);
		configparser(pParser, token_id, token, context);

		token = buffer_init();
	}
	buffer_free(token);

	if (ret != -1 && context->ok) {
		/* add an EOL at EOF, better than say sorry */
		configparser(pParser, TK_EOL, buffer_init_string("(EOL)"), context);
		if (context->ok) {
			configparser(pParser, 0, NULL, context);
		}
	}
	configparserFree(pParser, free);

	if (ret == -1) {
		log_error_write(srv, __FILE__, __LINE__, "sb",
				"configfile parser failed at:", lasttoken);
	} else if (context->ok == 0) {
		log_error_write(srv, __FILE__, __LINE__, "sbsdsdsb",
				"source:", t->source,
				"line:", t->line, "pos:", t->line_pos,
				"parser failed somehow near here:", lasttoken);
		ret = -1;
	}
	buffer_free(lasttoken);

	return ret == -1 ? -1 : 0;
}

static int tokenizer_init(tokenizer_t *t, const buffer *source, const char *input, size_t size) {

	t->source = source;
	t->input = input;
	t->size = size;
	t->offset = 0;
	t->line = 1;
	t->line_pos = 1;

	t->in_key = 1;
	t->in_brace = 0;
	t->in_cond = 0;
	return 0;
}

static int config_parse_file_stream(server *srv, config_t *context, const buffer *filename) {
	tokenizer_t t;
	stream s;
	int ret;

	if (0 != stream_open(&s, filename)) {
		log_error_write(srv, __FILE__, __LINE__, "sbss",
				"opening configfile ", filename, "failed:", strerror(errno));
		return -1;
	} else {
		tokenizer_init(&t, filename, s.start, s.size);
		ret = config_parse(srv, context, &t);
	}

	stream_close(&s);
	return ret;
}

int config_parse_file(server *srv, config_t *context, const char *fn) {
	buffer *filename;
	size_t i;
	int ret = -1;
      #ifdef GLOB_BRACE
	int flags = GLOB_BRACE;
      #else
	int flags = 0;
      #endif
	glob_t gl;

	if ((fn[0] == '/' || fn[0] == '\\') ||
	    (fn[0] == '.' && (fn[1] == '/' || fn[1] == '\\')) ||
	    (fn[0] == '.' && fn[1] == '.' && (fn[2] == '/' || fn[2] == '\\'))) {
		filename = buffer_init_string(fn);
	} else {
		filename = buffer_init_buffer(context->basedir);
		buffer_append_string(filename, fn);
	}

	switch (glob(filename->ptr, flags, NULL, &gl)) {
	case 0:
		for (i = 0; i < gl.gl_pathc; ++i) {
			buffer_copy_string(filename, gl.gl_pathv[i]);
			ret = config_parse_file_stream(srv, context, filename);
			if (0 != ret) break;
		}
		globfree(&gl);
		break;
	case GLOB_NOMATCH:
		if (filename->ptr[strcspn(filename->ptr, "*?[]{}")] != '\0') { /*(contains glob metachars)*/
			ret = 0; /* not an error if no files match glob pattern */
		}
		else {
			log_error_write(srv, __FILE__, __LINE__, "sb", "include file not found: ", filename);
		}
		break;
	case GLOB_ABORTED:
	case GLOB_NOSPACE:
		log_error_write(srv, __FILE__, __LINE__, "sbss", "glob()", filename, "failed:", strerror(errno));
		break;
	}

	buffer_free(filename);
	return ret;
}

static char* getCWD(void) {
	char *s, *s1;
	size_t len;
#ifdef PATH_MAX
	len = PATH_MAX;
#else
	len = 4096;
#endif

	s = malloc(len);
	if (!s) return NULL;
	while (NULL == getcwd(s, len)) {
		if (errno != ERANGE || SSIZE_MAX - len < len) {
			free(s);
			return NULL;
		}
		len *= 2;
		s1 = realloc(s, len);
		if (!s1) {
			free(s);
			return NULL;
		}
		s = s1;
	}
	return s;
}

int config_parse_cmd(server *srv, config_t *context, const char *cmd) {
	tokenizer_t t;
	int ret;
	buffer *source;
	buffer *out;
	char *oldpwd;

	if (NULL == (oldpwd = getCWD())) {
		log_error_write(srv, __FILE__, __LINE__, "s",
			"cannot get cwd", strerror(errno));
		return -1;
	}

	if (!buffer_string_is_empty(context->basedir)) {
		if (0 != chdir(context->basedir->ptr)) {
			log_error_write(srv, __FILE__, __LINE__, "sbs",
				"cannot change directory to", context->basedir, strerror(errno));
			free(oldpwd);
			return -1;
		}
	}

	source = buffer_init_string(cmd);
	out = buffer_init();

	if (0 != proc_open_buffer(cmd, NULL, out, NULL)) {
		log_error_write(srv, __FILE__, __LINE__, "sbss",
			"opening", source, "failed:", strerror(errno));
		ret = -1;
	} else {
		tokenizer_init(&t, source, CONST_BUF_LEN(out));
		ret = config_parse(srv, context, &t);
	}

	buffer_free(source);
	buffer_free(out);
	if (0 != chdir(oldpwd)) {
		log_error_write(srv, __FILE__, __LINE__, "sss",
			"cannot change directory to", oldpwd, strerror(errno));
		free(oldpwd);
		return -1;
	}
	free(oldpwd);
	return ret;
}

static void context_init(server *srv, config_t *context) {
	context->srv = srv;
	context->ok = 1;
	vector_config_weak_init(&context->configs_stack);
	context->basedir = buffer_init();
}

static void context_free(config_t *context) {
	vector_config_weak_clear(&context->configs_stack);
	buffer_free(context->basedir);
}

int config_read(server *srv, const char *fn) {
	config_t context;
	data_config *dc;
	data_integer *dpid;
	data_string *dcwd;
	int ret;
	char *pos;
	buffer *filename;

	context_init(srv, &context);
	context.all_configs = srv->config_context;

#ifdef __WIN32
	pos = strrchr(fn, '\\');
#else
	pos = strrchr(fn, '/');
#endif
	if (pos) {
		buffer_copy_string_len(context.basedir, fn, pos - fn + 1);
	}

	dc = data_config_init();
	buffer_copy_string_len(dc->key, CONST_STR_LEN("global"));

	force_assert(context.all_configs->used == 0);
	dc->context_ndx = context.all_configs->used;
	array_insert_unique(context.all_configs, (data_unset *)dc);
	context.current = dc;

	/* default context */
	dpid = data_integer_init();
	dpid->value = getpid();
	buffer_copy_string_len(dpid->key, CONST_STR_LEN("var.PID"));
	array_insert_unique(dc->value, (data_unset *)dpid);

	dcwd = data_string_init();
	buffer_string_prepare_copy(dcwd->value, 1023);
	if (NULL != getcwd(dcwd->value->ptr, dcwd->value->size - 1)) {
		buffer_commit(dcwd->value, strlen(dcwd->value->ptr));
		buffer_copy_string_len(dcwd->key, CONST_STR_LEN("var.CWD"));
		array_insert_unique(dc->value, (data_unset *)dcwd);
	} else {
		dcwd->free((data_unset*) dcwd);
	}

	filename = buffer_init_string(fn);
	ret = config_parse_file_stream(srv, &context, filename);
	buffer_free(filename);

	/* remains nothing if parser is ok */
	force_assert(!(0 == ret && context.ok && 0 != context.configs_stack.used));
	context_free(&context);

	if (0 != ret) {
		return ret;
	}

	if (0 != config_insert(srv)) {
		return -1;
	}

	return 0;
}

int config_set_defaults(server *srv) {
	size_t i;
	specific_config *s = srv->config_storage[0];
	struct stat st1, st2;

	struct ev_map { fdevent_handler_t et; const char *name; } event_handlers[] =
	{
		/* - epoll is most reliable
		 * - select works everywhere
		 */
#ifdef USE_LINUX_EPOLL
		{ FDEVENT_HANDLER_LINUX_SYSEPOLL, "linux-sysepoll" },
#endif
#ifdef USE_POLL
		{ FDEVENT_HANDLER_POLL,           "poll" },
#endif
#ifdef USE_SELECT
		{ FDEVENT_HANDLER_SELECT,         "select" },
#endif
#ifdef USE_LIBEV
		{ FDEVENT_HANDLER_LIBEV,          "libev" },
#endif
#ifdef USE_SOLARIS_DEVPOLL
		{ FDEVENT_HANDLER_SOLARIS_DEVPOLL,"solaris-devpoll" },
#endif
#ifdef USE_SOLARIS_PORT
		{ FDEVENT_HANDLER_SOLARIS_PORT,   "solaris-eventports" },
#endif
#ifdef USE_FREEBSD_KQUEUE
		{ FDEVENT_HANDLER_FREEBSD_KQUEUE, "freebsd-kqueue" },
		{ FDEVENT_HANDLER_FREEBSD_KQUEUE, "kqueue" },
#endif
		{ FDEVENT_HANDLER_UNSET,          NULL }
	};

	if (!buffer_string_is_empty(srv->srvconf.changeroot)) {
		if (-1 == stat(srv->srvconf.changeroot->ptr, &st1)) {
			log_error_write(srv, __FILE__, __LINE__, "sb",
					"server.chroot doesn't exist:", srv->srvconf.changeroot);
			return -1;
		}
		if (!S_ISDIR(st1.st_mode)) {
			log_error_write(srv, __FILE__, __LINE__, "sb",
					"server.chroot isn't a directory:", srv->srvconf.changeroot);
			return -1;
		}
	}

	if (!srv->srvconf.upload_tempdirs->used) {
		data_string *ds = data_string_init();
		const char *tmpdir = getenv("TMPDIR");
		if (NULL == tmpdir) tmpdir = "/var/tmp";
		buffer_copy_string(ds->value, tmpdir);
		array_insert_unique(srv->srvconf.upload_tempdirs, (data_unset *)ds);
	}

	if (srv->srvconf.upload_tempdirs->used) {
		buffer * const b = srv->tmp_buf;
		size_t len;
		if (!buffer_string_is_empty(srv->srvconf.changeroot)) {
			buffer_copy_buffer(b, srv->srvconf.changeroot);
			buffer_append_slash(b);
		} else {
			buffer_reset(b);
		}
		len = buffer_string_length(b);

		for (i = 0; i < srv->srvconf.upload_tempdirs->used; ++i) {
			const data_string * const ds = (data_string *)srv->srvconf.upload_tempdirs->data[i];
			buffer_string_set_length(b, len); /*(truncate)*/
			buffer_append_string_buffer(b, ds->value);
			if (-1 == stat(b->ptr, &st1)) {
				log_error_write(srv, __FILE__, __LINE__, "sb",
					"server.upload-dirs doesn't exist:", b);
			} else if (!S_ISDIR(st1.st_mode)) {
				log_error_write(srv, __FILE__, __LINE__, "sb",
					"server.upload-dirs isn't a directory:", b);
			}
		}
	}

	chunkqueue_set_tempdirs_default(
		srv->srvconf.upload_tempdirs,
		srv->srvconf.upload_temp_file_size);

	if (buffer_string_is_empty(s->document_root)) {
		log_error_write(srv, __FILE__, __LINE__, "s",
				"a default document-root has to be set");

		return -1;
	}

	buffer_copy_buffer(srv->tmp_buf, s->document_root);

	buffer_to_lower(srv->tmp_buf);

	if (2 == s->force_lowercase_filenames) { /* user didn't configure it in global section? */
		s->force_lowercase_filenames = 0; /* default to 0 */

		if (0 == stat(srv->tmp_buf->ptr, &st1)) {
			int is_lower = 0;

			is_lower = buffer_is_equal(srv->tmp_buf, s->document_root);

			/* lower-case existed, check upper-case */
			buffer_copy_buffer(srv->tmp_buf, s->document_root);

			buffer_to_upper(srv->tmp_buf);

			/* we have to handle the special case that upper and lower-casing results in the same filename
			 * as in server.document-root = "/" or "/12345/" */

			if (is_lower && buffer_is_equal(srv->tmp_buf, s->document_root)) {
				/* lower-casing and upper-casing didn't result in
				 * an other filename, no need to stat(),
				 * just assume it is case-sensitive. */

				s->force_lowercase_filenames = 0;
			} else if (0 == stat(srv->tmp_buf->ptr, &st2)) {

				/* upper case exists too, doesn't the FS handle this ? */

				/* upper and lower have the same inode -> case-insensitve FS */

				if (st1.st_ino == st2.st_ino) {
					/* upper and lower have the same inode -> case-insensitve FS */

					s->force_lowercase_filenames = 1;
				}
			}
		}
	}

	if (srv->srvconf.port == 0) {
		srv->srvconf.port = s->ssl_enabled ? 443 : 80;
	}

	if (buffer_string_is_empty(srv->srvconf.event_handler)) {
		/* choose a good default
		 *
		 * the event_handler list is sorted by 'goodness'
		 * taking the first available should be the best solution
		 */
		srv->event_handler = event_handlers[0].et;

		if (FDEVENT_HANDLER_UNSET == srv->event_handler) {
			log_error_write(srv, __FILE__, __LINE__, "s",
					"sorry, there is no event handler for this system");

			return -1;
		}
	} else {
		/*
		 * User override
		 */

		for (i = 0; event_handlers[i].name; i++) {
			if (0 == strcmp(event_handlers[i].name, srv->srvconf.event_handler->ptr)) {
				srv->event_handler = event_handlers[i].et;
				break;
			}
		}

		if (FDEVENT_HANDLER_UNSET == srv->event_handler) {
			log_error_write(srv, __FILE__, __LINE__, "sb",
					"the selected event-handler in unknown or not supported:",
					srv->srvconf.event_handler );

			return -1;
		}
	}

	if (s->ssl_enabled) {
		if (buffer_string_is_empty(s->ssl_pemfile)) {
			/* PEM file is require */

			log_error_write(srv, __FILE__, __LINE__, "s",
					"ssl.pemfile has to be set");
			return -1;
		}

#ifndef USE_OPENSSL
		log_error_write(srv, __FILE__, __LINE__, "s",
				"ssl support is missing, recompile with --with-openssl");

		return -1;
#endif
	}

	return 0;
}
