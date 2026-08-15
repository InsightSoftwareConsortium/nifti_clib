#
# Runtime hardening for optimized builds.
#
# _FORTIFY_SOURCE makes the C library bounds-check the string and memory
# functions wherever the compiler can determine the destination size, and
# abort rather than write past the end.  Level 3 (gcc 12+, clang 15+) also
# covers sizes known only at run time, the strlen(x) + n shape this library
# uses throughout its filename construction.
#
# At -O0 the compiler cannot determine object sizes and glibc warns if the
# macro is defined anyway, so it is applied only to optimized builds.
#

option(NIFTI_ENABLE_FORTIFY_SOURCE
       "Enable _FORTIFY_SOURCE buffer checks in optimized builds" ON)
mark_as_advanced(NIFTI_ENABLE_FORTIFY_SOURCE)

if(NOT NIFTI_ENABLE_FORTIFY_SOURCE)
  return()
endif()

if(NOT CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  return()
endif()

include(CheckCSourceCompiles)

# Both the compiler and the C library must support the level, so compile.
set(_nifti_fortify "")
foreach(_level 3 2)
  set(CMAKE_REQUIRED_FLAGS "-O2 -D_FORTIFY_SOURCE=${_level} -Werror")
  check_c_source_compiles(
    "#include <string.h>\n#include <stdlib.h>\nint main(void){char *p=malloc(8);if(!p)return 1;strcpy(p,\"x\");free(p);return 0;}"
    NIFTI_HAVE_FORTIFY_${_level})
  unset(CMAKE_REQUIRED_FLAGS)
  if(NIFTI_HAVE_FORTIFY_${_level})
    set(_nifti_fortify ${_level})
    break()
  endif()
endforeach()

if(NOT _nifti_fortify)
  return()
endif()

message(STATUS "Using _FORTIFY_SOURCE=${_nifti_fortify} for optimized builds")

add_compile_options(
  "$<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>:-D_FORTIFY_SOURCE=${_nifti_fortify}>")

unset(_nifti_fortify)
