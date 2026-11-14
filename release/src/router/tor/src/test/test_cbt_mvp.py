#!/usr/bin/env python3
# Copyright (c) 2026, The Tor Project, Inc.
# See LICENSE for licensing information.

"""Offline process integration for CBT state recovery and option transitions.

Usage: python3 src/test/test_cbt_mvp.py src/app/tor
Uses only a temporary directory and a local Unix control socket. No Tor network
is contacted. DROPTIMEOUTS is used only in these disposable test processes to
observe the effective cached policy through its existing controller event.
"""

import datetime
import pathlib
import re
import socket
import subprocess
import sys
import tempfile
import time


def command(sock, text):
    sock.sendall((text + "\r\n").encode())
    result = bytearray()
    while b"250 OK\r\n" not in result:
        chunk = sock.recv(8192)
        if not chunk:
            raise AssertionError("control connection closed")
        result.extend(chunk)
        if re.search(rb"(?:^|\r\n)[45]\d\d ", result):
            raise AssertionError(result.decode())
    return result.decode()


def policy(sock):
    result = command(sock, "DROPTIMEOUTS")
    while not re.search(r"650 BUILDTIMEOUT_SET RESET[^\r]*\r\n", result):
        chunk = sock.recv(8192)
        if not chunk:
            raise AssertionError("missing BUILDTIMEOUT_SET event")
        result += chunk.decode()
    return int(re.search(r"TIMEOUT_MS=(\d+)", result).group(1))


def run(tor, directory, adaptive, initial, reload=False, inspect_policy=True):
    control = directory / "control"
    log = directory / "process.log"
    with log.open("w") as output:
        proc = subprocess.Popen([
            tor, "--ignore-missing-torrc", "-f", str(directory / "torrc"),
            "--DataDirectory", str(directory), "--DisableNetwork", "1",
            "--SocksPort", "0", "--ControlPort", "0",
            "--ControlSocket", str(control), "--CookieAuthentication", "0",
            "--LearnCircuitBuildTimeout", str(adaptive),
            "--CircuitBuildTimeout", str(initial), "--Log", "notice stdout",
        ], stdout=output, stderr=subprocess.STDOUT)
        try:
            deadline = time.monotonic() + 15
            while not control.exists():
                if proc.poll() is not None or time.monotonic() > deadline:
                    raise AssertionError(log.read_text())
                time.sleep(0.05)
            with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as sock:
                sock.settimeout(10)
                sock.connect(str(control))
                command(sock, "AUTHENTICATE")
                command(sock, "SETEVENTS BUILDTIMEOUT_SET")
                if inspect_policy:
                    assert policy(sock) == initial * 1000
                if reload:
                    command(sock, "SETCONF LearnCircuitBuildTimeout=0 "
                            "CircuitBuildTimeout=120")
                    # Document the existing live-transition limitation. Do
                    # not silently claim that SETCONF repairs cached policy.
                    assert policy(sock) == initial * 1000
                command(sock, "SIGNAL SHUTDOWN")
            status = proc.wait(timeout=15)
            assert status == 0, "Tor exited with status {}".format(status)
        except Exception:
            print("Tor log for {}:".format(directory.name), file=sys.stderr)
            print(log.read_text(), file=sys.stderr)
            raise
        finally:
            if proc.poll() is None:
                proc.terminate()
                proc.wait(timeout=15)
    guards = [line for line in (directory / "state").read_text().splitlines()
              if line.startswith("Guard ")]
    assert guards
    return guards, log.read_text()


def main():
    tor = str(pathlib.Path(sys.argv[1]).resolve())
    now = datetime.datetime.now(datetime.timezone.utc).strftime(
        "%Y-%m-%dT%H:%M:%S")
    state = ("Guard in=default rsa_id=" + "12" * 20 +
             " nickname=PrivateGuardFixture sampled_on=" + now +
             " sampled_idx=0 sampled_by=0.4.9.13 listed=1\n"
             "TotalBuildTimes 1000\nCircuitBuildAbandonedCount 1000\n")
    with tempfile.TemporaryDirectory(prefix="cbt-mvp-", dir="/tmp") as root:
        root = pathlib.Path(root)
        for name in ("fixed", "adaptive", "persistence"):
            directory = root / name
            directory.mkdir(mode=0o700)
            (directory / "state").write_text(state)
        # Do not reset history or change options in the persistence scenario.
        # Otherwise DROPTIMEOUTS could perform the repair we mean to test.
        saved_guards, recovery_log = run(
            tor, root / "persistence", 1, 60, inspect_policy=False)
        assert "CBT history has no completed observations" in recovery_log
        saved_state = (root / "persistence" / "state").read_text()
        for key in ("TotalBuildTimes", "CircuitBuildAbandonedCount"):
            values = re.findall(r"^" + key + r" (\d+)$", saved_state, re.M)
            assert all(int(value) == 0 for value in values), saved_state
        assert not re.search(r"^CircuitBuildTimeBin ", saved_state, re.M)
        restarted_guards, restart_log = run(
            tor, root / "persistence", 1, 60, inspect_policy=False)
        assert restarted_guards == saved_guards
        assert "CBT history has no completed observations" not in restart_log
        assert "No valid circuit build time data" not in restart_log

        fixed_guards, fixed_log = run(tor, root / "fixed", 0, 60)
        guards, log = run(tor, root / "adaptive", 1, 60, reload=True)
        assert guards == fixed_guards
        summaries = [line for line in log.splitlines()
                     if "CBT history has no completed observations" in line]
        assert len(summaries) == 1
        assert "samples=1000 abandoned=1000" in summaries[0]
        assert "PrivateGuardFixture" not in summaries[0]
        assert "CBT history has no completed observations" not in fixed_log
        restarted_guards, _ = run(tor, root / "adaptive", 0, 120)
        assert restarted_guards == guards
    print("PASS: recovery persists across adaptive restart without DROPTIMEOUTS")
    print("PASS: recovery and fixed-mode restart preserve serialized guards")
    print("PASS: live SETCONF retains cached policy; restart applies 120s")


if __name__ == "__main__":
    main()
