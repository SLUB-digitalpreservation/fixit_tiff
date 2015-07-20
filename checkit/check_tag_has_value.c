#include "check.h"
#include "../fixit/fixit_tiff.h"
#include "../fixit/tiff_helper.h"

int check_tag_has_value(TIFF* tif, int tag, int value) {
  printf("check if tag %i (%s) has value %i\n", tag, TIFFFieldName(TIFFFieldWithTag(tif, tag)), value);
  if (NULL == tif) {
    fprintf( stderr, "TIFF pointer is empty\n");
    tif_fails("TIFF pointer is empty\n");
    return - FIXIT_TIFF_READ_PERMISSION_ERROR;
  };
  uint16 val;
  void * data;
  int found=TIFFGetField(tif, tag, &val, data);
  if (1==found) { /* check va-list */
        printf("### found: value=%i data=%p \n",val, data);
        /* we check only count, because we evaluate only int-values */
        if ((val == value) && (NULL == data)) { 
                return 0;
        } else {
          tif_fails("tag %i should have value %i, but have count/value=%i data=%p\n", tag, value, val, data);
          return 1;
        }
  } else { /* tag not defined */ 
    tif_fails("tag %i should exist, because defined\n", tag);
    return 1;
  }
  return 1;
}

int check_tag_has_valuelist(TIFF* tif, int tag, int count, int * values) {
  printf("check if tag %i (%s) has these %i-values", tag, TIFFFieldName(TIFFFieldWithTag(tif, tag)), count);
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
    printf("### found: value=%i(%x) data=%p \n",val,val, data);
    /* we check only count, because we evaluate only int-values */
    p = values;
    int has_error=0;
    for (i=0; i< count; i++) {
      printf("### value = %i", *p);
      if ((val == *p) && (NULL == data)) {
      } else {
      printf("error\n");
        has_error++; 
      }
      p++;
    }
    if (has_error > 0) {
      tif_fails("tag %i should have values, but have count/value=%i(%x) data=%p\n", tag, val, val, data);
      return 1;
    } else { /* no error */
      return 0;
    }
  } else { /* tag not defined */ 
    tif_fails("tag %i should exist, because defined\n", tag);
    return 1;
  }
  return 1;
}


