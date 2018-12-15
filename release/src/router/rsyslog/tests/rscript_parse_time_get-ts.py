#!/usr/bin/env python
# Added 2017-11-05 by Stephen Workman, released under ASL 2.0

#
# Produces a UNIX timestamp representing the specified RFC 3164 date/time
# string. Since this date/time format does not include a year, a simple
# algorithm is used to "guess" an appropriate one and append it to the
# date/time string to calculate a timestamp value.
#
# If the incoming date is within one month in the future (from now),
# it is assumed that it's either for the current year, or the next
# year (depending on whether it is December or not).
#   - For example:
#        * If today is December 13th 2017 and we get passed the date/time
#          string "Jan  4 01:00:00", we assume that it is for the next 
#          year (2018).
#        * If today is October 5th 2017, and we get passed the date/time
#          string "Nov  5 01:10:11", we assume that it is for this year.
# If the incoming date has a month "before" the current month, or does
# not fall into the situation above, it's assumed it's from the past.
#   - For example:
#        * If today is July 10th 2017, and the incoming date is for
#          a time in April, the year is assumed to be 2017.
#        * If today is July 10th 2017, and the incoming date is for
#          a time in September, the year is assumed to be 2016.
#

import re
import sys

from datetime import datetime, timedelta

err = 0

# Make tests below a little easier to read.
JAN = 1; FEB =  2; MAR =  3; APR =  4
MAY = 5; JUN =  6; JUL =  7; AUG =  8
SEP = 9; OCT = 10; NOV = 11; DEC = 12

# Run the provided expression and compare its result with the
# expected value. The function expects the expression to be
# passed in as a string so it can be printed to the screen
# as-is when there is an error.
def do_test(expr, val):
    global err

    # Run the expression and record the result
    result = eval(expr)

    # Print a message identifying the failing "test"
    if result != val:
        print("Error: %s. Expected %4d, got %4d!" % (expr, val, result))
        err += 1

# Use a sliding 12-month window (offset by one month)
# to determine the year that should be returned.
# cy - Current Year
# cm - Current Month
# im - Incoming Month
def estimate_year(cy, cm, im):
	im += 12

	if (im - cm) == 1:
		if cm == 12 and im == 13:
			return cy + 1

	if (im - cm) > 13:
		return cy - 1

	return cy;

# A quick and dirty unit test to validate that our
# estimate_year() function is working as it should. 
def self_test():

    # Where the incoming month is within one month
    # in the future. Should be the NEXT year if
    # the current date is in December, or the SAME
    # year if it's not December.
    do_test("estimate_year(2017, DEC, JAN)", 2018)
    do_test("estimate_year(2017, NOV, DEC)", 2017)
    do_test("estimate_year(2017, OCT, NOV)", 2017)
    do_test("estimate_year(2017, SEP, OCT)", 2017)
    do_test("estimate_year(2017, AUG, SEP)", 2017)

    # These tests validate months that are MORE than
    # one month in the future OR are before the current
    # month. If, numerically, the month comes after the 
    # current month, it's assumed to be for the year
    # PRIOR, otherwise it's assumed to be from THIS year.
    do_test("estimate_year(2017, NOV, JAN)", 2017)
    do_test("estimate_year(2017, NOV, FEB)", 2017)
    do_test("estimate_year(2017, AUG, OCT)", 2016)
    do_test("estimate_year(2017, AUG, MAR)", 2017)
    do_test("estimate_year(2017, APR, JUL)", 2016)

    do_test("estimate_year(2017, AUG, JAN)", 2017)
    do_test("estimate_year(2017, APR, FEB)", 2017)

    # Additional validations based on what was described
    # above.
    do_test("estimate_year(2017, JAN, DEC)", 2016)
    do_test("estimate_year(2017, JAN, FEB)", 2017)
    do_test("estimate_year(2017, JAN, MAR)", 2016)

# Convert a datetime.timedelta object to a UNIX timestamp
def get_total_seconds(dt):
    # timedelta.total_seconds() wasn't added until
    # Python 2.7, which CentOS 6 doesn't have.

    if hasattr(timedelta, "total_seconds"):
        return dt.total_seconds()
    return dt.seconds + dt.days * 24 * 3600

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Invalid number of arguments!")
        sys.exit(1)

    if sys.argv[1] == "selftest":
        self_test()

        # Exit with non-zero if there were failures,
        # zero otherwise.
        sys.exit(err)

    months = [None, "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"]

    current_datetime = datetime.utcnow()

    # The argument is expected to be an RFC 3164 timestamp
    # such as "Oct  5 01:10:11".
    incoming_datetime = sys.argv[1]

    # Get the name of the month from the date/time string that was passed in
    # and convert it to its ordinal number (1 for Jan, 10 for Oct, etc...)
    incoming_month = re.search(r"^([^ ]+) ", incoming_datetime).group(1)
    incoming_month = months.index(incoming_month)

    # Assume a year for the date/time passed in based off of today's date.
    estimated_year = estimate_year(
        current_datetime.year,
        current_datetime.month,
        incoming_month
    )

    # Convert the date/time string (now with a year, e.g. "Oct  5 01:10:11 2017") to
    # a python datetime object that we can use to calculate a UNIX timestamp
    calculated_datetime = datetime.strptime("%s %d" % (incoming_datetime, estimated_year), "%b %d %H:%M:%S %Y")

    # Convert the datetime object to a UNIX timestamp by subtracting it from the epoch
    print(int( get_total_seconds(calculated_datetime - datetime(1970,1,1)) ))
