.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: event (command)

event command
=============

Synopsis
--------

::

    event list

Description
-----------

The event command provides spy list.

This shows the following information:

Seq
    Sequence number of the spy, numbered from 0

Type
    Type of the spy, both as a number and a label. If `CONFIG_EVENT_DEBUG` is
    not enabled, the label just shows `(unknown)`.

Function
    Address of the function to call

ID
    ID string for this event, if `CONFIG_EVENT_DEBUG` is enabled. Otherwise this
    just shows `?`.


See :doc:`../../develop/event` for more information on events.

Example
-------

::

    => event list
    Seq  Type                              Function  ID
      0  7   misc_init_f               55a070517c68  ?

Configuration
-------------

The event command is only available if CONFIG_CMD_EVENT=y.
