.. SPDX-License-Identifier: GPL-2.0+:

Building documentation
======================

The U-Boot documentation is based on the Sphinx documentation generator.

In addition to the Python packages listed in ``doc/sphinx/requirements.txt``,
the following dependencies are needed to build the documentation:

* fontconfig

* graphviz

* imagemagick

* texinfo (if building the `Infodoc documentation`_)

HTML documentation
------------------

The *htmldocs* target is used to build the HTML documentation. It uses the
`Read the Docs Sphinx theme <https://sphinx-rtd-theme.readthedocs.io/en/stable/>`_.

.. code-block:: bash

    # Create Python environment 'myenv'
    python3 -m venv myenv
    # Activate the Python environment
    . myenv/bin/activate
    # Install build requirements
    python3 -m pip install -r doc/sphinx/requirements.txt
    # Build the documentation
    make htmldocs
    # Deactivate the Python environment
    deactivate
    # Display the documentation in a graphical web browser
    x-www-browser doc/output/index.html

The HTML documentation is published at https://docs.u-boot.org. The build
process for that site is controlled by the file *.readthedocs.yml*.

Infodoc documentation
---------------------

The *infodocs* target builds both a texinfo and an info file:

.. code-block:: bash

    # Create Python environment 'myenv'
    python3 -m venv myenv
    # Activate the Python environment
    . myenv/bin/activate
    # Install build requirements
    python3 -m pip install -r doc/sphinx/requirements.txt
    # Build the documentation
    make infodocs
    # Deactivate the Python environment
    deactivate
    # Display the documentation
    info doc/output/texinfo/u-boot.info

PDF documentation
-----------------

The *pdfdocs* target is meant to be used to build PDF documenation.
As v2023.01 it fails with 'LaTeX Error: Too deeply nested'.

We can use texi2pdf instead:

.. code-block:: bash

    # Create Python environment 'myenv'
    python3 -m venv myenv
    # Activate the Python environment
    . myenv/bin/activate
    # Install build requirements
    python3 -m pip install -r doc/sphinx/requirements.txt
    # Build the documentation
    make texinfodocs
    # Deactivate the Python environment
    deactivate
    # Convert to PDF
    texi2pdf doc/output/texinfo/u-boot.texi

Texinfo documentation
---------------------

To build only the texinfo documentation the *texinfodocs* target is used:

.. code-block:: bash

    # Create Python environment 'myenv'
    python3 -m venv myenv
    # Activate the Python environment
    . myenv/bin/activate
    # Install build requirements
    python3 -m pip install -r doc/sphinx/requirements.txt
    # Build the documentation
    make texinfodocs
    # Deactivate the Python environment
    deactivate

The output is in file *doc/output/texinfo/u-boot.texi*.
