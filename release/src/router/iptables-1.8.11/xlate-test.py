#!/usr/bin/env python3
# encoding: utf-8

import os
import sys
import shlex
import argparse
from subprocess import Popen, PIPE

def run_proc(args, shell = False, input = None):
    """A simple wrapper around Popen, returning (rc, stdout, stderr)"""
    process = Popen(args, text = True, shell = shell,
                    stdin = PIPE, stdout = PIPE, stderr = PIPE)
    output, error = process.communicate(input)
    return (process.returncode, output, error)

keywords = ("iptables-translate", "ip6tables-translate", "arptables-translate", "ebtables-translate")
xtables_nft_multi = 'xtables-nft-multi'

if sys.stdout.isatty():
    colors = {"magenta": "\033[95m", "green": "\033[92m", "yellow": "\033[93m",
              "red": "\033[91m", "end": "\033[0m"}
else:
    colors = {"magenta": "", "green": "", "yellow": "", "red": "", "end": ""}


def magenta(string):
    return colors["magenta"] + string + colors["end"]


def red(string):
    return colors["red"] + string + colors["end"]


def yellow(string):
    return colors["yellow"] + string + colors["end"]


def green(string):
    return colors["green"] + string + colors["end"]


def test_one_xlate(name, sourceline, expected, result):
    cmd = [xtables_nft_multi] + shlex.split(sourceline)
    rc, output, error = run_proc(cmd)
    if rc != 0:
        result.append(name + ": " + red("Error: ") + "Call failed: " + " ".join(cmd))
        result.append(error)
        return False

    translation = output.rstrip(" \n")
    if translation != expected:
        result.append(name + ": " + red("Fail"))
        result.append(magenta("src: ") + sourceline.rstrip(" \n"))
        result.append(magenta("exp: ") + expected)
        result.append(magenta("res: ") + translation + "\n")
        return False

    return True

def test_one_replay(name, sourceline, expected, result):
    global args

    searchline = None
    if sourceline.find(';') >= 0:
        sourceline, searchline = sourceline.split(';')

    srcwords = shlex.split(sourceline)

    srccmd = srcwords[0]
    ipt = srccmd.split('-')[0]
    table_idx = -1
    chain_idx = -1
    table_name = "filter"
    chain_name = None
    for idx in range(1, len(srcwords)):
        if srcwords[idx] in ["-A", "-I", "--append", "--insert"]:
            chain_idx = idx
            chain_name = srcwords[idx + 1]
        elif srcwords[idx] in ["-t", "--table"]:
            table_idx = idx
            table_name = srcwords[idx + 1]

    if not chain_name:
        return True     # nothing to do?

    if searchline is None:
        # adjust sourceline as required
        checkcmd = srcwords[:]
        checkcmd[0] = ipt
        checkcmd[chain_idx] = "--check"
    else:
        checkcmd = [ipt, "-t", table_name]
        checkcmd += ["--check", chain_name, searchline]

    fam = ""
    if srccmd.startswith("ip6"):
        fam = "ip6 "
    elif srccmd.startswith("arp"):
        fam = "arp "
    elif srccmd.startswith("ebt"):
        fam = "bridge "

    expected = [ l.removeprefix("nft ").strip(" '") for l in expected.split("\n") ]
    nft_input = [
            "flush ruleset",
            "add table " + fam + table_name,
            "add chain " + fam + table_name + " " + chain_name,
    ] + expected

    rc, output, error = run_proc([args.nft, "-f", "-"], shell = False, input = "\n".join(nft_input))
    if rc != 0:
        result.append(name + ": " + red("Replay Fail"))
        result.append(args.nft + " call failed: " + error.rstrip('\n'))
        for line in nft_input:
            result.append(magenta("input: ") + line)
        return False

    rc, output, error = run_proc([xtables_nft_multi] + checkcmd)
    if rc != 0:
        result.append(name + ": " + red("Check Fail"))
        result.append(magenta("check: ") + " ".join(checkcmd))
        result.append(magenta("error: ") + error)
        rc, output, error = run_proc([xtables_nft_multi, ipt + "-save"])
        for l in output.split("\n"):
            result.append(magenta("ipt: ") + l)
        rc, output, error = run_proc([args.nft, "list", "ruleset"])
        for l in output.split("\n"):
            result.append(magenta("nft: ") + l)
        return False

    return True


def run_test(name, payload):
    global xtables_nft_multi
    global args

    test_passed = True
    tests = passed = failed = errors = 0
    result = []

    line = payload.readline()
    while line:
        if not line.startswith(keywords):
            line = payload.readline()
            continue

        sourceline = replayline = line.rstrip("\n")
        if line.find(';') >= 0:
            sourceline = line.split(';')[0]

        expected = payload.readline().rstrip(" \n")
        next_expected = payload.readline()
        if next_expected.startswith("nft"):
            expected += "\n" + next_expected.rstrip(" \n")
            line = payload.readline()
        else:
            line = next_expected

        tests += 1
        if test_one_xlate(name, sourceline, expected, result):
            passed += 1
        else:
            errors += 1
            test_passed = False
            continue

        if args.replay:
            tests += 1
            if test_one_replay(name, replayline, expected, result):
                passed += 1
            else:
                errors += 1
                test_passed = False

            rc, output, error = run_proc([args.nft, "flush", "ruleset"])
            if rc != 0:
                result.append(name + ": " + red("Fail"))
                result.append("nft flush ruleset call failed: " + error)

    if (passed == tests):
        print(name + ": " + green("OK"))
    if not test_passed:
        print("\n".join(result), file=sys.stderr)
    return tests, passed, failed, errors


def load_test_files():
    test_files = total_tests = total_passed = total_error = total_failed = 0
    tests_path = os.path.join(os.path.dirname(sys.argv[0]), "extensions")
    tests = sorted(os.listdir(tests_path))
    for test in [os.path.join(tests_path, f)
                 for f in tests if f.endswith(".txlate")]:
        with open(test, "r") as payload:
            tests, passed, failed, errors = run_test(test, payload)
            test_files += 1
            total_tests += tests
            total_passed += passed
            total_failed += failed
            total_error += errors
    return (test_files, total_tests, total_passed, total_failed, total_error)


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


def main():
    global xtables_nft_multi

    if args.replay:
        if os.getuid() != 0:
            print("Replay test requires root, sorry", file=sys.stderr)
            return
        if not args.no_netns and not spawn_netns():
            print("Cannot run in own namespace, connectivity might break",
                  file=sys.stderr)

    if not args.host:
        os.putenv("XTABLES_LIBDIR", os.path.abspath("extensions"))
        xtables_nft_multi = os.path.abspath(os.path.curdir) \
                            + '/iptables/' + xtables_nft_multi

    files = tests = passed = failed = errors = 0
    for test in args.test:
        if not test.endswith(".txlate"):
            test += ".txlate"
        try:
            with open(test, "r") as payload:
                t, p, f, e = run_test(test, payload)
                files += 1
                tests += t
                passed += p
                failed += f
                errors += e
        except IOError:
            print(red("Error: ") + "test file does not exist", file=sys.stderr)
            return 99

    if files == 0:
        files, tests, passed, failed, errors = load_test_files()

    if files > 1:
        file_word = "files"
    else:
        file_word = "file"
    print("%d test %s, %d tests, %d tests passed, %d tests failed, %d errors"
            % (files, file_word, tests, passed, failed, errors))
    return passed - tests


parser = argparse.ArgumentParser()
parser.add_argument('-H', '--host', action='store_true',
                    help='Run tests against installed binaries')
parser.add_argument('-R', '--replay', action='store_true',
                    help='Replay tests to check iptables-nft parser')
parser.add_argument('-n', '--nft', type=str, default='nft',
                    help='Replay using given nft binary (default: \'%(default)s\')')
parser.add_argument('--no-netns', action='store_true',
                    help='Do not run testsuite in own network namespace')
parser.add_argument("test", nargs="*", help="run only the specified test file(s)")
args = parser.parse_args()
sys.exit(main())
