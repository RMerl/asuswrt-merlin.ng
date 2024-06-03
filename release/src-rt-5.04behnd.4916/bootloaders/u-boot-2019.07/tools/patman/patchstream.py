# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2011 The Chromium OS Authors.
#

import math
import os
import re
import shutil
import tempfile

import command
import commit
import gitutil
from series import Series

# Tags that we detect and remove
re_remove = re.compile('^BUG=|^TEST=|^BRANCH=|^Change-Id:|^Review URL:'
    '|Reviewed-on:|Commit-\w*:')

# Lines which are allowed after a TEST= line
re_allowed_after_test = re.compile('^Signed-off-by:')

# Signoffs
re_signoff = re.compile('^Signed-off-by: *(.*)')

# The start of the cover letter
re_cover = re.compile('^Cover-letter:')

# A cover letter Cc
re_cover_cc = re.compile('^Cover-letter-cc: *(.*)')

# Patch series tag
re_series_tag = re.compile('^Series-([a-z-]*): *(.*)')

# Commit series tag
re_commit_tag = re.compile('^Commit-([a-z-]*): *(.*)')

# Commit tags that we want to collect and keep
re_tag = re.compile('^(Tested-by|Acked-by|Reviewed-by|Patch-cc): (.*)')

# The start of a new commit in the git log
re_commit = re.compile('^commit ([0-9a-f]*)$')

# We detect these since checkpatch doesn't always do it
re_space_before_tab = re.compile('^[+].* \t')

# States we can be in - can we use range() and still have comments?
STATE_MSG_HEADER = 0        # Still in the message header
STATE_PATCH_SUBJECT = 1     # In patch subject (first line of log for a commit)
STATE_PATCH_HEADER = 2      # In patch header (after the subject)
STATE_DIFFS = 3             # In the diff part (past --- line)

class PatchStream:
    """Class for detecting/injecting tags in a patch or series of patches

    We support processing the output of 'git log' to read out the tags we
    are interested in. We can also process a patch file in order to remove
    unwanted tags or inject additional ones. These correspond to the two
    phases of processing.
    """
    def __init__(self, series, name=None, is_log=False):
        self.skip_blank = False          # True to skip a single blank line
        self.found_test = False          # Found a TEST= line
        self.lines_after_test = 0        # MNumber of lines found after TEST=
        self.warn = []                   # List of warnings we have collected
        self.linenum = 1                 # Output line number we are up to
        self.in_section = None           # Name of start...END section we are in
        self.notes = []                  # Series notes
        self.section = []                # The current section...END section
        self.series = series             # Info about the patch series
        self.is_log = is_log             # True if indent like git log
        self.in_change = 0               # Non-zero if we are in a change list
        self.blank_count = 0             # Number of blank lines stored up
        self.state = STATE_MSG_HEADER    # What state are we in?
        self.signoff = []                # Contents of signoff line
        self.commit = None               # Current commit

    def AddToSeries(self, line, name, value):
        """Add a new Series-xxx tag.

        When a Series-xxx tag is detected, we come here to record it, if we
        are scanning a 'git log'.

        Args:
            line: Source line containing tag (useful for debug/error messages)
            name: Tag name (part after 'Series-')
            value: Tag value (part after 'Series-xxx: ')
        """
        if name == 'notes':
            self.in_section = name
            self.skip_blank = False
        if self.is_log:
            self.series.AddTag(self.commit, line, name, value)

    def AddToCommit(self, line, name, value):
        """Add a new Commit-xxx tag.

        When a Commit-xxx tag is detected, we come here to record it.

        Args:
            line: Source line containing tag (useful for debug/error messages)
            name: Tag name (part after 'Commit-')
            value: Tag value (part after 'Commit-xxx: ')
        """
        if name == 'notes':
            self.in_section = 'commit-' + name
            self.skip_blank = False

    def CloseCommit(self):
        """Save the current commit into our commit list, and reset our state"""
        if self.commit and self.is_log:
            self.series.AddCommit(self.commit)
            self.commit = None
        # If 'END' is missing in a 'Cover-letter' section, and that section
        # happens to show up at the very end of the commit message, this is
        # the chance for us to fix it up.
        if self.in_section == 'cover' and self.is_log:
            self.series.cover = self.section
            self.in_section = None
            self.skip_blank = True
            self.section = []

    def ProcessLine(self, line):
        """Process a single line of a patch file or commit log

        This process a line and returns a list of lines to output. The list
        may be empty or may contain multiple output lines.

        This is where all the complicated logic is located. The class's
        state is used to move between different states and detect things
        properly.

        We can be in one of two modes:
            self.is_log == True: This is 'git log' mode, where most output is
                indented by 4 characters and we are scanning for tags

            self.is_log == False: This is 'patch' mode, where we already have
                all the tags, and are processing patches to remove junk we
                don't want, and add things we think are required.

        Args:
            line: text line to process

        Returns:
            list of output lines, or [] if nothing should be output
        """
        # Initially we have no output. Prepare the input line string
        out = []
        line = line.rstrip('\n')

        commit_match = re_commit.match(line) if self.is_log else None

        if self.is_log:
            if line[:4] == '    ':
                line = line[4:]

        # Handle state transition and skipping blank lines
        series_tag_match = re_series_tag.match(line)
        commit_tag_match = re_commit_tag.match(line)
        cover_match = re_cover.match(line)
        cover_cc_match = re_cover_cc.match(line)
        signoff_match = re_signoff.match(line)
        tag_match = None
        if self.state == STATE_PATCH_HEADER:
            tag_match = re_tag.match(line)
        is_blank = not line.strip()
        if is_blank:
            if (self.state == STATE_MSG_HEADER
                    or self.state == STATE_PATCH_SUBJECT):
                self.state += 1

            # We don't have a subject in the text stream of patch files
            # It has its own line with a Subject: tag
            if not self.is_log and self.state == STATE_PATCH_SUBJECT:
                self.state += 1
        elif commit_match:
            self.state = STATE_MSG_HEADER

        # If a tag is detected, or a new commit starts
        if series_tag_match or commit_tag_match or \
           cover_match or cover_cc_match or signoff_match or \
           self.state == STATE_MSG_HEADER:
            # but we are already in a section, this means 'END' is missing
            # for that section, fix it up.
            if self.in_section:
                self.warn.append("Missing 'END' in section '%s'" % self.in_section)
                if self.in_section == 'cover':
                    self.series.cover = self.section
                elif self.in_section == 'notes':
                    if self.is_log:
                        self.series.notes += self.section
                elif self.in_section == 'commit-notes':
                    if self.is_log:
                        self.commit.notes += self.section
                else:
                    self.warn.append("Unknown section '%s'" % self.in_section)
                self.in_section = None
                self.skip_blank = True
                self.section = []
            # but we are already in a change list, that means a blank line
            # is missing, fix it up.
            if self.in_change:
                self.warn.append("Missing 'blank line' in section 'Series-changes'")
                self.in_change = 0

        # If we are in a section, keep collecting lines until we see END
        if self.in_section:
            if line == 'END':
                if self.in_section == 'cover':
                    self.series.cover = self.section
                elif self.in_section == 'notes':
                    if self.is_log:
                        self.series.notes += self.section
                elif self.in_section == 'commit-notes':
                    if self.is_log:
                        self.commit.notes += self.section
                else:
                    self.warn.append("Unknown section '%s'" % self.in_section)
                self.in_section = None
                self.skip_blank = True
                self.section = []
            else:
                self.section.append(line)

        # Detect the commit subject
        elif not is_blank and self.state == STATE_PATCH_SUBJECT:
            self.commit.subject = line

        # Detect the tags we want to remove, and skip blank lines
        elif re_remove.match(line) and not commit_tag_match:
            self.skip_blank = True

            # TEST= should be the last thing in the commit, so remove
            # everything after it
            if line.startswith('TEST='):
                self.found_test = True
        elif self.skip_blank and is_blank:
            self.skip_blank = False

        # Detect the start of a cover letter section
        elif cover_match:
            self.in_section = 'cover'
            self.skip_blank = False

        elif cover_cc_match:
            value = cover_cc_match.group(1)
            self.AddToSeries(line, 'cover-cc', value)

        # If we are in a change list, key collected lines until a blank one
        elif self.in_change:
            if is_blank:
                # Blank line ends this change list
                self.in_change = 0
            elif line == '---':
                self.in_change = 0
                out = self.ProcessLine(line)
            else:
                if self.is_log:
                    self.series.AddChange(self.in_change, self.commit, line)
            self.skip_blank = False

        # Detect Series-xxx tags
        elif series_tag_match:
            name = series_tag_match.group(1)
            value = series_tag_match.group(2)
            if name == 'changes':
                # value is the version number: e.g. 1, or 2
                try:
                    value = int(value)
                except ValueError as str:
                    raise ValueError("%s: Cannot decode version info '%s'" %
                        (self.commit.hash, line))
                self.in_change = int(value)
            else:
                self.AddToSeries(line, name, value)
                self.skip_blank = True

        # Detect Commit-xxx tags
        elif commit_tag_match:
            name = commit_tag_match.group(1)
            value = commit_tag_match.group(2)
            if name == 'notes':
                self.AddToCommit(line, name, value)
                self.skip_blank = True

        # Detect the start of a new commit
        elif commit_match:
            self.CloseCommit()
            self.commit = commit.Commit(commit_match.group(1))

        # Detect tags in the commit message
        elif tag_match:
            # Remove Tested-by self, since few will take much notice
            if (tag_match.group(1) == 'Tested-by' and
                    tag_match.group(2).find(os.getenv('USER') + '@') != -1):
                self.warn.append("Ignoring %s" % line)
            elif tag_match.group(1) == 'Patch-cc':
                self.commit.AddCc(tag_match.group(2).split(','))
            else:
                out = [line]

        # Suppress duplicate signoffs
        elif signoff_match:
            if (self.is_log or not self.commit or
                self.commit.CheckDuplicateSignoff(signoff_match.group(1))):
                out = [line]

        # Well that means this is an ordinary line
        else:
            # Look for space before tab
            m = re_space_before_tab.match(line)
            if m:
                self.warn.append('Line %d/%d has space before tab' %
                    (self.linenum, m.start()))

            # OK, we have a valid non-blank line
            out = [line]
            self.linenum += 1
            self.skip_blank = False
            if self.state == STATE_DIFFS:
                pass

            # If this is the start of the diffs section, emit our tags and
            # change log
            elif line == '---':
                self.state = STATE_DIFFS

                # Output the tags (signeoff first), then change list
                out = []
                log = self.series.MakeChangeLog(self.commit)
                out += [line]
                if self.commit:
                    out += self.commit.notes
                out += [''] + log
            elif self.found_test:
                if not re_allowed_after_test.match(line):
                    self.lines_after_test += 1

        return out

    def Finalize(self):
        """Close out processing of this patch stream"""
        self.CloseCommit()
        if self.lines_after_test:
            self.warn.append('Found %d lines after TEST=' %
                    self.lines_after_test)

    def ProcessStream(self, infd, outfd):
        """Copy a stream from infd to outfd, filtering out unwanting things.

        This is used to process patch files one at a time.

        Args:
            infd: Input stream file object
            outfd: Output stream file object
        """
        # Extract the filename from each diff, for nice warnings
        fname = None
        last_fname = None
        re_fname = re.compile('diff --git a/(.*) b/.*')
        while True:
            line = infd.readline()
            if not line:
                break
            out = self.ProcessLine(line)

            # Try to detect blank lines at EOF
            for line in out:
                match = re_fname.match(line)
                if match:
                    last_fname = fname
                    fname = match.group(1)
                if line == '+':
                    self.blank_count += 1
                else:
                    if self.blank_count and (line == '-- ' or match):
                        self.warn.append("Found possible blank line(s) at "
                                "end of file '%s'" % last_fname)
                    outfd.write('+\n' * self.blank_count)
                    outfd.write(line + '\n')
                    self.blank_count = 0
        self.Finalize()


def GetMetaDataForList(commit_range, git_dir=None, count=None,
                       series = None, allow_overwrite=False):
    """Reads out patch series metadata from the commits

    This does a 'git log' on the relevant commits and pulls out the tags we
    are interested in.

    Args:
        commit_range: Range of commits to count (e.g. 'HEAD..base')
        git_dir: Path to git repositiory (None to use default)
        count: Number of commits to list, or None for no limit
        series: Series object to add information into. By default a new series
            is started.
        allow_overwrite: Allow tags to overwrite an existing tag
    Returns:
        A Series object containing information about the commits.
    """
    if not series:
        series = Series()
    series.allow_overwrite = allow_overwrite
    params = gitutil.LogCmd(commit_range, reverse=True, count=count,
                            git_dir=git_dir)
    stdout = command.RunPipe([params], capture=True).stdout
    ps = PatchStream(series, is_log=True)
    for line in stdout.splitlines():
        ps.ProcessLine(line)
    ps.Finalize()
    return series

def GetMetaData(start, count):
    """Reads out patch series metadata from the commits

    This does a 'git log' on the relevant commits and pulls out the tags we
    are interested in.

    Args:
        start: Commit to start from: 0=HEAD, 1=next one, etc.
        count: Number of commits to list
    """
    return GetMetaDataForList('HEAD~%d' % start, None, count)

def GetMetaDataForTest(text):
    """Process metadata from a file containing a git log. Used for tests

    Args:
        text:
    """
    series = Series()
    ps = PatchStream(series, is_log=True)
    for line in text.splitlines():
        ps.ProcessLine(line)
    ps.Finalize()
    return series

def FixPatch(backup_dir, fname, series, commit):
    """Fix up a patch file, by adding/removing as required.

    We remove our tags from the patch file, insert changes lists, etc.
    The patch file is processed in place, and overwritten.

    A backup file is put into backup_dir (if not None).

    Args:
        fname: Filename to patch file to process
        series: Series information about this patch set
        commit: Commit object for this patch file
    Return:
        A list of errors, or [] if all ok.
    """
    handle, tmpname = tempfile.mkstemp()
    outfd = os.fdopen(handle, 'w')
    infd = open(fname, 'r')
    ps = PatchStream(series)
    ps.commit = commit
    ps.ProcessStream(infd, outfd)
    infd.close()
    outfd.close()

    # Create a backup file if required
    if backup_dir:
        shutil.copy(fname, os.path.join(backup_dir, os.path.basename(fname)))
    shutil.move(tmpname, fname)
    return ps.warn

def FixPatches(series, fnames):
    """Fix up a list of patches identified by filenames

    The patch files are processed in place, and overwritten.

    Args:
        series: The series object
        fnames: List of patch files to process
    """
    # Current workflow creates patches, so we shouldn't need a backup
    backup_dir = None  #tempfile.mkdtemp('clean-patch')
    count = 0
    for fname in fnames:
        commit = series.commits[count]
        commit.patch = fname
        result = FixPatch(backup_dir, fname, series, commit)
        if result:
            print('%d warnings for %s:' % (len(result), fname))
            for warn in result:
                print('\t', warn)
            print
        count += 1
    print('Cleaned %d patches' % count)

def InsertCoverLetter(fname, series, count):
    """Inserts a cover letter with the required info into patch 0

    Args:
        fname: Input / output filename of the cover letter file
        series: Series object
        count: Number of patches in the series
    """
    fd = open(fname, 'r')
    lines = fd.readlines()
    fd.close()

    fd = open(fname, 'w')
    text = series.cover
    prefix = series.GetPatchPrefix()
    for line in lines:
        if line.startswith('Subject:'):
            # if more than 10 or 100 patches, it should say 00/xx, 000/xxx, etc
            zero_repeat = int(math.log10(count)) + 1
            zero = '0' * zero_repeat
            line = 'Subject: [%s %s/%d] %s\n' % (prefix, zero, count, text[0])

        # Insert our cover letter
        elif line.startswith('*** BLURB HERE ***'):
            # First the blurb test
            line = '\n'.join(text[1:]) + '\n'
            if series.get('notes'):
                line += '\n'.join(series.notes) + '\n'

            # Now the change list
            out = series.MakeChangeLog(None)
            line += '\n' + '\n'.join(out)
        fd.write(line)
    fd.close()
