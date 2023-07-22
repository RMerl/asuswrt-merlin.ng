#!/usr/bin/env python
# coding=utf-8

# Copyright (c) 2015 Remko Tronçon (https://el-tramo.be)
# Copied from https://github.com/remko/pycotap/
#
# Released under the MIT license
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


import unittest
import sys
import base64
from io import StringIO

# Log modes
class LogMode(object) :
  LogToError, LogToDiagnostics, LogToYAML, LogToAttachment = range(4)


class TAPTestResult(unittest.TestResult):
  def __init__(self, output_stream, error_stream, message_log, test_output_log):
    super(TAPTestResult, self).__init__(self, output_stream)
    self.output_stream = output_stream
    self.error_stream = error_stream
    self.orig_stdout = None
    self.orig_stderr = None
    self.message = None
    self.test_output = None
    self.message_log = message_log
    self.test_output_log = test_output_log
    self.output_stream.write("TAP version 13\n")
    self._set_streams()

  def printErrors(self):
    self.print_raw("1..%d\n" % self.testsRun)
    self._reset_streams()

  def _set_streams(self):
    self.orig_stdout = sys.stdout
    self.orig_stderr = sys.stderr
    if self.message_log == LogMode.LogToError:
      self.message = self.error_stream
    else:
      self.message = StringIO()
    if self.test_output_log == LogMode.LogToError:
      self.test_output = self.error_stream
    else:
      self.test_output = StringIO()

    if self.message_log == self.test_output_log:
      self.test_output = self.message
    sys.stdout = sys.stderr = self.test_output

  def _reset_streams(self):
    sys.stdout = self.orig_stdout
    sys.stderr = self.orig_stderr


  def print_raw(self, text):
    self.output_stream.write(text)
    self.output_stream.flush()

  def print_result(self, result, test, directive = None):
    self.output_stream.write("%s %d %s" % (result, self.testsRun, test.id()))
    if directive:
      self.output_stream.write(" # " + directive)
    self.output_stream.write("\n")
    self.output_stream.flush()

  def ok(self, test, directive = None):
    self.print_result("ok", test, directive)

  def not_ok(self, test):
    self.print_result("not ok", test)

  def startTest(self, test):
    super(TAPTestResult, self).startTest(test)

  def stopTest(self, test):
    super(TAPTestResult, self).stopTest(test)
    if self.message_log == self.test_output_log:
      logs = [(self.message_log, self.message, "output")]
    else:
      logs = [
          (self.test_output_log, self.test_output, "test_output"),
          (self.message_log, self.message, "message")
      ]
    for log_mode, log, log_name in logs:
      if log_mode != LogMode.LogToError:
        output = log.getvalue()
        if len(output):
          if log_mode == LogMode.LogToYAML:
            self.print_raw("  ---\n")
            self.print_raw("    " + log_name + ": |\n")
            self.print_raw("      " + output.rstrip().replace("\n", "\n      ") + "\n")
            self.print_raw("  ...\n")
          elif log_mode == LogMode.LogToAttachment:
            self.print_raw("  ---\n")
            self.print_raw("    " + log_name + ":\n")
            self.print_raw("      File-Name: " + log_name + ".txt\n")
            self.print_raw("      File-Type: text/plain\n")
            self.print_raw("      File-Content: " + base64.b64encode(output) + "\n")
            self.print_raw("  ...\n")
          else:
            self.print_raw("# " + output.rstrip().replace("\n", "\n# ") + "\n")
        # Truncate doesn't change the current stream position.
        # Seek to the beginning to avoid extensions on subsequent writes.
        log.seek(0)
        log.truncate(0)

  def addSuccess(self, test):
    super(TAPTestResult, self).addSuccess(test)
    self.ok(test)

  def addError(self, test, err):
    super(TAPTestResult, self).addError(test, err)
    self.message.write(self.errors[-1][1] + "\n")
    self.not_ok(test)

  def addFailure(self, test, err):
    super(TAPTestResult, self).addFailure(test, err)
    self.message.write(self.failures[-1][1] + "\n")
    self.not_ok(test)

  def addSkip(self, test, reason):
    super(TAPTestResult, self).addSkip(test, reason)
    self.ok(test, "SKIP " + reason)

  def addExpectedFailure(self, test, err):
    super(TAPTestResult, self).addExpectedFailure(test, err)
    self.ok(test)

  def addUnexpectedSuccess(self, test):
    super(TAPTestResult, self).addUnexpectedSuccess(test)
    self.message.write("Unexpected success" + "\n")
    self.not_ok(test)


class TAPTestRunner(object):
  def __init__(self,
      message_log = LogMode.LogToYAML,
      test_output_log = LogMode.LogToDiagnostics,
      output_stream = sys.stdout, error_stream = sys.stderr):
    self.output_stream = output_stream
    self.error_stream = error_stream
    self.message_log = message_log
    self.test_output_log = test_output_log

  def run(self, test):
    result = TAPTestResult(
        self.output_stream,
        self.error_stream,
        self.message_log,
        self.test_output_log)
    test(result)
    result.printErrors()

    return result
