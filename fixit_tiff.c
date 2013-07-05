/** fixes broken TIFF Files */
/* author: Andreas Romeyke, 2013 */
/* licensed under conditions of libtiff */

#include "fixit_tiff.h"

void help () {
	printf ("fixit_tiff broken_tiff_file corrected_tiff_file\n");
}

/** copy infile to outfile 
 * @param inf string with infile name 
 * @param outf string with outfile name 
 */
void copy_file (const char * inf, const char * outf) {
	FILE * in = fopen( inf, "rb");
	if (NULL == in) {
		fprintf(stderr, "could not open file '%s' for reading\n", inf);
		exit (-20);
	}
	FILE * out = fopen(outf, "wb");
	if (NULL == out) {
		fprintf(stderr, "could not open file '%s' for writing\n", outf);
		exit (-21);
	}
	/* first, copy infile to outfile */
    char            buffer[512];
    size_t          n;
    while ((n = fread(buffer, sizeof(char), sizeof(buffer), in)) > 0)
    {
        if (n != fwrite(buffer, sizeof(char), n, out)) {
            fprintf(stderr, "failure writing to file '%s'\n", outf);
            exit (-22);
		}
    }
    fclose(in);
    fclose(out);
}
	
    
/** corrects broken date string to expected format, see 
 * http://www.awaresystems.be/imaging/tiff/tifftags/datetime.html
 * @param broken_datetime string with wrong datetime
 * @return string with corrected datetime
 */
char * correct_datestring (char * broken_datetime) {
	int day;
	int month;
	int year;
	int hour;
	int min;
	int sec;
	/* TODO: if ret is wrong, you could try another rules to apply */
	/* RULE1: fix: '04.03.2010 09:59:17' => '04:03:2010 09:59:17' */
	int ret=sscanf(broken_datetime, "%02d.%02d.%04d%02d:%02d:%02d", &day, &month, &year, &hour, &min, &sec);
	if (ret != 6) {
		fprintf(stderr, "error in datetime parsing of string '%s', ret=%d, year=%04d, month=%02d, day=%02d, hour=%02d, min=%02d, sec=%02d\n", broken_datetime, ret, year, month, day, hour, min, sec);
		exit (-3);
	} 
	/* write corrected value to new string */
	char * fixed_date = NULL;
	fixed_date=malloc(sizeof(char) * 20); /* 20 comes from TIFF definition */
	if (NULL == fixed_date) {
		fprintf(stderr, "could not allocate memory for datetime conversion, abort\n");
		exit (-4);
	}
	int written = snprintf(fixed_date, 20, "%04d:%02d:%02d %02d:%02d:%02d", year, month, day, hour, min, sec);
	
	if (written != 19) {
		fprintf(stderr, "something wrong, instead 19 chars, %d chars were written\n", written);
		exit (-5);
	}
	return fixed_date;
}
	
/* main */
int main (int argc, char * argv[]) {
	if (argc != 3) {
		help();
		exit(-1);
	}
	const char *infilename = argv[1];
	const char *outfilename= argv[2];
	printf ("infile='%s', outfile='%s'\n", infilename, outfilename);
	copy_file (infilename, outfilename);
	/* load file */
	TIFF* tif = TIFFOpen(outfilename, "r+");
	if (NULL == tif) {
		fprintf( stderr, "file '%s' could not be opened\n", infilename);
		exit (-2);
	};
	/* find date-tag and fix it */
	printf("Before correction\n-----------------\n");	
	TIFFPrintDirectory(tif, stdout, TIFFPRINT_NONE);
	char *datetime;
	uint32 count;
	int found=TIFFGetField(tif, TIFFTAG_DATETIME, &datetime, &count);
	if (1==found) {
		printf("c=%u datetime:'%s'\n", count, datetime);
		/* correct TIFF DateTIME */
		TIFFSetField(tif, TIFFTAG_DATETIME, correct_datestring( datetime ));
		
    } else if (0 == found) {
		printf ("no datetime found!\n");
	}
	printf("After  correction\n-----------------\n");
	TIFFPrintDirectory(tif, stdout, TIFFPRINT_NONE);
	/* write data back */
	int written = TIFFRewriteDirectory(tif);
	if (1 != written) {
		fprintf(stderr, "something is wrong, tiffdir could not be written to file '%s'\n", outfilename);
		exit (-3);
	}
	/* write file */
	TIFFClose(tif);
	return 0;
}
	
