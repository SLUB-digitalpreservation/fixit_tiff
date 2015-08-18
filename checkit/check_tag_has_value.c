#include "check.h"
#include "../fixit/fixit_tiff.h"
#include <pcre.h>
/* #define DEBUG */



ret_t check_tag_has_value_quiet(TIFF* tif, tag_t tag, unsigned int value) {
  tifp_check( tif)
  TIFFDataType datatype =  TIFFGetRawTagType( tif, tag );
  switch (datatype) {
    case TIFF_LONG: {
                      return check_tag_has_u32value(tif, tag, value);
                      break;
                    }
    case TIFF_SHORT: {
                       return check_tag_has_u16value(tif, tag, value);
                       break;
                     }
    case TIFF_RATIONAL: {
                          return check_tag_has_fvalue(tif, tag, value);
                          break;
                        }
    default: /*  none */
                        tif_returns("tag %u (%s) should have values of type long, short or float, but was:%i\n", tag, TIFFTagName(tif, tag), datatype);
  }
}

ret_t check_tag_has_value(TIFF* tif, tag_t tag, unsigned int value) {
  printf("check if tag %u (%s) has value %u\n", tag, TIFFTagName(tif, tag), value);
  ret_t ret = check_tag_has_value_quiet( tif, tag, value);
  if (ret.returncode == 0) {
        return ret;
  } else {
        tif_fails(ret.returnmsg);
  }
}

ret_t check_tag_has_value_matching_regex(TIFF* tif, tag_t tag, const char * value) {
  printf("check if tag %u (%s) has value matching regex '%s'\n", tag, TIFFTagName(tif, tag), value);
  tifp_check( tif)
    TIFFDataType datatype =  TIFFGetRawTagType( tif, tag );
  switch (datatype) {
    case TIFF_ASCII: {
                       char * val;
                       uint32 count;
                       int found=TIFFGetField(tif, tag, &val, &count);
                       if (1 == found) {
                         count = strlen( val);
#define OVECCOUNT 30    /* should be a multiple of 3 */
                         pcre *re;
                         int erroffset;
                         const char * errorcode;
                         re = pcre_compile(
                             value,                /* the pattern */
                             0,                      /* default options */
                             &errorcode,             /* for error code */
                             &erroffset,             /* for error offset */
                             NULL);                  /* no compile context */
                         if (NULL != re) {

                           int ovector[OVECCOUNT];
                           int rc = pcre_exec( re, NULL, val, count, 0,PCRE_NOTEMPTY , ovector, OVECCOUNT);
#ifdef DEBUG
                           printf("tag %s with count=%d and val='%s' -> rc=%d\n", TIFFTagName(tif, tag), count, val, rc);
#endif
                           if (rc >= 0 ) {
                             ret_t res;
                             res.returnmsg=NULL;
                             res.returncode=0;
                             return res;
                           } else {
                             switch(rc) {
                               case PCRE_ERROR_NOMATCH: 
                                 tif_fails("tag %u with value '%s' no match to given regex '%s'\n", tag, val, value);
                                 break;
                                 /*
                                    Handle other special cases if you like
                                    */
                               default: 
                                 tif_fails("tag %u with value '%s' called regex '%s' with matching error %d\n", tag, val, value, rc); 
                                 break;
                             }
                         }
                         } else {
                           tif_fails("regex '%s' compile error: %s at offset: %i\n",value, errorcode, erroffset);
                         }
                       } else {
                         tif_fails("tag %u should exist, because defined\n", tag);
                       }
                     }
    default:  /*  none */
                     tif_fails("tag %u (%s) should have value of type ASCII, but was:%i\n", tag, TIFFTagName(tif, tag), datatype);
  }
}

ret_t check_tag_has_valuelist(TIFF* tif, tag_t tag, int count, unsigned int * values) {
  printf("check if tag %u (%s) has these %i-values", tag, TIFFTagName(tif, tag), count);
  tifp_check( tif)
  int i;
  unsigned int * p = values;
  for (i=0; i< count; i++) {
    printf (", %i", *p);
    p++;
  }
  printf("\n");
  TIFFDataType datatype =  TIFFGetRawTagType( tif, tag );
  switch (datatype) {
    case TIFF_LONG: { 
                      p = values;
                      ret_t res;
                      for (i=0; i< count; i++) {
#ifdef DEBUG
                        printf("### value = %u", *p);
#endif
                        res = check_tag_has_u32value(tif, tag, *p);
                        if (res.returncode != 0) return res;
                        p++;
                      }
                      return res;
                      break;
                    }
    case TIFF_SHORT: {
                       p = values;
                       ret_t res;
                       for (i=0; i< count; i++) {
#ifdef DEBUG
                         printf("### value = %u", *p);
#endif
                         res = check_tag_has_u16value(tif, tag, *p);
                         if (res.returncode != 0) return res;
                         p++;
                       }
                       return res;
                       break;
                     }
    case TIFF_RATIONAL: {
                          p = values;
                          ret_t res;
                          for (i=0; i< count; i++) {
#ifdef DEBUG
                            printf("### value = %u", *p);
#endif
                            res = check_tag_has_fvalue(tif, tag, *p);
                            if (res.returncode != 0) return res;
                            p++;
                          }
                          return res;
                          break;
                        }
    default: /*  none */
                        tif_fails("tag %u (%s) should have values of type long, short or float, but was:%i\n", tag, TIFFTagName(tif, tag), datatype);
  }

}


