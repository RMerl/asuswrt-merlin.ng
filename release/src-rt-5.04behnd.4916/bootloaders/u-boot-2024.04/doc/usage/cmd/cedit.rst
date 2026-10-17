.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: cedit (command)

cedit command
=============

Synopsis
--------

::

    cedit load <interface> <dev[:part]> <filename>
    cedit run
    cedit write_fdt <dev[:part]> <filename>
    cedit read_fdt <dev[:part]> <filename>
    cedit write_env [-v]
    cedit read_env [-v]
    cedit write_cmos [-v] [dev]

Description
-----------

The *cedit* command is used to load a configuration-editor description and allow
the user to interact with it.

It makes use of the expo subsystem.

The description is in the form of a devicetree file, as documented at
:ref:`expo_format`.

See :doc:`../../develop/cedit` for information about the configuration editor.

cedit load
~~~~~~~~~~

Loads a configuration-editor description from a file. It creates a new cedit
structure ready for use. Initially no settings are read, so default values are
used for each object.

cedit run
~~~~~~~~~

Runs the default configuration-editor event loop. This is very simple, just
accepting character input and moving through the objects under user control.
The implementation is at `cedit_run()`.

cedit write_fdt
~~~~~~~~~~~~~~~

Writes the current user settings to a devicetree file. For each menu item the
selected ID and its text string are written.

cedit read_fdt
~~~~~~~~~~~~~~

Reads the user settings from a devicetree file and updates the cedit with those
settings.

cedit read_env
~~~~~~~~~~~~~~

Reads the settings from the environment variables. For each menu item `<name>`,
cedit looks for a variable called `c.<name>` with the ID of the selected menu
item.

The `-v` flag enables verbose mode, where each variable is printed after it is
read.

cedit write_env
~~~~~~~~~~~~~~~

Writes the settings to environment variables. For each menu item the selected
ID and its text string are written, similar to:

   setenv c.<name> <selected_id>
   setenv c.<name>-str <selected_id's text string>

The `-v` flag enables verbose mode, where each variable is printed before it is
set.

cedit write_cmos
~~~~~~~~~~~~~~~~

Writes the settings to locations in the CMOS RAM. The locations used are
specified by the schema. See `expo_format_`.

The `-v` flag enables verbose mode, which shows which CMOS locations were
updated.

Normally the first RTC device is used to hold the data. You can specify a
different device by name using the `dev` parameter.


Example
-------

::

    => cedit load hostfs - fred.dtb
    => cedit run
    => cedit write_fdt hostfs - settings.dtb

That results in::

    / {
        cedit-values {
            cpu-speed = <0x00000006>;
            cpu-speed-str = "2 GHz";
            power-loss = <0x0000000a>;
            power-loss-str = "Always Off";
        };
    }

    => cedit read_fdt hostfs - settings.dtb

This shows settings being stored in the environment::

    => cedit write_env -v
    c.cpu-speed=7
    c.cpu-speed-str=2.5 GHz
    c.power-loss=12
    c.power-loss-str=Memory
    => print
    ...
    c.cpu-speed=6
    c.cpu-speed-str=2 GHz
    c.power-loss=10
    c.power-loss-str=Always Off
    ...

    => cedit read_env -v
    c.cpu-speed=7
    c.power-loss=12

This shows writing to CMOS RAM. Notice that the bytes at 80 and 84 change::

    => rtc read 80 8
    00000080: 00 00 00 00 00 2f 2a 08                          ...../*.
    =>  cedit write_cmos -v
    Write 2 bytes from offset 80 to 84
    => rtc read 80 8
    00000080: 01 00 00 00 08 2f 2a 08                          ...../*.
    => cedit read_cmos -v
    Read 2 bytes from offset 80 to 84

Here is an example with the device specified::

    => cedit write_cmos rtc@43
    =>
