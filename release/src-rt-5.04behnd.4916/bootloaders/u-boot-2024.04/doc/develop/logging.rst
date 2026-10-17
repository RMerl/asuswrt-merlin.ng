.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (c) 2017 Simon Glass <sjg@chromium.org>

Logging in U-Boot
=================

Introduction
------------

U-Boot's internal operation involves many different steps and actions. From
setting up the board to displaying a start-up screen to loading an Operating
System, there are many component parts each with many actions.

Most of the time this internal detail is not useful. Displaying it on the
console would delay booting (U-Boot's primary purpose) and confuse users.

But for digging into what is happening in a particular area, or for debugging
a problem it is often useful to see what U-Boot is doing in more detail than
is visible from the basic console output.

U-Boot's logging feature aims to satisfy this goal for both users and
developers.

Logging levels
--------------

There are a number logging levels available.

See enum :c:type:`log_level_t`

Logging category
----------------

Logging can come from a wide variety of places within U-Boot. Each log message
has a category which is intended to allow messages to be filtered according to
their source.

See enum :c:type:`log_category_t`

Enabling logging
----------------

The following options are used to enable logging at compile time:

* CONFIG_LOG - Enables the logging system
* CONFIG_LOG_MAX_LEVEL - Max log level to build (anything higher is compiled
  out)
* CONFIG_LOG_CONSOLE - Enable writing log records to the console

If CONFIG_LOG is not set, then no logging will be available.

The above have SPL and TPL versions also, e.g. CONFIG_SPL_LOG_MAX_LEVEL and
CONFIG_TPL_LOG_MAX_LEVEL.

If logging is disabled, the default behaviour is to output any message at
level LOGL_INFO and below. If logging is disabled and DEBUG is defined (at
the very top of a C file) then any message at LOGL_DEBUG will be written.

Temporary logging within a single file
--------------------------------------

Sometimes it is useful to turn on logging just in one file. You can use this

.. code-block:: c

   #define LOG_DEBUG

to enable building in of all logging statements in a single file. Put it at
the top of the file, before any #includes and any message in the file will be
written, regardless of the value of CONFIG_LOG_DEFAULT_LEVEL.

Using DEBUG
-----------

U-Boot has traditionally used a #define called DEBUG to enable debugging on a
file-by-file basis but LOG_DEBUG are intended to replace it with the logging
facilities; DEBUG is activated when LOG_DEBUG is activated.

With logging enabled, debug() statements are interpreted as logging output
with a level of LOGL_DEBUG and a category of LOG_CATEGORY.

With logging disabled, the debug() macro compiles to a printf() statement
if DEBUG is enabled and to an empty statement if not.

Logging statements
------------------

The main logging function is:

.. code-block:: c

   log(category, level, format_string, ...)

Also debug() and error() will generate log records  - these use LOG_CATEGORY
as the category, so you should #define this right at the top of the source
file to ensure the category is correct.

Generally each log format_string ends with a newline. If it does not, then the
next log statement will have the LOGRECF_CONT flag set. This can be used to
continue the statement on the same line as the previous one without emitting
new header information (such as category/level). This behaviour is implemented
with log_console. Here is an example that prints a list all on one line with
the tags at the start:

.. code-block:: c

   log_debug("Here is a list:");
   for (i = 0; i < count; i++)
      log_debug(" item %d", i);
   log_debug("\n");

Also see the special category LOGL_CONT and level LOGC_CONT.

You can also define CONFIG_LOG_ERROR_RETURN to enable the log_ret() macro. This
can be used whenever your function returns an error value:

.. code-block:: c

   return log_ret(uclass_first_device_err(UCLASS_MMC, &dev));

This will write a log record when an error code is detected (a value < 0). This
can make it easier to trace errors that are generated deep in the call stack.

The log_msg_ret() variant will print a short string if CONFIG_LOG_ERROR_RETURN
is enabled. So long as the string is unique within the function you can normally
determine exactly which call failed:

.. code-block:: c

   ret = gpio_request_by_name(dev, "cd-gpios", 0, &desc, GPIOD_IS_IN);
   if (ret)
      return log_msg_ret("gpio", ret);

Some functions return 0 for success and any other value is an error. For these,
log_retz() and log_msg_retz() are available.

Convenience functions
~~~~~~~~~~~~~~~~~~~~~

A number of convenience functions are available to shorten the code needed
for logging:

* log_err(_fmt...)
* log_warning(_fmt...)
* log_notice(_fmt...)
* log_info(_fmt...)
* log_debug(_fmt...)
* log_content(_fmt...)
* log_io(_fmt...)

With these the log level is implicit in the name. The category is set by
LOG_CATEGORY, which you can only define once per file, above all #includes, e.g.

.. code-block:: c

	#define LOG_CATEGORY LOGC_ALLOC

or

.. code-block:: c

	#define LOG_CATEGORY UCLASS_SPI

Remember that all uclasses IDs are log categories too.

Logging destinations
--------------------

If logging information goes nowhere then it serves no purpose. U-Boot provides
several possible determinations for logging information, all of which can be
enabled or disabled independently:

* console - goes to stdout
* syslog - broadcast RFC 3164 messages to syslog servers on UDP port 514

The syslog driver sends the value of environmental variable 'log_hostname' as
HOSTNAME if available.

Filters
-------

Filters are attached to log drivers to control what those drivers emit. FIlters
can either allow or deny a log message when they match it. Only records which
are allowed by a filter make it to the driver.

Filters can be based on several criteria:

* minimum or maximum log level
* in a set of categories
* in a set of files

If no filters are attached to a driver then a default filter is used, which
limits output to records with a level less than CONFIG_MAX_LOG_LEVEL.

Log command
-----------

The 'log' command provides access to several features:

* level - list log levels or set the default log level
* categories - list log categories
* drivers - list log drivers
* filter-list - list filters
* filter-add - add a new filter
* filter-remove - remove filters
* format - access the console log format
* rec - output a log record

Type 'help log' for details.

Log format
~~~~~~~~~~

You can control the log format using the 'log format' command. The basic
format is::

   LEVEL.category,file.c:123-func() message

In the above, file.c:123 is the filename where the log record was generated and
func() is the function name. By default ('log format default') only the message
is displayed on the console. You can control which fields are present, but not
the field order.

Adding Filters
~~~~~~~~~~~~~~

To add new filters at runtime, use the 'log filter-add' command. For example, to
suppress messages from the SPI and MMC subsystems, run::

    log filter-add -D -c spi -c mmc

You will also need to add another filter to allow other messages (because the
default filter no longer applies)::

    log filter-add -A -l info

Log levels may be either symbolic names (like above) or numbers. For example, to
disable all debug and above (log level 7) messages from ``drivers/core/lists.c``
and ``drivers/core/ofnode.c``, run::

    log filter-add -D -f drivers/core/lists.c,drivers/core/ofnode.c -L 7

To view active filters, use the 'log filter-list' command. Some example output
is::

    => log filter-list
    num policy level            categories files
      2   deny >= DEBUG                    drivers/core/lists.c,drivers/core/ofnode.c
      0   deny <= IO                   spi
                                       mmc
      1  allow <= INFO

Note that filters are processed in-order from top to bottom, not in the order of
their filter number. Filters are added to the top of the list if they deny when
they match, and to the bottom if they allow when they match. For more
information, consult the usage of the 'log' command, by running 'help log'.

Code size
---------

Code size impact depends largely on what is enabled. The following numbers are
generated by 'buildman -S' for snow, which is a Thumb-2 board (all units in
bytes)::

    This series: adds bss +20.0 data +4.0 rodata +4.0 text +44.0
    CONFIG_LOG: bss -52.0 data +92.0 rodata -635.0 text +1048.0
    CONFIG_LOG_MAX_LEVEL=7: bss +188.0 data +4.0 rodata +49183.0 text +98124.0

The last option turns every debug() statement into a logging call, which
bloats the code hugely. The advantage is that it is then possible to enable
all logging within U-Boot.

To Do
-----

There are lots of useful additions that could be made. None of the below is
implemented! If you do one, please add a test in test/log/log_test.c
log filter-add -D -f drivers/core/lists.c,drivers/core/ofnode.c -l 6
Convenience functions to support setting the category:

* log_arch(level, format_string, ...) - category LOGC_ARCH
* log_board(level, format_string, ...) - category LOGC_BOARD
* log_core(level, format_string, ...) - category LOGC_CORE
* log_dt(level, format_string, ...) - category LOGC_DT

More logging destinations:

* device - goes to a device (e.g. serial)
* buffer - recorded in a memory buffer

Convert debug() statements in the code to log() statements

Convert error() statements in the code to log() statements

Figure out what to do with BUG(), BUG_ON() and warn_non_spl()

Add a way to browse log records

Add a way to record log records for browsing using an external tool

Add commands to add and remove log devices

Allow sharing of printf format strings in log records to reduce storage size
for large numbers of log records

Consider making log() calls emit an automatic newline, perhaps with a logn()
function to avoid that

Passing log records through to linux (e.g. via device tree /chosen)

Provide a command to access the number of log records generated, and the
number dropped due to them being generated before the log system was ready.

Add a printf() format string pragma so that log statements are checked properly

Add a command to delete existing log records.
