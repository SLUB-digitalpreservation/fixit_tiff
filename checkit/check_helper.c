#include <math.h>
#include "check.h"
#include <unistd.h>


//------------------------------------------------------------------------------
ret_t check_tag_has_fvalue(TIFF*  tif, tag_t tag, float value)
{
  float val;
  int found=TIFFGetField(tif, tag, &val);
  if (1 == found) {
    if ( fabs(val - value) < 0.01 ) {
      ret_t res;
      res.returnmsg=NULL;
      res.returncode=0;
      return res;
    } else {
      tif_returns("tag %u should have value %f, but have count/value=%f\n", tag, value, val);
    }

  } else {
    tif_returns("tag %u should exist, because defined\n", tag);
  }
}

//------------------------------------------------------------------------------
ret_t check_tag_has_u16value(TIFF*  tif, tag_t tag, uint16 value)
{
  uint16 val;
  int found=TIFFGetField(tif, tag, &val);
  if (1 == found) {
    if ( val == value ) {  
      ret_t res;
      res.returnmsg=NULL;
      res.returncode=0;
      return res;
    } else {
      tif_returns("tag %u should have value %u, but have count/value=%u\n", tag, value, val);
    }

  } else {
    tif_returns("tag %u should exist, because defined\n", tag);
  }
}

//------------------------------------------------------------------------------
ret_t check_tag_has_u32value(TIFF*  tif, tag_t tag, uint32 value)
{
  uint32 val;
  int found=TIFFGetField(tif, tag, &val);
  if (1 == found) {
    if ( val == value )  {
      ret_t res;
      res.returnmsg=NULL;
      res.returncode=0;
      return res;
    } else {
      tif_returns("tag %u should have value %u, but have count/value=%u\n", tag, value, val);
    }

  } else {
    tif_returns("tag %u should exist, because defined\n", tag);
  }
}

const char * TIFFTagName( TIFF * tif, tag_t tag ) {
   const TIFFField* fieldp = TIFFFieldWithTag(tif, tag);
   if (NULL != fieldp) {
        return TIFFFieldName(fieldp);
   } else { return ("undefined tag"); }
}

ret_t check_has_only_one_ifd(TIFF* tif) {
  printf("check if only one IFD exists\n");
  /* next commented lines, because TIFFNumberOfDirectories are in endless loop,
   * if the TIFF file from https://github.com/EasyinnovaSL/DPFManager/blob/develop/src/test/resources/IFD%20struct/Circular%20E.tif
   * is read:
     if (1 == TIFFNumberOfDirectories( tif )) {
     ret_t res;
     res.returnmsg=NULL;
     res.returncode=0;
     return res;
     }*/
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
  lseek(fd, 12 * count, SEEK_CUR);
  /* next 4 bytes are the new IFDn entry, should be empty */
  uint32 IFDn;
  if (read(fd, &IFDn, 4) != 4)
    perror ("TIFF Header read error3");
  if (TIFFIsByteSwapped(tif))
    TIFFSwabLong(&IFDn);
  if (0 == IFDn) {
    ret_t res;
    res.returnmsg=NULL;
    res.returncode=0;
    return res;
  } else {
    tif_fails("baseline TIFF should have only one IFD, but IFD0 at 0x%08x has pointer to IFDn 0x%08x\n", offset, IFDn );
  }
}


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
    if (i == tagidx) {
      // printf("tag idx=%i, tag=%u (0x%04x) (0x%02x) (0x%02x)\n", i, tagid, tagid, hi, lo);
      free( ifdentries );
      return tagid;
    }
    e+=10;
  }
  /* loop each tag until end or given tag found */
  free( ifdentries );
  return 0;
}



/* scans first IDF and returns the type of the n-th tag */
uint32 TIFFGetRawTagTypeListEntry( TIFF  * tif, int tagidx ) {
  int count = TIFFGetRawTagListCount( tif);
#ifdef DEBUG
  printf(" count of tags = %i\n", count);
#endif
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
    if (i == tagidx) {
      // tag type check
      lo = *e; e++;
      hi = *e; e++;
      uint16 tagtype = (hi << 8) + lo;
      if (TIFFIsByteSwapped(tif))
        TIFFSwabShort(&tagtype);
#ifdef DEBUG
      printf("tag idx=%i, tag=%u (0x%04x) (0x%02x) (0x%02x)   tagtype=0x%04x\n", i, tagid, tagid, hi, lo, tagtype);
#endif
      free( ifdentries );
      return tagtype;
    }
    e+=10;
  }
  /* loop each tag until end or given tag found */
  free( ifdentries );
  return 0;
}


/* reads the datatype of given tag on specified TIFF,
 * because FieldType of libtiff does not return the true value (because it maps
 * its own datastructure), we need to use this function instead
 * @param tif pointer to TIFF structure
 * @param tag tag
 * @return datatype of given tag
 * if tag does not exists the function aborts with an error 
 */
TIFFDataType TIFFGetRawTagType(TIFF * tif, tag_t tag) {
  int tagidx = -1;
  int i;
  for (i= 0; i < TIFFGetRawTagListCount( tif ); i++) {
    if (tag == TIFFGetRawTagListEntry( tif, i ) && tag > 253) {
      tagidx= i;
      break;
    };
  };
  if (tagidx >= 0) {
    TIFFDataType datatype =  TIFFGetRawTagTypeListEntry( tif, tagidx );
#ifdef DEBUG
    printf("### found: value=%i data=%p \n",val, data);
    printf("### datatype=%i \n", datatype);
#endif
    return datatype;
  } else { /* tag not defined */ 
    fprintf(stderr, "tag %u (%s) should exist, because defined\n", tag, TIFFTagName(tif, tag));
    exit(EXIT_FAILURE);
  }
}


ret_t check_tagorder(TIFF* tif) {
  printf("check if tags are in ascending order\n");
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
  uint16 lasttag = 0;
  for (i = 0; i<count; i++) {
    uint8 lo = *e;
    e++;
    uint8 hi = *e;
    uint16 tag = (hi << 8) + lo;
    e++;
    if (TIFFIsByteSwapped(tif))
      TIFFSwabShort(&tag);
    if (i>0 && lasttag >= tag) {
      // printf("tag idx=%i, tag=%u (0x%04x) (0x%02x) (0x%02x)\n", i, tag, tag, hi, lo);
      free( ifdentries );
      tif_fails("Invalid TIFF directory; tags are not sorted in ascending order, previous tag:%u (%s) , actual tag:%u (%s)\n", lasttag,  TIFFTagName(tif, lasttag),  tag,  TIFFTagName(tif, tag));
    }
    lasttag = tag;
    e+=10;
  }
  /* loop each tag until end or given tag found */
  free( ifdentries );
  ret_t res;
  res.returnmsg=NULL;
  res.returncode=0;
  return res;
}

