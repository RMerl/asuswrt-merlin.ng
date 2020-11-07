#!/usr/bin/env python2.7

# Copyright (c) 2016 by Broadcom Corporation
# $Id:  $

"""
Purpose: Combine several 'event log' files into one .bea file, so DHD has to
load only one instead of multiple files. .bea uses the MSF Broadcom defined
format.
"""

from __future__ import print_function

import os
import sys
import logging
import re            # regular expressions for parsing e.g. firmware map file
import argparse
import struct        # to write binary C structures

import msf           # .bea file format follows Multi Segment File format

SIZEOF_UINT32 = 4

# BEA MSF segment types
BEA_TYP_RTECDC_BIN = 1  # contains the full rtecdc.bin
BEA_TYP_LOGSTRS_BIN = 2
BEA_TYP_FW_SYMBOLS = 3
BEA_TYP_ROML_BIN = 4    # contains the event log relevant section of roml.bin

BEA_TYP_TO_STR = {
    BEA_TYP_RTECDC_BIN: "RTECDC_BIN",
    BEA_TYP_LOGSTRS_BIN: "LOGSTRS_BIN",
    BEA_TYP_FW_SYMBOLS: "FW_SYMBOLS",
    BEA_TYP_ROML_BIN: "ROML_BIN",
}


def parse_args():
    """Defines command line argument and parses command line"""
    usage = "usage: %prog [options]"
    description = "binary_eventlog_archive.py - a Python utility to create " + \
        ".bea files for dongle event logging. rvossen@broadcom.com"

    version = "%prog v0.1 - a utility for Broadcom software development"
    usage = version + "\n" + usage

    parser = argparse.ArgumentParser(description=description)

    parser.add_argument("--version", action="version", version=version)

    create_group = parser.add_argument_group(
        title="create",
        description="arguments required when creating a .bea file")
    create_group.add_argument(
        "-c", "--create", action="store_true", dest="create", help="create")
    create_group.add_argument(
        "--rtecdc_bin", action="store", dest="rtecdc_bin",
        help="File path containing binary RAM firmware image")
    create_group.add_argument(
        "--logstrs_bin", action="store", dest="logstrs_bin",
        help="File path containing event log strings")
    create_group.add_argument(
        "--rtecdc_map", action="store", dest="rtecdc_map",
        help="File path containing ROM offload map file")
    create_group.add_argument(
        "--roml_bin", action="store", dest="roml_bin",
        help="File path containing binary ROM library image")
    create_group.add_argument(
        "--roml_map", action="store", dest="roml_map",
        help="File path containing ROM library map file")
    dump_group = parser.add_argument_group(
        title="dump", description="arguments required when dumping a .bea file")
    dump_group.add_argument(
        "-d", "--dump", action="store_true", dest="dump", help="dump")
    parser.add_argument(
        "beafilename", action="store",
        help="name of bea file to be created or parsed")

    args = parser.parse_args()

    if not args.dump and not args.create:
        print("Missing --dump or --create on command line")
        exit(-1)

    if args.beafilename == "":
        print("Missing .bea file on command line")
        exit(-1)

    # when creating a file, all 'create' command line arguments are mandatory
    if args.create:
        create_args = [args.rtecdc_bin, args.logstrs_bin, args.rtecdc_map,
                       args.roml_bin, args.roml_map]
        for arg in create_args:
            if arg == "":
                print("Missing one or more 'create' command line parameters")
                exit(-1)

    return args


def main():
    """the main method"""
    args = parse_args()

    if args.create:
        bea_file = BEAFile()
        rte_cdc_bin = BeaBinaryBlob(typ=BEA_TYP_RTECDC_BIN)
        rte_cdc_bin.set_content_from_file(args.rtecdc_bin)
        bea_file.append_segment(rte_cdc_bin)
        log_strs_bin = BeaBinaryBlob(typ=BEA_TYP_LOGSTRS_BIN)
        log_strs_bin.set_content_from_file(args.logstrs_bin)
        bea_file.append_segment(log_strs_bin)

        rtecdc_map = MapFile(args.rtecdc_map)
        rtecdc_symbols = BeaSymbols(
            rtecdc_map.get_symbol("text_start"),
            rtecdc_map.get_symbol("rodata_start"),
            rtecdc_map.get_symbol("rodata_end"))
        bea_file.append_segment(rtecdc_symbols)

        # read roml.bin file and copy a section of that file into a segment
        roml_bin = BeaBinaryBlob(typ=BEA_TYP_ROML_BIN)
        roml_bin.set_content(create_roml_bin_segment(args))
        bea_file.append_segment(roml_bin)

        bea_file.write_file(args.beafilename)

    if args.dump:
        bea_file = BEAFile()
        seg_header_list = msf.read_msf_file(args.beafilename)
        for (seg_type, seg_contents) in seg_header_list:
            if (
                seg_type == BEA_TYP_RTECDC_BIN or
                seg_type == BEA_TYP_LOGSTRS_BIN or
                seg_type == BEA_TYP_ROML_BIN
            ):
                new_segment = BeaBinaryBlob.create_from_bytestream(
                    seg_type, seg_contents)
            elif seg_type == BEA_TYP_FW_SYMBOLS:
                new_segment = BeaSymbols.create_from_bytestream(
                    seg_type, seg_contents)
            else:
                raise Exception(
                    "%d is not a valid .bea segment type" % seg_type)
            bea_file.append_segment(new_segment)
        print(str(bea_file))


class MapFile(object):
    """a file containing firmware symbols (e.g. rtecdc.map or roml.map)"""
    def __init__(self, file_name):
        self.fp = open(file_name, "r")

    def get_symbol(self, search_sym_name):
        """returns address of caller supplied symbol name"""
        self.fp.seek(0, os.SEEK_SET)  # rewind
        r_sym = re.compile(r"(\w+)\s+(\S+)\s+([\w$]+)")
        for line in self.fp:
            # line is eg '001e06d0 R rodata_start'
            mo = r_sym.match(line)
            if mo is None:
                print("Warning: could not parse .map line %s" % line)
            else:
                sym_addr = int(mo.group(1), 16)
                # sym_type = mo.group(2)
                sym_name = mo.group(3)
                if sym_name == search_sym_name:
                    return sym_addr
        return None


class MsfSegProperties(object):
    """Superclass, MSF segment properties"""
    def __init__(self, typ, length=0):
        self.typ = typ
        self.len = length

    def __str__(self):
        s = "typ = %d(%s) len = %d\n" % \
            (self.typ, BEA_TYP_TO_STR.get(self.typ, "?UNKNOWN?"),
             self.len)
        return s


class BeaBinaryBlob(MsfSegProperties):
    """A RAM / ROM offload image, or a logstr.bin embedded in a BEA file"""
    def __init__(self, typ=BEA_TYP_RTECDC_BIN):
        MsfSegProperties.__init__(self, typ=typ)
        self.payload = None

    @classmethod
    def create_from_bytestream(cls, seg_typ, byte_stream):
        """used when reading BEA file, construct object from file"""
        obj = cls()
        obj.typ = seg_typ
        obj.payload = byte_stream
        obj.len = len(byte_stream)
        return obj

    def get_byte_stream(self):
        """invoked when writing a file"""
        return self.payload

    def set_content(self, byte_stream):
        """called when constructing BEA files"""
        self.payload = byte_stream
        self.len = len(byte_stream)

    def set_content_from_file(self, rtecdc_bin_file_name):
        """called when constructing BEA files"""
        f = open(rtecdc_bin_file_name, "rb")
        self.set_content(f.read())
        f.close()


class BeaSymbols(MsfSegProperties):
    """Firmware symbol information embedded in a BEA file"""
    def __init__(self, text_start, rodata_start, rodata_end):
        MsfSegProperties.__init__(
            self, typ=BEA_TYP_FW_SYMBOLS, length=8 * SIZEOF_UINT32)
        self.text_start = text_start
        self.rodata_start = rodata_start
        self.rodata_end = rodata_end
        self.unused_syms = [0] * 5  # for future expansion

    def get_byte_stream(self):
        """called when constructing BEA files"""
        s = struct.pack(
            "III", self.text_start, self.rodata_start, self.rodata_end)
        s += struct.pack("IIIII", 0, 0, 0, 0, 0)  # unused words
        return s

    @classmethod
    def create_from_bytestream(cls, seg_type, byte_stream):
        """used when reading BEA file, construct object from file"""
        obj = cls(0, 0, 0)
        obj.typ = seg_type
        (obj.text_start, obj.rodata_start, obj.rodata_end) = \
            struct.unpack("III", byte_stream[0:3 * SIZEOF_UINT32])
        return obj

    def __str__(self):
        s = MsfSegProperties.__str__(self)
        s += "    text_start = 0x%08X rodata_start = 0x%08X " \
            "rodata_end = 0x%08X\n" % (
                self.text_start, self.rodata_start, self.rodata_end)
        return s


class BEAFile(object):
    """Class for reading and constructing a .bea file"""
    def __init__(self):
        self.segments = []

    def __str__(self):
        s = ""
        for segment in self.segments:
            s += str(segment)
        return s

    def append_segment(self, segment):
        """append MSF segment to file"""
        self.segments.append(segment)

    def write_file(self, output_file_name):
        """write .bea file that was constructed in memory"""
        seg_header_list = []
        for segment in self.segments:
            seg_header_list.append(
                (segment.typ, segment.get_byte_stream()))
        msf.create_msf_file(seg_header_list, output_file_name)


def create_roml_bin_segment(args):
    """reads a part of roml.bin"""
    fp = open(args.roml_bin, "rb")
    roml_map = MapFile(args.roml_map)
    text_start = roml_map.get_symbol("text_start")
    rodata_start = roml_map.get_symbol("rodata_start")
    rodata_end = roml_map.get_symbol("rodata_end")
    logfilebase = rodata_start - text_start
    fp.seek(logfilebase, os.SEEK_SET)
    contents = fp.read(rodata_end - rodata_start)
    fp.close()
    return contents


# Standard boilerplate to call the main() function
if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        logging.error(str(e))
        sys.exit(1)

# vim: ts = 8:sw = 4:tw = 80:et:
