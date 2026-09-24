#
# Compiler warning flags shared by every nifti_clib target.
#
# The set below is the CLEAN SET: every flag here has been measured at
# zero warnings across the whole tree.  A flag is added only once the
# tree is already clean under it, so that a warning always means a new
# defect rather than more noise.  The FUTURE SET at the bottom of this
# file records the flags that are wanted but not yet earned, each with
# its measured hit count.
#
# Run the census before promoting anything out of the future set:
#
#   cmake -G Ninja -S . -B build-census -DCMAKE_BUILD_TYPE=Release \
#         -DUSE_CIFTI_CODE=ON -DUSE_FSL_CODE=ON -DFSLSTYLE=ON \
#         -DCMAKE_C_FLAGS="<the candidate flag>"
#   ninja -C build-census -k 0 2>&1 | tee census.log
#   grep -oE '\[-W[a-z0-9-]+\]' census.log | sort | uniq -c | sort -rn
#
# Warnings are NOT errors by default; set NIFTI_WARNINGS_AS_ERRORS=ON to
# make CI fail on them.  That option is only safe to turn on in CI once
# the future set below is empty.
#

option(NIFTI_ENABLE_WARNINGS "Enable the project's compiler warning set" ON)
option(NIFTI_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
mark_as_advanced(NIFTI_ENABLE_WARNINGS NIFTI_WARNINGS_AS_ERRORS)

if(NOT NIFTI_ENABLE_WARNINGS)
  return()
endif()

set(_nifti_warnings "")

# ---------------------------------------------------------------------
# CLEAN SET - measured at zero, GCC and Clang
# ---------------------------------------------------------------------
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  list(APPEND _nifti_warnings
    -Wall
    -Wpedantic
    -Wformat=2            # printf/scanf checking, including nonliteral
    -Wmissing-declarations
    -Wnull-dereference
    -Wpointer-arith       # arithmetic on void * or function pointers
    -Wredundant-decls
    -Wshadow
    -Wstrict-prototypes
    -Wswitch-enum
    -Wundef               # #if on an undefined identifier
    -Wvla                 # variable length arrays
    -Wwrite-strings       # string literals are const
  )
endif()

# ---------------------------------------------------------------------
# CLEAN SET - Clang only
# ---------------------------------------------------------------------
if(CMAKE_C_COMPILER_ID MATCHES "Clang|AppleClang")
  list(APPEND _nifti_warnings
    -Wcomma
  )
endif()

if(NIFTI_WARNINGS_AS_ERRORS)
  if(MSVC)
    list(APPEND _nifti_warnings /WX)
  else()
    list(APPEND _nifti_warnings -Werror)
  endif()
endif()

add_compile_options(${_nifti_warnings})
unset(_nifti_warnings)

# =====================================================================
# FUTURE SET - wanted, not yet earned
# =====================================================================
#
# Counts measured 2026-09-21 at d773c59, AppleClang 21.0.0, Release,
# USE_CIFTI_CODE=ON USE_FSL_CODE=ON FSLSTYLE=ON, NIFTI_BUILD_TESTING=OFF.
# Promote a flag to the clean set above only in a PR that follows the
# PR fixing its warnings, so CI is green at every step.
#
#   flag                            hits   fix
#   ------------------------------------------------------------------
#   -Wsign-compare (via -Wextra)       3   PR #51
#   -Wcast-qual                        9   PR #45
#   -Wmissing-prototypes              18   PR #37
#   -Wsign-conversion                200   PR #53 / #52, split by dir:
#                                          znzlib 4, cifti 14,
#                                          fsliolib 45, nifti2 58,
#                                          niftilib 79
#
#   Clang-only:
#   -Wnewline-eof                      1   PR #35 covers one file only
#   -Wmissing-variable-declarations    2
#   -Wconditional-uninitialized       11   relates to PR #47 / #48
#   -Wcast-align                      19   relates to PR #44
#   -Wshorten-64-to-32                31   relates to PR #52
#   -Wextra-semi-stmt                 74   relates to PR #49
#
# -Wextra is held back only because it implies -Wsign-compare; once
# PR #51 lands it moves to the clean set with its 3 hits resolved.
#
# NOT MEASURED, do not add without a census first:
#
#   GCC-only    -Wcast-align=strict, -Wduplicated-branches,
#               -Wduplicated-cond, -Wjump-misses-init, -Wlogical-op,
#               -Wold-style-definition
#               GNU 13.3.0 runs in CI (ubuntu-latest) but these flags
#               have never been enabled, so their counts are unknown.
#
#   MSVC        /W3
#               There is no Windows job in any workflow, so nothing
#               compiles this branch at all.  Adding /W3 without a
#               Windows CI job asserts a cleanliness nobody can check.
#
# DELIBERATELY EXCLUDED, with reasons:
#
#   -Wdouble-promotion   ~240 hits.  Silencing them means calling sqrtf()
#                        instead of sqrt(), which changes the numerical
#                        results of the quaternion and matrix code.
#   -Wfloat-equal        ~150 hits.  Most are deliberate tests against an
#                        exact 0.0 or a sentinel, which is correct here.
#   -Wconversion         Largely subsumed by -Wsign-conversion, and the
#                        remainder overlaps -Wdouble-promotion above.
#   -Wunsafe-buffer-usage  clang-only, aimed at C++ span/array types that
#                        do not exist in this codebase.
#
