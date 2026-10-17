.. SPDX-License-Identifier: GPL-2.0-or-later:

.. index::
   single: pause (command)

pause command
=============

Synopsis
--------

::

    pause [prompt]


Description
-----------

The pause command delays execution waiting for any user input.

It can accept a single parameter to change the prompt message.

Examples
--------

Using with the default prompt:

::

    => pause
    Press any key to continue...


Using with a custom prompt:

::

    => pause 'Prompt for pause...'
    Prompt for pause...

Note that complex prompts require proper quoting:

::

    => pause Prompt for pause...
    pause - delay until user input
    
    Usage:
    pause [prompt] - Wait until users presses any key. [prompt] can be used to customize the message.

Return value
------------

The return value $? is always set to 0 (true), unless invoked in an invalid
manner.
