/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-auth-domain.c: HTTP Authentication Domain (server-side)
 *
 * Copyright (C) 2007 Novell, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "soup-auth-domain.h"
#include "soup.h"
#include "soup-path-map.h"

/**
 * SECTION:soup-auth-domain
 * @short_description: Server-side authentication
 * @see_also: #SoupServer
 *
 * A #SoupAuthDomain manages authentication for all or part of a
 * #SoupServer. To make a server require authentication, first create
 * an appropriate subclass of #SoupAuthDomain, and then add it to the
 * server with soup_server_add_auth_domain().
 *
 * In order for an auth domain to have any effect, you must add one or
 * more paths to it (via soup_auth_domain_add_path() or the
 * %SOUP_AUTH_DOMAIN_ADD_PATH property). To require authentication for
 * all ordinary requests, add the path "/". (Note that this does not
 * include the special "*" URI (eg, "OPTIONS *"), which must be added
 * as a separate path if you want to cover it.)
 *
 * If you need greater control over which requests should and
 * shouldn't be authenticated, add paths covering everything you
 * <emphasis>might</emphasis> want authenticated, and then use a
 * filter (soup_auth_domain_set_filter()) to bypass authentication for
 * those requests that don't need it.
 **/

enum {
	PROP_0,

	PROP_REALM,
	PROP_PROXY,
	PROP_ADD_PATH,
	PROP_REMOVE_PATH,
	PROP_FILTER,
	PROP_FILTER_DATA,
	PROP_GENERIC_AUTH_CALLBACK,
	PROP_GENERIC_AUTH_DATA,

	LAST_PROP
};

typedef struct {
	char *realm;
	gboolean proxy;
	SoupPathMap *paths;

	SoupAuthDomainFilter filter;
	gpointer filter_data;
	GDestroyNotify filter_dnotify;

	SoupAuthDomainGenericAuthCallback auth_callback;
	gpointer auth_data;
	GDestroyNotify auth_dnotify;

} SoupAuthDomainPrivate;

#define SOUP_AUTH_DOMAIN_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), SOUP_TYPE_AUTH_DOMAIN, SoupAuthDomainPrivate))

G_DEFINE_ABSTRACT_TYPE (SoupAuthDomain, soup_auth_domain, G_TYPE_OBJECT)

static void
soup_auth_domain_init (SoupAuthDomain *domain)
{
	SoupAuthDomainPrivate *priv = SOUP_AUTH_DOMAIN_GET_PRIVATE (domain);

	priv->paths = soup_path_map_new (NULL);
}

static void
soup_auth_domain_finalize (GObject *object)
{
	SoupAuthDomainPrivate *priv = SOUP_AUTH_DOMAIN_GET_PRIVATE (object);

	g_free (priv->realm);
	soup_path_map_free (priv->paths);

	if (priv->filter_dnotify)
		priv->filter_dnotify (priv->filter_data);
	if (priv->auth_dnotify)
		priv->auth_dnotify (priv->auth_data);

	G_OBJECT_CLASS (soup_auth_domain_parent_class)->finalize (object);
}

static void
soup_auth_domain_set_property (GObject *object, guint prop_id,
			       const GValue *value, GParamSpec *pspec)
{
	SoupAuthDomain *auth_domain = SOUP_AUTH_DOMAIN (object);
	SoupAuthDomainPrivate *priv = SOUP_AUTH_DOMAIN_GET_PRIVATE (object);

	switch (prop_id) {
	case PROP_REALM:
		g_free (priv->realm);
		priv->realm = g_value_dup_string (value);
		break;
	case PROP_PROXY:
		priv->proxy = g_value_get_boolean (value);
		break;
	case PROP_ADD_PATH:
		soup_auth_domain_add_path (auth_domain,
					   g_value_get_string (value));
		break;
	case PROP_REMOVE_PATH:
		soup_auth_domain_remove_path (auth_domain,
					      g_value_get_string (value));
		break;
	case PROP_FILTER:
		priv->filter = g_value_get_pointer (value);
		break;
	case PROP_FILTER_DATA:
		if (priv->filter_dnotify) {
			priv->filter_dnotify (priv->filter_data);
			priv->filter_dnotify = NULL;
		}
		priv->filter_data = g_value_get_pointer (value);
		break;
	case PROP_GENERIC_AUTH_CALLBACK:
		priv->auth_callback = g_value_get_pointer (value);
		break;
	case PROP_GENERIC_AUTH_DATA:
		if (priv->auth_dnotify) {
			priv->auth_dnotify (priv->auth_data);
			priv->auth_dnotify = NULL;
		}
		priv->auth_data = g_value_get_pointer (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
soup_auth_domain_get_property (GObject *object, guint prop_id,
			       GValue *value, GParamSpec *pspec)
{
	SoupAuthDomainPrivate *priv = SOUP_AUTH_DOMAIN_GET_PRIVATE (object);

	switch (prop_id) {
	case PROP_REALM:
		g_value_set_string (value, priv->realm);
		break;
	case PROP_PROXY:
		g_value_set_boolean (value, priv->proxy);
		break;
	case PROP_FILTER:
		g_value_set_pointer (value, priv->filter);
		break;
	case PROP_FILTER_DATA:
		g_value_set_pointer (value, priv->filter_data);
		break;
	case PROP_GENERIC_AUTH_CALLBACK:
		g_value_set_pointer (value, priv->auth_callback);
		break;
	case PROP_GENERIC_AUTH_DATA:
		g_value_set_pointer (value, priv->auth_data);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
soup_auth_domain_class_init (SoupAuthDomainClass *auth_domain_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (auth_domain_class);

	g_type_class_add_private (auth_domain_class, sizeof (SoupAuthDomainPrivate));

	object_class->finalize = soup_auth_domain_finalize;
	object_class->set_property = soup_auth_domain_set_property;
	object_class->get_property = soup_auth_domain_get_property;

	/**
	 * SOUP_AUTH_DOMAIN_REALM:
	 *
	 * Alias for the #SoupAuthDomain:realm property. (The realm of
	 * this auth domain.)
	 **/
	g_object_class_install_property (
		object_class, PROP_REALM,
		g_param_spec_string (SOUP_AUTH_DOMAIN_REALM,
				     "Realm",
				     "The realm of this auth domain",
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * SOUP_AUTH_DOMAIN_PROXY:
	 *
	 * Alias for the #SoupAuthDomain:proxy property. (Whether or
	 * not this is a proxy auth domain.)
	 **/
	g_object_class_install_property (
		object_class, PROP_PROXY,
		g_param_spec_boolean (SOUP_AUTH_DOMAIN_PROXY,
				      "Proxy",
				      "Whether or not this is a proxy auth domain",
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * SOUP_AUTH_DOMAIN_ADD_PATH:
	 *
	 * Alias for the #SoupAuthDomain:add-path property. (Shortcut
	 * for calling soup_auth_domain_add_path().)
	 **/
	g_object_class_install_property (
		object_class, PROP_ADD_PATH,
		g_param_spec_string (SOUP_AUTH_DOMAIN_ADD_PATH,
				     "Add a path",
				     "Add a path covered by this auth domain",
				     NULL,
				     G_PARAM_WRITABLE));
	/**
	 * SOUP_AUTH_DOMAIN_REMOVE_PATH:
	 *
	 * Alias for the #SoupAuthDomain:remove-path property.
	 * (Shortcut for calling soup_auth_domain_remove_path().)
	 **/
	g_object_class_install_property (
		object_class, PROP_REMOVE_PATH,
		g_param_spec_string (SOUP_AUTH_DOMAIN_REMOVE_PATH,
				     "Remove a path",
				     "Remove a path covered by this auth domain",
				     NULL,
				     G_PARAM_WRITABLE));
	/**
	 * SOUP_AUTH_DOMAIN_FILTER:
	 *
	 * Alias for the #SoupAuthDomain:filter property. (The
	 * #SoupAuthDomainFilter for the domain.)
	 **/
	g_object_class_install_property (
		object_class, PROP_FILTER,
		g_param_spec_pointer (SOUP_AUTH_DOMAIN_FILTER,
				      "Filter",
				      "A filter for deciding whether or not to require authentication",
				      G_PARAM_READWRITE));
	/**
	 * SOUP_AUTH_DOMAIN_FILTER_DATA:
	 *
	 * Alias for the #SoupAuthDomain:filter-data property. (Data
	 * to pass to the #SoupAuthDomainFilter.)
	 **/
	g_object_class_install_property (
		object_class, PROP_FILTER_DATA,
		g_param_spec_pointer (SOUP_AUTH_DOMAIN_FILTER_DATA,
				      "Filter data",
				      "Data to pass to filter",
				      G_PARAM_READWRITE));
	/**
	 * SOUP_AUTH_DOMAIN_GENERIC_AUTH_CALLBACK:
	 *
	 * Alias for the #SoupAuthDomain:generic-auth-callback property.
	 * (The #SoupAuthDomainGenericAuthCallback.)
	 **/
	g_object_class_install_property (
		object_class, PROP_GENERIC_AUTH_CALLBACK,
		g_param_spec_pointer (SOUP_AUTH_DOMAIN_GENERIC_AUTH_CALLBACK,
				      "Generic authentication callback",
				      "An authentication callback that can be used with any SoupAuthDomain subclass",
				      G_PARAM_READWRITE));
	/**
	 * SOUP_AUTH_DOMAIN_GENERIC_AUTH_DATA:
	 *
	 * Alias for the #SoupAuthDomain:generic-auth-data property.
	 * (The data to pass to the #SoupAuthDomainGenericAuthCallback.)
	 **/
	g_object_class_install_property (
		object_class, PROP_GENERIC_AUTH_DATA,
		g_param_spec_pointer (SOUP_AUTH_DOMAIN_GENERIC_AUTH_DATA,
				      "Authentication callback data",
				      "Data to pass to auth callback",
				      G_PARAM_READWRITE));
}

/**
 * soup_auth_domain_add_path:
 * @domain: a #SoupAuthDomain
 * @path: the path to add to @domain
 *
 * Adds @path to @domain, such that requests under @path on @domain's
 * server will require authentication (unless overridden by
 * soup_auth_domain_remove_path() or soup_auth_domain_set_filter()).
 *
 * You can also add paths by setting the %SOUP_AUTH_DOMAIN_ADD_PATH
 * property, which can also be used to add one or more paths at
 * construct time.
 **/
void
soup_auth_domain_add_path (SoupAuthDomain *domain, const char *path)
{
	SoupAuthDomainPrivate *priv = SOUP_AUTH_DOMAIN_GET_PRIVATE (domain);

	/* "" should not match "*" */
	if (!*path)
		path = "/";

	soup_path_map_add (priv->paths, path, GINT_TO_POINTER (TRUE));
}

/**
 * soup_auth_domain_remove_path:
 * @domain: a #SoupAuthDomain
 * @path: the path to remove from @domain
 *
 * Removes @path from @domain, such that requests under @path on
 * @domain's server will NOT require authentication.
 *
 * This is not simply an undo-er for soup_auth_domain_add_path(); it
 * can be used to "carve out" a subtree that does not require
 * authentication inside a hierarchy that does. Note also that unlike
 * with soup_auth_domain_add_path(), this cannot be overridden by
 * adding a filter, as filters can only bypass authentication that
 * would otherwise be required, not require it where it would
 * otherwise be unnecessary.
 *
 * You can also remove paths by setting the
 * %SOUP_AUTH_DOMAIN_REMOVE_PATH property, which can also be used to
 * remove one or more paths at construct time.
 **/
void
soup_auth_domain_remove_path (SoupAuthDomain *domain, const char *path)
{
	SoupAuthDomainPrivate *priv = SOUP_AUTH_DOMAIN_GET_PRIVATE (domain);

	/* "" should not match "*" */
	if (!*path)
		path = "/";

	soup_path_map_add (priv->paths, path, GINT_TO_POINTER (FALSE));
}

/**
 * SoupAuthDomainFilter:
 * @domain: a #SoupAuthDomain
 * @msg: a #SoupMessage
 * @user_data: the data passed to soup_auth_domain_set_filter()
 *
 * The prototype for a #SoupAuthDomain filter; see
 * soup_auth_domain_set_filter() for details.
 *
 * Return value: %TRUE if @msg requires authentication, %FALSE if not.
 **/

/**
 * soup_auth_domain_set_filter:
 * @domain: a #SoupAuthDomain
 * @filter: the auth filter for @domain
 * @filter_data: data to pass to @filter
 * @dnotify: destroy notifier to free @filter_data when @domain
 * is destroyed
 *
 * Adds @filter as an authentication filter to @domain. The filter
 * gets a chance to bypass authentication for certain requests that
 * would otherwise require it. Eg, it might check the message's path
 * in some way that is too complicated to do via the other methods, or
 * it might check the message's method, and allow GETs but not PUTs.
 *
 * The filter function returns %TRUE if the request should still
 * require authentication, or %FALSE if authentication is unnecessary
 * for this request.
 *
 * To help prevent security holes, your filter should return %TRUE by
 * default, and only return %FALSE under specifically-tested
 * circumstances, rather than the other way around. Eg, in the example
 * above, where you want to authenticate PUTs but not GETs, you should
 * check if the method is GET and return %FALSE in that case, and then
 * return %TRUE for all other methods (rather than returning %TRUE for
 * PUT and %FALSE for all other methods). This way if it turned out
 * (now or later) that some paths supported additional methods besides
 * GET and PUT, those methods would default to being NOT allowed for
 * unauthenticated users.
 *
 * You can also set the filter by setting the %SOUP_AUTH_DOMAIN_FILTER
 * and %SOUP_AUTH_DOMAIN_FILTER_DATA properties, which can also be
 * used to set the filter at construct time.
 **/
void
soup_auth_domain_set_filter (SoupAuthDomain *domain,
			     SoupAuthDomainFilter filter,
			     gpointer        filter_data,
			     GDestroyNotify  dnotify)
{
	SoupAuthDomainPrivate *priv = SOUP_AUTH_DOMAIN_GET_PRIVATE (domain);

	if (priv->filter_dnotify)
		priv->filter_dnotify (priv->filter_data);

	priv->filter = filter;
	priv->filter_data = filter_data;
	priv->filter_dnotify = dnotify;

	g_object_notify (G_OBJECT (domain), SOUP_AUTH_DOMAIN_FILTER);
	g_object_notify (G_OBJECT (domain), SOUP_AUTH_DOMAIN_FILTER_DATA);
}

/**
 * soup_auth_domain_get_realm:
 * @domain: a #SoupAuthDomain
 *
 * Gets the realm name associated with @domain
 *
 * Return value: @domain's realm
 **/
const char *
soup_auth_domain_get_realm (SoupAuthDomain *domain)
{
	SoupAuthDomainPrivate *priv = SOUP_AUTH_DOMAIN_GET_PRIVATE (domain);

	return priv->realm;
}

/**
 * SoupAuthDomainGenericAuthCallback:
 * @domain: a #SoupAuthDomain
 * @msg: the #SoupMessage being authenticated
 * @username: the username from @msg
 * @user_data: the data passed to
 * soup_auth_domain_set_generic_auth_callback()
 *
 * The prototype for a #SoupAuthDomain generic authentication callback.
 *
 * The callback should look up the user's password, call
 * soup_auth_domain_check_password(), and use the return value from
 * that method as its own return value.
 *
 * In general, for security reasons, it is preferable to use the
 * auth-domain-specific auth callbacks (eg,
 * #SoupAuthDomainBasicAuthCallback and
 * #SoupAuthDomainDigestAuthCallback), because they don't require
 * keeping a cleartext password database. Most users will use the same
 * password for many different sites, meaning if any site with a
 * cleartext password database is compromised, accounts on other
 * servers might be compromised as well. For many of the cases where
 * #SoupServer is used, this is not really relevant, but it may still
 * be worth considering.
 *
 * Return value: %TRUE if @msg is authenticated, %FALSE if not.
 **/

/**
 * soup_auth_domain_set_generic_auth_callback:
 * @domain: a #SoupAuthDomain
 * @auth_callback: the auth callback
 * @auth_data: data to pass to @auth_callback
 * @dnotify: destroy notifier to free @auth_data when @domain
 * is destroyed
 *
 * Sets @auth_callback as an authentication-handling callback for
 * @domain. Whenever a request comes in to @domain which cannot be
 * authenticated via a domain-specific auth callback (eg,
 * #SoupAuthDomainDigestAuthCallback), the generic auth callback
 * will be invoked. See #SoupAuthDomainGenericAuthCallback for information
 * on what the callback should do.
 **/
void
soup_auth_domain_set_generic_auth_callback (SoupAuthDomain *domain,
					    SoupAuthDomainGenericAuthCallback auth_callback,
					    gpointer        auth_data,
					    GDestroyNotify  dnotify)
{
	SoupAuthDomainPrivate *priv = SOUP_AUTH_DOMAIN_GET_PRIVATE (domain);

	if (priv->auth_dnotify)
		priv->auth_dnotify (priv->auth_data);

	priv->auth_callback = auth_callback;
	priv->auth_data = auth_data;
	priv->auth_dnotify = dnotify;

	g_object_notify (G_OBJECT (domain), SOUP_AUTH_DOMAIN_GENERIC_AUTH_CALLBACK);
	g_object_notify (G_OBJECT (domain), SOUP_AUTH_DOMAIN_GENERIC_AUTH_DATA);
}

gboolean
soup_auth_domain_try_generic_auth_callback (SoupAuthDomain *domain,
					    SoupMessage    *msg,
					    const char     *username)
{
	SoupAuthDomainPrivate *priv = SOUP_AUTH_DOMAIN_GET_PRIVATE (domain);

	if (priv->auth_callback)
		return priv->auth_callback (domain, msg, username, priv->auth_data);
	else
		return FALSE;
}

/**
 * soup_auth_domain_check_password:
 * @domain: a #SoupAuthDomain
 * @msg: a #SoupMessage
 * @username: a username
 * @password: a password
 *
 * Checks if @msg authenticates to @domain via @username and
 * @password. This would normally be called from a
 * #SoupAuthDomainGenericAuthCallback.
 *
 * Return value: whether or not the message is authenticated
 **/
gboolean
soup_auth_domain_check_password (SoupAuthDomain *domain,
				 SoupMessage    *msg,
				 const char     *username,
				 const char     *password)
{
	return SOUP_AUTH_DOMAIN_GET_CLASS (domain)->check_password (domain, msg,
								    username,
								    password);
}

/**
 * soup_auth_domain_covers:
 * @domain: a #SoupAuthDomain
 * @msg: a #SoupMessage
 *
 * Checks if @domain requires @msg to be authenticated (according to
 * its paths and filter function). This does not actually look at
 * whether @msg <emphasis>is</emphasis> authenticated, merely whether
 * or not it needs to be.
 *
 * This is used by #SoupServer internally and is probably of no use to
 * anyone else.
 *
 * Return value: %TRUE if @domain requires @msg to be authenticated
 **/
gboolean
soup_auth_domain_covers (SoupAuthDomain *domain, SoupMessage *msg)
{
	SoupAuthDomainPrivate *priv = SOUP_AUTH_DOMAIN_GET_PRIVATE (domain);
	const char *path;

	if (!priv->proxy) {
		path = soup_message_get_uri (msg)->path;
		if (!soup_path_map_lookup (priv->paths, path))
			return FALSE;
	}

	if (priv->filter && !priv->filter (domain, msg, priv->filter_data))
		return FALSE;
	else
		return TRUE;
}

/**
 * soup_auth_domain_accepts:
 * @domain: a #SoupAuthDomain
 * @msg: a #SoupMessage
 *
 * Checks if @msg contains appropriate authorization for @domain to
 * accept it. Mirroring soup_auth_domain_covers(), this does not check
 * whether or not @domain <emphasis>cares</emphasis> if @msg is
 * authorized.
 *
 * This is used by #SoupServer internally and is probably of no use to
 * anyone else.
 *
 * Return value: the username that @msg has authenticated as, if in
 * fact it has authenticated. %NULL otherwise.
 **/
char *
soup_auth_domain_accepts (SoupAuthDomain *domain, SoupMessage *msg)
{
	SoupAuthDomainPrivate *priv = SOUP_AUTH_DOMAIN_GET_PRIVATE (domain);
	const char *header;

	header = soup_message_headers_get_one (msg->request_headers,
					       priv->proxy ?
					       "Proxy-Authorization" :
					       "Authorization");
	if (!header)
		return NULL;
	return SOUP_AUTH_DOMAIN_GET_CLASS (domain)->accepts (domain, msg, header);
}

/**
 * soup_auth_domain_challenge:
 * @domain: a #SoupAuthDomain
 * @msg: a #SoupMessage
 *
 * Adds a "WWW-Authenticate" or "Proxy-Authenticate" header to @msg,
 * requesting that the client authenticate, and sets @msg's status
 * accordingly.
 *
 * This is used by #SoupServer internally and is probably of no use to
 * anyone else.
 **/
void
soup_auth_domain_challenge (SoupAuthDomain *domain, SoupMessage *msg)
{
	SoupAuthDomainPrivate *priv = SOUP_AUTH_DOMAIN_GET_PRIVATE (domain);
	char *challenge;

	challenge = SOUP_AUTH_DOMAIN_GET_CLASS (domain)->challenge (domain, msg);
	soup_message_set_status (msg, priv->proxy ?
				 SOUP_STATUS_PROXY_UNAUTHORIZED :
				 SOUP_STATUS_UNAUTHORIZED);
	soup_message_headers_append (msg->response_headers,
				     priv->proxy ?
				     "Proxy-Authenticate" :
				     "WWW-Authenticate",
				     challenge);
	g_free (challenge);
}
