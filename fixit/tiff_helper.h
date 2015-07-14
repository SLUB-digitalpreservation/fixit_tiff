#ifndef _FIXIT_TIFF_TIFF_HELPER
#define _FIXIT_TIFF_TIFF_HELPER
int TIFFGetRawTagListCount (TIFF * tif);
uint32 TIFFGetRawTagListEntry( TIFF  * tif, int tagidx );
void print_baseline_tags (TIFF * tif);
void print_required_tags (TIFF * tif);
#endif
/* _FIXIT_TIFF_TIFF_HELPER */
