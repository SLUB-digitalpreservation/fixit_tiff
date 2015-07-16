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







int lineno=0;
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
  funcp pred;
  int (*functionp)(funcp pred);
} f_t;

typedef struct f_int_s {
  funcp pred;
  int a;
  int (*functionp)(funcp pred, int a);
} f_int_t;

typedef struct f_intint_s {
  funcp pred;
  int a;
  int b;
  int (*functionp)(funcp pred, int a, int b);
} f_intint_t;

typedef enum { f_void, f_int, f_intint } ftype_t;

struct funcu {
  ftype_t ftype;
  union  {
    struct f_s * fvoidt;
    struct f_int_s * fintt;
    struct f_intint_s * fintintt;
  } fu;
};


int execute_plan ( executionplan_t * plan) {
  executionentry_t * this_exe_p = plan->start;
  while (NULL != this_exe_p) {
    printf("execute: %s\n", this_exe_p->name);
    funcp fp;
    fp = this_exe_p->fu_p;
    if (NULL != fp) {

      switch (fp->ftype) {
        case f_void: 
        {
                f_t * function = NULL;
                function = fp->fu.fvoidt;
                assert(NULL != function);
                assert(NULL != function->functionp);
                (function->functionp)(function->pred);
                break;
        }
        case f_int:
        {
                f_int_t * function = NULL;
                function = fp->fu.fintt;
                assert(NULL != function);
                assert(NULL != function->functionp);
                (function->functionp)(function->pred, function->a); 
                break;
        }
        case f_intint:
        {
                f_intint_t * function = NULL;
                function = fp->fu.fintintt;
                assert(NULL != function);
                assert(NULL != function->functionp);
                printf("debug: found a=%i b=%i\n", function->a, function->b);
                (function->functionp)(function->pred, function->a, function->b); 
                break;
        }
      }
    }
    this_exe_p = this_exe_p->next;
  }
}

int print_plan ( executionplan_t * plan) {
  printf("print plan:\n");
  executionentry_t * this_exe_p = plan->start;
  while (NULL != this_exe_p) {
     printf("next action is: %s\n", this_exe_p->name);
     this_exe_p = this_exe_p->next;
  }
}

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

static int check_range(funcp pred, int a, int b) {
  printf("check_range: a=%i, b=%i\n", a, b);
  return 0;
}

static int check_only(funcp pred, int a) {
  printf("check_only: a=%i\n", a);
  return 0;
}

void rule_addtag_config() {
        printf( "try to match tagline at line %i\n", getlineno());
        char fname[30];
        funcp f = NULL;
        f=malloc( sizeof( funcp ) );
        if (NULL == f) {
          fprintf (stderr, "could not alloc mem\n");
          exit(EXIT_FAILURE);
        };

        switch ( lastval ) {
          case range: {
                        int l = i_pop();
                        int r = i_pop();
                        snprintf(fname, 29, "tst_tag%i_%i_%s_%i_%i", gettag(), lastreq, "range", l, r);
                        /* create datastruct for fp */
                        struct f_intint_s * fsp = NULL;
                        fsp = malloc( sizeof( struct f_intint_s ));
                        if (NULL == fsp) {
                          fprintf (stderr, "could not alloc mem\n");
                          exit(EXIT_FAILURE);
                        };
                        fsp->a = l;
                        fsp->b = r;
                        fsp->pred = NULL;
                        fsp->functionp = &check_range;
                        f->ftype = f_intint;
                        f->fu.fintintt = fsp;
                        break;
                        }
          case logical_or: snprintf(fname, 29, "tst_tag%i_%i_%s_%i", gettag(), lastreq, "logical_or", i_pop()); break;
          case only: {
                        int v = i_pop();
                        snprintf(fname, 29, "tst_tag%i_%i_%s_%i", gettag(), lastreq, "only", v);
                        /* create datastruct for fp */
                        struct f_intint_s * fsp = NULL;
                        fsp = malloc( sizeof( struct f_int_s ));
                        if (NULL == fsp) {
                          fprintf (stderr, "could not alloc mem\n");
                          exit(EXIT_FAILURE);
                        };
                        fsp->a = v;
                        fsp->pred = NULL;
                        fsp->functionp = &check_only;
                        f->ftype = f_int;
                        f->fu.fintt = fsp;

                        break;
                        }
          case any: snprintf(fname, 29, "tst_tag%i_%i_%s", gettag(), lastreq, "any"); break;
        }
        printf("fname='%s'\n", fname);
        append_function_to_plan( &plan, f, fname);
}
void set_mandatory() { printf("tag '%i' is mandatory\n", gettag()); lastreq=mandatory; }
void set_optional() { printf("tag '%i' is optional\n", gettag()); lastreq=optional; }
void set_ifdepends() { printf("tag '%i' is set if depends\n", gettag()); lastreq=ifdepends; }
void set_range() { lastval = range;}
void set_logical_or() { lastval = logical_or;}
void set_only() { lastval = only;}
void set_any() { lastval = any;}


#include "config_dsl.grammar.c"   /* yyparse() */
int main()
{

  print_plan( &plan );
  while (yyparse())     /* repeat until EOF */
    ;
  print_plan( &plan );
  execute_plan( &plan );
  clean_plan( &plan );
  print_plan( &plan );
  return 0;
}

