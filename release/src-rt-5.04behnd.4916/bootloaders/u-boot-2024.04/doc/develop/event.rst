.. SPDX-License-Identifier: GPL-2.0+

Events
======

U-Boot supports a way for various events to be handled by interested
subsystems. This provide a generic way to handle 'hooks' like setting up the
CPUs after driver model is active, or reading a partition table after a new
block device is probed.

Rather than using weak functions and direct calls across subsystemss, it is
often easier to use an event.

An event consists of a type (e.g. EVT_DM_POST_INIT_F) and some optional data,
in `union event_data`. An event spy can be created to watch for events of a
particular type. When the event is created, it is sent to each spy in turn.


Declaring a spy
---------------

To declare a spy, use something like this::

    static int snow_check_temperature(void)
    {
        /* do something */
        return 0;
    }
    EVENT_SPY_SIMPLE(EVT_DM_POST_INIT_F, snow_check_temperature);

This function is called when EVT_DM_POST_INIT_F is emitted, i.e. after the
driver model is initialized (in U-Boot proper before and after relocation).

If you need access to the event data, use `EVENT_SPY_FULL`, like this::

    static int snow_setup_cpus(void *ctx, struct event *event)
    {
        /* do something that uses event->data*/
        return 0;
    }
    EVENT_SPY_FULL(EVT_DM_POST_INIT_F, snow_setup_cpus);

Note that the context is always NULL for a static spy. See below for information
about how to use a dynamic spy.

The return value is handled by the event emitter. If non-zero, then the error
is returned to the function which emitted the event, i.e. the one that called
`event_notify()`.

Debugging
---------

To assist with debugging events, enable `CONFIG_EVENT_DEBUG` and
`CONFIG_CMD_EVENT`. The :doc:`../usage/cmd/event` command can then be used to
provide a spy list.

It is also possible to list spy information from the U-Boot executable,, using
the `event_dump.py` script::

    $ scripts/event_dump.py /tmp/b/sandbox/u-boot
    Event type            Id                              Source location
    --------------------  ------------------------------  ------------------------------
    EVT_MISC_INIT_F       f:sandbox_misc_init_f           arch/sandbox/cpu/start.c:125

This shows each event spy in U-Boot, along with the event type, function name
(or ID) and source location.

Note that if `CONFIG_EVENT_DEBUG` is not enabled, the event ID is missing, so
the function is shown instead (with an `f:` prefix as above). Since the ID is
generally the same as the function name, this does not matter much.

The event type is decoded by the symbol used by U-Boot for the event linker
list. Symbols have the form::

    _u_boot_list_2_evspy_info_2_EVT_MISC_INIT_F

so the event type can be read from the end. To manually list spy information
in an image, use $(CROSS_COMPILE)nm::

    nm u-boot |grep evspy |grep list
    00000000002d6300 D _u_boot_list_2_evspy_info_2_EVT_MISC_INIT_F

Logging is also available. Events use category `LOGC_EVENT`, so you can enable
logging on that, or add `#define LOG_DEBUG` to the top of `common/event.c` to
see events being sent.


Dynamic events
--------------

Static events provide a way of dealing with events known at build time. In some
cases we want to attach an event handler at runtime. For example, we may wish
to be notified when a particular device is probed or removed.

This can be handled by enabling `CONFIG_EVENT_DYNAMIC`. It is then possible to
call `event_register()` to register a new handler for a particular event.

If some context is need for the spy, you can pass a pointer to
`event_register()` to provide that. Note that the context is only passed to
a spy registered with `EVENT_SPY_FULL`.

Dynamic event handlers are called after all the static event spy handlers have
been processed. Of course, since dynamic event handlers are created at runtime
it is not possible to use the `event_dump.py` to see them.

At present there is no way to list dynamic event handlers from the command line,
nor to deregister a dynamic event handler. These features can be added when
needed.
