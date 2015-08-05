/* fixes broken TIFF Files
 * 
 * fixes unused tags in Baseline-TIFFs,
 * based on http://www.awaresystems.be/imaging/tiff/tifftags/baseline.html
 *
 * author: Andreas Romeyke, 2013
 * licensed under conditions of libtiff 
 */

// #define YY_DEBUG
#include <stdio.h>      
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../fixit/fixit_tiff.h"
#include "../fixit/tiff_helper.h"
#include "config_parser.h"
#include "check.h"

/* TODO: handle stuff like DATETIME or COPYRIGHT */

/*
#define DEBUG
*/


#define YY_CTX_LOCAL

/* global vars */
parser_state_t parser_state;
executionplan_t plan;

/* redefined YY_INPUT to read from stream */
#define YY_INPUT(yyctx, buf, result, max_size)		\
  {							\
    int yyc= fgetc(parser_state.stream);		\
    result= (EOF == yyc) ? 0 : (*(buf)= yyc, 1);	\
    yyprintf((stderr, "<%c>", yyc));			\
  }                                                     


/* type specific calls of function pointers 
 * @param tif pointer to libtiff TIIFF structure
 * @param fp pointer to funcu structure to hold function and its parameters
 * @return ret_t structure for results of called function
 */
ret_t call_fp(TIFF* tif, funcp fp) {
  ret_t ret;
  ret.returncode=1;
  ret.returnmsg=NULL; /*@null@*/
  if (NULL != fp) {
    switch (fp->ftype) {
      case f_dummy: break;
      case f_tifp: 
                    {
                      f_tifp_t * function = NULL;
                      function = fp->fu.ftifp;
                      assert(NULL != function);
                      assert(NULL != function->functionp);
                      ret = (function->functionp)(tif);
                      break;
                    }
      case f_tifp_tag:
                    {
                      f_tifp_tag_t * function = NULL;
                      function = fp->fu.ftifp_tag;
                      assert(NULL != function);
                      assert(NULL != function->functionp);
                      ret = (function->functionp)(tif, function->a); 
                      break;
                    }
      case f_tifp_tag_uint:
                    {
                      f_tifp_tag_uint_t * function = NULL;
                      function = fp->fu.ftifp_tag_uint;
                      assert(NULL != function);
                      assert(NULL != function->functionp);
#ifdef DEBUG
                      printf("debug: found a=%i b=%u\n", function->a, function->b);
#endif
                      ret = (function->functionp)(tif, function->a, function->b); 
                      break;
                    }
      case f_tifp_tag_uint_uint:
                    {
                      f_tifp_tag_uint_uint_t * function = NULL;
                      function = fp->fu.ftifp_tag_uint_uint;
                      assert(NULL != function);
                      assert(NULL != function->functionp);
#ifdef DEBUG
                      printf("debug: found a=%i b=%u c=%u\n", function->a, function->b, function->c);
#endif
                      ret = (function->functionp)(tif, function->a, function->b, function->c); 
                      break;
                    }
      case f_tifp_tag_int_uintp:
                    {
                      f_tifp_tag_int_uintp_t * function = NULL;
                      function = fp->fu.ftifp_tag_int_uintp;
                      assert(NULL != function);
                      assert(NULL != function->functionp);
#ifdef DEBUG
                      printf("debug: found a=%i b=%i c=%p\n", function->a, function->b, function->c);
#endif
                      ret = (function->functionp)(tif, function->a, function->b, function->c); 
                      break;
                    }
      default:
                    fprintf(stderr, "call_fp() error, should not occure!\n");
                    exit(EXIT_FAILURE);
    }
  }
  return ret;
}

/* executes a plan (list) of functions, checks if predicate-function calls are
 * needed, too. The plan is a global variable. 
 * @param tif pointer to TIFF structure
 * @return return-code is 0 if all called functions are succeed 
 */
int execute_plan (TIFF * tif) {
  executionentry_t * this_exe_p = plan.start;
  int is_valid = 0; /* 0 means valid, >0 invalid */
  while (NULL != this_exe_p) {
#ifdef DEBUG
    printf("\n\nexecute: %s\n", this_exe_p->name);
#endif
    funcp fp;
    fp = this_exe_p->fu_p;
    ret_t res;
     if (NULL != fp) {
#ifdef DEBUG
      printf("execute: fp not null\n");
#endif
      if (NULL != fp->pred) { /* we have a predicate function, call it and
                                 decide if we continue or not */
#ifdef DEBUG
        printf("execute: we have a predicate... ");
#endif
        /* has the predicate a predicate? */
        if (NULL != fp->pred->pred) { /* yes */
          #ifdef DEBUG
          printf("execute: we have a predicate predicate... ");
#endif
          res =  call_fp(tif, fp->pred->pred);
          if (0 != res.returncode ) {
#ifdef DEBUG
            printf("execute: predicate predicate was not successfull\n");
#endif
            this_exe_p->result.returncode=0;
            this_exe_p->result.returnmsg="predicate predicate was not successfull, skipped check";
            goto exitcall;
            /* the predicate was not successfull, skip check */
          } else { /* predicate predicate successfull*/
#ifdef DEBUG
            printf("execute: predicate predicate was successfull\n");
#endif
          }
        };
        /* no predicate predicate or predicate predicate successfull*/
          res =  call_fp(tif, fp->pred);
          if (0 != res.returncode ) {
#ifdef DEBUG
            printf("execute: predicate was not successfull\n");
#endif
            this_exe_p->result.returncode=0;
            this_exe_p->result.returnmsg="predicate was not successfull, skipped check";
          /* the predicate was not successfull, skip check */
          goto exitcall;
        } else {
     /* predicate was successful */
#ifdef DEBUG
          printf("execute: predicate was successfull\n");
#endif
        }
     }
        /* no predicate or predicate successfull*/
      parser_state.called_tags[fp->tag]++;
      this_exe_p->result= call_fp (tif, fp );
      if (0 != this_exe_p->result.returncode) { is_valid++; }
     }
exitcall:    this_exe_p = this_exe_p->next;
  }
  /* now we know which tags are already checked, we need add a rule to
   * forbidden all other tags */
  printf("check if forbidden tags are still existing\n");
  int tag;
  for (tag=MINTAGS; tag<MAXTAGS; tag++) {
    if (0 == parser_state.called_tags[tag]) { /* only unchecked tags */
      ret_t res = check_notag( tif, tag);
      if (0 != res.returncode) { /* check if tag is not part of tif */
        /* tag does not exist */
        is_valid++;
      }
    }
  }
  if (is_valid > 0) {
        printf("found %i errors\n", is_valid);
  } else {
        printf("the given tif is valid\n");
  }
  return (is_valid);
}

/* prints a plan (list) of functions */
/*
void print_plan () {
  printf("print plan:\n");
  executionentry_t * this_exe_p = plan.start;
  while (NULL != this_exe_p) {
    printf("next action is: %s\n", this_exe_p->name);
    this_exe_p = this_exe_p->next;
  }
}
*/

/* prints a plan (list) of functions and their results*/
void print_plan_results() {
  printf("print plan results:\n");
  executionentry_t * this_exe_p = plan.start;
  while (NULL != this_exe_p) {
    const char * msg;
    if (0 == this_exe_p->result.returncode) { 
      msg = "passed"; 
    } else {
      msg=this_exe_p->result.returnmsg; 
    }
    printf("action was: %s, result=%s\n", this_exe_p->name, msg);
    this_exe_p = this_exe_p->next;
  }
}



/* adds a function to an execution plan
 *
 * the plan is a global variable
 * @param fp is a pointer to function
 * @param name is a string of fname
 * @return retirn 0 if succeed
 */
int append_function_to_plan (funcp fp, const char * name ) {
  assert(NULL != fp);
  assert(NULL != name);
  executionentry_t * entry = NULL;
  entry = malloc ( sizeof(executionentry_t) );
  if (NULL == entry) {
    fprintf(stderr, "could not alloc memory for execution plan");
    exit(EXIT_FAILURE);
  }
  entry->next = NULL;
  entry->fu_p = fp;
  entry->result.returncode = 0;
  entry->result.returnmsg = NULL;
  entry->name = malloc ( MAXSTRLEN*sizeof(char) );
  if (NULL == entry->name) {
    fprintf(stderr, "could not alloc memory for execution plan");
    exit(EXIT_FAILURE);
  }
  assert(NULL != name);
  strncpy(entry->name, name, MAXSTRLEN-1);
#ifdef DEBUG
  printf("entry has name:%s\n", entry->name);
#endif
 entry->result.returncode = 0;
  entry->result.returnmsg = NULL;

  assert(NULL != entry);
  if (NULL == plan.start) {
#ifdef DEBUG
    printf(" add first to plan\n");
#endif
    plan.start = entry; 
    plan.last = entry;
    assert(NULL != plan.last);
    assert(NULL == plan.last->next);
  } else {
#ifdef DEBUG
    printf(" add n-th to plan\n");
#endif
    assert(NULL != plan.last);
    assert(NULL == plan.last->next);
    executionentry_t * last = NULL;
    last = plan.last;
    assert(NULL != last);
    assert(NULL == last->next);
    last->next = entry;
    plan.last = entry;
  }
  return 0;
}

/* adds a function to struct funcu * f
 * @param f already allocated struct funcu * pointer
 * @param function adress of function
 * @param fname name of function as combined message (already allocated)
 */
void _helper_add_fsp_tifp(struct funcu * f, ret_t (* function)(TIFF *), char * fname) {
/* create datastruct for fp */
  struct f_tifp_s * fsp = NULL;
  fsp = malloc( sizeof( struct f_tifp_s ));
  if (NULL == fsp) {
    fprintf (stderr, "could not alloc mem for fsp\n");
    exit(EXIT_FAILURE);
  };
  fsp->functionp = function;
  f->ftype = f_tifp;
  f->fu.ftifp = fsp;

}


/* adds a function to struct funcu * f
 * @param f already allocated struct funcu * pointer
 * @param function adress of function
 * @param fname name of function as combined message (already allocated)
 * @param tag tag
 */
void _helper_add_fsp_tifp_tag(struct funcu * f, ret_t (* function)(TIFF *, tag_t), char * fname, tag_t tag) {
  struct f_tifp_tag_s * fsp = NULL;
  fsp = malloc( sizeof( struct f_tifp_tag_s ));
  if (NULL == fsp) {
    fprintf (stderr, "could not alloc mem for fsp\n");
    exit(EXIT_FAILURE);
  };
  fsp->a = tag;
  fsp->functionp = function;
  f->ftype = f_tifp_tag;
  f->fu.ftifp_tag = fsp;
}


/* adds a function to struct funcu * f
 * @param f already allocated struct funcu * pointer
 * @param function adress of function
 * @param fname name of function as combined message (already allocated)
 * @param tag tag
 * @param v param a for function
 */
void _helper_add_fsp_tifp_tag_uint(struct funcu * f, ret_t (* function)(TIFF *, tag_t, unsigned int), char * fname, tag_t tag, unsigned int v) {
  /* create datastruct for fp */
  struct f_tifp_tag_uint_s * fsp = NULL;
  fsp = malloc( sizeof( struct f_tifp_tag_uint_s ));
  if (NULL == fsp) {
    fprintf (stderr, "could not alloc mem for fsp\n");
    exit(EXIT_FAILURE);
  };
  fsp->a = tag;
  fsp->b = v;
  fsp->functionp = function;
  f->ftype = f_tifp_tag_uint;
  f->fu.ftifp_tag_uint = fsp;
}


/* adds a function to struct funcu * f
 * @param f already allocated struct funcu * pointer
 * @param function adress of function
 * @param fname name of function as combined message (already allocated)
 * @param tag tag
 * @param count_of_values count of parameters for function
 * @param rp pointer to parameter values
 */
void _helper_add_fsp_tifp_tag_uint_uintp(struct funcu * f,  ret_t (* function)(TIFF *, tag_t, int, unsigned int *), char * fname, tag_t tag, int count_of_values, unsigned int * rp) {
  /* create datastruct for fp */
  printf("count of values = %i\n", count_of_values);
  struct f_tifp_tag_int_uintp_s * fsp = NULL;
  fsp = malloc( sizeof( struct f_tifp_tag_int_uintp_s ));
  if (NULL == fsp) {
    fprintf (stderr, "could not alloc mem for fsp\n");
    exit(EXIT_FAILURE);
  };
  fsp->a = tag;
  fsp->b = count_of_values;
  fsp->c = rp;
  fsp->functionp = function;
  f->ftype = f_tifp_tag_int_uintp;
  f->fu.ftifp_tag_int_uintp = fsp;
}

/* adds a function to struct funcu * f
 * @param f already allocated struct funcu * pointer
 * @param function adress of function
 * @param fname name of function as combined message (already allocated)
 * @param tag tag
 * @param l param a for function
 * @param r param b for function
 */
void _helper_add_fsp_tifp_tag_uint_uint(struct funcu * f, ret_t (* function)(TIFF *, tag_t, unsigned int, unsigned int), char * fname, tag_t tag, unsigned int l, unsigned int r) {
  /* create datastruct for fp */
  struct f_tifp_tag_uint_uint_s * fsp = NULL;
  fsp = malloc( sizeof( struct f_tifp_tag_uint_uint_s ));
  if (NULL == fsp) {
    fprintf (stderr, "could not alloc mem for fsp\n");
    exit(EXIT_FAILURE);
  };
  fsp->a = tag;
  fsp->b = l;
  fsp->c = r;
  fsp->functionp = function;
  f->ftype = f_tifp_tag_uint_uint;
  f->fu.ftifp_tag_uint_uint = fsp;
}



/* stack function for parser */
void i_push (unsigned int i) {
  if (parser_state.i_stackp >= 40) {
    fprintf(stderr, "stackoverflow in i_stack\n");
    exit(EXIT_FAILURE);
  }
  parser_state.i_stack[parser_state.i_stackp++] = i;
}
/* stack function for parser */
unsigned int i_pop () {
  if (parser_state.i_stackp <= 0) {
    fprintf(stderr, "stackunderflow in i_stack\n");
    exit(EXIT_FAILURE);
  }
  return parser_state.i_stack[--parser_state.i_stackp];
}

/* function to clean an execution plan */
void clean_plan () {
  /*executionentry_t * last = plan.last; */
  executionentry_t * entry = plan.start;
  if (NULL != entry) {
    while (entry->next) {
      executionentry_t * next = entry->next;
      free (entry->name);
      free (entry);
      entry = next;
    }
  }
  plan.last = NULL;
  plan.start = NULL;
}



/* helper function for parser */
tag_t settag( tag_t tag) { parser_state.tag=tag; return tag; }
/* helper function for parser */
tag_t gettag( ) { return parser_state.tag;}
int incrlineno() {
  parser_state.lineno++; 
#ifdef DEBUG
  printf("##lineno=%i\n", parser_state.lineno);
#endif
  return parser_state.lineno; 
}
/* helper function for parser */
int getlineno() { return parser_state.lineno;}

/*
int rule_tagorder_in_dsl( int tag ) {
  int prevtag = gettag();
#ifdef DEBUG
  printf ("tag '%i' found (prev tag was: '%i')\n", tag, prevtag);
#endif
  if (prevtag > tag) {
    fprintf(stderr, "tag '%i' should be greater than previous tag '%i'\n", tag, prevtag);
    exit( -1 );
  }
}
*/

/* helper function for parser */
void tagline() {  
#ifdef DEBUG
  printf("tagline\n");
#endif
}
/* helper function for parser */
void commentline() { 
#ifdef DEBUG
  printf("commentline\n");
#endif
}
/* helper function for parser */
/*
void rule_should_not_occure(char* s) {
#ifdef DEBUG
  printf("no parser rule matched after line %i (prev tag was %u): '%s'\n", getlineno(), gettag(), s);
#endif
}
*/

/* helper function for parser */
void set_mandatory() { 
#ifdef DEBUG
  printf("tag '%u' is mandatory\n", gettag());
#endif
  parser_state.req=mandatory;
}

/* helper function for parser */
void set_optional() {
#ifdef DEBUG
  printf("tag '%u' is optional\n", gettag());
#endif
  parser_state.req=optional; 
}

/* helper function for parser */
void set_ifdepends() {
#ifdef DEBUG
  printf("tag '%u' is set if depends\n", gettag());
#endif
  parser_state.req=ifdepends; 
}

/* helper function for parser */
void set_optdepends() {
#ifdef DEBUG
  printf("tag '%u' is set optional depends\n", gettag());
#endif
  parser_state.req=optdepends; 
}

/* helper function for parser */
void set_range() { parser_state.val = range;}

/* helper function for parser */

void set_logical_or() { parser_state.val = logical_or;}

/* helper function for parser */

void set_only() { parser_state.val = only;}

/* helper function for parser */
void set_any() { parser_state.val = any;}

/* helper function for parser */
void set_any_reference() { parser_state.any_reference = 1;}

/* helper function for parser */
void reset_valuelist() {
  parser_state.valuelist = 0;
}
/* helper function for parser */
void incr_values () {
  parser_state.valuelist++;
}

/* this adds the config of a tagline to execution plan
 * HINT: order of calling arguments from stacks is IMPORTANT! */
void rule_addtag_config() {
#ifdef DEBUG
  printf( "try to match tagline at line %i\n", getlineno());
#endif
  char fname[MAXSTRLEN];
  int i;
  for (i= 0; i<MAXSTRLEN; i++) { fname[i]="\0"; }
  funcp f = NULL;
  f=malloc( sizeof( struct funcu ) );
  if (NULL == f) {
    fprintf (stderr, "could not alloc mem for f\n");
    exit(EXIT_FAILURE);
  };
  tag_t tag = parser_state.tag;
  f->tag=tag;
  /* HINT: order of evaluating last val and last req is IMPORTANT! */
  switch ( parser_state.val ) {
    case range: {
                  unsigned int r = i_pop();
                  unsigned int l = i_pop();
                  snprintf(fname, MAXSTRLEN-1, "tst_tag%u_%i_%s_%u_%u", parser_state.tag, parser_state.req, "range", l, r);
                  _helper_add_fsp_tifp_tag_uint_uint(f, &check_tag_has_value_in_range, fname, tag, l, r);
                  break;
                }
    case logical_or: {
                       int count_of_values = parser_state.valuelist;
                       snprintf(fname, MAXSTRLEN-1, "tst_tag%u_%i_%s_%i", parser_state.tag, parser_state.req, "logical_or", count_of_values); 
                       unsigned int * rp = NULL;
                       rp = malloc ( count_of_values * sizeof( int ) );
                       if (NULL == rp) {
                         fprintf (stderr, "could not alloc mem for rp\n");
                         exit(EXIT_FAILURE);
                       };
                       int i;
                       unsigned int * rnp=rp;
                       for (i=0; i<count_of_values; i++) {
                         *(rnp) = i_pop();
                         rnp++;
                       }
                       _helper_add_fsp_tifp_tag_uint_uintp(f, &check_tag_has_some_of_these_values, fname, tag, count_of_values, rp);
                       break;
                     }
    case only: {
                 int count_of_values = parser_state.valuelist;
                 if (1 == count_of_values) {
                   unsigned int v = i_pop();
                   snprintf(fname, MAXSTRLEN-1, "tst_tag%u_%i_%s_%u", parser_state.tag, parser_state.req, "only", v);
                   _helper_add_fsp_tifp_tag_uint(f, &check_tag_has_value, fname, tag, v);
                 } else { /* valuelist, pE. BitsPerSample */
                   snprintf(fname, MAXSTRLEN-1, "tst_tag%u_%i_%s_%i", parser_state.tag, parser_state.req, "onlym", count_of_values); 
                   unsigned int * rp = NULL;
                   rp = malloc ( count_of_values * sizeof( int ) );
                   if (NULL == rp) {
                     fprintf (stderr, "could not alloc mem for rp\n");
                     exit(EXIT_FAILURE);
                   };
                   int i;
                   unsigned int * rnp=rp;
                   for (i=0; i<count_of_values; i++) {
                     *(rnp) = i_pop();
                     rnp++;
                   }
                   _helper_add_fsp_tifp_tag_uint_uintp(f, &check_tag_has_valuelist, fname, tag, count_of_values, rp);
                 }
                 break;
               }
    case any: {
                snprintf(fname, MAXSTRLEN-1, "tst_tag%u_%i_%s", parser_state.tag, parser_state.req, "any");
                _helper_add_fsp_tifp_tag(f, &check_tag, fname, tag);
                break;
              }
  }
  /* set predicate if and only if lastreq = depends */
  /* HINT: order of evaluating last val and last req is IMPORTANT! */
  /* HINT: order of calling arguments from stacks is IMPORTANT! */
  switch ( parser_state.req ) {
    case ifdepends: {
                      funcp predicate = NULL;
                      predicate=malloc( sizeof( struct funcu ) );
                      if (NULL == predicate) {
                        fprintf (stderr, "could not alloc mem for pred\n");
                        exit(EXIT_FAILURE);
                      };
                      predicate->pred=NULL;
                      if (parser_state.any_reference == 0) {
                        unsigned int valreference = i_pop();
                        tag_t tagreference = i_pop();
                        printf("ifdepends %u references to %u.%u\n", tag, tagreference, valreference);
                        _helper_add_fsp_tifp_tag_uint(predicate, &check_tag_has_value_quiet, "predicate", tagreference, valreference);
                      } else { /* point to any reference */
                        tag_t tagreference = i_pop();
                        printf("ifdepends %u references to %u.any\n", tag, tagreference);
                        _helper_add_fsp_tifp_tag(predicate, &check_tag_quiet, "predicate", tagreference);
                      }
                      f->pred=predicate;
                      break;
                    }
    case mandatory: {
                      f->pred = NULL;
                      break;
                    }
    case optional: {
                     funcp predicate = NULL;
                     predicate=malloc( sizeof( struct funcu ) );
                      if (NULL == predicate) {
                        fprintf (stderr, "could not alloc mem for pred\n");
                        exit(EXIT_FAILURE);
                      };
                      predicate->pred=NULL;
                      _helper_add_fsp_tifp_tag(predicate, &check_tag_quiet, "predicate", tag);
                      f->pred=predicate;
                      break;
                   }
    case optdepends: {
                       funcp predicate = NULL;
                       predicate=malloc( sizeof( struct funcu ) );
                       if (NULL == predicate) {
                         fprintf (stderr, "could not alloc mem for pred\n");
                         exit(EXIT_FAILURE);
                       };
                       predicate->pred=NULL;
                       /* set predicate of predicate to check_tag */
                       funcp predicatepredicate = NULL;
                       predicatepredicate=malloc( sizeof( struct funcu ) );
                       if (NULL == predicatepredicate) {
                         fprintf (stderr, "could not alloc mem for pred\n");
                         exit(EXIT_FAILURE);
                       };
                       predicatepredicate->pred=NULL;
                       _helper_add_fsp_tifp_tag(predicatepredicate, &check_tag_quiet, "predicatepredicate", tag);
                       /* set rest of predicate */
                       predicate->pred = predicatepredicate;
                       if (parser_state.any_reference == 0) {
                         unsigned int valreference = i_pop();
                         tag_t tagreference = i_pop();
                         printf("optdepends %u references to %u.%u\n", tag, tagreference, valreference);
                         _helper_add_fsp_tifp_tag_uint(predicate, &check_tag_has_value_quiet, "predicate", tagreference, valreference);
                       } else { /* point to any reference */
                         tag_t tagreference = i_pop();
                         printf("optdepends %u references to %u.any\n", tag, tagreference);
                         _helper_add_fsp_tifp_tag(predicate, &check_tag_quiet, "predicate", tagreference);
                       }
                       f->pred=predicate;
                       break;

                     
    }
    default:
      printf("unknown parserstate.req, should not occure\n");
      exit(EXIT_FAILURE);
  }

#ifdef DEBUG
  printf("fname='%s'\n", fname);
#endif
  append_function_to_plan(f, fname);
  reset_valuelist();
  parser_state.any_reference = 0;
}


/* reset the parser state */
void reset_parser_state() {
  parser_state.lineno=1;
  parser_state.valuelist=0;
  parser_state.tag=0;
  parser_state.req=0;
  parser_state.val=0;
  parser_state.i_stackp=0;
  parser_state.any_reference=0;
  int i;
  for (i=0; i<MAXTAGS; i++) {
        parser_state.called_tags[i]= 0;
  }
}

/* include the PEG generated parser, see "man peg" for details */
#include "config_dsl.grammar.c"   /* yyparse() */

/* function to parse a config file from STDIN */
void parse_plan () {
  reset_parser_state();
  yycontext ctx;
  memset(&ctx, 0, sizeof(yycontext));

  parser_state.stream=stdin;
  while (yyparse(&ctx))     /* repeat until EOF */
    ;
  yyrelease(&ctx);

}

/* function to parse a config file from file stream */
void parse_plan_via_stream( FILE * file ) {
  reset_parser_state();
  yycontext ctx;
  memset(&ctx, 0, sizeof(yycontext));
  parser_state.stream=file;
  while (yyparse(&ctx))     /* repeat until EOF */
    ;
  yyrelease(&ctx);
}

/* set parse error
 * @param msg describes the details about what was going wrong
 * @param yytext gives context of the error
 */
void set_parse_error(char * msg, char * yytext) {
  fprintf(stderr, "%s at line %i (error at '%s')\n", msg, parser_state.lineno, yytext);
  exit(EXIT_FAILURE);
}
