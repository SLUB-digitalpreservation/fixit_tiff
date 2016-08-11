/*  checks ICC Profile, using Spec http://www.color.org/specification/ICC1v43_2010-12.pdf */
#include "fixit_tiff.h"
#include "check_icc_header.h"
/* 
#define USE_WARNING
#ifdef USE_WARNING
#define FAIL(...) {fprintf(stderr, __VA_ARGS__); return 1;};
#define INFO(...) fprintf(stderr,__VA_ARGS__);
#else
#define FAIL(...) {return 1;};
#define INFO(...) ;
#endif
*/
#define FAIL(...) {snprintf(errmessage, errsize, __VA_ARGS__); return 1;};
#define INFO(...) ;
// TODO: errormessage pointer und maxsize übergeben


int parse_icc_header_v240_v430(unsigned long iccsize, char * iccdata, unsigned long errsize, char * errmessage) {
  assert(iccdata != NULL);
  assert(errmessage != NULL);
  assert(errsize > 0);
  if (iccsize < 128) FAIL("Invalid ICC profile, minimum size is 128 byte");
  unsigned long profilesize = (
      ((iccdata[0] & 0x00ff) <<24) |
      ((iccdata[1] & 0x00ff) <<16) |
      ((iccdata[2] & 0x00ff) <<8) |
      (iccdata[3] & 0x00ff)
      ) & 0xffff;
  INFO("ICC: profilesize=%li %0x\n", profilesize, profilesize);
  if (profilesize != iccsize) FAIL("commited ICC size (%li) differs from encoded profilesize (%li)", iccsize, profilesize);
  /* -- */
  char preferredcmmtype[5]="    "; memcpy(preferredcmmtype, &iccdata[4],4);
  INFO("ICC: preferredcmmtype='%s'\n", preferredcmmtype);
  /*  TODO: check if this is really correct, because found some TIFFs with
   *  value 'Lino', handle with care!  */
  if ( /*  see document http://www.color.org/registry/signature/TagRegistry-2016-05.pdf */
      ('\0' != preferredcmmtype[0]) &&
      (0 != strncmp("ADBE", preferredcmmtype, 4))  &&
      (0 != strncmp("ACMS", preferredcmmtype, 4))  &&
      (0 != strncmp("appl", preferredcmmtype, 4))  &&
      (0 != strncmp("CCMS", preferredcmmtype, 4))  &&
      (0 != strncmp("UCCM", preferredcmmtype, 4))  &&
      (0 != strncmp("UCMS", preferredcmmtype, 4))  &&
      (0 != strncmp("EFI ", preferredcmmtype, 4))  &&
      (0 != strncmp("FF  ", preferredcmmtype, 4))  &&
      (0 != strncmp("EXAC", preferredcmmtype, 4))  &&
      (0 != strncmp("HCMM", preferredcmmtype, 4))  &&
      (0 != strncmp("argl", preferredcmmtype, 4))  &&
      (0 != strncmp("LgoS", preferredcmmtype, 4))  &&
      (0 != strncmp("HDM ", preferredcmmtype, 4))  &&
      (0 != strncmp("lcms", preferredcmmtype, 4))  &&
      (0 != strncmp("KCMS", preferredcmmtype, 4))  &&
      (0 != strncmp("MCML", preferredcmmtype, 4))  &&
      (0 != strncmp("WCS ", preferredcmmtype, 4))  &&
      (0 != strncmp("SIGN", preferredcmmtype, 4))  &&
      (0 != strncmp("RGMS", preferredcmmtype, 4))  &&
      (0 != strncmp("SICC", preferredcmmtype, 4))  &&
      (0 != strncmp("TCMM", preferredcmmtype, 4))  &&
      (0 != strncmp("32BT", preferredcmmtype, 4))  &&
      (0 != strncmp("WTG ", preferredcmmtype, 4))  &&
      (0 != strncmp("zc00", preferredcmmtype, 4))

     ) FAIL("preferred cmmtype ('%s') should be empty or (possibly, because ICC validation is alpha code) one of following strings: 'ADBE' 'ACMS' 'appl' 'CCMS' 'UCCM' 'UCMS' 'EFI ' 'FF  ' 'EXAC' 'HCMM' 'argl' 'LgoS' 'HDM ' 'lcms' 'KCMS' 'MCML' 'WCS ' 'SIGN' 'RGMS' 'SICC' 'TCMM' '32BT' 'WTG ' 'zc00'", preferredcmmtype);
  /* -- */
  char profileversion[6]="    "; snprintf(profileversion, 6, "%i.%i.%i", (iccdata[8]) & 0x000f, ((iccdata[9] & 0x00f0) >>4), (iccdata[9] & 0x000f));
  INFO("ICC: profileversion='%s'\n", profileversion);
  // TODO: 2.4.0 or 4.3.0, others not allowed?
  /* -- */
  char profileclass[5]="    "; memcpy(profileclass, &iccdata[12],4);
  INFO("ICC: profileclass='%s'\n", profileclass);
  if ( /*  see page 13, table 11 of http://www.color.org/ICC_Minor_Revision_for_Web.pdf */
      (0 != strncmp("scnr", profileclass, 4)) &&
      (0 != strncmp("mntr", profileclass, 4)) &&
      (0 != strncmp("prtr", profileclass, 4)) &&
      (0 != strncmp("link", profileclass, 4)) &&
      (0 != strncmp("spac", profileclass, 4)) &&
      (0 != strncmp("abst", profileclass, 4)) &&
      (0 != strncmp("nmcl", profileclass, 4))
     ) FAIL("profile class ('%s'), should be one of following strings for device classes: 'scnr', 'mntr', 'prtr' or for profile classes: 'link', 'spac', 'abst', 'nmcl'", profileclass);
  /* -- */
  char colorspacedata[5]="    "; memcpy(colorspacedata, &iccdata[16],4);
  INFO("ICC: colorspacedata='%s'\n", colorspacedata);
  if ( /*  see page 14, table 13 of http://www.color.org/ICC_Minor_Revision_for_Web.pdf */
      (0 != strncmp("XYZ ", colorspacedata, 4)) &&
      (0 != strncmp("Lab ", colorspacedata, 4)) &&
      (0 != strncmp("Luv ", colorspacedata, 4)) &&
      (0 != strncmp("YCbr", colorspacedata, 4)) &&
      (0 != strncmp("Yvx ", colorspacedata, 4)) &&
      (0 != strncmp("RGB ", colorspacedata, 4)) &&
      (0 != strncmp("GRAY", colorspacedata, 4)) &&
      (0 != strncmp("HSV ", colorspacedata, 4)) &&
      (0 != strncmp("HLS ", colorspacedata, 4)) &&
      (0 != strncmp("CMYK", colorspacedata, 4)) &&
      (0 != strncmp("CMY ", colorspacedata, 4)) &&
      (0 != strncmp("2CLR", colorspacedata, 4)) &&
      (0 != strncmp("3CLR", colorspacedata, 4)) &&
      (0 != strncmp("4CLR", colorspacedata, 4)) &&
      (0 != strncmp("5CLR", colorspacedata, 4)) &&
      (0 != strncmp("6CLR", colorspacedata, 4)) &&
      (0 != strncmp("7CLR", colorspacedata, 4)) &&
      (0 != strncmp("8CLR", colorspacedata, 4)) &&
      (0 != strncmp("9CLR", colorspacedata, 4)) &&
      (0 != strncmp("ACLR", colorspacedata, 4)) &&
      (0 != strncmp("BCLR", colorspacedata, 4)) &&
      (0 != strncmp("CCLR", colorspacedata, 4)) &&
      (0 != strncmp("DCLR", colorspacedata, 4)) &&
      (0 != strncmp("ECLR", colorspacedata, 4)) &&
      (0 != strncmp("FCLR", colorspacedata, 4))
     ) FAIL("colorspace data ('%s'), should be one of following strings: 'XYZ ' 'Lab ' 'Luv ' 'YCbr' 'Yvx ' 'RGB ' 'GRAY' 'HSV ' 'HLS ' 'CMYK' 'CMY ' '2CLR' '3CLR' '4CLR' '5CLR' '6CLR' '7CLR' '8CLR' '9CLR' 'ACLR' 'BCLR' 'CCLR' 'DCLR' 'ECLR' 'FCLR'", colorspacedata);
  /* -- */
  char connectionspacedata[5]="    "; memcpy(connectionspacedata, &iccdata[20],4);
  INFO("ICC: connectionspacedata(PCS)='%s'\n", connectionspacedata);
  if ( /*  see page 15, table 14 of http://www.color.org/ICC_Minor_Revision_for_Web.pdf */
      (0 != strncmp("XYZ ", connectionspacedata, 4)) &&
      (0 != strncmp("Lab ", connectionspacedata, 4))
     ) FAIL("connection space data ('%s') should be one of following strings: 'XYZ ' 'Lab '", connectionspacedata);
  /* -- */
  // datetime 24-35
  char datetime[20]; snprintf(datetime, 20, "%.4d:%.2d:%.2d %.2d:%.2d:%.2d", 
      ((iccdata[24] & 0x00ff)<<8 | (iccdata[25] & 0x00ff)), // yyyy
      ((iccdata[26] & 0x00ff)<<8 | (iccdata[27] & 0x00ff)), // MM
      ((iccdata[28] & 0x00ff)<<8 | (iccdata[29] & 0x00ff)), // DD
      ((iccdata[30] & 0x00ff)<<8 | (iccdata[31] & 0x00ff)), // hh
      ((iccdata[32] & 0x00ff)<<8 | (iccdata[33] & 0x00ff)), // mm
      ((iccdata[34] & 0x00ff)<<8 | (iccdata[35] & 0x00ff))  // ss
      );
  INFO("ICC: datetime='%s'\n", datetime);


  /* -- */
  char profilefilesignature[5]="    "; memcpy(profilefilesignature, &iccdata[36],4);
  INFO("ICC: profilefilesignature='%s'\n", profilefilesignature);
  /* -- */
  char primaryplattformsignature[5]="    "; memcpy(primaryplattformsignature, &iccdata[40],4);
  INFO("ICC: primaryplattformsignature='%s'\n", primaryplattformsignature);
  if ( /*  see page 15, table 15 of http://www.color.org/ICC_Minor_Revision_for_Web.pdf */
      ('\0' != primaryplattformsignature[0]) &&
      (0 != strncmp("APPL", primaryplattformsignature, 4)) &&
      (0 != strncmp("MSFT", primaryplattformsignature, 4)) &&
      (0 != strncmp("SGI ", primaryplattformsignature, 4)) &&
      (0 != strncmp("SUNW", primaryplattformsignature, 4)) &&
      (0 != strncmp("TGNT", primaryplattformsignature, 4))
     ) FAIL("primary plattform signature ('%s') should be empty or one of following strings: 'APPL', 'MSFT', 'SGI ', 'SUNW', 'TGNT'", primaryplattformsignature);

  /* -- */
  // Profile Flags 44-47
  /* -- */
  char devicemanufacturer[5]="    "; memcpy(devicemanufacturer, &iccdata[48],4);
  INFO("ICC: devicemanufacturer='%s'\n", devicemanufacturer);
  /* -- */
  char devicemodel[5]="    "; memcpy(devicemodel, &iccdata[52],4);
  INFO("ICC: devicemodel='%s'\n", devicemodel);
  /* -- */
  // device attributes 56-63
  /* -- */
  // rendering intent field 64-67
  /* -- */
  return 0;
}

/* checks a ICC header, see chapter 7 of http://www.color.org/specification/ICC1v43_2010-12.pdf */
int parse_icc_v430(unsigned long iccsize, char* iccdata, unsigned long errsize, char * errmessage) {
  assert(iccdata != NULL);
  assert(errmessage != NULL);
  assert(errsize > 0);
  if (iccsize < 128) FAIL ("Invalid ICC profile 1v43_2010, see http://www.color.org/specification/ICC1v43_2010-12.pdf for details");
  int ret_header =  parse_icc_header_v240_v430( iccsize, iccdata, errsize, errmessage);
  if (0 != ret_header) return ret_header;
  else {
  // PCS illuminant field 68-79
  /* -- */
  // Profile creator field 80-83
  /* -- */
  // Profile ID field 84-99
  /*  -- */
  // Reserved field bytes 100-127

  };
  return 0;
}

/* checks a ICC header, see chapter 6 of http://www.color.org/ICC_Minor_Revision_for_Web.pdf */
int parse_icc_v240(unsigned long iccsize, char* iccdata, unsigned long errsize, char * errmessage) {
  assert(iccdata != NULL);
  assert(errmessage != NULL);
  assert(errsize > 0);
  if (iccsize < 128) FAIL ("Invalid ICC profile ICC.1:2001-04, see http://www.color.org/ICC_Minor_Revision_for_Web.pdf for details");
  int ret_header =  parse_icc_header_v240_v430( iccsize, iccdata, errsize, errmessage);
  if (0 != ret_header) return ret_header;
  else {
  // PCS xyz illuminant  68-79
  /* -- */
  // Profile creator signature 80-83
  /* -- */
  // Reserved field bytes 84-127
  };
  return 0;
}

/*  returns 0 if valid, !0 if errorneous */
int parse_icc(unsigned long iccsize, char* iccdata, unsigned long errsize, char * errmessage) {
  assert(iccdata != NULL);
  assert(errmessage != NULL);
  assert(errsize > 0);
  if (iccsize < 10) FAIL ("Invalid ICC profile");
  char profileversion[6]="    "; snprintf(profileversion, 6, "%i.%i.%i", (iccdata[8]) & 0x000f, ((iccdata[9] & 0x00f0) >>4), (iccdata[9] & 0x000f));
  INFO("ICC: profileversion='%s'\n", profileversion);
  if (0==strncmp(profileversion, "4.3.0",5))  return parse_icc_v430(iccsize, iccdata, errsize, errmessage);
  else if (0==strncmp(profileversion, "2.4.0",5)) return parse_icc_v240(iccsize,iccdata, errsize, errmessage);
  else {
        return parse_icc_header_v240_v430(iccsize,iccdata, errsize, errmessage);
  }
  return 0;
}

/*  checks ICC header */
int check_icc_header (const char * filename ) {
  /* load file */
  TIFF* tif = TIFFOpen(filename, "r+");
  if (NULL == tif) {
    fprintf( stderr, "file '%s' could not be opened\n", filename);
    exit (FIXIT_TIFF_READ_PERMISSION_ERROR);
  };
  /* find date-tag and fix it */
  char *iccprofile=NULL;
  uint32 count=0;
  int returncode = 0;
  int found=TIFFGetField(tif, TIFFTAG_ICCPROFILE, &count, &iccprofile);
  if (1==found) { /* there exists a datetime field */
    unsigned long errsize=1024;
    char errmsg[errsize];
    returncode =  parse_icc(count, iccprofile, errsize, errmsg);
  } else if (0 == found) {
    if (FLAGGED == flag_be_verbose) printf ("no ICC profile found!\n");
  }
  TIFFClose(tif);
  if (0 == returncode) return FIXIT_TIFF_IS_VALID;
  else return FIXIT_TIFF_IS_INVALID;
}

