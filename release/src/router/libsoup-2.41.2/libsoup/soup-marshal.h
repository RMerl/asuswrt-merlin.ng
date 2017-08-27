
#ifndef ___soup_marshal_MARSHAL_H__
#define ___soup_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* NONE:BOXED (soup-marshal.list:1) */
#define _soup_marshal_VOID__BOXED	g_cclosure_marshal_VOID__BOXED
#define _soup_marshal_NONE__BOXED	_soup_marshal_VOID__BOXED

/* NONE:BOXED,BOXED (soup-marshal.list:2) */
extern void _soup_marshal_VOID__BOXED_BOXED (GClosure     *closure,
                                             GValue       *return_value,
                                             guint         n_param_values,
                                             const GValue *param_values,
                                             gpointer      invocation_hint,
                                             gpointer      marshal_data);
#define _soup_marshal_NONE__BOXED_BOXED	_soup_marshal_VOID__BOXED_BOXED

/* NONE:NONE (soup-marshal.list:3) */
#define _soup_marshal_VOID__VOID	g_cclosure_marshal_VOID__VOID
#define _soup_marshal_NONE__NONE	_soup_marshal_VOID__VOID

/* NONE:OBJECT (soup-marshal.list:4) */
#define _soup_marshal_VOID__OBJECT	g_cclosure_marshal_VOID__OBJECT
#define _soup_marshal_NONE__OBJECT	_soup_marshal_VOID__OBJECT

/* NONE:OBJECT,OBJECT (soup-marshal.list:5) */
extern void _soup_marshal_VOID__OBJECT_OBJECT (GClosure     *closure,
                                               GValue       *return_value,
                                               guint         n_param_values,
                                               const GValue *param_values,
                                               gpointer      invocation_hint,
                                               gpointer      marshal_data);
#define _soup_marshal_NONE__OBJECT_OBJECT	_soup_marshal_VOID__OBJECT_OBJECT

/* NONE:OBJECT,OBJECT,BOOLEAN (soup-marshal.list:6) */
extern void _soup_marshal_VOID__OBJECT_OBJECT_BOOLEAN (GClosure     *closure,
                                                       GValue       *return_value,
                                                       guint         n_param_values,
                                                       const GValue *param_values,
                                                       gpointer      invocation_hint,
                                                       gpointer      marshal_data);
#define _soup_marshal_NONE__OBJECT_OBJECT_BOOLEAN	_soup_marshal_VOID__OBJECT_OBJECT_BOOLEAN

/* NONE:OBJECT,POINTER (soup-marshal.list:7) */
extern void _soup_marshal_VOID__OBJECT_POINTER (GClosure     *closure,
                                                GValue       *return_value,
                                                guint         n_param_values,
                                                const GValue *param_values,
                                                gpointer      invocation_hint,
                                                gpointer      marshal_data);
#define _soup_marshal_NONE__OBJECT_POINTER	_soup_marshal_VOID__OBJECT_POINTER

/* NONE:STRING,BOXED (soup-marshal.list:8) */
extern void _soup_marshal_VOID__STRING_BOXED (GClosure     *closure,
                                              GValue       *return_value,
                                              guint         n_param_values,
                                              const GValue *param_values,
                                              gpointer      invocation_hint,
                                              gpointer      marshal_data);
#define _soup_marshal_NONE__STRING_BOXED	_soup_marshal_VOID__STRING_BOXED

/* NONE:STRING,STRING (soup-marshal.list:9) */
extern void _soup_marshal_VOID__STRING_STRING (GClosure     *closure,
                                               GValue       *return_value,
                                               guint         n_param_values,
                                               const GValue *param_values,
                                               gpointer      invocation_hint,
                                               gpointer      marshal_data);
#define _soup_marshal_NONE__STRING_STRING	_soup_marshal_VOID__STRING_STRING

G_END_DECLS

#endif /* ___soup_marshal_MARSHAL_H__ */

