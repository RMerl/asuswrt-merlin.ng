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

from wafsamba.samba_utils import (
    TO_LIST,
    dict_concat,
    subst_vars_error,
    unique_list,
    )

class ToListTests(TestCase):

    def test_none(self):
        self.assertEquals([], TO_LIST(None))

    def test_already_list(self):
        self.assertEquals(["foo", "bar", 1], TO_LIST(["foo", "bar", 1]))

    def test_default_delimiter(self):
        self.assertEquals(["foo", "bar"], TO_LIST("foo bar"))
        self.assertEquals(["foo", "bar"], TO_LIST("  foo bar  "))
        self.assertEquals(["foo ", "bar"], TO_LIST("  \"foo \" bar  "))

    def test_delimiter(self):
        self.assertEquals(["foo", "bar"], TO_LIST("foo,bar", ","))
        self.assertEquals(["  foo", "bar  "], TO_LIST("  foo,bar  ", ","))
        self.assertEquals(["  \" foo\"", " bar  "], TO_LIST("  \" foo\", bar  ", ","))


class UniqueListTests(TestCase):

    def test_unique_list(self):
        self.assertEquals(["foo", "bar"], unique_list(["foo", "bar", "foo"]))


class SubstVarsErrorTests(TestCase):

    def test_valid(self):
        self.assertEquals("", subst_vars_error("", {}))
        self.assertEquals("FOO bar", subst_vars_error("${F} bar", {"F": "FOO"}))

    def test_invalid(self):
        self.assertRaises(KeyError, subst_vars_error, "${F}", {})


class DictConcatTests(TestCase):

    def test_empty(self):
        ret = {}
        dict_concat(ret, {})
        self.assertEquals({}, ret)

    def test_same(self):
        ret = {"foo": "bar"}
        dict_concat(ret, {"foo": "bla"})
        self.assertEquals({"foo": "bar"}, ret)

    def test_simple(self):
        ret = {"foo": "bar"}
        dict_concat(ret, {"blie": "bla"})
        self.assertEquals({"foo": "bar", "blie": "bla"}, ret)
