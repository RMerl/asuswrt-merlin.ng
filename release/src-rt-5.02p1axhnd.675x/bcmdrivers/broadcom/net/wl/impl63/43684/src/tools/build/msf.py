#!/usr/bin/env python2.7

# Copyright (c) 2014 by Broadcom Corporation

"""
Pack generic data into an MSF-formatted file.

MSF is a generic data file format which is used to wrap multiple segments
of data into a single binary data file.

EXAMPLE:

    msf.py -f data.bin [type] -i 0x4350 [type] -s "any text" [type] -o msf.out

struct wl_segment {
    uint32 type;
    uint32 offset;
    uint32 length;
    uint32 crc32;
    uint32 flags;
};

/* MSF file header */
struct wl_segment_info {
    uint8        magic[4];
    uint32       hdr_len;
    uint32       crc32;
    uint32       file_type;
    uint32       num_segments;
    wl_segment_t segments[1];
};
"""

from __future__ import print_function

import argparse
import binascii
import logging
import struct
import sys

SIZEOF_UINT32 = 4
SIZEOF_UINT8 = 1

logging.basicConfig(
    format="[%(module)s:%(levelname)s] %(message)s",
    level=logging.INFO)
if sys.version_info[0] != 2 or sys.version_info[1] != 7:
    sys.exit("Currently supports only Python 2.7")


def create_msf_file(seg_header_list=None, fout="msf.out"):
    """writes MSF file to disk, given caller supplied type/payload list"""
    if not seg_header_list:
        return "No MSF segment"
    num_segments = len(seg_header_list)
    logging.info("Total segments: %d\n", len(seg_header_list))

    # MSF construction
    seg_struct = "<I I I I I"             # see struct wl_segment
    header_prefix_struct = "<4s I I I I"  # see struct wl_segment_info
    header_prefix_magic = "BLOB"

    seg = struct.pack(seg_struct, 0, 0, 0, 0, 0)
    header_prefix = struct.pack(header_prefix_struct, header_prefix_magic,
                                0, 0, 0, 0)
    header_prefix_len = len(header_prefix)
    header_len = header_prefix_len + len(seg) * num_segments
    header_prefix = struct.pack(header_prefix_struct, header_prefix_magic,
                                header_len, 0, 1, num_segments)

    seg_offset = header_len
    header_crc32 = binascii.crc32(header_prefix[12:])
    seg_list = []
    for seg_hdr in seg_header_list:
        seg_data_type = seg_hdr[0]    # this is a "tag" for the type of segment
        seg_data = seg_hdr[1]         # the "blob", a string of bytes
        seg_data_len = len(seg_data)
        seg_data_crc32 = binascii.crc32(seg_data) & 0xffffffff
        seg = struct.pack(seg_struct, seg_data_type, seg_offset,
                          seg_data_len, seg_data_crc32, 0)
        header_crc32 = binascii.crc32(seg, header_crc32)
        seg_offset += seg_data_len
        seg_list.append((seg, seg_data))
        logging.info("segment: - type %d len %d crc32 0x%x",
                     seg_data_type, seg_data_len, seg_data_crc32)

    header_crc32 = header_crc32 & 0xffffffff
    header_prefix = struct.pack(header_prefix_struct, header_prefix_magic,
                                header_len, header_crc32, 1, num_segments)
    logging.info("header: - num_segments %d len %d crc32 0x%x\n",
                 num_segments, header_len, header_crc32)

    # Write output
    try:
        with open(fout, "wb") as fout_hdl:
            fout_hdl.write(header_prefix)
            for seg in seg_list:
                fout_hdl.write(seg[0])
            for seg in seg_list:
                fout_hdl.write(seg[1])
    except IOError as err:
        return str(err)
    fout_hdl.close()


def read_msf_file(input_file_path):
    """reads MSF file from disk and returns segment contents"""
    try:
        with open(input_file_path, "rb") as fin_hdl:
            header_prefix_struct = "<4s I I I I"
            byte_stream = fin_hdl.read(4 * SIZEOF_UINT8 + 4 * SIZEOF_UINT32)
            (header_prefix_magic, hdr_len, crc32, file_type, num_segments) = \
                struct.unpack(header_prefix_struct, byte_stream)
            if header_prefix_magic != "BLOB":
                raise Exception("Not a MSF file, invalid magic number in " +
                                "file header")
            if hdr_len == 0:
                raise Exception("Not a MSF file, invalid header length")
            if file_type != 1:
                raise Exception(
                    "Not a MSF file, invalid header file type %d" % file_type)
            seg_list = []
            for i_seg in range(num_segments):
                seg_struct = "<I I I I I"            # see struct wl_segment
                byte_stream = fin_hdl.read(5 * SIZEOF_UINT32)
                (seg_data_type, seg_offset, seg_data_len, seg_data_crc32,
                 seg_data_flags) = struct.unpack(seg_struct, byte_stream)
                seg_list.append(
                    (seg_data_type, seg_offset, seg_data_len, seg_data_crc32,
                     seg_data_flags))
            seg_list2 = []
            for (seg_data_type, seg_offset, seg_data_len, seg_data_crc32,
                 seg_data_flags) in seg_list:
                fin_hdl.seek(seg_offset)
                seg_list2.append((seg_data_type, fin_hdl.read(seg_data_len)))
    except IOError as err:
        return str(err)
    fin_hdl.close()
    return seg_list2


def main_parse_args_for_create(opts):
    """parses command line options when user wants to create a MSF file"""
    fout = opts.output

    # MSF segments identification
    seg_header_list = []
    if opts.file:
        for arg in opts.file:
            if not arg:
                return "Missing data file name"
            fin = arg[0]
            try:
                with open(fin, "rb") as fin_hdl:
                    seg_data = bytearray(fin_hdl.read())
            except IOError as err:
                return str(err)
            seg_data_type = 0
            if len(arg) > 1:
                seg_data_type = int(arg[1])
            # Append CRC for segment if enabled
            if opts.crc:
                seg_data_crc_val = binascii.crc32(seg_data) & 0xffffffff
                seg_data_crc_buf = struct.pack('<I', seg_data_crc_val)
                seg_data.append(seg_data_crc_buf[0])
                seg_data.append(seg_data_crc_buf[1])
                seg_data.append(seg_data_crc_buf[2])
                seg_data.append(seg_data_crc_buf[3])
            seg_header_list.append((seg_data_type, seg_data))
            logging.info("segment: - src %s type %d len %d",
                         arg, seg_data_type, len(seg_data))
    if opts.integer:
        for arg in opts.integer:
            if not arg:
                return "Missing integer value"
            seg_data = struct.pack("<I", int(arg[0], 0))
            seg_data_type = 0
            if len(arg) > 1:
                seg_data_type = int(arg[1])
            seg_header_list.append((seg_data_type, seg_data))
            logging.info("segment: - src %s type %d len %d",
                         arg, seg_data_type, len(seg_data))
    if opts.string:
        for arg in opts.string:
            if not arg:
                return "Missing string value"
            seg_data = bytearray(arg[0])
            seg_data.append(0)
            seg_data_type = 0
            if len(arg) > 1:
                seg_data_type = int(arg[1])
            seg_header_list.append((seg_data_type, seg_data))
            logging.info("segment: - src %s type %d len %d",
                         arg, seg_data_type, len(seg_data))

    return (seg_header_list, fout)


def main():
    """Entry point for standalone use."""

    # Command line argument processing
    parser = argparse.ArgumentParser(
        epilog=__doc__.strip(),
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument(
        "-f", "--file",
        action="append",
        metavar=("IFILE_DAT", "segment_type"),
        nargs="+",
        help="Input data file: IFILE_DAT with optional segment type")
    parser.add_argument(
        "-c", "--crc",
        action="store_true",
        help="Generate segment embedded CRC (only for data segemnt created with -f option")
    parser.add_argument(
        "-i", "--integer",
        action="append",
        metavar=("IDATA_INT", "segment_type"),
        nargs="+",
        help="Input integer value: IDATA_INT with optional segment type")
    parser.add_argument(
        "-s", "--string",
        action="append",
        metavar=("IDATA_STR", "segment_type"),
        nargs="+",
        help="Input string value: IDATA_STR with optional segment type")
    parser.add_argument(
        "-o", "--output",
        metavar="OFILE_MSF",
        help="Output MSF file: OFILE_MSF")

    parser.add_argument(
        "-d", "--dump",
        metavar="IFILE_MSF",
        help="Input MSF file: IFILE_MSF")

    opts = parser.parse_args()

    if not opts.output and not opts.dump:
        return "Missing MSF file name"

    if opts.output:
        (seg_header_list, fout) = main_parse_args_for_create(opts)
        create_msf_file(seg_header_list, fout)

    if opts.dump:
        seg_header_list = read_msf_file(opts.dump)
        for seg in seg_header_list:
            print("seg_type=%d seg_len=%d" % (seg[0], len(seg[1])))


# Standard boilerplate to call the main() function
if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        logging.error(str(e))
        sys.exit(1)

# vim: ts=8:sw=4:tw=80:et:
