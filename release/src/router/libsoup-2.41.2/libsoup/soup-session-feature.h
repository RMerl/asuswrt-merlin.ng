/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2008 Red Hat, Inc.
 */

#ifndef SOUP_SESSION_FEATURE_H
#define SOUP_SESSION_FEATURE_H 1

#include <libsoup/soup-types.h>

G_BEGIN_DECLS

#define SOUP_TYPE_SESSION_FEATURE            (soup_session_feature_get_type ())
#define SOUP_SESSION_FEATURE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOUP_TYPE_SESSION_FEATURE, SoupSessionFeature))
#define SOUP_SESSION_FEATURE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_TYPE_SESSION_FEATURE, SoupSessionFeatureInterface))
#define SOUP_IS_SESSION_FEATURE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SOUP_TYPE_SESSION_FEATURE))
#define SOUP_IS_SESSION_FEATURE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SOUP_TYPE_SESSION_FEATURE))
#define SOUP_SESSION_FEATURE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), SOUP_TYPE_SESSION_FEATURE, SoupSessionFeatureInterface))

typedef struct {
	GTypeInterface parent;

	/* methods */
	void     (*attach)           (SoupSessionFeature *feature,
				      SoupSession        *session);
	void     (*detach)           (SoupSessionFeature *feature,
				      SoupSession        *session);

	void     (*request_queued)   (SoupSessionFeature *feature,
				      SoupSession        *session,
				      SoupMessage        *msg);
	void     (*request_started)  (SoupSessionFeature *feature,
				      SoupSession        *session,
				      SoupMessage        *msg,
				      SoupSocket         *socket);
	void     (*request_unqueued) (SoupSessionFeature *feature,
				      SoupSession        *session,
				      SoupMessage        *msg);

	gboolean (*add_feature)      (SoupSessionFeature *feature,
				      GType               type);
	gboolean (*remove_feature)   (SoupSessionFeature *feature,
				      GType               type);
	gboolean (*has_feature)      (SoupSessionFeature *feature,
				      GType               type);

} SoupSessionFeatureInterface;

SOUP_AVAILABLE_IN_2_24
GType    soup_session_feature_get_type       (void);

SOUP_AVAILABLE_IN_2_24
void     soup_session_feature_attach         (SoupSessionFeature *feature,
					      SoupSession        *session);
SOUP_AVAILABLE_IN_2_24
void     soup_session_feature_detach         (SoupSessionFeature *feature,
					      SoupSession        *session);

SOUP_AVAILABLE_IN_2_34
gboolean soup_session_feature_add_feature    (SoupSessionFeature *feature,
					      GType               type);
SOUP_AVAILABLE_IN_2_34
gboolean soup_session_feature_remove_feature (SoupSessionFeature *feature,
					      GType               type);
SOUP_AVAILABLE_IN_2_34
gboolean soup_session_feature_has_feature    (SoupSessionFeature *feature,
					      GType               type);

G_END_DECLS

#endif /* SOUP_SESSION_FEATURE_H */
