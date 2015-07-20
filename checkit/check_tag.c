#include "check.h"
#include "../fixit/fixit_tiff.h"
#include "../fixit/tiff_helper.h"
#define DEBUG

/* checks if TIF has a specified tag */
int check_tag(TIFF* tif, int tag) {
  printf("check if tag %i (%s) exists\n", tag, TIFFFieldName(TIFFFieldWithTag(tif, tag)));
  if (NULL == tif) {
    fprintf( stderr, "TIFF pointer is empty\n");
    tif_fails("TIFF pointer is empty\n");
    return - FIXIT_TIFF_READ_PERMISSION_ERROR;
  };
  uint32 tag_counter=TIFFGetRawTagListCount(tif);
  uint32 tagidx;
  for (tagidx=0; tagidx < tag_counter; tagidx++) {
    if (tag == TIFFGetRawTagListEntry( tif, tagidx )) return 0;
  }
  tif_fails("tag %i should exist, because defined\n", tag);
  return 1;
}

/* checks if TIF does not have a specified tag,
 * needed only for checks to ensure whitelist */
int check_notag(TIFF* tif, int tag) {
  if (NULL == tif) {
    fprintf( stderr, "TIFF pointer is empty\n");
    tif_fails("TIFF pointer is empty\n");
    return - FIXIT_TIFF_READ_PERMISSION_ERROR;
  };
  uint32 tag_counter=TIFFGetRawTagListCount(tif);
  uint32 tagidx;
  for (tagidx=0; tagidx < tag_counter; tagidx++) {
    if (tag == TIFFGetRawTagListEntry( tif, tagidx )) return 1;
  }
  return 0;
}

