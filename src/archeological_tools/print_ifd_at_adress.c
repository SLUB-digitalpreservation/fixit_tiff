/*
 * =====================================================================================
 *
 *       Filename:  print_ifds_at_adress.c
 *
 *    Description:  print a IFD on given adress
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

void print_ifd_at_adress (const char * infile, uint32 adress) {
  /* open infile */
  int fd_in = open(infile, O_RDWR );
  if (! fd_in) {
    perror("could not open input file");
  }
  uint32 filesize = lseek( fd_in, 0L, SEEK_END);
  if (adress > filesize) {
    fprintf(stderr, "Adress 0x%0x is outside of filesize (0x%0x", adress, filesize);
    exit(0);
  }
  lseek( fd_in, adress, SEEK_SET);
  uint16 countoftags = 0;
  if (read(fd_in, &countoftags, 2) != 2) perror ("input file not readable, countoftags");
  if (
      (countoftags > 4) &&
      ((countoftags *12 + adress)+4 < filesize )
     ) {
    printf("IFD at adress 0x%x\n", adress);
    uint32 nextifd = 0;
    lseek( fd_in, 12*countoftags, SEEK_CUR);
    if (read(fd_in, &nextifd, 4) != 4) perror("input file not readable, nextifd");
    if (0 == nextifd) { printf("no next IFD found, all fine\n"); }
    else { printf("IFD points to next IFD = 0x%0x\n", nextifd); }
    printf("found %i Tags\n", countoftags);
    lseek( fd_in, adress 
        + 2 /* countoftags */
        , SEEK_SET);
    for (int i = 1; i <countoftags; i++) {
      uint16 tagid = 0;
      uint16 fieldtype = 0;
      uint32 count = 0;
      uint32 value_or_offset = 0;
      if (read(fd_in, &tagid, 2) != 2) perror ("input file not readable, tagids");
      printf("\tfound TAG 0x%0x (%u), ", tagid, tagid);
      if (read(fd_in, &fieldtype, 2) != 2) perror ("input file not readable, fieldtypes");
      printf("type=%2i, ", fieldtype);
      if (read(fd_in, &count, 4) != 4) perror ("input file not readable, count");
      printf("count=%32u, ", count);
      if (read(fd_in, &value_or_offset, 4) != 4) perror ("input file not readable, count");
      printf("value/offset=%32u (0x%08x)\n", value_or_offset, value_or_offset);
    }
  }
  /* close infile */
  close( fd_in );
}


/** help function */
void help () {
  printf ("print_ifd_at_adress\n");
  printf ("call it with:\n");
  printf ("\tprint_ifd_at_adress [-h] -i <infile> [-a <adress>]\n");
  printf ("\t where <infile> is the (broken) TIFF file\n");
}

int main (int argc, char * argv[]) {
  const char *infilename = NULL;
  uint32 address=0;
  int c;
  while ((c = getopt (argc, argv, "hi:a:")) != -1) {
    switch (c)
    {
      case 'h': /* help */
        help();
        exit (0);
      case 'i': /* expects infile */
        infilename=optarg;
        break;
      case 'a': /* expects outfile */
        printf("'%s'\n", optarg);
        sscanf(optarg, "0x%x", &address);
        break;
      default:
        abort();
    }
  }

  if (NULL == infilename) {
    fprintf (stderr, "You need to specify infile with '-i filename', see '%s -h' for details\n", argv[0]);
    exit (FIXIT_TIFF_MISSED_INFILE);
  }

  if (0 == address) {
    fprintf (stderr, "You need to specify address with '-a 0xff', see '%s -h' for details\n", argv[0]);
    exit (-1);
  }
  print_ifd_at_adress( infilename, address);
}
