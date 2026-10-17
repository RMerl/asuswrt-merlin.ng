.. SPDX-License-Identifier: GPL-2.0+

Cyclic functions
================

The cyclic function execution infrastruture provides a way to periodically
execute code, e.g. every 100ms. Examples for such functions might be LED
blinking etc. The functions that are hooked into this cyclic list should
be small timewise as otherwise the execution of the other code that relies
on a high frequent polling (e.g. UART rx char ready check) might be
delayed too much. To detect cyclic functions with an excessive execution
time, the Kconfig option `CONFIG_CYCLIC_MAX_CPU_TIME_US` was introduced.
It defines the maximum allowable execution time for such a cyclic function. The
first time the execution of a cyclic function exceeds this interval, a warning
will be displayed indicating the problem to the user.

Registering a cyclic function
-----------------------------

To register a cyclic function, use something like this::

    static void cyclic_demo(void *ctx)
    {
        /* Just a small dummy delay here */
        udelay(10);
    }
    
    int board_init(void)
    {
        struct cyclic_info *cyclic;
        
        /* Register demo cyclic function */
        cyclic = cyclic_register(cyclic_demo, 10 * 1000, "cyclic_demo", NULL);
        if (!cyclic)
        printf("Registering of cyclic_demo failed\n");
        
        return 0;
    }

This will register the function `cyclic_demo()` to be periodically
executed all 10ms.

How is this cyclic functionality integrated /  executed?
--------------------------------------------------------

The cyclic infrastructure integrates the main function responsible for
calling all registered cyclic functions cyclic_run() into the common
WATCHDOG_RESET macro. This guarantees that cyclic_run() is executed
very often, which is necessary for the cyclic functions to get scheduled
and executed at their configured periods.
