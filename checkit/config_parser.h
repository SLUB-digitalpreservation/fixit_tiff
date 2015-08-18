#ifndef _FIXIT_TIFF_CONFIG_PARSER
#define _FIXIT_TIFF_CONFIG_PARSER
#include "../fixit/fixit_tiff.h"
#include "check.h"


/* struct to hold parsing configuration */
typedef struct funcu * funcp;
typedef struct {
        funcp fu_p;
        char * name;
        void * next;
        ret_t result;
} executionentry_t;

typedef struct {
        executionentry_t * start;
        executionentry_t * last; 
} executionplan_t;

typedef enum { mandatory, ifdepends, optdepends, optional } requirements_t;
typedef enum { range, logical_or, any, only, regex } values_t;


/* definitions of structs of function pointers to hold type information for "lazy evaluation" */

typedef struct f_tifp_s {
  ret_t (*functionp)(TIFF *);
} f_tifp_t;

typedef struct f_tifp_tag_s {
  int a;
  ret_t (*functionp)(TIFF *, tag_t a);
} f_tifp_tag_t;

typedef struct f_tifp_tag_uint_s {
  int a;
  unsigned int b;
  ret_t (*functionp)(TIFF*, tag_t a, unsigned int b);
} f_tifp_tag_uint_t;

typedef struct f_tifp_tag_uint_uint_s {
  int a;
  unsigned int b;
  unsigned int c;
  ret_t (*functionp)(TIFF*, tag_t a, unsigned int b, unsigned int c);
} f_tifp_tag_uint_uint_t;

typedef struct f_tifp_tag_int_uintp_s {
  int a;
  int b;
  unsigned int * c;
  ret_t (*functionp)(TIFF*, tag_t a, int b, unsigned int * c);
} f_tifp_tag_int_uintp_t;

typedef struct f_tifp_tag_charp_s {
  int a;
  const char * b;
  ret_t (*functionp)(TIFF*, tag_t a, char * b);
} f_tifp_tag_charp_t;


typedef enum { f_dummy, f_tifp, f_tifp_tag, f_tifp_tag_uint, f_tifp_tag_uint_uint, f_tifp_tag_int_uintp, f_tifp_tag_charp } ftype_t;

struct funcu {
  ftype_t ftype;
  funcp pred;
  tag_t tag;
  union  {
    struct f_tifp_s * ftifp;
    struct f_tifp_tag_s * ftifp_tag;
    struct f_tifp_tag_uint_s * ftifp_tag_uint;
    struct f_tifp_tag_uint_uint_s * ftifp_tag_uint_uint;
    struct f_tifp_tag_int_uintp_s * ftifp_tag_int_uintp;
    struct f_tifp_tag_charp_s * ftifp_tag_charp;
  } fu;
};

/* MINTAGS - MAXTAGS is range of possible existing TAG numbers */
#define MINTAGS 254
#define MAXTAGS 65536


typedef struct parser_state_s {
  int lineno;
  int valuelist;
  tag_t tag;
  values_t val;
  requirements_t req;
  unsigned int i_stack[40];
  int i_stackp;
  int called_tags[MAXTAGS];
  FILE * stream;
  int any_reference;
  const char * regex_string;
} parser_state_t;

void set_parse_error(char * msg, char * yytext);
int execute_plan (TIFF * tif) ;
void print_plan ();
void print_plan_results ();
void clean_plan ();
void parse_plan ();
void parse_plan_via_stream (FILE * stream);
void add_default_rules_to_plan();

/* helper */
void _helper_add_fsp_tifp(struct funcu * f, ret_t (* function)(TIFF *), char * fname);
void _helper_add_fsp_tifp_tag(struct funcu * f, ret_t (* function)(TIFF *, tag_t), char * fname, tag_t tag);
void _helper_add_fsp_tifp_tag_uint(struct funcu * f, ret_t (* function)(TIFF *, tag_t, unsigned int), char * fname, tag_t tag, unsigned int v);
void _helper_add_fsp_tifp_tag_uint_uint(struct funcu * f, ret_t (* function)(TIFF *, tag_t, unsigned int, unsigned int), char * fname, tag_t tag, unsigned int l, unsigned int r);
void _helper_add_fsp_tifp_tag_int_uintp(struct funcu * f,  ret_t (* function)(TIFF *, tag_t, int, unsigned int *), char * fname, tag_t tag, int count_of_values, unsigned int * rp);

#endif
/* _FIXIT_TIFF_CONFIG_PARSER */
