
Putting out a new release
-------------------------

Here are the steps that the maintainer should take when putting out a
new Tor release:

=== 0. Preliminaries

1. Get at least two of weasel/arma/Sebastian to put the new
   version number in their approved versions list.  Give them a few
   days to do this if you can.

2. If this is going to be an important security release, give the packagers
   some advance warning: See this list of packagers in IV.3 below.

3. Given the release date for Tor, ask the TB team about the likely release
   date of a TB that contains it.  See note below in "commit, upload,
   announce".

=== I. Make sure it works

1. Use it for a while, as a client, as a relay, as a hidden service,
   and as a directory authority. See if it has any obvious bugs, and
   resolve those.

   As applicable, merge the `maint-X` branch into the `release-X` branch.
   But you've been doing that all along, right?

2. Are all of the jenkins builders happy?  See jenkins.torproject.org.

   What about the bsd buildbots?
         See http://buildbot.pixelminers.net/builders/

   What about Coverity Scan?

   What about clang scan-build?

   Does 'make distcheck' complain?

   How about 'make test-stem' and 'make test-network' and
   `make test-network-full`?

       - Are all those tests still happy with --enable-expensive-hardening ?

   Any memory leaks?


=== II. Write a changelog


1a. (Alpha release variant)

   Gather the `changes/*` files into a changelog entry, rewriting many
   of them and reordering to focus on what users and funders would find
   interesting and understandable.

   To do this, first run `./scripts/maint/lintChanges.py changes/*` and
   fix as many warnings as you can.  Then run `./scripts/maint/sortChanges.py
   changes/* > changelog.in` to combine headings and sort the entries.
   After that, it's time to hand-edit and fix the issues that lintChanges
   can't find:

   1. Within each section, sort by "version it's a bugfix on", else by
      numerical ticket order.

   2. Clean them up:

      Make stuff very terse

      Make sure each section name ends with a colon

      Describe the user-visible problem right away

      Mention relevant config options by name.  If they're rare or unusual,
      remind people what they're for

      Avoid starting lines with open-paren

      Present and imperative tense: not past.

      'Relays', not 'servers' or 'nodes' or 'Tor relays'.

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
   identical to their original versions, with a 'backport from 0.x.y.z'
   note added to each section.  So in this case, once you have the items
   from the changes files copied together, don't use them to build a new
   changelog: instead, look up the corrected versions that were merged
   into ChangeLog in the master branch, and use those.

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


=== III. Making the source release.

1. In `maint-0.?.x`, bump the version number in `configure.ac` and run
   `perl scripts/maint/updateVersions.pl` to update version numbers in other
   places, and commit.  Then merge `maint-0.?.x` into `release-0.?.x`.

   (NOTE: To bump the version number, edit `configure.ac`, and then run
   either `make`, or `perl scripts/maint/updateVersions.pl`, depending on
   your version.)

   When you merge the maint branch forward to the next maint branch, or into
   master, merge it with "-s ours" to avoid a needless version bump.

2. Make distcheck, put the tarball up in somewhere (how about your
   homedir on your homedir on people.torproject.org?) , and tell `#tor`
   about it. Wait a while to see if anybody has problems building it.
   (Though jenkins is usually pretty good about catching these things.)

=== IV. Commit, upload, announce

1. Sign the tarball, then sign and push the git tag:

        gpg -ba <the_tarball>
        git tag -u <keyid> tor-0.3.x.y-status
        git push origin tag tor-0.3.x.y-status

   (You must do this before you update the website: it relies on finding
   the version by tag.)

2. scp the tarball and its sig to the dist website, i.e.
   `/srv/dist-master.torproject.org/htdocs/` on dist-master. When you want
   it to go live, you run "static-update-component dist.torproject.org"
   on dist-master.

   In the webwml.git repository, `include/versions.wmi` and `Makefile`
   to note the new version.

   (NOTE: Due to #17805, there can only be one stable version listed at
   once.  Nonetheless, do not call your version "alpha" if it is stable,
   or people will get confused.)

3. Email the packagers (cc'ing tor-team) that a new tarball is up.
   The current list of packagers is:

       - {weasel,gk,mikeperry} at torproject dot org
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

   Also, email tor-packagers@lists.torproject.org.

4. Add the version number to Trac.  To do this, go to Trac, log in,
    select "Admin" near the top of the screen, then select "Versions" from
    the menu on the left.  At the right, there will be an "Add version"
    box.  By convention, we enter the version in the form "Tor:
    0.2.2.23-alpha" (or whatever the version is), and we select the date as
    the date in the ChangeLog.

5. Double-check: did the version get recommended in the consensus yet?  Is
   the website updated?  If not, don't announce until they have the
   up-to-date versions, or people will get confused.

6. Mail the release blurb and ChangeLog to tor-talk (development release) or
   tor-announce (stable).

   Post the changelog on the blog as well. You can generate a
   blog-formatted version of the changelog with the -B option to
   format-changelog.

   When you post, include an estimate of when the next TorBrowser
   releases will come out that include this Tor release.  This will
   usually track https://wiki.mozilla.org/RapidRelease/Calendar , but it
   can vary.


=== V. Aftermath and cleanup

1. If it's a stable release, bump the version number in the
    `maint-x.y.z` branch to "newversion-dev", and do a `merge -s ours`
    merge to avoid taking that change into master.

2. Forward-port the ChangeLog (and ReleaseNotes if appropriate).

3. Keep an eye on the blog post, to moderate comments and answer questions.

