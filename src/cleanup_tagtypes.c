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

const char * _tagtype( uint16 tagtype ) {
  switch (tagtype) {
    case TIFF_NOTYPE: return "NOTYPE"; break;
    case TIFF_BYTE: return "BYTE"; break;
    case TIFF_ASCII: return "ASCII"; break;
    case TIFF_SHORT: return "SHORT"; break;
    case TIFF_LONG: return "LONG"; break;
    case TIFF_RATIONAL: return "RATIONAL"; break;
    case TIFF_SBYTE: return "SBYTE"; break;
    case TIFF_UNDEFINED: return "UNDEFINED"; break;
    case TIFF_SSHORT: return "SSHORT"; break;
    case TIFF_SLONG: return "SLONG"; break;
    case TIFF_SRATIONAL: return "SRATIONAL"; break;
    case TIFF_FLOAT: return "FLOAT"; break;
    case TIFF_DOUBLE: return "DOUBLE"; break;
    case TIFF_IFD: return "IFD"; break;
    case TIFF_LONG8: return "LONG8"; break;
    case TIFF_SLONG8: return "SLONG8"; break;
    case TIFF_IFD8: return "IFD8"; break;
    default: return "unknown type"; break;
  }
}


void _replace_type(TIFF * tif, uint32 tag, uint16 found, uint16 newtype) {
  if (TIFFIsByteSwapped(tif))
    TIFFSwabShort(&newtype);
  printf("found fieldtype=%s (%i) for tag=%i, try to repair with type=%s (%i)\n", _tagtype(found), found, tag, _tagtype(newtype), newtype);
  /*  via TIFFGetRawTagListEntry we have the tag
   *  read, the next 2 bytes are the type */
  int fd = TIFFFileno( tif);
  if (write(fd, &newtype, 2) != 2) {
    perror("TIFF write error in IFD0");
    exit(EXIT_FAILURE);
  }
  if (0 != close(fd)) {
    perror("TIFF could not be closed");
    exit(EXIT_FAILURE);
  }
}

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
              _replace_type(tif, tag, found, TIFF_LONG);
              goto EXIT;
            }
            break;
          }
        case TIFFTAG_RICHTIFFIPTC:
          {
            uint16 found = fip->field_type;
            if (found != TIFF_UNDEFINED && found != TIFF_BYTE) {
              _replace_type(tif, tag, found, TIFF_UNDEFINED);
              goto EXIT;
            }
            break;
          }
        default: 
          fprintf(stderr, "tagtype correction for tag %i not supported yet (file '%s')\n", tag, filename);
          exit (FIXIT_TIFF_CMDLINE_ARGUMENTS_ERROR);
      }
    }
  }
EXIT:
        if (FLAGGED == flag_be_verbose) printf("After  correction\n-----------------\n");
	if  (FIXIT_TIFF_IS_VALID == check_baseline (filename)) return FIXIT_TIFF_IS_CORRECTED;
	else return FIXIT_TIFF_IS_CHECKED;
}
/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 smarttab expandtab :*/
