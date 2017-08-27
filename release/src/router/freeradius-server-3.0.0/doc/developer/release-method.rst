Release Method
==============

As of 2.0, the release process is much simpler.  Edit the
Changelog with the version number and any last updates.

vi doc/ChangeLog
git commit doc/ChangeLog

	Change version numbers in the VERSION file:

vi VERSION
git commit VERSION

	Make the files

	Note that it also does "make dist-check", which checks
	the build rules for various packages.

make dist


	Validate that the packages are OK.  If so, tag the release.

	Note that this does NOT actually do the tagging!  You will
	have to run the command it prints out yourself.

make dist-tag

	Sign the packages.  You will need the correct GPG key for this
	to work.

make dist-sign


	Push to the FTP site.  You will need write access to the FTP site
	for this to work.

make dist-publish
