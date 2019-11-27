/* mod_deflate
 *
 *
 * bug fix on Robert Jakabosky from alphatrade.com's lighttp 1.4.10 mod_deflate patch
 *
 * Bug fix and new features:
 * 1) fix loop bug when content-length is bigger than work-block-size*k
 *
 * -------
 *
 * lighttpd-1.4.26.mod_deflate.patch from
 *   https://redmine.lighttpd.net/projects/1/wiki/Docs_ModDeflate
 *
 * -------
 *
 * Patch further modified in this incarnation.
 *
 * Note: this patch only handles completed responses (con->file_finished);
 *       this patch does not currently handle streaming dynamic responses,
 *       and therefore also does not worry about Transfer-Encoding: chunked
 *       (or having separate con->output_queue for chunked-encoded output)
 *       (or using separate buffers per connection instead of p->tmp_buf)
 *       (or handling interactions with block buffering and write timeouts)
 *
 * Bug fix:
 * - fixed major bug with compressing chunks with offset > 0
 *     x-ref:
 *       "Response breaking in mod_deflate"
 *       https://redmine.lighttpd.net/issues/986
 * - fix broken (in some cases) chunk accounting in deflate_compress_response()
 * - fix broken bzip2
 *     x-ref:
 *       "mod_deflate's bzip2 broken by default"
 *       https://redmine.lighttpd.net/issues/2035
 * - fix mismatch with current chunk interfaces
 *     x-ref:
 *       "Weird things in chunk.c (functions only handling specific cases, unexpected behaviour)"
 *       https://redmine.lighttpd.net/issues/1510
 *
 * Behavior changes from prior patch:
 * - deflate.mimetypes must now be configured to enable compression
 *     deflate.mimetypes = ( )          # compress nothing (disabled; default)
 *     deflate.mimetypes = ( "" )       # compress all mimetypes
 *     deflate.mimetypes = ( "text/" )  # compress text/... mimetypes
 *     x-ref:
 *       "mod_deflate enabled by default"
 *       https://redmine.lighttpd.net/issues/1394
 * - deflate.enabled directive removed (see new behavior of deflate.mimetypes)
 * - deflate.debug removed (was developer debug trace, not end-user debug)
 * - deflate.bzip2 replaced with deflate.allowed-encodings (like mod_compress)
 *     x-ref:
 *       "mod_deflate should allow limiting of compression algorithm from the configuration file"
 *       https://redmine.lighttpd.net/issues/996
 *       "mod_compress disabling methods"
 *       https://redmine.lighttpd.net/issues/1773
 * - deflate.nocompress-url removed since disabling compression for a URL
 *   can now easily be done by setting to a blank list either directive
 *   deflate.accept_encodings = () or deflate.mimetypes = () in a conditional
 *   block, e.g. $HTTP["url"] =~ "....." { deflate.mimetypes = ( ) }
 * - deflate.sync-flush removed; controlled by con->conf.stream_response_body
 *     (though streaming compression not currently implemented in mod_deflate)
 * - inactive directives in this patch (since con->file_finished required)
 *     deflate.work-block-size
 *     deflate.output-buffer-size
 * - remove weak file size check; SIGBUS is trapped, file that shrink will error
 *     x-ref:
 *       "mod_deflate: filesize check is too weak"
 *       https://redmine.lighttpd.net/issues/1512
 * - change default deflate.min-compress-size from 0 to now be 256
 *   http://webmasters.stackexchange.com/questions/31750/what-is-recommended-minimum-object-size-for-gzip-performance-benefits
 *   Apache 2.4 mod_deflate minimum is 68 bytes
 *   Akamai recommends minimum 860 bytes
 *   Google recommends minimum be somewhere in range between 150 and 1024 bytes
 * - deflate.max-compress-size new directive (in kb like compress.max_filesize)
 * - deflate.mem-level removed (too many knobs for little benefit)
 * - deflate.window-size removed (too many knobs for little benefit)
 *
 * Future:
 * - config directives may be changed, renamed, or removed
 *   e.g. A set of reasonable defaults might be chosen
 *        instead of making them configurable.
 *     deflate.min-compress-size
 * - might add deflate.mimetypes-exclude = ( ... ) for list of mimetypes
 *   to avoid compressing, even if a broader deflate.mimetypes matched,
 *   e.g. to compress all "text/" except "text/special".
 * - mod_compress and mod_deflate might merge overlapping feature sets
 *     (mod_compress.cache-dir does not yet have an equivalent in mod_deflate)
 *
 * Implementation notes:
 * - http_chunk_append_mem() used instead of http_chunk_append_buffer()
 *   so that p->tmp_buf can be large and re-used.  This results in an extra copy
 *   of compressed data before data is sent to network, though if the compressed
 *   size is larger than 64k, it ends up being sent to a temporary file on
 *   disk without suffering an extra copy in memory, and without extra chunk
 *   create and destroy.  If this is ever changed to give away buffers, then use
 *   a unique hctx->output buffer per hctx; do not reuse p->tmp_buf across
 *   multiple requests being handled in parallel.
 */
#include "first.h"

#include <sys/types.h>
#include <sys/stat.h>
#include "sys-mmap.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "base.h"
#include "log.h"
#include "buffer.h"
#include "etag.h"
#include "http_chunk.h"
#include "response.h"

#include "plugin.h"

#if defined HAVE_ZLIB_H && defined HAVE_LIBZ
# define USE_ZLIB
# include <zlib.h>
#endif
#ifndef Z_DEFAULT_COMPRESSION
#define Z_DEFAULT_COMPRESSION -1
#endif
#ifndef MAX_WBITS
#define MAX_WBITS 15
#endif

#if defined HAVE_BZLIB_H && defined HAVE_LIBBZ2
# define USE_BZ2LIB
/* we don't need stdio interface */
# define BZ_NO_STDIO
# include <bzlib.h>
#endif

#if defined HAVE_SYS_MMAN_H && defined HAVE_MMAP && defined ENABLE_MMAP
#define USE_MMAP

#include "sys-mmap.h"
#include <setjmp.h>
#include <signal.h>

static volatile int sigbus_jmp_valid;
static sigjmp_buf sigbus_jmp;

static void sigbus_handler(int sig) {
	UNUSED(sig);
	if (sigbus_jmp_valid) siglongjmp(sigbus_jmp, 1);
	log_failed_assert(__FILE__, __LINE__, "SIGBUS");
}
#endif

/* request: accept-encoding */
#define HTTP_ACCEPT_ENCODING_IDENTITY BV(0)
#define HTTP_ACCEPT_ENCODING_GZIP     BV(1)
#define HTTP_ACCEPT_ENCODING_DEFLATE  BV(2)
#define HTTP_ACCEPT_ENCODING_COMPRESS BV(3)
#define HTTP_ACCEPT_ENCODING_BZIP2    BV(4)
#define HTTP_ACCEPT_ENCODING_X_GZIP   BV(5)
#define HTTP_ACCEPT_ENCODING_X_BZIP2  BV(6)

#define KByte * 1024
#define MByte * 1024 KByte
#define GByte * 1024 MByte

typedef struct {
	array		*mimetypes;
	int		allowed_encodings;
	unsigned int	max_compress_size;
	unsigned short	min_compress_size;
	unsigned short	output_buffer_size;
	unsigned short	work_block_size;
	unsigned short	sync_flush;
	short		compression_level;
	double		max_loadavg;
} plugin_config;

typedef struct {
	PLUGIN_DATA;
	buffer *tmp_buf;
	array *encodings;

	plugin_config **config_storage;
	plugin_config conf;
} plugin_data;

typedef struct {
	union {
	      #ifdef USE_ZLIB
		z_stream z;
	      #endif
	      #ifdef USE_BZ2LIB
		bz_stream bz;
	      #endif
		int dummy;
	} u;
	off_t bytes_in;
	off_t bytes_out;
	chunkqueue *in_queue;
	buffer *output;
	plugin_data *plugin_data;
	int compression_type;
} handler_ctx;

static handler_ctx *handler_ctx_init() {
	handler_ctx *hctx;

	hctx = calloc(1, sizeof(*hctx));
	hctx->in_queue = chunkqueue_init();

	return hctx;
}

static void handler_ctx_free(handler_ctx *hctx) {
      #if 0
	if (hctx->output != p->tmp_buf) {
		buffer_free(hctx->output);
	}
      #endif
	chunkqueue_free(hctx->in_queue);
	free(hctx);
}

INIT_FUNC(mod_deflate_init) {
	plugin_data *p;

	p = calloc(1, sizeof(*p));

	p->encodings = array_init();
	p->tmp_buf = buffer_init();
	buffer_string_prepare_copy(p->tmp_buf, 64 KByte);

	return p;
}

FREE_FUNC(mod_deflate_free) {
	plugin_data *p = p_d;

	UNUSED(srv);

	if (!p) return HANDLER_GO_ON;

	if (p->config_storage) {
		size_t i;
		for (i = 0; i < srv->config_context->used; i++) {
			plugin_config *s = p->config_storage[i];

			if (!s) continue;

			array_free(s->mimetypes);
			free(s);
		}
		free(p->config_storage);
	}

	buffer_free(p->tmp_buf);
	array_free(p->encodings);

	free(p);

	return HANDLER_GO_ON;
}

SETDEFAULTS_FUNC(mod_deflate_setdefaults) {
	plugin_data *p = p_d;
	size_t i = 0;

	config_values_t cv[] = {
		{ "deflate.mimetypes",             NULL, T_CONFIG_ARRAY, T_CONFIG_SCOPE_CONNECTION },
		{ "deflate.allowed-encodings",     NULL, T_CONFIG_ARRAY, T_CONFIG_SCOPE_CONNECTION },
		{ "deflate.max-compress-size",     NULL, T_CONFIG_INT,   T_CONFIG_SCOPE_CONNECTION },
		{ "deflate.min-compress-size",     NULL, T_CONFIG_SHORT, T_CONFIG_SCOPE_CONNECTION },
		{ "deflate.compression-level",     NULL, T_CONFIG_SHORT, T_CONFIG_SCOPE_CONNECTION },
		{ "deflate.output-buffer-size",    NULL, T_CONFIG_SHORT, T_CONFIG_SCOPE_CONNECTION },
		{ "deflate.work-block-size",       NULL, T_CONFIG_SHORT, T_CONFIG_SCOPE_CONNECTION },
		{ "deflate.max-loadavg",           NULL, T_CONFIG_STRING,T_CONFIG_SCOPE_CONNECTION },
		{ NULL,                            NULL, T_CONFIG_UNSET, T_CONFIG_SCOPE_UNSET }
	};

	p->config_storage = calloc(1, srv->config_context->used * sizeof(plugin_config *));

	for (i = 0; i < srv->config_context->used; i++) {
		plugin_config *s;

		s = calloc(1, sizeof(plugin_config));
		s->mimetypes = array_init();
		s->allowed_encodings = 0;
		s->max_compress_size = 128*1024; /*(128 MB measured as num KB)*/
		s->min_compress_size = 256;
		s->output_buffer_size = 0;
		s->work_block_size = 2048;
		s->sync_flush = 0;
		s->compression_level = -1;
		s->max_loadavg = 0.0;

		array_reset(p->encodings); /* temp array for allowed encodings list */

		cv[0].destination = s->mimetypes;
		cv[1].destination = p->encodings;
		cv[2].destination = &(s->max_compress_size);
		cv[3].destination = &(s->min_compress_size);
		cv[4].destination = &(s->compression_level);
		cv[5].destination = &(s->output_buffer_size);
		cv[6].destination = &(s->work_block_size);
		cv[7].destination = p->tmp_buf;
		buffer_string_set_length(p->tmp_buf, 0);

		p->config_storage[i] = s;

		if (0 != config_insert_values_global(srv, ((data_config *)srv->config_context->data[i])->value, cv, i == 0 ? T_CONFIG_SCOPE_SERVER : T_CONFIG_SCOPE_CONNECTION)) {
			return HANDLER_ERROR;
		}

		if ((s->compression_level < 1 || s->compression_level > 9) &&
				s->compression_level != -1) {
			log_error_write(srv, __FILE__, __LINE__, "sd",
				"compression-level must be between 1 and 9:", s->compression_level);
			return HANDLER_ERROR;
		}

		if (!buffer_string_is_empty(p->tmp_buf)) {
			s->max_loadavg = strtod(p->tmp_buf->ptr, NULL);
		}

		if (p->encodings->used) {
			size_t j = 0;
			for (j = 0; j < p->encodings->used; j++) {
#if defined(USE_ZLIB) || defined(USE_BZ2LIB)
				data_string *ds = (data_string *)p->encodings->data[j];
#endif
#ifdef USE_ZLIB
				if (NULL != strstr(ds->value->ptr, "gzip"))
					s->allowed_encodings |= HTTP_ACCEPT_ENCODING_GZIP | HTTP_ACCEPT_ENCODING_X_GZIP;
				if (NULL != strstr(ds->value->ptr, "x-gzip"))
					s->allowed_encodings |= HTTP_ACCEPT_ENCODING_X_GZIP;
				if (NULL != strstr(ds->value->ptr, "deflate"))
					s->allowed_encodings |= HTTP_ACCEPT_ENCODING_DEFLATE;
				/*
				if (NULL != strstr(ds->value->ptr, "compress"))
					s->allowed_encodings |= HTTP_ACCEPT_ENCODING_COMPRESS;
				*/
#endif
#ifdef USE_BZ2LIB
				if (NULL != strstr(ds->value->ptr, "bzip2"))
					s->allowed_encodings |= HTTP_ACCEPT_ENCODING_BZIP2 | HTTP_ACCEPT_ENCODING_X_BZIP2;
				if (NULL != strstr(ds->value->ptr, "x-bzip2"))
					s->allowed_encodings |= HTTP_ACCEPT_ENCODING_X_BZIP2;
#endif
			}
		} else {
			/* default encodings */
#ifdef USE_ZLIB
			s->allowed_encodings |= HTTP_ACCEPT_ENCODING_GZIP
					     |  HTTP_ACCEPT_ENCODING_X_GZIP
					     |  HTTP_ACCEPT_ENCODING_DEFLATE;
#endif
#ifdef USE_BZ2LIB
			s->allowed_encodings |= HTTP_ACCEPT_ENCODING_BZIP2
					     |  HTTP_ACCEPT_ENCODING_X_BZIP2;
#endif
		}

		/* mod_deflate matches mimetype as prefix of Content-Type
		 * so ignore '*' at end of mimetype for end-user flexibility
		 * in specifying trailing wildcard to grouping of mimetypes */
		for (size_t m = 0; m < s->mimetypes->used; ++m) {
			buffer *mimetype = ((data_string *)s->mimetypes->data[m])->value;
			size_t len = buffer_string_length(mimetype);
			if (len > 2 && mimetype->ptr[len-1] == '*') {
				buffer_string_set_length(mimetype, len-1);
			}
		}
	}

	return HANDLER_GO_ON;

}


#if defined(USE_ZLIB) || defined(USE_BZ2LIB)
static int stream_http_chunk_append_mem(server *srv, connection *con, handler_ctx *hctx, size_t len) {
	/* future: might also write stream to hctx temporary file in compressed file cache */
	return http_chunk_append_mem(srv, con, hctx->output->ptr, len);
}
#endif


#ifdef USE_ZLIB

static int stream_deflate_init(handler_ctx *hctx) {
	z_stream * const z = &hctx->u.z;
	const plugin_data * const p = hctx->plugin_data;
	z->zalloc = Z_NULL;
	z->zfree = Z_NULL;
	z->opaque = Z_NULL;
	z->total_in = 0;
	z->total_out = 0;
	z->next_out = (unsigned char *)hctx->output->ptr;
	z->avail_out = hctx->output->size;

	if (Z_OK != deflateInit2(z,
				 p->conf.compression_level > 0
				  ? p->conf.compression_level
				  : Z_DEFAULT_COMPRESSION,
				 Z_DEFLATED,
				 (hctx->compression_type == HTTP_ACCEPT_ENCODING_GZIP)
				  ? (MAX_WBITS | 16) /*(0x10 flags gzip header, trailer)*/
				  : -MAX_WBITS,      /*(negate to suppress zlib header)*/
				 8, /* default memLevel */
				 Z_DEFAULT_STRATEGY)) {
		return -1;
	}

	return 0;
}

static int stream_deflate_compress(server *srv, connection *con, handler_ctx *hctx, unsigned char *start, off_t st_size) {
	z_stream * const z = &(hctx->u.z);
	size_t len;

	z->next_in = start;
	z->avail_in = st_size;
	hctx->bytes_in += st_size;

	/* compress data */
	do {
		if (Z_OK != deflate(z, Z_NO_FLUSH)) return -1;

		if (z->avail_out == 0 || z->avail_in > 0) {
			len = hctx->output->size - z->avail_out;
			hctx->bytes_out += len;
			stream_http_chunk_append_mem(srv, con, hctx, len);
			z->next_out = (unsigned char *)hctx->output->ptr;
			z->avail_out = hctx->output->size;
		}
	} while (z->avail_in > 0);

	return 0;
}

static int stream_deflate_flush(server *srv, connection *con, handler_ctx *hctx, int end) {
	z_stream * const z = &(hctx->u.z);
	const plugin_data *p = hctx->plugin_data;
	size_t len;
	int rc = 0;
	int done;

	/* compress data */
	do {
		done = 1;
		if (end) {
			rc = deflate(z, Z_FINISH);
			if (rc == Z_OK) {
				done = 0;
			} else if (rc != Z_STREAM_END) {
				return -1;
			}
		} else {
			if (p->conf.sync_flush) {
				rc = deflate(z, Z_SYNC_FLUSH);
				if (rc != Z_OK) return -1;
			} else if (z->avail_in > 0) {
				rc = deflate(z, Z_NO_FLUSH);
				if (rc != Z_OK) return -1;
			}
		}

		len = hctx->output->size - z->avail_out;
		if (z->avail_out == 0 || (len > 0 && (end || p->conf.sync_flush))) {
			hctx->bytes_out += len;
			stream_http_chunk_append_mem(srv, con, hctx, len);
			z->next_out = (unsigned char *)hctx->output->ptr;
			z->avail_out = hctx->output->size;
		}
	} while (z->avail_in != 0 || !done);

	return 0;
}

static int stream_deflate_end(server *srv, handler_ctx *hctx) {
	z_stream * const z = &(hctx->u.z);
	int rc = deflateEnd(z);
	if (Z_OK == rc || Z_DATA_ERROR == rc) return 0;

	if (z->msg != NULL) {
		log_error_write(srv, __FILE__, __LINE__, "sdss",
				"deflateEnd error ret=", rc, ", msg=", z->msg);
	} else {
		log_error_write(srv, __FILE__, __LINE__, "sd",
				"deflateEnd error ret=", rc);
	}
	return -1;
}

#endif


#ifdef USE_BZ2LIB

static int stream_bzip2_init(handler_ctx *hctx) {
	bz_stream * const bz = &hctx->u.bz;
	const plugin_data * const p = hctx->plugin_data;
	bz->bzalloc = NULL;
	bz->bzfree = NULL;
	bz->opaque = NULL;
	bz->total_in_lo32 = 0;
	bz->total_in_hi32 = 0;
	bz->total_out_lo32 = 0;
	bz->total_out_hi32 = 0;
	bz->next_out = hctx->output->ptr;
	bz->avail_out = hctx->output->size;

	if (BZ_OK != BZ2_bzCompressInit(bz,
					p->conf.compression_level > 0
					 ? p->conf.compression_level
					 : 9, /* blocksize = 900k */
					0,    /* verbosity */
					0)) { /* workFactor: default */
		return -1;
	}

	return 0;
}

static int stream_bzip2_compress(server *srv, connection *con, handler_ctx *hctx, unsigned char *start, off_t st_size) {
	bz_stream * const bz = &(hctx->u.bz);
	size_t len;

	bz->next_in = (char *)start;
	bz->avail_in = st_size;
	hctx->bytes_in += st_size;

	/* compress data */
	do {
		if (BZ_RUN_OK != BZ2_bzCompress(bz, BZ_RUN)) return -1;

		if (bz->avail_out == 0 || bz->avail_in > 0) {
			len = hctx->output->size - bz->avail_out;
			hctx->bytes_out += len;
			stream_http_chunk_append_mem(srv, con, hctx, len);
			bz->next_out = hctx->output->ptr;
			bz->avail_out = hctx->output->size;
		}
	} while (bz->avail_in > 0);

	return 0;
}

static int stream_bzip2_flush(server *srv, connection *con, handler_ctx *hctx, int end) {
	bz_stream * const bz = &(hctx->u.bz);
	const plugin_data *p = hctx->plugin_data;
	size_t len;
	int rc;
	int done;

	/* compress data */
	do {
		done = 1;
		if (end) {
			rc = BZ2_bzCompress(bz, BZ_FINISH);
			if (rc == BZ_FINISH_OK) {
				done = 0;
			} else if (rc != BZ_STREAM_END) {
				return -1;
			}
		} else if (bz->avail_in > 0) {
			/* p->conf.sync_flush not implemented here,
			 * which would loop on BZ_FLUSH while BZ_FLUSH_OK
			 * until BZ_RUN_OK returned */
			rc = BZ2_bzCompress(bz, BZ_RUN);
			if (rc != BZ_RUN_OK) {
				return -1;
			}
		}

		len = hctx->output->size - bz->avail_out;
		if (bz->avail_out == 0 || (len > 0 && (end || p->conf.sync_flush))) {
			hctx->bytes_out += len;
			stream_http_chunk_append_mem(srv, con, hctx, len);
			bz->next_out = hctx->output->ptr;
			bz->avail_out = hctx->output->size;
		}
	} while (bz->avail_in != 0 || !done);

	return 0;
}

static int stream_bzip2_end(server *srv, handler_ctx *hctx) {
	bz_stream * const bz = &(hctx->u.bz);
	int rc = BZ2_bzCompressEnd(bz);
	if (BZ_OK == rc || BZ_DATA_ERROR == rc) return 0;

	log_error_write(srv, __FILE__, __LINE__, "sd",
			"BZ2_bzCompressEnd error ret=", rc);
	return -1;
}

#endif


static int mod_deflate_stream_init(handler_ctx *hctx) {
	switch(hctx->compression_type) {
#ifdef USE_ZLIB
	case HTTP_ACCEPT_ENCODING_GZIP:
	case HTTP_ACCEPT_ENCODING_DEFLATE:
		return stream_deflate_init(hctx);
#endif
#ifdef USE_BZ2LIB
	case HTTP_ACCEPT_ENCODING_BZIP2:
		return stream_bzip2_init(hctx);
#endif
	default:
		return -1;
	}
}

static int mod_deflate_compress(server *srv, connection *con, handler_ctx *hctx, unsigned char *start, off_t st_size) {
	if (0 == st_size) return 0;
	switch(hctx->compression_type) {
#ifdef USE_ZLIB
	case HTTP_ACCEPT_ENCODING_GZIP:
	case HTTP_ACCEPT_ENCODING_DEFLATE:
		return stream_deflate_compress(srv, con, hctx, start, st_size);
#endif
#ifdef USE_BZ2LIB
	case HTTP_ACCEPT_ENCODING_BZIP2:
		return stream_bzip2_compress(srv, con, hctx, start, st_size);
#endif
	default:
		UNUSED(srv);
		UNUSED(con);
		UNUSED(start);
		return -1;
	}
}

static int mod_deflate_stream_flush(server *srv, connection *con, handler_ctx *hctx, int end) {
	if (0 == hctx->bytes_in) return 0;
	switch(hctx->compression_type) {
#ifdef USE_ZLIB
	case HTTP_ACCEPT_ENCODING_GZIP:
	case HTTP_ACCEPT_ENCODING_DEFLATE:
		return stream_deflate_flush(srv, con, hctx, end);
#endif
#ifdef USE_BZ2LIB
	case HTTP_ACCEPT_ENCODING_BZIP2:
		return stream_bzip2_flush(srv, con, hctx, end);
#endif
	default:
		UNUSED(srv);
		UNUSED(con);
		UNUSED(end);
		return -1;
	}
}

static void mod_deflate_note_ratio(server *srv, connection *con, handler_ctx *hctx) {
    /* store compression ratio in con->environment
     * for possible logging by mod_accesslog
     * (late in response handling, so not seen by most other modules) */
    /*(should be called only at end of successful response compression)*/
    char ratio[LI_ITOSTRING_LENGTH];
    if (0 == hctx->bytes_in) return;
    li_itostrn(ratio, sizeof(ratio), hctx->bytes_out * 100 / hctx->bytes_in);
    array_set_key_value(con->environment,
                        CONST_STR_LEN("ratio"),
                        ratio, strlen(ratio));
    UNUSED(srv);
}

static int mod_deflate_stream_end(server *srv, handler_ctx *hctx) {
	switch(hctx->compression_type) {
#ifdef USE_ZLIB
	case HTTP_ACCEPT_ENCODING_GZIP:
	case HTTP_ACCEPT_ENCODING_DEFLATE:
		return stream_deflate_end(srv, hctx);
#endif
#ifdef USE_BZ2LIB
	case HTTP_ACCEPT_ENCODING_BZIP2:
		return stream_bzip2_end(srv, hctx);
#endif
	default:
		UNUSED(srv);
		return -1;
	}
}

static void deflate_compress_cleanup(server *srv, connection *con, handler_ctx *hctx) {
	const plugin_data *p = hctx->plugin_data;
	con->plugin_ctx[p->id] = NULL;

	if (0 != mod_deflate_stream_end(srv, hctx)) {
		log_error_write(srv, __FILE__, __LINE__, "s", "error closing stream");
	}

      #if 1 /* unnecessary if deflate.min-compress-size is set to a reasonable value */
	if (hctx->bytes_in < hctx->bytes_out) {
		log_error_write(srv, __FILE__, __LINE__, "sbsdsd",
				"uri ", con->uri.path_raw, " in=", hctx->bytes_in, " smaller than out=", hctx->bytes_out);
	}
      #endif

	handler_ctx_free(hctx);
}


static int mod_deflate_file_chunk(server *srv, connection *con, handler_ctx *hctx, chunk *c, off_t st_size) {
	off_t abs_offset;
	off_t toSend = -1;
	char *start;
#ifdef USE_MMAP
	off_t we_want_to_mmap = 2 MByte;
	off_t we_want_to_send = st_size;
	volatile int mapped = 0;/* quiet warning: might be clobbered by 'longjmp' */
#else
	start = NULL;
#endif

	if (-1 == c->file.fd) {  /* open the file if not already open */
		if (-1 == (c->file.fd = fdevent_open_cloexec(c->file.name->ptr, O_RDONLY, 0))) {
			log_error_write(srv, __FILE__, __LINE__, "sbs", "open failed for:", c->file.name, strerror(errno));

			return -1;
		}
	}

	abs_offset = c->file.start + c->offset;

#ifdef USE_MMAP
	/* mmap the buffer
	 * - first mmap
	 * - new mmap as the we are at the end of the last one */
	if (c->file.mmap.start == MAP_FAILED ||
	    abs_offset == (off_t)(c->file.mmap.offset + c->file.mmap.length)) {

		/* Optimizations for the future:
		 *
		 * adaptive mem-mapping
		 *   the problem:
		 *     we mmap() the whole file. If someone has alot large files and 32bit
		 *     machine the virtual address area will be unrun and we will have a failing
		 *     mmap() call.
		 *   solution:
		 *     only mmap 16M in one chunk and move the window as soon as we have finished
		 *     the first 8M
		 *
		 * read-ahead buffering
		 *   the problem:
		 *     sending out several large files in parallel trashes the read-ahead of the
		 *     kernel leading to long wait-for-seek times.
		 *   solutions: (increasing complexity)
		 *     1. use madvise
		 *     2. use a internal read-ahead buffer in the chunk-structure
		 *     3. use non-blocking IO for file-transfers
		 *   */

		/* all mmap()ed areas are 512kb expect the last which might be smaller */
		off_t to_mmap;

		/* this is a remap, move the mmap-offset */
		if (c->file.mmap.start != MAP_FAILED) {
			munmap(c->file.mmap.start, c->file.mmap.length);
			c->file.mmap.offset += we_want_to_mmap;
		} else {
			/* in case the range-offset is after the first mmap()ed area we skip the area */
			c->file.mmap.offset = 0;

			while (c->file.mmap.offset + we_want_to_mmap < c->file.start) {
				c->file.mmap.offset += we_want_to_mmap;
			}
		}

		/* length is rel, c->offset too, assume there is no limit at the mmap-boundaries */
		to_mmap = (c->file.start + c->file.length) - c->file.mmap.offset;
		if (to_mmap > we_want_to_mmap) to_mmap = we_want_to_mmap;
		/* we have more to send than we can mmap() at once */
		if (we_want_to_send > to_mmap) we_want_to_send = to_mmap;

		if (MAP_FAILED == (c->file.mmap.start = mmap(0, (size_t)to_mmap, PROT_READ, MAP_SHARED, c->file.fd, c->file.mmap.offset))) {
			/* close it here, otherwise we'd have to set FD_CLOEXEC */

			log_error_write(srv, __FILE__, __LINE__, "ssbd", "mmap failed:",
					strerror(errno), c->file.name, c->file.fd);

			return -1;
		}

		c->file.mmap.length = to_mmap;
#ifdef HAVE_MADVISE
		/* don't advise files < 64Kb */
		if (c->file.mmap.length > (64 KByte) &&
		    0 != madvise(c->file.mmap.start, c->file.mmap.length, MADV_WILLNEED)) {
			log_error_write(srv, __FILE__, __LINE__, "ssbd", "madvise failed:",
					strerror(errno), c->file.name, c->file.fd);
		}
#endif

		/* chunk_reset() or chunk_free() will cleanup for us */
	}

	/* to_send = abs_mmap_end - abs_offset */
	toSend = (c->file.mmap.offset + c->file.mmap.length) - abs_offset;
	if (toSend > we_want_to_send) toSend = we_want_to_send;

	if (toSend < 0) {
		log_error_write(srv, __FILE__, __LINE__, "soooo",
				"toSend is negative:",
				toSend,
				c->file.mmap.length,
				abs_offset,
				c->file.mmap.offset);
		force_assert(toSend < 0);
	}

	start = c->file.mmap.start;
	mapped = 1;
#endif

	if (MAP_FAILED == c->file.mmap.start) {
		toSend = st_size;
		if (toSend > 2 MByte) toSend = 2 MByte;
		if (NULL == (start = malloc((size_t)toSend)) || -1 == lseek(c->file.fd, abs_offset, SEEK_SET) || toSend != read(c->file.fd, start, (size_t)toSend)) {
			log_error_write(srv, __FILE__, __LINE__, "sbss", "reading", c->file.name, "failed:", strerror(errno));

			free(start);
			return -1;
		}
	}

#ifdef USE_MMAP
	if (mapped) {
		signal(SIGBUS, sigbus_handler);
		sigbus_jmp_valid = 1;
		if (0 != sigsetjmp(sigbus_jmp, 1)) {
			sigbus_jmp_valid = 0;

			log_error_write(srv, __FILE__, __LINE__, "sbd", "SIGBUS in mmap:",
				c->file.name, c->file.fd);
			return -1;
		}
	}
#endif

	if (mod_deflate_compress(srv, con, hctx,
				(unsigned char *)start + (abs_offset - c->file.mmap.offset), toSend) < 0) {
		log_error_write(srv, __FILE__, __LINE__, "s",
				"compress failed.");
		toSend = -1;
	}

#ifdef USE_MMAP
	if (mapped)
		sigbus_jmp_valid = 0;
	else
#endif
		free(start);

	return toSend;
}


static handler_t deflate_compress_response(server *srv, connection *con, handler_ctx *hctx) {
	off_t len, max;
	int close_stream;

	/* move all chunk from write_queue into our in_queue, then adjust
	 * counters since con->write_queue is reused for compressed output */
	len = chunkqueue_length(con->write_queue);
	chunkqueue_remove_finished_chunks(con->write_queue);
	chunkqueue_append_chunkqueue(hctx->in_queue, con->write_queue);
	con->write_queue->bytes_in  -= len;
	con->write_queue->bytes_out -= len;

	max = chunkqueue_length(hctx->in_queue);
      #if 0
	/* calculate max bytes to compress for this call */
	if (p->conf.sync_flush && max > (len = p->conf.work_block_size << 10)) {
		max = len;
	}
      #endif

	/* Compress chunks from in_queue into chunks for write_queue */
	while (max) {
		chunk *c = hctx->in_queue->first;

		switch(c->type) {
		case MEM_CHUNK:
			len = buffer_string_length(c->mem) - c->offset;
			if (len > max) len = max;
			if (mod_deflate_compress(srv, con, hctx, (unsigned char *)c->mem->ptr+c->offset, len) < 0) {
				log_error_write(srv, __FILE__, __LINE__, "s",
						"compress failed.");
				return HANDLER_ERROR;
			}
			break;
		case FILE_CHUNK:
			len = c->file.length - c->offset;
			if (len > max) len = max;
			if ((len = mod_deflate_file_chunk(srv, con, hctx, c, len)) < 0) {
				log_error_write(srv, __FILE__, __LINE__, "s",
						"compress file chunk failed.");
				return HANDLER_ERROR;
			}
			break;
		default:
			log_error_write(srv, __FILE__, __LINE__, "ds", c, "type not known");
			return HANDLER_ERROR;
		}

		max -= len;
		chunkqueue_mark_written(hctx->in_queue, len);
	}

	/*(currently should always be true)*/
	/*(current implementation requires response be complete)*/
	close_stream = (con->file_finished && chunkqueue_is_empty(hctx->in_queue));
	if (mod_deflate_stream_flush(srv, con, hctx, close_stream) < 0) {
		log_error_write(srv, __FILE__, __LINE__, "s", "flush error");
		return HANDLER_ERROR;
	}

	return close_stream ? HANDLER_FINISHED : HANDLER_GO_ON;
}


#define PATCH(x) \
	p->conf.x = s->x;
static int mod_deflate_patch_connection(server *srv, connection *con, plugin_data *p) {
	size_t i, j;
	plugin_config *s = p->config_storage[0];

	PATCH(mimetypes);
	PATCH(allowed_encodings);
	PATCH(max_compress_size);
	PATCH(min_compress_size);
	PATCH(compression_level);
	PATCH(output_buffer_size);
	PATCH(work_block_size);
	PATCH(max_loadavg);

	/* skip the first, the global context */
	for (i = 1; i < srv->config_context->used; i++) {
		data_config *dc = (data_config *)srv->config_context->data[i];
		s = p->config_storage[i];

		/* condition didn't match */
		if (!config_check_cond(srv, con, dc)) continue;

		/* merge config */
		for (j = 0; j < dc->value->used; j++) {
			data_unset *du = dc->value->data[j];

			if (buffer_is_equal_string(du->key, CONST_STR_LEN("deflate.mimetypes"))) {
				PATCH(mimetypes);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("deflate.allowed-encodings"))) {
				PATCH(allowed_encodings);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("deflate.max-compress-size"))) {
				PATCH(max_compress_size);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("deflate.min-compress-size"))) {
				PATCH(min_compress_size);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("deflate.compression-level"))) {
				PATCH(compression_level);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("deflate.output-buffer-size"))) {
				PATCH(output_buffer_size);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("deflate.work-block-size"))) {
				PATCH(work_block_size);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("deflate.max-loadavg"))) {
				PATCH(max_loadavg);
			}
		}
	}

	return 0;
}
#undef PATCH

static int mod_deflate_choose_encoding (const char *value, plugin_data *p, const char **label) {
	/* get client side support encodings */
	int accept_encoding = 0;
#if !defined(USE_ZLIB) && !defined(USE_BZ2LIB)
	UNUSED(value);
#endif
#ifdef USE_ZLIB
	if (NULL != strstr(value, "gzip")) accept_encoding |= HTTP_ACCEPT_ENCODING_GZIP;
	else if (NULL != strstr(value, "x-gzip")) accept_encoding |= HTTP_ACCEPT_ENCODING_X_GZIP;
	if (NULL != strstr(value, "deflate")) accept_encoding |= HTTP_ACCEPT_ENCODING_DEFLATE;
#endif
	/* if (NULL != strstr(value, "compress")) accept_encoding |= HTTP_ACCEPT_ENCODING_COMPRESS; */
#ifdef USE_BZ2LIB
	if (p->conf.allowed_encodings & (HTTP_ACCEPT_ENCODING_BZIP2 | HTTP_ACCEPT_ENCODING_X_BZIP2)) {
		if (NULL != strstr(value, "bzip2")) accept_encoding |= HTTP_ACCEPT_ENCODING_BZIP2;
		else if (NULL != strstr(value, "x-bzip2")) accept_encoding |= HTTP_ACCEPT_ENCODING_X_BZIP2;
	}
#endif
	/* if (NULL != strstr(value, "identity")) accept_encoding |= HTTP_ACCEPT_ENCODING_IDENTITY; */

	/* mask to limit to allowed_encodings */
	accept_encoding &= p->conf.allowed_encodings;

	/* select best matching encoding */
#ifdef USE_BZ2LIB
	if (accept_encoding & HTTP_ACCEPT_ENCODING_BZIP2) {
		*label = "bzip2";
		return HTTP_ACCEPT_ENCODING_BZIP2;
	} else if (accept_encoding & HTTP_ACCEPT_ENCODING_X_BZIP2) {
		*label = "x-bzip2";
		return HTTP_ACCEPT_ENCODING_BZIP2;
	} else
#endif
	if (accept_encoding & HTTP_ACCEPT_ENCODING_GZIP) {
		*label = "gzip";
		return HTTP_ACCEPT_ENCODING_GZIP;
	} else if (accept_encoding & HTTP_ACCEPT_ENCODING_X_GZIP) {
		*label = "x-gzip";
		return HTTP_ACCEPT_ENCODING_GZIP;
	} else if (accept_encoding & HTTP_ACCEPT_ENCODING_DEFLATE) {
		*label = "deflate";
		return HTTP_ACCEPT_ENCODING_DEFLATE;
	} else {
		return 0;
	}
}

CONNECTION_FUNC(mod_deflate_handle_response_start) {
	plugin_data *p = p_d;
	data_string *ds;
	handler_ctx *hctx;
	const char *label;
	off_t len;
	size_t etaglen = 0;
	int compression_type;
	handler_t rc;

	/*(current implementation requires response be complete)*/
	if (!con->file_finished) return HANDLER_GO_ON;
	if (con->request.http_method == HTTP_METHOD_HEAD) return HANDLER_GO_ON;
	if (con->parsed_response & HTTP_TRANSFER_ENCODING_CHUNKED) return HANDLER_GO_ON;

	/* disable compression for some http status types. */
	switch(con->http_status) {
	case 100:
	case 101:
	case 204:
	case 205:
	case 304:
		/* disable compression as we have no response entity */
		return HANDLER_GO_ON;
	default:
		break;
	}

	mod_deflate_patch_connection(srv, con, p);

	/* check if deflate configured for any mimetypes */
	if (!p->conf.mimetypes->used) return HANDLER_GO_ON;

	/* check if size of response is below min-compress-size or exceeds max*/
	/* (con->file_finished checked at top of routine) */
	len = chunkqueue_length(con->write_queue);
	if (len <= (off_t)p->conf.min_compress_size) return HANDLER_GO_ON;
	if (p->conf.max_compress_size /*(max_compress_size in KB)*/
	    && len > ((off_t)p->conf.max_compress_size << 10)) {
		return HANDLER_GO_ON;
	}

	/* Check if response has a Content-Encoding. */
	ds = (data_string *)array_get_element(con->response.headers, "Content-Encoding");
	if (NULL != ds) return HANDLER_GO_ON;

	/* Check Accept-Encoding for supported encoding. */
	ds = (data_string *)array_get_element(con->request.headers, "Accept-Encoding");
	if (NULL == ds) return HANDLER_GO_ON;

	/* find matching encodings */
	compression_type = mod_deflate_choose_encoding(ds->value->ptr, p, &label);
	if (!compression_type) return HANDLER_GO_ON;

	/* Check mimetype in response header "Content-Type" */
	if (NULL != (ds = (data_string *)array_get_element(con->response.headers, "Content-Type"))) {
		int found = 0;
		size_t m;
		for (m = 0; m < p->conf.mimetypes->used; ++m) {
			data_string *mimetype = (data_string *)p->conf.mimetypes->data[m];
			if (0 == strncmp(mimetype->value->ptr, ds->value->ptr, buffer_string_length(mimetype->value))) {
				/* mimetype found */
				found = 1;
				break;
			}
		}
		if (!found) return HANDLER_GO_ON;

#if 0
		if (0 == strncasecmp(ds->value->ptr, "application/x-javascript", 24)) {
			/*reset compress type to deflate for javascript
			 * prevent buggy IE6 SP1 doesn't work for js in IFrame
			 */
			compression_type = HTTP_ACCEPT_ENCODING_DEFLATE;
		}
#endif
	} else {
		/* If no Content-Type set, compress only if first p->conf.mimetypes value is "" */
		data_string *mimetype = (data_string *)p->conf.mimetypes->data[0];
		if (!buffer_string_is_empty(mimetype->value)) return HANDLER_GO_ON;
	}

	/* Vary: Accept-Encoding (response might change according to request Accept-Encoding) */
	if (NULL != (ds = (data_string *)array_get_element(con->response.headers, "Vary"))) {
		if (NULL == strstr(ds->value->ptr, "Accept-Encoding")) {
			buffer_append_string_len(ds->value, CONST_STR_LEN(",Accept-Encoding"));
		}
	} else {
		response_header_insert(srv, con, CONST_STR_LEN("Vary"),
				CONST_STR_LEN("Accept-Encoding"));
	}

	/* check ETag as is done in http_response_handle_cachable()
	 * (slightly imperfect (close enough?) match of ETag "000000" to "000000-gzip") */
	ds = (data_string *)array_get_element(con->response.headers, "ETag");
	if (NULL != ds) {
		etaglen = buffer_string_length(ds->value);
		if (etaglen
		    && con->http_status < 300 /*(want 2xx only)*/
		    && con->request.http_if_none_match
		    && 0 == strncmp(con->request.http_if_none_match, ds->value->ptr, etaglen-1)
		    && con->request.http_if_none_match[etaglen-1] == '-'
		    && 0 == strncmp(con->request.http_if_none_match+etaglen, label, strlen(label))) {

			if (   HTTP_METHOD_GET  == con->request.http_method
			    || HTTP_METHOD_HEAD == con->request.http_method) {
				/* modify ETag response header in-place to remove '"' and append '-label"' */
				ds->value->ptr[etaglen-1] = '-'; /*(overwrite end '"')*/
				buffer_append_string(ds->value, label);
				buffer_append_string_len(ds->value, CONST_STR_LEN("\""));
				/*buffer_copy_buffer(con->physical.etag, ds->value);*//*(keep in sync?)*/
				con->http_status = 304;
			} else {
				con->http_status = 412;
			}

			/* response_start hook occurs after error docs have been handled.
			 * For now, send back empty response body.
			 * In the future, might extract the error doc code so that it
			 * might be run again if response_start hooks return with
			 * changed http_status and con->mode = DIRECT */
			con->response.transfer_encoding &= ~HTTP_TRANSFER_ENCODING_CHUNKED;
			con->parsed_response &= ~HTTP_CONTENT_LENGTH;
			chunkqueue_reset(con->write_queue);
			con->file_finished = 1;

			con->mode = DIRECT;
			return HANDLER_GO_ON;
		}
	}

	if (0.0 < p->conf.max_loadavg && p->conf.max_loadavg < srv->srvconf.loadavg[0]) {
		return HANDLER_GO_ON;
	}

	/* update ETag, if ETag response header is set */
	if (etaglen) {
		/* modify ETag response header in-place to remove '"' and append '-label"' */
		ds->value->ptr[etaglen-1] = '-'; /*(overwrite end '"')*/
		buffer_append_string(ds->value, label);
		buffer_append_string_len(ds->value, CONST_STR_LEN("\""));
		/*buffer_copy_buffer(con->physical.etag, ds->value);*//*(keep in sync?)*/
	}

	/* set Content-Encoding to show selected compression type */
	response_header_overwrite(srv, con, CONST_STR_LEN("Content-Encoding"), label, strlen(label));

	/* clear Content-Length and con->write_queue if HTTP HEAD request
	 * (alternatively, could return original Content-Length with HEAD
	 *  request if ETag not modified and Content-Encoding not added) */
	if (HTTP_METHOD_HEAD == con->request.http_method) {
		/* ensure that uncompressed Content-Length is not sent in HEAD response */
		chunkqueue_reset(con->write_queue);
		if (con->parsed_response & HTTP_CONTENT_LENGTH) {
			con->parsed_response &= ~HTTP_CONTENT_LENGTH;
			if (NULL != (ds = (data_string*) array_get_element(con->response.headers, "Content-Length"))) {
				buffer_reset(ds->value); /* headers with empty values are ignored for output */
			}
		}
		return HANDLER_GO_ON;
	}

	/* future: might use ETag to check if compressed content is in compressed file cache */
	/*if (etaglen) { ... } *//* return if in file cache after updating con->write_queue */

	/* enable compression */
	p->conf.sync_flush =
	  (con->conf.stream_response_body && 0 == p->conf.output_buffer_size);
	hctx = handler_ctx_init();
	hctx->plugin_data = p;
	hctx->compression_type = compression_type;
	/* setup output buffer */
	buffer_string_set_length(p->tmp_buf, 0);
	hctx->output = p->tmp_buf;
	if (0 != mod_deflate_stream_init(hctx)) {
		/*(should not happen unless ENOMEM)*/
		handler_ctx_free(hctx);
		log_error_write(srv, __FILE__, __LINE__, "ss",
				"Failed to initialize compression", label);
		/* restore prior Etag and unset Content-Encoding */
		if (etaglen) {
			ds->value->ptr[etaglen-1] = '"'; /*(overwrite '-')*/
			buffer_string_set_length(ds->value, etaglen);
		}
		ds = (data_string *)array_get_element(con->response.headers, "Content-Encoding");
		if (ds) buffer_reset(ds->value); /* headers with empty values are ignored for output */
		return HANDLER_GO_ON;
	}

	con->parsed_response &= ~HTTP_CONTENT_LENGTH;
	con->plugin_ctx[p->id] = hctx;

	rc = deflate_compress_response(srv, con, hctx);
	if (HANDLER_GO_ON != rc) {
		if (HANDLER_FINISHED == rc) {
			mod_deflate_note_ratio(srv, con, hctx);
		}
		deflate_compress_cleanup(srv, con, hctx);
		if (HANDLER_ERROR == rc) return HANDLER_ERROR;
	}

	return HANDLER_GO_ON;
}

static handler_t mod_deflate_cleanup(server *srv, connection *con, void *p_d) {
	plugin_data *p = p_d;
	handler_ctx *hctx = con->plugin_ctx[p->id];

	if (NULL != hctx) deflate_compress_cleanup(srv, con, hctx);

	return HANDLER_GO_ON;
}

int mod_deflate_plugin_init(plugin *p);
int mod_deflate_plugin_init(plugin *p) {
	p->version     = LIGHTTPD_VERSION_ID;
	p->name        = buffer_init_string("deflate");

	p->init		= mod_deflate_init;
	p->cleanup	= mod_deflate_free;
	p->set_defaults	= mod_deflate_setdefaults;
	p->connection_reset	= mod_deflate_cleanup;
	p->handle_connection_close	= mod_deflate_cleanup;
	p->handle_response_start	= mod_deflate_handle_response_start;

	p->data        = NULL;

	return 0;
}
