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
gen-casemap-txt.py - Generate test cases for case mapping from Unicode data.
See http://www.unicode.org/Public/UNIDATA/UnicodeCharacterDatabase.html
Usage:
    I consider the output of this program to be unrestricted.
    Use it as you will.
"""

import sys
import argparse


def main(argv):
    parser = argparse.ArgumentParser(
        description="Generate test cases for case mapping from Unicode data")
    parser.add_argument("UNICODE-VERSION")
    parser.add_argument("UnicodeData.txt")
    parser.add_argument("SpecialCasing.txt")
    args = parser.parse_args(argv[1:])
    version = getattr(args, "UNICODE-VERSION")
    filename_udata = getattr(args, "UnicodeData.txt")
    filename_casing = getattr(args, "SpecialCasing.txt")

    # Names of fields in Unicode data table.
    CODE, NAME, CATEGORY, COMBINING_CLASSES, BIDI_CATEGORY, DECOMPOSITION, \
        DECIMAL_VALUE, DIGIT_VALUE, NUMERIC_VALUE, MIRRORED, OLD_NAME, \
        COMMENT, UPPER, LOWER, TITLE = range(15)

    # Names of fields in the SpecialCasing table
    CASE_CODE, CASE_LOWER, CASE_TITLE, CASE_UPPER, CASE_CONDITION = range(5)

    upper = {}
    title = {}
    lower = {}

    def make_hex(codes):
        """Converts a string of white space separated code points encoded as
        hex values to a Unicode string. Any extra white space is ignored.
        """
        return "".join([chr(int(c, 16)) for c in codes.split()])

    def process_one(code, fields):
        type_ = fields[CATEGORY]
        if type_ == "Ll":
            upper[code] = make_hex(fields[UPPER])
            lower[code] = chr(code)
            title[code] = make_hex(fields[TITLE])
        elif type_ == "Lu":
            lower[code] = make_hex(fields[LOWER])
            upper[code] = chr(code)
            title[code] = make_hex(fields[TITLE])
        elif type_ == "Lt":
            upper[code] = make_hex(fields[UPPER])
            lower[code] = make_hex(fields[LOWER])
            title[code] = make_hex(fields[LOWER])

    with open(filename_udata, encoding="utf-8") as fileobj:
        last_code = -1
        for line in fileobj:
            line = line.strip()
            fields = [f.strip() for f in line.split(";")]
            if len(fields) != 15:
                raise SystemExit(
                    "Entry for %s has wrong number of fields (%d)" % (
                        fields[CODE], len(fields)))

            code = int(fields[CODE], 16)

            if code > last_code + 1:
                # Found a gap
                if fields[NAME].endswith("Last>"):
                    # Fill the gap with the last character read,
                    # since this was a range specified in the char database
                    gfields = fields
                else:
                    # The gap represents undefined characters.  Only the type
                    # matters.
                    gfields = ['', '', 'Cn', '0', '', '', '', '', '', '', '',
                               '', '', '', '']

                last_code += 1
                while last_code < code:
                    gfields[CODE] = "%04x" % last_code
                    process_one(last_code, gfields)
                    last_code += 1

            process_one(code, fields)
            last_code = code

    with open(filename_casing, encoding="utf-8") as fileobj:
        last_code = -1
        for line in fileobj:
            # strip comments and skip empty lines
            line = line.split("#", 1)[0].strip()
            if not line:
                continue

            # all lines end with ";" so just remove it
            line = line.rstrip(";").rstrip()
            fields = [f.strip() for f in line.split(";")]
            if len(fields) not in (4, 5):
                raise SystemExit(
                    "Entry for %s has wrong number of fields (%d)" % (
                        fields[CASE_CODE], len(fields)))

            if len(fields) == 5:
                # Ignore conditional special cases - we'll handle them manually
                continue

            code = int(fields[CASE_CODE], 16)

            upper[code] = make_hex(fields[CASE_UPPER])
            lower[code] = make_hex(fields[CASE_LOWER])
            title[code] = make_hex(fields[CASE_TITLE])

    print_tests(version, upper, title, lower)


def print_tests(version, upper, title, lower):
    print("""\
# Test cases generated from Unicode {} data
# by gen-casemap-txt.py. Do not edit.
#
# Some special hand crafted tests
#
tr_TR\ti\ti\t\u0130\t\u0130\t# i => LATIN CAPITAL LETTER I WITH DOT ABOVE
tr_TR\tI\t\u0131\tI\tI\t# I => LATIN SMALL LETTER DOTLESS I
tr_TR\tI\u0307\ti\tI\u0307\tI\u0307\t# I => LATIN SMALL LETTER DOTLESS I
tr_TR.UTF-8\ti\ti\t\u0130\t\u0130\t# i => LATIN CAPITAL LETTER I WITH DOT ABOVE
tr_TR.UTF-8\tI\t\u0131\tI\tI\t# I => LATIN SMALL LETTER DOTLESS I
tr_TR.UTF-8\tI\u0307\ti\tI\u0307\tI\u0307\t# I => LATIN SMALL LETTER DOTLESS I
# Test reordering of YPOGEGRAMMENI across other accents
\t\u03b1\u0345\u0314\t\u03b1\u0345\u0314\t\u0391\u0345\u0314\t\u0391\u0314\u0399\t
\t\u03b1\u0314\u0345\t\u03b1\u0314\u0345\t\u0391\u0314\u0345\t\u0391\u0314\u0399\t
# Handling of final and nonfinal sigma
\tΜΆΙΟΣ 	μάιος 	Μάιος 	ΜΆΙΟΣ 	
\tΜΆΙΟΣ	μάιος	Μάιος	ΜΆΙΟΣ	
\tΣΙΓΜΑ	σιγμα	Σιγμα	ΣΙΓΜΑ	
# Lithuanian rule of i followed by letter with dot. Not at all sure
# about the titlecase part here
lt_LT\ti\u0117\ti\u0117\tIe\tIE\t
lt_LT\tie\u0307\tie\u0307\tIe\tIE\t
lt_LT\t\u00cc\ti\u0307\u0300\t\u00cc\t\u00cc\t # LATIN CAPITAL LETTER I WITH GRAVE
lt_LT\t\u00CD\ti\u0307\u0301\t\u00CD\t\u00CD\t # LATIN CAPITAL LETTER I WITH ACUTE
lt_LT\t\u0128\ti\u0307\u0303\t\u0128\t\u0128\t # LATIN CAPITAL LETTER I WITH TILDE
lt_LT\tI\u0301\ti\u0307\u0301\tI\u0301\tI\u0301\t # LATIN CAPITAL LETTER I (with acute accent)
lt_LT\tI\u0300\ti\u0307\u0300\tI\u0300\tI\u0300\t # LATIN CAPITAL LETTER I (with grave accent)
lt_LT\tI\u0303\ti\u0307\u0303\tI\u0303\tI\u0303\t # LATIN CAPITAL LETTER I (with tilde above)
lt_LT\tI\u0328\u0301\ti\u0307\u0328\u0301\tI\u0328\u0301\tI\u0328\u0301\t # LATIN CAPITAL LETTER I (with ogonek and acute accent)
lt_LT\tJ\u0301\tj\u0307\u0301\tJ\u0301\tJ\u0301\t # LATIN CAPITAL LETTER J (with acute accent)
lt_LT\t\u012e\u0301\t\u012f\u0307\u0301\t\u012e\u0301\t\u012e\u0301\t # LATIN CAPITAL LETTER I WITH OGONEK (with acute accent)
lt_LT.UTF-8\ti\u0117\ti\u0117\tIe\tIE\t
lt_LT.UTF-8\tie\u0307\tie\u0307\tIe\tIE\t
lt_LT.UTF-8\t\u00cc\ti\u0307\u0300\t\u00cc\t\u00cc\t # LATIN CAPITAL LETTER I WITH GRAVE
lt_LT.UTF-8\t\u00CD\ti\u0307\u0301\t\u00CD\t\u00CD\t # LATIN CAPITAL LETTER I WITH ACUTE
lt_LT.UTF-8\t\u0128\ti\u0307\u0303\t\u0128\t\u0128\t # LATIN CAPITAL LETTER I WITH TILDE
lt_LT.UTF-8\tI\u0301\ti\u0307\u0301\tI\u0301\tI\u0301\t # LATIN CAPITAL LETTER I (with acute accent)
lt_LT.UTF-8\tI\u0300\ti\u0307\u0300\tI\u0300\tI\u0300\t # LATIN CAPITAL LETTER I (with grave accent)
lt_LT.UTF-8\tI\u0303\ti\u0307\u0303\tI\u0303\tI\u0303\t # LATIN CAPITAL LETTER I (with tilde above)
lt_LT.UTF-8\tI\u0328\u0301\ti\u0307\u0328\u0301\tI\u0328\u0301\tI\u0328\u0301\t # LATIN CAPITAL LETTER I (with ogonek and acute accent)
lt_LT.UTF-8\tJ\u0301\tj\u0307\u0301\tJ\u0301\tJ\u0301\t # LATIN CAPITAL LETTER J (with acute accent)
lt_LT.UTF-8\t\u012e\u0301\t\u012f\u0307\u0301\t\u012e\u0301\t\u012e\u0301\t # LATIN CAPITAL LETTER I WITH OGONEK (with acute accent)
# Special case not at initial position
\ta\ufb04\ta\ufb04\tAffl\tAFFL\t# FB04
#
# Now the automatic tests
#""".format(version))

    for i in range(0x10ffff):
        if i == 0x3A3:
            # Greek sigma needs special tests
            continue

        up = upper.get(i, "")
        lo = lower.get(i, "")
        ti = title.get(i, "")

        if any([up, lo, ti]):
            print("\t%s\t%s\t%s\t%s\t# %4X" % (chr(i), lo, ti, up, i))


if __name__ == "__main__":
    sys.exit(main(sys.argv))
