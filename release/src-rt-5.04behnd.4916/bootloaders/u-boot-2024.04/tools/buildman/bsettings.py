# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2012 The Chromium OS Authors.

import configparser
import os
import io

config_fname = None

def setup(fname=''):
    """Set up the buildman settings module by reading config files

    Args:
        config_fname:   Config filename to read ('' for default)
    """
    global settings
    global config_fname

    settings = configparser.ConfigParser()
    if fname is not None:
        config_fname = fname
        if config_fname == '':
            config_fname = '%s/.buildman' % os.getenv('HOME')
        if not os.path.exists(config_fname):
            print('No config file found ~/.buildman\nCreating one...\n')
            create_buildman_config_file(config_fname)
            print('To install tool chains, please use the --fetch-arch option')
        if config_fname:
            settings.read(config_fname)

def add_file(data):
    settings.readfp(io.StringIO(data))

def get_items(section):
    """Get the items from a section of the config.

    Args:
        section: name of section to retrieve

    Returns:
        List of (name, value) tuples for the section
    """
    try:
        return settings.items(section)
    except configparser.NoSectionError as e:
        return []
    except:
        raise

def get_global_item_value(name):
    """Get an item from the 'global' section of the config.

    Args:
        name: name of item to retrieve

    Returns:
        str: Value of item, or None if not present
    """
    return settings.get('global', name, fallback=None)

def set_item(section, tag, value):
    """Set an item and write it back to the settings file"""
    global settings
    global config_fname

    settings.set(section, tag, value)
    if config_fname is not None:
        with open(config_fname, 'w') as fd:
            settings.write(fd)

def create_buildman_config_file(config_fname):
    """Creates a new config file with no tool chain information.

    Args:
        config_fname: Config filename to create

    Returns:
        None
    """
    try:
        f = open(config_fname, 'w')
    except IOError:
        print("Couldn't create buildman config file '%s'\n" % config_fname)
        raise

    print('''[toolchain]
# name = path
# e.g. x86 = /opt/gcc-4.6.3-nolibc/x86_64-linux
other = /

[toolchain-prefix]
# name = path to prefix
# e.g. x86 = /opt/gcc-4.6.3-nolibc/x86_64-linux/bin/x86_64-linux-

[toolchain-alias]
# arch = alias
# Indicates which toolchain should be used to build for that arch
riscv = riscv32
sh = sh4
x86 = i386

[make-flags]
# Special flags to pass to 'make' for certain boards, e.g. to pass a test
# flag and build tag to snapper boards:
# snapper-boards=ENABLE_AT91_TEST=1
# snapper9260=${snapper-boards} BUILD_TAG=442
# snapper9g45=${snapper-boards} BUILD_TAG=443
''', file=f)
    f.close();
