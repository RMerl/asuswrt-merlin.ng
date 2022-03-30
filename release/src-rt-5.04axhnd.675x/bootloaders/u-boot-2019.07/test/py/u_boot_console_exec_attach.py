# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2015 Stephen Warren
# Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.

# Logic to interact with U-Boot running on real hardware, typically via a
# physical serial port.

import sys
from u_boot_spawn import Spawn
from u_boot_console_base import ConsoleBase

class ConsoleExecAttach(ConsoleBase):
    """Represents a physical connection to a U-Boot console, typically via a
    serial port. This implementation executes a sub-process to attach to the
    console, expecting that the stdin/out of the sub-process will be forwarded
    to/from the physical hardware. This approach isolates the test infra-
    structure from the user-/installation-specific details of how to
    communicate with, and the identity of, serial ports etc."""

    def __init__(self, log, config):
        """Initialize a U-Boot console connection.

        Args:
            log: A multiplexed_log.Logfile instance.
            config: A "configuration" object as defined in conftest.py.

        Returns:
            Nothing.
        """

        # The max_fifo_fill value might need tweaking per-board/-SoC?
        # 1 would be safe anywhere, but is very slow (a pexpect issue?).
        # 16 is a common FIFO size.
        # HW flow control would mean this could be infinite.
        super(ConsoleExecAttach, self).__init__(log, config, max_fifo_fill=16)

        with self.log.section('flash'):
            self.log.action('Flashing U-Boot')
            cmd = ['u-boot-test-flash', config.board_type, config.board_identity]
            runner = self.log.get_runner(cmd[0], sys.stdout)
            runner.run(cmd)
            runner.close()
            self.log.status_pass('OK')

    def get_spawn(self):
        """Connect to a fresh U-Boot instance.

        The target board is reset, so that U-Boot begins running from scratch.

        Args:
            None.

        Returns:
            A u_boot_spawn.Spawn object that is attached to U-Boot.
        """

        args = [self.config.board_type, self.config.board_identity]
        s = Spawn(['u-boot-test-console'] + args)

        try:
            self.log.action('Resetting board')
            cmd = ['u-boot-test-reset'] + args
            runner = self.log.get_runner(cmd[0], sys.stdout)
            runner.run(cmd)
            runner.close()
        except:
            s.close()
            raise

        return s
