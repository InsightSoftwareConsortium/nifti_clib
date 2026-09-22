#!/bin/sh
# Drive the two failure paths in nifti_tool's header modification, where
# the header and the duplicated name have to be released before returning.
# Both invocations are expected to fail, so a nonzero status cannot be the
# assertion: under a sanitizer it is also what a leak report exits with.
# The check is that the tool reported the failure and said nothing else.
#   usage: mod_header_errpaths.sh <nifti_tool> [nifti1_tool]
NT=$1
NT1=$2
[ -x "$NT" ] || { echo "usage: $0 <nifti_tool> [nifti1_tool]"; exit 1; }

if [ "$(id -u)" = "0" ]; then
  echo "skipping: a read-only directory does not stop root"
  exit 0
fi

tmp=$(mktemp -d) || exit 1
trap 'chmod 755 "$tmp/ro" 2>/dev/null; rm -rf "$tmp"' EXIT
cd "$tmp" || exit 1

$NT -make_im -new_dim 3 4 4 4 0 0 0 0 -prefix anat0.nii || exit 1
mkdir ro && chmod 555 ro

# A. the write fails, so both the header and the duplicated name are held
out=$($NT -mod_hdr2 -prefix ro/anat1 -infiles anat0.nii \
          -mod_field qoffset_x -17.325 2>&1)
if [ $? -eq 0 ]; then
  echo "FAIL: writing into a read-only directory succeeded"
  echo "$out"
  exit 1
fi
case "$out" in
  *Sanitizer*) echo "FAIL: sanitizer report on the write path"; echo "$out"; exit 1;;
esac

# B. the duplication fails, so the header alone is held
if [ -n "$NT1" ] && [ -x "$NT1" ]; then
  out=$($NT1 -mod_hdr -prefix ro/x1 -infiles anat0.nii \
             -mod_field qoffset_x -17.325 2>&1)
  if [ $? -eq 0 ]; then
    echo "FAIL: nifti1_tool accepted a NIFTI-2 input"
    echo "$out"
    exit 1
  fi
  case "$out" in
    *Sanitizer*) echo "FAIL: sanitizer report on the duplication path"; echo "$out"; exit 1;;
  esac
fi

echo "mod_header error paths reached"
