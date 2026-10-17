.. index::
   single: false (command)

false command
=============

Synopsis
--------

::

    false

Description
-----------

The false command sets the return value $? to 1 (false).

Example
-------

::

    => false; echo $?
    1
    =>

Configuration
-------------

The false command is only available if CONFIG_HUSH_PARSER=y.
