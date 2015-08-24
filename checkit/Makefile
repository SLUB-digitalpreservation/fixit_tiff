### needs libtiff (>= v4)
###       libpcre (>=v3)

LIB+=-lpcre -ltiff -lm
SANITIZE?=-fsanitize=undefined -fsanitize=shift \
-fsanitize=integer-divide-by-zero -fsanitize=unreachable \
-fsanitize=vla-bound -fsanitize=null -fsanitize=return \
-fsanitize=signed-integer-overflow 
#-fsanitize=leak
CFLAGS= -g -Wall $(SANITIZE)
GRAMMAR=config_dsl.grammar.c
OBJS=check_helper.o check_tag.o check_tag_has_value.o \
check_tag_has_value_in_range.o check_tag_has_some_of_these_values.o \
check_tag_has_valid_type.o check_datetime.o \
check_tag_has_valid_asciivalue.o config_parser.o \
check_ifd.o

# default target
all: checkit_tiff test

# remove debugging symbols (smaller size)
#strip: all
#	strip --strip-all checkit_tiff

config_parser.c: config_dsl.grammar.c

config_dsl.grammar.c: config_dsl.grammar.peg

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $<

# produce parser via PEG
%.c: %.peg
	peg $< > $@

# default executable
checkit_tiff: $(OBJS) checkit_tiff.o
	$(CC) -o $@ $(CFLAGS) $(INC) $^ $(LIB)

checkit_tiff.exe: $(OBJS) checkit_tiff.o
	$(CC) -o $@ -L/home/romeyke/Downloads/tiff/tiff-4.0.4beta/libtiff/.libs/ -L/home/romeyke/Downloads/pcre-8.37/.libs/ -static $^ -ltiff -lpcre -lm 

test: $(OBJS) test.o
	$(CC) -o $@ $(CFLAGS) $(INC) $^ $(LIB)

test.c: $(GRAMMAR)

# doc
doc: ../common/doxygen.conf
	@doxygen ../common/doxygen.conf

# clean workdir
clean:
	@rm -f *.o
	@rm -f checkit_tiff test

# mrproper clean
distclean: clean
	@rm -f *~
	@rm -f core
	@rm -Rf doc/

.PHONY: all clean distclean strip
