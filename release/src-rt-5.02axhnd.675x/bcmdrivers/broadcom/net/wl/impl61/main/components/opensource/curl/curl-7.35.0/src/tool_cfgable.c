/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2012, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/
#include "tool_setup.h"

#include "tool_cfgable.h"

#include "memdebug.h" /* keep this as LAST include */

void free_config_fields(struct Configurable *config)
{
  struct getout *urlnode;

  if(config->easy) {
    curl_easy_cleanup(config->easy);
    config->easy = NULL;
  }

  Curl_safefree(config->random_file);
  Curl_safefree(config->egd_file);
  Curl_safefree(config->useragent);
  Curl_safefree(config->cookie);
  Curl_safefree(config->cookiejar);
  Curl_safefree(config->cookiefile);

  Curl_safefree(config->postfields);
  Curl_safefree(config->referer);

  Curl_safefree(config->headerfile);
  Curl_safefree(config->ftpport);
  Curl_safefree(config->iface);

  Curl_safefree(config->range);

  Curl_safefree(config->userpwd);
  Curl_safefree(config->tls_username);
  Curl_safefree(config->tls_password);
  Curl_safefree(config->tls_authtype);
  Curl_safefree(config->proxyuserpwd);
  Curl_safefree(config->proxy);

  Curl_safefree(config->dns_ipv6_addr);
  Curl_safefree(config->dns_ipv4_addr);
  Curl_safefree(config->dns_interface);
  Curl_safefree(config->dns_servers);

  Curl_safefree(config->noproxy);

  Curl_safefree(config->mail_from);
  curl_slist_free_all(config->mail_rcpt);
  Curl_safefree(config->mail_auth);

  Curl_safefree(config->netrc_file);

  urlnode = config->url_list;
  while(urlnode) {
    struct getout *next = urlnode->next;
    Curl_safefree(urlnode->url);
    Curl_safefree(urlnode->outfile);
    Curl_safefree(urlnode->infile);
    Curl_safefree(urlnode);
    urlnode = next;
  }
  config->url_list = NULL;
  config->url_last = NULL;
  config->url_get = NULL;
  config->url_out = NULL;

  Curl_safefree(config->cipher_list);
  Curl_safefree(config->cert);
  Curl_safefree(config->cert_type);
  Curl_safefree(config->cacert);
  Curl_safefree(config->capath);
  Curl_safefree(config->crlfile);
  Curl_safefree(config->key);
  Curl_safefree(config->key_type);
  Curl_safefree(config->key_passwd);
  Curl_safefree(config->pubkey);
  Curl_safefree(config->hostpubmd5);
  Curl_safefree(config->engine);

  Curl_safefree(config->customrequest);
  Curl_safefree(config->krblevel);
  Curl_safefree(config->trace_dump);

  Curl_safefree(config->xoauth2_bearer);

  config->trace_stream = NULL; /* closed elsewhere when appropriate */

  Curl_safefree(config->writeout);

  config->errors = NULL; /* closed elsewhere when appropriate */

  curl_slist_free_all(config->quote);
  curl_slist_free_all(config->postquote);
  curl_slist_free_all(config->prequote);

  curl_slist_free_all(config->headers);

  if(config->httppost) {
    curl_formfree(config->httppost);
    config->httppost = NULL;
  }
  config->last_post = NULL;

  curl_slist_free_all(config->telnet_options);
  curl_slist_free_all(config->resolve);

  Curl_safefree(config->socksproxy);
  Curl_safefree(config->socks5_gssapi_service);

  Curl_safefree(config->ftp_account);
  Curl_safefree(config->ftp_alternative_to_user);

  Curl_safefree(config->libcurl);
}
