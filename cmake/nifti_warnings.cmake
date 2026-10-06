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
