/* fixes broken TIFF Files
 * 
 * fixes invalid DateTime-field in Baseline-TIFFs,
 * based on http://www.awaresystems.be/imaging/tiff/tifftags/baseline.html
 *
 * author: Andreas Romeyke, 2013
 * licensed under conditions of libtiff 
 */


#include "../fixit/fixit_tiff.h"
#include "config_parser.h"

/** main */
int main (int argc, char * argv[]) {
  if (argc != 3) {
        fprintf( stderr, "%s needs two arguments, first should be the tiff-filename, second the config-file\n", argv[0]);
        exit (EXIT_FAILURE);
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

  FILE * cfg = fopen(cfg_file, "r");
  if (NULL == cfg) {
    fprintf( stderr, "file '%s' could not be opened\n", cfg_file);
    exit (FIXIT_TIFF_READ_PERMISSION_ERROR);
  };
  clean_plan();
  parse_plan_via_stream( cfg );
  int is_valid = 0;
  ret_t res;
  /* special checks */
  res = check_has_only_one_ifd( tif); if (0 != res.returncode) {is_valid++;}
  res = check_tagorder( tif); if (0 != res.returncode) {is_valid++;}
  res = check_tag_quiet( tif, TIFFTAG_DATETIME);
  if (res.returncode == 0) { 
      res = check_datetime( tif );
      if (0 != res.returncode) {is_valid++;}
  }
  is_valid += execute_plan(tif);
  //print_plan_results();
  clean_plan();
  TIFFClose(tif);
  fclose(cfg);
  if (0 == is_valid) {
    exit(EXIT_SUCCESS);
  } else {
    exit(EXIT_FAILURE);
  }
}
