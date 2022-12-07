# Security overview

## General

libexif is a software library to process EXIF datablobs, which are usually
embedded in JPEG files.

It allows reading, writing, changing, and extraction (binary and textual versions)
of this data.


## Attack Surface

Any data blob put into the library should be assumed untrusted and
potentially malicious.

ABI parameters can be considered trusted.

The primary attack scenario is processing of files for EXIF content
extraction (displaying) via unattended services, up to and including
webservices where files can be uploaded by potential attackers.

## Bugs considered security issues

(Mostly for CVE assigments rules.)

Triggering memory corruption of any form is considered in scope.
Triggering endless loops is considered in scope. (would block services)
Triggering unintentional aborts is considered in scope.

Common library usage patterns are in scope.

Crashes during writing out of data as EXIF could be in scope.

## Bugs not considered security issues

Crashes caused by debugging functionality are not in scope.

## Bugreports

Bugreports can be filed as github issues.

If you want to report an embargoed security bug report, reach out to dan@coneharvesters.com and marcus@jet.franken.de.
