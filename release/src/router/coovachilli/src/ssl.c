/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2009-2012 David Bird (Coova Technologies) <support@coova.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */
#include "system.h"
#ifdef HAVE_SSL
#include "chilli.h"
#include "debug.h"

static int openssl_init = 0;
static openssl_env * sslenv_svr = 0;
static openssl_env * sslenv_cli = 0;

#ifdef HAVE_CYASSL
#define HAVE_OPENSSL 1
#else
#define HAVE_OPENSSL_ENGINE 1
#endif

openssl_env * initssl() {
  if (sslenv_svr == 0) {
    if (openssl_init == 0) {
      openssl_init = 1;
#ifdef HAVE_OPENSSL
#if OPENSSL_VERSION_NUMBER < 0x10100000L
      if (_options.debug) {
	SSL_load_error_strings();
      }
      SSL_library_init();
      OpenSSL_add_all_algorithms();
#endif

#else
      matrixSslOpen();
      log_dbg("MatrixSslOpen()");
#endif
    }
    openssl_env_init(sslenv_svr = calloc(1, sizeof(openssl_env)), 0, 1);
  }
  return sslenv_svr;
}

openssl_env * initssl_cli() {
  if (sslenv_cli == 0) {
    if (openssl_init == 0) {
      openssl_init = 1;
#ifdef HAVE_OPENSSL
#if OPENSSL_VERSION_NUMBER < 0x10100000L
      if (_options.debug) {
	SSL_load_error_strings();
      }
      SSL_library_init();
      OpenSSL_add_all_algorithms();
#endif
#else
      matrixSslOpen();
      log_dbg("MatrixSslOpen()");
#endif
    }
    openssl_env_init(sslenv_cli = calloc(1, sizeof(openssl_env)), 0, 0);
  }
  return sslenv_cli;
}

#ifdef HAVE_OPENSSL
static int
openssl_verify_peer_cb(int ok, X509_STORE_CTX *ctx) {
  int err = X509_STORE_CTX_get_error(ctx);
  if (err != X509_V_OK) {
    log_err(errno, "peer certificate error: #%d : %s\n", 
              err, X509_verify_cert_error_string(err));
    return 0;
  }
  return 1;
}

int
openssl_verify_peer(openssl_env *env, int mode) {
  if (!mode) mode = OPENSSL_NO_CERT;
  SSL_CTX_set_verify(env->ctx, mode, openssl_verify_peer_cb);
  return 1;
}

int
openssl_use_certificate(openssl_env *env, char *file) {
  if (file)
    if (SSL_CTX_use_certificate_file(env->ctx, file, SSL_FILETYPE_PEM) > 0)
      return 1;
  log_err(errno, "could not load certificate file %s\n",file);
  return 0;
}

int
openssl_use_privatekey(openssl_env *env, char *file) {
  int err1=-1, err2=-1;
  if (file) {
    if ((err1 = SSL_CTX_use_PrivateKey_file(env->ctx, file, SSL_FILETYPE_PEM)) > 0 &&
        (err2 = SSL_CTX_check_private_key(env->ctx)))
      return 1;
  }
  log_err(errno, "could not load private key file %s (%d,%d)\n",file,err1,err2);
  /*
    {
    BIO *bio_err = NULL;
    bio_err = BIO_new_fp(stderr, BIO_NOCLOSE);
    BIO_printf(bio_err,"unable to set private key file\n");
    #if(_debug_)
    ERR_print_errors(bio_err);
    #endif
    }
  */
  return 0;
}

int
openssl_cacert_location(openssl_env *env, char *file, char *dir) {
  int err = SSL_CTX_load_verify_locations(env->ctx, file, dir);
  if (!err)
    log_err(errno, "unable to load CA certificates.\n");
  return err;
}

#if(0)
static RSA *
openssl_tmpRSA_cb(SSL *ssl, int export, int len) {
  openssl_con *con = (openssl_con *)SSL_get_app_data(ssl);
  openssl_env *env = con->env;
  RSA *rsa = 0;

  if (export) {
    /* an export cipher is being used */
    if (len == 512)
      rsa = (RSA *)env->tmpKeys[OPENSSL_TMPKEY_RSA512];
    else if (len == 1024)
      rsa = (RSA *)env->tmpKeys[OPENSSL_TMPKEY_RSA1024];
    else
      /* too expensive to generate on-the-fly, use 1024bit */
      rsa = (RSA *)env->tmpKeys[OPENSSL_TMPKEY_RSA1024];
  }
  else {
    /* sign-only certificate situation exists */
    rsa = (RSA *)env->tmpKeys[OPENSSL_TMPKEY_RSA1024];
  }
  return rsa;
}

static DH *
openssl_tmpDH_cb(SSL *ssl, int export, int len) {
  openssl_con *con = (openssl_con *)SSL_get_app_data(ssl);
  openssl_env *env = con->env;
  DH *dh = 0;

  if (export) {
    /* an export cipher is being used */
    if (len == 512)
      dh = (DH *)env->tmpKeys[OPENSSL_TMPKEY_DH512];
    else if (len == 1024)
      dh = (DH *)env->tmpKeys[OPENSSL_TMPKEY_DH1024];
    else
      /* too expensive to generate on-the-fly, use 1024bit */
      dh = (DH *)env->tmpKeys[OPENSSL_TMPKEY_DH1024];
  }
  else {
    /* sign-only certificate situation exists */
    dh = (DH *)env->tmpKeys[OPENSSL_TMPKEY_DH1024];
  }
  return dh;
}

static void
openssl_tmp_genkeys(openssl_env *env) {

  if ((env->tmpKeys[OPENSSL_TMPKEY_RSA512] = RSA_generate_key(512, RSA_F4, NULL, NULL)) == NULL) {
    log_err(errno, "could not generate tmp 512bit RSA key\n");
  }

  if ((env->tmpKeys[OPENSSL_TMPKEY_RSA1024] = RSA_generate_key(1024, RSA_F4, NULL, NULL)) == NULL) {
    log_err(errno, "could not generate tmp 1024bit RSA key\n");
  }

  if ((env->tmpKeys[OPENSSL_TMPKEY_DH512] = openssl_dh_tmpkey(512)) == NULL) {
    log_err(errno, "could not generate tmp 512bit DH key\n");
  }

  if ((env->tmpKeys[OPENSSL_TMPKEY_DH1024] = openssl_dh_tmpkey(1024)) == NULL) {
    log_err(errno, "could not generate tmp 512bit DH key\n");
  }
}
#endif

int
_openssl_env_init(openssl_env *env, char *engine, int server) {
  /*
   * Create an OpenSSL environment (method and context).
   * If ``server'' is 1, the environment is that of a SSL 
   * server.
   */
  env->meth = SSLv23_method();
  env->ctx = SSL_CTX_new(env->meth);
  SSL_CTX_set_options(env->ctx, SSL_OP_ALL);
#ifdef HAVE_OPENSSL_ENGINE
  if (engine) {
  retry:
    if ((env->engine = ENGINE_by_id(engine)) == NULL) {
      fprintf(stderr,"invalid engine \"%s\"\n", engine);
      ENGINE_free(env->engine);
      engine = "openssl";
      goto retry;
    }
    if (!ENGINE_set_default(env->engine, ENGINE_METHOD_ALL)) {
      fprintf(stderr,"can't use that engine\n");
      ENGINE_free(env->engine);
      engine = "openssl";
      goto retry;
    }
  }
#endif

#ifdef HAVE_OPENSSL_ENGINE
  SSL_CTX_set_app_data(env->ctx, env);
#endif

  if (server) {
    SSL_CTX_set_options(env->ctx, SSL_OP_SINGLE_DH_USE);
    SSL_CTX_set_session_cache_mode(env->ctx, SSL_SESS_CACHE_OFF);
    SSL_CTX_set_quiet_shutdown(env->ctx, 1);
  }
  return 1;
}
#endif

#ifdef HAVE_OPENSSL
static int _openssl_passwd(char *buf, int size, int rwflag, void *ud) {
  safe_strncpy(buf, _options.sslkeypass, size);
  memset(_options.sslkeypass,'x',strlen(_options.sslkeypass));
  return 0;
}
#endif

int
openssl_env_init(openssl_env *env, char *engine, int server) {

  if (!_options.sslcertfile || !_options.sslkeyfile) {
    log_err(0, "options sslcertfile and sslkeyfile are required");
    return 0;
  }

#ifdef HAVE_OPENSSL
  {
    int err = _openssl_env_init(env, engine, server);

    if (_options.sslkeypass) {
      SSL_CTX_set_default_passwd_cb(env->ctx, _openssl_passwd);
    }
    
    if (!openssl_use_certificate(env, _options.sslcertfile) ||
	!openssl_use_privatekey(env, _options.sslkeyfile)) {
      log_err(0, "failed reading setup sslcertfile and/or sslkeyfile");
      return 0;
    }

    if (_options.sslcafile) {
      if (!openssl_cacert_location(env, _options.sslcafile, 0)) {
	log_err(0, "failed reading sslcafile");
	return 0;
      }
    }
    
    env->ready = 1;
    return err;
  }
#else
  log_dbg("MatrixSSL Setup:");
  log_dbg("SSL cert: %s",_options.sslcertfile);
  log_dbg("SSL key: %s",_options.sslkeyfile);
  log_dbg("SSL pass: %s",_options.sslkeypass?_options.sslkeypass:"null");
  log_dbg("SSL ca: %s",_options.sslcafile?_options.sslcafile:"null");
  if ( matrixSslReadKeys( &env->keys,
			  _options.sslcertfile, 
			  _options.sslkeyfile, 
			  _options.sslkeypass, 
			  _options.sslcafile ) < 0 ) {
    log_err(errno, "could not load ssl certificate or and/or key file");
    return 0;
  }

  env->ready = 1;
  return 1;
#endif
}

#ifdef HAVE_MATRIXSSL
static int certValidator(sslCertInfo_t *t, void *arg) {
  log_dbg("MatrixSSL: certValidator()");
  return 1;
}
#endif

openssl_con *
openssl_connect_fd(openssl_env *env, int fd, int timeout) {
  openssl_con *c = (openssl_con *)calloc(1, sizeof(*c));
  if (!c) return 0;

  c->env = env;
#ifdef HAVE_OPENSSL
  c->con = (SSL *)SSL_new(env->ctx); 
#elif  HAVE_MATRIXSSL
  c->con = (SSL *)SSL_new(env->keys, 0); 
#endif
  c->sock = fd;
  c->timeout = timeout;

  SSL_set_fd(c->con, c->sock);

#ifdef HAVE_OPENSSL  
#ifdef HAVE_OPENSSL_ENGINE
  SSL_set_app_data(c->con, c);
#endif
  SSL_set_connect_state(c->con);

  if (SSL_connect(c->con) < 0) {
    char is_error = 0;
#if(_debug_)
    unsigned long error;
    while ((error = ERR_get_error())) {
      log_dbg("TLS: %s", ERR_error_string(error, NULL));
      is_error = 1;
    }
#endif
    if (is_error) {
      openssl_free(c);
      return 0;
    }
  }
#elif  HAVE_MATRIXSSL
  if (!SSL_connect(c->con, certValidator, c)) {
    log_err(errno, "openssl_connect_fd");
    openssl_free(c);
    return 0;
  }
#endif

  return c;
}

int
openssl_check_accept(openssl_con *c, struct redir_conn_t *conn) {

#ifdef HAVE_OPENSSL  
  int rc;

  if (!c || !c->con) return -1;

  if (!SSL_is_init_finished(c->con)) {

    if ((rc = SSL_accept(c->con)) <= 0) {

      int err = SSL_get_error(c->con, rc);
      
      switch (err) {
      case SSL_ERROR_WANT_READ:
      case SSL_ERROR_WANT_WRITE: 
	return 1;
	
      case SSL_ERROR_SYSCALL:
	if (errno != EINTR) {
	  if (errno > 0)
	    log_err(errno, "SSL handshake interrupted by system [Hint: Stop button pressed in browser?!]");
	  else
	    log_err(errno, "Spurious SSL handshake interrupt [Hint: Usually just one of those OpenSSL confusions!?]");
	}
	break;
      }
      
      return -1;
      
    } else {
      
#ifdef HAVE_OPENSSL_ENGINE
      X509 *peer_cert = SSL_get_peer_certificate(c->con);
      
      if (peer_cert) {
	char subj[1024];
	
	X509_NAME_oneline(X509_get_subject_name(peer_cert),subj,sizeof(subj));
	
	if (SSL_get_verify_result(c->con) != X509_V_OK) {
	  log_dbg("auth_failed: %s", subj);
	  X509_free(peer_cert);
	  return -1;
	}
		
	log_dbg("auth_success: %s", subj);
	if (conn) conn->s_params.flags |= ADMIN_LOGIN;

	if (_options.debug) {
	  EVP_PKEY *pktmp = X509_get_pubkey(peer_cert);
#if OPENSSL_VERSION_NUMBER >= 0x10000000L
 	  const
#endif
	  SSL_CIPHER *cipher;
	  char b[512];
	  log_dbg("Debugging: SSL Information:\n");
	  cipher = SSL_get_current_cipher(c->con);
	  log_dbg("  Protocol: %s, %s with %.*s bit key\n", 
		  SSL_CIPHER_get_version(cipher),
		  (char*)SSL_CIPHER_get_name(cipher),
		  sprintf(b, "%d", EVP_PKEY_bits(pktmp)), b);
	  log_dbg("  Subject:  %s\n", subj);
	  X509_NAME_oneline(X509_get_issuer_name(peer_cert),b,sizeof(b));
	  log_dbg("  Issuer:   %s\n", b);
	  EVP_PKEY_free(pktmp);
	}
	
	X509_free(peer_cert);
      } else {
	log_dbg("no SSL certificate");
      }
#endif
    }
  }
#elif  HAVE_MATRIXSSL

  if (!c || !c->con) return -1;

  if (!SSL_is_init_finished(c->con)) {

    if (SSL_accept2(c->con) < 0) {
      return -1;
    }

    if (!SSL_is_init_finished(c->con))
      return 1;
  }

#endif

  return 0;
}

openssl_con *
openssl_accept_fd(openssl_env *env, int fd, int timeout, struct redir_conn_t *conn) {
  openssl_con *c = (openssl_con *)calloc(1, sizeof(openssl_con));
  int rc;

  if (!c) return 0;

  if (!env || !env->ready) {
    log_err(0, "SSL not available!");
    return 0;
  }

  c->env = env;
#ifdef HAVE_OPENSSL
  c->con = (SSL *)SSL_new(env->ctx); 
#elif  HAVE_MATRIXSSL
  c->con = (SSL *)SSL_new(env->keys, SSL_FLAGS_SERVER); 
#endif
  c->sock = fd;
  c->timeout = timeout;

  SSL_set_fd(c->con, c->sock);

#ifdef HAVE_OPENSSL  
#ifdef HAVE_OPENSSL_ENGINE
  SSL_clear(c->con);
#endif

#ifdef HAVE_OPENSSL_ENGINE
  SSL_set_app_data(c->con, c);
#endif
  SSL_set_accept_state(c->con);

#ifdef HAVE_OPENSSL_ENGINE
  SSL_set_verify_result(c->con, X509_V_OK);
#endif

  if ((rc = openssl_check_accept(c, conn)) < 0) {
    SSL_set_shutdown(c->con, SSL_RECEIVED_SHUTDOWN);
    openssl_free(c);
    return 0;
  }

#elif  HAVE_MATRIXSSL
  
  /* ndelay_off(c->sock); */

  matrixSslSetCertValidator(c->con->ssl, certValidator, c->con->keys);

  if ((rc = SSL_accept2(c->con)) < 0) {
    log_err(c->con->status, "SSL accept failure");
    openssl_free(c);
    return 0;
  }

  SSL_is_init_finished(c->con);

  /* ndelay_on(c->sock);*/

#else
#error NO SSL SUPPORT
#endif

  return c;
}

int
openssl_error(openssl_con *con, int ret, char *func) {
#ifdef HAVE_OPENSSL  
  int err = -1;
  if (con->con) {
    err = SSL_get_error(con->con, ret);
#if(_debug_ > 1)
    log_dbg("SSL: (%s()) %s", func,
	    err == SSL_ERROR_NONE ? "None": 
            err == SSL_ERROR_ZERO_RETURN ? "Return!":
	    err == SSL_ERROR_WANT_READ ? "Read (continue)":
	    err == SSL_ERROR_WANT_WRITE ? "Write (continue)":
	    err == SSL_ERROR_WANT_X509_LOOKUP ? "Lookup (continue)":
	    err == SSL_ERROR_SYSCALL ? "Syscall error, abort!":
	    err == SSL_ERROR_SSL ? "SSL error, abort!":
	    "Error");
#endif
    switch (err) {
    case SSL_ERROR_NONE: return 0;
    case SSL_ERROR_WANT_READ: return 1;
    case SSL_ERROR_WANT_WRITE: return 2;
    case SSL_ERROR_SYSCALL: 
      /*
       * This is a protocol violation, but we got
       * an EOF (remote connection did a shutdown(fd, 1).
       * We will treat it as a zero value.
       */
      if (ret == 0) return 0;
      /* If some other error, fall through */
    case SSL_ERROR_ZERO_RETURN: openssl_shutdown(con, 0);
    case SSL_ERROR_SSL: return -1;
    default: break;
    }
    return 1;
  }
  return err;
#else
  log_err(errno, "ssl error in %s", func);
  return 0;
#endif
}

void
openssl_shutdown(openssl_con *con, int state) {
#ifdef HAVE_OPENSSL
  int i;
  /*
   * state is the same as in shutdown(2)
   */
  if (con) {
    switch(state) {
    case 0: SSL_set_shutdown(con->con, SSL_RECEIVED_SHUTDOWN); break;
    case 1: SSL_set_shutdown(con->con, SSL_SENT_SHUTDOWN); break;
    case 2: SSL_set_shutdown(con->con, SSL_RECEIVED_SHUTDOWN|SSL_SENT_SHUTDOWN); break;
    }
    for (i = 0; i < 4; i++) 
      if (SSL_shutdown(con->con)) 
	break;
  }
#endif
}
  
int
openssl_read(openssl_con *con, char *b, int l, int t) {
  int rbytes = 0;
  int err = 0;

  if (!con) return -1;

  if (t && !(openssl_pending(con))) {
    fd_set rfds;
    fd_set wfds;
    struct timeval tv;
    int fd = con->sock;

    tv.tv_sec = t;
    tv.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_SET(fd, &rfds);
    FD_SET(fd, &wfds);

    if (select(fd + 1,&rfds,&wfds,(fd_set *) 0,&tv) == -1) return -1;
    if (!FD_ISSET(fd, &rfds) && !FD_ISSET(fd, &wfds)) return 0;
  }
    
 repeat_read:

  rbytes = SSL_read(con->con, b, l);

  log_dbg("--- SSL_read() = %d", rbytes);

  if (rbytes <= 0) {
    err = openssl_error(con, rbytes, "openssl_read");
  }

  if (rbytes > 0) return rbytes;
  if (err > 0) goto repeat_read;
  return (err == -1) ? -1: 0;
}

int
openssl_write(openssl_con *con, char *b, int l, int t) {
  size_t sent = 0;
  ssize_t wrt;
  int err;

  if (t) {
    fd_set wfds;
    struct timeval tv;
    int fd = con->sock;
    
    tv.tv_sec = t;
    tv.tv_usec = 0;
    
    FD_ZERO(&wfds);
    FD_SET(fd, &wfds);
    
    if (select(fd + 1,(fd_set *) 0,&wfds,(fd_set *) 0,&tv) == -1) return -1;
    if (!FD_ISSET(fd, &wfds)) return 0;
  }

  while (sent < l) {

  repeat_write:
    
    wrt = SSL_write(con->con, b+sent, l-sent);
    
    if (wrt <= 0) {
      err = openssl_error(con, wrt, "openssl_write");
      if (err == -1) return err;
      else if (err > 0) {
#if(_debug_)
	log_dbg("ssl_repeart_write");
#endif
	goto repeat_write;
      }
      break;
    } 

    sent += wrt;
  }

  return sent;
}

void
openssl_free(openssl_con *con) {
  SSL *c = con->con;
  if (c) {
#ifdef HAVE_OPENSSL
    SSL_set_connect_state(c); 
#endif
    SSL_free(c); 
    con->con = 0; 
  }
  free(con);
}

void
openssl_env_free(openssl_env *env) {
#if(_debug_)
  log_dbg("Freeing SSL environemnt");
#endif
#ifdef HAVE_OPENSSL
  if (env->ctx) SSL_CTX_free(env->ctx);
#ifdef HAVE_OPENSSL_ENGINE
  if (env->engine) ENGINE_free(env->engine);
#endif
#endif
  free(env);
}

int 
openssl_pending(openssl_con *con) {
  if (con->con) {
    int pending = SSL_pending(con->con);
    /*log_dbg("openssl_pending(%d)", pending);*/
    return pending;
  }
  return 0;
}

#endif
