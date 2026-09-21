#!/bin/sh
# Collect the dynamic symbols exported by the built shared libraries.
# Linux/ELF only: it reads lib*.so with GNU nm -D.
#
#   collect_exported_symbols.sh <build-dir> <output-file>
#
# The output is sorted "<library> <symbol>" lines, so a diff against the
# committed baseline names both the library and the symbol that moved.
#
# The library name is reported with any version suffix stripped, so a
# SOVERSION bump does not rewrite every line of the baseline:
# libniftiio.so.3.0.0 is reported as libniftiio.so.
#
# Regenerate the baseline after an intended ABI change:
#   cmake -B build -DBUILD_SHARED_LIBS=ON -DUSE_CIFTI_CODE=ON -DUSE_FSL_CODE=ON
#   cmake --build build
#   sh cmake/collect_exported_symbols.sh build cmake/exported_symbols_linux.txt

set -eu

if [ $# -ne 2 ]; then
    echo "usage: $0 <build-dir> <output-file>" >&2
    exit 2
fi

build_dir=$1
output=$2

if [ ! -d "$build_dir" ]; then
    echo "$0: no such build directory: $build_dir" >&2
    exit 1
fi

# Match versioned sonames too (libfoo.so.3.0.0), and resolve symlinks so
# libfoo.so -> libfoo.so.3.0.0 is not counted twice.
find "$build_dir" -name 'lib*.so*' -exec readlink -f {} \; \
    | LC_ALL=C sort -u > "$output.libs"

if [ ! -s "$output.libs" ]; then
    rm -f "$output.libs"
    echo "$0: no shared libraries under $build_dir;" \
         "configure with -DBUILD_SHARED_LIBS=ON" >&2
    exit 1
fi

: > "$output.tmp"

# nm writes to a file, not a pipe, so set -e sees its exit status.
while IFS= read -r lib; do
    name=$(basename "$lib" | sed -E 's/\.so(\.[0-9]+)*$/.so/')
    nm -D --defined-only "$lib" > "$output.nm"
    awk -v name="$name" 'NF >= 3 && $3 != "" { print name, $3 }' \
        "$output.nm" >> "$output.tmp"
done < "$output.libs"

LC_ALL=C sort -u "$output.tmp" > "$output"
rm -f "$output.tmp" "$output.nm" "$output.libs"

echo "$0: wrote $(wc -l < "$output") symbols from" \
     "$(awk '{print $1}' "$output" | LC_ALL=C sort -u | wc -l) libraries to $output"
