# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for a Chromium OS EC image (read-write section)
#

from blob_named_by_arg import Entry_blob_named_by_arg


class Entry_cros_ec_rw(Entry_blob_named_by_arg):
    """A blob entry which contains a Chromium OS read-write EC image

    Properties / Entry arguments:
        - cros-ec-rw-path: Filename containing the EC image

    This entry holds a Chromium OS EC (embedded controller) image, for use in
    updating the EC on startup via software sync.
    """
    def __init__(self, section, etype, node):
        Entry_blob_named_by_arg.__init__(self, section, etype, node,
                                         'cros-ec-rw')
