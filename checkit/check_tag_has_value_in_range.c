#include "check.h"
#include "../fixit/fixit_tiff.h"
#include "../fixit/tiff_helper.h"

/* 
#define DEBUG
*/

ret_t check_tag_has_value_in_range(TIFF* tif, tag_t tag, unsigned int a, unsigned int b) {
  printf("check if tag %u (%s) has value in range %u - %u\n", tag, TIFFTagName(tif, tag), a, b);
  tifp_check( tif)
  if (a > b) { unsigned int c=a; a=b; b=c; }
  TIFFDataType datatype =  TIFFFieldDataType(TIFFFieldWithTag(tif, tag));
  switch (datatype) {
    case TIFF_LONG: {
                      uint32 val;
                      int found=TIFFGetField(tif, tag, &val);
                      if (1 == found) {
                        if ((val >= a && val <= b )) { 
                          ret_t res;
                          res.returnmsg=NULL;
                          res.returncode=0;
                          return res;

                        } else {
                          tif_fails("tag %u should have value in range %u - %u, but have count/value=%u\n", tag, a, b, val);
                        }
                      } else {
                        tif_fails("tag %u should exist, because defined\n", tag);
                      }

                      break;
                    }
    case TIFF_SHORT: {
                       uint16 val;
                      int found=TIFFGetField(tif, tag, &val);
                      if (1 == found) {
                        if ((val >= a && val <= b )) { 
                          ret_t res;
                          res.returnmsg=NULL;
                          res.returncode=0;
                          return res;

                        } else {
                          tif_fails("tag %u should have value in range %u - %u, but have count/value=%u\n", tag, a, b, val);
                        }
                      } else {
                        tif_fails("tag %u should exist, because defined\n", tag);
                      }

                      break;
                     }
    case TIFF_RATIONAL: {
                       float val;
                      int found=TIFFGetField(tif, tag, &val);
                      if (1 == found) {
                        if ((val >= a && val <= b )) { 
                          ret_t res;
                          res.returnmsg=NULL;
                          res.returncode=0;
                          return res;

                        } else {
                          tif_fails("tag %u should have value in range %u - %u, but have count/value=%f\n", tag, a, b, val);
                        }
                      } else {
                        tif_fails("tag %u should exist, because defined\n", tag);
                      }

                      break;
                        }
    default: /*  none */
                        tif_fails("tag %u should have values of type long, short or float, but was:%i\n", tag, datatype);
  }

}
