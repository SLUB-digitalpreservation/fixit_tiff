#include "check.h"
#include "../fixit/fixit_tiff.h"
/* #define DEBUG */


/* scans first IDF and returns count of tags
 * Hint: sideeffect, if succeed the seek points to beginning of the first
 * IFD-entry */
int TIFFGetRawTagListCount (TIFF * tif) {
  int fd = TIFFFileno( tif);
  /* seek the image file directory (bytes 4-7) */
  lseek(fd, (off_t) 4, SEEK_SET);
  uint32 offset;
  if (read(fd, &offset, 4) != 4)
    perror ("TIFF Header read error");
  if (TIFFIsByteSwapped(tif))
    TIFFSwabLong(&offset);
  // printf("diroffset to %i (0x%04lx)\n", offset, offset);
  //printf("byte swapped? %s\n", (TIFFIsByteSwapped(tif)?"true":"false")); 
  /* read and seek to IFD address */
  lseek(fd, (off_t) offset, SEEK_SET);
  uint16 count;
  if (read(fd, &count, 2) != 2)
    perror ("TIFF Header read error2");
  if (TIFFIsByteSwapped(tif))
    TIFFSwabShort(&count);
  return count;
}

/* scans first IDF and returns the n-th tag */
uint32 TIFFGetRawTagListEntry( TIFF  * tif, int tagidx ) {
  int count = TIFFGetRawTagListCount( tif);
  int fd = TIFFFileno( tif);
  //printf("count %i\n", count);
  /* read count of tags (2 Bytes) */
  int i;
  /* replace i/o operatrions with in-memory-operations */
  uint8 * ifdentries = NULL;
  ifdentries = malloc ( sizeof(uint8) * 12 * count);
  if (read(fd, ifdentries, 12 * count) != 12*count)
    perror ("TIFF Header read error2");
  uint8 * e = ifdentries;
  for (i = 0; i<count; i++) {
    uint8 lo = *e;
    e++;
    uint8 hi = *e;
    uint16 tagid = (hi << 8) + lo;
    e++;
    if (TIFFIsByteSwapped(tif))
      TIFFSwabShort(&tagid);
    //printf("tag idx=%i, tag=%u (0x%x) (0x%x) (0x%x)\n", i, tagid, tagid, hi, lo);
    if (i == tagidx) {
      free( ifdentries );
      return tagid;
    }
    e+=10;
  }
  /* loop each tag until end or given tag found */
  free( ifdentries );
  return 0;
}

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
    return res;
  } else {
    tif_fails("tag %u should exist, because defined\n", tag);
  }
}

/* checks if TIF does not have a specified tag,
 * needed only for checks to ensure whitelist */
ret_t check_notag(TIFF* tif, tag_t tag) {
  tifp_check( tif)
    ret_t res = check_tag_quiet( tif, tag);
  if (res.returncode == 0) {
    tif_fails("found tag %u which is not whitelisted\n", tag);
  } else {
    res.returnmsg=NULL;
    res.returncode=0;
    return res;
  }
}

