# How to Release Tor

Here are the steps that the maintainer should take when putting out a
new Tor release. It is split in 3 stages and coupled with our Tor CI Release
pipeline.

Before we begin, first rule is to make sure:

   - Our CIs (*nix and Windows) pass for each version to release
   - Coverity has no new alerts

## 0. Security Release

To start with, if you are doing a security release, this must be done few days
prior to the release:

   1. If this is going to be an important security release, give the packagers
      advance warning, via `tor-packagers@lists.torproject.org`.


## 1. Preliminaries

The following must be done **2 days** at the very least prior to the release:

   1. Add the version(s) in the dirauth-conf git repository as the
      RecommendedVersion and RequiredVersion so they can be approved by the
      authorities and be in the consensus before the release.

   2. Send a pre-release announcement to `tor-project@lists.torproject.org` in
      order to inform every teams in Tor of the upcoming release. This is so
      we can avoid creating release surprises and sync with other teams.

   3. Ask the network-team to review the `changes/` files in all versions we
      are about to release. This step is encouraged but not mandatory.


## 2. Tarballs

To build the tarballs to release, we need to launch the CI release pipeline:

   https://gitlab.torproject.org/tpo/core/tor-ci-release

The `versions.yml` needs to be modified with the Tor versions you want to
release. Once done, git commit and push to trigger the release pipeline.

The first two stages (Preliminary and Patches) will be run automatically. The
Build stage needs to be triggered manually once all generated patches have
been merged upstream.

   1. Download the generated patches from the `Patches` stage.

      Apply these patches to the `main` or `release` branch as appropriate.
      (Version bumps apply to `maint`; anything touching the changelog should
      apply only to `main` or `release`.)

      When updating the version, it will be on `maint` branches and so to
      merge-forward, use `git merge -s ours`. For instance, if merging the
      version change of `maint-0.4.5` into `maint-0.4.6`, do on `maint-0.4.6`
      this command: `git merge -s ours maint-0.4.5`. And then you can proceed
      with a git-merge-forward.

   2. For the ChangeLog and ReleaseNotes, you need to write a blurb at the top
      explaining a bit the release.

   3. Review, modify if needed, and merge them upstream.

   4. Manually trigger the `maintained` job in the `Build` stage so the CI can
      build the tarballs without errors.

Once this is done, each selected developers need to build the tarballs in a
reproducible way using:

   https://gitlab.torproject.org/tpo/core/tor-ci-reproducible

Steps are:

   1. Run `./build.sh` which will download everything you need, including the
      latest tarballs from the release CI, and auto-commit the signatures if
      the checksum match. You will need to confirm the commits.

   2. If all is good, `git push origin main` your signatures.

Once all signatures from all selected developers have been committed:

   1. Manually trigger the `signature` job in the `Post-process` stage of the
      CI release pipeline.

   2. If it passes, the tarball(s) and signature(s) will be available as
      artifacts and should be used for the release.

   3. Put them on `dist.torproject.org`:

      Upload the tarball and its sig to the dist website:

         `rsync -avP tor-*.gz{,.asc} dist-master.torproject.org:/srv/dist-master.torproject.org/htdocs/`

      Then, on dist-master.torproject.org, run:

         `static-update-component dist.torproject.org`

      For an alpha or latest stable, open an MR in
      https://gitlab.torproject.org/tpo/web/tpo that updates the
      `databags/versions.ini` to note the new version.

      (NOTE: Due to #17805, there can only be one stable version listed at once.
      Nonetheless, do not call your version "alpha" if it is stable, or people
      will get confused.)

      (NOTE: It will take a while for the website update scripts to update the
      website.)


## 3. Post Process

Once the tarballs have been uploaded and are ready to be announced, we need to
do the following:

   1. Tag versions (`main` branch or `release` branch as appropriate) using
      `git tag -s tor-0.x.y.z-<status>` and then push the tag(s):
      `git push origin tor-0.x.y.z-<status>`

      (This should be the `main` or `release` branch because that is the one
      from which the tarballs are built.  We want our tags to match our
      tarballs.)

   2. Merge upstream the artifacts from the `patches` job in the
      `Post-process` stage of the CI release pipeline.

      Like step (2.1) above, the `-dev` version bump need to be done manually
      with a `git merge -s ours`.

   3. Write and post the release announcement for the `forum.torproject.net`
      in the `News -> Tor Release Announcement` category.

      If possible, mention in which Tor Browser version (with dates) the
      release will be in. This usually only applies to the latest stable.

   4. Inform `tor-announce@lists.torproject.org` with the releasing pointing to
      the Forum. Append the ChangeLog there. We do this until we can automate
      such post from the forum directly.

   5. Update torproject.org website by submitting a MR to
      https://gitlab.torproject.org/tpo/web/tpo

      The `databags/versions.ini` file is the one to change with the newly
      released version(s).

### New Stable

   1. Create the `maint-x.y.z` and `release-x.y.z` branches at the version
      tag. Then update the `./scripts/git/git-list-tor-branches.sh` with the
      new version.

   2. Update `./scripts/git/git-list-tor-branches.sh` and
      `./scripts/ci/ci-driver.sh` with the new version in `maint-x.y.z` and
      then merge forward into main. (If you haven't pushed remotely the new
      branches, merge the local branch).

   3. In `main`, bump version to the next series: `tor-x.y.0-alpha-dev` and
      then tag it: `git tag -s tor-x.y.0-alpha-dev`

## Appendix: An alternative means to notify packagers

If for some reason you need to contact a bunch of packagers without
using the publicly archived tor-packagers list, you can try these
people:

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
       - {security} at brave.com
