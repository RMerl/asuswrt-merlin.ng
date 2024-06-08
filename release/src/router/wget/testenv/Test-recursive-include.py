#!/usr/bin/env python3
from sys import exit
from test.http_test import HTTPTest
from test.base_test import HTTP
from misc.wget_file import WgetFile

"""
    Basic test of --recursive.
"""
# File Definitions ###############################################
File1 = """<html><body>
<a href=\"/a/File2.html\">text</a>
<a href=\"/b/File3.html\">text</a>
<picture>
  Hey, a source <source type=\"image/svg+xml\" lolli=\"xxx\" srcset=\"/a/logo.svg\"/>.
  Hey, a srcset <img src=\"/a/picture.png\" srcset=\"/a/picture1.png, /a/picture2.png 150w,/a/picture3.png 100x\"/>.
</picture>
</body></html>"""
File2 = "With lemon or cream?"
File3 = "Surely you're joking Mr. Feynman"
File4 = "logosvg"
File5 = "picturepng"
File6 = "picture1png"
File7 = "picture2png"
File8 = "picture3png"

File1_File = WgetFile("a/File1.html", File1)
File2_File = WgetFile("a/File2.html", File2)
File3_File = WgetFile("b/File3.html", File3)
File4_File = WgetFile("a/logo.svg", File4)
File5_File = WgetFile("a/picture.png", File5)
File6_File = WgetFile("a/picture1.png", File6)
File7_File = WgetFile("a/picture2.png", File7)
File8_File = WgetFile("a/picture3.png", File8)

WGET_OPTIONS = "--recursive --no-host-directories --include-directories=a"
WGET_URLS = [["a/File1.html"]]

Servers = [HTTP]

Files = [[File1_File, File2_File, File3_File, File4_File, File5_File, File6_File, File7_File, File8_File]]
Existing_Files = []

ExpectedReturnCode = 0
ExpectedDownloadedFiles = [File1_File, File2_File, File4_File, File5_File, File6_File, File7_File, File8_File]
Request_List = [[
    "GET /a/File1.html",
    "GET /a/File2.html",
    "GET /a/logo.svg",
    "GET /a/picture.png",
    "GET /a/picture1.png",
    "GET /a/picture2.png",
    "GET /a/picture3.png",
]]

# Pre and Post Test Hooks #####################################
pre_test = {
    "ServerFiles": Files,
    "LocalFiles": Existing_Files
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
    protocols=Servers
).begin()

exit(err)
