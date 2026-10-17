Binman Entry Documentation
==========================

This file describes the entry types supported by binman. These entry types can
be placed in an image one by one to build up a final firmware image. It is
fairly easy to create new entry types. Just add a new file to the 'etype'
directory. You can use the existing entries as examples.

Note that some entries are subclasses of others, using and extending their
features to produce new behaviours.



.. _etype_atf_bl31:

Entry: atf-bl31: ARM Trusted Firmware (ATF) BL31 blob
-----------------------------------------------------

Properties / Entry arguments:
    - atf-bl31-path: Filename of file to read into entry. This is typically
        called bl31.bin or bl31.elf

This entry holds the run-time firmware, typically started by U-Boot SPL.
See the U-Boot README for your architecture or board for how to use it. See
https://github.com/ARM-software/arm-trusted-firmware for more information
about ATF.



.. _etype_atf_fip:

Entry: atf-fip: ARM Trusted Firmware's Firmware Image Package (FIP)
-------------------------------------------------------------------

A FIP_ provides a way to group binaries in a firmware image, used by ARM's
Trusted Firmware A (TF-A) code. It is a simple format consisting of a
table of contents with information about the type, offset and size of the
binaries in the FIP. It is quite similar to FMAP, with the major difference
that it uses UUIDs to indicate the type of each entry.

Note: It is recommended to always add an fdtmap to every image, as well as
any FIPs so that binman and other tools can access the entire image
correctly.

The UUIDs correspond to useful names in `fiptool`, provided by ATF to
operate on FIPs. Binman uses these names to make it easier to understand
what is going on, although it is possible to provide a UUID if needed.

The contents of the FIP are defined by subnodes of the atf-fip entry, e.g.::

    atf-fip {
        soc-fw {
            filename = "bl31.bin";
        };

        scp-fwu-cfg {
            filename = "bl2u.bin";
        };

        u-boot {
            fip-type = "nt-fw";
        };
    };

This describes a FIP with three entries: soc-fw, scp-fwu-cfg and nt-fw.
You can use normal (non-external) binaries like U-Boot simply by adding a
FIP type, with the `fip-type` property, as above.

Since FIP exists to bring blobs together, Binman assumes that all FIP
entries are external binaries. If a binary may not exist, you can use the
`--allow-missing` flag to Binman, in which case the image is still created,
even though it will not actually work.

The size of the FIP depends on the size of the binaries. There is currently
no way to specify a fixed size. If the `atf-fip` node has a `size` entry,
this affects the space taken up by the `atf-fip` entry, but the FIP itself
does not expand to use that space.

Some other FIP features are available with Binman. The header and the
entries have 64-bit flag works. The flag flags do not seem to be defined
anywhere, but you can use `fip-hdr-flags` and fip-flags` to set the values
of the header and entries respectively.

FIP entries can be aligned to a particular power-of-two boundary. Use
fip-align for this.

Binman only understands the entry types that are included in its
implementation. It is possible to specify a 16-byte UUID instead, using the
fip-uuid property. In this case Binman doesn't know what its type is, so
just uses the UUID. See the `u-boot` node in this example::

    binman {
        atf-fip {
            fip-hdr-flags = /bits/ 64 <0x123>;
            fip-align = <16>;
            soc-fw {
                fip-flags = /bits/ 64 <0x456>;
                filename = "bl31.bin";
            };

            scp-fwu-cfg {
                filename = "bl2u.bin";
            };

            u-boot {
                fip-uuid = [fc 65 13 92 4a 5b 11 ec
                            94 35 ff 2d 1c fc 79 9c];
            };
        };
        fdtmap {
        };
    };

Binman allows reading and updating FIP entries after the image is created,
provided that an FDPMAP is present too. Updates which change the size of a
FIP entry will cause it to be expanded or contracted as needed.

Properties for top-level atf-fip node
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

fip-hdr-flags (64 bits)
    Sets the flags for the FIP header.

Properties for subnodes
~~~~~~~~~~~~~~~~~~~~~~~

fip-type (str)
    FIP type to use for this entry. This is needed if the entry
    name is not a valid type. Value types are defined in `fip_util.py`.
    The FIP type defines the UUID that is used (they map 1:1).

fip-uuid (16 bytes)
    If there is no FIP-type name defined, or it is not supported by Binman,
    this property sets the UUID. It should be a 16-byte value, following the
    hex digits of the UUID.

fip-flags (64 bits)
    Set the flags for a FIP entry. Use in one of the subnodes of the
    7atf-fip entry.

fip-align
    Set the alignment for a FIP entry, FIP entries can be aligned to a
    particular power-of-two boundary. The default is 1.

Adding new FIP-entry types
~~~~~~~~~~~~~~~~~~~~~~~~~~

When new FIP entries are defined by TF-A they appear in the
`TF-A source tree`_. You can use `fip_util.py` to update Binman to support
new types, then `send a patch`_ to the U-Boot mailing list. There are two
source files that the tool examples:

- `include/tools_share/firmware_image_package.h` has the UUIDs
- `tools/fiptool/tbbr_config.c` has the name and descripion for each UUID

To run the tool::

    $ tools/binman/fip_util.py  -s /path/to/arm-trusted-firmware
    Warning: UUID 'UUID_NON_TRUSTED_WORLD_KEY_CERT' is not mentioned in tbbr_config.c file
    Existing code in 'tools/binman/fip_util.py' is up-to-date

If it shows there is an update, it writes a new version of `fip_util.py`
to `fip_util.py.out`. You can change the output file using the `-i` flag.
If you have a problem, use `-D` to enable traceback debugging.

FIP commentary
~~~~~~~~~~~~~~

As a side effect of use of UUIDs, FIP does not support multiple
entries of the same type, such as might be used to store fonts or graphics
icons, for example. For verified boot it could be used for each part of the
image (e.g. separate FIPs for A and B) but cannot describe the whole
firmware image. As with FMAP there is no hierarchy defined, although FMAP
works around this by having 'section' areas which encompass others. A
similar workaround would be possible with FIP but is not currently defined.

It is recommended to always add an fdtmap to every image, as well as any
FIPs so that binman and other tools can access the entire image correctly.

.. _FIP: https://trustedfirmware-a.readthedocs.io/en/latest/design/firmware-design.html#firmware-image-package-fip
.. _`TF-A source tree`: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
.. _`send a patch`: https://www.denx.de/wiki/U-Boot/Patches



.. _etype_blob:

Entry: blob: Arbitrary binary blob
----------------------------------

Note: This should not be used by itself. It is normally used as a parent
class by other entry types.

Properties / Entry arguments:
    - filename: Filename of file to read into entry
    - compress: Compression algorithm to use:
        none: No compression
        lz4: Use lz4 compression (via 'lz4' command-line utility)

This entry reads data from a file and places it in the entry. The
default filename is often specified specified by the subclass. See for
example the 'u-boot' entry which provides the filename 'u-boot.bin'.

If compression is enabled, an extra 'uncomp-size' property is written to
the node (if enabled with -u) which provides the uncompressed size of the
data.



.. _etype_blob_dtb:

Entry: blob-dtb: A blob that holds a device tree
------------------------------------------------

This is a blob containing a device tree. The contents of the blob are
obtained from the list of available device-tree files, managed by the
'state' module.

Additional attributes:
    prepend: Header used (e.g. 'length')



.. _etype_blob_ext:

Entry: blob-ext: Externally built binary blob
---------------------------------------------

Note: This should not be used by itself. It is normally used as a parent
class by other entry types.

If the file providing this blob is missing, binman can optionally ignore it
and produce a broken image with a warning.

See 'blob' for Properties / Entry arguments.



.. _etype_blob_ext_list:

Entry: blob-ext-list: List of externally built binary blobs
-----------------------------------------------------------

This is like blob-ext except that a number of blobs can be provided,
typically with some sort of relationship, e.g. all are DDC parameters.

If any of the external files needed by this llist is missing, binman can
optionally ignore it and produce a broken image with a warning.

Args:
    filenames: List of filenames to read and include



.. _etype_blob_named_by_arg:

Entry: blob-named-by-arg: A blob entry which gets its filename property from its subclass
-----------------------------------------------------------------------------------------

Properties / Entry arguments:
    - <xxx>-path: Filename containing the contents of this entry (optional,
        defaults to None)

where <xxx> is the blob_fname argument to the constructor.

This entry cannot be used directly. Instead, it is used as a parent class
for another entry, which defined blob_fname. This parameter is used to
set the entry-arg or property containing the filename. The entry-arg or
property is in turn used to set the actual filename.

See cros_ec_rw for an example of this.



.. _etype_blob_phase:

Entry: blob-phase: Section that holds a phase binary
----------------------------------------------------

This is a base class that should not normally be used directly. It is used
when converting a 'u-boot' entry automatically into a 'u-boot-expanded'
entry; similarly for SPL.



.. _etype_cbfs:

Entry: cbfs: Coreboot Filesystem (CBFS)
---------------------------------------

A CBFS provides a way to group files into a group. It has a simple directory
structure and allows the position of individual files to be set, since it is
designed to support execute-in-place in an x86 SPI-flash device. Where XIP
is not used, it supports compression and storing ELF files.

CBFS is used by coreboot as its way of orgnanising SPI-flash contents.

The contents of the CBFS are defined by subnodes of the cbfs entry, e.g.::

    cbfs {
        size = <0x100000>;
        u-boot {
            cbfs-type = "raw";
        };
        u-boot-dtb {
            cbfs-type = "raw";
        };
    };

This creates a CBFS 1MB in size two files in it: u-boot.bin and u-boot.dtb.
Note that the size is required since binman does not support calculating it.
The contents of each entry is just what binman would normally provide if it
were not a CBFS node. A blob type can be used to import arbitrary files as
with the second subnode below::

    cbfs {
        size = <0x100000>;
        u-boot {
            cbfs-name = "BOOT";
            cbfs-type = "raw";
        };

        dtb {
            type = "blob";
            filename = "u-boot.dtb";
            cbfs-type = "raw";
            cbfs-compress = "lz4";
            cbfs-offset = <0x100000>;
        };
    };

This creates a CBFS 1MB in size with u-boot.bin (named "BOOT") and
u-boot.dtb (named "dtb") and compressed with the lz4 algorithm.


Properties supported in the top-level CBFS node:

cbfs-arch:
    Defaults to "x86", but you can specify the architecture if needed.


Properties supported in the CBFS entry subnodes:

cbfs-name:
    This is the name of the file created in CBFS. It defaults to the entry
    name (which is the node name), but you can override it with this
    property.

cbfs-type:
    This is the CBFS file type. The following are supported:

    raw:
        This is a 'raw' file, although compression is supported. It can be
        used to store any file in CBFS.

    stage:
        This is an ELF file that has been loaded (i.e. mapped to memory), so
        appears in the CBFS as a flat binary. The input file must be an ELF
        image, for example this puts "u-boot" (the ELF image) into a 'stage'
        entry::

            cbfs {
                size = <0x100000>;
                u-boot-elf {
                    cbfs-name = "BOOT";
                    cbfs-type = "stage";
                };
            };

        You can use your own ELF file with something like::

            cbfs {
                size = <0x100000>;
                something {
                    type = "blob";
                    filename = "cbfs-stage.elf";
                    cbfs-type = "stage";
                };
            };

        As mentioned, the file is converted to a flat binary, so it is
        equivalent to adding "u-boot.bin", for example, but with the load and
        start addresses specified by the ELF. At present there is no option
        to add a flat binary with a load/start address, similar to the
        'add-flat-binary' option in cbfstool.

cbfs-offset:
    This is the offset of the file's data within the CBFS. It is used to
    specify where the file should be placed in cases where a fixed position
    is needed. Typical uses are for code which is not relocatable and must
    execute in-place from a particular address. This works because SPI flash
    is generally mapped into memory on x86 devices. The file header is
    placed before this offset so that the data start lines up exactly with
    the chosen offset. If this property is not provided, then the file is
    placed in the next available spot.

The current implementation supports only a subset of CBFS features. It does
not support other file types (e.g. payload), adding multiple files (like the
'files' entry with a pattern supported by binman), putting files at a
particular offset in the CBFS and a few other things.

Of course binman can create images containing multiple CBFSs, simply by
defining these in the binman config::


    binman {
        size = <0x800000>;
        cbfs {
            offset = <0x100000>;
            size = <0x100000>;
            u-boot {
                cbfs-type = "raw";
            };
            u-boot-dtb {
                cbfs-type = "raw";
            };
        };

        cbfs2 {
            offset = <0x700000>;
            size = <0x100000>;
            u-boot {
                cbfs-type = "raw";
            };
            u-boot-dtb {
                cbfs-type = "raw";
            };
            image {
                type = "blob";
                filename = "image.jpg";
            };
        };
    };

This creates an 8MB image with two CBFSs, one at offset 1MB, one at 7MB,
both of size 1MB.



.. _etype_collection:

Entry: collection: An entry which contains a collection of other entries
------------------------------------------------------------------------

Properties / Entry arguments:
    - content: List of phandles to entries to include

This allows reusing the contents of other entries. The contents of the
listed entries are combined to form this entry. This serves as a useful
base class for entry types which need to process data from elsewhere in
the image, not necessarily child entries.

The entries can generally be anywhere in the same image, even if they are in
a different section from this entry.



.. _etype_cros_ec_rw:

Entry: cros-ec-rw: A blob entry which contains a Chromium OS read-write EC image
--------------------------------------------------------------------------------

Properties / Entry arguments:
    - cros-ec-rw-path: Filename containing the EC image

This entry holds a Chromium OS EC (embedded controller) image, for use in
updating the EC on startup via software sync.



.. _etype_efi_capsule:

Entry: capsule: Entry for generating EFI Capsule files
------------------------------------------------------

The parameters needed for generation of the capsules can be provided
as properties in the entry.

Properties / Entry arguments:
    - image-index: Unique number for identifying corresponding
      payload image. Number between 1 and descriptor count, i.e.
      the total number of firmware images that can be updated. Mandatory
      property.
    - image-guid: Image GUID which will be used for identifying the
      updatable image on the board. Mandatory property.
    - hardware-instance: Optional number for identifying unique
      hardware instance of a device in the system. Default value of 0
      for images where value is not to be used.
    - fw-version: Value of image version that can be put on the capsule
      through the Firmware Management Protocol(FMP) header.
    - monotonic-count: Count used when signing an image.
    - private-key: Path to PEM formatted .key private key file. Mandatory
      property for generating signed capsules.
    - public-key-cert: Path to PEM formatted .crt public key certificate
      file. Mandatory property for generating signed capsules.
    - oem-flags - OEM flags to be passed through capsule header.

    Since this is a subclass of Entry_section, all properties of the parent
    class also apply here. Except for the properties stated as mandatory, the
    rest of the properties are optional.

For more details on the description of the capsule format, and the capsule
update functionality, refer Section 8.5 and Chapter 23 in the `UEFI
specification`_.

The capsule parameters like image index and image GUID are passed as
properties in the entry. The payload to be used in the capsule is to be
provided as a subnode of the capsule entry.

A typical capsule entry node would then look something like this::

    capsule {
            type = "efi-capsule";
            image-index = <0x1>;
            /* Image GUID for testing capsule update */
            image-guid = SANDBOX_UBOOT_IMAGE_GUID;
            hardware-instance = <0x0>;
            private-key = "path/to/the/private/key";
            public-key-cert = "path/to/the/public-key-cert";
            oem-flags = <0x8000>;

            u-boot {
            };
    };

In the above example, the capsule payload is the U-Boot image. The
capsule entry would read the contents of the payload and put them
into the capsule. Any external file can also be specified as the
payload using the blob-ext subnode.

.. _`UEFI specification`: https://uefi.org/sites/default/files/resources/UEFI_Spec_2_10_Aug29.pdf



.. _etype_efi_empty_capsule:

Entry: efi-empty-capsule: Entry for generating EFI Empty Capsule files
----------------------------------------------------------------------

The parameters needed for generation of the empty capsules can
be provided as properties in the entry.

Properties / Entry arguments:
    - image-guid: Image GUID which will be used for identifying the
      updatable image on the board. Mandatory for accept capsule.
    - capsule-type - String to indicate type of capsule to generate. Valid
      values are 'accept' and 'revert'.

For more details on the description of the capsule format, and the capsule
update functionality, refer Section 8.5 and Chapter 23 in the `UEFI
specification`_. For more information on the empty capsule, refer the
sections 2.3.2 and 2.3.3 in the `Dependable Boot specification`_.

A typical accept empty capsule entry node would then look something
like this::

    empty-capsule {
            type = "efi-empty-capsule";
            /* GUID of the image being accepted */
            image-type-id = SANDBOX_UBOOT_IMAGE_GUID;
            capsule-type = "accept";
    };

A typical revert empty capsule entry node would then look something
like this::

    empty-capsule {
            type = "efi-empty-capsule";
            capsule-type = "revert";
    };

The empty capsules do not have any input payload image.

.. _`UEFI specification`: https://uefi.org/sites/default/files/resources/UEFI_Spec_2_10_Aug29.pdf
.. _`Dependable Boot specification`: https://git.codelinaro.org/linaro/dependable-boot/mbfw/uploads/6f7ddfe3be24e18d4319e108a758d02e/mbfw.pdf



.. _etype_encrypted:

Entry: encrypted: Externally built encrypted binary blob
--------------------------------------------------------

This entry provides the functionality to include information about how to
decrypt an encrypted binary. This information is added to the
resulting device tree by adding a new cipher node in the entry's parent
node (i.e. the binary).

The key that must be used to decrypt the binary is either directly embedded
in the device tree or indirectly by specifying a key source. The key source
can be used as an id of a key that is stored in an external device.

Using an embedded key
~~~~~~~~~~~~~~~~~~~~~

This is an example using an embedded key::

    blob-ext {
        filename = "encrypted-blob.bin";
    };

    encrypted {
        algo = "aes256-gcm";
        iv-filename = "encrypted-blob.bin.iv";
        key-filename = "encrypted-blob.bin.key";
    };

This entry generates the following device tree structure form the example
above::

    data = [...]
    cipher {
        algo = "aes256-gcm";
        key = <0x...>;
        iv = <0x...>;
    };

The data property is generated by the blob-ext etype, the cipher node and
its content is generated by this etype.

Using an external key
~~~~~~~~~~~~~~~~~~~~~

Instead of embedding the key itself into the device tree, it is also
possible to address an externally stored key by specifying a 'key-source'
instead of the 'key'::

    blob-ext {
        filename = "encrypted-blob.bin";
    };

    encrypted {
        algo = "aes256-gcm";
        iv-filename = "encrypted-blob.bin.iv";
        key-source = "external-key-id";
    };

This entry generates the following device tree structure form the example
above::

    data = [...]
    cipher {
        algo = "aes256-gcm";
        key-source = "external-key-id";
        iv = <0x...>;
    };

Properties
~~~~~~~~~~

Properties / Entry arguments:
    - algo: The encryption algorithm. Currently no algorithm is supported
            out-of-the-box. Certain algorithms will be added in future
            patches.
    - iv-filename: The name of the file containing the initialization
                   vector (in short iv). See
                   https://en.wikipedia.org/wiki/Initialization_vector
    - key-filename: The name of the file containing the key. Either
                    key-filename or key-source must be provided.
    - key-source: The key that should be used. Either key-filename or
                  key-source must be provided.



.. _etype_fdtmap:

Entry: fdtmap: An entry which contains an FDT map
-------------------------------------------------

Properties / Entry arguments:
    None

An FDT map is just a header followed by an FDT containing a list of all the
entries in the image. The root node corresponds to the image node in the
original FDT, and an image-name property indicates the image name in that
original tree.

The header is the string _FDTMAP_ followed by 8 unused bytes.

When used, this entry will be populated with an FDT map which reflects the
entries in the current image. Hierarchy is preserved, and all offsets and
sizes are included.

Note that the -u option must be provided to ensure that binman updates the
FDT with the position of each entry.

Example output for a simple image with U-Boot and an FDT map::

    / {
        image-name = "binman";
        size = <0x00000112>;
        image-pos = <0x00000000>;
        offset = <0x00000000>;
        u-boot {
            size = <0x00000004>;
            image-pos = <0x00000000>;
            offset = <0x00000000>;
        };
        fdtmap {
            size = <0x0000010e>;
            image-pos = <0x00000004>;
            offset = <0x00000004>;
        };
    };

If allow-repack is used then 'orig-offset' and 'orig-size' properties are
added as necessary. See the binman README.

When extracting files, an alternative 'fdt' format is available for fdtmaps.
Use `binman extract -F fdt ...` to use this. It will export a devicetree,
without the fdtmap header, so it can be viewed with `fdtdump`.



.. _etype_files:

Entry: files: A set of files arranged in a section
--------------------------------------------------

Properties / Entry arguments:
    - pattern: Filename pattern to match the files to include
    - files-compress: Compression algorithm to use:
        none: No compression
        lz4: Use lz4 compression (via 'lz4' command-line utility)
    - files-align: Align each file to the given alignment

This entry reads a number of files and places each in a separate sub-entry
within this entry. To access these you need to enable device-tree updates
at run-time so you can obtain the file positions.



.. _etype_fill:

Entry: fill: An entry which is filled to a particular byte value
----------------------------------------------------------------

Properties / Entry arguments:
    - fill-byte: Byte to use to fill the entry

Note that the size property must be set since otherwise this entry does not
know how large it should be.

You can often achieve the same effect using the pad-byte property of the
overall image, in that the space between entries will then be padded with
that byte. But this entry is sometimes useful for explicitly setting the
byte value of a region.



.. _etype_fit:

Entry: fit: Flat Image Tree (FIT)
---------------------------------

This calls mkimage to create a FIT (U-Boot Flat Image Tree) based on the
input provided.

Nodes for the FIT should be written out in the binman configuration just as
they would be in a file passed to mkimage.

For example, this creates an image containing a FIT with U-Boot SPL::

    binman {
        fit {
            description = "Test FIT";
            fit,fdt-list = "of-list";

            images {
                kernel@1 {
                    description = "SPL";
                    os = "u-boot";
                    type = "rkspi";
                    arch = "arm";
                    compression = "none";
                    load = <0>;
                    entry = <0>;

                    u-boot-spl {
                    };
                };
            };
        };
    };

More complex setups can be created, with generated nodes, as described
below.

Properties (in the 'fit' node itself)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Special properties have a `fit,` prefix, indicating that they should be
processed but not included in the final FIT.

The top-level 'fit' node supports the following special properties:

    fit,external-offset
        Indicates that the contents of the FIT are external and provides the
        external offset. This is passed to mkimage via the -E and -p flags.

    fit,align
        Indicates what alignment to use for the FIT and its external data,
        and provides the alignment to use. This is passed to mkimage via
        the -B flag.

    fit,fdt-list
        Indicates the entry argument which provides the list of device tree
        files for the gen-fdt-nodes operation (as below). This is often
        `of-list` meaning that `-a of-list="dtb1 dtb2..."` should be passed
        to binman.

    fit,fdt-list-val
        As an alternative to fit,fdt-list the list of device tree files
        can be provided in this property as a string list, e.g.::

            fit,fdt-list-val = "dtb1", "dtb2";

Substitutions
~~~~~~~~~~~~~

Node names and property values support a basic string-substitution feature.
Available substitutions for '@' nodes (and property values) are:

SEQ:
    Sequence number of the generated fdt (1, 2, ...)
NAME
    Name of the dtb as provided (i.e. without adding '.dtb')

The `default` property, if present, will be automatically set to the name
if of configuration whose devicetree matches the `default-dt` entry
argument, e.g. with `-a default-dt=sun50i-a64-pine64-lts`.

Available substitutions for property values in these nodes are:

DEFAULT-SEQ:
    Sequence number of the default fdt, as provided by the 'default-dt'
    entry argument

Available operations
~~~~~~~~~~~~~~~~~~~~

You can add an operation to an '@' node to indicate which operation is
required::

    @fdt-SEQ {
        fit,operation = "gen-fdt-nodes";
        ...
    };

Available operations are:

gen-fdt-nodes
    Generate FDT nodes as above. This is the default if there is no
    `fit,operation` property.

split-elf
    Split an ELF file into a separate node for each segment.

Generating nodes from an FDT list (gen-fdt-nodes)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

U-Boot supports creating fdt and config nodes automatically. To do this,
pass an `of-list` property (e.g. `-a of-list=file1 file2`). This tells
binman that you want to generates nodes for two files: `file1.dtb` and
`file2.dtb`. The `fit,fdt-list` property (see above) indicates that
`of-list` should be used. If the property is missing you will get an error.

Then add a 'generator node', a node with a name starting with '@'::

    images {
        @fdt-SEQ {
            description = "fdt-NAME";
            type = "flat_dt";
            compression = "none";
        };
    };

This tells binman to create nodes `fdt-1` and `fdt-2` for each of your two
files. All the properties you specify will be included in the node. This
node acts like a template to generate the nodes. The generator node itself
does not appear in the output - it is replaced with what binman generates.
A 'data' property is created with the contents of the FDT file.

You can create config nodes in a similar way::

    configurations {
        default = "@config-DEFAULT-SEQ";
        @config-SEQ {
            description = "NAME";
            firmware = "atf";
            loadables = "uboot";
            fdt = "fdt-SEQ";
        };
    };

This tells binman to create nodes `config-1` and `config-2`, i.e. a config
for each of your two files.

Note that if no devicetree files are provided (with '-a of-list' as above)
then no nodes will be generated.

Generating nodes from an ELF file (split-elf)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This uses the node as a template to generate multiple nodes. The following
special properties are available:

split-elf
    Split an ELF file into a separate node for each segment. This uses the
    node as a template to generate multiple nodes. The following special
    properties are available:

    fit,load
        Generates a `load = <...>` property with the load address of the
        segment

    fit,entry
        Generates a `entry = <...>` property with the entry address of the
        ELF. This is only produced for the first entry

    fit,data
        Generates a `data = <...>` property with the contents of the segment

    fit,firmware
        Generates a `firmware = <...>` property. Provides a list of possible
        nodes to be used as the `firmware` property value. The first valid
        node is picked as the firmware. Any remaining valid nodes is
        prepended to the `loadable` property generated by `fit,loadables`

    fit,loadables
        Generates a `loadable = <...>` property with a list of the generated
        nodes (including all nodes if this operation is used multiple times)


Here is an example showing ATF, TEE and a device tree all combined::

    fit {
        description = "test-desc";
        #address-cells = <1>;
        fit,fdt-list = "of-list";

        images {
            u-boot {
                description = "U-Boot (64-bit)";
                type = "standalone";
                os = "U-Boot";
                arch = "arm64";
                compression = "none";
                load = <CONFIG_TEXT_BASE>;
                u-boot-nodtb {
                };
            };
            @fdt-SEQ {
                description = "fdt-NAME.dtb";
                type = "flat_dt";
                compression = "none";
            };
            @atf-SEQ {
                fit,operation = "split-elf";
                description = "ARM Trusted Firmware";
                type = "firmware";
                arch = "arm64";
                os = "arm-trusted-firmware";
                compression = "none";
                fit,load;
                fit,entry;
                fit,data;

                atf-bl31 {
                };
                hash {
                    algo = "sha256";
                };
            };

            @tee-SEQ {
                fit,operation = "split-elf";
                description = "TEE";
                type = "tee";
                arch = "arm64";
                os = "tee";
                compression = "none";
                fit,load;
                fit,entry;
                fit,data;

                tee-os {
                };
                hash {
                    algo = "sha256";
                };
            };
        };

        configurations {
            default = "@config-DEFAULT-SEQ";
            @config-SEQ {
                description = "conf-NAME.dtb";
                fdt = "fdt-SEQ";
                fit,firmware = "atf-1", "u-boot";
                fit,loadables;
            };
        };
    };

If ATF-BL31 is available, this generates a node for each segment in the
ELF file, for example::

    images {
        atf-1 {
            data = <...contents of first segment...>;
            data-offset = <0x00000000>;
            entry = <0x00040000>;
            load = <0x00040000>;
            compression = "none";
            os = "arm-trusted-firmware";
            arch = "arm64";
            type = "firmware";
            description = "ARM Trusted Firmware";
            hash {
                algo = "sha256";
                value = <...hash of first segment...>;
            };
        };
        atf-2 {
            data = <...contents of second segment...>;
            load = <0xff3b0000>;
            compression = "none";
            os = "arm-trusted-firmware";
            arch = "arm64";
            type = "firmware";
            description = "ARM Trusted Firmware";
            hash {
                algo = "sha256";
                value = <...hash of second segment...>;
            };
        };
    };

The same applies for OP-TEE if that is available.

If each binary is not available, the relevant template node (@atf-SEQ or
@tee-SEQ) is removed from the output.

This also generates a `config-xxx` node for each device tree in `of-list`.
Note that the U-Boot build system uses `-a of-list=$(CONFIG_OF_LIST)`
so you can use `CONFIG_OF_LIST` to define that list. In this example it is
set up for `firefly-rk3399` with a single device tree and the default set
with `-a default-dt=$(CONFIG_DEFAULT_DEVICE_TREE)`, so the resulting output
is::

    configurations {
        default = "config-1";
        config-1 {
            loadables = "u-boot", "atf-2", "atf-3", "tee-1", "tee-2";
            description = "rk3399-firefly.dtb";
            fdt = "fdt-1";
            firmware = "atf-1";
        };
    };

U-Boot SPL can then load the firmware (ATF) and all the loadables (U-Boot
proper, ATF and TEE), then proceed with the boot.



.. _etype_fmap:

Entry: fmap: An entry which contains an Fmap section
----------------------------------------------------

Properties / Entry arguments:
    None

FMAP is a simple format used by flashrom, an open-source utility for
reading and writing the SPI flash, typically on x86 CPUs. The format
provides flashrom with a list of areas, so it knows what it in the flash.
It can then read or write just a single area, instead of the whole flash.

The format is defined by the flashrom project, in the file lib/fmap.h -
see www.flashrom.org/Flashrom for more information.

When used, this entry will be populated with an FMAP which reflects the
entries in the current image. Note that any hierarchy is squashed, since
FMAP does not support this. Sections are represented as an area appearing
before its contents, so that it is possible to reconstruct the hierarchy
from the FMAP by using the offset information. This convention does not
seem to be documented, but is used in Chromium OS.

To mark an area as preserved, use the normal 'preserved' flag in the entry.
This will result in the corresponding FMAP area having the
FMAP_AREA_PRESERVE flag. This flag does not automatically propagate down to
child entries.

CBFS entries appear as a single entry, i.e. the sub-entries are ignored.



.. _etype_gbb:

Entry: gbb: An entry which contains a Chromium OS Google Binary Block
---------------------------------------------------------------------

Properties / Entry arguments:
    - hardware-id: Hardware ID to use for this build (a string)
    - keydir: Directory containing the public keys to use
    - bmpblk: Filename containing images used by recovery

Chromium OS uses a GBB to store various pieces of information, in particular
the root and recovery keys that are used to verify the boot process. Some
more details are here:

    https://www.chromium.org/chromium-os/firmware-porting-guide/2-concepts

but note that the page dates from 2013 so is quite out of date. See
README.chromium for how to obtain the required keys and tools.



.. _etype_image_header:

Entry: image-header: An entry which contains a pointer to the FDT map
---------------------------------------------------------------------

Properties / Entry arguments:
    location: Location of header ("start" or "end" of image). This is
        optional. If omitted then the entry must have an offset property.

This adds an 8-byte entry to the start or end of the image, pointing to the
location of the FDT map. The format is a magic number followed by an offset
from the start or end of the image, in twos-compliment format.

This entry must be in the top-level part of the image.

NOTE: If the location is at the start/end, you will probably need to specify
sort-by-offset for the image, unless you actually put the image header
first/last in the entry list.



.. _etype_intel_cmc:

Entry: intel-cmc: Intel Chipset Micro Code (CMC) file
-----------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry

This file contains microcode for some devices in a special format. An
example filename is 'Microcode/C0_22211.BIN'.

See README.x86 for information about x86 binary blobs.



.. _etype_intel_descriptor:

Entry: intel-descriptor: Intel flash descriptor block (4KB)
-----------------------------------------------------------

Properties / Entry arguments:
    filename: Filename of file containing the descriptor. This is typically
        a 4KB binary file, sometimes called 'descriptor.bin'

This entry is placed at the start of flash and provides information about
the SPI flash regions. In particular it provides the base address and
size of the ME (Management Engine) region, allowing us to place the ME
binary in the right place.

With this entry in your image, the position of the 'intel-me' entry will be
fixed in the image, which avoids you needed to specify an offset for that
region. This is useful, because it is not possible to change the position
of the ME region without updating the descriptor.

See README.x86 for information about x86 binary blobs.



.. _etype_intel_fit:

Entry: intel-fit: Intel Firmware Image Table (FIT)
--------------------------------------------------

This entry contains a dummy FIT as required by recent Intel CPUs. The FIT
contains information about the firmware and microcode available in the
image.

At present binman only supports a basic FIT with no microcode.



.. _etype_intel_fit_ptr:

Entry: intel-fit-ptr: Intel Firmware Image Table (FIT) pointer
--------------------------------------------------------------

This entry contains a pointer to the FIT. It is required to be at address
0xffffffc0 in the image.



.. _etype_intel_fsp:

Entry: intel-fsp: Intel Firmware Support Package (FSP) file
-----------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry

This file contains binary blobs which are used on some devices to make the
platform work. U-Boot executes this code since it is not possible to set up
the hardware using U-Boot open-source code. Documentation is typically not
available in sufficient detail to allow this.

An example filename is 'FSP/QUEENSBAY_FSP_GOLD_001_20-DECEMBER-2013.fd'

See README.x86 for information about x86 binary blobs.



.. _etype_intel_fsp_m:

Entry: intel-fsp-m: Intel Firmware Support Package (FSP) memory init
--------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry

This file contains a binary blob which is used on some devices to set up
SDRAM. U-Boot executes this code in SPL so that it can make full use of
memory. Documentation is typically not available in sufficient detail to
allow U-Boot do this this itself..

An example filename is 'fsp_m.bin'

See README.x86 for information about x86 binary blobs.



.. _etype_intel_fsp_s:

Entry: intel-fsp-s: Intel Firmware Support Package (FSP) silicon init
---------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry

This file contains a binary blob which is used on some devices to set up
the silicon. U-Boot executes this code in U-Boot proper after SDRAM is
running, so that it can make full use of memory. Documentation is typically
not available in sufficient detail to allow U-Boot do this this itself.

An example filename is 'fsp_s.bin'

See README.x86 for information about x86 binary blobs.



.. _etype_intel_fsp_t:

Entry: intel-fsp-t: Intel Firmware Support Package (FSP) temp ram init
----------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry

This file contains a binary blob which is used on some devices to set up
temporary memory (Cache-as-RAM or CAR). U-Boot executes this code in TPL so
that it has access to memory for its stack and initial storage.

An example filename is 'fsp_t.bin'

See README.x86 for information about x86 binary blobs.



.. _etype_intel_ifwi:

Entry: intel-ifwi: Intel Integrated Firmware Image (IFWI) file
--------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry. This is either the
        IFWI file itself, or a file that can be converted into one using a
        tool
    - convert-fit: If present this indicates that the ifwitool should be
        used to convert the provided file into a IFWI.

This file contains code and data used by the SoC that is required to make
it work. It includes U-Boot TPL, microcode, things related to the CSE
(Converged Security Engine, the microcontroller that loads all the firmware)
and other items beyond the wit of man.

A typical filename is 'ifwi.bin' for an IFWI file, or 'fitimage.bin' for a
file that will be converted to an IFWI.

The position of this entry is generally set by the intel-descriptor entry.

The contents of the IFWI are specified by the subnodes of the IFWI node.
Each subnode describes an entry which is placed into the IFWFI with a given
sub-partition (and optional entry name).

Properties for subnodes:
    - ifwi-subpart: sub-parition to put this entry into, e.g. "IBBP"
    - ifwi-entry: entry name t use, e.g. "IBBL"
    - ifwi-replace: if present, indicates that the item should be replaced
      in the IFWI. Otherwise it is added.

See README.x86 for information about x86 binary blobs.



.. _etype_intel_me:

Entry: intel-me: Intel Management Engine (ME) file
--------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry

This file contains code used by the SoC that is required to make it work.
The Management Engine is like a background task that runs things that are
not clearly documented, but may include keyboard, display and network
access. For platform that use ME it is not possible to disable it. U-Boot
does not directly execute code in the ME binary.

A typical filename is 'me.bin'.

The position of this entry is generally set by the intel-descriptor entry.

See README.x86 for information about x86 binary blobs.



.. _etype_intel_mrc:

Entry: intel-mrc: Intel Memory Reference Code (MRC) file
--------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry

This file contains code for setting up the SDRAM on some Intel systems. This
is executed by U-Boot when needed early during startup. A typical filename
is 'mrc.bin'.

See README.x86 for information about x86 binary blobs.



.. _etype_intel_refcode:

Entry: intel-refcode: Intel Reference Code file
-----------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry

This file contains code for setting up the platform on some Intel systems.
This is executed by U-Boot when needed early during startup. A typical
filename is 'refcode.bin'.

See README.x86 for information about x86 binary blobs.



.. _etype_intel_vbt:

Entry: intel-vbt: Intel Video BIOS Table (VBT) file
---------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry

This file contains code that sets up the integrated graphics subsystem on
some Intel SoCs. U-Boot executes this when the display is started up.

See README.x86 for information about Intel binary blobs.



.. _etype_intel_vga:

Entry: intel-vga: Intel Video Graphics Adaptor (VGA) file
---------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry

This file contains code that sets up the integrated graphics subsystem on
some Intel SoCs. U-Boot executes this when the display is started up.

This is similar to the VBT file but in a different format.

See README.x86 for information about Intel binary blobs.



.. _etype_mkimage:

Entry: mkimage: Binary produced by mkimage
------------------------------------------

Properties / Entry arguments:
    - args: Arguments to pass
    - data-to-imagename: Indicates that the -d data should be passed in as
      the image name also (-n)
    - multiple-data-files: boolean to tell binman to pass all files as
      datafiles to mkimage instead of creating a temporary file the result
      of datafiles concatenation
    - filename: filename of output binary generated by mkimage

The data passed to mkimage via the -d flag is collected from subnodes of the
mkimage node, e.g.::

    mkimage {
        filename = "imximage.bin";
        args = "-n test -T imximage";

        u-boot-spl {
        };
    };

This calls mkimage to create an imximage with `u-boot-spl.bin` as the data
file, with mkimage being called like this::

    mkimage -d <data_file> -n test -T imximage <output_file>

The output from mkimage then becomes part of the image produced by
binman but also is written into `imximage.bin` file. If you need to put
multiple things in the data file, you can use a section, or just multiple
subnodes like this::

    mkimage {
        args = "-n test -T imximage";

        u-boot-spl {
        };

        u-boot-tpl {
        };
    };

Note that binman places the contents (here SPL and TPL) into a single file
and passes that to mkimage using the -d option.

To pass all datafiles untouched to mkimage::

    mkimage {
            args = "-n rk3399 -T rkspi";
            multiple-data-files;

            u-boot-tpl {
            };

            u-boot-spl {
            };
    };

This calls mkimage to create a Rockchip RK3399-specific first stage
bootloader, made of TPL+SPL. Since this first stage bootloader requires to
align the TPL and SPL but also some weird hacks that is handled by mkimage
directly, binman is told to not perform the concatenation of datafiles prior
to passing the data to mkimage.

To use CONFIG options in the arguments, use a string list instead, as in
this example which also produces four arguments::

    mkimage {
        args = "-n", CONFIG_SYS_SOC, "-T imximage";

        u-boot-spl {
        };
    };

If you need to pass the input data in with the -n argument as well, then use
the 'data-to-imagename' property::

    mkimage {
        args = "-T imximage";
        data-to-imagename;

        u-boot-spl {
        };
    };

That will pass the data to mkimage both as the data file (with -d) and as
the image name (with -n). In both cases, a filename is passed as the
argument, with the actual data being in that file.

If need to pass different data in with -n, then use an `imagename` subnode::

    mkimage {
        args = "-T imximage";

        imagename {
            blob {
                filename = "spl/u-boot-spl.cfgout"
            };
        };

        u-boot-spl {
        };
    };

This will pass in u-boot-spl as the input data and the .cfgout file as the
-n data.



.. _etype_null:

Entry: null: An entry which has no contents of its own
------------------------------------------------------

Note that the size property must be set since otherwise this entry does not
know how large it should be.

The contents are set by the containing section, e.g. the section's pad
byte.



.. _etype_opensbi:

Entry: opensbi: RISC-V OpenSBI fw_dynamic blob
----------------------------------------------

Properties / Entry arguments:
    - opensbi-path: Filename of file to read into entry. This is typically
        called fw_dynamic.bin

This entry holds the run-time firmware, typically started by U-Boot SPL.
See the U-Boot README for your architecture or board for how to use it. See
https://github.com/riscv/opensbi for more information about OpenSBI.



.. _etype_powerpc_mpc85xx_bootpg_resetvec:

Entry: powerpc-mpc85xx-bootpg-resetvec: PowerPC mpc85xx bootpg + resetvec code for U-Boot
-----------------------------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot-br.bin (default 'u-boot-br.bin')

This entry is valid for PowerPC mpc85xx cpus. This entry holds
'bootpg + resetvec' code for PowerPC mpc85xx CPUs which needs to be
placed at offset 'RESET_VECTOR_ADDRESS - 0xffc'.



.. _etype_pre_load:

Entry: pre-load: Pre load image header
--------------------------------------

Properties / Entry arguments:
    - pre-load-key-path: Path of the directory that store key (provided by
      the environment variable PRE_LOAD_KEY_PATH)
    - content: List of phandles to entries to sign
    - algo-name: Hash and signature algo to use for the signature
    - padding-name: Name of the padding (pkcs-1.5 or pss)
    - key-name: Filename of the private key to sign
    - header-size: Total size of the header
    - version: Version of the header

This entry creates a pre-load header that contains a global
image signature.

For example, this creates an image with a pre-load header and a binary::

    binman {
        image2 {
            filename = "sandbox.bin";

            pre-load {
                content = <&image>;
                algo-name = "sha256,rsa2048";
                padding-name = "pss";
                key-name = "private.pem";
                header-size = <4096>;
                version = <1>;
            };

            image: blob-ext {
                filename = "sandbox.itb";
            };
        };
    };



.. _etype_rockchip_tpl:

Entry: rockchip-tpl: Rockchip TPL binary
----------------------------------------

Properties / Entry arguments:
    - rockchip-tpl-path: Filename of file to read into the entry,
                         typically <soc>_ddr_<version>.bin

This entry holds an external TPL binary used by some Rockchip SoCs
instead of normal U-Boot TPL, typically to initialize DRAM.



.. _etype_scp:

Entry: scp: System Control Processor (SCP) firmware blob
--------------------------------------------------------

Properties / Entry arguments:
    - scp-path: Filename of file to read into the entry, typically scp.bin

This entry holds firmware for an external platform-specific coprocessor.



.. _etype_section:

Entry: section: Entry that contains other entries
-------------------------------------------------

A section is an entry which can contain other entries, thus allowing
hierarchical images to be created. See 'Sections and hierarchical images'
in the binman README for more information.

The base implementation simply joins the various entries together, using
various rules about alignment, etc.

Subclassing
~~~~~~~~~~~

This class can be subclassed to support other file formats which hold
multiple entries, such as CBFS. To do this, override the following
functions. The documentation here describes what your function should do.
For example code, see etypes which subclass `Entry_section`, or `cbfs.py`
for a more involved example::

   $ grep -l \(Entry_section tools/binman/etype/*.py

ReadNode()
    Call `super().ReadNode()`, then read any special properties for the
    section. Then call `self.ReadEntries()` to read the entries.

    Binman calls this at the start when reading the image description.

ReadEntries()
    Read in the subnodes of the section. This may involve creating entries
    of a particular etype automatically, as well as reading any special
    properties in the entries. For each entry, entry.ReadNode() should be
    called, to read the basic entry properties. The properties should be
    added to `self._entries[]`, in the correct order, with a suitable name.

    Binman calls this at the start when reading the image description.

BuildSectionData(required)
    Create the custom file format that you want and return it as bytes.
    This likely sets up a file header, then loops through the entries,
    adding them to the file. For each entry, call `entry.GetData()` to
    obtain the data. If that returns None, and `required` is False, then
    this method must give up and return None. But if `required` is True then
    it should assume that all data is valid.

    Binman calls this when packing the image, to find out the size of
    everything. It is called again at the end when building the final image.

SetImagePos(image_pos):
    Call `super().SetImagePos(image_pos)`, then set the `image_pos` values
    for each of the entries. This should use the custom file format to find
    the `start offset` (and `image_pos`) of each entry. If the file format
    uses compression in such a way that there is no offset available (other
    than reading the whole file and decompressing it), then the offsets for
    affected entries can remain unset (`None`). The size should also be set
    if possible.

    Binman calls this after the image has been packed, to update the
    location that all the entries ended up at.

ReadChildData(child, decomp, alt_format):
    The default version of this may be good enough, if you are able to
    implement SetImagePos() correctly. But that is a bit of a bypass, so
    you can override this method to read from your custom file format. It
    should read the entire entry containing the custom file using
    `super().ReadData(True)`, then parse the file to get the data for the
    given child, then return that data.

    If your file format supports compression, the `decomp` argument tells
    you whether to return the compressed data (`decomp` is False) or to
    uncompress it first, then return the uncompressed data (`decomp` is
    True). This is used by the `binman extract -U` option.

    If your entry supports alternative formats, the alt_format provides the
    alternative format that the user has selected. Your function should
    return data in that format. This is used by the 'binman extract -l'
    option.

    Binman calls this when reading in an image, in order to populate all the
    entries with the data from that image (`binman ls`).

WriteChildData(child):
    Binman calls this after `child.data` is updated, to inform the custom
    file format about this, in case it needs to do updates.

    The default version of this does nothing and probably needs to be
    overridden for the 'binman replace' command to work. Your version should
    use `child.data` to update the data for that child in the custom file
    format.

    Binman calls this when updating an image that has been read in and in
    particular to update the data for a particular entry (`binman replace`)

Properties / Entry arguments
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

See :ref:`develop/package/binman:Image description format` for more
information.

align-default
    Default alignment for this section, if no alignment is given in the
    entry

pad-byte
    Pad byte to use when padding

sort-by-offset
    True if entries should be sorted by offset, False if they must be
    in-order in the device tree description

end-at-4gb
    Used to build an x86 ROM which ends at 4GB (2^32)

name-prefix
    Adds a prefix to the name of every entry in the section when writing out
    the map

skip-at-start
    Number of bytes before the first entry starts. These effectively adjust
    the starting offset of entries. For example, if this is 16, then the
    first entry would start at 16. An entry with offset = 20 would in fact
    be written at offset 4 in the image file, since the first 16 bytes are
    skipped when writing.

filename
    filename to write the unpadded section contents to within the output
    directory (None to skip this).

Since a section is also an entry, it inherits all the properies of entries
too.

Note that the `allow_missing` member controls whether this section permits
external blobs to be missing their contents. The option will produce an
image but of course it will not work. It is useful to make sure that
Continuous Integration systems can build without the binaries being
available. This is set by the `SetAllowMissing()` method, if
`--allow-missing` is passed to binman.



.. _etype_tee_os:

Entry: tee-os: Entry containing an OP-TEE Trusted OS (TEE) blob
---------------------------------------------------------------

Properties / Entry arguments:
    - tee-os-path: Filename of file to read into entry. This is typically
        called tee.bin or tee.elf

This entry holds the run-time firmware, typically started by U-Boot SPL.
See the U-Boot README for your architecture or board for how to use it. See
https://github.com/OP-TEE/optee_os for more information about OP-TEE.

Note that if the file is in ELF format, it must go in a FIT. In that case,
this entry will mark itself as absent, providing the data only through the
read_elf_segments() method.

Marking this entry as absent means that it if is used in the wrong context
it can be automatically dropped. Thus it is possible to add an OP-TEE entry
like this::

    binman {
        tee-os {
        };
    };

and pass either an ELF or plain binary in with -a tee-os-path <filename>
and have binman do the right thing:

   - include the entry if tee.bin is provided and it does NOT have the v1
     header
   - drop it otherwise

When used within a FIT, we can do::

    binman {
        fit {
            tee-os {
            };
        };
    };

which will split the ELF into separate nodes for each segment, if an ELF
file is provided (see :ref:`etype_fit`), or produce a single node if the
OP-TEE binary v1 format is provided (see optee_doc_) .

.. _optee_doc: https://optee.readthedocs.io/en/latest/architecture/core.html#partitioning-of-the-binary



.. _etype_text:

Entry: text: An entry which contains text
-----------------------------------------

The text can be provided either in the node itself or by a command-line
argument. There is a level of indirection to allow multiple text strings
and sharing of text.

Properties / Entry arguments:
    text-label: The value of this string indicates the property / entry-arg
        that contains the string to place in the entry
    <xxx> (actual name is the value of text-label): contains the string to
        place in the entry.
    <text>: The text to place in the entry (overrides the above mechanism).
        This is useful when the text is constant.

Example node::

    text {
        size = <50>;
        text-label = "message";
    };

You can then use:

    binman -amessage="this is my message"

and binman will insert that string into the entry.

It is also possible to put the string directly in the node::

    text {
        size = <8>;
        text-label = "message";
        message = "a message directly in the node"
    };

or just::

    text {
        size = <8>;
        text = "some text directly in the node"
    };

The text is not itself nul-terminated. This can be achieved, if required,
by setting the size of the entry to something larger than the text.



.. _etype_ti_board_config:

Entry: ti-board-config: An entry containing a TI schema validated board config binary
-------------------------------------------------------------------------------------

This etype supports generation of two kinds of board configuration
binaries: singular board config binary as well as combined board config
binary.

Properties / Entry arguments:
    - config-file: File containing board configuration data in YAML
    - schema-file: File containing board configuration YAML schema against
      which the config file is validated

Output files:
    - board config binary: File containing board configuration binary

These above parameters are used only when the generated binary is
intended to be a single board configuration binary. Example::

    my-ti-board-config {
        ti-board-config {
            config = "board-config.yaml";
            schema = "schema.yaml";
        };
    };

To generate a combined board configuration binary, we pack the
needed individual binaries into a ti-board-config binary. In this case,
the available supported subnode names are board-cfg, pm-cfg, sec-cfg and
rm-cfg. The final binary is prepended with a header containing details about
the included board config binaries. Example::

    my-combined-ti-board-config {
        ti-board-config {
            board-cfg {
                config = "board-cfg.yaml";
                schema = "schema.yaml";
            };
            sec-cfg {
                config = "sec-cfg.yaml";
                schema = "schema.yaml";
            };
        }
    }



.. _etype_ti_dm:

Entry: ti-dm: TI Device Manager (DM) blob
-----------------------------------------

Properties / Entry arguments:
    - ti-dm-path: Filename of file to read into the entry, typically ti-dm.bin

This entry holds the device manager responsible for resource and power management
in K3 devices. See https://software-dl.ti.com/tisci/esd/latest/ for more information
about TI DM.



.. _etype_ti_secure:

Entry: ti-secure: Entry containing a TI x509 certificate binary
---------------------------------------------------------------

Properties / Entry arguments:
    - content: List of phandles to entries to sign
    - keyfile: Filename of file containing key to sign binary with
    - sha: Hash function to be used for signing

Output files:
    - input.<unique_name> - input file passed to openssl
    - config.<unique_name> - input file generated for openssl (which is
      used as the config file)
    - cert.<unique_name> - output file generated by openssl (which is
      used as the entry contents)

openssl signs the provided data, using the TI templated config file and
writes the signature in this entry. This allows verification that the
data is genuine.



.. _etype_ti_secure_rom:

Entry: ti-secure-rom: Entry containing a TI x509 certificate binary for images booted by ROM
--------------------------------------------------------------------------------------------

Properties / Entry arguments:
    - keyfile: Filename of file containing key to sign binary with
    - combined: boolean if device follows combined boot flow
    - countersign: boolean if device contains countersigned system firmware
    - load: load address of SPL
    - sw-rev: software revision
    - sha: Hash function to be used for signing
    - core: core on which bootloader runs, valid cores are 'secure' and 'public'
    - content: phandle of SPL in case of legacy bootflow or phandles of component binaries
      in case of combined bootflow
    - core-opts (optional): lockstep (0) or split (2) mode set to 0 by default

The following properties are only for generating a combined bootflow binary:
    - sysfw-inner-cert: boolean if binary contains sysfw inner certificate
    - dm-data: boolean if binary contains dm-data binary
    - content-sbl: phandle of SPL binary
    - content-sysfw: phandle of sysfw binary
    - content-sysfw-data: phandle of sysfw-data or tifs-data binary
    - content-sysfw-inner-cert (optional): phandle of sysfw inner certificate binary
    - content-dm-data (optional): phandle of dm-data binary
    - load-sysfw: load address of sysfw binary
    - load-sysfw-data: load address of sysfw-data or tifs-data binary
    - load-sysfw-inner-cert (optional): load address of sysfw inner certificate binary
    - load-dm-data (optional): load address of dm-data binary

Output files:
    - input.<unique_name> - input file passed to openssl
    - config.<unique_name> - input file generated for openssl (which is
      used as the config file)
    - cert.<unique_name> - output file generated by openssl (which is
      used as the entry contents)

openssl signs the provided data, using the TI templated config file and
writes the signature in this entry. This allows verification that the
data is genuine.



.. _etype_u_boot:

Entry: u-boot: U-Boot flat binary
---------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot.bin (default 'u-boot.bin')

This is the U-Boot binary, containing relocation information to allow it
to relocate itself at runtime. The binary typically includes a device tree
blob at the end of it.

U-Boot can access binman symbols at runtime. See :ref:`binman_fdt`.

Note that this entry is automatically replaced with u-boot-expanded unless
--no-expanded is used or the node has a 'no-expanded' property.



.. _etype_u_boot_dtb:

Entry: u-boot-dtb: U-Boot device tree
-------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot.dtb (default 'u-boot.dtb')

This is the U-Boot device tree, containing configuration information for
U-Boot. U-Boot needs this to know what devices are present and which drivers
to activate.

Note: This is mostly an internal entry type, used by others. This allows
binman to know which entries contain a device tree.



.. _etype_u_boot_dtb_with_ucode:

Entry: u-boot-dtb-with-ucode: A U-Boot device tree file, with the microcode removed
-----------------------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot.dtb (default 'u-boot.dtb')

See Entry_u_boot_ucode for full details of the three entries involved in
this process. This entry provides the U-Boot device-tree file, which
contains the microcode. If the microcode is not being collated into one
place then the offset and size of the microcode is recorded by this entry,
for use by u-boot-with-ucode_ptr. If it is being collated, then this
entry deletes the microcode from the device tree (to save space) and makes
it available to u-boot-ucode.



.. _etype_u_boot_elf:

Entry: u-boot-elf: U-Boot ELF image
-----------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot (default 'u-boot')

This is the U-Boot ELF image. It does not include a device tree but can be
relocated to any address for execution.



.. _etype_u_boot_env:

Entry: u-boot-env: An entry which contains a U-Boot environment
---------------------------------------------------------------

Properties / Entry arguments:
    - filename: File containing the environment text, with each line in the
        form var=value



.. _etype_u_boot_expanded:

Entry: u-boot-expanded: U-Boot flat binary broken out into its component parts
------------------------------------------------------------------------------

This is a section containing the U-Boot binary and a devicetree. Using this
entry type automatically creates this section, with the following entries
in it:

   u-boot-nodtb
   u-boot-dtb

Having the devicetree separate allows binman to update it in the final
image, so that the entries positions are provided to the running U-Boot.



.. _etype_u_boot_img:

Entry: u-boot-img: U-Boot legacy image
--------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot.img (default 'u-boot.img')

This is the U-Boot binary as a packaged image, in legacy format. It has a
header which allows it to be loaded at the correct address for execution.

You should use FIT (Flat Image Tree) instead of the legacy image for new
applications.



.. _etype_u_boot_nodtb:

Entry: u-boot-nodtb: U-Boot flat binary without device tree appended
--------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename to include (default 'u-boot-nodtb.bin')

This is the U-Boot binary, containing relocation information to allow it
to relocate itself at runtime. It does not include a device tree blob at
the end of it so normally cannot work without it. You can add a u-boot-dtb
entry after this one, or use a u-boot entry instead, normally expands to a
section containing u-boot and u-boot-dtb



.. _etype_u_boot_spl:

Entry: u-boot-spl: U-Boot SPL binary
------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot-spl.bin (default 'spl/u-boot-spl.bin')

This is the U-Boot SPL (Secondary Program Loader) binary. This is a small
binary which loads before U-Boot proper, typically into on-chip SRAM. It is
responsible for locating, loading and jumping to U-Boot. Note that SPL is
not relocatable so must be loaded to the correct address in SRAM, or written
to run from the correct address if direct flash execution is possible (e.g.
on x86 devices).

SPL can access binman symbols at runtime. See :ref:`binman_fdt`.

in the binman README for more information.

The ELF file 'spl/u-boot-spl' must also be available for this to work, since
binman uses that to look up symbols to write into the SPL binary.

Note that this entry is automatically replaced with u-boot-spl-expanded
unless --no-expanded is used or the node has a 'no-expanded' property.



.. _etype_u_boot_spl_bss_pad:

Entry: u-boot-spl-bss-pad: U-Boot SPL binary padded with a BSS region
---------------------------------------------------------------------

Properties / Entry arguments:
    None

This holds the padding added after the SPL binary to cover the BSS (Block
Started by Symbol) region. This region holds the various variables used by
SPL. It is set to 0 by SPL when it starts up. If you want to append data to
the SPL image (such as a device tree file), you must pad out the BSS region
to avoid the data overlapping with U-Boot variables. This entry is useful in
that case. It automatically pads out the entry size to cover both the code,
data and BSS.

The contents of this entry will a certain number of zero bytes, determined
by __bss_size

The ELF file 'spl/u-boot-spl' must also be available for this to work, since
binman uses that to look up the BSS address.



.. _etype_u_boot_spl_dtb:

Entry: u-boot-spl-dtb: U-Boot SPL device tree
---------------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot.dtb (default 'spl/u-boot-spl.dtb')

This is the SPL device tree, containing configuration information for
SPL. SPL needs this to know what devices are present and which drivers
to activate.



.. _etype_u_boot_spl_elf:

Entry: u-boot-spl-elf: U-Boot SPL ELF image
-------------------------------------------

Properties / Entry arguments:
    - filename: Filename of SPL u-boot (default 'spl/u-boot-spl')

This is the U-Boot SPL ELF image. It does not include a device tree but can
be relocated to any address for execution.



.. _etype_u_boot_spl_expanded:

Entry: u-boot-spl-expanded: U-Boot SPL flat binary broken out into its component parts
--------------------------------------------------------------------------------------

Properties / Entry arguments:
    - spl-dtb: Controls whether this entry is selected (set to 'y' or '1' to
        select)

This is a section containing the U-Boot binary, BSS padding if needed and a
devicetree. Using this entry type automatically creates this section, with
the following entries in it:

   u-boot-spl-nodtb
   u-boot-spl-bss-pad
   u-boot-dtb

Having the devicetree separate allows binman to update it in the final
image, so that the entries positions are provided to the running U-Boot.

This entry is selected based on the value of the 'spl-dtb' entryarg. If
this is non-empty (and not 'n' or '0') then this expanded entry is selected.



.. _etype_u_boot_spl_nodtb:

Entry: u-boot-spl-nodtb: SPL binary without device tree appended
----------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename to include (default 'spl/u-boot-spl-nodtb.bin')

This is the U-Boot SPL binary, It does not include a device tree blob at
the end of it so may not be able to work without it, assuming SPL needs
a device tree to operate on your platform. You can add a u-boot-spl-dtb
entry after this one, or use a u-boot-spl entry instead' which normally
expands to a section containing u-boot-spl-dtb, u-boot-spl-bss-pad and
u-boot-spl-dtb

SPL can access binman symbols at runtime. See :ref:`binman_fdt`.

in the binman README for more information.

The ELF file 'spl/u-boot-spl' must also be available for this to work, since
binman uses that to look up symbols to write into the SPL binary.



.. _etype_u_boot_spl_pubkey_dtb:

Entry: u-boot-spl-pubkey-dtb: U-Boot SPL device tree including public key
-------------------------------------------------------------------------

Properties / Entry arguments:
    - key-name-hint: Public key name without extension (.crt).
                Default is determined by underlying
                bintool (fdt_add_pubkey), usually 'key'.
    - algo: (Optional) Algorithm used for signing. Default is determined by
            underlying bintool (fdt_add_pubkey), usually 'sha1,rsa2048'
    - required: (Optional) If present this indicates that the key must be
                verified for the image / configuration to be
                considered valid

The following example shows an image containing an SPL which
is packed together with the dtb. Binman will add a signature
node to the dtb.

Example node::

    image {
    ...
        spl {
            filename = "spl.bin"

            u-boot-spl-nodtb {
            };
            u-boot-spl-pubkey-dtb {
                algo = "sha384,rsa4096";
                required = "conf";
                key-name-hint = "dev";
            };
        };
    ...
    }



.. _etype_u_boot_spl_with_ucode_ptr:

Entry: u-boot-spl-with-ucode-ptr: U-Boot SPL with embedded microcode pointer
----------------------------------------------------------------------------

This is used when SPL must set up the microcode for U-Boot.

See Entry_u_boot_ucode for full details of the entries involved in this
process.



.. _etype_u_boot_tpl:

Entry: u-boot-tpl: U-Boot TPL binary
------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot-tpl.bin (default 'tpl/u-boot-tpl.bin')

This is the U-Boot TPL (Tertiary Program Loader) binary. This is a small
binary which loads before SPL, typically into on-chip SRAM. It is
responsible for locating, loading and jumping to SPL, the next-stage
loader. Note that SPL is not relocatable so must be loaded to the correct
address in SRAM, or written to run from the correct address if direct
flash execution is possible (e.g. on x86 devices).

SPL can access binman symbols at runtime. See :ref:`binman_fdt`.

in the binman README for more information.

The ELF file 'tpl/u-boot-tpl' must also be available for this to work, since
binman uses that to look up symbols to write into the TPL binary.

Note that this entry is automatically replaced with u-boot-tpl-expanded
unless --no-expanded is used or the node has a 'no-expanded' property.



.. _etype_u_boot_tpl_bss_pad:

Entry: u-boot-tpl-bss-pad: U-Boot TPL binary padded with a BSS region
---------------------------------------------------------------------

Properties / Entry arguments:
    None

This holds the padding added after the TPL binary to cover the BSS (Block
Started by Symbol) region. This region holds the various variables used by
TPL. It is set to 0 by TPL when it starts up. If you want to append data to
the TPL image (such as a device tree file), you must pad out the BSS region
to avoid the data overlapping with U-Boot variables. This entry is useful in
that case. It automatically pads out the entry size to cover both the code,
data and BSS.

The contents of this entry will a certain number of zero bytes, determined
by __bss_size

The ELF file 'tpl/u-boot-tpl' must also be available for this to work, since
binman uses that to look up the BSS address.



.. _etype_u_boot_tpl_dtb:

Entry: u-boot-tpl-dtb: U-Boot TPL device tree
---------------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot.dtb (default 'tpl/u-boot-tpl.dtb')

This is the TPL device tree, containing configuration information for
TPL. TPL needs this to know what devices are present and which drivers
to activate.



.. _etype_u_boot_tpl_dtb_with_ucode:

Entry: u-boot-tpl-dtb-with-ucode: U-Boot TPL with embedded microcode pointer
----------------------------------------------------------------------------

This is used when TPL must set up the microcode for U-Boot.

See Entry_u_boot_ucode for full details of the entries involved in this
process.



.. _etype_u_boot_tpl_elf:

Entry: u-boot-tpl-elf: U-Boot TPL ELF image
-------------------------------------------

Properties / Entry arguments:
    - filename: Filename of TPL u-boot (default 'tpl/u-boot-tpl')

This is the U-Boot TPL ELF image. It does not include a device tree but can
be relocated to any address for execution.



.. _etype_u_boot_tpl_expanded:

Entry: u-boot-tpl-expanded: U-Boot TPL flat binary broken out into its component parts
--------------------------------------------------------------------------------------

Properties / Entry arguments:
    - tpl-dtb: Controls whether this entry is selected (set to 'y' or '1' to
        select)

This is a section containing the U-Boot binary, BSS padding if needed and a
devicetree. Using this entry type automatically creates this section, with
the following entries in it:

   u-boot-tpl-nodtb
   u-boot-tpl-bss-pad
   u-boot-dtb

Having the devicetree separate allows binman to update it in the final
image, so that the entries positions are provided to the running U-Boot.

This entry is selected based on the value of the 'tpl-dtb' entryarg. If
this is non-empty (and not 'n' or '0') then this expanded entry is selected.



.. _etype_u_boot_tpl_nodtb:

Entry: u-boot-tpl-nodtb: TPL binary without device tree appended
----------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename to include (default 'tpl/u-boot-tpl-nodtb.bin')

This is the U-Boot TPL binary, It does not include a device tree blob at
the end of it so may not be able to work without it, assuming TPL needs
a device tree to operate on your platform. You can add a u-boot-tpl-dtb
entry after this one, or use a u-boot-tpl entry instead, which normally
expands to a section containing u-boot-tpl-dtb, u-boot-tpl-bss-pad and
u-boot-tpl-dtb

TPL can access binman symbols at runtime. See :ref:`binman_fdt`.

in the binman README for more information.

The ELF file 'tpl/u-boot-tpl' must also be available for this to work, since
binman uses that to look up symbols to write into the TPL binary.



.. _etype_u_boot_tpl_with_ucode_ptr:

Entry: u-boot-tpl-with-ucode-ptr: U-Boot TPL with embedded microcode pointer
----------------------------------------------------------------------------

See Entry_u_boot_ucode for full details of the entries involved in this
process.



.. _etype_u_boot_ucode:

Entry: u-boot-ucode: U-Boot microcode block
-------------------------------------------

Properties / Entry arguments:
    None

The contents of this entry are filled in automatically by other entries
which must also be in the image.

U-Boot on x86 needs a single block of microcode. This is collected from
the various microcode update nodes in the device tree. It is also unable
to read the microcode from the device tree on platforms that use FSP
(Firmware Support Package) binaries, because the API requires that the
microcode is supplied before there is any SRAM available to use (i.e.
the FSP sets up the SRAM / cache-as-RAM but does so in the call that
requires the microcode!). To keep things simple, all x86 platforms handle
microcode the same way in U-Boot (even non-FSP platforms). This is that
a table is placed at _dt_ucode_base_size containing the base address and
size of the microcode. This is either passed to the FSP (for FSP
platforms), or used to set up the microcode (for non-FSP platforms).
This all happens in the build system since it is the only way to get
the microcode into a single blob and accessible without SRAM.

There are two cases to handle. If there is only one microcode blob in
the device tree, then the ucode pointer it set to point to that. This
entry (u-boot-ucode) is empty. If there is more than one update, then
this entry holds the concatenation of all updates, and the device tree
entry (u-boot-dtb-with-ucode) is updated to remove the microcode. This
last step ensures that that the microcode appears in one contiguous
block in the image and is not unnecessarily duplicated in the device
tree. It is referred to as 'collation' here.

Entry types that have a part to play in handling microcode:

    Entry_u_boot_with_ucode_ptr:
        Contains u-boot-nodtb.bin (i.e. U-Boot without the device tree).
        It updates it with the address and size of the microcode so that
        U-Boot can find it early on start-up.
    Entry_u_boot_dtb_with_ucode:
        Contains u-boot.dtb. It stores the microcode in a
        'self.ucode_data' property, which is then read by this class to
        obtain the microcode if needed. If collation is performed, it
        removes the microcode from the device tree.
    Entry_u_boot_ucode:
        This class. If collation is enabled it reads the microcode from
        the Entry_u_boot_dtb_with_ucode entry, and uses it as the
        contents of this entry.



.. _etype_u_boot_vpl:

Entry: u-boot-vpl: U-Boot VPL binary
------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot-vpl.bin (default 'vpl/u-boot-vpl.bin')

This is the U-Boot VPL (Verifying Program Loader) binary. This is a small
binary which loads before SPL, typically into on-chip SRAM. It is
responsible for locating, loading and jumping to SPL, the next-stage
loader. Note that VPL is not relocatable so must be loaded to the correct
address in SRAM, or written to run from the correct address if direct
flash execution is possible (e.g. on x86 devices).

SPL can access binman symbols at runtime. See :ref:`binman_fdt`.

in the binman README for more information.

The ELF file 'vpl/u-boot-vpl' must also be available for this to work, since
binman uses that to look up symbols to write into the VPL binary.



.. _etype_u_boot_vpl_bss_pad:

Entry: u-boot-vpl-bss-pad: U-Boot VPL binary padded with a BSS region
---------------------------------------------------------------------

Properties / Entry arguments:
    None

This holds the padding added after the VPL binary to cover the BSS (Block
Started by Symbol) region. This region holds the various variables used by
VPL. It is set to 0 by VPL when it starts up. If you want to append data to
the VPL image (such as a device tree file), you must pad out the BSS region
to avoid the data overlapping with U-Boot variables. This entry is useful in
that case. It automatically pads out the entry size to cover both the code,
data and BSS.

The contents of this entry will a certain number of zero bytes, determined
by __bss_size

The ELF file 'vpl/u-boot-vpl' must also be available for this to work, since
binman uses that to look up the BSS address.



.. _etype_u_boot_vpl_dtb:

Entry: u-boot-vpl-dtb: U-Boot VPL device tree
---------------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot.dtb (default 'vpl/u-boot-vpl.dtb')

This is the VPL device tree, containing configuration information for
VPL. VPL needs this to know what devices are present and which drivers
to activate.



.. _etype_u_boot_vpl_elf:

Entry: u-boot-vpl-elf: U-Boot VPL ELF image
-------------------------------------------

Properties / Entry arguments:
    - filename: Filename of VPL u-boot (default 'vpl/u-boot-vpl')

This is the U-Boot VPL ELF image. It does not include a device tree but can
be relocated to any address for execution.



.. _etype_u_boot_vpl_expanded:

Entry: u-boot-vpl-expanded: U-Boot VPL flat binary broken out into its component parts
--------------------------------------------------------------------------------------

Properties / Entry arguments:
    - vpl-dtb: Controls whether this entry is selected (set to 'y' or '1' to
        select)

This is a section containing the U-Boot binary, BSS padding if needed and a
devicetree. Using this entry type automatically creates this section, with
the following entries in it:

   u-boot-vpl-nodtb
   u-boot-vpl-bss-pad
   u-boot-dtb

Having the devicetree separate allows binman to update it in the final
image, so that the entries positions are provided to the running U-Boot.

This entry is selected based on the value of the 'vpl-dtb' entryarg. If
this is non-empty (and not 'n' or '0') then this expanded entry is selected.



.. _etype_u_boot_vpl_nodtb:

Entry: u-boot-vpl-nodtb: VPL binary without device tree appended
----------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename to include (default 'vpl/u-boot-vpl-nodtb.bin')

This is the U-Boot VPL binary, It does not include a device tree blob at
the end of it so may not be able to work without it, assuming VPL needs
a device tree to operate on your platform. You can add a u_boot_vpl_dtb
entry after this one, or use a u_boot_vpl entry instead, which normally
expands to a section containing u-boot-vpl-dtb, u-boot-vpl-bss-pad and
u-boot-vpl-dtb

VPL can access binman symbols at runtime. See :ref:`binman_fdt`.

The ELF file 'vpl/u-boot-vpl' must also be available for this to work, since
binman uses that to look up symbols to write into the VPL binary.



.. _etype_u_boot_with_ucode_ptr:

Entry: u-boot-with-ucode-ptr: U-Boot with embedded microcode pointer
--------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot-nodtb.bin (default 'u-boot-nodtb.bin')
    - optional-ucode: boolean property to make microcode optional. If the
        u-boot.bin image does not include microcode, no error will
        be generated.

See Entry_u_boot_ucode for full details of the three entries involved in
this process. This entry updates U-Boot with the offset and size of the
microcode, to allow early x86 boot code to find it without doing anything
complicated. Otherwise it is the same as the u-boot entry.



.. _etype_vblock:

Entry: vblock: An entry which contains a Chromium OS verified boot block
------------------------------------------------------------------------

Properties / Entry arguments:
    - content: List of phandles to entries to sign
    - keydir: Directory containing the public keys to use
    - keyblock: Name of the key file to use (inside keydir)
    - signprivate: Name of provide key file to use (inside keydir)
    - version: Version number of the vblock (typically 1)
    - kernelkey: Name of the kernel key to use (inside keydir)
    - preamble-flags: Value of the vboot preamble flags (typically 0)

Output files:
    - input.<unique_name> - input file passed to futility
    - vblock.<unique_name> - output file generated by futility (which is
        used as the entry contents)

Chromium OS signs the read-write firmware and kernel, writing the signature
in this block. This allows U-Boot to verify that the next firmware stage
and kernel are genuine.



.. _etype_x509_cert:

Entry: x509-cert: An entry which contains an X509 certificate
-------------------------------------------------------------

Properties / Entry arguments:
    - content: List of phandles to entries to sign

Output files:
    - input.<unique_name> - input file passed to openssl
    - cert.<unique_name> - output file generated by openssl (which is
        used as the entry contents)

openssl signs the provided data, writing the signature in this entry. This
allows verification that the data is genuine



.. _etype_x86_reset16:

Entry: x86-reset16: x86 16-bit reset code for U-Boot
----------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot-x86-reset16.bin (default
        'u-boot-x86-reset16.bin')

x86 CPUs start up in 16-bit mode, even if they are 32-bit CPUs. This code
must be placed at a particular address. This entry holds that code. It is
typically placed at offset CONFIG_RESET_VEC_LOC. The code is responsible
for jumping to the x86-start16 code, which continues execution.

For 64-bit U-Boot, the 'x86_reset16_spl' entry type is used instead.



.. _etype_x86_reset16_spl:

Entry: x86-reset16-spl: x86 16-bit reset code for U-Boot
--------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot-x86-reset16.bin (default
        'u-boot-x86-reset16.bin')

x86 CPUs start up in 16-bit mode, even if they are 32-bit CPUs. This code
must be placed at a particular address. This entry holds that code. It is
typically placed at offset CONFIG_RESET_VEC_LOC. The code is responsible
for jumping to the x86-start16 code, which continues execution.

For 32-bit U-Boot, the 'x86_reset_spl' entry type is used instead.



.. _etype_x86_reset16_tpl:

Entry: x86-reset16-tpl: x86 16-bit reset code for U-Boot
--------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot-x86-reset16.bin (default
        'u-boot-x86-reset16.bin')

x86 CPUs start up in 16-bit mode, even if they are 32-bit CPUs. This code
must be placed at a particular address. This entry holds that code. It is
typically placed at offset CONFIG_RESET_VEC_LOC. The code is responsible
for jumping to the x86-start16 code, which continues execution.

For 32-bit U-Boot, the 'x86_reset_tpl' entry type is used instead.



.. _etype_x86_start16:

Entry: x86-start16: x86 16-bit start-up code for U-Boot
-------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot-x86-start16.bin (default
        'u-boot-x86-start16.bin')

x86 CPUs start up in 16-bit mode, even if they are 32-bit CPUs. This code
must be placed in the top 64KB of the ROM. The reset code jumps to it. This
entry holds that code. It is typically placed at offset
CONFIG_SYS_X86_START16. The code is responsible for changing to 32-bit mode
and jumping to U-Boot's entry point, which requires 32-bit mode (for 32-bit
U-Boot).

For 64-bit U-Boot, the 'x86_start16_spl' entry type is used instead.



.. _etype_x86_start16_spl:

Entry: x86-start16-spl: x86 16-bit start-up code for SPL
--------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of spl/u-boot-x86-start16-spl.bin (default
        'spl/u-boot-x86-start16-spl.bin')

x86 CPUs start up in 16-bit mode, even if they are 32-bit CPUs. This code
must be placed in the top 64KB of the ROM. The reset code jumps to it. This
entry holds that code. It is typically placed at offset
CONFIG_SYS_X86_START16. The code is responsible for changing to 32-bit mode
and jumping to U-Boot's entry point, which requires 32-bit mode (for 32-bit
U-Boot).

For 32-bit U-Boot, the 'x86-start16' entry type is used instead.



.. _etype_x86_start16_tpl:

Entry: x86-start16-tpl: x86 16-bit start-up code for TPL
--------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of tpl/u-boot-x86-start16-tpl.bin (default
        'tpl/u-boot-x86-start16-tpl.bin')

x86 CPUs start up in 16-bit mode, even if they are 32-bit CPUs. This code
must be placed in the top 64KB of the ROM. The reset code jumps to it. This
entry holds that code. It is typically placed at offset
CONFIG_SYS_X86_START16. The code is responsible for changing to 32-bit mode
and jumping to U-Boot's entry point, which requires 32-bit mode (for 32-bit
U-Boot).

If TPL is not being used, the 'x86-start16-spl or 'x86-start16' entry types
may be used instead.



.. _etype_xilinx_bootgen:

Entry: xilinx-bootgen: Signed SPL boot image for Xilinx ZynqMP devices
----------------------------------------------------------------------

Properties / Entry arguments:
    - auth-params: (Optional) Authentication parameters passed to bootgen
    - fsbl-config: (Optional) FSBL parameters passed to bootgen
    - keysrc-enc: (Optional) Key source when using decryption engine
    - pmufw-filename: Filename of PMU firmware. Default: pmu-firmware.elf
    - psk-key-name-hint: Name of primary secret key to use for signing the
                         secondardy public key. Format: .pem file
    - ssk-key-name-hint: Name of secondardy secret key to use for signing
                         the boot image. Format: .pem file

The etype is used to create a boot image for Xilinx ZynqMP
devices.

Information for signed images:

In AMD/Xilinx SoCs, two pairs of public and secret keys are used
- primary and secondary. The function of the primary public/secret key pair
is to authenticate the secondary public/secret key pair.
The function of the secondary key is to sign/verify the boot image. [1]

AMD/Xilinx uses the following terms for private/public keys [1]:

    PSK = Primary Secret Key (Used to sign Secondary Public Key)
    PPK = Primary Public Key (Used to verify Secondary Public Key)
    SSK = Secondary Secret Key (Used to sign the boot image/partitions)
    SPK = Used to verify the actual boot image

The following example builds a signed boot image. The fuses of
the primary public key (ppk) should be fused together with the RSA_EN flag.

Example node::

    spl {
        filename = "boot.signed.bin";

        xilinx-bootgen {
            psk-key-name-hint = "psk0";
            ssk-key-name-hint = "ssk0";
            auth-params = "ppk_select=0", "spk_id=0x00000000";

            u-boot-spl-nodtb {
            };
            u-boot-spl-pubkey-dtb {
                algo = "sha384,rsa4096";
                required = "conf";
                key-name-hint = "dev";
            };
        };
    };

For testing purposes, e.g. if no RSA_EN should be fused, one could add
the "bh_auth_enable" flag in the fsbl-config field. This will skip the
verification of the ppk fuses and boot the image, even if ppk hash is
invalid.

Example node::

    xilinx-bootgen {
        psk-key-name-hint = "psk0";
        psk-key-name-hint = "ssk0";
        ...
        fsbl-config = "bh_auth_enable";
        ...
    };

[1] https://docs.xilinx.com/r/en-US/ug1283-bootgen-user-guide/Using-Authentication




