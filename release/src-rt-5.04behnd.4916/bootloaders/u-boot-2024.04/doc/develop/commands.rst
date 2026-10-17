.. SPDX-License-Identifier: GPL-2.0+

Implementing shell commands
===========================

Command definition
------------------

Commands are added to U-Boot by creating a new command structure.
This is done by first including command.h, then using the U_BOOT_CMD() or the
U_BOOT_CMD_COMPLETE macro to fill in a struct cmd_tbl structure.

.. code-block:: c

    U_BOOT_CMD(name, maxargs, repeatable, command, "usage", "help")
    U_BOOT_CMD_COMPLETE(name, maxargs, repeatable, command, "usage, "help", comp)

name
    The name of the command. This is **not** a string.

maxargs
    The maximum number of arguments this function takes including
    the command itself.

repeatable
    Either 0 or 1 to indicate if autorepeat is allowed.

command
    Pointer to the command function. This is the function that is
    called when the command is issued.

usage
    Short description. This is a string.

help
    Long description. This is a string. The long description is
    only available if CONFIG_SYS_LONGHELP is defined.

comp
    Pointer to the completion function. May be NULL.
    This function is called if the user hits the TAB key while
    entering the command arguments to complete the entry. Command
    completion is only available if CONFIG_AUTO_COMPLETE is defined.

Sub-command definition
----------------------

Likewise an array of struct cmd_tbl holding sub-commands can be created using
either of the following macros:

.. code-block:: c

    U_BOOT_CMD_MKENT(name, maxargs, repeatable, command, "usage", "help")
    U_BOOT_CMD_MKENTCOMPLETE(name, maxargs, repeatable, command, "usage, "help", comp)

This table has to be evaluated in the command function of the main command, e.g.

.. code-block:: c

    static struct cmd_tbl cmd_sub[] = {
        U_BOOT_CMD_MKENT(foo, CONFIG_SYS_MAXARGS, 1, do_foo, "", ""),
        U_BOOT_CMD_MKENT(bar, CONFIG_SYS_MAXARGS, 1, do_bar, "", ""),
    };

    static int do_cmd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
    {
        struct cmd_tbl *cp;

        if (argc < 2)
                return CMD_RET_USAGE;

        /* drop sub-command argument */
        argc--;
        argv++;

        cp = find_cmd_tbl(argv[0], cmd_ut_sub, ARRAY_SIZE(cmd_sub));

        if (cp)
            return cp->cmd(cmdtp, flag, argc, argv);

        return CMD_RET_USAGE;
    }

Command function
----------------

The command function pointer has to be of type

.. code-block:: c

    int (*cmd)(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);

cmdtp
    Table entry describing the command (see above).

flag
    A bitmap which may contain the following bits

    * CMD_FLAG_REPEAT - The last command is repeated.
    * CMD_FLAG_BOOTD  - The command is called by the bootd command.
    * CMD_FLAG_ENV    - The command is called by the run command.

argc
    Number of arguments including the command.

argv
    Arguments.

Allowable return value are:

CMD_RET_SUCCESS
    The command was successfully executed.

CMD_RET_FAILURE
    The command failed.

CMD_RET_USAGE
    The command was called with invalid parameters. This value
    leads to the display of the usage string.

Completion function
-------------------

The completion function pointer has to be of type

.. code-block:: c

    int (*complete)(int argc, char *const argv[], char last_char,
                    int maxv, char *cmdv[]);

argc
    Number of arguments including the command.

argv
    Arguments.

last_char
    The last character in the command line buffer.

maxv
    Maximum number of possible completions that may be returned by
    the function.

cmdv
    Used to return possible values for the last argument. The last
    possible completion must be followed by NULL.

The function returns the number of possible completions (without the terminating
NULL value).

Behind the scene
----------------

The structure created is named with a special prefix and placed by
the linker in a special section using the linker lists mechanism
(see include/linker_lists.h)

This makes it possible for the final link to extract all commands
compiled into any object code and construct a static array so the
command array can be iterated over using the linker lists macros.

The linker lists feature ensures that the linker does not discard
these symbols when linking full U-Boot even though they are not
referenced in the source code as such.

If a new board is defined do not forget to define the command section
by writing in u-boot.lds ($(srctree)/board/boardname/u-boot.lds) these
3 lines:

.. code-block:: c

    __u_boot_list : {
        KEEP(*(SORT(__u_boot_list*)));
    }

Writing tests
-------------

All new commands should have tests. Tests for existing commands are very
welcome.

It is fairly easy to write a test for a command. Enable it in sandbox, and
then add code that runs the command and checks the output.

Here is an example:

.. code-block:: c

    /* Test 'acpi items' command */
    static int dm_test_acpi_cmd_items(struct unit_test_state *uts)
    {
        struct acpi_ctx ctx;
        void *buf;

        buf = malloc(BUF_SIZE);
        ut_assertnonnull(buf);

        ctx.current = buf;
        ut_assertok(acpi_fill_ssdt(&ctx));
        console_record_reset();
        run_command("acpi items", 0);
        ut_assert_nextline("dev 'acpi-test', type 1, size 2");
        ut_assert_nextline("dev 'acpi-test2', type 1, size 2");
        ut_assert_console_end();

        ctx.current = buf;
        ut_assertok(acpi_inject_dsdt(&ctx));
        console_record_reset();
        run_command("acpi items", 0);
        ut_assert_nextline("dev 'acpi-test', type 2, size 2");
        ut_assert_nextline("dev 'acpi-test2', type 2, size 2");
        ut_assert_console_end();

        console_record_reset();
        run_command("acpi items -d", 0);
        ut_assert_nextline("dev 'acpi-test', type 2, size 2");
        ut_assert_nextlines_are_dump(2);
        ut_assert_nextline("%s", "");
        ut_assert_nextline("dev 'acpi-test2', type 2, size 2");
        ut_assert_nextlines_are_dump(2);
        ut_assert_nextline("%s", "");
        ut_assert_console_end();

        return 0;
    }
    DM_TEST(dm_test_acpi_cmd_items, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
