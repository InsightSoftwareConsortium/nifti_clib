// Apple provides strlcpy and strlcat, so don't redefine them.
#ifndef __APPLE__

// libc 2.38 and above provides strlcpy and strlcat, so don't redefine them.
#if defined (__GLIBC__) && ((__GLIBC__ < 2) || ((__GLIBC__ == 2) && (__GLIBC_MINOR__ < 38)))

#include <stddef.h>

size_t strlcpy(char* restrict ioDestination, const char* restrict inSource, size_t inDestinationSize);

size_t strlcat(char* restrict ioDestination, const char* restrict inSource, size_t inDestinationSize);

#endif
#endif
