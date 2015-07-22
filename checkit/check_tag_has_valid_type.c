
#include "check.h"
#include "../fixit/fixit_tiff.h"
#include "../fixit/tiff_helper.h"
/* #define DEBUG */

/* checks if TIF has a specified tag */
ret_t check_tag_has_valid_type(TIFF* tif, int tag) {
  printf("check if tag %i (%s) has valid type\n", tag, TIFFFieldName(TIFFFieldWithTag(tif, tag)));
  if (NULL == tif) {
    fprintf( stderr, "TIFF pointer is empty\n");
    tif_fails("TIFF pointer is empty\n");
  };
  uint16 val;
  void * data;
  int found=TIFFGetField(tif, tag, &val, &data);
  if (1==found) { /* check va-list */
    TIFFDataType datatype =  TIFFFieldDataType(TIFFFieldWithTag(tif, tag));
#ifdef DEBUG
    printf("### found: value=%i data=%p \n",val, data);
    printf("### datatype=%i \n", datatype);
#endif
    int res = 0;
    switch (tag) {
      case TIFFTAG_ARTIST:            res=(datatype  ==  TIFF_ASCII); break;
      case TIFFTAG_BADFAXLINES:       res=(datatype  ==  TIFF_LONG); break;
      case TIFFTAG_BITSPERSAMPLE:     res=(datatype  ==  TIFF_SHORT); break;
      case TIFFTAG_COLORMAP:          res=(datatype  ==  TIFF_SHORT); break;
      case TIFFTAG_COMPRESSION:       res=(datatype  ==  TIFF_SHORT); break;
      case TIFFTAG_COPYRIGHT:         res=(datatype  ==  TIFF_ASCII); break;
      case TIFFTAG_DATETIME:          res=(datatype  ==  TIFF_ASCII); break;
      case TIFFTAG_DOCUMENTNAME:      res=(datatype  ==  TIFF_ASCII); break;
      case TIFFTAG_FILLORDER:         res=(datatype  ==  TIFF_SHORT); break;
      case TIFFTAG_HOSTCOMPUTER:      res=(datatype  ==  TIFF_ASCII); break;
      case TIFFTAG_ICCPROFILE:        res=(datatype  ==  TIFF_UNDEFINED); break;		
      case TIFFTAG_IMAGEDEPTH:        res=(datatype  ==  TIFF_LONG); break;
      case TIFFTAG_IMAGEDESCRIPTION:  res=(datatype  ==  TIFF_ASCII); break;
      case TIFFTAG_IMAGELENGTH:       res=(datatype  ==  TIFF_LONG) || (datatype ==  TIFF_SHORT);; break;
      case TIFFTAG_IMAGEWIDTH:        res=(datatype  ==  TIFF_LONG) || (datatype ==  TIFF_SHORT);; break;
      case TIFFTAG_MAKE:              res=(datatype  ==  TIFF_ASCII); break;
      case TIFFTAG_MAXSAMPLEVALUE:    res=(datatype  ==  TIFF_SHORT); break;
      case TIFFTAG_MINSAMPLEVALUE:    res=(datatype  ==  TIFF_SHORT); break;
      case TIFFTAG_MODEL:             res=(datatype  ==  TIFF_ASCII); break;
      case TIFFTAG_ORIENTATION:       res=(datatype  ==  TIFF_SHORT); break;
      case TIFFTAG_PHOTOMETRIC:       res=(datatype  ==  TIFF_SHORT); break;
      case TIFFTAG_RESOLUTIONUNIT:    res=(datatype  ==  TIFF_SHORT); break;
      case TIFFTAG_ROWSPERSTRIP:      res=(datatype  ==  TIFF_LONG) || (datatype ==  TIFF_SHORT); break;
      case TIFFTAG_SAMPLEFORMAT:      res=(datatype  ==  TIFF_SHORT); break;
      case TIFFTAG_SAMPLESPERPIXEL:   res=(datatype  ==  TIFF_SHORT); break;
      case TIFFTAG_SOFTWARE:          res=(datatype  ==  TIFF_ASCII); break;
      case TIFFTAG_STRIPBYTECOUNTS:   res=(datatype  ==  TIFF_LONG) || (datatype ==  TIFF_SHORT); break;
      case TIFFTAG_STRIPOFFSETS:      res=(datatype  ==  TIFF_LONG) || (datatype ==  TIFF_SHORT); break;
      case TIFFTAG_XRESOLUTION:       res=(datatype  ==  TIFF_RATIONAL); break;
      case TIFFTAG_YRESOLUTION:       res=(datatype  ==  TIFF_RATIONAL); break;

      default: res = 1;
    }
    if (!res) {
      tif_fails("tag %i has incorrect type: %i\n", tag, datatype);
    } else {
      ret_t res;
      res.returnmsg=NULL;
      res.returncode=0;
      return res;

    }
    /* we check only count, because we evaluate only int-values */
  } else { /* tag not defined */ 
    tif_fails("tag %i should exist, because defined\n", tag);
  }
}

