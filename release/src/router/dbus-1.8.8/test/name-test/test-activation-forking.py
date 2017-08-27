#!/usr/bin/env python

import os,sys

try:
    import gobject
    import dbus
    import dbus.mainloop.glib
except:
    print "Failed import, aborting test"
    sys.exit(0)

dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
loop = gobject.MainLoop()

exitcode = 0

bus = dbus.SessionBus()
bus_iface = dbus.Interface(bus.get_object('org.freedesktop.DBus', '/org/freedesktop/DBus'), 'org.freedesktop.DBus')

o = bus.get_object('org.freedesktop.DBus.TestSuiteForkingEchoService', '/org/freedesktop/TestSuite')
i = dbus.Interface(o, 'org.freedesktop.TestSuite')

# Start it up
reply = i.Echo("hello world")
print "TestSuiteForkingEchoService initial reply OK"

def ignore(*args, **kwargs):
    pass

# Now monitor for exits, when that happens, start it up again.
# The goal here is to try to hit any race conditions in activation.
counter = 0
def on_forking_echo_owner_changed(name, old, new):
    global counter
    global o
    global i
    if counter > 10:
        print "Activated 10 times OK, TestSuiteForkingEchoService pass"
        loop.quit()
        return
    counter += 1
    if new == '':
        o = bus.get_object('org.freedesktop.DBus.TestSuiteForkingEchoService', '/org/freedesktop/TestSuite')
        i = dbus.Interface(o, 'org.freedesktop.TestSuite')
        i.Echo("counter %r" % counter)
        i.Exit(reply_handler=ignore, error_handler=ignore)

bus_iface.connect_to_signal('NameOwnerChanged', on_forking_echo_owner_changed, arg0='org.freedesktop.DBus.TestSuiteForkingEchoService')

i.Exit(reply_handler=ignore, error_handler=ignore)

def check_counter():
    if counter == 0:
        print "Failed to get NameOwnerChanged for TestSuiteForkingEchoService"
        sys.exit(1)
gobject.timeout_add(15000, check_counter)

loop.run()
sys.exit(0)
