/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright (C) 2010 Red Hat, Inc.
 * Copyright (C) 2009 Codethink Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2 of the licence or (at
 * your option) any later version.
 *
 * See the included COPYING file for more information.
 *
 * Authors: David Zeuthen <davidz@redhat.com>
 */

/**
 * SECTION:gunixcredentialsmessage
 * @title: GUnixCredentialsMessage
 * @short_description: A GSocketControlMessage containing credentials
 * @include: gio/gunixcredentialsmessage.h
 * @see_also: #GUnixConnection, #GSocketControlMessage
 *
 * This #GSocketControlMessage contains a #GCredentials instance.  It
 * may be sent using g_socket_send_message() and received using
 * g_socket_receive_message() over UNIX sockets (ie: sockets in the
 * %G_SOCKET_FAMILY_UNIX family).
 *
 * For an easier way to send and receive credentials over
 * stream-oriented UNIX sockets, see
 * g_unix_connection_send_credentials() and
 * g_unix_connection_receive_credentials(). To receive credentials of
 * a foreign process connected to a socket, use
 * g_socket_get_credentials().
 */

#include "config.h"

/* ---------------------------------------------------------------------------------------------------- */
#ifdef __linux__
#define G_UNIX_CREDENTIALS_MESSAGE_SUPPORTED 1
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#define G_UNIX_CREDENTIALS_MESSAGE_SUPPORTED 1
#else
/* TODO: please add support for your UNIX flavor */
#define G_UNIX_CREDENTIALS_MESSAGE_SUPPORTED 0
#endif

/* ---------------------------------------------------------------------------------------------------- */

#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "gunixcredentialsmessage.h"
#include "gcredentials.h"
#include "gnetworking.h"

#include "glibintl.h"

struct _GUnixCredentialsMessagePrivate
{
  GCredentials *credentials;
};

enum
{
  PROP_0,
  PROP_CREDENTIALS
};

G_DEFINE_TYPE_WITH_PRIVATE (GUnixCredentialsMessage, g_unix_credentials_message, G_TYPE_SOCKET_CONTROL_MESSAGE)
static gsize
g_unix_credentials_message_get_size (GSocketControlMessage *message)
{
#ifdef __linux__
  return sizeof (struct ucred);
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
  return sizeof (struct cmsgcred);
#else
  return 0;
#endif
}

static int
g_unix_credentials_message_get_level (GSocketControlMessage *message)
{
#ifdef __linux__
  return SOL_SOCKET;
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
  return SOL_SOCKET;
#else
  return 0;
#endif
}

static int
g_unix_credentials_message_get_msg_type (GSocketControlMessage *message)
{
#ifdef __linux__
  return SCM_CREDENTIALS;
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
  return SCM_CREDS;
#else
  return 0;
#endif
}

static GSocketControlMessage *
g_unix_credentials_message_deserialize (gint     level,
                                        gint     type,
                                        gsize    size,
                                        gpointer data)
{
  GSocketControlMessage *message;

  message = NULL;

#ifdef __linux__
  {
    GCredentials *credentials;
    struct ucred *ucred;

    if (level != SOL_SOCKET || type != SCM_CREDENTIALS)
      goto out;

    if (size != sizeof (struct ucred))
      {
        g_warning ("Expected a struct ucred (%" G_GSIZE_FORMAT " bytes) but "
                   "got %" G_GSIZE_FORMAT " bytes of data",
                   sizeof (struct ucred),
                   size);
        goto out;
      }

    ucred = data;

    if (ucred->uid == (uid_t)-1 &&
	ucred->gid == (gid_t)-1)
      {
	/* This happens if the remote side didn't pass the credentials */
	goto out;
      }

    credentials = g_credentials_new ();
    g_credentials_set_native (credentials, G_CREDENTIALS_TYPE_LINUX_UCRED, ucred);
    message = g_unix_credentials_message_new_with_credentials (credentials);
    g_object_unref (credentials);
 out:
    ;
  }
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
  {
    GCredentials *credentials;
    struct cmsgcred *cred;

    if (level != SOL_SOCKET || type != SCM_CREDS)
      {
        goto out;
      }
    if (size < sizeof *cred)
      {
        g_warning ("Expected a struct cmsgcred (%" G_GSIZE_FORMAT " bytes) but "
                   "got %" G_GSIZE_FORMAT " bytes of data",
                   CMSG_LEN (sizeof *cred),
                   size);
        goto out;
      }

    cred = data;

    credentials = g_credentials_new ();
    g_credentials_set_native (credentials, G_CREDENTIALS_TYPE_FREEBSD_CMSGCRED, cred);
    message = g_unix_credentials_message_new_with_credentials (credentials);
    g_object_unref (credentials);
 out:
    ;
  }
#endif

  return message;
}

static void
g_unix_credentials_message_serialize (GSocketControlMessage *_message,
                                      gpointer               data)
{
  GUnixCredentialsMessage *message = G_UNIX_CREDENTIALS_MESSAGE (_message);
#ifdef __linux__
  memcpy (data,
          g_credentials_get_native (message->priv->credentials,
                                    G_CREDENTIALS_TYPE_LINUX_UCRED),
          sizeof (struct ucred));
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
  memcpy (data,
          g_credentials_get_native (message->priv->credentials,
                                    G_CREDENTIALS_TYPE_FREEBSD_CMSGCRED),
          sizeof (struct cmsgcred));

#endif
}

static void
g_unix_credentials_message_finalize (GObject *object)
{
  GUnixCredentialsMessage *message = G_UNIX_CREDENTIALS_MESSAGE (object);

  if (message->priv->credentials != NULL)
    g_object_unref (message->priv->credentials);

  G_OBJECT_CLASS (g_unix_credentials_message_parent_class)->finalize (object);
}

static void
g_unix_credentials_message_init (GUnixCredentialsMessage *message)
{
  message->priv = g_unix_credentials_message_get_instance_private (message);
}

static void
g_unix_credentials_message_get_property (GObject    *object,
                                         guint       prop_id,
                                         GValue     *value,
                                         GParamSpec *pspec)
{
  GUnixCredentialsMessage *message = G_UNIX_CREDENTIALS_MESSAGE (object);

  switch (prop_id)
    {
    case PROP_CREDENTIALS:
      g_value_set_object (value, message->priv->credentials);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
g_unix_credentials_message_set_property (GObject      *object,
                                         guint         prop_id,
                                         const GValue *value,
                                         GParamSpec   *pspec)
{
  GUnixCredentialsMessage *message = G_UNIX_CREDENTIALS_MESSAGE (object);

  switch (prop_id)
    {
    case PROP_CREDENTIALS:
      message->priv->credentials = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
g_unix_credentials_message_constructed (GObject *object)
{
  GUnixCredentialsMessage *message = G_UNIX_CREDENTIALS_MESSAGE (object);

  if (message->priv->credentials == NULL)
    message->priv->credentials = g_credentials_new ();

  if (G_OBJECT_CLASS (g_unix_credentials_message_parent_class)->constructed != NULL)
    G_OBJECT_CLASS (g_unix_credentials_message_parent_class)->constructed (object);
}

static void
g_unix_credentials_message_class_init (GUnixCredentialsMessageClass *class)
{
  GSocketControlMessageClass *scm_class;
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (class);
  gobject_class->get_property = g_unix_credentials_message_get_property;
  gobject_class->set_property = g_unix_credentials_message_set_property;
  gobject_class->finalize = g_unix_credentials_message_finalize;
  gobject_class->constructed = g_unix_credentials_message_constructed;

  scm_class = G_SOCKET_CONTROL_MESSAGE_CLASS (class);
  scm_class->get_size = g_unix_credentials_message_get_size;
  scm_class->get_level = g_unix_credentials_message_get_level;
  scm_class->get_type = g_unix_credentials_message_get_msg_type;
  scm_class->serialize = g_unix_credentials_message_serialize;
  scm_class->deserialize = g_unix_credentials_message_deserialize;

  /**
   * GUnixCredentialsMessage:credentials:
   *
   * The credentials stored in the message.
   *
   * Since: 2.26
   */
  g_object_class_install_property (gobject_class,
                                   PROP_CREDENTIALS,
                                   g_param_spec_object ("credentials",
                                                        P_("Credentials"),
                                                        P_("The credentials stored in the message"),
                                                        G_TYPE_CREDENTIALS,
                                                        G_PARAM_READABLE |
                                                        G_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB |
                                                        G_PARAM_STATIC_NICK));

}

/* ---------------------------------------------------------------------------------------------------- */

/**
 * g_unix_credentials_message_is_supported:
 *
 * Checks if passing #GCredentials on a #GSocket is supported on this platform.
 *
 * Returns: %TRUE if supported, %FALSE otherwise
 *
 * Since: 2.26
 */
gboolean
g_unix_credentials_message_is_supported (void)
{
  return G_UNIX_CREDENTIALS_MESSAGE_SUPPORTED;
}

/* ---------------------------------------------------------------------------------------------------- */

/**
 * g_unix_credentials_message_new:
 *
 * Creates a new #GUnixCredentialsMessage with credentials matching the current processes.
 *
 * Returns: a new #GUnixCredentialsMessage
 *
 * Since: 2.26
 */
GSocketControlMessage *
g_unix_credentials_message_new (void)
{
  g_return_val_if_fail (g_unix_credentials_message_is_supported (), NULL);
  return g_object_new (G_TYPE_UNIX_CREDENTIALS_MESSAGE,
                       NULL);
}

/**
 * g_unix_credentials_message_new_with_credentials:
 * @credentials: A #GCredentials object.
 *
 * Creates a new #GUnixCredentialsMessage holding @credentials.
 *
 * Returns: a new #GUnixCredentialsMessage
 *
 * Since: 2.26
 */
GSocketControlMessage *
g_unix_credentials_message_new_with_credentials (GCredentials *credentials)
{
  g_return_val_if_fail (G_IS_CREDENTIALS (credentials), NULL);
  g_return_val_if_fail (g_unix_credentials_message_is_supported (), NULL);
  return g_object_new (G_TYPE_UNIX_CREDENTIALS_MESSAGE,
                       "credentials", credentials,
                       NULL);
}

/**
 * g_unix_credentials_message_get_credentials:
 * @message: A #GUnixCredentialsMessage.
 *
 * Gets the credentials stored in @message.
 *
 * Returns: (transfer none): A #GCredentials instance. Do not free, it is owned by @message.
 *
 * Since: 2.26
 */
GCredentials *
g_unix_credentials_message_get_credentials (GUnixCredentialsMessage *message)
{
  g_return_val_if_fail (G_IS_UNIX_CREDENTIALS_MESSAGE (message), NULL);
  return message->priv->credentials;
}
