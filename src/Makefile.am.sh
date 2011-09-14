#!/bin/sh

list() {
	ls -1 "$@" | sort -V | tr '\n' ' ' | fmt -w60 | tr '\n' '|' |
		sed -e 's,|$,,' -e 's,|, \\\n\t,g'
}

cd "${0%/*}"
cat <<EOT | tee Makefile.am
AM_CFLAGS = @AM_CFLAGS@ \$(libev_CFLAGS)

lib_LTLIBRARIES = libeh.la

libeh_la_SOURCES = \\
	$(list *.c)

include_HEADERS = \\
	$(list *.h)
EOT
