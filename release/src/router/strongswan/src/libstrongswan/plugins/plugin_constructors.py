#!/usr/bin/env python
#
# Copyright (C) 2017 Tobias Brunner
# HSR Hochschule fuer Technik Rapperswil
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.

import sys
from argparse import ArgumentParser

def generate_output(plugins):
	"""Generate a source file containing plugin constructor registrations"""
	print("/**")
	print(" * Register plugin constructors for static libraries")
	print(" * Created by {0}".format(__file__))
	print(" */")
	print("")
	print("#include <plugins/plugin.h>")
	print("#include <plugins/plugin_loader.h>")
	print("")

	for plugin in plugins:
		print("plugin_t *{0}_plugin_create();".format(plugin.replace('-', '_')))

	print("")
	print("static void register_plugins() __attribute__ ((constructor));")
	print("static void register_plugins()")
	print("{")

	for plugin in plugins:
		print('	plugin_constructor_register("{0}", {1}_plugin_create);'.format(plugin, plugin.replace('-', '_')))

	print("}")

	print("")
	print("static void unregister_plugins() __attribute__ ((destructor));")
	print("static void unregister_plugins()")
	print("{")

	for plugin in plugins:
		print('	plugin_constructor_register("{0}", NULL);'.format(plugin))

	print("}")
	print("")

parser = ArgumentParser(description = "Generate constructor registration for a list of plugins")
parser.add_argument('plugins', metavar="plugin", nargs="*",
					help = "name of a plugin for which to generate constructor registration")


args = parser.parse_args()
generate_output(args.plugins);
