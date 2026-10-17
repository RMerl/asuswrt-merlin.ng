.. SPDX-License-Identifier: GPL-2.0+:

U-Boot Development Process
==========================

Management Summary
------------------

* Development happens in Release Cycles of 3 months.

* The first 3 weeks of the cycle are referred to as the Merge Window, which is
  followed by a Stabilization Period.

* Patches with new code get only accepted while the Merge Window is open.

* A patch that is generally in good shape and that was submitted while the
  Merge Window was open is eligible to go into the upcoming release, even if
  changes and resubmits are needed.

* During the Stabilization Period, only patches that contain bug fixes get
  applied.

Phases of the Development Process
---------------------------------

U-Boot development takes place in `Release Cycles
<https://www.denx.de/wiki/U-Boot/ReleaseCycle>`_.  A Release Cycle lasts
normally for three months.

The first three weeks of each Release Cycle are called *Merge Window*.

It is followed by a *Stabilization Period*.

The end of a Release Cycle is marked by the release of a new U-Boot version.

Merge Window
------------

The Merge Window is the period when new patches get submitted (and hopefully
accepted) for inclusion into U-Boot mainline. This period lasts for 21 days (3
weeks) and ends with the release of ``"-rc1"``.

This is the only time when new code (like support for new processors or new
boards, or other new features or reorganization of code) is accepted.

Twilight Time
-------------

Usually patches do not get accepted as they are - the peer review that takes
place will usually require changes and resubmissions of the patches before they
are considered to be ripe for inclusion into mainline.

Also the review often happens not immediately after a patch was submitted,
but only when somebody (usually the responsible custodian) finds time to do
this.

The result is that the final version of such patches gets submitted after the
merge window has been closed.

It is current practice in U-Boot that such patches are eligible to go into the
upcoming release.

The result is that the release of the ``"-rc1"`` version and formal closing of
the Merge Window does not preclude patches that were already posted from being
merged for the upcoming release.

Stabilization Period
--------------------

During the Stabilization Period only patches containing bug fixes get
applied.

Corner Cases
------------

Sometimes it is not clear if a patch contains a bug fix or not.
For example, changes that remove dead code, unused macros etc. or
that contain Coding Style fixes are not strict bug fixes.

In such situations it is up to the responsible custodian to decide if they
apply such patches even when the Merge Window is closed.

Exception: at the end of the Stabilization Period only strict bug
fixes my be applied.

Sometimes patches miss the Merge Window slightly - say by a few
hours or even a day. Patch acceptance is not as critical as a
financial transaction, or such. So if there is such a slight delay,
the custodian is free to turn a blind eye and accept it anyway. The
idea of the development process is to make it foreseeable,
but not to slow down development.

It makes more sense if an engineer spends another day on testing and
cleanup and submits the patch a couple of hours late, instead of
submitting a green patch which will waste efforts from several people
during several rounds of review and reposts.

Differences to the Linux Development Process
--------------------------------------------

* In Linux, top-level maintainers will collect patches in their trees and send
  pull requests to Linus as soon as the merge window opens.
  So far, most U-Boot custodians do not work like that; they send pull requests
  only at (or even after) the end of the merge window.

* In Linux, the closing of the merge window is marked by the release of the
  next ``"-rc1"``
  In U-Boot, ``"-rc1"`` will only be released after all (or at least most of
  the) patches that were submitted during the merge window have been applied.

.. _custodians:

Custodians
----------

The Custodians take responsibility for some area of the U-Boot code.  The
in-tree ``MAINTAINERS`` files list who is responsible for which areas.

It is their responsibility to pick up patches from the mailing list
that fall into their responsibility, and to process these.

A very important responsibility of each custodian is to provide
feedback to the submitter of a patch about what is going on: if the
patch was accepted, or if it was rejected (which exact list of
reasons), if it needs to be reworked (with respective review
comments). Even a "I have no time now, will look into it later"
message is better than nothing. Also, if there are remarks to a
patch, these should leave no doubt if they were just comments and the
patch will be accepted anyway, or if the patch should be
reworked/resubmitted, or if it was rejected.

Review Process, Git Tags
------------------------

There are a number of *git tags* that are used to document the origin and the
processing of patches on their way into the mainline U-Boot code. The following
is an attempt to document how these are usually handled in the U-Boot project.

In general, we try to follow the established procedures from other projects,
especially the Linux kernel, but there may be smaller differences. For
reference, see the Linux kernel's `Submitting patches
<https://www.kernel.org/doc/html/latest/process/submitting-patches.html>`_
document.

.. _dco:

* Signed-off-by: the *Signed-off-by:* is a line at the end of the commit
  message by which the signer certifies that they were involved in the development
  of the patch and that they accept the `Developer Certificate of Origin
  <https://developercertificate.org/>`_. Following this and adding a
  ``Signed-off-by:`` line that contains the developer's name and email address
  is required.

   * Please note that in U-Boot, we do not add a ``Signed-off-by`` tag if we
     just pass on a patch without any changes.

   * Please note that when importing code from other projects you must say
     where it comes from, and what revision you are importing. You must not
     however copy ``Signed-off-by`` or other tags.

* Everybody who can is invited to review and test the changes. Typically, we
  follow the same guidelines as the Linux kernel for `Acked-by
  <https://www.kernel.org/doc/html/latest/process/submitting-patches.html#when-to-use-acked-by-cc-and-co-developed-by>`_
  as well as `Reviewed-by
  <https://www.kernel.org/doc/html/latest/process/submitting-patches.html#using-reported-by-tested-by-reviewed-by-suggested-by-and-fixes>`_
  and similar additional tags.

* Reviewed-by: The patch has been reviewed and found acceptable according to
  the `Reviewer's statement of oversight
  <https://www.kernel.org/doc/html/latest/process/submitting-patches.html#reviewer-s-statement-of-oversight>`_.
  A *Reviewed-by:* tag is a statement of opinion that the patch is an
  appropriate modification of the code without any remaining serious technical
  issues. Any interested reviewer (who has done the work) can offer a
  *Reviewed-by:* tag for a patch.

* Acked-by: If a person was not directly involved in the preparation or
  handling of a patch but wishes to signify and record their approval of it
  then they can arrange to have an *Acked-by:* line added to the patch's
  changelog.

* Tested-by: A *Tested-by:* tag indicates that the patch has been successfully
  tested (in some environment) by the person named. Andrew Morton: "I think
  it's very useful information to have. For a start, it tells you who has the
  hardware and knows how to build a kernel. So if you're making a change to a
  driver and want it tested, you can troll the file's changelog looking for
  people who might be able to help."

* Reported-by: If this patch fixes a problem reported by somebody else,
  consider adding a *Reported-by:* tag to credit the reporter for their
  contribution. Please note that this tag should not be added without the
  reporter's permission, especially if the problem was not reported in a public
  forum.

* Cc: If a person should have the opportunity to comment on a patch, you may
  optionally add a *Cc:* tag to the patch. Git tools (git send-email) will then
  automatically arrange that they receives a copy of the patch when you submit
  it to the mailing list. This is the only tag which might be added without an
  explicit action by the person it names. This tag documents that potentially
  interested parties have been included in the discussion.
  For example, when your change affects a specific board or driver, then makes
  a lot of sense to put the respective maintainer of this code on Cc:

Work flow of a Custodian
------------------------

The normal flow of work in the U-Boot development process will look
like this:

#. The responsible custodian inspects this patch, especially for:

   #. The commit message is useful, descriptive and makes correct and
      appropriate usage of required *git tags*.

   #. :doc:`codingstyle`

   #. Basic logic:

      * The patch fixes a real problem.

      * The patch does not introduce new problems, especially it does not break
        other boards or architectures

   #. U-Boot Philosophy, as documented in :doc:`designprinciples`.

   #. Applies cleanly to the source tree.  The custodian is expected to put in
      a "best effort" if a patch does not apply cleanly, but can be made to apply
      still.  It is up to the custodian to decide how recent of a commit the
      patch must be against.  It is acceptable to request patches against the
      last officially released version of U-Boot or newer.  Of course a
      custodian can also accept patches against older code.  It can be
      difficult to find the correct balance between putting too much work on
      the custodian or too much work on an individual submitting a patch when
      something does not apply cleanly.

   #. Passes :doc:`ci_testing` as this checks for new warnings and other issues.

#. Note that in some cases more than one custodian may feel responsible for a
   particular change.  To avoid duplicated efforts, the custodian who starts
   processing the patch should follow up to the email saying they intend to
   pick it up.

#. Commits must show original author in the ``author`` field and include all of
   the ``Signed-off-by``, ``Reviewed-by``, etc, tags that have been submitted.

#. The final decision to accept or reject a patch comes down to the custodian
   in question.

#. If accepted, the custodian adds the patch to their public git repository.
   Ideally, they will also follow up on the mailing list with some notification
   that it has been applied.  This is not always easy given different custodian
   workflows and environments however.

#. Although a custodian is supposed to perform their own tests it is a
   well-known and accepted fact that they need help from other developers who
   - for example - have access to the required hardware or other relevant
   environments.  Custodians are expected to ask for assistance with testing
   when required.

#. Custodians are expected to submit a timely pull request of their git
   repository to the main repository.  It is strongly encouraged that a CI run
   has been completed prior to submission, but not required.
