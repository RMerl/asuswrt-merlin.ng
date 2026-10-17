# SPDX-License-Identifier: GPL-2.0+
# Copyright 2022 Google LLC

"""Common utility functions for FIT tests"""

import os

import u_boot_utils as util

def make_fname(cons, basename):
    """Make a temporary filename

    Args:
        cons (ConsoleBase): u_boot_console to use
        basename (str): Base name of file to create (within temporary directory)
    Return:
        Temporary filename
    """

    return os.path.join(cons.config.build_dir, basename)

def make_its(cons, base_its, params, basename='test.its'):
    """Make a sample .its file with parameters embedded

    Args:
        cons (ConsoleBase): u_boot_console to use
        base_its (str): Template text for the .its file, typically containing
            %() references
        params (dict of str): Parameters to embed in the %() strings
        basename (str): base name to write to (will be placed in the temp dir)
    Returns:
        str: Filename of .its file created
    """
    its = make_fname(cons, basename)
    with open(its, 'w', encoding='utf-8') as outf:
        print(base_its % params, file=outf)
    return its

def make_fit(cons, mkimage, base_its, params, basename='test.fit', base_fdt=None):
    """Make a sample .fit file ready for loading

    This creates a .its script with the selected parameters and uses mkimage to
    turn this into a .fit image.

    Args:
        cons (ConsoleBase): u_boot_console to use
        mkimage (str): Filename of 'mkimage' utility
        base_its (str): Template text for the .its file, typically containing
            %() references
        params (dict of str): Parameters to embed in the %() strings
        basename (str): base name to write to (will be placed in the temp dir)
    Return:
        Filename of .fit file created
    """
    fit = make_fname(cons, basename)
    its = make_its(cons, base_its, params)
    util.run_and_log(cons, [mkimage, '-f', its, fit])
    if base_fdt:
        with open(make_fname(cons, 'u-boot.dts'), 'w') as fd:
            fd.write(base_fdt)
    return fit

def make_kernel(cons, basename, text):
    """Make a sample kernel with test data

    Args:
        cons (ConsoleBase): u_boot_console to use
        basename (str): base name to write to (will be placed in the temp dir)
        text (str): Contents of the kernel file (will be repeated 100 times)
    Returns:
        str: Full path and filename of the kernel it created
    """
    fname = make_fname(cons, basename)
    data = ''
    for i in range(100):
        data += f'this {text} {i} is unlikely to boot\n'
    with open(fname, 'w', encoding='utf-8') as outf:
        print(data, file=outf)
    return fname

def make_dtb(cons, base_fdt, basename):
    """Make a sample .dts file and compile it to a .dtb

    Returns:
        cons (ConsoleBase): u_boot_console to use
        Filename of .dtb file created
    """
    src = make_fname(cons, f'{basename}.dts')
    dtb = make_fname(cons, f'{basename}.dtb')
    with open(src, 'w', encoding='utf-8') as outf:
        outf.write(base_fdt)
    util.run_and_log(cons, ['dtc', src, '-O', 'dtb', '-o', dtb])
    return dtb
