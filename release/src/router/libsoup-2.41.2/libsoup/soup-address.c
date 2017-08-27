/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-address.c: Internet address handing
 *
 * Copyright (C) 2010 Red Hat, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "soup-address.h"
#include "soup.h"
#include "soup-marshal.h"

/**
 * SECTION:soup-address
 * @short_description: DNS support
 *
 * #SoupAddress represents the address of a TCP connection endpoint:
 * both the IP address and the port. (It is somewhat like an
 * object-oriented version of struct sockaddr.)
 **/

enum {
	PROP_0,

	PROP_NAME,
	PROP_FAMILY,
	PROP_PORT,
	PROP_PROTOCOL,
	PROP_PHYSICAL,
	PROP_SOCKADDR,

	LAST_PROP
};

typedef struct {
	struct sockaddr_storage *sockaddr;
	int n_addrs, offset;

	char *name, *physical;
	guint port;
	const char *protocol;

	GMutex lock;
	GSList *async_lookups;
} SoupAddressPrivate;
#define SOUP_ADDRESS_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), SOUP_TYPE_ADDRESS, SoupAddressPrivate))

/* sockaddr generic macros */
#define SOUP_SIN(priv) ((struct sockaddr_in *)priv->sockaddr)
#define SOUP_SIN6(priv) ((struct sockaddr_in6 *)priv->sockaddr)

/* sockaddr family macros */
#define SOUP_ADDRESS_GET_FAMILY(priv) (priv->sockaddr->ss_family)
#define SOUP_ADDRESS_SET_FAMILY(priv, family) \
	(priv->sockaddr->ss_family = family)
#define SOUP_ADDRESS_FAMILY_IS_VALID(family) \
	(family == AF_INET || family == AF_INET6)
#define SOUP_ADDRESS_FAMILY_SOCKADDR_SIZE(family) \
	(family == AF_INET ? sizeof (struct sockaddr_in) : \
			     sizeof (struct sockaddr_in6))
#define SOUP_ADDRESS_FAMILY_DATA_SIZE(family) \
	(family == AF_INET ? sizeof (struct in_addr) : \
			     sizeof (struct in6_addr))

/* sockaddr port macros */
#define SOUP_ADDRESS_PORT_IS_VALID(port) (port >= 0 && port <= 65535)
#define SOUP_ADDRESS_GET_PORT(priv) \
	(priv->sockaddr->ss_family == AF_INET ? \
		SOUP_SIN(priv)->sin_port : \
		SOUP_SIN6(priv)->sin6_port)
#define SOUP_ADDRESS_SET_PORT(priv, port) \
	G_STMT_START {					\
	if (priv->sockaddr->ss_family == AF_INET)	\
		SOUP_SIN(priv)->sin_port = port;	\
	else						\
		SOUP_SIN6(priv)->sin6_port = port;	\
	} G_STMT_END

/* sockaddr data macros */
#define SOUP_ADDRESS_GET_DATA(priv) \
	(priv->sockaddr->ss_family == AF_INET ? \
		(gpointer)&SOUP_SIN(priv)->sin_addr : \
		(gpointer)&SOUP_SIN6(priv)->sin6_addr)
#define SOUP_ADDRESS_SET_DATA(priv, data, length) \
	memcpy (SOUP_ADDRESS_GET_DATA (priv), data, length)


static void soup_address_connectable_iface_init (GSocketConnectableIface *connectable_iface);

G_DEFINE_TYPE_WITH_CODE (SoupAddress, soup_address, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (G_TYPE_SOCKET_CONNECTABLE,
						soup_address_connectable_iface_init))

static void
soup_address_init (SoupAddress *addr)
{
	SoupAddressPrivate *priv = SOUP_ADDRESS_GET_PRIVATE (addr);

	g_mutex_init (&priv->lock);
}

static void
soup_address_finalize (GObject *object)
{
	SoupAddress *addr = SOUP_ADDRESS (object);
	SoupAddressPrivate *priv = SOUP_ADDRESS_GET_PRIVATE (addr);

	g_free (priv->sockaddr);
	g_free (priv->name);
	g_free (priv->physical);

	g_mutex_clear (&priv->lock);

	G_OBJECT_CLASS (soup_address_parent_class)->finalize (object);
}

static GObject *
soup_address_constructor (GType                  type,
			  guint                  n_construct_properties,
			  GObjectConstructParam *construct_properties)
{
	GObject *addr;
	SoupAddressPrivate *priv;

	addr = G_OBJECT_CLASS (soup_address_parent_class)->constructor (
		type, n_construct_properties, construct_properties);
	if (!addr)
		return NULL;
	priv = SOUP_ADDRESS_GET_PRIVATE (addr);

	if (!priv->name && !priv->sockaddr) {
		g_object_unref (addr);
		return NULL;
	}

	return addr;
}

static void
soup_address_set_property (GObject *object, guint prop_id,
			   const GValue *value, GParamSpec *pspec)
{
	SoupAddressPrivate *priv = SOUP_ADDRESS_GET_PRIVATE (object);
	SoupAddressFamily family;
	struct sockaddr *sa;
	int len, port;

	/* This is a mess because the properties are mostly orthogonal,
	 * but g_object_constructor wants to set a default value for each
	 * of them.
	 */

	switch (prop_id) {
	case PROP_NAME:
		priv->name = g_value_dup_string (value);
		break;

	case PROP_FAMILY:
		family = g_value_get_enum (value);
		if (family == SOUP_ADDRESS_FAMILY_INVALID)
			return;
		g_return_if_fail (SOUP_ADDRESS_FAMILY_IS_VALID (family));
		g_return_if_fail (priv->sockaddr == NULL);

		priv->sockaddr = g_malloc0 (SOUP_ADDRESS_FAMILY_SOCKADDR_SIZE (family));
		SOUP_ADDRESS_SET_FAMILY (priv, family);
		SOUP_ADDRESS_SET_PORT (priv, htons (priv->port));
		priv->n_addrs = 1;
		break;

	case PROP_PORT:
		port = g_value_get_int (value);
		if (port == -1)
			return;
		g_return_if_fail (SOUP_ADDRESS_PORT_IS_VALID (port));

		priv->port = port;
		if (priv->sockaddr)
			SOUP_ADDRESS_SET_PORT (priv, htons (port));
		break;

	case PROP_PROTOCOL:
		priv->protocol = g_intern_string (g_value_get_string (value));
		break;

	case PROP_SOCKADDR:
		sa = g_value_get_pointer (value);
		if (!sa)
			return;
		g_return_if_fail (priv->sockaddr == NULL);

		len = SOUP_ADDRESS_FAMILY_SOCKADDR_SIZE (sa->sa_family);
		priv->sockaddr = g_memdup (sa, len);
		priv->n_addrs = 1;
		priv->port = ntohs (SOUP_ADDRESS_GET_PORT (priv));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
soup_address_get_property (GObject *object, guint prop_id,
			   GValue *value, GParamSpec *pspec)
{
	SoupAddressPrivate *priv = SOUP_ADDRESS_GET_PRIVATE (object);

	switch (prop_id) {
	case PROP_NAME:
		g_value_set_string (value, priv->name);
		break;
	case PROP_FAMILY:
		if (priv->sockaddr)
			g_value_set_enum (value, SOUP_ADDRESS_GET_FAMILY (priv));
		else
			g_value_set_enum (value, 0);
		break;
	case PROP_PORT:
		g_value_set_int (value, priv->port);
		break;
	case PROP_PHYSICAL:
		g_value_set_string (value, soup_address_get_physical (SOUP_ADDRESS (object)));
		break;
	case PROP_PROTOCOL:
		g_value_set_string (value, priv->protocol);
		break;
	case PROP_SOCKADDR:
		g_value_set_pointer (value, priv->sockaddr);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
soup_address_class_init (SoupAddressClass *address_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (address_class);

	g_type_class_add_private (address_class, sizeof (SoupAddressPrivate));

	/* virtual method override */
	object_class->constructor  = soup_address_constructor;
	object_class->finalize     = soup_address_finalize;
	object_class->set_property = soup_address_set_property;
	object_class->get_property = soup_address_get_property;

	/* properties */
	/**
	 * SOUP_ADDRESS_NAME:
	 *
	 * Alias for the #SoupAddress:name property. (The hostname for
	 * this address.)
	 **/
	g_object_class_install_property (
		object_class, PROP_NAME,
		g_param_spec_string (SOUP_ADDRESS_NAME,
				     "Name",
				     "Hostname for this address",
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * SOUP_ADDRESS_FAMILY:
	 *
	 * Alias for the #SoupAddress:family property. (The
	 * #SoupAddressFamily for this address.)
	 **/
	g_object_class_install_property (
		object_class, PROP_FAMILY,
		g_param_spec_enum (SOUP_ADDRESS_FAMILY,
				   "Family",
				   "Address family for this address",
				   SOUP_TYPE_ADDRESS_FAMILY,
				   SOUP_ADDRESS_FAMILY_INVALID,
				   G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * SOUP_ADDRESS_PORT:
	 *
	 * An alias for the #SoupAddress:port property. (The port for
	 * this address.)
	 **/
	g_object_class_install_property (
		object_class, PROP_PORT,
		g_param_spec_int (SOUP_ADDRESS_PORT,
				  "Port",
				  "Port for this address",
				  -1, 65535, -1,
				  G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * SOUP_ADDRESS_PROTOCOL:
	 *
	 * Alias for the #SoupAddress:protocol property. (The URI scheme
	 * used with this address.)
	 **/
	g_object_class_install_property (
		object_class, PROP_PROTOCOL,
		g_param_spec_string (SOUP_ADDRESS_PROTOCOL,
				     "Protocol",
				     "URI scheme for this address",
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	/**
	 * SOUP_ADDRESS_PHYSICAL:
	 *
	 * An alias for the #SoupAddress:physical property. (The
	 * stringified IP address for this address.)
	 **/
	g_object_class_install_property (
		object_class, PROP_PHYSICAL,
		g_param_spec_string (SOUP_ADDRESS_PHYSICAL,
				     "Physical address",
				     "IP address for this address",
				     NULL,
				     G_PARAM_READABLE));
	/**
	 * SOUP_ADDRESS_SOCKADDR:
	 *
	 * An alias for the #SoupAddress:sockaddr property. (A pointer
	 * to the struct sockaddr for this address.)
	 **/
	g_object_class_install_property (
		object_class, PROP_SOCKADDR,
		g_param_spec_pointer (SOUP_ADDRESS_SOCKADDR,
				      "sockaddr",
				      "struct sockaddr for this address",
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}

/**
 * soup_address_new:
 * @name: a hostname or physical address
 * @port: a port number
 *
 * Creates a #SoupAddress from @name and @port. The #SoupAddress's IP
 * address may not be available right away; the caller can call
 * soup_address_resolve_async() or soup_address_resolve_sync() to
 * force a DNS resolution.
 *
 * Return value: a #SoupAddress
 **/
SoupAddress *
soup_address_new (const char *name, guint port)
{
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (SOUP_ADDRESS_PORT_IS_VALID (port), NULL);

	return g_object_new (SOUP_TYPE_ADDRESS,
			     SOUP_ADDRESS_NAME, name,
			     SOUP_ADDRESS_PORT, port,
			     NULL);
}

/**
 * soup_address_new_from_sockaddr:
 * @sa: a pointer to a sockaddr
 * @len: size of @sa
 *
 * Returns a #SoupAddress equivalent to @sa (or %NULL if @sa's
 * address family isn't supported)
 *
 * Return value: (allow-none): the new #SoupAddress
 **/
SoupAddress *
soup_address_new_from_sockaddr (struct sockaddr *sa, int len)
{
	g_return_val_if_fail (sa != NULL, NULL);
	g_return_val_if_fail (SOUP_ADDRESS_FAMILY_IS_VALID (sa->sa_family), NULL);
	g_return_val_if_fail (len == SOUP_ADDRESS_FAMILY_SOCKADDR_SIZE (sa->sa_family), NULL);

	return g_object_new (SOUP_TYPE_ADDRESS,
			     SOUP_ADDRESS_SOCKADDR, sa,
			     NULL);
}

/**
 * SoupAddressFamily:
 * @SOUP_ADDRESS_FAMILY_INVALID: an invalid %SoupAddress
 * @SOUP_ADDRESS_FAMILY_IPV4: an IPv4 address
 * @SOUP_ADDRESS_FAMILY_IPV6: an IPv6 address
 *
 * The supported address families.
 **/

/**
 * SOUP_ADDRESS_ANY_PORT:
 *
 * This can be passed to any #SoupAddress method that expects a port,
 * to indicate that you don't care what port is used.
 **/

/**
 * soup_address_new_any:
 * @family: the address family
 * @port: the port number (usually %SOUP_ADDRESS_ANY_PORT)
 *
 * Returns a #SoupAddress corresponding to the "any" address
 * for @family (or %NULL if @family isn't supported), suitable for
 * using as a listening #SoupSocket.
 *
 * Return value: (allow-none): the new #SoupAddress
 **/
SoupAddress *
soup_address_new_any (SoupAddressFamily family, guint port)
{
	g_return_val_if_fail (SOUP_ADDRESS_FAMILY_IS_VALID (family), NULL);
	g_return_val_if_fail (SOUP_ADDRESS_PORT_IS_VALID (port), NULL);

	return g_object_new (SOUP_TYPE_ADDRESS,
			     SOUP_ADDRESS_FAMILY, family,
			     SOUP_ADDRESS_PORT, port,
			     NULL);
}

/**
 * soup_address_get_name:
 * @addr: a #SoupAddress
 *
 * Returns the hostname associated with @addr.
 *
 * This method is not thread-safe; if you call it while @addr is being
 * resolved in another thread, it may return garbage. You can use
 * soup_address_is_resolved() to safely test whether or not an address
 * is resolved before fetching its name or address.
 *
 * Return value: (allow-none): the hostname, or %NULL if it is not known.
 **/
const char *
soup_address_get_name (SoupAddress *addr)
{
	g_return_val_if_fail (SOUP_IS_ADDRESS (addr), NULL);

	return SOUP_ADDRESS_GET_PRIVATE (addr)->name;
}

/**
 * soup_address_get_sockaddr:
 * @addr: a #SoupAddress
 * @len: return location for sockaddr length
 *
 * Returns the sockaddr associated with @addr, with its length in
 * *@len. If the sockaddr is not yet known, returns %NULL.
 *
 * This method is not thread-safe; if you call it while @addr is being
 * resolved in another thread, it may return garbage. You can use
 * soup_address_is_resolved() to safely test whether or not an address
 * is resolved before fetching its name or address.
 *
 * Return value: (allow-none) (transfer none): the sockaddr, or %NULL
 **/
struct sockaddr *
soup_address_get_sockaddr (SoupAddress *addr, int *len)
{
	SoupAddressPrivate *priv;

	g_return_val_if_fail (SOUP_IS_ADDRESS (addr), NULL);
	priv = SOUP_ADDRESS_GET_PRIVATE (addr);

	if (priv->sockaddr && len)
		*len = SOUP_ADDRESS_FAMILY_SOCKADDR_SIZE (SOUP_ADDRESS_GET_FAMILY (priv));
	return (struct sockaddr *)priv->sockaddr;
}

/**
 * soup_address_get_gsockaddr:
 * @addr: a #SoupAddress
 *
 * Creates a new #GSocketAddress corresponding to @addr (which is assumed
 * to only have one socket address associated with it).
 *
 * Return value: (transfer full): a new #GSocketAddress
 *
 * Since: 2.32
 */
GSocketAddress *
soup_address_get_gsockaddr (SoupAddress *addr)
{
	SoupAddressPrivate *priv = SOUP_ADDRESS_GET_PRIVATE (addr);

	return g_socket_address_new_from_native (priv->sockaddr,
						 SOUP_ADDRESS_FAMILY_SOCKADDR_SIZE (SOUP_ADDRESS_GET_FAMILY (priv)));
}

static GInetAddress *
soup_address_make_inet_address (SoupAddress *addr)
{
	SoupAddressPrivate *priv = SOUP_ADDRESS_GET_PRIVATE (addr);
	GSocketAddress *gsa;
	GInetAddress *gia;

	gsa = g_socket_address_new_from_native (priv->sockaddr,
						SOUP_ADDRESS_FAMILY_SOCKADDR_SIZE (SOUP_ADDRESS_GET_FAMILY (priv)));
	gia = g_inet_socket_address_get_address ((GInetSocketAddress *)gsa);
	g_object_ref (gia);
	g_object_unref (gsa);
	return gia;
}

/**
 * soup_address_get_physical:
 * @addr: a #SoupAddress
 *
 * Returns the physical address associated with @addr as a string.
 * (Eg, "127.0.0.1"). If the address is not yet known, returns %NULL.
 *
 * This method is not thread-safe; if you call it while @addr is being
 * resolved in another thread, it may return garbage. You can use
 * soup_address_is_resolved() to safely test whether or not an address
 * is resolved before fetching its name or address.
 *
 * Return value: (allow-none): the physical address, or %NULL
 **/
const char *
soup_address_get_physical (SoupAddress *addr)
{
	SoupAddressPrivate *priv;

	g_return_val_if_fail (SOUP_IS_ADDRESS (addr), NULL);
	priv = SOUP_ADDRESS_GET_PRIVATE (addr);

	if (!priv->sockaddr)
		return NULL;

	if (!priv->physical) {
		GInetAddress *gia;

		gia = soup_address_make_inet_address (addr);
		priv->physical = g_inet_address_to_string (gia);
		g_object_unref (gia);
	}

	return priv->physical;
}

/**
 * soup_address_get_port:
 * @addr: a #SoupAddress
 *
 * Returns the port associated with @addr.
 *
 * Return value: the port
 **/
guint
soup_address_get_port (SoupAddress *addr)
{
	g_return_val_if_fail (SOUP_IS_ADDRESS (addr), 0);

	return SOUP_ADDRESS_GET_PRIVATE (addr)->port;
}


static guint
update_addrs (SoupAddress *addr, GList *addrs, GError *error)
{
	SoupAddressPrivate *priv = SOUP_ADDRESS_GET_PRIVATE (addr);
	GInetAddress *gia;
	GSocketAddress *gsa;
	int i;

	if (error) {
		if (error->domain == G_IO_ERROR &&
		    error->code == G_IO_ERROR_CANCELLED)
			return SOUP_STATUS_CANCELLED;
		else
			return SOUP_STATUS_CANT_RESOLVE;
	} else if (!addrs)
		return SOUP_STATUS_CANT_RESOLVE;
	else if (priv->sockaddr)
		return SOUP_STATUS_OK;

	priv->n_addrs = g_list_length (addrs);
	priv->sockaddr = g_new (struct sockaddr_storage, priv->n_addrs);
	for (i = 0; addrs; addrs = addrs->next, i++) {
		gia = addrs->data;
		gsa = g_inet_socket_address_new (gia, priv->port);

		if (!g_socket_address_to_native (gsa, &priv->sockaddr[i],
						 sizeof (struct sockaddr_storage),
						 NULL)) {
			/* can't happen: We know the address format is supported
			 * and the buffer is large enough
			 */
			g_warn_if_reached ();
		}
		g_object_unref (gsa);
	}

	return SOUP_STATUS_OK;
}

static guint
update_name (SoupAddress *addr, const char *name, GError *error)
{
	SoupAddressPrivate *priv = SOUP_ADDRESS_GET_PRIVATE (addr);

	if (error) {
		if (error->domain == G_IO_ERROR &&
		    error->code == G_IO_ERROR_CANCELLED)
			return SOUP_STATUS_CANCELLED;
		else
			return SOUP_STATUS_CANT_RESOLVE;
	} else if (!name)
		return SOUP_STATUS_CANT_RESOLVE;
	else if (priv->name)
		return SOUP_STATUS_OK;

	priv->name = g_strdup (name);
	return SOUP_STATUS_OK;
}

typedef struct {
	SoupAddressCallback  callback;
	gpointer             callback_data;
} SoupAddressResolveAsyncData;

static void
complete_resolve_async (SoupAddress *addr, guint status)
{
	SoupAddressPrivate *priv = SOUP_ADDRESS_GET_PRIVATE (addr);
	SoupAddressResolveAsyncData *res_data;
	GSList *lookups, *l;
	GSource *current_source;
	GMainContext *current_context;

	lookups = priv->async_lookups;
	priv->async_lookups = NULL;

	/* Awful hack; to make soup_socket_connect_async() with an
	 * non-default async_context work correctly, we need to ensure
	 * that the non-default context (which we're now running in)
	 * is the thread-default when the callbacks are run...
	 */
	current_source = g_main_current_source ();
	if (current_source && !g_source_is_destroyed (current_source))
		current_context = g_source_get_context (current_source);
	else
		current_context = NULL;
	g_main_context_push_thread_default (current_context);

	for (l = lookups; l; l = l->next) {
		res_data = l->data;

		if (res_data->callback) {
			res_data->callback (addr, status,
					    res_data->callback_data);
		}
		g_slice_free (SoupAddressResolveAsyncData, res_data);
	}
	g_slist_free (lookups);

	g_main_context_pop_thread_default (current_context);

	g_object_unref (addr);
}

static void
lookup_resolved (GObject *source, GAsyncResult *result, gpointer user_data)
{
	GResolver *resolver = G_RESOLVER (source);
	SoupAddress *addr = user_data;
	SoupAddressPrivate *priv = SOUP_ADDRESS_GET_PRIVATE (addr);
	GError *error = NULL;
	guint status;

	if (!priv->sockaddr) {
		GList *addrs;

		addrs = g_resolver_lookup_by_name_finish (resolver, result,
							  &error);
		status = update_addrs (addr, addrs, error);
		g_resolver_free_addresses (addrs);
	} else if (!priv->name) {
		char *name;

		name = g_resolver_lookup_by_address_finish (resolver, result,
							    &error);
		status = update_name (addr, name, error);
		g_free (name);
	} else
		status = SOUP_STATUS_OK;

	/* For the enumerator impl, below */
	g_object_ref (addr);
	g_object_set_data (G_OBJECT (addr), "async-resolved-error", error);

	complete_resolve_async (addr, status);

	g_object_set_data (G_OBJECT (addr), "async-resolved-error", NULL);
	g_object_unref (addr);
	if (error)
		g_error_free (error);
}

static gboolean
idle_complete_resolve (gpointer addr)
{
	complete_resolve_async (addr, SOUP_STATUS_OK);
	return FALSE;
}

/**
 * SoupAddressCallback:
 * @addr: the #SoupAddress that was resolved
 * @status: %SOUP_STATUS_OK, %SOUP_STATUS_CANT_RESOLVE, or
 * %SOUP_STATUS_CANCELLED
 * @user_data: the user data that was passed to
 * soup_address_resolve_async()
 *
 * The callback function passed to soup_address_resolve_async().
 **/

/**
 * soup_address_resolve_async:
 * @addr: a #SoupAddress
 * @async_context: (allow-none): the #GMainContext to call @callback from
 * @cancellable: a #GCancellable object, or %NULL
 * @callback: (scope async): callback to call with the result
 * @user_data: data for @callback
 *
 * Asynchronously resolves the missing half of @addr (its IP address
 * if it was created with soup_address_new(), or its hostname if it
 * was created with soup_address_new_from_sockaddr() or
 * soup_address_new_any().)
 *
 * If @cancellable is non-%NULL, it can be used to cancel the
 * resolution. @callback will still be invoked in this case, with a
 * status of %SOUP_STATUS_CANCELLED.
 *
 * It is safe to call this more than once on a given address, from the
 * same thread, with the same @async_context (and doing so will not
 * result in redundant DNS queries being made). But it is not safe to
 * call from multiple threads, or with different @async_contexts, or
 * mixed with calls to soup_address_resolve_sync().
 **/
void
soup_address_resolve_async (SoupAddress *addr, GMainContext *async_context,
			    GCancellable *cancellable,
			    SoupAddressCallback callback, gpointer user_data)
{
	SoupAddressPrivate *priv;
	SoupAddressResolveAsyncData *res_data;
	GResolver *resolver;
	gboolean already_started;

	g_return_if_fail (SOUP_IS_ADDRESS (addr));
	priv = SOUP_ADDRESS_GET_PRIVATE (addr);
	g_return_if_fail (priv->name || priv->sockaddr);

	/* We don't need to do locking here because the async case is
	 * not intended to be thread-safe.
	 */

	if (priv->name && priv->sockaddr && !callback)
		return;

	res_data = g_slice_new0 (SoupAddressResolveAsyncData);
	res_data->callback = callback;
	res_data->callback_data = user_data;

	already_started = priv->async_lookups != NULL;
	priv->async_lookups = g_slist_prepend (priv->async_lookups, res_data);

	if (already_started)
		return;

	g_object_ref (addr);

	if (priv->name && priv->sockaddr) {
		soup_add_completion (async_context, idle_complete_resolve, addr);
		return;
	}

	resolver = g_resolver_get_default ();
	if (async_context)
		g_main_context_push_thread_default (async_context);

	if (priv->name) {
		g_resolver_lookup_by_name_async (resolver, priv->name,
						 cancellable,
						 lookup_resolved, addr);
	} else {
		GInetAddress *gia;

		gia = soup_address_make_inet_address (addr);
		g_resolver_lookup_by_address_async (resolver, gia,
						    cancellable,
						    lookup_resolved, addr);
		g_object_unref (gia);
	}

	if (async_context)
		g_main_context_pop_thread_default (async_context);
	g_object_unref (resolver);
}

static guint
resolve_sync_internal (SoupAddress *addr, GCancellable *cancellable, GError **error)
{
	SoupAddressPrivate *priv = SOUP_ADDRESS_GET_PRIVATE (addr);
	GResolver *resolver;
	guint status;
	GError *my_err = NULL;

	resolver = g_resolver_get_default ();

	/* We could optimize this to avoid multiple lookups the same
	 * way _resolve_async does, but we don't currently. So, first
	 * lock the mutex to ensure we have a consistent view of
	 * priv->sockaddr and priv->name, unlock it around the
	 * blocking op, and then re-lock it to modify @addr.
	 */
	g_mutex_lock (&priv->lock);
	if (!priv->sockaddr) {
		GList *addrs;

		g_mutex_unlock (&priv->lock);
		addrs = g_resolver_lookup_by_name (resolver, priv->name,
						   cancellable, &my_err);
		g_mutex_lock (&priv->lock);

		status = update_addrs (addr, addrs, my_err);
		g_resolver_free_addresses (addrs);
	} else if (!priv->name) {
		GInetAddress *gia;
		char *name;

		g_mutex_unlock (&priv->lock);
		gia = soup_address_make_inet_address (addr);
		name = g_resolver_lookup_by_address (resolver, gia,
						     cancellable, &my_err);
		g_object_unref (gia);
		g_mutex_lock (&priv->lock);

		status = update_name (addr, name, my_err);
		g_free (name);
	} else
		status = SOUP_STATUS_OK;
	g_mutex_unlock (&priv->lock);

	if (my_err)
		g_propagate_error (error, my_err);
	g_object_unref (resolver);

	return status;
}

/**
 * soup_address_resolve_sync:
 * @addr: a #SoupAddress
 * @cancellable: a #GCancellable object, or %NULL
 *
 * Synchronously resolves the missing half of @addr, as with
 * soup_address_resolve_async().
 *
 * If @cancellable is non-%NULL, it can be used to cancel the
 * resolution. soup_address_resolve_sync() will then return a status
 * of %SOUP_STATUS_CANCELLED.
 *
 * It is safe to call this more than once, even from different
 * threads, but it is not safe to mix calls to
 * soup_address_resolve_sync() with calls to
 * soup_address_resolve_async() on the same address.
 *
 * Return value: %SOUP_STATUS_OK, %SOUP_STATUS_CANT_RESOLVE, or
 * %SOUP_STATUS_CANCELLED.
 **/
guint
soup_address_resolve_sync (SoupAddress *addr, GCancellable *cancellable)
{
	SoupAddressPrivate *priv;

	g_return_val_if_fail (SOUP_IS_ADDRESS (addr), SOUP_STATUS_MALFORMED);
	priv = SOUP_ADDRESS_GET_PRIVATE (addr);
	g_return_val_if_fail (priv->name || priv->sockaddr, SOUP_STATUS_MALFORMED);

	return resolve_sync_internal (addr, cancellable, NULL);
}

/**
 * soup_address_is_resolved:
 * @addr: a #SoupAddress
 *
 * Tests if @addr has already been resolved. Unlike the other
 * #SoupAddress "get" methods, this is safe to call when @addr might
 * be being resolved in another thread.
 *
 * Return value: %TRUE if @addr has been resolved.
 **/
gboolean
soup_address_is_resolved (SoupAddress *addr)
{
	SoupAddressPrivate *priv;
	gboolean resolved;

	g_return_val_if_fail (SOUP_IS_ADDRESS (addr), FALSE);
	priv = SOUP_ADDRESS_GET_PRIVATE (addr);

	g_mutex_lock (&priv->lock);
	resolved = priv->sockaddr && priv->name;
	g_mutex_unlock (&priv->lock);

	return resolved;
}

/**
 * soup_address_hash_by_name:
 * @addr: (type Soup.Address): a #SoupAddress
 *
 * A hash function (for #GHashTable) that corresponds to
 * soup_address_equal_by_name(), qv
 *
 * Return value: the named-based hash value for @addr.
 *
 * Since: 2.26
 **/
guint
soup_address_hash_by_name (gconstpointer addr)
{
	SoupAddressPrivate *priv = SOUP_ADDRESS_GET_PRIVATE (addr);

	g_return_val_if_fail (priv->name != NULL, 0);
	return g_str_hash (priv->name);
}

/**
 * soup_address_equal_by_name:
 * @addr1: (type Soup.Address): a #SoupAddress with a resolved name
 * @addr2: (type Soup.Address): another #SoupAddress with a resolved
 *   name
 *
 * Tests if @addr1 and @addr2 have the same "name". This method can be
 * used with soup_address_hash_by_name() to create a #GHashTable that
 * hashes on address "names".
 *
 * Comparing by name normally means comparing the addresses by their
 * hostnames. But if the address was originally created using an IP
 * address literal, then it will be compared by that instead.
 *
 * In particular, if "www.example.com" has the IP address 10.0.0.1,
 * and @addr1 was created with the name "www.example.com" and @addr2
 * was created with the name "10.0.0.1", then they will compare as
 * unequal for purposes of soup_address_equal_by_name().
 *
 * This would be used to distinguish hosts in situations where
 * different virtual hosts on the same IP address should be considered
 * different. Eg, for purposes of HTTP authentication or cookies, two
 * hosts with the same IP address but different names are considered
 * to be different hosts.
 *
 * See also soup_address_equal_by_ip(), which compares by IP address
 * rather than by name.
 *
 * Return value: whether or not @addr1 and @addr2 have the same name
 *
 * Since: 2.26
 **/
gboolean
soup_address_equal_by_name (gconstpointer addr1, gconstpointer addr2)
{
	SoupAddressPrivate *priv1 = SOUP_ADDRESS_GET_PRIVATE (addr1);
	SoupAddressPrivate *priv2 = SOUP_ADDRESS_GET_PRIVATE (addr2);

	g_return_val_if_fail (priv1->name != NULL, FALSE);
	g_return_val_if_fail (priv2->name != NULL, FALSE);
	return !g_ascii_strcasecmp (priv1->name, priv2->name);
}

/**
 * soup_address_hash_by_ip:
 * @addr: (type Soup.Address): a #SoupAddress
 *
 * A hash function (for #GHashTable) that corresponds to
 * soup_address_equal_by_ip(), qv
 *
 * Return value: the IP-based hash value for @addr.
 *
 * Since: 2.26
 **/
guint
soup_address_hash_by_ip (gconstpointer addr)
{
	SoupAddressPrivate *priv = SOUP_ADDRESS_GET_PRIVATE (addr);
	guint hash;

	g_return_val_if_fail (priv->sockaddr != NULL, 0);

	memcpy (&hash, SOUP_ADDRESS_GET_DATA (priv),
		MIN (sizeof (hash), SOUP_ADDRESS_FAMILY_DATA_SIZE (priv->sockaddr->ss_family)));
	return hash;
}

/**
 * soup_address_equal_by_ip:
 * @addr1: (type Soup.Address): a #SoupAddress with a resolved IP
 *   address
 * @addr2: (type Soup.Address): another #SoupAddress with a resolved
 *   IP address
 *
 * Tests if @addr1 and @addr2 have the same IP address. This method
 * can be used with soup_address_hash_by_ip() to create a
 * #GHashTable that hashes on IP address.
 *
 * This would be used to distinguish hosts in situations where
 * different virtual hosts on the same IP address should be considered
 * the same. Eg, if "www.example.com" and "www.example.net" have the
 * same IP address, then a single connection can be used to talk
 * to either of them.
 *
 * See also soup_address_equal_by_name(), which compares by name
 * rather than by IP address.
 *
 * Return value: whether or not @addr1 and @addr2 have the same IP
 * address.
 *
 * Since: 2.26
 **/
gboolean
soup_address_equal_by_ip (gconstpointer addr1, gconstpointer addr2)
{
	SoupAddressPrivate *priv1 = SOUP_ADDRESS_GET_PRIVATE (addr1);
	SoupAddressPrivate *priv2 = SOUP_ADDRESS_GET_PRIVATE (addr2);
	int size;

	g_return_val_if_fail (priv1->sockaddr != NULL, FALSE);
	g_return_val_if_fail (priv2->sockaddr != NULL, FALSE);

	size = SOUP_ADDRESS_FAMILY_SOCKADDR_SIZE (priv1->sockaddr->ss_family);
	return (priv1->sockaddr->ss_family ==
		priv2->sockaddr->ss_family &&
		!memcmp (priv1->sockaddr, priv2->sockaddr, size));
}


#define SOUP_TYPE_ADDRESS_ADDRESS_ENUMERATOR (_soup_address_address_enumerator_get_type ())
#define SOUP_ADDRESS_ADDRESS_ENUMERATOR(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOUP_TYPE_ADDRESS_ADDRESS_ENUMERATOR, SoupAddressAddressEnumerator))

typedef struct {
	GSocketAddressEnumerator parent_instance;

	SoupAddress *addr;
	int orig_offset;
	int n;
} SoupAddressAddressEnumerator;

typedef struct {
	GSocketAddressEnumeratorClass parent_class;

} SoupAddressAddressEnumeratorClass;

GType _soup_address_address_enumerator_get_type (void);
G_DEFINE_TYPE (SoupAddressAddressEnumerator, _soup_address_address_enumerator, G_TYPE_SOCKET_ADDRESS_ENUMERATOR)

static void
soup_address_address_enumerator_finalize (GObject *object)
{
	SoupAddressAddressEnumerator *addr_enum =
		SOUP_ADDRESS_ADDRESS_ENUMERATOR (object);

	g_object_unref (addr_enum->addr);

	G_OBJECT_CLASS (_soup_address_address_enumerator_parent_class)->finalize (object);
}

static GSocketAddress *
next_address (SoupAddressAddressEnumerator *addr_enum)
{
	SoupAddressPrivate *priv = SOUP_ADDRESS_GET_PRIVATE (addr_enum->addr);
	struct sockaddr_storage *ss;
	int next_addr;

	/* If there are two addresses but the first one is unusable
	 * (eg, it's IPv6 and we can only do IPv4), then we don't want to
	 * try the bad one every time. So we use priv->offset to remember
	 * the offset of the first usable address (ie, the first address
	 * that we weren't called again after returning).
	 */
	next_addr = (addr_enum->orig_offset + addr_enum->n) % priv->n_addrs;
	priv->offset = next_addr;

	if (addr_enum->n >= priv->n_addrs)
		return NULL;
	addr_enum->n++;

	ss = &priv->sockaddr[next_addr];
	return g_socket_address_new_from_native (ss, SOUP_ADDRESS_FAMILY_SOCKADDR_SIZE (ss->ss_family));
}

static GSocketAddress *
soup_address_address_enumerator_next (GSocketAddressEnumerator  *enumerator,
				      GCancellable              *cancellable,
				      GError                   **error)
{
	SoupAddressAddressEnumerator *addr_enum =
		SOUP_ADDRESS_ADDRESS_ENUMERATOR (enumerator);
	SoupAddressPrivate *priv = SOUP_ADDRESS_GET_PRIVATE (addr_enum->addr);

	if (!priv->sockaddr) {
		if (resolve_sync_internal (addr_enum->addr, cancellable, error) != SOUP_STATUS_OK)
			return NULL;
	}

	return next_address (addr_enum);
}

static void
got_addresses (SoupAddress *addr, guint status, gpointer user_data)
{
	GTask *task = user_data;
	GError *error;

	error = g_object_get_data (G_OBJECT (addr), "async-resolved-error");
	if (error)
		g_task_return_error (task, g_error_copy (error));
	else {
		GSocketAddress *addr;

		addr = next_address (g_task_get_source_object (task));
		g_task_return_pointer (task, addr, g_object_unref);
	}
	g_object_unref (task);
}

static void
soup_address_address_enumerator_next_async (GSocketAddressEnumerator  *enumerator,
					    GCancellable              *cancellable,
					    GAsyncReadyCallback        callback,
					    gpointer                   user_data)
{
	SoupAddressAddressEnumerator *addr_enum =
		SOUP_ADDRESS_ADDRESS_ENUMERATOR (enumerator);
	SoupAddressPrivate *priv = SOUP_ADDRESS_GET_PRIVATE (addr_enum->addr);
	GTask *task;

	task = g_task_new (enumerator, cancellable, callback, user_data);
	if (!priv->sockaddr) {
		soup_address_resolve_async (addr_enum->addr, NULL, cancellable,
					    got_addresses, task);
	} else {
		g_task_return_pointer (task, next_address (addr_enum), g_object_unref);
		g_object_unref (task);
	}
}

static GSocketAddress *
soup_address_address_enumerator_next_finish (GSocketAddressEnumerator  *enumerator,
					     GAsyncResult              *result,
					     GError                   **error)
{
	return g_task_propagate_pointer (G_TASK (result), error);
}

static void
_soup_address_address_enumerator_init (SoupAddressAddressEnumerator *enumerator)
{
}

static void
_soup_address_address_enumerator_class_init (SoupAddressAddressEnumeratorClass *addrenum_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (addrenum_class);
	GSocketAddressEnumeratorClass *enumerator_class =
		G_SOCKET_ADDRESS_ENUMERATOR_CLASS (addrenum_class);

	enumerator_class->next = soup_address_address_enumerator_next;
	enumerator_class->next_async = soup_address_address_enumerator_next_async;
	enumerator_class->next_finish = soup_address_address_enumerator_next_finish;
	object_class->finalize = soup_address_address_enumerator_finalize;
}

static GSocketAddressEnumerator *
soup_address_connectable_enumerate (GSocketConnectable *connectable)
{
	SoupAddressAddressEnumerator *addr_enum;
	SoupAddressPrivate *priv;

	addr_enum = g_object_new (SOUP_TYPE_ADDRESS_ADDRESS_ENUMERATOR, NULL);
	addr_enum->addr = g_object_ref (connectable);

	priv = SOUP_ADDRESS_GET_PRIVATE (addr_enum->addr);
	addr_enum->orig_offset = priv->offset;

	return (GSocketAddressEnumerator *)addr_enum;
}

static GSocketAddressEnumerator *
soup_address_connectable_proxy_enumerate (GSocketConnectable *connectable)
{
	SoupAddress *addr = SOUP_ADDRESS (connectable);
	SoupAddressPrivate *priv = SOUP_ADDRESS_GET_PRIVATE (addr);
	GSocketAddressEnumerator *proxy_enum;
	SoupURI *uri;
	char *uri_string;

	/* We cheerily assume "http" here because you shouldn't be
	 * using SoupAddress any more if you're not doing HTTP anyway.
	 */
	uri = soup_uri_new (NULL);
	soup_uri_set_scheme (uri, priv->protocol ? priv->protocol : "http");
	soup_uri_set_host (uri, priv->name ? priv->name : soup_address_get_physical (addr));
	soup_uri_set_port (uri, priv->port);
	soup_uri_set_path (uri, "");
	uri_string = soup_uri_to_string (uri, FALSE);

	proxy_enum = g_object_new (G_TYPE_PROXY_ADDRESS_ENUMERATOR,
				   "connectable", connectable,
				   "uri", uri_string,
				   NULL);
	g_free (uri_string);
	soup_uri_free (uri);

	return proxy_enum;
}

static void
soup_address_connectable_iface_init (GSocketConnectableIface *connectable_iface)
{
  connectable_iface->enumerate       = soup_address_connectable_enumerate;
  connectable_iface->proxy_enumerate = soup_address_connectable_proxy_enumerate;
}
