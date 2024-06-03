# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
#
# Base class for all entries
#

from __future__ import print_function

from collections import namedtuple

# importlib was introduced in Python 2.7 but there was a report of it not
# working in 2.7.12, so we work around this:
# http://lists.denx.de/pipermail/u-boot/2016-October/269729.html
try:
    import importlib
    have_importlib = True
except:
    have_importlib = False

import os
from sets import Set
import sys

import fdt_util
import state
import tools

modules = {}

our_path = os.path.dirname(os.path.realpath(__file__))


# An argument which can be passed to entries on the command line, in lieu of
# device-tree properties.
EntryArg = namedtuple('EntryArg', ['name', 'datatype'])


class Entry(object):
    """An Entry in the section

    An entry corresponds to a single node in the device-tree description
    of the section. Each entry ends up being a part of the final section.
    Entries can be placed either right next to each other, or with padding
    between them. The type of the entry determines the data that is in it.

    This class is not used by itself. All entry objects are subclasses of
    Entry.

    Attributes:
        section: Section object containing this entry
        node: The node that created this entry
        offset: Offset of entry within the section, None if not known yet (in
            which case it will be calculated by Pack())
        size: Entry size in bytes, None if not known
        contents_size: Size of contents in bytes, 0 by default
        align: Entry start offset alignment, or None
        align_size: Entry size alignment, or None
        align_end: Entry end offset alignment, or None
        pad_before: Number of pad bytes before the contents, 0 if none
        pad_after: Number of pad bytes after the contents, 0 if none
        data: Contents of entry (string of bytes)
    """
    def __init__(self, section, etype, node, read_node=True, name_prefix=''):
        self.section = section
        self.etype = etype
        self._node = node
        self.name = node and (name_prefix + node.name) or 'none'
        self.offset = None
        self.size = None
        self.data = None
        self.contents_size = 0
        self.align = None
        self.align_size = None
        self.align_end = None
        self.pad_before = 0
        self.pad_after = 0
        self.offset_unset = False
        self.image_pos = None
        self._expand_size = False
        if read_node:
            self.ReadNode()

    @staticmethod
    def Lookup(section, node_path, etype):
        """Look up the entry class for a node.

        Args:
            section:   Section object containing this node
            node_node: Path name of Node object containing information about
                       the entry to create (used for errors)
            etype:   Entry type to use

        Returns:
            The entry class object if found, else None
        """
        # Convert something like 'u-boot@0' to 'u_boot' since we are only
        # interested in the type.
        module_name = etype.replace('-', '_')
        if '@' in module_name:
            module_name = module_name.split('@')[0]
        module = modules.get(module_name)

        # Also allow entry-type modules to be brought in from the etype directory.

        # Import the module if we have not already done so.
        if not module:
            old_path = sys.path
            sys.path.insert(0, os.path.join(our_path, 'etype'))
            try:
                if have_importlib:
                    module = importlib.import_module(module_name)
                else:
                    module = __import__(module_name)
            except ImportError as e:
                raise ValueError("Unknown entry type '%s' in node '%s' (expected etype/%s.py, error '%s'" %
                                 (etype, node_path, module_name, e))
            finally:
                sys.path = old_path
            modules[module_name] = module

        # Look up the expected class name
        return getattr(module, 'Entry_%s' % module_name)

    @staticmethod
    def Create(section, node, etype=None):
        """Create a new entry for a node.

        Args:
            section: Section object containing this node
            node:    Node object containing information about the entry to
                     create
            etype:   Entry type to use, or None to work it out (used for tests)

        Returns:
            A new Entry object of the correct type (a subclass of Entry)
        """
        if not etype:
            etype = fdt_util.GetString(node, 'type', node.name)
        obj = Entry.Lookup(section, node.path, etype)

        # Call its constructor to get the object we want.
        return obj(section, etype, node)

    def ReadNode(self):
        """Read entry information from the node

        This reads all the fields we recognise from the node, ready for use.
        """
        if 'pos' in self._node.props:
            self.Raise("Please use 'offset' instead of 'pos'")
        self.offset = fdt_util.GetInt(self._node, 'offset')
        self.size = fdt_util.GetInt(self._node, 'size')
        self.align = fdt_util.GetInt(self._node, 'align')
        if tools.NotPowerOfTwo(self.align):
            raise ValueError("Node '%s': Alignment %s must be a power of two" %
                             (self._node.path, self.align))
        self.pad_before = fdt_util.GetInt(self._node, 'pad-before', 0)
        self.pad_after = fdt_util.GetInt(self._node, 'pad-after', 0)
        self.align_size = fdt_util.GetInt(self._node, 'align-size')
        if tools.NotPowerOfTwo(self.align_size):
            raise ValueError("Node '%s': Alignment size %s must be a power "
                             "of two" % (self._node.path, self.align_size))
        self.align_end = fdt_util.GetInt(self._node, 'align-end')
        self.offset_unset = fdt_util.GetBool(self._node, 'offset-unset')
        self.expand_size = fdt_util.GetBool(self._node, 'expand-size')

    def GetDefaultFilename(self):
        return None

    def GetFdtSet(self):
        """Get the set of device trees used by this entry

        Returns:
            Set containing the filename from this entry, if it is a .dtb, else
            an empty set
        """
        fname = self.GetDefaultFilename()
        # It would be better to use isinstance(self, Entry_blob_dtb) here but
        # we cannot access Entry_blob_dtb
        if fname and fname.endswith('.dtb'):
            return Set([fname])
        return Set()

    def ExpandEntries(self):
        pass

    def AddMissingProperties(self):
        """Add new properties to the device tree as needed for this entry"""
        for prop in ['offset', 'size', 'image-pos']:
            if not prop in self._node.props:
                state.AddZeroProp(self._node, prop)
        err = state.CheckAddHashProp(self._node)
        if err:
            self.Raise(err)

    def SetCalculatedProperties(self):
        """Set the value of device-tree properties calculated by binman"""
        state.SetInt(self._node, 'offset', self.offset)
        state.SetInt(self._node, 'size', self.size)
        state.SetInt(self._node, 'image-pos',
                       self.image_pos - self.section.GetRootSkipAtStart())
        state.CheckSetHashValue(self._node, self.GetData)

    def ProcessFdt(self, fdt):
        """Allow entries to adjust the device tree

        Some entries need to adjust the device tree for their purposes. This
        may involve adding or deleting properties.

        Returns:
            True if processing is complete
            False if processing could not be completed due to a dependency.
                This will cause the entry to be retried after others have been
                called
        """
        return True

    def SetPrefix(self, prefix):
        """Set the name prefix for a node

        Args:
            prefix: Prefix to set, or '' to not use a prefix
        """
        if prefix:
            self.name = prefix + self.name

    def SetContents(self, data):
        """Set the contents of an entry

        This sets both the data and content_size properties

        Args:
            data: Data to set to the contents (string)
        """
        self.data = data
        self.contents_size = len(self.data)

    def ProcessContentsUpdate(self, data):
        """Update the contens of an entry, after the size is fixed

        This checks that the new data is the same size as the old.

        Args:
            data: Data to set to the contents (string)

        Raises:
            ValueError if the new data size is not the same as the old
        """
        if len(data) != self.contents_size:
            self.Raise('Cannot update entry size from %d to %d' %
                       (len(data), self.contents_size))
        self.SetContents(data)

    def ObtainContents(self):
        """Figure out the contents of an entry.

        Returns:
            True if the contents were found, False if another call is needed
            after the other entries are processed.
        """
        # No contents by default: subclasses can implement this
        return True

    def Pack(self, offset):
        """Figure out how to pack the entry into the section

        Most of the time the entries are not fully specified. There may be
        an alignment but no size. In that case we take the size from the
        contents of the entry.

        If an entry has no hard-coded offset, it will be placed at @offset.

        Once this function is complete, both the offset and size of the
        entry will be know.

        Args:
            Current section offset pointer

        Returns:
            New section offset pointer (after this entry)
        """
        if self.offset is None:
            if self.offset_unset:
                self.Raise('No offset set with offset-unset: should another '
                           'entry provide this correct offset?')
            self.offset = tools.Align(offset, self.align)
        needed = self.pad_before + self.contents_size + self.pad_after
        needed = tools.Align(needed, self.align_size)
        size = self.size
        if not size:
            size = needed
        new_offset = self.offset + size
        aligned_offset = tools.Align(new_offset, self.align_end)
        if aligned_offset != new_offset:
            size = aligned_offset - self.offset
            new_offset = aligned_offset

        if not self.size:
            self.size = size

        if self.size < needed:
            self.Raise("Entry contents size is %#x (%d) but entry size is "
                       "%#x (%d)" % (needed, needed, self.size, self.size))
        # Check that the alignment is correct. It could be wrong if the
        # and offset or size values were provided (i.e. not calculated), but
        # conflict with the provided alignment values
        if self.size != tools.Align(self.size, self.align_size):
            self.Raise("Size %#x (%d) does not match align-size %#x (%d)" %
                  (self.size, self.size, self.align_size, self.align_size))
        if self.offset != tools.Align(self.offset, self.align):
            self.Raise("Offset %#x (%d) does not match align %#x (%d)" %
                  (self.offset, self.offset, self.align, self.align))

        return new_offset

    def Raise(self, msg):
        """Convenience function to raise an error referencing a node"""
        raise ValueError("Node '%s': %s" % (self._node.path, msg))

    def GetEntryArgsOrProps(self, props, required=False):
        """Return the values of a set of properties

        Args:
            props: List of EntryArg objects

        Raises:
            ValueError if a property is not found
        """
        values = []
        missing = []
        for prop in props:
            python_prop = prop.name.replace('-', '_')
            if hasattr(self, python_prop):
                value = getattr(self, python_prop)
            else:
                value = None
            if value is None:
                value = self.GetArg(prop.name, prop.datatype)
            if value is None and required:
                missing.append(prop.name)
            values.append(value)
        if missing:
            self.Raise('Missing required properties/entry args: %s' %
                       (', '.join(missing)))
        return values

    def GetPath(self):
        """Get the path of a node

        Returns:
            Full path of the node for this entry
        """
        return self._node.path

    def GetData(self):
        return self.data

    def GetOffsets(self):
        return {}

    def SetOffsetSize(self, pos, size):
        self.offset = pos
        self.size = size

    def SetImagePos(self, image_pos):
        """Set the position in the image

        Args:
            image_pos: Position of this entry in the image
        """
        self.image_pos = image_pos + self.offset

    def ProcessContents(self):
        pass

    def WriteSymbols(self, section):
        """Write symbol values into binary files for access at run time

        Args:
          section: Section containing the entry
        """
        pass

    def CheckOffset(self):
        """Check that the entry offsets are correct

        This is used for entries which have extra offset requirements (other
        than having to be fully inside their section). Sub-classes can implement
        this function and raise if there is a problem.
        """
        pass

    @staticmethod
    def GetStr(value):
        if value is None:
            return '<none>  '
        return '%08x' % value

    @staticmethod
    def WriteMapLine(fd, indent, name, offset, size, image_pos):
        print('%s  %s%s  %s  %s' % (Entry.GetStr(image_pos), ' ' * indent,
                                    Entry.GetStr(offset), Entry.GetStr(size),
                                    name), file=fd)

    def WriteMap(self, fd, indent):
        """Write a map of the entry to a .map file

        Args:
            fd: File to write the map to
            indent: Curent indent level of map (0=none, 1=one level, etc.)
        """
        self.WriteMapLine(fd, indent, self.name, self.offset, self.size,
                          self.image_pos)

    def GetEntries(self):
        """Return a list of entries contained by this entry

        Returns:
            List of entries, or None if none. A normal entry has no entries
                within it so will return None
        """
        return None

    def GetArg(self, name, datatype=str):
        """Get the value of an entry argument or device-tree-node property

        Some node properties can be provided as arguments to binman. First check
        the entry arguments, and fall back to the device tree if not found

        Args:
            name: Argument name
            datatype: Data type (str or int)

        Returns:
            Value of argument as a string or int, or None if no value

        Raises:
            ValueError if the argument cannot be converted to in
        """
        value = state.GetEntryArg(name)
        if value is not None:
            if datatype == int:
                try:
                    value = int(value)
                except ValueError:
                    self.Raise("Cannot convert entry arg '%s' (value '%s') to integer" %
                               (name, value))
            elif datatype == str:
                pass
            else:
                raise ValueError("GetArg() internal error: Unknown data type '%s'" %
                                 datatype)
        else:
            value = fdt_util.GetDatatype(self._node, name, datatype)
        return value

    @staticmethod
    def WriteDocs(modules, test_missing=None):
        """Write out documentation about the various entry types to stdout

        Args:
            modules: List of modules to include
            test_missing: Used for testing. This is a module to report
                as missing
        """
        print('''Binman Entry Documentation
===========================

This file describes the entry types supported by binman. These entry types can
be placed in an image one by one to build up a final firmware image. It is
fairly easy to create new entry types. Just add a new file to the 'etype'
directory. You can use the existing entries as examples.

Note that some entries are subclasses of others, using and extending their
features to produce new behaviours.


''')
        modules = sorted(modules)

        # Don't show the test entry
        if '_testing' in modules:
            modules.remove('_testing')
        missing = []
        for name in modules:
            module = Entry.Lookup(name, name, name)
            docs = getattr(module, '__doc__')
            if test_missing == name:
                docs = None
            if docs:
                lines = docs.splitlines()
                first_line = lines[0]
                rest = [line[4:] for line in lines[1:]]
                hdr = 'Entry: %s: %s' % (name.replace('_', '-'), first_line)
                print(hdr)
                print('-' * len(hdr))
                print('\n'.join(rest))
                print()
                print()
            else:
                missing.append(name)

        if missing:
            raise ValueError('Documentation is missing for modules: %s' %
                             ', '.join(missing))

    def GetUniqueName(self):
        """Get a unique name for a node

        Returns:
            String containing a unique name for a node, consisting of the name
            of all ancestors (starting from within the 'binman' node) separated
            by a dot ('.'). This can be useful for generating unique filesnames
            in the output directory.
        """
        name = self.name
        node = self._node
        while node.parent:
            node = node.parent
            if node.name == 'binman':
                break
            name = '%s.%s' % (node.name, name)
        return name

    def ExpandToLimit(self, limit):
        """Expand an entry so that it ends at the given offset limit"""
        if self.offset + self.size < limit:
            self.size = limit - self.offset
            # Request the contents again, since changing the size requires that
            # the data grows. This should not fail, but check it to be sure.
            if not self.ObtainContents():
                self.Raise('Cannot obtain contents when expanding entry')
