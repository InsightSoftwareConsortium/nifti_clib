/* With REJECT_COMPLEX, nifti_image_read() must report a complex image to
   its caller and keep reading non-complex images. */

#include <stdio.h>
#include <stdlib.h>
#include "nifti1_io.h"

static int write_image(const char *fname, int datatype)
{
  int dims[8] = { 3, 4, 4, 4, 1, 1, 1, 1 };
  nifti_image *nim = nifti_make_new_nim(dims, datatype, 1);
  if( nim == NULL ){
    fprintf(stderr, "FAILURE: could not create image for %s\n", fname);
    return 1;
  }
  if( nifti_set_filenames(nim, fname, 0, 1) != 0 ){
    fprintf(stderr, "FAILURE: could not set filenames for %s\n", fname);
    nifti_image_free(nim);
    return 1;
  }
  nifti_image_write(nim);
  nifti_image_free(nim);
  return 0;
}

int main(void)
{
  const char *cplx = "reject_complex_cplx.nii";
  const char *real = "reject_complex_real.nii";
  nifti_image *nim;

  if( write_image(cplx, DT_COMPLEX64) ) return 1;
  if( write_image(real, DT_FLOAT32) ) return 1;

  nim = nifti_image_read(cplx, 1);
  if( nim != NULL ){
    fprintf(stderr, "FAILURE: complex image was accepted\n");
    nifti_image_free(nim);
    return 1;
  }

  /* the rejection must be specific, not a blanket refusal */
  nim = nifti_image_read(real, 1);
  if( nim == NULL ){
    fprintf(stderr, "FAILURE: non-complex image was rejected\n");
    return 1;
  }
  nifti_image_free(nim);

  printf("REJECT_COMPLEX test passed.\n");
  return 0;
}
