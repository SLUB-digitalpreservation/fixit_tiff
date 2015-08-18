/* fixes broken TIFF Files
 * 
 * fixes invalid DateTime-field in Baseline-TIFFs,
 * based on http://www.awaresystems.be/imaging/tiff/tifftags/baseline.html
 *
 * author: Andreas Romeyke, 2013
 * licensed under conditions of libtiff 
 */

#include "check.h"
#include "../fixit/fixit_tiff.h"

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
ret_t check_datetime(TIFF* tif ) {
  tifp_check( tif)
  printf("check if tag %u (%s) is correct\n", TIFFTAG_DATETIME, TIFFTagName(tif, TIFFTAG_DATETIME));
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
    
    int r = 0;
    int i;
    for (i=0; i<count; i++) {
        if (datetime[i] == '\0') {
          r = i+1;
          break;
        }
    }
    printf(" count=%u\n\n", count);
    if (0 == r) {
      if (6 == sscanf(datetime, "%04d:%02d:%02d%02d:%02d:%02d", &year, &month, &day, &hour, &min, &sec)) {
        if (0 == test_plausibility(&year, &month, &day, &hour, &min, &sec)) {
          ret_t res;
          res.returnmsg=NULL;
          res.returncode=0;
          return res;

        } else {
          tif_fails("tag %u (%s) value of datetime not plausible, should be  \"yyyy:MM:DD hh:mm:ss\", but was \"%s\"\n", TIFFTAG_DATETIME, TIFFTagName(tif, TIFFTAG_DATETIME), datetime);
        }
      } else {
        tif_fails("tag %u (%s) value of datetime should be \"yyyy:MM:DD hh:mm:ss\", but was \"%s\"\n", TIFFTAG_DATETIME, TIFFTagName(tif, TIFFTAG_DATETIME), datetime);
      }
    } else {
      tif_fails("tag %u (%s) value of datetime should be \"yyyy:MM:DD hh:mm:ss\", but was \"%s\" and contains a \\0 at %i (count=%u)\n", TIFFTAG_DATETIME, TIFFTagName(tif, TIFFTAG_DATETIME), datetime, r, count);
    }

  }
  ret_t res;
  res.returnmsg=NULL;
  res.returncode=0;
  return res;
}

