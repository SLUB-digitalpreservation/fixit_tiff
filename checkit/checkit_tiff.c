/* rule based checks if given TIFF is a specific baseline TIFF
 * 
 * author: Andreas Romeyke, 2015
 * licensed under conditions of libtiff 
 * (see http://libtiff.maptools.org/misc.html)
 *
 */


#include "config_parser.h"
#include <unistd.h>
#include <assert.h>

#define FLAGGED 1
#define UNFLAGGED 0

/** help function */
void help () {
  printf ("checkit_tiff\n");
  printf ("\tversion: %s, ", VERSION);
  printf ("\tuses libtiff version %s\n", TIFFGetVersion());
  printf ("call it with:\n");
  printf ("\tcheckit_tiff [-h|-m|-d|-r] <tifffile> <configfile>\n");
  printf ("\nwhere <tifffile> is the tiff file (or directory) to be validated\n");
  printf ("and <configfile> is the file name of the validation profile\n");
  printf ("\t-h this help\n");
  printf ("\t-m uses memmapped I/O (faster validation, but needs more RAM)\n");
  printf ("\t-d check all files in that directory\n");
  printf ("\t-r creates a report file\n");
  printf ("example:\n\tcheckit_tiff tiffs_should_pass/minimal_valid.tiff example_configs/baseline_minimal.cfg\n");
  printf ("\n");
}


/** main */
int main (int argc, char * argv[]) {
  printf("'%s' version: %s, ", argv[0], VERSION);
  printf("licensed under conditions of libtiff (see http://libtiff.maptools.org/misc.html)\n");
  int c;
  int flag_check_directory=UNFLAGGED;
  int flag_use_memorymapped_io=UNFLAGGED;
  const char * reportfilename = NULL;
  while ((c = getopt (argc, argv, "hmdr:")) != -1) {
	  switch (c)
	  {
		  case 'h': /* help */
			  help();
			  exit (0);
		  case 'd': /* check directory */
			  flag_check_directory = FLAGGED;
			  printf("\nWarning, option not working yet\n");
			  break;
		  case 'm': /* use memory mapped I/O */
			  flag_use_memorymapped_io=FLAGGED;
			  printf("\nWarning, option not working yet\n");
			  break;
		  case 'r': /* expects infile */
			  reportfilename=optarg;
			  printf("\nWarning, option not working yet\n");
			  break;
		  case '?': /* something goes wrong */
			  if (optopt == 'r') {
				  fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				  return (-1);
			  }
			  else if (isprint (optopt)) {
				  fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				  return (-1);
			  }
			  else if (0 !=optopt) {
				  fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
				  return (-1);
			  }
			  break;
		  default:
			  abort();
	  }


  }
  if (argc - optind != 2) {
	help();
        fprintf( stderr, "%s needs at least two arguments, first should be the tiff-filename, second the config-file, example:\n\t %s tiffs_should_pass/minimal_valid.tiff example_configs/baseline_minimal.cfg\n", argv[0], argv[0]);
        exit (EXIT_FAILURE);
  }
  const char *tiff_file=argv[optind];
  const char *cfg_file=argv[optind+1];
  printf("tiff file=%s\n", tiff_file);
  printf("cfg_file=%s\n", cfg_file);
  /* load tiff file */
  TIFF* tif = NULL;
  if (flag_use_memorymapped_io == FLAGGED) {
	  tif = TIFFOpen(tiff_file, "rM");
  } else { /* slow */
	  tif = TIFFOpen( tiff_file, "rm");
  }

  if (NULL == tif) {
    fprintf( stderr, "file '%s' could not be opened\n", tiff_file);
    exit (EXIT_FAILURE);
  };

  FILE * cfg = fopen(cfg_file, "r");
  if (NULL == cfg) {
    fprintf( stderr, "file '%s' could not be opened\n", cfg_file);
    exit (EXIT_FAILURE);
  };
  clean_plan();
  parse_plan_via_stream( cfg );
  int is_valid = 0;
  ret_t res;
  /* special checks */
  res = check_all_IFDs_are_word_aligned( tif); if (0 != res.returncode) {is_valid++;}
  res = check_has_only_one_ifd( tif); if (0 != res.returncode) {is_valid++;}
  res = check_tagorder( tif); if (0 != res.returncode) {is_valid++;}
  res = check_all_offsets_are_used_once_only( tif ); if (0 != res.returncode) {is_valid++;}
  res = check_all_offsets_are_word_aligned( tif );  if (0 != res.returncode) {is_valid++;}
  res = check_tag_quiet( tif, TIFFTAG_DATETIME);
  if (res.returncode == 0) { 
      res = check_datetime( tif );
      if (0 != res.returncode) {is_valid++;}
  }
  is_valid += execute_plan(tif);
   if (is_valid > 0) {
        printf("found %i errors\n", is_valid);
  } else {
        printf("the given tif is valid\n");
  }
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
