# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Creates binary images from input files controlled by a description
#

from collections import OrderedDict
import glob
try:
    import importlib.resources
except ImportError:  # pragma: no cover
    # for Python 3.6
    import importlib_resources
import os
import pkg_resources
import re

import sys

from binman import bintool
from binman import cbfs_util
from binman import elf
from binman import entry
from dtoc import fdt_util
from u_boot_pylib import command
from u_boot_pylib import tools
from u_boot_pylib import tout

# These are imported if needed since they import libfdt
state = None
Image = None

# List of images we plan to create
# Make this global so that it can be referenced from tests
images = OrderedDict()

# Help text for each type of missing blob, dict:
#    key: Value of the entry's 'missing-msg' or entry name
#    value: Text for the help
missing_blob_help = {}

def _ReadImageDesc(binman_node, use_expanded):
    """Read the image descriptions from the /binman node

    This normally produces a single Image object called 'image'. But if
    multiple images are present, they will all be returned.

    Args:
        binman_node: Node object of the /binman node
        use_expanded: True if the FDT will be updated with the entry information
    Returns:
        OrderedDict of Image objects, each of which describes an image
    """
    # For Image()
    # pylint: disable=E1102
    images = OrderedDict()
    if 'multiple-images' in binman_node.props:
        for node in binman_node.subnodes:
            if not node.name.startswith('template'):
                images[node.name] = Image(node.name, node,
                                          use_expanded=use_expanded)
    else:
        images['image'] = Image('image', binman_node, use_expanded=use_expanded)
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

def _ReadMissingBlobHelp():
    """Read the missing-blob-help file

    This file containins help messages explaining what to do when external blobs
    are missing.

    Returns:
        Dict:
            key: Message tag (str)
            value: Message text (str)
    """

    def _FinishTag(tag, msg, result):
        if tag:
            result[tag] = msg.rstrip()
            tag = None
            msg = ''
        return tag, msg

    my_data = pkg_resources.resource_string(__name__, 'missing-blob-help')
    re_tag = re.compile('^([-a-z0-9]+):$')
    result = {}
    tag = None
    msg = ''
    for line in my_data.decode('utf-8').splitlines():
        if not line.startswith('#'):
            m_tag = re_tag.match(line)
            if m_tag:
                _, msg = _FinishTag(tag, msg, result)
                tag = m_tag.group(1)
            elif tag:
                msg += line + '\n'
    _FinishTag(tag, msg, result)
    return result

def _ShowBlobHelp(level, path, text, fname):
    tout.do_output(level, '%s (%s):' % (path, fname))
    for line in text.splitlines():
        tout.do_output(level, '   %s' % line)
    tout.do_output(level, '')

def _ShowHelpForMissingBlobs(level, missing_list):
    """Show help for each missing blob to help the user take action

    Args:
        missing_list: List of Entry objects to show help for
    """
    global missing_blob_help

    if not missing_blob_help:
        missing_blob_help = _ReadMissingBlobHelp()

    for entry in missing_list:
        tags = entry.GetHelpTags()

        # Show the first match help message
        shown_help = False
        for tag in tags:
            if tag in missing_blob_help:
                _ShowBlobHelp(level, entry._node.path, missing_blob_help[tag],
                              entry.GetDefaultFilename())
                shown_help = True
                break
        # Or a generic help message
        if not shown_help:
            _ShowBlobHelp(level, entry._node.path, "Missing blob",
                          entry.GetDefaultFilename())

def GetEntryModules(include_testing=True):
    """Get a set of entry class implementations

    Returns:
        Set of paths to entry class filenames
    """
    glob_list = pkg_resources.resource_listdir(__name__, 'etype')
    glob_list = [fname for fname in glob_list if fname.endswith('.py')]
    return set([os.path.splitext(os.path.basename(item))[0]
                for item in glob_list
                if include_testing or '_testing' not in item])

def WriteEntryDocs(modules, test_missing=None):
    """Write out documentation for all entries

    Args:
        modules: List of Module objects to get docs for
        test_missing: Used for testing only, to force an entry's documentation
            to show as missing even if it is present. Should be set to None in
            normal use.
    """
    from binman.entry import Entry
    Entry.WriteDocs(modules, test_missing)


def write_bintool_docs(modules, test_missing=None):
    """Write out documentation for all bintools

    Args:
        modules: List of Module objects to get docs for
        test_missing: Used for testing only, to force an entry's documentation
            to show as missing even if it is present. Should be set to None in
            normal use.
    """
    bintool.Bintool.WriteDocs(modules, test_missing)


def ListEntries(image_fname, entry_paths):
    """List the entries in an image

    This decodes the supplied image and displays a table of entries from that
    image, preceded by a header.

    Args:
        image_fname: Image filename to process
        entry_paths: List of wildcarded paths (e.g. ['*dtb*', 'u-boot*',
                                                     'section/u-boot'])
    """
    image = Image.FromFile(image_fname)

    entries, lines, widths = image.GetListEntries(entry_paths)

    num_columns = len(widths)
    for linenum, line in enumerate(lines):
        if linenum == 1:
            # Print header line
            print('-' * (sum(widths) + num_columns * 2))
        out = ''
        for i, item in enumerate(line):
            width = -widths[i]
            if item.startswith('>'):
                width = -width
                item = item[1:]
            txt = '%*s  ' % (width, item)
            out += txt
        print(out.rstrip())


def ReadEntry(image_fname, entry_path, decomp=True):
    """Extract an entry from an image

    This extracts the data from a particular entry in an image

    Args:
        image_fname: Image filename to process
        entry_path: Path to entry to extract
        decomp: True to return uncompressed data, if the data is compress
            False to return the raw data

    Returns:
        data extracted from the entry
    """
    global Image
    from binman.image import Image

    image = Image.FromFile(image_fname)
    image.CollectBintools()
    entry = image.FindEntryPath(entry_path)
    return entry.ReadData(decomp)


def ShowAltFormats(image):
    """Show alternative formats available for entries in the image

    This shows a list of formats available.

    Args:
        image (Image): Image to check
    """
    alt_formats = {}
    image.CheckAltFormats(alt_formats)
    print('%-10s  %-20s  %s' % ('Flag (-F)', 'Entry type', 'Description'))
    for name, val in alt_formats.items():
        entry, helptext = val
        print('%-10s  %-20s  %s' % (name, entry.etype, helptext))


def ExtractEntries(image_fname, output_fname, outdir, entry_paths,
                   decomp=True, alt_format=None):
    """Extract the data from one or more entries and write it to files

    Args:
        image_fname: Image filename to process
        output_fname: Single output filename to use if extracting one file, None
            otherwise
        outdir: Output directory to use (for any number of files), else None
        entry_paths: List of entry paths to extract
        decomp: True to decompress the entry data

    Returns:
        List of EntryInfo records that were written
    """
    image = Image.FromFile(image_fname)
    image.CollectBintools()

    if alt_format == 'list':
        ShowAltFormats(image)
        return

    # Output an entry to a single file, as a special case
    if output_fname:
        if not entry_paths:
            raise ValueError('Must specify an entry path to write with -f')
        if len(entry_paths) != 1:
            raise ValueError('Must specify exactly one entry path to write with -f')
        entry = image.FindEntryPath(entry_paths[0])
        data = entry.ReadData(decomp, alt_format)
        tools.write_file(output_fname, data)
        tout.notice("Wrote %#x bytes to file '%s'" % (len(data), output_fname))
        return

    # Otherwise we will output to a path given by the entry path of each entry.
    # This means that entries will appear in subdirectories if they are part of
    # a sub-section.
    einfos = image.GetListEntries(entry_paths)[0]
    tout.notice('%d entries match and will be written' % len(einfos))
    for einfo in einfos:
        entry = einfo.entry
        data = entry.ReadData(decomp, alt_format)
        path = entry.GetPath()[1:]
        fname = os.path.join(outdir, path)

        # If this entry has children, create a directory for it and put its
        # data in a file called 'root' in that directory
        if entry.GetEntries():
            if fname and not os.path.exists(fname):
                os.makedirs(fname)
            fname = os.path.join(fname, 'root')
        tout.notice("Write entry '%s' size %x to '%s'" %
                    (entry.GetPath(), len(data), fname))
        tools.write_file(fname, data)
    return einfos


def BeforeReplace(image, allow_resize):
    """Handle getting an image ready for replacing entries in it

    Args:
        image: Image to prepare
    """
    state.PrepareFromLoadedData(image)
    image.CollectBintools()
    image.LoadData(decomp=False)

    # If repacking, drop the old offset/size values except for the original
    # ones, so we are only left with the constraints.
    if image.allow_repack and allow_resize:
        image.ResetForPack()


def ReplaceOneEntry(image, entry, data, do_compress, allow_resize):
    """Handle replacing a single entry an an image

    Args:
        image: Image to update
        entry: Entry to write
        data: Data to replace with
        do_compress: True to compress the data if needed, False if data is
            already compressed so should be used as is
        allow_resize: True to allow entries to change size (this does a re-pack
            of the entries), False to raise an exception
    """
    if not entry.WriteData(data, do_compress):
        if not image.allow_repack:
            entry.Raise('Entry data size does not match, but allow-repack is not present for this image')
        if not allow_resize:
            entry.Raise('Entry data size does not match, but resize is disabled')


def AfterReplace(image, allow_resize, write_map):
    """Handle write out an image after replacing entries in it

    Args:
        image: Image to write
        allow_resize: True to allow entries to change size (this does a re-pack
            of the entries), False to raise an exception
        write_map: True to write a map file
    """
    tout.info('Processing image')
    ProcessImage(image, update_fdt=True, write_map=write_map,
                 get_contents=False, allow_resize=allow_resize)


def WriteEntryToImage(image, entry, data, do_compress=True, allow_resize=True,
                      write_map=False):
    BeforeReplace(image, allow_resize)
    tout.info('Writing data to %s' % entry.GetPath())
    ReplaceOneEntry(image, entry, data, do_compress, allow_resize)
    AfterReplace(image, allow_resize=allow_resize, write_map=write_map)


def WriteEntry(image_fname, entry_path, data, do_compress=True,
               allow_resize=True, write_map=False):
    """Replace an entry in an image

    This replaces the data in a particular entry in an image. This size of the
    new data must match the size of the old data unless allow_resize is True.

    Args:
        image_fname: Image filename to process
        entry_path: Path to entry to extract
        data: Data to replace with
        do_compress: True to compress the data if needed, False if data is
            already compressed so should be used as is
        allow_resize: True to allow entries to change size (this does a re-pack
            of the entries), False to raise an exception
        write_map: True to write a map file

    Returns:
        Image object that was updated
    """
    tout.info("Write entry '%s', file '%s'" % (entry_path, image_fname))
    image = Image.FromFile(image_fname)
    image.CollectBintools()
    entry = image.FindEntryPath(entry_path)
    WriteEntryToImage(image, entry, data, do_compress=do_compress,
                      allow_resize=allow_resize, write_map=write_map)

    return image


def ReplaceEntries(image_fname, input_fname, indir, entry_paths,
                   do_compress=True, allow_resize=True, write_map=False):
    """Replace the data from one or more entries from input files

    Args:
        image_fname: Image filename to process
        input_fname: Single input filename to use if replacing one file, None
            otherwise
        indir: Input directory to use (for any number of files), else None
        entry_paths: List of entry paths to replace
        do_compress: True if the input data is uncompressed and may need to be
            compressed if the entry requires it, False if the data is already
            compressed.
        write_map: True to write a map file

    Returns:
        List of EntryInfo records that were written
    """
    image_fname = os.path.abspath(image_fname)
    image = Image.FromFile(image_fname)

    image.mark_build_done()

    # Replace an entry from a single file, as a special case
    if input_fname:
        if not entry_paths:
            raise ValueError('Must specify an entry path to read with -f')
        if len(entry_paths) != 1:
            raise ValueError('Must specify exactly one entry path to write with -f')
        entry = image.FindEntryPath(entry_paths[0])
        data = tools.read_file(input_fname)
        tout.notice("Read %#x bytes from file '%s'" % (len(data), input_fname))
        WriteEntryToImage(image, entry, data, do_compress=do_compress,
                          allow_resize=allow_resize, write_map=write_map)
        return

    # Otherwise we will input from a path given by the entry path of each entry.
    # This means that files must appear in subdirectories if they are part of
    # a sub-section.
    einfos = image.GetListEntries(entry_paths)[0]
    tout.notice("Replacing %d matching entries in image '%s'" %
                (len(einfos), image_fname))

    BeforeReplace(image, allow_resize)

    for einfo in einfos:
        entry = einfo.entry
        if entry.GetEntries():
            tout.info("Skipping section entry '%s'" % entry.GetPath())
            continue

        path = entry.GetPath()[1:]
        fname = os.path.join(indir, path)

        if os.path.exists(fname):
            tout.notice("Write entry '%s' from file '%s'" %
                        (entry.GetPath(), fname))
            data = tools.read_file(fname)
            ReplaceOneEntry(image, entry, data, do_compress, allow_resize)
        else:
            tout.warning("Skipping entry '%s' from missing file '%s'" %
                         (entry.GetPath(), fname))

    AfterReplace(image, allow_resize=allow_resize, write_map=write_map)
    return image

def SignEntries(image_fname, input_fname, privatekey_fname, algo, entry_paths,
                write_map=False):
    """Sign and replace the data from one or more entries from input files

    Args:
        image_fname: Image filename to process
        input_fname: Single input filename to use if replacing one file, None
            otherwise
        algo: Hashing algorithm
        entry_paths: List of entry paths to sign
        privatekey_fname: Private key filename
        write_map (bool): True to write the map file
    """
    image_fname = os.path.abspath(image_fname)
    image = Image.FromFile(image_fname)

    image.mark_build_done()

    BeforeReplace(image, allow_resize=True)

    for entry_path in entry_paths:
        entry = image.FindEntryPath(entry_path)
        entry.UpdateSignatures(privatekey_fname, algo, input_fname)

    AfterReplace(image, allow_resize=True, write_map=write_map)

def _ProcessTemplates(parent):
    """Handle any templates in the binman description

    Args:
        parent: Binman node to process (typically /binman)

    Returns:
        bool: True if any templates were processed

    Search though each target node looking for those with an 'insert-template'
    property. Use that as a list of references to template nodes to use to
    adjust the target node.

    Processing involves copying each subnode of the template node into the
    target node.

    This is done recursively, so templates can be at any level of the binman
    image, e.g. inside a section.

    See 'Templates' in the Binman documnentation for details.
    """
    found = False
    for node in parent.subnodes:
        tmpl = fdt_util.GetPhandleList(node, 'insert-template')
        if tmpl:
            node.copy_subnodes_from_phandles(tmpl)
            found = True

        found |= _ProcessTemplates(node)
    return found

def _RemoveTemplates(parent):
    """Remove any templates in the binman description
    """
    for node in parent.subnodes:
        if node.name.startswith('template'):
            node.Delete()

def PrepareImagesAndDtbs(dtb_fname, select_images, update_fdt, use_expanded):
    """Prepare the images to be processed and select the device tree

    This function:
    - reads in the device tree
    - finds and scans the binman node to create all entries
    - selects which images to build
    - Updates the device tress with placeholder properties for offset,
        image-pos, etc.

    Args:
        dtb_fname: Filename of the device tree file to use (.dts or .dtb)
        selected_images: List of images to output, or None for all
        update_fdt: True to update the FDT wth entry offsets, etc.
        use_expanded: True to use expanded versions of entries, if available.
            So if 'u-boot' is called for, we use 'u-boot-expanded' instead. This
            is needed if update_fdt is True (although tests may disable it)

    Returns:
        OrderedDict of images:
            key: Image name (str)
            value: Image object
    """
    # Import these here in case libfdt.py is not available, in which case
    # the above help option still works.
    from dtoc import fdt
    from dtoc import fdt_util
    global images

    # Get the device tree ready by compiling it and copying the compiled
    # output into a file in our output directly. Then scan it for use
    # in binman.
    dtb_fname = fdt_util.EnsureCompiled(dtb_fname)
    fname = tools.get_output_filename('u-boot.dtb.out')
    tools.write_file(fname, tools.read_file(dtb_fname))
    dtb = fdt.FdtScan(fname)

    node = _FindBinmanNode(dtb)
    if not node:
        raise ValueError("Device tree '%s' does not have a 'binman' "
                            "node" % dtb_fname)

    if _ProcessTemplates(node):
        dtb.Sync(True)
        fname = tools.get_output_filename('u-boot.dtb.tmpl1')
        tools.write_file(fname, dtb.GetContents())

        _RemoveTemplates(node)
        dtb.Sync(True)

        # Rescan the dtb to pick up the new phandles
        dtb.Scan()
        node = _FindBinmanNode(dtb)
        fname = tools.get_output_filename('u-boot.dtb.tmpl2')
        tools.write_file(fname, dtb.GetContents())

    images = _ReadImageDesc(node, use_expanded)

    if select_images:
        skip = []
        new_images = OrderedDict()
        for name, image in images.items():
            if name in select_images:
                new_images[name] = image
            else:
                skip.append(name)
        images = new_images
        tout.notice('Skipping images: %s' % ', '.join(skip))

    state.Prepare(images, dtb)

    # Prepare the device tree by making sure that any missing
    # properties are added (e.g. 'pos' and 'size'). The values of these
    # may not be correct yet, but we add placeholders so that the
    # size of the device tree is correct. Later, in
    # SetCalculatedProperties() we will insert the correct values
    # without changing the device-tree size, thus ensuring that our
    # entry offsets remain the same.
    for image in images.values():
        image.gen_entries()
        image.CollectBintools()
        if update_fdt:
            image.AddMissingProperties(True)
        image.ProcessFdt(dtb)

    for dtb_item in state.GetAllFdts():
        dtb_item.Sync(auto_resize=True)
        dtb_item.Pack()
        dtb_item.Flush()
    return images


def ProcessImage(image, update_fdt, write_map, get_contents=True,
                 allow_resize=True, allow_missing=False,
                 allow_fake_blobs=False):
    """Perform all steps for this image, including checking and # writing it.

    This means that errors found with a later image will be reported after
    earlier images are already completed and written, but that does not seem
    important.

    Args:
        image: Image to process
        update_fdt: True to update the FDT wth entry offsets, etc.
        write_map: True to write a map file
        get_contents: True to get the image contents from files, etc., False if
            the contents is already present
        allow_resize: True to allow entries to change size (this does a re-pack
            of the entries), False to raise an exception
        allow_missing: Allow blob_ext objects to be missing
        allow_fake_blobs: Allow blob_ext objects to be faked with dummy files

    Returns:
        True if one or more external blobs are missing or faked,
        False if all are present
    """
    if get_contents:
        image.SetAllowMissing(allow_missing)
        image.SetAllowFakeBlob(allow_fake_blobs)
        image.GetEntryContents()
        image.drop_absent()
    image.GetEntryOffsets()

    # We need to pack the entries to figure out where everything
    # should be placed. This sets the offset/size of each entry.
    # However, after packing we call ProcessEntryContents() which
    # may result in an entry changing size. In that case we need to
    # do another pass. Since the device tree often contains the
    # final offset/size information we try to make space for this in
    # AddMissingProperties() above. However, if the device is
    # compressed we cannot know this compressed size in advance,
    # since changing an offset from 0x100 to 0x104 (for example) can
    # alter the compressed size of the device tree. So we need a
    # third pass for this.
    passes = 5
    for pack_pass in range(passes):
        try:
            image.PackEntries()
        except Exception as e:
            if write_map:
                fname = image.WriteMap()
                print("Wrote map file '%s' to show errors"  % fname)
            raise
        image.SetImagePos()
        if update_fdt:
            image.SetCalculatedProperties()
            for dtb_item in state.GetAllFdts():
                dtb_item.Sync()
                dtb_item.Flush()
        image.WriteSymbols()
        sizes_ok = image.ProcessEntryContents()
        if sizes_ok:
            break
        image.ResetForPack()
    tout.info('Pack completed after %d pass(es)' % (pack_pass + 1))
    if not sizes_ok:
        image.Raise('Entries changed size after packing (tried %s passes)' %
                    passes)

    image.BuildImage()
    if write_map:
        image.WriteMap()

    missing_list = []
    image.CheckMissing(missing_list)
    if missing_list:
        tout.error("Image '%s' is missing external blobs and is non-functional: %s\n" %
                   (image.name, ' '.join([e.name for e in missing_list])))
        _ShowHelpForMissingBlobs(tout.ERROR, missing_list)

    faked_list = []
    image.CheckFakedBlobs(faked_list)
    if faked_list:
        tout.warning(
            "Image '%s' has faked external blobs and is non-functional: %s\n" %
            (image.name, ' '.join([os.path.basename(e.GetDefaultFilename())
                                   for e in faked_list])))

    optional_list = []
    image.CheckOptional(optional_list)
    if optional_list:
        tout.warning(
            "Image '%s' is missing optional external blobs but is still functional: %s\n" %
            (image.name, ' '.join([e.name for e in optional_list])))
        _ShowHelpForMissingBlobs(tout.WARNING, optional_list)

    missing_bintool_list = []
    image.check_missing_bintools(missing_bintool_list)
    if missing_bintool_list:
        tout.warning(
            "Image '%s' has missing bintools and is non-functional: %s\n" %
            (image.name, ' '.join([os.path.basename(bintool.name)
                                   for bintool in missing_bintool_list])))
    return any([missing_list, faked_list, missing_bintool_list])


def Binman(args):
    """The main control code for binman

    This assumes that help and test options have already been dealt with. It
    deals with the core task of building images.

    Args:
        args: Command line arguments Namespace object
    """
    global Image
    global state

    if args.full_help:
        with importlib.resources.path('binman', 'README.rst') as readme:
            tools.print_full_help(str(readme))
        return 0

    # Put these here so that we can import this module without libfdt
    from binman.image import Image
    from binman import state

    tool_paths = []
    if args.toolpath:
        tool_paths += args.toolpath
    if args.tooldir:
        tool_paths.append(args.tooldir)
    tools.set_tool_paths(tool_paths or None)
    bintool.Bintool.set_tool_dir(args.tooldir)

    if args.cmd in ['ls', 'extract', 'replace', 'tool', 'sign']:
        try:
            tout.init(args.verbosity)
            if args.cmd == 'replace':
                tools.prepare_output_dir(args.outdir, args.preserve)
            else:
                tools.prepare_output_dir(None)
            if args.cmd == 'ls':
                ListEntries(args.image, args.paths)

            if args.cmd == 'extract':
                ExtractEntries(args.image, args.filename, args.outdir, args.paths,
                               not args.uncompressed, args.format)

            if args.cmd == 'replace':
                ReplaceEntries(args.image, args.filename, args.indir, args.paths,
                               do_compress=not args.compressed,
                               allow_resize=not args.fix_size, write_map=args.map)

            if args.cmd == 'sign':
                SignEntries(args.image, args.file, args.key, args.algo, args.paths)

            if args.cmd == 'tool':
                if args.list:
                    bintool.Bintool.list_all()
                elif args.fetch:
                    if not args.bintools:
                        raise ValueError(
                            "Please specify bintools to fetch or 'all' or 'missing'")
                    bintool.Bintool.fetch_tools(bintool.FETCH_ANY,
                                                args.bintools)
                else:
                    raise ValueError("Invalid arguments to 'tool' subcommand")
        except:
            raise
        finally:
            tools.finalise_output_dir()
        return 0

    elf_params = None
    if args.update_fdt_in_elf:
        elf_params = args.update_fdt_in_elf.split(',')
        if len(elf_params) != 4:
            raise ValueError('Invalid args %s to --update-fdt-in-elf: expected infile,outfile,begin_sym,end_sym' %
                             elf_params)

    # Try to figure out which device tree contains our image description
    if args.dt:
        dtb_fname = args.dt
    else:
        board = args.board
        if not board:
            raise ValueError('Must provide a board to process (use -b <board>)')
        board_pathname = os.path.join(args.build_dir, board)
        dtb_fname = os.path.join(board_pathname, 'u-boot.dtb')
        if not args.indir:
            args.indir = ['.']
        args.indir.append(board_pathname)

    try:
        tout.init(args.verbosity)
        elf.debug = args.debug
        cbfs_util.VERBOSE = args.verbosity > 2
        state.use_fake_dtb = args.fake_dtb

        # Normally we replace the 'u-boot' etype with 'u-boot-expanded', etc.
        # When running tests this can be disabled using this flag. When not
        # updating the FDT in image, it is not needed by binman, but we use it
        # for consistency, so that the images look the same to U-Boot at
        # runtime.
        use_expanded = not args.no_expanded
        try:
            tools.set_input_dirs(args.indir)
            tools.prepare_output_dir(args.outdir, args.preserve)
            state.SetEntryArgs(args.entry_arg)
            state.SetThreads(args.threads)

            images = PrepareImagesAndDtbs(dtb_fname, args.image,
                                          args.update_fdt, use_expanded)

            if args.test_section_timeout:
                # Set the first image to timeout, used in testThreadTimeout()
                images[list(images.keys())[0]].test_section_timeout = True
            invalid = False
            bintool.Bintool.set_missing_list(
                args.force_missing_bintools.split(',') if
                args.force_missing_bintools else None)

            # Create the directory here instead of Entry.check_fake_fname()
            # since that is called from a threaded context so different threads
            # may race to create the directory
            if args.fake_ext_blobs:
                entry.Entry.create_fake_dir()

            for image in images.values():
                invalid |= ProcessImage(image, args.update_fdt, args.map,
                                       allow_missing=args.allow_missing,
                                       allow_fake_blobs=args.fake_ext_blobs)

            # Write the updated FDTs to our output files
            for dtb_item in state.GetAllFdts():
                tools.write_file(dtb_item._fname, dtb_item.GetContents())

            if elf_params:
                data = state.GetFdtForEtype('u-boot-dtb').GetContents()
                elf.UpdateFile(*elf_params, data)

            bintool.Bintool.set_missing_list(None)

            # This can only be True if -M is provided, since otherwise binman
            # would have raised an error already
            if invalid:
                msg = 'Some images are invalid'
                if args.ignore_missing:
                    tout.warning(msg)
                else:
                    tout.error(msg)
                    return 103

            # Use this to debug the time take to pack the image
            #state.TimingShow()
        finally:
            tools.finalise_output_dir()
    finally:
        tout.uninit()

    return 0
