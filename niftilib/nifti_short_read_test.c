/* A .nii whose data section is short of the header's declared size must be
   rejected, not handed back with uninitialized tail bytes. */

#include <stdio.h>
#include <stdlib.h>
#include "nifti1_io.h"

static const char *WHOLE = "short_read_whole.nii";
static const char *SHORT = "short_read_short.nii";
static const long MISSING = 1000;

static int write_whole(void)
{
  int dims[8] = { 3, 31, 31, 31, 1, 1, 1, 1 };
  nifti_image *nim = nifti_make_new_nim(dims, DT_FLOAT32, 1);
  if( nim == NULL ) return 1;
  if( nifti_set_filenames(nim, WHOLE, 0, 1) != 0 ){
    nifti_image_free(nim);
    return 1;
  }
  nifti_image_write(nim);
  nifti_image_free(nim);
  return 0;
}

static int copy_truncated(void)
{
  FILE *in = fopen(WHOLE, "rb"), *out;
  long size;
  char *buf;
  size_t got;

  if( in == NULL ) return 1;
  fseek(in, 0, SEEK_END);
  size = ftell(in);
  fseek(in, 0, SEEK_SET);
  if( size <= MISSING ){ fclose(in); return 1; }

  buf = (char *)malloc((size_t)size);
  if( buf == NULL ){ fclose(in); return 1; }
  got = fread(buf, 1, (size_t)size, in);
  fclose(in);
  if( got != (size_t)size ){ free(buf); return 1; }

  out = fopen(SHORT, "wb");
  if( out == NULL ){ free(buf); return 1; }
  fwrite(buf, 1, (size_t)(size - MISSING), out);
  fclose(out);
  free(buf);
  return 0;
}

int main(void)
{
  nifti_image *nim;

  if( write_whole() ){
    fprintf(stderr, "FAILURE: could not write the reference image\n");
    return 1;
  }
  if( copy_truncated() ){
    fprintf(stderr, "FAILURE: could not write the truncated copy\n");
    return 1;
  }

  nim = nifti_image_read(SHORT, 1);
  if( nim != NULL ){
    fprintf(stderr, "FAILURE: truncated image was accepted, nvox=%d\n",
            (int)nim->nvox);
    nifti_image_free(nim);
    return 1;
  }

  /* the rejection must be specific to the short read */
  nim = nifti_image_read(WHOLE, 1);
  if( nim == NULL ){
    fprintf(stderr, "FAILURE: the whole image was rejected\n");
    return 1;
  }
  if( nim->data == NULL ){
    fprintf(stderr, "FAILURE: the whole image came back with no data\n");
    nifti_image_free(nim);
    return 1;
  }
  nifti_image_free(nim);

  printf("Short-read rejection test passed.\n");
  return 0;
}
