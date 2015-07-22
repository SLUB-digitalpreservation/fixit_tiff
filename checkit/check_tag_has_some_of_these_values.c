#include "check.h"
#include "../fixit/fixit_tiff.h"
#include "../fixit/tiff_helper.h"
/*
#define DEBUG
*/

ret_t check_tag_has_some_of_these_values(TIFF* tif, int tag, int count, unsigned int * values) {
  printf("check if tag %i (%s) has some of these %i-values", tag, TIFFFieldName(TIFFFieldWithTag(tif, tag)), count);
  int i;
  unsigned int * p = values;
  for (i=0; i< count; i++) {
    printf (", %u", *p);
    p++;
  }
  printf("\n");
  uint16 val;
  void * data;
  int found=TIFFGetField(tif, tag, &val, &data);
  if (1==found) { /* check va-list */
#ifdef DEBUG
    printf("### found: value=%u data=%p \n",val, data);
#endif
    /* we check only count, because we evaluate only int-values */
    p = values;
    for (i=0; i< count; i++) {
      if ((val == *p)) { 
        ret_t res;
        res.returnmsg=NULL;
        res.returncode=0;
        return res;
      }
    }
    tif_fails("tag %i should have some of the values, but have count/value=%i\n", tag, val);
  } else { /* tag not defined */ 
    tif_fails("tag %i should exist, because defined\n", tag);
  }
}

