# Putting out a new release

Here are the steps that the maintainer should take when putting out a
new Tor release:

## 0. Preliminaries

1. Get at least two of weasel/arma/Sebastian to put the new
   version number in their approved versions list.  Give them a few
   days to do this if you can.

2. If this is going to be an important security release, give these packagers
   some advance warning:

       - {weasel,sysrqb,mikeperry} at torproject dot org
       - {blueness} at gentoo dot org
       - {paul} at invizbox dot io
       - {vincent} at invizbox dot com
       - {lfleischer} at archlinux dot org
       - {Nathan} at freitas dot net
       - {mike} at tig dot as
       - {tails-rm} at boum dot org
       - {simon} at sdeziel.info
       - {yuri} at freebsd.org
       - {mh+tor} at scrit.ch

3. Given the release date for Tor, ask the TB team about the likely release
   date of a TB that contains it.  See note below in "commit, upload,
   announce".

## I. Make sure it works

1. Make sure that CI passes: have a look at Travis
   (https://travis-ci.org/torproject/tor/branches), Appveyor
   (https://ci.appveyor.com/project/torproject/tor/history), and
   Jenkins (https://jenkins.torproject.org/view/tor/).
   Make sure you're looking at the right branches.

   If there are any unexplained failures, try to fix them or figure them
   out.

2. Verify that there are no big outstanding issues.  You might find such
   issues --

    * On Trac

    * On coverity scan

    * On OSS-Fuzz

## II. Write a changelog

1a. (Alpha release variant)

   Gather the `changes/*` files into a changelog entry, rewriting many
   of them and reordering to focus on what users and funders would find
   interesting and understandable.

   To do this, run `./scripts/maint/sortChanges.py changes/* > changelog.in`
   to combine headings and sort the entries.  Copy the changelog.in file into
   the ChangeLog.  Run `format_changelog.py --inplace` (see below) to clean up
   the line breaks.

   Remove the `changes/*` files that you just merged into the ChangeLog.

   After that, it's time to hand-edit and fix the issues that
   lintChanges can't find:

   1. Within each section, sort by "version it's a bugfix on", else by
      numerical ticket order.

   2. Clean them up:

      Make stuff very terse

      Describe the user-visible problem right away

      Mention relevant config options by name.  If they're rare or unusual,
      remind people what they're for

      Avoid starting lines with open-paren

      Present and imperative tense: not past.

      "Relays", not "servers" or "nodes" or "Tor relays".

      "Onion services", not "hidden services".

      "Stop FOOing", not "Fix a bug where we would FOO".

      Try not to let any given section be longer than about a page. Break up
      long sections into subsections by some sort of common subtopic. This
      guideline is especially important when organizing Release Notes for
      new stable releases.

      If a given changes stanza showed up in a different release (e.g.
      maint-0.2.1), be sure to make the stanzas identical (so people can
      distinguish if these are the same change).

   3. Clean everything one last time.

   4. Run `./scripts/maint/format_changelog.py --inplace` to make it prettier

1b. (old-stable release variant)

   For stable releases that backport things from later, we try to compose
   their releases, we try to make sure that we keep the changelog entries
   identical to their original versions, with a "backport from 0.x.y.z"
   note added to each section.  So in this case, once you have the items
   from the changes files copied together, don't use them to build a new
   changelog: instead, look up the corrected versions that were merged
   into ChangeLog in the master branch, and use those.

   Add "backport from X.Y.Z" in the section header for these entries.

2. Compose a short release blurb to highlight the user-facing
   changes. Insert said release blurb into the ChangeLog stanza. If it's
   a stable release, add it to the ReleaseNotes file too. If we're adding
   to a release-* branch, manually commit the changelogs to the later
   git branches too.

3. If there are changes that require or suggest operator intervention
   before or during the update, mail operators (either dirauth or relays
   list) with a headline that indicates that an action is required or
   appreciated.

4. If you're doing the first stable release in a series, you need to
   create a ReleaseNotes for the series as a whole.  To get started
   there, copy all of the Changelog entries from the series into a new
   file, and run `./scripts/maint/sortChanges.py` on it.  That will
   group them by category.  Then kill every bugfix entry for fixing
   bugs that were introduced within that release series; those aren't
   relevant changes since the last series.  At that point, it's time
   to start sorting and condensing entries.  (Generally, we don't edit the
   text of existing entries, though.)

## III. Making the source release.

1. In `maint-0.?.x`, bump the version number in `configure.ac` and run
   `make update-versions` to update version numbers in other
   places, and commit.  Then merge `maint-0.?.x` into `release-0.?.x`.

   When you merge the maint branch forward to the next maint branch, or into
   master, merge it with "-s ours" to avoid conflict with the version
   bump.

2. Make distcheck, put the tarball up in somewhere (how about your
   homedir on people.torproject.org?) , and tell `#tor-dev`
   about it.

   If you want, wait until at least one person has built it
   successfully.  (We used to say "wait for others to test it", but our
   CI has successfully caught these kinds of errors for the last several
   years.)

3. Make sure that the new version is recommended in the latest consensus.
   (Otherwise, users will get confused when it complains to them
   about its status.)

   If it is not, you'll need to poke Roger, Weasel, and Sebastian again: see
   item 0.1 at the start of this document.

## IV. Commit, upload, announce

1. Sign the tarball, then sign and push the git tag:

```console
$ gpg -ba <the_tarball>
$ git tag -s tor-0.4.x.y-<status>
$ git push origin tag tor-0.4.x.y-<status>
```

   (You must do this before you update the website: the website scripts
   rely on finding the version by tag.)

   (If your default PGP key is not the one you want to sign with, then say
   "-u <keyid>" instead of "-s".)

2. scp the tarball and its sig to the dist website, i.e.
   `/srv/dist-master.torproject.org/htdocs/` on dist-master. Run
   "static-update-component dist.torproject.org" on dist-master.

   In the project/web/tpo.git repository, update `databags/versions.ini`
   to note the new version.  Push these changes to master.

   (NOTE: Due to #17805, there can only be one stable version listed at
   once.  Nonetheless, do not call your version "alpha" if it is stable,
   or people will get confused.)

   (NOTE: It will take a while for the website update scripts to update
   the website.)

3. Email the tor-packagers@lists.torproject.org mailing list to tell them
   about the new release.

   Also, email tor-packagers@lists.torproject.org.

   Mention where to download the tarball (https://dist.torproject.org).

   Include a link to the changelog.

4. Wait for the download page to be updated. (If you don't do this before you
   announce, people will be confused.)

5. Mail the release blurb and ChangeLog to tor-talk (development release) or
   tor-announce (stable).

   Post the changelog on the blog as well. You can generate a
   blog-formatted version of the changelog with
      `./scripts/maint/format_changelog.py -B`

   When you post, include an estimate of when the next TorBrowser
   releases will come out that include this Tor release.  This will
   usually track https://wiki.mozilla.org/RapidRelease/Calendar , but it
   can vary.

   For templates to use when announcing, see:
       https://gitlab.torproject.org/tpo/core/team/-/wikis/NetworkTeam/AnnouncementTemplates

## V. Aftermath and cleanup

1. If it's a stable release, bump the version number in the
   `maint-x.y.z` branch to "newversion-dev", and do a `merge -s ours`
   merge to avoid taking that change into master.

2. If there is a new `maint-x.y.z` branch, create a Travis CI cron job that
   builds the release every week. (It's ok to skip the weekly build if the
   branch was updated in the last 24 hours.)

3. Forward-port the ChangeLog (and ReleaseNotes if appropriate) to the
   master branch.

4. Keep an eye on the blog post, to moderate comments and answer questions.
