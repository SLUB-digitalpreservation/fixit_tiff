LIB+=-ltiff
CFLAGS=-O2 -g -Wall
GRAMMAR=config_dsl.grammar.c

# default target
#all: checkit_tiff test
all: test

# remove debugging symbols (smaller size)
#strip: all
#	strip --strip-all checkit_tiff

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $<

# produce parser via PEG
%.c: %.peg
	peg $< > $@

# default executable
#checkit_tiff: cleanup_datetime.o cleanup_baseline.o cleanup_order.o checkit_tiff.o
#	$(CC) $(CFLAGS) $(INC) -o $@ $^ $(LIB) 

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
	@rm -Rf doc/

.PHONY: all clean distclean strip
