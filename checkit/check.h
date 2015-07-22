#ifndef _FIXIT_TIFF_CHECK
#define _FIXIT_TIFF_CHECK
#include <tiff.h>
#include <tiffio.h>
typedef struct ret_s {
  int returncode;
  char * returnmsg;
} ret_t;

#define tif_fails(args...) {ret_t res;  char * str =malloc( sizeof(char) *80 ); if (NULL==str) { fprintf(stderr, "could not allocate memory for tif_fails\n"); exit(EXIT_FAILURE); }; snprintf (str, 79, args); printf("\t%s", str); res.returnmsg = str; res.returncode=1; return res;}
ret_t check_tag_has_some_of_these_values( TIFF* tif, int tag, int count, unsigned int * values);
ret_t check_tag_has_valuelist( TIFF* tif, int tag, int count, unsigned int * values);
ret_t check_tag_has_value_in_range(TIFF* tif, int tag, unsigned int a, unsigned int b);
ret_t check_tag_has_value(TIFF* tif, int tag, unsigned int value);
ret_t check_tag(TIFF* tif, int tag);
ret_t check_notag(TIFF* tif, int tag);
ret_t check_tag_has_valid_type();
ret_t check_datetime(TIFF* tif);

#endif
/* _FIXIT_TIFF_CHECK */
