#ifndef _FIXIT_TIFF_CHECK
#define _FIXIT_TIFF_CHECK
#include <stdlib.h>
#include <tiff.h>
#include <tiffio.h>

typedef struct ret_s {
  int returncode;
  char * returnmsg;
} ret_t;

typedef struct ifd_entry_s {
  uint16 count;
  TIFFDataType datatype;
  enum{ is_offset, is_value } value_or_offset;
  union {
    uint32 data32;
    uint16 data16[2];
    uint8 data8[4];
    uint32 data32offset;
  };
} ifd_entry_t;

typedef struct offset_s {
  uint16 count;
  TIFFDataType datatype;
  union {
    uint8  *data8p;
    uint16 *data16p;
    uint32 *data32p;
    char   *datacharp;
    int8   *datas8p;
    int16  *datas16p;
    int32  *datas32p;
    float  *datafloatp;
    double *datadoublep;
    uint64 *data64p;
    int64  *datas64p;
  };
} offset_t;

typedef unsigned int tag_t;

#define MAXSTRLEN 160

#define tif_fails(args...) {ret_t res;  char * str =malloc( sizeof(char) *MAXSTRLEN ); if (NULL==str) { fprintf(stderr, "could not allocate memory for tif_fails\n"); exit(EXIT_FAILURE); }; snprintf (str, MAXSTRLEN-1, args); printf("\t%s", str); res.returnmsg = str; res.returncode=1; return res;}

#define tifp_check( tif ) {if (NULL == tif) { tif_fails("TIFF pointer is empty\n"); } }

#define tif_returns(args...) {ret_t res;  char * str =malloc( sizeof(char) *MAXSTRLEN ); if (NULL==str) { fprintf(stderr, "could not allocate memory for tif_fails\n"); exit(EXIT_FAILURE); }; snprintf (str, MAXSTRLEN-1, args); res.returnmsg = str; res.returncode=1; return res;}


ret_t check_tag_has_some_of_these_values( TIFF* tif, tag_t tag, int count, unsigned int * values);
ret_t check_tag_has_valuelist( TIFF* tif, tag_t tag, int count, unsigned int * values);
ret_t check_tag_has_value_in_range(TIFF* tif, tag_t tag, unsigned int a, unsigned int b);
ret_t check_tag_has_value(TIFF* tif, tag_t tag, unsigned int value);
ret_t check_tag_has_value_quiet(TIFF* tif, tag_t tag, unsigned int value);
ret_t check_tag(TIFF* tif, tag_t tag);
ret_t check_tag_quiet(TIFF* tif, tag_t tag);
ret_t check_notag(TIFF* tif, tag_t tag);
ret_t check_tag_has_valid_type();
ret_t check_datetime(TIFF* tif);
ret_t check_has_only_one_ifd(TIFF* tif);
ret_t check_tagorder(TIFF* tif);
ret_t check_tag_has_valid_asciivalue(TIFF* tif, tag_t tag); 
ret_t check_tag_has_value_matching_regex(TIFF* tif, tag_t tag, const char* value);
/* helper */
ret_t check_tag_has_fvalue(TIFF*  tif, tag_t tag, float value);
ret_t check_tag_has_u16value(TIFF*  tif, tag_t tag, uint16 value);
ret_t check_tag_has_u32value(TIFF*  tif, tag_t tag, uint32 value);
const char * TIFFTagName (  TIFF * tif, tag_t tag );
uint32 TIFFGetRawTagTypeListEntry( TIFF  * tif, int tagidx );
uint32 TIFFGetRawTagListEntry( TIFF  * tif, int tagidx ) ;
int TIFFGetRawTagListCount (TIFF * tif) ;
TIFFDataType TIFFGetRawTagType(TIFF * tif, tag_t tag);
ifd_entry_t TIFFGetRawIFDEntry( TIFF * tif, tag_t tag);
ifd_entry_t TIFFGetRawTagIFDListEntry( TIFF  * tif, int tagidx );
offset_t read_offsetdata( TIFF * tif, uint32 address, uint16 count, uint16 datatype);
#endif
/* _FIXIT_TIFF_CHECK */
