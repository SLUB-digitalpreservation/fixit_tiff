#include "check.h"

int check_tag_has_some_of_these_values(TIFF* tif, int tag, int count, int * values) {
  /* TODO */
  printf("check if tag %i has some of these %i-values", tag, count);
  int i;
  int * p = values;
  for (i=0; i< count; i++) {
    printf (", %i", *p);
    p++;
  }
  printf("\n");
  return 0;
}

