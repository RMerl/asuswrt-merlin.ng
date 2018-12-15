# How to Contribute

Rsyslog is a real open source project and open to contributions.
By contributing, you help improve the state of logging as well as improve
your own professional profile. Contributing is easy, and there are options
for everyone - you do not need to be a developer.

-------------------------------------------------------------------------------------
LEGAL GDPR NOTICE:
According to the European data protection laws (GDPR), we would like to make you
aware that contributing to rsyslog via git will permanently store the
name and email address you provide as well as the actual commit and the
time and date you made it inside git's version history. This is inevitable,
because it is a main feature git. If you are concerned about your
privacy, we strongly recommend to use

--author "anonymous <gdpr@example.com>"

together with your commit. Also please do NOT sign your commit in this case,
as that potentially could lead back to you. Please note that if you use your
real identity, the GDPR grants you the right to have this information removed
later. However, we have valid reasons why we cannot remove that information
later on. The reasons are:

* this would break git history and make future merges unworkable
* the rsyslog projects has legitimate interest to keep a permanent record of the
  contributor identity, once given, for
  - copyright verification
  - being able to provide proof should a malicious commit be made

Please also note that your commit is public and as such will potentially be
processed by many third-parties. Git's distributed nature makes it impossible
to track where exactly your commit, and thus your personal data, will be stored
and be processed. If you would not like to accept this risk, please do either
commit anonymously or refrain from contributing to the rsyslog project.

-------------------------------------------------------------------------------------

With that legal stuff said, let's get to the real thing:

These are many ways to contribute to the project:
 * become a rsyslog ambassador and let other people know about rsyslog and how to utilize it for best results. Help rsyslog getting backlinks, be present on Internet news sites or at meetings you attend.
 * help others by offering support on
   * the rsyslog forums at http://kb.monitorware.com/rsyslog-f40.html
   * the rsyslog mailing list at http://lists.adiscon.net/mailman/listinfo/rsyslog
 * help with the documentation; you can either contribute
   * to the [rsyslog doc directory](https://github.com/rsyslog/rsyslog/tree/master/doc), which is shown on http://rsyslog.com/doc
   * to the rsyslog project web site -- just ask us for account creation
   * on the rsyslog wiki at http://wiki.rsyslog.com/
 * become a bug-hunter and help with testing rsyslog development releases
 * help driving the rsyslog infrastructure with its web sites, wikis and the like
 * help creating packages
 * or, obviously, help with rsyslog code development

This list is not conclusive. There for sure are many more ways to contribute and if you find one, just let us know. We are very open to new suggestions and like to try out new things.

## When to submit Pull Requests?

It is OK to submit PRs that are not yet fully ready for merging. You want to
do this in order to get early CI system coverage for your patch. However,
all patches should be reasonably complete and "work" in a sense.

If you submit such PRs, please flag them as "work in progress" by adding
"WiP:" in front of the title. We will NOT merge these PRs before you tell us
they are now ready for merging.

If you just want/need to do a temporary experiment, you may open a PR, flag it
as "EXPERIMENT - DO NOT MERGE", let the CI tests run, check results and close
the PR thereafter. This prevents unnecessary cluttering of the open PR list.
We will take the liberty to close such PRs if they are left open for more
than a day or two.

Please note, though, that the rsyslog repo is fully set up to use Travis CI.
Travis covers about 95% of all essential testing. So we highly recommend
that you use Travis to do initial checks on your work and create the PR
only after this looks good. That saves both you and us some time.

## Requirements for patches

In order to ensure good code quality, after applying the path the code must

- no legacy configuration statements ($someSetting) must be added,
  all configuration must be in v6+ style (RainerScript)
- compile cleanly without WARNING messages under both gcc and clang
- pass clang static analyzer without any report
- pass all CI tests
- new functionality must have associated
  * testbench tests
  * doc additions in the rsyslog-doc sister project

### Testbench Coverage

If you fix a bug that is not detected by the current testbench, it is
appreciated if you also add testbench test to make sure the problem does
not re-occur in the future.

In contrast to new feature PRs, this is not a hard requirement, but it
helps to speed up merging. If there is no testbench test added, the
core rsyslog developers will try to add one based on the patch. That
means merging needs to wait until we have time to do this.

### Compiler Diagnostics

Note that both warning messages and static analyzer warnings may be false
positives. We have decided to accept that fate and work around it (e.g. by
re-arranging the code, etc). Otherwise, we cannot use these useful features.

As a last resort, compiler warnings can be turned off via
   #pragma diagnostic
directives. This should really only be done if there is no other known
way around it. If so, it should be applied to a single function, only and
not to full source file. Be sure to re-enable the warning after the function
in question. We have done this in some few cases ourselves, and if someone
can fix the root cause, we would appreciate help. But, again, this is a
last resort which should normally not be used.


### Continuous Integration Testing

All patches are run though our continuous integration system, which ensures
no regressions are inside the code as well as rsyslog project policies are
followed (as far as we can check in an automated way).

For pull requests submitted via github, these two conditions are 
verified automatically. See the PR for potential failures. For patches
submitted otherwise, they will be verified semi-manually.

Also, patches are requested to not break the testbench. Unfortunately, the
current testbench has some racy tests, which are still useful enough so that
we do not want to disable them until the root cause has been found. If your
PR runs into something that you think is not related to your code, just sit
back and relax. The rsyslog core developer team reviews PRs regularly and
restarts tests which we know to look racy. If the problem persists, we will
contact you.

All PRs will be tested on a variety of systems, with the help of both Travis
CI and buildbot. The core goal of this multi-platform testing is to find
issues that surface only on some systems (e.g. 32bit related issues, etc).
We continuously strive to update the CI system coverage. If you can provide
a buildbot slave for a not-yet-supported test platform, please let us know.
We will gladly add it.

Note that test coverage differs between platforms. For example, not all
databases etc. are tested on each platform. Also note that due to resource
constraints some very lengthy tests are only execute on some (maybe only
a single) platform.

Note that we always try to merge with the most recent master branch and
try a build from that version (if automatic merging is possible). If this
test fails but no other, chances are good that there is an inter-PR issue.
If this happens, it is suggested to rebase to git master branch and update
the PR.

## Note to developers

Please address pull requests against the master branch.


## Testbench coding Tips

- the "cmp" command requires two parameters to work reliably accross multiple
  platforms. Using "cmp - file" make you compare stdin, as in:
  echo "test" | cmp - rsyslog.out.log
