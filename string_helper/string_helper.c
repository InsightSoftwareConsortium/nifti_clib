#include <assert.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

// Apple provides strlcpy and strlcat, so don't redefine them.
#ifndef __APPLE__

// libc 2.38 and above provides strlcpy and strlcat, so don't redefine them.
#if defined (__GLIBC__) && ((__GLIBC__ < 2) || ((__GLIBC__ == 2) && (__GLIBC_MINOR__ < 38)))

// ---------------------------------------------------------------------------------------------------------------
size_t strlcpy(char* restrict ioDestination, const char* restrict inSource, size_t inDestinationSize)
{
	assert(inSource);

	size_t sourceLength = strlen(inSource);

	if (inDestinationSize)
	{
		size_t length = (sourceLength >= inDestinationSize) ? inDestinationSize - 1 : sourceLength;
		memcpy(ioDestination, inSource, length);
		ioDestination[length] = '\0';
	}

	return sourceLength;
}


// ---------------------------------------------------------------------------------------------------------------
size_t strlcat(char* restrict ioDestination, const char* restrict inSource, size_t inDestinationSize)
{
	assert(ioDestination);
	assert(inSource);

	size_t destinationLength = strlen(ioDestination);
	size_t sourceLength = strlen(inSource);
	size_t totalLength = destinationLength + sourceLength;

	assert(destinationLength < inDestinationSize);

	ioDestination += destinationLength;
	inDestinationSize -= destinationLength;
	if (sourceLength >= inDestinationSize)
	{
		sourceLength = inDestinationSize - 1;
	}
	memcpy(ioDestination, inSource, sourceLength);
	ioDestination[sourceLength] = '\0';

	return totalLength;
}

#endif
#endif


