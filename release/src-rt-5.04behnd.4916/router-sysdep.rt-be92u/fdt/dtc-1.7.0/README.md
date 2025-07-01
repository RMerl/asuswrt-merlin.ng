# Device Tree Compiler and libfdt

The source tree contains the Device Tree Compiler (dtc) toolchain for
working with device tree source and binary files and also libfdt, a
utility library for reading and manipulating the binary format.

dtc and libfdt are maintained by:

* [David Gibson `<david@gibson.dropbear.id.au>`](mailto:david@gibson.dropbear.id.au)

## Python library

A Python library wrapping libfdt is also available. To build this you
will need to install `swig` and Python development files. On Debian
distributions:

```
$ sudo apt-get install swig python3-dev
```

The library provides an `Fdt` class which you can use like this:

```
$ PYTHONPATH=../pylibfdt python3
>>> import libfdt
>>> fdt = libfdt.Fdt(open('test_tree1.dtb', mode='rb').read())
>>> node = fdt.path_offset('/subnode@1')
>>> print(node)
124
>>> prop_offset = fdt.first_property_offset(node)
>>> prop = fdt.get_property_by_offset(prop_offset)
>>> print('%s=%s' % (prop.name, prop.as_str()))
compatible=subnode1
>>> node2 = fdt.path_offset('/')
>>> print(fdt.getprop(node2, 'compatible').as_str())
test_tree1
```

You will find tests in `tests/pylibfdt_tests.py` showing how to use each
method. Help is available using the Python help command, e.g.:

```
$ cd pylibfdt
$ python3 -c "import libfdt; help(libfdt)"
```

If you add new features, please check code coverage:

```
$ sudo apt-get install python3-coverage
$ cd tests
# It's just 'coverage' on most other distributions
$ python3-coverage run pylibfdt_tests.py
$ python3-coverage html
# Open 'htmlcov/index.html' in your browser
```

The library can be installed with pip from a local source tree:

```
$ pip install . [--user|--prefix=/path/to/install_dir]
```

Or directly from a remote git repo:

```
$ pip install git+git://git.kernel.org/pub/scm/utils/dtc/dtc.git@main
```

The install depends on libfdt shared library being installed on the
host system first. Generally, using `--user` or `--prefix` is not
necessary and pip will use the default location for the Python
installation which varies if the user is root or not.

You can also install everything via make if you like, but pip is
recommended.

To install both libfdt and pylibfdt you can use:

```
$ make install [PREFIX=/path/to/install_dir]
```

To disable building the python library, even if swig and Python are available,
use:

```
$ make NO_PYTHON=1
```

More work remains to support all of libfdt, including access to numeric
values.

## Mailing lists

* The [devicetree-compiler](mailto:devicetree-compiler@vger.kernel.org)
  list is for discussion about dtc and libfdt implementation.
* Core device tree bindings are discussed on the
  [devicetree-spec](mailto:devicetree-spec@vger.kernel.org) list.

