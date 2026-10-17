# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2011 The Chromium OS Authors.
#

"""Handles parsing a stream of commits/emails from 'git log' or other source"""

import collections
import datetime
import io
import math
import os
import re
import queue
import shutil
import tempfile

from patman import commit
from patman import gitutil
from patman.series import Series
from u_boot_pylib import command

# Tags that we detect and remove
RE_REMOVE = re.compile(r'^BUG=|^TEST=|^BRANCH=|^Review URL:'
                       r'|Reviewed-on:|Commit-\w*:')

# Lines which are allowed after a TEST= line
RE_ALLOWED_AFTER_TEST = re.compile('^Signed-off-by:')

# Signoffs
RE_SIGNOFF = re.compile('^Signed-off-by: *(.*)')

# Cover letter tag
RE_COVER = re.compile('^Cover-([a-z-]*): *(.*)')

# Patch series tag
RE_SERIES_TAG = re.compile('^Series-([a-z-]*): *(.*)')

# Change-Id will be used to generate the Message-Id and then be stripped
RE_CHANGE_ID = re.compile('^Change-Id: *(.*)')

# Commit series tag
RE_COMMIT_TAG = re.compile('^Commit-([a-z-]*): *(.*)')

# Commit tags that we want to collect and keep
RE_TAG = re.compile('^(Tested-by|Acked-by|Reviewed-by|Patch-cc|Fixes): (.*)')

# The start of a new commit in the git log
RE_COMMIT = re.compile('^commit ([0-9a-f]*)$')

# We detect these since checkpatch doesn't always do it
RE_SPACE_BEFORE_TAB = re.compile('^[+].* \t')

# Match indented lines for changes
RE_LEADING_WHITESPACE = re.compile(r'^\s')

# Detect a 'diff' line
RE_DIFF = re.compile(r'^>.*diff --git a/(.*) b/(.*)$')

# Detect a context line, like '> @@ -153,8 +153,13 @@ CheckPatch
RE_LINE = re.compile(r'>.*@@ \-(\d+),\d+ \+(\d+),\d+ @@ *(.*)')

# Detect line with invalid TAG
RE_INV_TAG = re.compile('^Serie-([a-z-]*): *(.*)')

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
    def __init__(self, series, is_log=False, keep_change_id=False):
        self.skip_blank = False          # True to skip a single blank line
        self.found_test = False          # Found a TEST= line
        self.lines_after_test = 0        # Number of lines found after TEST=
        self.linenum = 1                 # Output line number we are up to
        self.in_section = None           # Name of start...END section we are in
        self.notes = []                  # Series notes
        self.section = []                # The current section...END section
        self.series = series             # Info about the patch series
        self.is_log = is_log             # True if indent like git log
        self.keep_change_id = keep_change_id  # True to keep Change-Id tags
        self.in_change = None            # Name of the change list we are in
        self.change_version = 0          # Non-zero if we are in a change list
        self.change_lines = []           # Lines of the current change
        self.blank_count = 0             # Number of blank lines stored up
        self.state = STATE_MSG_HEADER    # What state are we in?
        self.commit = None               # Current commit
        # List of unquoted test blocks, each a list of str lines
        self.snippets = []
        self.cur_diff = None             # Last 'diff' line seen (str)
        self.cur_line = None             # Last context (@@) line seen (str)
        self.recent_diff = None          # 'diff' line for current snippet (str)
        self.recent_line = None          # '@@' line for current snippet (str)
        self.recent_quoted = collections.deque([], 5)
        self.recent_unquoted = queue.Queue()
        self.was_quoted = None

    @staticmethod
    def process_text(text, is_comment=False):
        """Process some text through this class using a default Commit/Series

        Args:
            text (str): Text to parse
            is_comment (bool): True if this is a comment rather than a patch.
                If True, PatchStream doesn't expect a patch subject at the
                start, but jumps straight into the body

        Returns:
            PatchStream: object with results
        """
        pstrm = PatchStream(Series())
        pstrm.commit = commit.Commit(None)
        infd = io.StringIO(text)
        outfd = io.StringIO()
        if is_comment:
            pstrm.state = STATE_PATCH_HEADER
        pstrm.process_stream(infd, outfd)
        return pstrm

    def _add_warn(self, warn):
        """Add a new warning to report to the user about the current commit

        The new warning is added to the current commit if not already present.

        Args:
            warn (str): Warning to report

        Raises:
            ValueError: Warning is generated with no commit associated
        """
        if not self.commit:
            print('Warning outside commit: %s' % warn)
        elif warn not in self.commit.warn:
            self.commit.warn.append(warn)

    def _add_to_series(self, line, name, value):
        """Add a new Series-xxx tag.

        When a Series-xxx tag is detected, we come here to record it, if we
        are scanning a 'git log'.

        Args:
            line (str): Source line containing tag (useful for debug/error
                messages)
            name (str): Tag name (part after 'Series-')
            value (str): Tag value (part after 'Series-xxx: ')
        """
        if name == 'notes':
            self.in_section = name
            self.skip_blank = False
        if self.is_log:
            warn = self.series.AddTag(self.commit, line, name, value)
            if warn:
                self.commit.warn.append(warn)

    def _add_to_commit(self, name):
        """Add a new Commit-xxx tag.

        When a Commit-xxx tag is detected, we come here to record it.

        Args:
            name (str): Tag name (part after 'Commit-')
        """
        if name == 'notes':
            self.in_section = 'commit-' + name
            self.skip_blank = False

    def _add_commit_rtag(self, rtag_type, who):
        """Add a response tag to the current commit

        Args:
            rtag_type (str): rtag type (e.g. 'Reviewed-by')
            who (str): Person who gave that rtag, e.g.
                 'Fred Bloggs <fred@bloggs.org>'
        """
        self.commit.add_rtag(rtag_type, who)

    def _close_commit(self):
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

        self.cur_diff = None
        self.recent_diff = None
        self.recent_line = None

    def _parse_version(self, value, line):
        """Parse a version from a *-changes tag

        Args:
            value (str): Tag value (part after 'xxx-changes: '
            line (str): Source line containing tag

        Returns:
            int: The version as an integer

        Raises:
            ValueError: the value cannot be converted
        """
        try:
            return int(value)
        except ValueError:
            raise ValueError("%s: Cannot decode version info '%s'" %
                             (self.commit.hash, line))

    def _finalise_change(self):
        """_finalise a (multi-line) change and add it to the series or commit"""
        if not self.change_lines:
            return
        change = '\n'.join(self.change_lines)

        if self.in_change == 'Series':
            self.series.AddChange(self.change_version, self.commit, change)
        elif self.in_change == 'Cover':
            self.series.AddChange(self.change_version, None, change)
        elif self.in_change == 'Commit':
            self.commit.add_change(self.change_version, change)
        self.change_lines = []

    def _finalise_snippet(self):
        """Finish off a snippet and add it to the list

        This is called when we get to the end of a snippet, i.e. the we enter
        the next block of quoted text:

            This is a comment from someone.

            Something else

            > Now we have some code          <----- end of snippet
            > more code

            Now a comment about the above code

        This adds the snippet to our list
        """
        quoted_lines = []
        while self.recent_quoted:
            quoted_lines.append(self.recent_quoted.popleft())
        unquoted_lines = []
        valid = False
        while not self.recent_unquoted.empty():
            text = self.recent_unquoted.get()
            if not (text.startswith('On ') and text.endswith('wrote:')):
                unquoted_lines.append(text)
            if text:
                valid = True
        if valid:
            lines = []
            if self.recent_diff:
                lines.append('> File: %s' % self.recent_diff)
            if self.recent_line:
                out = '> Line: %s / %s' % self.recent_line[:2]
                if self.recent_line[2]:
                    out += ': %s' % self.recent_line[2]
                lines.append(out)
            lines += quoted_lines + unquoted_lines
            if lines:
                self.snippets.append(lines)

    def process_line(self, line):
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
            line (str): text line to process

        Returns:
            list: list of output lines, or [] if nothing should be output

        Raises:
            ValueError: a fatal error occurred while parsing, e.g. an END
                without a starting tag, or two commits with two change IDs
        """
        # Initially we have no output. Prepare the input line string
        out = []
        line = line.rstrip('\n')

        commit_match = RE_COMMIT.match(line) if self.is_log else None

        if self.is_log:
            if line[:4] == '    ':
                line = line[4:]

        # Handle state transition and skipping blank lines
        series_tag_match = RE_SERIES_TAG.match(line)
        change_id_match = RE_CHANGE_ID.match(line)
        commit_tag_match = RE_COMMIT_TAG.match(line)
        cover_match = RE_COVER.match(line)
        signoff_match = RE_SIGNOFF.match(line)
        leading_whitespace_match = RE_LEADING_WHITESPACE.match(line)
        diff_match = RE_DIFF.match(line)
        line_match = RE_LINE.match(line)
        invalid_match = RE_INV_TAG.match(line)
        tag_match = None
        if self.state == STATE_PATCH_HEADER:
            tag_match = RE_TAG.match(line)
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
        if series_tag_match or commit_tag_match or change_id_match or \
           cover_match or signoff_match or self.state == STATE_MSG_HEADER:
            # but we are already in a section, this means 'END' is missing
            # for that section, fix it up.
            if self.in_section:
                self._add_warn("Missing 'END' in section '%s'" % self.in_section)
                if self.in_section == 'cover':
                    self.series.cover = self.section
                elif self.in_section == 'notes':
                    if self.is_log:
                        self.series.notes += self.section
                elif self.in_section == 'commit-notes':
                    if self.is_log:
                        self.commit.notes += self.section
                else:
                    # This should not happen
                    raise ValueError("Unknown section '%s'" % self.in_section)
                self.in_section = None
                self.skip_blank = True
                self.section = []
            # but we are already in a change list, that means a blank line
            # is missing, fix it up.
            if self.in_change:
                self._add_warn("Missing 'blank line' in section '%s-changes'" %
                               self.in_change)
                self._finalise_change()
                self.in_change = None
                self.change_version = 0

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
                    # This should not happen
                    raise ValueError("Unknown section '%s'" % self.in_section)
                self.in_section = None
                self.skip_blank = True
                self.section = []
            else:
                self.section.append(line)

        # If we are not in a section, it is an unexpected END
        elif line == 'END':
            raise ValueError("'END' wihout section")

        # Detect the commit subject
        elif not is_blank and self.state == STATE_PATCH_SUBJECT:
            self.commit.subject = line

        # Detect the tags we want to remove, and skip blank lines
        elif RE_REMOVE.match(line) and not commit_tag_match:
            self.skip_blank = True

            # TEST= should be the last thing in the commit, so remove
            # everything after it
            if line.startswith('TEST='):
                self.found_test = True
        elif self.skip_blank and is_blank:
            self.skip_blank = False

        # Detect Cover-xxx tags
        elif cover_match:
            name = cover_match.group(1)
            value = cover_match.group(2)
            if name == 'letter':
                self.in_section = 'cover'
                self.skip_blank = False
            elif name == 'letter-cc':
                self._add_to_series(line, 'cover-cc', value)
            elif name == 'changes':
                self.in_change = 'Cover'
                self.change_version = self._parse_version(value, line)

        # If we are in a change list, key collected lines until a blank one
        elif self.in_change:
            if is_blank:
                # Blank line ends this change list
                self._finalise_change()
                self.in_change = None
                self.change_version = 0
            elif line == '---':
                self._finalise_change()
                self.in_change = None
                self.change_version = 0
                out = self.process_line(line)
            elif self.is_log:
                if not leading_whitespace_match:
                    self._finalise_change()
                self.change_lines.append(line)
            self.skip_blank = False

        # Detect Series-xxx tags
        elif series_tag_match:
            name = series_tag_match.group(1)
            value = series_tag_match.group(2)
            if name == 'changes':
                # value is the version number: e.g. 1, or 2
                self.in_change = 'Series'
                self.change_version = self._parse_version(value, line)
            else:
                self._add_to_series(line, name, value)
                self.skip_blank = True

        # Detect Change-Id tags
        elif change_id_match:
            if self.keep_change_id:
                out = [line]
            value = change_id_match.group(1)
            if self.is_log:
                if self.commit.change_id:
                    raise ValueError(
                        "%s: Two Change-Ids: '%s' vs. '%s'" %
                        (self.commit.hash, self.commit.change_id, value))
                self.commit.change_id = value
            self.skip_blank = True

        # Detect Commit-xxx tags
        elif commit_tag_match:
            name = commit_tag_match.group(1)
            value = commit_tag_match.group(2)
            if name == 'notes':
                self._add_to_commit(name)
                self.skip_blank = True
            elif name == 'changes':
                self.in_change = 'Commit'
                self.change_version = self._parse_version(value, line)
            else:
                self._add_warn('Line %d: Ignoring Commit-%s' %
                               (self.linenum, name))

        # Detect invalid tags
        elif invalid_match:
            raise ValueError("Line %d: Invalid tag = '%s'" %
                (self.linenum, line))

        # Detect the start of a new commit
        elif commit_match:
            self._close_commit()
            self.commit = commit.Commit(commit_match.group(1))

        # Detect tags in the commit message
        elif tag_match:
            rtag_type, who = tag_match.groups()
            self._add_commit_rtag(rtag_type, who)
            # Remove Tested-by self, since few will take much notice
            if (rtag_type == 'Tested-by' and
                    who.find(os.getenv('USER') + '@') != -1):
                self._add_warn("Ignoring '%s'" % line)
            elif rtag_type == 'Patch-cc':
                self.commit.add_cc(who.split(','))
            else:
                out = [line]

        # Suppress duplicate signoffs
        elif signoff_match:
            if (self.is_log or not self.commit or
                    self.commit.check_duplicate_signoff(signoff_match.group(1))):
                out = [line]

        # Well that means this is an ordinary line
        else:
            # Look for space before tab
            mat = RE_SPACE_BEFORE_TAB.match(line)
            if mat:
                self._add_warn('Line %d/%d has space before tab' %
                               (self.linenum, mat.start()))

            # OK, we have a valid non-blank line
            out = [line]
            self.linenum += 1
            self.skip_blank = False

            if diff_match:
                self.cur_diff = diff_match.group(1)

            # If this is quoted, keep recent lines
            if not diff_match and self.linenum > 1 and line:
                if line.startswith('>'):
                    if not self.was_quoted:
                        self._finalise_snippet()
                        self.recent_line = None
                    if not line_match:
                        self.recent_quoted.append(line)
                    self.was_quoted = True
                    self.recent_diff = self.cur_diff
                else:
                    self.recent_unquoted.put(line)
                    self.was_quoted = False

            if line_match:
                self.recent_line = line_match.groups()

            if self.state == STATE_DIFFS:
                pass

            # If this is the start of the diffs section, emit our tags and
            # change log
            elif line == '---':
                self.state = STATE_DIFFS

                # Output the tags (signoff first), then change list
                out = []
                log = self.series.MakeChangeLog(self.commit)
                out += [line]
                if self.commit:
                    out += self.commit.notes
                out += [''] + log
            elif self.found_test:
                if not RE_ALLOWED_AFTER_TEST.match(line):
                    self.lines_after_test += 1

        return out

    def finalise(self):
        """Close out processing of this patch stream"""
        self._finalise_snippet()
        self._finalise_change()
        self._close_commit()
        if self.lines_after_test:
            self._add_warn('Found %d lines after TEST=' % self.lines_after_test)

    def _write_message_id(self, outfd):
        """Write the Message-Id into the output.

        This is based on the Change-Id in the original patch, the version,
        and the prefix.

        Args:
            outfd (io.IOBase): Output stream file object
        """
        if not self.commit.change_id:
            return

        # If the count is -1 we're testing, so use a fixed time
        if self.commit.count == -1:
            time_now = datetime.datetime(1999, 12, 31, 23, 59, 59)
        else:
            time_now = datetime.datetime.now()

        # In theory there is email.utils.make_msgid() which would be nice
        # to use, but it already produces something way too long and thus
        # will produce ugly commit lines if someone throws this into
        # a "Link:" tag in the final commit.  So (sigh) roll our own.

        # Start with the time; presumably we wouldn't send the same series
        # with the same Change-Id at the exact same second.
        parts = [time_now.strftime("%Y%m%d%H%M%S")]

        # These seem like they would be nice to include.
        if 'prefix' in self.series:
            parts.append(self.series['prefix'])
        if 'postfix' in self.series:
            parts.append(self.series['postfix'])
        if 'version' in self.series:
            parts.append("v%s" % self.series['version'])

        parts.append(str(self.commit.count + 1))

        # The Change-Id must be last, right before the @
        parts.append(self.commit.change_id)

        # Join parts together with "." and write it out.
        outfd.write('Message-Id: <%s@changeid>\n' % '.'.join(parts))

    def process_stream(self, infd, outfd):
        """Copy a stream from infd to outfd, filtering out unwanting things.

        This is used to process patch files one at a time.

        Args:
            infd (io.IOBase): Input stream file object
            outfd (io.IOBase): Output stream file object
        """
        # Extract the filename from each diff, for nice warnings
        fname = None
        last_fname = None
        re_fname = re.compile('diff --git a/(.*) b/.*')

        self._write_message_id(outfd)

        while True:
            line = infd.readline()
            if not line:
                break
            out = self.process_line(line)

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
                        self._add_warn("Found possible blank line(s) at end of file '%s'" %
                                       last_fname)
                    outfd.write('+\n' * self.blank_count)
                    outfd.write(line + '\n')
                    self.blank_count = 0
        self.finalise()

def insert_tags(msg, tags_to_emit):
    """Add extra tags to a commit message

    The tags are added after an existing block of tags if found, otherwise at
    the end.

    Args:
        msg (str): Commit message
        tags_to_emit (list): List of tags to emit, each a str

    Returns:
        (str) new message
    """
    out = []
    done = False
    emit_tags = False
    emit_blank = False
    for line in msg.splitlines():
        if not done:
            signoff_match = RE_SIGNOFF.match(line)
            tag_match = RE_TAG.match(line)
            if tag_match or signoff_match:
                emit_tags = True
            if emit_tags and not tag_match and not signoff_match:
                out += tags_to_emit
                emit_tags = False
                done = True
            emit_blank = not (signoff_match or tag_match)
        else:
            emit_blank = line
        out.append(line)
    if not done:
        if emit_blank:
            out.append('')
        out += tags_to_emit
    return '\n'.join(out)

def get_list(commit_range, git_dir=None, count=None):
    """Get a log of a list of comments

    This returns the output of 'git log' for the selected commits

    Args:
        commit_range (str): Range of commits to count (e.g. 'HEAD..base')
        git_dir (str): Path to git repositiory (None to use default)
        count (int): Number of commits to list, or None for no limit

    Returns
        str: String containing the contents of the git log
    """
    params = gitutil.log_cmd(commit_range, reverse=True, count=count,
                            git_dir=git_dir)
    return command.run_pipe([params], capture=True).stdout

def get_metadata_for_list(commit_range, git_dir=None, count=None,
                          series=None, allow_overwrite=False):
    """Reads out patch series metadata from the commits

    This does a 'git log' on the relevant commits and pulls out the tags we
    are interested in.

    Args:
        commit_range (str): Range of commits to count (e.g. 'HEAD..base')
        git_dir (str): Path to git repositiory (None to use default)
        count (int): Number of commits to list, or None for no limit
        series (Series): Object to add information into. By default a new series
            is started.
        allow_overwrite (bool): Allow tags to overwrite an existing tag

    Returns:
        Series: Object containing information about the commits.
    """
    if not series:
        series = Series()
    series.allow_overwrite = allow_overwrite
    stdout = get_list(commit_range, git_dir, count)
    pst = PatchStream(series, is_log=True)
    for line in stdout.splitlines():
        pst.process_line(line)
    pst.finalise()
    return series

def get_metadata(branch, start, count):
    """Reads out patch series metadata from the commits

    This does a 'git log' on the relevant commits and pulls out the tags we
    are interested in.

    Args:
        branch (str): Branch to use (None for current branch)
        start (int): Commit to start from: 0=branch HEAD, 1=next one, etc.
        count (int): Number of commits to list

    Returns:
        Series: Object containing information about the commits.
    """
    return get_metadata_for_list(
        '%s~%d' % (branch if branch else 'HEAD', start), None, count)

def get_metadata_for_test(text):
    """Process metadata from a file containing a git log. Used for tests

    Args:
        text:

    Returns:
        Series: Object containing information about the commits.
    """
    series = Series()
    pst = PatchStream(series, is_log=True)
    for line in text.splitlines():
        pst.process_line(line)
    pst.finalise()
    return series

def fix_patch(backup_dir, fname, series, cmt, keep_change_id=False):
    """Fix up a patch file, by adding/removing as required.

    We remove our tags from the patch file, insert changes lists, etc.
    The patch file is processed in place, and overwritten.

    A backup file is put into backup_dir (if not None).

    Args:
        backup_dir (str): Path to directory to use to backup the file
        fname (str): Filename to patch file to process
        series (Series): Series information about this patch set
        cmt (Commit): Commit object for this patch file
        keep_change_id (bool): Keep the Change-Id tag.

    Return:
        list: A list of errors, each str, or [] if all ok.
    """
    handle, tmpname = tempfile.mkstemp()
    outfd = os.fdopen(handle, 'w', encoding='utf-8')
    infd = open(fname, 'r', encoding='utf-8')
    pst = PatchStream(series, keep_change_id=keep_change_id)
    pst.commit = cmt
    pst.process_stream(infd, outfd)
    infd.close()
    outfd.close()

    # Create a backup file if required
    if backup_dir:
        shutil.copy(fname, os.path.join(backup_dir, os.path.basename(fname)))
    shutil.move(tmpname, fname)
    return cmt.warn

def fix_patches(series, fnames, keep_change_id=False):
    """Fix up a list of patches identified by filenames

    The patch files are processed in place, and overwritten.

    Args:
        series (Series): The Series object
        fnames (:type: list of str): List of patch files to process
        keep_change_id (bool): Keep the Change-Id tag.
    """
    # Current workflow creates patches, so we shouldn't need a backup
    backup_dir = None  #tempfile.mkdtemp('clean-patch')
    count = 0
    for fname in fnames:
        cmt = series.commits[count]
        cmt.patch = fname
        cmt.count = count
        result = fix_patch(backup_dir, fname, series, cmt,
                           keep_change_id=keep_change_id)
        if result:
            print('%d warning%s for %s:' %
                  (len(result), 's' if len(result) > 1 else '', fname))
            for warn in result:
                print('\t%s' % warn)
            print()
        count += 1
    print('Cleaned %d patch%s' % (count, 'es' if count > 1 else ''))

def insert_cover_letter(fname, series, count):
    """Inserts a cover letter with the required info into patch 0

    Args:
        fname (str): Input / output filename of the cover letter file
        series (Series): Series object
        count (int): Number of patches in the series
    """
    fil = open(fname, 'r')
    lines = fil.readlines()
    fil.close()

    fil = open(fname, 'w')
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
        fil.write(line)
    fil.close()
