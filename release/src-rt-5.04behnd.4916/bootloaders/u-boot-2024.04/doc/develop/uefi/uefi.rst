.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (c) 2018 Heinrich Schuchardt

UEFI on U-Boot
==============

The Unified Extensible Firmware Interface Specification (UEFI) [1] has become
the default for booting on AArch64 and x86 systems. It provides a stable API for
the interaction of drivers and applications with the firmware. The API comprises
access to block storage, network, and console to name a few. The Linux kernel
and boot loaders like GRUB or the FreeBSD loader can be executed.

Development target
------------------

The implementation of UEFI in U-Boot strives to reach the requirements described
in the "Embedded Base Boot Requirements (EBBR) Specification - Release v2.1.0"
[2]. The "Server Base Boot Requirements System Software on ARM Platforms" [3]
describes a superset of the EBBR specification and may be used as further
reference.

A full blown UEFI implementation would contradict the U-Boot design principle
"keep it small".

Building U-Boot for UEFI
------------------------

The UEFI standard supports only little-endian systems. The UEFI support can be
activated for ARM and x86 by specifying::

    CONFIG_CMD_BOOTEFI=y
    CONFIG_EFI_LOADER=y

in the .config file.

Support for attaching virtual block devices, e.g. iSCSI drives connected by the
loaded UEFI application [4], requires::

    CONFIG_BLK=y
    CONFIG_PARTITIONS=y

Executing a UEFI binary
~~~~~~~~~~~~~~~~~~~~~~~

The bootefi command is used to start UEFI applications or to install UEFI
drivers. It takes two parameters::

    bootefi <image address> [fdt address]

* image address - the memory address of the UEFI binary
* fdt address - the memory address of the flattened device tree

Below you find the output of an example session starting GRUB::

    => load mmc 0:2 ${fdt_addr_r} boot/dtb
    29830 bytes read in 14 ms (2 MiB/s)
    => load mmc 0:1 ${kernel_addr_r} efi/debian/grubaa64.efi
    reading efi/debian/grubaa64.efi
    120832 bytes read in 7 ms (16.5 MiB/s)
    => bootefi ${kernel_addr_r} ${fdt_addr_r}

When booting from a memory location it is unknown from which file it was loaded.
Therefore the bootefi command uses the device path of the block device partition
or the network adapter and the file name of the most recently loaded PE-COFF
file when setting up the loaded image protocol.

Launching a UEFI binary from a FIT image
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A signed FIT image can be used to securely boot a UEFI image via the
bootm command. This feature is available if U-Boot is configured with::

    CONFIG_BOOTM_EFI=y

A sample configuration is provided as file doc/uImage.FIT/uefi.its.

Below you find the output of an example session starting GRUB::

    => load mmc 0:1 ${kernel_addr_r} image.fit
    4620426 bytes read in 83 ms (53.1 MiB/s)
    => bootm ${kernel_addr_r}#config-grub-nofdt
    ## Loading kernel from FIT Image at 40400000 ...
       Using 'config-grub-nofdt' configuration
       Verifying Hash Integrity ... sha256,rsa2048:dev+ OK
       Trying 'efi-grub' kernel subimage
         Description:  GRUB EFI Firmware
         Created:      2019-11-20   8:18:16 UTC
         Type:         Kernel Image (no loading done)
         Compression:  uncompressed
         Data Start:   0x404000d0
         Data Size:    450560 Bytes = 440 KiB
         Hash algo:    sha256
         Hash value:   4dbee00021112df618f58b3f7cf5e1595533d543094064b9ce991e8b054a9eec
       Verifying Hash Integrity ... sha256+ OK
       XIP Kernel Image (no loading done)
    ## Transferring control to EFI (at address 404000d0) ...
    Welcome to GRUB!

See doc/uImage.FIT/howto.txt for an introduction to FIT images.

Configuring UEFI secure boot
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The UEFI specification[1] defines a secure way of executing UEFI images
by verifying a signature (or message digest) of image with certificates.
This feature on U-Boot is enabled with::

    CONFIG_EFI_SECURE_BOOT=y

To make the boot sequence safe, you need to establish a chain of trust;
In UEFI secure boot the chain trust is defined by the following UEFI variables

* PK - Platform Key
* KEK - Key Exchange Keys
* db - white list database
* dbx - black list database

An in depth description of UEFI secure boot is beyond the scope of this
document. Please, refer to the UEFI specification and available online
documentation. Here is a simple example that you can follow for your initial
attempt (Please note that the actual steps will depend on your system and
environment.):

Install the required tools on your host

* openssl
* efitools
* sbsigntool

Create signing keys and the key database on your host:

The platform key

.. code-block:: bash

    openssl req -x509 -sha256 -newkey rsa:2048 -subj /CN=TEST_PK/ \
            -keyout PK.key -out PK.crt -nodes -days 365
    cert-to-efi-sig-list -g 11111111-2222-3333-4444-123456789abc \
            PK.crt PK.esl;
    sign-efi-sig-list -c PK.crt -k PK.key PK PK.esl PK.auth

The key exchange keys

.. code-block:: bash

    openssl req -x509 -sha256 -newkey rsa:2048 -subj /CN=TEST_KEK/ \
            -keyout KEK.key -out KEK.crt -nodes -days 365
    cert-to-efi-sig-list -g 11111111-2222-3333-4444-123456789abc \
            KEK.crt KEK.esl
    sign-efi-sig-list -c PK.crt -k PK.key KEK KEK.esl KEK.auth

The whitelist database

.. code-block:: bash

    openssl req -x509 -sha256 -newkey rsa:2048 -subj /CN=TEST_db/ \
            -keyout db.key -out db.crt -nodes -days 365
    cert-to-efi-sig-list -g 11111111-2222-3333-4444-123456789abc \
            db.crt db.esl
    sign-efi-sig-list -c KEK.crt -k KEK.key db db.esl db.auth

Copy the \*.auth files to media, say mmc, that is accessible from U-Boot.

Sign an image with one of the keys in "db" on your host

.. code-block:: bash

    sbsign --key db.key --cert db.crt helloworld.efi

Now in U-Boot install the keys on your board::

    fatload mmc 0:1 <tmpaddr> PK.auth
    setenv -e -nv -bs -rt -at -i <tmpaddr>:$filesize PK
    fatload mmc 0:1 <tmpaddr> KEK.auth
    setenv -e -nv -bs -rt -at -i <tmpaddr>:$filesize KEK
    fatload mmc 0:1 <tmpaddr> db.auth
    setenv -e -nv -bs -rt -at -i <tmpaddr>:$filesize db

Set up boot parameters on your board::

    efidebug boot add -b 1 HELLO mmc 0:1 /helloworld.efi.signed ""

Since kernel 5.7 there's an alternative way of loading an initrd using
LoadFile2 protocol if CONFIG_EFI_LOAD_FILE2_INITRD is enabled.
The initrd path can be specified with::

    efidebug boot add -b ABE0 'kernel' mmc 0:1 Image -i mmc 0:1 initrd

Now your board can run the signed image via the boot manager (see below).
You can also try this sequence by running Pytest, test_efi_secboot,
on the sandbox

.. code-block:: bash

    cd <U-Boot source directory>
    pytest test/py/tests/test_efi_secboot/test_signed.py --bd sandbox

UEFI binaries may be signed by Microsoft using the following certificates:

* KEK: Microsoft Corporation KEK CA 2011
  http://go.microsoft.com/fwlink/?LinkId=321185.
* db: Microsoft Windows Production PCA 2011
  http://go.microsoft.com/fwlink/p/?linkid=321192.
* db: Microsoft Corporation UEFI CA 2011
  http://go.microsoft.com/fwlink/p/?linkid=321194.

Using OP-TEE for EFI variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Instead of implementing UEFI variable services inside U-Boot they can
also be provided in the secure world by a module for OP-TEE[1]. The
interface between U-Boot and OP-TEE for variable services is enabled by
CONFIG_EFI_MM_COMM_TEE=y.

Tianocore EDK II's standalone management mode driver for variables can
be linked to OP-TEE for this purpose. This module uses the Replay
Protected Memory Block (RPMB) of an eMMC device for persisting
non-volatile variables. When calling the variable services via the
OP-TEE API U-Boot's OP-TEE supplicant relays calls to the RPMB driver
which has to be enabled via CONFIG_SUPPORT_EMMC_RPMB=y.

EDK2 Build instructions
***********************

.. code-block:: bash

    $ git clone https://github.com/tianocore/edk2.git
    $ git clone https://github.com/tianocore/edk2-platforms.git
    $ cd edk2
    $ git submodule init && git submodule update --init --recursive
    $ cd ..
    $ export WORKSPACE=$(pwd)
    $ export PACKAGES_PATH=$WORKSPACE/edk2:$WORKSPACE/edk2-platforms
    $ export ACTIVE_PLATFORM="Platform/StandaloneMm/PlatformStandaloneMmPkg/PlatformStandaloneMmRpmb.dsc"
    $ export GCC5_AARCH64_PREFIX=aarch64-linux-gnu-
    $ source edk2/edksetup.sh
    $ make -C edk2/BaseTools
    $ build -p $ACTIVE_PLATFORM -b RELEASE -a AARCH64 -t GCC5 -n `nproc`

OP-TEE Build instructions
*************************

.. code-block:: bash

    $ git clone https://github.com/OP-TEE/optee_os.git
    $ cd optee_os
    $ ln -s ../Build/MmStandaloneRpmb/RELEASE_GCC5/FV/BL32_AP_MM.fd
    $ export ARCH=arm
    $ CROSS_COMPILE32=arm-linux-gnueabihf- make -j32 CFG_ARM64_core=y \
        PLATFORM=<myboard> CFG_STMM_PATH=BL32_AP_MM.fd CFG_RPMB_FS=y \
        CFG_RPMB_FS_DEV_ID=0 CFG_CORE_HEAP_SIZE=524288 CFG_RPMB_WRITE_KEY=y \
        CFG_CORE_DYN_SHM=y CFG_RPMB_TESTKEY=y CFG_REE_FS=n \
        CFG_CORE_ARM64_PA_BITS=48 CFG_TEE_CORE_LOG_LEVEL=1 \
        CFG_TEE_TA_LOG_LEVEL=1 CFG_SCTLR_ALIGNMENT_CHECK=n

U-Boot Build instructions
*************************

Although the StandAloneMM binary comes from EDK2, using and storing the
variables is currently available in U-Boot only.

.. code-block:: bash

    $ git clone https://github.com/u-boot/u-boot.git
    $ cd u-boot
    $ export CROSS_COMPILE=aarch64-linux-gnu-
    $ export ARCH=<arch>
    $ make <myboard>_defconfig
    $ make menuconfig

Enable ``CONFIG_OPTEE``, ``CONFIG_CMD_OPTEE_RPMB`` and ``CONFIG_EFI_MM_COMM_TEE``

.. warning::

    - Your OP-TEE platform port must support Dynamic shared memory, since that's
      the only kind of memory U-Boot supports for now.

[1] https://optee.readthedocs.io/en/latest/building/efi_vars/stmm.html

.. _uefi_capsule_update_ref:

Enabling UEFI Capsule Update feature
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Support has been added for the UEFI capsule update feature which
enables updating the U-Boot image using the UEFI firmware management
protocol (FMP). The capsules are not passed to the firmware through
the UpdateCapsule runtime service. Instead, capsule-on-disk
functionality is used for fetching capsules from the EFI System
Partition (ESP) by placing capsule files under the directory::

    \EFI\UpdateCapsule

The directory is checked for capsules only within the
EFI system partition on the device specified in the active boot option,
which is determined by BootXXXX variable in BootNext, or if not, the highest
priority one within BootOrder. Any BootXXXX variables referring to devices
not present are ignored when determining the active boot option.

Please note that capsules will be applied in the alphabetic order of
capsule file names.

Creating a capsule file
***********************

A capsule file can be created by using tools/mkeficapsule.
To build this tool, enable::

    CONFIG_TOOLS_MKEFICAPSULE=y
    CONFIG_TOOLS_LIBCRYPTO=y

Run the following command

.. code-block:: console

    $ mkeficapsule \
      --index <index> --instance 0 \
      --guid <image GUID> \
      <capsule_file_name>

Capsule with firmware version
*****************************

The UEFI specification does not define the firmware versioning mechanism.
EDK II reference implementation inserts the FMP Payload Header right before
the payload. It coutains the fw_version and lowest supported version,
EDK II reference implementation uses these information to implement the
firmware versioning and anti-rollback protection, the firmware version and
lowest supported version is stored into EFI non-volatile variable.

In U-Boot, the firmware versioning is implemented utilizing
the FMP Payload Header same as EDK II reference implementation,
reads the FMP Payload Header and stores the firmware version into
"FmpStateXXXX" EFI non-volatile variable. XXXX indicates the image index,
since FMP protocol handles multiple image indexes.

To add the fw_version into the FMP Payload Header,
add --fw-version option in mkeficapsule tool.

.. code-block:: console

    $ mkeficapsule \
      --index <index> --instance 0 \
      --guid <image GUID> \
      --fw-version 5 \
      <capsule_file_name>

If the --fw-version option is not set, FMP Payload Header is not inserted
and fw_version is set as 0.

Capsule Generation through binman
*********************************

Support has also been added to generate capsules during U-Boot build
through binman. This requires the platform's DTB to be populated with
the capsule entry nodes for binman. The capsules then can be generated
by specifying the capsule parameters as properties in the capsule
entry node.

Check the test/py/tests/test_efi_capsule/capsule_gen_binman.dts file
as reference for how a typical binman node for capsule generation
looks like. For generating capsules as part of the platform's build, a
capsule node would then have to be included into the platform's
devicetree.

A typical binman node for generating a capsule would look like::

	capsule {
		filename = "u-boot.capsule";
		efi-capsule {
			image-index = <0x1>;
			image-guid = "09d7cf52-0720-4710-91d1-08469b7fe9c8";

			u-boot {
			};
		};
	};

In the above example, a capsule file named u-boot.capsule will be
generated with u-boot.bin as it's input payload. The capsule
generation parameters like image-index and image-guid are being
specified as properties. Similarly, other properties like the private
and public key certificate can be specified for generating signed
capsules. Refer :ref:`etype_efi_capsule` for documentation about the
efi-capsule binman entry type, which describes all the properties that
can be specified.

Dumping capsule headers
***********************

The mkeficapsule tool also provides a command-line option to dump the
contents of the capsule header. This is a useful functionality when
trying to understand the structure of a capsule and is also used in
capsule verification. This feature is used in testing the capsule
contents in binman's test framework.

To check the contents of the capsule headers, the mkeficapsule command
can be used.

.. code-block:: console

    $ mkeficapsule --dump-capsule \
      <capsule_file_name>

Performing the update
*********************

Put capsule files under the directory mentioned above.
Then, following the UEFI specification, you'll need to set
the EFI_OS_INDICATIONS_FILE_CAPSULE_DELIVERY_SUPPORTED
bit in OsIndications variable with

.. code-block:: console

    => setenv -e -nv -bs -rt -v OsIndications =0x0000000000000004

Since U-Boot doesn't currently support SetVariable at runtime, its value
won't be taken over across the reboot. If this is the case, you can skip
this feature check with the Kconfig option (CONFIG_EFI_IGNORE_OSINDICATIONS)
set.

A few values need to be defined in the board file for performing the
capsule update. These values are defined in the board file by
initialisation of a structure which provides information needed for
capsule updates. The following structures have been defined for
containing the image related information

.. code-block:: c

	struct efi_fw_image {
		efi_guid_t image_type_id;
		u16 *fw_name;
		u8 image_index;
	};

	struct efi_capsule_update_info {
		const char *dfu_string;
		struct efi_fw_image *images;
	};


A string is defined which is to be used for populating the
dfu_alt_info variable. This string is used by the function
set_dfu_alt_info. Instead of taking the variable from the environment,
the capsule update feature requires that the variable be set through
the function, since that is more robust. Allowing the user to change
the location of the firmware updates is not a very secure
practice. Getting this information from the firmware itself is more
secure, assuming the firmware has been verified by a previous stage
boot loader.

The firmware images structure defines the GUID values, image index
values and the name of the images that are to be updated through
the capsule update feature. These values are to be defined as part of
an array. These GUID values would be used by the Firmware Management
Protocol(FMP) to populate the image descriptor array and also
displayed as part of the ESRT table. The image index values defined in
the array should be one greater than the dfu alt number that
corresponds to the firmware image. So, if the dfu alt number for an
image is 2, the value of image index in the fw_images array for that
image should be 3. The dfu alt number can be obtained by running the
following command::

    dfu list

When the FWU Multi Bank Update feature is enabled on the platform, the
image index is used only to identify the image index with the image
GUID. The image index would not correspond to the dfu alt number. This
is because the FWU feature supports multiple partitions(banks) of
updatable images, and the actual dfu alt number to which the image is
to be written to is determined at runtime, based on the value of the
update bank to which the image is to be written. For more information
on the FWU Multi Bank Update feature, please refer to
:doc:`/develop/uefi/fwu_updates`.

When using the FMP for FIT images, the image index value needs to be
set to 1.

Finally, the capsule update can be initiated by rebooting the board.

An example of setting the values in the struct efi_fw_image and
struct efi_capsule_update_info is shown below

.. code-block:: c

	struct efi_fw_image fw_images[] = {
		{
			.image_type_id = DEVELOPERBOX_UBOOT_IMAGE_GUID,
			.fw_name = u"DEVELOPERBOX-UBOOT",
			.image_index = 1,
		},
		{
			.image_type_id = DEVELOPERBOX_FIP_IMAGE_GUID,
			.fw_name = u"DEVELOPERBOX-FIP",
			.image_index = 2,
		},
		{
			.image_type_id = DEVELOPERBOX_OPTEE_IMAGE_GUID,
			.fw_name = u"DEVELOPERBOX-OPTEE",
			.image_index = 3,
		},
	};

	struct efi_capsule_update_info update_info = {
		.dfu_string = "mtd nor1=u-boot.bin raw 200000 100000;"
				"fip.bin raw 180000 78000;"
				"optee.bin raw 500000 100000",
		.images = fw_images,
	};

Platforms must declare a variable update_info of type struct
efi_capsule_update_info as shown in the example above. The platform
will also define a fw_images array which contains information of all
the firmware images that are to be updated through capsule update
mechanism. The dfu_string is the string that is to be set as
dfu_alt_info. In the example above, the image index to be set for
u-boot.bin binary is 0x1, for fip.bin is 0x2 and for optee.bin is 0x3.

As an example, for generating the capsule for the optee.bin image, the
following command can be issued

.. code-block:: bash

    $ ./tools/mkeficapsule \
      --index 0x3 --instance 0 \
      --guid c1b629f1-ce0e-4894-82bf-f0a38387e630 \
      optee.bin optee.capsule


Enabling Capsule Authentication
*******************************

The UEFI specification defines a way of authenticating the capsule to
be updated by verifying the capsule signature. The capsule signature
is computed and prepended to the capsule payload at the time of
capsule generation. This signature is then verified by using the
public key stored as part of the X509 certificate. This certificate is
in the form of an efi signature list (esl) file, which is embedded in
a device tree.

The capsule authentication feature can be enabled through the
following config, in addition to the configs listed above for capsule
update::

    CONFIG_EFI_CAPSULE_AUTHENTICATE=y

The public and private keys used for the signing process are generated
and used by the steps highlighted below.

1. Install utility commands on your host
       * openssl
       * efitools

2. Create signing keys and certificate files on your host

.. code-block:: console

    $ openssl req -x509 -sha256 -newkey rsa:2048 -subj /CN=CRT/ \
        -keyout CRT.key -out CRT.crt -nodes -days 365
    $ cert-to-efi-sig-list CRT.crt CRT.esl

3. Run the following command to create and sign the capsule file

.. code-block:: console

    $ mkeficapsule --monotonic-count 1 \
      --private-key CRT.key \
      --certificate CRT.crt \
      --index 1 --instance 0 \
      [--fit | --raw | --guid <guid-string] \
      <image_blob> <capsule_file_name>

4. Insert the signature list into a device tree in the following format::

    {
            signature {
                    capsule-key = [ <binary of signature list> ];
            }
            ...
    }

You can perform step-4 through the Kconfig symbol
CONFIG_EFI_CAPSULE_ESL_FILE. This symbol points to the esl file
generated in step-2. Once the symbol has been populated with the path
to the esl file, it will automatically get embedded into the
platform's dtb as part of U-Boot build.

Anti-rollback Protection
************************

Anti-rollback prevents unintentional installation of outdated firmware.
To enable anti-rollback, you must add the lowest-supported-version property
to dtb and specify --fw-version when creating a capsule file with the
mkeficapsule tool.
When executing capsule update, U-Boot checks if fw_version is greater than
or equal to lowest-supported-version. If fw_version is less than
lowest-supported-version, the update will fail.
For example, if lowest-supported-version is set to 7 and you run capsule
update using a capsule file with --fw-version of 5, the update will fail.
When the --fw-version in the capsule file is updated, lowest-supported-version
in the dtb might be updated accordingly.

If user needs to enforce anti-rollback to any older version,
the lowest-supported-version property in dtb must be always updated manually.

Note that the lowest-supported-version property specified in U-Boot's control
device tree can be changed by U-Boot fdt command.
Secure systems should not enable this command.

To insert the lowest supported version into a dtb

.. code-block:: console

    $ dtc -@ -I dts -O dtb -o version.dtbo version.dtso
    $ fdtoverlay -i orig.dtb -o new.dtb -v version.dtbo

where version.dtso looks like::

    /dts-v1/;
    /plugin/;
    &{/} {
            firmware-version {
                    image1 {
                            image-type-id = "09D7CF52-0720-4710-91D1-08469B7FE9C8";
                            image-index = <1>;
                            lowest-supported-version = <3>;
                    };
            };
    };

The properties of image-type-id and image-index must match the value
defined in the efi_fw_image array as image_type_id and image_index.

Executing the boot manager
~~~~~~~~~~~~~~~~~~~~~~~~~~

The UEFI specification foresees to define boot entries and boot sequence via
UEFI variables. Booting according to these variables is possible via::

    bootefi bootmgr [fdt address]

As of U-Boot v2020.10 UEFI variables cannot be set at runtime. The U-Boot
command 'efidebug' can be used to set the variables.

UEFI HTTP Boot
~~~~~~~~~~~~~~

HTTP Boot provides the capability for system deployment and configuration
over the network. HTTP Boot can be activated by specifying::

    CONFIG_EFI_HTTP_BOOT

Enabling that will automatically select::

    CONFIG_CMD_DNS
    CONFIG_CMD_WGET
    CONFIG_BLKMAP

Set up the load option specifying the target URI::

    efidebug boot add -u 1 netinst http://foo/bar

When this load option is selected as boot selection, resolve the
host ip address by dns, then download the file with wget.
If the downloaded file extension is .iso or .img file, efibootmgr tries to
mount the image and boot with the default file(e.g. EFI/BOOT/BOOTAA64.EFI).
If the downloaded file is PE-COFF image, load the downloaded file and
start it.

The current implementation tries to resolve the IP address as a host name.
If the uri is like "http://192.168.1.1/foobar",
the dns process tries to resolve the host "192.168.1.1" and it will
end up with "host not found".

We need to preset the "httpserverip" environment variable to proceed the wget::

    setenv httpserverip 192.168.1.1

Executing the built in hello world application
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A hello world UEFI application can be built with::

    CONFIG_CMD_BOOTEFI_HELLO_COMPILE=y

It can be embedded into the U-Boot binary with::

    CONFIG_CMD_BOOTEFI_HELLO=y

The bootefi command is used to start the embedded hello world application::

    bootefi hello [fdt address]

Below you find the output of an example session::

    => bootefi hello ${fdtcontroladdr}
    ## Starting EFI application at 01000000 ...
    WARNING: using memory device/image path, this may confuse some payloads!
    Hello, world!
    Running on UEFI 2.7
    Have SMBIOS table
    Have device tree
    Load options: root=/dev/sdb3 init=/sbin/init rootwait ro
    ## Application terminated, r = 0

The environment variable fdtcontroladdr points to U-Boot's internal device tree
(if available).

Executing the built-in self-test
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

An UEFI self-test suite can be embedded in U-Boot by building with::

    CONFIG_CMD_BOOTEFI_SELFTEST=y

For testing the UEFI implementation the bootefi command can be used to start the
self-test::

    bootefi selftest [fdt address]

The environment variable 'efi_selftest' can be used to select a single test. If
it is not provided all tests are executed except those marked as 'on request'.
If the environment variable is set to 'list' a list of all tests is shown.

Below you can find the output of an example session::

    => setenv efi_selftest simple network protocol
    => bootefi selftest
    Testing EFI API implementation
    Selected test: 'simple network protocol'
    Setting up 'simple network protocol'
    Setting up 'simple network protocol' succeeded
    Executing 'simple network protocol'
    DHCP Discover
    DHCP reply received from 192.168.76.2 (52:55:c0:a8:4c:02)
      as broadcast message.
    Executing 'simple network protocol' succeeded
    Tearing down 'simple network protocol'
    Tearing down 'simple network protocol' succeeded
    Boot services terminated
    Summary: 0 failures
    Preparing for reset. Press any key.

The UEFI life cycle
-------------------

After the U-Boot platform has been initialized the UEFI API provides two kinds
of services:

* boot services
* runtime services

The API can be extended by loading UEFI drivers which come in two variants:

* boot drivers
* runtime drivers

UEFI drivers are installed with U-Boot's bootefi command. With the same command
UEFI applications can be executed.

Loaded images of UEFI drivers stay in memory after returning to U-Boot while
loaded images of applications are removed from memory.

An UEFI application (e.g. an operating system) that wants to take full control
of the system calls ExitBootServices. After a UEFI application calls
ExitBootServices

* boot services are not available anymore
* timer events are stopped
* the memory used by U-Boot except for runtime services is released
* the memory used by boot time drivers is released

So this is a point of no return. Afterwards the UEFI application can only return
to U-Boot by rebooting.

The UEFI object model
---------------------

UEFI offers a flexible and expandable object model. The objects in the UEFI API
are devices, drivers, and loaded images. These objects are referenced by
handles.

The interfaces implemented by the objects are referred to as protocols. These
are identified by GUIDs. They can be installed and uninstalled by calling the
appropriate boot services.

Handles are created by the InstallProtocolInterface or the
InstallMultipleProtocolinterfaces service if NULL is passed as handle.

Handles are deleted when the last protocol has been removed with the
UninstallProtocolInterface or the UninstallMultipleProtocolInterfaces service.

Devices offer the EFI_DEVICE_PATH_PROTOCOL. A device path is the concatenation
of device nodes. By their device paths all devices of a system are arranged in a
tree.

Drivers offer the EFI_DRIVER_BINDING_PROTOCOL. This protocol is used to connect
a driver to devices (which are referenced as controllers in this context).

Loaded images offer the EFI_LOADED_IMAGE_PROTOCOL. This protocol provides meta
information about the image and a pointer to the unload callback function.

The UEFI events
---------------

In the UEFI terminology an event is a data object referencing a notification
function which is queued for calling when the event is signaled. The following
types of events exist:

* periodic and single shot timer events
* exit boot services events, triggered by calling the ExitBootServices() service
* virtual address change events
* memory map change events
* read to boot events
* reset system events
* system table events
* events that are only triggered programmatically

Events can be created with the CreateEvent service and deleted with CloseEvent
service.

Events can be assigned to an event group. If any of the events in a group is
signaled, all other events in the group are also set to the signaled state.

The UEFI driver model
---------------------

A driver is specific for a single protocol installed on a device. To install a
driver on a device the ConnectController service is called. In this context
controller refers to the device for which the driver is installed.

The relevant drivers are identified using the EFI_DRIVER_BINDING_PROTOCOL. This
protocol has three functions:

* supported - determines if the driver is compatible with the device
* start - installs the driver by opening the relevant protocol with
  attribute EFI_OPEN_PROTOCOL_BY_DRIVER
* stop - uninstalls the driver

The driver may create child controllers (child devices). E.g. a driver for block
IO devices will create the device handles for the partitions. The child
controllers  will open the supported protocol with the attribute
EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER.

A driver can be detached from a device using the DisconnectController service.

U-Boot devices mapped as UEFI devices
-------------------------------------

Some of the U-Boot devices are mapped as UEFI devices

* block IO devices
* console
* graphical output
* network adapter

As of U-Boot 2018.03 the logic for doing this is hard coded.

The development target is to integrate the setup of these UEFI devices with the
U-Boot driver model [5]. So when a U-Boot device is discovered a handle should
be created and the device path protocol and the relevant IO protocol should be
installed. The UEFI driver then would be attached by calling ConnectController.
When a U-Boot device is removed DisconnectController should be called.

UEFI devices mapped as U-Boot devices
-------------------------------------

UEFI drivers binaries and applications may create new (virtual) devices, install
a protocol and call the ConnectController service. Now the matching UEFI driver
is determined by iterating over the implementations of the
EFI_DRIVER_BINDING_PROTOCOL.

It is the task of the UEFI driver to create a corresponding U-Boot device and to
proxy calls for this U-Boot device to the controller.

In U-Boot 2018.03 this has only been implemented for block IO devices.

UEFI uclass
~~~~~~~~~~~

An UEFI uclass driver (lib/efi_driver/efi_uclass.c) has been created that
takes care of initializing the UEFI drivers and providing the
EFI_DRIVER_BINDING_PROTOCOL implementation for the UEFI drivers.

A linker created list is used to keep track of the UEFI drivers. To create an
entry in the list the UEFI driver uses the U_BOOT_DRIVER macro specifying
UCLASS_EFI_LOADER as the ID of its uclass, e.g::

    /* Identify as UEFI driver */
    U_BOOT_DRIVER(efi_block) = {
        .name  = "EFI block driver",
        .id    = UCLASS_EFI_LOADER,
        .ops   = &driver_ops,
    };

The available operations are defined via the structure struct efi_driver_ops::

    struct efi_driver_ops {
        const efi_guid_t *protocol;
        const efi_guid_t *child_protocol;
        int (*bind)(efi_handle_t handle, void *interface);
    };

When the supported() function of the EFI_DRIVER_BINDING_PROTOCOL is called the
uclass checks if the protocol GUID matches the protocol GUID of the UEFI driver.
In the start() function the bind() function of the UEFI driver is called after
checking the GUID.
The stop() function of the EFI_DRIVER_BINDING_PROTOCOL disconnects the child
controllers created by the UEFI driver and the UEFI driver. (In U-Boot v2013.03
this is not yet completely implemented.)

UEFI block IO driver
~~~~~~~~~~~~~~~~~~~~

The UEFI block IO driver supports devices exposing the EFI_BLOCK_IO_PROTOCOL.

When connected it creates a new U-Boot block IO device with interface type
UCLASS_EFI_LOADER, adds child controllers mapping the partitions, and installs
the EFI_SIMPLE_FILE_SYSTEM_PROTOCOL on these. This can be used together with the
software iPXE to boot from iSCSI network drives [4].

This driver is only available if U-Boot is configured with::

    CONFIG_BLK=y
    CONFIG_PARTITIONS=y

Miscellaneous
-------------

Load file 2 protocol
~~~~~~~~~~~~~~~~~~~~

The load file 2 protocol can be used by the Linux kernel to load the initial
RAM disk. U-Boot can be configured to provide an implementation with::

    EFI_LOAD_FILE2_INITRD=y

When the option is enabled the user can add the initrd path with the efidebug
command.

Load options Boot#### have a FilePathList[] member.  The first element of
the array (FilePathList[0]) is the EFI binary to execute.  When an initrd
is specified the Device Path for the initrd is denoted by a VenMedia node
with the EFI_INITRD_MEDIA_GUID. Each entry of the array is terminated by the
'end of entire device path' subtype (0xff). If a user wants to define multiple
initrds, those must by separated by the 'end of this instance' identifier of
the end node (0x01).

So our final format of the FilePathList[] is::

    Loaded image - end node (0xff) - VenMedia - initrd_1 - [end node (0x01) - initrd_n ...] - end node (0xff)

Links
-----

* [1] http://uefi.org/specifications - UEFI specifications
* [2] https://github.com/ARM-software/ebbr/releases/download/v2.1.0/ebbr-v2.1.0.pdf -
  Embedded Base Boot Requirements (EBBR) Specification - Release v2.1.0
* [3] https://developer.arm.com/docs/den0044/latest/server-base-boot-requirements-system-software-on-arm-platforms-version-11 -
  Server Base Boot Requirements System Software on ARM Platforms - Version 1.1
* [4] :doc:`iscsi`
* [5] :doc:`../driver-model/index`
