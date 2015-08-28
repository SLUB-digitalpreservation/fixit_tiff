
#include "../check.h"
#include "../check_helper.h"
/* #define DEBUG */

/* checks if TIF has a specified tag */
ret_t check_tag_has_valid_type(TIFF* tif, tag_t tag) {
  printf("check if tag %u (%s) has valid type\n", tag, TIFFTagName(tif, tag));
  tifp_check( tif)
  TIFFDataType datatype =  TIFFGetRawTagType( tif, tag );
#ifdef DEBUG
  printf("### found: value=%i data=%p \n",val, data);
  printf("### datatype=%i \n", datatype);
#endif
  int res = 0;
  switch (tag) {
    case TIFFTAG_ARTIST:            res=(datatype  ==  TIFF_ASCII); break;
    case TIFFTAG_BADFAXLINES:       res=(datatype  ==  TIFF_LONG); break;
    case TIFFTAG_BITSPERSAMPLE:     res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_CELLLENGTH:        res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_CELLWIDTH:         res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_COLORMAP:          res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_COMPRESSION:       res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_COPYRIGHT:         res=(datatype  ==  TIFF_ASCII); break;
    case TIFFTAG_DATETIME:          res=(datatype  ==  TIFF_ASCII); break;
    case TIFFTAG_DOCUMENTNAME:      res=(datatype  ==  TIFF_ASCII); break;
    case TIFFTAG_DOTRANGE:          res=(datatype  ==  TIFF_BYTE) || (datatype ==  TIFF_SHORT);; break;
    case TIFFTAG_EXTRASAMPLES:      res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_FILLORDER:         res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_FREEBYTECOUNTS:    res=(datatype  ==  TIFF_LONG); break;
    case TIFFTAG_FREEOFFSETS:       res=(datatype  ==  TIFF_LONG); break;
    case TIFFTAG_GRAYRESPONSECURVE: res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_GRAYRESPONSEUNIT:  res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_HALFTONEHINTS:     res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_HOSTCOMPUTER:      res=(datatype  ==  TIFF_ASCII); break;
    case TIFFTAG_ICCPROFILE:        res=(datatype  ==  TIFF_UNDEFINED); break;		
    case TIFFTAG_IMAGEDEPTH:        res=(datatype  ==  TIFF_LONG); break;
    case TIFFTAG_IMAGEDESCRIPTION:  res=(datatype  ==  TIFF_ASCII); break;
    case TIFFTAG_IMAGELENGTH:       res=(datatype  ==  TIFF_LONG) || (datatype ==  TIFF_SHORT);; break;
    case TIFFTAG_IMAGEWIDTH:        res=(datatype  ==  TIFF_LONG) || (datatype ==  TIFF_SHORT);; break;
    case TIFFTAG_INKNAMES:          res=(datatype  ==  TIFF_ASCII); break;
    case TIFFTAG_INKSET:            res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_JPEGPROC:          res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_MAKE:              res=(datatype  ==  TIFF_ASCII); break;
    case TIFFTAG_MAXSAMPLEVALUE:    res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_MINSAMPLEVALUE:    res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_MODEL:             res=(datatype  ==  TIFF_ASCII); break;
    case TIFFTAG_NUMBEROFINKS:      res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_ORIENTATION:       res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_OSUBFILETYPE:      res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_PAGENUMBER:        res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_PHOTOMETRIC:       res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_PLANARCONFIG:      res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_PREDICTOR:         res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_PRIMARYCHROMATICITIES:  res=(datatype  ==  TIFF_RATIONAL); break;
    case TIFFTAG_REFERENCEBLACKWHITE:    res=(datatype  ==  TIFF_LONG); break;
    case TIFFTAG_RESOLUTIONUNIT:    res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_ROWSPERSTRIP:      res=(datatype  ==  TIFF_LONG) || (datatype ==  TIFF_SHORT); break;
    case TIFFTAG_SAMPLEFORMAT:      res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_SAMPLESPERPIXEL:   res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_SMAXSAMPLEVALUE:   res=(datatype  == TIFF_ANY); break;
    case TIFFTAG_SMINSAMPLEVALUE:   res=(datatype  == TIFF_ANY); break;
    case TIFFTAG_SOFTWARE:          res=(datatype  ==  TIFF_ASCII); break;
    case TIFFTAG_STRIPBYTECOUNTS:   res=(datatype  ==  TIFF_LONG) || (datatype ==  TIFF_SHORT); break;
    case TIFFTAG_STRIPOFFSETS:      res=(datatype  ==  TIFF_LONG) || (datatype ==  TIFF_SHORT); break;
    case TIFFTAG_SUBFILETYPE:       res=(datatype  ==  TIFF_LONG); break;
    case TIFFTAG_T4OPTIONS:         res=(datatype  ==  TIFF_LONG); break;
    case TIFFTAG_T6OPTIONS:         res=(datatype  ==  TIFF_LONG); break;
    case TIFFTAG_TARGETPRINTER:     res=(datatype  ==  TIFF_ASCII); break;
    case TIFFTAG_THRESHHOLDING:     res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_TILEBYTECOUNTS:    res=(datatype  ==  TIFF_LONG) || (datatype ==  TIFF_SHORT); break;
    case TIFFTAG_TILELENGTH:        res=(datatype  ==  TIFF_LONG) || (datatype ==  TIFF_SHORT); break;
    case TIFFTAG_TILEOFFSETS:       res=(datatype  ==  TIFF_LONG); break;
    case TIFFTAG_TILEWIDTH:         res=(datatype  ==  TIFF_LONG) || (datatype ==  TIFF_SHORT); break;
    case TIFFTAG_TRANSFERFUNCTION:  res=(datatype  ==  TIFF_SHORT); break;
    case TIFFTAG_WHITEPOINT:        res=(datatype  ==  TIFF_RATIONAL); break;
    case TIFFTAG_XRESOLUTION:       res=(datatype  ==  TIFF_RATIONAL); break;
    case TIFFTAG_YRESOLUTION:       res=(datatype  ==  TIFF_RATIONAL); break;
    default: res = 1;
  }
  if (!res) {
    tif_fails("tag %u (%s) has incorrect type: %i\n", tag,TIFFTagName(tif, tag), datatype);
  } else {
    ret_t res;
    res.returnmsg=NULL;
    res.returncode=0;
    return res;
  }
  /* we check only count, because we evaluate only int-values */
}

