#!/usr/bin/python
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2017 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#

"""Scanning of U-Boot source for drivers and structs

This scans the source tree to find out things about all instances of
U_BOOT_DRIVER(), UCLASS_DRIVER and all struct declarations in header files.

See doc/driver-model/of-plat.rst for more informaiton
"""

import collections
import os
import re
import sys


def conv_name_to_c(name):
    """Convert a device-tree name to a C identifier

    This uses multiple replace() calls instead of re.sub() since it is faster
    (400ms for 1m calls versus 1000ms for the 're' version).

    Args:
        name (str): Name to convert
    Return:
        str: String containing the C version of this name
    """
    new = name.replace('@', '_at_')
    new = new.replace('-', '_')
    new = new.replace(',', '_')
    new = new.replace('.', '_')
    if new == '/':
        return 'root'
    return new

def get_compat_name(node):
    """Get the node's list of compatible string as a C identifiers

    Args:
        node (fdt.Node): Node object to check
    Return:
        list of str: List of C identifiers for all the compatible strings
    """
    compat = node.props['compatible'].value
    if not isinstance(compat, list):
        compat = [compat]
    return [conv_name_to_c(c) for c in compat]


class Driver:
    """Information about a driver in U-Boot

    Attributes:
        name: Name of driver. For U_BOOT_DRIVER(x) this is 'x'
        fname: Filename where the driver was found
        uclass_id: Name of uclass, e.g. 'UCLASS_I2C'
        compat: Driver data for each compatible string:
            key: Compatible string, e.g. 'rockchip,rk3288-grf'
            value: Driver data, e,g, 'ROCKCHIP_SYSCON_GRF', or None
        fname: Filename where the driver was found
        priv (str): struct name of the priv_auto member, e.g. 'serial_priv'
        plat (str): struct name of the plat_auto member, e.g. 'serial_plat'
        child_priv (str): struct name of the per_child_auto member,
            e.g. 'pci_child_priv'
        child_plat (str): struct name of the per_child_plat_auto member,
            e.g. 'pci_child_plat'
        used (bool): True if the driver is used by the structs being output
        phase (str): Which phase of U-Boot to use this driver
        headers (list): List of header files needed for this driver (each a str)
            e.g. ['<asm/cpu.h>']
        dups (list): Driver objects with the same name as this one, that were
            found after this one
        warn_dups (bool): True if the duplicates are not distinguisble using
            the phase
        uclass (Uclass): uclass for this driver
    """
    def __init__(self, name, fname):
        self.name = name
        self.fname = fname
        self.uclass_id = None
        self.compat = None
        self.priv = ''
        self.plat = ''
        self.child_priv = ''
        self.child_plat = ''
        self.used = False
        self.phase = ''
        self.headers = []
        self.dups = []
        self.warn_dups = False
        self.uclass = None

    def __eq__(self, other):
        return (self.name == other.name and
                self.uclass_id == other.uclass_id and
                self.compat == other.compat and
                self.priv == other.priv and
                self.plat == other.plat and
                self.used == other.used)

    def __repr__(self):
        return ("Driver(name='%s', used=%s, uclass_id='%s', compat=%s, priv=%s)" %
                (self.name, self.used, self.uclass_id, self.compat, self.priv))


class UclassDriver:
    """Holds information about a uclass driver

    Attributes:
        name: Uclass name, e.g. 'i2c' if the driver is for UCLASS_I2C
        uclass_id: Uclass ID, e.g. 'UCLASS_I2C'
        priv: struct name of the private data, e.g. 'i2c_priv'
        per_dev_priv (str): struct name of the priv_auto member, e.g. 'spi_info'
        per_dev_plat (str): struct name of the plat_auto member, e.g. 'i2c_chip'
        per_child_priv (str): struct name of the per_child_auto member,
            e.g. 'pci_child_priv'
        per_child_plat (str): struct name of the per_child_plat_auto member,
            e.g. 'pci_child_plat'
        alias_num_to_node (dict): Aliases for this uclasses (for sequence
                numbers)
            key (int): Alias number, e.g. 2 for "pci2"
            value (str): Node the alias points to
        alias_path_to_num (dict): Convert a path to an alias number
            key (str): Full path to node (e.g. '/soc/pci')
            seq (int): Alias number, e.g. 2 for "pci2"
        devs (list): List of devices in this uclass, each a Node
        node_refs (dict): References in the linked list of devices:
            key (int): Sequence number (0=first, n-1=last, -1=head, n=tail)
            value (str): Reference to the device at that position
    """
    def __init__(self, name):
        self.name = name
        self.uclass_id = None
        self.priv = ''
        self.per_dev_priv = ''
        self.per_dev_plat = ''
        self.per_child_priv = ''
        self.per_child_plat = ''
        self.alias_num_to_node = {}
        self.alias_path_to_num = {}
        self.devs = []
        self.node_refs = {}

    def __eq__(self, other):
        return (self.name == other.name and
                self.uclass_id == other.uclass_id and
                self.priv == other.priv)

    def __repr__(self):
        return ("UclassDriver(name='%s', uclass_id='%s')" %
                (self.name, self.uclass_id))

    def __hash__(self):
        # We can use the uclass ID since it is unique among uclasses
        return hash(self.uclass_id)


class Struct:
    """Holds information about a struct definition

    Attributes:
        name: Struct name, e.g. 'fred' if the struct is 'struct fred'
        fname: Filename containing the struct, in a format that C files can
            include, e.g. 'asm/clk.h'
    """
    def __init__(self, name, fname):
        self.name = name
        self.fname =fname

    def __repr__(self):
        return ("Struct(name='%s', fname='%s')" % (self.name, self.fname))


class Scanner:
    """Scanning of the U-Boot source tree

    Properties:
        _basedir (str): Base directory of U-Boot source code. Defaults to the
            grandparent of this file's directory
        _drivers: Dict of valid driver names found in drivers/
            key: Driver name
            value: Driver for that driver
        _driver_aliases: Dict that holds aliases for driver names
            key: Driver alias declared with
                DM_DRIVER_ALIAS(driver_alias, driver_name)
            value: Driver name declared with U_BOOT_DRIVER(driver_name)
        _drivers_additional (list or str): List of additional drivers to use
            during scanning
        _warnings: Dict of warnings found:
            key: Driver name
            value: Set of warnings
        _of_match: Dict holding information about compatible strings
            key: Name of struct udevice_id variable
            value: Dict of compatible info in that variable:
               key: Compatible string, e.g. 'rockchip,rk3288-grf'
               value: Driver data, e,g, 'ROCKCHIP_SYSCON_GRF', or None
        _compat_to_driver: Maps compatible strings to Driver
        _uclass: Dict of uclass information
            key: uclass name, e.g. 'UCLASS_I2C'
            value: UClassDriver
        _structs: Dict of all structs found in U-Boot:
            key: Name of struct
            value: Struct object
        _phase: The phase of U-Boot that we are generating data for, e.g. 'spl'
             or 'tpl'. None if not known
    """
    def __init__(self, basedir, drivers_additional, phase=''):
        """Set up a new Scanner
        """
        if not basedir:
            basedir = sys.argv[0].replace('tools/dtoc/dtoc', '')
            if basedir == '':
                basedir = './'
        self._basedir = basedir
        self._drivers = {}
        self._driver_aliases = {}
        self._drivers_additional = drivers_additional or []
        self._missing_drivers = set()
        self._warnings = collections.defaultdict(set)
        self._of_match = {}
        self._compat_to_driver = {}
        self._uclass = {}
        self._structs = {}
        self._phase = phase

    def get_driver(self, name):
        """Get a driver given its name

        Args:
            name (str): Driver name

        Returns:
            Driver: Driver or None if not found
        """
        return self._drivers.get(name)

    def get_normalized_compat_name(self, node):
        """Get a node's normalized compat name

        Returns a valid driver name by retrieving node's list of compatible
        string as a C identifier and performing a check against _drivers
        and a lookup in driver_aliases printing a warning in case of failure.

        Args:
            node (Node): Node object to check
        Return:
            Tuple:
                Driver name associated with the first compatible string
                List of C identifiers for all the other compatible strings
                    (possibly empty)
                In case of no match found, the return will be the same as
                get_compat_name()
        """
        if not node.parent:
            compat_list_c = ['root_driver']
        else:
            compat_list_c = get_compat_name(node)

        for compat_c in compat_list_c:
            if not compat_c in self._drivers.keys():
                compat_c = self._driver_aliases.get(compat_c)
                if not compat_c:
                    continue

            aliases_c = compat_list_c
            if compat_c in aliases_c:
                aliases_c.remove(compat_c)
            return compat_c, aliases_c

        name = compat_list_c[0]
        self._missing_drivers.add(name)
        self._warnings[name].add(
            'WARNING: the driver %s was not found in the driver list' % name)

        return compat_list_c[0], compat_list_c[1:]

    def _parse_structs(self, fname, buff):
        """Parse a H file to extract struct definitions contained within

        This parses 'struct xx {' definitions to figure out what structs this
        header defines.

        Args:
            buff (str): Contents of file
            fname (str): Filename (to use when printing errors)
        """
        structs = {}

        re_struct = re.compile(r'^struct ([a-z0-9_]+) {$')
        re_asm = re.compile(r'../arch/[a-z0-9]+/include/asm/(.*)')
        prefix = ''
        for line in buff.splitlines():
            # Handle line continuation
            if prefix:
                line = prefix + line
                prefix = ''
            if line.endswith('\\'):
                prefix = line[:-1]
                continue

            m_struct = re_struct.match(line)
            if m_struct:
                name = m_struct.group(1)
                include_dir = os.path.join(self._basedir, 'include')
                rel_fname = os.path.relpath(fname, include_dir)
                m_asm = re_asm.match(rel_fname)
                if m_asm:
                    rel_fname = 'asm/' + m_asm.group(1)
                structs[name] = Struct(name, rel_fname)
        self._structs.update(structs)

    @classmethod
    def _get_re_for_member(cls, member):
        """_get_re_for_member: Get a compiled regular expression

        Args:
            member (str): Struct member name, e.g. 'priv_auto'

        Returns:
            re.Pattern: Compiled regular expression that parses:

               .member = sizeof(struct fred),

            and returns "fred" as group 1
        """
        return re.compile(r'^\s*.%s\s*=\s*sizeof\(struct\s+(.*)\),$' % member)

    def _parse_uclass_driver(self, fname, buff):
        """Parse a C file to extract uclass driver information contained within

        This parses UCLASS_DRIVER() structs to obtain various pieces of useful
        information.

        It updates the following member:
            _uclass: Dict of uclass information
                key: uclass name, e.g. 'UCLASS_I2C'
                value: UClassDriver

        Args:
            fname (str): Filename being parsed (used for warnings)
            buff (str): Contents of file
        """
        uc_drivers = {}

        # Collect the driver name and associated Driver
        driver = None
        re_driver = re.compile(r'^UCLASS_DRIVER\((.*)\)')

        # Collect the uclass ID, e.g. 'UCLASS_SPI'
        re_id = re.compile(r'\s*\.id\s*=\s*(UCLASS_[A-Z0-9_]+)')

        # Matches the header/size information for uclass-private data
        re_priv = self._get_re_for_member('priv_auto')

        # Set up parsing for the auto members
        re_per_device_priv = self._get_re_for_member('per_device_auto')
        re_per_device_plat = self._get_re_for_member('per_device_plat_auto')
        re_per_child_priv = self._get_re_for_member('per_child_auto')
        re_per_child_plat = self._get_re_for_member('per_child_plat_auto')

        prefix = ''
        for line in buff.splitlines():
            # Handle line continuation
            if prefix:
                line = prefix + line
                prefix = ''
            if line.endswith('\\'):
                prefix = line[:-1]
                continue

            driver_match = re_driver.search(line)

            # If we have seen UCLASS_DRIVER()...
            if driver:
                m_id = re_id.search(line)
                m_priv = re_priv.match(line)
                m_per_dev_priv = re_per_device_priv.match(line)
                m_per_dev_plat = re_per_device_plat.match(line)
                m_per_child_priv = re_per_child_priv.match(line)
                m_per_child_plat = re_per_child_plat.match(line)
                if m_id:
                    driver.uclass_id = m_id.group(1)
                elif m_priv:
                    driver.priv = m_priv.group(1)
                elif m_per_dev_priv:
                    driver.per_dev_priv = m_per_dev_priv.group(1)
                elif m_per_dev_plat:
                    driver.per_dev_plat = m_per_dev_plat.group(1)
                elif m_per_child_priv:
                    driver.per_child_priv = m_per_child_priv.group(1)
                elif m_per_child_plat:
                    driver.per_child_plat = m_per_child_plat.group(1)
                elif '};' in line:
                    if not driver.uclass_id:
                        raise ValueError(
                            "%s: Cannot parse uclass ID in driver '%s'" %
                            (fname, driver.name))
                    uc_drivers[driver.uclass_id] = driver
                    driver = None

            elif driver_match:
                driver_name = driver_match.group(1)
                driver = UclassDriver(driver_name)

        self._uclass.update(uc_drivers)

    def _parse_driver(self, fname, buff):
        """Parse a C file to extract driver information contained within

        This parses U_BOOT_DRIVER() structs to obtain various pieces of useful
        information.

        It updates the following members:
            _drivers - updated with new Driver records for each driver found
                in the file
            _of_match - updated with each compatible string found in the file
            _compat_to_driver - Maps compatible string to Driver
            _driver_aliases - Maps alias names to driver name

        Args:
            fname (str): Filename being parsed (used for warnings)
            buff (str): Contents of file

        Raises:
            ValueError: Compatible variable is mentioned in .of_match in
                U_BOOT_DRIVER() but not found in the file
        """
        # Dict holding information about compatible strings collected in this
        # function so far
        #    key: Name of struct udevice_id variable
        #    value: Dict of compatible info in that variable:
        #       key: Compatible string, e.g. 'rockchip,rk3288-grf'
        #       value: Driver data, e,g, 'ROCKCHIP_SYSCON_GRF', or None
        of_match = {}

        # Dict holding driver information collected in this function so far
        #    key: Driver name (C name as in U_BOOT_DRIVER(xxx))
        #    value: Driver
        drivers = {}

        # Collect the driver info
        driver = None
        re_driver = re.compile(r'^U_BOOT_DRIVER\((.*)\)')

        # Collect the uclass ID, e.g. 'UCLASS_SPI'
        re_id = re.compile(r'\s*\.id\s*=\s*(UCLASS_[A-Z0-9_]+)')

        # Collect the compatible string, e.g. 'rockchip,rk3288-grf'
        compat = None
        re_compat = re.compile(r'{\s*\.compatible\s*=\s*"(.*)"\s*'
                               r'(,\s*\.data\s*=\s*(\S*))?\s*},')

        # This is a dict of compatible strings that were found:
        #    key: Compatible string, e.g. 'rockchip,rk3288-grf'
        #    value: Driver data, e,g, 'ROCKCHIP_SYSCON_GRF', or None
        compat_dict = {}

        # Holds the var nane of the udevice_id list, e.g.
        # 'rk3288_syscon_ids_noc' in
        # static const struct udevice_id rk3288_syscon_ids_noc[] = {
        ids_name = None
        re_ids = re.compile(r'struct udevice_id (.*)\[\]\s*=')

        # Matches the references to the udevice_id list
        re_of_match = re.compile(
            r'\.of_match\s*=\s*(of_match_ptr\()?([a-z0-9_]+)([^,]*),')

        re_phase = re.compile(r'^\s*DM_PHASE\((.*)\).*$')
        re_hdr = re.compile(r'^\s*DM_HEADER\((.*)\).*$')
        re_alias = re.compile(r'DM_DRIVER_ALIAS\(\s*(\w+)\s*,\s*(\w+)\s*\)')

        # Matches the struct name for priv, plat
        re_priv = self._get_re_for_member('priv_auto')
        re_plat = self._get_re_for_member('plat_auto')
        re_child_priv = self._get_re_for_member('per_child_auto')
        re_child_plat = self._get_re_for_member('per_child_plat_auto')

        prefix = ''
        for line in buff.splitlines():
            # Handle line continuation
            if prefix:
                line = prefix + line
                prefix = ''
            if line.endswith('\\'):
                prefix = line[:-1]
                continue

            driver_match = re_driver.search(line)

            # If this line contains U_BOOT_DRIVER()...
            if driver:
                m_id = re_id.search(line)
                m_of_match = re_of_match.search(line)
                m_priv = re_priv.match(line)
                m_plat = re_plat.match(line)
                m_cplat = re_child_plat.match(line)
                m_cpriv = re_child_priv.match(line)
                m_phase = re_phase.match(line)
                m_hdr = re_hdr.match(line)
                if m_priv:
                    driver.priv = m_priv.group(1)
                elif m_plat:
                    driver.plat = m_plat.group(1)
                elif m_cplat:
                    driver.child_plat = m_cplat.group(1)
                elif m_cpriv:
                    driver.child_priv = m_cpriv.group(1)
                elif m_id:
                    driver.uclass_id = m_id.group(1)
                elif m_of_match:
                    compat = m_of_match.group(2)
                    suffix = m_of_match.group(3)
                    if suffix and suffix != ')':
                        self._warnings[driver.name].add(
                            "%s: Warning: unexpected suffix '%s' on .of_match line for compat '%s'" %
                            (fname, suffix, compat))
                elif m_phase:
                    driver.phase = m_phase.group(1)
                elif m_hdr:
                    driver.headers.append(m_hdr.group(1))
                elif '};' in line:
                    is_root = driver.name == 'root_driver'
                    if driver.uclass_id and (compat or is_root):
                        if not is_root:
                            if compat not in of_match:
                                raise ValueError(
                                    "%s: Unknown compatible var '%s' (found: %s)" %
                                    (fname, compat, ','.join(of_match.keys())))
                            driver.compat = of_match[compat]

                            # This needs to be deterministic, since a driver may
                            # have multiple compatible strings pointing to it.
                            # We record the one earliest in the alphabet so it
                            # will produce the same result on all machines.
                            for compat_id in of_match[compat]:
                                old = self._compat_to_driver.get(compat_id)
                                if not old or driver.name < old.name:
                                    self._compat_to_driver[compat_id] = driver
                        drivers[driver.name] = driver
                    else:
                        # The driver does not have a uclass or compat string.
                        # The first is required but the second is not, so just
                        # ignore this.
                        if not driver.uclass_id:
                            warn = 'Missing .uclass'
                        else:
                            warn = 'Missing .compatible'
                        self._warnings[driver.name].add('%s in %s' %
                                                        (warn, fname))
                    driver = None
                    ids_name = None
                    compat = None
                    compat_dict = {}

            elif ids_name:
                compat_m = re_compat.search(line)
                if compat_m:
                    compat_dict[compat_m.group(1)] = compat_m.group(3)
                elif '};' in line:
                    of_match[ids_name] = compat_dict
                    ids_name = None
            elif driver_match:
                driver_name = driver_match.group(1)
                driver = Driver(driver_name, fname)
            else:
                ids_m = re_ids.search(line)
                m_alias = re_alias.match(line)
                if ids_m:
                    ids_name = ids_m.group(1)
                elif m_alias:
                    self._driver_aliases[m_alias.group(2)] = m_alias.group(1)

        # Make the updates based on what we found
        for driver in drivers.values():
            if driver.name in self._drivers:
                orig = self._drivers[driver.name]
                if self._phase:
                    # If the original driver matches our phase, use it
                    if orig.phase == self._phase:
                        orig.dups.append(driver)
                        continue

                    # Otherwise use the new driver, which is assumed to match
                else:
                    # We have no way of distinguishing them
                    driver.warn_dups = True
                driver.dups.append(orig)
            self._drivers[driver.name] = driver
        self._of_match.update(of_match)

    def show_warnings(self):
        """Show any warnings that have been collected"""
        used_drivers = [drv.name for drv in self._drivers.values() if drv.used]
        missing = self._missing_drivers.copy()
        for name in sorted(self._warnings.keys()):
            if name in missing or name in used_drivers:
                warns = sorted(list(self._warnings[name]))
                print('%s: %s' % (name, warns[0]))
                indent = ' ' * len(name)
                for warn in warns[1:]:
                    print('%-s: %s' % (indent, warn))
                if name in missing:
                    missing.remove(name)
                print()

    def scan_driver(self, fname):
        """Scan a driver file to build a list of driver names and aliases

        It updates the following members:
            _drivers - updated with new Driver records for each driver found
                in the file
            _of_match - updated with each compatible string found in the file
            _compat_to_driver - Maps compatible string to Driver
            _driver_aliases - Maps alias names to driver name

        Args
            fname: Driver filename to scan
        """
        with open(fname, encoding='utf-8') as inf:
            try:
                buff = inf.read()
            except UnicodeDecodeError:
                # This seems to happen on older Python versions
                print("Skipping file '%s' due to unicode error" % fname)
                return

            # If this file has any U_BOOT_DRIVER() declarations, process it to
            # obtain driver information
            if 'U_BOOT_DRIVER' in buff:
                self._parse_driver(fname, buff)
            if 'UCLASS_DRIVER' in buff:
                self._parse_uclass_driver(fname, buff)

    def scan_header(self, fname):
        """Scan a header file to build a list of struct definitions

        It updates the following members:
            _structs - updated with new Struct records for each struct found
                in the file

        Args
            fname: header filename to scan
        """
        with open(fname, encoding='utf-8') as inf:
            try:
                buff = inf.read()
            except UnicodeDecodeError:
                # This seems to happen on older Python versions
                print("Skipping file '%s' due to unicode error" % fname)
                return

            # If this file has any U_BOOT_DRIVER() declarations, process it to
            # obtain driver information
            if 'struct' in buff:
                self._parse_structs(fname, buff)

    def scan_drivers(self):
        """Scan the driver folders to build a list of driver names and aliases

        This procedure will populate self._drivers and self._driver_aliases
        """
        for (dirpath, _, filenames) in os.walk(self._basedir):
            rel_path = dirpath[len(self._basedir):]
            if rel_path.startswith('/'):
                rel_path = rel_path[1:]
            if rel_path.startswith('build') or rel_path.startswith('.git'):
                continue
            for fname in filenames:
                pathname = dirpath + '/' + fname
                if fname.endswith('.c'):
                    self.scan_driver(pathname)
                elif fname.endswith('.h'):
                    self.scan_header(pathname)
        for fname in self._drivers_additional:
            if not isinstance(fname, str) or len(fname) == 0:
                continue
            if fname[0] == '/':
                self.scan_driver(fname)
            else:
                self.scan_driver(self._basedir + '/' + fname)

        # Get the uclass for each driver
        # TODO: Can we just get the uclass for the ones we use, e.g. in
        # mark_used()?
        for driver in self._drivers.values():
            driver.uclass = self._uclass.get(driver.uclass_id)

    def mark_used(self, nodes):
        """Mark the drivers associated with a list of nodes as 'used'

        This takes a list of nodes, finds the driver for each one and marks it
        as used.

        If two used drivers have the same name, issue a warning.

        Args:
            nodes (list of None): Nodes that are in use
        """
        # Figure out which drivers we actually use
        for node in nodes:
            struct_name, _ = self.get_normalized_compat_name(node)
            driver = self._drivers.get(struct_name)
            if driver:
                driver.used = True
                if driver.dups and driver.warn_dups:
                    print("Warning: Duplicate driver name '%s' (orig=%s, dups=%s)" %
                          (driver.name, driver.fname,
                           ', '.join([drv.fname for drv in driver.dups])))

    def add_uclass_alias(self, name, num, node):
        """Add an alias to a uclass

        Args:
            name: Name of uclass, e.g. 'i2c'
            num: Alias number, e.g. 2 for alias 'i2c2'
            node: Node the alias points to, or None if None

        Returns:
            True if the node was added
            False if the node was not added (uclass of that name not found)
            None if the node could not be added because it was None
        """
        for uclass in self._uclass.values():
            if uclass.name == name:
                if node is None:
                    return None
                uclass.alias_num_to_node[int(num)] = node
                uclass.alias_path_to_num[node.path] = int(num)
                return True
        return False

    def assign_seq(self, node):
        """Figure out the sequence number for a node

        This looks in the node's uclass and assigns a sequence number if needed,
        based on the aliases and other nodes in that uclass.

        It updates the uclass alias_path_to_num and alias_num_to_node

        Args:
            node (Node): Node object to look up
        """
        if node.driver and node.seq == -1 and node.uclass:
            uclass = node.uclass
            num = uclass.alias_path_to_num.get(node.path)
            if num is not None:
                return num
            else:
                # Dynamically allocate the next available value after all
                # existing ones
                if uclass.alias_num_to_node:
                    start = max(uclass.alias_num_to_node.keys())
                else:
                    start = -1
                for seq in range(start + 1, 1000):
                    if seq not in uclass.alias_num_to_node:
                        break
                uclass.alias_path_to_num[node.path] = seq
                uclass.alias_num_to_node[seq] = node
                return seq
        return None
