#!/bin/sh
# Verify nii_cmp compares NIfTI content rather than compressed bytes.
#
# gzip -1 and gzip -9 encode the same input differently, which manufactures
# the byte difference locally instead of waiting for a runner whose system
# zlib happens to be zlib-ng.
#   usage: nii_cmp_selftest.sh <helper-dir>
. "$1/nii_cmp.sh"

tmp=$(mktemp -d) || exit 1
trap 'rm -rf "$tmp"' EXIT
cd "$tmp" || exit 1

head -c 65536 /dev/urandom > payload
gzip -1 -c payload > a.nii.gz
gzip -9 -c payload > b.nii.gz
head -c 65536 /dev/urandom > other
gzip -c other > c.nii.gz

if cmp -s a.nii.gz b.nii.gz; then
  echo "SKIP: this gzip encodes -1 and -9 identically, nothing to compare"
  exit 0
fi

nii_cmp a.nii.gz b.nii.gz || {
  echo "FAIL: identical content reported different"; exit 1; }
nii_cmp a.nii.gz c.nii.gz >/dev/null 2>&1 && {
  echo "FAIL: differing content reported same"; exit 1; }
cp payload d.nii && cp payload e.nii
nii_cmp d.nii e.nii || { echo "FAIL: uncompressed path broken"; exit 1; }

echo "nii_cmp selftest passed"
