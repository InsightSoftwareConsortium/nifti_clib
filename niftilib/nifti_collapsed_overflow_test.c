/* A collapsed read whose byte count does not fit in the int that
   nifti_read_collapsed_image() returns must be refused. */

#include <stdio.h>
#include <stdlib.h>
#include "nifti1_io.h"

int main(void)
{
  /* 16384 x 16384 doubles is 2^31 bytes, one past INT_MAX */
  int dims[8]  = { 3, 16384, 16384, 16, 1, 1, 1, 1 };
  int cdims[8] = { 0, -1, -1, 0, -1, -1, -1, -1 };
  void *data = NULL;
  int rv;
  nifti_image *nim = nifti_make_new_nim(dims, DT_FLOAT64, 0);

  if( nim == NULL ){
    fprintf(stderr, "FAILURE: could not create image\n");
    return 1;
  }
  if( nifti_set_filenames(nim, "collapsed_overflow.nii", 0, 1) != 0 ){
    fprintf(stderr, "FAILURE: could not set filenames\n");
    nifti_image_free(nim);
    return 1;
  }

  rv = nifti_read_collapsed_image(nim, cdims, &data);
  nifti_image_free(nim);
  if( rv >= 0 || data != NULL ){
    fprintf(stderr, "FAILURE: oversized collapsed read returned %d\n", rv);
    free(data);
    return 1;
  }

  printf("collapsed overflow test passed.\n");
  return 0;
}
