# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2017 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Test for the image module

import unittest

from binman.image import Image
from u_boot_pylib.test_util import capture_sys_output

class TestImage(unittest.TestCase):
    def testInvalidFormat(self):
        image = Image('name', 'node', test=True)
        with self.assertRaises(ValueError) as e:
            image.LookupSymbol('_binman_something_prop_', False, 'msg', 0)
        self.assertIn(
            "msg: Symbol '_binman_something_prop_' has invalid format",
            str(e.exception))

    def testMissingSymbol(self):
        image = Image('name', 'node', test=True)
        image._entries = {}
        with self.assertRaises(ValueError) as e:
            image.LookupSymbol('_binman_type_prop_pname', False, 'msg', 0)
        self.assertIn("msg: Entry 'type' not found in list ()",
                      str(e.exception))

    def testMissingSymbolOptional(self):
        image = Image('name', 'node', test=True)
        image._entries = {}
        with capture_sys_output() as (stdout, stderr):
            val = image.LookupSymbol('_binman_type_prop_pname', True, 'msg', 0)
        self.assertEqual(val, None)
        self.assertEqual("Warning: msg: Entry 'type' not found in list ()\n",
                         stderr.getvalue())
        self.assertEqual('', stdout.getvalue())

    def testBadProperty(self):
        image = Image('name', 'node', test=True)
        image._entries = {'u-boot': 1}
        with self.assertRaises(ValueError) as e:
            image.LookupSymbol('_binman_u_boot_prop_bad', False, 'msg', 0)
        self.assertIn("msg: No such property 'bad", str(e.exception))
