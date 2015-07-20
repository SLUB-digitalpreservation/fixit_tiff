#include "check.h"
#include "../fixit/fixit_tiff.h"
#include "../fixit/tiff_helper.h"

/* 
#define DEBUG
*/

int check_tag_has_value_in_range(TIFF* tif, int tag, unsigned int a, unsigned int b) {
  printf("check if tag %i (%s) has value in range %u - %u\n", tag, TIFFFieldName(TIFFFieldWithTag(tif, tag)), a, b);
   if (NULL == tif) {
    fprintf( stderr, "TIFF pointer is empty\n");
    tif_fails("TIFF pointer is empty\n");
    return - FIXIT_TIFF_READ_PERMISSION_ERROR;
  };
  uint16 val;
  void * data;
  
  if (a > b) { unsigned int c=a; a=b; b=c; }

  int found=TIFFGetField(tif, tag, &val, &data);
  if (1==found) { /* check va-list */
#ifdef DEBUG
    printf("### found: value=%u data=%p \n",val, data);
#endif
    /* we check only count, because we evaluate only int-values */
    if ((val >= a || val <= b ) && (NULL == data)) { 
      return 0;
    } else {
      tif_fails("tag %i should have value in range %u - %u, but have count/value=%i data=%p\n", tag, a, b, val, data);
      return 1;
    }
  } else { /* tag not defined */ 
    tif_fails("tag %i should exist, because defined\n", tag);
    return 1;
  }
  return 1;
}

