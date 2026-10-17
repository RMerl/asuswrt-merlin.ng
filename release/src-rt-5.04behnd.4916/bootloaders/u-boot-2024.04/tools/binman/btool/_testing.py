# SPDX-License-Identifier: GPL-2.0+
# Copyright 2022 Google LLC
#
"""Bintool used for testing

This is not a real bintool, just one used for testing"""

from binman import bintool

# pylint: disable=C0103
class Bintool_testing(bintool.Bintool):
    """Bintool used for testing"""
    def __init__(self, name):
        super().__init__(name, 'testing')
        self.present = False
        self.install = False
        self.disable = False

    def is_present(self):
        if self.present is None:
            return super().is_present()
        return self.present

    def version(self):
        return '123'

    def fetch(self, method):
        if self.disable:
            return super().fetch(method)
        if method == bintool.FETCH_BIN:
            if self.install:
                return self.apt_install('package')
            return self.fetch_from_drive('junk')
        if method == bintool.FETCH_BUILD:
            return self.build_from_git('url', 'target', 'pathname')
        return None
