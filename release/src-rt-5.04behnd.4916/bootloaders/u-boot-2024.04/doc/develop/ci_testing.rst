.. SPDX-License-Identifier: GPL-2.0+

Continuous Integration testing
==============================

All changes require passing our continuous integration tests prior to being
merged in to mainline.  To help facilitate merges being accepted quickly,
custodians are encouraged but not required to run a pipeline prior to sending a
pull request.  Individual developers submitting significant or widespread
changes are encouraged to run a pipeline themselves prior to posting.

In order to make this process as easy as possible, the ability to run a CI
pipeline is provided in both Azure and GitLab.  Both of these pipelines perform
their Linux build jobs on the same Docker container image and to cover the same
platforms.  In addition, Azure is also used to confirm that our host tools can
be built with mingw to run on Windows.

Each of the pipelines is written in such as way as to be a "world build" style
test and as such we try and build all possible platforms.  In addition, for all
platforms that support being run in QEMU we run them in QEMU and use our pytest
suite.  See :doc:`py_testing` for more information about those tests.

Azure Pipelines
---------------

This pipeline is defined in the top-level ``.azure-pipelines.yml`` file.
Currently there are two ways to run a Microsoft Azure Pipeline test for U-Boot.

The first way is to create an account with Microsoft at
https://azure.microsoft.com/en-us/services/devops/ and then use the
``.azure-pipelines.yml`` file in the U-Boot repository as the pipeline
description.

The second way is to use GitHub.  This requires a GitHub account
and to fork the repository at https://github.com/u-boot/u-boot and to then
submit a pull request as this will trigger an Azure pipeline run.  Clicking on
your pull request on the list at https://github.com/u-boot/u-boot/pulls and
then the "Checks" tab will show the results.

GitLab CI Pipelines
-------------------

This pipeline is defined in the top-level ``.gitlab-ci.yml`` file.  Currently,
we support running GitLab CI pipelines only for custodians, due to the
resources the project has available.  For Custodians, it is a matter of
enabling the pipeline feature in your project repository following the standard
GitLab documentation.  For non-custodians, the pipeline itself is part of the
tree and should be able to be used on any GitLab instance, with whatever
runners you are able to provide.  While it is intended to be able to run this
pipeline on the free public instances provided at https://gitlab.com/ a problem
with our squashfs tests currently prevents this.

To push to Gitlab without triggering a pipeline use:

.. code-block:: bash

    git push -o ci.skip

Docker container
----------------

As previously stated, both of the above pipelines build using the same Docker
container image.  This is maintained in the U-Boot source tree at
``tools/docker/Dockerfile`` and new images are made as needed to support new
tests or features.  This file needs to be updated whenever adding new external
tool requirements to tests.

Customizing CI
--------------

As noted above, the CI pipelines perform a world build.  While this is good for
overall project testing, it can be less useful for testing specific cases or
developing features.  In that case, it can be useful as part of your own
testing cycle to edit these pipelines in separate local commits to pair them
down to just the jobs you're interested in.  These changes must be removed
prior to submission.
