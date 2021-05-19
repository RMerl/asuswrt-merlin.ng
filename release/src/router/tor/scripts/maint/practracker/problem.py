"""
In this file we define a ProblemVault class where we store all the
exceptions and all the problems we find with the code.

The ProblemVault is capable of registering problems and also figuring out if a
problem is worse than a registered exception so that it only warns when things
get worse.
"""

# Future imports for Python 2.7, mandatory in 3.0
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import os.path
import re
import sys

STATUS_ERR = 2
STATUS_WARN = 1
STATUS_OK = 0

class ProblemVault(object):
    """
    Singleton where we store the various new problems we
    found in the code, and also the old problems we read from the exception
    file.
    """
    def __init__(self, exception_fname=None):
        # Exception dictionary: { problem.key() : Problem object }
        self.exceptions = {}
        # Exception list: list of Problem objects, in the order added.
        self.exception_list = []
        # Exception dictionary: maps key to the problem it was used to
        # suppress.
        self.used_exception_for = {}

        if exception_fname == None:
            return

        try:
            with open(exception_fname, 'r') as exception_f:
                self.register_exceptions(exception_f)
        except IOError:
            print("No exception file provided", file=sys.stderr)

    def register_exceptions(self, exception_file):
        # Register exceptions
        for lineno, line in enumerate(exception_file, 1):
            try:
                problem = get_old_problem_from_exception_str(line)
            except ValueError as v:
                print("Exception file line {} not recognized: {}"
                      .format(lineno,v),
                      file=sys.stderr)
                continue

            if problem is None:
                continue

            # Fail if we see dup exceptions. There is really no reason to have dup exceptions.
            if problem.key() in self.exceptions:
                print("Duplicate exceptions lines found in exception file:\n\t{}\n\t{}\nAborting...".format(problem, self.exceptions[problem.key()]),
                      file=sys.stderr)
                sys.exit(1)

            self.exceptions[problem.key()] = problem
            self.exception_list.append(problem)
            #print "Registering exception: %s" % problem

    def register_problem(self, problem):
        """
        Register this problem to the problem value. Return true if it was a new
        problem or it worsens an already existing problem.  A true
        value may be STATUS_ERR to indicate a hard violation, or STATUS_WARN
        to indicate a warning.
        """
        # This is a new problem, print it
        if problem.key() not in self.exceptions:
            return STATUS_ERR

        # If it's an old problem, we don't warn if the situation got better
        # (e.g. we went from 4k LoC to 3k LoC), but we do warn if the
        # situation worsened (e.g. we went from 60 includes to 80).
        status = problem.is_worse_than(self.exceptions[problem.key()])

        # Remember that we used this exception, so that we can later
        # determine whether the exception was overbroad.
        self.used_exception_for[problem.key()] = problem

        return status

    def list_overbroad_exceptions(self):
        """Return an iterator of tuples containing (ex,prob) where ex is an
           exceptions in this vault that are stricter than it needs to be, and
           prob is the worst problem (if any) that it covered.
        """
        for k in self.exceptions:
            e = self.exceptions[k]
            p = self.used_exception_for.get(k)
            if p is None or e.is_worse_than(p):
                yield (e, p)

    def list_exceptions_without_overbroad(self):
        """Return an iterator of new problems, such that overbroad
           exceptions are replaced with minimally broad versions, or removed.
        """
        for e in self.exception_list:
            p = self.used_exception_for.get(e.key())
            if p is None:
                # This exception wasn't needed at all.
                continue
            if e.is_worse_than(p):
                # The exception is worse than the problem we found.
                # Yield the problem as the new exception value.
                yield p
            else:
                # The problem is as bad as the exception, or worse.
                # Yield the exception.
                yield e

    def set_tolerances(self, fns):
        """Adjust the tolerances for the exceptions in this vault.  Takes
           a map of problem type to a function that adjusts the permitted
           function to its new maximum value."""
        for k in self.exceptions:
            ex = self.exceptions[k]
            fn = fns.get(ex.problem_type)
            if fn is not None:
                ex.metric_value = fn(ex.metric_value)

class ProblemFilter(object):
    def __init__(self):
        self.thresholds = dict()

    def addThreshold(self, item):
        self.thresholds[(item.get_type(),item.get_file_type())] = item

    def matches(self, item):
        key = (item.get_type(), item.get_file_type())
        filt = self.thresholds.get(key, None)
        if filt is None:
            return False
        return item.is_worse_than(filt)

    def filter(self, sequence):
        for item in iter(sequence):
            if self.matches(item):
                yield item

class Item(object):
    """
    A generic measurement about some aspect of our source code. See
    the subclasses below for the specific problems we are trying to tackle.
    """
    def __init__(self, problem_type, problem_location, metric_value):
        self.problem_location = problem_location
        self.metric_value = int(metric_value)
        self.warning_threshold = self.metric_value
        self.problem_type = problem_type

    def is_worse_than(self, other_problem):
        """Return STATUS_ERR if this is a worse problem than other_problem.
           Return STATUS_WARN if it is a little worse, but falls within the
           warning threshold.  Return STATUS_OK if this problem is not
           at all worse than other_problem.
        """
        if self.metric_value > other_problem.metric_value:
            return STATUS_ERR
        elif self.metric_value > other_problem.warning_threshold:
            return STATUS_WARN
        else:
            return STATUS_OK

    def key(self):
        """Generate a unique key that describes this problem that can be used as a dictionary key"""
        # Item location is a filesystem path, so we need to normalize this
        # across platforms otherwise same paths are not gonna match.
        canonical_location = os.path.normcase(self.problem_location)
        return "%s:%s" % (canonical_location, self.problem_type)

    def __str__(self):
        return "problem %s %s %s" % (self.problem_type, self.problem_location, self.metric_value)

    def get_type(self):
        return self.problem_type

    def get_file_type(self):
        if self.problem_location.endswith(".h"):
            return "*.h"
        else:
            return "*.c"

class FileSizeItem(Item):
    """
    Denotes a problem with the size of a .c file.

    The 'problem_location' is the filesystem path of the .c file, and the
    'metric_value' is the number of lines in the .c file.
    """
    def __init__(self, problem_location, metric_value):
        super(FileSizeItem, self).__init__("file-size", problem_location, metric_value)

class IncludeCountItem(Item):
    """
    Denotes a problem with the number of #includes in a .c file.

    The 'problem_location' is the filesystem path of the .c file, and the
    'metric_value' is the number of #includes in the .c file.
    """
    def __init__(self, problem_location, metric_value):
        super(IncludeCountItem, self).__init__("include-count", problem_location, metric_value)

class FunctionSizeItem(Item):
    """
    Denotes a problem with a size of a function in a .c file.

    The 'problem_location' is "<path>:<function>()" where <path> is the
    filesystem path of the .c file and <function> is the name of the offending
    function.

    The 'metric_value' is the size of the offending function in lines.
    """
    def __init__(self, problem_location, metric_value):
        super(FunctionSizeItem, self).__init__("function-size", problem_location, metric_value)

class DependencyViolationItem(Item):
    """
    Denotes a dependency violation in a .c or .h file.  A dependency violation
    occurs when a file includes a file from some module that is not listed
    in its .may_include file.

    The 'problem_location' is the file that contains the problem.

    The 'metric_value' is the number of forbidden includes.
    """
    def __init__(self, problem_location, metric_value):
        super(DependencyViolationItem, self).__init__("dependency-violation",
                                                      problem_location,
                                                      metric_value)

comment_re = re.compile(r'#.*$')

def get_old_problem_from_exception_str(exception_str):
    orig_str = exception_str
    exception_str = comment_re.sub("", exception_str)
    fields = exception_str.split()
    if len(fields) == 0:
        # empty line or comment
        return None
    elif len(fields) == 4:
        # valid line
        _, problem_type, problem_location, metric_value = fields
    else:
        raise ValueError("Misformatted line {!r}".format(orig_str))

    if problem_type == "file-size":
        return FileSizeItem(problem_location, metric_value)
    elif problem_type == "include-count":
        return IncludeCountItem(problem_location, metric_value)
    elif problem_type == "function-size":
        return FunctionSizeItem(problem_location, metric_value)
    elif problem_type == "dependency-violation":
        return DependencyViolationItem(problem_location, metric_value)
    else:
        raise ValueError("Unknown exception type {!r}".format(orig_str))
