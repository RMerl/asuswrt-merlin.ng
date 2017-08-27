Python module for freeradius
Copyright 2002 Miguel A Paraz <mparaz@mparaz.com>
Copyright 2002 Imperium Technology, Inc.

PURPOSE:
To allow module writers to write modules in a high-level language,
for implementation or for prototyping.

REQUIRES:
Python - tested with 2.2

BUILDING:
./configure --with-experimental-modules


USAGE:
Make your module available to the Python interpreter by either putting it
in a standard location, or 'EXPORT PYTHONPATH=$location'.





BUGS:
1. Can't compile statically (./configure --enable-shared=no)  - causes
SIGSEGV on the first malloc() in main().

Design:
1. Support for all module functions.
2. One module per function allowed, for example, from experimental.conf:

	python {
		mod_instantiate = radiusd_test
		func_instantiate = instantiate

		mod_authorize = radiusd_test
		func_authorize = authorize

		mod_accounting = radiusd_test
		func_accounting = accounting

		mod_preacct = radiusd_test
		func_preacct = preacct

		mod_detach = radiusd_test
		func_detach = detach

	}


3. Different functions are wrappers around the same core.
4. func_detach is passed no parameters, returns module return value.
5. If functions returns None (plain 'return' no return), default to RLM_OK
6. Python instantation function can return -1 to signal failure and abort
   startup.

Available to module:
import radiusd
radiusd.rad_log(radiusd.L_XXX, message_string)
radiusd.RLM_XXX



TODO:
1. Do we need to support other pair operations beyond set (:=) ?
2. Should we pass the value pair info as a dict and not a tuple? Faster?
2. Give access to more radiusd variables like the dictionary.
3. Give access to other C functions.
   Let the Python module deal with the structures directly, instead of
   letting our C code do it afterwards.
   What's a good way to represent this?





