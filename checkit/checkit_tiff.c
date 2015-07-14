/* fixes broken TIFF Files
 * 
 * fixes invalid DateTime-field in Baseline-TIFFs,
 * based on http://www.awaresystems.be/imaging/tiff/tifftags/baseline.html
 *
 * author: Andreas Romeyke, 2013
 * licensed under conditions of libtiff 
 */


#include "../fixit/fixit_tiff.h"
#include "../fixit/tiff_helper.h"

/** main */
int main (int argc, char * argv[]) {
  if (argc != 3) {
        fprintf( stderr, "%s needs two arguments, first should be the tiff-filename, second the config-file\n", argv[0]);
        exit (-1);
  }
  const char *tiff_file=argv[1];
  const char *cfg_file=argv[2];
  printf("tiff file=%s\n", tiff_file);
  printf("cfg_file=%s\n", cfg_file);
  /* load tiff file */
  TIFF* tif = TIFFOpen(tiff_file, "r");
  if (NULL == tif) {
    fprintf( stderr, "file '%s' could not be opened\n", tiff_file);
    exit (FIXIT_TIFF_READ_PERMISSION_ERROR);
  };
  uint32 tag_counter=TIFFGetRawTagListCount(tif);
  uint32 tagidx;
  uint32 tags[tag_counter];
  for (tagidx=0; tagidx < tag_counter; tagidx++) {
    tags[tagidx] = TIFFGetRawTagListEntry( tif, tagidx );
  }
  /* TODO: load parser
   * call check_functions
   */
  TIFFClose(tif);
  exit(0);
  }
