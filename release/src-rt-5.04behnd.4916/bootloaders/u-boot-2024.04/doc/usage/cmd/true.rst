.. index::
   single: true (command)

true command
============

Synopsis
--------

::

    true

Description
-----------

The true command sets the return value $? to 0 (true).

Example
-------

::

    => true; echo $?
    0
    =>

Configuration
-------------

The true command is only available if CONFIG_HUSH_PARSER=y.
