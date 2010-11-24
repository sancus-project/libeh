#!/bin/sh
HTML="${0%.sh}" DIR="${0%/*}"
rm -rf "$HTML"/*
(cd "$DIR"; doxygen)
cd "$HTML"
git ls-files -o -m | xargs git add
