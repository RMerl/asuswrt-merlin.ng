#!/usr/bin/python

from __future__ import absolute_import, print_function, unicode_literals

from optparse import OptionParser, make_option
import os
import sys
import uuid
import dbus
import dbus.service
import dbus.mainloop.glib
try:
  from gi.repository import GObject
except ImportError:
  import gobject as GObject

class Profile(dbus.service.Object):
	fd = -1

	@dbus.service.method("org.bluez.Profile1",
					in_signature="", out_signature="")
	def Release(self):
		print("Release")
		mainloop.quit()

	@dbus.service.method("org.bluez.Profile1",
					in_signature="", out_signature="")
	def Cancel(self):
		print("Cancel")

	@dbus.service.method("org.bluez.Profile1",
				in_signature="oha{sv}", out_signature="")
	def NewConnection(self, path, fd, properties):
		self.fd = fd.take()
		print("NewConnection(%s, %d)" % (path, self.fd))
		for key in properties.keys():
			if key == "Version" or key == "Features":
				print("  %s = 0x%04x" % (key, properties[key]))
			else:
				print("  %s = %s" % (key, properties[key]))

	@dbus.service.method("org.bluez.Profile1",
				in_signature="o", out_signature="")
	def RequestDisconnection(self, path):
		print("RequestDisconnection(%s)" % (path))

		if (self.fd > 0):
			os.close(self.fd)
			self.fd = -1

if __name__ == '__main__':
	dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

	bus = dbus.SystemBus()

	manager = dbus.Interface(bus.get_object("org.bluez",
				"/org/bluez"), "org.bluez.ProfileManager1")

	option_list = [
			make_option("-u", "--uuid", action="store",
					type="string", dest="uuid",
					default=None),
			make_option("-p", "--path", action="store",
					type="string", dest="path",
					default="/foo/bar/profile"),
			make_option("-n", "--name", action="store",
					type="string", dest="name",
					default=None),
			make_option("-s", "--server",
					action="store_const",
					const="server", dest="role"),
			make_option("-c", "--client",
					action="store_const",
					const="client", dest="role"),
			make_option("-a", "--auto-connect",
					action="store_true",
					dest="auto_connect", default=False),
			make_option("-P", "--PSM", action="store",
					type="int", dest="psm",
					default=None),
			make_option("-C", "--channel", action="store",
					type="int", dest="channel",
					default=None),
			make_option("-r", "--record", action="store",
					type="string", dest="record",
					default=None),
			make_option("-S", "--service", action="store",
					type="string", dest="service",
					default=None),
			]

	parser = OptionParser(option_list=option_list)

	(options, args) = parser.parse_args()

	profile = Profile(bus, options.path)

	mainloop = GObject.MainLoop()

	opts = {
			"AutoConnect" :	options.auto_connect,
		}

	if (options.name):
		opts["Name"] = options.name

	if (options.role):
		opts["Role"] = options.role

	if (options.psm is not None):
		opts["PSM"] = dbus.UInt16(options.psm)

	if (options.channel is not None):
		opts["Channel"] = dbus.UInt16(options.channel)

	if (options.record):
		opts["ServiceRecord"] = options.record

	if (options.service):
		opts["Service"] = options.service

	if not options.uuid:
		options.uuid = str(uuid.uuid4())

	manager.RegisterProfile(options.path, options.uuid, opts)

	mainloop.run()
