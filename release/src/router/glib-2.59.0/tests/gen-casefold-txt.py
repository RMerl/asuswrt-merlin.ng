#!/usr/bin/env python3
# Copyright (C) 1998, 1999 Tom Tromey
# Copyright (C) 2001 Red Hat Software
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.

"""
gen-casefold-txt.py - Generate test cases for casefolding from Unicode data.
See http://www.unicode.org/Public/UNIDATA/UnicodeCharacterDatabase.html
Usage:
    I consider the output of this program to be unrestricted.
    Use it as you will.
"""

import sys
import argparse


def main(argv):
    parser = argparse.ArgumentParser(
        description="Generate test cases for casefolding from Unicode data")
    parser.add_argument("UNICODE-VERSION")
    parser.add_argument("CaseFolding.txt")
    args = parser.parse_args(argv[1:])
    version = getattr(args, "UNICODE-VERSION")
    filename = getattr(args, "CaseFolding.txt")

    print("""\
# Test cases generated from Unicode {} data
# by gen-casefold-txt.py. Do not edit.
#
# Some special hand crafted tests
#
AaBbCc@@\taabbcc@@
#
# Now the automatic tests
#""".format(version))

    # Names of fields in the CaseFolding table
    CODE, STATUS, MAPPING = range(3)

    with open(filename, encoding="utf-8") as fileobj:
        for line in fileobj:
            # strip comments and skip empty lines
            line = line.split("#", 1)[0].strip()
            if not line:
                continue

            fields = [f.strip() for f in line.split(";", 3)[:3]]
            if len(fields) != 3:
                raise SystemExit(
                    "Entry for %s has wrong number of fields (%d)" % (
                        fields[CODE], len(fields)))

            status = fields[STATUS]
            # skip simple and Turkic mappings
            if status in "ST":
                continue

            code = chr(int(fields[CODE], 16))
            values = "".join(
                [chr(int(v, 16)) for v in fields[MAPPING].split()])
            print("{}\t{}".format(code, values))


if __name__ == "__main__":
    sys.exit(main(sys.argv))
