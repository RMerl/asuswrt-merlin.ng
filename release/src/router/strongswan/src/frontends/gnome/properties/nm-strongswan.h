/*
 * Copyright (C) 2008 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
 * Copyright (C) 2008 Dan Williams
 *
 * Based on NetworkManager's vpnc plugin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#ifndef _NM_STRONGSWAN_H_
#define _NM_STRONGSWAN_H_

#include <glib-object.h>

typedef enum
{
	STRONGSWAN_PLUGIN_UI_ERROR_UNKNOWN = 0,
	STRONGSWAN_PLUGIN_UI_ERROR_INVALID_PROPERTY,
	STRONGSWAN_PLUGIN_UI_ERROR_MISSING_PROPERTY
} StrongswanPluginUiError;

#define STRONGSWAN_TYPE_PLUGIN_UI_ERROR (strongswan_plugin_ui_error_get_type ())
GType strongswan_plugin_ui_error_get_type (void);

#define STRONGSWAN_TYPE_PLUGIN_UI            (strongswan_plugin_ui_get_type ())
#define STRONGSWAN_PLUGIN_UI(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), STRONGSWAN_TYPE_PLUGIN_UI, StrongswanPluginUi))
#define STRONGSWAN_PLUGIN_UI_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), STRONGSWAN_TYPE_PLUGIN_UI, StrongswanPluginUiClass))
#define STRONGSWAN_IS_PLUGIN_UI(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), STRONGSWAN_TYPE_PLUGIN_UI))
#define STRONGSWAN_IS_PLUGIN_UI_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), STRONGSWAN_TYPE_PLUGIN_UI))
#define STRONGSWAN_PLUGIN_UI_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), STRONGSWAN_TYPE_PLUGIN_UI, StrongswanPluginUiClass))

typedef struct _StrongswanPluginUi StrongswanPluginUi;
typedef struct _StrongswanPluginUiClass StrongswanPluginUiClass;

struct _StrongswanPluginUi {
	GObject parent;
};

struct _StrongswanPluginUiClass {
	GObjectClass parent;
};

GType strongswan_plugin_ui_get_type (void);


#define STRONGSWAN_TYPE_PLUGIN_UI_WIDGET            (strongswan_plugin_ui_widget_get_type ())
#define STRONGSWAN_PLUGIN_UI_WIDGET(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), STRONGSWAN_TYPE_PLUGIN_UI_WIDGET, StrongswanPluginUiWidget))
#define STRONGSWAN_PLUGIN_UI_WIDGET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), STRONGSWAN_TYPE_PLUGIN_UI_WIDGET, StrongswanPluginUiWidgetClass))
#define STRONGSWAN_IS_PLUGIN_UI_WIDGET(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), STRONGSWAN_TYPE_PLUGIN_UI_WIDGET))
#define STRONGSWAN_IS_PLUGIN_UI_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), STRONGSWAN_TYPE_PLUGIN_UI_WIDGET))
#define STRONGSWAN_PLUGIN_UI_WIDGET_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), STRONGSWAN_TYPE_PLUGIN_UI_WIDGET, StrongswanPluginUiWidgetClass))

typedef struct _StrongswanPluginUiWidget StrongswanPluginUiWidget;
typedef struct _StrongswanPluginUiWidgetClass StrongswanPluginUiWidgetClass;

struct _StrongswanPluginUiWidget {
	GObject parent;
};

struct _StrongswanPluginUiWidgetClass {
	GObjectClass parent;
};

GType strongswan_plugin_ui_widget_get_type (void);

#endif	/* _NM_STRONGSWAN_H_ */

