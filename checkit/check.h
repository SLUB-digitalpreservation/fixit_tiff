#ifndef _FIXIT_TIFF_CHECK
#define _FIXIT_TIFF_CHECK
#include <tiff.h>
#include <tiffio.h>
#define tif_fails(args...) {char str[80]; sprintf (str, args); printf("\t%s", str);}
int check_tag_has_some_of_these_values( TIFF* tif, int tag, int count, int * values);
int check_tag_has_value_in_range(TIFF* tif, int tag, int a, int b);
int check_tag_has_value(TIFF* tif, int tag, int value);
int check_tag(TIFF* tif, int tag);
int check_notag(TIFF* tif, int tag);
#endif
/* _FIXIT_TIFF_CHECK */
