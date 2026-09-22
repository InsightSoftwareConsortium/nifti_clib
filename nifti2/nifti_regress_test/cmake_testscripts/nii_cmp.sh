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
