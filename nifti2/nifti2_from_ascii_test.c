/* sscanf matches nothing here; run under memcheck to catch an uninitialized read */
#include <stdio.h>
#include <nifti2_io.h>

int main(void)
{
   int bytes_read = 0;
   int errors = 0;
   nifti_image * nim = nifti_image_from_ascii(" \n\t ", &bytes_read);

   if( nim != NULL ) {
      printf("ERROR: nifti_image_from_ascii accepted whitespace-only input\n");
      nifti_image_free(nim);
      errors++;
   }

   printf("TOTAL ERRORS=%d\n", errors);
   return errors;
}
