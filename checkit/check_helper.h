/* rule based checks if given TIFF is a specific baseline TIFF
 * 
 * author: Andreas Romeyke, 2015
 * licensed under conditions of libtiff 
 * (see http://libtiff.maptools.org/misc.html)
 *
 */

#ifndef _FIXIT_TIFF_CHECK_HELPER
#define _FIXIT_TIFF_CHECK_HELPER
#include <stdlib.h>
#include <tiff.h>
#include <tiffio.h>

/* helper */
ret_t check_tag_has_fvalue(TIFF*  tif, tag_t tag, float value);
ret_t check_tag_has_u16value(TIFF*  tif, tag_t tag, uint16 value);
ret_t check_tag_has_u32value(TIFF*  tif, tag_t tag, uint32 value);
const char * TIFFTagName (  TIFF * tif, tag_t tag );
uint32 TIFFGetRawTagTypeListEntry( TIFF  * tif, int tagidx );
tag_t TIFFGetRawTagListEntry( TIFF  * tif, int tagidx ) ;
int TIFFGetRawTagListCount (TIFF * tif) ;
TIFFDataType TIFFGetRawTagType(TIFF * tif, tag_t tag);
ifd_entry_t TIFFGetRawIFDEntry( TIFF * tif, tag_t tag);
ifd_entry_t TIFFGetRawTagIFDListEntry( TIFF  * tif, int tagidx );
offset_t read_offsetdata( TIFF * tif, uint32 address, uint16 count, uint16 datatype);
uint32 get_first_IFD(TIFF * tif);
//int TIFFIsByteSwapped(TIFF * tif);
void clear_cache();

#endif
