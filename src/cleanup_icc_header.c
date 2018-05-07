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
#include "tiff_helper.h"

char * correct_iccprofile(unsigned long iccsize, char * iccdata) {
  /*  fixes only wrong 'APPL ' -> 'appl ' in preferredcmmtype  */
  char preferredcmmtype[5]="    "; memcpy(preferredcmmtype, &iccdata[4],4);
  if  (0 == strncmp("APPL", preferredcmmtype, 4)) {
    printf("Found wrong 'APPL' in preferred cmmtype, try to correct it\n");
    char * newiccdata = malloc( sizeof( char ) * iccsize);
    if (NULL == newiccdata) {
      perror("Could not allocate memory for new iccdata");
      exit(FIXIT_TIFF_MEMORY_ALLOCATION_ERROR);
    }
    memcpy (newiccdata, iccdata, iccsize);
    memcpy(&newiccdata[4], "appl", 4);
    return newiccdata;
  } else if (0 == strncmp("Lino", preferredcmmtype, 4)) {
    /*  fixes only unregistered entry 'Lino' -> 0x00000000*/
    printf("Found unregistered entry 'Lino' in preferred cmmtype, try to correct it\n");
    char * newiccdata = malloc( sizeof( char ) * iccsize);
    if (NULL == newiccdata) {
      perror("Could not allocate memory for new iccdata");
      exit(FIXIT_TIFF_MEMORY_ALLOCATION_ERROR);
    }
    memcpy (newiccdata, iccdata, iccsize);
    memset(&newiccdata[4], 0x0, 4);
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
    if (FLAGGED == flag_be_verbose) printf("Before correction\n-----------------\n");	
    if (FLAGGED == flag_be_verbose) TIFFPrintDirectory(tif, stdout, TIFFPRINT_NONE);
    /* find ICC-tag and fix it */
    char *iccprofile=NULL;

    uint32 tag_counter=TIFFGetRawTagListCount(tif);
    uint32 tagidx;
    for (tagidx=0; tagidx < tag_counter; tagidx++) {
      uint32 tag = TIFFGetRawTagListEntry( tif, tagidx );
      if (tag == TIFFTAG_ICCPROFILE) {
        /*  via TIFFGetRawTagListEntry we have the tag
         *  read, the next 2 bytes are the type, next 2 count, last 4 value/offset */
        int fd = TIFFFileno( tif);
        uint16 tagtype=0;
        if ( read( fd, &tagtype, 2) != 2 ) {
          perror ("TIFF Header read error tagtype");
          exit( FIXIT_TIFF_READ_ERROR );
        }
        if (TIFFIsByteSwapped(tif))
          TIFFSwabShort(&tagtype);
        assert(TIFF_UNDEFINED == tagtype);
        uint32 count=0;
        if ( read( fd, &count, 4) != 4 ) {
          perror ("TIFF Header read error tagcount");
          exit( FIXIT_TIFF_READ_ERROR );
        }
        if (TIFFIsByteSwapped(tif))
          TIFFSwabLong(&count);
        uint32 offset=0;
        if ( read( fd, &offset, 4) != 4 ) {
          perror ("TIFF Header read error offset");
          exit( FIXIT_TIFF_READ_ERROR );
        }
        if (TIFFIsByteSwapped(tif))
          TIFFSwabLong(&offset);
        if (FLAGGED == flag_be_verbose) printf("tag=%i tagtype=%i c=%u offset=%i\n", tag, tagtype, count, offset);
        iccprofile = malloc( sizeof( char ) * count);
        if (NULL == iccprofile) {
          perror("Could not allocate memory for new iccdata");
          exit(FIXIT_TIFF_MEMORY_ALLOCATION_ERROR);
        }
        /*  rewind to offset */
        if (lseek(fd, offset, SEEK_SET) != offset) {
          perror("TIFF ICC profile seek error to offset");
          exit( FIXIT_TIFF_READ_ERROR );
        }
        if (read(fd, iccprofile, count) != count) {
          perror ("TIFF ICC profile read error offset");
          exit( FIXIT_TIFF_READ_ERROR );
        }
        /* repair */
        char * new_iccprofile = correct_iccprofile(count, iccprofile );
        if (NULL == new_iccprofile) {
          perror("Could not allocate memory for new icc profile");
          exit(FIXIT_TIFF_MEMORY_ALLOCATION_ERROR);
        }
        /*  rewind to offset */
        if (lseek(fd, offset, SEEK_SET) != offset) {
          perror("TIFF ICC profile seek error to offset");
          exit( FIXIT_TIFF_READ_ERROR );
        }
        /* write data back, only if no flag_check_only is set */
        if (write(fd, new_iccprofile, count) != count) {
          perror("TIFF ICC Profile write error");
          exit(FIXIT_TIFF_WRITE_ERROR);
        }
        if (0 != close(fd)) {
          perror("TIFF could not be closed");
          exit(EXIT_FAILURE);
        }
        if (FLAGGED == flag_be_verbose) {
          printf("tag=%i tagtype=%i count=%i offset=%i (0x%04x)\n", tag, tagtype, count, offset, offset);
          printf("old_profile=\n");
          for (int i=0; i <40; i++) {
            printf("0x%02x ", *(iccprofile++));
          }
          char * p=new_iccprofile;
          printf("\nnew_profile=\n");
          for (int i=0; i <40; i++) {
            printf("0x%02x ", *(p++));
          }
          printf("\n");
        }
        if (NULL != new_iccprofile) free(new_iccprofile);

      }
      if (FLAGGED == flag_be_verbose) printf("After  correction\n-----------------\n");
    }
    if  (FIXIT_TIFF_IS_VALID == check_icc_header (filename)) return FIXIT_TIFF_IS_CORRECTED;
    else return FIXIT_TIFF_IS_CHECKED;
  }
}


