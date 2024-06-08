#!/usr/bin/env python3

import os
import platform
from sys import exit

from test.base_test import HTTPS, SKIP_TEST
from test.http_test import HTTPTest
from misc.wget_file import WgetFile

"""
Test that Wget handles the --convert-links (-k) option correctly.

Ensure that when downloading, Wget retains the protocol of the host URL
if the link wasn't followed and an absolute link is to be added to the
local file.
"""

if os.getenv('SSL_TESTS') is None:
    exit(SKIP_TEST)

############################## File Definitions ##############################

index = """
<html>
  <head>
    <title>Index</title>
  </head>
  <body>
    <a href="sub.html">Site</a>
    <a href="missing.xhtml">Missing</a>
    <a href="//localhost:{{port}}/missing2.html">Missing2</a>
  </body>
</html>
"""

converted = """
<html>
  <head>
    <title>Index</title>
  </head>
  <body>
    <a href="sub.html">Site</a>
    <a href="https://localhost:{{port}}/missing.xhtml">Missing</a>
    <a href="https://localhost:{{port}}/missing2.html">Missing2</a>
  </body>
</html>
"""

site = """
<html>
  <head>
    <title>Site</title>
  </head>
  <body>
    Subsite
  </body>
</html>
"""

IndexPage = WgetFile("index.html", index)
SubSite = WgetFile("sub.html", site)
LocalIndexPage = WgetFile("index.html", converted)

print(platform.system())
restrict = "unix" if platform.system() in ["Linux", "Darwin"] else "windows"

WGET_OPTIONS = "-k -r -nH --reject-regex '.*\\.xhtml' --no-check-certificate"
WGET_URLS = [["index.html"]]

Files = [[IndexPage, SubSite]]

Servers = [HTTPS]

ExpectedReturnCode = 8
ExpectedDownloadedFiles = [LocalIndexPage, SubSite]

########################### Pre and Post Test Hooks ##########################
pre_test = {
    "ServerFiles": Files,
}
test_options = {
    "WgetCommands": WGET_OPTIONS,
    "Urls": WGET_URLS
}
post_test = {
    "ExpectedFiles": ExpectedDownloadedFiles,
    "ExpectedRetcode": ExpectedReturnCode
}

err = HTTPTest(
    pre_hook=pre_test,
    test_params=test_options,
    post_hook=post_test,
    protocols=Servers,
).begin()

exit(err)
