.. SPDX-License-Identifier: GPL-2.0-or-later:

.. index::
   single: env (command)

env command
===========

Synopsis
--------

::

	env ask name [message] [size]
	env callbacks
	env default [-f] (-a | var [...])
	env delete [-f] var [...]
	env edit name
	env exists name
	env export [-t | -b | -c] [-s size] addr [var ...]
	env flags
	env grep [-e] [-n | -v | -b] string [...]
	env import [-d] [-t [-r] | -b | -c] addr [size] [var ...]
	env info [-d] [-p] [-q]
	env print [-a | name ...]
	env print -e [-guid guid] [-n] [name ...]
	env run var [...]
	env save
	env erase
	env load
	env select [target]
	env set [-f] name [value]
	env set -e [-nv][-bs][-rt][-at][-a][-i addr:size][-v] name [value]

Description
-----------

The *env* commands is used to handle the U-Boot (:doc:`../environment`) or
the UEFI variables.

The next commands are kept as alias and for compatibility:

+ *editenv* = *env edit*
+ *grepenv* = *env grep*
+ *setenv* = *env set*
+ *askenv* = *env ask*
+ *run* = *env run*

Ask
~~~

The *env ask* command asks for the new value of an environment variable
(alias :doc:`askenv`).

    name
        name of the environment variable.

    message
        message to be displayed while the command waits for the value to be
        entered from stdin. If no message is specified, a default message
        "Please enter name:" will be displayed.

    size
        maximum number of characters that will be stored in the environment
        variable name. This is in decimal number format (unlike in
        other commands where size values are hexa-decimal). The default
        value of size is 1023 (CONFIG_SYS_CBSIZE - 1).

Callbacks
~~~~~~~~~

The *env callbacks* command prints callbacks and their associated variables.

Default
~~~~~~~

The *env default* command resets the selected variables in the U-Boot
environment to their default values.

    var
        list of variable name.
    \-a
        all U-Boot environment.
    \-f
        forcibly, overwrite read-only/write-once variables.

Delete
~~~~~~

The *env delete* command deletes the selected variables from the U-Boot
environment.

    var
        name of the variable to delete.
    \-f
        forcibly, overwrite read-only/write-once variables.

Edit
~~~~

The *env edit* command edits an environment variable.

    name
        name of the variable.

Exists
~~~~~~

The *env exists* command tests for existence of variable.

    name
        name of the variable.

Export
~~~~~~

The *env export* command exports the U-Boot environment in memory; on success,
the variable $filesize will be set.

    addr
        memory address where environment gets stored.
    var
        list of variable names that get included into the export.
        Without arguments, the whole environment gets exported.
    \-b
        export as binary format (name=value pairs separated by
        list end marked by double "\0\0").
    \-t
        export as text format; if size is given, data will be
        padded with '\0' bytes; if not, one terminating '\0'
        will be added.
    \-c
        Export as checksum protected environment format as used by
        'env save' command.
    \-s size
        size of output buffer.

Flags
~~~~~

The *env flags* command prints variables that have non-default flags.

Grep
~~~~

The *env grep* command searches environment, list environment name=value pairs
matching the requested 'string'.

    string
        string to search in U-Boot environment.
    \-e
        enable regular expressions.
    \-n
        search string in variable names.
    \-v
        search string in vairable values.
    \-b
        search both names and values (default).

Import
~~~~~~

The *env import* command imports environment from memory.

    addr
        memory address to read from.
    size
        length of input data; if missing, proper '\0' termination is mandatory
        if var is set and size should be missing (i.e. '\0' termination),
        set size to '-'.
    var
        List of the names of the only variables that get imported from
        the environment at address 'addr'. Without arguments, the whole
        environment gets imported.
    \-d
        delete existing environment before importing if no var is passed;
        if vars are passed, if one var is in the current environment but not
        in the environment at addr, delete var from current environment;
        otherwise overwrite / append to existing definitions.
    \-t
        assume text format; either "size" must be given or the text data must
        be '\0' terminated.
    \-r
        handle CRLF like LF, that means exported variables with a content which
        ends with \r won't get imported. Used to import text files created with
        editors which are using CRLF for line endings.
        Only effective in addition to -t.
    \-b
        assume binary format ('\0' separated, "\0\0" terminated).
    \-c
        assume checksum protected environment format.

Info
~~~~

The *env info* command displays (without argument) or evaluates the U-Boot
environment information.

    \-d
        evaluate if the default environment is used.
    \-p
        evaluate if environment can be persisted.
    \-q
        quiet output,  use only for command result, by example with
        'test' command.

Print
~~~~~

The *env print* command prints the selected variables in U-Boot environment or
in UEFI variables.

    name
        list of variable name.
    \-a
        all U-Boot environment, when 'name' is absent.
    \-e
        print UEFI variables, all by default when 'name'.
    \-guid guid
        print only the UEFI variables matching this GUID (any by default)
        with guid format = "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx".
    \-n
         suppress dumping variable's value for UEFI.

Run
~~~

The *env run* command runs commands in an environment variable.

    var
        name of the variable.

Save
~~~~

The *env save* command saves the U-Boot environment in persistent storage.

Erase
~~~~~

The *env erase* command erases the U-Boot environment.

Load
~~~~

The *env load* command loads the U-Boot environment from persistent storage.

Select
~~~~~~

The *env select* command selects an U-Boot environment target, it is useful to
overid the default location when several U-Boot environment backend are
availables.

    target
        name of the U-Boot environment backend to select: EEPROM, EXT4, FAT,
        Flash, MMC, NAND, nowhere, NVRAM, OneNAND, Remote, SATA, SPIFlash, UBI.


Set
~~~

The *env set* command sets or delete (when 'value' or '-i' are absent)
U-Boot variable in environment or UEFI variables (when -e is specified).

    name
        variable name to modify.
    value
        when present, set the environment variable 'name' to 'value'
        when absent, delete the environment variable 'name'.
    \-f
        forcibly, overwrite read-only/write-once U-Boot variables.
    \-e
        update UEFI variables.
    \-nv
        set non-volatile attribute (UEFI).
    \-bs
        set boot-service attribute (UEFI).
    \-rt
        set runtime attribute (UEFI).
    \-at
        set time-based authentication attribute (UEFI).
    \-a
        append-write (UEFI).
    \-i addr:size
        use <addr,size> as variable's value (UEFI).
    \-v
        verbose message (UEFI).

Example
-------

Print the U-Boot environment variables::

    => env print -a
    => env print bootcmd stdout

Update environment variable in memory::

    => env set bootcmd "run distro_bootcmd"
    => env set stdout "serial,vidconsole"

Delete environment variable in memory::

    => env delete bootcmd
    => env set bootcmd

Reset environment variable to default value, in memory::

    => env default bootcmd
    => env default -a

Save current environment in persistent storage::

    => env save

Restore the default environment in persistent storage::

    => env erase

Create a text snapshot/backup of the current settings in RAM
(${filesize} can be use to save the snapshot in file)::

    => env export -t ${backup_addr}

Re-import this snapshot, deleting all other settings::

    => env import -d -t ${backup_addr}

Save environment if default enviromnent is used and persistent storage is
selected::

    => if env info -p -d -q; then env save; fi

Configuration
-------------

The env command is always available but some sub-commands depend on
configuration options:

ask
    CONFIG_CMD_ASKENV

callback
    CONFIG_CMD_ENV_CALLBACK

edit
    CONFIG_CMD_EDITENV

exists
    CONFIG_CMD_ENV_EXISTS

flags
    CONFIG_CMD_ENV_FLAGS

erase
    CONFIG_CMD_ERASEENV

export
    CONFIG_CMD_EXPORTENV

grep
    CONFIG_CMD_GREPENV, CONFIG_REGEX for '-e' option

import
    CONFIG_CMD_IMPORTENV

info
    CONFIG_CMD_NVEDIT_INFO

load
    CONFIG_CMD_NVEDIT_LOAD

run
    CONFIG_CMD_RUN

save
    CONFIG_CMD_SAVEENV

select
    CONFIG_CMD_NVEDIT_SELECT

set, print
    CONFIG_CMD_NVEDIT_EFI for '-e' option
