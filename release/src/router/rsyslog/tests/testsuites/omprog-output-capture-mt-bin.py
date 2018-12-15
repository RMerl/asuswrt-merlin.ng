#!/usr/bin/env python

import sys
import os
import logging

lineLength = int(sys.argv[1])
linePrefix = "[{0:09d}] ".format(os.getpid())

logLine = sys.stdin.readline()
while logLine:
    logLine = logLine.strip()
    numRepeats = lineLength / len(logLine)
    lineToStdout = (linePrefix + "[stdout] " + logLine*numRepeats)[:lineLength]
    lineToStderr = (linePrefix + "[stderr] " + logLine*numRepeats)[:lineLength]

    # Write to stdout without flushing. Since stdout is block-buffered when redirected to a pipe,
    # and multiple processes are writing to the pipe, this will cause intermingled lines in the
    # output file. However, we avoid this by executing this script with 'stdbuf -oL' (see the
    # test code), which forces line buffering for stdout. We could alternatively call
    # sys.stdout.flush() after the write (this also causes a single 'write' syscall, since the
    # size of the block buffer is generally greater than PIPE_BUF).
    sys.stdout.write(lineToStdout + "\n")

    # Write to stderr using two writes. Since stderr is unbuffered, each write will be written
    # immediately to the pipe, and this will cause intermingled lines in the output file.
    # However, we avoid this by executing this script with 'stdbuf -eL', which forces line
    # buffering for stderr. We could alternatively do a single write.
    sys.stderr.write(lineToStderr)
    sys.stderr.write("\n")

    # Note: In future versions of Python3, stderr will possibly be line buffered (see
    # https://bugs.python.org/issue13601).
    # Note: When writing to stderr using the Python logging module, it seems that line
    # buffering is also used (although this could depend on the Python version).

    logLine = sys.stdin.readline()
