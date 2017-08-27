Configurable Module Fail Over
=============================

Before configurable module failover, we had this kind of entry in
``radiusd.conf``:

::

  #---
  authorize {
    preprocess
    files
  }
  #---

This entry instructed the ``authorize`` section to first process the
request through the ``preprocess`` module, and if that returned success,
to process it through ``files`` module.  If that sequence returned
success, then the ``authorize`` stage itself would then return success.
Processing was strictly linear and if one module failed, the whole
section would fail immediately.

Configurable failover provides more flexibility. It takes advantage
of the tree structure of radiusd.conf to support a configuration
language that allows you to ``group`` modules that should work together
in ways other than simple lists.  You can control the flow of any
stage (e.g. ``authorize``) to fit your needs, without touching C code,
just by altering radiusd.conf.

This configurable fail-over has a convenient short-hand, too.
Administrators commonly want to say things like "try SQL1, if it's
down, try SQL2, otherwise drop the request."

For example:

::

  #---
  modules {
    sql sql1 {
      # configuration to connect to SQL database one
    }
    sql sql2 {
      # configuration to connect to SQL database two
    }
    always handled {
      rcode = handled
    }
  }

  #  Handle accounting packets
  accounting {
      detail			# always log to detail, stopping if it fails
      redundant {
        sql1			# try module sql1
        sql2			# if that's down, try module sql2
	handled			# otherwise drop the request as
				# it's been ``handled`` by the ``always``
				# module (see doc/rlm_always)
      }
  }
  #---

The ``redundant`` section is a configuration directive which tells the
server to process the second module if the first one fails.  Any
number of modules can be listed in a ``redundant`` section.  The server
will process each in turn, until one of the modules succeeds.  It will then stop processing the ``redundant`` list.

Rewriting results for single modules
------------------------------------

Normally, when a module fails, the entire section (``authorize``,
``accounting``, etc.) stops being processed.  In some cases, we may want
to permit  "soft failures".  That is, we may want to tell the server
that it is "ok" for a module to fail, and that the failure should not
be treated as a fatal error.

In this case, the module is treated as a "section", rather than just
as a single lne in ``radiusd.conf``.  The configuration entries for
that section are taken from the ``configurable fail-over`` code, and not
from the configuration information for that module.

For example, the ``detail`` module normally returns ``fail`` if it is
unable to write its information to the ``detail`` file.  As a test, we
can configure the server so that it continues processing the request,
even if the ``detail`` module fails.  The following example shows how:

::

  #--
  #  Handle accounting packets
  accounting {
      detail {
        fail = 1
      }
      redundant {
        sql1
        sql2
	handled
      }
  }
  #--

The ``fail = 1`` entry tells the server to remember the ``fail`` code,
with priority ``1``.  The normal configuration is ``fail = return``, which
means ``if the detail module fails, stop processing the accounting
section``.

Fail-over configuration entries
-------------------------------

Modules normally return on of the following codes as their result:

+-----------+-----------------------------------------------------+
|Code	    | Meaning                                             |
+===========+=====================================================+
|notfound   | the user was not found                              |
+-----------+-----------------------------------------------------+
|noop	    | the module did nothing                              |
+-----------+-----------------------------------------------------+
|ok	    | the module succeeded                                |
+-----------+-----------------------------------------------------+
|updated    | the module updated information in the request       |
+-----------+-----------------------------------------------------+
|fail       | the module failed                                   |
+-----------+-----------------------------------------------------+
|reject     | the module rejected the user                        |
+-----------+-----------------------------------------------------+
|userlock   | the user was locked out                             |
+-----------+-----------------------------------------------------+
|invalid    | the user's configuration entry was invalid          |
+-----------+-----------------------------------------------------+
|handled    | the module has done everything to handle the request|
+-----------+-----------------------------------------------------+

In a configurable fail-over section, each of these codes may be
listed, with a value.  If the code is not listed, or a configurable
fail-over section is not defined, then values that make sense for the
requested ``group`` (group, redundant, load-balance, etc) are used.

The special code ``default`` can be used to set all return codes to
the specified value.  This value will be used with a lower priority
than ones that are explicitly set.

The values for each code may be one of two things:

+---------+---------------------------------------------------------------+
|Value	  | Meaning                                                       |
+=========+===============================================================+
|<number> | Priority for this return code.                                |
+---------+---------------------------------------------------------------+
|return	  | Stop processing this configurable fail-over list.             |
+---------+---------------------------------------------------------------+
|reject	  | Stop processing this configurable fail-over list and          |
|         | immediately return a reject.                                  |
+---------+---------------------------------------------------------------+

The ``<number>`` used for a value may be any decimal number between 1
and 99999.  The number is used when processing a list of modules, to
determine which code is returned from the list.  For example, if
``module1`` returns ``fail`` with priority ``1``, and a later ``module2``
returns ``ok`` with priority ``3``, the return code from the list of
modules will be ``ok``, because it has higher priority than ``fail``.

This configurability allows the administrator to permit some modules
to fail, so long as a later module succeeds.


More Complex Configurations
---------------------------

The ``authorize`` section is normally a list of module names.  We can
create sub-lists by using the section name ``group``.  The ``redundant``
section above is just a short-hand for ``group``, with a set of default
return codes, which are different than the normal ``stop processing the
list on failure``.

For example, we can configure two detail modules, and allow either
to fail, so long as one of them succeeds.

::

  #--
  #  Handle accounting packets
  accounting {
      group {
        detail1 {
          fail = 1		# remember ``fail`` with priority 1
	  ok = return		# if we succeed, don't do ``detail2``
        }
	detail2 {
	  fail = 1		# remember ``fail`` with priority 1
	  ok = return		# if we succeed, return ``ok``
				# if ``detail1`` returned ``fail``
	}
      }			# returns ``fail`` only if BOTH modules returned ``fail``
      redundant {
        sql1
        sql2
	handled
      }
  }
  #--

This configuration says:

	- log to ``detail1``, and stop processing the ``group`` list if ``detail1`` returned OK.

	- If ``detail1`` returned ``fail``, then continue, but remember the ``fail`` code, with priority 1.

	- If ``detail2`` fails, then remember ``fail`` with priority 1.

	- If ``detail2`` returned ``ok``, return ``ok`` from the ``group``.

The return code from the ``group`` is the return code which was either
forced to return (e.g. ``ok`` for ``detail1``), or the highest priority
return code found by processing the list.

This process can be extended to any number of modules listed in a
``group`` section.


Virtual Modules
---------------

Some configurations may require using the same list of modules, in
the same order, in multiple sections.  For those systems, the
configuration can be simplified through the use of ``virtual`` modules.
These modules are configured as named sub-sections of the
``instantiate`` section, as follows:

::

	instantiate {
		...

		redundant sql1_or_2 {
			sql1
			sql2
		}
	}

The name ``sql1_or_2`` can then be used in any other section, such as
``authorize`` or ``accounting``.  The result will be *exactly* as if that
section was placed at the location of the ``sql1_or_2`` reference.

These virtual modules are full-fledged objects in and of themselves.
One virtual module can refer to another virtual module, and they can
contain ``if`` conditions, or any other configuration permitted in a
section.


Redundancy and Load-Balancing
-----------------------------

See ``man unlang`` or ``doc/load-balance`` for information on simple
redundancy (fail-over) and load balancing.


The Gory Details
-----------------

The fundamental object is called a MODCALLABLE, because it is something that
can be passed a specific radius request and returns one of the RLM_MODULE_*
results. It is a function - if you can accept the fact that pieces of
radiusd.conf are functions. There are two kinds of MODCALLABLEs: GROUPs and
SINGLEs.

A SINGLE is a reference to a module instance that was set up in the modules{}
section of radiusd.conf, like ``preprocess`` or ``sql1``. When a SINGLE is
called, the corresponding function in the rlm is invoked, and whichever
RLM_MODULE_* it returns becomes the RESULT of the SINGLE.

A GROUP is a section of radiusd.conf that includes some MODCALLABLEs.
Examples of GROUPs above include ``authorize{...}``, which implements the C
function module_authorize, and ``redundant{...}``, which contains two SINGLEs
that refer to a couple of redundant databases. Note that a GROUP can contain
other GROUPs - ``Auth-Type SQL{...}`` is also a GROUP, which implements the C
function module_authenticate when Auth-Type is set to SQL.

Now here's the fun part - what happens when a GROUP is called? It simply runs
through all of its children in order, and calls each one, whether it is
another GROUP or a SINGLE. It then looks at the RESULT of that child, and
takes some ACTION, which is basically either ``return that RESULT immediately``
or ``Keep going``. In the first example, any ``bad`` RESULT from the preprocess
module causes an immediate return, and any ``good`` RESULT causes the
authorize{...} GROUP to proceed to the files module.

We can see the exact rules by writing them out the long way:

::

  authorize {
    preprocess {
      notfound = 1
      noop     = 2
      ok       = 3
      updated  = 4
      fail     = return
      reject   = return
      userlock = return
      invalid  = return
      handled  = return
    }
    files {
      notfound = 1
      noop     = 2
      ok       = 3
      updated  = 4
      fail     = return
      reject   = return
      userlock = return
      invalid  = return
      handled  = return
    }
  }

This is the same as the first example, with the behavior explicitly
spelled out. Each SINGLE becomes its own section, containing a list of
RESULTs that it may return and what ACTION should follow from them. So
preprocess is called, and if it returns for example RLM_MODULE_REJECT,
then the reject=return rule is applied, and the authorize{...} GROUP
itself immediately returns RLM_MODULE_REJECT.

If preprocess returns RLM_MODULE_NOOP, the corresponding ACTION is ``2``. An
integer ACTION serves two purposes - first, it tells the parent GROUP to go
on to the next module. Second, it is a hint as to how desirable this RESULT
is as a candidate for the GROUP's own RESULT. So files is called... suppose
it returns RLM_MODULE_NOTFOUND. The ACTION for notfound inside the files{...}
block is ``1``. We have now reached the end of the authorize{...} GROUP and we
look at the RESULTs we accumulated along the way - there is a noop with
preference level 2, and a notfound with preference level 1, so the
authorize{...} GROUP as a whole returns RLM_MODULE_NOOP, which makes sense
because to say the user was not found at all would be a lie, since preprocess
apparently found him, or else it would have returned RLM_MODULE_NOTFOUND too.

We could use the ``default`` code to simplify the above example a
little.  The following two configurations are identical:

::

  files {
    notfound = 1
    noop     = 2
    ok       = 3
    updated  = 4
    default  = return
  }


When putting the ``default`` first, later definitions over-ride it's
return code:

::

  files {
    default  = return
    notfound = 1
    noop     = 2
    ok       = 3
    updated  = 4
  }

[Take a deep breath - the worst is over]

That RESULT preference/desirability stuff is pretty complex, but my hope is
that it will be complex enough to handle the needs of everyone's real-world
imperfect systems, while staying out of sight most of the time since the
defaults will be right for the most common configurations.

So where does redundant{...} fit in with all that? Well, redundant{...} is
simply a group that changes the default ACTIONs to something like

::

  fail = 1
  everythingelse = return

so that when one module fails, we keep trying until we find one that doesn't
fail, then return whatever it returned. And at the end, if they all failed,
the redundant GROUP as a whole returns RLM_MODULE_FAIL, just as you'd want it
to (I hope).

There are two other kinds of grouping: ``group{...}`` which does not have any
specialized default ACTIONs, and ``append{...}``, which should be used when you
have separate but similarly structured databases that are guaranteed not to
overlap.

That's all that really needs to be said. But now a few random notes:

GROUPs may have RESULT=ACTION
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

It would look like this:

::

  authorize {
    preprocess
    redundant {
      sql1
      sql2
      notfound = return
    }
    files
  }

which would prevent ``files`` from being called if neither of the SQL
instances could find the user.

redundant{...} and append{...} are just shortcuts
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You could write:

::

    group {
      sql1 {
        fail     = 1
        notfound = 2
        noop     = return
        ok       = return
        updated  = return
        reject   = return
        userlock = return
        invalid  = return
        handled  = return
      }
      sql2 {
        fail     = 1
        notfound = 2
        noop     = return
        ok       = return
        updated  = return
        reject   = return
        userlock = return
        invalid  = return
        handled  = return
      }
    }
  instead of
    redundant {
      sql1
      sql2
    }

but the latter is just a whole lot easier to read.

``authenticate{...}`` is not a GROUP
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

even though it contains a list of ``Auth-Type`` GROUPs, because its
semantics are totally different - it uses ``Auth-Type`` to decide which of
its members to call, and their order is irrelevant.

The default rules are context-sensitive
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For ``authorize``, the defaults are
what you saw above - notfound, noop, ok, and updated are considered
success, and anything else has an ACTION of ``return``. For authenticate, the
default is to return on success *or* reject, and only try the second and
following items if the first one fails. You can read all the default ACTIONs
in modcall.c (int defaultactions[][][]), or just trust me. They do the right
thing.

There are some rules that can't be implemented in this language
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

things like ``notfound = 1-reject``, ``noop = 2-ok``, ``ok = 3-ok``, etc. But I don't feel
justified adding that complexity in the first draft.
There are already enough things here that may never see real-world usage.
Like append{...}

-- Pac. 9/18/2000
