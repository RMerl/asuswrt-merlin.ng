# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2011 The Chromium OS Authors.
#

from __future__ import print_function

import itertools
import os

import get_maintainer
import gitutil
import settings
import terminal

# Series-xxx tags that we understand
valid_series = ['to', 'cc', 'version', 'changes', 'prefix', 'notes', 'name',
                'cover_cc', 'process_log']

class Series(dict):
    """Holds information about a patch series, including all tags.

    Vars:
        cc: List of aliases/emails to Cc all patches to
        commits: List of Commit objects, one for each patch
        cover: List of lines in the cover letter
        notes: List of lines in the notes
        changes: (dict) List of changes for each version, The key is
            the integer version number
        allow_overwrite: Allow tags to overwrite an existing tag
    """
    def __init__(self):
        self.cc = []
        self.to = []
        self.cover_cc = []
        self.commits = []
        self.cover = None
        self.notes = []
        self.changes = {}
        self.allow_overwrite = False

        # Written in MakeCcFile()
        #  key: name of patch file
        #  value: list of email addresses
        self._generated_cc = {}

    # These make us more like a dictionary
    def __setattr__(self, name, value):
        self[name] = value

    def __getattr__(self, name):
        return self[name]

    def AddTag(self, commit, line, name, value):
        """Add a new Series-xxx tag along with its value.

        Args:
            line: Source line containing tag (useful for debug/error messages)
            name: Tag name (part after 'Series-')
            value: Tag value (part after 'Series-xxx: ')
        """
        # If we already have it, then add to our list
        name = name.replace('-', '_')
        if name in self and not self.allow_overwrite:
            values = value.split(',')
            values = [str.strip() for str in values]
            if type(self[name]) != type([]):
                raise ValueError("In %s: line '%s': Cannot add another value "
                        "'%s' to series '%s'" %
                            (commit.hash, line, values, self[name]))
            self[name] += values

        # Otherwise just set the value
        elif name in valid_series:
            if name=="notes":
                self[name] = [value]
            else:
                self[name] = value
        else:
            raise ValueError("In %s: line '%s': Unknown 'Series-%s': valid "
                        "options are %s" % (commit.hash, line, name,
                            ', '.join(valid_series)))

    def AddCommit(self, commit):
        """Add a commit into our list of commits

        We create a list of tags in the commit subject also.

        Args:
            commit: Commit object to add
        """
        commit.CheckTags()
        self.commits.append(commit)

    def ShowActions(self, args, cmd, process_tags):
        """Show what actions we will/would perform

        Args:
            args: List of patch files we created
            cmd: The git command we would have run
            process_tags: Process tags as if they were aliases
        """
        to_set = set(gitutil.BuildEmailList(self.to));
        cc_set = set(gitutil.BuildEmailList(self.cc));

        col = terminal.Color()
        print('Dry run, so not doing much. But I would do this:')
        print()
        print('Send a total of %d patch%s with %scover letter.' % (
                len(args), '' if len(args) == 1 else 'es',
                self.get('cover') and 'a ' or 'no '))

        # TODO: Colour the patches according to whether they passed checks
        for upto in range(len(args)):
            commit = self.commits[upto]
            print(col.Color(col.GREEN, '   %s' % args[upto]))
            cc_list = list(self._generated_cc[commit.patch])
            for email in set(cc_list) - to_set - cc_set:
                if email == None:
                    email = col.Color(col.YELLOW, "<alias '%s' not found>"
                            % tag)
                if email:
                    print('      Cc: ', email)
        print
        for item in to_set:
            print('To:\t ', item)
        for item in cc_set - to_set:
            print('Cc:\t ', item)
        print('Version: ', self.get('version'))
        print('Prefix:\t ', self.get('prefix'))
        if self.cover:
            print('Cover: %d lines' % len(self.cover))
            cover_cc = gitutil.BuildEmailList(self.get('cover_cc', ''))
            all_ccs = itertools.chain(cover_cc, *self._generated_cc.values())
            for email in set(all_ccs) - to_set - cc_set:
                    print('      Cc: ', email)
        if cmd:
            print('Git command: %s' % cmd)

    def MakeChangeLog(self, commit):
        """Create a list of changes for each version.

        Return:
            The change log as a list of strings, one per line

            Changes in v4:
            - Jog the dial back closer to the widget

            Changes in v3: None
            Changes in v2:
            - Fix the widget
            - Jog the dial

            etc.
        """
        final = []
        process_it = self.get('process_log', '').split(',')
        process_it = [item.strip() for item in process_it]
        need_blank = False
        for change in sorted(self.changes, reverse=True):
            out = []
            for this_commit, text in self.changes[change]:
                if commit and this_commit != commit:
                    continue
                if 'uniq' not in process_it or text not in out:
                    out.append(text)
            line = 'Changes in v%d:' % change
            have_changes = len(out) > 0
            if 'sort' in process_it:
                out = sorted(out)
            if have_changes:
                out.insert(0, line)
            else:
                out = [line + ' None']
            if need_blank:
                out.insert(0, '')
            final += out
            need_blank = have_changes
        if self.changes:
            final.append('')
        return final

    def DoChecks(self):
        """Check that each version has a change log

        Print an error if something is wrong.
        """
        col = terminal.Color()
        if self.get('version'):
            changes_copy = dict(self.changes)
            for version in range(1, int(self.version) + 1):
                if self.changes.get(version):
                    del changes_copy[version]
                else:
                    if version > 1:
                        str = 'Change log missing for v%d' % version
                        print(col.Color(col.RED, str))
            for version in changes_copy:
                str = 'Change log for unknown version v%d' % version
                print(col.Color(col.RED, str))
        elif self.changes:
            str = 'Change log exists, but no version is set'
            print(col.Color(col.RED, str))

    def MakeCcFile(self, process_tags, cover_fname, raise_on_error,
                   add_maintainers, limit):
        """Make a cc file for us to use for per-commit Cc automation

        Also stores in self._generated_cc to make ShowActions() faster.

        Args:
            process_tags: Process tags as if they were aliases
            cover_fname: If non-None the name of the cover letter.
            raise_on_error: True to raise an error when an alias fails to match,
                False to just print a message.
            add_maintainers: Either:
                True/False to call the get_maintainers to CC maintainers
                List of maintainers to include (for testing)
            limit: Limit the length of the Cc list
        Return:
            Filename of temp file created
        """
        col = terminal.Color()
        # Look for commit tags (of the form 'xxx:' at the start of the subject)
        fname = '/tmp/patman.%d' % os.getpid()
        fd = open(fname, 'w')
        all_ccs = []
        for commit in self.commits:
            cc = []
            if process_tags:
                cc += gitutil.BuildEmailList(commit.tags,
                                               raise_on_error=raise_on_error)
            cc += gitutil.BuildEmailList(commit.cc_list,
                                           raise_on_error=raise_on_error)
            if type(add_maintainers) == type(cc):
                cc += add_maintainers
            elif add_maintainers:
                cc += get_maintainer.GetMaintainer(commit.patch)
            for x in set(cc) & set(settings.bounces):
                print(col.Color(col.YELLOW, 'Skipping "%s"' % x))
            cc = set(cc) - set(settings.bounces)
            cc = [m.encode('utf-8') if type(m) != str else m for m in cc]
            if limit is not None:
                cc = cc[:limit]
            all_ccs += cc
            print(commit.patch, ', '.join(set(cc)), file=fd)
            self._generated_cc[commit.patch] = cc

        if cover_fname:
            cover_cc = gitutil.BuildEmailList(self.get('cover_cc', ''))
            cover_cc = [m.encode('utf-8') if type(m) != str else m
                        for m in cover_cc]
            cc_list = ', '.join([x.decode('utf-8')
                                 for x in set(cover_cc + all_ccs)])
            print(cover_fname, cc_list.encode('utf-8'), file=fd)

        fd.close()
        return fname

    def AddChange(self, version, commit, info):
        """Add a new change line to a version.

        This will later appear in the change log.

        Args:
            version: version number to add change list to
            info: change line for this version
        """
        if not self.changes.get(version):
            self.changes[version] = []
        self.changes[version].append([commit, info])

    def GetPatchPrefix(self):
        """Get the patch version string

        Return:
            Patch string, like 'RFC PATCH v5' or just 'PATCH'
        """
        git_prefix = gitutil.GetDefaultSubjectPrefix()
        if git_prefix:
            git_prefix = '%s][' % git_prefix
        else:
            git_prefix = ''

        version = ''
        if self.get('version'):
            version = ' v%s' % self['version']

        # Get patch name prefix
        prefix = ''
        if self.get('prefix'):
            prefix = '%s ' % self['prefix']
        return '%s%sPATCH%s' % (git_prefix, prefix, version)
