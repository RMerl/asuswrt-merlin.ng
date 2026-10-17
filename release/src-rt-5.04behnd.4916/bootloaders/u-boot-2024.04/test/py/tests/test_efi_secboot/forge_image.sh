# SPDX-License-Identifier: GPL-2.0-or-later
#!/bin/sh

replace_exp="s/H\0e\0l\0l\0o\0/h\0E\0L\0L\0O\0/g"
perl -p -e ${replace_exp} < $1 > $2
