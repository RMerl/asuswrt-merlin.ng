# -*- coding: utf-8 -*-
# SPDX-License-Identifier:	GPL-2.0+
#
# Copyright 2017 Google, Inc
#

import contextlib
import os
import re
import shutil
import sys
import tempfile
import unittest

import gitutil
import patchstream
import settings


@contextlib.contextmanager
def capture():
    import sys
    from cStringIO import StringIO
    oldout,olderr = sys.stdout, sys.stderr
    try:
        out=[StringIO(), StringIO()]
        sys.stdout,sys.stderr = out
        yield out
    finally:
        sys.stdout,sys.stderr = oldout, olderr
        out[0] = out[0].getvalue()
        out[1] = out[1].getvalue()


class TestFunctional(unittest.TestCase):
    def setUp(self):
        self.tmpdir = tempfile.mkdtemp(prefix='patman.')

    def tearDown(self):
        shutil.rmtree(self.tmpdir)

    @staticmethod
    def GetPath(fname):
        return os.path.join(os.path.dirname(os.path.realpath(sys.argv[0])),
                            'test', fname)

    @classmethod
    def GetText(self, fname):
        return open(self.GetPath(fname)).read()

    @classmethod
    def GetPatchName(self, subject):
        fname = re.sub('[ :]', '-', subject)
        return fname.replace('--', '-')

    def CreatePatchesForTest(self, series):
        cover_fname = None
        fname_list = []
        for i, commit in enumerate(series.commits):
            clean_subject = self.GetPatchName(commit.subject)
            src_fname = '%04d-%s.patch' % (i + 1, clean_subject[:52])
            fname = os.path.join(self.tmpdir, src_fname)
            shutil.copy(self.GetPath(src_fname), fname)
            fname_list.append(fname)
        if series.get('cover'):
            src_fname = '0000-cover-letter.patch'
            cover_fname = os.path.join(self.tmpdir, src_fname)
            fname = os.path.join(self.tmpdir, src_fname)
            shutil.copy(self.GetPath(src_fname), fname)

        return cover_fname, fname_list

    def testBasic(self):
        """Tests the basic flow of patman

        This creates a series from some hard-coded patches build from a simple
        tree with the following metadata in the top commit:

            Series-to: u-boot
            Series-prefix: RFC
            Series-cc: Stefan Brüns <stefan.bruens@rwth-aachen.de>
            Cover-letter-cc: Lord Mëlchett <clergy@palace.gov>
            Series-version: 2
            Series-changes: 4
            - Some changes

            Cover-letter:
            test: A test patch series
            This is a test of how the cover
            leter
            works
            END

        and this in the first commit:

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
            - Series-to, Series-cc, Series-prefix, Cover-letter
            - Cover-letter-cc, Series-version, Series-changes, Series-notes
            - Commit-notes
        """
        process_tags = True
        ignore_bad_tags = True
        stefan = u'Stefan Brüns <stefan.bruens@rwth-aachen.de>'
        rick = 'Richard III <richard@palace.gov>'
        mel = u'Lord Mëlchett <clergy@palace.gov>'
        ed = u'Lond Edmund Blackaddër <weasel@blackadder.org'
        fred = 'Fred Bloggs <f.bloggs@napier.net>'
        add_maintainers = [stefan, rick]
        dry_run = True
        in_reply_to = mel
        count = 2
        settings.alias = {
                'fdt': ['simon'],
                'u-boot': ['u-boot@lists.denx.de'],
                'simon': [ed],
                'fred': [fred],
        }

        text = self.GetText('test01.txt')
        series = patchstream.GetMetaDataForTest(text)
        cover_fname, args = self.CreatePatchesForTest(series)
        with capture() as out:
            patchstream.FixPatches(series, args)
            if cover_fname and series.get('cover'):
                patchstream.InsertCoverLetter(cover_fname, series, count)
            series.DoChecks()
            cc_file = series.MakeCcFile(process_tags, cover_fname,
                                        not ignore_bad_tags, add_maintainers,
                                        None)
            cmd = gitutil.EmailPatches(series, cover_fname, args,
                    dry_run, not ignore_bad_tags, cc_file,
                    in_reply_to=in_reply_to, thread=None)
            series.ShowActions(args, cmd, process_tags)
        cc_lines = open(cc_file).read().splitlines()
        os.remove(cc_file)

        lines = out[0].splitlines()
        #print '\n'.join(lines)
        self.assertEqual('Cleaned %s patches' % len(series.commits), lines[0])
        self.assertEqual('Change log missing for v2', lines[1])
        self.assertEqual('Change log missing for v3', lines[2])
        self.assertEqual('Change log for unknown version v4', lines[3])
        self.assertEqual("Alias 'pci' not found", lines[4])
        self.assertIn('Dry run', lines[5])
        self.assertIn('Send a total of %d patches' % count, lines[7])
        line = 8
        for i, commit in enumerate(series.commits):
            self.assertEqual('   %s' % args[i], lines[line + 0])
            line += 1
            while 'Cc:' in lines[line]:
                line += 1
        self.assertEqual('To:	  u-boot@lists.denx.de', lines[line])
        self.assertEqual('Cc:	  %s' % stefan.encode('utf-8'), lines[line + 1])
        self.assertEqual('Version:  3', lines[line + 2])
        self.assertEqual('Prefix:\t  RFC', lines[line + 3])
        self.assertEqual('Cover: 4 lines', lines[line + 4])
        line += 5
        self.assertEqual('      Cc:  %s' % mel.encode('utf-8'), lines[line + 0])
        self.assertEqual('      Cc:  %s' % rick, lines[line + 1])
        self.assertEqual('      Cc:  %s' % fred, lines[line + 2])
        self.assertEqual('      Cc:  %s' % ed.encode('utf-8'), lines[line + 3])
        expected = ('Git command: git send-email --annotate '
                    '--in-reply-to="%s" --to "u-boot@lists.denx.de" '
                    '--cc "%s" --cc-cmd "%s --cc-cmd %s" %s %s'
                    % (in_reply_to, stefan, sys.argv[0], cc_file, cover_fname,
                       ' '.join(args))).encode('utf-8')
        line += 4
        self.assertEqual(expected, lines[line])

        self.assertEqual(('%s %s, %s' % (args[0], rick, stefan))
                         .encode('utf-8'), cc_lines[0])
        self.assertEqual(('%s %s, %s, %s, %s' % (args[1], fred, rick, stefan,
                                            ed)).encode('utf-8'), cc_lines[1])

        expected = '''
This is a test of how the cover
leter
works

some notes
about some things
from the first commit

Changes in v4:
- Some changes

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
        lines = open(cover_fname).read().splitlines()
        #print '\n'.join(lines)
        self.assertEqual(
                'Subject: [RFC PATCH v3 0/2] test: A test patch series',
                lines[3])
        self.assertEqual(expected.splitlines(), lines[7:])

        for i, fname in enumerate(args):
            lines = open(fname).read().splitlines()
            #print '\n'.join(lines)
            subject = [line for line in lines if line.startswith('Subject')]
            self.assertEqual('Subject: [RFC %d/%d]' % (i + 1, count),
                             subject[0][:18])
            if i == 0:
                # Check that we got our commit notes
                self.assertEqual('---', lines[17])
                self.assertEqual('Some notes about', lines[18])
                self.assertEqual('the first commit', lines[19])
