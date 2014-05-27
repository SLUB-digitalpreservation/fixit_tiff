#ifndef _FIXIT_TIFF
#define _FIXIT_TIFF
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <tiffio.h>
#include <tiff.h>

/* define specific exit-codes */
#define FIXIT_TIFF_IS_VALID 0
#define FIXIT_TIFF_IS_CHECKED 1
#define FIXIT_TIFF_IS_CORRECTED 1
#define FIXIT_TIFF_MISSED_OUTFILE -1
#define FIXIT_TIFF_MISSED_INFILE -2
#define FIXIT_TIFF_CMDLINE_ARGUMENTS_ERROR -3
#define FIXIT_TIFF_MEMORY_ALLOCATION_ERROR -4
#define FIXIT_TIFF_DATETIME_RULE_NOT_FOUND -6
#define FIXIT_TIFF_STRING_COPY_ERROR -5
#define FIXIT_TIFF_READ_PERMISSION_ERROR -20
#define FIXIT_TIFF_WRITE_PERMISSION_ERROR -21
#define FIXIT_TIFF_WRITE_ERROR -22

/** 20 comes from TIFF definition 
 */
#define TIFFDATETIMELENGTH 20

int check_datetime (const char *);
int check_baseline (const char *); 
int cleanup_datetime (const char *);
int cleanup_baseline (const char *); 

#define FLAGGED 1
#define UNFLAGGED 0

/** global variables */
static int flag_be_verbose=FLAGGED;
static int flag_check_only=UNFLAGGED;


#endif _FIXIT_TIFF
