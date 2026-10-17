# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2011 The Chromium OS Authors.
#

import collections
import concurrent.futures
import os
import re
import sys

from patman import gitutil
from u_boot_pylib import command
from u_boot_pylib import terminal

EMACS_PREFIX = r'(?:[0-9]{4}.*\.patch:[0-9]+: )?'
TYPE_NAME = r'([A-Z_]+:)?'
RE_ERROR = re.compile(r'ERROR:%s (.*)' % TYPE_NAME)
RE_WARNING = re.compile(EMACS_PREFIX + r'WARNING:%s (.*)' % TYPE_NAME)
RE_CHECK = re.compile(r'CHECK:%s (.*)' % TYPE_NAME)
RE_FILE = re.compile(r'#(\d+): (FILE: ([^:]*):(\d+):)?')
RE_NOTE = re.compile(r'NOTE: (.*)')


def find_check_patch():
    top_level = gitutil.get_top_level()
    try_list = [
        os.getcwd(),
        os.path.join(os.getcwd(), '..', '..'),
        os.path.join(top_level, 'tools'),
        os.path.join(top_level, 'scripts'),
        '%s/bin' % os.getenv('HOME'),
        ]
    # Look in current dir
    for path in try_list:
        fname = os.path.join(path, 'checkpatch.pl')
        if os.path.isfile(fname):
            return fname

    # Look upwwards for a Chrome OS tree
    while not os.path.ismount(path):
        fname = os.path.join(path, 'src', 'third_party', 'kernel', 'files',
                'scripts', 'checkpatch.pl')
        if os.path.isfile(fname):
            return fname
        path = os.path.dirname(path)

    sys.exit('Cannot find checkpatch.pl - please put it in your ' +
             '~/bin directory or use --no-check')


def check_patch_parse_one_message(message):
    """Parse one checkpatch message

    Args:
        message: string to parse

    Returns:
        dict:
            'type'; error or warning
            'msg': text message
            'file' : filename
            'line': line number
    """

    if RE_NOTE.match(message):
        return {}

    item = {}

    err_match = RE_ERROR.match(message)
    warn_match = RE_WARNING.match(message)
    check_match = RE_CHECK.match(message)
    if err_match:
        item['cptype'] = err_match.group(1)
        item['msg'] = err_match.group(2)
        item['type'] = 'error'
    elif warn_match:
        item['cptype'] = warn_match.group(1)
        item['msg'] = warn_match.group(2)
        item['type'] = 'warning'
    elif check_match:
        item['cptype'] = check_match.group(1)
        item['msg'] = check_match.group(2)
        item['type'] = 'check'
    else:
        message_indent = '    '
        print('patman: failed to parse checkpatch message:\n%s' %
              (message_indent + message.replace('\n', '\n' + message_indent)),
              file=sys.stderr)
        return {}

    file_match = RE_FILE.search(message)
    # some messages have no file, catch those here
    no_file_match = any(s in message for s in [
        '\nSubject:', 'Missing Signed-off-by: line(s)',
        'does MAINTAINERS need updating'
    ])

    if file_match:
        err_fname = file_match.group(3)
        if err_fname:
            item['file'] = err_fname
            item['line'] = int(file_match.group(4))
        else:
            item['file'] = '<patch>'
            item['line'] = int(file_match.group(1))
    elif no_file_match:
        item['file'] = '<patch>'
    else:
        message_indent = '    '
        print('patman: failed to find file / line information:\n%s' %
              (message_indent + message.replace('\n', '\n' + message_indent)),
              file=sys.stderr)

    return item


def check_patch_parse(checkpatch_output, verbose=False):
    """Parse checkpatch.pl output

    Args:
        checkpatch_output: string to parse
        verbose: True to print out every line of the checkpatch output as it is
            parsed

    Returns:
        namedtuple containing:
            ok: False=failure, True=ok
            problems (list of problems): each a dict:
                'type'; error or warning
                'msg': text message
                'file' : filename
                'line': line number
            errors: Number of errors
            warnings: Number of warnings
            checks: Number of checks
            lines: Number of lines
            stdout: checkpatch_output
    """
    fields = ['ok', 'problems', 'errors', 'warnings', 'checks', 'lines',
              'stdout']
    result = collections.namedtuple('CheckPatchResult', fields)
    result.stdout = checkpatch_output
    result.ok = False
    result.errors, result.warnings, result.checks = 0, 0, 0
    result.lines = 0
    result.problems = []

    # total: 0 errors, 0 warnings, 159 lines checked
    # or:
    # total: 0 errors, 2 warnings, 7 checks, 473 lines checked
    emacs_stats = r'(?:[0-9]{4}.*\.patch )?'
    re_stats = re.compile(emacs_stats +
                          r'total: (\d+) errors, (\d+) warnings, (\d+)')
    re_stats_full = re.compile(emacs_stats +
                               r'total: (\d+) errors, (\d+) warnings, (\d+)'
                               r' checks, (\d+)')
    re_ok = re.compile(r'.*has no obvious style problems')
    re_bad = re.compile(r'.*has style problems, please review')

    # A blank line indicates the end of a message
    for message in result.stdout.split('\n\n'):
        if verbose:
            print(message)

        # either find stats, the verdict, or delegate
        match = re_stats_full.match(message)
        if not match:
            match = re_stats.match(message)
        if match:
            result.errors = int(match.group(1))
            result.warnings = int(match.group(2))
            if len(match.groups()) == 4:
                result.checks = int(match.group(3))
                result.lines = int(match.group(4))
            else:
                result.lines = int(match.group(3))
        elif re_ok.match(message):
            result.ok = True
        elif re_bad.match(message):
            result.ok = False
        else:
            problem = check_patch_parse_one_message(message)
            if problem:
                result.problems.append(problem)

    return result


def check_patch(fname, verbose=False, show_types=False, use_tree=False):
    """Run checkpatch.pl on a file and parse the results.

    Args:
        fname: Filename to check
        verbose: True to print out every line of the checkpatch output as it is
            parsed
        show_types: Tell checkpatch to show the type (number) of each message
        use_tree (bool): If False we'll pass '--no-tree' to checkpatch.

    Returns:
        namedtuple containing:
            ok: False=failure, True=ok
            problems: List of problems, each a dict:
                'type'; error or warning
                'msg': text message
                'file' : filename
                'line': line number
            errors: Number of errors
            warnings: Number of warnings
            checks: Number of checks
            lines: Number of lines
            stdout: Full output of checkpatch
    """
    chk = find_check_patch()
    args = [chk]
    if not use_tree:
        args.append('--no-tree')
    if show_types:
        args.append('--show-types')
    output = command.output(*args, fname, raise_on_error=False)

    return check_patch_parse(output, verbose)


def get_warning_msg(col, msg_type, fname, line, msg):
    '''Create a message for a given file/line

    Args:
        msg_type: Message type ('error' or 'warning')
        fname: Filename which reports the problem
        line: Line number where it was noticed
        msg: Message to report
    '''
    if msg_type == 'warning':
        msg_type = col.build(col.YELLOW, msg_type)
    elif msg_type == 'error':
        msg_type = col.build(col.RED, msg_type)
    elif msg_type == 'check':
        msg_type = col.build(col.MAGENTA, msg_type)
    line_str = '' if line is None else '%d' % line
    return '%s:%s: %s: %s\n' % (fname, line_str, msg_type, msg)

def check_patches(verbose, args, use_tree):
    '''Run the checkpatch.pl script on each patch'''
    error_count, warning_count, check_count = 0, 0, 0
    col = terminal.Color()

    with concurrent.futures.ThreadPoolExecutor(max_workers=16) as executor:
        futures = []
        for fname in args:
            f = executor.submit(check_patch, fname, verbose, use_tree=use_tree)
            futures.append(f)

        for fname, f in zip(args, futures):
            result = f.result()
            if not result.ok:
                error_count += result.errors
                warning_count += result.warnings
                check_count += result.checks
                print('%d errors, %d warnings, %d checks for %s:' % (result.errors,
                        result.warnings, result.checks, col.build(col.BLUE, fname)))
                if (len(result.problems) != result.errors + result.warnings +
                        result.checks):
                    print("Internal error: some problems lost")
                # Python seems to get confused by this
                # pylint: disable=E1133
                for item in result.problems:
                    sys.stderr.write(
                        get_warning_msg(col, item.get('type', '<unknown>'),
                            item.get('file', '<unknown>'),
                            item.get('line', 0), item.get('msg', 'message')))
                print
    if error_count or warning_count or check_count:
        str = 'checkpatch.pl found %d error(s), %d warning(s), %d checks(s)'
        color = col.GREEN
        if warning_count:
            color = col.YELLOW
        if error_count:
            color = col.RED
        print(col.build(color, str % (error_count, warning_count, check_count)))
        return False
    return True
