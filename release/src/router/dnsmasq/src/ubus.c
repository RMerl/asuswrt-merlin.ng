/* dnsmasq is Copyright (c) 2000-2018 Simon Kelley

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

static struct ubus_context *ubus = NULL;
static struct blob_buf b;

static int ubus_handle_metrics(struct ubus_context *ctx, struct ubus_object *obj,
			       struct ubus_request_data *req, const char *method,
			       struct blob_attr *msg);
static struct ubus_method ubus_object_methods[] = {
  {.name = "metrics", .handler = ubus_handle_metrics},
};

static struct ubus_object_type ubus_object_type = UBUS_OBJECT_TYPE("dnsmasq", ubus_object_methods);

static struct ubus_object ubus_object = {
  .name = "dnsmasq",
  .type = &ubus_object_type,
  .methods = ubus_object_methods,
  .n_methods = ARRAY_SIZE(ubus_object_methods),
};

void set_ubus_listeners()
{
  if (!ubus)
    return;

  poll_listen(ubus->sock.fd, POLLIN);
  poll_listen(ubus->sock.fd, POLLERR);
  poll_listen(ubus->sock.fd, POLLHUP);
}

void check_ubus_listeners()
{
  if (!ubus)
    {
      ubus = ubus_connect(NULL);
      if (!ubus)
	return;
      ubus_add_object(ubus, &ubus_object);
    }
  
  if (poll_check(ubus->sock.fd, POLLIN))
    ubus_handle_event(ubus);
  
  if (poll_check(ubus->sock.fd, POLLHUP))
    {
      ubus_free(ubus);
      ubus = NULL;
    }
}


static int ubus_handle_metrics(struct ubus_context *ctx, struct ubus_object *obj,
			       struct ubus_request_data *req, const char *method,
			       struct blob_attr *msg)
{
  int i;
  blob_buf_init(&b, 0);

  for(i=0; i < __METRIC_MAX; i++)
    blobmsg_add_u32(&b, get_metric_name(i), daemon->metrics[i]);
  
  ubus_send_reply(ctx, req, b.head);
  
  return 0;
}

void ubus_event_bcast(const char *type, const char *mac, const char *ip, const char *name, const char *interface)
{
  if (!ubus || !ubus_object.has_subscribers)
    return;

  blob_buf_init(&b, 0);
  if (mac)
    blobmsg_add_string(&b, "mac", mac);
  if (ip)
    blobmsg_add_string(&b, "ip", ip);
  if (name)
    blobmsg_add_string(&b, "name", name);
  if (interface)
    blobmsg_add_string(&b, "interface", interface);
  
  ubus_notify(ubus, &ubus_object, type, b.head, -1);
}


#endif /* HAVE_UBUS */
