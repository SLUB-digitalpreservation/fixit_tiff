/* fixes broken TIFF Files
 * 
 * fixes unused tags in Baseline-TIFFs,
 * based on http://www.awaresystems.be/imaging/tiff/tifftags/baseline.html
 *
 * author: Andreas Romeyke, 2013
 * licensed under conditions of libtiff 
 */


#include "fixit_tiff.h"
#include "tiff_helper.h"
#include <tiffio.h>

/* scans first IDF and returns count of tags */
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

  for (i = 0; i<count; i++) {
    uint16 tagid;
    if (read(fd, &tagid, 2) != 2)
      perror ("TIFF IFD read error");
    if (TIFFIsByteSwapped(tif))
      TIFFSwabShort(&tagid);
    if (i == tagidx) return tagid;
    //printf("tag idx=%i, tag=%i (0x%lx)\n", i, tagid, tagid);
    lseek(fd, (off_t) 10, SEEK_CUR);
  }
  /* loop each tag until end or given tag found */
  return -1;
}

void print_baseline_tags (TIFF * tif) {
  int i;
  int tag_counter=TIFFGetRawTagListCount(tif);
  int tagidx;
  uint32 tags[tag_counter];
  printf ("tag count=%i, [*] means: tag is a baseline tag\n", tag_counter);
  for (tagidx=0; tagidx < tag_counter; tagidx++) {
    tags[tagidx] = TIFFGetRawTagListEntry( tif, tagidx );
  } 
  for (tagidx=0; tagidx < tag_counter; tagidx++) {
    int found = 0;
    for (i=0; i<count_of_baselinetags; i++) {
      /* printf ("tags[%i]=%i\n",tagidx, tags[tagidx]); */
      if (tags[tagidx] == baselinetags[i]) {
        found=1;
        break;
      }
    }
    printf ("\ttag %5i (0x%4x) %s\n", tags[tagidx], tags[tagidx], (found==1?"[*]":""));
  }
}

void print_required_tags (TIFF * tif) {
  int i;
  int tag_counter=TIFFGetRawTagListCount(tif);
  int tagidx;
  uint32 tags[tag_counter];
  printf ("[*] means: tag already exists\n");
  for (tagidx=0; tagidx < tag_counter; tagidx++) {
    tags[tagidx] = TIFFGetRawTagListEntry( tif, tagidx );
  } 
  for (i=0; i<count_of_required_baselinetags; i++) {
    int found = 0;
    for (tagidx=0; tagidx < tag_counter; tagidx++) {
      // printf ("tags[%i]=%i\n",tagidx, tags[tagidx]);
      if (tags[tagidx] == required_baselinetags[i]) {
        found=1;
        break;
      }
    }
    printf ("###\ttag %5i (0x%4x) %s\n", required_baselinetags[i], required_baselinetags[i], (found==1?"[*]":""));
  }
 }


