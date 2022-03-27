#!/usr/bin/python

from __future__ import absolute_import, print_function, unicode_literals
# -*- coding: utf-8 -*-

import sys
import dbus
import dbus.service
from dbus.mainloop.glib import DBusGMainLoop
try:
  from gi.repository import GObject
except ImportError:
  import gobject as GObject

BUS_NAME = 'org.bluez'
PATH = '/org/bluez'
ADAPTER_INTERFACE = 'org.bluez.Adapter1'
HEALTH_MANAGER_INTERFACE = 'org.bluez.HealthManager1'
HEALTH_DEVICE_INTERFACE = 'org.bluez.HealthDevice1'

DBusGMainLoop(set_as_default=True)
loop = GObject.MainLoop()

bus = dbus.SystemBus()

type = 4103
if len(sys.argv) > 1:
	type = int(sys.argv[1])

hdp_manager = dbus.Interface(bus.get_object(BUS_NAME, PATH),
						HEALTH_MANAGER_INTERFACE)
app_path = hdp_manager.CreateApplication({"DataType": dbus.types.UInt16(type),
					"Role": "sink"})

print(app_path)

manager = dbus.Interface(bus.get_object(BUS_NAME, "/"),
					"org.freedesktop.DBus.ObjectManager")

objects = manager.GetManagedObjects()
adapters = []

for path, ifaces in objects.iteritems():
	if ifaces.has_key(ADAPTER_INTERFACE):
		adapters.append(path)

i = 1
for ad in adapters:
	print("%d. %s" % (i, ad))
	i = i + 1

print("Select an adapter: ",)
select = None
while select == None:
	try:
		pos = int(sys.stdin.readline()) - 1
		if pos < 0:
			raise TypeError
		select = adapters[pos]
	except (TypeError, IndexError, ValueError):
		print("Wrong selection, try again: ",)
	except KeyboardInterrupt:
		sys.exit()

adapter =  dbus.Interface(bus.get_object(BUS_NAME, select),
						ADAPTER_INTERFACE)

devices = []
for path, interfaces in objects.iteritems():
	if "org.bluez.Device1" not in interfaces:
		continue
	properties = interfaces["org.bluez.Device1"]
	if properties["Adapter"] != select:
		continue;

	if HEALTH_DEVICE_INTERFACE not in interfaces:
		continue
	devices.append(path)

if len(devices) == 0:
	print("No devices available")
	sys.exit()

i = 1
for dev in devices:
	print("%d. %s" % (i, dev))
	i = i + 1

print("Select a device: ",)
select = None
while select == None:
	try:
		pos = int(sys.stdin.readline()) - 1
		if pos < 0:
			raise TypeError
		select = devices[pos]
	except (TypeError, IndexError, ValueError):
		print("Wrong selection, try again: ",)
	except KeyboardInterrupt:
		sys.exit()

print("Connecting to %s" % (select))
device = dbus.Interface(bus.get_object(BUS_NAME, select),
						HEALTH_DEVICE_INTERFACE)

chan = device.CreateChannel(app_path, "Any")

print(chan)

print("Push Enter for finishing")
sys.stdin.readline()

hdp_manager.DestroyApplication(app_path)
