LIB+=-ltiff
CFLAGS=-O2 -g -Wall
GRAMMAR=config_dsl.grammar.c
OBJS=../fixit/tiff_helper.o check_tag.o check_tag_has_value.o \
check_tag_has_value_in_range.o check_tag_has_some_of_these_values.o \
check_tag_has_valid_type.o config_parser.o

# default target
all: checkit_tiff test

# remove debugging symbols (smaller size)
#strip: all
#	strip --strip-all checkit_tiff

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
