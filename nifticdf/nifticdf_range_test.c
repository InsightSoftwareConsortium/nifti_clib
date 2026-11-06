/* cdfbin must reject an out-of-range "which" selector the way its ten
   siblings in nifticdf.c do: *status = -1 and *bound = the limit. */

#include <stdio.h>
#include "nifticdf.h"

static int check(int which, double expected_bound)
{
   int    status = 999;
   double p = 0.5, q = 0.5, s = 2.0, xn = 5.0, pr = 0.5, ompr = 0.5;
   double bound = -999.0;

   cdfbin(&which, &p, &q, &s, &xn, &pr, &ompr, &status, &bound);

   if( status != -1 || bound != expected_bound ) {
      fprintf(stderr, "** cdfbin(which=%d): status=%d bound=%g,"
                      " expected status=-1 bound=%g\n",
              which, status, bound, expected_bound);
      return 1;
   }
   printf("cdfbin(which=%d): status=%d bound=%g\n", which, status, bound);
   return 0;
}

int main(void)
{
   int errs = 0;

   errs += check(9, 4.0);   /* above the legal range */
   errs += check(0, 1.0);   /* below the legal range */

   /* a legal selector must still compute, not report a range error */
   {
      int    which = 1, status = 999;
      double p = 0.0, q = 0.0, s = 2.0, xn = 5.0, pr = 0.5, ompr = 0.5;
      double bound = -999.0;

      cdfbin(&which, &p, &q, &s, &xn, &pr, &ompr, &status, &bound);
      if( status != 0 ) {
         fprintf(stderr, "** cdfbin(which=1): status=%d, expected 0\n", status);
         errs++;
      } else {
         printf("cdfbin(which=1): status=0 p=%g\n", p);
      }
   }

   if( errs ) { fprintf(stderr, "** %d failure(s)\n", errs); return 1; }
   printf("nifticdf range test passed\n");
   return 0;
}
