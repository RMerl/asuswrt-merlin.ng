#!/usr/bin/env python3

"""Ensure that Wget correctly encodes url() parameters in CSS."""

from test.base_test import HTTP
from test.http_test import HTTPTest

from misc.wget_file import WgetFile

############################## File Definitions ###############################
FILE1 = """<html>
    <head>
<style>
body {
    background-image: url(image%201.html);
}
</style>
    </head>
    <body>Hello</body>
</html>"""


FILE2 = "This is an image"

File1_File = WgetFile("index.html", FILE1)
File2_File = WgetFile("image 1.html", FILE2)
File2_ServerFile = WgetFile("image%201.html", FILE2)

WGET_OPTIONS = "--recursive --convert-links --no-host-directories"
WGET_URLS = [[""]]

Servers = [HTTP]

Files = [[File1_File, File2_ServerFile]]
Existing_Files = []

ExpectedReturnCode = 0
ExpectedDownloadedFiles = [File1_File, File2_File]
# Request_List = [["GET /",
#                  "GET /image 1.html"]]

################ Pre and Post Test Hooks #####################################
pre_test = {
    "ServerFiles"       : Files,
    "LocalFiles"        : Existing_Files
}
test_options = {
    "WgetCommands"      : WGET_OPTIONS,
    "Urls"              : WGET_URLS
}
post_test = {
    "ExpectedFiles"     : ExpectedDownloadedFiles,
    "ExpectedRetcode"   : ExpectedReturnCode
}

err = HTTPTest (
                pre_hook=pre_test,
                test_params=test_options,
                post_hook=post_test,
                protocols=Servers
).begin ()

exit (err)
