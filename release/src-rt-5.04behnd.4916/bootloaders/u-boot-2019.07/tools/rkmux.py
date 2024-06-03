#!/usr/bin/env python2

# Script to create enums from datasheet register tables
#
# Usage:
#
# First, create a text file from the datasheet:
#    pdftotext -layout /path/to/rockchip-3288-trm.pdf /tmp/asc
#
# Then use this script to output the #defines for a particular register:
#    ./tools/rkmux.py GRF_GPIO4C_IOMUX
#
# It will create output suitable for putting in a header file, with SHIFT and
# MASK values for each bitfield in the register.
#
# Note: this tool is not perfect and you may need to edit the resulting code.
# But it should speed up the process.

import csv
import re
import sys

tab_to_col = 3

class RegField:
    def __init__(self, cols=None):
        if cols:
            self.bits, self.attr, self.reset_val, self.desc = (
                [x.strip() for x in cols])
            self.desc = [self.desc]
        else:
            self.bits = ''
            self.attr = ''
            self.reset_val = ''
            self.desc = []

    def Setup(self, cols):
        self.bits, self.attr, self.reset_val = cols[0:3]
        if len(cols) > 3:
            self.desc.append(cols[3])

    def AddDesc(self, desc):
        self.desc.append(desc)

    def Show(self):
        print self
        print
        self.__init__()

    def __str__(self):
        return '%s,%s,%s,%s' % (self.bits, self.attr, self.reset_val,
                                '\n'.join(self.desc))

class Printer:
    def __init__(self, name):
        self.first = True
        self.name = name
        self.re_sel = re.compile("[1-9]'b([01]+): (.*)")

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        if not self.first:
            self.output_footer()

    def output_header(self):
        print '/* %s */' % self.name
        print 'enum {'

    def output_footer(self):
        print '};';

    def output_regfield(self, regfield):
        lines = regfield.desc
        field = lines[0]
        #print 'field:', field
        if field in ['reserved', 'reserve', 'write_enable', 'write_mask']:
            return
        if field.endswith('_sel') or field.endswith('_con'):
            field = field[:-4]
        elif field.endswith(' iomux'):
            field = field[:-6]
        elif field.endswith('_mode') or field.endswith('_mask'):
            field = field[:-5]
        #else:
            #print 'bad field %s' % field
            #return
        field = field.upper()
        if ':' in regfield.bits:
            bit_high, bit_low = [int(x) for x in regfield.bits.split(':')]
        else:
            bit_high = bit_low = int(regfield.bits)
        bit_width = bit_high - bit_low + 1
        mask = (1 << bit_width) - 1
        if self.first:
            self.first = False
            self.output_header()
        else:
            print
        out_enum(field, 'shift', bit_low)
        out_enum(field, 'mask', mask)
        next_val = -1
        #print 'lines: %s', lines
        for line in lines:
            m = self.re_sel.match(line)
            if m:
                val, enum = int(m.group(1), 2), m.group(2)
                if enum not in ['reserved', 'reserve']:
                    out_enum(field, enum, val, val == next_val)
                    next_val = val + 1


def process_file(name, fd):
    field = RegField()
    reg = ''

    fields = []

    def add_it(field):
        if field.bits:
            if reg == name:
                fields.append(field)
            field = RegField()
        return field

    def is_field_start(line):
       if '=' in line or '+' in line:
           return False
       if (line.startswith('gpio') or line.startswith('peri_') or
                line.endswith('_sel') or line.endswith('_con')):
           return True
       if not ' ' in line: # and '_' in line:
           return True
       return False

    for line in fd:
        line = line.rstrip()
        if line[:4] in ['GRF_', 'PMU_', 'CRU_']:
            field = add_it(field)
            reg = line
            do_this = name == reg
        elif not line or not line.startswith(' '):
            continue
        line = line.replace('\xe2\x80\x99', "'")
        leading = len(line) - len(line.lstrip())
        line = line.lstrip()
        cols = re.split(' *', line, 3)
        if leading > 15 or (len(cols) > 3 and is_field_start(cols[3])):
            if is_field_start(line):
                field = add_it(field)
            field.AddDesc(line)
        else:
            if cols[0] == 'Bit' or len(cols) < 3:
                continue
            #print
            #print field
            field = add_it(field)
            field.Setup(cols)
    field = add_it(field)

    with Printer(name) as printer:
        for field in fields:
            #print field
            printer.output_regfield(field)
            #print

def out_enum(field, suffix, value, skip_val=False):
    str = '%s_%s' % (field.upper(), suffix.upper())
    if not skip_val:
        tabs = tab_to_col - len(str) / 8
        if value > 9:
            val_str = '%#x' % value
        else:
            val_str = '%d' % value

        str += '%s= %s' % ('\t' * tabs, val_str)
    print '\t%s,' % str

# Process a CSV file, e.g. from tabula
def process_csv(name, fd):
    reader = csv.reader(fd)

    rows = []

    field = RegField()
    for row in reader:
        #print field.desc
        if not row[0]:
            field.desc.append(row[3])
            continue
        if field.bits:
            if field.bits != 'Bit':
                rows.append(field)
        #print row
        field = RegField(row)

    with Printer(name) as printer:
        for row in rows:
            #print field
            printer.output_regfield(row)
            #print

fname = sys.argv[1]
name = sys.argv[2]

# Read output from pdftotext -layout
if 1:
    with open(fname, 'r') as fd:
        process_file(name, fd)

# Use tabula
# It seems to be better at outputting text for an entire cell in one cell.
# But it does not always work. E.g. GRF_GPIO7CH_IOMUX.
# So there is no point in using it.
if 0:
    with open(fname, 'r') as fd:
        process_csv(name, fd)
