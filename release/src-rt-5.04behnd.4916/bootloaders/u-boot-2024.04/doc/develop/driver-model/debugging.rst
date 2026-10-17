.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Simon Glass <sjg@chromium.org>

Debugging driver model
======================

This document aims to provide help when you cannot work out why driver model is
not doing what you expect.


Useful techniques in general
----------------------------

Here are some useful debugging features generally.

   - If you are writing a new feature, consider doing it in sandbox instead of
     on your board. Sandbox has no limits, allows easy debugging (e.g. gdb) and
     you can write emulators for most common devices.
   - Put '#define DEBUG' at the top of a file, to activate all the debug() and
     log_debug() statements in that file.
   - Where logging is used, change the logging level, e.g. in SPL with
     CONFIG_SPL_LOG_MAX_LEVEL=7 (which is LOGL_DEBUG) and
     CONFIG_LOG_DEFAULT_LEVEL=7
   - Where logging of return values is implemented with log_msg_ret(), set
     CONFIG_LOG_ERROR_RETURN=y to see exactly where the error is happening
   - Make sure you have a debug UART enabled - see CONFIG_DEBUG_UART. With this
     you can get serial output (printf(), etc.) before the serial driver is
     running.
   - Use a JTAG emulator to set breakpoints and single-step through code

Not that most of these increase code/data size somewhat when enabled.


Failure to locate a device
--------------------------

Let's say you have uclass_first_device_err() and it is not finding anything.

If it is returning an error, then that gives you a clue. Look up linux/errno.h
to see errors. Common ones are:

   - -ENOMEM which indicates that memory is short. If it happens in SPL or
     before relocation in U-Boot, check CONFIG_SPL_SYS_MALLOC_F_LEN and
     CONFIG_SYS_MALLOC_F_LEN as they may need to be larger. Add '#define DEBUG'
     at the very top of malloc_simple.c to get an idea of where your memory is
     going.
   - -EINVAL which typically indicates that something was missing or wrong in
     the device tree node. Check that everything is correct and look at the
     of_to_plat() method in the driver.

If there is no error, you should check if the device is actually bound. Call
dm_dump_tree() just before you locate the device to make sure it exists.

If it does not exist, check your device tree compatible strings match up with
what the driver expects (in the struct udevice_id array).

If you are using of-platdata (e.g. CONFIG_SPL_OF_PLATDATA), check that the
driver name is the same as the first compatible string in the device tree (with
invalid-variable characters converted to underscore).

If you are really stuck, putting '#define LOG_DEBUG' at the top of
drivers/core/lists.c should show you what is going on.
