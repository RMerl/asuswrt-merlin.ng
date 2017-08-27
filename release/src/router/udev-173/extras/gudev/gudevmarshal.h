
#ifndef __g_udev_marshal_MARSHAL_H__
#define __g_udev_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:STRING,OBJECT (extras/gudev/gudevmarshal.list:1) */
extern void g_udev_marshal_VOID__STRING_OBJECT (GClosure     *closure,
                                                GValue       *return_value,
                                                guint         n_param_values,
                                                const GValue *param_values,
                                                gpointer      invocation_hint,
                                                gpointer      marshal_data);

G_END_DECLS

#endif /* __g_udev_marshal_MARSHAL_H__ */

