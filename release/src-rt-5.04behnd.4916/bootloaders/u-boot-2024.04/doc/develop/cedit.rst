.. SPDX-License-Identifier: GPL-2.0+

Configuration Editor
====================

Introduction
------------

U-Boot provides a configuration editor which allows settings to be changed in
a GUI or text environment.


This feature is still in development and has a number of limitations. For
example, cedit only supports menu items (there is no numeric or text entry),
provides no support for colour text and does not support scrolling. Still it is
possible to use it for simple applications.


Overview
--------

The configuration editor makes use of :doc:`expo` to build a description of the
configuration screens and allow user to interact with it.

To create a single-scene cedit for your application:

#. Design the scene, i.e. the objects that need to be present and what their
   possible values are

#. Enter this in .dts format

#. Create a header file containing the IDs

#. Run the 'expo.py' tool to generate a .dtb file containing the layout, which
   can be used by U-Boot

#. Use the :doc:`../usage/cmd/cedit` to create the cedit, read the settings,
   present the cedit to the user and save the settings afterwards.

Each of these is described in a separate section. See :ref:`expo_example` for
an example file.


Design a scene
--------------

Using a piece of paper or a drawing tool, lay out the objects you want in your
scene. Typically you will use the default layout engine, which simply puts items
one after the other from top to bottom. So use a single column and show the
prompt and value for each object.

For menu items, show one of the values, but keep in mind what else you need.


Create an expo-format file
--------------------------

The description is in the form of a devicetree file, as documented at
:ref:`expo_format`. Since everything in an expo has an ID number (an integer
greater than 1) the description is written terms of these IDs. They each have
an enum value. which is typically taken care of by the `expo.py` tool.

The expo should have a `scenes` node with a named scene as a subnode. Within the
scene, add properties for the scene, then a subnode for each object in the
scene.

All object nodes require an `id` value and a `type` property. Other properties
depend on the type. For example, a menu has a `title` and an `item-label` list
proving the text for the menu items, as well as an `item-id` list providing the
ID of each menu item, so it can be selected.

Text properties may have two variants. For example `title` specifies the title
of a menu, but you can instead use `title-id` to specify the string ID to use as
the title. String are defined in a separate area, common to the whole expo,
which contains a subnode for each string. Within that subnode are the ID and the
`value` (i.e. the text). For now only English is supported, but in future it may
be possible to append a language identifier to provide other values (e.g.
'value-es' for Spanish).


Create an ID header-file
------------------------

Expo needs to know the integer value to use for every ID referenced in your
expo-format file. For example, if you have defined a `cpu-speed` node with an
id of `ID_CPU_SPEED`, then Expo needs to know the value of `ID_CPU_SPEED`.

When you write C code to use the expo, you may need to know the IDs. For
example, to find which value the user selected in `cpu-speed` menu, you must
use the `ID_CPU_SPEED` ID. The ID is the only way to refer to anything in Expo.

Since we need a shared set of IDs, it is best to have a header file containing
them. Expo supports doing this with an enum, where every ID is listed in the
enum::

    enum {
        ZERO,

        ID_PROMPT,

        ID_SCENE1,
        ID_SCENE1_TITLE,
        ...
    };

The C compiler can parse this directly. The `expo.py` tool parses it for expo.

Create a header file containing every ID mentioned in your expo. Try to group
related things together.


Build the expo layout
---------------------

Use the `expo.py` tool to build a .dtb for your expo::

    ./tools/expo.py -e expo_ids.h -l expo_layout.dts -o expo.dtb

This uses the enum in the provided header file to get the ID numbers, grabs
the `.dts` file, inserts the ID numbers and then uses the devicetree compiler to
build a `.dtb` file.

If you get an error::

    Devicetree compiler error:
    Error: <stdin>:9.19-20 syntax error
    FATAL ERROR: Unable to parse input tree

that means that something is wrong with your syntax, or perhaps you have an ID
in the `.dts` file that is not mentioned in your enum. Check both files and try
again.


Use the command interface
-------------------------

See the :doc:`../usage/cmd/cedit` command for information on available commands.
Typically you will use `cedit load` to load the `.dtb` file and `cedit run` to
let the user interact with it.


Multiple scenes
---------------

Expo supports multiple scenes but has no pre-determined way of moving between
them. You could use selection of a menu item as a signal to change the scene,
but this is not currently implemented in the cedit code (see `cedit_run()`).


Themes
------

The configuration editor uses simple expo themes. The theme is read from
`/bootstd/cedit-theme` in the devicetree.


Reading and writing settings
----------------------------

Cedit provides several options for persistent settings:

- Writing an FDT file to a filesystem
- Writing to U-Boot's environment variables, which are then typically stored in
  a persistent manner
- Writing to CMOS RAM registers (common on x86 machines). Note that textline
  objects do not appear in CMOS RAM registers

For now, reading and writing settings is not automatic. See the
:doc:`../usage/cmd/cedit` for how to do this on the command line or in a
script.
