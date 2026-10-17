.. SPDX-License-Identifier: GPL-2.0+
.. Copyright 2022, Heinrich Schuchardt <xypron.glpk@gmx.de>

.. index::
   single: sound (command)

sound command
=============

Synopsis
--------

::

    sound init
    sound play [[len freq] ...] [len [freq]]

Description
-----------

The *sound* command is used to play one or multiple beep sounds.

sound init
    initializes the sound driver.

sound play
    plays a square wave sound. It does not depend on previously calling
    *sound init*.

len
    duration of the sound in ms, defaults to 1000 ms

freq
    frequency of the sound in Hz, defaults to 400 Hz

Examples
--------

Beep at 400 Hz for 1000 ms::

    sound play

Beep at 400 Hz for 600 ms::

    sound play 600

Beep at 500 Hz for 600 ms::

    sound play 600 500

Play melody::

    sound play 500 1047 500 880 500 0 500 1047 500 880 500 0 500 784 500 698 500 784 1000 698

Configuration
-------------

The sound command is enabled by CONFIG_CMD_SOUND=y.

Return value
------------

The return value $? is 0 (true) if the command succeeds, 1 (false) otherwise.
