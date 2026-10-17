.. SPDX-License-Identifier: GPL-2.0+:

Documentation Style
===================

Documentation is crucial for the U-Boot project. It has to encompass the needs
of different reader groups from first time users to developers and maintainers.
This requires different types of documentation like tutorials, how-to-guides,
explanatory texts, and reference.

We want to be able to generate documentation in different target formats. We
therefore use `Sphinx <https://www.sphinx-doc.org>`_ for the generation of
documents from reStructured text.

We apply the following rules:

* Documentation files are located in *doc/* or its sub-directories.
* Each documentation file is added to an index page to allow navigation
  to the document.
* For documentation we use reStructured text conforming to the requirements
  of `Sphinx <https://www.sphinx-doc.org>`_.
* For documentation within code we follow the Linux kernel guide
  `Writing kernel-doc comments <https://www.kernel.org/doc/html/latest/doc-guide/kernel-doc.html>`_.
* We try to stick to 80 columns per line in documents.
* For tables we prefer simple tables over grid tables. We avoid list tables
  as they make the reStructured text documents hard to read.
* Before submitting documentation patches we build the HTML documentation and
  fix all warnings. The build process is described in
  :doc:`/build/documentation`.
