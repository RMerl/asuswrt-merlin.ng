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

"""Tests for wafsamba."""

from unittest import (
    TestCase,
    TestLoader,
    )

def test_suite():
    names = [
        'abi',
        'bundled',
        'utils',
        ]
    module_names = ['wafsamba.tests.test_' + name for name in names]
    loader = TestLoader()
    result = loader.suiteClass()
    suite = loader.loadTestsFromNames(module_names)
    result.addTests(suite)
    return result
