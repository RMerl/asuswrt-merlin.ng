# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2020 Google LLC
#
"""Talks to the patchwork service to figure out what patches have been reviewed
and commented on. Provides a way to display review tags and comments.
Allows creation of a new branch based on the old but with the review tags
collected from patchwork.
"""

import collections
import concurrent.futures
from itertools import repeat
import re

import pygit2
import requests

from patman import patchstream
from patman.patchstream import PatchStream
from u_boot_pylib import terminal
from u_boot_pylib import tout

# Patches which are part of a multi-patch series are shown with a prefix like
# [prefix, version, sequence], for example '[RFC, v2, 3/5]'. All but the last
# part is optional. This decodes the string into groups. For single patches
# the [] part is not present:
# Groups: (ignore, ignore, ignore, prefix, version, sequence, subject)
RE_PATCH = re.compile(r'(\[(((.*),)?(.*),)?(.*)\]\s)?(.*)$')

# This decodes the sequence string into a patch number and patch count
RE_SEQ = re.compile(r'(\d+)/(\d+)')

def to_int(vals):
    """Convert a list of strings into integers, using 0 if not an integer

    Args:
        vals (list): List of strings

    Returns:
        list: List of integers, one for each input string
    """
    out = [int(val) if val.isdigit() else 0 for val in vals]
    return out


class Patch(dict):
    """Models a patch in patchwork

    This class records information obtained from patchwork

    Some of this information comes from the 'Patch' column:

        [RFC,v2,1/3] dm: Driver and uclass changes for tiny-dm

    This shows the prefix, version, seq, count and subject.

    The other properties come from other columns in the display.

    Properties:
        pid (str): ID of the patch (typically an integer)
        seq (int): Sequence number within series (1=first) parsed from sequence
            string
        count (int): Number of patches in series, parsed from sequence string
        raw_subject (str): Entire subject line, e.g.
            "[1/2,v2] efi_loader: Sort header file ordering"
        prefix (str): Prefix string or None (e.g. 'RFC')
        version (str): Version string or None (e.g. 'v2')
        raw_subject (str): Raw patch subject
        subject (str): Patch subject with [..] part removed (same as commit
            subject)
    """
    def __init__(self, pid):
        super().__init__()
        self.id = pid  # Use 'id' to match what the Rest API provides
        self.seq = None
        self.count = None
        self.prefix = None
        self.version = None
        self.raw_subject = None
        self.subject = None

    # These make us more like a dictionary
    def __setattr__(self, name, value):
        self[name] = value

    def __getattr__(self, name):
        return self[name]

    def __hash__(self):
        return hash(frozenset(self.items()))

    def __str__(self):
        return self.raw_subject

    def parse_subject(self, raw_subject):
        """Parse the subject of a patch into its component parts

        See RE_PATCH for details. The parsed info is placed into seq, count,
        prefix, version, subject

        Args:
            raw_subject (str): Subject string to parse

        Raises:
            ValueError: the subject cannot be parsed
        """
        self.raw_subject = raw_subject.strip()
        mat = RE_PATCH.search(raw_subject.strip())
        if not mat:
            raise ValueError("Cannot parse subject '%s'" % raw_subject)
        self.prefix, self.version, seq_info, self.subject = mat.groups()[3:]
        mat_seq = RE_SEQ.match(seq_info) if seq_info else False
        if mat_seq is None:
            self.version = seq_info
            seq_info = None
        if self.version and not self.version.startswith('v'):
            self.prefix = self.version
            self.version = None
        if seq_info:
            if mat_seq:
                self.seq = int(mat_seq.group(1))
                self.count = int(mat_seq.group(2))
        else:
            self.seq = 1
            self.count = 1


class Review:
    """Represents a single review email collected in Patchwork

    Patches can attract multiple reviews. Each consists of an author/date and
    a variable number of 'snippets', which are groups of quoted and unquoted
    text.
    """
    def __init__(self, meta, snippets):
        """Create new Review object

        Args:
            meta (str): Text containing review author and date
            snippets (list): List of snippets in th review, each a list of text
                lines
        """
        self.meta = ' : '.join([line for line in meta.splitlines() if line])
        self.snippets = snippets

def compare_with_series(series, patches):
    """Compare a list of patches with a series it came from

    This prints any problems as warnings

    Args:
        series (Series): Series to compare against
        patches (:type: list of Patch): list of Patch objects to compare with

    Returns:
        tuple
            dict:
                key: Commit number (0...n-1)
                value: Patch object for that commit
            dict:
                key: Patch number  (0...n-1)
                value: Commit object for that patch
    """
    # Check the names match
    warnings = []
    patch_for_commit = {}
    all_patches = set(patches)
    for seq, cmt in enumerate(series.commits):
        pmatch = [p for p in all_patches if p.subject == cmt.subject]
        if len(pmatch) == 1:
            patch_for_commit[seq] = pmatch[0]
            all_patches.remove(pmatch[0])
        elif len(pmatch) > 1:
            warnings.append("Multiple patches match commit %d ('%s'):\n   %s" %
                            (seq + 1, cmt.subject,
                             '\n   '.join([p.subject for p in pmatch])))
        else:
            warnings.append("Cannot find patch for commit %d ('%s')" %
                            (seq + 1, cmt.subject))


    # Check the names match
    commit_for_patch = {}
    all_commits = set(series.commits)
    for seq, patch in enumerate(patches):
        cmatch = [c for c in all_commits if c.subject == patch.subject]
        if len(cmatch) == 1:
            commit_for_patch[seq] = cmatch[0]
            all_commits.remove(cmatch[0])
        elif len(cmatch) > 1:
            warnings.append("Multiple commits match patch %d ('%s'):\n   %s" %
                            (seq + 1, patch.subject,
                             '\n   '.join([c.subject for c in cmatch])))
        else:
            warnings.append("Cannot find commit for patch %d ('%s')" %
                            (seq + 1, patch.subject))

    return patch_for_commit, commit_for_patch, warnings

def call_rest_api(url, subpath):
    """Call the patchwork API and return the result as JSON

    Args:
        url (str): URL of patchwork server, e.g. 'https://patchwork.ozlabs.org'
        subpath (str): URL subpath to use

    Returns:
        dict: Json result

    Raises:
        ValueError: the URL could not be read
    """
    full_url = '%s/api/1.2/%s' % (url, subpath)
    response = requests.get(full_url)
    if response.status_code != 200:
        raise ValueError("Could not read URL '%s'" % full_url)
    return response.json()

def collect_patches(series, series_id, url, rest_api=call_rest_api):
    """Collect patch information about a series from patchwork

    Uses the Patchwork REST API to collect information provided by patchwork
    about the status of each patch.

    Args:
        series (Series): Series object corresponding to the local branch
            containing the series
        series_id (str): Patch series ID number
        url (str): URL of patchwork server, e.g. 'https://patchwork.ozlabs.org'
        rest_api (function): API function to call to access Patchwork, for
            testing

    Returns:
        list: List of patches sorted by sequence number, each a Patch object

    Raises:
        ValueError: if the URL could not be read or the web page does not follow
            the expected structure
    """
    data = rest_api(url, 'series/%s/' % series_id)

    # Get all the rows, which are patches
    patch_dict = data['patches']
    count = len(patch_dict)
    num_commits = len(series.commits)
    if count != num_commits:
        tout.warning('Warning: Patchwork reports %d patches, series has %d' %
                     (count, num_commits))

    patches = []

    # Work through each row (patch) one at a time, collecting the information
    warn_count = 0
    for pw_patch in patch_dict:
        patch = Patch(pw_patch['id'])
        patch.parse_subject(pw_patch['name'])
        patches.append(patch)
    if warn_count > 1:
        tout.warning('   (total of %d warnings)' % warn_count)

    # Sort patches by patch number
    patches = sorted(patches, key=lambda x: x.seq)
    return patches

def find_new_responses(new_rtag_list, review_list, seq, cmt, patch, url,
                       rest_api=call_rest_api):
    """Find new rtags collected by patchwork that we don't know about

    This is designed to be run in parallel, once for each commit/patch

    Args:
        new_rtag_list (list): New rtags are written to new_rtag_list[seq]
            list, each a dict:
                key: Response tag (e.g. 'Reviewed-by')
                value: Set of people who gave that response, each a name/email
                    string
        review_list (list): New reviews are written to review_list[seq]
            list, each a
                List of reviews for the patch, each a Review
        seq (int): Position in new_rtag_list to update
        cmt (Commit): Commit object for this commit
        patch (Patch): Corresponding Patch object for this patch
        url (str): URL of patchwork server, e.g. 'https://patchwork.ozlabs.org'
        rest_api (function): API function to call to access Patchwork, for
            testing
    """
    if not patch:
        return

    # Get the content for the patch email itself as well as all comments
    data = rest_api(url, 'patches/%s/' % patch.id)
    pstrm = PatchStream.process_text(data['content'], True)

    rtags = collections.defaultdict(set)
    for response, people in pstrm.commit.rtags.items():
        rtags[response].update(people)

    data = rest_api(url, 'patches/%s/comments/' % patch.id)

    reviews = []
    for comment in data:
        pstrm = PatchStream.process_text(comment['content'], True)
        if pstrm.snippets:
            submitter = comment['submitter']
            person = '%s <%s>' % (submitter['name'], submitter['email'])
            reviews.append(Review(person, pstrm.snippets))
        for response, people in pstrm.commit.rtags.items():
            rtags[response].update(people)

    # Find the tags that are not in the commit
    new_rtags = collections.defaultdict(set)
    base_rtags = cmt.rtags
    for tag, people in rtags.items():
        for who in people:
            is_new = (tag not in base_rtags or
                      who not in base_rtags[tag])
            if is_new:
                new_rtags[tag].add(who)
    new_rtag_list[seq] = new_rtags
    review_list[seq] = reviews

def show_responses(rtags, indent, is_new):
    """Show rtags collected

    Args:
        rtags (dict): review tags to show
            key: Response tag (e.g. 'Reviewed-by')
            value: Set of people who gave that response, each a name/email string
        indent (str): Indentation string to write before each line
        is_new (bool): True if this output should be highlighted

    Returns:
        int: Number of review tags displayed
    """
    col = terminal.Color()
    count = 0
    for tag in sorted(rtags.keys()):
        people = rtags[tag]
        for who in sorted(people):
            terminal.tprint(indent + '%s %s: ' % ('+' if is_new else ' ', tag),
                           newline=False, colour=col.GREEN, bright=is_new)
            terminal.tprint(who, colour=col.WHITE, bright=is_new)
            count += 1
    return count

def create_branch(series, new_rtag_list, branch, dest_branch, overwrite,
                  repo=None):
    """Create a new branch with review tags added

    Args:
        series (Series): Series object for the existing branch
        new_rtag_list (list): List of review tags to add, one for each commit,
                each a dict:
            key: Response tag (e.g. 'Reviewed-by')
            value: Set of people who gave that response, each a name/email
                string
        branch (str): Existing branch to update
        dest_branch (str): Name of new branch to create
        overwrite (bool): True to force overwriting dest_branch if it exists
        repo (pygit2.Repository): Repo to use (use None unless testing)

    Returns:
        int: Total number of review tags added across all commits

    Raises:
        ValueError: if the destination branch name is the same as the original
            branch, or it already exists and @overwrite is False
    """
    if branch == dest_branch:
        raise ValueError(
            'Destination branch must not be the same as the original branch')
    if not repo:
        repo = pygit2.Repository('.')
    count = len(series.commits)
    new_br = repo.branches.get(dest_branch)
    if new_br:
        if not overwrite:
            raise ValueError("Branch '%s' already exists (-f to overwrite)" %
                             dest_branch)
        new_br.delete()
    if not branch:
        branch = 'HEAD'
    target = repo.revparse_single('%s~%d' % (branch, count))
    repo.branches.local.create(dest_branch, target)

    num_added = 0
    for seq in range(count):
        parent = repo.branches.get(dest_branch)
        cherry = repo.revparse_single('%s~%d' % (branch, count - seq - 1))

        repo.merge_base(cherry.oid, parent.target)
        base_tree = cherry.parents[0].tree

        index = repo.merge_trees(base_tree, parent, cherry)
        tree_id = index.write_tree(repo)

        lines = []
        if new_rtag_list[seq]:
            for tag, people in new_rtag_list[seq].items():
                for who in people:
                    lines.append('%s: %s' % (tag, who))
                    num_added += 1
        message = patchstream.insert_tags(cherry.message.rstrip(),
                                          sorted(lines))

        repo.create_commit(
            parent.name, cherry.author, cherry.committer, message, tree_id,
            [parent.target])
    return num_added

def check_patchwork_status(series, series_id, branch, dest_branch, force,
                           show_comments, url, rest_api=call_rest_api,
                           test_repo=None):
    """Check the status of a series on Patchwork

    This finds review tags and comments for a series in Patchwork, displaying
    them to show what is new compared to the local series.

    Args:
        series (Series): Series object for the existing branch
        series_id (str): Patch series ID number
        branch (str): Existing branch to update, or None
        dest_branch (str): Name of new branch to create, or None
        force (bool): True to force overwriting dest_branch if it exists
        show_comments (bool): True to show the comments on each patch
        url (str): URL of patchwork server, e.g. 'https://patchwork.ozlabs.org'
        rest_api (function): API function to call to access Patchwork, for
            testing
        test_repo (pygit2.Repository): Repo to use (use None unless testing)
    """
    patches = collect_patches(series, series_id, url, rest_api)
    col = terminal.Color()
    count = len(series.commits)
    new_rtag_list = [None] * count
    review_list = [None] * count

    patch_for_commit, _, warnings = compare_with_series(series, patches)
    for warn in warnings:
        tout.warning(warn)

    patch_list = [patch_for_commit.get(c) for c in range(len(series.commits))]

    with concurrent.futures.ThreadPoolExecutor(max_workers=16) as executor:
        futures = executor.map(
            find_new_responses, repeat(new_rtag_list), repeat(review_list),
            range(count), series.commits, patch_list, repeat(url),
            repeat(rest_api))
    for fresponse in futures:
        if fresponse:
            raise fresponse.exception()

    num_to_add = 0
    for seq, cmt in enumerate(series.commits):
        patch = patch_for_commit.get(seq)
        if not patch:
            continue
        terminal.tprint('%3d %s' % (patch.seq, patch.subject[:50]),
                       colour=col.BLUE)
        cmt = series.commits[seq]
        base_rtags = cmt.rtags
        new_rtags = new_rtag_list[seq]

        indent = ' ' * 2
        show_responses(base_rtags, indent, False)
        num_to_add += show_responses(new_rtags, indent, True)
        if show_comments:
            for review in review_list[seq]:
                terminal.tprint('Review: %s' % review.meta, colour=col.RED)
                for snippet in review.snippets:
                    for line in snippet:
                        quoted = line.startswith('>')
                        terminal.tprint('    %s' % line,
                                       colour=col.MAGENTA if quoted else None)
                    terminal.tprint()

    terminal.tprint("%d new response%s available in patchwork%s" %
                   (num_to_add, 's' if num_to_add != 1 else '',
                    '' if dest_branch
                    else ' (use -d to write them to a new branch)'))

    if dest_branch:
        num_added = create_branch(series, new_rtag_list, branch,
                                  dest_branch, force, test_repo)
        terminal.tprint(
            "%d response%s added from patchwork into new branch '%s'" %
            (num_added, 's' if num_added != 1 else '', dest_branch))
