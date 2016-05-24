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


/** RULE0: default rule (string is correct) */
static int rule_default (const char * datestring, int * year, int * month, int * day, int * hour, int * min, int * sec) {
  if (FLAGGED == flag_be_verbose) printf ("rule00\n");
  if (6 == sscanf(datestring, "%04d:%02d:%02d%02d:%02d:%02d", year, month, day, hour, min, sec)) {
    return test_plausibility(year, month, day, hour, min, sec);
  } else {
    return -2;
  }
}

/** RULE1: fix: '18.03.2010 09:59:17' => '2010:03:18 09:59:17' */
static int rule_ddmmyyhhmmss_01 (const char * datestring, int * year, int * month, int * day, int * hour, int * min, int * sec) {
  if (FLAGGED == flag_be_verbose) printf ("rule01\n");
  if (6 == sscanf(datestring, "%02d.%02d.%04d%02d:%02d:%02d", day, month, year, hour, min, sec)) {
    return test_plausibility(year, month, day, hour, min, sec);
  } else {
    return -2;
  }
}

/** RULE2: fix: '2010-03-18 09:59:17' => '2010:03:18 09:59:17' */
static int rule_ddmmyyhhmmss_02 (const char * datestring, int * year, int * month, int * day, int * hour, int * min, int * sec) {
  if (FLAGGED == flag_be_verbose) printf ("rule02\n");
  if (6 == sscanf(datestring, "%04d-%02d-%02d%02d:%02d:%02d", year, month, day , hour, min, sec)) {
    return test_plausibility(year, month, day, hour, min, sec);
  } else {
    return -2;
  }
}

/** RULE3: fix 'Tue Dec 19 09:18:54 2006%0A' => '2006:12:19 09:18:45' */
static int rule_ddmmyyhhmmss_03 (const char * datestring, int * year, int * month, int * day, int * hour, int * min, int * sec) {
  if (FLAGGED == flag_be_verbose) printf ("rule03\n");
  char dow[4] = "\0\0\0\0";
  char monthstring[4] = "\0\0\0\0";
  int i;
  static char* months[] = {
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
  };
  fprintf (stderr, "Datestring '%s'\n", datestring);
  int ret = 0;
  ret = sscanf(datestring, "%3s%3s%02d%02d:%02d:%02d%04d", dow, monthstring, day, hour, min, sec, year);
  if (7 == ret ) {
    *month=-1;
    for ( i = 0; i<= 11; i++) {
      if (strncmp (months[i], monthstring, 3) == 0) {
        *month=i+1;
        break;
      }
    }
    return test_plausibility(year, month, day, hour, min, sec);
  } else {
    return -2;
  }
}

/** RULENOFIX: dummy rule if no other rule matches, calls only exit */
static int rule_nofix (const char * datestring, int * year, int * month, int * day, int * hour, int * min, int * sec) {
  fprintf(stderr, "rule nofix, there is no applyable rule left, aborted without fixing problem\n");
  exit(FIXIT_TIFF_DATETIME_RULE_NOT_FOUND);
}

/** used for array of rules */
#define COUNT_OF_RULES 5
/** Array of rules */
int (*rules_ptr[COUNT_OF_RULES])(const char *, int *, int *, int *, int *, int *, int *) = {
  rule_default,
  rule_ddmmyyhhmmss_01,
  rule_ddmmyyhhmmss_02,
  rule_ddmmyyhhmmss_03,
  rule_nofix
};

/** corrects broken date string to expected format, see 
 * http://www.awaresystems.be/imaging/tiff/tifftags/datetime.html
 * @param broken_datetime string with wrong datetime
 * @return string with corrected datetime
 */
char * correct_datestring (const char * broken_datetime) {
  int day;
  int month;
  int year;
  int hour;
  int min;
  int sec;
  /* if ret is wrong, you could try another rules to apply */
  int r;
  for (r = 0; r < COUNT_OF_RULES; r++) {
    if (FLAGGED == flag_be_verbose) printf("Applying rule%i", r);
    if (0 != (*rules_ptr[r])(broken_datetime, &year, &month, &day, &hour, &min, &sec)) {
      if (FLAGGED == flag_be_verbose) printf("applying next rule\n");
    } else {
      break;
    }
  }
  if (FLAGGED == flag_be_verbose) printf("datetime parsing of string '%s', year=%04d, month=%02d, day=%02d, hour=%02d, min=%02d, sec=%02d\n", broken_datetime, year, month, day, hour, min, sec);
  /* write corrected value to new string */
  char * fixed_date = NULL;
  fixed_date=malloc(sizeof(char) * TIFFDATETIMELENGTH); /* 20 comes from TIFF definition */
  if (NULL == fixed_date) {
    fprintf(stderr, "could not allocate memory for datetime conversion, abort\n");
    exit (FIXIT_TIFF_MEMORY_ALLOCATION_ERROR);
  }
  int written = snprintf(fixed_date, TIFFDATETIMELENGTH, "%04d:%02d:%02d %02d:%02d:%02d", year, month, day, hour, min, sec);

  if (written != (TIFFDATETIMELENGTH)-1) {
    fprintf(stderr, "something wrong, instead %d chars, %d chars were written\n",TIFFDATETIMELENGTH-1 ,written);
    exit (FIXIT_TIFF_STRING_COPY_ERROR);
  }
  return fixed_date;
}


/** loads a tiff, fix it if needed, stores tiff
 * @param filename filename which should be processed, repaired
 */
int cleanup_datetime(const char * filename ) {
  if (FIXIT_TIFF_IS_VALID == check_datetime(filename))  return FIXIT_TIFF_IS_VALID;
  else {
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
      if (FLAGGED == flag_be_verbose) printf("Before correction\n-----------------\n");	
      if (FLAGGED == flag_be_verbose) TIFFPrintDirectory(tif, stdout, TIFFPRINT_NONE);
      if (FLAGGED == flag_be_verbose) printf("c=%u datetime:'%s'\n", count, datetime);
      /* should be corrected? */
      char * new_datetime = correct_datestring( datetime );
      /* repair */
          TIFFSetField(tif, TIFFTAG_DATETIME, new_datetime);
          if (FLAGGED == flag_be_verbose) printf("After  correction\n-----------------\n");
          if (FLAGGED == flag_be_verbose) TIFFPrintDirectory(tif, stdout, TIFFPRINT_NONE);
          /* write data back, only if no flag_check_only is set */
          int written = TIFFRewriteDirectory(tif);
          if (1 != written) {
            fprintf(stderr, "something is wrong, tiffdir could not be written to file '%s'\n", filename);
            exit (FIXIT_TIFF_WRITE_ERROR);
          }
	  if (NULL != new_datetime) free(new_datetime);
        } else { /* checked via check_datetime() */
        }
    TIFFClose(tif);
  }
  if  (FIXIT_TIFF_IS_VALID == check_datetime (filename)) return FIXIT_TIFF_IS_CORRECTED;
  else return FIXIT_TIFF_IS_CHECKED;

}

