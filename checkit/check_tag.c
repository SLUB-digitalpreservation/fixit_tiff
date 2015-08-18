#include "check.h"
/* #define DEBUG */


/* checks if TIF has a specified tag */
ret_t check_tag_quiet(TIFF* tif, tag_t tag) {
  tifp_check( tif)
    int i;
  ret_t res;
  res.returnmsg=NULL;
  for (i= 0; i < TIFFGetRawTagListCount( tif ); i++) {
    if (tag == TIFFGetRawTagListEntry( tif, i ) && tag > 253) {
      res.returncode=0;
      return res;
    };
  }
  res.returncode=1;
  return res;
}

/* checks if TIF has a specified tag */
ret_t check_tag(TIFF* tif, tag_t tag) {
  printf("check if tag %u (%s) exists\n", tag, TIFFTagName(tif, tag));
  tifp_check( tif)
    ret_t res = check_tag_quiet( tif, tag);
  if (res.returncode == 0) {
    res = check_tag_has_valid_type( tif, tag);
    if (res.returncode == 0) {
      res = check_tag_has_valid_asciivalue(tif, tag);
    }
    return res;
  } else {
    tif_fails("tag %u (%s) should exist, because defined\n", tag, TIFFTagName(tif, tag));
  }
}

/* checks if TIF does not have a specified tag,
 * needed only for checks to ensure whitelist */
ret_t check_notag(TIFF* tif, tag_t tag) {
  tifp_check( tif)
    ret_t res = check_tag_quiet( tif, tag);
  if (res.returncode == 0) {
    tif_fails("found tag %u (%s) which is not whitelisted\n", tag,  TIFFTagName(tif, tag));
  } else {
    res.returnmsg=NULL;
    res.returncode=0;
    return res;
  }
}

