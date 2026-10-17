.. SPDX-License-Identifier: GPL-2.0+

Environment implementation
==========================

See :doc:`../usage/environment` for usage information.

Callback functions for environment variables
--------------------------------------------

For some environment variables, the behavior of u-boot needs to change
when their values are changed.  This functionality allows functions to
be associated with arbitrary variables.  On creation, overwrite, or
deletion, the callback will provide the opportunity for some side
effect to happen or for the change to be rejected.

The callbacks are named and associated with a function using the
U_BOOT_ENV_CALLBACK macro in your board or driver code.

These callbacks are associated with variables in one of two ways.  The
static list can be added to by defining CFG_ENV_CALLBACK_LIST_STATIC
in the board configuration to a string that defines a list of
associations.  The list must be in the following format::

    entry = variable_name[:callback_name]
    list = entry[,list]

If the callback name is not specified, then the callback is deleted.
Spaces are also allowed anywhere in the list.

Callbacks can also be associated by defining the ".callbacks" variable
with the same list format above.  Any association in ".callbacks" will
override any association in the static list. You can define
CONFIG_ENV_CALLBACK_LIST_DEFAULT to a list (string) to define the
".callbacks" environment variable in the default or embedded environment.

If CONFIG_REGEX is defined, the variable_name above is evaluated as a
regular expression. This allows multiple variables to be connected to
the same callback without explicitly listing them all out.

The signature of the callback functions is::

    int callback(const char *name, const char *value, enum env_op op, int flags)

* name - changed environment variable
* value - new value of the environment variable
* op - operation (create, overwrite, or delete)
* flags - attributes of the environment variable change, see flags H_* in
  include/search.h

The return value is 0 if the variable change is accepted and 1 otherwise.
