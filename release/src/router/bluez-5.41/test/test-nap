#!/usr/bin/python

from __future__ import absolute_import, print_function, unicode_literals

from optparse import OptionParser, make_option
import sys
import time
import dbus
import bluezutils
import dbus.mainloop.glib
try:
  from gi.repository import GObject
except ImportError:
  import gobject as GObject

bus = dbus.SystemBus()

option_list = [
		make_option("-i", "--device", action="store",
				type="string", dest="dev_id"),
		]
parser = OptionParser(option_list=option_list)

(options, args) = parser.parse_args()

adapter_path = bluezutils.find_adapter(options.dev_id).object_path
server = dbus.Interface(bus.get_object("org.bluez", adapter_path),
						"org.bluez.NetworkServer1")

service = "nap"

if (len(args) < 1):
	bridge = "tether"
else:
	bridge = args[0]

dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

mainloop = GObject.MainLoop()

server.Register(service, bridge)

print("Server for %s registered for %s" % (service, bridge))

print("Press CTRL-C to disconnect")

mainloop.run()
