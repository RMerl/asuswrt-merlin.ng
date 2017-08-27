The mods-config Directory
=========================

This directory contains module-specific configuration files.  These
files are in a format different from the one used by the main
`radiusd.conf` files.  Earlier versions of the server had many
module-specific files in the main `raddb` directory.  The directory
contained many files, and it was not clear which files did what.

For Version 3 of FreeRADIUS, we have moved to a consistent naming
scheme.  Each module-specific configuration file is placed in this
directory, in a subdirectory named for the module.  Where necessary,
files in the subdirectory have been named for the processing section
where they are used.

For example, the `users` file is now located in
`mods-config/files/authorize`.  That filename tells us three things:

1. The file is used in the `authorize` section.
2. The file is used by the `files` module.
3. It is a "module configuration" file, which is a specific format.

