#!/bin/sh

if [ $# -lt 2 ]
then
echo Missing nifti tool and Binary directory name
exit 1
fi

NT=$1
DATA=$2
OUT_DATA=$(dirname ${DATA}) #Need to write to separate directory
cd ${OUT_DATA}


# Compare two NIfTI files by content rather than by compressed bytes.
#
# gzip output is not reproducible across zlib implementations: zlib-ng,
# which Arch, CachyOS and other current distributions ship as the system
# zlib, encodes the same input differently from stock zlib.  Comparing
# the .gz files directly therefore fails on those systems even though the
# image data round-trips perfectly.  Decompress first and compare that.
nii_cmp() {
    if [ "${1##*.}" = "gz" ]; then
        gzip -dc "$1" > "$1.raw" && gzip -dc "$2" > "$2.raw" || return 1
        cmp "$1.raw" "$2.raw"
        return $?
    fi
    cmp "$1" "$2"
}

# note the main input file and prefix for all output files
infile=$DATA/e4.60005.nii.gz
prefix=out.c22

rm -f $prefix*

# ------------------------------------------------------------
# tests with -copy_image and -clb, along with data conversion
# options -convert2dtype, -convert_fail_choice, -convert_verify
#
# keep simple: convert between types and test for binary match
# test with both -cbl and -copy_image


# dupe initially, to get local endian (for binary diff tests),
# and change to 4 dims, rather than 5 (time series has 5th dim)
if ${NT} -cbl -infile $infile -prefix $prefix.0.i16.nii.gz
then
echo ""
else
echo $prefix initial cbl failed
exit 1
fi

# back and forth to i64, -copy_image
${NT} -copy_image -infile ${prefix}.0.i16.nii.gz \
                  -prefix ${prefix}.1.i64.nii.gz \
                  -convert2dtype NIFTI_TYPE_INT64 -convert_verify
${NT} -copy_image -infile ${prefix}.1.i64.nii.gz   \
                  -prefix ${prefix}.2.0.i16.nii.gz \
                  -convert2dtype NIFTI_TYPE_INT16 -convert_verify
if nii_cmp ${prefix}.0.i16.nii.gz ${prefix}.2.0.i16.nii.gz
then
echo ""
else
echo "** not a binary match: ${prefix}.0.i16.nii.gz ${prefix}.2.0.i16.nii.gz"
exit 1
fi

# back and forth to f32, -cbl
${NT} -cbl -infile ${prefix}.0.i16.nii.gz \
           -prefix ${prefix}.1.f32.nii.gz \
           -convert2dtype NIFTI_TYPE_FLOAT32 -convert_fail_choice fail
${NT} -copy_image -infile ${prefix}.1.f32.nii.gz   \
                  -prefix ${prefix}.2.1.i16.nii.gz \
                  -convert2dtype NIFTI_TYPE_INT16 -convert_fail_choice warn
if nii_cmp ${prefix}.0.i16.nii.gz ${prefix}.2.1.i16.nii.gz
then
echo ""
else
echo "** not a binary match: ${prefix}.0.i16.nii.gz ${prefix}.2.1.i16.nii.gz"
fi

