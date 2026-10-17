.. SPDX-License-Identifier: GPL-2.0+:

U-Boot Standard Boot
====================

Introduction
------------

Standard boot provides a built-in way for U-Boot to automatically boot
an Operating System without custom scripting and other customisation. It
introduces the following concepts:

   - bootdev  - a device which can hold or access a distro (e.g. MMC, Ethernet)
   - bootmeth - a method to scan a bootdev to find bootflows (e.g. distro boot)
   - bootflow - a description of how to boot (provided by the distro)

For Linux, the distro (Linux distribution, e.g. Debian, Fedora) is responsible
for creating a bootflow for each kernel combination that it wants to offer.
These bootflows are stored on media so they can be discovered by U-Boot. This
feature is typically called `distro boot` (see :doc:`distro`) because it is
a way for distributions to boot on any hardware.

Traditionally U-Boot has relied on scripts to implement this feature. See
distro_bootcmd_ for details. This is done because U-Boot has no native support
for scanning devices. While the scripts work remarkably well, they can be hard
to understand and extend, and the feature does not include tests. They are also
making it difficult to move away from ad-hoc CONFIGs, since they are implemented
using the environment and a lot of #defines.

Standard boot is a generalisation of distro boot. It provides a more built-in
way to boot with U-Boot. The feature is extensible to different Operating
Systems (such as Chromium OS) and devices (beyond just block and network
devices). It supports EFI boot and EFI bootmgr too.

Finally, standard boot supports the operation of :doc:`vbe`.

Bootflow
--------

A bootflow is a file that describes how to boot a distro. Conceptually there can
be different formats for that file but at present U-Boot only supports the
BootLoaderSpec_ format. which looks something like this::

   menu autoboot Welcome to Fedora-Workstation-armhfp-31-1.9. Automatic boot in # second{,s}. Press a key for options.
   menu title Fedora-Workstation-armhfp-31-1.9 Boot Options.
   menu hidden

   label Fedora-Workstation-armhfp-31-1.9 (5.3.7-301.fc31.armv7hl)
       kernel /vmlinuz-5.3.7-301.fc31.armv7hl
       append ro root=UUID=9732b35b-4cd5-458b-9b91-80f7047e0b8a rhgb quiet LANG=en_US.UTF-8 cma=192MB cma=256MB
       fdtdir /dtb-5.3.7-301.fc31.armv7hl/
       initrd /initramfs-5.3.7-301.fc31.armv7hl.img

As you can see it specifies a kernel, a ramdisk (initrd) and a directory from
which to load devicetree files. The details are described in distro_bootcmd_.

The bootflow is provided by the distro. It is not part of U-Boot. U-Boot's job
is simply to interpret the file and carry out the instructions. This allows
distros to boot on essentially any device supported by U-Boot.

Typically the first available bootflow is selected and booted. If that fails,
then the next one is tried.


Bootdev
-------

Where does U-Boot find the media that holds the operating systems? That is the
job of bootdev. A bootdev is simply a layer on top of a media device (such as
MMC, NVMe). The bootdev accesses the device, including partitions and
filesystems that might contain things related to an operating system.

For example, an MMC bootdev provides access to the individual partitions on the
MMC device. It scans through these to find filesystems with the boot flag set,
then provides a list of these for consideration.

Some bootdevs are not visible until a bus is enumerated, e.g. flash sticks
attached via USB. To deal with this, each bootdev has an associated 'hunter'
which can hunt for bootdevs of a particular uclass type. For example, the SCSI
bootdev scans the SCSI bus looking for devices, creating a bootdev for each
Logical Unit Number (LUN) that it finds.


Bootmeth
--------

Once the list of filesystems is provided, how does U-Boot find the bootflow
files in these filesystems. That is the job of bootmeth. Each boot method has
its own way of doing this.

For example, the distro bootmeth simply looks through the provided filesystem
for a file called `extlinux/extlinux.conf`. This files constitutes a bootflow.
If the distro bootmeth is used on multiple partitions it may produce multiple
bootflows.

Note: it is possible to have a bootmeth that uses a partition or a whole device
directly, but it is more common to use a filesystem.

Note that some bootmeths are 'global', meaning that they select the bootdev
themselves. Examples include VBE and EFI boot manager. In this case, they
provide a `read_bootflow()` method which checks whatever bootdevs it likes, then
returns the bootflow, if found. Some of these bootmeths may be very slow, if
they scan a lot of devices.


Boot process
------------

U-Boot tries to use the 'lazy init' approach whereever possible and distro boot
is no exception. The algorithm is::

   while (get next bootdev)
      while (get next bootmeth)
          while (get next bootflow)
              try to boot it

So U-Boot works its way through the bootdevs, trying each bootmeth in turn to
obtain bootflows, until it either boots or exhausts the available options.

Instead of 500 lines of #defines and a 4KB boot script, all that is needed is
the following command::

   bootflow scan -lb

which scans for available bootflows, optionally listing each find it finds (-l)
and trying to boot it (-b).

When global bootmeths are available, these are typically checked before the
above bootdev scanning.


Controlling ordering
--------------------

By default, faster bootdevs (or those which are assumed to be faster) are used
first, since they are more likely to be able to boot the device quickly.

Several options are available to control the ordering of boot scanning:


boot_targets
~~~~~~~~~~~~

This environment variable can be used to control the list of bootdevs searched
and their ordering, for example::

   setenv boot_targets "mmc0 mmc1 usb pxe"

Entries may be removed or re-ordered in this list to affect the boot order. If
the variable is empty, the default ordering is used, based on the priority of
bootdevs and their sequence numbers.


bootmeths
~~~~~~~~~

By default bootmeths are checked in name order. Use `bootmeth list` to see the
ordering. Note that the `extlinux` and `script` bootmeth is first, to preserve the behaviour
used by the old distro scripts.

This environment variable can be used to control the list of bootmeths used and
their ordering for example::

   setenv bootmeths "extlinux efi"

Entries may be removed or re-ordered in this list to affect the order the
bootmeths are tried on each bootdev. If the variable is empty, the default
ordering is used, based on the bootmeth sequence numbers, which can be
controlled by aliases.

The :ref:`usage/cmd/bootmeth:bootmeth command` (`bootmeth order`) operates in
the same way as setting this variable.

Bootdev uclass
--------------

The bootdev uclass provides an simple API call to obtain a bootflows from a
device::

   int bootdev_get_bootflow(struct udevice *dev, struct bootflow_iter *iter,
                            struct bootflow *bflow);

This takes a iterator which indicates the bootdev, partition and bootmeth to
use. It returns a bootflow. This is the core of the bootdev implementation. The
bootdev drivers that implement this differ depending on the media they are
reading from, but each is responsible for returning a valid bootflow if
available.

A helper called `bootdev_find_in_blk()` makes it fairly easy to implement this
function for each media device uclass, in a few lines of code. For many types
ot bootdevs, the `get_bootflow` member can be NULL, indicating that the default
handler is used. This is called `default_get_bootflow()` and it only works with
block devices.


Bootdev drivers
---------------

A bootdev driver is typically fairly simple. Here is one for mmc::

    static int mmc_bootdev_bind(struct udevice *dev)
    {
        struct bootdev_uc_plat *ucp = dev_get_uclass_plat(dev);

        ucp->prio = BOOTDEVP_2_INTERNAL_FAST;

        return 0;
    }

    struct bootdev_ops mmc_bootdev_ops = {
    };

    static const struct udevice_id mmc_bootdev_ids[] = {
        { .compatible = "u-boot,bootdev-mmc" },
        { }
    };

    U_BOOT_DRIVER(mmc_bootdev) = {
        .name        = "mmc_bootdev",
        .id        = UCLASS_BOOTDEV,
        .ops        = &mmc_bootdev_ops,
        .bind        = mmc_bootdev_bind,
        .of_match    = mmc_bootdev_ids,
    };

You may notice that the `get_bootflow` memory is not provided, so is NULL. This
means that `default_get_bootflow()` is used. This simply obtains the
block device and calls a bootdev helper function to do the rest. The
implementation of `bootdev_find_in_blk()` checks the partition table, and
attempts to read a file from a filesystem on the partition number given by the
`@iter->part` parameter. If there are any bootable partitions in the table,
then only bootable partitions are considered.

Each bootdev has a priority, which indicates the order in which it is used,
if `boot_targets` is not used. Faster bootdevs are used first, since they are
more likely to be able to boot the device quickly.


Environment Variables
---------------------

Various environment variables are used by standard boot. These allow the board
to control where things are placed when booting the OS. You should ensure that
your boards sets values for these.

fdtfile
    Name of the flattened device tree (FDT) file to load, e.g.
    "rockchip/rk3399-rockpro64.dtb"

fdt_addr_r
    Address at which to load the FDT, e.g. 0x01f00000

fdtoverlay_addr_r (needed if overlays are used)
    Address at which to load the overlay for the FDT, e.g. 0x02000000

kernel_addr_r
    Address at which to load the kernel, e.g. 0x02080000

kernel_comp_addr_r
    Address to which to decompress the kernel, e.g. 0x08000000

kernel_comp_size
    Size of available space for decompressed kernel, e.g. 0x2000000

pxefile_addr_r
    Address at which to load the PXE file, e.g. 0x00600000

ramdisk_addr_r
    Address at which to load the ramdisk, e.g. 0x06000000

scriptaddr
    Address at which to load the U-Boot script, e.g. 0x00500000

script_offset_f
    SPI flash offset from which to load the U-Boot script, e.g. 0xffe000

script_size_f
    Size of the script to load, e.g. 0x2000

Some variables are set by script bootmeth:

devtype
    Device type being used for boot, e.g. mmc

devnum
    Device number being used for boot, e.g. 1

distro_bootpart
    Partition being used for boot, e.g. 2

prefix
    Directory containing the script

mmc_bootdev
    Device number being used for boot (e.g. 1). This is only used by MMC on
    sunxi boards.


Device hierarchy
----------------

A bootdev device is a child of the media device. In this example, you can see
that the bootdev is a sibling of the block device and both are children of
media device::

    mmc           0  [ + ]   bcm2835-sdhost        |   |-- mmc@7e202000
    blk           0  [ + ]   mmc_blk               |   |   |-- mmc@7e202000.blk
    bootdev       0  [   ]   mmc_bootdev           |   |   `-- mmc@7e202000.bootdev
    mmc           1  [ + ]   sdhci-bcm2835         |   |-- sdhci@7e300000
    blk           1  [   ]   mmc_blk               |   |   |-- sdhci@7e300000.blk
    bootdev       1  [   ]   mmc_bootdev           |   |   `-- sdhci@7e300000.bootdev

The bootdev device is typically created automatically in the media uclass'
`post_bind()` method by calling `bootdev_setup_for_dev()` or
`bootdev_setup_for_sibling_blk()`. The code typically something like this::

    /* dev is the Ethernet device */
    ret = bootdev_setup_for_dev(dev, "eth_bootdev");
    if (ret)
        return log_msg_ret("bootdev", ret);

or::

    /* blk is the block device (child of MMC device)
    ret = bootdev_setup_for_sibling_blk(blk, "mmc_bootdev");
    if (ret)
        return log_msg_ret("bootdev", ret);


Here, `eth_bootdev` is the name of the Ethernet bootdev driver and `dev`
is the ethernet device. This function is safe to call even if standard boot is
not enabled, since it does nothing in that case. It can be added to all uclasses
which implement suitable media.


The bootstd device
------------------

Standard boot requires a single instance of the bootstd device to make things
work. This includes global information about the state of standard boot. See
`struct bootstd_priv` for this structure, accessed with `bootstd_get_priv()`.

Within the devicetree, if you add bootmeth devices, they should be children of
the bootstd device. See `arch/sandbox/dts/test.dts` for an example of this.


.. _`Automatic Devices`:

Automatic devices
-----------------

It is possible to define all the required devices in the devicetree manually,
but it is not necessary. The bootstd uclass includes a `dm_scan_other()`
function which creates the bootstd device if not found. If no bootmeth devices
are found at all, it creates one for each available bootmeth driver.

If your devicetree has any bootmeth device it must have all of them that you
want to use, since no bootmeth devices will be created automatically in that
case.


Using devicetree
----------------

If a bootdev is complicated or needs configuration information, it can be
added to the devicetree as a child of the media device. For example, imagine a
bootdev which reads a bootflow from SPI flash. The devicetree fragment might
look like this::

    spi@0 {
        flash@0 {
            reg = <0>;
            compatible = "spansion,m25p16", "jedec,spi-nor";
            spi-max-frequency = <40000000>;

            bootdev {
                compatible = "u-boot,sf-bootdev";
                offset = <0x2000>;
                size = <0x1000>;
            };
        };
    };

The `sf-bootdev` driver can implement a way to read from the SPI flash, using
the offset and size provided, and return that bootflow file back to the caller.
When distro boot wants to read the kernel it calls distro_getfile() which must
provide a way to read from the SPI flash. See `distro_boot()` at distro_boot_
for more details.

Of course this is all internal to U-Boot. All the distro sees is another way
to boot.


Configuration
-------------

Standard boot is enabled with `CONFIG_BOOTSTD`. Each bootmeth has its own CONFIG
option also. For example, `CONFIG_BOOTMETH_EXTLINUX` enables support for
booting from a disk using an `extlinux.conf` file.

To enable all feature sof standard boot, use `CONFIG_BOOTSTD_FULL`. This
includes the full set of commands, more error messages when things go wrong and
bootmeth ordering with the bootmeths environment variable.

You should probably also enable `CONFIG_BOOTSTD_DEFAULTS`, which provides
several filesystem and network features (if `CONFIG_NET` is enabled) so that
a good selection of boot options is available.


Available bootmeth drivers
--------------------------

Bootmeth drivers are provided for:

   - extlinux / syslinux boot from a disk
   - extlinux boot from a network (PXE)
   - U-Boot scripts from disk, network or SPI flash
   - EFI boot using bootefi from disk
   - VBE
   - EFI boot using boot manager


Command interface
-----------------

Three commands are available:

`bootdev`
    Allows listing of available bootdevs, selecting a particular one and
    getting information about it. See :doc:`../usage/cmd/bootdev`

`bootflow`
    Allows scanning one or more bootdevs for bootflows, listing available
    bootflows, selecting one, obtaining information about it and booting it.
    See :doc:`../usage/cmd/bootflow`

`bootmeth`
    Allow listing of available bootmethds and setting the order in which they
    are tried. See :doc:`../usage/cmd/bootmeth`

.. _BootflowStates:

Bootflow states
---------------

Here is a list of states that a bootflow can be in:

=======  =======================================================================
State    Meaning
=======  =======================================================================
base     Starting-out state, indicates that no media/partition was found. For an
         SD card socket it may indicate that the card is not inserted.
media    Media was found (e.g. SD card is inserted) but no partition information
         was found. It might lack a partition table or have a read error.
part     Partition was found but a filesystem could not be read. This could be
         because the partition does not hold a filesystem or the filesystem is
         very corrupted.
fs       Filesystem was found but the file could not be read. It could be
         missing or in the wrong subdirectory.
file     File was found and its size detected, but it could not be read. This
         could indicate filesystem corruption.
ready    File was loaded and is ready for use. In this state the bootflow is
         ready to be booted.
=======  =======================================================================


Migrating from distro_boot
--------------------------

To migrate from distro_boot:

#. Update your board header files to remove the BOOTENV and BOOT_TARGET_xxx
   defines. Standard boot finds available boot devices automatically.

#. Remove the "boot_targets" variable unless you need it. Standard boot uses a
   default order from fastest to slowest, which generally matches the order used
   by boards.

#. Make sure that CONFIG_BOOTSTD_DEFAULTS is enabled by your board, so it can
   boot common Linux distributions.

An example patch is at migrate_patch_.

If you are using custom boot scripts for your board, consider creating your
own bootmeth to hold the logic. There are various examples at
`boot/bootmeth_...`.


Theory of operation
-------------------

This describes how standard boot progresses through to booting an operating
system.

To start. all the necessary devices must be bound, including bootstd, which
provides the top-level `struct bootstd_priv` containing optional configuration
information. The bootstd device is also holds the various lists used while
scanning. This step is normally handled automatically by driver model, as
described in `Automatic Devices`_.

Bootdevs are also required, to provide access to the media to use. These are not
useful by themselves: bootmeths are needed to provide the means of scanning
those bootdevs. So, all up, we need a single bootstd device, one or more bootdev
devices and one or more bootmeth devices.

Once these are ready, typically a `bootflow scan` command is issued. This kicks
of the iteration process, which involves hunting for bootdevs and looking
through the bootdevs and their partitions one by one to find bootflows.

Iteration is kicked off using `bootflow_scan_first()`.

The iterator is set up with `bootflow_iter_init()`. This simply creates an
empty one with the given flags. Flags are used to control whether each
iteration is displayed, whether to return iterations even if they did not result
in a valid bootflow, whether to iterate through just a single bootdev, etc.

Then the iterator is set up to according to the parameters given:

- When `dev` is provided, then a single bootdev is scanned. In this case,
  `BOOTFLOWIF_SKIP_GLOBAL` and `BOOTFLOWIF_SINGLE_DEV` are set. No hunters are
  used in this case

- Otherwise, when `label` is provided, then a single label or named bootdev is
  scanned. In this case `BOOTFLOWIF_SKIP_GLOBAL` is set and there are three
  options (with an effect on the `iter_incr()` function described later):

  - If `label` indicates a numeric bootdev number (e.g. "2") then
    `BOOTFLOW_METHF_SINGLE_DEV` is set. In this case, moving to the next bootdev
    simple stops, since there is only one. No hunters are used.
  - If `label` indicates a particular media device (e.g. "mmc1") then
    `BOOTFLOWIF_SINGLE_MEDIA` is set. In this case, moving to the next bootdev
    processes just the children of the media device. Hunters are used, in this
    example just the "mmc" hunter.
  - If `label` indicates a media uclass (e.g. "mmc") then
    `BOOTFLOWIF_SINGLE_UCLASS` is set. In this case, all bootdevs in that uclass
    are used. Hunters are used, in this example just the "mmc" hunter

- Otherwise, none of the above flags is set and iteration is set up to work
  through `boot_targets` environment variable (or `bootdev-order` device tree
  property) in order, running the relevant hunter first. In this case
  `cur_label` is used to indicate the label being processed. If there is no list
  of labels, then all bootdevs are processed in order of priority, running the
  hunters as it goes.

With the above it is therefore possible to iterate in a variety of ways.

No attempt is made to determine the ordering of bootdevs, since this cannot be
known in advance if we are using the hunters. Any hunter might discover a new
bootdev and disturb the original ordering.

Next, the ordering of bootmeths is determined, by `bootmeth_setup_iter_order()`.
By default the ordering is again by sequence number, i.e. the `/aliases` node,
or failing that the order in the devicetree. But the `bootmeth order` command
or `bootmeths` environment variable can be used to set up an ordering. If that
has been done, the ordering is in `struct bootstd_priv`, so that ordering is
simply copied into the iterator. Either way, the `method_order` array it set up,
along with `num_methods`.

Note that global bootmeths are always put at the end of the ordering. If any are
present, `cur_method` is set to the first one, so that global bootmeths are done
first. Once all have been used, these bootmeths are dropped from the iteration.
When there are no global bootmeths, `cur_method` is set to 0.

At this point the iterator is ready to use, with the first bootmeth selected.
Most of the other fields are 0. This means that the current partition
is 0, which is taken to mean the whole device, since partition numbers start at
1. It also means that `max_part` is 0, i.e. the maximum partition number we know
about is 0, meaning that, as far as we know, there is no partition table on this
bootdev.

With the iterator ready, `bootflow_scan_first()` checks whether the current
settings produce a valid bootflow. This is handled by `bootflow_check()`, which
either returns 0 (if it got something) or an error if not (more on that later).
If the `BOOTFLOWIF_ALL` iterator flag is set, even errors are returned as
incomplete bootflows, but normally an error results in moving onto the next
iteration.

Note that `bootflow_check()` handles global bootmeths explicitly, by calling
`bootmeth_get_bootflow()` on each one. The `doing_global` flag indicates when
the iterator is in that state.

The `bootflow_scan_next()` function handles moving onto the next iteration and
checking it. In fact it sits in a loop doing that repeatedly until it finds
something it wants to return.

The actual 'moving on' part is implemented in `iter_incr()`. This is a fairly
simple function. It increments the first counter. If that hits its maximum, it
sets it to zero and increments the second counter. You can think of all the
counters together as a number with three digits which increment in order, with
the least-sigificant digit on the right, counting like this:

   ========    =======    =======
   bootdev     part       method
   ========    =======    =======
   0           0          0
   0           0          1
   0           0          2
   0           1          0
   0           1          1
   0           1          2
   1           0          0
   1           0          1
   ...
   ========    =======    =======

The maximum value for `method` is `num_methods - 1` so when it exceeds that, it
goes back to 0 and the next `part` is considered. The maximum value for that is
`max_part`, which is initially zero for all bootdevs. If we find a partition
table on that bootdev, `max_part` can be updated during the iteration to a
higher value - see `bootdev_find_in_blk()` for that, described later. If that
exceeds its maximum, then the next bootdev is used. In this way, iter_incr()
works its way through all possibilities, moving forward one each time it is
called.

Note that global bootmeths introduce a subtlety into the above description.
When `doing_global` is true, the iteration takes place only among the bootmeths,
i.e. the last column above. The global bootmeths are at the end of the list.
Assuming that they are entries 3 and 4 in the list, the iteration then looks
like this:

   ========    =======    =======   =======================================
   bootdev     part       method    notes
   ========    =======    =======   =======================================
   .           .          3         doing_global = true, method_count = 5
   .           .          4
   0           0          0         doing_global = false, method_count = 3
   0           0          1
   0           0          2
   0           1          0
   0           1          1
   0           1          2
   1           0          0
   1           0          1
   ...
   ========    =======    =======   =======================================

The changeover of the value of `doing_global` from true to false is handled in
`iter_incr()` as well.

Note that the value in the `bootdev` column above is not actually stored - it is
just for illustration. In practice, `iter_incr()` uses the flags to determine
whether to move to the next bootdev in the uclass, the next child of the media
device, the next label, or the next priority level, depending on the flag
settings (see `BOOTFLOW_METHF_SINGLE_DEV`, etc. above).

There is no expectation that iteration will actually finish. Quite often a
valid bootflow is found early on. With `bootflow scan -b`, that causes the
bootflow to be immediately booted. Assuming it is successful, the iteration never
completes.

Also note that the iterator hold the **current** combination being considered.
So when `iter_incr()` is called, it increments to the next one and returns it,
the new **current** combination.

Note also the `err` field in `struct bootflow_iter`. This is normally 0 and has
thus has no effect on `iter_inc()`. But if it is non-zero, signalling an error,
it indicates to the iterator what it should do when called. It can force moving
to the next partition, or bootdev, for example. The special values
`BF_NO_MORE_PARTS` and `BF_NO_MORE_DEVICES` handle this. When `iter_incr` sees
`BF_NO_MORE_PARTS` it knows that it should immediately move to the next bootdev.
When it sees `BF_NO_MORE_DEVICES` it knows that there is nothing more it can do
so it should immediately return. The caller of `iter_incr()` is responsible for
updating the `err` field, based on the return value it sees.

The above describes the iteration process at a high level. It is basically a
very simple increment function with a checker called `bootflow_check()` that
checks the result of each iteration generated, to determine whether it can
produce a bootflow.

So what happens inside of `bootflow_check()`? It simply calls the uclass
method `bootdev_get_bootflow()` to ask the bootdev to return a bootflow. It
passes the iterator to the bootdev method, so that function knows what we are
talking about. At first, the bootflow is set up in the state `BOOTFLOWST_BASE`,
with just the `method` and `dev` intiialised. But the bootdev may fill in more,
e.g. updating the state, depending on what it finds. For global bootmeths the
`bootmeth_get_bootflow()` function is called instead of
`bootdev_get_bootflow()`.

Based on what the bootdev or bootmeth responds with, `bootflow_check()` either
returns a valid bootflow, or a partial one with an error. A partial bootflow
is one that has some fields set up, but did not reach the `BOOTFLOWST_READY`
state. As noted before, if the `BOOTFLOWIF_ALL` iterator flag is set, then all
bootflows are returned, even partial ones. This can help with debugging.

So at this point you can see that total control over whether a bootflow can
be generated from a particular iteration, or not, rests with the bootdev (or
global bootmeth). Each one can adopt its own approach.

Going down a level, what does the bootdev do in its `get_bootflow()` method?
Let us consider the MMC bootdev. In that case the call to
`bootdev_get_bootflow()` ends up in `default_get_bootflow()`. It locates the
parent device of the bootdev, i.e. the `UCLASS_MMC` device itself, then finds
the block device associated with it. It then calls the helper function
`bootdev_find_in_blk()` to do all the work. This is common with just about any
bootdev that is based on a media device.

The `bootdev_find_in_blk()` helper is implemented in the bootdev uclass. It
names the bootflow and copies the partition number in from the iterator. Then it
calls the bootmeth device to check if it can support this device. This is
important since some bootmeths only work with network devices, for example. If
that check fails, it stops.

Assuming the bootmeth is happy, or at least indicates that it is willing to try
(by returning 0 from its `check()` method), the next step is to try the
partition. If that works it tries to detect a file system. If that works then it
calls the bootmeth device once more, this time to read the bootflow.

Note: Normally a filesystem is needed for the bootmeth to be called on block
devices, but bootmeths which don't need that can set the BOOTMETHF_ANY_PART
flag to indicate that they can scan any partition. An example is the ChromiumOS
bootmeth which can store a kernel in a raw partition. Note also that sandbox is
a special case, since in that case the host filesystem can be accessed even
though the block device is NULL.

If we take the example of the `bootmeth_extlinux` driver, this call ends up at
`extlinux_read_bootflow()`. It has the filesystem ready, so tries various
filenames to try to find the `extlinux.conf` file, reading it if possible. If
all goes well the bootflow ends up in the `BOOTFLOWST_READY` state.

At this point, we fall back from the bootmeth driver, to
`bootdev_find_in_blk()`, then back to `default_get_bootflow()`, then to
`bootdev_get_bootflow()`, then to `bootflow_check()` and finally to its caller,
either `bootflow_scan_first()` or `bootflow_scan_next()`. In either case,
the bootflow is returned as the result of this iteration, assuming it made it to
the  `BOOTFLOWST_READY` state.

That is the basic operation of scanning for bootflows. The process of booting a
bootflow is handled by the bootmeth driver for that bootflow. In the case of
extlinux boot, this parses and processes the `extlinux.conf` file that was read.
See `extlinux_boot()` for how that works. The processing may involve reading
additional files, which is handled by the `read_file()` method, which is
`extlinux_read_file()` in this case. All bootmethds should support reading
files, since the bootflow is typically only the basic instructions and does not
include the operating system itself, ramdisk, device tree, etc.

The vast majority of the bootstd code is concerned with iterating through
partitions on bootdevs and using bootmethds to find bootflows.

How about bootdevs which are not block devices? They are handled by the same
methods as above, but with a different implementation. For example, the bootmeth
for PXE boot (over a network) uses `tftp` to read files rather than `fs_read()`.
But other than that it is very similar.


Tests
-----

Tests are located in `test/boot` and cover the core functionality as well as
the commands. All tests use sandbox so can be run on a standard Linux computer
and in U-Boot's CI.

For testing, a DOS-formatted disk image is used with a FAT partition on it and
a second unused partition. This is created in `setup_bootflow_image()`, with a
canned one from the source tree used if it cannot be created (e.g. in CI).


Bootflow internals
------------------

The bootstd device holds a linked list of scanned bootflows as well as the
currently selected bootdev and bootflow (for use by commands). This is in
`struct bootstd_priv`.

Each bootdev device has its own `struct bootdev_uc_plat` which holds a
list of scanned bootflows just for that device.

The bootflow itself is documented in bootflow_h_. It includes various bits of
information about the bootflow and a buffer to hold the file.


Future
------

Apart from the to-do items below, different types of bootflow files may be
implemented in future, e.g. Chromium OS support which is currently only
available as a script in chromebook_coral.


To do
-----

Some things that need to be done to completely replace the distro-boot scripts:

- implement extensions (devicetree overlays with add-on boards)

Other ideas:

- `bootflow prep` to load everything preparing for boot, so that `bootflow boot`
  can just do the boot.
- automatically load kernel, FDT, etc. to suitable addresses so the board does
  not need to specify things like `pxefile_addr_r`


.. _distro_bootcmd: https://github.com/u-boot/u-boot/blob/master/include/config_distro_bootcmd.h
.. _BootLoaderSpec: http://www.freedesktop.org/wiki/Specifications/BootLoaderSpec/
.. _distro_boot: https://github.com/u-boot/u-boot/blob/master/boot/distro.c
.. _bootflow_h: https://github.com/u-boot/u-boot/blob/master/include/bootflow.h
.. _migrate_patch: https://patchwork.ozlabs.org/project/uboot/patch/20230727215433.578830-2-sjg@chromium.org/
