#include <stdio.h>
#include <string.h>

#include "afni_xml.h"

static int is_target(afni_xml_t * ax, int depth)
{
   (void)depth;
   return ax->name && strcmp(ax->name, "target") == 0;
}

static int check(const char * label, afni_xml_t * got, afni_xml_t * want)
{
   if( got == want ) return 0;
   fprintf(stderr, "FAIL %s: got %p, want %p\n", label, (void *)got, (void *)want);
   return 1;
}

int main(void)
{
   char root_name[] = "root", mid_name[] = "mid";
   char other_name[] = "other", target_name[] = "target";
   afni_xml_t root, mid, other, target;
   afni_xml_t * root_kids[2];
   afni_xml_t * mid_kids[1];
   int nfail = 0;

   memset(&root, 0, sizeof(root));
   memset(&mid, 0, sizeof(mid));
   memset(&other, 0, sizeof(other));
   memset(&target, 0, sizeof(target));

   root.name   = root_name;
   other.name  = other_name;
   mid.name    = mid_name;
   target.name = target_name;

   root_kids[0] = &other;
   root_kids[1] = &mid;
   root.nchild  = 2;
   root.xchild  = root_kids;

   mid_kids[0] = &target;
   mid.nchild  = 1;
   mid.xchild  = mid_kids;

   nfail += check("depth limit reaches target", axml_recur_find_xml(is_target, &root, 0, 2), &target);
   nfail += check("depth limit stops short", axml_recur_find_xml(is_target, &root, 0, 1), NULL);
   nfail += check("unlimited depth", axml_recur_find_xml(is_target, &root, 0, 0), &target);
   nfail += check("leaf without match", axml_recur_find_xml(is_target, &other, 0, 0), NULL);

   if( nfail ) return 1;
   printf("axml_recur_find_xml: all checks passed\n");
   return 0;
}
