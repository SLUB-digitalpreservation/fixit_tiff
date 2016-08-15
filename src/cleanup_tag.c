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
	for (tagidx=0; tagidx < tag_counter; tagidx++) {
		uint32 tag = TIFFGetRawTagListEntry( tif, tagidx );
		if (tag == tag_to_remove) {
			if (FLAGGED == flag_be_verbose) printf("removed tag %i\n", tags[tagidx]);
			TIFFUnsetField(tif, tag_to_remove);
			break;
		}

	}
	if (FLAGGED == flag_be_verbose) printf("After  correction\n-----------------\n");
	if (FLAGGED == flag_be_verbose) TIFFPrintDirectory(tif, stdout, TIFFPRINT_NONE);
	/* write data back, only if no flag_check_only is set */
	int written = TIFFWriteDirectory(tif); /*  HINT: if errorneous, use TIFFReWriteDirectory instead */
	if (1 != written) {
		fprintf(stderr, "something is wrong, tiffdir could not be written to file '%s'\n", filename);
		exit (FIXIT_TIFF_WRITE_ERROR);
	}
	TIFFClose(tif);
	if  (FIXIT_TIFF_IS_VALID == check_baseline (filename)) return FIXIT_TIFF_IS_CORRECTED;
	else return FIXIT_TIFF_IS_CHECKED;
}

