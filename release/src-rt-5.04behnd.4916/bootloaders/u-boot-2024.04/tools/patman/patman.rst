.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (c) 2011 The Chromium OS Authors
.. Simon Glass <sjg@chromium.org>
.. Maxim Cournoyer <maxim.cournoyer@savoirfairelinux.com>
.. v1, v2, 19-Oct-11
.. revised v3 24-Nov-11
.. revised v4 Independence Day 2020, with Patchwork integration

Patman patch manager
====================

This tool is a Python script which:

- Creates patch directly from your branch
- Cleans them up by removing unwanted tags
- Inserts a cover letter with change lists
- Runs the patches through checkpatch.pl and its own checks
- Optionally emails them out to selected people

It also has some Patchwork features:

- shows review tags from Patchwork so you can update your local patches
- pulls these down into a new branch on request
- lists comments received on a series

It is intended to automate patch creation and make it a less
error-prone process. It is useful for U-Boot and Linux work so far,
since they use the checkpatch.pl script.

It is configured almost entirely by tags it finds in your commits.
This means that you can work on a number of different branches at
once, and keep the settings with each branch rather than having to
git format-patch, git send-email, etc. with the correct parameters
each time. So for example if you put::

    Series-to: fred.blogs@napier.co.nz

in one of your commits, the series will be sent there.

In Linux and U-Boot this will also call get_maintainer.pl on each of your
patches automatically (unless you use -m to disable this).


Installation
------------

You can install patman using::

   pip install patch-manager

The name is chosen since patman conflicts with an existing package.

If you are using patman within the U-Boot tree, it may be easiest to add a
symlink from your local `~/.bin` directory to `/path/to/tools/patman/patman`.

How to use this tool
--------------------

This tool requires a certain way of working:

- Maintain a number of branches, one for each patch series you are
  working on
- Add tags into the commits within each branch to indicate where the
  series should be sent, cover letter, version, etc. Most of these are
  normally in the top commit so it is easy to change them with 'git
  commit --amend'
- Each branch tracks the upstream branch, so that this script can
  automatically determine the number of commits in it (optional)
- Check out a branch, and run this script to create and send out your
  patches. Weeks later, change the patches and repeat, knowing that you
  will get a consistent result each time.


How to configure it
-------------------

For most cases of using patman for U-Boot development, patman can use the
file 'doc/git-mailrc' in your U-Boot directory to supply the email aliases
you need. To make this work, tell git where to find the file by typing
this once::

    git config sendemail.aliasesfile doc/git-mailrc

For both Linux and U-Boot the 'scripts/get_maintainer.pl' handles
figuring out where to send patches pretty well. For other projects,
you may want to specify a different script to be run, for example via
a project-specific `.patman` file::

    # .patman configuration file at the root of some project

    [settings]
    get_maintainer_script: etc/teams.scm get-maintainer

The `get_maintainer_script` option corresponds to the
`--get-maintainer-script` argument of the `send` command.  It is
looked relatively to the root of the current git repository, as well
as on PATH.  It can also be provided arguments, as shown above.  The
contract is that the script should accept a patch file name and return
a list of email addresses, one per line, like `get_maintainer.pl`
does.

During the first run patman creates a config file for you by taking the default
user name and email address from the global .gitconfig file.

To add your own, create a file `~/.patman` like this::

    # patman alias file

    [alias]
    me: Simon Glass <sjg@chromium.org>

    u-boot: U-Boot Mailing List <u-boot@lists.denx.de>
    wolfgang: Wolfgang Denk <wd@denx.de>
    others: Mike Frysinger <vapier@gentoo.org>, Fred Bloggs <f.bloggs@napier.net>

As hinted above, Patman will also look for a `.patman` configuration
file at the root of the current project git repository, which makes it
possible to override the `project` settings variable or anything else
in a project-specific way. The values of this "local" configuration
file take precedence over those of the "global" one.

Aliases are recursive.

The checkpatch.pl in the U-Boot tools/ subdirectory will be located and
used. Failing that you can put it into your path or ~/bin/checkpatch.pl

If you want to avoid sending patches to email addresses that are picked up
by patman but are known to bounce you can add a [bounces] section to your
.patman file. Unlike the [alias] section these are simple key: value pairs
that are not recursive::

    [bounces]
    gonefishing: Fred Bloggs <f.bloggs@napier.net>


If you want to change the defaults for patman's command-line arguments,
you can add a [settings] section to your .patman file.  This can be used
for any command line option by referring to the "dest" for the option in
patman.py.  For reference, the useful ones (at the moment) shown below
(all with the non-default setting)::

    [settings]
    ignore_errors: True
    process_tags: False
    verbose: True
    smtp_server: /path/to/sendmail
    patchwork_url: https://patchwork.ozlabs.org

If you want to adjust settings (or aliases) that affect just a single
project you can add a section that looks like [project_settings] or
[project_alias].  If you want to use tags for your linux work, you could do::

    [linux_settings]
    process_tags: True


How to run it
-------------

First do a dry run:

.. code-block:: bash

    ./tools/patman/patman send -n

If it can't detect the upstream branch, try telling it how many patches
there are in your series

.. code-block:: bash

    ./tools/patman/patman -c5 send -n

This will create patch files in your current directory and tell you who
it is thinking of sending them to. Take a look at the patch files:

.. code-block:: bash

    ./tools/patman/patman -c5 -s1 send -n

Similar to the above, but skip the first commit and take the next 5. This
is useful if your top commit is for setting up testing.


How to install it
-----------------

The most up to date version of patman can be found in the U-Boot sources.
However to use it on other projects it may be more convenient to install it as
a standalone application. A distutils installer is included, this can be used
to install patman:

.. code-block:: bash

    cd tools/patman && python setup.py install


How to add tags
---------------

To make this script useful you must add tags like the following into any
commit. Most can only appear once in the whole series.

Series-to: email / alias
    Email address / alias to send patch series to (you can add this
    multiple times)

Series-cc: email / alias, ...
    Email address / alias to Cc patch series to (you can add this
    multiple times)

Series-version: n
    Sets the version number of this patch series

Series-prefix: prefix
    Sets the subject prefix. Normally empty but it can be RFC for
    RFC patches, or RESEND if you are being ignored. The patch subject
    is like [RFC PATCH] or [RESEND PATCH].
    In the meantime, git format.subjectprefix option will be added as
    well. If your format.subjectprefix is set to InternalProject, then
    the patch shows like: [InternalProject][RFC/RESEND PATCH]

Series-postfix: postfix
    Sets the subject "postfix". Normally empty, but can be the name of a
    tree such as net or net-next if that needs to be specified. The patch
    subject is like [PATCH net] or [PATCH net-next].

Series-name: name
    Sets the name of the series. You don't need to have a name, and
    patman does not yet use it, but it is convenient to put the branch
    name here to help you keep track of multiple upstreaming efforts.

Series-links: [id | version:id]...
    Set the ID of the series in patchwork. You can set this after you send
    out the series and look in patchwork for the resulting series. The
    URL you want is the one for the series itself, not any particular patch.
    E.g. for http://patchwork.ozlabs.org/project/uboot/list/?series=187331
    the series ID is 187331. This property can have a list of series IDs,
    one for each version of the series, e.g.

    ::

       Series-links: 1:187331 2:188434 189372

    Patman always uses the one without a version, since it assumes this is
    the latest one. When this tag is provided, patman can compare your local
    branch against patchwork to see what new reviews your series has
    collected ('patman status').

Series-patchwork-url: url
    This allows specifying the Patchwork URL for a branch. This overrides
    both the setting files ("patchwork_url") and the command-line argument.
    The URL should include the protocol and web site, with no trailing slash,
    for example 'https://patchwork.ozlabs.org/project'

Cover-letter:
    Sets the cover letter contents for the series. The first line
    will become the subject of the cover letter::

        Cover-letter:
        This is the patch set title
        blah blah
        more blah blah
        END

Cover-letter-cc: email / alias
    Additional email addresses / aliases to send cover letter to (you
    can add this multiple times)

Series-notes:
    Sets some notes for the patch series, which you don't want in
    the commit messages, but do want to send, The notes are joined
    together and put after the cover letter. Can appear multiple
    times::

        Series-notes:
        blah blah
        blah blah
        more blah blah
        END

Commit-notes:
    Similar, but for a single commit (patch). These notes will appear
    immediately below the ``---`` cut in the patch file::

        Commit-notes:
        blah blah
        blah blah
        more blah blah

Signed-off-by: Their Name <email>
    A sign-off is added automatically to your patches (this is
    probably a bug). If you put this tag in your patches, it will
    override the default signoff that patman automatically adds.
    Multiple duplicate signoffs will be removed.

Tested-by / Reviewed-by / Acked-by
    These indicate that someone has tested/reviewed/acked your patch.
    When you get this reply on the mailing list, you can add this
    tag to the relevant commit and the script will include it when
    you send out the next version. If 'Tested-by:' is set to
    yourself, it will be removed. No one will believe you.

    Example::

        Tested-by: Their Name <fred@bloggs.com>
        Reviewed-by: Their Name <email>
        Acked-by: Their Name <email>

Series-changes: n
    This can appear in any commit. It lists the changes for a
    particular version n of that commit. The change list is
    created based on this information. Each commit gets its own
    change list and also the whole thing is repeated in the cover
    letter (where duplicate change lines are merged).

    By adding your change lists into your commits it is easier to
    keep track of what happened. When you amend a commit, remember
    to update the log there and then, knowing that the script will
    do the rest.

    Example::

        Series-changes: n
        - Guinea pig moved into its cage
        - Other changes ending with a blank line
        <blank line>

Commit-changes: n
    This tag is like Series-changes, except changes in this changelog will
    only appear in the changelog of the commit this tag is in. This is
    useful when you want to add notes which may not make sense in the cover
    letter. For example, you can have short changes such as "New" or
    "Lint".

    Example::

        Commit-changes: n
        - This line will not appear in the cover-letter changelog
        <blank line>

Cover-changes: n
    This tag is like Series-changes, except changes in this changelog will
    only appear in the cover-letter changelog. This is useful to summarize
    changes made with Commit-changes, or to add additional context to
    changes.

    Example::

        Cover-changes: n
        - This line will only appear in the cover letter
        <blank line>

Patch-cc: Their Name <email>
    This copies a single patch to another email address. Note that the
    Cc: used by git send-email is ignored by patman, but will be
    interpreted by git send-email if you use it.

Series-process-log: sort, uniq
    This tells patman to sort and/or uniq the change logs. Changes may be
    multiple lines long, as long as each subsequent line of a change begins
    with a whitespace character. For example,

    Example::

        - This change
          continues onto the next line
        - But this change is separate

    Use 'sort' to sort the entries, and 'uniq' to include only
    unique entries. If omitted, no change log processing is done.
    Separate each tag with a comma.

Change-Id:
    This tag is used to generate the Message-Id of the emails that
    will be sent. When you keep the Change-Id the same you are
    asserting that this is a slightly different version (but logically
    the same patch) as other patches that have been sent out with the
    same Change-Id. The Change-Id tag line is removed from outgoing
    patches, unless the `keep_change_id` settings is set to `True`.

Various other tags are silently removed, like these Chrome OS and
Gerrit tags::

    BUG=...
    TEST=...
    Review URL:
    Reviewed-on:
    Commit-xxxx: (except Commit-notes)

Exercise for the reader: Try adding some tags to one of your current
patch series and see how the patches turn out.


Where Patches Are Sent
----------------------

Once the patches are created, patman sends them using git send-email. The
whole series is sent to the recipients in Series-to: and Series-cc.
You can Cc individual patches to other people with the Patch-cc: tag. Tags
in the subject are also picked up to Cc patches. For example, a commit like
this::

    commit 10212537b85ff9b6e09c82045127522c0f0db981
    Author: Mike Frysinger <vapier@gentoo.org>
    Date:    Mon Nov 7 23:18:44 2011 -0500

    x86: arm: add a git mailrc file for maintainers

    This should make sending out e-mails to the right people easier.

    Patch-cc: sandbox, mikef, ag
    Patch-cc: afleming

will create a patch which is copied to x86, arm, sandbox, mikef, ag and
afleming.

If you have a cover letter it will get sent to the union of the Patch-cc
lists of all of the other patches. If you want to sent it to additional
people you can add a tag::

    Cover-letter-cc: <list of addresses>

These people will get the cover letter even if they are not on the To/Cc
list for any of the patches.


Patchwork Integration
---------------------

Patman has a very basic integration with Patchwork. If you point patman to
your series on patchwork it can show you what new reviews have appeared since
you sent your series.

To set this up, add a Series-link tag to one of the commits in your series
(see above).

Then you can type:

.. code-block:: bash

    patman status

and patman will show you each patch and what review tags have been collected,
for example::

    ...
     21 x86: mtrr: Update the command to use the new mtrr
        Reviewed-by: Wolfgang Wallner <wolfgang.wallner@br-automation.com>
      + Reviewed-by: Bin Meng <bmeng.cn@gmail.com>
     22 x86: mtrr: Restructure so command execution is in
        Reviewed-by: Wolfgang Wallner <wolfgang.wallner@br-automation.com>
      + Reviewed-by: Bin Meng <bmeng.cn@gmail.com>
    ...

This shows that patch 21 and 22 were sent out with one review but have since
attracted another review each. If the series needs changes, you can update
these commits with the new review tag before sending the next version of the
series.

To automatically pull into these tags into a new branch, use the -d option:

.. code-block:: bash

    patman status -d mtrr4

This will create a new 'mtrr4' branch which is the same as your current branch
but has the new review tags in it. The tags are added in alphabetic order and
are placed immediately after any existing ack/review/test/fixes tags, or at the
end. You can check that this worked with:

.. code-block:: bash

    patman -b mtrr4 status

which should show that there are no new responses compared to this new branch.

There is also a -C option to list the comments received for each patch.


Example Work Flow
-----------------

The basic workflow is to create your commits, add some tags to the top
commit, and type 'patman' to check and send them.

Here is an example workflow for a series of 4 patches. Let's say you have
these rather contrived patches in the following order in branch us-cmd in
your tree where 'us' means your upstreaming activity (newest to oldest as
output by git log --oneline)::

    7c7909c wip
    89234f5 Don't include standard parser if hush is used
    8d640a7 mmc: sparc: Stop using builtin_run_command()
    0c859a9 Rename run_command2() to run_command()
    a74443f sandbox: Rename run_command() to builtin_run_command()

The first patch is some test things that enable your code to be compiled,
but that you don't want to submit because there is an existing patch for it
on the list. So you can tell patman to create and check some patches
(skipping the first patch) with:

.. code-block:: bash

    patman -s1 send -n

If you want to do all of them including the work-in-progress one, then
(if you are tracking an upstream branch):

.. code-block:: bash

    patman send -n

Let's say that patman reports an error in the second patch. Then:

.. code-block:: bash

    git rebase -i HEAD~6
    # change 'pick' to 'edit' in 89234f5
    # use editor to make code changes
    git add -u
    git rebase --continue

Now you have an updated patch series. To check it:

.. code-block:: bash

    patman -s1 send -n

Let's say it is now clean and you want to send it. Now you need to set up
the destination. So amend the top commit with:

.. code-block:: bash

    git commit --amend

Use your editor to add some tags, so that the whole commit message is::

    The current run_command() is really only one of the options, with
    hush providing the other. It really shouldn't be called directly
    in case the hush parser is bring used, so rename this function to
    better explain its purpose::

    Series-to: u-boot
    Series-cc: bfin, marex
    Series-prefix: RFC
    Cover-letter:
    Unified command execution in one place

    At present two parsers have similar code to execute commands. Also
    cmd_usage() is called all over the place. This series adds a single
    function which processes commands called cmd_process().
    END

    Change-Id: Ica71a14c1f0ecb5650f771a32fecb8d2eb9d8a17


You want this to be an RFC and Cc the whole series to the bfin alias and
to Marek. Two of the patches have tags (those are the bits at the front of
the subject that say mmc: sparc: and sandbox:), so 8d640a7 will be Cc'd to
mmc and sparc, and the last one to sandbox.

Now to send the patches, take off the -n flag:

.. code-block:: bash

   patman -s1 send

The patches will be created, shown in your editor, and then sent along with
the cover letter. Note that patman's tags are automatically removed so that
people on the list don't see your secret info.

Of course patches often attract comments and you need to make some updates.
Let's say one person sent comments and you get an Acked-by: on one patch.
Also, the patch on the list that you were waiting for has been merged,
so you can drop your wip commit.

Take a look on patchwork and find out the URL of the series. This will be
something like `http://patchwork.ozlabs.org/project/uboot/list/?series=187331`
Add this to a tag in your top commit::

   Series-links: 187331

You can use then patman to collect the Acked-by tag to the correct commit,
creating a new 'version 2' branch for us-cmd:

.. code-block:: bash

    patman status -d us-cmd2
    git checkout us-cmd2

You can look at the comments in Patchwork or with:

.. code-block:: bash

    patman status -C

Then you can resync with upstream:

.. code-block:: bash

    git fetch origin        # or whatever upstream is called
    git rebase origin/master

and use git rebase -i to edit the commits, dropping the wip one.

Then update the `Series-cc:` in the top commit to add the person who reviewed
the v1 series::

    Series-cc: bfin, marex, Heiko Schocher <hs@denx.de>

and remove the Series-prefix: tag since it it isn't an RFC any more. The
series is now version two, so the series info in the top commit looks like
this::

    Series-to: u-boot
    Series-cc: bfin, marex, Heiko Schocher <hs@denx.de>
    Series-version: 2
    Cover-letter:
    ...

Finally, you need to add a change log to the two commits you changed. You
add change logs to each individual commit where the changes happened, like
this::

    Series-changes: 2
    - Updated the command decoder to reduce code size
    - Wound the torque propounder up a little more

(note the blank line at the end of the list)

When you run patman it will collect all the change logs from the different
commits and combine them into the cover letter, if you have one. So finally
you have a new series of commits::

    faeb973 Don't include standard parser if hush is used
    1b2f2fe mmc: sparc: Stop using builtin_run_command()
    cfbe330 Rename run_command2() to run_command()
    0682677 sandbox: Rename run_command() to builtin_run_command()

so to send them:

.. code-block:: bash

    patman

and it will create and send the version 2 series.


General points
--------------

#. When you change back to the us-cmd branch days or weeks later all your
   information is still there, safely stored in the commits. You don't need
   to remember what version you are up to, who you sent the last lot of patches
   to, or anything about the change logs.
#. If you put tags in the subject, patman will Cc the maintainers
   automatically in many cases.
#. If you want to keep the commits from each series you sent so that you can
   compare change and see what you did, you can either create a new branch for
   each version, or just tag the branch before you start changing it:

   .. code-block:: bash

        git tag sent/us-cmd-rfc
        # ...later...
        git tag sent/us-cmd-v2

#. If you want to modify the patches a little before sending, you can do
   this in your editor, but be careful!
#. If you want to run git send-email yourself, use the -n flag which will
   print out the command line patman would have used.
#. It is a good idea to add the change log info as you change the commit,
   not later when you can't remember which patch you changed. You can always
   go back and change or remove logs from commits.
#. Some mailing lists have size limits and when we add binary contents to
   our patches it's easy to exceed the size limits. Use "--no-binary" to
   generate patches without any binary contents. You are supposed to include
   a link to a git repository in your "Commit-notes", "Series-notes" or
   "Cover-letter" for maintainers to fetch the original commit.
#. Patches will have no changelog entries for revisions where they did not
   change. For clarity, if there are no changes for this patch in the most
   recent revision of the series, a note will be added. For example, a patch
   with the following tags in the commit::

        Series-version: 5
        Series-changes: 2
        - Some change

        Series-changes: 4
        - Another change

   would have a changelog of:::

        (no changes since v4)

        Changes in v4:
        - Another change

        Changes in v2:
        - Some change


Other thoughts
--------------

This script has been split into sensible files but still needs work.
Most of these are indicated by a TODO in the code.

It would be nice if this could handle the In-reply-to side of things.

The tests are incomplete, as is customary. Use the 'test' subcommand to run
them:

.. code-block:: bash

    $ tools/patman/patman test

Note that since the test suite depends on data files only available in
the git checkout, the `test` command is hidden unless `patman` is
invoked from the U-Boot git repository.

Alternatively, you can run the test suite via Pytest:

.. code-block:: bash

    $ cd tools/patman && pytest

Error handling doesn't always produce friendly error messages - e.g.
putting an incorrect tag in a commit may provide a confusing message.

There might be a few other features not mentioned in this README. They
might be bugs. In particular, tags are case sensitive which is probably
a bad thing.
