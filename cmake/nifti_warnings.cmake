#
# Compiler warning flags shared by every nifti_clib target.
#
# Every flag in the GCC/Clang and Clang-only blocks has been measured at
# zero warnings across the whole tree.  A flag is added only once the
# tree is already clean under it, so that a warning always means a new
# defect rather than more noise.
#
# Run the census before adding a flag:
#
#   cmake -G Ninja -S . -B build-census -DCMAKE_BUILD_TYPE=Release \
#         -DUSE_CIFTI_CODE=ON -DUSE_FSL_CODE=ON -DFSLSTYLE=ON \
#         -DCMAKE_C_FLAGS="<the candidate flag>"
#   ninja -C build-census -k 0 2>&1 | tee census.log
#   grep -oE '\[-W[a-z0-9-]+\]' census.log | sort | uniq -c | sort -rn
#
# Warnings are NOT errors by default; set NIFTI_WARNINGS_AS_ERRORS=ON to
# make CI fail on them.
#
# This branch also enables flags that are not yet clean, so CI measures
# them on every run.  Keep NIFTI_WARNINGS_AS_ERRORS OFF here.
#

option(NIFTI_ENABLE_WARNINGS "Enable the project's compiler warning set" ON)
option(NIFTI_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
mark_as_advanced(NIFTI_ENABLE_WARNINGS NIFTI_WARNINGS_AS_ERRORS)

if(NOT NIFTI_ENABLE_WARNINGS)
  return()
endif()

set(_nifti_warnings "")

# ---------------------------------------------------------------------
# GCC and Clang
# ---------------------------------------------------------------------
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  list(APPEND _nifti_warnings
    -Wall
    -Wpedantic
    -Wformat=2
    -Wmissing-declarations
    -Wnull-dereference
    -Wpointer-arith
    -Wredundant-decls
    -Wshadow
    -Wstrict-prototypes
    -Wswitch-enum
    -Wundef
    -Wvla
  )
endif()

# ---------------------------------------------------------------------
# Clang only
# ---------------------------------------------------------------------
if(CMAKE_C_COMPILER_ID MATCHES "Clang|AppleClang")
  list(APPEND _nifti_warnings
    -Wcomma
  )
endif()

# ---------------------------------------------------------------------
# Ready to promote: measured at zero, not yet in the clean set
# ---------------------------------------------------------------------
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  list(APPEND _nifti_warnings
    -Wcast-qual
  )
endif()

if(CMAKE_C_COMPILER_ID MATCHES "Clang|AppleClang")
  list(APPEND _nifti_warnings
    -Wnewline-eof
  )
endif()

# ---------------------------------------------------------------------
# Future set: still warns
# ---------------------------------------------------------------------
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  list(APPEND _nifti_warnings
    -Wextra
    -Wmissing-prototypes
    -Wsign-conversion
  )
endif()

if(CMAKE_C_COMPILER_ID MATCHES "Clang|AppleClang")
  list(APPEND _nifti_warnings
    -Wcast-align
    -Wconditional-uninitialized
    -Wextra-semi-stmt
    -Wmissing-variable-declarations
    -Wshorten-64-to-32
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
