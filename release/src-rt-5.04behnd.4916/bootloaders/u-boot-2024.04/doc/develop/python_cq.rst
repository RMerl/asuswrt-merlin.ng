.. SPDX-License-Identifier: GPL-2.0+

Python code quality
===================

U-Boot has about 60k lines of Python code, mainly in the following areas:

- tests
- pytest hooks
- patman patch submission tool
- buildman build / analysis tool
- dtoc devicetree-to-C tool
- binman firmware packaging tool

`PEP 8`_ is used for the code style, with the single quote (') used by default for
strings and double quote for doc strings. All non-trivial functions should be
commented.

Pylint is used to help check this code and keep a consistent code style. The
build system tracks the current 'score' of the source code and detects
regressions in any module.

To run this locally you should use this version of pylint::

    # pylint --version
    pylint 2.11.1
    astroid 2.8.6
    Python 3.8.10 (default, Sep 28 2021, 16:10:42)
    [GCC 9.3.0]


You should be able to select and this install other required tools with::

    pip install pylint==2.11.1
    pip install -r test/py/requirements.txt
    pip install asteval pyopenssl

Note that if your distribution is a year or two old, you make need to use `pip3`
instead.

To configure pylint, make sure it has docparams enabled, e.g. with::

    echo "[MASTER]" >> .pylintrc
    echo "load-plugins=pylint.extensions.docparams" >> .pylintrc

Once everything is ready, use this to check the code::

    make pylint

This creates a directory called `pylint.out` which contains the pylint output
for each Python file in U-Boot. It also creates a summary file called
`pylint.cur` which shows the pylint score for each module::

    _testing 0.83
    atf_bl31 -6.00
    atf_fip 0.49
    binman.cbfs_util 7.70
    binman.cbfs_util_test 9.19
    binman.cmdline 7.73
    binman.control 4.39
    binman.elf 6.42
    binman.elf_test 5.41
    ...

This file is in alphabetical order. The build system compares the score of each
module to `scripts/pylint.base` (which must also be sorted and have exactly the
same modules in it) and reports any files where the score has dropped. Use
pylint to check what is wrong and fix up the code before you send out your
patches.

New or removed files results in an error which can be resolved by updating the
`scripts/pylint.base` file to add/remove lines for those files, e.g.::

    meld pylint.cur scripts/pylint.base

If the pylint version is updated in CI, this may result in needing to regenerate
`scripts/pylint.base`.


Checking for errors
-------------------

If you only want to check for pylint errors, use::

   PYTHONPATH=/path/to/scripts/dtc/pylibfdt/ make pylint_err

This will show only pylint errors. Note that you must set PYTHONPATH to point
to the pylibfdt directory build by U-Boot (typically the sandbox_spl board). If
you have used `make qcheck` then it sill be in `board-sandbox_spl`.

.. _`PEP 8`: https://www.python.org/dev/peps/pep-0008/
