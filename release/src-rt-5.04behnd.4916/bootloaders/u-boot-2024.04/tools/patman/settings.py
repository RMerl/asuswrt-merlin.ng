# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2011 The Chromium OS Authors.
# Copyright (c) 2022 Maxim Cournoyer <maxim.cournoyer@savoirfairelinux.com>
#

try:
    import configparser as ConfigParser
except Exception:
    import ConfigParser

import argparse
import os
import re

from patman import gitutil

"""Default settings per-project.

These are used by _ProjectConfigParser.  Settings names should match
the "dest" of the option parser from patman.py.
"""
_default_settings = {
    "u-boot": {},
    "linux": {
        "process_tags": "False",
        "check_patch_use_tree": "True",
    },
    "gcc": {
        "process_tags": "False",
        "add_signoff": "False",
        "check_patch": "False",
    },
}


class _ProjectConfigParser(ConfigParser.ConfigParser):
    """ConfigParser that handles projects.

    There are two main goals of this class:
    - Load project-specific default settings.
    - Merge general default settings/aliases with project-specific ones.

    # Sample config used for tests below...
    >>> from io import StringIO
    >>> sample_config = '''
    ... [alias]
    ... me: Peter P. <likesspiders@example.com>
    ... enemies: Evil <evil@example.com>
    ...
    ... [sm_alias]
    ... enemies: Green G. <ugly@example.com>
    ...
    ... [sm2_alias]
    ... enemies: Doc O. <pus@example.com>
    ...
    ... [settings]
    ... am_hero: True
    ... '''

    # Check to make sure that bogus project gets general alias.
    >>> config = _ProjectConfigParser("zzz")
    >>> config.readfp(StringIO(sample_config))
    >>> str(config.get("alias", "enemies"))
    'Evil <evil@example.com>'

    # Check to make sure that alias gets overridden by project.
    >>> config = _ProjectConfigParser("sm")
    >>> config.readfp(StringIO(sample_config))
    >>> str(config.get("alias", "enemies"))
    'Green G. <ugly@example.com>'

    # Check to make sure that settings get merged with project.
    >>> config = _ProjectConfigParser("linux")
    >>> config.readfp(StringIO(sample_config))
    >>> sorted((str(a), str(b)) for (a, b) in config.items("settings"))
    [('am_hero', 'True'), ('check_patch_use_tree', 'True'), ('process_tags', 'False')]

    # Check to make sure that settings works with unknown project.
    >>> config = _ProjectConfigParser("unknown")
    >>> config.readfp(StringIO(sample_config))
    >>> sorted((str(a), str(b)) for (a, b) in config.items("settings"))
    [('am_hero', 'True')]
    """
    def __init__(self, project_name):
        """Construct _ProjectConfigParser.

        In addition to standard ConfigParser initialization, this also
        loads project defaults.

        Args:
            project_name: The name of the project.
        """
        self._project_name = project_name
        ConfigParser.ConfigParser.__init__(self)

        # Update the project settings in the config based on
        # the _default_settings global.
        project_settings = "%s_settings" % project_name
        if not self.has_section(project_settings):
            self.add_section(project_settings)
        project_defaults = _default_settings.get(project_name, {})
        for setting_name, setting_value in project_defaults.items():
            self.set(project_settings, setting_name, setting_value)

    def get(self, section, option, *args, **kwargs):
        """Extend ConfigParser to try project_section before section.

        Args:
            See ConfigParser.
        Returns:
            See ConfigParser.
        """
        try:
            val = ConfigParser.ConfigParser.get(
                self, "%s_%s" % (self._project_name, section), option,
                *args, **kwargs
            )
        except (ConfigParser.NoSectionError, ConfigParser.NoOptionError):
            val = ConfigParser.ConfigParser.get(
                self, section, option, *args, **kwargs
            )
        return val

    def items(self, section, *args, **kwargs):
        """Extend ConfigParser to add project_section to section.

        Args:
            See ConfigParser.
        Returns:
            See ConfigParser.
        """
        project_items = []
        has_project_section = False
        top_items = []

        # Get items from the project section
        try:
            project_items = ConfigParser.ConfigParser.items(
                self, "%s_%s" % (self._project_name, section), *args, **kwargs
            )
            has_project_section = True
        except ConfigParser.NoSectionError:
            pass

        # Get top-level items
        try:
            top_items = ConfigParser.ConfigParser.items(
                self, section, *args, **kwargs
            )
        except ConfigParser.NoSectionError:
            # If neither section exists raise the error on...
            if not has_project_section:
                raise

        item_dict = dict(top_items)
        item_dict.update(project_items)
        return {(item, val) for item, val in item_dict.items()}


def ReadGitAliases(fname):
    """Read a git alias file. This is in the form used by git:

    alias uboot  u-boot@lists.denx.de
    alias wd     Wolfgang Denk <wd@denx.de>

    Args:
        fname: Filename to read
    """
    try:
        fd = open(fname, 'r', encoding='utf-8')
    except IOError:
        print("Warning: Cannot find alias file '%s'" % fname)
        return

    re_line = re.compile(r'alias\s+(\S+)\s+(.*)')
    for line in fd.readlines():
        line = line.strip()
        if not line or line[0] == '#':
            continue

        m = re_line.match(line)
        if not m:
            print("Warning: Alias file line '%s' not understood" % line)
            continue

        list = alias.get(m.group(1), [])
        for item in m.group(2).split(','):
            item = item.strip()
            if item:
                list.append(item)
        alias[m.group(1)] = list

    fd.close()


def CreatePatmanConfigFile(config_fname):
    """Creates a config file under $(HOME)/.patman if it can't find one.

    Args:
        config_fname: Default config filename i.e., $(HOME)/.patman

    Returns:
        None
    """
    name = gitutil.get_default_user_name()
    if name is None:
        name = input("Enter name: ")

    email = gitutil.get_default_user_email()

    if email is None:
        email = input("Enter email: ")

    try:
        f = open(config_fname, 'w')
    except IOError:
        print("Couldn't create patman config file\n")
        raise

    print('''[alias]
me: %s <%s>

[bounces]
nxp = Zhikang Zhang <zhikang.zhang@nxp.com>
''' % (name, email), file=f)
    f.close()


def _UpdateDefaults(main_parser, config):
    """Update the given OptionParser defaults based on config.

    We'll walk through all of the settings from all parsers.
    For each setting we'll look for a default in the option parser.
    If it's found we'll update the option parser default.

    The idea here is that the .patman file should be able to update
    defaults but that command line flags should still have the final
    say.

    Args:
        parser: An instance of an ArgumentParser whose defaults will be
            updated.
        config: An instance of _ProjectConfigParser that we will query
            for settings.
    """
    # Find all the parsers and subparsers
    parsers = [main_parser]
    parsers += [subparser for action in main_parser._actions
                if isinstance(action, argparse._SubParsersAction)
                for _, subparser in action.choices.items()]

    # Collect the defaults from each parser
    defaults = {}
    parser_defaults = []
    for parser in parsers:
        pdefs = parser.parse_known_args()[0]
        parser_defaults.append(pdefs)
        defaults.update(vars(pdefs))

    # Go through the settings and collect defaults
    for name, val in config.items('settings'):
        if name in defaults:
            default_val = defaults[name]
            if isinstance(default_val, bool):
                val = config.getboolean('settings', name)
            elif isinstance(default_val, int):
                val = config.getint('settings', name)
            elif isinstance(default_val, str):
                val = config.get('settings', name)
            defaults[name] = val
        else:
            print("WARNING: Unknown setting %s" % name)

    # Set all the defaults and manually propagate them to subparsers
    main_parser.set_defaults(**defaults)
    for parser, pdefs in zip(parsers, parser_defaults):
        parser.set_defaults(**{k: v for k, v in defaults.items()
                               if k in pdefs})


def _ReadAliasFile(fname):
    """Read in the U-Boot git alias file if it exists.

    Args:
        fname: Filename to read.
    """
    if os.path.exists(fname):
        bad_line = None
        with open(fname, encoding='utf-8') as fd:
            linenum = 0
            for line in fd:
                linenum += 1
                line = line.strip()
                if not line or line.startswith('#'):
                    continue
                words = line.split(None, 2)
                if len(words) < 3 or words[0] != 'alias':
                    if not bad_line:
                        bad_line = "%s:%d:Invalid line '%s'" % (fname, linenum,
                                                                line)
                    continue
                alias[words[1]] = [s.strip() for s in words[2].split(',')]
        if bad_line:
            print(bad_line)


def _ReadBouncesFile(fname):
    """Read in the bounces file if it exists

    Args:
        fname: Filename to read.
    """
    if os.path.exists(fname):
        with open(fname) as fd:
            for line in fd:
                if line.startswith('#'):
                    continue
                bounces.add(line.strip())


def GetItems(config, section):
    """Get the items from a section of the config.

    Args:
        config: _ProjectConfigParser object containing settings
        section: name of section to retrieve

    Returns:
        List of (name, value) tuples for the section
    """
    try:
        return config.items(section)
    except ConfigParser.NoSectionError:
        return []


def Setup(parser, project_name, config_fname=None):
    """Set up the settings module by reading config files.

    Unless `config_fname` is specified, a `.patman` config file local
    to the git repository is consulted, followed by the global
    `$HOME/.patman`. If none exists, the later is created. Values
    defined in the local config file take precedence over those
    defined in the global one.

    Args:
        parser:         The parser to update.
        project_name:   Name of project that we're working on; we'll look
            for sections named "project_section" as well.
        config_fname:   Config filename to read.  An error is raised if it
            does not exist.
    """
    # First read the git alias file if available
    _ReadAliasFile('doc/git-mailrc')
    config = _ProjectConfigParser(project_name)

    if config_fname and not os.path.exists(config_fname):
        raise Exception(f'provided {config_fname} does not exist')

    if not config_fname:
        config_fname = '%s/.patman' % os.getenv('HOME')
    has_config = os.path.exists(config_fname)

    git_local_config_fname = os.path.join(gitutil.get_top_level(), '.patman')
    has_git_local_config = os.path.exists(git_local_config_fname)

    # Read the git local config last, so that its values override
    # those of the global config, if any.
    if has_config:
        config.read(config_fname)
    if has_git_local_config:
        config.read(git_local_config_fname)

    if not (has_config or has_git_local_config):
        print("No config file found.\nCreating ~/.patman...\n")
        CreatePatmanConfigFile(config_fname)

    for name, value in GetItems(config, 'alias'):
        alias[name] = value.split(',')

    _ReadBouncesFile('doc/bounces')
    for name, value in GetItems(config, 'bounces'):
        bounces.add(value)

    _UpdateDefaults(parser, config)


# These are the aliases we understand, indexed by alias. Each member is a list.
alias = {}
bounces = set()

if __name__ == "__main__":
    import doctest

    doctest.testmod()
