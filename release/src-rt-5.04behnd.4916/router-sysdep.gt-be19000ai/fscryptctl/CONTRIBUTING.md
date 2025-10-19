# How to Contribute to fscryptctl

We'd love to accept your patches and contributions to this project. There are
just a few small guidelines you need to follow.

## Contributor License Agreement

Contributions to this project must be accompanied by a Contributor License
Agreement. You (or your employer) retain the copyright to your contribution,
this simply gives us permission to use and redistribute your contributions as
part of the project. Head over to <https://cla.developers.google.com/> to see
your current agreements on file or to sign a new one.

You generally only need to submit a CLA once, so if you've already submitted one
(even if it was for a different project), you probably don't need to do it
again.

## Code reviews

All submissions, including submissions by project members, require review. We
use GitHub pull requests for this purpose. Consult
[GitHub Help](https://help.github.com/articles/about-pull-requests/) for more
information on using pull requests.

## Before you submit a pull request

When making any changes to `fscryptctl`, you will need the following commands:
*   `make format` which formats all of the C code (requires `clang-format`)
*   `make test` which runs the tests for fscryptctl (requires `python` and the
    `pytest` and `keyutils` python packages). Note that to run all of the tests,
    the environment variable `TEST_FILESYSTEM_ROOT` must be set to the
    mountpoint of an ext4 filesystem setup for encryption that the user can
    mount and unmount.
*   `make all` - Runs the above commands and builds `fscryptctl`.

Make sure all these commands are run and the tests pass before submitting a pull
request. All the above dependencies can be installed with:
``` bash
> sudo apt-get install python-pip libkeyutils-dev clang-format
> sudo -H pip install -U pip pytest keyutils
```
