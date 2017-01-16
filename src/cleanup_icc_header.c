/* fixes broken TIFF Files
 * 
 * fixes invalid ICC header-field in Baseline-TIFFs,
 * based on http://www.awaresystems.be/imaging/tiff/tifftags/baseline.html
 *
 * author: Andreas Romeyke, 2016
 * licensed under conditions of libtiff 
 */


#include "fixit_tiff.h"
#include "check_icc_header.h"

char * correct_iccprofile(unsigned long iccsize, char * iccdata) {
  /*  fixes only wrong 'APPL ' -> 'appl ' in preferredcmmtype  */
  char preferredcmmtype[5]="    "; memcpy(preferredcmmtype, &iccdata[4],4);
  if  (0 == strncmp("APPL", preferredcmmtype, 4)) {
    printf("Found wrong 'APPL' in preferred cmmtype, try to correct it\n");
    char * newiccdata = malloc( sizeof( char ) * iccsize);
    memcpy (newiccdata, iccdata, iccsize);
    memcpy(&newiccdata[4], "appl", 4);
    return newiccdata;
  }
  return iccdata;
}


/** loads a tiff, fix it if needed, stores tiff
 * @param filename filename which should be processed, repaired
 */
int cleanup_icc_header(const char * filename ) {
  if (FIXIT_TIFF_IS_VALID == check_icc_header(filename))  return FIXIT_TIFF_IS_VALID;
  else {
    /* load file */
    TIFF* tif = TIFFOpen(filename, "r+");
    if (NULL == tif) {
      fprintf( stderr, "file '%s' could not be opened\n", filename);
      exit (FIXIT_TIFF_READ_PERMISSION_ERROR);
    };
    /* find ICC-tag and fix it */
    char *iccprofile=NULL;
    uint32 count=0;
    int found=TIFFGetField(tif, TIFFTAG_ICCPROFILE, &count, &iccprofile);
    if (1==found) { /* there exists a datetime field */
      if (FLAGGED == flag_be_verbose) printf("Before correction\n-----------------\n");	
      if (FLAGGED == flag_be_verbose) TIFFPrintDirectory(tif, stdout, TIFFPRINT_NONE);
      if (FLAGGED == flag_be_verbose) printf("c=%u iccprofile:'%p'\n", count, iccprofile);
      /* should be corrected? */
      char * new_iccprofile = correct_iccprofile(count, iccprofile );
      /* repair */
      TIFFSetField(tif, TIFFTAG_ICCPROFILE, count, new_iccprofile);
      if (FLAGGED == flag_be_verbose) printf("After  correction\n-----------------\n");
      if (FLAGGED == flag_be_verbose) TIFFPrintDirectory(tif, stdout, TIFFPRINT_NONE);
      /* write data back, only if no flag_check_only is set */
      int written = TIFFWriteDirectory(tif); /* HINT: if errorneous, use TIFFReWriteDirectory to append IFD at end */
      if (1 != written) {
        fprintf(stderr, "something is wrong, tiffdir could not be written to file '%s'\n", filename);
        exit (FIXIT_TIFF_WRITE_ERROR);
      }
      if (NULL != new_iccprofile) free(new_iccprofile);
    } else { /* checked via check_datetime() */
    }
    TIFFClose(tif);
    /* FIXME: workaround for wrong EXIFIFD type set by libTIFF, should later be
     * replaced by a more simple variant */
    cleanup_tagtype(filename, 34665); // EXIFIFDOFFSET
  }
  if  (FIXIT_TIFF_IS_VALID == check_icc_header (filename)) return FIXIT_TIFF_IS_CORRECTED;
  else return FIXIT_TIFF_IS_CHECKED;

}


