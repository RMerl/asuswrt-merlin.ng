# -*- coding: utf-8 -*-
# SPDX-License-Identifier:	GPL-2.0+
#
# Copyright 2017 Google, Inc
#

"""Functional tests for checking that patman behaves correctly"""

import contextlib
import os
import pathlib
import re
import shutil
import sys
import tempfile
import unittest


from patman.commit import Commit
from patman import control
from patman import gitutil
from patman import patchstream
from patman.patchstream import PatchStream
from patman.series import Series
from patman import settings
from u_boot_pylib import terminal
from u_boot_pylib import tools
from u_boot_pylib.test_util import capture_sys_output

import pygit2
from patman import status

PATMAN_DIR = pathlib.Path(__file__).parent
TEST_DATA_DIR = PATMAN_DIR / 'test/'


@contextlib.contextmanager
def directory_excursion(directory):
    """Change directory to `directory` for a limited to the context block."""
    current = os.getcwd()
    try:
        os.chdir(directory)
        yield
    finally:
        os.chdir(current)


class TestFunctional(unittest.TestCase):
    """Functional tests for checking that patman behaves correctly"""
    leb = (b'Lord Edmund Blackadd\xc3\xabr <weasel@blackadder.org>'.
           decode('utf-8'))
    fred = 'Fred Bloggs <f.bloggs@napier.net>'
    joe = 'Joe Bloggs <joe@napierwallies.co.nz>'
    mary = 'Mary Bloggs <mary@napierwallies.co.nz>'
    commits = None
    patches = None

    def setUp(self):
        self.tmpdir = tempfile.mkdtemp(prefix='patman.')
        self.gitdir = os.path.join(self.tmpdir, 'git')
        self.repo = None

    def tearDown(self):
        shutil.rmtree(self.tmpdir)
        terminal.set_print_test_mode(False)

    @staticmethod
    def _get_path(fname):
        """Get the path to a test file

        Args:
            fname (str): Filename to obtain

        Returns:
            str: Full path to file in the test directory
        """
        return TEST_DATA_DIR / fname

    @classmethod
    def _get_text(cls, fname):
        """Read a file as text

        Args:
            fname (str): Filename to read

        Returns:
            str: Contents of file
        """
        return open(cls._get_path(fname), encoding='utf-8').read()

    @classmethod
    def _get_patch_name(cls, subject):
        """Get the filename of a patch given its subject

        Args:
            subject (str): Patch subject

        Returns:
            str: Filename for that patch
        """
        fname = re.sub('[ :]', '-', subject)
        return fname.replace('--', '-')

    def _create_patches_for_test(self, series):
        """Create patch files for use by tests

        This copies patch files from the test directory as needed by the series

        Args:
            series (Series): Series containing commits to convert

        Returns:
            tuple:
                str: Cover-letter filename, or None if none
                fname_list: list of str, each a patch filename
        """
        cover_fname = None
        fname_list = []
        for i, commit in enumerate(series.commits):
            clean_subject = self._get_patch_name(commit.subject)
            src_fname = '%04d-%s.patch' % (i + 1, clean_subject[:52])
            fname = os.path.join(self.tmpdir, src_fname)
            shutil.copy(self._get_path(src_fname), fname)
            fname_list.append(fname)
        if series.get('cover'):
            src_fname = '0000-cover-letter.patch'
            cover_fname = os.path.join(self.tmpdir, src_fname)
            fname = os.path.join(self.tmpdir, src_fname)
            shutil.copy(self._get_path(src_fname), fname)

        return cover_fname, fname_list

    def test_basic(self):
        """Tests the basic flow of patman

        This creates a series from some hard-coded patches build from a simple
        tree with the following metadata in the top commit:

            Series-to: u-boot
            Series-prefix: RFC
            Series-postfix: some-branch
            Series-cc: Stefan Brüns <stefan.bruens@rwth-aachen.de>
            Cover-letter-cc: Lord Mëlchett <clergy@palace.gov>
            Series-version: 3
            Patch-cc: fred
            Series-process-log: sort, uniq
            Series-changes: 4
            - Some changes
            - Multi
              line
              change

            Commit-changes: 2
            - Changes only for this commit

'            Cover-changes: 4
            - Some notes for the cover letter

            Cover-letter:
            test: A test patch series
            This is a test of how the cover
            letter
            works
            END

        and this in the first commit:

            Commit-changes: 2
            - second revision change

            Series-notes:
            some notes
            about some things
            from the first commit
            END

            Commit-notes:
            Some notes about
            the first commit
            END

        with the following commands:

           git log -n2 --reverse >/path/to/tools/patman/test/test01.txt
           git format-patch --subject-prefix RFC --cover-letter HEAD~2
           mv 00* /path/to/tools/patman/test

        It checks these aspects:
            - git log can be processed by patchstream
            - emailing patches uses the correct command
            - CC file has information on each commit
            - cover letter has the expected text and subject
            - each patch has the correct subject
            - dry-run information prints out correctly
            - unicode is handled correctly
            - Series-to, Series-cc, Series-prefix, Series-postfix, Cover-letter
            - Cover-letter-cc, Series-version, Series-changes, Series-notes
            - Commit-notes
        """
        process_tags = True
        ignore_bad_tags = False
        stefan = b'Stefan Br\xc3\xbcns <stefan.bruens@rwth-aachen.de>'.decode('utf-8')
        rick = 'Richard III <richard@palace.gov>'
        mel = b'Lord M\xc3\xablchett <clergy@palace.gov>'.decode('utf-8')
        add_maintainers = [stefan, rick]
        dry_run = True
        in_reply_to = mel
        count = 2
        settings.alias = {
            'fdt': ['simon'],
            'u-boot': ['u-boot@lists.denx.de'],
            'simon': [self.leb],
            'fred': [self.fred],
        }

        text = self._get_text('test01.txt')
        series = patchstream.get_metadata_for_test(text)
        cover_fname, args = self._create_patches_for_test(series)
        get_maintainer_script = str(pathlib.Path(__file__).parent.parent.parent
                                    / 'get_maintainer.pl') + ' --norolestats'
        with capture_sys_output() as out:
            patchstream.fix_patches(series, args)
            if cover_fname and series.get('cover'):
                patchstream.insert_cover_letter(cover_fname, series, count)
            series.DoChecks()
            cc_file = series.MakeCcFile(process_tags, cover_fname,
                                        not ignore_bad_tags, add_maintainers,
                                        None, get_maintainer_script)
            cmd = gitutil.email_patches(
                series, cover_fname, args, dry_run, not ignore_bad_tags,
                cc_file, in_reply_to=in_reply_to, thread=None)
            series.ShowActions(args, cmd, process_tags)
        cc_lines = open(cc_file, encoding='utf-8').read().splitlines()
        os.remove(cc_file)

        lines = iter(out[0].getvalue().splitlines())
        self.assertEqual('Cleaned %s patches' % len(series.commits),
                         next(lines))
        self.assertEqual('Change log missing for v2', next(lines))
        self.assertEqual('Change log missing for v3', next(lines))
        self.assertEqual('Change log for unknown version v4', next(lines))
        self.assertEqual("Alias 'pci' not found", next(lines))
        while next(lines) != 'Cc processing complete':
            pass
        self.assertIn('Dry run', next(lines))
        self.assertEqual('', next(lines))
        self.assertIn('Send a total of %d patches' % count, next(lines))
        prev = next(lines)
        for i, commit in enumerate(series.commits):
            self.assertEqual('   %s' % args[i], prev)
            while True:
                prev = next(lines)
                if 'Cc:' not in prev:
                    break
        self.assertEqual('To:	  u-boot@lists.denx.de', prev)
        self.assertEqual('Cc:	  %s' % stefan, next(lines))
        self.assertEqual('Version:  3', next(lines))
        self.assertEqual('Prefix:\t  RFC', next(lines))
        self.assertEqual('Postfix:\t  some-branch', next(lines))
        self.assertEqual('Cover: 4 lines', next(lines))
        self.assertEqual('      Cc:  %s' % self.fred, next(lines))
        self.assertEqual('      Cc:  %s' % self.leb,
                         next(lines))
        self.assertEqual('      Cc:  %s' % mel, next(lines))
        self.assertEqual('      Cc:  %s' % rick, next(lines))
        expected = ('Git command: git send-email --annotate '
                    '--in-reply-to="%s" --to "u-boot@lists.denx.de" '
                    '--cc "%s" --cc-cmd "%s send --cc-cmd %s" %s %s'
                    % (in_reply_to, stefan, sys.argv[0], cc_file, cover_fname,
                       ' '.join(args)))
        self.assertEqual(expected, next(lines))

        self.assertEqual(('%s %s\0%s' % (args[0], rick, stefan)), cc_lines[0])
        self.assertEqual(
            '%s %s\0%s\0%s\0%s' % (args[1], self.fred, self.leb, rick, stefan),
            cc_lines[1])

        expected = '''
This is a test of how the cover
letter
works

some notes
about some things
from the first commit

Changes in v4:
- Multi
  line
  change
- Some changes
- Some notes for the cover letter

Simon Glass (2):
  pci: Correct cast for sandbox
  fdt: Correct cast for sandbox in fdtdec_setup_mem_size_base()

 cmd/pci.c                   | 3 ++-
 fs/fat/fat.c                | 1 +
 lib/efi_loader/efi_memory.c | 1 +
 lib/fdtdec.c                | 3 ++-
 4 files changed, 6 insertions(+), 2 deletions(-)

--\x20
2.7.4

'''
        lines = open(cover_fname, encoding='utf-8').read().splitlines()
        self.assertEqual(
            'Subject: [RFC PATCH some-branch v3 0/2] test: A test patch series',
            lines[3])
        self.assertEqual(expected.splitlines(), lines[7:])

        for i, fname in enumerate(args):
            lines = open(fname, encoding='utf-8').read().splitlines()
            subject = [line for line in lines if line.startswith('Subject')]
            self.assertEqual('Subject: [RFC %d/%d]' % (i + 1, count),
                             subject[0][:18])

            # Check that we got our commit notes
            start = 0
            expected = ''

            if i == 0:
                start = 17
                expected = '''---
Some notes about
the first commit

(no changes since v2)

Changes in v2:
- second revision change'''
            elif i == 1:
                start = 17
                expected = '''---

Changes in v4:
- Multi
  line
  change
- Some changes

Changes in v2:
- Changes only for this commit'''

            if expected:
                expected = expected.splitlines()
                self.assertEqual(expected, lines[start:(start+len(expected))])

    def make_commit_with_file(self, subject, body, fname, text):
        """Create a file and add it to the git repo with a new commit

        Args:
            subject (str): Subject for the commit
            body (str): Body text of the commit
            fname (str): Filename of file to create
            text (str): Text to put into the file
        """
        path = os.path.join(self.gitdir, fname)
        tools.write_file(path, text, binary=False)
        index = self.repo.index
        index.add(fname)
        # pylint doesn't seem to find this
        # pylint: disable=E1101
        author = pygit2.Signature('Test user', 'test@email.com')
        committer = author
        tree = index.write_tree()
        message = subject + '\n' + body
        self.repo.create_commit('HEAD', author, committer, message, tree,
                                [self.repo.head.target])

    def make_git_tree(self):
        """Make a simple git tree suitable for testing

        It has three branches:
            'base' has two commits: PCI, main
            'first' has base as upstream and two more commits: I2C, SPI
            'second' has base as upstream and three more: video, serial, bootm

        Returns:
            pygit2.Repository: repository
        """
        repo = pygit2.init_repository(self.gitdir)
        self.repo = repo
        new_tree = repo.TreeBuilder().write()

        # pylint doesn't seem to find this
        # pylint: disable=E1101
        author = pygit2.Signature('Test user', 'test@email.com')
        committer = author
        _ = repo.create_commit('HEAD', author, committer, 'Created master',
                               new_tree, [])

        self.make_commit_with_file('Initial commit', '''
Add a README

''', 'README', '''This is the README file
describing this project
in very little detail''')

        self.make_commit_with_file('pci: PCI implementation', '''
Here is a basic PCI implementation

''', 'pci.c', '''This is a file
it has some contents
and some more things''')
        self.make_commit_with_file('main: Main program', '''
Hello here is the second commit.
''', 'main.c', '''This is the main file
there is very little here
but we can always add more later
if we want to

Series-to: u-boot
Series-cc: Barry Crump <bcrump@whataroa.nz>
''')
        base_target = repo.revparse_single('HEAD')
        self.make_commit_with_file('i2c: I2C things', '''
This has some stuff to do with I2C
''', 'i2c.c', '''And this is the file contents
with some I2C-related things in it''')
        self.make_commit_with_file('spi: SPI fixes', '''
SPI needs some fixes
and here they are

Signed-off-by: %s

Series-to: u-boot
Commit-notes:
title of the series
This is the cover letter for the series
with various details
END
''' % self.leb, 'spi.c', '''Some fixes for SPI in this
file to make SPI work
better than before''')
        first_target = repo.revparse_single('HEAD')

        target = repo.revparse_single('HEAD~2')
        # pylint doesn't seem to find this
        # pylint: disable=E1101
        repo.reset(target.oid, pygit2.GIT_CHECKOUT_FORCE)
        self.make_commit_with_file('video: Some video improvements', '''
Fix up the video so that
it looks more purple. Purple is
a very nice colour.
''', 'video.c', '''More purple here
Purple and purple
Even more purple
Could not be any more purple''')
        self.make_commit_with_file('serial: Add a serial driver', '''
Here is the serial driver
for my chip.

Cover-letter:
Series for my board
This series implements support
for my glorious board.
END
Series-links: 183237
''', 'serial.c', '''The code for the
serial driver is here''')
        self.make_commit_with_file('bootm: Make it boot', '''
This makes my board boot
with a fix to the bootm
command
''', 'bootm.c', '''Fix up the bootm
command to make the code as
complicated as possible''')
        second_target = repo.revparse_single('HEAD')

        repo.branches.local.create('first', first_target)
        repo.config.set_multivar('branch.first.remote', '', '.')
        repo.config.set_multivar('branch.first.merge', '', 'refs/heads/base')

        repo.branches.local.create('second', second_target)
        repo.config.set_multivar('branch.second.remote', '', '.')
        repo.config.set_multivar('branch.second.merge', '', 'refs/heads/base')

        repo.branches.local.create('base', base_target)
        return repo

    def test_branch(self):
        """Test creating patches from a branch"""
        repo = self.make_git_tree()
        target = repo.lookup_reference('refs/heads/first')
        # pylint doesn't seem to find this
        # pylint: disable=E1101
        self.repo.checkout(target, strategy=pygit2.GIT_CHECKOUT_FORCE)
        control.setup()
        orig_dir = os.getcwd()
        try:
            os.chdir(self.gitdir)

            # Check that it can detect the current branch
            self.assertEqual(2, gitutil.count_commits_to_branch(None))
            col = terminal.Color()
            with capture_sys_output() as _:
                _, cover_fname, patch_files = control.prepare_patches(
                    col, branch=None, count=-1, start=0, end=0,
                    ignore_binary=False, signoff=True)
            self.assertIsNone(cover_fname)
            self.assertEqual(2, len(patch_files))

            # Check that it can detect a different branch
            self.assertEqual(3, gitutil.count_commits_to_branch('second'))
            with capture_sys_output() as _:
                _, cover_fname, patch_files = control.prepare_patches(
                    col, branch='second', count=-1, start=0, end=0,
                    ignore_binary=False, signoff=True)
            self.assertIsNotNone(cover_fname)
            self.assertEqual(3, len(patch_files))

            # Check that it can skip patches at the end
            with capture_sys_output() as _:
                _, cover_fname, patch_files = control.prepare_patches(
                    col, branch='second', count=-1, start=0, end=1,
                    ignore_binary=False, signoff=True)
            self.assertIsNotNone(cover_fname)
            self.assertEqual(2, len(patch_files))
        finally:
            os.chdir(orig_dir)

    def test_custom_get_maintainer_script(self):
        """Validate that a custom get_maintainer script gets used."""
        self.make_git_tree()
        with directory_excursion(self.gitdir):
            # Setup git.
            os.environ['GIT_CONFIG_GLOBAL'] = '/dev/null'
            os.environ['GIT_CONFIG_SYSTEM'] = '/dev/null'
            tools.run('git', 'config', 'user.name', 'Dummy')
            tools.run('git', 'config', 'user.email', 'dumdum@dummy.com')
            tools.run('git', 'branch', 'upstream')
            tools.run('git', 'branch', '--set-upstream-to=upstream')
            tools.run('git', 'add', '.')
            tools.run('git', 'commit', '-m', 'new commit')

            # Setup patman configuration.
            with open('.patman', 'w', buffering=1) as f:
                f.write('[settings]\n'
                        'get_maintainer_script: dummy-script.sh\n'
                        'check_patch: False\n')
            with open('dummy-script.sh', 'w', buffering=1) as f:
                f.write('#!/usr/bin/env python\n'
                        'print("hello@there.com")\n')
            os.chmod('dummy-script.sh', 0x555)

            # Finally, do the test
            with capture_sys_output():
                output = tools.run(PATMAN_DIR / 'patman', '--dry-run')
                # Assert the email address is part of the dry-run
                # output.
                self.assertIn('hello@there.com', output)

    def test_tags(self):
        """Test collection of tags in a patchstream"""
        text = '''This is a patch

Signed-off-by: Terminator
Reviewed-by: %s
Reviewed-by: %s
Tested-by: %s
''' % (self.joe, self.mary, self.leb)
        pstrm = PatchStream.process_text(text)
        self.assertEqual(pstrm.commit.rtags, {
            'Reviewed-by': {self.joe, self.mary},
            'Tested-by': {self.leb}})

    def test_invalid_tag(self):
        """Test invalid tag in a patchstream"""
        text = '''This is a patch

Serie-version: 2
'''
        with self.assertRaises(ValueError) as exc:
            pstrm = PatchStream.process_text(text)
        self.assertEqual("Line 3: Invalid tag = 'Serie-version: 2'",
                         str(exc.exception))

    def test_missing_end(self):
        """Test a missing END tag"""
        text = '''This is a patch

Cover-letter:
This is the title
missing END after this line
Signed-off-by: Fred
'''
        pstrm = PatchStream.process_text(text)
        self.assertEqual(["Missing 'END' in section 'cover'"],
                         pstrm.commit.warn)

    def test_missing_blank_line(self):
        """Test a missing blank line after a tag"""
        text = '''This is a patch

Series-changes: 2
- First line of changes
- Missing blank line after this line
Signed-off-by: Fred
'''
        pstrm = PatchStream.process_text(text)
        self.assertEqual(["Missing 'blank line' in section 'Series-changes'"],
                         pstrm.commit.warn)

    def test_invalid_commit_tag(self):
        """Test an invalid Commit-xxx tag"""
        text = '''This is a patch

Commit-fred: testing
'''
        pstrm = PatchStream.process_text(text)
        self.assertEqual(["Line 3: Ignoring Commit-fred"], pstrm.commit.warn)

    def test_self_test(self):
        """Test a tested by tag by this user"""
        test_line = 'Tested-by: %s@napier.com' % os.getenv('USER')
        text = '''This is a patch

%s
''' % test_line
        pstrm = PatchStream.process_text(text)
        self.assertEqual(["Ignoring '%s'" % test_line], pstrm.commit.warn)

    def test_space_before_tab(self):
        """Test a space before a tab"""
        text = '''This is a patch

+ \tSomething
'''
        pstrm = PatchStream.process_text(text)
        self.assertEqual(["Line 3/0 has space before tab"], pstrm.commit.warn)

    def test_lines_after_test(self):
        """Test detecting lines after TEST= line"""
        text = '''This is a patch

TEST=sometest
more lines
here
'''
        pstrm = PatchStream.process_text(text)
        self.assertEqual(["Found 2 lines after TEST="], pstrm.commit.warn)

    def test_blank_line_at_end(self):
        """Test detecting a blank line at the end of a file"""
        text = '''This is a patch

diff --git a/lib/fdtdec.c b/lib/fdtdec.c
index c072e54..942244f 100644
--- a/lib/fdtdec.c
+++ b/lib/fdtdec.c
@@ -1200,7 +1200,8 @@ int fdtdec_setup_mem_size_base(void)
 	}

 	gd->ram_size = (phys_size_t)(res.end - res.start + 1);
-	debug("%s: Initial DRAM size %llx\n", __func__, (u64)gd->ram_size);
+	debug("%s: Initial DRAM size %llx\n", __func__,
+	      (unsigned long long)gd->ram_size);
+
diff --git a/lib/efi_loader/efi_memory.c b/lib/efi_loader/efi_memory.c

--
2.7.4

 '''
        pstrm = PatchStream.process_text(text)
        self.assertEqual(
            ["Found possible blank line(s) at end of file 'lib/fdtdec.c'"],
            pstrm.commit.warn)

    def test_no_upstream(self):
        """Test CountCommitsToBranch when there is no upstream"""
        repo = self.make_git_tree()
        target = repo.lookup_reference('refs/heads/base')
        # pylint doesn't seem to find this
        # pylint: disable=E1101
        self.repo.checkout(target, strategy=pygit2.GIT_CHECKOUT_FORCE)

        # Check that it can detect the current branch
        orig_dir = os.getcwd()
        try:
            os.chdir(self.gitdir)
            with self.assertRaises(ValueError) as exc:
                gitutil.count_commits_to_branch(None)
            self.assertIn(
                "Failed to determine upstream: fatal: no upstream configured for branch 'base'",
                str(exc.exception))
        finally:
            os.chdir(orig_dir)

    @staticmethod
    def _fake_patchwork(url, subpath):
        """Fake Patchwork server for the function below

        This handles accessing a series, providing a list consisting of a
        single patch

        Args:
            url (str): URL of patchwork server
            subpath (str): URL subpath to use
        """
        re_series = re.match(r'series/(\d*)/$', subpath)
        if re_series:
            series_num = re_series.group(1)
            if series_num == '1234':
                return {'patches': [
                    {'id': '1', 'name': 'Some patch'}]}
        raise ValueError('Fake Patchwork does not understand: %s' % subpath)

    def test_status_mismatch(self):
        """Test Patchwork patches not matching the series"""
        series = Series()

        with capture_sys_output() as (_, err):
            status.collect_patches(series, 1234, None, self._fake_patchwork)
        self.assertIn('Warning: Patchwork reports 1 patches, series has 0',
                      err.getvalue())

    def test_status_read_patch(self):
        """Test handling a single patch in Patchwork"""
        series = Series()
        series.commits = [Commit('abcd')]

        patches = status.collect_patches(series, 1234, None,
                                         self._fake_patchwork)
        self.assertEqual(1, len(patches))
        patch = patches[0]
        self.assertEqual('1', patch.id)
        self.assertEqual('Some patch', patch.raw_subject)

    def test_parse_subject(self):
        """Test parsing of the patch subject"""
        patch = status.Patch('1')

        # Simple patch not in a series
        patch.parse_subject('Testing')
        self.assertEqual('Testing', patch.raw_subject)
        self.assertEqual('Testing', patch.subject)
        self.assertEqual(1, patch.seq)
        self.assertEqual(1, patch.count)
        self.assertEqual(None, patch.prefix)
        self.assertEqual(None, patch.version)

        # First patch in a series
        patch.parse_subject('[1/2] Testing')
        self.assertEqual('[1/2] Testing', patch.raw_subject)
        self.assertEqual('Testing', patch.subject)
        self.assertEqual(1, patch.seq)
        self.assertEqual(2, patch.count)
        self.assertEqual(None, patch.prefix)
        self.assertEqual(None, patch.version)

        # Second patch in a series
        patch.parse_subject('[2/2] Testing')
        self.assertEqual('Testing', patch.subject)
        self.assertEqual(2, patch.seq)
        self.assertEqual(2, patch.count)
        self.assertEqual(None, patch.prefix)
        self.assertEqual(None, patch.version)

        # RFC patch
        patch.parse_subject('[RFC,3/7] Testing')
        self.assertEqual('Testing', patch.subject)
        self.assertEqual(3, patch.seq)
        self.assertEqual(7, patch.count)
        self.assertEqual('RFC', patch.prefix)
        self.assertEqual(None, patch.version)

        # Version patch
        patch.parse_subject('[v2,3/7] Testing')
        self.assertEqual('Testing', patch.subject)
        self.assertEqual(3, patch.seq)
        self.assertEqual(7, patch.count)
        self.assertEqual(None, patch.prefix)
        self.assertEqual('v2', patch.version)

        # All fields
        patch.parse_subject('[RESEND,v2,3/7] Testing')
        self.assertEqual('Testing', patch.subject)
        self.assertEqual(3, patch.seq)
        self.assertEqual(7, patch.count)
        self.assertEqual('RESEND', patch.prefix)
        self.assertEqual('v2', patch.version)

        # RFC only
        patch.parse_subject('[RESEND] Testing')
        self.assertEqual('Testing', patch.subject)
        self.assertEqual(1, patch.seq)
        self.assertEqual(1, patch.count)
        self.assertEqual('RESEND', patch.prefix)
        self.assertEqual(None, patch.version)

    def test_compare_series(self):
        """Test operation of compare_with_series()"""
        commit1 = Commit('abcd')
        commit1.subject = 'Subject 1'
        commit2 = Commit('ef12')
        commit2.subject = 'Subject 2'
        commit3 = Commit('3456')
        commit3.subject = 'Subject 2'

        patch1 = status.Patch('1')
        patch1.subject = 'Subject 1'
        patch2 = status.Patch('2')
        patch2.subject = 'Subject 2'
        patch3 = status.Patch('3')
        patch3.subject = 'Subject 2'

        series = Series()
        series.commits = [commit1]
        patches = [patch1]
        patch_for_commit, commit_for_patch, warnings = (
            status.compare_with_series(series, patches))
        self.assertEqual(1, len(patch_for_commit))
        self.assertEqual(patch1, patch_for_commit[0])
        self.assertEqual(1, len(commit_for_patch))
        self.assertEqual(commit1, commit_for_patch[0])

        series.commits = [commit1]
        patches = [patch1, patch2]
        patch_for_commit, commit_for_patch, warnings = (
            status.compare_with_series(series, patches))
        self.assertEqual(1, len(patch_for_commit))
        self.assertEqual(patch1, patch_for_commit[0])
        self.assertEqual(1, len(commit_for_patch))
        self.assertEqual(commit1, commit_for_patch[0])
        self.assertEqual(["Cannot find commit for patch 2 ('Subject 2')"],
                         warnings)

        series.commits = [commit1, commit2]
        patches = [patch1]
        patch_for_commit, commit_for_patch, warnings = (
            status.compare_with_series(series, patches))
        self.assertEqual(1, len(patch_for_commit))
        self.assertEqual(patch1, patch_for_commit[0])
        self.assertEqual(1, len(commit_for_patch))
        self.assertEqual(commit1, commit_for_patch[0])
        self.assertEqual(["Cannot find patch for commit 2 ('Subject 2')"],
                         warnings)

        series.commits = [commit1, commit2, commit3]
        patches = [patch1, patch2]
        patch_for_commit, commit_for_patch, warnings = (
            status.compare_with_series(series, patches))
        self.assertEqual(2, len(patch_for_commit))
        self.assertEqual(patch1, patch_for_commit[0])
        self.assertEqual(patch2, patch_for_commit[1])
        self.assertEqual(1, len(commit_for_patch))
        self.assertEqual(commit1, commit_for_patch[0])
        self.assertEqual(["Cannot find patch for commit 3 ('Subject 2')",
                          "Multiple commits match patch 2 ('Subject 2'):\n"
                          '   Subject 2\n   Subject 2'],
                         warnings)

        series.commits = [commit1, commit2]
        patches = [patch1, patch2, patch3]
        patch_for_commit, commit_for_patch, warnings = (
            status.compare_with_series(series, patches))
        self.assertEqual(1, len(patch_for_commit))
        self.assertEqual(patch1, patch_for_commit[0])
        self.assertEqual(2, len(commit_for_patch))
        self.assertEqual(commit1, commit_for_patch[0])
        self.assertEqual(["Multiple patches match commit 2 ('Subject 2'):\n"
                          '   Subject 2\n   Subject 2',
                          "Cannot find commit for patch 3 ('Subject 2')"],
                         warnings)

    def _fake_patchwork2(self, url, subpath):
        """Fake Patchwork server for the function below

        This handles accessing series, patches and comments, providing the data
        in self.patches to the caller

        Args:
            url (str): URL of patchwork server
            subpath (str): URL subpath to use
        """
        re_series = re.match(r'series/(\d*)/$', subpath)
        re_patch = re.match(r'patches/(\d*)/$', subpath)
        re_comments = re.match(r'patches/(\d*)/comments/$', subpath)
        if re_series:
            series_num = re_series.group(1)
            if series_num == '1234':
                return {'patches': self.patches}
        elif re_patch:
            patch_num = int(re_patch.group(1))
            patch = self.patches[patch_num - 1]
            return patch
        elif re_comments:
            patch_num = int(re_comments.group(1))
            patch = self.patches[patch_num - 1]
            return patch.comments
        raise ValueError('Fake Patchwork does not understand: %s' % subpath)

    def test_find_new_responses(self):
        """Test operation of find_new_responses()"""
        commit1 = Commit('abcd')
        commit1.subject = 'Subject 1'
        commit2 = Commit('ef12')
        commit2.subject = 'Subject 2'

        patch1 = status.Patch('1')
        patch1.parse_subject('[1/2] Subject 1')
        patch1.name = patch1.raw_subject
        patch1.content = 'This is my patch content'
        comment1a = {'content': 'Reviewed-by: %s\n' % self.joe}

        patch1.comments = [comment1a]

        patch2 = status.Patch('2')
        patch2.parse_subject('[2/2] Subject 2')
        patch2.name = patch2.raw_subject
        patch2.content = 'Some other patch content'
        comment2a = {
            'content': 'Reviewed-by: %s\nTested-by: %s\n' %
                       (self.mary, self.leb)}
        comment2b = {'content': 'Reviewed-by: %s' % self.fred}
        patch2.comments = [comment2a, comment2b]

        # This test works by setting up commits and patch for use by the fake
        # Rest API function _fake_patchwork2(). It calls various functions in
        # the status module after setting up tags in the commits, checking that
        # things behaves as expected
        self.commits = [commit1, commit2]
        self.patches = [patch1, patch2]
        count = 2
        new_rtag_list = [None] * count
        review_list = [None, None]

        # Check that the tags are picked up on the first patch
        status.find_new_responses(new_rtag_list, review_list, 0, commit1,
                                  patch1, None, self._fake_patchwork2)
        self.assertEqual(new_rtag_list[0], {'Reviewed-by': {self.joe}})

        # Now the second patch
        status.find_new_responses(new_rtag_list, review_list, 1, commit2,
                                  patch2, None, self._fake_patchwork2)
        self.assertEqual(new_rtag_list[1], {
            'Reviewed-by': {self.mary, self.fred},
            'Tested-by': {self.leb}})

        # Now add some tags to the commit, which means they should not appear as
        # 'new' tags when scanning comments
        new_rtag_list = [None] * count
        commit1.rtags = {'Reviewed-by': {self.joe}}
        status.find_new_responses(new_rtag_list, review_list, 0, commit1,
                                  patch1, None, self._fake_patchwork2)
        self.assertEqual(new_rtag_list[0], {})

        # For the second commit, add Ed and Fred, so only Mary should be left
        commit2.rtags = {
            'Tested-by': {self.leb},
            'Reviewed-by': {self.fred}}
        status.find_new_responses(new_rtag_list, review_list, 1, commit2,
                                  patch2, None, self._fake_patchwork2)
        self.assertEqual(new_rtag_list[1], {'Reviewed-by': {self.mary}})

        # Check that the output patches expectations:
        #   1 Subject 1
        #     Reviewed-by: Joe Bloggs <joe@napierwallies.co.nz>
        #   2 Subject 2
        #     Tested-by: Lord Edmund Blackaddër <weasel@blackadder.org>
        #     Reviewed-by: Fred Bloggs <f.bloggs@napier.net>
        #   + Reviewed-by: Mary Bloggs <mary@napierwallies.co.nz>
        # 1 new response available in patchwork

        series = Series()
        series.commits = [commit1, commit2]
        terminal.set_print_test_mode()
        status.check_patchwork_status(series, '1234', None, None, False, False,
                                      None, self._fake_patchwork2)
        lines = iter(terminal.get_print_test_lines())
        col = terminal.Color()
        self.assertEqual(terminal.PrintLine('  1 Subject 1', col.BLUE),
                         next(lines))
        self.assertEqual(
            terminal.PrintLine('    Reviewed-by: ', col.GREEN, newline=False,
                               bright=False),
            next(lines))
        self.assertEqual(terminal.PrintLine(self.joe, col.WHITE, bright=False),
                         next(lines))

        self.assertEqual(terminal.PrintLine('  2 Subject 2', col.BLUE),
                         next(lines))
        self.assertEqual(
            terminal.PrintLine('    Reviewed-by: ', col.GREEN, newline=False,
                               bright=False),
            next(lines))
        self.assertEqual(terminal.PrintLine(self.fred, col.WHITE, bright=False),
                         next(lines))
        self.assertEqual(
            terminal.PrintLine('    Tested-by: ', col.GREEN, newline=False,
                               bright=False),
            next(lines))
        self.assertEqual(terminal.PrintLine(self.leb, col.WHITE, bright=False),
                         next(lines))
        self.assertEqual(
            terminal.PrintLine('  + Reviewed-by: ', col.GREEN, newline=False),
            next(lines))
        self.assertEqual(terminal.PrintLine(self.mary, col.WHITE),
                         next(lines))
        self.assertEqual(terminal.PrintLine(
            '1 new response available in patchwork (use -d to write them to a new branch)',
            None), next(lines))

    def _fake_patchwork3(self, url, subpath):
        """Fake Patchwork server for the function below

        This handles accessing series, patches and comments, providing the data
        in self.patches to the caller

        Args:
            url (str): URL of patchwork server
            subpath (str): URL subpath to use
        """
        re_series = re.match(r'series/(\d*)/$', subpath)
        re_patch = re.match(r'patches/(\d*)/$', subpath)
        re_comments = re.match(r'patches/(\d*)/comments/$', subpath)
        if re_series:
            series_num = re_series.group(1)
            if series_num == '1234':
                return {'patches': self.patches}
        elif re_patch:
            patch_num = int(re_patch.group(1))
            patch = self.patches[patch_num - 1]
            return patch
        elif re_comments:
            patch_num = int(re_comments.group(1))
            patch = self.patches[patch_num - 1]
            return patch.comments
        raise ValueError('Fake Patchwork does not understand: %s' % subpath)

    def test_create_branch(self):
        """Test operation of create_branch()"""
        repo = self.make_git_tree()
        branch = 'first'
        dest_branch = 'first2'
        count = 2
        gitdir = os.path.join(self.gitdir, '.git')

        # Set up the test git tree. We use branch 'first' which has two commits
        # in it
        series = patchstream.get_metadata_for_list(branch, gitdir, count)
        self.assertEqual(2, len(series.commits))

        patch1 = status.Patch('1')
        patch1.parse_subject('[1/2] %s' % series.commits[0].subject)
        patch1.name = patch1.raw_subject
        patch1.content = 'This is my patch content'
        comment1a = {'content': 'Reviewed-by: %s\n' % self.joe}

        patch1.comments = [comment1a]

        patch2 = status.Patch('2')
        patch2.parse_subject('[2/2] %s' % series.commits[1].subject)
        patch2.name = patch2.raw_subject
        patch2.content = 'Some other patch content'
        comment2a = {
            'content': 'Reviewed-by: %s\nTested-by: %s\n' %
                       (self.mary, self.leb)}
        comment2b = {
            'content': 'Reviewed-by: %s' % self.fred}
        patch2.comments = [comment2a, comment2b]

        # This test works by setting up patches for use by the fake Rest API
        # function _fake_patchwork3(). The fake patch comments above should
        # result in new review tags that are collected and added to the commits
        # created in the destination branch.
        self.patches = [patch1, patch2]
        count = 2

        # Expected output:
        #   1 i2c: I2C things
        #   + Reviewed-by: Joe Bloggs <joe@napierwallies.co.nz>
        #   2 spi: SPI fixes
        #   + Reviewed-by: Fred Bloggs <f.bloggs@napier.net>
        #   + Reviewed-by: Mary Bloggs <mary@napierwallies.co.nz>
        #   + Tested-by: Lord Edmund Blackaddër <weasel@blackadder.org>
        # 4 new responses available in patchwork
        # 4 responses added from patchwork into new branch 'first2'
        # <unittest.result.TestResult run=8 errors=0 failures=0>

        terminal.set_print_test_mode()
        status.check_patchwork_status(series, '1234', branch, dest_branch,
                                      False, False, None, self._fake_patchwork3,
                                      repo)
        lines = terminal.get_print_test_lines()
        self.assertEqual(12, len(lines))
        self.assertEqual(
            "4 responses added from patchwork into new branch 'first2'",
            lines[11].text)

        # Check that the destination branch has the new tags
        new_series = patchstream.get_metadata_for_list(dest_branch, gitdir,
                                                       count)
        self.assertEqual(
            {'Reviewed-by': {self.joe}},
            new_series.commits[0].rtags)
        self.assertEqual(
            {'Tested-by': {self.leb},
             'Reviewed-by': {self.fred, self.mary}},
            new_series.commits[1].rtags)

        # Now check the actual test of the first commit message. We expect to
        # see the new tags immediately below the old ones.
        stdout = patchstream.get_list(dest_branch, count=count, git_dir=gitdir)
        lines = iter([line.strip() for line in stdout.splitlines()
                      if '-by:' in line])

        # First patch should have the review tag
        self.assertEqual('Reviewed-by: %s' % self.joe, next(lines))

        # Second patch should have the sign-off then the tested-by and two
        # reviewed-by tags
        self.assertEqual('Signed-off-by: %s' % self.leb, next(lines))
        self.assertEqual('Reviewed-by: %s' % self.fred, next(lines))
        self.assertEqual('Reviewed-by: %s' % self.mary, next(lines))
        self.assertEqual('Tested-by: %s' % self.leb, next(lines))

    def test_parse_snippets(self):
        """Test parsing of review snippets"""
        text = '''Hi Fred,

This is a comment from someone.

Something else

On some recent date, Fred wrote:
> This is why I wrote the patch
> so here it is

Now a comment about the commit message
A little more to say

Even more

> diff --git a/file.c b/file.c
> Some more code
> Code line 2
> Code line 3
> Code line 4
> Code line 5
> Code line 6
> Code line 7
> Code line 8
> Code line 9

And another comment

> @@ -153,8 +143,13 @@ def check_patch(fname, show_types=False):
>  further down on the file
>  and more code
> +Addition here
> +Another addition here
>  codey
>  more codey

and another thing in same file

> @@ -253,8 +243,13 @@
>  with no function context

one more thing

> diff --git a/tools/patman/main.py b/tools/patman/main.py
> +line of code
now a very long comment in a different file
line2
line3
line4
line5
line6
line7
line8
'''
        pstrm = PatchStream.process_text(text, True)
        self.assertEqual([], pstrm.commit.warn)

        # We expect to the filename and up to 5 lines of code context before
        # each comment. The 'On xxx wrote:' bit should be removed.
        self.assertEqual(
            [['Hi Fred,',
              'This is a comment from someone.',
              'Something else'],
             ['> This is why I wrote the patch',
              '> so here it is',
              'Now a comment about the commit message',
              'A little more to say', 'Even more'],
             ['> File: file.c', '> Code line 5', '> Code line 6',
              '> Code line 7', '> Code line 8', '> Code line 9',
              'And another comment'],
             ['> File: file.c',
              '> Line: 153 / 143: def check_patch(fname, show_types=False):',
              '>  and more code', '> +Addition here', '> +Another addition here',
              '>  codey', '>  more codey', 'and another thing in same file'],
             ['> File: file.c', '> Line: 253 / 243',
              '>  with no function context', 'one more thing'],
             ['> File: tools/patman/main.py', '> +line of code',
              'now a very long comment in a different file',
              'line2', 'line3', 'line4', 'line5', 'line6', 'line7', 'line8']],
            pstrm.snippets)

    def test_review_snippets(self):
        """Test showing of review snippets"""
        def _to_submitter(who):
            m_who = re.match('(.*) <(.*)>', who)
            return {
                'name': m_who.group(1),
                'email': m_who.group(2)
                }

        commit1 = Commit('abcd')
        commit1.subject = 'Subject 1'
        commit2 = Commit('ef12')
        commit2.subject = 'Subject 2'

        patch1 = status.Patch('1')
        patch1.parse_subject('[1/2] Subject 1')
        patch1.name = patch1.raw_subject
        patch1.content = 'This is my patch content'
        comment1a = {'submitter': _to_submitter(self.joe),
                     'content': '''Hi Fred,

On some date Fred wrote:

> diff --git a/file.c b/file.c
> Some code
> and more code

Here is my comment above the above...


Reviewed-by: %s
''' % self.joe}

        patch1.comments = [comment1a]

        patch2 = status.Patch('2')
        patch2.parse_subject('[2/2] Subject 2')
        patch2.name = patch2.raw_subject
        patch2.content = 'Some other patch content'
        comment2a = {
            'content': 'Reviewed-by: %s\nTested-by: %s\n' %
                       (self.mary, self.leb)}
        comment2b = {'submitter': _to_submitter(self.fred),
                     'content': '''Hi Fred,

On some date Fred wrote:

> diff --git a/tools/patman/commit.py b/tools/patman/commit.py
> @@ -41,6 +41,9 @@ class Commit:
>          self.rtags = collections.defaultdict(set)
>          self.warn = []
>
> +    def __str__(self):
> +        return self.subject
> +
>      def add_change(self, version, info):
>          """Add a new change line to the change list for a version.
>
A comment

Reviewed-by: %s
''' % self.fred}
        patch2.comments = [comment2a, comment2b]

        # This test works by setting up commits and patch for use by the fake
        # Rest API function _fake_patchwork2(). It calls various functions in
        # the status module after setting up tags in the commits, checking that
        # things behaves as expected
        self.commits = [commit1, commit2]
        self.patches = [patch1, patch2]

        # Check that the output patches expectations:
        #   1 Subject 1
        #     Reviewed-by: Joe Bloggs <joe@napierwallies.co.nz>
        #   2 Subject 2
        #     Tested-by: Lord Edmund Blackaddër <weasel@blackadder.org>
        #     Reviewed-by: Fred Bloggs <f.bloggs@napier.net>
        #   + Reviewed-by: Mary Bloggs <mary@napierwallies.co.nz>
        # 1 new response available in patchwork

        series = Series()
        series.commits = [commit1, commit2]
        terminal.set_print_test_mode()
        status.check_patchwork_status(series, '1234', None, None, False, True,
                                      None, self._fake_patchwork2)
        lines = iter(terminal.get_print_test_lines())
        col = terminal.Color()
        self.assertEqual(terminal.PrintLine('  1 Subject 1', col.BLUE),
                         next(lines))
        self.assertEqual(
            terminal.PrintLine('  + Reviewed-by: ', col.GREEN, newline=False),
            next(lines))
        self.assertEqual(terminal.PrintLine(self.joe, col.WHITE), next(lines))

        self.assertEqual(terminal.PrintLine('Review: %s' % self.joe, col.RED),
                         next(lines))
        self.assertEqual(terminal.PrintLine('    Hi Fred,', None), next(lines))
        self.assertEqual(terminal.PrintLine('', None), next(lines))
        self.assertEqual(terminal.PrintLine('    > File: file.c', col.MAGENTA),
                         next(lines))
        self.assertEqual(terminal.PrintLine('    > Some code', col.MAGENTA),
                         next(lines))
        self.assertEqual(terminal.PrintLine('    > and more code', col.MAGENTA),
                         next(lines))
        self.assertEqual(terminal.PrintLine(
            '    Here is my comment above the above...', None), next(lines))
        self.assertEqual(terminal.PrintLine('', None), next(lines))

        self.assertEqual(terminal.PrintLine('  2 Subject 2', col.BLUE),
                         next(lines))
        self.assertEqual(
            terminal.PrintLine('  + Reviewed-by: ', col.GREEN, newline=False),
            next(lines))
        self.assertEqual(terminal.PrintLine(self.fred, col.WHITE),
                         next(lines))
        self.assertEqual(
            terminal.PrintLine('  + Reviewed-by: ', col.GREEN, newline=False),
            next(lines))
        self.assertEqual(terminal.PrintLine(self.mary, col.WHITE),
                         next(lines))
        self.assertEqual(
            terminal.PrintLine('  + Tested-by: ', col.GREEN, newline=False),
            next(lines))
        self.assertEqual(terminal.PrintLine(self.leb, col.WHITE),
                         next(lines))

        self.assertEqual(terminal.PrintLine('Review: %s' % self.fred, col.RED),
                         next(lines))
        self.assertEqual(terminal.PrintLine('    Hi Fred,', None), next(lines))
        self.assertEqual(terminal.PrintLine('', None), next(lines))
        self.assertEqual(terminal.PrintLine(
            '    > File: tools/patman/commit.py', col.MAGENTA), next(lines))
        self.assertEqual(terminal.PrintLine(
            '    > Line: 41 / 41: class Commit:', col.MAGENTA), next(lines))
        self.assertEqual(terminal.PrintLine(
            '    > +        return self.subject', col.MAGENTA), next(lines))
        self.assertEqual(terminal.PrintLine(
            '    > +', col.MAGENTA), next(lines))
        self.assertEqual(
            terminal.PrintLine('    >      def add_change(self, version, info):',
                               col.MAGENTA),
            next(lines))
        self.assertEqual(terminal.PrintLine(
            '    >          """Add a new change line to the change list for a version.',
            col.MAGENTA), next(lines))
        self.assertEqual(terminal.PrintLine(
            '    >', col.MAGENTA), next(lines))
        self.assertEqual(terminal.PrintLine(
            '    A comment', None), next(lines))
        self.assertEqual(terminal.PrintLine('', None), next(lines))

        self.assertEqual(terminal.PrintLine(
            '4 new responses available in patchwork (use -d to write them to a new branch)',
            None), next(lines))

    def test_insert_tags(self):
        """Test inserting of review tags"""
        msg = '''first line
second line.'''
        tags = [
            'Reviewed-by: Bin Meng <bmeng.cn@gmail.com>',
            'Tested-by: Bin Meng <bmeng.cn@gmail.com>'
            ]
        signoff = 'Signed-off-by: Simon Glass <sjg@chromium.com>'
        tag_str = '\n'.join(tags)

        new_msg = patchstream.insert_tags(msg, tags)
        self.assertEqual(msg + '\n\n' + tag_str, new_msg)

        new_msg = patchstream.insert_tags(msg + '\n', tags)
        self.assertEqual(msg + '\n\n' + tag_str, new_msg)

        msg += '\n\n' + signoff
        new_msg = patchstream.insert_tags(msg, tags)
        self.assertEqual(msg + '\n' + tag_str, new_msg)
