# Modules in Tor #

This document describes the build system and coding standards when writing a
module in Tor.

## What is a module? ##

In the context of the tor code base, a module is a subsystem that we can
selectively enable or disable, at `configure` time.

Currently, there is only one module:

  - Directory Authority subsystem (dirauth)

It is located in its own directory in `src/feature/dirauth/`. To disable it,
one need to pass `--disable-module-dirauth` at configure time. All modules
are currently enabled by default.

## Build System ##

The changes to the build system are pretty straightforward.

1. Locate in the `configure.ac` file this define: `m4_define(MODULES`. It
   contains a list (white-space separated) of the module in tor. Add yours to
   the list.

2. Use the `AC_ARG_ENABLE([module-dirauth]` template for your new module. We
   use the "disable module" approach instead of enabling them one by one. So,
   by default, tor will build all the modules.

   This will define the `HAVE_MODULE_<name>` statement which can be used in
   the C code to conditionally compile things for your module. And the
   `BUILD_MODULE_<name>` is also defined for automake files (e.g: include.am).

3. In the `src/core/include.am` file, locate the `MODULE_DIRAUTH_SOURCES`
   value.  You need to create your own `_SOURCES` variable for your module
   and then conditionally add the it to `LIBTOR_A_SOURCES` if you should
   build the module.

   It is then **very** important to add your SOURCES variable to
   `src_or_libtor_testing_a_SOURCES` so the tests can build it.

4. Do the same for header files, locate `ORHEADERS +=` which always add all
   headers of all modules so the symbol can be found for the module entry
   points.

Finally, your module will automatically be included in the
`TOR_MODULES_ALL_ENABLED` variable which is used to build the unit tests. They
always build everything in order to tests everything.

## Coding ##

As mentioned above, a module must be isolated in its own directory (name of
the module) in `src/feature/`.

There are couples of "rules" you want to follow:

* Minimize as much as you can the number of entry points into your module.
  Less is always better but of course that doesn't work out for every use
  case. However, it is a good thing to always keep that in mind.

* Do **not** use the `HAVE_MODULE_<name>` define outside of the module code
  base. Every entry point should have a second definition if the module is
  disabled. For instance:

  ```
  #ifdef HAVE_MODULE_DIRAUTH

  int sr_init(int save_to_disk);

  #else /* HAVE_MODULE_DIRAUTH */

  static inline int
  sr_init(int save_to_disk)
  {
    (void) save_to_disk;
    return 0;
  }

  #endif /* HAVE_MODULE_DIRAUTH */

  ```

  The main reason for this approach is to avoid having conditional code
  everywhere in the code base. It should be centralized as much as possible
  which helps maintainability but also avoids conditional spaghetti code
  making the code much more difficult to follow/understand.

* It is possible that you end up with code that needs to be used by the rest
  of the code base but is still part of your module. As a good example, if
  you look at `src/feature/shared_random_client.c`: it contains code needed
  by the hidden service subsystem but mainly related to the shared random
  subsystem very specific to the dirauth module.

  This is fine but try to keep it as lean as possible and never use the same
  filename as the one in the module. For example, this is a bad idea and
  should never be done:

    - `src/feature/dirclient/shared_random.c`
    - `src/feature/dirauth/shared_random.c`

* When you include headers from the module, **always** use the full module
  path in your statement. Example:

  `#include "feature/dirauth/dirvote.h"`

  The main reason is that we do **not** add the module include path by default
  so it needs to be specified. But also, it helps our human brain understand
  which part comes from a module or not.

  Even **in** the module itself, use the full include path like above.
