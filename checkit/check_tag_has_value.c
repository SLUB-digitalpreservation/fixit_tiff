#include "check.h"
#include <pcre.h>
#include <string.h>

/*
#define DEBUG
*/


ret_t check_tag_has_value_quiet(TIFF* tif, tag_t tag, unsigned int value) {
  tifp_check( tif)
    ifd_entry_t ifd_entry = TIFFGetRawIFDEntry(tif, tag);
  if (ifd_entry.count > 1)
    tif_returns("tag %u (%s) should have single value of type long, short or float, but has type:%i and count:%i\n", tag, TIFFTagName(tif, tag), ifd_entry.datatype, ifd_entry.count);
  switch (ifd_entry.datatype) {
    case TIFF_LONG: {
                      if (value != ifd_entry.data32) {
                        tif_returns("tag %u (%s), tagvalue %u differs from value %u (long)",  tag, TIFFTagName(tif, tag),  ifd_entry.data32, value); 
                      }
                      break;
                    }
    case TIFF_SHORT: {
                       if (value != ifd_entry.data16[0])  {
                         tif_returns("tag %u (%s), tagvalue %u differs from value %u (short)",  tag, TIFFTagName(tif, tag),  ifd_entry.data16[0], value); 
                       }
                       break;
                     }
    case TIFF_RATIONAL: {
                          if (value - (ifd_entry.data16[0] / ifd_entry.data16[1]) > 1) {
                            tif_returns("tag %u (%s), tagvalue %u/%u differs from value %u (rational)",  tag, TIFFTagName(tif, tag),  ifd_entry.data16[0], ifd_entry.data16[1], value); 
                            break;
                          }
                          default: /*  none */
                          tif_returns("tag %u (%s) should have single value of type long, short or float, but was datatype:%u\n", tag, TIFFTagName(tif, tag), ifd_entry.datatype);
                          break;

                        };
  };
  ret_t ret;
  ret.returncode=0; ret.returnmsg=NULL; return ret;
}

ret_t check_tag_has_value(TIFF* tif, tag_t tag, unsigned int value) {
  printf("check if tag %u (%s) has value %u\n", tag, TIFFTagName(tif, tag), value);
  ret_t ret = check_tag_has_value_quiet( tif, tag, value);
  if (ret.returncode == 0) {
        return ret;
  } else {
        tif_fails("%s", ret.returnmsg);
  }
}

ret_t check_tag_has_value_matching_regex(TIFF* tif, tag_t tag, const char * regex_string) {
  printf("check if tag %u (%s) has value matching regex '%s'\n", tag, TIFFTagName(tif, tag), regex_string);
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
                             regex_string,                /* the pattern */
                             0,                      /* default options */
                             &errorcode,             /* for error code */
                             &erroffset,             /* for error offset */
                             NULL);                  /* no compile context */
                         if (NULL != re) {

                           int ovector[OVECCOUNT];
                           int rc = pcre_exec( re, NULL, val, count, 0,PCRE_NOTEMPTY , ovector, OVECCOUNT);
#ifdef DEBUG
                           printf("tag %s with count=%d and value='%s' -> rc=%d\n", TIFFTagName(tif, tag), count, val, rc);
#endif
                           pcre_free( re );
                           if (rc >= 0 ) {
                             ret_t res;
                             res.returnmsg=NULL;
                             res.returncode=0;
                             return res;
                           } else {
                             switch(rc) {
                               case PCRE_ERROR_NOMATCH: 
                                 tif_fails("tag %u with value '%s' no match to given regex '%s'\n", tag, val, regex_string);
                                 break;
                                 /*
                                    Handle other special cases if you like
                                    */
                               default: 
                                 tif_fails("tag %u with value '%s' called regex '%s' with matching error %d\n", tag, val, regex_string, rc); 
                                 break;
                             }
                         }
                         } else {
                           tif_fails("regex '%s' compile error: %s at offset: %i\n",regex_string, errorcode, erroffset);
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
  unsigned int v[count];
  for (i=0; i< count; i++) {
    v[i] = *values;
    printf (", %i", v[i]);
    values++;
  }
  printf("\n");
  ifd_entry_t ifd_entry = TIFFGetRawIFDEntry(tif, tag);
  if (count != ifd_entry.count) {
    tif_fails("tag %u (%s) has %u values, but list has %u values\n", tag, TIFFTagName(tif, tag), ifd_entry.count, count);
  }
  ret_t res;
  res.returncode=0;
  res.returnmsg=NULL;
  switch (ifd_entry.datatype) {
    case TIFF_LONG: { 
                      /*  value */
                      if (ifd_entry.value_or_offset == is_value) {
                        for (i=0; i< count; i++) {
                          if (v[i] != ifd_entry.data32) {
                            tif_fails("tag %u (%s), tagvalue[%i]=%u differs from value=%u (long)\n",  tag, TIFFTagName(tif, tag), i, ifd_entry.data32, v[i]); 
                          }
                        }
                      }
                      /*  offset */
                      if (ifd_entry.value_or_offset == is_offset) {
                        offset_t offset = read_offsetdata(tif, ifd_entry.data32offset, count, ifd_entry.datatype);
                        uint32 * p = offset.data32p;
                        for (i=0; i< count; i++) {
                          if (TIFFIsByteSwapped(tif))
                            TIFFSwabLong(p);
#ifdef DEBUG
                          printf("OFFSET: v[%i]=%u p[%i]=%u\n", i,v[i],i,*p);
#endif
                          if (v[i] != *p) {
                            tif_fails("tag %u (%s), tagvalue[%i]=%u differs from value=%u (long offset) \n",  tag, TIFFTagName(tif, tag), i, *p, v[i]); 
                          }
                          p++;
                        }
                      }

                      return res;
                      break;
                    }
    case TIFF_SHORT: {
                      /*  value */
                       if (ifd_entry.value_or_offset == is_value) {
                         for (i=0; i< count; i++) {
                           int c = (v[i]) == (ifd_entry.data16[i]);
                           if (!c) {
                             tif_fails("tag %u (%s), tagvalue[%i]=%u differs from value[%i]=%u (short)\n",  tag, TIFFTagName(tif, tag), i, ifd_entry.data16[i], i, v[i]); 
                           }
                         } 
                       }
                       /*  offset */
                       if (ifd_entry.value_or_offset == is_offset) {
                         offset_t offset = read_offsetdata(tif, ifd_entry.data32offset, count, ifd_entry.datatype);
                         uint16 * p = offset.data16p;

                         for (i=0; i< count; i++) {
                           if (TIFFIsByteSwapped(tif))
                             TIFFSwabShort(p);
#ifdef DEBUG
                           printf("OFFSET: v[%i]=%u p[%i]=%u\n", i,v[i],i,*p);
#endif
                           if (v[i] != *p) {
                             tif_fails("tag %u (%s), tagvalue[%i]=%u differs from value=%u (short offset)\n",  tag, TIFFTagName(tif, tag), i, *p, v[i]); 
                           }
                           p++;
                         }
                       }

                       return res;
                       break;
                     }
    default: /*  none */
                     tif_fails("tag %u (%s) should have values of type long, short or float, but was:%u\n", tag, TIFFTagName(tif, tag), ifd_entry.datatype);
  }

}


