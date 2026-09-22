/* Exercises znzprintf(), which is compiled only under
   COMPILE_NIFTIUNUSED_CODE and has no caller inside this tree. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "znzlib.h"

#define BIG_LEN 2000000

static const char expected[] = "abc\n";

int main(int argc, char *argv[])
{
  const char *path;
  znzFile     zf;
  char       *big;
  int         normal;
  int         truncated;
  int         status = 0;
  size_t      got;
  char        readback[64];

  if( argc < 2 ){ fprintf(stderr,"usage: %s OUTFILE.gz\n", argv[0]); return 1; }
  path = argv[1];

  zf = znzopen(path, "wb", 1);
  if( zf == NULL ){ fprintf(stderr,"** cannot open %s\n", path); return 1; }

  normal = znzprintf(zf, "%s", expected);
  if( normal != (int)strlen(expected) ){
    fprintf(stderr,"** FAIL: ordinary write returned %d, expected %d\n",
            normal, (int)strlen(expected));
    status = 1;
  }

  big = (char *)malloc(BIG_LEN + 1);
  if( big == NULL ){ fprintf(stderr,"** cannot allocate\n"); znzclose(zf); return 1; }
  memset(big, 'x', BIG_LEN);
  big[BIG_LEN] = '\0';

  truncated = znzprintf(zf, "%s", big);
  if( truncated >= 0 ){
    fprintf(stderr,"** FAIL: truncating write returned %d, expected a "
                   "negative value\n", truncated);
    status = 1;
  }
  free(big);

  if( znzclose(zf) != 0 ){ fprintf(stderr,"** cannot close %s\n", path); return 1; }

  zf = znzopen(path, "rb", 1);
  if( zf == NULL ){ fprintf(stderr,"** cannot reopen %s\n", path); return 1; }
  memset(readback, 0, sizeof(readback));
  got = znzread(readback, 1, sizeof(readback) - 1, zf);
  znzclose(zf);

  if( got != strlen(expected) || strcmp(readback, expected) != 0 ){
    fprintf(stderr,"** FAIL: file holds %zu bytes, expected only the %d "
                   "bytes of the ordinary write\n", got, (int)strlen(expected));
    status = 1;
  }

  if( status == 0 ) printf("znzprintf test passed\n");
  return status;
}
