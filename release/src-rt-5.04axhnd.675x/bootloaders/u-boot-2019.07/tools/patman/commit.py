# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2011 The Chromium OS Authors.
#

import re

# Separates a tag: at the beginning of the subject from the rest of it
re_subject_tag = re.compile('([^:\s]*):\s*(.*)')

class Commit:
    """Holds information about a single commit/patch in the series.

    Args:
        hash: Commit hash (as a string)

    Variables:
        hash: Commit hash
        subject: Subject line
        tags: List of maintainer tag strings
        changes: Dict containing a list of changes (single line strings).
            The dict is indexed by change version (an integer)
        cc_list: List of people to aliases/emails to cc on this commit
        notes: List of lines in the commit (not series) notes
    """
    def __init__(self, hash):
        self.hash = hash
        self.subject = None
        self.tags = []
        self.changes = {}
        self.cc_list = []
        self.signoff_set = set()
        self.notes = []

    def AddChange(self, version, info):
        """Add a new change line to the change list for a version.

        Args:
            version: Patch set version (integer: 1, 2, 3)
            info: Description of change in this version
        """
        if not self.changes.get(version):
            self.changes[version] = []
        self.changes[version].append(info)

    def CheckTags(self):
        """Create a list of subject tags in the commit

        Subject tags look like this:

            propounder: fort: Change the widget to propound correctly

        Here the tags are propounder and fort. Multiple tags are supported.
        The list is updated in self.tag.

        Returns:
            None if ok, else the name of a tag with no email alias
        """
        str = self.subject
        m = True
        while m:
            m = re_subject_tag.match(str)
            if m:
                tag = m.group(1)
                self.tags.append(tag)
                str = m.group(2)
        return None

    def AddCc(self, cc_list):
        """Add a list of people to Cc when we send this patch.

        Args:
            cc_list:    List of aliases or email addresses
        """
        self.cc_list += cc_list

    def CheckDuplicateSignoff(self, signoff):
        """Check a list of signoffs we have send for this patch

        Args:
            signoff:    Signoff line
        Returns:
            True if this signoff is new, False if we have already seen it.
        """
        if signoff in self.signoff_set:
          return False
        self.signoff_set.add(signoff)
        return True
