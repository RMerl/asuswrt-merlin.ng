.. index::
   single: for (command)

for command
===========

Synopsis
--------

::

    for <variable> in <items>; do <commands>; done

Description
-----------

The for command is used to loop over a list of values and execute a series of
commands for each of these.

The counter variable of the loop is a shell variable. Please, keep in mind that
an environment variable takes precedence over a shell variable of the same name.

variable
    name of the counter variable

items
    space separated item list

commands
    commands to execute

Example
-------

::

    => setenv c
    => for c in 1 2 3; do echo item ${c}; done
    item 1
    item 2
    item 3
    => echo ${c}
    3
    => setenv c x
    => for c in 1 2 3; do echo item ${c}; done
    item x
    item x
    item x
    =>

The first line ensures that there is no environment variable *c*. Hence in the
first loop the shell variable *c* is printed.

After defining an environment variable of name *c* it takes precedence over the
shell variable and the environment variable is printed.

Return value
------------

The return value $? after the done statement is the return value of the last
statement executed in the loop.

::

    => for i in true false; do ${i}; done; echo $?
    1
    => for i in false true; do ${i}; done; echo $?
    0
