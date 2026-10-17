.. SPDX-License-Identifier: GPL-2.0-only

Create build database for IDEs
==============================

gen_compile_commands (scripts/gen_compile_commands.py) is a script used to
generate a compilation database (compile_commands.json). This database consists
of an array of "command objects" describing how each translation unit was
compiled.

Example::

  {
  "command": "gcc -Wp,-MD,arch/x86/cpu/.lapic.o.d -nostdinc -isystem (...)"
  "directory": "/home/jmcosta/u-boot",
  "file": "/home/jmcosta/u-boot/arch/x86/cpu/lapic.c"
  }

Such information comes from parsing the respective .cmd file of each translation
unit. In the previous example, that would be `arch/x86/cpu/.lapic.o.cmd`.

For more details on the database format, please refer to the official
documentation at https://clang.llvm.org/docs/JSONCompilationDatabase.html.

The compilation database is quite useful for text editors (and IDEs) that use
Clangd LSP. It allows jumping to definitions and declarations. Since it relies
on parsing .cmd files, one needs to have a target (e.g. configs/xxx_defconfig)
built before running the script.

Example::

  make sandbox_defconfig
  make
  ./scripts/gen_compile_commands.py

Beware that depending on the changes you made to the project's source code, you
may need to run the script again (presuming you recompiled your target, of
course) to have an up-to-date database.

The database will be in the root of the repository. No further modifications are
needed for it to be usable by the LSP, unless you set a name for the database
other than it's default one (compile_commands.json).

Compatible IDEs
===============

Several popular integrated development environments (IDEs) support the use
of JSON compilation databases for C/C++ development, making it easier to
manage build configurations and code analysis. Some of these IDEs include:

1. **Visual Studio Code (VS Code)**: IntelliSense in VS Code can be set up to
   use compile_commands.json by following the instructions in
   https://code.visualstudio.com/docs/cpp/faq-cpp#_how-do-i-get-intellisense-to-work-correctly.

2. **CLion**: JetBrains' CLion IDE supports JSON compilation databases out
   of the box. You can configure your project to use a compile_commands.json
   file via the project settings. Details on setting up CLion with JSON
   compilation databases can be found at
   https://www.jetbrains.com/help/clion/compilation-database.html.

3. **Qt Creator**: Qt Creator, a popular IDE for Qt development, also
   supports compile_commands.json for C/C++ projects. Instructions on how to
   use this feature can be found at
   https://doc.qt.io/qtcreator/creator-clang-codemodel.html#using-compilation-databases.

4. **Eclipse CDT**: Eclipse's C/C++ Development Tools (CDT) can be
   configured to use JSON compilation databases for better project management.
   You can find guidance on setting up JSON compilation database support at the
   wiki: https://wiki.eclipse.org/CDT/User/NewIn910#Build.

For Vim, Neovim, and Emacs, if you are using Clangd as your LSP, placing the
compile_commands.json in the root of the repository should suffice to enable
code navigation.

Usage
=====

For further details on the script's options, please refer to its help message,
as in the example below.

Help::

  ./scripts/gen_compile_commands.py --help
