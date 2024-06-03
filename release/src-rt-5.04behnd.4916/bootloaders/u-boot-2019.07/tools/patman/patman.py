#!/usr/bin/env python2
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2011 The Chromium OS Authors.
#

"""See README for more information"""

from optparse import OptionParser
import os
import re
import sys
import unittest

# Our modules
try:
    from patman import checkpatch, command, gitutil, patchstream, \
        project, settings, terminal, test
except ImportError:
    import checkpatch
    import command
    import gitutil
    import patchstream
    import project
    import settings
    import terminal
    import test


parser = OptionParser()
parser.add_option('-H', '--full-help', action='store_true', dest='full_help',
       default=False, help='Display the README file')
parser.add_option('-c', '--count', dest='count', type='int',
       default=-1, help='Automatically create patches from top n commits')
parser.add_option('-i', '--ignore-errors', action='store_true',
       dest='ignore_errors', default=False,
       help='Send patches email even if patch errors are found')
parser.add_option('-m', '--no-maintainers', action='store_false',
       dest='add_maintainers', default=True,
       help="Don't cc the file maintainers automatically")
parser.add_option('-l', '--limit-cc', dest='limit', type='int',
       default=None, help='Limit the cc list to LIMIT entries [default: %default]')
parser.add_option('-n', '--dry-run', action='store_true', dest='dry_run',
       default=False, help="Do a dry run (create but don't email patches)")
parser.add_option('-p', '--project', default=project.DetectProject(),
                  help="Project name; affects default option values and "
                  "aliases [default: %default]")
parser.add_option('-r', '--in-reply-to', type='string', action='store',
                  help="Message ID that this series is in reply to")
parser.add_option('-s', '--start', dest='start', type='int',
       default=0, help='Commit to start creating patches from (0 = HEAD)')
parser.add_option('-t', '--ignore-bad-tags', action='store_true',
                  default=False, help='Ignore bad tags / aliases')
parser.add_option('--test', action='store_true', dest='test',
                  default=False, help='run tests')
parser.add_option('-v', '--verbose', action='store_true', dest='verbose',
       default=False, help='Verbose output of errors and warnings')
parser.add_option('--cc-cmd', dest='cc_cmd', type='string', action='store',
       default=None, help='Output cc list for patch file (used by git)')
parser.add_option('--no-check', action='store_false', dest='check_patch',
                  default=True,
                  help="Don't check for patch compliance")
parser.add_option('--no-tags', action='store_false', dest='process_tags',
                  default=True, help="Don't process subject tags as aliaes")
parser.add_option('--smtp-server', type='str',
                  help="Specify the SMTP server to 'git send-email'")
parser.add_option('-T', '--thread', action='store_true', dest='thread',
                  default=False, help='Create patches as a single thread')

parser.usage += """

Create patches from commits in a branch, check them and email them as
specified by tags you place in the commits. Use -n to do a dry run first."""


# Parse options twice: first to get the project and second to handle
# defaults properly (which depends on project).
(options, args) = parser.parse_args()
settings.Setup(parser, options.project, '')
(options, args) = parser.parse_args()

if __name__ != "__main__":
    pass

# Run our meagre tests
elif options.test:
    import doctest
    import func_test

    sys.argv = [sys.argv[0]]
    result = unittest.TestResult()
    for module in (test.TestPatch, func_test.TestFunctional):
        suite = unittest.TestLoader().loadTestsFromTestCase(module)
        suite.run(result)

    for module in ['gitutil', 'settings']:
        suite = doctest.DocTestSuite(module)
        suite.run(result)

    # TODO: Surely we can just 'print' result?
    print(result)
    for test, err in result.errors:
        print(err)
    for test, err in result.failures:
        print(err)

# Called from git with a patch filename as argument
# Printout a list of additional CC recipients for this patch
elif options.cc_cmd:
    fd = open(options.cc_cmd, 'r')
    re_line = re.compile('(\S*) (.*)')
    for line in fd.readlines():
        match = re_line.match(line)
        if match and match.group(1) == args[0]:
            for cc in match.group(2).split(', '):
                cc = cc.strip()
                if cc:
                    print(cc)
    fd.close()

elif options.full_help:
    pager = os.getenv('PAGER')
    if not pager:
        pager = 'more'
    fname = os.path.join(os.path.dirname(os.path.realpath(sys.argv[0])),
                         'README')
    command.Run(pager, fname)

# Process commits, produce patches files, check them, email them
else:
    gitutil.Setup()

    if options.count == -1:
        # Work out how many patches to send if we can
        options.count = gitutil.CountCommitsToBranch() - options.start

    col = terminal.Color()
    if not options.count:
        str = 'No commits found to process - please use -c flag'
        sys.exit(col.Color(col.RED, str))

    # Read the metadata from the commits
    if options.count:
        series = patchstream.GetMetaData(options.start, options.count)
        cover_fname, args = gitutil.CreatePatches(options.start, options.count,
                series)

    # Fix up the patch files to our liking, and insert the cover letter
    patchstream.FixPatches(series, args)
    if cover_fname and series.get('cover'):
        patchstream.InsertCoverLetter(cover_fname, series, options.count)

    # Do a few checks on the series
    series.DoChecks()

    # Check the patches, and run them through 'git am' just to be sure
    if options.check_patch:
        ok = checkpatch.CheckPatches(options.verbose, args)
    else:
        ok = True

    cc_file = series.MakeCcFile(options.process_tags, cover_fname,
                                not options.ignore_bad_tags,
                                options.add_maintainers, options.limit)

    # Email the patches out (giving the user time to check / cancel)
    cmd = ''
    its_a_go = ok or options.ignore_errors
    if its_a_go:
        cmd = gitutil.EmailPatches(series, cover_fname, args,
                options.dry_run, not options.ignore_bad_tags, cc_file,
                in_reply_to=options.in_reply_to, thread=options.thread,
                smtp_server=options.smtp_server)
    else:
        print(col.Color(col.RED, "Not sending emails due to errors/warnings"))

    # For a dry run, just show our actions as a sanity check
    if options.dry_run:
        series.ShowActions(args, cmd, options.process_tags)
        if not its_a_go:
            print(col.Color(col.RED, "Email would not be sent"))

    os.remove(cc_file)
