.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Copyright 2011 The Chromium OS Authors

Devicetree Control in U-Boot
============================

This feature provides for run-time configuration of U-Boot via a flattened
devicetree (fdt).

This feature aims to make it possible for a single U-Boot binary to support
multiple boards, with the exact configuration of each board controlled by
a flattened devicetree (fdt). This is the approach  taken by Linux kernel for
ARM and RISC-V and has been used by PowerPC for some time.

The fdt is a convenient vehicle for implementing run-time configuration
for three reasons:

- There is already excellent infrastructure for the fdt: a compiler checks
  the text file and converts it to a compact binary format, and a library
  is already available in U-Boot (libfdt) for handling this format
- It is extensible since it consists of nodes and properties in a nice
  hierarchical format
- It is fairly efficient to read incrementally

The arch/<arch>/dts directories contains a Makefile for building the devicetree
blob and embedding it in the U-Boot image. This is useful since it allows
U-Boot to configure itself according to what it finds there. If you have
a number of similar boards with different peripherals, you can describe
the features of each board in the devicetree file, and have a single
generic source base.

To enable this feature, add CONFIG_OF_CONTROL to your board config file.


What is a Flattened Devicetree?
-------------------------------

An fdt can be specified in source format as a text file. To read about
the fdt syntax, take a look at the specification (dtspec_).

There is also a mailing list (dtlist_) for the compiler and associated
tools.

In case you are wondering, OF stands for Open Firmware. This follows the
convention used in Linux.


Tools
-----

To create flattened device trees the device tree compiler is used. This is
provided by U-Boot automatically. If you have a system version of dtc
(typically in the 'device-tree-compiler' package), that system version is
currently not used.

If you want to build your own dtc, it is kept here::

    git://git.kernel.org/pub/scm/utils/dtc/dtc.git

You can decode a binary file with::

    dtc -I dtb -O dts <filename.dtb>

That repo also includes `fdtget`/`fdtput` for reading and writing properties in
a binary file. U-Boot adds its own `fdtgrep` for creating subsets of the file.


Where do I get a devicetree file for my board?
----------------------------------------------

You may find that the Linux kernel has a suitable file. Look in the
kernel source in arch/<arch>/boot/dts.

If not you might find other boards with suitable files that you can
modify to your needs. Look in the board directories for files with a
.dts extension.

Failing that, you could write one from scratch yourself!


Configuration
-------------

Use::

   #define CONFIG_DEFAULT_DEVICE_TREE	"<name>"

to set the filename of the devicetree source. Then put your devicetree
file into::

   arch/<arch>/dts/<name>.dts

This should include your CPU or SOC's devicetree file, placed in
`arch/<arch>/dts`, and then make any adjustments required using a u-boot-dtsi
file for your board.

If CONFIG_OF_EMBED is defined, then it will be picked up and built into
the U-Boot image (including u-boot.bin). This is suitable for debugging
and development only and is not recommended for production devices.

If CONFIG_OF_SEPARATE is defined, then it will be built and placed in
a u-boot.dtb file alongside u-boot-nodtb.bin with the combined result placed
in u-boot.bin so you can still just flash u-boot.bin onto your board. If you are
using CONFIG_SPL_FRAMEWORK, then u-boot.img will be built to include the device
tree binary.

If CONFIG_OF_BOARD is defined, a board-specific routine will provide the
devicetree at runtime, for example if an earlier bootloader stage creates
it and passes it to U-Boot.

If CONFIG_BLOBLIST is defined, the devicetree may come from a bloblist passed
from a previous stage, if present.

If CONFIG_SANDBOX is defined, then it will be read from a file on
startup. Use the -d flag to U-Boot to specify the file to read, -D for the
default and -T for the test devicetree, used to run sandbox unit tests.

You cannot use more than one of these options at the same time.

To use a devicetree file that you have compiled yourself, pass
EXT_DTB=<filename> to 'make', as in::

   make EXT_DTB=boot/am335x-boneblack-pubkey.dtb

Then U-Boot will copy that file to u-boot.dtb, put it in the .img file
if used, and u-boot-dtb.bin.

If you wish to put the fdt at a different address in memory, you can
define the "fdtcontroladdr" environment variable. This is the hex
address of the fdt binary blob, and will override either of the options.
Be aware that this environment variable is checked prior to relocation,
when only the compiled-in environment is available. Therefore it is not
possible to define this variable in the saved SPI/NAND flash
environment, for example (it will be ignored). After relocation, this
variable will be set to the address of the newly relocated fdt blob.
It is read-only and cannot be changed. It can optionally be used to
control the boot process of Linux with bootm/bootz commands.

To use this, put something like this in your board header file::

   #define CFG_EXTRA_ENV_SETTINGS	"fdtcontroladdr=10000\0"

Build:

After the board configuration is done, fdt supported u-boot can be built in two
ways:

#  build the default dts which is defined from CONFIG_DEFAULT_DEVICE_TREE::

    $ make

#  build the user specified dts file::

    $ make DEVICE_TREE=<dts-file-name>


.. _dttweaks:

Adding tweaks for U-Boot
------------------------

It is strongly recommended that devicetree files in U-Boot are an exact copy of
those in Linux, so that it is easy to sync them up from time to time.

U-Boot is of course a very different project from Linux, e.g. it operates under
much more restrictive memory and code-size constraints. Where Linux may use a
full clock driver with Common Clock Format (CCF) to find the input clock to the
UART, U-Boot typically wants to output a banner as early as possible before too
much code has run.

A second difference is that U-Boot includes different phases. For SPL,
constraints are even more extreme and the devicetree is shrunk to remove
unwanted nodes, or even turned into C code to avoid access overhead.

U-Boot automatically looks for and includes a file with updates to the standard
devicetree for your board, searching for them in the same directory as the
main file, in this order::

   <orig_filename>-u-boot.dtsi
   <CONFIG_SYS_SOC>-u-boot.dtsi
   <CONFIG_SYS_CPU>-u-boot.dtsi
   <CONFIG_SYS_VENDOR>-u-boot.dtsi
   u-boot.dtsi

Only one of these is selected but of course you can #include another one within
that file, to create a hierarchy of shared files.


External .dtsi fragments
------------------------

Apart from describing the hardware present, U-Boot also uses its
control dtb for various configuration purposes. For example, the
public key(s) used for Verified Boot are embedded in a specific format
in a /signature node.

As mentioned above, the U-Boot build system automatically includes a
`*-u-boot.dtsi` file, if found, containing U-Boot specific
quirks. However, some data, such as the mentioned public keys, are not
appropriate for upstream U-Boot but are better kept and maintained
outside the U-Boot repository. You can use CONFIG_DEVICE_TREE_INCLUDES
to specify a list of .dtsi files that will also be included when
building .dtb files.


Relocation, SPL and TPL
-----------------------

U-Boot can be divided into three phases: TPL, SPL and U-Boot proper.

The full devicetree is available to U-Boot proper, but normally only a subset
(or none at all) is available to TPL and SPL. See 'Pre-Relocation Support' and
'SPL Support' in doc/driver-model/design.rst for more details.


Using several DTBs in the SPL (CONFIG_SPL_MULTI_DTB)
----------------------------------------------------
In some rare cases it is desirable to let SPL be able to select one DTB among
many. This usually not very useful as the DTB for the SPL is small and usually
fits several platforms. However the DTB sometimes include information that do
work on several platforms (like IO tuning parameters).
In this case it is possible to use CONFIG_SPL_MULTI_DTB. This option appends to
the SPL a FIT image containing several DTBs listed in SPL_OF_LIST.
board_fit_config_name_match() is called to select the right DTB.

If board_fit_config_name_match() relies on DM (DM driver to access an EEPROM
containing the board ID for example), it possible to start with a generic DTB
and then switch over to the right DTB after the detection. For this purpose,
the platform code must call fdtdec_resetup(). Based on the returned flag, the
platform may have to re-initialise the DM subsystem using dm_uninit() and
dm_init_and_scan().


Limitations
-----------

Devicetrees can help reduce the complexity of supporting variants of boards
which use the same SOC / CPU.

However U-Boot is designed to build for a single architecture type and CPU
type. So for example it is not possible to build a single ARM binary
which runs on your AT91 and OMAP boards, relying on an fdt to configure
the various features. This is because you must select one of
the CPU families within arch/arm/cpu/arm926ejs (omap or at91) at build
time. Similarly U-Boot cannot be built for multiple cpu types or
architectures.

It is important to understand that the fdt only selects options
available in the platform / drivers. It cannot add new drivers (yet). So
you must still have the CONFIG option to enable the driver. For example,
you need to define CONFIG_SYS_NS16550 to bring in the NS16550 driver,
but can use the fdt to specific the UART clock, peripheral address, etc.
In very broad terms, the CONFIG options in general control *what* driver
files are pulled in, and the fdt controls *how* those files work.

History
-------

U-Boot configuration was previous done using CONFIG options in the board
config file. This eventually got out of hand with nearly 10,000 options.

U-Boot adopted devicetrees around the same time as Linux and early boards
used it before Linux (e.g. snow). The two projects developed in parallel
and there are still some differences in the bindings for certain boards.
While there has been discussion of having a separate repository for devicetree
files, in practice the Linux kernel Git repository has become the place where
these are stored, with U-Boot taking copies and adding tweaks with u-boot.dtsi
files.

.. _dtspec: https://www.devicetree.org/specifications/
.. _dtlist: https://www.spinics.net/lists/devicetree-compiler/
