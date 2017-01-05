/* fixes broken TIFF Files
 * 
 * fixes wrong tagtypes of tags in Baseline-TIFFs,
 * based on http://www.awaresystems.be/imaging/tiff/tifftags/baseline.html
 *
 * author: Andreas Romeyke, 2013-2017
 * licensed under conditions of libtiff 
 */


#include "fixit_tiff.h"
#include "check_baseline.h"
#include "tiff_helper.h"
#include <tiffio.h>
#include "tif_dir.h"
/** load a tiff, clean it up if needed, store tiff
 * @param filename filename which should be processed, repaired
 * only TAG 34665 (EXIFIFDOFFset) is supported yet
 */
int cleanup_tagtype(const char * filename, uint32 tag_to_fix ) {
	/* load file */
	TIFF* tif = TIFFOpen(filename, "r+");
	if (NULL == tif) {
		fprintf( stderr, "file '%s' could not be opened\n", filename);
		exit (FIXIT_TIFF_READ_PERMISSION_ERROR);
	};
	uint32 tag_counter=TIFFGetRawTagListCount(tif);
	uint32 tagidx;
	uint32 tags[tag_counter];
	for (tagidx=0; tagidx < tag_counter; tagidx++) {
		uint32 tag = TIFFGetRawTagListEntry( tif, tagidx );
		if (tag == tag_to_fix) {
			if (FLAGGED == flag_be_verbose) printf("tag to fix %i\n", tags[tagidx]);
                        const struct _TIFFField * fip = TIFFFieldWithTag(tif, tag);
                        if (NULL == fip) {
                          fprintf(stderr, "tagtype correction for tag %i fails, because requested tag does not exist (file '%s')\n", tag, filename);
                          exit (FIXIT_TIFF_CMDLINE_ARGUMENTS_ERROR);
                        }
                        switch (tag) {
                          case TIFFTAG_EXIFIFD:
                            {
                              uint16 found = fip->field_type;
                              if (found != TIFF_LONG) {
                                uint16 newtype = TIFF_LONG;
                                printf("found fieldtype=%i for tag=%i, try to repair with type=%i\n", found, tag, newtype);
                                /*  via TIFFGetRawTagListEntry we have the tag
                                 *  read, the next 2 bytes are the type */
                                int fd = TIFFFileno( tif);
                                write(fd, &newtype, sizeof( uint16));
                                close(fd);
                              }
                              break;
                            }
                          default: 
                            fprintf(stderr, "tagtype correction for tag %i not supported yet (file '%s')\n", tag, filename);
                            exit (FIXIT_TIFF_CMDLINE_ARGUMENTS_ERROR);
                        }
                }
        }
        if (FLAGGED == flag_be_verbose) printf("After  correction\n-----------------\n");
	/* write data back, only if no flag_check_only is set */
	if  (FIXIT_TIFF_IS_VALID == check_baseline (filename)) return FIXIT_TIFF_IS_CORRECTED;
	else return FIXIT_TIFF_IS_CHECKED;
}

