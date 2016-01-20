/* rule based checks if given TIFF is a specific baseline TIFF
 * 
 * author: Andreas Romeyke, 2015
 * licensed under conditions of libtiff 
 * (see http://libtiff.maptools.org/misc.html)
 *
 */

#include "../check.h"
#include "../check_helper.h"
/*
#define DEBUG
*/

ret_t check_tag_has_some_of_these_values(TIFF* tif, tag_t tag, int count, unsigned int * values) {
  printf("check if tag %u (%s) has some of these %i-values", tag, TIFFTagName(tif, tag), count);
   tifp_check( tif)

  int i;
  unsigned int * p = values;
  for (i=0; i< count; i++) {
    printf (", %u", *p);
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
                        if (res.returncode == 0) return res;
                        p++;
                      }
                      uint32 val;
                      TIFFGetField(tif, tag, &val);
                      tif_fails("tag %u (%s) does not have some of expected values (but have:%u)\n", tag, TIFFTagName(tif, tag), val);
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
                         if (res.returncode == 0) return res;
                         p++;
                       }
                       uint16 val;
                       TIFFGetField(tif, tag, &val);
                       tif_fails("tag %u (%s) does not have some of expected values (but have:%u)\n", tag, TIFFTagName(tif, tag), val);
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
                            if (res.returncode == 0) return res;
                            p++;
                          }
                          float val;
                          TIFFGetField(tif, tag, &val);
                          tif_fails("tag %u (%s) does not have some of expected values (but have:%f)\n", tag, TIFFTagName(tif, tag), val);
                          break;
                        }
    default: /*  none */
                        tif_fails("tag %u (%s) should have values of type long, short or float, but was:%i\n", tag, TIFFTagName(tif, tag), datatype);
  }
}

