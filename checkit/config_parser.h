#ifndef _FIXIT_TIFF_CONFIG_PARSER
#define _FIXIT_TIFF_CONFIG_PARSER

/* struct to hold parsing configuration */
typedef struct funcu * funcp;
typedef struct {
        funcp fu_p;
        char * name;
        void * next;
        int result;
} executionentry_t;

typedef struct {
        executionentry_t * start;
        executionentry_t * last; 
} executionplan_t;

typedef enum { mandatory, ifdepends, optional } requirements_t;
typedef enum { range, logical_or, any, only } values_t;

/* definitons of structs of function pointers to hold type information for "lazy evaluation" */

typedef struct f_s {
  int (*functionp)(TIFF *);
} f_t;

typedef struct f_int_s {
  int a;
  int (*functionp)(TIFF *, int a);
} f_int_t;

typedef struct f_intint_s {
  int a;
  int b;
  int (*functionp)(TIFF*, int a, int b);
} f_intint_t;

typedef struct f_intintint_s {
  int a;
  int b;
  int c;
  int (*functionp)(TIFF*, int a, int b, int c);
} f_intintint_t;

typedef enum { f_dummy, f_void, f_int, f_intint, f_intintint } ftype_t;

struct funcu {
  ftype_t ftype;
  funcp pred;
  union  {
    struct f_s * fvoidt;
    struct f_int_s * fintt;
    struct f_intint_s * fintintt;
    struct f_intintint_s * fintintintt;
  } fu;
};

typedef struct parser_state_s {
  int lineno;
  int logical_or;
  int tag;
  values_t val;
  requirements_t req;
  int i_stack[40];
  int i_stackp;
} parser_state_t;

#endif
/* _FIXIT_TIFF_CONFIG_PARSER */
