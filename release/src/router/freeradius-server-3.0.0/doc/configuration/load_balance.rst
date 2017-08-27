Load Balancing
==============

As of version 2.0.0, the load balance documentation is in the
available in the "unlang" man page.  The text below may not be up to
date, and is here only for historical purposes.

As of version 1.1.0, FreeRADIUS supports load balancing in module
sections.  Please see the "configurable_failover" file in this
directory for a more complete description of module sections.

The short summary is that you can use a "load-balance" section in
any place where a module name may be used.  The semantics of the
"load-balance" section are that one of the modules in the section will
be chosen at random, evenly spread over the modules in the list.

An example is below::

    accounting {
            load-balance {
                    sql1
                    sql2
                    sql2
            }
    }

In this case, 1/3 of the RADIUS requests will be processed by
"sql1", one third by "sql2", and 1/3 by "sql3".

The "load-balance" section can be nested in a "redundant" section,
or vice-versa::

    accounting {
            load-balance {		# between two redundant sections below
                    redundant {
                            sql1
                            sql2
                    }
                    redundant {
                            sql2
                            sql1
                    }
            }
    }

This says "load balance between sql1 and sql2, but if sql1 is down,
use sql2, and if sql2 is down, use sql1".  That way, you can guarantee
both that load balancing occurs, and that the requests are *always*
logged to one of the databases::

    accounting {
            redundant {
                    load-balance {
                            sql1
                            sql2
                    }
                    detail
            }
    }

This says "load balance between sql1 and sql2, but if the one being
used is down, then log to detail".

And finally::

    accounting {
            redundant {			# between load-balance & detail
                    load-balance {		# between two redundant sections
                            redundant {
                                    sql1
                                    sql2
                            }
                            redundant {
                                    sql2
                                    sql1
                            }
                    }
                    detail
            }
    }

This says "try to load balance between sql1 and sql2; if sql1 is down,
use sql2; if sql2 is down use sql1; if both sql1 and sql2 are down,
then log to the detail file"


More complicated scenarios
--------------------------

If you want to do redundancy and load-balancing among three
modules, the configuration is quite complex::

    load-balance {
        redundant {
            sql1
            load-balance {
                redundant {
                    sql2
                    sql3
                }
                redundant {
                    sql3
                    sql2
                }
            }
        } # sql1, etc.
        redundant {
            sql2
            load-balance {
                redundant {
                    sql3
                    sql1
                }
                redundant {
                    sql1
                    sql3
                }
            }
        } # sql2, etc.
        redundant {
            sql3
            load-balance {
                redundant {
                    sql1
                    sql2
                }
                redundant {
                    sql2
                    sql1
                }
            }
        } # sql3, etc.
    }

For four or more modules, it quickly becomes unmanageable.

The solution is to use the "redundant-load-balance" section, which
combines the features of "load-balance", with "redundant" fail-over
between members.  The above complex configuration for three modules
then becomes::

    redundant-load-balance {
            sql1
            sql2
            sql3
    }


Which means "load-balance evenly among all three servers.  If the
one picked for load-balancing is down, load-balance among the
remaining two.  If that one is down, pick the one remaining 'live'
server".

The "redundant-load-balance" section can contain any number of
modules.


Interaction with "if" and "else"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

It's best to have "if" and "else" blocks contain "load-balance" or
"redundant-load-balance" sections, rather than the other way around.
The "else" and "elsif" sections cannot appear inside of a
"load-balance" or "redundant-load-balance" section, because the "else"
condition would be chose as one of the modules for load-balancing,
which is not what you want.

It's OK to have a plain "if" block inside of a "load-balance" or
"redundant-load-balance" section.  In that case, the "if" condition
checks the return code of the module or group that executed just
before the "load-balance" section.  It does *not* check the return
code of the previous module in the section.
