#include <math.h>
#include "check.h"
#include "../fixit/fixit_tiff.h"
#include "../fixit/tiff_helper.h"

//------------------------------------------------------------------------------
ret_t check_tag_has_fvalue(TIFF*  tif, tag_t tag, float value)
{
  float val;
  int found=TIFFGetField(tif, tag, &val);
  if (1 == found) {
    if ( fabs(val - value) < 0.01 ) {
      ret_t res;
      res.returnmsg=NULL;
      res.returncode=0;
      return res;
    } else {
      tif_fails("tag %u should have value %f, but have count/value=%f\n", tag, value, val);
    }

  } else {
    tif_fails("tag %u should exist, because defined\n", tag);
  }
}

//------------------------------------------------------------------------------
ret_t check_tag_has_u16value(TIFF*  tif, tag_t tag, uint16 value)
{
  uint16 val;
  int found=TIFFGetField(tif, tag, &val);
  if (1 == found) {
    if ( val == value ) {  
      ret_t res;
      res.returnmsg=NULL;
      res.returncode=0;
      return res;
    } else {
      tif_fails("tag %u should have value %u, but have count/value=%u\n", tag, value, val);
    }

  } else {
    tif_fails("tag %u should exist, because defined\n", tag);
  }
}

//------------------------------------------------------------------------------
ret_t check_tag_has_u32value(TIFF*  tif, tag_t tag, uint32 value)
{
  uint32 val;
  int found=TIFFGetField(tif, tag, &val);
  if (1 == found) {
    if ( val == value )  {
      ret_t res;
      res.returnmsg=NULL;
      res.returncode=0;
      return res;
    } else {
      tif_fails("tag %u should have value %u, but have count/value=%u\n", tag, value, val);
    }

  } else {
    tif_fails("tag %u should exist, because defined\n", tag);
  }
}

const char * TIFFTagName( TIFF * tif, tag_t tag ) {
   const TIFFField* fieldp = TIFFFieldWithTag(tif, tag);
   if (NULL != fieldp) {
        return TIFFFieldName(fieldp);
   } else { return ("undefined tag"); }
}
