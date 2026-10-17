.. SPDX-License-Identifier: GPL-2.0+
.. Copyright 2023, Heinrich Schuchardt <heinrich.schuchardt@canonical.com>

.. index::
   single: blkcache (command)

blkcache command
================

Synopsis
--------

::

    blkcache show
    blkcache configure <blocks> <entries>

Description
-----------

The *blkcache* command is used to control the size of the block cache and to
display statistics.

The block cache buffers data read from block devices. This speeds up the access
to file-systems.

show
    show and reset statistics

configure
    set the maximum number of cache entries and the maximum number of blocks per
    entry

blocks
    maximum number of blocks per cache entry. The block size is device specific.
    The initial value is 8.

entries
    maximum number of entries in the cche. The initial value is 32.

Example
-------

.. code-block::

    => blkcache show
    hits: 296
    misses: 149
    entries: 7
    max blocks/entry: 8
    max cache entries: 32
    => blkcache show
    hits: 0
    misses: 0
    entries: 7
    max blocks/entry: 8
    max cache entries: 32
    => blkcache configure 16 64
    changed to max of 64 entries of 16 blocks each
    => blkcache show
    hits: 0
    misses: 0
    entries: 0
    max blocks/entry: 16
    max cache entries: 64
    =>

Configuration
-------------

The blkcache command is only available if CONFIG_CMD_BLOCK_CACHE=y.

Return code
-----------

If the command succeeds, the return code $? is set 0 (true). In case of an
error the return code is set to 1 (false).
