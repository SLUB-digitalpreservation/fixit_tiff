/* fixes broken TIFF Files
 * 
 * fixes invalid DateTime-field in Baseline-TIFFs,
 * based on http://www.awaresystems.be/imaging/tiff/tifftags/baseline.html
 *
 * author: Andreas Romeyke, 2013
 * licensed under conditions of libtiff 
 */

#include "fixit_tiff.h"
#include "check_datetime.h"

/** check if date / time values are within correct ranges 
 * @param year year
 * @param month month
 * @param day day
 * @param hour hour
 * @param min min
 * @param sec sec
 * @return 0 if success, otherwise -1
 */
int test_plausibility (int * year, int * month, int * day, int * hour, int * min, int * sec) {
  if (FLAGGED == flag_be_verbose) printf ("found: y=%d m=%d d=%d h=%d m=%d s=%d\n", *year, *month, *day, *hour, *min, *sec);
  if (
      1500 < *year && 
      2100 > *year &&
      0 < *month &&
      13 > *month &&
      0 < *day &&
      32 > *day &&
      0 <= *hour &&
      24 > *hour &&
      0 <= *min &&
      60 > *min &&
      0 <= *sec &&
      60 > *sec
     ) {
    return 0;
  } else {
    return -1;
  }
}


/** loads a tiff, fix it if needed, stores tiff
 * @param filename filename which should be processed, repaired
 */
int check_datetime(const char * filename ) {
  /* load file */
  TIFF* tif = TIFFOpen(filename, "r+");
  if (NULL == tif) {
    fprintf( stderr, "file '%s' could not be opened\n", filename);
    exit (FIXIT_TIFF_READ_PERMISSION_ERROR);
  };
  /* find date-tag and fix it */
  char *datetime=NULL;
  uint32 count=0;
  int found=TIFFGetField(tif, TIFFTAG_DATETIME, &datetime, &count);
  if (1==found) { /* there exists a datetime field */
    int day=0;
    int month=0;
    int year=0;
    int hour=0;
    int min=0;
    int sec=0;

    if (6 == sscanf(datetime, "%04d:%02d:%02d%02d:%02d:%02d", &year, &month, &day, &hour, &min, &sec)) {
      return test_plausibility(&year, &month, &day, &hour, &min, &sec);
    } else {
      return -2;
    }
  } else if (0 == found) {
    if (FLAGGED == flag_be_verbose) printf ("no datetime found!\n");
  }
  TIFFClose(tif);
  return FIXIT_TIFF_IS_VALID;
}

