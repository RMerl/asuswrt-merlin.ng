#
#	Scripts to format dictionary files.
#
#	$Id$
#
.PHONY: format

#
#  This should only be run by hand, and then sanity checked by hand!
#
format: dictionary*
	@for x in dictionary* ; do \
		cat $$x | ./format.pl > tmp; \
		mv tmp $$x; \
	done
