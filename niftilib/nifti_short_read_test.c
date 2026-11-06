/* A short or corrupt image must be rejected, not returned with an
   uninitialized tail. */

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include "nifti1_io.h"

static const char *WHOLE = "short_read_whole.nii";
static const char *SHORT = "short_read_short.nii";
static const char *SHORT_GZ = "short_read_short.nii.gz";
static const char *CORRUPT_GZ = "short_read_corrupt.nii.gz";
static const long MISSING = 1000;

static int write_whole(void)
{
  int dims[8] = { 3, 31, 31, 31, 1, 1, 1, 1 };
  int rv;
  nifti_image *nim = nifti_make_new_nim(dims, DT_FLOAT32, 1);
  if( nim == NULL ) return 1;
  if( nifti_set_filenames(nim, WHOLE, 0, 1) != 0 ){
    nifti_image_free(nim);
    return 1;
  }
  rv = nifti_image_write_status(nim);
  nifti_image_free(nim);
  return rv != 0;
}

static char *read_whole(long *size)
{
  FILE *in = fopen(WHOLE, "rb");
  char *buf = NULL;

  if( in == NULL ) return NULL;
  if( fseek(in, 0, SEEK_END) != 0 ||
      (*size = ftell(in)) <= MISSING ||
      fseek(in, 0, SEEK_SET) != 0 ){
    fclose(in);
    return NULL;
  }
  buf = (char *)malloc((size_t)*size);
  if( buf != NULL && fread(buf, 1, (size_t)*size, in) != (size_t)*size ){
    free(buf);
    buf = NULL;
  }
  fclose(in);
  return buf;
}

static int write_bytes(const char *path, const void *buf, size_t n)
{
  FILE *out = fopen(path, "wb");
  size_t put;
  if( out == NULL ) return 1;
  put = fwrite(buf, 1, n, out);
  if( fclose(out) != 0 ) return 1;
  return put != n;
}

/* Truncating a gzip stream makes gzread return a short count, not -1. */
static int write_short_gz(const char *buf, long size)
{
  gzFile gz = gzopen(SHORT_GZ, "wb");
  if( gz == NULL ) return 1;
  if( gzwrite(gz, buf, (unsigned)(size - MISSING)) != (int)(size - MISSING) ){
    gzclose(gz);
    return 1;
  }
  return gzclose(gz) != Z_OK;
}

/* A reserved block type after valid data makes gzread return -1 mid-image. */
static int write_corrupt_gz(const char *buf, long size)
{
  z_stream zs = { 0 };
  uLong cap;
  unsigned char *out;
  int rv = 1;

  if( deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8,
                   Z_DEFAULT_STRATEGY) != Z_OK ) return 1;
  cap = deflateBound(&zs, (uLong)size) + 1;
  out = (unsigned char *)malloc(cap);
  if( out != NULL ){
    zs.next_in = (Bytef *)buf;
    zs.avail_in = (uInt)(size / 2);
    zs.next_out = out;
    zs.avail_out = (uInt)(cap - 1);
    if( deflate(&zs, Z_FULL_FLUSH) == Z_OK && zs.avail_in == 0 ){
      out[zs.total_out] = 0x07;   /* BFINAL=1, BTYPE=11 */
      rv = write_bytes(CORRUPT_GZ, out, (size_t)zs.total_out + 1);
    }
    free(out);
  }
  deflateEnd(&zs);
  return rv;
}

static int expect_rejected(const char *path)
{
  nifti_image *nim = nifti_image_read(path, 1);
  if( nim == NULL ) return 0;
  fprintf(stderr, "FAILURE: %s was accepted, nvox=%d\n", path,
          (int)nim->nvox);
  nifti_image_free(nim);
  return 1;
}

static int run(void)
{
  nifti_image *nim;
  char *buf;
  long size = 0;
  int bad;

  if( write_whole() ){
    fprintf(stderr, "FAILURE: could not write the reference image\n");
    return 1;
  }
  buf = read_whole(&size);
  if( buf == NULL ){
    fprintf(stderr, "FAILURE: could not read back the reference image\n");
    return 1;
  }
  bad = write_bytes(SHORT, buf, (size_t)(size - MISSING)) ||
        write_short_gz(buf, size) || write_corrupt_gz(buf, size);
  free(buf);
  if( bad ){
    fprintf(stderr, "FAILURE: could not write the damaged copies\n");
    return 1;
  }

  bad = expect_rejected(SHORT);
  bad |= expect_rejected(SHORT_GZ);
  bad |= expect_rejected(CORRUPT_GZ);
  if( bad ) return 1;

  /* the rejection must be specific to the damage */
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
  return 0;
}

int main(void)
{
  int rv = run();
  remove(WHOLE);
  remove(SHORT);
  remove(SHORT_GZ);
  remove(CORRUPT_GZ);
  if( rv == 0 ) printf("Short-read rejection test passed.\n");
  return rv;
}
