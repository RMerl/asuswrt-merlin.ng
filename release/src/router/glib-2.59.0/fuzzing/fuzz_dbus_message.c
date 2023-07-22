#include "fuzz.h"

const static GDBusCapabilityFlags flags = G_DBUS_CAPABILITY_FLAGS_UNIX_FD_PASSING;

int
LLVMFuzzerTestOneInput (const unsigned char *data, size_t size)
{
  gssize bytes;
  GDBusMessage *msg = NULL;
  guchar *blob = NULL;
  gsize msg_size;

  fuzz_set_logging_func ();

  bytes = g_dbus_message_bytes_needed ((guchar*) data, size, NULL);
  if (bytes <= 0)
    return 0;

  msg = g_dbus_message_new_from_blob ((guchar*) data, size, flags, NULL);
  if (msg == NULL)
    return 0;

  blob = g_dbus_message_to_blob (msg, &msg_size, flags, NULL);

  g_free (blob);
  g_object_unref (msg);
  return 0;
}
