#!/bin/sh
# Packs dist/miscom-<target>.tar.gz (.zip for Windows) for av: the
# program at the top, with its sounds/ (where it has sound), COPYING,
# README, ChangeLog, the man page and PATCHES.md. Build first with
# asciiverse/Makefile.
#
#   asciiverse/package.sh x86_64-unknown-linux-musl
set -eu
cd "$(dirname "$0")/.."
target="$1"
case "$target" in *windows*) exe=.exe ;; *) exe= ;; esac
stage=stage/miscom
rm -rf "$stage" && mkdir -p "$stage" dist
cp "miscom$exe" COPYING README ChangeLog miscom.6 PATCHES.md "$stage/"
case "$target" in *linux*) cp -r sounds "$stage/" ;; esac
if [ -n "$exe" ]; then
  (cd "$stage" && zip -qr "../../dist/miscom-$target.zip" .)
else
  tar -czf "dist/miscom-$target.tar.gz" -C "$stage" .
fi
ls dist
