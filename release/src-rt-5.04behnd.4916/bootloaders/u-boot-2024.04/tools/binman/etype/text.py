# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#

from collections import OrderedDict

from binman.entry import Entry, EntryArg
from dtoc import fdt_util
from u_boot_pylib import tools


class Entry_text(Entry):
    """An entry which contains text

    The text can be provided either in the node itself or by a command-line
    argument. There is a level of indirection to allow multiple text strings
    and sharing of text.

    Properties / Entry arguments:
        text-label: The value of this string indicates the property / entry-arg
            that contains the string to place in the entry
        <xxx> (actual name is the value of text-label): contains the string to
            place in the entry.
        <text>: The text to place in the entry (overrides the above mechanism).
            This is useful when the text is constant.

    Example node::

        text {
            size = <50>;
            text-label = "message";
        };

    You can then use:

        binman -amessage="this is my message"

    and binman will insert that string into the entry.

    It is also possible to put the string directly in the node::

        text {
            size = <8>;
            text-label = "message";
            message = "a message directly in the node"
        };

    or just::

        text {
            size = <8>;
            text = "some text directly in the node"
        };

    The text is not itself nul-terminated. This can be achieved, if required,
    by setting the size of the entry to something larger than the text.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        value = fdt_util.GetString(self._node, 'text')
        if value:
            value = tools.to_bytes(value)
        else:
            label, = self.GetEntryArgsOrProps([EntryArg('text-label', str)])
            self.text_label = label
            if self.text_label:
                value, = self.GetEntryArgsOrProps([EntryArg(self.text_label,
                                                            str)])
                value = tools.to_bytes(value) if value is not None else value
        self.value = value

    def ObtainContents(self):
        if not self.value:
            self.Raise("No value provided for text label '%s'" %
                       self.text_label)
        self.SetContents(self.value)
        return True
