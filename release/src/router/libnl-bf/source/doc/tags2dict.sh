#!/bin/bash
xmlstarlet sel	-t \
		-m "/libnltags/libnltag[@href]" \
		-v "@short" \
		-o "=api/" \
		-v "@href" \
		-n

