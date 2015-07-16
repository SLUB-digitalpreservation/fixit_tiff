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
int lineno=0;
int count_of_numbers=0;

// struct to hold parsing configuration
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

/* Definiton der Funktionszeiger auf die Checkfunktionen */


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

int check_range(TIFF* tif, int tag, int a, int b) {
  printf("check_range of tag %i : a=%i, b=%i\n", tag, a, b);
  return 0;
}

int check_has_tag_value(TIFF* tif, int tag, int value) {
  printf("check_has_tag_value: tag=%i, value=%i\n", tag, value);
  return 0;
}

int check_only(TIFF* tif, int tag, int a) {
  printf("check_only of tag %i: a=%i\n", tag, a);
  return 0;
}

int check_any(TIFF* tif, int tag) {
  printf("check_only of tag %i\n", tag);
  return 0;
}

/* type specific calls of function pointers */
int call_fp(TIFF* tif, funcp fp) {
  if (NULL != fp) {
    switch (fp->ftype) {
      case f_dummy: break;
      case f_void: 
        {
          f_t * function = NULL;
          function = fp->fu.fvoidt;
          assert(NULL != function);
          assert(NULL != function->functionp);
          (function->functionp)(tif);
          break;
        }
      case f_int:
        {
          f_int_t * function = NULL;
          function = fp->fu.fintt;
          assert(NULL != function);
          assert(NULL != function->functionp);
          (function->functionp)(tif, function->a); 
          break;
        }
      case f_intint:
        {
          f_intint_t * function = NULL;
          function = fp->fu.fintintt;
          assert(NULL != function);
          assert(NULL != function->functionp);
          printf("debug: found a=%i b=%i\n", function->a, function->b);
          (function->functionp)(tif, function->a, function->b); 
          break;
        }
         case f_intintint:
        {
          f_intintint_t * function = NULL;
          function = fp->fu.fintintintt;
          assert(NULL != function);
          assert(NULL != function->functionp);
          printf("debug: found a=%i b=%i c=%i\n", function->a, function->b, function->c);
          (function->functionp)(tif, function->a, function->b, function->c); 
          break;
        }
        default:
          fprintf(stderr, "call_fp() error, should not occure!\n");
          exit(EXIT_FAILURE);
    }
  }
}

/* executes a plan (list) of functions, checks if predicate-function calls are
 * needed, too. */
int execute_plan (TIFF * tif, executionplan_t * plan) {
  executionentry_t * this_exe_p = plan->start;
  while (NULL != this_exe_p) {
    printf("execute: %s\n", this_exe_p->name);
    funcp fp;
    fp = this_exe_p->fu_p;
    if (NULL != fp) {
      printf("execute: fp not null\n");
      if (NULL != fp->pred) { /* we have a predicate function, call it and
                                 decide if we continue or not */
        printf("execute: we have a predicate\n");
        if (0 != call_fp(tif, fp->pred)) {
          printf("execute: predicate was not successfull\n");
          /* the predicate was not successfull, skip check */
        } else { /* predicate was successful */
          printf("execute: predicate was successfull\n");
          call_fp (tif, fp );
        }
      } else { /* no predicate, call function */
        printf("execute: we have no predicate\n");
        call_fp (tif, fp );
      }
    }
    this_exe_p = this_exe_p->next;
  }
}

/* prints a plan (list) of functions */
int print_plan ( executionplan_t * plan) {
  printf("print plan:\n");
  executionentry_t * this_exe_p = plan->start;
  while (NULL != this_exe_p) {
     printf("next action is: %s\n", this_exe_p->name);
     this_exe_p = this_exe_p->next;
  }
}

/* adds a function to an execution plan */
int append_function_to_plan ( executionplan_t * plan, void * fp, const char * name ) {
  executionentry_t * entry = NULL;
  executionentry_t * last = NULL;
  assert (NULL != plan);
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
    printf("entry has name:%s\n", entry->name);
    entry->result=-1;
    assert(NULL != entry);
    if (NULL == plan->start) {
      printf(" add first to plan\n");
      plan->start = entry; 
      plan->last = entry;
      assert(NULL != plan->last);
      assert(NULL == plan->last->next);
    } else {
      printf(" add n-th to plan\n");
      assert(NULL != plan->last);
      assert(NULL == plan->last->next);
      last = plan->last;
      assert(NULL != last);
      assert(NULL == last->next);
      last->next = entry;
      plan->last = entry;
    }
    return 0;
}

/* stack functions for parser */
char * s_stack[40];
int s_stackp = 0;
void s_push (char * s) {
  if (s_stackp >= 40) {
    fprintf(stderr, "stackoverflow in s_stack\n");
    exit(EXIT_FAILURE);
  }
  s_stack[s_stackp++] = s;
}
char * s_pop () {
  if (s_stackp <= 0) {
    fprintf(stderr, "stackunderflow in s_stack\n");
    exit(EXIT_FAILURE);
  }
  return s_stack[--s_stackp];
}
void s_clear() {
  s_stackp = 0;
}

int i_stack[40];
int i_stackp = 0;
void i_push (int i) {
 if (i_stackp >= 40) {
    fprintf(stderr, "stackoverflow in i_stack\n");
    exit(EXIT_FAILURE);
  }

  i_stack[i_stackp++] = i;
}
int i_pop () {
 if (i_stackp <= 0) {
    fprintf(stderr, "stackunderflow in i_stack\n");
    exit(EXIT_FAILURE);
  }
  return i_stack[--i_stackp];
}
void i_clear() {
  i_stackp = 0;
}

/* function to clean an execution plan */
int clean_plan ( executionplan_t * plan) {
   executionentry_t * last = plan->last;
   executionentry_t * entry = plan->start; 
   while (entry->next) {
        executionentry_t * next = entry->next;
        free (entry->name);
        free (entry);
        entry = next;
   }
   plan->last = NULL;
   plan->start = NULL;
}


executionplan_t plan;

int lasttag = -1;
requirements_t lastreq = 0;
values_t lastval = 0;
int settag( int tag) { lasttag=tag; return tag; }
int gettag( ) { return lasttag;}
int incrlineno() {lineno++; printf("##lineno=%i\n", lineno); return lineno; }
int getlineno() { return lineno;}
int rule_tagorder_in_dsl( int tag ) {
  int prevtag = gettag();
  printf ("tag '%i' found (prev tag was: '%i')\n", tag, prevtag);
  if (prevtag > tag) {
    fprintf(stderr, "tag '%i' should be greater than previous tag '%i'\n", tag, prevtag);
    exit( -1 );
  }
}
void tagline() {  printf("tagline\n");}
void commentline() {  printf("commentline\n");}
void rule_should_not_occure(char c) {
  printf("no parser rule matched after line %i (prev tag was %i), '%c'\n", getlineno(), gettag(), c);
}
void set_mandatory() { printf("tag '%i' is mandatory\n", gettag()); lastreq=mandatory; }
void set_optional() { printf("tag '%i' is optional\n", gettag()); lastreq=optional; }
void set_ifdepends() { printf("tag '%i' is set if depends\n", gettag()); lastreq=ifdepends; }
void set_range() { lastval = range;}
void set_logical_or() { lastval = logical_or; i_push(count_of_numbers);}
void set_only() { lastval = only;}
void set_any() { lastval = any;}
void reset() {
  count_of_numbers = 0;
}
void incr_values () {
  count_of_numbers++;
}

/* this adds the config of a tagline to execution plan */
/* HINT: order of calling arguments from stacks is IMPORTANT! */
void rule_addtag_config() {
        
        printf( "try to match tagline at line %i\n", getlineno());
        char fname[30];
        funcp f = NULL;
        f=malloc( sizeof( funcp ) );
        if (NULL == f) {
          fprintf (stderr, "could not alloc mem for f\n");
          exit(EXIT_FAILURE);
        };
     
        /* HINT: order of evaluating last val and last req is IMPORTANT! */
        switch ( lastval ) {
          case range: {
                        int r = i_pop();
                        int l = i_pop();
                        int tag = gettag();
                        snprintf(fname, 29, "tst_tag%i_%i_%s_%i_%i", tag, lastreq, "range", l, r);
                        /* create datastruct for fp */
                        struct f_intintint_s * fsp = NULL;
                        fsp = malloc( sizeof( struct f_intint_s ));
                        if (NULL == fsp) {
                          fprintf (stderr, "could not alloc mem for fsp\n");
                          exit(EXIT_FAILURE);
                        };
                        fsp->a = tag;
                        fsp->b = l;
                        fsp->c = r;
                        fsp->functionp = &check_range;
                        f->ftype = f_intintint;
                        f->fu.fintintintt = fsp;
                        break;
                        }
          case logical_or: {
                        int count_of_values = i_pop();
                        printf("count of values = %i\n", count_of_values);
                        /* TODO */
                        int i;
                        for (i=0; i<count_of_values; i++) {
                          i_pop();
                        }
                        f->ftype = f_dummy;
                        snprintf(fname, 29, "tst_tag%i_%i_%s_%i", gettag(), lastreq, "logical_or", count_of_values); 
                        break;
                        }
          case only: {
                        int v = i_pop();
                        int tag = gettag();
                        snprintf(fname, 29, "tst_tag%i_%i_%s_%i", tag, lastreq, "only", v);
                        /* create datastruct for fp */
                        struct f_intint_s * fsp = NULL;
                        fsp = malloc( sizeof( struct f_intint_s ));
                        if (NULL == fsp) {
                          fprintf (stderr, "could not alloc mem for fsp\n");
                          exit(EXIT_FAILURE);
                        };
                        fsp->a = tag;
                        fsp->b = v;
                        fsp->functionp = &check_only;
                        f->ftype = f_intint;
                        f->fu.fintintt = fsp;

                        break;
                        }
          case any: {
                        int tag = gettag();
                        snprintf(fname, 29, "tst_tag%i_%i_%s", tag, lastreq, "any");
/* create datastruct for fp */
                        struct f_int_s * fsp = NULL;
                        fsp = malloc( sizeof( struct f_int_s ));
                        if (NULL == fsp) {
                          fprintf (stderr, "could not alloc mem for fsp\n");
                          exit(EXIT_FAILURE);
                        };
                        fsp->a = tag;
                        fsp->functionp = &check_any;
                        f->ftype = f_int;
                        f->fu.fintt = fsp;
                        break;
          }
        }
        /* set predicate if and only if lastreq = depends */
        /* HINT: order of evaluating last val and last req is IMPORTANT! */
        /* HINT: order of calling arguments from stacks is IMPORTANT! */
        switch ( lastreq ) {
          case ifdepends: {
                        int valreference = i_pop();
                        int tagreference = i_pop();
                        printf("ifdepends references to %i.%i\n", tagreference, valreference);
                        funcp predicate = NULL;
                        predicate=malloc( sizeof( funcp ) );
                        if (NULL == predicate) {
                          fprintf (stderr, "could not alloc mem for pred\n");
                          exit(EXIT_FAILURE);
                        };
                        predicate->pred=NULL;
                        struct f_intint_s * fsp = NULL;
                        fsp = malloc( sizeof( struct f_intint_s ));
                        if (NULL == fsp) {
                          fprintf (stderr, "could not alloc mem for pred fsp\n");
                          exit(EXIT_FAILURE);
                        };
                        fsp->a = tagreference;
                        fsp->b = valreference;
                        fsp->functionp = &check_has_tag_value;
                        predicate->ftype = f_intint;
                        predicate->fu.fintintt = fsp;
                        f->pred=predicate;
                        break;
          }
          default: f->pred = NULL;
        }

        printf("fname='%s'\n", fname);
        append_function_to_plan( &plan, f, fname);
        reset();
}


#include "config_dsl.grammar.c"   /* yyparse() */

/* *********** main ************* */

int main()
{
  TIFF * tif = NULL;
  printf("((( print empty plan )))\n");
  print_plan( &plan );
  printf("((( parse config file )))\n");
  while (yyparse())     /* repeat until EOF */
    ;
  printf("((( print execution plan )))\n");
  print_plan( &plan );
  printf("((( execute execution plan )))\n");
  execute_plan(tif, &plan );
  printf("((( clean execution plan )))\n");
  clean_plan( &plan );
  printf("((( print empty plan)))\n");
  print_plan( &plan );
  return 0;
}

