# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for a blob where the filename comes from a property in the
# node or an entry argument. The property is called '<blob_fname>-path' where
# <blob_fname> is provided by the subclass using this entry type.

from collections import OrderedDict

from blob import Entry_blob
from entry import EntryArg


class Entry_blob_named_by_arg(Entry_blob):
    """A blob entry which gets its filename property from its subclass

    Properties / Entry arguments:
        - <xxx>-path: Filename containing the contents of this entry (optional,
            defaults to 0)

    where <xxx> is the blob_fname argument to the constructor.

    This entry cannot be used directly. Instead, it is used as a parent class
    for another entry, which defined blob_fname. This parameter is used to
    set the entry-arg or property containing the filename. The entry-arg or
    property is in turn used to set the actual filename.

    See cros_ec_rw for an example of this.
    """
    def __init__(self, section, etype, node, blob_fname):
        Entry_blob.__init__(self, section, etype, node)
        self._filename, = self.GetEntryArgsOrProps(
            [EntryArg('%s-path' % blob_fname, str)])
