# SPDX-License-Identifier: LGPL-2.1-or-later

import dbus
import bluezutils

bus = dbus.SystemBus()


dummy = dbus.Interface(bus.get_object('org.bluez', '/'), 'org.freedesktop.DBus.Introspectable')

#print dummy.Introspect()


try:
	adapter = bluezutils.find_adapter()
except:
	pass
