/* dnsmasq is Copyright (c) 2000-2021 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991, or
   (at your option) version 3 dated 29 June, 2007.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
     
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dnsmasq.h"

#ifdef HAVE_UBUS

#include <libubus.h>

static struct blob_buf b;
static int notify;
static int error_logged = 0;

static int ubus_handle_metrics(struct ubus_context *ctx, struct ubus_object *obj,
			       struct ubus_request_data *req, const char *method,
			       struct blob_attr *msg);

static void ubus_subscribe_cb(struct ubus_context *ctx, struct ubus_object *obj);

static const struct ubus_method ubus_object_methods[] = {
  UBUS_METHOD_NOARG("metrics", ubus_handle_metrics),
};

static struct ubus_object_type ubus_object_type =
  UBUS_OBJECT_TYPE("dnsmasq", ubus_object_methods);

static struct ubus_object ubus_object = {
  .name = NULL,
  .type = &ubus_object_type,
  .methods = ubus_object_methods,
  .n_methods = ARRAY_SIZE(ubus_object_methods),
  .subscribe_cb = ubus_subscribe_cb,
};

static void ubus_subscribe_cb(struct ubus_context *ctx, struct ubus_object *obj)
{
  (void)ctx;

  my_syslog(LOG_DEBUG, _("UBus subscription callback: %s subscriber(s)"), obj->has_subscribers ? "1" : "0");
  notify = obj->has_subscribers;
}

static void ubus_destroy(struct ubus_context *ubus)
{
  // Forces re-initialization when we're reusing the same definitions later on.
  ubus_object.id = 0;
  ubus_object_type.id = 0;

  ubus_free(ubus);
  daemon->ubus = NULL;
}

static void ubus_disconnect_cb(struct ubus_context *ubus)
{
  int ret;

  ret = ubus_reconnect(ubus, NULL);
  if (ret)
    {
      my_syslog(LOG_ERR, _("Cannot reconnect to UBus: %s"), ubus_strerror(ret));

      ubus_destroy(ubus);
    }
}

void ubus_init()
{
  struct ubus_context *ubus = NULL;
  int ret = 0;

  ubus = ubus_connect(NULL);
  if (!ubus)
    {
      if (!error_logged)
        {
          my_syslog(LOG_ERR, _("Cannot initialize UBus: connection failed"));
          error_logged = 1;
        }

      ubus_destroy(ubus);
      return;
    }

  ubus_object.name = daemon->ubus_name;
  ret = ubus_add_object(ubus, &ubus_object);
  if (ret)
    {
      if (!error_logged)
        {
          my_syslog(LOG_ERR, _("Cannot add object to UBus: %s"), ubus_strerror(ret));
          error_logged = 1;
        }
      ubus_destroy(ubus);
      return;
    }

  ubus->connection_lost = ubus_disconnect_cb;
  daemon->ubus = ubus;
  error_logged = 0;

  my_syslog(LOG_INFO, _("Connected to system UBus"));
}

void set_ubus_listeners()
{
  struct ubus_context *ubus = (struct ubus_context *)daemon->ubus;
  if (!ubus)
    {
      if (!error_logged)
        {
          my_syslog(LOG_ERR, _("Cannot set UBus listeners: no connection"));
          error_logged = 1;
        }
      return;
    }

  error_logged = 0;

  poll_listen(ubus->sock.fd, POLLIN);
  poll_listen(ubus->sock.fd, POLLERR);
  poll_listen(ubus->sock.fd, POLLHUP);
}

void check_ubus_listeners()
{
  struct ubus_context *ubus = (struct ubus_context *)daemon->ubus;
  if (!ubus)
    {
      if (!error_logged)
        {
          my_syslog(LOG_ERR, _("Cannot poll UBus listeners: no connection"));
          error_logged = 1;
        }
      return;
    }
  
  error_logged = 0;

  if (poll_check(ubus->sock.fd, POLLIN))
    ubus_handle_event(ubus);
  
  if (poll_check(ubus->sock.fd, POLLHUP | POLLERR))
    {
      my_syslog(LOG_INFO, _("Disconnecting from UBus"));

      ubus_destroy(ubus);
    }
}

static int ubus_handle_metrics(struct ubus_context *ctx, struct ubus_object *obj,
			       struct ubus_request_data *req, const char *method,
			       struct blob_attr *msg)
{
  int i;

  (void)obj;
  (void)method;
  (void)msg;

  blob_buf_init(&b, BLOBMSG_TYPE_TABLE);

  for (i=0; i < __METRIC_MAX; i++)
    blobmsg_add_u32(&b, get_metric_name(i), daemon->metrics[i]);
  
  return ubus_send_reply(ctx, req, b.head);
}

void ubus_event_bcast(const char *type, const char *mac, const char *ip, const char *name, const char *interface)
{
  struct ubus_context *ubus = (struct ubus_context *)daemon->ubus;
  int ret;

  if (!ubus || !notify)
    return;

  blob_buf_init(&b, BLOBMSG_TYPE_TABLE);
  if (mac)
    blobmsg_add_string(&b, "mac", mac);
  if (ip)
    blobmsg_add_string(&b, "ip", ip);
  if (name)
    blobmsg_add_string(&b, "name", name);
  if (interface)
    blobmsg_add_string(&b, "interface", interface);
  
  ret = ubus_notify(ubus, &ubus_object, type, b.head, -1);
  if (!ret)
    my_syslog(LOG_ERR, _("Failed to send UBus event: %s"), ubus_strerror(ret));
}


#endif /* HAVE_UBUS */
