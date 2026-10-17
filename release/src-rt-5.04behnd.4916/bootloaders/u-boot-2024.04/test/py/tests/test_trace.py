# SPDX-License-Identifier: GPL-2.0
# Copyright 2022 Google LLC
# Written by Simon Glass <sjg@chromium.org>

import os
import pytest
import re

import u_boot_utils as util

# This is needed for Azure, since the default '..' directory is not writeable
TMPDIR = '/tmp/test_trace'

# Decode a function-graph line
RE_LINE = re.compile(r'.*0\.\.\.\.\. \s*([0-9.]*): func.*[|](\s*)(\S.*)?([{};])$')


def collect_trace(cons):
    """Build U-Boot and run it to collect a trace

    Args:
        cons (ConsoleBase): U-Boot console

    Returns:
        tuple:
            str: Filename of the output trace file
            int: Microseconds taken for initf_dm according to bootstage
    """
    cons.run_command('trace pause')
    out = cons.run_command('trace stats')

    # The output is something like this:
    #    251,003 function sites
    #  1,160,283 function calls
    #          0 untracked function calls
    #  1,230,758 traced function calls (341538 dropped due to overflow)
    #         33 maximum observed call depth
    #         15 call depth limit
    #    748,268 calls not traced due to depth
    #  1,230,758 max function calls

    # Get a dict of values from the output
    lines = [line.split(maxsplit=1) for line in out.splitlines() if line]
    vals = {key: val.replace(',', '') for val, key in lines}

    assert int(vals['function sites']) > 100000
    assert int(vals['function calls']) > 200000
    assert int(vals['untracked function calls']) == 0
    assert int(vals['maximum observed call depth']) > 30
    assert (vals['call depth limit'] ==
            cons.config.buildconfig.get('config_trace_call_depth_limit'))
    assert int(vals['calls not traced due to depth']) > 100000

    out = cons.run_command('bootstage report')
    # Accumulated time:
    #           19,104  dm_r
    #           23,078  of_live
    #           46,280  dm_f
    dm_f_time = [line.split()[0] for line in out.replace(',', '').splitlines()
                 if 'dm_f' in line]

    # Read out the trace data
    addr = 0x02000000
    size = 0x02000000
    out = cons.run_command(f'trace calls {addr:x} {size:x}')
    print(out)
    fname = os.path.join(TMPDIR, 'trace')
    out = cons.run_command(
        'host save hostfs - %x %s ${profoffset}' % (addr, fname))
    return fname, int(dm_f_time[0])


def check_function(cons, fname, proftool, map_fname, trace_dat):
    """Check that the 'function' output works

    Args:
        cons (ConsoleBase): U-Boot console
        fname (str): Filename of trace file
        proftool (str): Filename of proftool
        map_fname (str): Filename of System.map
        trace_dat (str): Filename of output file
    """
    out = util.run_and_log(
        cons, [proftool, '-t', fname, '-o', trace_dat, '-m', map_fname,
               'dump-ftrace'])

    # Check that trace-cmd can read it
    out = util.run_and_log(cons, ['trace-cmd', 'dump', trace_dat])

    # Tracing meta data in file /tmp/test_trace/trace.dat:
    #    [Initial format]
    #            6        [Version]
    #            0        [Little endian]
    #            4        [Bytes in a long]
    #            4096        [Page size, bytes]
    #    [Header page, 205 bytes]
    #    [Header event, 205 bytes]
    #    [Ftrace format, 3 events]
    #    [Events format, 0 systems]
    #    [Kallsyms, 342244 bytes]
    #    [Trace printk, 0 bytes]
    #    [Saved command lines, 9 bytes]
    #    1 [CPUs with tracing data]
    #    [6 options]
    #    [Flyrecord tracing data]
    #    [Tracing clock]
    #            [local] global counter uptime perf mono mono_raw boot x86-tsc
    assert '[Flyrecord tracing data]' in out
    assert '4096	[Page size, bytes]' in out
    kallsyms = [line.split() for line in out.splitlines() if 'Kallsyms' in line]
    # [['[Kallsyms,', '342244', 'bytes]']]
    val = int(kallsyms[0][1])
    assert val > 50000  # Should be at least 50KB of symbols

    # Check that the trace has something useful
    cmd = f"trace-cmd report -l {trace_dat} |grep -E '(initf_|initr_)'"
    out = util.run_and_log(cons, ['sh', '-c', cmd])

    # Format:
    #      u-boot-1     0.....    60.805596: function:             initf_malloc
    #      u-boot-1     0.....    60.805597: function:             initf_malloc
    #      u-boot-1     0.....    60.805601: function:             initf_bootstage
    #      u-boot-1     0.....    60.805607: function:             initf_bootstage

    lines = [line.replace(':', '').split() for line in out.splitlines()]
    vals = {items[4]: float(items[2]) for items in lines if len(items) == 5}
    base = None
    max_delta = 0
    for timestamp in vals.values():
        if base:
            max_delta = max(max_delta, timestamp - base)
        else:
            base = timestamp

    # Check for some expected functions
    assert 'initf_malloc' in vals.keys()
    assert 'initr_watchdog' in vals.keys()
    assert 'initr_dm' in vals.keys()

    # All the functions should be executed within five seconds at most
    assert max_delta < 5


def check_funcgraph(cons, fname, proftool, map_fname, trace_dat):
    """Check that the 'funcgraph' output works

    Args:
        cons (ConsoleBase): U-Boot console
        fname (str): Filename of trace file
        proftool (str): Filename of proftool
        map_fname (str): Filename of System.map
        trace_dat (str): Filename of output file

    Returns:
        int: Time taken by the first part of the initf_dm() function, in us
    """

    # Generate the funcgraph format
    out = util.run_and_log(
        cons, [proftool, '-t', fname, '-o', trace_dat, '-m', map_fname,
               'dump-ftrace', '-f', 'funcgraph'])

    # Check that the trace has what we expect
    cmd = f'trace-cmd report -l {trace_dat} |head -n 70'
    out = util.run_and_log(cons, ['sh', '-c', cmd])

    # First look for this:
    #  u-boot-1     0.....   282.101360: funcgraph_entry:        0.004 us   |    initf_malloc();
    # ...
    #  u-boot-1     0.....   282.101369: funcgraph_entry:                   |    initf_bootstage() {
    #  u-boot-1     0.....   282.101369: funcgraph_entry:                   |      bootstage_init() {
    #  u-boot-1     0.....   282.101369: funcgraph_entry:                   |        dlmalloc() {
    # ...
    #  u-boot-1     0.....   282.101375: funcgraph_exit:         0.001 us   |        }
    # Then look for this:
    #  u-boot-1     0.....   282.101375: funcgraph_exit:         0.006 us   |      }
    # Then check for this:
    #  u-boot-1     0.....   282.101375: funcgraph_entry:        0.000 us   |    initcall_is_event();

    expected_indent = None
    found_start = False
    found_end = False
    upto = None

    # Look for initf_bootstage() entry and make sure we see the exit
    # Collect the time for initf_dm()
    for line in out.splitlines():
        m = RE_LINE.match(line)
        if m:
            timestamp, indent, func, brace = m.groups()
            if found_end:
                upto = func
                break
            elif func == 'initf_bootstage() ':
                found_start = True
                expected_indent = indent + '  '
            elif found_start and indent == expected_indent and brace == '}':
                found_end = True

    # The next function after initf_bootstage() exits should be
    # initcall_is_event()
    assert upto == 'initcall_is_event()'

    # Now look for initf_dm() and dm_timer_init() so we can check the bootstage
    # time
    cmd = f"trace-cmd report -l {trace_dat} |grep -E '(initf_dm|dm_timer_init)'"
    out = util.run_and_log(cons, ['sh', '-c', cmd])

    start_timestamp = None
    end_timestamp = None
    for line in out.splitlines():
        m = RE_LINE.match(line)
        if m:
            timestamp, indent, func, brace = m.groups()
            if func == 'initf_dm() ':
                start_timestamp = timestamp
            elif func == 'dm_timer_init() ':
                end_timestamp = timestamp
                break
    assert start_timestamp and end_timestamp

    # Convert the time to microseconds
    return int((float(end_timestamp) - float(start_timestamp)) * 1000000)


def check_flamegraph(cons, fname, proftool, map_fname, trace_fg):
    """Check that the 'flamegraph' output works

    This spot checks a few call counts and estimates the time taken by the
    initf_dm() function

    Args:
        cons (ConsoleBase): U-Boot console
        fname (str): Filename of trace file
        proftool (str): Filename of proftool
        map_fname (str): Filename of System.map
        trace_fg (str): Filename of output file

    Returns:
        int: Approximate number of microseconds used by the initf_dm() function
    """

    # Generate the flamegraph format
    out = util.run_and_log(
        cons, [proftool, '-t', fname, '-o', trace_fg, '-m', map_fname,
               'dump-flamegraph'])

    # We expect dm_timer_init() to be called twice: once before relocation and
    # once after
    look1 = 'initf_dm;dm_timer_init 1'
    look2 = 'board_init_r;initcall_run_list;initr_dm_devices;dm_timer_init 1'
    found = 0
    with open(trace_fg, 'r') as fd:
        for line in fd:
            line = line.strip()
            if line == look1 or line == look2:
                found += 1
    assert found == 2

    # Generate the timing graph
    out = util.run_and_log(
        cons, [proftool, '-t', fname, '-o', trace_fg, '-m', map_fname,
               'dump-flamegraph', '-f', 'timing'])

    # Add up all the time spend in initf_dm() and its children
    total = 0
    with open(trace_fg, 'r') as fd:
        for line in fd:
            line = line.strip()
            if line.startswith('initf_dm'):
                func, val = line.split()
                count = int(val)
                total += count
    return total

check_flamegraph
@pytest.mark.slow
@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('trace')
def test_trace(u_boot_console):
    """Test we can build sandbox with trace, collect and process a trace"""
    cons = u_boot_console

    if not os.path.exists(TMPDIR):
        os.mkdir(TMPDIR)
    proftool = os.path.join(cons.config.build_dir, 'tools', 'proftool')
    map_fname = os.path.join(cons.config.build_dir, 'System.map')
    trace_dat = os.path.join(TMPDIR, 'trace.dat')
    trace_fg = os.path.join(TMPDIR, 'trace.fg')

    fname, dm_f_time = collect_trace(cons)

    check_function(cons, fname, proftool, map_fname, trace_dat)
    trace_time = check_funcgraph(cons, fname, proftool, map_fname, trace_dat)

    # Check that bootstage and funcgraph agree to within 10 microseconds
    diff = abs(trace_time - dm_f_time)
    print(f'trace_time {trace_time}, dm_f_time {dm_f_time}')
    assert diff / dm_f_time < 0.01

    fg_time = check_flamegraph(cons, fname, proftool, map_fname, trace_fg)

    # Check that bootstage and flamegraph agree to within 30%
    # This allows for CI being slow to run
    diff = abs(fg_time - dm_f_time)
    assert diff / dm_f_time < 0.3
