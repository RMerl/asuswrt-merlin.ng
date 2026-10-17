# SPDX-License-Identifier: GPL-2.0
# (C) Copyright 2023, Advanced Micro Devices, Inc.

import pytest
import random
import string
import test_net

"""
Note: This test relies on boardenv_* containing configuration values to define
RPU applications information for AMD's ZynqMP SoC which contains, application
names, processors, address where it is built, expected output and the tftp load
addresses. This test will be automatically skipped without this.

It also relies on dhcp or setup_static net test to support tftp to load
application on DDR. All the environment parameters are stored sequentially.
The length of all parameters values should be same. For example, if 2 app_names
are defined in a list as a value of parameter 'app_name' then the other
parameters value also should have a list with 2 items.
It will run RPU cases for all the applications defined in boardenv_*
configuration file.

Example:
env__zynqmp_rpu_apps = {
    'app_name': ['hello_world_r5_0_ddr.elf', 'hello_world_r5_1_ddr.elf'],
    'proc': ['rpu0', 'rpu1'],
    'cpu_num': [4, 5],
    'addr': [0xA00000, 0xB00000],
    'output': ['Successfully ran Hello World application on DDR from RPU0',
               'Successfully ran Hello World application on DDR from RPU1'],
    'tftp_addr': [0x100000, 0x200000],
}
"""

# Get rpu apps params from env
def get_rpu_apps_env(u_boot_console):
    rpu_apps = u_boot_console.config.env.get('env__zynqmp_rpu_apps', False)
    if not rpu_apps:
        pytest.skip('ZynqMP RPU application info not defined!')

    apps = rpu_apps.get('app_name', None)
    if not apps:
        pytest.skip('No RPU application found!')

    procs = rpu_apps.get('proc', None)
    if not procs:
        pytest.skip('No RPU application processor provided!')

    cpu_nums = rpu_apps.get('cpu_num', None)
    if not cpu_nums:
        pytest.skip('No CPU number for respective processor provided!')

    addrs = rpu_apps.get('addr', None)
    if not addrs:
        pytest.skip('No RPU application build address found!')

    outputs = rpu_apps.get('output', None)
    if not outputs:
        pytest.skip('Expected output not found!')

    tftp_addrs = rpu_apps.get('tftp_addr', None)
    if not tftp_addrs:
        pytest.skip('TFTP address to load application not found!')

    return apps, procs, cpu_nums, addrs, outputs, tftp_addrs

# Check return code
def ret_code(u_boot_console):
    return u_boot_console.run_command('echo $?')

# Initialize tcm
def tcminit(u_boot_console, rpu_mode):
    output = u_boot_console.run_command('zynqmp tcminit %s' % rpu_mode)
    assert 'Initializing TCM overwrites TCM content' in output
    return ret_code(u_boot_console)

# Load application in DDR
def load_app_ddr(u_boot_console, tftp_addr, app):
    output = u_boot_console.run_command('tftpboot %x %s' % (tftp_addr, app))
    assert 'TIMEOUT' not in output
    assert 'Bytes transferred = ' in output

    # Load elf
    u_boot_console.run_command('bootelf -p %x' % tftp_addr)
    assert ret_code(u_boot_console).endswith('0')

# Disable cpus
def disable_cpus(u_boot_console, cpu_nums):
    for num in cpu_nums:
        u_boot_console.run_command(f'cpu {num} disable')

# Load apps on RPU cores
def rpu_apps_load(u_boot_console, rpu_mode):
    apps, procs, cpu_nums, addrs, outputs, tftp_addrs = get_rpu_apps_env(
        u_boot_console)
    test_net.test_net_dhcp(u_boot_console)
    if not test_net.net_set_up:
        test_net.test_net_setup_static(u_boot_console)

    try:
        assert tcminit(u_boot_console, rpu_mode).endswith('0')

        for i in range(len(apps)):
            if rpu_mode == 'lockstep' and procs[i] != 'rpu0':
                continue

            load_app_ddr(u_boot_console, tftp_addrs[i], apps[i])
            rel_addr = int(addrs[i] + 0x3C)

            # Release cpu at app load address
            cpu_num = cpu_nums[i]
            cmd = 'cpu %d release %x %s' % (cpu_num, rel_addr, rpu_mode)
            output = u_boot_console.run_command(cmd)
            exp_op = f'Using TCM jump trampoline for address {hex(rel_addr)}'
            assert exp_op in output
            assert f'R5 {rpu_mode} mode' in output
            u_boot_console.wait_for(outputs[i])
            assert ret_code(u_boot_console).endswith('0')
    finally:
        disable_cpus(u_boot_console, cpu_nums)

@pytest.mark.buildconfigspec('cmd_zynqmp')
def test_zynqmp_rpu_app_load_split(u_boot_console):
    rpu_apps_load(u_boot_console, 'split')

@pytest.mark.buildconfigspec('cmd_zynqmp')
def test_zynqmp_rpu_app_load_lockstep(u_boot_console):
    rpu_apps_load(u_boot_console, 'lockstep')

@pytest.mark.buildconfigspec('cmd_zynqmp')
def test_zynqmp_rpu_app_load_negative(u_boot_console):
    apps, procs, cpu_nums, addrs, outputs, tftp_addrs = get_rpu_apps_env(
        u_boot_console)

    # Invalid commands
    u_boot_console.run_command('zynqmp tcminit mode')
    assert ret_code(u_boot_console).endswith('1')

    rand_str = ''.join(random.choices(string.ascii_lowercase, k=4))
    u_boot_console.run_command('zynqmp tcminit %s' % rand_str)
    assert ret_code(u_boot_console).endswith('1')

    rand_num = random.randint(2, 100)
    u_boot_console.run_command('zynqmp tcminit %d' % rand_num)
    assert ret_code(u_boot_console).endswith('1')

    test_net.test_net_dhcp(u_boot_console)
    if not test_net.net_set_up:
        test_net.test_net_setup_static(u_boot_console)

    try:
        rpu_mode = 'split'
        assert tcminit(u_boot_console, rpu_mode).endswith('0')

        for i in range(len(apps)):
            load_app_ddr(u_boot_console, tftp_addrs[i], apps[i])

            # Run in split mode at different load address
            rel_addr = int(addrs[i]) + random.randint(200, 1000)
            cpu_num = cpu_nums[i]
            cmd = 'cpu %d release %x %s' % (cpu_num, rel_addr, rpu_mode)
            output = u_boot_console.run_command(cmd)
            exp_op = f'Using TCM jump trampoline for address {hex(rel_addr)}'
            assert exp_op in output
            assert f'R5 {rpu_mode} mode' in output
            assert not outputs[i] in output

            # Invalid rpu mode
            rand_str = ''.join(random.choices(string.ascii_lowercase, k=4))
            cmd = 'cpu %d release %x %s' % (cpu_num, rel_addr, rand_str)
            output = u_boot_console.run_command(cmd)
            assert exp_op in output
            assert f'Unsupported mode' in output
            assert not ret_code(u_boot_console).endswith('0')

        # Switch to lockstep mode, without disabling CPUs
        rpu_mode = 'lockstep'
        u_boot_console.run_command('zynqmp tcminit %s' % rpu_mode)
        assert not ret_code(u_boot_console).endswith('0')

        # Disable cpus
        disable_cpus(u_boot_console, cpu_nums)

        # Switch to lockstep mode, after disabling CPUs
        output = u_boot_console.run_command('zynqmp tcminit %s' % rpu_mode)
        assert 'Initializing TCM overwrites TCM content' in output
        assert ret_code(u_boot_console).endswith('0')

        # Run lockstep mode for RPU1
        for i in range(len(apps)):
            if procs[i] == 'rpu0':
                continue

            load_app_ddr(u_boot_console, tftp_addrs[i], apps[i])
            rel_addr = int(addrs[i] + 0x3C)
            cpu_num = cpu_nums[i]
            cmd = 'cpu %d release %x %s' % (cpu_num, rel_addr, rpu_mode)
            output = u_boot_console.run_command(cmd)
            exp_op = f'Using TCM jump trampoline for address {hex(rel_addr)}'
            assert exp_op in output
            assert f'R5 {rpu_mode} mode' in output
            assert u_boot_console.p.expect([outputs[i]])
    finally:
        disable_cpus(u_boot_console, cpu_nums)
        # This forces the console object to be shutdown, so any subsequent test
        # will reset the board back into U-Boot.
        u_boot_console.drain_console()
        u_boot_console.cleanup_spawn()
