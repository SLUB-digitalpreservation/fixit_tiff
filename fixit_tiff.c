/* fixes broken TIFF Files
 * 
 * fixes invalid DateTime-field in Baseline-TIFFs,
 * based on http://www.awaresystems.be/imaging/tiff/tifftags/baseline.html
 *
 * author: Andreas Romeyke, 2013
 * licensed under conditions of libtiff 
 */


#include "fixit_tiff.h"

#define count_of_baselinetags 36
const static uint32 baselinetags[count_of_baselinetags]={
  TIFFTAG_SUBFILETYPE,
  TIFFTAG_OSUBFILETYPE,
  TIFFTAG_IMAGEWIDTH,
  TIFFTAG_IMAGELENGTH,
  TIFFTAG_BITSPERSAMPLE,
  TIFFTAG_COMPRESSION,
  TIFFTAG_PHOTOMETRIC,
  TIFFTAG_THRESHHOLDING,
  TIFFTAG_CELLWIDTH,
  TIFFTAG_CELLLENGTH,
  TIFFTAG_FILLORDER,
  TIFFTAG_IMAGEDESCRIPTION,
  TIFFTAG_MAKE,
  TIFFTAG_MODEL,
  TIFFTAG_STRIPOFFSETS,
  TIFFTAG_ORIENTATION,
  TIFFTAG_SAMPLESPERPIXEL,
  TIFFTAG_ROWSPERSTRIP,
  TIFFTAG_STRIPBYTECOUNTS,
  TIFFTAG_MINSAMPLEVALUE,
  TIFFTAG_MAXSAMPLEVALUE,
  TIFFTAG_XRESOLUTION,
  TIFFTAG_YRESOLUTION,
  TIFFTAG_PLANARCONFIG,
  TIFFTAG_FREEOFFSETS,
  TIFFTAG_FREEBYTECOUNTS,
  TIFFTAG_GRAYRESPONSEUNIT,
  TIFFTAG_GRAYRESPONSECURVE,
  TIFFTAG_RESOLUTIONUNIT,
  TIFFTAG_SOFTWARE,
  TIFFTAG_DATETIME,
  TIFFTAG_ARTIST,
  TIFFTAG_HOSTCOMPUTER,
  TIFFTAG_COLORMAP,
  TIFFTAG_EXTRASAMPLES,
  TIFFTAG_COPYRIGHT
};

/** help function */
void help () {
  printf ("fixit_tiff\n");
  printf ("call it with:\n");
  printf ("\tfixit_tiff [-h|-c|-s|-b] -i <infile> [-o <outfile>]\n");
  printf ("\nwhere <infile> is the possibly broken file\n");
  printf ("and <outfile> is the name of the corrected file\n");
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
  while ((c = getopt (argc, argv, "s::cbq::hi:o:")) != -1) {
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
           case '?': /* something goes wrong */
             if (optopt == 'i' || optopt == 'o')
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
    exit (check_datetime(infilename) || check_baseline(infilename));
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

