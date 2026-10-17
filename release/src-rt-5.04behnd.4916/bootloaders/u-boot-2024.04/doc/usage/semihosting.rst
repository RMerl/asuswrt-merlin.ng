.. SPDX-License-Identifier: GPL-2.0+
.. Copyright 2014 Broadcom Corporation.

Semihosting
===========

Semihosting is ARM's way of having a real or virtual target communicate
with a host or host debugger for basic operations such as file I/O,
console I/O, etc. Please see `Arm's semihosting documentation
<https://developer.arm.com/documentation/100863/latest/>`_ for more
information.

Platform Support
----------------

Versatile Express
^^^^^^^^^^^^^^^^^

For developing on armv8 virtual fastmodel platforms, semihosting is a
valuable tool since it allows access to image/configuration files before
eMMC or other NV media are available.

There are two main ARM virtual Fixed Virtual Platform (FVP) models,
`Versatile Express (VE) FVP and BASE FVP
<http://www.arm.com/products/tools/models/fast-models/foundation-model.php>`_.
The initial vexpress64 U-Boot board created here runs on the VE virtual
platform using the license-free Foundation_v8 simulator. Fortunately,
the Foundation_v8 simulator also supports the BASE_FVP model which
companies can purchase licenses for and contain much more functionality.
So we can, in U-Boot, run either model by either using the VE FVP (default),
or turning on ``CONFIG_BASE_FVP`` for the more full featured model.

Rather than create a new armv8 board similar to ``armltd/vexpress64``, add
semihosting calls to the existing one, enabled with ``CONFIG_SEMIHOSTING``
and ``CONFIG_BASE_FVP`` both set. Also reuse the existing board config file
vexpress_aemv8.h but differentiate the two models by the presence or
absence of ``CONFIG_BASE_FVP``. This change is tested and works on both the
Foundation and Base fastmodel simulators.

QEMU
^^^^

Another ARM emulator which supports semihosting is `QEMU
<https://www.qemu.org/>`_. To enable semihosting, enable
``CONFIG_SERIAL_PROBE_ALL`` when configuring U-Boot, and use
``-semihosting`` when invoking QEMU. Adding ``-nographic`` can also be
helpful. When using a semihosted serial console, QEMU will block waiting
for input. This will cause the GUI to become unresponsive. To mitigate
this, try adding ``-nographic``. For more information about building and
running QEMU, refer to the :doc:`board documentation
<../board/emulation/qemu-arm>`.

OpenOCD
^^^^^^^

Any ARM platform can use semihosting with an attached debugger. One such
debugger with good support for a variety of boards and JTAG adapters is
`OpenOCD <https://openocd.org/>`_. Semihosting is not enabled by default,
so you will need to enable it::

    $ openocd -f <your board config> -c init -c halt -c \
          'arm semihosting enable' -c resume

Note that enabling semihosting can only be done after attaching to the
board with ``init``, and must be done while the CPU is halted. For a more
extended example, refer to the :ref:`LS1046ARDB docs <ls1046ardb_jtag>`.

Loading files
-------------

The semihosting code adds a "semihosting filesystem"::

  load hostfs - <address> <image>

That will load an image from the host filesystem into RAM at the specified
address. If you are using U-Boot SPL, you can also use ``BOOT_DEVICE_SMH``
which will load ``CONFIG_SPL_FS_LOAD_PAYLOAD_NAME``.

Host console
------------

U-Boot can use the host's console instead of a physical serial device by
enabling ``CONFIG_SERIAL_SEMIHOSTING``. If you don't have
``CONFIG_DM_SERIAL`` enabled, make sure you disable any other serial
drivers.

Migrating from ``smhload``
--------------------------

If you were using the ``smhload`` command, you can migrate commands like::

    smhload <file> <address> [<end var>]

to a generic load command like::

    load hostfs - <address> <file>

The ``load`` command will set the ``filesize`` variable with the size of
the file. The ``fdt chosen`` command has been updated to take a size
instead of an end address. If you were adding the initramfs to your device
tree like::

    fdt chosen <address> <end var>

you can now run::

    fdt chosen <address> $filesize
