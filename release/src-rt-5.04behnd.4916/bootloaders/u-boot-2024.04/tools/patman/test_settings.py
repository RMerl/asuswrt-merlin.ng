# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2022 Maxim Cournoyer <maxim.cournoyer@savoirfairelinux.com>
#

import argparse
import contextlib
import os
import sys
import tempfile

from patman import settings
from u_boot_pylib import tools


@contextlib.contextmanager
def empty_git_repository():
    with tempfile.TemporaryDirectory() as tmpdir:
        os.chdir(tmpdir)
        tools.run('git', 'init', raise_on_error=True)
        yield tmpdir


@contextlib.contextmanager
def cleared_command_line_args():
    old_value = sys.argv[:]
    sys.argv = [sys.argv[0]]
    try:
        yield
    finally:
        sys.argv = old_value


def test_git_local_config():
    # Clearing the command line arguments is required, otherwise
    # arguments passed to the test running such as in 'pytest -k
    # filter' would be processed by _UpdateDefaults and fail.
    with cleared_command_line_args():
        with empty_git_repository():
            with tempfile.NamedTemporaryFile() as global_config:
                global_config.write(b'[settings]\n'
                                    b'project=u-boot\n')
                global_config.flush()
                parser = argparse.ArgumentParser()
                parser.add_argument('-p', '--project', default='unknown')
                subparsers = parser.add_subparsers(dest='cmd')
                send = subparsers.add_parser('send')
                send.add_argument('--no-check', action='store_false',
                                  dest='check_patch', default=True)

                # Test "global" config is used.
                settings.Setup(parser, 'unknown', global_config.name)
                args, _ = parser.parse_known_args([])
                assert args.project == 'u-boot'
                send_args, _ = send.parse_known_args([])
                assert send_args.check_patch

                # Test local config can shadow it.
                with open('.patman', 'w', buffering=1) as f:
                    f.write('[settings]\n'
                            'project: guix-patches\n'
                            'check_patch: False\n')
                settings.Setup(parser, 'unknown', global_config.name)
                args, _ = parser.parse_known_args([])
                assert args.project == 'guix-patches'
                send_args, _ = send.parse_known_args([])
                assert not send_args.check_patch
