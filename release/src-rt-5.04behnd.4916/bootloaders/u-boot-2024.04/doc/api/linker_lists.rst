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

  __u_boot_list_ + 2_ + @_list + _2_ + @_entry

and the C variable name is

::

  _u_boot_list + _2_ + @_list + _2_ + @_entry

This ensures uniqueness for both input section and C variable name.

Note that the names differ only in the characters, "__" for the
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

  __u_boot_list_2_array_1
  __u_boot_list_2_array_2_first
  __u_boot_list_2_array_2_second
  __u_boot_list_2_array_2_third
  __u_boot_list_2_array_3

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

Alignment issues
----------------

The linker script uses alphabetic sorting to group the different linker
lists together. Each group has its own struct and potentially its own
alignment. But when the linker packs the structs together it cannot ensure
that a linker list starts on the expected alignment boundary.

For example, if the first list has a struct size of 8 and we place 3 of
them in the image, that means that the next struct will start at offset
0x18 from the start of the linker_list section. If the next struct has
a size of 16 then it will start at an 8-byte aligned offset, but not a
16-byte aligned offset.

With sandbox on x86_64, a reference to a linker list item using
ll_entry_get() can force alignment of that particular linker_list item,
if it is in the same file as the linker_list item is declared.

Consider this example, where struct driver is 0x80 bytes::

    ll_entry_declare(struct driver, fred, driver)

    ...

    void *p = ll_entry_get(struct driver, fred, driver)

If these two lines of code are in the same file, then the entry is forced
to be aligned at the 'struct driver' alignment, which is 16 bytes. If the
second line of code is in a different file, then no action is taken, since
the compiler cannot update the alignment of the linker_list item.

In the first case, an 8-byte 'fill' region is added::

   __u_boot_list_2_driver_2_testbus_drv
               0x0000000000270018       0x80 test/built-in.o
               0x0000000000270018                _u_boot_list_2_driver_2_testbus_drv
   __u_boot_list_2_driver_2_testfdt1_drv
               0x0000000000270098       0x80 test/built-in.o
               0x0000000000270098                _u_boot_list_2_driver_2_testfdt1_drv
   *fill*         0x0000000000270118        0x8
   __u_boot_list_2_driver_2_testfdt_drv
               0x0000000000270120       0x80 test/built-in.o
               0x0000000000270120                _u_boot_list_2_driver_2_testfdt_drv
   __u_boot_list_2_driver_2_testprobe_drv
               0x00000000002701a0       0x80 test/built-in.o
               0x00000000002701a0                _u_boot_list_2_driver_2_testprobe_drv

With this, the linker_list no-longer works since items after testfdt1_drv
are not at the expected address.

Ideally we would have a way to tell gcc not to align structs in this way.
It is not clear how we could do this, and in any case it would require us
to adjust every struct used by the linker_list feature.

The simplest fix seems to be to force each separate linker_list to start
on the largest possible boundary that can be required by the compiler. This
is the purpose of CONFIG_LINKER_LIST_ALIGN


.. kernel-doc:: include/linker_lists.h
   :internal:
