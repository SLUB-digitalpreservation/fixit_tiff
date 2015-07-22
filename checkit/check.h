#ifndef _FIXIT_TIFF_CHECK
#define _FIXIT_TIFF_CHECK
#include <tiff.h>
#include <tiffio.h>
#define tif_fails(args...) {char * str =malloc( sizeof(char) *80 ); if (NULL==str) { fprintf(stderr, "could not allocate memory for tif_fails\n"); exit(EXIT_FAILURE); }; snprintf (str, 79, args); printf("\t%s", str); return (char *)str;}
const char * check_tag_has_some_of_these_values( TIFF* tif, int tag, int count, unsigned int * values);
const char * check_tag_has_valuelist( TIFF* tif, int tag, int count, unsigned int * values);
const char * check_tag_has_value_in_range(TIFF* tif, int tag, unsigned int a, unsigned int b);
const char * check_tag_has_value(TIFF* tif, int tag, unsigned int value);
const char * check_tag(TIFF* tif, int tag);
const char * check_notag(TIFF* tif, int tag);
const char * check_tag_has_valid_type();
const char * check_datetime(TIFF* tif);

#endif
/* _FIXIT_TIFF_CHECK */
