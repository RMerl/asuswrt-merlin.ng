# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2023 Tobias Deiminger <tdmg@linutronix.de>

"""Test for unexpected leftovers after make clean"""

import itertools
import os
import pathlib
import shutil
import sys

import pytest

# pylint: disable=redefined-outer-name


@pytest.fixture
def tmp_copy_of_builddir(u_boot_config, tmp_path):
    """For each test, provide a temporary copy of the initial build directory."""
    if os.path.realpath(u_boot_config.source_dir) == os.path.realpath(
        u_boot_config.build_dir
    ):
        pytest.skip("Leftover detection requires out of tree build.")
        return None
    shutil.copytree(
        u_boot_config.build_dir,
        tmp_path,
        symlinks=True,
        dirs_exist_ok=True,
    )
    return tmp_path


@pytest.fixture(scope="module")
def run_make(u_boot_log):
    """Provide function to run and log make without connecting to u-boot console."""
    runner = u_boot_log.get_runner("make", sys.stdout)

    def _run_make(build_dir, target):
        cmd = ["make", f"O={build_dir}", target]
        runner.run(cmd)

    yield _run_make
    runner.close()


@pytest.fixture(scope="module")
def most_generated_files():
    """Path.glob style patterns to describe what should be removed by 'make clean'."""
    return (
        "**/*.c",
        "**/*.dtb",
        "**/*.dtbo",
        "**/*.o",
        "**/*.py",
        "**/*.pyc",
        "**/*.so",
        "**/*.srec",
        "u-boot*",
        "[svt]pl/u-boot*",
    )


@pytest.fixture(scope="module")
def all_generated_files(most_generated_files):
    """Path.glob style patterns to describe what should be removed by 'make mrproper'."""
    return most_generated_files + (".config", "**/*.h")


def find_files(search_dir, include_patterns, exclude_dirs=None):
    """Find files matching include_patterns, unless it's in one of exclude_dirs.

    include_patterns -- Path.glob style pattern relative to search dir
    exclude_dir -- directories to exclude, expected relative to search dir
    """
    matches = []
    exclude_dirs = [] if exclude_dirs is None else exclude_dirs
    for abs_path in itertools.chain.from_iterable(
        pathlib.Path(search_dir).glob(pattern) for pattern in include_patterns
    ):
        if abs_path.is_dir():
            continue
        rel_path = pathlib.Path(os.path.relpath(abs_path, search_dir))
        if not any(
            rel_path.is_relative_to(exclude_dir) for exclude_dir in exclude_dirs
        ):
            matches.append(rel_path)
    return matches


def test_clean(run_make, tmp_copy_of_builddir, most_generated_files):
    """Test if 'make clean' deletes most generated files."""
    run_make(tmp_copy_of_builddir, "clean")
    leftovers = find_files(
        tmp_copy_of_builddir,
        most_generated_files,
        exclude_dirs=["scripts", "test/overlay"],
    )
    assert not leftovers, f"leftovers: {', '.join(map(str, leftovers))}"


def test_mrproper(run_make, tmp_copy_of_builddir, all_generated_files):
    """Test if 'make mrproper' deletes current configuration and all generated files."""
    run_make(tmp_copy_of_builddir, "mrproper")
    leftovers = find_files(
        tmp_copy_of_builddir,
        all_generated_files,
        exclude_dirs=["test/overlay"],
    )
    assert not leftovers, f"leftovers: {', '.join(map(str, leftovers))}"
