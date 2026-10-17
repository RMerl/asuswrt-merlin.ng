# SPDX-License-Identifier: GPL-2.0+
# Copyright 2022 Google LLC
# Written by Simon Glass <sjg@chromium.org>
#

"""Tests for the Bintool class"""

import collections
import os
import shutil
import tempfile
import unittest
import unittest.mock
import urllib.error

from binman import bintool
from binman.bintool import Bintool

from u_boot_pylib import command
from u_boot_pylib import terminal
from u_boot_pylib import test_util
from u_boot_pylib import tools

# pylint: disable=R0904
class TestBintool(unittest.TestCase):
    """Tests for the Bintool class"""
    def setUp(self):
        # Create a temporary directory for test files
        self._indir = tempfile.mkdtemp(prefix='bintool.')
        self.seq = None
        self.count = None
        self.fname = None
        self.btools = None

    def tearDown(self):
        """Remove the temporary input directory and its contents"""
        if self._indir:
            shutil.rmtree(self._indir)
        self._indir = None

    def test_missing_btype(self):
        """Test that unknown bintool types are detected"""
        with self.assertRaises(ValueError) as exc:
            Bintool.create('missing')
        self.assertIn("No module named 'binman.btool.missing'",
                      str(exc.exception))

    def test_fresh_bintool(self):
        """Check that the _testing bintool is not cached"""
        btest = Bintool.create('_testing')
        btest.present = True
        btest2 = Bintool.create('_testing')
        self.assertFalse(btest2.present)

    def test_version(self):
        """Check handling of a tool being present or absent"""
        btest = Bintool.create('_testing')
        with test_util.capture_sys_output() as (stdout, _):
            btest.show()
        self.assertFalse(btest.is_present())
        self.assertIn('-', stdout.getvalue())
        btest.present = True
        self.assertTrue(btest.is_present())
        self.assertEqual('123', btest.version())
        with test_util.capture_sys_output() as (stdout, _):
            btest.show()
        self.assertIn('123', stdout.getvalue())

    def test_fetch_present(self):
        """Test fetching of a tool"""
        btest = Bintool.create('_testing')
        btest.present = True
        col = terminal.Color()
        self.assertEqual(bintool.PRESENT,
                         btest.fetch_tool(bintool.FETCH_ANY, col, True))

    @classmethod
    def check_fetch_url(cls, fake_download, method):
        """Check the output from fetching a tool

        Args:
            fake_download (function): Function to call instead of
                tools.download()
            method (bintool.FETCH_...: Fetch method to use

        Returns:
            str: Contents of stdout
        """
        btest = Bintool.create('_testing')
        col = terminal.Color()
        with unittest.mock.patch.object(tools, 'download',
                                        side_effect=fake_download):
            with test_util.capture_sys_output() as (stdout, _):
                btest.fetch_tool(method, col, False)
        return stdout.getvalue()

    def test_fetch_url_err(self):
        """Test an error while fetching a tool from a URL"""
        def fail_download(url):
            """Take the tools.download() function by raising an exception"""
            raise urllib.error.URLError('my error')

        stdout = self.check_fetch_url(fail_download, bintool.FETCH_ANY)
        self.assertIn('my error', stdout)

    def test_fetch_url_exception(self):
        """Test an exception while fetching a tool from a URL"""
        def cause_exc(url):
            raise ValueError('exc error')

        stdout = self.check_fetch_url(cause_exc, bintool.FETCH_ANY)
        self.assertIn('exc error', stdout)

    def test_fetch_method(self):
        """Test fetching using a particular method"""
        def fail_download(url):
            """Take the tools.download() function by raising an exception"""
            raise urllib.error.URLError('my error')

        stdout = self.check_fetch_url(fail_download, bintool.FETCH_BIN)
        self.assertIn('my error', stdout)

    def test_fetch_pass_fail(self):
        """Test fetching multiple tools with some passing and some failing"""
        def handle_download(_):
            """Take the tools.download() function by writing a file"""
            if self.seq:
                raise urllib.error.URLError('not found')
            self.seq += 1
            tools.write_file(fname, expected)
            return fname, dirname

        expected = b'this is a test'
        dirname = os.path.join(self._indir, 'download_dir')
        os.mkdir(dirname)
        fname = os.path.join(dirname, 'downloaded')

        # Rely on bintool to create this directory
        destdir = os.path.join(self._indir, 'dest_dir')

        dest_fname = os.path.join(destdir, '_testing')
        self.seq = 0

        with unittest.mock.patch.object(bintool.Bintool, 'tooldir', destdir):
            with unittest.mock.patch.object(tools, 'download',
                                            side_effect=handle_download):
                with test_util.capture_sys_output() as (stdout, _):
                    Bintool.fetch_tools(bintool.FETCH_ANY, ['_testing'] * 2)
        self.assertTrue(os.path.exists(dest_fname))
        data = tools.read_file(dest_fname)
        self.assertEqual(expected, data)

        lines = stdout.getvalue().splitlines()
        self.assertTrue(len(lines) > 2)
        self.assertEqual('Tools fetched:    1: _testing', lines[-2])
        self.assertEqual('Failures:         1: _testing', lines[-1])

    def test_tool_list(self):
        """Test listing available tools"""
        self.assertGreater(len(Bintool.get_tool_list()), 3)

    def check_fetch_all(self, method):
        """Helper to check the operation of fetching all tools"""

        # pylint: disable=W0613
        def fake_fetch(method, col, skip_present):
            """Fakes the Binutils.fetch() function

            Returns FETCHED and FAIL on alternate calls
            """
            self.seq += 1
            result = bintool.FETCHED if self.seq & 1 else bintool.FAIL
            self.count[result] += 1
            return result

        self.seq = 0
        self.count = collections.defaultdict(int)
        with unittest.mock.patch.object(bintool.Bintool, 'fetch_tool',
                                        side_effect=fake_fetch):
            with test_util.capture_sys_output() as (stdout, _):
                Bintool.fetch_tools(method, ['all'])
        lines = stdout.getvalue().splitlines()
        self.assertIn(f'{self.count[bintool.FETCHED]}: ', lines[-2])
        self.assertIn(f'{self.count[bintool.FAIL]}: ', lines[-1])

    def test_fetch_all(self):
        """Test fetching all tools"""
        self.check_fetch_all(bintool.FETCH_ANY)

    def test_fetch_all_specific(self):
        """Test fetching all tools with a specific method"""
        self.check_fetch_all(bintool.FETCH_BIN)

    def test_fetch_missing(self):
        """Test fetching missing tools"""
        # pylint: disable=W0613
        def fake_fetch2(method, col, skip_present):
            """Fakes the Binutils.fetch() function

            Returns PRESENT only for the '_testing' bintool
            """
            btool = list(self.btools.values())[self.seq]
            self.seq += 1
            print('fetch', btool.name)
            if btool.name == '_testing':
                return bintool.PRESENT
            return bintool.FETCHED

        # Preload a list of tools to return when get_tool_list() and create()
        # are called
        all_tools = Bintool.get_tool_list(True)
        self.btools = collections.OrderedDict()
        for name in all_tools:
            self.btools[name] = Bintool.create(name)
        self.seq = 0
        with unittest.mock.patch.object(bintool.Bintool, 'fetch_tool',
                                        side_effect=fake_fetch2):
            with unittest.mock.patch.object(bintool.Bintool,
                                            'get_tool_list',
                                            side_effect=[all_tools]):
                with unittest.mock.patch.object(bintool.Bintool, 'create',
                                                side_effect=self.btools.values()):
                    with test_util.capture_sys_output() as (stdout, _):
                        Bintool.fetch_tools(bintool.FETCH_ANY, ['missing'])
        lines = stdout.getvalue().splitlines()
        num_tools = len(self.btools)
        fetched = [line for line in lines if 'Tools fetched:' in line].pop()
        present = [line for line in lines if 'Already present:' in line].pop()
        self.assertIn(f'{num_tools - 1}: ', fetched)
        self.assertIn('1: ', present)

    def check_build_method(self, write_file):
        """Check the output from fetching using the BUILD method

        Args:
            write_file (bool): True to write the output file when 'make' is
                called

        Returns:
            tuple:
                str: Filename of written file (or missing 'make' output)
                str: Contents of stdout
        """
        def fake_run(*cmd):
            if cmd[0] == 'make':
                # See Bintool.build_from_git()
                tmpdir = cmd[2]
                self.fname = os.path.join(tmpdir, 'pathname')
                if write_file:
                    tools.write_file(self.fname, b'hello')

        btest = Bintool.create('_testing')
        col = terminal.Color()
        self.fname = None
        with unittest.mock.patch.object(bintool.Bintool, 'tooldir',
                                        self._indir):
            with unittest.mock.patch.object(tools, 'run', side_effect=fake_run):
                with test_util.capture_sys_output() as (stdout, _):
                    btest.fetch_tool(bintool.FETCH_BUILD, col, False)
        fname = os.path.join(self._indir, '_testing')
        return fname if write_file else self.fname, stdout.getvalue()

    def test_build_method(self):
        """Test fetching using the build method"""
        fname, stdout = self.check_build_method(write_file=True)
        self.assertTrue(os.path.exists(fname))
        self.assertIn(f"writing to '{fname}", stdout)

    def test_build_method_fail(self):
        """Test fetching using the build method when no file is produced"""
        fname, stdout = self.check_build_method(write_file=False)
        self.assertFalse(os.path.exists(fname))
        self.assertIn(f"File '{fname}' was not produced", stdout)

    def test_install(self):
        """Test fetching using the install method"""
        btest = Bintool.create('_testing')
        btest.install = True
        col = terminal.Color()
        with unittest.mock.patch.object(tools, 'run', return_value=None):
            with test_util.capture_sys_output() as _:
                result = btest.fetch_tool(bintool.FETCH_BIN, col, False)
        self.assertEqual(bintool.FETCHED, result)

    def test_no_fetch(self):
        """Test fetching when there is no method"""
        btest = Bintool.create('_testing')
        btest.disable = True
        col = terminal.Color()
        with test_util.capture_sys_output() as _:
            result = btest.fetch_tool(bintool.FETCH_BIN, col, False)
        self.assertEqual(bintool.FAIL, result)

    def test_all_bintools(self):
        """Test that all bintools can handle all available fetch types"""
        def handle_download(_):
            """Take the tools.download() function by writing a file"""
            tools.write_file(fname, expected)
            return fname, dirname

        def fake_run(*cmd):
            if cmd[0] == 'make':
                # See Bintool.build_from_git()
                tmpdir = cmd[2]
                self.fname = os.path.join(tmpdir, 'pathname')
                tools.write_file(self.fname, b'hello')

        expected = b'this is a test'
        dirname = os.path.join(self._indir, 'download_dir')
        os.mkdir(dirname)
        fname = os.path.join(dirname, 'downloaded')

        with unittest.mock.patch.object(tools, 'run', side_effect=fake_run):
            with unittest.mock.patch.object(tools, 'download',
                                            side_effect=handle_download):
                with test_util.capture_sys_output() as _:
                    for name in Bintool.get_tool_list():
                        btool = Bintool.create(name)
                        for method in range(bintool.FETCH_COUNT):
                            result = btool.fetch(method)
                            self.assertTrue(result is not False)
                            if result is not True and result is not None:
                                result_fname, _ = result
                                self.assertTrue(os.path.exists(result_fname))
                                data = tools.read_file(result_fname)
                                self.assertEqual(expected, data)
                                os.remove(result_fname)

    def test_all_bintool_versions(self):
        """Test handling of bintool version when it cannot be run"""
        all_tools = Bintool.get_tool_list()
        for name in all_tools:
            btool = Bintool.create(name)
            with unittest.mock.patch.object(
                btool, 'run_cmd_result', return_value=command.CommandResult()):
                self.assertEqual('unknown', btool.version())

    def test_force_missing(self):
        btool = Bintool.create('_testing')
        btool.present = True
        self.assertTrue(btool.is_present())

        btool.present = None
        Bintool.set_missing_list(['_testing'])
        self.assertFalse(btool.is_present())

    def test_failed_command(self):
        """Check that running a command that does not exist returns None"""
        destdir = os.path.join(self._indir, 'dest_dir')
        os.mkdir(destdir)
        with unittest.mock.patch.object(bintool.Bintool, 'tooldir', destdir):
            btool = Bintool.create('_testing')
            result = btool.run_cmd_result('fred')
        self.assertIsNone(result)


if __name__ == "__main__":
    unittest.main()
