.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: bootd (command)

bootd command
=============

Synopsis
--------

::

    bootd

Description
-----------

The bootd command executes the command stored in the environment variable
*bootcmd*, i.e. it does the same thing as *run bootcmd*.

Example
-------

::

    => setenv bootcmd 'echo Hello World'
    => bootd
    Hello World
    => setenv bootcmd true
    => bootd; echo $?
    0
    => setenv bootcmd false
    => bootd; echo $?
    1

Return value
------------

The return value $? of the bootd command is the return value of the command in
the environment variable *bootcmd*.
