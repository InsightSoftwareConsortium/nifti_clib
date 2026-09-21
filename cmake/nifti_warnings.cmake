#
# Compiler warning flags shared by every nifti_clib target.
#
# Every flag below has been measured at zero warnings across the whole
# tree.  A flag is added only once the tree is already clean under it, so
# that a warning always means a new defect rather than more noise.
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

if(NIFTI_WARNINGS_AS_ERRORS)
  if(MSVC)
    list(APPEND _nifti_warnings /WX)
  else()
    list(APPEND _nifti_warnings -Werror)
  endif()
endif()

add_compile_options(${_nifti_warnings})
unset(_nifti_warnings)
