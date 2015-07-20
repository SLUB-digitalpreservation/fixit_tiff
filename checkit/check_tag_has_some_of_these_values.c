#include "check.h"
#include "../fixit/fixit_tiff.h"
#include "../fixit/tiff_helper.h"


int check_tag_has_some_of_these_values(TIFF* tif, int tag, int count, int * values) {
  printf("check if tag %i (%s) has some of these %i-values", tag, TIFFFieldName(TIFFFieldWithTag(tif, tag)), count);
  int i;
  int * p = values;
  for (i=0; i< count; i++) {
    printf (", %i", *p);
    p++;
  }
  printf("\n");
  uint16 val;
  void * data;
  int found=TIFFGetField(tif, tag, &val, data);
  if (1==found) { /* check va-list */
    printf("### found: value=%i data=%p \n",val, data);
    /* we check only count, because we evaluate only int-values */
    p = values;
    for (i=0; i< count; i++) {
      if ((val == *p) && (NULL == data)) { 
        return 0;
      }
    }
    tif_fails("tag %i should have some of the values, but have count/value=%i data=%p\n", tag, val, data);
    return 1;
  } else { /* tag not defined */ 
    tif_fails("tag %i should exist, because defined\n", tag);
    return 1;
  }
  return 1;
}

