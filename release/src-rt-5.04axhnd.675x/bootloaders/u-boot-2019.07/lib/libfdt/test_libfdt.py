#!/usr/bin/python

import os
import sys

our_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(our_path, '../../b/sandbox_spl/tools'))

import libfdt

with open('b/sandbox_spl/u-boot.dtb') as fd:
    fdt = fd.read()

print libfdt.fdt_path_offset(fdt, "/aliases")
