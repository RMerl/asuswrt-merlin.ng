.. SPDX-License-Identifier: GPL-2.0+ */
.. Copyright (c) 2014 The Chromium OS Authors.
.. sectionauthor:: Simon Glass <sjg@chromium.org>

Sandbox block devices (implementation)
======================================

(See :ref:`sandbox_blk` for operation)

Sandbox block devices are implemented using the `UCLASS_HOST` uclass. Only one
driver is provided (`host_sb_drv`) so all devices in the uclass use the same
driver.

The uclass has a simple API allowing files to be attached and detached.
Attaching a file results in it appearing as a block device in sandbox. This
allows filesystems and whole disk images to be accessed from U-Boot. This is
particularly useful for tests.

Devices are created using `host_create_device()`. This sets up a new
`UCLASS_HOST`.

The device can then be attached to a file with `host_attach_file()`. This
creates the child block device (and bootdev device).

The host device's block device must be probed before use, as normal.

To destroy a device, call host_destroy_device(). This removes the device (and
its children of course), then closes any attached file, then unbinds the device.

There is no arbitrary limit to the number of host devices that can be created.


Uclass API
----------

This is incomplete as it isn't clear how to make Sphinx do the right thing for
struct host_ops. See `include/sandbox_host.h` for full details.

.. kernel-doc:: include/sandbox_host.h
