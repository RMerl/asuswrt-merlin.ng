.. SPDX-License-Identifier: GPL-2.0+

Environment Variables
=====================

U-Boot supports user configuration using environment variables which
can be made persistent by saving to persistent storage, for example flash
memory.

Environment variables are set using "env set" (alias "setenv"), printed using
"env print" (alias "printenv"), and saved to persistent storage using
"env save" (alias "saveenv"). Using "env set"
without a value can be used to delete a variable from the
environment. As long as you don't save the environment, you are
working with an in-memory copy. In case the Flash area containing the
environment is erased by accident, a default environment is provided.

See :doc:`cmd/env` for details.

Some configuration is controlled by Environment Variables, so that setting the
variable can adjust the behaviour of U-Boot (e.g. autoboot delay, autoloading
from tftp).

Text-based Environment
----------------------

The default environment for a board is created using a `.env` environment file
using a simple text format. The base filename for this is defined by
`CONFIG_ENV_SOURCE_FILE`, or `CONFIG_SYS_BOARD` if that is empty.

The file must be in the board directory and have a .env extension, so
assuming that there is a board vendor, the resulting filename is therefore::

   board/<vendor>/<board>/<CONFIG_ENV_SOURCE_FILE>.env

or::

   board/<vendor>/<board>/<CONFIG_SYS_BOARD>.env

This is a plain text file where you can type your environment variables in
the form `var=value`. Blank lines and multi-line variables are supported.
The conversion script looks for a line that starts in column 1 with a string
and has an equals sign immediately afterwards. Spaces before the = are not
permitted. It is a good idea to indent your scripts so that only the 'var='
appears at the start of a line.

To add additional text to a variable you can use `var+=value`. This text is
merged into the variable during the make process and made available as a
single value to U-Boot. Variables can contain `+` characters but in the unlikely
event that you want to have a variable name ending in plus, put a backslash
before the `+` so that the script knows you are not adding to an existing
variable but assigning to a new one::

    maximum\+=value

This file can include C-style comments. Blank lines and multi-line
variables are supported, and you can use normal C preprocessor directives
and CONFIG defines from your board config also.

For example, for snapper9260 you would create a text file called
`board/bluewater/snapper9260.env` containing the environment text.

Example::

    stdout=serial
    #ifdef CONFIG_VIDEO
    stdout+=,vidconsole
    #endif
    bootcmd=
        /* U-Boot script for booting */

        if [ -z ${tftpserverip} ]; then
            echo "Use 'setenv tftpserverip a.b.c.d' to set IP address."
        fi

        usb start; setenv autoload n; bootp;
        tftpboot ${tftpserverip}:
        bootm
    failed=
        /* Print a message when boot fails */
        echo CONFIG_SYS_BOARD boot failed - please check your image
        echo Load address is CONFIG_SYS_LOAD_ADDR

Settings which are common to a group of boards can use #include to bring in
a common file in the `include/env` directory, containing environment
settings. For example::

   #include <env/ti/mmc.env>

If CONFIG_ENV_SOURCE_FILE is empty and the default filename is not present, then
the old-style C environment is used instead. See below.

Old-style C environment
-----------------------

Traditionally, the default environment is created in `include/env_default.h`,
and can be augmented by various `CONFIG` defines. See that file for details. In
particular you can define `CFG_EXTRA_ENV_SETTINGS` in your board file
to add environment variables.

Board maintainers are encouraged to migrate to the text-based environment as it
is easier to maintain. The distro-board script still requires the old-style
environments, so use :doc:`../develop/bootstd` instead.


List of environment variables
-----------------------------

Some device configuration options can be set using environment variables. In
many cases the value in the default environment comes from a CONFIG option - see
`include/env_default.h`) for this.

This is most-likely not complete:

autostart
    If set to "yes" (actually any string starting with 1, y, Y, t, or T) an
    image loaded with one of the commands listed below will be automatically
    started by internally invoking the bootm command.

    * bootelf - Boot from an ELF image in memory
    * bootp - boot image via network using BOOTP/TFTP protocol
    * dhcp - boot image via network using DHCP/TFTP protocol
    * diskboot - boot from ide device
    * nboot - boot from NAND device
    * nfs - boot image via network using NFS protocol
    * rarpboot - boot image via network using RARP/TFTP protocol
    * scsiboot - boot from SCSI device
    * tftpboot - boot image via network using TFTP protocol
    * usbboot - boot from USB device

    If the environment variable autostart is not set to a value starting with
    1, y, Y, t, or T, an image passed to the "bootm" command will be copied to
    the load address (and eventually uncompressed), but NOT be started.
    This can be used to load and uncompress arbitrary data.

baudrate
    Used to set the baudrate of the UART - it defaults to CONFIG_BAUDRATE (which
    defaults to 115200).

bootdelay
    Delay before automatically running bootcmd. During this time the user
    can choose to enter the shell (or the boot menu if
    CONFIG_AUTOBOOT_MENU_SHOW=y):

    - 0 to autoboot with no delay, but you can stop it by key input.
    - -1 to disable autoboot.
    - -2 to autoboot with no delay and not check for abort

    The default value is defined by CONFIG_BOOTDELAY.
    The value of 'bootdelay' is overridden by the /config/bootdelay value in
    the device-tree if CONFIG_OF_CONTROL=y.

bootcmd
    The command that is run if the user does not enter the shell during the
    boot delay.

bootargs
    Command line arguments passed when booting an operating system or binary
    image

bootfile
    Name of the image to load with TFTP

bootm_low
    Memory range available for image processing in the bootm
    command can be restricted. This variable is given as
    a hexadecimal number and defines lowest address allowed
    for use by the bootm command. See also "bootm_size"
    environment variable. Address defined by "bootm_low" is
    also the base of the initial memory mapping for the Linux
    kernel -- see the description of CFG_SYS_BOOTMAPSZ and
    bootm_mapsize.

bootm_mapsize
    Size of the initial memory mapping for the Linux kernel.
    This variable is given as a hexadecimal number and it
    defines the size of the memory region starting at base
    address bootm_low that is accessible by the Linux kernel
    during early boot.  If unset, CFG_SYS_BOOTMAPSZ is used
    as the default value if it is defined, and bootm_size is
    used otherwise.

bootm_size
    Memory range available for image processing in the bootm
    command can be restricted. This variable is given as
    a hexadecimal number and defines the size of the region
    allowed for use by the bootm command. See also "bootm_low"
    environment variable.

bootstopkeysha256, bootdelaykey, bootstopkey
    See README.autoboot

button_cmd_0, button_cmd_0_name ... button_cmd_N, button_cmd_N_name
    Used to map commands to run when a button is held during boot.
    See CONFIG_BUTTON_CMD.

updatefile
    Location of the software update file on a TFTP server, used
    by the automatic software update feature. Please refer to
    documentation in doc/README.update for more details.

autoload
    if set to "no" (any string beginning with 'n'),
    "bootp" and "dhcp" will just load perform a lookup of the
    configuration from the BOOTP server, but not try to
    load any image.

fdt_high
    if set this restricts the maximum address that the
    flattened device tree will be copied into upon boot.
    For example, if you have a system with 1 GB memory
    at physical address 0x10000000, while Linux kernel
    only recognizes the first 704 MB as low memory, you
    may need to set fdt_high as 0x3C000000 to have the
    device tree blob be copied to the maximum address
    of the 704 MB low memory, so that Linux kernel can
    access it during the boot procedure.

    If this is set to the special value 0xffffffff (32-bit machines) or
    0xffffffffffffffff (64-bit machines) then
    the fdt will not be copied at all on boot.  For this
    to work it must reside in writable memory, have
    sufficient padding on the end of it for U-Boot to
    add the information it needs into it, and the memory
    must be accessible by the kernel. This usage is strongly discouraged
    however as it also stops U-Boot from ensuring the device tree starting
    address is properly aligned and a misaligned tree will cause OS failures.

fdtcontroladdr
    if set this is the address of the control flattened
    device tree used by U-Boot when CONFIG_OF_CONTROL is
    defined.

initrd_high
    restrict positioning of initrd images:
    If this variable is not set, initrd images will be
    copied to the highest possible address in RAM; this
    is usually what you want since it allows for
    maximum initrd size. If for some reason you want to
    make sure that the initrd image is loaded below the
    CFG_SYS_BOOTMAPSZ limit, you can set this environment
    variable to a value of "no" or "off" or "0".
    Alternatively, you can set it to a maximum upper
    address to use (U-Boot will still check that it
    does not overwrite the U-Boot stack and data).

    For instance, when you have a system with 16 MB
    RAM, and want to reserve 4 MB from use by Linux,
    you can do this by adding "mem=12M" to the value of
    the "bootargs" variable. However, now you must make
    sure that the initrd image is placed in the first
    12 MB as well - this can be done with::

        setenv initrd_high 00c00000

    If you set initrd_high to 0xffffffff (32-bit machines) or
    0xffffffffffffffff (64-bit machines), this is an
    indication to U-Boot that all addresses are legal
    for the Linux kernel, including addresses in flash
    memory. In this case U-Boot will NOT COPY the
    ramdisk at all. This may be useful to reduce the
    boot time on your system, but requires that this
    feature is supported by your Linux kernel. This usage however requires
    that the user ensure that there will be no overlap with other parts of the
    image such as the Linux kernel BSS. It should not be enabled by default
    and only done as part of optimizing a deployment.

ipaddr
    IP address; needed for tftpboot command

loadaddr
    Default load address for commands like "bootp",
    "rarpboot", "tftpboot", "loadb" or "diskboot".  Note that the optimal
    default values here will vary between architectures.  On 32bit ARM for
    example, some offset from start of memory is used as the Linux kernel
    zImage has a self decompressor and it's best if we stay out of where that
    will be working.

loads_echo
    see CONFIG_LOADS_ECHO

serverip
    TFTP server IP address; needed for tftpboot command

bootretry
    see CONFIG_BOOT_RETRY_TIME

bootdelaykey
    see CONFIG_AUTOBOOT_DELAY_STR

bootstopkey
    see CONFIG_AUTOBOOT_STOP_STR

ethprime
    controls which network interface is used first.

ethact
    controls which interface is currently active.
    For example you can do the following::

    => setenv ethact FEC
    => ping 192.168.0.1 # traffic sent on FEC
    => setenv ethact SCC
    => ping 10.0.0.1 # traffic sent on SCC

ethrotate
    When set to "no" U-Boot does not go through all
    available network interfaces.
    It just stays at the currently selected interface. When unset or set to
    anything other than "no", U-Boot does go through all
    available network interfaces.

httpdstp
    If this is set, the value is used for HTTP's TCP
    destination port instead of the default port 80.

netretry
    When set to "no" each network operation will
    either succeed or fail without retrying.
    When set to "once" the network operation will
    fail when all the available network interfaces
    are tried once without success.
    Useful on scripts which control the retry operation
    themselves.

silent_linux
    If set then Linux will be told to boot silently, by
    adding 'console=' to its command line. If "yes" it will be
    made silent. If "no" it will not be made silent. If
    unset, then it will be made silent if the U-Boot console
    is silent.

tftpsrcp
    If this is set, the value is used for TFTP's
    UDP source port.

tftpdstp
    If this is set, the value is used for TFTP's UDP
    destination port instead of the default port 69.

tftpblocksize
    Block size to use for TFTP transfers; if not set,
    we use the TFTP server's default block size

tftptimeout
    Retransmission timeout for TFTP packets (in milli-
    seconds, minimum value is 1000 = 1 second). Defines
    when a packet is considered to be lost so it has to
    be retransmitted. The default is 5000 = 5 seconds.
    Lowering this value may make downloads succeed
    faster in networks with high packet loss rates or
    with unreliable TFTP servers.

tftptimeoutcountmax
    maximum count of TFTP timeouts (no
    unit, minimum value = 0). Defines how many timeouts
    can happen during a single file transfer before that
    transfer is aborted. The default is 10, and 0 means
    'no timeouts allowed'. Increasing this value may help
    downloads succeed with high packet loss rates, or with
    unreliable TFTP servers or client hardware.

tftpwindowsize
    if this is set, the value is used for TFTP's
    window size as described by RFC 7440.
    This means the count of blocks we can receive before
    sending ack to server.

vlan
    When set to a value < 4095 the traffic over
    Ethernet is encapsulated/received over 802.1q
    VLAN tagged frames.

    Note: This appears not to be used in U-Boot. See `README.VLAN`.

bootpretryperiod
    Period during which BOOTP/DHCP sends retries.
    Unsigned value, in milliseconds. If not set, the period will
    be either the default (28000), or a value based on
    CONFIG_NET_RETRY_COUNT, if defined. This value has
    precedence over the value based on CONFIG_NET_RETRY_COUNT.

memmatches
    Number of matches found by the last 'ms' command, in hex

memaddr
    Address of the last match found by the 'ms' command, in hex,
    or 0 if none

mempos
    Index position of the last match found by the 'ms' command,
    in units of the size (.b, .w, .l) of the search

zbootbase
    (x86 only) Base address of the bzImage 'setup' block

zbootaddr
    (x86 only) Address of the loaded bzImage, typically
    BZIMAGE_LOAD_ADDR which is 0x100000


Image locations
---------------

The following image location variables contain the location of images
used in booting. The "Image" column gives the role of the image and is
not an environment variable name. The other columns are environment
variable names. "File Name" gives the name of the file on a TFTP
server, "RAM Address" gives the location in RAM the image will be
loaded to, and "Flash Location" gives the image's address in NOR
flash or offset in NAND flash.

*Note* - these variables don't have to be defined for all boards, some
boards currently use other variables for these purposes, and some
boards use these variables for other purposes.

Also note that most of these variables are just a commonly used set of variable
names, used in some other variable definitions, but are not hard-coded anywhere
in U-Boot code.

================= ============== ================ ==============
Image             File Name      RAM Address      Flash Location
================= ============== ================ ==============
Linux kernel      bootfile       kernel_addr_r    kernel_addr
device tree blob  fdtfile        fdt_addr_r       fdt_addr
ramdisk           ramdiskfile    ramdisk_addr_r   ramdisk_addr
================= ============== ================ ==============

When setting the RAM addresses for `kernel_addr_r`, `fdt_addr_r` and
`ramdisk_addr_r` there are several types of constraints to keep in mind. The
one type of constraint is payload requirement. For example, a device tree MUST
be loaded at an 8-byte aligned address as that is what the specification
requires. In a similar manner, the operating system may define restrictions on
where in memory space payloads can be. This is documented for example in Linux,
with both the `Booting ARM Linux`_ and `Booting AArch64 Linux`_ documents.
Finally, there are practical constraints. We do not know the size of a given
payload a user will use but each payload must not overlap or it will corrupt
the other payload. A similar problem can happen when a payload ends up being in
the OS BSS area. For these reasons we need to ensure our default values here
are both unlikely to lead to failure to boot and sufficiently explained so that
they can be optimized for boot time or adjusted for smaller memory
configurations.

On different architectures we will have different constraints. It is important
that we follow whatever documented requirements are available to best ensure
forward compatibility. What follows are examples to highlight how to provide
reasonable default values in different cases.

Texas Instruments OMAP2PLUS (ARMv7) example
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

On these families of processors we are on a 32bit ARMv7 core.  As booting some
form of Linux is our most common payload we will also keep in mind the
documented requirements for booting that Linux provides.  These values are also
known to be fine for booting a number of other operating systems (or their
loaders).  In this example we define the following variables and values::

    loadaddr=0x82000000
    kernel_addr_r=${loadaddr}
    fdt_addr_r=0x88000000
    ramdisk_addr_r=0x88080000
    bootm_size=0x10000000

The first thing to keep in mind is that DRAM starts at 0x80000000. We set a
32MiB buffer from the start of memory as our default load address and set
``kernel_addr_r`` to that. This is because the Linux ``zImage`` decompressor
will typically then be able to avoid doing a relocation itself. It also MUST be
within the first 128MiB of memory. The next value is we set ``fdt_addr_r`` to
be at 128MiB offset from the start of memory. This location is suggested by the
kernel documentation and is exceedingly unlikely to be overwritten by the
kernel itself given other architectural constraints.  We then allow for the
device tree to be up to 512KiB in size before placing the ramdisk in memory. We
then say that everything should be within the first 256MiB of memory so that
U-Boot can relocate things as needed to ensure proper alignment. We pick 256MiB
as our value here because we know there are very few platforms on in this
family with less memory. It could be as high as 768MiB and still ensure that
everything would be visible to the kernel, but again we go with what we assume
is the safest assumption.

Automatically updated variables
-------------------------------

The following environment variables may be used and automatically
updated by the network boot commands ("bootp" and "rarpboot"),
depending the information provided by your boot server:

=========  ===================================================
Variable   Notes
=========  ===================================================
bootfile   see above
dnsip      IP address of your Domain Name Server
dnsip2     IP address of your secondary Domain Name Server
gatewayip  IP address of the Gateway (Router) to use
hostname   Target hostname
ipaddr     See above
netmask    Subnet Mask
rootpath   Pathname of the root filesystem on the NFS server
serverip   see above
=========  ===================================================


Special environment variables
-----------------------------

There are two special Environment Variables:

serial#
    contains hardware identification information such as type string and/or
    serial number
ethaddr
    Ethernet address. If CONFIG_REGEX=y, also eth*addr (where * is an integer).

These variables can be set only once (usually during manufacturing of
the board). U-Boot refuses to delete or overwrite these variables
once they have been set, unless CONFIG_ENV_OVERWRITE is enabled in the board
configuration.

Also:

ver
    Contains the U-Boot version string as printed
    with the "version" command. This variable is
    readonly (see CONFIG_VERSION_VARIABLE).

Please note that changes to some configuration parameters may take
only effect after the next boot (yes, that's just like Windows).


External environment file
-------------------------

The `CONFIG_USE_DEFAULT_ENV_FILE` option provides a way to bypass the
environment generation in U-Boot. If enabled, then `CONFIG_DEFAULT_ENV_FILE`
provides the name of a file which is converted into the environment,
completely bypassing the standard environment variables in `env_default.h`.

The format is the same as accepted by the mkenvimage tool, with lines containing
key=value pairs. Blank lines and lines beginning with # are ignored.

Future work may unify this feature with the text-based environment, perhaps
moving the contents of `env_default.h` to a text file.

Implementation
--------------

See :doc:`../develop/environment` for internal development details.

.. _`Booting ARM Linux`: https://www.kernel.org/doc/html/latest/arm/booting.html
.. _`Booting AArch64 Linux`: https://www.kernel.org/doc/html/latest/arm64/booting.html
