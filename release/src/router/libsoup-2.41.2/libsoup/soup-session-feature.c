/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-session-feature.c: Miscellaneous session feature-provider interface
 *
 * Copyright (C) 2008 Red Hat, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "soup-session-feature.h"
#include "soup.h"
#include "soup-message-private.h"

/**
 * SECTION:soup-session-feature
 * @short_description: Interface for miscellaneous session features
 *
 * #SoupSessionFeature is the interface used by classes that extend
 * the functionality of a #SoupSession. Some features like HTTP
 * authentication handling are implemented internally via
 * #SoupSessionFeature<!-- -->s. Other features can be added to the session
 * by the application. (Eg, #SoupLogger, #SoupCookieJar.)
 *
 * See soup_session_add_feature(), etc, to add a feature to a session.
 **/

/**
 * SoupSessionFeature:
 *
 * An object that implement some sort of optional feature for
 * #SoupSession.
 *
 * Since: 2.24
 **/

/**
 * SoupSessionFeatureInterface:
 * @parent: The parent interface.
 * @attach: Perform setup when a feature is added to a session
 * @detach: Perform cleanup when a feature is removed from a session
 * @request_queued: Proxies the session's #SoupSession::request_queued signal
 * @request_started: Proxies the session's #SoupSession::request_started signal
 * @request_unqueued: Proxies the session's #SoupSession::request_unqueued signal
 * @add_feature: adds a sub-feature to the main feature
 * @remove_feature: removes a sub-feature from the main feature
 * @has_feature: tests if the feature includes a sub-feature
 *
 * The interface implemented by #SoupSessionFeature<!-- -->s.
 *
 * Since: 2.24
 **/

static void soup_session_feature_default_init (SoupSessionFeatureInterface *iface);

G_DEFINE_INTERFACE (SoupSessionFeature, soup_session_feature, G_TYPE_OBJECT)

static void
weak_notify_unref (gpointer feature, GObject *ex_object)
{
	g_object_unref (feature);
}

static void
request_queued (SoupSession *session, SoupMessage *msg, gpointer feature)
{
	if (soup_message_disables_feature (msg, feature))
		return;

	SOUP_SESSION_FEATURE_GET_CLASS (feature)->
		request_queued (feature, session, msg);
}

static void
request_started (SoupSession *session, SoupMessage *msg,
		 SoupSocket *socket, gpointer feature)
{
	if (soup_message_disables_feature (msg, feature))
		return;

	SOUP_SESSION_FEATURE_GET_CLASS (feature)->
		request_started (feature, session, msg, socket);
}

static void
request_unqueued (SoupSession *session, SoupMessage *msg, gpointer feature)
{
	if (soup_message_disables_feature (msg, feature))
		return;

	SOUP_SESSION_FEATURE_GET_CLASS (feature)->
		request_unqueued (feature, session, msg);
}

static void
soup_session_feature_real_attach (SoupSessionFeature *feature, SoupSession *session)
{
	g_object_weak_ref (G_OBJECT (session),
			   weak_notify_unref, g_object_ref (feature));

	if (SOUP_SESSION_FEATURE_GET_CLASS (feature)->request_queued) {
		g_signal_connect (session, "request_queued",
				  G_CALLBACK (request_queued), feature);
	}

	if (SOUP_SESSION_FEATURE_GET_CLASS (feature)->request_started) {
		g_signal_connect (session, "request_started",
				  G_CALLBACK (request_started), feature);
	}

	if (SOUP_SESSION_FEATURE_GET_CLASS (feature)->request_unqueued) {
		g_signal_connect (session, "request_unqueued",
				  G_CALLBACK (request_unqueued), feature);
	}
}

void
soup_session_feature_attach (SoupSessionFeature *feature,
			     SoupSession        *session)
{
	SOUP_SESSION_FEATURE_GET_CLASS (feature)->attach (feature, session);
}

static void
soup_session_feature_real_detach (SoupSessionFeature *feature, SoupSession *session)
{
	g_object_weak_unref (G_OBJECT (session), weak_notify_unref, feature);

	g_signal_handlers_disconnect_by_func (session, request_queued, feature);
	g_signal_handlers_disconnect_by_func (session, request_started, feature);
	g_signal_handlers_disconnect_by_func (session, request_unqueued, feature);

	g_object_unref (feature);
}

void
soup_session_feature_detach (SoupSessionFeature *feature,
			     SoupSession        *session)
{
	SOUP_SESSION_FEATURE_GET_CLASS (feature)->detach (feature, session);
}

static void
soup_session_feature_default_init (SoupSessionFeatureInterface *iface)
{
	iface->attach = soup_session_feature_real_attach;
	iface->detach = soup_session_feature_real_detach;
}

/**
 * soup_session_feature_add_feature:
 * @feature: the "base" feature
 * @type: the #GType of a "sub-feature"
 *
 * Adds a "sub-feature" of type @type to the base feature @feature.
 * This is used for features that can be extended with multiple
 * different types. Eg, the authentication manager can be extended
 * with subtypes of #SoupAuth.
 *
 * Return value: %TRUE if @feature accepted @type as a subfeature.
 *
 * Since: 2.34
 */
gboolean
soup_session_feature_add_feature (SoupSessionFeature *feature,
				  GType               type)
{
	SoupSessionFeatureInterface *feature_iface =
              SOUP_SESSION_FEATURE_GET_CLASS (feature);

	if (feature_iface->add_feature)
		return feature_iface->add_feature (feature, type);
	else
		return FALSE;
}

/**
 * soup_session_feature_remove_feature:
 * @feature: the "base" feature
 * @type: the #GType of a "sub-feature"
 *
 * Removes the "sub-feature" of type @type from the base feature
 * @feature. See soup_session_feature_add_feature().
 *
 * Return value: %TRUE if @type was removed from @feature
 *
 * Since: 2.34
 */
gboolean
soup_session_feature_remove_feature (SoupSessionFeature *feature,
				     GType               type)
{
	SoupSessionFeatureInterface *feature_iface =
              SOUP_SESSION_FEATURE_GET_CLASS (feature);

	if (feature_iface->remove_feature)
		return feature_iface->remove_feature (feature, type);
	else
		return FALSE;
}

/**
 * soup_session_feature_has_feature:
 * @feature: the "base" feature
 * @type: the #GType of a "sub-feature"
 *
 * Tests if @feature has a "sub-feature" of type @type. See
 * soup_session_feature_add_feature().
 *
 * Return value: %TRUE if @feature has a subfeature of type @type
 *
 * Since: 2.34
 */
gboolean
soup_session_feature_has_feature (SoupSessionFeature *feature,
				  GType               type)
{
	SoupSessionFeatureInterface *feature_iface =
              SOUP_SESSION_FEATURE_GET_CLASS (feature);

	if (feature_iface->has_feature)
		return feature_iface->has_feature (feature, type);
	else
		return FALSE;
}
