.. SPDX-License-Identifier: GPL-2.0+

Linker-Generated Arrays
=======================

A linker list is constructed by grouping together linker input
sections, each containing one entry of the list. Each input section
contains a constant initialized variable which holds the entry's
content. Linker list input sections are constructed from the list
and entry names, plus a prefix which allows grouping all lists
together. Assuming _list and _entry are the list and entry names,
then the corresponding input section name is

::

  .u_boot_list_ + 2_ + @_list + _2_ + @_entry

and the C variable name is

::

  _u_boot_list + _2_ + @_list + _2_ + @_entry

This ensures uniqueness for both input section and C variable name.

Note that the names differ only in the first character, "." for the
section and "_" for the variable, so that the linker cannot confuse
section and symbol names. From now on, both names will be referred
to as

::

  %u_boot_list_ + 2_ + @_list + _2_ + @_entry

Entry variables need never be referred to directly.

The naming scheme for input sections allows grouping all linker lists
into a single linker output section and grouping all entries for a
single list.

Note the two '_2_' constant components in the names: their presence
allows putting a start and end symbols around a list, by mapping
these symbols to sections names with components "1" (before) and
"3" (after) instead of "2" (within).
Start and end symbols for a list can generally be defined as

::

  %u_boot_list_2_ + @_list + _1_...
  %u_boot_list_2_ + @_list + _3_...

Start and end symbols for the whole of the linker lists area can be
defined as

::

  %u_boot_list_1_...
  %u_boot_list_3_...

Here is an example of the sorted sections which result from a list
"array" made up of three entries : "first", "second" and "third",
iterated at least once.

::

  .u_boot_list_2_array_1
  .u_boot_list_2_array_2_first
  .u_boot_list_2_array_2_second
  .u_boot_list_2_array_2_third
  .u_boot_list_2_array_3

If lists must be divided into sublists (e.g. for iterating only on
part of a list), one can simply give the list a name of the form
'outer_2_inner', where 'outer' is the global list name and 'inner'
is the sub-list name. Iterators for the whole list should use the
global list name ("outer"); iterators for only a sub-list should use
the full sub-list name ("outer_2_inner").

Here is an example of the sections generated from a global list
named "drivers", two sub-lists named "i2c" and "pci", and iterators
defined for the whole list and each sub-list:

::

  %u_boot_list_2_drivers_1
  %u_boot_list_2_drivers_2_i2c_1
  %u_boot_list_2_drivers_2_i2c_2_first
  %u_boot_list_2_drivers_2_i2c_2_first
  %u_boot_list_2_drivers_2_i2c_2_second
  %u_boot_list_2_drivers_2_i2c_2_third
  %u_boot_list_2_drivers_2_i2c_3
  %u_boot_list_2_drivers_2_pci_1
  %u_boot_list_2_drivers_2_pci_2_first
  %u_boot_list_2_drivers_2_pci_2_second
  %u_boot_list_2_drivers_2_pci_2_third
  %u_boot_list_2_drivers_2_pci_3
  %u_boot_list_2_drivers_3

.. kernel-doc:: include/linker_lists.h
   :internal:
