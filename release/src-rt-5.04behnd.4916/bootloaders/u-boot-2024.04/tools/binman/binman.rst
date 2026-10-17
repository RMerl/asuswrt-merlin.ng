.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (c) 2016 Google, Inc

Introduction
============

Firmware often consists of several components which must be packaged together.
For example, we may have SPL, U-Boot, a device tree and an environment area
grouped together and placed in MMC flash. When the system starts, it must be
able to find these pieces.

Building firmware should be separate from packaging it. Many of the complexities
of modern firmware build systems come from trying to do both at once. With
binman, you build all the pieces that are needed, using whatever assortment of
projects and build systems are needed, then use binman to stitch everything
together.


What it does
------------

Binman reads your board's device tree and finds a node which describes the
required image layout. It uses this to work out what to place where.

Binman provides a mechanism for building images, from simple SPL + U-Boot
combinations, to more complex arrangements with many parts. It also allows
users to inspect images, extract and replace binaries within them, repacking if
needed.


Features
--------

Apart from basic padding, alignment and positioning features, Binman supports
hierarchical images, compression, hashing and dealing with the binary blobs
which are a sad trend in open-source firmware at present.

Executable binaries can access the location of other binaries in an image by
using special linker symbols (zero-overhead but somewhat limited) or by reading
the devicetree description of the image.

Binman is designed primarily for use with U-Boot and associated binaries such
as ARM Trusted Firmware, but it is suitable for use with other projects, such
as Zephyr. Binman also provides facilities useful in Chromium OS, such as CBFS,
vblocks and the like.

Binman provides a way to process binaries before they are included, by adding a
Python plug-in.

Binman is intended for use with U-Boot but is designed to be general enough
to be useful in other image-packaging situations.


Motivation
----------

As mentioned above, packaging of firmware is quite a different task from
building the various parts. In many cases the various binaries which go into
the image come from separate build systems. For example, ARM Trusted Firmware
is used on ARMv8 devices but is not built in the U-Boot tree. If a Linux kernel
is included in the firmware image, it is built elsewhere.

It is of course possible to add more and more build rules to the U-Boot
build system to cover these cases. It can shell out to other Makefiles and
build scripts. But it seems better to create a clear divide between building
software and packaging it.

At present this is handled by manual instructions, different for each board,
on how to create images that will boot. By turning these instructions into a
standard format, we can support making valid images for any board without
manual effort, lots of READMEs, etc.

Benefits:

  - Each binary can have its own build system and tool chain without creating
    any dependencies between them
  - Avoids the need for a single-shot build: individual parts can be updated
    and brought in as needed
  - Provides for a standard image description available in the build and at
    run-time
  - SoC-specific image-signing tools can be accommodated
  - Avoids cluttering the U-Boot build system with image-building code
  - The image description is automatically available at run-time in U-Boot,
    SPL. It can be made available to other software also
  - The image description is easily readable (it's a text file in device-tree
    format) and permits flexible packing of binaries


Terminology
-----------

Binman uses the following terms:

- image - an output file containing a firmware image
- binary - an input binary that goes into the image


Installation
------------

You can install binman using::

   pip install binary-manager

The name is chosen since binman conflicts with an existing package.

If you are using binman within the U-Boot tree, it may be easiest to add a
symlink from your local `~/.bin` directory to `/path/to/tools/binman/binman`.


Relationship to FIT
-------------------

FIT is U-Boot's official image format. It supports multiple binaries with
load / execution addresses, compression. It also supports verification
through hashing and RSA signatures.

FIT was originally designed to support booting a Linux kernel (with an
optional ramdisk) and device tree chosen from various options in the FIT.
Now that U-Boot supports configuration via device tree, it is possible to
load U-Boot from a FIT, with the device tree chosen by SPL.

Binman considers FIT to be one of the binaries it can place in the image.

Where possible it is best to put as much as possible in the FIT, with binman
used to deal with cases not covered by FIT. Examples include initial
execution (since FIT itself does not have an executable header) and dealing
with device boundaries, such as the read-only/read-write separation in SPI
flash.

For U-Boot, binman should not be used to create ad-hoc images in place of
FIT.

Note that binman can itself create a FIT. This helps to move mkimage
invocations out of the Makefile and into binman image descriptions. It also
helps by removing the need for ad-hoc tools like `make_fit_atf.py`.


Relationship to mkimage
-----------------------

The mkimage tool provides a means to create a FIT. Traditionally it has
needed an image description file: a device tree, like binman, but in a
different format. More recently it has started to support a '-f auto' mode
which can generate that automatically.

More relevant to binman, mkimage also permits creation of many SoC-specific
image types. These can be listed by running 'mkimage -T list'. Examples
include 'rksd', the Rockchip SD/MMC boot format. The mkimage tool is often
called from the U-Boot build system for this reason.

Binman considers the output files created by mkimage to be binary blobs
which it can place in an image. Binman does not replace the mkimage tool or
this purpose. It would be possible in some situations to create a new entry
type for the images in mkimage, but this would not add functionality. It
seems better to use the mkimage tool to generate binaries and avoid blurring
the boundaries between building input files (mkimage) and packaging then
into a final image (binman).

Note that binman can itself invoke mkimage. This helps to move mkimage
invocations out of the Makefile and into binman image descriptions.


Using binman
============

Example use of binman in U-Boot
-------------------------------

Binman aims to replace some of the ad-hoc image creation in the U-Boot
build system.

Consider sunxi. It has the following steps:

  #. It uses a custom mksunxiboot tool to build an SPL image called
     sunxi-spl.bin. This should probably move into mkimage.

  #. It uses mkimage to package U-Boot into a legacy image file (so that it can
     hold the load and execution address) called u-boot.img.

  #. It builds a final output image called u-boot-sunxi-with-spl.bin which
     consists of sunxi-spl.bin, some padding and u-boot.img.

Binman is intended to replace the last step. The U-Boot build system builds
u-boot.bin and sunxi-spl.bin. Binman can then take over creation of
sunxi-spl.bin by calling mksunxiboot or mkimage. In any case, it would then
create the image from the component parts.

This simplifies the U-Boot Makefile somewhat, since various pieces of logic
can be replaced by a call to binman.


Invoking binman within U-Boot
-----------------------------

Within U-Boot, binman is invoked by the build system, i.e. when you type 'make'
or use buildman to build U-Boot. There is no need to run binman independently
during development. Everything happens automatically and is set up for your
SoC or board so that binman produced the right things.

The general policy is that the Makefile builds all the binaries in INPUTS-y
(the 'inputs' rule), then binman is run to produce the final images (the 'all'
rule).

There should be only one invocation of binman in Makefile, the very last step
that pulls everything together. At present there are some arch-specific
invocations as well, but these should be dropped when those architectures are
converted to use binman properly.

As above, the term 'binary' is used for something in INPUTS-y and 'image' is
used for the things that binman creates. So the binaries are inputs to the
image(s) and it is the image that is actually loaded on the board.

Again, at present, there are a number of things created in Makefile which should
be done by binman (when we get around to it), like `u-boot-ivt.img`,
`lpc32xx-spl.img`, `u-boot-with-nand-spl.imx`, `u-boot-spl-padx4.sfp` and
`u-boot-mtk.bin`, just to pick on a few. When completed this will remove about
400 lines from `Makefile`.

Since binman is invoked only once, it must of course create all the images that
are needed, in that one invocation. It does this by working through the image
descriptions one by one, collecting the input binaries, processing them as
needed and producing the final images.

The same binaries may be used by multiple images. For example binman may be used
to produce an SD-card image and a SPI-flash image. In this case the binaries
going into the process are the same, but binman produces slightly different
images in each case.

For some SoCs, U-Boot is not the only project that produces the necessary
binaries. For example, ARM Trusted Firmware (ATF) is a project that produces
binaries which must be incorporate, such as `bl31.elf` or `bl31.bin`. For this
to work you must have built ATF before you build U-Boot and you must tell U-Boot
where to find the bl31 image, using the BL31 environment variable.

How do you know how to incorporate ATF? It is handled by the atf-bl31 entry type
(etype). An etype is an implementation of reading a binary into binman, in this
case the `bl31.bin` file. When you build U-Boot but do not set the BL31
environment variable, binman provides a help message, which comes from
`missing-blob-help`::

    See the documentation for your board. You may need to build ARM Trusted
    Firmware and build with BL31=/path/to/bl31.bin

The mechanism by which binman is advised of this is also in the Makefile. See
the `-a atf-bl31-path=${BL31}` piece in `cmd_binman`. This tells binman to
set the EntryArg `atf-bl31-path` to the value of the `BL31` environment
variable. Within binman, this EntryArg is picked up by the `Entry_atf_bl31`
etype. An EntryArg is simply an argument to the entry. The `atf-bl31-path`
name is documented in :ref:`etype_atf_bl31`.

Taking this a little further, when binman is used to create a FIT, it supports
using an ELF file, e.g. `bl31.elf` and splitting it into separate pieces (with
`fit,operation = "split-elf"`), each with its own load address.


Invoking binman outside U-Boot
------------------------------

While binman is invoked from within the U-Boot build system, it is also possible
to invoke it separately. This is typically used in a production build system,
where signing is completed (with real keys) and any missing binaries are
provided.

For example, for build testing there is no need to provide a real signature,
nor is there any need to provide a real ATF BL31 binary (for example). These can
be added later by invoking binman again, providing all the required inputs
from the first time, plus any that were missing or placeholders.

So in practice binman is often used twice:

- once within the U-Boot build system, for development and testing
- again outside U-Boot to assembly and final production images

While the same input binaries are used in each case, you will of course you will
need to create your own binman command line, similar to that in `cmd_binman` in
the Makefile. You may find the -I and --toolpath options useful. The
device tree file is provided to binman in binary form, so there is no need to
have access to the original `.dts` sources.


Assembling the image description
--------------------------------

Since binman uses the device tree for its image description, you can use the
same files that describe your board's hardware to describe how the image is
assembled. Typically the images description is in a common file used by all
boards with a particular SoC (e.g. `imx8mp-u-boot.dtsi`).

Where a particular boards needs to make changes, it can override properties in
the SoC file, just as it would for any other device tree property. It can also
add a image that is specific to the board.

Another way to control the image description to make use of CONFIG options in
the description. For example, if the start offset of a particular entry varies
by board, you can add a Kconfig for that and reference it in the description::

    u-boot-spl {
    };

    fit {
        offset = <CONFIG_SPL_PAD_TO>;
        ...
    };

The SoC can provide a default value but boards can override that as needed and
binman will take care of it.

It is even possible to control which entries appear in the image, by using the
C preprocessor::

    #ifdef CONFIG_HAVE_MRC
        intel-mrc {
                offset = <CFG_X86_MRC_ADDR>;
        };
    #endif

Only boards which enable `HAVE_MRC` will include this entry.

Obviously a similar approach can be used to control which images are produced,
with a Kconfig option to enable a SPI image, for example. However there is
generally no harm in producing an image that is not used. If a board uses MMC
but not SPI, but the SoC supports booting from both, then both images can be
produced, with only on or other being used by particular boards. This can help
reduce the need for having multiple defconfig targets for a board where the
only difference is the boot media, enabling / disabling secure boot, etc.

Of course you can use the device tree itself to pass any board-specific
information that is needed by U-Boot at runtime (see binman_syms_ for how to
make binman insert these values directly into executables like SPL).

There is one more way this can be done: with individual .dtsi files for each
image supported by the SoC. Then the board `.dts` file can include the ones it
wants. This is not recommended, since it is likely to be difficult to maintain
and harder to understand the relationship between the different boards.


Producing images for multiple boards
------------------------------------

When invoked within U-Boot, binman only builds a single set of images, for
the chosen board. This is set by the `CONFIG_DEFAULT_DEVICE_TREE` option.

However, U-Boot generally builds all the device tree files associated with an
SoC. These are written to the (e.g. for ARM) `arch/arm/dts` directory. Each of
these contains the full binman description for that board. Often the best
approach is to build a single image that includes all these device tree binaries
and allow SPL to select the correct one on boot.

However, it is also possible to build separate images for each board, simply by
invoking binman multiple times, once for each device tree file, using a
different output directory. This will produce one set of images for each board.


Example use of binman for x86
-----------------------------

In most cases x86 images have a lot of binary blobs, 'black-box' code
provided by Intel which must be run for the platform to work. Typically
these blobs are not relocatable and must be placed at fixed areas in the
firmware image.

Currently this is handled by ifdtool, which places microcode, FSP, MRC, VGA
BIOS, reference code and Intel ME binaries into a u-boot.rom file.

Binman is intended to replace all of this, with ifdtool left to handle only
the configuration of the Intel-format descriptor.


Installing binman
-----------------

First install prerequisites, e.g:

.. code-block:: bash

    sudo apt-get install python-pyelftools python3-pyelftools lzma-alone \
        liblz4-tool

You can run binman directly if you put it on your PATH. But if you want to
install into your `~/.local` Python directory, use:

.. code-block:: bash

    pip install tools/patman tools/dtoc tools/binman

Note that binman makes use of libraries from patman and dtoc, which is why these
need to be installed. Also you need `libfdt` and `pylibfdt` which can be
installed like this:

.. code-block:: bash

   git clone git://git.kernel.org/pub/scm/utils/dtc/dtc.git
   cd dtc
   pip install .
   make NO_PYTHON=1 install

This installs the `libfdt.so` library into `~/lib` so you can use
`LD_LIBRARY_PATH=~/lib` when running binman. If you want to install it in the
system-library directory, replace the last line with:

.. code-block:: bash

   make NO_PYTHON=1 PREFIX=/ install

Running binman
--------------

Type:

.. code-block:: bash

   make NO_PYTHON=1 PREFIX=/ install
    binman build -b <board_name>

to build an image for a board. The board name is the same name used when
configuring U-Boot (e.g. for sandbox_defconfig the board name is 'sandbox').
Binman assumes that the input files for the build are in ../b/<board_name>.

Or you can specify this explicitly:

.. code-block:: bash

   make NO_PYTHON=1 PREFIX=/ install
    binman build -I <build_path>

where <build_path> is the build directory containing the output of the U-Boot
build.

(Future work will make this more configurable)

In either case, binman picks up the device tree file (u-boot.dtb) and looks
for its instructions in the 'binman' node.

Binman has a few other options which you can see by running 'binman -h'.


Enabling binman for a board
---------------------------

At present binman is invoked from a rule in the main Makefile. You should be
able to enable CONFIG_BINMAN to enable this rule.

The output file is typically named image.bin and is located in the output
directory. If input files are needed to you add these to INPUTS-y either in the
main Makefile or in a config.mk file in your arch subdirectory.

Once binman is executed it will pick up its instructions from a device-tree
file, typically <soc>-u-boot.dtsi, where <soc> is your CONFIG_SYS_SOC value.
You can use other, more specific CONFIG options - see 'Automatic .dtsi
inclusion' below.

.. _binman_syms:

Access to binman entry offsets at run time (symbols)
----------------------------------------------------

Binman assembles images and determines where each entry is placed in the image.
This information may be useful to U-Boot at run time. For example, in SPL it
is useful to be able to find the location of U-Boot so that it can be executed
when SPL is finished.

Binman allows you to declare symbols in the SPL image which are filled in
with their correct values during the build. For example:

.. code-block:: c

    binman_sym_declare(ulong, u_boot_any, image_pos);

declares a ulong value which will be assigned to the image-pos of any U-Boot
image (u-boot.bin, u-boot.img, u-boot-nodtb.bin) that is present in the image.
You can access this value with something like:

.. code-block:: c

    ulong u_boot_offset = binman_sym(ulong, u_boot_any, image_pos);

Thus u_boot_offset will be set to the image-pos of U-Boot in memory, assuming
that the whole image has been loaded, or is available in flash. You can then
jump to that address to start U-Boot.

At present this feature is only supported in SPL and TPL. In principle it is
possible to fill in such symbols in U-Boot proper, as well, but a future C
library is planned for this instead, to read from the device tree.

As well as image-pos, it is possible to read the size of an entry and its
offset (which is the start position of the entry within its parent).

A small technical note: Binman automatically adds the base address of the image
(i.e. __image_copy_start) to the value of the image-pos symbol, so that when the
image is loaded to its linked address, the value will be correct and actually
point into the image.

For example, say SPL is at the start of the image and linked to start at address
80108000. If U-Boot's image-pos is 0x8000 then binman will write an image-pos
for U-Boot of 80110000 into the SPL binary, since it assumes the image is loaded
to 80108000, with SPL at 80108000 and U-Boot at 80110000.

For x86 devices (with the end-at-4gb property) this base address is not added
since it is assumed that images are XIP and the offsets already include the
address.

While U-Boot's symbol updating is handled automatically by the u-boot-spl
entry type (and others), it is possible to use this feature with any blob. To
do this, add a `write-symbols` (boolean) property to the node, set the ELF
filename using `elf-filename` and set 'elf-base-sym' to the base symbol for the
start of the binary image (this defaults to `__image_copy_start` which is what
U-Boot uses). See `testBlobSymbol()` for an example.

.. _binman_fdt:

Access to binman entry offsets at run time (fdt)
------------------------------------------------

Binman can update the U-Boot FDT to include the final position and size of
each entry in the images it processes. The option to enable this is -u and it
causes binman to make sure that the 'offset', 'image-pos' and 'size' properties
are set correctly for every entry. Since it is not necessary to specify these in
the image definition, binman calculates the final values and writes these to
the device tree. These can be used by U-Boot at run-time to find the location
of each entry.

Alternatively, an FDT map entry can be used to add a special FDT containing
just the information about the image. This is preceded by a magic string so can
be located anywhere in the image. An image header (typically at the start or end
of the image) can be used to point to the FDT map. See fdtmap and image-header
entries for more information.

Map files
---------

The -m option causes binman to output a .map file for each image that it
generates. This shows the offset and size of each entry. For example::

      Offset      Size  Name
    00000000  00000028  main-section
     00000000  00000010  section@0
      00000000  00000004  u-boot
     00000010  00000010  section@1
      00000000  00000004  u-boot

This shows a hierarchical image with two sections, each with a single entry. The
offsets of the sections are absolute hex byte offsets within the image. The
offsets of the entries are relative to their respective sections. The size of
each entry is also shown, in bytes (hex). The indentation shows the entries
nested inside their sections.


Passing command-line arguments to entries
-----------------------------------------

Sometimes it is useful to pass binman the value of an entry property from the
command line. For example some entries need access to files and it is not
always convenient to put these filenames in the image definition (device tree).

The -a option supports this::

    -a <prop>=<value>

where::

    <prop> is the property to set
    <value> is the value to set it to

Not all properties can be provided this way. Only some entries support it,
typically for filenames.


Image description format
========================

The binman node is called 'binman'. An example image description is shown
below::

    binman {
        filename = "u-boot-sunxi-with-spl.bin";
        pad-byte = <0xff>;
        blob {
            filename = "spl/sunxi-spl.bin";
        };
        u-boot {
            offset = <CONFIG_SPL_PAD_TO>;
        };
    };


This requests binman to create an image file called u-boot-sunxi-with-spl.bin
consisting of a specially formatted SPL (spl/sunxi-spl.bin, built by the
normal U-Boot Makefile), some 0xff padding, and a U-Boot legacy image. The
padding comes from the fact that the second binary is placed at
CONFIG_SPL_PAD_TO. If that line were omitted then the U-Boot binary would
immediately follow the SPL binary.

The binman node describes an image. The sub-nodes describe entries in the
image. Each entry represents a region within the overall image. The name of
the entry (blob, u-boot) tells binman what to put there. For 'blob' we must
provide a filename. For 'u-boot', binman knows that this means 'u-boot.bin'.

Entries are normally placed into the image sequentially, one after the other.
The image size is the total size of all entries. As you can see, you can
specify the start offset of an entry using the 'offset' property.

Note that due to a device tree requirement, all entries must have a unique
name. If you want to put the same binary in the image multiple times, you can
use any unique name, with the 'type' property providing the type.

The attributes supported for entries are described below.

offset:
    This sets the offset of an entry within the image or section containing
    it. The first byte of the image is normally at offset 0. If 'offset' is
    not provided, binman sets it to the end of the previous region, or the
    start of the image's entry area (normally 0) if there is no previous
    region.

align:
    This sets the alignment of the entry. The entry offset is adjusted
    so that the entry starts on an aligned boundary within the containing
    section or image. For example 'align = <16>' means that the entry will
    start on a 16-byte boundary. This may mean that padding is added before
    the entry. The padding is part of the containing section but is not
    included in the entry, meaning that an empty space may be created before
    the entry starts. Alignment should be a power of 2. If 'align' is not
    provided, no alignment is performed.

size:
    This sets the size of the entry. The contents will be padded out to
    this size. If this is not provided, it will be set to the size of the
    contents.

min-size:
    Sets the minimum size of the entry. This size includes explicit padding
    ('pad-before' and 'pad-after'), but not padding added to meet alignment
    requirements. While this does not affect the contents of the entry within
    binman itself (the padding is performed only when its parent section is
    assembled), the end result will be that the entry ends with the padding
    bytes, so may grow. Defaults to 0.

pad-before:
    Padding before the contents of the entry. Normally this is 0, meaning
    that the contents start at the beginning of the entry. This can be used
    to offset the entry contents a little. While this does not affect the
    contents of the entry within binman itself (the padding is performed
    only when its parent section is assembled), the end result will be that
    the entry starts with the padding bytes, so may grow. Defaults to 0.

pad-after:
    Padding after the contents of the entry. Normally this is 0, meaning
    that the entry ends at the last byte of content (unless adjusted by
    other properties). This allows room to be created in the image for
    this entry to expand later. While this does not affect the contents of
    the entry within binman itself (the padding is performed only when its
    parent section is assembled), the end result will be that the entry ends
    with the padding bytes, so may grow. Defaults to 0.

align-size:
    This sets the alignment of the entry size. For example, to ensure
    that the size of an entry is a multiple of 64 bytes, set this to 64.
    While this does not affect the contents of the entry within binman
    itself (the padding is performed only when its parent section is
    assembled), the end result is that the entry ends with the padding
    bytes, so may grow. If 'align-size' is not provided, no alignment is
    performed.

align-end:
    This sets the alignment of the end of an entry with respect to the
    containing section. Some entries require that they end on an alignment
    boundary, regardless of where they start. This does not move the start
    of the entry, so the contents of the entry will still start at the
    beginning. But there may be padding at the end. While this does not
    affect the contents of the entry within binman itself (the padding is
    performed only when its parent section is assembled), the end result
    is that the entry ends with the padding bytes, so may grow.
    If 'align-end' is not provided, no alignment is performed.

filename:
    For 'blob' types this provides the filename containing the binary to
    put into the entry. If binman knows about the entry type (like
    u-boot-bin), then there is no need to specify this.

type:
    Sets the type of an entry. This defaults to the entry name, but it is
    possible to use any name, and then add (for example) 'type = "u-boot"'
    to specify the type.

offset-unset:
    Indicates that the offset of this entry should not be set by placing
    it immediately after the entry before. Instead, is set by another
    entry which knows where this entry should go. When this boolean
    property is present, binman will give an error if another entry does
    not set the offset (with the GetOffsets() method).

image-pos:
    This cannot be set on entry (or at least it is ignored if it is), but
    with the -u option, binman will set it to the absolute image position
    for each entry. This makes it easy to find out exactly where the entry
    ended up in the image, regardless of parent sections, etc.

extend-size:
    Extend the size of this entry to fit available space. This space is only
    limited by the size of the image/section and the position of the next
    entry.

compress:
    Sets the compression algortihm to use (for blobs only). See the entry
    documentation for details.

missing-msg:
    Sets the tag of the message to show if this entry is missing. This is
    used for external blobs. When they are missing it is helpful to show
    information about what needs to be fixed. See missing-blob-help for the
    message for each tag.

no-expanded:
    By default binman substitutes entries with expanded versions if available,
    so that a `u-boot` entry type turns into `u-boot-expanded`, for example. The
    `--no-expanded` command-line option disables this globally. The
    `no-expanded` property disables this just for a single entry. Put the
    `no-expanded` boolean property in the node to select this behaviour.

optional:
    External blobs are normally required to be present for the image to be
    built (but see `External blobs`_). This properly allows an entry to be
    optional, so that when it is cannot be found, this problem is ignored and
    an empty file is used for this blob. This should be used only when the blob
    is entirely optional and is not needed for correct operation of the image.
    Note that missing, optional blobs do not produce a non-zero exit code from
    binman, although it does show a warning about the missing external blob.

insert-template:
    This is not strictly speaking an entry property, since it is processed early
    in Binman before the entries are read. It is a list of phandles of nodes to
    include in the current (target) node. For each node, its subnodes and their
    properties are brought into the target node. See Templates_ below for
    more information.

The attributes supported for images and sections are described below. Several
are similar to those for entries.

size:
    Sets the image size in bytes, for example 'size = <0x100000>' for a
    1MB image.

offset:
    This is similar to 'offset' in entries, setting the offset of a section
    within the image or section containing it. The first byte of the section
    is normally at offset 0. If 'offset' is not provided, binman sets it to
    the end of the previous region, or the start of the image's entry area
    (normally 0) if there is no previous region.

align-size:
    This sets the alignment of the image size. For example, to ensure
    that the image ends on a 512-byte boundary, use 'align-size = <512>'.
    If 'align-size' is not provided, no alignment is performed.

pad-before:
    This sets the padding before the image entries. The first entry will
    be positioned after the padding. This defaults to 0.

pad-after:
    This sets the padding after the image entries. The padding will be
    placed after the last entry. This defaults to 0.

pad-byte:
    This specifies the pad byte to use when padding in the image. It
    defaults to 0. To use 0xff, you would add 'pad-byte = <0xff>'.

filename:
    This specifies the image filename. It defaults to 'image.bin'.

sort-by-offset:
    This causes binman to reorder the entries as needed to make sure they
    are in increasing positional order. This can be used when your entry
    order may not match the positional order. A common situation is where
    the 'offset' properties are set by CONFIG options, so their ordering is
    not known a priori.

    This is a boolean property so needs no value. To enable it, add a
    line 'sort-by-offset;' to your description.

multiple-images:
    Normally only a single image is generated. To create more than one
    image, put this property in the binman node. For example, this will
    create image1.bin containing u-boot.bin, and image2.bin containing
    both spl/u-boot-spl.bin and u-boot.bin::

        binman {
            multiple-images;
            image1 {
                u-boot {
                };
            };

            image2 {
                spl {
                };
                u-boot {
                };
            };
        };

end-at-4gb:
    For x86 machines the ROM offsets start just before 4GB and extend
    up so that the image finished at the 4GB boundary. This boolean
    option can be enabled to support this. The image size must be
    provided so that binman knows when the image should start. For an
    8MB ROM, the offset of the first entry would be 0xfff80000 with
    this option, instead of 0 without this option.

skip-at-start:
    This property specifies the entry offset of the first entry.

    For PowerPC mpc85xx based CPU, CONFIG_TEXT_BASE is the entry
    offset of the first entry. It can be 0xeff40000 or 0xfff40000 for
    nor flash boot, 0x201000 for sd boot etc.

    'end-at-4gb' property is not applicable where CONFIG_TEXT_BASE +
    Image size != 4gb.

align-default:
    Specifies the default alignment for entries in this section, if they do
    not specify an alignment. Note that this only applies to top-level entries
    in the section (direct subentries), not any subentries of those entries.
    This means that each section must specify its own default alignment, if
    required.

symlink:
    Adds a symlink to the image with string given in the symlink property.

overlap:
    Indicates that this entry overlaps with others in the same section. These
    entries should appear at the end of the section. Overlapping entries are not
    packed with other entries, but their contents are written over other entries
    in the section. Overlapping entries must have an explicit offset and size.

write-symbols:
    Indicates that the blob should be updated with symbol values calculated by
    binman. This is automatic for certain entry types, e.g. `u-boot-spl`. See
    binman_syms_ for more information.

no-write-symbols:
    Disables symbol writing for this entry. This can be used in entry types
    where symbol writing is automatic. For example, if `u-boot-spl` refers to
    the `u_boot_any_image_pos` symbol but U-Boot is not available in the image
    containing SPL, this can be used to disable the writing. Quite likely this
    indicates a bug in your setup.

elf-filename:
    Sets the file name of a blob's associated ELF file. For example, if the
    blob is `zephyr.bin` then the ELF file may be `zephyr.elf`. This allows
    binman to locate symbols and understand the structure of the blob. See
    binman_syms_ for more information.

elf-base-sym:
    Sets the name of the ELF symbol that points to the start of a blob. For
    U-Boot this is `__image_copy_start` and that is the default used by binman
    if this property is missing. For other projects, a difference symbol may be
    needed. Add this symbol to the properties for the blob so that symbols can
    be read correctly. See binman_syms_ for more information.

offset-from-elf:
    Sets the offset of an entry based on a symbol value in an another entry.
    The format is <&phandle>, "sym_name", <offset> where phandle is the entry
    containing the blob (with associated ELF file providing symbols), <sym_name>
    is the symbol to lookup (relative to elf-base-sym) and <offset> is an offset
    to add to that value.

preserve:
    Indicates that this entry should be preserved by any firmware updates. This
    flag should be checked by the updater when it is deciding which entries to
    update. This flag is normally attached to sections but can be attached to
    a single entry in a section if the updater supports it. Not that binman
    itself has no control over the updater's behaviour, so this is just a
    signal. It is not enforced by binman.

Examples of the above options can be found in the tests. See the
tools/binman/test directory.

It is possible to have the same binary appear multiple times in the image,
either by using a unit number suffix (u-boot@0, u-boot@1) or by using a
different name for each and specifying the type with the 'type' attribute.


Sections and hierachical images
-------------------------------

Sometimes it is convenient to split an image into several pieces, each of which
contains its own set of binaries. An example is a flash device where part of
the image is read-only and part is read-write. We can set up sections for each
of these, and place binaries in them independently. The image is still produced
as a single output file.

This feature provides a way of creating hierarchical images. For example here
is an example image with two copies of U-Boot. One is read-only (ro), intended
to be written only in the factory. Another is read-write (rw), so that it can be
upgraded in the field. The sizes are fixed so that the ro/rw boundary is known
and can be programmed::

    binman {
        section@0 {
            read-only;
            name-prefix = "ro-";
            size = <0x100000>;
            u-boot {
            };
        };
        section@1 {
            name-prefix = "rw-";
            size = <0x100000>;
            u-boot {
            };
        };
    };

This image could be placed into a SPI flash chip, with the protection boundary
set at 1MB.

A few special properties are provided for sections:

read-only:
    Indicates that this section is read-only. This has no impact on binman's
    operation, but his property can be read at run time.

name-prefix:
    This string is prepended to all the names of the binaries in the
    section. In the example above, the 'u-boot' binaries which actually be
    renamed to 'ro-u-boot' and 'rw-u-boot'. This can be useful to
    distinguish binaries with otherwise identical names.

filename:
    This allows the contents of the section to be written to a file in the
    output directory. This can sometimes be useful to use the data in one
    section in different image, since there is currently no way to share data
    beteen images other than through files.

Image Properties
----------------

Image nodes act like sections but also have a few extra properties:

filename:
    Output filename for the image. This defaults to image.bin (or in the
    case of multiple images <nodename>.bin where <nodename> is the name of
    the image node.

allow-repack:
    Create an image that can be repacked. With this option it is possible
    to change anything in the image after it is created, including updating
    the position and size of image components. By default this is not
    permitted since it is not possibly to know whether this might violate a
    constraint in the image description. For example, if a section has to
    increase in size to hold a larger binary, that might cause the section
    to fall out of its allow region (e.g. read-only portion of flash).

    Adding this property causes the original offset and size values in the
    image description to be stored in the FDT and fdtmap.


Image dependencies
------------------

Binman does not currently support images that depend on each other. For example,
if one image creates `fred.bin` and then the next uses this `fred.bin` to
produce a final `image.bin`, then the behaviour is undefined. It may work, or it
may produce an error about `fred.bin` being missing, or it may use a version of
`fred.bin` from a previous run.

Often this can be handled by incorporating the dependency into the second
image. For example, instead of::

    binman {
        multiple-images;

        fred {
            u-boot {
            };
            fill {
                size = <0x100>;
            };
        };

        image {
            blob {
                filename = "fred.bin";
            };
            u-boot-spl {
            };
        };

you can do this::

    binman {
        image {
            fred {
                type = "section";
                u-boot {
                };
                fill {
                    size = <0x100>;
                };
            };
            u-boot-spl {
            };
        };



Hashing Entries
---------------

It is possible to ask binman to hash the contents of an entry and write that
value back to the device-tree node. For example::

    binman {
        u-boot {
            hash {
                algo = "sha256";
            };
        };
    };

Here, a new 'value' property will be written to the 'hash' node containing
the hash of the 'u-boot' entry. Only SHA256 is supported at present. Whole
sections can be hased if desired, by adding the 'hash' node to the section.

The has value can be chcked at runtime by hashing the data actually read and
comparing this has to the value in the device tree.


Expanded entries
----------------

Binman automatically replaces 'u-boot' with an expanded version of that, i.e.
'u-boot-expanded'. This means that when you write::

    u-boot {
    };

you actually get::

    u-boot {
        type = "u-boot-expanded';
    };

which in turn expands to::

    u-boot {
        type = "section";

        u-boot-nodtb {
        };

        u-boot-dtb {
        };
    };

U-Boot's various phase binaries actually comprise two or three pieces.
For example, u-boot.bin has the executable followed by a devicetree.

With binman we want to be able to update that devicetree with full image
information so that it is accessible to the executable. This is tricky
if it is not clear where the devicetree starts.

The above feature ensures that the devicetree is clearly separated from the
U-Boot executable and can be updated separately by binman as needed. It can be
disabled with the --no-expanded flag if required.

The same applies for u-boot-spl and u-boot-tpl. In those cases, the expansion
includes the BSS padding, so for example::

    spl {
        type = "u-boot-spl"
    };

you actually get::

    spl {
        type = "u-boot-expanded';
    };

which in turn expands to::

    spl {
        type = "section";

        u-boot-spl-nodtb {
        };

        u-boot-spl-bss-pad {
        };

        u-boot-spl-dtb {
        };
    };

Of course we should not expand SPL if it has no devicetree. Also if the BSS
padding is not needed (because BSS is in RAM as with CONFIG_SPL_SEPARATE_BSS),
the 'u-boot-spl-bss-pad' subnode should not be created. The use of the expaned
entry type is controlled by the UseExpanded() method. In the SPL case it checks
the 'spl-dtb' entry arg, which is 'y' or '1' if SPL has a devicetree.

For the BSS case, a 'spl-bss-pad' entry arg controls whether it is present. All
entry args are provided by the U-Boot Makefile.


Optional entries
----------------

Some entries need to exist only if certain conditions are met. For example, an
entry may want to appear in the image only if a file has a particular format.
Obviously the entry must exist in the image description for it to be processed
at all, so a way needs to be found to have the entry remove itself.

To handle this, when entry.ObtainContents() is called, the entry can call
entry.mark_absent() to mark itself as absent, passing a suitable message as the
reason.

Any absent entries are dropped immediately after ObtainContents() has been
called on all entries.

It is not possible for an entry to mark itself absent at any other point in the
processing. It must happen in the ObtainContents() method.

The effect is as if the entry had never been present at all, since the image
is packed without it and it disappears from the list of entries.


Compression
-----------

Binman support compression for 'blob' entries (those of type 'blob' and
derivatives). To enable this for an entry, add a 'compress' property::

    blob {
        filename = "datafile";
        compress = "lz4";
    };

The entry will then contain the compressed data, using the 'lz4' compression
algorithm. Currently this is the only one that is supported. The uncompressed
size is written to the node in an 'uncomp-size' property, if -u is used.

Compression is also supported for sections. In that case the entire section is
compressed in one block, including all its contents. This means that accessing
an entry from the section required decompressing the entire section. Also, the
size of a section indicates the space that it consumes in its parent section
(and typically the image). With compression, the section may contain more data,
and the uncomp-size property indicates that, as above. The contents of the
section is compressed first, before any padding is added. This ensures that the
padding itself is not compressed, which would be a waste of time.


Automatic .dtsi inclusion
-------------------------

It is sometimes inconvenient to add a 'binman' node to the .dts file for each
board. This can be done by using #include to bring in a common file. Another
approach supported by the U-Boot build system is to automatically include
a common header. You can then put the binman node (and anything else that is
specific to U-Boot, such as bootph-all properies) in that header file.

Binman will search for the following files in arch/<arch>/dts::

   <dts>-u-boot.dtsi where <dts> is the base name of the .dts file
   <CONFIG_SYS_SOC>-u-boot.dtsi
   <CONFIG_SYS_CPU>-u-boot.dtsi
   <CONFIG_SYS_VENDOR>-u-boot.dtsi
   u-boot.dtsi

U-Boot will only use the first one that it finds. If you need to include a
more general file you can do that from the more specific file using #include.
If you are having trouble figuring out what is going on, you can use
`DEVICE_TREE_DEBUG=1` with your build::

   make DEVICE_TREE_DEBUG=1
   scripts/Makefile.lib:334: Automatic .dtsi inclusion: options:
     arch/arm/dts/juno-r2-u-boot.dtsi arch/arm/dts/-u-boot.dtsi
     arch/arm/dts/armv8-u-boot.dtsi arch/arm/dts/armltd-u-boot.dtsi
     arch/arm/dts/u-boot.dtsi ... found: "arch/arm/dts/juno-r2-u-boot.dtsi"


Templates
=========

Sometimes multiple images need to be created which have all have a common
part. For example, a board may generate SPI and eMMC images which both include
a FIT. Since the FIT includes many entries, it is tedious to repeat them twice
in the image description.

Templates provide a simple way to handle this::

    binman {
        multiple-images;
        common_part: template-1 {
            some-property;
            fit {
                ... lots of entries in here
            };

            text {
                text = "base image";
            };
        };

        spi-image {
            filename = "image-spi.bin";
            insert-template = <&fit>;

            /* things specific to SPI follow */
            footer {
            ];

            text {
                text = "SPI image";
            };
        };

        mmc-image {
            filename = "image-mmc.bin";
            insert-template = <&fit>;

            /* things specific to MMC follow */
            footer {
            ];

            text {
                text = "MMC image";
            };
        };
    };

The template node name must start with 'template', so it is not considered to be
an image itself.

The mechanism is very simple. For each phandle in the 'insert-templates'
property, the source node is looked up. Then the subnodes of that source node
are copied into the target node, i.e. the one containing the `insert-template`
property.

If the target node has a node with the same name as a template, its properties
override corresponding properties in the template. This allows the template to
be uses as a base, with the node providing updates to the properties as needed.
The overriding happens recursively.

Template nodes appear first in each node that they are inserted into and
ordering of template nodes is preserved. Other nodes come afterwards. If a
template node also appears in the target node, then the template node sets the
order. Thus the template can be used to set the ordering, even if the target
node provides all the properties. In the above example, `fit` and `text` appear
first in the `spi-image` and `mmc-image` images, followed by `footer`.

Where there are multiple template nodes, they are inserted in that order. so
the first template node appears first, then the second.

Properties in the template node are inserted into the destination node if they
do not exist there. In the example above, `some-property` is added to each of
`spi-image` and `mmc-image`.

Note that template nodes are removed from the binman description after
processing and before binman builds the image descriptions.

The initial devicetree produced by the templating process is written to the
`u-boot.dtb.tmpl1` file. This can be useful to see what is going on if there is
a failure before the final `u-boot.dtb.out` file is written. A second
`u-boot.dtb.tmpl2` file is written when the templates themselves are removed.

Dealing with phandles
---------------------

Templates can contain phandles and these are copied to the destination node.
However this should be used with care, since if a template is instantiated twice
then the phandle will be copied twice, resulting in a devicetree with duplicate
phandles, i.e. the same phandle used by two different nodes. Binman detects this
situation and produces an error, for example::

  Duplicate phandle 1 in nodes /binman/image/fit/images/atf/atf-bl31 and
  /binman/image-2/fit/images/atf/atf-bl31

In this case an atf-bl31 node containing a phandle has been copied into two
different target nodes, resulting in the same phandle for each. See
testTemplatePhandleDup() for the test case.

The solution is typically to put the phandles in the corresponding target nodes
(one for each) and remove the phandle from the template.

Updating an ELF file
====================

For the EFI app, where U-Boot is loaded from UEFI and runs as an app, there is
no way to update the devicetree after U-Boot is built. Normally this works by
creating a new u-boot.dtb.out with he updated devicetree, which is automatically
built into the output image. With ELF this is not possible since the ELF is
not part of an image, just a stand-along file. We must create an updated ELF
file with the new devicetree.

This is handled by the --update-fdt-in-elf option. It takes four arguments,
separated by comma:

   infile     - filename of input ELF file, e.g. 'u-boot's
   outfile    - filename of output ELF file, e.g. 'u-boot.out'
   begin_sym - symbol at the start of the embedded devicetree, e.g.
   '__dtb_dt_begin'
   end_sym   - symbol at the start of the embedded devicetree, e.g.
   '__dtb_dt_end'

When this flag is used, U-Boot does all the normal packaging, but as an
additional step, it creates a new ELF file with the new devicetree embedded in
it.

If logging is enabled you will see a message like this::

   Updating file 'u-boot' with data length 0x400a (16394) between symbols
   '__dtb_dt_begin' and '__dtb_dt_end'

There must be enough space for the updated devicetree. If not, an error like
the following is produced::

   ValueError: Not enough space in 'u-boot' for data length 0x400a (16394);
   size is 0x1744 (5956)


Entry Documentation
===================

For details on the various entry types supported by binman and how to use them,
see entries.rst which is generated from the source code using:

    binman entry-docs >tools/binman/entries.rst

.. toctree::
   :maxdepth: 2

   entries


Managing images
===============

Listing images
--------------

It is possible to list the entries in an existing firmware image created by
binman, provided that there is an 'fdtmap' entry in the image. For example::

    $ binman ls -i image.bin
    Name              Image-pos  Size  Entry-type    Offset  Uncomp-size
    ----------------------------------------------------------------------
    main-section                  c00  section            0
      u-boot                  0     4  u-boot             0
      section                     5fc  section            4
        cbfs                100   400  cbfs               0
          u-boot            138     4  u-boot            38
          u-boot-dtb        180   108  u-boot-dtb        80          3b5
        u-boot-dtb          500   1ff  u-boot-dtb       400          3b5
      fdtmap                6fc   381  fdtmap           6fc
      image-header          bf8     8  image-header     bf8

This shows the hierarchy of the image, the position, size and type of each
entry, the offset of each entry within its parent and the uncompressed size if
the entry is compressed.

It is also possible to list just some files in an image, e.g.::

    $ binman ls -i image.bin section/cbfs
    Name              Image-pos  Size  Entry-type  Offset  Uncomp-size
    --------------------------------------------------------------------
        cbfs                100   400  cbfs             0
          u-boot            138     4  u-boot          38
          u-boot-dtb        180   108  u-boot-dtb      80          3b5

or with wildcards::

    $ binman ls -i image.bin "*cb*" "*head*"
    Name              Image-pos  Size  Entry-type    Offset  Uncomp-size
    ----------------------------------------------------------------------
        cbfs                100   400  cbfs               0
          u-boot            138     4  u-boot            38
          u-boot-dtb        180   108  u-boot-dtb        80          3b5
      image-header          bf8     8  image-header     bf8

If an older version of binman is used to list images created by a newer one, it
is possible that it will contain entry types that are not supported. These still
show with the correct type, but binman just sees them as blobs (plain binary
data). Any special features of that etype are not supported by the old binman.


Extracting files from images
----------------------------

You can extract files from an existing firmware image created by binman,
provided that there is an 'fdtmap' entry in the image. For example::

    $ binman extract -i image.bin section/cbfs/u-boot

which will write the uncompressed contents of that entry to the file 'u-boot' in
the current directory. You can also extract to a particular file, in this case
u-boot.bin::

    $ binman extract -i image.bin section/cbfs/u-boot -f u-boot.bin

It is possible to extract all files into a destination directory, which will
put files in subdirectories matching the entry hierarchy::

    $ binman extract -i image.bin -O outdir

or just a selection::

    $ binman extract -i image.bin "*u-boot*" -O outdir

Some entry types have alternative formats, for example fdtmap which allows
extracted just the devicetree binary without the fdtmap header::

    $ binman extract -i /tmp/b/odroid-c4/image.bin -f out.dtb -F fdt fdtmap
    $ fdtdump out.dtb
    /dts-v1/;
    // magic:               0xd00dfeed
    // totalsize:           0x8ab (2219)
    // off_dt_struct:       0x38
    // off_dt_strings:      0x82c
    // off_mem_rsvmap:      0x28
    // version:             17
    // last_comp_version:   2
    // boot_cpuid_phys:     0x0
    // size_dt_strings:     0x7f
    // size_dt_struct:      0x7f4

    / {
        image-node = "binman";
        image-pos = <0x00000000>;
        size = <0x0011162b>;
        ...

Use `-F list` to see what alternative formats are available::

    $ binman extract -i /tmp/b/odroid-c4/image.bin -F list
    Flag (-F)   Entry type            Description
    fdt         fdtmap                Extract the devicetree blob from the fdtmap


Replacing files in an image
---------------------------

You can replace files in an existing firmware image created by binman, provided
that there is an 'fdtmap' entry in the image. For example::

    $ binman replace -i image.bin section/cbfs/u-boot

which will write the contents of the file 'u-boot' from the current directory
to the that entry, compressing if necessary. If the entry size changes, you must
add the 'allow-repack' property to the original image before generating it (see
above), otherwise you will get an error.

You can also use a particular file, in this case u-boot.bin::

    $ binman replace -i image.bin section/cbfs/u-boot -f u-boot.bin

It is possible to replace all files from a source directory which uses the same
hierarchy as the entries::

    $ binman replace -i image.bin -I indir

Files that are missing will generate a warning.

You can also replace just a selection of entries::

    $ binman replace -i image.bin "*u-boot*" -I indir

It is possible to replace whole sections as well, but in that case any
information about entries within the section may become outdated. This is
because Binman cannot know whether things have moved around or resized within
the section, once you have updated its data.

Technical note: With 'allow-repack', Binman writes information about the
original offset and size properties of each entry, if any were specified, in
the 'orig-offset' and 'orig-size' properties. This allows Binman to distinguish
between an entry which ended up being packed at an offset (or assigned a size)
and an entry which had a particular offset / size requested in the Binman
configuration. Where are particular offset / size was requested, this is treated
as set in stone, so Binman will ensure it doesn't change. Without this feature,
repacking an entry might cause it to disobey the original constraints provided
when it was created.


Signing FIT container with private key in an image
--------------------------------------------------

You can sign FIT container with private key in your image.
For example::

    $ binman sign -i image.bin -k privatekey -a sha256,rsa4096 fit

binman will extract FIT container, sign and replace it immediately.

If you want to sign and replace FIT container in place::

    $ binman sign -i image.bin -k privatekey -a sha256,rsa4096 -f fit.fit fit

which will sign FIT container with private key and replace it immediately
inside your image.

.. _`BinmanLogging`:

Logging
-------

Binman normally operates silently unless there is an error, in which case it
just displays the error. The -D/--debug option can be used to create a full
backtrace when errors occur. You can use BINMAN_DEBUG=1 when building to select
this.

Internally binman logs some output while it is running. This can be displayed
by increasing the -v/--verbosity from the default of 1:

   0: silent
   1: warnings only
   2: notices (important messages)
   3: info about major operations
   4: detailed information about each operation
   5: debug (all output)

You can use BINMAN_VERBOSE=5 (for example) when building to select this.


Bintools
========

`Bintool` is the name binman gives to a binary tool which it uses to create and
manipulate binaries that binman cannot handle itself. Bintools are often
necessary since Binman only supports a subset of the available file formats
natively.

Many SoC vendors invent ways to load code into their SoC using new file formats,
sometimes changing the format with successive SoC generations. Sometimes the
tool is available as Open Source. Sometimes it is a pre-compiled binary that
must be downloaded from the vendor's website. Sometimes it is available in
source form but difficult or slow to build.

Even for images that use bintools, binman still assembles the image from its
image description. It may handle parts of the image natively and part with
various bintools.

Binman relies on these tools so provides various features to manage them:

- Determining whether the tool is currently installed
- Downloading or building the tool
- Determining the version of the tool that is installed
- Deciding which tools are needed to build an image

The Bintool class is an interface to the tool, a thin level of abstration, using
Python functions to run the tool for each purpose (e.g. creating a new
structure, adding a file to an existing structure) rather than just lists of
string arguments.

As with external blobs, bintools (which are like 'external' tools) can be
missing. When building an image requires a bintool and it is not installed,
binman detects this and reports the problem, but continues to build an image.
This is useful in CI systems which want to check that everything is correct but
don't have access to the bintools.

To make this work, all calls to bintools (e.g. with Bintool.run_cmd()) must cope
with the tool being missing, i.e. when None is returned, by:

- Calling self.record_missing_bintool()
- Setting up some fake contents so binman can continue

Of course the image will not work, but binman reports which bintools are needed
and also provide a way to fetch them.

To see the available bintools, use::

    binman tool --list

To fetch tools which are missing, use::

    binman tool --fetch missing

You can also use `--fetch all` to fetch all tools or `--fetch <tool>` to fetch
a particular tool. Some tools are built from source code, in which case you will
need to have at least the `build-essential` and `git` packages installed.

Tools are fetched into the `~/.binman-tools` directory. This directory is
automatically added to the toolpath so there is no need to use `--toolpath` to
specify it. If you want to use these tools outside binman, you may want to
add this directory to your `PATH`. For example, if you use bash, add this to
the end of `.bashrc`::

   PATH="$HOME/.binman-tools:$PATH"

To select a custom directory, use the `--tooldir` option.

Bintool Documentation
=====================

To provide details on the various bintools supported by binman, bintools.rst is
generated from the source code using:

    binman bintool-docs >tools/binman/bintools.rst

.. toctree::
   :maxdepth: 2

   bintools

Binman commands and arguments
=============================

Usage::

    binman [-h] [-B BUILD_DIR] [-D] [--tooldir TOOLDIR] [-H]
        [--toolpath TOOLPATH] [-T THREADS] [--test-section-timeout]
        [-v VERBOSITY] [-V]
        {build,bintool-docs,entry-docs,ls,extract,replace,test,tool} ...

Binman provides the following commands:

- **build** - build images
- **bintools-docs** - generate documentation about bintools
- **entry-docs** - generate documentation about entry types
- **ls** - list an image
- **extract** - extract files from an image
- **replace** - replace one or more entries in an image
- **test** - run tests
- **tool** - manage bintools

Options:

-h, --help
    Show help message and exit

-B BUILD_DIR, --build-dir BUILD_DIR
    Directory containing the build output

-D, --debug
    Enabling debugging (provides a full traceback on error)

--tooldir TOOLDIR     Set the directory to store tools

-H, --full-help
    Display the README file

--toolpath TOOLPATH
    Add a path to the list of directories containing tools

-T THREADS, --threads THREADS
    Number of threads to use (0=single-thread). Note that -T0 is useful for
    debugging since everything runs in one thread.

-v VERBOSITY, --verbosity VERBOSITY
    Control verbosity: 0=silent, 1=warnings, 2=notices, 3=info, 4=detail,
    5=debug

-V, --version
    Show the binman version

Test options:

--test-section-timeout
    Use a zero timeout for section multi-threading (for testing)

Commands are described below.

binman build
------------

This builds one or more images using the provided image description.

Usage::

    binman build [-h] [-a ENTRY_ARG] [-b BOARD] [-d DT] [--fake-dtb]
        [--fake-ext-blobs] [--force-missing-bintools FORCE_MISSING_BINTOOLS]
        [-i IMAGE] [-I INDIR] [-m] [-M] [-n] [-O OUTDIR] [-p] [-u]
        [--update-fdt-in-elf UPDATE_FDT_IN_ELF] [-W]

Options:

-h, --help
    Show help message and exit

-a ENTRY_ARG, --entry-arg ENTRY_ARG
    Set argument value `arg=value`. See
    `Passing command-line arguments to entries`_.

-b BOARD, --board BOARD
    Board name to build. This can be used instead of `-d`, in which case the
    file `u-boot.dtb` is used, within the build directory's board subdirectory.

-d DT, --dt DT
    Configuration file (.dtb) to use. This must have a top-level node called
    `binman`. See `Image description format`_.

-i IMAGE, --image IMAGE
    Image filename to build (if not specified, build all)

-I INDIR, --indir INDIR
    Add a path to the list of directories to use for input files. This can be
    specified multiple times to add more than one path.

-m, --map
    Output a map file for each image. See `Map files`_.

-M, --allow-missing
    Allow external blobs and bintools to be missing. See `External blobs`_.

-n, --no-expanded
    Don't use 'expanded' versions of entries where available; normally 'u-boot'
    becomes 'u-boot-expanded', for example. See `Expanded entries`_.

-O OUTDIR, --outdir OUTDIR
    Path to directory to use for intermediate and output files

-p, --preserve
    Preserve temporary output directory even if option -O is not given

-u, --update-fdt
    Update the binman node with offset/size info. See
    `Access to binman entry offsets at run time (fdt)`_.

--update-fdt-in-elf UPDATE_FDT_IN_ELF
    Update an ELF file with the output dtb. The argument is a string consisting
    of four parts, separated by commas. See `Updating an ELF file`_.

-W, --ignore-missing
    Return success even if there are missing blobs/bintools (requires -M)

Options used only for testing:

--fake-dtb
    Use fake device tree contents

--fake-ext-blobs
    Create fake ext blobs with dummy content

--force-missing-bintools FORCE_MISSING_BINTOOLS
    Comma-separated list of bintools to consider missing

binman bintool-docs
-------------------

Usage::

    binman bintool-docs [-h]

This outputs documentation for the bintools in rST format. See
`Bintool Documentation`_.

binman entry-docs
-----------------

Usage::

    binman entry-docs [-h]

This outputs documentation for the entry types in rST format. See
`Entry Documentation`_.

binman ls
---------

Usage::

    binman ls [-h] -i IMAGE [paths ...]

Positional arguments:

paths
    Paths within file to list (wildcard)

Pptions:

-h, --help
    show help message and exit

-i IMAGE, --image IMAGE
    Image filename to list

This lists an image, showing its contents. See `Listing images`_.

binman extract
--------------

Usage::

    binman extract [-h] [-F FORMAT] -i IMAGE [-f FILENAME] [-O OUTDIR] [-U]
        [paths ...]

Positional arguments:

Paths
    Paths within file to extract (wildcard)

Options:

-h, --help
    show help message and exit

-F FORMAT, --format FORMAT
    Select an alternative format for extracted data

-i IMAGE, --image IMAGE
    Image filename to extract

-f FILENAME, --filename FILENAME
    Output filename to write to

-O OUTDIR, --outdir OUTDIR
    Path to directory to use for output files

-U, --uncompressed
    Output raw uncompressed data for compressed entries

This extracts the contents of entries from an image. See
`Extracting files from images`_.

binman replace
--------------

Usage::

    binman replace [-h] [-C] -i IMAGE [-f FILENAME] [-F] [-I INDIR] [-m]
        [paths ...]

Positional arguments:

paths
    Paths within file to replace (wildcard)

Options:

-h, --help
    show help message and exit

-C, --compressed
    Input data is already compressed if needed for the entry

-i IMAGE, --image IMAGE
    Image filename to update

-f FILENAME, --filename FILENAME
    Input filename to read from

-F, --fix-size
    Don't allow entries to be resized

-I INDIR, --indir INDIR
    Path to directory to use for input files

-m, --map
    Output a map file for the updated image

-O OUTDIR, --outdir OUTDIR
    Path to directory to use for intermediate and output files

-p, --preserve
    Preserve temporary output directory even if option -O is not given

This replaces one or more entries in an existing image. See
`Replacing files in an image`_.

binman test
-----------

Usage::

    binman test [-h] [-P PROCESSES] [-T] [-X] [tests ...]

Positional arguments:

tests
    Test names to run (omit for all)

Options:

-h, --help
    show help message and exit

-P PROCESSES, --processes PROCESSES
    set number of processes to use for running tests. This defaults to the
    number of CPUs on the machine

-T, --test-coverage
    run tests and check for 100% coverage

-X, --test-preserve-dirs
    Preserve and display test-created input directories; also preserve the
    output directory if a single test is run (pass test name at the end of the
    command line

binman sign
-----------

Usage::

    binman sign [-h] -a ALGO [-f FILE] -i IMAGE -k KEY [paths ...]

positional arguments:

paths
    Paths within file to sign (wildcard)

options:

-h, --help
    show this help message and exit

-a ALGO, --algo ALGO
    Hash algorithm e.g. sha256,rsa4096

-f FILE, --file FILE
    Input filename to sign

-i IMAGE, --image IMAGE
    Image filename to update

-k KEY, --key KEY
    Private key file for signing

binman tool
-----------

Usage::

    binman tool [-h] [-l] [-f] [bintools ...]

Positional arguments:

bintools
    Bintools to process

Options:

-h, --help
    show help message and exit

-l, --list
    List all known bintools

-f, --fetch
    Fetch a bintool from a known location. Use `all` to fetch all and `missing`
    to fetch any missing tools.


Technical details
=================

Order of image creation
-----------------------

Image creation proceeds in the following order, for each entry in the image.

1. AddMissingProperties() - binman can add calculated values to the device
tree as part of its processing, for example the offset and size of each
entry. This method adds any properties associated with this, expanding the
device tree as needed. These properties can have placeholder values which are
set later by SetCalculatedProperties(). By that stage the size of sections
cannot be changed (since it would cause the images to need to be repacked),
but the correct values can be inserted.

2. ProcessFdt() - process the device tree information as required by the
particular entry. This may involve adding or deleting properties. If the
processing is complete, this method should return True. If the processing
cannot complete because it needs the ProcessFdt() method of another entry to
run first, this method should return False, in which case it will be called
again later.

3. GetEntryContents() - the contents of each entry are obtained, normally by
reading from a file. This calls the Entry.ObtainContents() to read the
contents. The default version of Entry.ObtainContents() calls
Entry.GetDefaultFilename() and then reads that file. So a common mechanism
to select a file to read is to override that function in the subclass. The
functions must return True when they have read the contents. Binman will
retry calling the functions a few times if False is returned, allowing
dependencies between the contents of different entries.

4. GetEntryOffsets() - calls Entry.GetOffsets() for each entry. This can
return a dict containing entries that need updating. The key should be the
entry name and the value is a tuple (offset, size). This allows an entry to
provide the offset and size for other entries. The default implementation
of GetEntryOffsets() returns {}.

5. PackEntries() - calls Entry.Pack() which figures out the offset and
size of an entry. The 'current' image offset is passed in, and the function
returns the offset immediately after the entry being packed. The default
implementation of Pack() is usually sufficient.

Note: for sections, this also checks that the entries do not overlap, nor extend
outside the section. If the section does not have a defined size, the size is
set large enough to hold all the entries. For entries that are explicitly marked
as overlapping, this check is skipped.

6. SetImagePos() - sets the image position of every entry. This is the absolute
position 'image-pos', as opposed to 'offset' which is relative to the containing
section. This must be done after all offsets are known, which is why it is quite
late in the ordering.

7. SetCalculatedProperties() - update any calculated properties in the device
tree. This sets the correct 'offset' and 'size' vaues, for example.

8. ProcessEntryContents() - this calls Entry.ProcessContents() on each entry.
The default implementatoin does nothing. This can be overriden to adjust the
contents of an entry in some way. For example, it would be possible to create
an entry containing a hash of the contents of some other entries. At this
stage the offset and size of entries should not be adjusted unless absolutely
necessary, since it requires a repack (going back to PackEntries()).

9. ResetForPack() - if the ProcessEntryContents() step failed, in that an entry
has changed its size, then there is no alternative but to go back to step 5 and
try again, repacking the entries with the updated size. ResetForPack() removes
the fixed offset/size values added by binman, so that the packing can start from
scratch.

10. WriteSymbols() - write the value of symbols into the U-Boot SPL binary.
See 'Access to binman entry offsets at run time' below for a description of
what happens in this stage.

11. BuildImage() - builds the image and writes it to a file

12. WriteMap() - writes a text file containing a map of the image. This is the
final step.


.. _`External tools`:

External tools
--------------

Binman can make use of external command-line tools to handle processing of
entry contents or to generate entry contents. These tools are executed using
the 'tools' module's Run() method. The tools generally must exist on the PATH,
but the --toolpath option can be used to specify additional search paths to
use. This option can be specified multiple times to add more than one path.

For some compile tools binman will use the versions specified by commonly-used
environment variables like CC and HOSTCC for the C compiler, based on whether
the tool's output will be used for the target or for the host machine. If those
aren't given, it will also try to derive target-specific versions from the
CROSS_COMPILE environment variable during a cross-compilation.

If the tool is not available in the path you can use BINMAN_TOOLPATHS to specify
a space-separated list of paths to search, e.g.::

   BINMAN_TOOLPATHS="/tools/g12a /tools/tegra" binman ...


.. _`External blobs`:

External blobs
--------------

Binary blobs, even if the source code is available, complicate building
firmware. The instructions can involve multiple steps and the binaries may be
hard to build or obtain. Binman at least provides a unified description of how
to build the final image, no matter what steps are needed to get there.

Binman also provides a `blob-ext` entry type that pulls in a binary blob from an
external file. If the file is missing, binman can optionally complete the build
and just report a warning. Use the `-M/--allow-missing` option to enble this.
This is useful in CI systems which want to check that everything is correct but
don't have access to the blobs.

If the blobs are in a different directory, you can specify this with the `-I`
option.

For U-Boot, you can set the BINMAN_INDIRS environment variable to provide a
space-separated list of directories to search for binary blobs::

   BINMAN_INDIRS="odroid-c4/fip/g12a \
       odroid-c4/build/board/hardkernel/odroidc4/firmware \
       odroid-c4/build/scp_task" binman ...

Note that binman fails with exit code 103 when there are missing blobs. If you
wish binman to continue anyway, you can pass `-W` to binman.


Code coverage
-------------

Binman is a critical tool and is designed to be very testable. Entry
implementations target 100% test coverage. Run 'binman test -T' to check this.

To enable Python test coverage on Debian-type distributions (e.g. Ubuntu)::

   $ sudo apt-get install python-coverage python3-coverage python-pytest


Exit status
-----------

Binman produces the following exit codes:

0
    Success

1
    Any sort of failure - see output for more details

103
    There are missing external blobs or bintools. This is only returned if
    -M is passed to binman, otherwise missing blobs return an exit status of 1.
    Note, if -W is passed as well as -M, then this is converted into a warning
    and will return an exit status of 0 instead.


U-Boot environment variables for binman
---------------------------------------

The U-Boot Makefile supports various environment variables to control binman.
All of these are set within the Makefile and result in passing various
environment variables (or make flags) to binman:

BINMAN_DEBUG
    Enables backtrace debugging by adding a `-D` argument. See
    :ref:`BinmanLogging`.

BINMAN_INDIRS
    Sets the search path for input files used by binman by adding one or more
    `-I` arguments. See :ref:`External blobs`.

BINMAN_TOOLPATHS
    Sets the search path for external tool used by binman by adding one or more
    `--toolpath` arguments. See :ref:`External tools`.

BINMAN_VERBOSE
    Sets the logging verbosity of binman by adding a `-v` argument. See
    :ref:`BinmanLogging`.


Error messages
--------------

This section provides some guidance for some of the less obvious error messages
produced by binman.


Expected __bss_size symbol
~~~~~~~~~~~~~~~~~~~~~~~~~~

Example::

   binman: Node '/binman/u-boot-spl-ddr/u-boot-spl/u-boot-spl-bss-pad':
      Expected __bss_size symbol in spl/u-boot-spl

This indicates that binman needs the `__bss_size` symbol to be defined in the
SPL binary, where `spl/u-boot-spl` is the ELF file containing the symbols. The
symbol tells binman the size of the BSS region, in bytes. It needs this to be
able to pad the image so that the following entries do not overlap the BSS,
which would cause them to be overwritte by variable access in SPL.

This symbols is normally defined in the linker script, immediately after
_bss_start and __bss_end are defined, like this::

    __bss_size = __bss_end - __bss_start;

You may need to add it to your linker script if you get this error.


Concurrent tests
----------------

Binman tries to run tests concurrently. This means that the tests make use of
all available CPUs to run.

 To enable this::

   $ sudo apt-get install python-subunit python3-subunit

Use '-P 1' to disable this. It is automatically disabled when code coverage is
being used (-T) since they are incompatible.


Debugging tests
---------------

Sometimes when debugging tests it is useful to keep the input and output
directories so they can be examined later. Use -X or --test-preserve-dirs for
this.


Running tests on non-x86 architectures
--------------------------------------

Binman's tests have been written under the assumption that they'll be run on a
x86-like host and there hasn't been an attempt to make them portable yet.
However, it's possible to run the tests by cross-compiling to x86.

To install an x86 cross-compiler on Debian-type distributions (e.g. Ubuntu)::

  $ sudo apt-get install gcc-x86-64-linux-gnu

Then, you can run the tests under cross-compilation::

  $ CROSS_COMPILE=x86_64-linux-gnu- binman test -T

You can also use gcc-i686-linux-gnu similar to the above.


Writing new entries and debugging
---------------------------------

The behaviour of entries is defined by the Entry class. All other entries are
a subclass of this. An important subclass is Entry_blob which takes binary
data from a file and places it in the entry. In fact most entry types are
subclasses of Entry_blob.

Each entry type is a separate file in the tools/binman/etype directory. Each
file contains a class called Entry_<type> where <type> is the entry type.
New entry types can be supported by adding new files in that directory.
These will automatically be detected by binman when needed.

Entry properties are documented in entry.py. The entry subclasses are free
to change the values of properties to support special behaviour. For example,
when Entry_blob loads a file, it sets content_size to the size of the file.
Entry classes can adjust other entries. For example, an entry that knows
where other entries should be positioned can set up those entries' offsets
so they don't need to be set in the binman decription. It can also adjust
entry contents.

Most of the time such essoteric behaviour is not needed, but it can be
essential for complex images.

If you need to specify a particular device-tree compiler to use, you can define
the DTC environment variable. This can be useful when the system dtc is too
old.

To enable a full backtrace and other debugging features in binman, pass
BINMAN_DEBUG=1 to your build::

   make qemu-x86_defconfig
   make BINMAN_DEBUG=1

To enable verbose logging from binman, base BINMAN_VERBOSE to your build, which
adds a -v<level> option to the call to binman::

   make qemu-x86_defconfig
   make BINMAN_VERBOSE=5


Building sections in parallel
-----------------------------

By default binman uses multiprocessing to speed up compilation of large images.
This works at a section level, with one thread for each entry in the section.
This can speed things up if the entries are large and use compression.

This feature can be disabled with the '-T' flag, which defaults to a suitable
value for your machine. This depends on the Python version, e.g on v3.8 it uses
12 threads on an 8-core machine. See ConcurrentFutures_ for more details.

The special value -T0 selects single-threaded mode, useful for debugging during
development, since dealing with exceptions and problems in threads is more
difficult. This avoids any use of ThreadPoolExecutor.


Collecting data for an entry type
---------------------------------

Some entry types deal with data obtained from others. For example,
`Entry_mkimage` calls the `mkimage` tool with data from its subnodes::

    mkimage {
        args = "-n test -T script";

        u-boot-spl {
        };

        u-boot {
        };
    };

This shows mkimage being passed a file consisting of SPL and U-Boot proper. It
is created by calling `Entry.collect_contents_to_file()`. Note that in this
case, the data is passed to mkimage for processing but does not appear
separately in the image. It may not appear at all, depending on what mkimage
does. The contents of the `mkimage` entry are entirely dependent on the
processing done by the entry, with the provided subnodes (`u-boot-spl` and
`u-boot`) simply providing the input data for that processing.

Note that `Entry.collect_contents_to_file()` simply concatenates the data from
the different entries together, with no control over alignment, etc. Another
approach is to subclass `Entry_section` so that those features become available,
such as `size` and `pad-byte`. Then the contents of the entry can be obtained by
calling `super().BuildSectionData()` in the entry's BuildSectionData()
implementation to get the input data, then write it to a file and process it
however is desired.

There are other ways to obtain data also, depending on the situation. If the
entry type is simply signing data which exists elsewhere in the image, then
you can use `Entry_collection`  as a base class. It lets you use a property
called `content` which lists the entries containing data to be processed. This
is used by `Entry_vblock`, for example::

    u_boot: u-boot {
    };

    vblock {
        content = <&u_boot &dtb>;
        keyblock = "firmware.keyblock";
        signprivate = "firmware_data_key.vbprivk";
        version = <1>;
        kernelkey = "kernel_subkey.vbpubk";
        preamble-flags = <1>;
    };

    dtb: u-boot-dtb {
    };

which shows an image containing `u-boot` and `u-boot-dtb`, with the `vblock`
image collecting their contents to produce input for its signing process,
without affecting those entries, which still appear in the final image
untouched.

Another example is where an entry type needs several independent pieces of input
to function. For example, `Entry_fip` allows a number of different binary blobs
to be placed in their own individual places in a custom data structure in the
output image. To make that work you can add subnodes for each of them and call
`Entry.Create()` on each subnode, as `Entry_fip` does. Then the data for each
blob can come from any suitable place, such as an `Entry_u_boot` or an
`Entry_blob` or anything else::

    atf-fip {
        fip-hdr-flags = /bits/ 64 <0x123>;
        soc-fw {
            fip-flags = /bits/ 64 <0x123456789abcdef>;
            filename = "bl31.bin";
        };

        u-boot {
            fip-uuid = [fc 65 13 92 4a 5b 11 ec
                    94 35 ff 2d 1c fc 79 9c];
        };
    };

The `soc-fw` node is a `blob-ext` (i.e. it reads in a named binary file) whereas
`u-boot` is a normal entry type. This works because `Entry_fip` selects the
`blob-ext` entry type if the node name (here `soc-fw`) is recognised as being
a known blob type.

When adding new entry types you are encouraged to use subnodes to provide the
data for processing, unless the `content` approach is more suitable. Consider
whether the input entries are contained within (or consumed by) the entry, vs
just being 'referenced' by the entry. In the latter case, the `content` approach
makes more sense. Ad-hoc properties and other methods of obtaining data are
discouraged, since it adds to confusion for users.

History / Credits
-----------------

Binman takes a lot of inspiration from a Chrome OS tool called
'cros_bundle_firmware', which I wrote some years ago. That tool was based on
a reasonably simple and sound design but has expanded greatly over the
years. In particular its handling of x86 images is convoluted.

Quite a few lessons have been learned which are hopefully applied here.


Design notes
------------

On the face of it, a tool to create firmware images should be fairly simple:
just find all the input binaries and place them at the right place in the
image. The difficulty comes from the wide variety of input types (simple
flat binaries containing code, packaged data with various headers), packing
requirments (alignment, spacing, device boundaries) and other required
features such as hierarchical images.

The design challenge is to make it easy to create simple images, while
allowing the more complex cases to be supported. For example, for most
images we don't much care exactly where each binary ends up, so we should
not have to specify that unnecessarily.

New entry types should aim to provide simple usage where possible. If new
core features are needed, they can be added in the Entry base class.


To do
-----

Some ideas:

- Use of-platdata to make the information available to code that is unable
  to use device tree (such as a very small SPL image). For now, limited info is
  available via linker symbols
- Allow easy building of images by specifying just the board name
- Support building an image for a board (-b) more completely, with a
  configurable build directory
- Detect invalid properties in nodes
- Sort the fdtmap by offset
- Output temporary files to a different directory
- Rationalise the fdt, fdt_util and pylibfdt modules which currently have some
  overlapping and confusing functionality
- Update the fdt library to use a better format for Prop.value (the current one
  is useful for dtoc but not much else)
- Figure out how to make Fdt support changing the node order, so that
  Node.AddSubnode() can support adding a node before another, existing node.
  Perhaps it should completely regenerate the flat tree?
- Support images which depend on each other

--
Simon Glass <sjg@chromium.org>
7/7/2016

.. _ConcurrentFutures: https://docs.python.org/3/library/concurrent.futures.html#concurrent.futures.ThreadPoolExecutor
