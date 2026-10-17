.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: trace (command)

trace command
=============

Synopsis
--------

::

    trace stats
    trace pause
    trace resume
    trace funclist [<addr> <size>]
    trace calls [<addr> <size>]

Description
-----------

The *trace* command is used to control the U-Boot tracing system. It allows
tracing to be paused and resumed, shows statistics for traces and provides a
way to dump out the trace information.


trace stats
~~~~~~~~~~~

This display tracing statistics, as follows:

function sites
    Functions are binned as a way of reducing the amount of space needed to
    hold all the function information. This is controlled by FUNC_SITE_SIZE in
    the trace.h header file. The usual value is 4, which provides the finest
    granularity (assuming a minimum instruction size of 4 bytes) which means
    that every function can be resolved individually.

function calls
    Total number of function calls, including those which were not traced due
    to buffer space. This count does not include functions which exceeded the
    depth limit.

untracked function calls
    Total number of function calls which did not appear in the U-Boot image.
    This can happen if a function is called outside the normal code area.

traced function calls
    Total number of function calls which were actually traced, i.e. are included
    in the recorded trace data.

dropped due to overflow
    If the trace buffer was exhausted then this shows the number of records that
    were dropped. Try reducing the depth limit or expanding the buffer size.

maximum observed call depth
    Maximum observed call depth while tracing.

calls not traced due to depth
    Counts the number of function calls that were not recorded because they
    exceeded the maximum call depth.

max function calls
    Maximum number of function calls which can be recorded in the trace buffer,
    given its size. Once `function calls` hits this value, recording stops.

trace buffer
    Address of trace buffer

call records
    Address of first trace record. This is near the start of the trace buffer,
    after the function-call counts.


trace pause
~~~~~~~~~~~

Pauses tracing, so that no more data is added to the trace buffer.


trace resume
~~~~~~~~~~~~

Resumes tracing, so that new function calls are added to the trace buffer if
there is sufficient space.


trace funclist [<addr> <size>]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Dumps a list of functions into the provided buffer. The file uses a format
specific to U-Boot: a header, following by the function offset and call count.

If the address and size are not given, these are obtained from
:ref:`develop/trace:environment variables`. In any case the environment
variables are updated after the command runs.

The resulting data should be written out to the host, e.g. using Ethernet or
a filesystem. There are no tools provided to read this sdata.


trace calls [<addr> <size>]
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Dumps a list of function calls into the provided buffer. The file uses a format
specific to U-Boot: a header, following by the list of calls. The proftool
tool can be used to convert this information ready for further analysis.


Example
-------

::

    => trace stats
            269,252 function sites
         38,025,059 function calls
                  3 untracked function calls
          7,382,690 traced function calls
                 17 maximum observed call depth
                 15 call depth limit
         68,667,432 calls not traced due to depth
         22,190,112 max function calls

    trace buffer 6c000000 call records 6c20de78
    => trace resume
    => trace pause

This shows that resuming the trace causes the buffer to overflow::

    => trace stats
            269,252 function sites
         49,573,694 function calls
                  3 untracked function calls
         22,190,112 traced function calls (8289848 dropped due to overflow)
                 17 maximum observed call depth
                 15 call depth limit
         68,667,432 calls not traced due to depth
         22,190,112 max function calls

    trace buffer 6c000000 call records 6c20de78
    => trace funcs 30000000 0x100000
    Function trace dumped to 30000000, size 0x1e70

This shows collecting and writing out the result trace data:

::
    => trace calls 20000000 0x10000000
    Call list dumped to 20000000, size 0xfdf21a0
    => save mmc 1:1 20000000 /trace ${profoffset}
    File System is consistent
    file found, deleting
    update journal finished
    File System is consistent
    update journal finished
    266281376 bytes written in 18584 ms (13.7 MiB/s)

From here you can use proftool to convert it:

.. code-block:: bash

    tools/proftool -m System.map -t trace -o asc.fg dump-ftrace


.. _`ACPI specification`: https://uefi.org/sites/default/files/resources/ACPI_6_3_final_Jan30.pdf
