/* FslSeekVolume() reports success as 0 for compressed and uncompressed
   files alike, and FslReadVolumes() survives a zero volume size. */

#include <stdio.h>
#include <stdlib.h>
#include "fslio.h"

static int write_image(const char *fname)
{
  int dims[8] = { 4, 4, 4, 4, 3, 1, 1, 1 };
  nifti_image *nim = nifti_make_new_nim(dims, DT_INT16, 1);
  if( nim == NULL ) return 1;
  if( nifti_set_filenames(nim, fname, 0, 1) != 0 ){
    nifti_image_free(nim);
    return 1;
  }
  nifti_image_write(nim);
  nifti_image_free(nim);
  return 0;
}

static int check_seek(const char *fname)
{
  FSLIO *fslio;
  int rv;

  if( write_image(fname) ){
    fprintf(stderr, "FAILURE: could not write %s\n", fname);
    return 1;
  }
  fslio = FslOpen(fname, "rb");
  if( fslio == NULL ){
    fprintf(stderr, "FAILURE: could not open %s\n", fname);
    return 1;
  }
  rv = FslSeekVolume(fslio, 2);
  FslClose(fslio);
  free(fslio);
  if( rv != 0 ){
    fprintf(stderr, "FAILURE: FslSeekVolume on %s returned %d\n", fname, rv);
    return 1;
  }
  return 0;
}

static int check_zero_volume(const char *fname)
{
  char buf[8];
  size_t got;
  FSLIO *fslio = FslOpen(fname, "rb");

  if( fslio == NULL ){
    fprintf(stderr, "FAILURE: could not open %s\n", fname);
    return 1;
  }
  fslio->niftiptr->nx = 0;
  got = FslReadVolumes(fslio, buf, 1);
  fslio->niftiptr->data = NULL;
  FslClose(fslio);
  free(fslio);
  if( got != 0 ){
    fprintf(stderr, "FAILURE: zero-size volume read returned %zu\n", got);
    return 1;
  }
  return 0;
}

int main(void)
{
  int failures = 0;

  failures += check_seek("fslio_seek_test.nii");
  failures += check_seek("fslio_seek_test.nii.gz");
  failures += check_zero_volume("fslio_seek_test.nii");

  if( failures ) return 1;
  printf("FslSeekVolume test passed.\n");
  return 0;
}
