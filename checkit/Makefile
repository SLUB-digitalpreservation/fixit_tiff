LIB+=-ltiff -lpcre -lm
SANITIZE=-fsanitize=undefined -fsanitize=shift \
-fsanitize=integer-divide-by-zero -fsanitize=unreachable \
-fsanitize=vla-bound -fsanitize=null -fsanitize=return \
-fsanitize=signed-integer-overflow 
#-fsanitize=leak
CFLAGS=-O2 -g -Wall $(SANITIZE)
GRAMMAR=config_dsl.grammar.c
OBJS=check_helper.o check_tag.o check_tag_has_value.o \
check_tag_has_value_in_range.o check_tag_has_some_of_these_values.o \
check_tag_has_valid_type.o check_datetime.o config_parser.o \
check_tag_has_valid_asciivalue.o

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
	$(CC) $(CFLAGS) $(INC) -o $@ $^ $(LIB)

test: $(OBJS) test.o
	$(CC) $(CFLAGS) $(INC) -o $@ $^ $(LIB)

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
