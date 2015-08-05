#include "check.h"
#include "../fixit/fixit_tiff.h"
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
                         if (res.returncode == 0) return res;
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
                            if (res.returncode == 0) return res;
                            p++;
                          }
                          return res;
                          break;
                        }
    default: /*  none */
                        tif_fails("tag %u (%s) should have values of type long, short or float, but was:%i\n", tag, TIFFTagName(tif, tag), datatype);
  }
}

