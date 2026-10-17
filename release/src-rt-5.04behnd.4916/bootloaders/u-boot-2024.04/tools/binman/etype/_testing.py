# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for testing purposes. Not used in real images.
#

from collections import OrderedDict

from binman.entry import Entry, EntryArg
from dtoc import fdt_util
from u_boot_pylib import tools


class Entry__testing(Entry):
    """A fake entry used for testing

    This entry should not be used in normal images. It is a special entry with
    strange features used for testing.

    Properties / Entry arguments
        test-str-fdt: Test string, normally in the node
        test-int-fdt: Test integer, normally in the node
        test-str-arg: Test string, normally in the entry arguments
        test-int-arg: Test integer, normally in the entry arguments

    The entry has a single 'a' byte as its contents. Operation is controlled by
    a number of properties in the node, as follows:

    Properties:
        return-invalid-entry: Return an invalid entry from GetOffsets()
        return-unknown-contents: Refuse to provide any contents (to cause a
            failure)
        bad-update-contents: Return a larger size in ProcessContents
        bad-shrink-contents: Return a larger size in ProcessContents
        never-complete-process-fdt: Refund to process the FDT (to cause a
            failure)
        require-args: Require that all used args are present (generating an
            error if not)
        force-bad-datatype: Force a call to GetEntryArgsOrProps() with a bad
            data type (generating an error)
        require-bintool-for-contents: Raise an error if the specified
            bintool isn't usable in ObtainContents()
        require-bintool-for-pack: Raise an error if the specified
            bintool isn't usable in Pack()
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)

    def ReadNode(self):
        super().ReadNode()
        self.return_invalid_entry = fdt_util.GetBool(self._node,
                                                     'return-invalid-entry')
        self.return_unknown_contents = fdt_util.GetBool(self._node,
                                                     'return-unknown-contents')
        self.bad_update_contents = fdt_util.GetBool(self._node,
                                                    'bad-update-contents')
        self.bad_shrink_contents = fdt_util.GetBool(self._node,
                                                    'bad-shrink-contents')
        self.return_contents_once = fdt_util.GetBool(self._node,
                                                     'return-contents-once')
        self.bad_update_contents_twice = fdt_util.GetBool(self._node,
                                                    'bad-update-contents-twice')
        self.return_contents_later = fdt_util.GetBool(self._node,
                                                     'return-contents-later')
        self.set_to_absent = fdt_util.GetBool(self._node, 'set-to-absent')

        # Set to True when the entry is ready to process the FDT.
        self.process_fdt_ready = False
        self.never_complete_process_fdt = fdt_util.GetBool(self._node,
                                                'never-complete-process-fdt')
        self.require_args = fdt_util.GetBool(self._node, 'require-args')

        # This should be picked up by GetEntryArgsOrProps()
        self.test_existing_prop = 'existing'
        self.force_bad_datatype = fdt_util.GetBool(self._node,
                                                   'force-bad-datatype')
        (self.test_str_fdt, self.test_str_arg, self.test_int_fdt,
         self.test_int_arg, existing) = self.GetEntryArgsOrProps([
            EntryArg('test-str-fdt', str),
            EntryArg('test-str-arg', str),
            EntryArg('test-int-fdt', int),
            EntryArg('test-int-arg', int),
            EntryArg('test-existing-prop', str)], self.require_args)
        if self.force_bad_datatype:
            self.GetEntryArgsOrProps([EntryArg('test-bad-datatype-arg', bool)])
        self.return_contents = True
        self.contents = b'aa'

        # Set to the required bintool when collecting bintools.
        self.bintool_for_contents = None
        self.require_bintool_for_contents = fdt_util.GetString(self._node,
                                              'require-bintool-for-contents')
        if self.require_bintool_for_contents == '':
            self.require_bintool_for_contents = '_testing'

        self.bintool_for_pack = None
        self.require_bintool_for_pack = fdt_util.GetString(self._node,
                                                  'require-bintool-for-pack')
        if self.require_bintool_for_pack == '':
            self.require_bintool_for_pack = '_testing'

    def Pack(self, offset):
        """Figure out how to pack the entry into the section"""
        if self.require_bintool_for_pack:
            if self.bintool_for_pack is None:
                self.Raise("Required bintool unusable in Pack()")
        return super().Pack(offset)

    def ObtainContents(self, fake_size=0):
        if self.return_unknown_contents or not self.return_contents:
            return False
        if self.return_contents_later:
            self.return_contents_later = False
            return False
        self.data = self.contents
        self.contents_size = len(self.data)
        if self.return_contents_once:
            self.return_contents = False
        if self.require_bintool_for_contents:
            if self.bintool_for_contents is None:
                self.Raise("Required bintool unusable in ObtainContents()")
        if self.set_to_absent:
            self.mark_absent('for testing purposes')
        return True

    def GetOffsets(self):
        if self.return_invalid_entry :
            return {'invalid-entry': [1, 2]}
        return {}

    def ProcessContents(self):
        data = self.contents
        if self.bad_update_contents:
            # Request to update the contents with something larger, to cause a
            # failure.
            if self.bad_update_contents_twice:
                data = self.data + b'a'
            else:
                data = b'aaa'
            return self.ProcessContentsUpdate(data)
        if self.bad_shrink_contents:
            # Request to update the contents with something smaller, to cause a
            # failure.
            data = b'a'
            return self.ProcessContentsUpdate(data)
        if self.bad_shrink_contents:
            # Request to update the contents with something smaller, to cause a
            # failure.
            data = b'a'
            return self.ProcessContentsUpdate(data)
        return True

    def ProcessFdt(self, fdt):
        """Force reprocessing the first time"""
        ready = self.process_fdt_ready
        if not self.never_complete_process_fdt:
            self.process_fdt_ready = True
        return ready

    def AddBintools(self, btools):
        """Add the bintools used by this entry type"""
        if self.require_bintool_for_contents is not None:
            self.bintool_for_contents = self.AddBintool(btools,
                    self.require_bintool_for_contents)
        if self.require_bintool_for_pack is not None:
            self.bintool_for_pack = self.AddBintool(btools,
                    self.require_bintool_for_pack)
