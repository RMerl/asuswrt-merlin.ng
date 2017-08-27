
/* Generated data (by glib-mkenums) */

#include <gudev.h>

/* enumerations from "extras/gudev/gudevenums.h" */
GType
g_udev_device_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { G_UDEV_DEVICE_TYPE_NONE, "G_UDEV_DEVICE_TYPE_NONE", "none" },
        { G_UDEV_DEVICE_TYPE_BLOCK, "G_UDEV_DEVICE_TYPE_BLOCK", "block" },
        { G_UDEV_DEVICE_TYPE_CHAR, "G_UDEV_DEVICE_TYPE_CHAR", "char" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GUdevDeviceType"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}


/* Generated data ends here */

