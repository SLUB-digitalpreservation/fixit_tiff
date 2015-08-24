#include "check.h"
#include <unistd.h>


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
  uint32 offset = get_first_IFD( fd );

  if (read(fd, &offset, 4) != 4) {
    perror ("TIFF Header read error");
    exit(EXIT_FAILURE);
  }
  if (TIFFIsByteSwapped(tif))
    TIFFSwabLong(&offset);
  // printf("diroffset to %i (0x%04lx)\n", offset, offset);
  //printf("byte swapped? %s\n", (TIFFIsByteSwapped(tif)?"true":"false")); 
  /* read and seek to IFD address */
  lseek(fd, (off_t) offset, SEEK_SET);
  uint16 count;
  if (read(fd, &count, 2) != 2) {
    perror ("TIFF Header read error2");
    exit(EXIT_FAILURE);
  }
  if (TIFFIsByteSwapped(tif))
    TIFFSwabShort(&count);
  lseek(fd, 12 * count, SEEK_CUR);
  /* next 4 bytes are the new IFDn entry, should be empty */
  uint32 IFDn;
  if (read(fd, &IFDn, 4) != 4) {
    perror ("TIFF Header read error3");
    exit(EXIT_FAILURE);
  }
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

/* check if offsets are word aligned */
ret_t check_all_offsets_are_word_aligned(TIFF * tif) {
  printf("check if all offsets are word aligned\n");
  int count = TIFFGetRawTagListCount( tif);
  int tagidx;
  for (tagidx = 0; tagidx< count; tagidx++) {
    ifd_entry_t ifd_entry = TIFFGetRawTagIFDListEntry( tif, tagidx );
    if (ifd_entry.value_or_offset==is_offset) {
      uint32 offset = ifd_entry.data32offset;
      if ( 0 != (offset & 1)) {
        uint32 tag = TIFFGetRawTagListEntry( tif, tagidx);
        tif_fails("offset of tag %u (%s) points to 0x%08x and is not word-aligned\n", tag, TIFFTagName(tif, tag), offset);
      }
    }
  }
  ret_t res;
  res.returnmsg=NULL;
  res.returncode=0;
  return res;
}

/* check if offsets are used only once */
ret_t check_all_offsets_are_used_once_only(TIFF * tif) {
  printf("check if all offsets are used once only\n");
  int count = TIFFGetRawTagListCount( tif);
  int tagidx;
  uint32 offsets[ count ];
  uint32 tags[ count ];
  int count_of_offsets = 0;
  int i;
  for (tagidx = 0; tagidx< count; tagidx++) {
    ifd_entry_t ifd_entry = TIFFGetRawTagIFDListEntry( tif, tagidx );
    if (ifd_entry.value_or_offset==is_offset) {
      uint32 offset = ifd_entry.data32offset;
      uint32 tag = TIFFGetRawTagListEntry( tif, tagidx);
      for (i=0; i< count_of_offsets; i++) {
        if (offsets[ i ] == offset) {
          tif_fails("offset of tag %u (%s) points to %08x, which address is used previously by tag %u (%s)\n", tag, TIFFTagName(tif, tag), offset, tags[i], TIFFTagName(tif, tags[i]) );
        }
      }
      offsets[ ++count_of_offsets ] = offset;
      tags[ count_of_offsets ] = tag;
    }
  }
  ret_t res;
  res.returnmsg=NULL;
  res.returncode=0;
  return res;
}

