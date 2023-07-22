#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (C) 2018 Collabora Inc.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General
# Public License along with this library; if not, see <http://www.gnu.org/licenses/>.
#
# Author: Xavier Claessens <xavier.claessens@collabora.com>

import os
import sys
import tempfile
import subprocess

if not 'GLIB_TEST_COMPILATION' in os.environ:
  print('''Test disabled because GLIB_TEST_COMPILATION is not set in the env.
If you wish to run this test, set GLIB_TEST_COMPILATION=1 in the env,
and make sure you have glib build dependencies installed, including
meson.''')
  sys.exit(0)

if len(sys.argv) != 2:
  print('Usage: {} <gio-2.0.pc dir>'.format(os.path.basename(sys.argv[0])))
  sys.exit(1)

test_dir = os.path.dirname(sys.argv[0])

with tempfile.TemporaryDirectory() as builddir:
  env = os.environ.copy()
  env['PKG_CONFIG_PATH'] = sys.argv[1]
  sourcedir = os.path.join(test_dir, 'static-link')

  # Ensure we can static link and run a test app
  subprocess.check_call(['meson', sourcedir, builddir], env=env)
  subprocess.check_call(['ninja', '-C', builddir, 'test'], env=env)
  # FIXME: This probably only works on Linux
  out = subprocess.check_output(['ldd', os.path.join(builddir, 'test-static-link')], env=env).decode()
  if 'libgio' in out:
    print('test-static-link is dynamically linked on libgio')
    exit(1)
