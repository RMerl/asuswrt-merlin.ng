#!/usr/bin/python3
# SPDX-License-Identifier: LGPL-2.1-or-later

import sys
import dbus
import dbus.service
import numpy

try:
  from termcolor import colored, cprint
  set_green = lambda x: colored(x, 'green', attrs=['bold'])
  set_cyan = lambda x: colored(x, 'cyan', attrs=['bold'])
except ImportError:
  set_green = lambda x: x
  set_cyan = lambda x: x

AGENT_IFACE = 'org.bluez.mesh.ProvisionAgent1'
AGENT_PATH = "/mesh/test/agent"

bus = None

def array_to_string(b_array):
	str_value = ""
	for b in b_array:
		str_value += "%02x" % b
	return str_value

class Agent(dbus.service.Object):
	def __init__(self, bus):
		self.path = AGENT_PATH
		self.bus = bus
		dbus.service.Object.__init__(self, bus, self.path)

	def get_properties(self):
		caps = []
		oob = []
		caps.append('out-numeric')
		caps.append('static-oob')
		oob.append('other')
		return {
			AGENT_IFACE: {
				'Capabilities': dbus.Array(caps, 's'),
				'OutOfBandInfo': dbus.Array(oob, 's')
			}
		}

	def get_path(self):
		return dbus.ObjectPath(self.path)

	@dbus.service.method(AGENT_IFACE, in_signature="", out_signature="")
	def Cancel(self):
		print("Cancel")

	@dbus.service.method(AGENT_IFACE, in_signature="su", out_signature="")
	def DisplayNumeric(self, type, value):
		print(set_cyan('DisplayNumeric ('), type,
				set_cyan(') number ='), set_green(value))

	@dbus.service.method(AGENT_IFACE, in_signature="s", out_signature="ay")
	def PromptStatic(self, type):
		static_key = numpy.random.randint(0, 255, 16)
		key_str = array_to_string(static_key)

		print(set_cyan('PromptStatic ('), type, set_cyan(')'))
		print(set_cyan('Enter 16 octet key on remote device: '),
							set_green(key_str));

		return dbus.Array(static_key, signature='y')
