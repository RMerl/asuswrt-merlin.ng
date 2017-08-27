/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2008 Red Hat, Inc.
 */

#ifndef SOUP_AUTH_MANAGER_NTLM_H
#define SOUP_AUTH_MANAGER_NTLM_H 1

#include "soup-auth-manager.h"

G_BEGIN_DECLS

#define SOUP_TYPE_AUTH_MANAGER_NTLM            (soup_auth_manager_ntlm_get_type ())
#define SOUP_AUTH_MANAGER_NTLM(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), SOUP_TYPE_AUTH_MANAGER_NTLM, SoupAuthManagerNTLM))
#define SOUP_AUTH_MANAGER_NTLM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_TYPE_AUTH_MANAGER_NTLM, SoupAuthManagerNTLMClass))
#define SOUP_IS_AUTH_MANAGER_NTLM(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), SOUP_TYPE_AUTH_MANAGER_NTLM))
#define SOUP_IS_AUTH_MANAGER_NTLM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SOUP_TYPE_AUTH_MANAGER_NTLM))
#define SOUP_AUTH_MANAGER_NTLM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_TYPE_AUTH_MANAGER_NTLM, SoupAuthManagerNTLMClass))

typedef struct {
	SoupAuthManager parent;

} SoupAuthManagerNTLM;

typedef struct {
	SoupAuthManagerClass parent_class;

} SoupAuthManagerNTLMClass;

#define SOUP_AUTH_MANAGER_NTLM_USE_NTLM "use-ntlm"

GType soup_auth_manager_ntlm_get_type (void);

G_END_DECLS

#endif /* SOUP_AUTH_MANAGER_NTLM_NTLM_H */
