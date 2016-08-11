/* fixes broken TIFF Files
 * 
 * fixes invalid DateTime-field in Baseline-TIFFs,
 * based on http://www.awaresystems.be/imaging/tiff/tifftags/baseline.html
 *
 * author: Andreas Romeyke, 2013
 * licensed under conditions of libtiff 
 */


#include "fixit_tiff.h"
#include "check_baseline.h"
#include "check_datetime.h"

/** help function */
void help () {
  printf ("fixit_tiff\n");
  printf ("uses libtiff version %s\n", TIFFGetVersion());
  printf ("call it with:\n");
  printf ("\tfixit_tiff [-h|-c|-s|-b|-x <tag>|-p] -i <infile> [-o <outfile>]\n");
  printf ("\nwhere <infile> is the possibly broken file\n");
  printf ("and <outfile> is the name of the corrected file\n");
  printf ("\t-c checks file only\n");
  printf ("\t-s inplace file substitution (only -i needed, dangerous)\n");
  printf ("\t-b clean up (eliminates tags to be baseline tiff conform)\n");
  printf ("\t-q disables describing messages\n");
  printf ("\t-t try to fix tagorder (dangerous)\n");
  printf ("\t-x tag clean tiff from given tag\n");
  printf ("\t-p try to repair ICC header profile\n");
  printf ("\tHint: 'fixit_tiff -i <infile> -o <outfile>' repairs date only\n");
}


/** copy infile to outfile 
 * @param inf string with infile name 
 * @param outf string with outfile name 
 */
void copy_file (const char * inf, const char * outf) {
  FILE * in = fopen( inf, "rb");
  if (NULL == in) {
    fprintf(stderr, "could not open file '%s' for reading\n", inf);
    exit (FIXIT_TIFF_READ_PERMISSION_ERROR);
  }
  FILE * out = fopen(outf, "wb");
  if (NULL == out) {
    fprintf(stderr, "could not open file '%s' for writing\n", outf);
    exit (FIXIT_TIFF_WRITE_PERMISSION_ERROR);
  }
  /* copy infile to outfile */
  char            buffer[512];
  size_t          n;
  while ((n = fread(buffer, sizeof(char), sizeof(buffer), in)) > 0)
  {
    if (n != fwrite(buffer, sizeof(char), n, out)) {
      fprintf(stderr, "failure writing to file '%s'\n", outf);
      exit (FIXIT_TIFF_WRITE_ERROR);
    }
  }
  fclose(in);
  fclose(out);
}


/** main */
int main (int argc, char * argv[]) {
  const char *infilename = NULL;
  const char *outfilename= NULL;
  //opterr = 0;
  int c;
  int flag_substitute_only=UNFLAGGED;
  int flag_baseline_cleanup=UNFLAGGED;
  int flag_tagorder=UNFLAGGED;
  int flag_clean_icc=UNFLAGGED;
  int flag_check_only=UNFLAGGED;
  int clean_tag=UNFLAGGED;
  flag_be_verbose = FLAGGED;
  while ((c = getopt (argc, argv, "s::cbqtp::hi:o:x:")) != -1) {
      switch (c)
           {
           case 'h': /* help */
             help();
             exit (0);
           case 's': /* inplace file substitution (no new output file is written) */
             flag_substitute_only = FLAGGED;
             break;
           case 'b': /* clean up to be baseline tiff conform */
             flag_baseline_cleanup=FLAGGED;
             break;
           case 't': /* try to fix tagorder */
             flag_tagorder=FLAGGED;
             break;
           case 'c': /* reports only if repair needed */
             flag_check_only = FLAGGED; 
             break;
           case 'q': /* disables describing messages */
             flag_be_verbose = UNFLAGGED;
             break;
           case 'i': /* expects infile */
             infilename=optarg;
             break;
           case 'o': /* expects outfile */
             outfilename=optarg;
             break;
           case 'x': /* expects tagnumber */
             clean_tag =atoi(optarg);
             break;
           case 'p': /* try to clean ICC header */
             flag_clean_icc = FLAGGED;
             break;
           case '?': /* something goes wrong */
             if (optopt == 'i' || optopt == 'o' || optopt == 'x')
               fprintf (stderr, "Option -%c requires an argument.\n", optopt);
             else if (isprint (optopt))
               fprintf (stderr, "Unknown option `-%c'.\n", optopt);
             else if (0 !=optopt) {
               fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
               return (-1);
             }
             break;
           default:
             abort();
           }
  }
  /* added additional checks */
  /* TODO: add check that no inline and no check if cleantag */
  if ((UNFLAGGED != clean_tag) && (clean_tag < 254) && (clean_tag > 65535)) {
    fprintf(stderr, "The option '-x' expects a value in range 254..65535, see '%s -h' for details\n", argv[0]);
    exit(FIXIT_TIFF_CMDLINE_ARGUMENTS_ERROR);
  }
  if ((FLAGGED == flag_substitute_only) && (FLAGGED == flag_check_only)) {
    fprintf (stderr, "The options '-s' and '-c' could not be used in combination, see '%s -h' for details\n", argv[0]);
    exit (FIXIT_TIFF_CMDLINE_ARGUMENTS_ERROR);
  }
  if (NULL == infilename) {
    fprintf (stderr, "You need to specify infile with '-i filename', see '%s -h' for details\n", argv[0]);
    exit (FIXIT_TIFF_MISSED_INFILE);
  }
  if ((FLAGGED!=flag_substitute_only) && (FLAGGED!=flag_check_only)) {
    if (NULL == outfilename) {
      fprintf (stderr, "You need to specify outfile with '-o outfilename', see '%s -h' for details\n", argv[0]);
      exit (FIXIT_TIFF_MISSED_OUTFILE);
    }
  }
  if (FLAGGED == flag_be_verbose) printf ("infile='%s', outfile='%s'\n", infilename, outfilename);
  /* check only if file is valid */
  if (FLAGGED == flag_check_only) {
    /*
     exit (
        check_required(infilename));
        */

    exit (
        check_required(infilename) ||
        check_baseline(infilename) || 
        check_datetime(infilename) 
        );

  }
  /* try to fix tag order */
  if (FLAGGED == flag_tagorder) {
    copy_file (infilename, outfilename);
    if (FLAGGED == flag_be_verbose) printf ("tagorder cleanup infile='%s', outfile='%s'\n", infilename, outfilename);
    cleanup_tagorder(outfilename);
        exit (FIXIT_TIFF_IS_CORRECTED);
  }
  /*  try to fix ICC header */
  if (FLAGGED == flag_clean_icc) {
    copy_file (infilename, outfilename);
    if (FLAGGED == flag_be_verbose) printf ("ICC header cleanup infile='%s', outfile='%s'\n", infilename, outfilename);
    cleanup_icc_header(outfilename);
    exit (FIXIT_TIFF_IS_CORRECTED);
  }

  if (UNFLAGGED != clean_tag) { /* explicite correction via source target, clean given tag */
    copy_file (infilename, outfilename);
    if (FLAGGED == flag_be_verbose) printf ("tag cleanup infile='%s', outfile='%s'\n", infilename, outfilename);
    cleanup_tag(outfilename, clean_tag);
    exit (FIXIT_TIFF_IS_CORRECTED);
  }
  /* inplace correction */
  if (FLAGGED == flag_substitute_only) { /* inplace correction */
    cleanup_datetime(infilename);
    if (FLAGGED == flag_baseline_cleanup) { /* baseline tiff cleanup */
      cleanup_baseline(infilename);
    }
  } else { /* explicite correction via source target */
    copy_file (infilename, outfilename);
    cleanup_datetime(outfilename);
    if (FLAGGED == flag_baseline_cleanup) { /* baseline tiff cleanup */
      if (FLAGGED == flag_be_verbose) printf ("baseline cleanup infile='%s', outfile='%s'\n", infilename, outfilename);
      cleanup_baseline(outfilename);
    }
  }
  exit (FIXIT_TIFF_IS_CORRECTED);
}

