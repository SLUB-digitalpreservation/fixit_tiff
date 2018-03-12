/* fixes broken TIFF Files
 * 
 * fixes unused tags in Baseline-TIFFs,
 * based on http://www.awaresystems.be/imaging/tiff/tifftags/baseline.html
 *
 * author: Andreas Romeyke, 2013
 * licensed under conditions of libtiff 
 */


#include "fixit_tiff.h"
#include "check_baseline.h"
#include "tiff_helper.h"
#include <tiffio.h>
#include <assert.h>
/** load a tiff, clean it up if needed, store tiff
 * @param filename filename which should be processed, repaired
 */
int cleanup_tag(const char * filename, uint32 tag_to_remove ) {
	/* load file */
	TIFF* tif = TIFFOpen(filename, "r+");
	if (NULL == tif) {
		fprintf( stderr, "file '%s' could not be opened\n", filename);
		exit (FIXIT_TIFF_READ_PERMISSION_ERROR);
	};
	uint32 tag_counter=TIFFGetRawTagListCount(tif);
	uint32 tagidx;
	uint32 tags[tag_counter];
  int fd = TIFFFileno( tif);
  off_t count_pos=lseek(fd, -2, SEEK_CUR);
  if (count_pos < 0) {
    perror("TIFF seek error in IFD0");
    exit(EXIT_FAILURE);
  }
  if (lseek(fd,2, SEEK_CUR) < 0) {
    perror("TIFF seek error in IFD0");
    exit(EXIT_FAILURE);
  }
	for (tagidx=0; tagidx < tag_counter; tagidx++) {
		uint32 tag = TIFFGetRawTagListEntry( tif, tagidx );
		if (tag == tag_to_remove) {
      if (FLAGGED == flag_be_verbose) printf("removed tag %i\n", tags[tagidx]);
      /*  via TIFFGetRawTagListEntry we have the tag read
       *  now we should seek 10 bytes forward and read the entire IFD block until (including) end
       *  then we should seek n*12 + 4 bytes backward and write
       *  the Block out.
       *  Also we rewind to IFD0 and write current count to
       *  count-1
       */
      off_t actual_pos = lseek(fd, -2, SEEK_CUR);
      if (actual_pos < 0) { 
        perror("TIFF seek error in IFD0");
        exit(EXIT_FAILURE);
      }
      off_t next_tag = lseek(fd, 12, SEEK_CUR); /*  12 bytes forward to next IFD */
      if (next_tag < 0) { 
        perror("TIFF seek error in IFD0");
        exit(EXIT_FAILURE);
      }
      int unread_tags = tag_counter - tagidx -1;
      assert (unread_tags >= 0);
      off_t last_tag = next_tag + 12*(tag_counter - tagidx) + 4; /* last tags and offset to next ifd */
      int bytes = last_tag - next_tag;
      char buf[ bytes ];
      if (read(fd, &buf, bytes) != bytes) {
        perror("TIFF read error in IFD0");
        exit(EXIT_FAILURE);
      }
      if (lseek(fd, actual_pos, SEEK_SET) != actual_pos) {
        perror("TIFF seek error in IFD0");
        exit(EXIT_FAILURE);
      }
      if (write(fd, &buf, bytes) != bytes) {
        perror("TIFF write error in IFD0");
        exit(EXIT_FAILURE);
      }
      /*  update count */
      if (lseek(fd, count_pos, SEEK_SET) != count_pos) {
        perror("TIFF seek error in IFD0");
        exit(EXIT_FAILURE);
      }
      uint16 count=0;
      if (read(fd, &count, 2) != 2) {
        perror("TIFF read error in IFD0");
        exit(EXIT_FAILURE);
      }
      if (lseek(fd, count_pos, SEEK_SET) != count_pos) {
        perror("TIFF seek error in IFD0");
        exit(EXIT_FAILURE);
      }
      //printf("found count=%i (0x%04x)\n", count, count);
      if (TIFFIsByteSwapped(tif))
        TIFFSwabShort(&count);
      count--;
      assert(count >= 0);
      if (TIFFIsByteSwapped(tif)) // Swab again to write correctly out
        TIFFSwabShort(&count);
      //printf("(-1)  count=%i (0x%04x)\n", count, count);
      if (write(fd, &count, 2) != 2) {
        perror("TIFF write error in IFD0");
        exit(EXIT_FAILURE);
      }
      if (0 != close(fd)) {
        perror("TIFF could not be closed");
        exit(EXIT_FAILURE);
      }
      goto EXIT;
      /* break; */
    }
  }
EXIT:
  if (FLAGGED == flag_be_verbose) printf("After  correction\n-----------------\n");
	if  (FIXIT_TIFF_IS_VALID == check_baseline (filename)) return FIXIT_TIFF_IS_CORRECTED;
	else return FIXIT_TIFF_IS_CHECKED;
}
/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 smarttab expandtab :*/
