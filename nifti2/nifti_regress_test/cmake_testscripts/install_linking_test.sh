#!/bin/sh
set -e

# Get the build tool executable
if [ $# -lt 1 ]
then
    echo Missing build tool, guessing that make is used
    export BUILD_TOOL=make
else
    export BUILD_TOOL=$1
fi

# Where the project source lives.  This used to be hard-coded as
# ../../nifti_clib relative to the build directory, which only resolved
# when the build tree happened to be a sibling of a source tree named
# exactly "nifti_clib" -- so the test failed for an in-tree build, for a
# build directory named anything else, and on CI.
if [ $# -lt 2 ]
then
    echo Missing source directory
    exit 1
fi
SRC_DIR=$2

# Set variables for local install
export DESTDIR=installed
export PATH="$PWD/$DESTDIR/usr/local/bin:$PATH"

# Install and execute tools to see if linkage failures have occurred
$BUILD_TOOL install
nifti_tool 1>/dev/null
nifti1_tool 1>/dev/null
nifti_stats 1>/dev/null


# Run an example of a downstream project linking against the nifti targets
rm -rf downstream_example
mkdir downstream_example
cd downstream_example
cmake \
    -G 'Unix Makefiles' \
    -DCMAKE_MODULE_PATH=../installed/usr/local/share \
    "${SRC_DIR}/real_easy/minimal_example_of_downstream_usage"
make

echo Success
