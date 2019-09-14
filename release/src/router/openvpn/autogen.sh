#!/bin/sh
autoreconf -if
[ ! -f .gitignore ]; touch .gitignore
[ ! -f .gitattributes ]; touch .gitattributes
