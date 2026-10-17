.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: button (command)

button command
==============

Synopsis
--------

::

    button list
    button <name>

Description
-----------

The button command is used to retrieve the status of a button. To show the
status of a button with name 'button1' you would issue the command

::

    button button1

The status of the button is both written to the console as *ON* or *OFF* and
set in the return value variable *$?* as 0 (true) or 1 (false). To retrieve
the status of a button with name *button1* and to write it to environment
variable *status1* you would execute the commands

::

    button button1
    setenv status1 $?

A list of all available buttons and their status can be displayed using

::

    button list

If a button device has not been probed yet, its status will be shown as
*<inactive>* in the list.

Configuration
-------------

To use the button command you must specify CONFIG_CMD_BUTTON=y and enable a
button driver. The available buttons are defined in the device-tree.

Return value
------------

The variable *$?* takes the following values

+---+-----------------------------+
| 0 | ON, the button is pressed   |
+---+-----------------------------+
| 1 | OFF, the button is released |
+---+-----------------------------+
| 0 | button list was shown       |
+---+-----------------------------+
| 1 | button not found            |
+---+-----------------------------+
| 1 | invalid arguments           |
+---+-----------------------------+
