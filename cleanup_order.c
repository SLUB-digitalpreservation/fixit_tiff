/* fixes broken TIFF Files
 * 
 * fixes unused tags in Baseline-TIFFs,
 * based on http://www.awaresystems.be/imaging/tiff/tifftags/baseline.html
 *
 * author: Andreas Romeyke, 2013
 * licensed under conditions of libtiff 
 */

#include "fixit_tiff.h"
#include <tiffio.h>
#include <fcntl.h>

int tagcmpfunc (const void * a, const void * b) {
        uint8 * aa = (uint8 *) a;
        uint8 * ab = (uint8 *) b;
        uint16 taga = (*(aa+1) << 8) + *aa;
        uint16 tagb = (*(ab+1) << 8) + *ab;
        /* printf ("a:%i b:%i\n", taga, tagb); */
        return ( taga -tagb );
}

/* sort order of tifftags */
int cleanup_tagorder (const char * filename) {
  int fd = open( filename, O_RDWR );
  if (fd < 0) {
    return -1;
  } else {

    /* find header */
    /* seek the image file directory (bytes 4-7) */
    uint32 hdr;
    if (read(fd, &hdr, 4) != 4) perror ("TIFF Header read error");
    if (hdr != 0x002a4949) perror ("only little endian support yet");
    uint32 offset;
    if (read(fd, &offset, 4) != 4)
      perror ("TIFF Header read error");
    // printf("diroffset to %i (0x%04lx)\n", offset, offset);
    //printf("byte swapped? %s\n", (TIFFIsByteSwapped(tif)?"true":"false")); 
    /* read and seek to IFD address */
    lseek(fd, (off_t) offset, SEEK_SET);
    uint16 count;
    if (read(fd, &count, 2) != 2)
      perror ("TIFF Header read error2");
    /* printf( "tag_counter=%i\n", count); */
    /* read the old ifd entries */
    uint8 buffer[ count * 12];
    int i;
    if (read(fd, buffer, count * 12) != (count *12))
      perror ("TIFF IFD read error");
    /* ok, now sort buffer */
    qsort(buffer, count, sizeof(uint8) * 12, tagcmpfunc);
    /* print tags */
    /*
    for (i=0; i < count; i++) {
        uint8 * base=buffer+12*i;
        uint16 tag = (* (base+1) << 8) + *base;
        printf ("i:%i tag=%i\n", i, tag); 
    }
    */
    /* write data back */
    lseek(fd, (off_t) offset + 2, SEEK_SET);
    if (write(fd, buffer, count * 12) != (count *12))
      perror ("TIFF IFD write error");
  }
  return 0;
}
