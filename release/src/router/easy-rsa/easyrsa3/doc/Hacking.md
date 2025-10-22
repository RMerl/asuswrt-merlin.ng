Easy-RSA 3 Hacking Guide
===

This document is aimed at programmers looking to improve on the existing
codebase.

Compatibility
---

The `easyrsa` code is written in POSIX shell (and any cases where it is not is
considered a bug to be fixed.) The only exceptions are the `local` keyword and
the construct `export FOO=baz`, both well-supported.

As such, modifications to the code should also be POSIX; platform-specific code
should be placed under the `distro/` dir and listed by target platform.

Coding conventions
---

While there aren't strict syntax standards associated with the project, please
follow the existing format and flow when possible; however, specific exceptions
can be made if there is a significant reason or benefit.

Do try to:

  * Keep variables locally-scoped when possible
  * Comment sections of code for readability
  * Use the conventions for prefixes on global variables
  * Set editors for tab stops of 8 spaces
  * Use tabs for code indents; use aligned spaces for console text

Keeping code, docs, and examples in sync
---

Changes that adjust, add, or remove features should have relevant docs, help
output, and examples updated at the same time.

Release versioning
---

A point-release bump (eg: 3.0 to 3.1) is required when the frontend interface
changes in a non-backwards compatible way. Always assume someone has an
automated process that relies on the current functionality for official
(non-beta, non-rc) releases. A possible exception exists for bugfixes that do
break backwards-compatibility; caution is to be used in such cases.

The addition of a new command may or may not require a point-release depending
on the significance of the feature; the same holds true for additional optional
arguments to commands.

Project layout
---

The project's files are structured as follows:

  * `easyrsa3/` is the primary project code. On Linux/Unix-alikes, all the core
    code and supporting files are stored here.
  * `Licensing/` is for license docs.
  * `build/` is for build information and scripts.
  * `contrib/` is for externally-contributed files, such as useful external
    scripts or interfaces for other systems/languages.
  * `distro/` is for distro-specific supporting files, such as the Windows
    frontend wrappers. Code components that are not platform-neutral should go
    here.
  * `doc/` is for documentation. Much of this is in Markdown format which can be
    easily converted to HTML for easy viewing under Windows.
  * `release-keys/` list current and former KeyIDs used to sign release packages
    (not necessarily git tags) available for download.
  * The top-level dir includes files for basic project info and reference
    appropriate locations for more detail.

As a brief note, it is actually possible to take just the easyrsa3/ dir and end
up with a functional project; the remaining structure includes docs, build prep,
distro-specific wrappers, and contributed files.

Git conventions
---

As of Easy-RSA 3, the following git conventions should be used. These are mostly
useful for people with repo access in order to keep a standard meaning to commit
messages and merge actions.

### Signed-off-by: and related commit message lines

  Committers with push access should ensure a `Signed-off-by:` line exists at
  the end of the commit message with their name on it. This indicates that the
  committer has reviewed the changes to the commit in question and approve of
  the feature and code in question. It also helps verify the code came from an
  acceptable source that won't cause issues with the license.

  This can be automatically added by git using `git commit -s`.

  Additional references can be included as well. If multiple people reviewed the
  change, the committer may add their names in additional `Signed-off-by:`
  lines; do get permission from that person before using their name, however ;)

  The following references may be useful as well:

  * `Signed-off-by:` -- discussed above, indicates review of the commit
  * `Author:` -- references an author of a particular feature, in full or
    significant part
  * `Changes-by:` -- indicates the listed party contributed changes or
    modifications to a feature
  * `Acked-by:` -- indicates review of the feature, code, and/or functional
    correctness

### Merging from external sources (forks, patches, etc)

  Contributions can come in many forms: GitHub "pull requests" from cloned
  repos, references to external repos, patches to the ML, or others. Those won't
  necessarily have `Signed-off-by:` lines or may contain less info in the commit
  message than is desirable to explain the changes.

  The committing author to this project should make a merge-commit in this case
  with the appropriate details provided there. If additional code changes are
  necessary, this can be done on a local branch prior to merging back into the
  mainline branch.

  This merge-commit should list involved contributors with `Author:` or similar
  lines as required. The individual commits involved in a merge also retain the
  original committer; regardless, the merge-commit message should give a clear
  indication of what the entire set of commits does as a whole.

### Tagging

  Tags should follow the convention:

    vM.m.p

  where `M` is the major version, `m` is the minor "point-release" version, and
  `p` is the patch-level. Suffixes of `-rc#`, `-beta#`, etc can be added for
  pre-release versions as required.

  Currently tags are taken from the mainline development branch in question. The
  ChangeLog should thus be updated prior to tagging. Tags should also be
  annotated with an appropriate commit message and signed-off. This can be done
  as shown below (don't use `-s` unless you intend to use GPG with git.)

    git tag -a v1.2.3

  Corresponding release downloads can be uploaded to release distribution points
  as required.
