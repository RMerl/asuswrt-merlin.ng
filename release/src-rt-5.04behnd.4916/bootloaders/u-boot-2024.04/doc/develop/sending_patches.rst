.. SPDX-License-Identifier: GPL-2.0+

Sending patches
===============

*Before you begin* to implement any new ideas or concepts it is always a good
idea to present your plans on the `U-Boot mailing list
<https://lists.denx.de/listinfo/u-boot>`_. U-Boot supports a huge amount of
very different systems, and it is often impossible for the individual developer
to oversee the consequences of a specific change to all architectures.
Discussing concepts early can help you to avoid spending effort on code which,
when submitted as a patch, might be rejected and/or will need lots of rework
because it does not fit for some reason. Early peer review is an important
resource - use it. Being familiar with the :doc:`process` is also important.

A good introduction how to prepare for submitting patches can be found in the
LWN article `How to Get Your Change Into the Linux Kernel
<http://lwn.net/Articles/139918/>`_ as the same rules apply to U-Boot, too.

Using patman
------------

You can use a tool called patman to prepare, check and send patches. It creates
change logs, cover letters and patch notes. It also simplifies the process of
sending multiple versions of a series.

See more details at :doc:`patman`.

General Patch Submission Rules
------------------------------

* All patches must be sent to the `u-boot@lists.denx.de
  <https://lists.denx.de/listinfo/u-boot>`_ mailing list.

* If your patch affects the code maintained by one of the :ref:`custodians`, CC
  them when emailing your patch. The easiest way to make sure you don't forget
  this even when you resubmit the patch later is to add a ``Cc: name
  <address>`` line after your ``Signed-off-by:`` line (see the example below).

* Take a look at the commit logs of the files you are modifying. Authors of
  past commits might have input to your change, so also CC them if you think
  they may have feedback.

* Patches should always contain exactly one complete logical change, i.e.

   * Changes that contain different, unrelated modifications shall be submitted
     as *separate* patches, one patch per changeset.

   * If one logical set of modifications affects or creates several files, all
     these changes shall be submitted in a *single* patch.

* Non-functional changes, i.e. whitespace and reformatting changes, should be
  done in separate patches marked as ``cosmetic``. This separation of functional
  and cosmetic changes greatly facilitates the review process.

* Some comments on running :doc:`checkpatch.pl <checkpatch>`:

   * Checkpatch is a tool that can help you find some style problems, but is
     imperfect, and the things it complains about are of varying importance.
     So use common sense in interpreting the results.

   * Warnings that clearly only make sense in the Linux kernel can be ignored.
     This includes ``Use #include <linux/$file> instead of <asm/$file>`` for
     example.

   * If you encounter warnings for existing code, not modified by your patch,
     consider submitting a separate, cosmetic-only patch -- clearly described
     as such -- that *precedes* your substantive patch.

   * For minor modifications (e.g. changed arguments of a function call),
     adhere to the present coding style of the module. Relating checkpatch
     warnings can be ignored in this case. A respective note in the commit or
     cover letter why they are ignored is desired.

* Send your patches as plain text messages: no HTML, no MIME, no links, no
  compression, no attachments. Just plain text. The best way the generate
  patches is by using the ``git format-patch`` command. Please use the
  ``master`` branch of the mainline U-Boot git repository
  (``https://source.denx.de/u-boot/u-boot.git``) as reference, unless (usually
  late in a release cycle) there has been an announcement to use the ``next``
  branch of this repository instead.

* Make sure that your mailer does not mangle the patch by automatic changes
  like wrapping of longer lines etc.
  The best way to send patches is by not using your regular mail tool, but by
  using either ``git send-email`` or the ``git imap-send`` command instead.
  If you believe you need to use a mailing list for testing (instead of any
  regular mail address you own), we have a special test list for such purposes.
  It would be best to subscribe to the list for the duration of your tests to
  avoid repeated moderation - see https://lists.denx.de/listinfo/test

* Choose a meaningful Subject: - keep in mind that the Subject will also be
  visible as headline of your commit message. Make sure the subject does not
  exceed 60 characters or so.

* The start of the subject should be a meaningful tag (arm:, ppc:, tegra:,
  net:, ext2:, etc)

* Include the string "PATCH" in the Subject: line of your message, e. g.
  "[PATCH] Add support for feature X". ``git format-patch`` should automatically
  do this.

* If you are sending a patch series composed of multiple patches, make sure
  their titles clearly state the patch order and total number of patches (``git
  format-patch -n``). Also, often times an introductory email describing what
  the patchset does is useful (``git format-patch -n --cover-letter``). As an
  example::

   [PATCH 0/3] Add support for new SuperCPU2000
      (This email does not contain a patch, just a description)
   [PATCH 1/3] Add core support for SuperCPU2000
   [PATCH 2/3] Add support for SuperCPU2000's on-chip I2C controller
   [PATCH 3/3] Add support for SuperCPU2000's on-chip UART

* In the message body, include a description of your changes.

   * For bug fixes: a description of the bug and how your patch fixes this bug.
     Please try to include a way of demonstrating that the patch actually fixes
     something.

   * For new features: a description of the feature and your implementation.

* Additional comments which you don't want included in U-Boot's history can be
  included below the first "``---``" in the message body.

* If your description gets too long, that's a strong indication that you should
  split up your patch.

* Remember that there is a size limit of 100 kB on the mailing list. In most
  cases, you did something wrong if your patch exceeds this limit. Think again
  if you should not split it into separate logical parts.

Attributing Code, Copyrights, Signing
-------------------------------------

* Sign your changes, i. e. add a *Signed-off-by:* line to the message body.
  This can be automated by using ``git commit -s``. Please see the
  :ref:`Developer Certificate of Origin <dco>` section for more details here.

* If you change or add *significant* parts to a file, then please make sure to
  add your copyright to that file, for example like this::

   (C) Copyright 2010  Joe Hacker <jh@hackers.paradise.com>

	  Please do *not* include a detailed description of your
	  changes. We use the *git* commit messages for this purpose.

* If you add new files, please always make sure that these contain your
  copyright note and a GPLv2+ SPDX-License-Identifier, for example like this::

   (C) Copyright 2010  Joe Hacker <jh@hackers.paradise.com>

   SPDX-License-Identifier:<TAB>GPL-2.0+

* If you are copying or adapting code from other projects, like the Linux
  kernel, or BusyBox, or similar, please make sure to state clearly where you
  copied the code from, and provide terse but precise information which exact
  version or even commit ID was used. Follow the ideas of this note from the
  Linux "SubmittingPatches" document::

   Special note to back-porters: It seems to be a common and useful practice
   to insert an indication of the origin of a patch at the top of the commit
   message (just after the subject line) to facilitate tracking. For instance,
   here's what we see in 2.6-stable :

	 Date:	Tue May 13 19:10:30 2008 +0000

		  SCSI: libiscsi regression in 2.6.25: fix nop timer handling

		  commit 4cf1043593db6a337f10e006c23c69e5fc93e722 upstream

   And here's what appears in 2.4 :

	 Date:	Tue May 13 22:12:27 2008 +0200

		  wireless, airo: waitbusy() won't delay

		  [backport of 2.6 commit b7acbdfbd1f277c1eb23f344f899cfa4cd0bf36a]

Whatever the format, this information provides a valuable help to people
tracking your trees, and to people trying to trouble-shoot bugs in your
tree.

Commit message conventions
--------------------------

Please adhere to the following conventions when writing your commit
log messages.

* The first line of the log message is the summary line. Keep this less than 70
  characters long.

* Don't use periods to end the summary line (e.g., don't do "Add support for
  X.")

* Use the present tense in your summary line (e.g., "Add support for X" rather
  than "Added support for X"). Furthermore, use the present tense in your log
  message to describe what the patch is doing. This isn't a strict rule -- it's
  OK to use the past tense for describing things that were happening in the old
  code for example.

* Use the imperative tense in your summary line (e.g., "Add support for X"
  rather than "Adds support for X"). In general, you can think of the summary
  line as "this commit is meant to 'Add support for X'"

* If applicable, prefix the summary line with a word describing what area of
  code is being affected followed by a colon. This is a standard adopted by
  both U-Boot and Linux. For example, if your change affects all mpc85xx
  boards, prefix your summary line with "mpc85xx:". If your change affects the
  PCI common code, prefix your summary line with "pci:". The best thing to do
  is look at the "git log <file>" output to see what others have done so you
  don't break conventions.

* Insert a blank line after the summary line

* For bug fixes, it's good practice to briefly describe how things behaved
  before this commit

* Put a detailed description after the summary and blank line. If the summary
  line is sufficient to describe the change (e.g. it is a trivial spelling
  correction or whitespace update), you can omit the blank line and detailed
  description.

* End your log message with S.O.B. (Signed-off-by) line. This is done
  automatically when you use ``git commit -s``. Please see the
  :ref:`Developer Certificate of Origin <dco>` section for more details here.

* Keep EVERY line under 72 characters. That is, your message should be
  line-wrapped with line-feeds. However, don't get carried away and wrap it too
  short either since this also looks funny.

* Detail level: The audience of the commit log message that you should cater to
  is those familiar with the underlying source code you are modifying, but who
  are _not_ familiar with the patch you are submitting. They should be able to
  determine what is being changed and why. Avoid excessive low-level detail.
  Before submitting, re-read your commit log message with this audience in mind
  and adjust as needed.

Sending updated patch versions
------------------------------

It is pretty normal that the first version of a patch you are submitting does
not get accepted as is, and that you are asked to submit another, improved
version.

When re-posting such a new version of your patch(es), please always make sure
to observe the following rules.

* Make an appropriate note that this is a re-submission in the subject line,
  e.g. "[PATCH v2] Add support for feature X". ``git format-patch
  --subject-prefix="PATCH v2"`` can be used in this case (see the example
  below).

* Please make sure to keep a "change log", i.e. a description of what you have
  changed compared to previous versions of this patch. This change log should
  be added below the "``---``" line in the patch, which starts the "comment
  section", i.e. which contains text that does not get included into the
  actual commit message.
  Note: it is *not* sufficient to provide a change log in some cover letter
  that gets sent as a separate message with the patch series. The reason is
  that such cover letters are not as easily reviewed in our `patchwork queue
  <http://patchwork.ozlabs.org/project/uboot/list/>`_ so they are not helpful
  to any reviewers using this tool. Example::

   From: Joe Hacker <jh@hackers.paradise.com>
   Date: Thu, 1 Jan 2222 12:21:22 +0200
   Subject: [PATCH 1/2 v3] FOO: add timewarp-support

   This patch adds timewarp-support for the FOO family of processors.

   adapted for the current kernel structures.

   Signed-off-by: Joe Hacker <jh@hackers.paradise.com>
   Cc: Tom Maintainer <tm@u-boot.custodians.org>
   ---
   Changes for v2:
   - Coding Style cleanup
   - fixed miscalculation of time-space discontinuities
   Changes for v3:
   - fixed compiler warnings observed with GCC-17.3.5
   - worked around integer overflow in warp driver

    arch/foo/cpu/spacetime.c |	 8 +
    drivers/warp/Kconfig     |	 7 +
    drivers/warp/Makefile    |	42 +++
    drivers/warp/warp-core.c | 255 +++++++++++++++++++++++++

* Make sure that your mailer adds or keeps correct ``In-reply-to:`` and
  ``References:`` headers, so threading of messages is working and everybody
  can see that the new message refers to some older posting of the same topic.

Uncommented and un-threaded repostings are extremely annoying and
time-consuming, as we have to try to remember if anything similar has been
posted before, look up the old threads, and then manually compare if anything
has been changed, or what.

If you have problems with your e-mail client, for example because it mangles
white space or wraps long lines, then please read this article about `Email
Clients and Patches <http://kerneltrap.org/Linux/Email_Clients_and_Patches>`_.

Notes
-----

1. U-Boot is Free Software that can redistributed and/or modified under the
   terms of the `GNU General Public License
   <http://www.fsf.org/licensing/licenses/gpl.html>`_ (GPL). Currently (August
   2022) version 2 of the GPL applies. Please see :download:`Licensing
   <../../Licenses/README>` for details. To allow that later versions of U-Boot
   may be released under a later version of the GPL, all new code that gets
   added to U-Boot shall use a "GPL-2.0+" SPDX-License-Identifier.

2. All code must follow the :doc:`codingstyle` requirements.

3. Before sending the patch, you *must* run some form of local testing.
   Submitting a patch that does not build or function correctly is a mistake. For
   non-trivial patches, either building a number of platforms locally or making
   use of :doc:`ci_testing` is strongly encouraged in order to avoid problems
   that can be found when attempting to merge the patch.

4. If you modify existing code, make sure that your new code does not add to
   the memory footprint of the code. Remember: Small is beautiful! When adding
   new features follow the guidelines laid out in :doc:`system_configuration`.

Patch Tracking
--------------

Like some other projects, U-Boot uses `Patchwork <http://patchwork.ozlabs.org/>`_
to track the state of patches. This is one of the reasons why it is mandatory
to submit all patches to the U-Boot mailing list - only then they will be
picked up by patchwork.

At http://patchwork.ozlabs.org/project/uboot/list/ you can find the list of
open U-Boot patches. By using the "Filters" link (Note: requires JavaScript)
you can also select other views, for example, to include old patches that have,
for example, already been applied or rejected.

Note that Patchwork automatically tracks and collects a number of git tags from
follow-up mails, so it is usually better to apply a patch through the Patchwork
commandline interface than just manually applying it from a posting on the
mailing list (in which case you have to do all the tracking and adding of git
tags yourself). This also obviates the need of a developer to resubmit a patch
only in order to collect these tags.

A Custodian has additional privileges and can:

* **Delegate** a patch

* **Change the state** of a patch. The following states exist:

   * New

   * Under Review

   * Accepted

   * Rejected

   * RFC

   * Not Applicable

   * Changes Requested

   * Awaiting Upstream

   * Superseded

   * Deferred

   * Archived

Patchwork work-flow
^^^^^^^^^^^^^^^^^^^

The following are a "rule of thumb" as to how the states are used in patchwork
today. Not all states are used by all custodians.

* New: Patch has been submitted to the list, and none of the maintainers has
  changed it's state since.

* Under Review: A custodian is reviewing the patch currently.

* Accepted: When a patch has been applied to a custodian repository that gets
  used for pulling from into upstream, they are put into "accepted" state.

* Rejected: Rejected means we just don't want to do what the patch does.

* RFC: The patch is not intended to be applied to any of the mainline
  repositories, but merely for discussing or testing some idea or new feature.

* Not Applicable: The patch either was not intended to be applied, as it was
  a debugging or discussion aide that patchwork picked up, or was cross-posted
  to our list but intended for another project entirely.

* Changes Requested: The patch looks mostly OK, but requires some rework before
  it will be accepted for mainline.

* Awaiting Upstream: A custodian may have applied this to the ``next`` branch
  and has not merged yet to master, or has queued the patch up to be submitted
  to be merged, but has not yet.

* Superseded: Patches are marked as 'superseded' when the poster submits a
  new version of these patches.

* Deferred: Deferred usually means the patch depends on something else that
  isn't upstream, such as patches that only apply against some specific other
  repository. This is also used when a patch has been in patchwork for over a
  year and it is unlikely to be applied as-is.

* Archived: Archiving puts the patch away somewhere where it doesn't appear in
  the normal pages and needs extra effort to get to.

Apply patches
^^^^^^^^^^^^^

To apply a patch from the `patchwork queue
<http://patchwork.ozlabs.org/project/uboot/list/>`_ using ``git``, download the
mbox file and apply it using::

   git am file

The `openembedded wiki <http://wiki.openembedded.net/>`_ also provides a script
named `pw-am.sh
<http://cgit.openembedded.org/cgit.cgi/openembedded/tree/contrib/patchwork/pw-am.sh>`_
which can be used to fetch an 'mbox' patch from patchwork and git am it::

   usage: pw-am.sh <number>
   example: 'pw-am.sh 71002' will get and apply the patch from http://patchwork.ozlabs.org/patch/71002/

Update the state of patches
^^^^^^^^^^^^^^^^^^^^^^^^^^^

You have to register to be able to update the state of patches. You can use the
Web interface, `pwclient`, or `pwparser`.

pwclient
^^^^^^^^

The `pwclient` command line tool can be used for example to retrieve patches,
search the queue or update the state.

All necessary information for `pwclient` is linked from the bottom of
http://patchwork.ozlabs.org/project/uboot/

Use::

   pwclient help

for an overview on how to use it.

pwparser
^^^^^^^^

See http://www.mail-archive.com/patchwork@lists.ozlabs.org/msg00057.html
