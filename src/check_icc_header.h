#include <string.h>
#include <assert.h>
#include <stdio.h>

int parse_icc(unsigned long iccsize, char* iccdata, unsigned long errsize, char * errmessage);
int check_icc_header (const char * filename );
