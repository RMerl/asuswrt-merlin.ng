# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2012 The Chromium OS Authors.


"""A single board which can be selected and built"""

class Board:
    """A particular board that we can build"""
    def __init__(self, status, arch, cpu, soc, vendor, board_name, target, cfg_name):
        """Create a new board type.

        Args:
            status: define whether the board is 'Active' or 'Orphaned'
            arch: Architecture name (e.g. arm)
            cpu: Cpu name (e.g. arm1136)
            soc: Name of SOC, or '' if none (e.g. mx31)
            vendor: Name of vendor (e.g. armltd)
            board_name: Name of board (e.g. integrator)
            target: Target name (use make <target>_defconfig to configure)
            cfg_name: Config-file name (in includes/configs/)
        """
        self.target = target
        self.arch = arch
        self.cpu = cpu
        self.soc = soc
        self.vendor = vendor
        self.board_name = board_name
        self.cfg_name = cfg_name
        self.props = [self.target, self.arch, self.cpu, self.board_name,
                      self.vendor, self.soc, self.cfg_name]
        self.build_it = False
