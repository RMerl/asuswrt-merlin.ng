#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2014 Google, Inc
#
# Intel microcode update tool

from optparse import OptionParser
import os
import re
import struct
import sys

MICROCODE_DIR = 'arch/x86/dts/microcode'

class Microcode:
    """Holds information about the microcode for a particular model of CPU.

    Attributes:
        name:  Name of the CPU this microcode is for, including any version
                   information (e.g. 'm12206a7_00000029')
        model: Model code string (this is cpuid(1).eax, e.g. '206a7')
        words: List of hex words containing the microcode. The first 16 words
                   are the public header.
    """
    def __init__(self, name, data):
        self.name = name
        # Convert data into a list of hex words
        self.words = []
        for value in ''.join(data).split(','):
            hexval = value.strip()
            if hexval:
                self.words.append(int(hexval, 0))

        # The model is in the 4rd hex word
        self.model = '%x' % self.words[3]

def ParseFile(fname):
    """Parse a micrcode.dat file and return the component parts

    Args:
        fname: Filename to parse
    Returns:
        3-Tuple:
            date:         String containing date from the file's header
            license_text: List of text lines for the license file
            microcodes:   List of Microcode objects from the file
    """
    re_date = re.compile('/\* *(.* [0-9]{4}) *\*/$')
    re_license = re.compile('/[^-*+] *(.*)$')
    re_name = re.compile('/\* *(.*)\.inc *\*/', re.IGNORECASE)
    microcodes = {}
    license_text = []
    date = ''
    data = []
    name = None
    with open(fname) as fd:
        for line in fd:
            line = line.rstrip()
            m_date = re_date.match(line)
            m_license = re_license.match(line)
            m_name = re_name.match(line)
            if m_name:
                if name:
                    microcodes[name] = Microcode(name, data)
                name = m_name.group(1).lower()
                data = []
            elif m_license:
                license_text.append(m_license.group(1))
            elif m_date:
                date = m_date.group(1)
            else:
                data.append(line)
    if name:
        microcodes[name] = Microcode(name, data)
    return date, license_text, microcodes

def ParseHeaderFiles(fname_list):
    """Parse a list of header files and return the component parts

    Args:
        fname_list: List of files to parse
    Returns:
            date:         String containing date from the file's header
            license_text: List of text lines for the license file
            microcodes:   List of Microcode objects from the file
    """
    microcodes = {}
    license_text = []
    date = ''
    name = None
    for fname in fname_list:
        name = os.path.basename(fname).lower()
        name = os.path.splitext(name)[0]
        data = []
        with open(fname) as fd:
            license_start = False
            license_end = False
            for line in fd:
                line = line.rstrip()

                if len(line) >= 2:
                    if line[0] == '/' and line[1] == '*':
                        license_start = True
                        continue
                    if line[0] == '*' and line[1] == '/':
                        license_end = True
                        continue
                if license_start and not license_end:
                    # Ignore blank line
                    if len(line) > 0:
                        license_text.append(line)
                    continue
                # Omit anything after the last comma
                words = line.split(',')[:-1]
                data += [word + ',' for word in words]
        microcodes[name] = Microcode(name, data)
    return date, license_text, microcodes


def List(date, microcodes, model):
    """List the available microcode chunks

    Args:
        date:           Date of the microcode file
        microcodes:     Dict of Microcode objects indexed by name
        model:          Model string to search for, or None
    """
    print('Date: %s' % date)
    if model:
        mcode_list, tried = FindMicrocode(microcodes, model.lower())
        print('Matching models %s:' % (', '.join(tried)))
    else:
        print('All models:')
        mcode_list = [microcodes[m] for m in list(microcodes.keys())]
    for mcode in mcode_list:
        print('%-20s: model %s' % (mcode.name, mcode.model))

def FindMicrocode(microcodes, model):
    """Find all the microcode chunks which match the given model.

    This model is something like 306a9 (the value returned in eax from
    cpuid(1) when running on Intel CPUs). But we allow a partial match,
    omitting the last 1 or two characters to allow many families to have the
    same microcode.

    If the model name is ambiguous we return a list of matches.

    Args:
        microcodes: Dict of Microcode objects indexed by name
        model:      String containing model name to find
    Returns:
        Tuple:
            List of matching Microcode objects
            List of abbreviations we tried
    """
    # Allow a full name to be used
    mcode = microcodes.get(model)
    if mcode:
        return [mcode], []

    tried = []
    found = []
    for i in range(3):
        abbrev = model[:-i] if i else model
        tried.append(abbrev)
        for mcode in list(microcodes.values()):
            if mcode.model.startswith(abbrev):
                found.append(mcode)
        if found:
            break
    return found, tried

def CreateFile(date, license_text, mcodes, outfile):
    """Create a microcode file in U-Boot's .dtsi format

    Args:
        date:       String containing date of original microcode file
        license:    List of text lines for the license file
        mcodes:      Microcode objects to write (normally only 1)
        outfile:    Filename to write to ('-' for stdout)
    """
    out = '''/*%s
 * ---
 * This is a device tree fragment. Use #include to add these properties to a
 * node.
 *
 * Date: %s
 */

compatible = "intel,microcode";
intel,header-version = <%d>;
intel,update-revision = <%#x>;
intel,date-code = <%#x>;
intel,processor-signature = <%#x>;
intel,checksum = <%#x>;
intel,loader-revision = <%d>;
intel,processor-flags = <%#x>;

/* The first 48-bytes are the public header which repeats the above data */
data = <%s
\t>;'''
    words = ''
    add_comments = len(mcodes) > 1
    for mcode in mcodes:
        if add_comments:
            words += '\n/* %s */' % mcode.name
        for i in range(len(mcode.words)):
            if not (i & 3):
                words += '\n'
            val = mcode.words[i]
            # Change each word so it will be little-endian in the FDT
            # This data is needed before RAM is available on some platforms so
            # we cannot do an endianness swap on boot.
            val = struct.unpack("<I", struct.pack(">I", val))[0]
            words += '\t%#010x' % val

    # Use the first microcode for the headers
    mcode = mcodes[0]

    # Take care to avoid adding a space before a tab
    text = ''
    for line in license_text:
        if line[0] == '\t':
            text += '\n *' + line
        else:
            text += '\n * ' + line
    args = [text, date]
    args += [mcode.words[i] for i in range(7)]
    args.append(words)
    if outfile == '-':
        print(out % tuple(args))
    else:
        if not outfile:
            if not os.path.exists(MICROCODE_DIR):
                print("Creating directory '%s'" % MICROCODE_DIR, file=sys.stderr)
                os.makedirs(MICROCODE_DIR)
            outfile = os.path.join(MICROCODE_DIR, mcode.name + '.dtsi')
        print("Writing microcode for '%s' to '%s'" % (
                ', '.join([mcode.name for mcode in mcodes]), outfile), file=sys.stderr)
        with open(outfile, 'w') as fd:
            print(out % tuple(args), file=fd)

def MicrocodeTool():
    """Run the microcode tool"""
    commands = 'create,license,list'.split(',')
    parser = OptionParser()
    parser.add_option('-d', '--mcfile', type='string', action='store',
                    help='Name of microcode.dat file')
    parser.add_option('-H', '--headerfile', type='string', action='append',
                    help='Name of .h file containing microcode')
    parser.add_option('-m', '--model', type='string', action='store',
                    help="Model name to extract ('all' for all)")
    parser.add_option('-M', '--multiple', type='string', action='store',
                    help="Allow output of multiple models")
    parser.add_option('-o', '--outfile', type='string', action='store',
                    help='Filename to use for output (- for stdout), default is'
                    ' %s/<name>.dtsi' % MICROCODE_DIR)
    parser.usage += """ command

    Process an Intel microcode file (use -h for help). Commands:

       create     Create microcode .dtsi file for a model
       list       List available models in microcode file
       license    Print the license

    Typical usage:

       ./tools/microcode-tool -d microcode.dat -m 306a create

    This will find the appropriate file and write it to %s.""" % MICROCODE_DIR

    (options, args) = parser.parse_args()
    if not args:
        parser.error('Please specify a command')
    cmd = args[0]
    if cmd not in commands:
        parser.error("Unknown command '%s'" % cmd)

    if (not not options.mcfile) != (not not options.mcfile):
        parser.error("You must specify either header files or a microcode file, not both")
    if options.headerfile:
        date, license_text, microcodes = ParseHeaderFiles(options.headerfile)
    elif options.mcfile:
        date, license_text, microcodes = ParseFile(options.mcfile)
    else:
        parser.error('You must specify a microcode file (or header files)')

    if cmd == 'list':
        List(date, microcodes, options.model)
    elif cmd == 'license':
        print('\n'.join(license_text))
    elif cmd == 'create':
        if not options.model:
            parser.error('You must specify a model to create')
        model = options.model.lower()
        if options.model == 'all':
            options.multiple = True
            mcode_list = list(microcodes.values())
            tried = []
        else:
            mcode_list, tried = FindMicrocode(microcodes, model)
        if not mcode_list:
            parser.error("Unknown model '%s' (%s) - try 'list' to list" %
                        (model, ', '.join(tried)))
        if not options.multiple and len(mcode_list) > 1:
            parser.error("Ambiguous model '%s' (%s) matched %s - try 'list' "
                        "to list or specify a particular file" %
                        (model, ', '.join(tried),
                        ', '.join([m.name for m in mcode_list])))
        CreateFile(date, license_text, mcode_list, options.outfile)
    else:
        parser.error("Unknown command '%s'" % cmd)

if __name__ == "__main__":
    MicrocodeTool()
