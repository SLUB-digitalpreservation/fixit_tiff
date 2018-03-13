/*
 * =====================================================================================
 *       Filename:  find_potential_IFD_offsets.c
 *
 *    Description:  uses heuristics to list all addresses which are candidates
 *                  for IFDs in given broken TIFF files
 *
 *       Compiler:  gcc
 *
 *         Author:  Andreas Romeyke, 2018
 *         licensed under conditions of libtiff
 *
 * =====================================================================================
 */
#include "fixit_tiff.h"
#include <fcntl.h>
#ifdef __unix__
      #include <unistd.h>
      #include <sys/stat.h>
      #include <sys/types.h>
#else
      /* #include <sys\stat.h> */
      #include <sys/stat.h>
#endif

/** help function */
void help () {
  printf ("find_potential_IFD_offsets\n");
  printf ("call it with:\n");
  printf ("\tfind_potential_IFD_offsets [-h] -i <infile> [-o <outfile>]\n");
  printf ("\t where <infile> is the (broken) TIFF file\n");
  printf ("\t and <outfile> is a text file which lists the addresses in\n");
  printf ("\t sedecimal notation and also report which criteria match\n");
}

void scan_file_for_ifds(const char * infile, const char * outfile) {
  /* open infile */
  int fd_in = open(infile, O_RDWR );
  if (! fd_in) {
    perror("could not open input file");
  }
  /* open outfile */
  FILE *fd_out = fopen(outfile, "w");
  if(fd_out == NULL) {
    perror("could not open output file");
  }
  fprintf( fd_out, "#address,weight,is_sorted,has_required_baseline\n");

  uint32 filesize = lseek( fd_in, 0L, SEEK_END);
  /* for each odd address, do */
  for (uint32 address = 8; address < filesize; address+=2) {
    if (address % 1024 == 0) {
      printf ("\r%li %%", (uint64) 100*address/filesize);
    }
    lseek( fd_in, address, SEEK_SET);
    /* check if "count of tags" is greater 4 (hard criteria) */
    uint16 countoftags = 0;
    if (read(fd_in, &countoftags, 2) != 2) perror ("input file not readable, countoftags");
    if (
        (countoftags > 4) &&
        ((countoftags *12 + address)+4 < filesize )
        ) {
      uint32 nextifd = 0;
      lseek( fd_in, 12*countoftags, SEEK_CUR);
      if (read(fd_in, &nextifd, 4) != 4) perror("input file not readable, nextifd");
      if (nextifd % 2 == 0) {
//printf("0x%0x countoftags=%i nextifd=%0x\n", address, countoftags, nextifd);
        /* check for each "tag" if fieldtype is in range 1..18 (in original 12, but libtiff supports 18) (hard criteria) */
        int tagids_ok = 0;
        int fieldtypes_ok = 0;
        int has_baselinetags[count_of_required_baselinetags];
        for (int i = 0; i < count_of_required_baselinetags; i++) {
          has_baselinetags[i]=0;
        }
        int sorted = 0;
        lseek( fd_in, address 
            + 2 /* countoftags */
            , SEEK_SET);
        int last_tagid = 0;
        for (int i = 1; i <countoftags; i++) {
          uint16 tagid = 0;
          uint16 fieldtype = 0;
          if (i>1) {
            lseek( fd_in, 4 + 4, SEEK_CUR );
          }
          if (read(fd_in, &tagid, 2) != 2) perror ("input file not readable, tagids");
          if (tagid < 0x00fe) { tagids_ok = 1; break; }
          if (last_tagid >= tagid) { sorted =1; }
          last_tagid=tagid;
          for (int j = 0; j < count_of_required_baselinetags; j++) {
            if (tagid == required_baselinetags[j]) {
              has_baselinetags[j]++;
            }
          }
          if (read(fd_in, &fieldtype, 2) != 2) perror ("input file not readable, fieldtypes");
          if (fieldtype < 1 || fieldtype > 18) { fieldtypes_ok = 1; break; }
        }
        if (tagids_ok == 0 && fieldtypes_ok == 0) {
          /* do some soft checks if possible */
          int weight = 0;
          /*
           *   check if required tags are present (soft criteria)
           *   check for each "tag" if tagid is sorted (soft criteria)
           *   check for each "tag" if value/offset is a offset and this offset is odd
           *     and point to address within file (soft criteria)
           */

          /* check if required tags are present (soft criteria) */
          int all_baselinetags = 0;
          for (int j = 0; j < count_of_required_baselinetags; j++) {
            if (has_baselinetags[j] == 1) {
              all_baselinetags++;
            }
          }
          if (all_baselinetags == count_of_required_baselinetags) {
            weight++;
          }
          /* check for each "tag" if tagid is sorted (soft criteria)  */
          if (sorted == 0) { weight++; }
          /* print result */
          fprintf( fd_out, "0x%04x,%i,%c,%c\n",
              address,
              weight,
              (sorted==0)?'y':'n',
              all_baselinetags==count_of_required_baselinetags?'y':'n'
          );
        }
      }
    }
  }
  /* close outfile */
  fclose( fd_out );
  /* close infile */
  close( fd_in );
}

int main (int argc, char * argv[]) {
  const char *infilename = NULL;
  const char *outfilename= NULL;
  int c;
  while ((c = getopt (argc, argv, "hi:o:")) != -1) {
    switch (c)
    {
      case 'h': /* help */
        help();
        exit (0);
      case 'i': /* expects infile */
        infilename=optarg;
        break;
      case 'o': /* expects outfile */
        outfilename=optarg;
        break;
      default:
        abort();
    }
  }

  if (NULL == infilename) {
    fprintf (stderr, "You need to specify infile with '-i filename', see '%s -h' for details\n", argv[0]);
    exit (FIXIT_TIFF_MISSED_INFILE);
  }

  if (NULL == outfilename) {
    fprintf (stderr, "You need to specify outfile with '-o filename', see '%s -h' for details\n", argv[0]);
    exit (FIXIT_TIFF_MISSED_OUTFILE);
  }
  scan_file_for_ifds( infilename, outfilename);

}
