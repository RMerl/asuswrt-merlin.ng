# Copyright (C) 2012 Jelmer Vernooij <jelmer@samba.org>

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 2.1 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

from wafsamba.tests import TestCase

from wafsamba.samba_abi import (
    abi_write_vscript,
    normalise_signature,
    )

from cStringIO import StringIO


class NormaliseSignatureTests(TestCase):

    def test_function_simple(self):
        self.assertEquals("int (const struct GUID *, const struct GUID *)",
            normalise_signature("$2 = {int (const struct GUID *, const struct GUID *)} 0xe871 <GUID_compare>"))

    def test_maps_Bool(self):
        # Some types have different internal names
        self.assertEquals("bool (const struct GUID *)",
            normalise_signature("$1 = {_Bool (const struct GUID *)} 0xe75b <GUID_all_zero>"))

    def test_function_keep(self):
        self.assertEquals(
            "enum ndr_err_code (struct ndr_push *, int, const union winreg_Data *)",
            normalise_signature("enum ndr_err_code (struct ndr_push *, int, const union winreg_Data *)"))

    def test_struct_constant(self):
        self.assertEquals(
            'uuid = {time_low = 0, time_mid = 0, time_hi_and_version = 0, clock_seq = "\\000", node = "\\000\\000\\000\\000\\000"}, if_version = 0',
            normalise_signature('$239 = {uuid = {time_low = 0, time_mid = 0, time_hi_and_version = 0, clock_seq = "\\000", node = "\\000\\000\\000\\000\\000"}, if_version = 0}'))

    def test_incomplete_sequence(self):
        # Newer versions of gdb insert these incomplete sequence elements
        self.assertEquals(
            'uuid = {time_low = 2324192516, time_mid = 7403, time_hi_and_version = 4553, clock_seq = "\\237\\350", node = "\\b\\000+\\020H`"}, if_version = 2',
            normalise_signature('$244 = {uuid = {time_low = 2324192516, time_mid = 7403, time_hi_and_version = 4553, clock_seq = "\\237", <incomplete sequence \\350>, node = "\\b\\000+\\020H`"}, if_version = 2}'))
        self.assertEquals(
            'uuid = {time_low = 2324192516, time_mid = 7403, time_hi_and_version = 4553, clock_seq = "\\237\\350", node = "\\b\\000+\\020H`"}, if_version = 2',
            normalise_signature('$244 = {uuid = {time_low = 2324192516, time_mid = 7403, time_hi_and_version = 4553, clock_seq = "\\237\\350", node = "\\b\\000+\\020H`"}, if_version = 2}'))


class WriteVscriptTests(TestCase):

    def test_one(self):
        f = StringIO()
        abi_write_vscript(f, "MYLIB", "1.0", [], {
            "old": "1.0",
            "new": "1.0"}, ["*"])
        self.assertEquals(f.getvalue(), """\
1.0 {
\tglobal:
\t\t*;
};
""")

    def test_simple(self):
        # No restrictions.
        f = StringIO()
        abi_write_vscript(f, "MYLIB", "1.0", ["0.1"], {
            "old": "0.1",
            "new": "1.0"}, ["*"])
        self.assertEquals(f.getvalue(), """\
MYLIB_0.1 {
\tglobal:
\t\told;
};

1.0 {
\tglobal:
\t\t*;
};
""")

    def test_exclude(self):
        f = StringIO()
        abi_write_vscript(f, "MYLIB", "1.0", [], {
            "exc_old": "0.1",
            "old": "0.1",
            "new": "1.0"}, ["!exc_*"])
        self.assertEquals(f.getvalue(), """\
1.0 {
\tglobal:
\t\t*;
\tlocal:
\t\texc_*;
};
""")

    def test_excludes_and_includes(self):
        f = StringIO()
        abi_write_vscript(f, "MYLIB", "1.0", [], {
            "pub_foo": "1.0",
            "exc_bar": "1.0",
            "other": "1.0"
            }, ["pub_*", "!exc_*"])
        self.assertEquals(f.getvalue(), """\
1.0 {
\tglobal:
\t\tpub_*;
\tlocal:
\t\texc_*;
\t\t*;
};
""")
