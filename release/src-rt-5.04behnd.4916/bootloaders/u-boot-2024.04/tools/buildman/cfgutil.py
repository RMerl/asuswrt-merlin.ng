# SPDX-License-Identifier: GPL-2.0+
# Copyright 2022 Google LLC
# Written by Simon Glass <sjg@chromium.org>
#

"""Utility functions for dealing with Kconfig .confing files"""

import re

from u_boot_pylib import tools

RE_LINE = re.compile(r'(# )?CONFIG_([A-Z0-9_]+)(=(.*)| is not set)')
RE_CFG = re.compile(r'(~?)(CONFIG_)?([A-Z0-9_]+)(=.*)?')

def make_cfg_line(opt, adj):
    """Make a new config line for an option

    Args:
        opt (str): Option to process, without CONFIG_ prefix
        adj (str): Adjustment to make (C is config option without prefix):
             C to enable C
             ~C to disable C
             C=val to set the value of C (val must have quotes if C is
                 a string Kconfig)

    Returns:
        str: New line to use, one of:
            CONFIG_opt=y               - option is enabled
            # CONFIG_opt is not set    - option is disabled
            CONFIG_opt=val             - option is getting a new value (val is
                in quotes if this is a string)
    """
    if adj[0] == '~':
        return f'# CONFIG_{opt} is not set'
    if '=' in adj:
        return f'CONFIG_{adj}'
    return f'CONFIG_{opt}=y'

def adjust_cfg_line(line, adjust_cfg, done=None):
    """Make an adjustment to a single of line from a .config file

    This processes a .config line, producing a new line if a change for this
    CONFIG is requested in adjust_cfg

    Args:
        line (str): line to process, e.g. '# CONFIG_FRED is not set' or
            'CONFIG_FRED=y' or 'CONFIG_FRED=0x123' or 'CONFIG_FRED="fred"'
        adjust_cfg (dict of str): Changes to make to .config file before
                building:
             key: str config to change, without the CONFIG_ prefix, e.g.
                 FRED
             value: str change to make (C is config option without prefix):
                 C to enable C
                 ~C to disable C
                 C=val to set the value of C (val must have quotes if C is
                     a string Kconfig)
        done (set of set): Adds the config option to this set if it is changed
            in some way. This is used to track which ones have been processed.
            None to skip.

    Returns:
        tuple:
            str: New string for this line (maybe unchanged)
            str: Adjustment string that was used
    """
    out_line = line
    m_line = RE_LINE.match(line)
    adj = None
    if m_line:
        _, opt, _, _ = m_line.groups()
        adj = adjust_cfg.get(opt)
        if adj:
            out_line = make_cfg_line(opt, adj)
            if done is not None:
                done.add(opt)

    return out_line, adj

def adjust_cfg_lines(lines, adjust_cfg):
    """Make adjustments to a list of lines from a .config file

    Args:
        lines (list of str): List of lines to process
        adjust_cfg (dict of str): Changes to make to .config file before
                building:
             key: str config to change, without the CONFIG_ prefix, e.g.
                 FRED
             value: str change to make (C is config option without prefix):
                 C to enable C
                 ~C to disable C
                 C=val to set the value of C (val must have quotes if C is
                     a string Kconfig)

    Returns:
        list of str: New list of lines resulting from the processing
    """
    out_lines = []
    done = set()
    for line in lines:
        out_line, _ = adjust_cfg_line(line, adjust_cfg, done)
        out_lines.append(out_line)

    for opt in adjust_cfg:
        if opt not in done:
            adj = adjust_cfg.get(opt)
            out_line = make_cfg_line(opt, adj)
            out_lines.append(out_line)

    return out_lines

def adjust_cfg_file(fname, adjust_cfg):
    """Make adjustments to a .config file

    Args:
        fname (str): Filename of .config file to change
        adjust_cfg (dict of str): Changes to make to .config file before
                building:
             key: str config to change, without the CONFIG_ prefix, e.g.
                 FRED
             value: str change to make (C is config option without prefix):
                 C to enable C
                 ~C to disable C
                 C=val to set the value of C (val must have quotes if C is
                     a string Kconfig)
    """
    lines = tools.read_file(fname, binary=False).splitlines()
    out_lines = adjust_cfg_lines(lines, adjust_cfg)
    out = '\n'.join(out_lines) + '\n'
    tools.write_file(fname, out, binary=False)

def convert_list_to_dict(adjust_cfg_list):
    """Convert a list of config changes into the dict used by adjust_cfg_file()

    Args:
        adjust_cfg_list (list of str): List of changes to make to .config file
            before building. Each is one of (where C is the config option with
            or without the CONFIG_ prefix)

                C to enable C
                ~C to disable C
                C=val to set the value of C (val must have quotes if C is
                    a string Kconfig

    Returns:
        dict of str: Changes to make to .config file before building:
             key: str config to change, without the CONFIG_ prefix, e.g. FRED
             value: str change to make (C is config option without prefix):
                 C to enable C
                 ~C to disable C
                 C=val to set the value of C (val must have quotes if C is
                     a string Kconfig)

    Raises:
        ValueError: if an item in adjust_cfg_list has invalid syntax
    """
    result = {}
    for cfg in adjust_cfg_list or []:
        m_cfg = RE_CFG.match(cfg)
        if not m_cfg:
            raise ValueError(f"Invalid CONFIG adjustment '{cfg}'")
        negate, _, opt, val = m_cfg.groups()
        result[opt] = f'%s{opt}%s' % (negate or '', val or '')

    return result

def check_cfg_lines(lines, adjust_cfg):
    """Check that lines do not conflict with the requested changes

    If a line enables a CONFIG which was requested to be disabled, etc., then
    this is an error. This function finds such errors.

    Args:
        lines (list of str): List of lines to process
        adjust_cfg (dict of str): Changes to make to .config file before
                building:
             key: str config to change, without the CONFIG_ prefix, e.g.
                 FRED
             value: str change to make (C is config option without prefix):
                 C to enable C
                 ~C to disable C
                 C=val to set the value of C (val must have quotes if C is
                     a string Kconfig)

    Returns:
        list of tuple: list of errors, each a tuple:
            str: cfg adjustment requested
            str: line of the config that conflicts
    """
    bad = []
    done = set()
    for line in lines:
        out_line, adj = adjust_cfg_line(line, adjust_cfg, done)
        if out_line != line:
            bad.append([adj, line])

    for opt in adjust_cfg:
        if opt not in done:
            adj = adjust_cfg.get(opt)
            out_line = make_cfg_line(opt, adj)
            bad.append([adj, f'Missing expected line: {out_line}'])

    return bad

def check_cfg_file(fname, adjust_cfg):
    """Check that a config file has been adjusted according to adjust_cfg

    Args:
        fname (str): Filename of .config file to change
        adjust_cfg (dict of str): Changes to make to .config file before
                building:
             key: str config to change, without the CONFIG_ prefix, e.g.
                 FRED
             value: str change to make (C is config option without prefix):
                 C to enable C
                 ~C to disable C
                 C=val to set the value of C (val must have quotes if C is
                     a string Kconfig)

    Returns:
        str: None if OK, else an error string listing the problems
    """
    lines = tools.read_file(fname, binary=False).splitlines()
    bad_cfgs = check_cfg_lines(lines, adjust_cfg)
    if bad_cfgs:
        out = [f'{cfg:20}  {line}' for cfg, line in bad_cfgs]
        content = '\\n'.join(out)
        return f'''
Some CONFIG adjustments did not take effect. This may be because
the request CONFIGs do not exist or conflict with others.

Failed adjustments:

{content}
'''
    return None
