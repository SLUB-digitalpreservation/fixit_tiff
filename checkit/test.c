/* fixes broken TIFF Files
 * 
 * fixes unused tags in Baseline-TIFFs,
 * based on http://www.awaresystems.be/imaging/tiff/tifftags/baseline.html
 *
 * author: Andreas Romeyke, 2013
 * licensed under conditions of libtiff 
 */

#define YY_DEBUG
#include <stdio.h>      
#include <stdlib.h>

int lineno=0;
// struct to hold parsing configuration
int lasttag = -1;

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
  printf("no parser rule matched at line %i (prev tag was %i), '%c'\n", getlineno(), gettag(), c);
}
void rule_addtag_config() {printf( "try to match tagline at line %i\n", getlineno());
}
void rule_set_mandatory() { printf("tag '%i' is mandatory\n", gettag()); }
void rule_set_optional() { printf("tag '%i' is optional\n", gettag()); }
void rule_set_ifdepends() { printf("tag '%i' is set if depends\n", gettag()); }


#include "config_dsl.grammar.c"   /* yyparse() */
int main()
{
  while (yyparse())     /* repeat until EOF */
    ;
  return 0;
}

