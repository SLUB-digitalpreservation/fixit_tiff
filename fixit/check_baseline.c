/* fixes broken TIFF Files
 * 
 * fixes unused tags in Baseline-TIFFs,
 * based on http://www.awaresystems.be/imaging/tiff/tifftags/baseline.html
 *
 * author: Andreas Romeyke, 2013
 * licensed under conditions of libtiff 
 */


#include "fixit_tiff.h"
#include <tiffio.h>


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


