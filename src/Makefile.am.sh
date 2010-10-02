#!/bin/sh

list() {
	ls -1 "$@" | sort -V | tr '\n' ' ' |
		sed -e 's,^ *,,' -e 's, *$,,'
}

cat <<EOT | tee Makefile.am
AM_CFLAGS = \$(CWARNFLAGS)

lib_LTLIBRARIES = libeh.la

libeh_la_SOURCES = $(list *.c)

include_HEADERS = $(list *.h)
EOT
