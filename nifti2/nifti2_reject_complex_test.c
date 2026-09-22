/* With REJECT_COMPLEX, nifti_image_read() must report a complex image to
   its caller, in either header format, and keep reading other images. */

#include <stdio.h>
#include <stdlib.h>
#include "nifti2_io.h"

static int write_image(const char *fname, int datatype, int ftype)
{
  int64_t dims[8] = { 3, 4, 4, 4, 1, 1, 1, 1 };
  nifti_image *nim = nifti_make_new_nim(dims, datatype, 1);
  if( nim == NULL ){
    fprintf(stderr, "FAILURE: could not create image for %s\n", fname);
    return 1;
  }
  nim->nifti_type = ftype;
  if( nifti_set_filenames(nim, fname, 0, 1) != 0 ){
    fprintf(stderr, "FAILURE: could not set filenames for %s\n", fname);
    nifti_image_free(nim);
    return 1;
  }
  nifti_image_write(nim);
  nifti_image_free(nim);
  return 0;
}

static int expect(const char *fname, int accepted)
{
  nifti_image *nim = nifti_image_read(fname, 1);
  if( (nim != NULL) != accepted ){
    fprintf(stderr, "FAILURE: %s was %s\n", fname,
            nim ? "accepted" : "rejected");
    nifti_image_free(nim);
    return 1;
  }
  nifti_image_free(nim);
  return 0;
}

int main(void)
{
  int failures = 0;

  if( write_image("reject_complex2_n1.nii", DT_COMPLEX64,
                  NIFTI_FTYPE_NIFTI1_1) ) return 1;
  if( write_image("reject_complex2_n2.nii", DT_COMPLEX128,
                  NIFTI_FTYPE_NIFTI2_1) ) return 1;
  if( write_image("reject_complex2_real.nii", DT_FLOAT32,
                  NIFTI_FTYPE_NIFTI2_1) ) return 1;

  failures += expect("reject_complex2_n1.nii", 0);
  failures += expect("reject_complex2_n2.nii", 0);
  failures += expect("reject_complex2_real.nii", 1);

  if( failures ) return 1;
  printf("REJECT_COMPLEX test passed.\n");
  return 0;
}
