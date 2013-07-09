/** fixes broken TIFF Files */
/* author: Andreas Romeyke, 2013 */
/* licensed under conditions of libtiff */

#include "fixit_tiff.h"

/* 20 comes from TIFF definition */
#define TIFFDATETIMELENGTH 20

void help () {
  printf ("fixit_tiff broken_tiff_file corrected_tiff_file\n");
}

/** copy infile to outfile 
 * @param inf string with infile name 
 * @param outf string with outfile name 
 */
void copy_file (const char * inf, const char * outf) {
  FILE * in = fopen( inf, "rb");
  if (NULL == in) {
    fprintf(stderr, "could not open file '%s' for reading\n", inf);
    exit (-20);
  }
  FILE * out = fopen(outf, "wb");
  if (NULL == out) {
    fprintf(stderr, "could not open file '%s' for writing\n", outf);
    exit (-21);
  }
  /* first, copy infile to outfile */
  char            buffer[512];
  size_t          n;
  while ((n = fread(buffer, sizeof(char), sizeof(buffer), in)) > 0)
  {
    if (n != fwrite(buffer, sizeof(char), n, out)) {
      fprintf(stderr, "failure writing to file '%s'\n", outf);
      exit (-22);
    }
  }
  fclose(in);
  fclose(out);
}

/* check if date / time values are within correct ranges */
int test_plausibility (int * year, int * month, int * day, int * hour, int * min, int * sec) {
  printf ("found: y=%d m=%d d=%d h=%d m=%d s=%d\n", *year, *month, *day, *hour, *min, *sec);
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

/* RULE0: default rule (string is correct) */
int rule_default (char * datestring, int * year, int * month, int * day, int * hour, int * min, int * sec) {
  printf ("rule00\n");
  if (6 == sscanf(datestring, "%04d:%02d:%02d%02d:%02d:%02d", year, month, day, hour, min, sec)) {
    test_plausibility(year, month, day, hour, min, sec);
  } else {
    return -2;
  }
}

/* RULE1: fix: '04.03.2010 09:59:17' => '04:03:2010 09:59:17' */
int rule_ddmmyyhhmmss_01 (char * datestring, int * year, int * month, int * day, int * hour, int * min, int * sec) {
  printf ("rule01\n");
  if (6 == sscanf(datestring, "%02d.%02d.%04d%02d:%02d:%02d", day, month, year, hour, min, sec)) {
    test_plausibility(year, month, day, hour, min, sec);
  } else {
    return -2;
  }
}

#define COUNT_OF_RULES 2
/* Array of rules */
int (*rules_ptr[COUNT_OF_RULES])(char *, int *, int *,int *, int *,int *, int *) = {
  rule_default,
  rule_ddmmyyhhmmss_01
};

/** corrects broken date string to expected format, see 
 * http://www.awaresystems.be/imaging/tiff/tifftags/datetime.html
 * @param broken_datetime string with wrong datetime
 * @return string with corrected datetime
 */
char * correct_datestring (char * broken_datetime) {
  int day;
  int month;
  int year;
  int hour;
  int min;
  int sec;
  /* if ret is wrong, you could try another rules to apply */
  int r;
  for (r = 0; r < COUNT_OF_RULES; r++) {
    printf("Applying rule%i", r);
    if (0 != (*rules_ptr[r])(broken_datetime, &year, &month, &day, &hour, &min, &sec)) {
      fprintf(stderr, "applying next rule\n");
    } else {
      break;
    }
  }
  printf("datetime parsing of string '%s', year=%04d, month=%02d, day=%02d, hour=%02d, min=%02d, sec=%02d\n", broken_datetime, year, month, day, hour, min, sec);
  /* write corrected value to new string */
  char * fixed_date = NULL;
  fixed_date=malloc(sizeof(char) * TIFFDATETIMELENGTH); /* 20 comes from TIFF definition */
  if (NULL == fixed_date) {
    fprintf(stderr, "could not allocate memory for datetime conversion, abort\n");
    exit (-4);
  }
  int written = snprintf(fixed_date, TIFFDATETIMELENGTH, "%04d:%02d:%02d %02d:%02d:%02d", year, month, day, hour, min, sec);

  if (written != (TIFFDATETIMELENGTH)-1) {
    fprintf(stderr, "something wrong, instead %d chars, %d chars were written\n",TIFFDATETIMELENGTH ,written);
    exit (-5);
  }
  return fixed_date;
}

/* loads a tiff, fix it if needed, stores tiff */
void fix_tiff(const char * filename) {
  /* load file */
  TIFF* tif = TIFFOpen(filename, "r+");
  if (NULL == tif) {
    fprintf( stderr, "file '%s' could not be opened\n", filename);
    exit (-2);
  };
  /* find date-tag and fix it */
  char *datetime;
  uint32 count;
  int found=TIFFGetField(tif, TIFFTAG_DATETIME, &datetime, &count);
  if (1==found) { /* there exists a datetime field */
    printf("Before correction\n-----------------\n");	
    TIFFPrintDirectory(tif, stdout, TIFFPRINT_NONE);
    printf("c=%u datetime:'%s'\n", count, datetime);
    /* should be corrected? */
    char * new_datetime = correct_datestring( datetime );
    if (0 != strncmp(datetime, new_datetime, TIFFDATETIMELENGTH)) {
      /* yes, correct TIFF DateTIME */
      TIFFSetField(tif, TIFFTAG_DATETIME, new_datetime);
      printf("After  correction\n-----------------\n");
      TIFFPrintDirectory(tif, stdout, TIFFPRINT_NONE);
      /* write data back */
      int written = TIFFRewriteDirectory(tif);
      if (1 != written) {
        fprintf(stderr, "something is wrong, tiffdir could not be written to file '%s'\n", filename);
        exit (-3);
      }
    } else { /* no, should not be touched */
      printf ("no correction needed\n");
    }
  } else if (0 == found) {
    printf ("no datetime found!\n");
  }

  TIFFClose(tif);
}

/* main */
int main (int argc, char * argv[]) {
  if (argc != 3) {
    help();
    exit(-1);
  }
  const char *infilename = argv[1];
  const char *outfilename= argv[2];
  printf ("infile='%s', outfile='%s'\n", infilename, outfilename);
  copy_file (infilename, outfilename);
  fix_tiff(outfilename);
  return 0;
}


