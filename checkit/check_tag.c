#include "check.h"
#include "../fixit/fixit_tiff.h"
#include "../fixit/tiff_helper.h"
/* #define DEBUG */

/* checks if TIF has a specified tag */
ret_t check_tag(TIFF* tif, tag_t tag) {
  printf("check if tag %u (%s) exists\n", tag, TIFFTagName(tif, tag));
  tifp_check( tif)
  uint32 tag_counter=TIFFGetRawTagListCount(tif);
  uint32 tagidx;
  for (tagidx=0; tagidx < tag_counter; tagidx++) {
    if (tag == TIFFGetRawTagListEntry( tif, tagidx )) {
        ret_t res;
        res.returnmsg=NULL;
        res.returncode=0;
        return res;
    };
  }
  tif_fails("tag %u should exist, because defined\n", tag);
  ;
}

/* checks if TIF does not have a specified tag,
 * needed only for checks to ensure whitelist */
ret_t check_notag(TIFF* tif, tag_t tag) {
  tifp_check( tif)
  uint32 tag_counter=TIFFGetRawTagListCount(tif);
  uint32 tagidx;
  for (tagidx=0; tagidx < tag_counter; tagidx++) {
    if (tag == TIFFGetRawTagListEntry( tif, tagidx )) tif_fails("found tag %u which is not whitelisted\n", tag);
  }
  ret_t res;
  res.returnmsg=NULL;
  res.returncode=0;
  return res;
}

