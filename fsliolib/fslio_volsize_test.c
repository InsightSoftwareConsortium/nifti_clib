/* FslGetVolSize() must evaluate nx*ny*nz at size_t width, not int width. */

#include <stdio.h>
#include <stdlib.h>
#include "fslio.h"

static int check(const char *what, size_t got, size_t want)
{
  if( got == want ) return 0;
  fprintf(stderr, "FAILURE: %s: got %llu, expected %llu\n", what,
          (unsigned long long)got, (unsigned long long)want);
  return 1;
}

int main(void)
{
  FSLIO fslio;
  nifti_image nim;
  int failures = 0;

  memset(&fslio, 0, sizeof(fslio));
  memset(&nim, 0, sizeof(nim));
  fslio.niftiptr = &nim;
  fslio.mincptr = NULL;

  nim.nx = 64; nim.ny = 64; nim.nz = 32;
  failures += check("64x64x32", FslGetVolSize(&fslio), (size_t)64*64*32);

  /* 2048*2048*1024 == 2^32: zero when the product is formed in int. */
  nim.nx = 2048; nim.ny = 2048; nim.nz = 1024;
  failures += check("2048x2048x1024", FslGetVolSize(&fslio),
                    (size_t)2048*2048*1024);

  /* 3000*3000*300 == 2,700,000,000: negative when formed in int. */
  nim.nx = 3000; nim.ny = 3000; nim.nz = 300;
  failures += check("3000x3000x300", FslGetVolSize(&fslio),
                    (size_t)3000*3000*300);

  if( failures ) return 1;
  printf("FslGetVolSize 64-bit arithmetic test passed.\n");
  return 0;
}
