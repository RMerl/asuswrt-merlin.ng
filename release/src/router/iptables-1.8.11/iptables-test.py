#!/usr/bin/env python3
#
# (C) 2012-2013 by Pablo Neira Ayuso <pablo@netfilter.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This software has been sponsored by Sophos Astaro <http://www.sophos.com>
#

from __future__ import print_function
import sys
import os
import subprocess
import argparse
from difflib import unified_diff

IPTABLES = "iptables"
IP6TABLES = "ip6tables"
ARPTABLES = "arptables"
EBTABLES = "ebtables"

IPTABLES_SAVE = "iptables-save"
IP6TABLES_SAVE = "ip6tables-save"
ARPTABLES_SAVE = "arptables-save"
EBTABLES_SAVE = "ebtables-save"
#IPTABLES_SAVE = ['xtables-save','-4']
#IP6TABLES_SAVE = ['xtables-save','-6']

EXTENSIONS_PATH = "extensions"
TESTS_PATH = os.path.join(os.path.dirname(sys.argv[0]), "extensions")
LOGFILE="/tmp/iptables-test.log"
log_file = None

STDOUT_IS_TTY = sys.stdout.isatty()
STDERR_IS_TTY = sys.stderr.isatty()

def maybe_colored(color, text, isatty):
    terminal_sequences = {
        'green': '\033[92m',
        'red': '\033[91m',
    }

    return (
        terminal_sequences[color] + text + '\033[0m' if isatty else text
    )


def print_error(reason, filename=None, lineno=None, log_file=sys.stderr):
    '''
    Prints an error with nice colors, indicating file and line number.
    '''
    print(filename + ": " + maybe_colored('red', "ERROR", log_file.isatty()) +
        ": line %d (%s)" % (lineno, reason), file=log_file)


def delete_rule(iptables, rule, filename, lineno, netns = None):
    '''
    Removes an iptables rule

    Remove any --set-counters arguments, --delete rejects them.
    '''
    delrule = rule.split()
    for i in range(len(delrule)):
        if delrule[i] in ['-c', '--set-counters']:
            delrule.pop(i)
            if ',' in delrule.pop(i):
                break
            if len(delrule) > i and delrule[i].isnumeric():
                delrule.pop(i)
            break
    rule = " ".join(delrule)

    cmd = iptables + " -D " + rule
    ret = execute_cmd(cmd, filename, lineno, netns)
    if ret != 0:
        reason = "cannot delete: " + iptables + " -I " + rule
        print_error(reason, filename, lineno)
        return -1

    return 0


def run_test(iptables, rule, rule_save, res, filename, lineno, netns, stderr=sys.stderr):
    '''
    Executes an unit test. Returns the output of delete_rule().

    Parameters:
    :param iptables: string with the iptables command to execute
    :param rule: string with iptables arguments for the rule to test
    :param rule_save: string to find the rule in the output of iptables-save
    :param res: expected result of the rule. Valid values: "OK", "FAIL"
    :param filename: name of the file tested (used for print_error purposes)
    :param lineno: line number being tested (used for print_error purposes)
    :param netns: network namespace to call commands in (or None)
    '''
    ret = 0

    cmd = iptables + " -A " + rule
    ret = execute_cmd(cmd, filename, lineno, netns)

    #
    # report failed test
    #
    if ret:
        if res != "FAIL":
            reason = "cannot load: " + cmd
            print_error(reason, filename, lineno, stderr)
            return -1
        else:
            # do not report this error
            return 0
    else:
        if res == "FAIL":
            reason = "should fail: " + cmd
            print_error(reason, filename, lineno, stderr)
            delete_rule(iptables, rule, filename, lineno, netns)
            return -1

    matching = 0
    tokens = iptables.split(" ")
    if len(tokens) == 2:
        if tokens[1] == '-4':
            command = IPTABLES_SAVE
        elif tokens[1] == '-6':
            command = IP6TABLES_SAVE
    elif len(tokens) == 1:
        if tokens[0] == IPTABLES:
            command = IPTABLES_SAVE
        elif tokens[0] == IP6TABLES:
            command = IP6TABLES_SAVE
        elif tokens[0] == ARPTABLES:
            command = ARPTABLES_SAVE
        elif tokens[0] == EBTABLES:
            command = EBTABLES_SAVE

    command = EXECUTABLE + " " + command

    if netns:
            command = "ip netns exec " + netns + " " + command

    args = tokens[1:]
    proc = subprocess.Popen(command, shell=True,
                            stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = proc.communicate()
    if len(err):
        print(err, file=log_file)

    #
    # check for segfaults
    #
    if proc.returncode == -11:
        reason = command + " segfaults!"
        print_error(reason, filename, lineno, stderr)
        delete_rule(iptables, rule, filename, lineno, netns)
        return -1

    # find the rule
    matching = out.find("\n-A {}\n".format(rule_save).encode('utf-8'))

    if matching < 0:
        if res == "OK":
            reason = "cannot find: " + iptables + " -I " + rule
            print_error(reason, filename, lineno, stderr)
            delete_rule(iptables, rule, filename, lineno, netns)
            return -1
        else:
            # do not report this error
            return 0
    else:
        if res != "OK":
            reason = "should not match: " + cmd
            print_error(reason, filename, lineno, stderr)
            delete_rule(iptables, rule, filename, lineno, netns)
            return -1

    # Test "ip netns del NETNS" path with rules in place
    if netns:
        return 0

    return delete_rule(iptables, rule, filename, lineno)

def execute_cmd(cmd, filename, lineno = 0, netns = None):
    '''
    Executes a command, checking for segfaults and returning the command exit
    code.

    :param cmd: string with the command to be executed
    :param filename: name of the file tested (used for print_error purposes)
    :param lineno: line number being tested (used for print_error purposes)
    :param netns: network namespace to run command in
    '''
    global log_file
    if cmd.startswith('iptables ') or cmd.startswith('ip6tables ') or cmd.startswith('ebtables ') or cmd.startswith('arptables '):
        cmd = EXECUTABLE + " " + cmd

    if netns:
        cmd = "ip netns exec " + netns + " " + cmd

    print("command: {}".format(cmd), file=log_file)
    ret = subprocess.call(cmd, shell=True, universal_newlines=True,
        stderr=subprocess.STDOUT, stdout=log_file)
    log_file.flush()

    # generic check for segfaults
    if ret == -11:
        reason = "command segfaults: " + cmd
        print_error(reason, filename, lineno)
    return ret


def variant_res(res, variant, alt_res=None):
    '''
    Adjust expected result with given variant

    If expected result is scoped to a variant, the other one yields a different
    result. Therefore map @res to itself if given variant is current, use the
    alternate result, @alt_res, if specified, invert @res otherwise.

    :param res: expected result from test spec ("OK", "FAIL" or "NOMATCH")
    :param variant: variant @res is scoped to by test spec ("NFT" or "LEGACY")
    :param alt_res: optional expected result for the alternate variant.
    '''
    variant_executable = {
        "NFT": "xtables-nft-multi",
        "LEGACY": "xtables-legacy-multi"
    }
    res_inverse = {
        "OK": "FAIL",
        "FAIL": "OK",
        "NOMATCH": "OK"
    }

    if variant_executable[variant] == EXECUTABLE:
        return res
    if alt_res is not None:
        return alt_res
    return res_inverse[res]

def fast_run_possible(filename):
    '''
    Return true if fast test run is possible.

    To keep things simple, run only for simple test files:
    - no external commands
    - no multiple tables
    - no variant-specific results

    :param filename: test file to inspect
    '''
    table = None
    rulecount = 0
    for line in open(filename):
        if line[0] in ["#", ":"] or len(line.strip()) == 0:
            continue
        if line[0] == "*":
            if table or rulecount > 0:
                return False
            table = line.rstrip()[1:]
        if line[0] in ["@", "%"]:
            return False
        if len(line.split(";")) > 3:
            return False
        rulecount += 1

    return True

def run_test_file_fast(iptables, filename, netns):
    '''
    Run a test file, but fast

    Add all non-failing rules at once by use of iptables-restore, then check
    all rules' listing at once by use of iptables-save.

    :param filename: name of the file with the test rules
    :param netns: network namespace to perform test run in
    '''

    f = open(filename)

    rules = {}
    table = "filter"
    chain_array = []
    tests = 0

    for lineno, line in enumerate(f):
        if line[0] == "#" or len(line.strip()) == 0:
            continue

        if line[0] == "*":
            table = line.rstrip()[1:]
            continue

        if line[0] == ":":
            chain_array = line.rstrip()[1:].split(",")
            continue

        if len(chain_array) == 0:
            return -1

        tests += 1

        for chain in chain_array:
            item = line.split(";")
            rule = chain + " " + item[0]

            if item[1] == "=":
                rule_save = chain + " " + item[0]
            else:
                rule_save = chain + " " + item[1]

            if iptables == EBTABLES and rule_save.find('-j') < 0:
                rule_save += " -j CONTINUE"

            res = item[2].rstrip()
            if res != "OK":
                rule = chain + " -t " + table + " " + item[0]
                ret = run_test(iptables, rule, rule_save,
                               res, filename, lineno + 1, netns, log_file)

                if ret < 0:
                    return -1
                continue

            if not chain in rules.keys():
                rules[chain] = []
            rules[chain].append((rule, rule_save))

    restore_data = ["*" + table]
    out_expect = []
    for chain in ["PREROUTING", "INPUT", "FORWARD", "OUTPUT", "POSTROUTING"]:
        if not chain in rules.keys():
            continue
        for rule in rules[chain]:
            restore_data.append("-A " + rule[0])
            out_expect.append("-A " + rule[1])
    restore_data.append("COMMIT")

    out_expect = "\n".join(out_expect)

    # load all rules via iptables_restore

    command = EXECUTABLE + " " + iptables + "-restore"
    if netns:
        command = "ip netns exec " + netns + " " + command

    for line in restore_data:
        print(iptables + "-restore: " + line, file=log_file)

    proc = subprocess.Popen(command, shell = True, text = True,
                            stdin = subprocess.PIPE,
                            stdout = subprocess.PIPE,
                            stderr = subprocess.PIPE)
    restore_data = "\n".join(restore_data) + "\n"
    out, err = proc.communicate(input = restore_data)
    if len(err):
        print(err, file=log_file)

    if proc.returncode == -11:
        reason = iptables + "-restore segfaults!"
        print_error(reason, filename, lineno)
        msg = [iptables + "-restore segfault from:"]
        msg.extend(["input: " + l for l in restore_data.split("\n")])
        print("\n".join(msg), file=log_file)
        return -1

    if proc.returncode != 0:
        print("%s-restore returned %d: %s" % (iptables, proc.returncode, err),
              file=log_file)
        return -1

    # find all rules in iptables_save output

    command = EXECUTABLE + " " + iptables + "-save"
    if netns:
        command = "ip netns exec " + netns + " " + command

    proc = subprocess.Popen(command, shell = True,
                            stdin = subprocess.PIPE,
                            stdout = subprocess.PIPE,
                            stderr = subprocess.PIPE)
    out, err = proc.communicate()
    if len(err):
        print(err, file=log_file)

    if proc.returncode == -11:
        reason = iptables + "-save segfaults!"
        print_error(reason, filename, lineno)
        return -1

    cmd = iptables + " -F -t " + table
    execute_cmd(cmd, filename, 0, netns)

    out = out.decode('utf-8').rstrip()
    if out.find(out_expect) < 0:
        print("dumps differ!", file=log_file)
        out_clean = [ l for l in out.split("\n")
                        if not l[0] in ['*', ':', '#']]
        diff = unified_diff(out_expect.split("\n"), out_clean,
                            fromfile="expect", tofile="got", lineterm='')
        print("\n".join(diff), file=log_file)
        return -1

    return tests

def _run_test_file(iptables, filename, netns, suffix):
    '''
    Runs a test file

    :param iptables: string with the iptables command to execute
    :param filename: name of the file with the test rules
    :param netns: network namespace to perform test run in
    '''

    fast_failed = False
    if fast_run_possible(filename):
        tests = run_test_file_fast(iptables, filename, netns)
        if tests > 0:
            print(filename + ": " + maybe_colored('green', "OK", STDOUT_IS_TTY) + suffix)
            return tests, tests
        fast_failed = True

    f = open(filename)

    tests = 0
    passed = 0
    table = ""
    chain_array = []
    total_test_passed = True

    if netns:
        execute_cmd("ip netns add " + netns, filename)

    for lineno, line in enumerate(f):
        if line[0] == "#" or len(line.strip()) == 0:
            continue

        if line[0] == ":":
            chain_array = line.rstrip()[1:].split(",")
            continue

        # external command invocation, executed as is.
        # detects iptables commands to prefix with EXECUTABLE automatically
        if line[0] in ["@", "%"]:
            external_cmd = line.rstrip()[1:]
            execute_cmd(external_cmd, filename, lineno, netns)
            continue

        if line[0] == "*":
            table = line.rstrip()[1:]
            continue

        if len(chain_array) == 0:
            print_error("broken test, missing chain",
                        filename = filename, lineno = lineno)
            total_test_passed = False
            break

        test_passed = True
        tests += 1

        for chain in chain_array:
            item = line.split(";")
            if table == "":
                rule = chain + " " + item[0]
            else:
                rule = chain + " -t " + table + " " + item[0]

            if item[1] == "=":
                rule_save = chain + " " + item[0]
            else:
                rule_save = chain + " " + item[1]

            if iptables == EBTABLES and rule_save.find('-j') < 0:
                rule_save += " -j CONTINUE"

            res = item[2].rstrip()
            if len(item) > 3:
                variant = item[3].rstrip()
                if len(item) > 4:
                    alt_res = item[4].rstrip()
                else:
                    alt_res = None
                res = variant_res(res, variant, alt_res)

            ret = run_test(iptables, rule, rule_save,
                           res, filename, lineno + 1, netns)

            if ret < 0:
                test_passed = False
                total_test_passed = False
                break

        if test_passed:
            passed += 1

    if netns:
        execute_cmd("ip netns del " + netns, filename)
    if total_test_passed:
        if fast_failed:
            suffix += maybe_colored('red', " but fast mode failed!", STDOUT_IS_TTY)
        print(filename + ": " + maybe_colored('green', "OK", STDOUT_IS_TTY) + suffix)

    f.close()
    return tests, passed

def run_test_file(filename, netns):
    '''
    Runs a test file

    :param filename: name of the file with the test rules
    :param netns: network namespace to perform test run in
    '''
    #
    # if this is not a test file, skip.
    #
    if not filename.endswith(".t"):
        return 0, 0

    if "libipt_" in filename:
        xtables = [ IPTABLES ]
    elif "libip6t_" in filename:
        xtables = [ IP6TABLES ]
    elif "libxt_"  in filename:
        xtables = [ IPTABLES, IP6TABLES ]
    elif "libarpt_" in filename:
        # only supported with nf_tables backend
        if EXECUTABLE != "xtables-nft-multi":
           return 0, 0
        xtables = [ ARPTABLES ]
    elif "libebt_" in filename:
        # only supported with nf_tables backend
        if EXECUTABLE != "xtables-nft-multi":
           return 0, 0
        xtables = [ EBTABLES ]
    else:
        # default to iptables if not known prefix
        xtables = [ IPTABLES ]

    tests = 0
    passed = 0
    print_result = False
    suffix = ""
    for iptables in xtables:
        if len(xtables) > 1:
            suffix = "({})".format(iptables)

        file_tests, file_passed = _run_test_file(iptables, filename, netns, suffix)
        if file_tests:
            tests += file_tests
            passed += file_passed

    return tests, passed

def show_missing():
    '''
    Show the list of missing test files
    '''
    file_list = os.listdir(TESTS_PATH)
    testfiles = [i for i in file_list if i.endswith('.t')]
    libfiles = [i for i in file_list
                if i.startswith('lib') and i.endswith('.c')]

    def test_name(x):
        return x[0:-2] + '.t'
    missing = [test_name(i) for i in libfiles
               if not test_name(i) in testfiles]

    print('\n'.join(missing))

def spawn_netns():
    # prefer unshare module
    try:
        import unshare
        unshare.unshare(unshare.CLONE_NEWNET)
        return True
    except:
        pass

    # sledgehammer style:
    # - call ourselves prefixed by 'unshare -n' if found
    # - pass extra --no-netns parameter to avoid another recursion
    try:
        import shutil

        unshare = shutil.which("unshare")
        if unshare is None:
            return False

        sys.argv.append("--no-netns")
        os.execv(unshare, [unshare, "-n", sys.executable] + sys.argv)
    except:
        pass

    return False

#
# main
#
def main():
    parser = argparse.ArgumentParser(description='Run iptables tests')
    parser.add_argument('filename', nargs='*',
                        metavar='path/to/file.t',
                        help='Run only this test')
    parser.add_argument('-H', '--host', action='store_true',
                        help='Run tests against installed binaries')
    parser.add_argument('-l', '--legacy', action='store_true',
                        help='Test iptables-legacy')
    parser.add_argument('-m', '--missing', action='store_true',
                        help='Check for missing tests')
    parser.add_argument('-n', '--nftables', action='store_true',
                        help='Test iptables-over-nftables')
    parser.add_argument('-N', '--netns', action='store_const',
                        const='____iptables-container-test',
                        help='Test netnamespace path')
    parser.add_argument('--no-netns', action='store_true',
                        help='Do not run testsuite in own network namespace')
    args = parser.parse_args()

    #
    # show list of missing test files
    #
    if args.missing:
        show_missing()
        return

    variants = []
    if args.legacy:
        variants.append("legacy")
    if args.nftables:
        variants.append("nft")
    if len(variants) == 0:
        variants = [ "legacy", "nft" ]

    if os.getuid() != 0:
        print("You need to be root to run this, sorry", file=sys.stderr)
        return 77

    if not args.netns and not args.no_netns and not spawn_netns():
        print("Cannot run in own namespace, connectivity might break",
              file=sys.stderr)

    if not args.host:
        os.putenv("XTABLES_LIBDIR", os.path.abspath(EXTENSIONS_PATH))
        os.putenv("PATH", "%s/iptables:%s" % (os.path.abspath(os.path.curdir),
                                              os.getenv("PATH")))

    total_test_files = 0
    total_passed = 0
    total_tests = 0
    for variant in variants:
        global EXECUTABLE
        EXECUTABLE = "xtables-" + variant + "-multi"

        test_files = 0
        tests = 0
        passed = 0

        # setup global var log file
        global log_file
        try:
            log_file = open(LOGFILE, 'w')
        except IOError:
            print("Couldn't open log file %s" % LOGFILE, file=sys.stderr)
            return

        if args.filename:
            file_list = args.filename
        else:
            file_list = [os.path.join(TESTS_PATH, i)
                         for i in os.listdir(TESTS_PATH)
                         if i.endswith('.t')]
            file_list.sort()

        for filename in file_list:
            file_tests, file_passed = run_test_file(filename, args.netns)
            if file_tests:
                tests += file_tests
                passed += file_passed
                test_files += 1

        print("%s: %d test files, %d unit tests, %d passed"
              % (variant, test_files, tests, passed))

        total_passed += passed
        total_tests += tests
        total_test_files = max(total_test_files, test_files)

    if len(variants) > 1:
        print("total: %d test files, %d unit tests, %d passed"
              % (total_test_files, total_tests, total_passed))
    return total_passed - total_tests

if __name__ == '__main__':
    sys.exit(main())
