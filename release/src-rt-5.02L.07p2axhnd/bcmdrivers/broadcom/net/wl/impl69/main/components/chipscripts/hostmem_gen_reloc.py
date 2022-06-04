#!/usr/bin/env python

#
#
# $ Copyright Broadcom $
#
#
# <<Broadcom-WL-IPTag/Proprietary:>>

"""
Implementation of host memory offload feature

This script generates the firmware combo file having below format
| MAGIC | TYPE | LEN | DATA | ... | TYPE | LEN | DATA |
 - MAGIC - COMBO file magic number - 0x434F464C
 - TLV blocks
      - TYPE 0 - Content of rtecdc.bin -  To be downloaded to RAM
      - TYPE 1 - Content of rtecdc_host.bin -
         This section contains portion of code to be
         offloaded to host memory
      - TYPE 2 - Relocation information for rtecdc.bin
         DHD uses this data to patch rtecdc.bin before downloading
      - TYPE 3 - Relocation information for rtecdc_host.bin
         DHD uses this data to patch rtecdc_host.bin before
         copying it to host memory
"""

from __future__ import print_function

import argparse
import os
import struct

COMBO_MAGIC = 0x434F464C


def generate_reloc_info(image, symbols, out, host_start_addr, host_end_addr):
    """
    Generate relocation information
    Algorithm:
    Calls from RAM to host offloaded sections use veneers only because of
    offset > 22 bits.
    To locate these veneers and function pointer table entries, read each
    word in the binary file and verify if value/address is within host
    memory region and bit 0 is set.
    If yes, check if map file has an entry for this address, if yes
    this is most likely a veneer entry or function pointer table entry.
    In future we may need to change the algorithm if there is
    coincidental match with real data or instructions.
    """

    offset = 0
    image_data = []
    image_size = 0
    with open(image, "rb") as image_fh:
        image_data = image_fh.read()
        image_size = len(image_data)

    reloc_fh = open(out, "wb")

    while (offset + 4) < image_size:
        val = struct.unpack("<L", image_data[offset:offset + 4])[0]
        # Check if the value is within the host address range
        # It is possible that some of the values may coincidently same
        # This case is not covered for now
        if host_start_addr <= val <= host_end_addr:
            # Function pointers will alway has bit 0 set in thumb mode
            if val & 1:
                ptr = val & 0xFFFFFFFE
                # Check if map file has an entry for this address
                if hex(ptr) in symbols:
                    # if yes mark this as a relocation entry
                    reloc_fh.write(struct.pack("<L", offset))
            else:
                # Since only text is relocated to host memory,
                # even address should not match
                if hex(val) in symbols:
                    # Just print for now to investigate specific entry
                    print("Data Symbol!!!", offset, val, symbols[hex(val)])

        offset += 4

    reloc_fh.close()


def write_data(out_fh, tlv_type, image):
    """ Write content of "image" file to output file in TLV format."""

    with open(image, "rb") as fh:
        tlv_len = os.stat(image).st_size
        data = fh.read()

        out_fh.write(struct.pack("<L", int(tlv_type, 16)))
        out_fh.write(struct.pack("<L", tlv_len))
        out_fh.write(data)


def get_symbols(map_file):
    """ Read all symbols from map file. """

    symbols = {}
    with open(map_file) as map_fh:
        for line in map_fh:
            (addr, _sym_type, symb) = line.split(" ")
            symbols[hex(int(addr, 16))] = symb

    return symbols


def main():
    """ Entry function. """

    parser = argparse.ArgumentParser(
        epilog=__doc__.strip(),
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument(
        "-ram",
        help="RAM Firmware image file",
        required=True
    )
    parser.add_argument(
        "-host",
        help="Firmware image file to be offloaded to host",
        required=True
    )
    parser.add_argument(
        "-map",
        help="Firmware map file",
        default="rtecdc.map"
    )
    parser.add_argument(
        "-ram_start",
        help="Start address of RAM",
        default="0x00000000"
    )
    parser.add_argument(
        "-host_start",
        help="Start address of host memory",
        default="0x00000000"
    )
    parser.add_argument(
        "-out",
        help="Output file name",
        required=True
    )
    parser.add_argument(
        "-ram_reloc",
        help="RAM image relocation info output file",
        default="rtecdc.ram.reloc"
    )
    parser.add_argument(
        "-host_reloc",
        help="Host image relocation info output file",
        default="rtecdc.host.reloc"
    )

    opts = parser.parse_args()

    # Host start address
    host_start_addr = int(opts.host_start, 0)

    # Generate relocation information if requested
    if host_start_addr:
        symbols = {}
        # read all symbols from the map file
        symbols = get_symbols(opts.map)

        # End address of the host image
        host_end_addr = os.stat(opts.host).st_size + host_start_addr

        # Generate ram image relocation info
        generate_reloc_info(opts.ram, symbols, opts.ram_reloc,
                            host_start_addr, host_end_addr)

        # Generate host binary relocation info
        generate_reloc_info(opts.host, symbols, opts.host_reloc,
                            host_start_addr, host_end_addr)

    # Pack all data into a combo file
    with open(opts.out, "wb") as out_fh:

        # Write Combo file magic number 0x434F464C
        out_fh.write(struct.pack("<L", COMBO_MAGIC))

        # Write RAM firmware image
        write_data(out_fh, "0", opts.ram)

        # Write host offloaded portion of firmware image
        write_data(out_fh, "1", opts.host)

        if host_start_addr:
            # Write ram image relocation info
            write_data(out_fh, "2", opts.ram_reloc)

            # Write host image relocation info
            write_data(out_fh, "3", opts.host_reloc)


if __name__ == "__main__":
    main()

# vim: ts=8:sw=4:et:tw=80
