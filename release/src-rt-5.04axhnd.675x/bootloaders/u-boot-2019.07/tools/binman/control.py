# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Creates binary images from input files controlled by a description
#

from collections import OrderedDict
import os
import sys
import tools

import command
import elf
from image import Image
import state
import tout

# List of images we plan to create
# Make this global so that it can be referenced from tests
images = OrderedDict()

def _ReadImageDesc(binman_node):
    """Read the image descriptions from the /binman node

    This normally produces a single Image object called 'image'. But if
    multiple images are present, they will all be returned.

    Args:
        binman_node: Node object of the /binman node
    Returns:
        OrderedDict of Image objects, each of which describes an image
    """
    images = OrderedDict()
    if 'multiple-images' in binman_node.props:
        for node in binman_node.subnodes:
            images[node.name] = Image(node.name, node)
    else:
        images['image'] = Image('image', binman_node)
    return images

def _FindBinmanNode(dtb):
    """Find the 'binman' node in the device tree

    Args:
        dtb: Fdt object to scan
    Returns:
        Node object of /binman node, or None if not found
    """
    for node in dtb.GetRoot().subnodes:
        if node.name == 'binman':
            return node
    return None

def WriteEntryDocs(modules, test_missing=None):
    """Write out documentation for all entries

    Args:
        modules: List of Module objects to get docs for
        test_missing: Used for testing only, to force an entry's documeentation
            to show as missing even if it is present. Should be set to None in
            normal use.
    """
    from entry import Entry
    Entry.WriteDocs(modules, test_missing)

def Binman(options, args):
    """The main control code for binman

    This assumes that help and test options have already been dealt with. It
    deals with the core task of building images.

    Args:
        options: Command line options object
        args: Command line arguments (list of strings)
    """
    global images

    if options.full_help:
        pager = os.getenv('PAGER')
        if not pager:
            pager = 'more'
        fname = os.path.join(os.path.dirname(os.path.realpath(sys.argv[0])),
                            'README')
        command.Run(pager, fname)
        return 0

    # Try to figure out which device tree contains our image description
    if options.dt:
        dtb_fname = options.dt
    else:
        board = options.board
        if not board:
            raise ValueError('Must provide a board to process (use -b <board>)')
        board_pathname = os.path.join(options.build_dir, board)
        dtb_fname = os.path.join(board_pathname, 'u-boot.dtb')
        if not options.indir:
            options.indir = ['.']
        options.indir.append(board_pathname)

    try:
        # Import these here in case libfdt.py is not available, in which case
        # the above help option still works.
        import fdt
        import fdt_util

        tout.Init(options.verbosity)
        elf.debug = options.debug
        state.use_fake_dtb = options.fake_dtb
        try:
            tools.SetInputDirs(options.indir)
            tools.PrepareOutputDir(options.outdir, options.preserve)
            state.SetEntryArgs(options.entry_arg)

            # Get the device tree ready by compiling it and copying the compiled
            # output into a file in our output directly. Then scan it for use
            # in binman.
            dtb_fname = fdt_util.EnsureCompiled(dtb_fname)
            fname = tools.GetOutputFilename('u-boot.dtb.out')
            tools.WriteFile(fname, tools.ReadFile(dtb_fname))
            dtb = fdt.FdtScan(fname)

            node = _FindBinmanNode(dtb)
            if not node:
                raise ValueError("Device tree '%s' does not have a 'binman' "
                                 "node" % dtb_fname)

            images = _ReadImageDesc(node)

            if options.image:
                skip = []
                for name, image in images.iteritems():
                    if name not in options.image:
                        del images[name]
                        skip.append(name)
                if skip and options.verbosity >= 2:
                    print 'Skipping images: %s' % ', '.join(skip)

            state.Prepare(images, dtb)

            # Prepare the device tree by making sure that any missing
            # properties are added (e.g. 'pos' and 'size'). The values of these
            # may not be correct yet, but we add placeholders so that the
            # size of the device tree is correct. Later, in
            # SetCalculatedProperties() we will insert the correct values
            # without changing the device-tree size, thus ensuring that our
            # entry offsets remain the same.
            for image in images.values():
                image.ExpandEntries()
                if options.update_fdt:
                    image.AddMissingProperties()
                image.ProcessFdt(dtb)

            for dtb_item in state.GetFdts():
                dtb_item.Sync(auto_resize=True)
                dtb_item.Pack()
                dtb_item.Flush()

            for image in images.values():
                # Perform all steps for this image, including checking and
                # writing it. This means that errors found with a later
                # image will be reported after earlier images are already
                # completed and written, but that does not seem important.
                image.GetEntryContents()
                image.GetEntryOffsets()
                try:
                    image.PackEntries()
                    image.CheckSize()
                    image.CheckEntries()
                except Exception as e:
                    if options.map:
                        fname = image.WriteMap()
                        print "Wrote map file '%s' to show errors"  % fname
                    raise
                image.SetImagePos()
                if options.update_fdt:
                    image.SetCalculatedProperties()
                    for dtb_item in state.GetFdts():
                        dtb_item.Sync()
                image.ProcessEntryContents()
                image.WriteSymbols()
                image.BuildImage()
                if options.map:
                    image.WriteMap()

            # Write the updated FDTs to our output files
            for dtb_item in state.GetFdts():
                tools.WriteFile(dtb_item._fname, dtb_item.GetContents())

        finally:
            tools.FinaliseOutputDir()
    finally:
        tout.Uninit()

    return 0
