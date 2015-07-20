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
#include "config_parser.h"
#include "check.h"

/* TODO: handle stuff like BITS PER SAMPLE with multiple values */
/* TODO: handle stuff like DATETIME or COPYRIGHT */

/*
#define DEBUG
*/


/* global vars */
parser_state_t parser_state;
executionplan_t plan;


#define YY_INPUT(buf, result, max_size)		\
  {							\
    int yyc= fgetc(parser_state.stream);		\
    result= (EOF == yyc) ? 0 : (*(buf)= yyc, 1);	\
    yyprintf((stderr, "<%c>", yyc));			\
  }                                                     


/* type specific calls of function pointers */
int call_fp(TIFF* tif, funcp fp) {
  int ret = 0;
  if (NULL != fp) {
    switch (fp->ftype) {
      case f_dummy: break;
      case f_void: 
                    {
                      f_t * function = NULL;
                      function = fp->fu.fvoidt;
                      assert(NULL != function);
                      assert(NULL != function->functionp);
                      ret = (function->functionp)(tif);
                      break;
                    }
      case f_int:
                    {
                      f_int_t * function = NULL;
                      function = fp->fu.fintt;
                      assert(NULL != function);
                      assert(NULL != function->functionp);
                      ret = (function->functionp)(tif, function->a); 
                      break;
                    }
      case f_intint:
                    {
                      f_intint_t * function = NULL;
                      function = fp->fu.fintintt;
                      assert(NULL != function);
                      assert(NULL != function->functionp);
#ifdef DEBUG
                      printf("debug: found a=%i b=%i\n", function->a, function->b);
#endif
                      ret = (function->functionp)(tif, function->a, function->b); 
                      break;
                    }
      case f_intintint:
                    {
                      f_intintint_t * function = NULL;
                      function = fp->fu.fintintintt;
                      assert(NULL != function);
                      assert(NULL != function->functionp);
#ifdef DEBUG
                      printf("debug: found a=%i b=%i c=%i\n", function->a, function->b, function->c);
#endif
                      ret = (function->functionp)(tif, function->a, function->b, function->c); 
                      break;
                    }
      case f_intintintp:
                    {
                      f_intintintp_t * function = NULL;
                      function = fp->fu.fintintintpt;
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
  assert( ret >= 0);
  return ret;
}

/* executes a plan (list) of functions, checks if predicate-function calls are
 * needed, too. */
int execute_plan (TIFF * tif) {
  executionentry_t * this_exe_p = plan.start;
  int is_valid = 0;
  while (NULL != this_exe_p) {
#ifdef DEBUG
    printf("execute: %s\n", this_exe_p->name);
#endif
    funcp fp;
    fp = this_exe_p->fu_p;
    if (NULL != fp) {
#ifdef DEBUG
      printf("execute: fp not null\n");
#endif
      if (NULL != fp->pred) { /* we have a predicate function, call it and
                                 decide if we continue or not */
        printf("execute: we have a predicate... ");
        if (0 != call_fp(tif, fp->pred)) {
          printf("execute: predicate was not successfull\n");
          /* the predicate was not successfull, skip check */
        } else { /* predicate was successful */
          printf("execute: predicate was successfull\n");
          parser_state.called_tags[fp->tag]++;
          is_valid+= call_fp (tif, fp );
        }
      } else { /* no predicate, call function */
#ifdef DEBUG
        printf("execute: we have no predicate\n");
#endif
        parser_state.called_tags[fp->tag]++;
        is_valid+=call_fp (tif, fp );
      }
    }
    this_exe_p = this_exe_p->next;
  }
  /* now we know which tags are already checked, we need add a rule to
   * forbidden all other tags */
  printf("check if forbidden tags are still existing\n");
  int tag;
  for (tag=0; tag<MAXTAGS; tag++) {
    if (0 == parser_state.called_tags[tag]) { /* only unchecked tags */
      if (0 != check_notag( tif, tag)) { /* check if tag is not part of tif */
        /* tag does not exist */
        tif_fails("tag %i should not be part of this TIF\n", tag);
        is_valid++;
      }
    } else { /* additional checks for already checked tags */ 
        is_valid += check_tag_has_valid_type( tif, tag);
        switch (tag) {
                case TIFFTAG_DATETIME: is_valid += check_datetime(tif);
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
void print_plan () {
  printf("print plan:\n");
  executionentry_t * this_exe_p = plan.start;
  while (NULL != this_exe_p) {
    printf("next action is: %s\n", this_exe_p->name);
    this_exe_p = this_exe_p->next;
  }
}

/* adds a function to an execution plan */
int append_function_to_plan (void * fp, const char * name ) {
  executionentry_t * entry = NULL;
  executionentry_t * last = NULL;
  entry = malloc ( sizeof(executionentry_t) );
  if (NULL == entry) {
    fprintf(stderr, "could not alloc memory for execution plan");
    exit(EXIT_FAILURE);
  }
  entry->next = NULL;
  entry->fu_p = fp;
  entry->name = malloc ( 30*sizeof(char) );
  if (NULL == entry->name) {
    fprintf(stderr, "could not alloc memory for execution plan");
    exit(EXIT_FAILURE);
  }
  assert(NULL != name);
  strncpy(entry->name, name, 30-1);
#ifdef DEBUG
  printf("entry has name:%s\n", entry->name);
#endif
  entry->result=-1;
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
    last = plan.last;
    assert(NULL != last);
    assert(NULL == last->next);
    last->next = entry;
    plan.last = entry;
  }
  return 0;
}

/* stack functions for parser */
void i_push (int i) {
  if (parser_state.i_stackp >= 40) {
    fprintf(stderr, "stackoverflow in i_stack\n");
    exit(EXIT_FAILURE);
  }
  parser_state.i_stack[parser_state.i_stackp++] = i;
}
int i_pop () {
  if (parser_state.i_stackp <= 0) {
    fprintf(stderr, "stackunderflow in i_stack\n");
    exit(EXIT_FAILURE);
  }
  return parser_state.i_stack[--parser_state.i_stackp];
}
void i_clear() {
  parser_state.i_stackp = 0;
}

/* function to clean an execution plan */
void clean_plan () {
  executionentry_t * last = plan.last;
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



int settag( int tag) { parser_state.tag=tag; return tag; }
int gettag( ) { return parser_state.tag;}
int incrlineno() {
  parser_state.lineno++; 
#ifdef DEBUG
  printf("##lineno=%i\n", parser_state.lineno);
#endif
  return parser_state.lineno; 
}
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
void tagline() {  
#ifdef DEBUG
  printf("tagline\n");
#endif
}
void commentline() { 
#ifdef DEBUG
  printf("commentline\n");
#endif
}
void rule_should_not_occure(char c) {
#ifdef DEBUG
  printf("no parser rule matched after line %i (prev tag was %i), '%c'\n", getlineno(), gettag(), c);
#endif
}
void set_mandatory() { 
#ifdef DEBUG
  printf("tag '%i' is mandatory\n", gettag());
#endif
  parser_state.req=mandatory; }
  void set_optional() {
#ifdef DEBUG
    printf("tag '%i' is optional\n", gettag());
#endif
    parser_state.req=optional; }
    void set_ifdepends() {
#ifdef DEBUG
      printf("tag '%i' is set if depends\n", gettag());
#endif
      parser_state.req=ifdepends; }
      void set_range() { parser_state.val = range;}
      void set_logical_or() { parser_state.val = logical_or;}
      void set_only() { parser_state.val = only;}
      void set_any() { parser_state.val = any;}
      void set_any_reference() { parser_state.any_reference = 1;}
      void reset_valuelist() {
        parser_state.valuelist = 0;
      }
void incr_values () {
  parser_state.valuelist++;
}

/* this adds the config of a tagline to execution plan */
/* HINT: order of calling arguments from stacks is IMPORTANT! */
void rule_addtag_config() {
#ifdef DEBUG
  printf( "try to match tagline at line %i\n", getlineno());
#endif
  char fname[30];
  funcp f = NULL;
  f=malloc( sizeof( funcp ) );
  if (NULL == f) {
    fprintf (stderr, "could not alloc mem for f\n");
    exit(EXIT_FAILURE);
  };
  int tag = parser_state.tag;
  f->tag=tag;
  /* HINT: order of evaluating last val and last req is IMPORTANT! */
  switch ( parser_state.val ) {
    case range: {
                  int r = i_pop();
                  int l = i_pop();
                  snprintf(fname, 29, "tst_tag%i_%i_%s_%i_%i", parser_state.tag, parser_state.req, "range", l, r);
                  /* create datastruct for fp */
                  struct f_intintint_s * fsp = NULL;
                  fsp = malloc( sizeof( struct f_intintint_s ));
                  if (NULL == fsp) {
                    fprintf (stderr, "could not alloc mem for fsp\n");
                    exit(EXIT_FAILURE);
                  };
                  fsp->a = tag;
                  fsp->b = l;
                  fsp->c = r;
                  fsp->functionp = &check_tag_has_value_in_range;
                  f->ftype = f_intintint;
                  f->fu.fintintintt = fsp;
                  break;
                }
    case logical_or: {
                       int count_of_values = parser_state.valuelist;
                       snprintf(fname, 29, "tst_tag%i_%i_%s_%i", parser_state.tag, parser_state.req, "logical_or", count_of_values); 
                       /* create datastruct for fp */
                       printf("count of values = %i\n", count_of_values);
                       struct f_intintintp_s * fsp = NULL;
                       fsp = malloc( sizeof( struct f_intintintp_s ));
                       if (NULL == fsp) {
                         fprintf (stderr, "could not alloc mem for fsp\n");
                         exit(EXIT_FAILURE);
                       };
                       int * rp = NULL;
                       rp = malloc ( count_of_values * sizeof( int ) );
                       if (NULL == rp) {
                         fprintf (stderr, "could not alloc mem for rp\n");
                         exit(EXIT_FAILURE);
                       };
                       int i;
                       int * rnp=rp;
                       for (i=0; i<count_of_values; i++) {
                         *(rnp) = i_pop();
                         rnp++;
                       }
                       fsp->a = tag;
                       fsp->b = count_of_values;
                       fsp->c = rp;
                       fsp->functionp = &check_tag_has_some_of_these_values;
                       f->ftype = f_intintintp;
                       f->fu.fintintintpt = fsp;
                       break;
                     }
    case only: {
                 int count_of_values = parser_state.valuelist;
                 if (1 == count_of_values) {
                   int v = i_pop();
                   snprintf(fname, 29, "tst_tag%i_%i_%s_%i", parser_state.tag, parser_state.req, "only", v);
                   /* create datastruct for fp */
                   struct f_intint_s * fsp = NULL;
                   fsp = malloc( sizeof( struct f_intint_s ));
                   if (NULL == fsp) {
                     fprintf (stderr, "could not alloc mem for fsp\n");
                     exit(EXIT_FAILURE);
                   };
                   fsp->a = tag;
                   fsp->b = v;
                   fsp->functionp = &check_tag_has_value;
                   f->ftype = f_intint;
                   f->fu.fintintt = fsp;
                 } else { /* valuelist, pE. BitsPerSample */
                   snprintf(fname, 29, "tst_tag%i_%i_%s_%i", parser_state.tag, parser_state.req, "onlym", count_of_values); 
                   /* create datastruct for fp */
                   printf("count of values = %i\n", count_of_values);
                   struct f_intintintp_s * fsp = NULL;
                   fsp = malloc( sizeof( struct f_intintintp_s ));
                   if (NULL == fsp) {
                     fprintf (stderr, "could not alloc mem for fsp\n");
                     exit(EXIT_FAILURE);
                   };
                   int * rp = NULL;
                   rp = malloc ( count_of_values * sizeof( int ) );
                   if (NULL == rp) {
                     fprintf (stderr, "could not alloc mem for rp\n");
                     exit(EXIT_FAILURE);
                   };
                   int i;
                   int * rnp=rp;
                   for (i=0; i<count_of_values; i++) {
                     *(rnp) = i_pop();
                     rnp++;
                   }
                   fsp->a = tag;
                   fsp->b = count_of_values;
                   fsp->c = rp;
                   fsp->functionp = &check_tag_has_valuelist;
                   f->ftype = f_intintintp;
                   f->fu.fintintintpt = fsp;
                 }
                 break;
               }
    case any: {
                snprintf(fname, 29, "tst_tag%i_%i_%s", parser_state.tag, parser_state.req, "any");
                /* create datastruct for fp */
                struct f_int_s * fsp = NULL;
                fsp = malloc( sizeof( struct f_int_s ));
                if (NULL == fsp) {
                  fprintf (stderr, "could not alloc mem for fsp\n");
                  exit(EXIT_FAILURE);
                };
                fsp->a = tag;
                fsp->functionp = &check_tag;
                f->ftype = f_int;
                f->fu.fintt = fsp;
                break;
              }
  }
  /* set predicate if and only if lastreq = depends */
  /* HINT: order of evaluating last val and last req is IMPORTANT! */
  /* HINT: order of calling arguments from stacks is IMPORTANT! */
  switch ( parser_state.req ) {
    case ifdepends: {
                      funcp predicate = NULL;
                      predicate=malloc( sizeof( funcp ) );
                      if (NULL == predicate) {
                        fprintf (stderr, "could not alloc mem for pred\n");
                        exit(EXIT_FAILURE);
                      };
                      predicate->pred=NULL;
                      if (parser_state.any_reference == 0) {
                        int valreference = i_pop();
                        int tagreference = i_pop();
                        printf("ifdepends references to %i.%i\n", tagreference, valreference);
                        struct f_intint_s * fsp = NULL;
                        fsp = malloc( sizeof( struct f_intint_s ));
                        if (NULL == fsp) {
                          fprintf (stderr, "could not alloc mem for pred fsp\n");
                          exit(EXIT_FAILURE);
                        };
                        fsp->a = tagreference;
                        fsp->b = valreference;
                        fsp->functionp = &check_tag_has_value;
                        predicate->ftype = f_intint;
                        predicate->fu.fintintt = fsp;
                      } else { /* point to any reference */
                        int tagreference = i_pop();
                        printf("ifdepends references to %i.any\n", tagreference);
                        struct f_int_s * fsp = NULL;
                        fsp = malloc( sizeof( struct f_int_s ));
                        if (NULL == fsp) {
                          fprintf (stderr, "could not alloc mem for pred fsp\n");
                          exit(EXIT_FAILURE);
                        };
                        fsp->a = tagreference;
                        fsp->functionp = &check_tag;
                        predicate->ftype = f_int;
                        predicate->fu.fintt = fsp;
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
                      predicate=malloc( sizeof( funcp ) );
                      if (NULL == predicate) {
                        fprintf (stderr, "could not alloc mem for pred\n");
                        exit(EXIT_FAILURE);
                      };
                      predicate->pred=NULL;
                      struct f_int_s * fsp = NULL;
                      fsp = malloc( sizeof( struct f_int_s ));
                      if (NULL == fsp) {
                        fprintf (stderr, "could not alloc mem for pred fsp\n");
                        exit(EXIT_FAILURE);
                      };
                      fsp->a = tag;
                      fsp->functionp = &check_tag;
                      predicate->ftype = f_int;
                      predicate->fu.fintt = fsp;
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
}

void reset_parser_state() {
  parser_state.lineno=0;
  parser_state.valuelist=0;
  parser_state.tag=-1;
  parser_state.req=0;
  parser_state.val=0;
  parser_state.i_stackp=0;
  parser_state.any_reference=0;
  int i;
  for (i=0; i<MAXTAGS; i++) {
        parser_state.called_tags[i]= 0;
  }
}

#include "config_dsl.grammar.c"   /* yyparse() */

void parse_plan () {
 clean_plan();
 reset_parser_state();
 parser_state.stream=stdin;
  while (yyparse())     /* repeat until EOF */
    ;
}

void parse_plan_via_stream( FILE * file ) {
  clean_plan();
  reset_parser_state();
  parser_state.stream=file;
  while (yyparse())     /* repeat until EOF */
    ;
}

