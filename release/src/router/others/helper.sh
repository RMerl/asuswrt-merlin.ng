#!/bin/sh

# Asuswrt-Merlin helper functions
# For use with Postconf or addon scripts

_quote() {
	printf "%s\n" "$1" | sed 's/[]\/$*.^&[]/\\&/g'
}

# This function looks for a string, and inserts a specified string after it inside a given file
# $1: the line to locate, $2: the line to insert, $3: Config file where to insert
pc_insert() {
	PATTERN=$(_quote "$1")
	CONTENT=$(_quote "$2")
	sed -i "/$PATTERN/a$CONTENT" $3
}

# This function looks for a string, and replace it with a different string inside a given file
# $1: the line to locate, $2: the line to replace with, $3: Config file where to insert
pc_replace() {
	PATTERN=$(_quote "$1")
	CONTENT=$(_quote "$2")
	sed -i "s/$PATTERN/$CONTENT/" $3
}

# This function will append a given string at the end of a given file
# $1 The line to append at the end, $2: Config file where to append
pc_append() {
	echo "$1" >> $2
}

# This function will delete a line containing a given string inside a given file
# $1 The line to locate, $2: Config file where to delete
pc_delete() {
	PATTERN=$(_quote "$1")
	sed -i "/$PATTERN/d" $2
}

# This function is used to find either the first available mount point for a
# new custom webui page, or return the mount point currently used if your page
# is already mounted on the webui.
#
# This will take the full path to the new page as argument.
# On return, the am_webui_page variable with will contain either the filename
# of the first available mount point, the filename your page is already using,
# or "none" if there are no available mount points.
am_get_webui_page() {
        am_webui_page="none"
        # look for a match first in case the page is already there
        for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do
                page="/www/user/user$i.asp"
                if [ -f "$page" ] && [ "$(md5sum < "$1")" = "$(md5sum < "$page")" ]; then
                        am_webui_page="user$i.asp"
                        return
                elif [ "$am_webui_page" = "none" ] && [ ! -f "$page" ]; then
                        am_webui_page="user$i.asp"
                fi
        done
}


_am_settings_path=/jffs/addons/custom_settings.txt

# This function will return the value associated to a specific variable.
# Example: VERSION=$(am_settings_get addon_version)
am_settings_get() {
	if [ ! -f $_am_settings_path ]; then
		touch $_am_settings_path
	fi
	grep -E "^$1 " $_am_settings_path | cut -f2- -d ' '
}

# This function will set a variable to the desired value.
# Example:  am_settings_set addon_title Cool Addon 1.0
am_settings_set() {
	if [ ! -f $_am_settings_path ]; then
		touch $_am_settings_path
	fi
	sed -i "\\~^$1 ~d" $_am_settings_path
	echo "$@" >> $_am_settings_path
}
