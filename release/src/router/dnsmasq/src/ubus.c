/* dnsmasq is Copyright (c) 2000-2024 Simon Kelley

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
static int error_logged = 0;

static int ubus_handle_metrics(struct ubus_context *ctx, struct ubus_object *obj,
			       struct ubus_request_data *req, const char *method,
			       struct blob_attr *msg);

#ifdef HAVE_CONNTRACK
enum {
  SET_CONNMARK_ALLOWLIST_MARK,
  SET_CONNMARK_ALLOWLIST_MASK,
  SET_CONNMARK_ALLOWLIST_PATTERNS
};
static const struct blobmsg_policy set_connmark_allowlist_policy[] = {
  [SET_CONNMARK_ALLOWLIST_MARK] = {
    .name = "mark",
    .type = BLOBMSG_TYPE_INT32
  },
  [SET_CONNMARK_ALLOWLIST_MASK] = {
    .name = "mask",
    .type = BLOBMSG_TYPE_INT32
  },
  [SET_CONNMARK_ALLOWLIST_PATTERNS] = {
    .name = "patterns",
    .type = BLOBMSG_TYPE_ARRAY
  }
};
static int ubus_handle_set_connmark_allowlist(struct ubus_context *ctx, struct ubus_object *obj,
					      struct ubus_request_data *req, const char *method,
					      struct blob_attr *msg);
#endif

static void ubus_subscribe_cb(struct ubus_context *ctx, struct ubus_object *obj);

static const struct ubus_method ubus_object_methods[] = {
  UBUS_METHOD_NOARG("metrics", ubus_handle_metrics),
#ifdef HAVE_CONNTRACK
  UBUS_METHOD("set_connmark_allowlist", ubus_handle_set_connmark_allowlist, set_connmark_allowlist_policy),
#endif
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
}

static void ubus_destroy(struct ubus_context *ubus)
{
  ubus_free(ubus);
  daemon->ubus = NULL;
  
  /* Forces re-initialization when we're reusing the same definitions later on. */
  ubus_object.id = 0;
  ubus_object_type.id = 0;
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

char *ubus_init()
{
  struct ubus_context *ubus = NULL;
  int ret = 0;

  if (!(ubus = ubus_connect(NULL)))
    return NULL;
  
  ubus_object.name = daemon->ubus_name;
  ret = ubus_add_object(ubus, &ubus_object);
  if (ret)
    {
      ubus_destroy(ubus);
      return (char *)ubus_strerror(ret);
    }    
  
  ubus->connection_lost = ubus_disconnect_cb;
  daemon->ubus = ubus;
  error_logged = 0;

  return NULL;
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

#define CHECK(stmt) \
  do { \
    int e = (stmt); \
    if (e) \
      { \
	my_syslog(LOG_ERR, _("UBus command failed: %d (%s)"), e, #stmt); \
	return (UBUS_STATUS_UNKNOWN_ERROR); \
      } \
  } while (0)

static int ubus_handle_metrics(struct ubus_context *ctx, struct ubus_object *obj,
			       struct ubus_request_data *req, const char *method,
			       struct blob_attr *msg)
{
  int i;

  (void)obj;
  (void)method;
  (void)msg;

  CHECK(blob_buf_init(&b, BLOBMSG_TYPE_TABLE));

  for (i=0; i < __METRIC_MAX; i++)
    CHECK(blobmsg_add_u32(&b, get_metric_name(i), daemon->metrics[i]));
  
  CHECK(ubus_send_reply(ctx, req, b.head));
  return UBUS_STATUS_OK;
}

#ifdef HAVE_CONNTRACK
static int ubus_handle_set_connmark_allowlist(struct ubus_context *ctx, struct ubus_object *obj,
					      struct ubus_request_data *req, const char *method,
					      struct blob_attr *msg)
{
  const struct blobmsg_policy *policy = set_connmark_allowlist_policy;
  size_t policy_len = countof(set_connmark_allowlist_policy);
  struct allowlist *allowlists = NULL, **allowlists_pos;
  char **patterns = NULL, **patterns_pos;
  u32 mark, mask = UINT32_MAX;
  size_t num_patterns = 0;
  struct blob_attr *tb[policy_len];
  struct blob_attr *attr;
  
  if (blobmsg_parse(policy, policy_len, tb, blob_data(msg), blob_len(msg)))
    return UBUS_STATUS_INVALID_ARGUMENT;
  
  if (!tb[SET_CONNMARK_ALLOWLIST_MARK])
    return UBUS_STATUS_INVALID_ARGUMENT;
  mark = blobmsg_get_u32(tb[SET_CONNMARK_ALLOWLIST_MARK]);
  if (!mark)
    return UBUS_STATUS_INVALID_ARGUMENT;
  
  if (tb[SET_CONNMARK_ALLOWLIST_MASK])
    {
      mask = blobmsg_get_u32(tb[SET_CONNMARK_ALLOWLIST_MASK]);
      if (!mask || (mark & ~mask))
	return UBUS_STATUS_INVALID_ARGUMENT;
    }
  
  if (tb[SET_CONNMARK_ALLOWLIST_PATTERNS])
    {
      struct blob_attr *head = blobmsg_data(tb[SET_CONNMARK_ALLOWLIST_PATTERNS]);
      size_t len = blobmsg_data_len(tb[SET_CONNMARK_ALLOWLIST_PATTERNS]);
      __blob_for_each_attr(attr, head, len)
	{
	  char *pattern;
	  if (blob_id(attr) != BLOBMSG_TYPE_STRING)
	    return UBUS_STATUS_INVALID_ARGUMENT;
	  if (!(pattern = blobmsg_get_string(attr)))
	    return UBUS_STATUS_INVALID_ARGUMENT;
	  if (strcmp(pattern, "*") && !is_valid_dns_name_pattern(pattern))
	    return UBUS_STATUS_INVALID_ARGUMENT;
	  num_patterns++;
	}
    }
  
  for (allowlists_pos = &daemon->allowlists; *allowlists_pos; allowlists_pos = &(*allowlists_pos)->next)
    if ((*allowlists_pos)->mark == mark && (*allowlists_pos)->mask == mask)
      {
	struct allowlist *allowlists_next = (*allowlists_pos)->next;
	for (patterns_pos = (*allowlists_pos)->patterns; *patterns_pos; patterns_pos++)
	  {
	    free(*patterns_pos);
	    *patterns_pos = NULL;
	  }
	free((*allowlists_pos)->patterns);
	(*allowlists_pos)->patterns = NULL;
	free(*allowlists_pos);
	*allowlists_pos = allowlists_next;
	break;
      }
  
  if (!num_patterns)
    return UBUS_STATUS_OK;
  
  patterns = whine_malloc((num_patterns + 1) * sizeof(char *));
  if (!patterns)
    goto fail;
  patterns_pos = patterns;
  if (tb[SET_CONNMARK_ALLOWLIST_PATTERNS])
    {
      struct blob_attr *head = blobmsg_data(tb[SET_CONNMARK_ALLOWLIST_PATTERNS]);
      size_t len = blobmsg_data_len(tb[SET_CONNMARK_ALLOWLIST_PATTERNS]);
      __blob_for_each_attr(attr, head, len)
	{
	  char *pattern;
	  if (!(pattern = blobmsg_get_string(attr)))
	    goto fail;
	  if (!(*patterns_pos = whine_malloc(strlen(pattern) + 1)))
	    goto fail;
	  strcpy(*patterns_pos++, pattern);
	}
    }
  
  allowlists = whine_malloc(sizeof(struct allowlist));
  if (!allowlists)
    goto fail;
  memset(allowlists, 0, sizeof(struct allowlist));
  allowlists->mark = mark;
  allowlists->mask = mask;
  allowlists->patterns = patterns;
  allowlists->next = daemon->allowlists;
  daemon->allowlists = allowlists;
  return UBUS_STATUS_OK;
  
fail:
  if (patterns)
    {
      for (patterns_pos = patterns; *patterns_pos; patterns_pos++)
	{
	  free(*patterns_pos);
	  *patterns_pos = NULL;
	}
      free(patterns);
      patterns = NULL;
    }
  if (allowlists)
    {
      free(allowlists);
      allowlists = NULL;
    }
  return UBUS_STATUS_UNKNOWN_ERROR;
}
#endif

#undef CHECK

#define CHECK(stmt) \
  do { \
    int e = (stmt); \
    if (e) \
      { \
	my_syslog(LOG_ERR, _("UBus command failed: %d (%s)"), e, #stmt); \
	return; \
      } \
  } while (0)

void ubus_event_bcast(const char *type, const char *mac, const char *ip, const char *name, const char *interface)
{
  struct ubus_context *ubus = (struct ubus_context *)daemon->ubus;

  if (!ubus || !ubus_object.has_subscribers)
    return;

  CHECK(blob_buf_init(&b, BLOBMSG_TYPE_TABLE));
  if (mac)
    CHECK(blobmsg_add_string(&b, "mac", mac));
  if (ip)
    CHECK(blobmsg_add_string(&b, "ip", ip));
  if (name)
    CHECK(blobmsg_add_string(&b, "name", name));
  if (interface)
    CHECK(blobmsg_add_string(&b, "interface", interface));
  
  CHECK(ubus_notify(ubus, &ubus_object, type, b.head, -1));
}

#ifdef HAVE_CONNTRACK
void ubus_event_bcast_connmark_allowlist_refused(u32 mark, const char *name)
{
  struct ubus_context *ubus = (struct ubus_context *)daemon->ubus;

  if (!ubus || !ubus_object.has_subscribers)
    return;

  CHECK(blob_buf_init(&b, 0));
  CHECK(blobmsg_add_u32(&b, "mark", mark));
  CHECK(blobmsg_add_string(&b, "name", name));
  
  CHECK(ubus_notify(ubus, &ubus_object, "connmark-allowlist.refused", b.head, -1));
}

void ubus_event_bcast_connmark_allowlist_resolved(u32 mark, const char *name, const char *value, u32 ttl)
{
  struct ubus_context *ubus = (struct ubus_context *)daemon->ubus;

  if (!ubus || !ubus_object.has_subscribers)
    return;

  CHECK(blob_buf_init(&b, 0));
  CHECK(blobmsg_add_u32(&b, "mark", mark));
  CHECK(blobmsg_add_string(&b, "name", name));
  CHECK(blobmsg_add_string(&b, "value", value));
  CHECK(blobmsg_add_u32(&b, "ttl", ttl));
  
  /* Set timeout to allow UBus subscriber to configure firewall rules before returning. */
  CHECK(ubus_notify(ubus, &ubus_object, "connmark-allowlist.resolved", b.head, /* timeout: */ 1000));
}
#endif

#undef CHECK

#endif /* HAVE_UBUS */
