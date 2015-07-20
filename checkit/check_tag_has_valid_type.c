
#include "check.h"
#include "../fixit/fixit_tiff.h"
#include "../fixit/tiff_helper.h"
#define DEBUG

/* checks if TIF has a specified tag */
int check_tag_has_valid_type(TIFF* tif, int tag) {
  printf("check if tag %i (%s) has valid type\n", tag, TIFFFieldName(TIFFFieldWithTag(tif, tag)));
  if (NULL == tif) {
    fprintf( stderr, "TIFF pointer is empty\n");
    tif_fails("TIFF pointer is empty\n");
    return - FIXIT_TIFF_READ_PERMISSION_ERROR;
  };
  uint16 val;
  void * data;
  int found=TIFFGetField(tif, tag, &val, data);
  if (1==found) { /* check va-list */
        TIFFDataType datatype =  TIFFFieldDataType(TIFFFieldWithTag(tif, tag));
        printf("### found: value=%i data=%p \n",val, data);
        printf("### datatype=%i\n", datatype);
        switch (tag) {
        case TIFFTAG_ARTIST:        return (datatype == TIFF_ASCII);
        case TIFFTAG_BADFAXLINES:   return (datatype == TIFF_LONG);
        case TIFFTAG_BITSPERSAMPLE: return (datatype == TIFF_SHORT);
        case TIFFTAG_COLORMAP:      return (datatype == TIFF_SHORT);
        case TIFFTAG_COMPRESSION:   return (datatype == TIFF_SHORT);
        case TIFFTAG_COPYRIGHT:     return (datatype == TIFF_ASCII);
        case TIFFTAG_DATETIME:      return (datatype == TIFF_ASCII);
	case TIFFTAG_DOCUMENTNAME:  return (datatype == TIFF_ASCII);
        case TIFFTAG_FILLORDER:     return (datatype == TIFF_SHORT);
        case TIFFTAG_HOSTCOMPUTER:  return (datatype == TIFF_ASCII);
        case TIFFTAG_ICCPROFILE:    return (datatype == TIFF_IFD);		
        case TIFFTAG_IMAGEDEPTH:    return (datatype == TIFF_LONG);
        case TIFFTAG_IMAGEDESCRIPTION: return (datatype == TIFF_ASCII);
	case TIFFTAG_IMAGELENGTH:   return (datatype == TIFF_LONG);
        case TIFFTAG_IMAGEWIDTH:    return (datatype == TIFF_LONG);
        case TIFFTAG_MAKE:          return (datatype == TIFF_ASCII);
        case TIFFTAG_MAXSAMPLEVALUE:return (datatype == TIFF_SHORT);
        case TIFFTAG_MINSAMPLEVALUE:return (datatype == TIFF_SHORT);
        case TIFFTAG_MODEL:         return (datatype == TIFF_ASCII);
        case TIFFTAG_ORIENTATION:   return (datatype == TIFF_SHORT);
        case TIFFTAG_PHOTOMETRIC:   return (datatype == TIFF_SHORT);
        case TIFFTAG_RESOLUTIONUNIT:return (datatype == TIFF_SHORT);
        case TIFFTAG_ROWSPERSTRIP:  return (datatype == TIFF_LONG);
	case TIFFTAG_SAMPLEFORMAT:  return (datatype == TIFF_SHORT);
        case TIFFTAG_SAMPLESPERPIXEL: return (datatype == TIFF_SHORT);
        case TIFFTAG_SOFTWARE:      return (datatype == TIFF_ASCII);
        case TIFFTAG_XRESOLUTION:   return (datatype == TIFF_FLOAT);
        case TIFFTAG_YRESOLUTION:   return (datatype == TIFF_FLOAT);
        default: return 0;
        }
        /* we check only count, because we evaluate only int-values */
  } else { /* tag not defined */ 
    tif_fails("tag %i should exist, because defined\n", tag);
    return 1;
  }
  return 1;
}

