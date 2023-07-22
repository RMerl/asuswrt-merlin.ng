#include <gio/gio.h>

int main(int argc, char *argv[])
{
  GApplication *app = g_application_new (NULL, 0);
  g_object_unref (app);
  return 0;
}
