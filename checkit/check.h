#ifndef _FIXIT_TIFF_CHECK
#define _FIXIT_TIFF_CHECK
#include <tiff.h>
#include <tiffio.h>
int check_tag_has_value_in_range(TIFF* tif, int tag, int a, int b);
int check_tag_has_value(TIFF* tif, int tag, int value);
int check_tag(TIFF* tif, int tag);
#endif
/* _FIXIT_TIFF_CHECK */
