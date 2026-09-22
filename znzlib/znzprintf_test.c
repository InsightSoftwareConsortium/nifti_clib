/* Exercises znzprintf(), which is compiled only under
   COMPILE_NIFTIUNUSED_CODE and has no caller inside this tree. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "znzlib.h"

/* must match the strlen(format) + 1000000 buffer in znzprintf, znzlib.c */
#define ZNZPRINTF_BOUND 1000000
/* past zlib's 8192 byte gzprintf buffer, inside ZNZPRINTF_BOUND */
#define MID_LEN 100000
#define BIG_LEN (2 * ZNZPRINTF_BOUND)

static const char expected[] = "abc\n";

static char *filled(size_t len)
{
  char *s = (char *)malloc(len + 1);
  if( s != NULL ){ memset(s, 'x', len); s[len] = '\0'; }
  return s;
}

int main(int argc, char *argv[])
{
  const char *path;
  znzFile     zf;
  char       *mid;
  char       *big;
  char       *readback;
  int         ret;
  int         status = 0;
  size_t      want = sizeof(expected) - 1 + MID_LEN;
  size_t      got;

  if( argc < 2 ){ fprintf(stderr,"usage: %s OUTFILE.gz\n", argv[0]); return 1; }
  path = argv[1];

  ret = znzprintf(NULL, "x");
  if( ret >= 0 ){
    fprintf(stderr,"** FAIL: NULL stream returned %d, expected a negative value\n", ret);
    status = 1;
  }

  mid = filled(MID_LEN);
  big = filled(BIG_LEN);
  readback = (char *)calloc(1, want + 2);
  if( mid == NULL || big == NULL || readback == NULL ){
    fprintf(stderr,"** cannot allocate\n");
    free(mid); free(big); free(readback);
    return 1;
  }

  zf = znzopen(path, "wb", 1);
  if( zf == NULL ){
    fprintf(stderr,"** cannot open %s\n", path);
    free(mid); free(big); free(readback);
    return 1;
  }

  ret = znzprintf(zf, "%s", expected);
  if( ret != (int)strlen(expected) ){
    fprintf(stderr,"** FAIL: ordinary write returned %d, expected %d\n",
            ret, (int)strlen(expected));
    status = 1;
  }

  ret = znzprintf(zf, "%s", mid);
  if( ret != MID_LEN ){
    fprintf(stderr,"** FAIL: %d byte write returned %d, expected %d\n",
            MID_LEN, ret, MID_LEN);
    status = 1;
  }

  ret = znzprintf(zf, "%s", big);
  if( ret >= 0 ){
    fprintf(stderr,"** FAIL: truncating write returned %d, expected a "
                   "negative value\n", ret);
    status = 1;
  }

  if( znzclose(zf) != 0 ){
    fprintf(stderr,"** cannot close %s\n", path);
    free(mid); free(big); free(readback);
    return 1;
  }

  zf = znzopen(path, "rb", 1);
  if( zf == NULL ){
    fprintf(stderr,"** cannot reopen %s\n", path);
    free(mid); free(big); free(readback);
    return 1;
  }
  got = znzread(readback, 1, want + 1, zf);
  znzclose(zf);

  if( got != want || strncmp(readback, expected, strlen(expected)) != 0
      || strcmp(readback + strlen(expected), mid) != 0 ){
    fprintf(stderr,"** FAIL: file holds %zu bytes, expected only the %zu "
                   "bytes of the two complete writes\n", got, want);
    status = 1;
  }

  free(mid); free(big); free(readback);
  if( status == 0 ) printf("znzprintf test passed\n");
  return status;
}
