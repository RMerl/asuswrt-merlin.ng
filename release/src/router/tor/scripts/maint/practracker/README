Practracker is a simple python tool that keeps track of places where
our code is ugly, and tries to warn us about new ones or ones that
get worse.

Right now, practracker looks for the following kinds of
best-practices violations:

  .c files greater than 3000 lines long
  .h files greater than 500 lines long
  .c files with more than 50 includes
  .h files with more than 15 includes

  All files that include a local header not listed in a .may_include
  file in the same directory, when that .may_include file has an
  "!advisory" marker.

The list of current violations is tracked in exceptions.txt; slight
deviations of the current exceptions cause warnings, whereas large
ones cause practracker to fail.

For usage information, run "practracker.py --help".
