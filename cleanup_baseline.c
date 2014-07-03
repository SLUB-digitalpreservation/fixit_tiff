/* fixes broken TIFF Files
 * 
 * fixes invalid DateTime-field in Baseline-TIFFs,
 * based on http://www.awaresystems.be/imaging/tiff/tifftags/baseline.html
 *
 * author: Andreas Romeyke, 2013
 * licensed under conditions of libtiff 
 */


#include "fixit_tiff.h"
#include <tiffio.h>

#define count_of_baselinetags 36
const static uint32 baselinetags[count_of_baselinetags]={
  TIFFTAG_SUBFILETYPE,
  TIFFTAG_OSUBFILETYPE,
  TIFFTAG_IMAGEWIDTH,
  TIFFTAG_IMAGELENGTH,
  TIFFTAG_BITSPERSAMPLE,
  TIFFTAG_COMPRESSION,
  TIFFTAG_PHOTOMETRIC,
  TIFFTAG_THRESHHOLDING,
  TIFFTAG_CELLWIDTH,
  TIFFTAG_CELLLENGTH,
  TIFFTAG_FILLORDER,
  TIFFTAG_IMAGEDESCRIPTION,
  TIFFTAG_MAKE,
  TIFFTAG_MODEL,
  TIFFTAG_STRIPOFFSETS,
  TIFFTAG_ORIENTATION,
  TIFFTAG_SAMPLESPERPIXEL,
  TIFFTAG_ROWSPERSTRIP,
  TIFFTAG_STRIPBYTECOUNTS,
  TIFFTAG_MINSAMPLEVALUE,
  TIFFTAG_MAXSAMPLEVALUE,
  TIFFTAG_XRESOLUTION,
  TIFFTAG_YRESOLUTION,
  TIFFTAG_PLANARCONFIG,
  TIFFTAG_FREEOFFSETS,
  TIFFTAG_FREEBYTECOUNTS,
  TIFFTAG_GRAYRESPONSEUNIT,
  TIFFTAG_GRAYRESPONSECURVE,
  TIFFTAG_RESOLUTIONUNIT,
  TIFFTAG_SOFTWARE,
  TIFFTAG_DATETIME,
  TIFFTAG_ARTIST,
  TIFFTAG_HOSTCOMPUTER,
  TIFFTAG_COLORMAP,
  TIFFTAG_EXTRASAMPLES,
  TIFFTAG_COPYRIGHT
};

#define count_of_required_baselinetags 12
const static uint32 required_baselinetags[count_of_required_baselinetags]={
  TIFFTAG_IMAGEWIDTH,
  TIFFTAG_IMAGELENGTH,
  TIFFTAG_COMPRESSION,
  TIFFTAG_PHOTOMETRIC,
  TIFFTAG_STRIPOFFSETS,
  TIFFTAG_SAMPLESPERPIXEL,
  TIFFTAG_ROWSPERSTRIP,
  TIFFTAG_STRIPBYTECOUNTS,
  TIFFTAG_XRESOLUTION,
  TIFFTAG_YRESOLUTION,
  TIFFTAG_RESOLUTIONUNIT,
  TIFFTAG_BITSPERSAMPLE
};

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

/** loads a tiff, check if all required baselinetags exists
 * @param filename filename which should be processed
 */
int check_required (const char * filename ) {
  /* load file */
  TIFF* tif = TIFFOpen(filename, "r");
  if (NULL == tif) {
    fprintf( stderr, "file '%s' could not be opened\n", filename);
    exit (FIXIT_TIFF_READ_PERMISSION_ERROR);
  };
  printf("these tags are required:\n");
  print_required_tags(tif);
  int tag_counter=TIFFGetRawTagListCount(tif);
  int tagidx;
  uint32 tags[tag_counter];
  for (tagidx=0; tagidx < tag_counter; tagidx++) {
    tags[tagidx] = TIFFGetRawTagListEntry( tif, tagidx );
  }
  /* check if only baselinetags are exists,
   * iterate through all tiff-tags in tiff file
   */
  int found = 0;
  for (tagidx=0; tagidx < tag_counter; tagidx++) {
    int baseline_index=0;
    for (baseline_index = 0; baseline_index < count_of_required_baselinetags; baseline_index++) {
      if (tags[tagidx] == required_baselinetags[baseline_index]) {  
        found++;
        break;
      }
    }
  }
  printf("##### found=%i required=%i\n", found, count_of_required_baselinetags);
  if (found != count_of_required_baselinetags ) {
    if (FLAGGED == flag_be_verbose) {
      printf("tiff does not have all required tags for baseline rgb\n");
      printf("these tags are required:\n");
      print_required_tags(tif);
    }
    return FIXIT_TIFF_IS_CHECKED;
  }
  TIFFClose(tif);
  if (FLAGGED == flag_be_verbose) printf("tiff comes with all required tags for baseline rgb\n");
  return FIXIT_TIFF_IS_VALID;
}

/** loads a tiff, checks if all tags are only baseline tags
 * @param filename filename which should be processed
 */
int check_baseline(const char * filename ) {
  /* load file */
  TIFF* tif = TIFFOpen(filename, "r");
  if (NULL == tif) {
    fprintf( stderr, "file '%s' could not be opened\n", filename);
    exit (FIXIT_TIFF_READ_PERMISSION_ERROR);
  };
  uint32 tag_counter=TIFFGetRawTagListCount(tif);
  uint32 tagidx;
  uint32 tags[tag_counter];
  for (tagidx=0; tagidx < tag_counter; tagidx++) {
    tags[tagidx] = TIFFGetRawTagListEntry( tif, tagidx );
  }
  /* check if only baselinetags are exists,
   * iterate through all tiff-tags in tiff file
   */
  for (tagidx=0; tagidx < tag_counter; tagidx++) {
    int found = 0;
    int baseline_index=0;
    for (baseline_index = 0; baseline_index < count_of_baselinetags; baseline_index++) {
      if (tags[tagidx] == baselinetags[baseline_index]) { 
        found = 1; 
        break;
      }
    }
    if (found == 0 ) {
      if (FLAGGED == flag_be_verbose) {
        printf("tiff is not a baseline, because it has additional tags for baseline rgb\n");
        printf("these tags are allowed only:\n");
        print_baseline_tags(tif);
      }
      return FIXIT_TIFF_IS_CHECKED;
    }
  }
  TIFFClose(tif);
  if (FLAGGED == flag_be_verbose) printf("tiff comes only with allowed tags for baseline rgb\n");
  return FIXIT_TIFF_IS_VALID;
}

/** load a tiff, clean it up if needed, store tiff
 * @param filename filename which should be processed, repaired
 */
int cleanup_baseline(const char * filename ) {
  if (
      (FIXIT_TIFF_IS_VALID == check_baseline (filename)) &&
      (FIXIT_TIFF_IS_VALID == check_required (filename))
     ) return FIXIT_TIFF_IS_VALID;
  else {
    /* load file */
    TIFF* tif = TIFFOpen(filename, "r+");
    if (NULL == tif) {
      fprintf( stderr, "file '%s' could not be opened\n", filename);
      exit (FIXIT_TIFF_READ_PERMISSION_ERROR);
    };
    uint32 tag_counter=TIFFGetRawTagListCount(tif);
    uint32 tagidx;
    uint32 tags[tag_counter];
    for (tagidx=0; tagidx < tag_counter; tagidx++) {
      tags[tagidx] = TIFFGetRawTagListEntry( tif, tagidx );
    }
    /* iterate through all tiff-tags in tiff file
     * delete all tags not in baselinetags
     */
    for (tagidx=0; tagidx < tag_counter; tagidx++) { 
      printf ("found tag %i [0x%x] (%i)\n", tags[tagidx],tags[tagidx], tagidx); 
      int found = 0;
      int baseline_index=0;
      for (baseline_index = 0; baseline_index < count_of_baselinetags; baseline_index++) {
        if (tags[tagidx] == baselinetags[baseline_index]) { 
          printf ("DEBUG tag=%i base=%i idx=%i\n", tags[tagidx], baselinetags[baseline_index], baseline_index); 
          found = 1; 
          break;
        }
      }
      if (found == 0 ) {
        if (FLAGGED == flag_be_verbose) printf("removed tag %i\n", tags[tagidx]);
        TIFFUnsetField(tif, tags[tagidx]);
      }
    }
    if (FLAGGED == flag_be_verbose) printf("After  correction\n-----------------\n");
    if (FLAGGED == flag_be_verbose) TIFFPrintDirectory(tif, stdout, TIFFPRINT_NONE);
    /* write data back, only if no flag_check_only is set */
    int written = TIFFRewriteDirectory(tif);
    if (1 != written) {
      fprintf(stderr, "something is wrong, tiffdir could not be written to file '%s'\n", filename);
      exit (FIXIT_TIFF_WRITE_ERROR);
    }
    TIFFClose(tif);
  }
  if  (FIXIT_TIFF_IS_VALID == check_baseline (filename)) return FIXIT_TIFF_IS_CORRECTED;
  else return FIXIT_TIFF_IS_CHECKED;
}

