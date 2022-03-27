#!/usr/bin/python

from __future__ import absolute_import, print_function, unicode_literals

from optparse import OptionParser, make_option
import dbus
import dbus.mainloop.glib
try:
  from gi.repository import GObject
except ImportError:
  import gobject as GObject
import bluezutils

compact = False
devices = {}

def print_compact(address, properties):
	name = ""
	address = "<unknown>"

	for key, value in properties.iteritems():
		if type(value) is dbus.String:
			value = unicode(value).encode('ascii', 'replace')
		if (key == "Name"):
			name = value
		elif (key == "Address"):
			address = value

	if "Logged" in properties:
		flag = "*"
	else:
		flag = " "

	print("%s%s %s" % (flag, address, name))

	properties["Logged"] = True

def print_normal(address, properties):
	print("[ " + address + " ]")

	for key in properties.keys():
		value = properties[key]
		if type(value) is dbus.String:
			value = unicode(value).encode('ascii', 'replace')
		if (key == "Class"):
			print("    %s = 0x%06x" % (key, value))
		else:
			print("    %s = %s" % (key, value))

	print()

	properties["Logged"] = True

def skip_dev(old_dev, new_dev):
	if not "Logged" in old_dev:
		return False
	if "Name" in old_dev:
		return True
	if not "Name" in new_dev:
		return True
	return False

def interfaces_added(path, interfaces):
	properties = interfaces["org.bluez.Device1"]
	if not properties:
		return

	if path in devices:
		dev = devices[path]

		if compact and skip_dev(dev, properties):
			return
		devices[path] = dict(devices[path].items() + properties.items())
	else:
		devices[path] = properties

	if "Address" in devices[path]:
		address = properties["Address"]
	else:
		address = "<unknown>"

	if compact:
		print_compact(address, devices[path])
	else:
		print_normal(address, devices[path])

def properties_changed(interface, changed, invalidated, path):
	if interface != "org.bluez.Device1":
		return

	if path in devices:
		dev = devices[path]

		if compact and skip_dev(dev, changed):
			return
		devices[path] = dict(devices[path].items() + changed.items())
	else:
		devices[path] = changed

	if "Address" in devices[path]:
		address = devices[path]["Address"]
	else:
		address = "<unknown>"

	if compact:
		print_compact(address, devices[path])
	else:
		print_normal(address, devices[path])

if __name__ == '__main__':
	dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

	bus = dbus.SystemBus()

	option_list = [
			make_option("-i", "--device", action="store",
					type="string", dest="dev_id"),
			make_option("-u", "--uuids", action="store",
					type="string", dest="uuids",
					help="Filtered service UUIDs [uuid1,uuid2,...]"),
			make_option("-r", "--rssi", action="store",
					type="int", dest="rssi",
					help="RSSI threshold value"),
			make_option("-p", "--pathloss", action="store",
					type="int", dest="pathloss",
					help="Pathloss threshold value"),
			make_option("-t", "--transport", action="store",
					type="string", dest="transport",
					help="Type of scan to run (le/bredr/auto)"),
			make_option("-c", "--compact",
					action="store_true", dest="compact"),
			]
	parser = OptionParser(option_list=option_list)

	(options, args) = parser.parse_args()

	adapter = bluezutils.find_adapter(options.dev_id)

	if options.compact:
		compact = True;

	bus.add_signal_receiver(interfaces_added,
			dbus_interface = "org.freedesktop.DBus.ObjectManager",
			signal_name = "InterfacesAdded")

	bus.add_signal_receiver(properties_changed,
			dbus_interface = "org.freedesktop.DBus.Properties",
			signal_name = "PropertiesChanged",
			arg0 = "org.bluez.Device1",
			path_keyword = "path")

	om = dbus.Interface(bus.get_object("org.bluez", "/"),
				"org.freedesktop.DBus.ObjectManager")
	objects = om.GetManagedObjects()
	for path, interfaces in objects.iteritems():
		if "org.bluez.Device1" in interfaces:
			devices[path] = interfaces["org.bluez.Device1"]

	scan_filter = dict()

	if options.uuids:
		uuids = []
		uuid_list = options.uuids.split(',')
		for uuid in uuid_list:
			uuids.append(uuid)

		scan_filter.update({ "UUIDs": uuids })

	if options.rssi:
		scan_filter.update({ "RSSI": dbus.Int16(options.rssi) })

	if options.pathloss:
		scan_filter.update({ "Pathloss": dbus.UInt16(options.pathloss) })

	if options.transport:
		scan_filter.update({ "Transport": options.transport })

	adapter.SetDiscoveryFilter(scan_filter)
	adapter.StartDiscovery()

	mainloop = GObject.MainLoop()
	mainloop.run()
