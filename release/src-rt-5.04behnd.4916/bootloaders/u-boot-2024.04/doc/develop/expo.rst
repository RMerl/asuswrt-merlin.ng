.. SPDX-License-Identifier: GPL-2.0+

Expo menu
=========

U-Boot provides a menu implementation for use with selecting bootflows and
changing U-Boot settings. This is in early stages of development.

Motivation
----------

U-Boot already has a text-based menu system accessed via the
:doc:`../usage/cmd/bootmenu`. This works using environment variables, or via
some EFI-specific hacks.

The command makes use of a lower-level `menu` implementation, which is quite
flexible and can be used to make menu hierarchies.

However this system is not flexible enough for use with standard boot. It does
not support a graphical user interface and cannot currently support anything
more than a very simple list of items. While it does support multiple menus in
hierarchies, these are implemented by the caller. See for example `eficonfig.c`.

Another challenge with the current menu implementation is that it controls
the event loop, such that bootmenu_loop() does not return until a key is
pressed. This makes it difficult to implement dynamic displays or to do other
things while the menu is running, such as searching for more bootflows.

For these reasons an attempt has been made to develop a more flexible system
which can handle menus as well as other elements. This is called 'expo', short
for exposition, in an attempt to avoid common words like display, screen, menu
and the like. The primary goal is to support Verified Boot for Embedded (VBE),
although it is available to any boot method, using the 'bootflow menu' command.

Efforts have been made to use common code with the existing menu, including
key processing in particular.

Previous work looked at integrating Nuklear into U-Boot. This works fine and
could provide a way to provide a more flexible UI, perhaps with expo dealing
with the interface to Nuklear. But this is quite a big step and it may be years
before this becomes desirable, if at all. For now, U-Boot only needs a fairly
simple set of menus and options, so rendering them directly is fairly
straightforward.

Concepts
--------

The creator of the expo is here called a `controller` and it controls most
aspects of the expo. This is the code that you must write to use expo.

An `expo` is a set of scenes which can be presented to the user one at a time,
to show information and obtain input from the user.

A `scene` is a collection of objects which are displayed together on the screen.
Only one scene is visible at a time and scenes do not share objects.

A `scene object` is something that appears in the scene, such as some text, an
image or a menu. Objects can be positioned and hidden.

A `menu object` contains a title, a set of `menu items` and a pointer to the
current item. Menu items consist of a keypress (indicating what to press to
select the item), label and description. All three are shown in a single line
within the menu. Items can also have a preview image, which is shown when the
item is highlighted.

A `textline object` contains a label and an editable string.

All components have a name. This is mostly for debugging, so it is easy to see
what object is referred to, although the name is also used for saving values.
Of course the ID numbers can help as well, but they are less easy to
distinguish.

While the expo implementation provides support for handling keypresses and
rendering on the display or serial port, it does not actually deal with reading
input from the user, nor what should be done when a particular menu item is
selected. This is deliberate since having the event loop outside the expo is
more flexible, particularly in a single-threaded environment like U-Boot.

Everything within an expo has a unique ID number. This is done so that it is
easy to refer to things after the expo has been created. The expectation is that
the controller declares an enum containing all of the elements in the expo,
passing the ID of each object as it is created. When a menu item is selected,
its ID is returned. When a object's font or position needs to change, the ID is
passed to expo functions to indicate which object it is. It is possible for expo
to auto-allocate IDs, but this is not recommended. The use of IDs is a
convenience, removing the need for the controller to store pointers to objects,
or even the IDs of objects. Programmatic creation of many items in a loop can be
handled by allocating space in the enum for a maximum number of items, then
adding the loop count to the enum values to obtain unique IDs.

Where dynamic IDs are need, use expo_set_dynamic_start() to set the start value,
so that they are allocated above the starting (enum) IDs.

All text strings are stored in a structure attached to the expo, referenced by
a text ID. This makes it easier at some point to implement multiple languages or
to support Unicode strings.

Menu objects do not have their own text and image objects. Instead they simply
refer to objects which have been created. So a menu item is just a collection
of IDs of text and image objects. When adding a menu item you must create these
objects first, then create the menu item, passing in the relevant IDs.

Creating an expo
----------------

To create an expo programmatically, use `expo_new()` followed by `scene_new()`
to create a scene. Then add objects to the scene, using functions like
`scene_txt_str()` and `scene_menu()`. For every menu item, add text and image
objects, then create the menu item with `scene_menuitem()`, referring to those
objects.

To create an expo using a description file, see :ref:`expo_format` below.

Layout
------

Individual objects can be positioned using `scene_obj_set_pos()`. Menu items
cannot be positioned manually: this is done by `scene_arrange()` which is called
automatically when something changes. The menu itself determines the position of
its items.

Rendering
---------

Rendering is performed by calling `expo_render()`. This uses either the
vidconsole, if present, or the serial console in `text mode`. Expo handles
presentation automatically in either case, without any change in how the expo is
created.

For the vidconsole, Truetype fonts can be used if enabled, to enhance the
quality of the display. For text mode, each menu item is shown in a single line,
allowing easy selection using arrow keys.

Input
-----

The controller is responsible for collecting keyboard input. A good way to do
this is to use `cli_ch_process()`, since it handles conversion of escape
sequences into keys. However, expo has some special menu-key codes for
navigating the interface. These are defined in `enum bootmenu_key` and include
`BKEY_UP` for moving up and `BKEY_SELECT` for selecting an item. You can use
`bootmenu_conv_key()` to convert an ASCII key into one of these, but if it
returns a value >= `BKEY_FIRST_EXTRA` then you should pass the unmodified ASCII
key to the expo, since it may be used by textline objects.

Once a keypress is decoded, call `expo_send_key()` to send it to the expo. This
may cause an update to the expo state and may produce an action.

Actions
-------

Call `expo_action_get()` in the event loop to check for any actions that the
expo wants to report. These can include selecting a particular menu item, or
quitting the menu. Processing of these is the responsibility of your controller.

Event loop
----------

Expo is intended to be used in an event loop. For an example loop, see
`bootflow_menu_run()`. It is possible to perform other work in your event loop,
such as scanning devices for more bootflows.

Themes
------

Expo supports simple themes, for setting the font size, for example. Use the
expo_apply_theme() function to load a theme, passing a node with the required
properties:

font-size
    Font size to use for all text (type: u32)

menu-inset
    Number of pixels to inset the menu on the sides and top (type: u32)

menuitem-gap-y
    Number of pixels between menu items

Pop-up mode
-----------

Expos support two modes. The simple mode is used for selecting from a single
menu, e.g. when choosing with OS to boot. In this mode the menu items are shown
in a list (label, > pointer, key and description) and can be chosen using arrow
keys and enter::

   U-Boot Boot Menu

   UP and DOWN to choose, ENTER to select

   mmc1           > 0  Fedora-Workstation-armhfp-31-1.9
   mmc3             1  Armbian

The popup mode allows multiple menus to be present in a scene. Each is shown
just as its title and label, as with the `CPU Speed` and `AC Power` menus here::

              Test Configuration


   CPU Speed        <2 GHz>  (highlighted)

   AC Power         Always Off


     UP and DOWN to choose, ENTER to select


.. _expo_format:

Expo Format
-----------

It can be tedious to create a complex expo using code. Expo supports a
data-driven approach, where the expo description is in a devicetree file. This
makes it easier and faster to create and edit the description. An expo builder
is provided to convert this format into an expo structure.

Layout of the expo scenes is handled automatically, based on a set of simple
rules. The :doc:`../usage/cmd/cedit` can be used to load a configuration
and create an expo from it.

Top-level node
~~~~~~~~~~~~~~

The top-level node has the following properties:

dynamic-start
    type: u32, optional

    Specifies the start of the dynamically allocated objects. This results in
    a call to expo_set_dynamic_start().

The top-level node has the following subnodes:

scenes
    Specifies the scenes in the expo, each one being a subnode

strings
    Specifies the strings in the expo, each one being a subnode

`scenes` node
~~~~~~~~~~~~~

Contains a list of scene subnodes. The name of each subnode is passed as the
name to `scene_new()`.

`strings` node
~~~~~~~~~~~~~~

Contains a list of string subnodes. The name of each subnode is ignored.

`strings` subnodes
~~~~~~~~~~~~~~~~~~

Each subnode defines a string which can be used by scenes and objects. Each
string has an ID number which is used to refer to it.

The `strings` subnodes have the following properties:

id
    type: u32, required

    Specifies the ID number for the string.

value:
    type: string, required

    Specifies the string text. For now only a single value is supported. Future
    work may add support for multiple languages by using a value for each
    language.

Scene nodes (`scenes` subnodes)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Each subnode of the `scenes` node contains a scene description.

Most properties can use either a string or a string ID. For example, a `title`
property can be used to provide the title for a menu; alternatively a `title-id`
property can provide the string ID of the title. If both are present, the
ID takes preference, except that if a string with that ID does not exist, it
falls back to using the string from the property (`title` in this example). The
description below shows these are alternative properties with the same
description.

The scene nodes have the following properties:

id
    type: u32, required

    Specifies the ID number for the string.

title / title-id
    type: string / u32, required

    Specifies the title of the scene. This is shown at the top of the scene.

prompt / prompt-id
    type: string / u32, required

    Specifies a prompt for the scene. This is shown at the bottom of the scene.

The scene nodes have a subnode for each object in the scene.

Object nodes
~~~~~~~~~~~~

The object-node name is used as the name of the object, e.g. when calling
`scene_menu()` to create a menu.

Object nodes have the following common properties:

type
    type: string, required

    Specifies the type of the object. Valid types are:

    "menu"
        Menu containing items which can be selected by the user

    "textline"
        A line of text which can be edited

id
    type: u32, required

    Specifies the ID of the object. This is used when referring to the object.

Where CMOS RAM is used for reading and writing settings, the following
additional properties are required:

start-bit
    Specifies the first bit in the CMOS RAM to use for this setting. For a RAM
    with 0x100 bytes, there are 0x800 bit locations. For example, register 0x80
    holds bits 0x400 to 0x407.

bit-length
    Specifies the number of CMOS RAM bits to use for this setting. The bits
    extend from `start-bit` to `start-bit + bit-length - 1`. Note that the bits
    must be contiguous.

Menu nodes have the following additional properties:

title / title-id
    type: string / u32, required

    Specifies the title of the menu. This is shown to the left of the area for
    this menu.

item-id
    type: u32 list, required

    Specifies the ID for each menu item. These are used for checking which item
    has been selected.

item-label / item-label-id
    type: string list / u32 list, required

    Specifies the label for each item in the menu. These are shown to the user.
    In 'popup' mode these form the items in the menu.

key-label / key-label-id
    type: string list / u32 list, optional

    Specifies the key for each item in the menu. These are currently only
    intended for use in simple mode.

desc-label / desc-label-id
    type: string list / u32 list, optional

    Specifies the description for each item in the menu. These are currently
    only intended for use in simple mode.

Textline nodes have the following additional properties:

label / label-id
    type: string / u32, required

    Specifies the label of the textline. This is shown to the left of the area
    for this textline.

edit-id
    type: u32, required

    Specifies the ID of the of the editable text object. This can be used to
    obtain the text from the textline

max-chars:
    type: u32, required

    Specifies the maximum number of characters permitted to be in the textline.
    The user will be prevented from adding more.


Expo layout
~~~~~~~~~~~

The `expo_arrange()` function can be called to arrange the expo objects in a
suitable manner. For each scene it puts the title at the top, the prompt at the
bottom and the objects in order from top to bottom.


.. _expo_example:

Expo format example
~~~~~~~~~~~~~~~~~~~

This example shows an expo with a single scene consisting of two menus. The
scene title is specified using a string from the strings table, but all other
strings are provided inline in the nodes where they are used.

::

    /* this comment is parsed by the expo.py tool to insert the values below

    enum {
        ZERO,
        ID_PROMPT,
        ID_SCENE1,
        ID_SCENE1_TITLE,

        ID_CPU_SPEED,
        ID_CPU_SPEED_TITLE,
        ID_CPU_SPEED_1,
        ID_CPU_SPEED_2,
        ID_CPU_SPEED_3,

        ID_POWER_LOSS,
        ID_AC_OFF,
        ID_AC_ON,
        ID_AC_MEMORY,

        ID_MACHINE_NAME,
        ID_MACHINE_NAME_EDIT,

        ID_DYNAMIC_START,
    */

    &cedit {
        dynamic-start = <ID_DYNAMIC_START>;

        scenes {
            main {
                id = <ID_SCENE1>;

                /* value refers to the matching id in /strings */
                title-id = <ID_SCENE1_TITLE>;

                /* simple string is used as it is */
                prompt = "UP and DOWN to choose, ENTER to select";

                /* defines a menu within the scene */
                cpu-speed {
                    type = "menu";
                    id = <ID_CPU_SPEED>;

                    /*
                     * has both string and ID. The string is ignored
                     * if the ID is present and points to a string
                     */
                    title = "CPU speed";
                    title-id = <ID_CPU_SPEED_TITLE>;

                    /* menu items as simple strings */
                    item-label = "2 GHz", "2.5 GHz", "3 GHz";

                    /* IDs for the menu items */
                    item-id = <ID_CPU_SPEED_1 ID_CPU_SPEED_2
                        ID_CPU_SPEED_3>;
                };

                power-loss {
                    type = "menu";
                    id = <ID_POWER_LOSS>;

                    title = "AC Power";
                    item-label = "Always Off", "Always On",
                        "Memory";

                    item-id = <ID_AC_OFF ID_AC_ON ID_AC_MEMORY>;
                };

            machine-name {
                id = <ID_MACHINE_NAME>;
                type = "textline";
                max-chars = <20>;
                title = "Machine name";
                edit-id = <ID_MACHINE_NAME_EDIT>;
            };
        };

        strings {
            title {
                id = <ID_SCENE1_TITLE>;
                value = "Test Configuration";
                value-es = "configuración de prueba";
            };
        };
    };


API documentation
-----------------

.. kernel-doc:: include/expo.h

Future ideas
------------

Some ideas for future work:

- Default menu item and a timeout
- Image formats other than BMP
- Use of ANSI sequences to control a serial terminal
- Colour selection
- Support for more widgets, e.g. numeric, radio/option
- Mouse support
- Integrate Nuklear, NxWidgets or some other library for a richer UI
- Optimise rendering by only updating the display with changes since last render
- Use expo to replace the existing menu implementation
- Add a Kconfig option to drop the names to save code / data space
- Add a Kconfig option to disable vidconsole support to save code / data space
- Support both graphical and text menus at the same time on different devices
- Support unicode
- Support curses for proper serial-terminal menus
- Add support for large menus which need to scroll
- Update expo.py tool to check for overlapping names and CMOS locations

.. Simon Glass <sjg@chromium.org>
.. 7-Oct-22
