#include "check.h"
#include "../fixit/fixit_tiff.h"
#include "../fixit/tiff_helper.h"
/* #define DEBUG */


ret_t check_tag_has_value(TIFF* tif, int tag, unsigned int value) {
  printf("check if tag %i (%s) has value %u\n", tag, TIFFFieldName(TIFFFieldWithTag(tif, tag)), value);
  if (NULL == tif) {
    fprintf( stderr, "TIFF pointer is empty\n");
    tif_fails("TIFF pointer is empty\n");
  };
  uint16 val;
  void * data;
  int found=TIFFGetField(tif, tag, &val, &data);
  if (1==found) { /* check va-list */
#ifdef DEBUG
    printf("### found: value=%i data=%p \n",val, data);
#endif
    /* we check only count, because we evaluate only int-values */
    if ((val == value)) {
      ret_t res;
      res.returnmsg=NULL;
      res.returncode=0;
      return res;
    } else {
      tif_fails("tag %i should have value %i, but have count/value=%u\n", tag, value, val);
    }
  } else { /* tag not defined */ 
    tif_fails("tag %i should exist, because defined\n", tag);
  }
}

ret_t check_tag_has_valuelist(TIFF* tif, int tag, int count, unsigned int * values) {
  printf("check if tag %i (%s) has these %i-values", tag, TIFFFieldName(TIFFFieldWithTag(tif, tag)), count);
  int i;
  unsigned int * p = values;
  for (i=0; i< count; i++) {
    printf (", %i", *p);
    p++;
  }
  printf("\n");
  uint16 val;
  void * data;
  int found=TIFFGetField(tif, tag, &val, &data);
  if (1==found) { /* check va-list */
#ifdef DEBUG
    printf("### found: value=%i(%x) data=%p \n",val,val, data);
#endif
    /* we check only count, because we evaluate only int-values */
    p = values;
    int has_error=0;
    for (i=0; i< count; i++) {
#ifdef DEBUG
      printf("### value = %u", *p);
#endif
      if ((val == *p)) {

      } else {
#ifdef DEBUG

        printf("error\n");
#endif
        has_error++; 
      }
      p++;
    }
    if (has_error > 0) {
      tif_fails("tag %i should have values, but have count/value=%i(%x)\n", tag, val, val);
    } else { /* no error */
      ret_t res;
      res.returnmsg=NULL;
      res.returncode=0;
      return res;
    }
  } else { /* tag not defined */ 
    tif_fails("tag %i should exist, because defined\n", tag);
  }
}


