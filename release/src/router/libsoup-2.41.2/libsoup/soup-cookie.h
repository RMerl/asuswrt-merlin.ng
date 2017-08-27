/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * Copyright 2007, 2008 Red Hat, Inc.
 */

#ifndef SOUP_COOKIE_H
#define SOUP_COOKIE_H 1

#include <libsoup/soup-types.h>

G_BEGIN_DECLS

struct _SoupCookie {
	char     *name;
	char     *value;
	char     *domain;
	char     *path;
	SoupDate *expires;
	gboolean  secure;
	gboolean  http_only;
};

SOUP_AVAILABLE_IN_2_24
GType soup_cookie_get_type (void);
#define SOUP_TYPE_COOKIE (soup_cookie_get_type())

#define SOUP_COOKIE_MAX_AGE_ONE_HOUR (60 * 60)
#define SOUP_COOKIE_MAX_AGE_ONE_DAY  (SOUP_COOKIE_MAX_AGE_ONE_HOUR * 24)
#define SOUP_COOKIE_MAX_AGE_ONE_WEEK (SOUP_COOKIE_MAX_AGE_ONE_DAY * 7)
#define SOUP_COOKIE_MAX_AGE_ONE_YEAR (SOUP_COOKIE_MAX_AGE_ONE_DAY * 365.2422)

SOUP_AVAILABLE_IN_2_24
SoupCookie *soup_cookie_new                     (const char  *name,
						 const char  *value,
						 const char  *domain,
						 const char  *path,
						 int          max_age);
SOUP_AVAILABLE_IN_2_24
SoupCookie *soup_cookie_parse                   (const char  *header,
						 SoupURI     *origin);
SOUP_AVAILABLE_IN_2_24
SoupCookie *soup_cookie_copy                    (SoupCookie  *cookie);

SOUP_AVAILABLE_IN_2_32
const char *soup_cookie_get_name                (SoupCookie  *cookie);
SOUP_AVAILABLE_IN_2_24
void        soup_cookie_set_name                (SoupCookie  *cookie,
						 const char  *name);
SOUP_AVAILABLE_IN_2_32
const char *soup_cookie_get_value               (SoupCookie  *cookie);
SOUP_AVAILABLE_IN_2_24
void        soup_cookie_set_value               (SoupCookie  *cookie,
						 const char  *value);
SOUP_AVAILABLE_IN_2_32
const char *soup_cookie_get_domain              (SoupCookie  *cookie);
SOUP_AVAILABLE_IN_2_24
void        soup_cookie_set_domain              (SoupCookie  *cookie,
						 const char  *domain);
SOUP_AVAILABLE_IN_2_32
const char *soup_cookie_get_path                (SoupCookie  *cookie);
SOUP_AVAILABLE_IN_2_24
void        soup_cookie_set_path                (SoupCookie  *cookie,
						 const char  *path);
SOUP_AVAILABLE_IN_2_24
void        soup_cookie_set_max_age             (SoupCookie  *cookie,
						 int          max_age);
SOUP_AVAILABLE_IN_2_32
SoupDate   *soup_cookie_get_expires             (SoupCookie  *cookie);
SOUP_AVAILABLE_IN_2_24
void        soup_cookie_set_expires             (SoupCookie  *cookie,
						 SoupDate    *expires);
SOUP_AVAILABLE_IN_2_32
gboolean    soup_cookie_get_secure              (SoupCookie  *cookie);
SOUP_AVAILABLE_IN_2_24
void        soup_cookie_set_secure              (SoupCookie  *cookie,
						 gboolean     secure);
SOUP_AVAILABLE_IN_2_32
gboolean    soup_cookie_get_http_only           (SoupCookie  *cookie);
SOUP_AVAILABLE_IN_2_24
void        soup_cookie_set_http_only           (SoupCookie  *cookie,
						 gboolean     http_only);

SOUP_AVAILABLE_IN_2_24
char       *soup_cookie_to_set_cookie_header    (SoupCookie  *cookie);
SOUP_AVAILABLE_IN_2_24
char       *soup_cookie_to_cookie_header        (SoupCookie  *cookie);

SOUP_AVAILABLE_IN_2_24
gboolean    soup_cookie_applies_to_uri          (SoupCookie  *cookie,
						 SoupURI     *uri);
SOUP_AVAILABLE_IN_2_24
gboolean    soup_cookie_equal                   (SoupCookie  *cookie1,
						 SoupCookie  *cookie2);

SOUP_AVAILABLE_IN_2_24
void        soup_cookie_free                    (SoupCookie  *cookie);

SOUP_AVAILABLE_IN_2_24
GSList     *soup_cookies_from_response          (SoupMessage *msg);
SOUP_AVAILABLE_IN_2_24
GSList     *soup_cookies_from_request           (SoupMessage *msg);

SOUP_AVAILABLE_IN_2_24
void        soup_cookies_to_response            (GSList      *cookies,
						 SoupMessage *msg);
SOUP_AVAILABLE_IN_2_24
void        soup_cookies_to_request             (GSList      *cookies,
						 SoupMessage *msg);

SOUP_AVAILABLE_IN_2_24
void        soup_cookies_free                   (GSList      *cookies);

SOUP_AVAILABLE_IN_2_24
char       *soup_cookies_to_cookie_header       (GSList      *cookies);

SOUP_AVAILABLE_IN_2_30
gboolean    soup_cookie_domain_matches          (SoupCookie  *cookie,
						 const char  *host);

G_END_DECLS

#endif /* SOUP_COOKIE_H */
