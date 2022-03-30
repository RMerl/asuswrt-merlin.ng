# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for testing purposes. Not used in real images.
#

from collections import OrderedDict

from entry import Entry, EntryArg
import fdt_util
import tools


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
        bad-update-contents: Implement ProcessContents() incorrectly so as to
            cause a failure
        never-complete-process-fdt: Refund to process the FDT (to cause a
            failure)
        require-args: Require that all used args are present (generating an
            error if not)
        force-bad-datatype: Force a call to GetEntryArgsOrProps() with a bad
            data type (generating an error)
    """
    def __init__(self, section, etype, node):
        Entry.__init__(self, section, etype, node)
        self.return_invalid_entry = fdt_util.GetBool(self._node,
                                                     'return-invalid-entry')
        self.return_unknown_contents = fdt_util.GetBool(self._node,
                                                     'return-unknown-contents')
        self.bad_update_contents = fdt_util.GetBool(self._node,
                                                    'bad-update-contents')
        self.return_contents_once = fdt_util.GetBool(self._node,
                                                     'return-contents-once')

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

    def ObtainContents(self):
        if self.return_unknown_contents or not self.return_contents:
            return False
        self.data = 'a'
        self.contents_size = len(self.data)
        if self.return_contents_once:
            self.return_contents = False
        return True

    def GetOffsets(self):
        if self.return_invalid_entry :
            return {'invalid-entry': [1, 2]}
        return {}

    def ProcessContents(self):
        if self.bad_update_contents:
            # Request to update the conents with something larger, to cause a
            # failure.
            self.ProcessContentsUpdate('aa')

    def ProcessFdt(self, fdt):
        """Force reprocessing the first time"""
        ready = self.process_fdt_ready
        if not self.never_complete_process_fdt:
            self.process_fdt_ready = True
        return ready
