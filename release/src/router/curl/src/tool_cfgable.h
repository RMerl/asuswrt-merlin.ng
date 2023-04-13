#ifndef HEADER_CURL_TOOL_CFGABLE_H
#define HEADER_CURL_TOOL_CFGABLE_H
/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * SPDX-License-Identifier: curl
 *
 ***************************************************************************/
#include "tool_setup.h"
#include "tool_sdecls.h"
#include "tool_urlglob.h"
#include "tool_formparse.h"

struct GlobalConfig;

struct State {
  struct getout *urlnode;
  struct URLGlob *inglob;
  struct URLGlob *urls;
  char *outfiles;
  char *httpgetfields;
  char *uploadfile;
  unsigned long infilenum; /* number of files to upload */
  unsigned long up;  /* upload file counter within a single upload glob */
  unsigned long urlnum; /* how many iterations this single URL has with ranges
                           etc */
  unsigned long li;
};

struct OperationConfig {
  bool remote_time;
  char *useragent;
  struct curl_slist *cookies;  /* cookies to serialize into a single line */
  char *cookiejar;          /* write to this file */
  struct curl_slist *cookiefiles;  /* file(s) to load cookies from */
  char *altsvc;             /* alt-svc cache file name */
  char *hsts;               /* HSTS cache file name */
  bool cookiesession;       /* new session? */
  bool encoding;            /* Accept-Encoding please */
  bool tr_encoding;         /* Transfer-Encoding please */
  unsigned long authtype;   /* auth bitmask */
  bool use_resume;
  bool resume_from_current;
  bool disable_epsv;
  bool disable_eprt;
  bool ftp_pret;
  char *proto_str;
  bool proto_present;
  char *proto_redir_str;
  bool proto_redir_present;
  char *proto_default;
  curl_off_t resume_from;
  char *postfields;
  curl_off_t postfieldsize;
  char *referer;
  char *query;
  long timeout_ms;
  long connecttimeout_ms;
  long maxredirs;
  curl_off_t max_filesize;
  char *output_dir;
  char *headerfile;
  char *ftpport;
  char *iface;
  long localport;
  long localportrange;
  unsigned short porttouse;
  char *range;
  long low_speed_limit;
  long low_speed_time;
  char *dns_servers;   /* dot notation: 1.1.1.1;2.2.2.2 */
  char *dns_interface; /* interface name */
  char *dns_ipv4_addr; /* dot notation */
  char *dns_ipv6_addr; /* dot notation */
  char *userpwd;
  char *login_options;
  char *tls_username;
  char *tls_password;
  char *tls_authtype;
  char *proxy_tls_username;
  char *proxy_tls_password;
  char *proxy_tls_authtype;
  char *proxyuserpwd;
  char *proxy;
  int proxyver;             /* set to CURLPROXY_HTTP* define */
  char *noproxy;
  char *mail_from;
  struct curl_slist *mail_rcpt;
  char *mail_auth;
  bool mail_rcpt_allowfails; /* --mail-rcpt-allowfails */
  char *sasl_authzid;       /* Authorization identity (identity to use) */
  bool sasl_ir;             /* Enable/disable SASL initial response */
  bool proxytunnel;
  bool ftp_append;          /* APPE on ftp */
  bool use_ascii;           /* select ascii or text transfer */
  bool autoreferer;         /* automatically set referer */
  bool failonerror;         /* fail on (HTTP) errors */
  bool failwithbody;        /* fail on (HTTP) errors but still store body */
  bool show_headers;        /* show headers to data output */
  bool no_body;             /* don't get the body */
  bool dirlistonly;         /* only get the FTP dir list */
  bool followlocation;      /* follow http redirects */
  bool unrestricted_auth;   /* Continue to send authentication (user+password)
                               when following ocations, even when hostname
                               changed */
  bool netrc_opt;
  bool netrc;
  char *netrc_file;
  struct getout *url_list;  /* point to the first node */
  struct getout *url_last;  /* point to the last/current node */
  struct getout *url_get;   /* point to the node to fill in URL */
  struct getout *url_out;   /* point to the node to fill in outfile */
  struct getout *url_ul;    /* point to the node to fill in upload */
  char *doh_url;
  char *cipher_list;
  char *proxy_cipher_list;
  char *cipher13_list;
  char *proxy_cipher13_list;
  char *cert;
  char *proxy_cert;
  char *cert_type;
  char *proxy_cert_type;
  char *cacert;
  char *proxy_cacert;
  char *capath;
  char *proxy_capath;
  char *crlfile;
  char *proxy_crlfile;
  char *pinnedpubkey;
  char *proxy_pinnedpubkey;
  char *key;
  char *proxy_key;
  char *key_type;
  char *proxy_key_type;
  char *key_passwd;
  char *proxy_key_passwd;
  char *pubkey;
  char *hostpubmd5;
  char *hostpubsha256;
  char *engine;
  char *etag_save_file;
  char *etag_compare_file;
  bool crlf;
  char *customrequest;
  char *ssl_ec_curves;
  char *krblevel;
  char *request_target;
  long httpversion;
  bool http09_allowed;
  bool nobuffer;
  bool readbusy;            /* set when reading input returns EAGAIN */
  bool globoff;
  bool use_httpget;
  bool insecure_ok;         /* set TRUE to allow insecure SSL connects */
  bool doh_insecure_ok;     /* set TRUE to allow insecure SSL connects
                               for DoH */
  bool proxy_insecure_ok;   /* set TRUE to allow insecure SSL connects
                               for proxy */
  bool terminal_binary_ok;
  bool verifystatus;
  bool doh_verifystatus;
  bool create_dirs;
  bool ftp_create_dirs;
  bool ftp_skip_ip;
  bool proxynegotiate;
  bool proxyntlm;
  bool proxydigest;
  bool proxybasic;
  bool proxyanyauth;
  bool jsoned; /* added json content-type */
  char *writeout;           /* %-styled format string to output */
  struct curl_slist *quote;
  struct curl_slist *postquote;
  struct curl_slist *prequote;
  long ssl_version;
  long ssl_version_max;
  long proxy_ssl_version;
  long ip_version;
  long create_file_mode; /* CURLOPT_NEW_FILE_PERMS */
  curl_TimeCond timecond;
  curl_off_t condtime;
  struct curl_slist *headers;
  struct curl_slist *proxyheaders;
  struct tool_mime *mimeroot;
  struct tool_mime *mimecurrent;
  curl_mime *mimepost;
  struct curl_slist *telnet_options;
  struct curl_slist *resolve;
  struct curl_slist *connect_to;
  HttpReq httpreq;

  /* for bandwidth limiting features: */
  curl_off_t sendpersecond; /* send to peer */
  curl_off_t recvpersecond; /* receive from peer */

  bool ftp_ssl;
  bool ftp_ssl_reqd;
  bool ftp_ssl_control;
  bool ftp_ssl_ccc;
  int ftp_ssl_ccc_mode;
  char *preproxy;
  bool socks5_gssapi_nec;   /* The NEC reference server does not protect the
                               encryption type exchange */
  unsigned long socks5_auth;/* auth bitmask for socks5 proxies */
  char *proxy_service_name; /* set authentication service name for HTTP and
                               SOCKS5 proxies */
  char *service_name;       /* set authentication service name for DIGEST-MD5,
                               Kerberos 5 and SPNEGO */

  bool tcp_nodelay;
  bool tcp_fastopen;
  long req_retry;           /* number of retries */
  bool retry_all_errors;    /* retry on any error */
  bool retry_connrefused;   /* set connection refused as a transient error */
  long retry_delay;         /* delay between retries (in seconds) */
  long retry_maxtime;       /* maximum time to keep retrying */

  char *ftp_account;        /* for ACCT */
  char *ftp_alternative_to_user;  /* send command if USER/PASS fails */
  int ftp_filemethod;
  long mime_options;        /* Mime option flags. */
  long tftp_blksize;        /* TFTP BLKSIZE option */
  bool tftp_no_options;     /* do not send TFTP options requests */
  bool ignorecl;            /* --ignore-content-length */
  bool disable_sessionid;

  bool raw;
  bool post301;
  bool post302;
  bool post303;
  bool nokeepalive;         /* for keepalive needs */
  long alivetime;
  bool content_disposition; /* use Content-disposition filename */

  int default_node_flags;   /* default flags to search for each 'node', which
                               is basically each given URL to transfer */

  bool xattr;               /* store metadata in extended attributes */
  long gssapi_delegation;
  bool ssl_allow_beast;     /* allow this SSL vulnerability */
  bool proxy_ssl_allow_beast; /* allow this SSL vulnerability for proxy */
  bool ssl_no_revoke;       /* disable SSL certificate revocation checks */
  bool ssl_revoke_best_effort; /* ignore SSL revocation offline/missing
                                  revocation list errors */

  bool native_ca_store;        /* use the native os ca store */
  bool ssl_auto_client_cert;   /* automatically locate and use a client
                                  certificate for authentication (Schannel) */
  bool proxy_ssl_auto_client_cert; /* proxy version of ssl_auto_client_cert */
  char *oauth_bearer;             /* OAuth 2.0 bearer token */
  bool noalpn;                    /* enable/disable TLS ALPN extension */
  char *unix_socket_path;         /* path to Unix domain socket */
  bool abstract_unix_socket;      /* path to an abstract Unix domain socket */
  bool falsestart;
  bool path_as_is;
  long expect100timeout_ms;
  bool suppress_connect_headers;  /* suppress proxy CONNECT response headers
                                     from user callbacks */
  bool synthetic_error;           /* if TRUE, this is tool-internal error */
  bool ssh_compression;           /* enable/disable SSH compression */
  long happy_eyeballs_timeout_ms; /* happy eyeballs timeout in milliseconds.
                                     0 is valid. default: CURL_HET_DEFAULT. */
  bool haproxy_protocol;          /* whether to send HAProxy protocol v1 */
  bool disallow_username_in_url;  /* disallow usernames in URLs */
  char *aws_sigv4;
  enum {
    CLOBBER_DEFAULT, /* Provides compatibility with previous versions of curl,
                        by using the default behavior for -o, -O, and -J.
                        If those options would have overwritten files, like
                        -o and -O would, then overwrite them. In the case of
                        -J, this will not overwrite any files. */
    CLOBBER_NEVER, /* If the file exists, always fail */
    CLOBBER_ALWAYS /* If the file exists, always overwrite it */
  } file_clobber_mode;
  struct GlobalConfig *global;
  struct OperationConfig *prev;
  struct OperationConfig *next;   /* Always last in the struct */
  struct State state;             /* for create_transfer() */
  bool rm_partial;                /* on error, remove partially written output
                                     files */
};

struct GlobalConfig {
  bool showerror;                 /* show errors when silent */
  bool silent;                    /* don't show messages, --silent given */
  bool noprogress;                /* don't show progress bar */
  bool isatty;                    /* Updated internally if output is a tty */
  char *trace_dump;               /* file to dump the network trace to */
  FILE *trace_stream;
  bool trace_fopened;
  trace tracetype;
  bool tracetime;                 /* include timestamp? */
  int progressmode;               /* CURL_PROGRESS_BAR / CURL_PROGRESS_STATS */
  char *libcurl;                  /* Output libcurl code to this file name */
  bool fail_early;                /* exit on first transfer error */
  bool styled_output;             /* enable fancy output style detection */
  long ms_per_transfer;           /* start next transfer after (at least) this
                                     many milliseconds */
#ifdef CURLDEBUG
  bool test_event_based;
#endif
  bool parallel;
  long parallel_max;
  bool parallel_connect;
  char *help_category;            /* The help category, if set */
  struct OperationConfig *first;
  struct OperationConfig *current;
  struct OperationConfig *last;   /* Always last in the struct */
};

void config_init(struct OperationConfig *config);
void config_free(struct OperationConfig *config);

#endif /* HEADER_CURL_TOOL_CFGABLE_H */
