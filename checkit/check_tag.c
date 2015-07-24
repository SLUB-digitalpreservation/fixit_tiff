#include "check.h"
#include "../fixit/fixit_tiff.h"
/* #define DEBUG */

/* checks if TIF has a specified tag */
ret_t check_tag(TIFF* tif, tag_t tag) {
  printf("check if tag %u (%s) exists\n", tag, TIFFTagName(tif, tag));
  tifp_check( tif)
    void * c;
    if (1 == TIFFGetField(tif, tag, &c)) {
        ret_t res;
        res.returnmsg=NULL;
        res.returncode=0;
        return res;
    };
  tif_fails("tag %u should exist, because defined\n", tag);
  ;
}

/* checks if TIF does not have a specified tag,
 * needed only for checks to ensure whitelist */
ret_t check_notag(TIFF* tif, tag_t tag) {
  tifp_check( tif)
  if (NULL != TIFFFindField(tif, tag, TIFF_ANY)) { /* finds all possible fields */
    /* now check if field exists in tif */
    void * c;
    if (1 == TIFFGetField(tif, tag, &c)) {
        tif_fails("found tag %u which is not whitelisted\n", tag);
    }
  }
  ret_t res;
  res.returnmsg=NULL;
  res.returncode=0;
  return res;
}

