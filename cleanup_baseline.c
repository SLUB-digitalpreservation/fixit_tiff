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

/** loads a tiff, cleanup it if needed, stores tiff
 * @param filename filename which should be processed, repaired
 */
int check_baseline(const char * filename ) {
  /* load file */
  TIFF* tif = TIFFOpen(filename, "r+");
  if (NULL == tif) {
    fprintf( stderr, "file '%s' could not be opened\n", filename);
    exit (FIXIT_TIFF_READ_PERMISSION_ERROR);
  };
  uint32 tag_counter=TIFFGetTagListCount(tif);
  uint32 tagidx;
  uint32 tags[tag_counter];
  for (tagidx=0; tagidx < tag_counter; tagidx++) {
    tags[tagidx] = TIFFGetTagListEntry( tif, tagidx );
  }
  /* iterate through all tiff-tags in tiff file
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
        return FIXIT_TIFF_IS_CHECKED;
    }
  }
  TIFFClose(tif);
  return FIXIT_TIFF_IS_VALID;
}

/** load a tiff, clean it up if needed, store tiff
 * @param filename filename which should be processed, repaired
 */
int cleanup_baseline(const char * filename ) {
  if (FIXIT_TIFF_IS_VALID == check_baseline (filename)) return FIXIT_TIFF_IS_VALID;
  else {
    /* load file */
    TIFF* tif = TIFFOpen(filename, "r+");
    if (NULL == tif) {
      fprintf( stderr, "file '%s' could not be opened\n", filename);
      exit (FIXIT_TIFF_READ_PERMISSION_ERROR);
    };
    uint32 tag_counter=TIFFGetTagListCount(tif);
    uint32 tagidx;
    uint32 tags[tag_counter];
    for (tagidx=0; tagidx < tag_counter; tagidx++) {
      tags[tagidx] = TIFFGetTagListEntry( tif, tagidx );
    }
    /* iterate through all tiff-tags in tiff file
     * delete all tags not in baselinetags
     */
    for (tagidx=0; tagidx < tag_counter; tagidx++) {
      /* printf ("found tag %i [0x%x] (%i)\n", tags[tagidx],tags[tagidx], tagidx); */
      int found = 0;
      int baseline_index=0;
      for (baseline_index = 0; baseline_index < count_of_baselinetags; baseline_index++) {
        if (tags[tagidx] == baselinetags[baseline_index]) { 
          /* printf ("DEBUG tag=%i base=%i idx=%i\n", tags[tagidx], baselinetags[baseline_index], baseline_index); */
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

