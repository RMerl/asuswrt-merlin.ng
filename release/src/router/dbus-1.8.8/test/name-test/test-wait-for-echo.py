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

def handle_noreceipt():
    print "Failed to get signal"
    global exitcode
    exitcode = 1
    loop.quit()

gobject.timeout_add(7000, handle_noreceipt)

bus = dbus.SessionBus()

def sighandler(*args, **kwargs):
    print "got signal"
    loop.quit()   

bus.add_signal_receiver(sighandler, dbus_interface='org.freedesktop.TestSuite', signal_name='Foo')

o = bus.get_object('org.freedesktop.DBus.TestSuiteEchoService', '/org/freedesktop/TestSuite')
i = dbus.Interface(o, 'org.freedesktop.TestSuite')
def nullhandler(*args, **kwargs):
    pass
i.EmitFoo(reply_handler=nullhandler, error_handler=nullhandler)

loop.run()
sys.exit(exitcode)
