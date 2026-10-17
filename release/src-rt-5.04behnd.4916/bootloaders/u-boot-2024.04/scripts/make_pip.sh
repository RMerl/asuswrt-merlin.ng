#!/bin/bash
# SPDX-License-Identifier: GPL-2.0+

# Packages a U-Boot tool
#
# Usage: make_pip.sh <tool_name> [--real]
#
# Where tool_name is one of patman, buildman, dtoc, binman, u_boot_pylib
#
# and --real means to upload to the real server (otherwise the test one is used)
#
# The username for upload is always __token__ so set TWINE_PASSWORD to your
# password before running this script:
#
# export TWINE_PASSWORD=pypi-xxx
#
# To test your new packages:
#
# pip install -i https://test.pypi.org/simple/ <tool_name>
#

# DO NOT use patman or binman

set -xe

# Repo to upload to
repo="--repository testpypi"

# Non-empty to do the actual upload
upload=1

# Non-empty to delete files used for testing
delete_testfiles=1

tool="$1"
shift
flags="$*"

if [[ "${tool}" =~ ^(patman|buildman|dtoc|binman|u_boot_pylib)$ ]]; then
	echo "Building dist package for tool ${tool}"
else
	echo "Unknown tool ${tool}: use u_boot_pylib, patman, buildman, dtoc or binman"
	exit 1
fi

for flag in "${flags}"; do
	if [ "${flag}" == "--real" ]; then
		echo "Using real server"
		repo=
	fi
	if [ "${flag}" == "-n" ]; then
		echo "Doing dry run"
		upload=
	fi
done

if [ -n "${upload}" ]; then
	if [ -z "${TWINE_PASSWORD}" ]; then
		echo "Please set TWINE_PASSWORD to your password and retry"
		exit 1
	fi
fi

if [[ "${tool}" =~ ^(patman|u_boot_pylib)$ ]]; then
	# Leave test_util.py and patman test files alone
	delete_testfiles=
fi

# Create a temp dir to work in
dir=$(mktemp -d)

# Copy in some basic files
cp -v tools/${tool}/pyproject.toml ${dir}
cp -v Licenses/gpl-2.0.txt ${dir}/LICENSE
readme="tools/${tool}/README.*"

# Copy in the README, dropping some Sphinx constructs that PyPi doesn't like
cat ${readme} | sed -E 's/:(doc|ref):`.*`//; /sectionauthor/d; /toctree::/d' \
	> ${dir}/$(basename ${readme})

# Copy the top-level Python and doc files
dest=${dir}/src/${tool}
mkdir -p ${dest}
cp -v tools/$tool/{*.py,*.rst} ${dest}

# Copy over the subdirectories, including any sub files. Drop any cache files
# and other such things
pushd tools/${tool}
for subdir in $(find . -maxdepth 1 -type d | \
		grep -vE "(__pycache__|home|usr|scratch|\.$|pyproject)"); do
	pathname="${dest}/${subdir}"
	echo "Copy ${pathname}"
	cp -a ${subdir} ${pathname}
done
popd

# Remove cache files that accidentally made it through
find ${dest} -name __pycache__ -type f -exec rm {} \;
find ${dest} -depth -name __pycache__ -exec rmdir 112 \;

# Remove test files
if [ -n "${delete_testfiles}" ]; then
	rm -rfv ${dest}/*test*
fi

mkdir ${dir}/tests
cd ${dir}

# Make sure the tools are up to date
python3 -m pip install --upgrade build
python3 -m pip install --upgrade twine

# Build the PyPi package
python3 -m build

echo "Completed build of ${tool}"

# Use --skip-existing to work even if the version is already present
if [ -n "${upload}" ]; then
	echo "Uploading from ${dir}"
	python3 -m twine upload ${repo} -u __token__ dist/*
	echo "Completed upload of ${tool}"
fi

rm -rf "${dir}"

echo -e "done\n\n"
