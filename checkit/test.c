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
  if (prevtag > tag)
    fprintf(stderr, "tag '%i' should be greater than previous tag '%i'\n", tag, prevtag);
    exit( -1 );
}

#define YY_DEBUG

#include "config_dsl.grammar.c"   /* yyparse() */
int main()
{
  while (yyparse())     /* repeat until EOF */
    ;
  return 0;
}

