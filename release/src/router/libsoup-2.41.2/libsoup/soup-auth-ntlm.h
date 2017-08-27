/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2007 Red Hat, Inc.
 */

#ifndef SOUP_AUTH_NTLM_H
#define SOUP_AUTH_NTLM_H 1

#include "soup-auth.h"

#define SOUP_AUTH_NTLM(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), SOUP_TYPE_AUTH_NTLM, SoupAuthNTLM))
#define SOUP_AUTH_NTLM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_TYPE_AUTH_NTLM, SoupAuthNTLMClass))
#define SOUP_IS_AUTH_NTLM(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), SOUP_TYPE_AUTH_NTLM))
#define SOUP_IS_AUTH_NTLM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SOUP_TYPE_AUTH_NTLM))
#define SOUP_AUTH_NTLM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_TYPE_AUTH_NTLM, SoupAuthNTLMClass))

typedef struct {
	SoupAuth parent;

} SoupAuthNTLM;

typedef struct {
	SoupAuthClass parent_class;

} SoupAuthNTLMClass;

SoupAuth   *soup_auth_ntlm_new          (const char *realm,
					 const char *host);
const char *soup_auth_ntlm_get_username (SoupAuth   *auth);
const char *soup_auth_ntlm_get_password (SoupAuth   *auth);

#endif /* SOUP_AUTH_NTLM_H */
