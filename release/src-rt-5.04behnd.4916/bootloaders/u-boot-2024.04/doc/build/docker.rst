GitLab CI / U-Boot runner container
===================================

In order to have a reproducible and portable build environment for CI we use a container for building in.  This means that developers can also reproduce the CI environment, to a large degree at least, locally.  This file is located in the tools/docker directory.  To build the image yourself

.. code-block:: bash

    sudo docker build -t your-namespace:your-tag .

Or to use an existing container

.. code-block:: bash

    sudo docker pull trini/u-boot-gitlab-ci-runner:bionic-20200807-02Sep2020
