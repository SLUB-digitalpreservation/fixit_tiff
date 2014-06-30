/* fixes broken TIFF Files
 * 
 * fixes invalid DateTime-field in Baseline-TIFFs,
 * based on http://www.awaresystems.be/imaging/tiff/tifftags/baseline.html
 *
 * author: Andreas Romeyke, 2013
 * licensed under conditions of libtiff 
 */


#include "fixit_tiff.h"

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

int TIFFGetAllTagListCount (TIFF * tif) {
  int tagcounter=0;
  uint32 i;
  uint32 dummycount;
  void * dummydata;
  for (i=0; i< 65536; i++) {
    if (i != TIFFTAG_OSUBFILETYPE) {
      int found=0;
      found=TIFFGetField( tif, i, &dummycount, &dummydata);
      if (1 == found) { /* found*/
        tagcounter++;
      }
    }
  }
  return tagcounter;
}

uint32 TIFFGetAllTagListEntry( TIFF  * tif, int tagidx ) {
  int tagcounter=0;
  uint32 i;
  uint32 dummycount;
  void * dummydata;
  for (i=0; i< 65536; i++) {
    if (i != TIFFTAG_OSUBFILETYPE) {
      int found=0;
      found=TIFFGetField( tif, i, &dummycount, &dummydata);
      if (1 == found) { /* found*/
        if (tagidx == tagcounter) {
          return i;
        }
        tagcounter++;
      }
    }
  }
  return -1;
}

void print_baseline_tags (TIFF * tif) {
  int i;
  TIFFReadDirectory( tif);
  TIFFReadDirectory( tif);
  int tag_counter=TIFFGetAllTagListCount(tif);
  int tagidx;
  uint32 tags[tag_counter];
  printf ("tag count=%i, [*] means: tag is a baseline tag\n", tag_counter);
  for (tagidx=0; tagidx < tag_counter; tagidx++) {
    tags[tagidx] = TIFFGetAllTagListEntry( tif, tagidx );
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
  int tag_counter=TIFFGetAllTagListCount(tif);
  int tagidx;
  uint32 tags[tag_counter];
  printf ("[*] means: tag already exists\n");
  for (tagidx=0; tagidx < tag_counter; tagidx++) {
    tags[tagidx] = TIFFGetAllTagListEntry( tif, tagidx );
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
  int tag_counter=TIFFGetAllTagListCount(tif);
  int tagidx;
  uint32 tags[tag_counter];
  for (tagidx=0; tagidx < tag_counter; tagidx++) {
    tags[tagidx] = TIFFGetAllTagListEntry( tif, tagidx );
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
  uint32 tag_counter=TIFFGetAllTagListCount(tif);
  uint32 tagidx;
  uint32 tags[tag_counter];
  for (tagidx=0; tagidx < tag_counter; tagidx++) {
    tags[tagidx] = TIFFGetAllTagListEntry( tif, tagidx );
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
    uint32 tag_counter=TIFFGetAllTagListCount(tif);
    uint32 tagidx;
    uint32 tags[tag_counter];
    for (tagidx=0; tagidx < tag_counter; tagidx++) {
      tags[tagidx] = TIFFGetAllTagListEntry( tif, tagidx );
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

