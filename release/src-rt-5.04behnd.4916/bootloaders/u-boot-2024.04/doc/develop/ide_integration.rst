Integration with IDEs
=====================

IDEs and text editors (e.g., VSCode, Emacs, Vim, Neovim) typically offer
plugins to enhance the development experience, such as Clangd LSP. These
plugins provide features like code navigation (i.e., jumping to definitions
and declarations), code completion, and code formatting.

U-Boot provides a script (i.e., scripts/gen_compile_commands.py) that
generates a compilation database to be utilized by Clangd LSP for code
navigation. For detailed usage instructions, please refer to the script's
documentation: :doc:`../build/gen_compile_commands`.
